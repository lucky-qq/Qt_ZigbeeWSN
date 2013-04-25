#include "ipsum.h"

Ipsum::Ipsum(PacketQueue * ipsumSendQueue, PacketQueue * ipsumReceiveQueue, std::mutex * mainConditionVariableMutex, std::condition_variable * mainConditionVariable, std::mutex * ipsumConditionVariableMutex, std::condition_variable * ipsumConditionVariable) : ipsumSendQueue(ipsumSendQueue), ipsumReceiveQueue(ipsumReceiveQueue), mainConditionVariableMutex(mainConditionVariableMutex), ipsumConditionVariableMutex(ipsumConditionVariableMutex), mainConditionVariable(mainConditionVariable), ipsumConditionVariable(ipsumConditionVariable)
{
	std::cout << "ipsum constructor" << std::endl;
	localIpsumSendQueue = new std::queue<Packet*>;
    http = new Http("http://ipsum.groept.be", "a31dd4f1-9169-4475-b316-764e1e737653");
}

Ipsum::~Ipsum()
{
	delete http;
	delete localIpsumSendQueue;
}

void Ipsum::operator()()
{
	std::cout << "Ipsum::operator()()" << std::endl;
    while(true)
    {
        std::unique_lock<std::mutex> uniqueLock(*ipsumConditionVariableMutex);
        ipsumConditionVariable->wait(uniqueLock, [this] {return (!ipsumSendQueue->empty());});

        std::cout << "ipsum condition variable notified" << std::endl;

        while(!ipsumSendQueue->empty())
        {
            localIpsumSendQueue->push(ipsumSendQueue->getPacket());
        }

        Packet * ipsumPacket;
        while(!localIpsumSendQueue->empty())
        {
            ipsumPacket = localIpsumSendQueue->front();
            localIpsumSendQueue->pop();

            switch(ipsumPacket->getPacketType())
            {
                case IPSUM_UPLOAD:
                    uploadDataHandler(dynamic_cast<IpsumUploadPacket *> (ipsumPacket));
                break;
                case IPSUM_CHANGE_IN_USE:
                    changeInUseHandler(dynamic_cast<IpsumChangeInUsePacket *> (ipsumPacket));
                break;
                default:
                std::cout << "packet not found" << std::endl;
                // Packet not recognized

            }
        }
    }
}


void Ipsum::uploadDataHandler(IpsumUploadPacket * packet)
{
    std::cout << "Ipsum::uploadDataHandler(IpsumUploadPacket * packet)" << std::endl;
    try
    {
        http->uploadData(packet);
    }
    catch(HttpError)
    {
        std::cerr << "Error trying to upload data to ipsum" << std::endl;
    }
    delete packet;
}

void Ipsum::changeInUseHandler(IpsumChangeInUsePacket * packet)
{
    std::cout << "Ipsum::changeInUseHandler(IpsumChangeInUsePacket * packet)" << std::endl;
    try
    {
        http->changeInUse(packet);
    }
    catch(HttpError)
    {
        std::cerr << "Error trying to changing inuse on ipsum" << std::endl;
    }
    delete packet;
}

void Ipsum::changeFrequencyHandler(IpsumChangeFreqPacket * packet)
{
    std::cout << "Ipsum::changeInUseHandler(IpsumChangeInUsePacket * packet)" << std::endl;
    try
    {
        http->changeFreq(packet);
    }
    catch(HttpError)
    {
        std::cerr << "Error trying to changing change frequency" << std::endl;
    }
    delete packet;
}
