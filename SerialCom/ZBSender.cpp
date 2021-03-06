#include "ZBSender.h"

ZBSender::ZBSender(bool * stop, int connectionDescriptor, std::mutex * zbSenderConditionVariableMutex, std::condition_variable * zbSenderConditionVariable, PacketQueue * zbSendQueue) : stop(stop), connectionDescriptor(connectionDescriptor), zbSenderConditionVariableMutex(zbSenderConditionVariableMutex), zbSenderConditionVariable(zbSenderConditionVariable), zbSendQueue(zbSendQueue)
{
    #ifdef ZBSENDER_DEBUG
        std::cout << "ZBSender constructor" << std::endl;
    #endif
    logFile.open("sentpacketlog.txt", std::ios::in | std::ios::app);
}

std::vector<unsigned char> ZBSender::escape(std::vector<unsigned char> data)
{
	std::vector<unsigned char> escapedData;
    #ifdef ZBSENDER_DEBUG
        std::cout << std::endl << "escaping packet" << std::endl;
    #endif
	escapedData.push_back(0X7E);
    #ifdef ZBSENDER_DEBUG
        std::cout << std::endl << "7E" ;
    #endif
	for(auto it = data.begin() + 1; it < data.end(); ++it)
	{
		if ((*it) == 0X7E || (*it) == 0X7D || (*it) == 0X11 || (*it) == 0X13)
		{
			escapedData.push_back(0x7D);
            #ifdef ZBSENDER_DEBUG
                std::cout << " " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex  << (int)0x7D;
            #endif
			escapedData.push_back((*it) ^ 0x20);
            #ifdef ZBSENDER_DEBUG
                std::cout << " " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex  << (int)((*it) ^ 0x20);
            #endif
        }
		else
		{
			escapedData.push_back(*it);
            std::cout << " " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex  << (int)(*it) ;
		}

	}
    std::cout << std::endl;
	return escapedData;
}

void ZBSender::operator() ()
{
    while(!(*stop))
	{
        std::unique_lock<std::mutex> uniqueLock(*zbSenderConditionVariableMutex);
        zbSenderConditionVariable->wait(uniqueLock, [this]{return (!zbSendQueue->empty() || (*stop));});
        std::cout << "zb sender out of wait" << std::endl;
        OutgoingPacket * packet;

        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time(); //use the clock

		while(!zbSendQueue->empty())
		{
            #ifdef ZBSENDER_DEBUG
                std::cout << "Sendable packet received in ZBSender" << std::endl;
            #endif
            packet = dynamic_cast<OutgoingPacket *> (zbSendQueue->getPacket());
            if(packet != 0)
            {
                #ifdef ZBSENDER_DEBUG
                    std::cout << "sending : " << *packet << std::endl;
                #endif
                auto data = escape(packet->getEncodedPacket());
                if (write(connectionDescriptor, (void*) data.data(),  data.size()) != (signed int) data.size())
                {
                    std::cerr << "In ZBSender::operator() () | write didn't return data.size()" << std::endl;
                }
                fsync(connectionDescriptor);

                #ifdef PACKET_LOGGING
                logFile << boost::posix_time::to_simple_string(now) << " : sent packet of type " << packet->getPacketType() << ": " << dynamic_cast<ZBPacket*> (packet) << std::endl;
                #endif

                switch(packet->getPacketType())
                {
                    case ZB_LIBEL_ADD_NODE:
                    case ZB_LIBEL_CHANGE_FREQ:

                        /*
                         *  Only usefull for packets which can be resent
                         *  These are change frequency and add node
                         */
                        packet->incrementNumberOfResends();
                        packet->setTimeOfLastSending(time(NULL));


                    break;

                    default:
                        delete packet;

                }
                #ifdef ZBSENDER_DEBUG
                    std::cout << "packet succesfully sent" << std::endl;
                #endif
            }

		}
	}
}
