#include "mainclass.h"



MainClass::MainClass(int argc, char * argv[], int packetExpirationTime) : packetExpirationTime(packetExpirationTime)
{
    //Config::loadConfig("configFile.txt");
    int fd = open("errors.txt");
    dup2(fd, STDERR_FILENO);

    socket = new Http("http://ipsum.groept.be", "a31dd4f1-9169-4475-b316-764e1e737653");

    try
    {
        socket->ipsumInfo();
    }
    catch(HttpError)
    {
        std::cerr << "Could not connect to Ipsum" << std::endl;
        //return 1;
    }


    std::cout << "argc: " << argc << std::endl;
    if(argc != 2)
    {
        std::cerr << "also provide the port number" << std::endl;
        //return 1;
    }
    db = new Sql("../zigbee.dbs");
    con = new Connection();
    int connectionDescriptor = con->openPort(atoi(argv[1]), 9600);

    addNodeSentPackets = new SentPackets<LibelAddNodePacket *, LibelAddNodeResponse *>;
    changeFreqSentPackets = new SentPackets<LibelChangeFreqPacket *, LibelChangeFreqResponse *>;
    wsQueue = new PacketQueue();
    zbReceiveQueue = new PacketQueue();
    localZBReceiveQueue = new std::queue<Packet *>;
    localWSQueue = new std::queue<Packet *>;

    mainConditionVariable = new std::condition_variable;
    conditionVariableMutex = new std::mutex;

    zbSenderQueue = new PacketQueue();
    zbSenderConditionVariableMutex = new std::mutex;
    zbSenderConditionVariable = new std::condition_variable;

    zbSender = new ZBSender(connectionDescriptor, zbSenderConditionVariableMutex, zbSenderConditionVariable, zbSenderQueue);
    zbSenderThread = new boost::thread(boost::ref(*zbSender));

    wsConditionVariable = new std::condition_variable;
    wsConditionVariableMutex = new std::mutex;

    zbReceiver = new ZBReceiver(connectionDescriptor, conditionVariableMutex, mainConditionVariable, zbReceiveQueue);
    zbReceiverThread = new boost::thread(boost::ref(*zbReceiver));

    webService = new Webservice (wsQueue, mainConditionVariable, conditionVariableMutex, wsConditionVariable, wsConditionVariableMutex);

    ipsumSendQueue = new PacketQueue();
    ipsumReceiveQueue = new PacketQueue();
    ipsumConditionVariable = new std::condition_variable;
    ipsumConditionVariableMutex = new std::mutex;
    localIpsumSendQueue = new std::queue<Packet *>;
    localIpsumReceiveQueue = new std::queue<Packet *>;

    sentZBPackets = new std::queue<Packet *>;

    ipsum = new Ipsum(ipsumSendQueue, ipsumReceiveQueue, conditionVariableMutex, mainConditionVariable, ipsumConditionVariableMutex, ipsumConditionVariable);
    ipsumThread = new boost::thread(boost::ref(*ipsum));

    localZBSenderQueue = new std::vector<Packet *>;
}

MainClass::~MainClass()
{
    delete socket;
    delete con;
    delete db;

    delete addNodeSentPackets;
    delete changeFreqSentPackets;

    delete mainConditionVariable;
    delete conditionVariableMutex;

    delete zbSenderQueue;
    delete zbSenderConditionVariableMutex;
    delete zbSenderConditionVariable;
    delete zbSender;
    delete zbSenderThread;

    delete zbReceiveQueue;
    delete localZBReceiveQueue;
    delete zbReceiver;

    delete wsQueue;
    delete localWSQueue;
    delete webService;

    delete ipsumSendQueue;
    delete ipsumReceiveQueue;
    delete ipsumConditionVariable;
    delete ipsumConditionVariableMutex;
    delete localIpsumSendQueue;
    delete localIpsumReceiveQueue;
    delete sentZBPackets;
    delete ipsum;

    delete localZBSenderQueue;
//	delete wsThread;

}

void MainClass::operator() ()
{
    std::cout << "going into main while loop" << std::endl;

/*
    std::cout << "sending add node packet" << std::endl;

    std::vector<SensorType> sensors{TEMP, BAT, PRES};
    std::vector<unsigned char> zigbee64BitAddress{0x00, 0x13, 0xA2, 0x00, 0x40, 0x69, 0x73, 0x7c};
    LibelAddNodePacket * packet =  new LibelAddNodePacket(zigbee64BitAddress, sensors);
    std::cout << "packet to be sent: " << *packet << std::endl;
    zbSenderQueue->addPacket(dynamic_cast<Packet *> (packet));

    addNodeSentPackets->addPacket(packet);
    {
        //std::lock_guard<std::mutex> lg(*zbSenderConditionVariableMutex);
        //zbSenderConditionVariable->notify_all();
    }
    std::cout << "zbSender notified" << std::endl;
*/

    while(true)
    {
        //checkExpiredPackets();

        {	// Scope of unique_lock
            std::unique_lock<std::mutex> uniqueLock(*conditionVariableMutex);
            mainConditionVariable->wait(uniqueLock, [this]{ return ((!zbReceiveQueue->empty()) || (!wsQueue->empty() || (!ipsumReceiveQueue->empty()))); });
            std::cout << "mainconditionvariable notification received" << std::endl;
            while(!zbReceiveQueue->empty())
            {
                localZBReceiveQueue->push(zbReceiveQueue->getPacket());
                std::cout << "adding ZBPacket to local ZBReceiverQueue" << std::endl;
            }

            while(!wsQueue->empty())
            {
                std::cout << "type of ws packet from wsQueue: " << typeid(wsQueue->getPacket()).name() << std::endl;
                std::cout << "adding WSPacket to local WSQueue" << std::endl;
                localWSQueue->push(wsQueue->getPacket());
            }

            while(!ipsumReceiveQueue->empty())
            {
                localIpsumReceiveQueue->push(ipsumReceiveQueue->getPacket());
                std::cout << "adding IpsumPacket to local IpsumReceiveQueue" << std::endl;
            }
        }
        // Shared queue is no longer locked, now ready to process the packets
        Packet * packet;
        while(!localZBReceiveQueue->empty())
        {
            packet = localZBReceiveQueue->front();
            localZBReceiveQueue->pop();
            std::cout << "popped ZBPacket from local ZBQueue, type:" << typeid(packet).name() << std::endl;
            if(packet->getPacketType() == ZB_LIBEL_IO)
            {
                std::cout << "ZB_LIBEL_IO received in main" << std::endl;
                libelIOHandler(packet);
            }
            else if (packet->getPacketType() == ZB_LIBEL_MASK_RESPONSE)
            {
                std::cout << "ZB_LIBEL_MASK_RESPONSE received in main" << std::endl;
                libelMaskResponseHandler(packet);
            }
            else if (packet->getPacketType() == ZB_LIBEL_CHANGE_FREQ_RESPONSE)
            {
                std::cout << "ZB_LIBEL_CHANGE_FREQ_RESPONSE received in main" << std::endl;
                libelChangeFreqResponseHandler(packet);
            }
            else if (packet->getPacketType() == ZB_LIBEL_CHANGE_NODE_FREQ_RESPONSE)
            {
                std::cout << "ZB_LIBEL_CHANGE_NODE_FREQ_RESPONSE received in main" << std::endl;
                libelChangeNodeFreqResponseHandler(packet);
            }
            else if (packet->getPacketType() == ZB_LIBEL_ADD_NODE_RESPONSE)
            {
                std::cout << "ZB_LIBEL_ADD_NODE_RESPONSE received in main" << std::endl;
                libelAddNodeResponseHandler(packet);
            }
        }

        while(!localWSQueue->empty())
        {
            packet = localWSQueue->front();
            localWSQueue->pop();
            std::cout << "popped WSPacket from local WSQueue, type:" << typeid(packet).name() << std::endl;
            if(packet->getPacketType() == WS_COMMAND)
            {
                std::cout << "WS_PACKET received in main" << std::endl;
                webserviceHandler(packet);
            }
        }

        while(!localIpsumReceiveQueue->empty())
        {
            packet = localIpsumReceiveQueue->front();
            localIpsumReceiveQueue->pop();
            /*
            if(typeid(packet) ==  typeid(IpsumPacket *))
            {
                std::cout << "Ipsum_PACKET received in main" << std::endl;

                std::cout << "post data: "  << (dynamic_cast<WSPacket *> (packet))->getRequestData() << std::endl;
            }
            */
        }


    }
zbReceiverThread->join();
}

void MainClass::checkExpiredPackets()
{
    std::vector<LibelAddNodePacket *> expiredAddNodePackets = addNodeSentPackets->findExpiredPacket(packetExpirationTime);
    for( auto it = expiredAddNodePackets.begin(); it < expiredAddNodePackets.end(); ++it )
    {
        std::cerr << "LibelAddNodePacket received no reply. " << (*it) <<  std::endl;
        addNodeSentPackets->removePacket(*it);
    }
}

std::string MainClass::ucharVectToString(const std::vector<unsigned char>& ucharVect)
{
    std::stringstream stream;
    for(auto it = ucharVect.begin(); it < ucharVect.end(); ++it)
    {
        stream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex  << (int) (*it);
    }
    return stream.str();
}

void MainClass::libelIOHandler(Packet * packet)
{
    LibelIOPacket * libelIOPacket = dynamic_cast<LibelIOPacket *> (packet);

    std::vector<unsigned char> zigbee64BitAddress = libelIOPacket->getZigbee64BitAddress();

    localZBSenderQueue->erase(std::remove_if(localZBSenderQueue->begin(), localZBSenderQueue->end(), [&zigbee64BitAddress, this](Packet * packet) {
            TransmitRequestPacket * zbPacket = dynamic_cast<TransmitRequestPacket *>(packet);
            if(zbPacket == NULL)
            {
                std::cerr << "localZBSenderQueue had invalid packet type (MainClass)" << std::endl;
            }
            else
            {
                if(zbPacket->getZigbee64BitAddress() == zigbee64BitAddress)
                {
                    zbSenderQueue->addPacket(dynamic_cast<Packet *> (zbPacket));
                    return true;
                }
            }}), localZBSenderQueue->end());

    {
        std::lock_guard<std::mutex> lgSender(*zbSenderConditionVariableMutex);
        zbSenderConditionVariable->notify_all();
        std::cout << "zbsender notified from libeliohandler" << std::endl;
    }


    std::string zigbee64BitAddressString(ucharVectToString(zigbee64BitAddress));
    int nodeID, installationID;
    std::map<SensorType, int> availableSensors;
    std::cout << "zigbee64BitAddressString: " << zigbee64BitAddressString << std::endl;
    try
    {
        std::cout << "before getNodeID" << std::endl;
        nodeID = db->getNodeID(zigbee64BitAddressString);
        std::cout << "before getInstallationID" << std::endl;
        installationID = db->getInstallationID(zigbee64BitAddressString);
        std::cout << "before getSensorsFromNode" << std::endl;
        availableSensors = db->getSensorsFromNode(nodeID);
    }
    catch (SqlError)
    {
        std::cerr << "Could not upload data since this sensor was not known to the sql db" << std::endl;
        return;
    }
    std::map<SensorType, float> sensorData = libelIOPacket->getSensorData();

    std::vector<std::tuple<SensorType, int, float>> data;

    for(auto it = sensorData.begin(); it != sensorData.end(); ++it)
    {
        auto sensorField = availableSensors.find(it->first);
        if(sensorField == availableSensors.end())
        {
            std::cerr << "Data received from sensor which could not be found in database"  << std::endl;
        }
        else
        {
            std::cout << "sensor data that will be uploaded: " << it->first << std::endl;
            data.push_back(std::tuple<SensorType, int, float>(it->first, sensorField->second, it->second ));
        }

    }

    delete packet;


    IpsumUploadPacket * ipsumUploadPacket = new IpsumUploadPacket(installationID, nodeID, data);

    std::vector<std::tuple<SensorType, int, float> > retreivedData = ipsumUploadPacket->getData();
    for(auto it = retreivedData.begin(); it < retreivedData.end(); ++it)
    {
        std::cout << "sensortypes in retreived data: " << std::get<0>(*it) << std::endl;
    }


    ipsumSendQueue->addPacket(dynamic_cast<Packet*> (ipsumUploadPacket));
    std::cout << "ipsumuploadpacket added" << std::endl;
    std::lock_guard<std::mutex> lg(*ipsumConditionVariableMutex);
    ipsumConditionVariable->notify_all();

}

void MainClass::libelMaskResponseHandler(Packet * packet)
{
    LibelMaskResponse * libelMaskResponse = dynamic_cast<LibelMaskResponse *> (packet);

    delete libelMaskResponse;
}


void MainClass::libelChangeFreqResponseHandler(Packet * packet)
{
    LibelChangeFreqResponse * libelChangeFreqResponse = dynamic_cast<LibelChangeFreqResponse *> (packet);
    LibelChangeFreqPacket * libelChangeFreqPacket = changeFreqSentPackets->retrieveCorrespondingPacket(libelChangeFreqResponse);

    if(libelChangeFreqPacket != nullptr)
    {
        changeFreqSentPackets->removePacket(libelChangeFreqPacket);

        int installationID = db->getInstallationID(ucharVectToString(libelChangeFreqResponse->getZigbee64BitAddress()));
        int sensorGroupID = db->getNodeID(ucharVectToString(libelChangeFreqResponse->getZigbee64BitAddress()));
        std::map<SensorType, int> sensors = db->getSensorsFromNode(sensorGroupID); // sensors is a vector of sensorType + ipsum ID
        std::map<SensorType, int> frequencies = libelChangeFreqResponse->getFrequencies(); // frequencies is a map of sensortype + frequency(interval)
        std::vector<std::pair<int, int> > ipsumFreqVector;
        for(auto it = sensors.begin(); it != sensors.end(); ++it)
        {
            auto found = frequencies.find(it->first);
            if (found != frequencies.end())
            {
                ipsumFreqVector.push_back(std::pair<int, int> (it->second, found->second));      // adding ipsumId and frequency to the output vector
            }
            else
            {
                std::cerr << "MainClass: A sensor frequency has been changed for an unknown sensor" << std::endl;
            }
        }
        IpsumChangeFreqPacket * ipsumChangeFreqPacket = new IpsumChangeFreqPacket(installationID, sensorGroupID, ipsumFreqVector);

        ipsumSendQueue->addPacket(ipsumChangeFreqPacket);
        std::lock_guard<std::mutex> lg(*ipsumConditionVariableMutex);
        ipsumConditionVariable->notify_all();
        std::cout << "ChangeFreqResponse handled and ipsum packet created" << std::endl;
    }
    std::cout << "exiting libelChangeFreqResponseHandler" << std::endl;

    delete libelChangeFreqResponse;
}

void MainClass::libelChangeNodeFreqResponseHandler(Packet * packet)
{
    LibelChangeNodeFreqResponse * libelChangeNodeFreqResponse = dynamic_cast<LibelChangeNodeFreqResponse *> (packet);

    delete libelChangeNodeFreqResponse;
}

void MainClass::libelAddNodeResponseHandler(Packet * packet)
{
    std::cout << "entering libelAddNodeResponseHandler()" << std::endl;
    LibelAddNodeResponse * libelAddNodeResponse = dynamic_cast<LibelAddNodeResponse *> (packet);
    std::cout << "entering libelAddNodeResponseHandler()2" << std::endl;
    //LibelAddNodePacket * libelAddNodePacket = addNodeSentPackets->retrieveCorrespondingPacket(libelAddNodeResponse);
    std::cout << "entering libelAddNodeResponseHandler()3" << std::endl;
    //if(libelAddNodePacket != nullptr)
    {
        //addNodeSentPackets->removePacket(libelAddNodePacket);
        std::cout << "removed libelAddNodePacket from sentQueue" << std::endl;
        int installationID = db->getInstallationID(ucharVectToString(libelAddNodeResponse->getZigbee64BitAddress()));
        int sensorGroupID = db->getNodeID(ucharVectToString(libelAddNodeResponse->getZigbee64BitAddress()));
        IpsumChangeInUsePacket * ipsumChangeInUsePacket = new IpsumChangeInUsePacket(installationID, sensorGroupID, true);
        std::cout << "adding ipsumpacket" << std::endl;
        ipsumSendQueue->addPacket(ipsumChangeInUsePacket);
        std::lock_guard<std::mutex> lg(*ipsumConditionVariableMutex);
        ipsumConditionVariable->notify_all();
        std::cout << "AddNodeResponse handled and ipsum packet created" << std::endl;
    }
    std::cout << "exiting libelAddNodeResponseHandler" << std::endl;

    delete libelAddNodeResponse;
}


void MainClass::webserviceHandler(Packet * packet)
{
    WSPacket * wsPacket = dynamic_cast<WSPacket *> (packet);
    switch(wsPacket->getRequestType())
    {
        case CHANGE_FREQUENCY:
            std::cout << "CHANGE_FREQUENCY request being handled" << std::endl;
            changeFrequencyHandler(wsPacket);
            break;
        case ADD_NODE:
            std::cout << "ADD_NODE request being handled" << std::endl;
            addNodeHandler(wsPacket);

            break;
        case ADD_SENSOR:
            std::cout << "ADD_SENSOR request being handled" << std::endl;
            try
            {
                addSensorHandler(wsPacket);
            }
            catch(InvalidWSXML)
            {
                std::cerr << "invalid XML in webservice request" << std::endl;
                // Could send a reply to the client by using a queue going to the webservice.
            }

        break;
        case REQUEST_DATA:
            requestIOHandler(wsPacket);
                break;
        default:
             std::cerr << "unrecognized packet" << std::endl;

    }
    delete wsPacket;
}

void MainClass::requestIOHandler(WSPacket * wsPacket) throw (InvalidWSXML)
{
    XML XMLParser;
    xercesc::DOMDocument * doc = XMLParser.parseToDom( wsPacket->getRequestData() );
    char * temp;

    xercesc::DOMElement * docElement = doc->getDocumentElement();
    xercesc::DOMElement * nextElement;
    nextElement = docElement->getFirstElementChild();

    int sensorGroupID = -1;

    XMLCh * sensorString = xercesc::XMLString::transcode("sensorID");
    XMLCh * sensorGroupIDString = xercesc::XMLString::transcode("sensorGroupID");

    std::vector<int> sensors;      // IpsumID + frequency (interval) in seconds

    while(nextElement != NULL)
    {
        if(xercesc::XMLString::compareIString(nextElement->getTagName(), sensorGroupIDString) == 0)
        {
            temp = xercesc::XMLString::transcode(nextElement->getTextContent());
            sensorGroupID = boost::lexical_cast<int>(std::string(temp));
            xercesc::XMLString::release(&temp);
        }
        else if(xercesc::XMLString::compareIString(nextElement->getTagName(), sensorString) == 0)
        {

                temp = xercesc::XMLString::transcode(nextElement->getTextContent());
                sensors.push_back(boost::lexical_cast<int>(temp));
                xercesc::XMLString::release(&temp);

        }
        else
        {
            throw InvalidWSXML();
        }
        nextElement = nextElement->getNextElementSibling();
    }

    std::string zigbee64BitAddress = db->getNodeAddress(sensorGroupID);
    std::map<SensorType,int> sensorsFromDB = db->getSensorsFromNode(sensorGroupID);

    std::vector<SensorType> sensorTypes;    // vector of SensorTypes used to construct a LibelRequestIOPacket
    for(auto sensorsIt = sensors.begin(); sensorsIt < sensors.end(); ++sensorsIt)
    {
        for(auto sensorsFromDBIt = sensorsFromDB.begin(); sensorsFromDBIt != sensorsFromDB.end(); ++sensorsFromDBIt)
        {
            if(sensorsFromDBIt->second == (*sensorsIt))
            {
                sensorTypes.push_back(sensorsFromDBIt->first);  // interval / 10 since ZB works with 10s as a unit of time
            }
        }

    }

    LibelRequestIOPacket * libelRequestIOPacket = new LibelRequestIOPacket(std::vector<unsigned char>(zigbee64BitAddress.begin(), zigbee64BitAddress.end()), sensorTypes);

    localZBSenderQueue->push_back(libelRequestIOPacket);
    //zbSenderQueue->addPacket(libelRequestIOPacket);
    //std::lock_guard<std::mutex> lg(*zbSenderConditionVariableMutex);
    //zbSenderConditionVariable->notify_all();

}

void MainClass::changeFrequencyHandler(WSPacket * wsPacket) throw (InvalidWSXML)
{
    char * temp;
    XML XMLParser;
    xercesc::DOMDocument * doc = XMLParser.parseToDom(wsPacket->getRequestData());

    xercesc::DOMElement * docElement = doc->getDocumentElement();
    xercesc::DOMElement * nextElement;
    nextElement = docElement->getFirstElementChild();

    int sensorGroupID = -1;

    XMLCh * sensorString = xercesc::XMLString::transcode("sensorID");
    XMLCh * sensorIDString = xercesc::XMLString::transcode("sensorID");
    XMLCh * sensorGroupIDString = xercesc::XMLString::transcode("sensorGroupID");
    XMLCh * frequencyString = xercesc::XMLString::transcode("frequency");

    std::vector<std::pair<int, int> > frequencies;      // IpsumID + frequency (interval) in seconds

    while(nextElement != NULL)
    {
        if(xercesc::XMLString::compareIString(nextElement->getTagName(), sensorGroupIDString) == 0)
        {
            temp = xercesc::XMLString::transcode(nextElement->getTextContent());
            sensorGroupID = boost::lexical_cast<int>(std::string(temp));
            xercesc::XMLString::release(&temp);

        }
        if(xercesc::XMLString::compareIString(nextElement->getTagName(), sensorString) == 0)
        {
            std::pair<int, int> freq;
            xercesc::DOMElement * child;
            child = nextElement->getFirstElementChild();

            if(xercesc::XMLString::compareIString(child->getTagName(), sensorIDString) == 0)
            {
                temp = xercesc::XMLString::transcode(child->getTextContent());
                freq.first = boost::lexical_cast<int>(temp);
                xercesc::XMLString::release(&temp);

            }
            else
            {
                throw InvalidWSXML();
            }

            child = child->getNextElementSibling();

            if(xercesc::XMLString::compareIString(child->getTagName(), frequencyString) == 0)
            {
                temp = xercesc::XMLString::transcode(child->getTextContent());
                freq.second = boost::lexical_cast<int> (temp);

                xercesc::XMLString::release(&temp);
            }
            else
            {
                throw InvalidWSXML();
            }
            std::cout << "adding sensor to vector" << std::endl;
            frequencies.push_back(freq);
        }
        else
        {
            std::cerr << "invalid XML" << std::endl;
            throw InvalidWSXML();
        }

        nextElement = nextElement->getNextElementSibling();
    }

    std::string zigbee64BitAddress = db->getNodeAddress(sensorGroupID);
    std::map<SensorType,int> sensors = db->getSensorsFromNode(sensorGroupID);

    std::vector<std::pair<SensorType, int> > newFrequencies;    // SensorType + frequency(interval) in seconds
    for(auto it = frequencies.begin(); it < frequencies.end(); ++it)
    {
        for(auto sensorsIt = sensors.begin(); sensorsIt != sensors.end(); ++it)
        {
            if(sensorsIt->second == it->first)
            {
                newFrequencies.push_back(std::pair<SensorType, int>(sensorsIt->first, it->second/10));  // interval / 10 since ZB works with 10s as a unit of time
            }
        }

    }

    LibelChangeFreqPacket * libelChangeFreqPacket = new LibelChangeFreqPacket(std::vector<unsigned char>(zigbee64BitAddress.begin(), zigbee64BitAddress.end()), newFrequencies);

    localZBSenderQueue->push_back(libelChangeFreqPacket);

    //zbSenderQueue->addPacket(libelChangeFreqPacket);
    //std::lock_guard<std::mutex> lg(*zbSenderConditionVariableMutex);
    //zbSenderConditionVariable->notify_all();

    changeFreqSentPackets->addPacket(libelChangeFreqPacket);
    std::cout << "end of addnodehandler()" << std::endl;
}

void MainClass::addNodeHandler(WSPacket * wsPacket) throw (InvalidWSXML)
{
    char * temp;
    XML XMLParser;
    xercesc::DOMDocument * doc = XMLParser.parseToDom(wsPacket->getRequestData());

    std::string installationID, sensorGroupID, zigbeeAddress;

    xercesc::DOMElement * docElement = doc->getDocumentElement();
    xercesc::DOMElement * nextElement;
    nextElement = docElement->getFirstElementChild();

    XMLCh * installationIDString = xercesc::XMLString::transcode("installationID");
    XMLCh * sensorGroupIDString = xercesc::XMLString::transcode("sensorGroupID");
    XMLCh * zigbeeAddressString = xercesc::XMLString::transcode("zigbeeAddress");
    while(nextElement != NULL)
    {
        if(xercesc::XMLString::compareIString(nextElement->getTagName(), installationIDString) == 0)
        {
            temp = xercesc::XMLString::transcode(nextElement->getTextContent());
            installationID = std::string(temp);
            xercesc::XMLString::release(&temp);

        }
        else if(xercesc::XMLString::compareIString(nextElement->getTagName(), sensorGroupIDString) == 0)
        {
            temp = xercesc::XMLString::transcode(nextElement->getTextContent());
            sensorGroupID = std::string(temp);
            xercesc::XMLString::release(&temp);

        }
        else if(xercesc::XMLString::compareIString(nextElement->getTagName(), zigbeeAddressString) == 0)
        {
            temp = xercesc::XMLString::transcode(nextElement->getTextContent());
            zigbeeAddress = std::string(temp);
            xercesc::XMLString::release(&temp);
        }
        else
        {
            std::cerr << "invalid XML: " << std::endl;
            std::cerr << "textContent of invalid xml: " << std::string(xercesc::XMLString::transcode(nextElement->getTextContent())) << std::endl;
            std::cerr << "tagname of invalid XML: " << std::string(xercesc::XMLString::transcode(nextElement->getTagName())) << std::endl;
            throw InvalidWSXML();
        }

        nextElement = nextElement->getNextElementSibling();
    }


    std::string makeNewNode(int installationID, int nodeID, std::string zigbee64bitAddress);

    xercesc::XMLString::release(&installationIDString);
    xercesc::XMLString::release(&sensorGroupIDString);
    xercesc::XMLString::release(&zigbeeAddressString);

    db->makeNewNode(boost::lexical_cast<int> (installationID), boost::lexical_cast<int> (sensorGroupID), zigbeeAddress);
}

void MainClass::addSensorHandler(WSPacket * wsPacket) throw (InvalidWSXML)
{
    char * temp;
    XML XMLParser;
    xercesc::DOMDocument * doc = XMLParser.parseToDom(wsPacket->getRequestData());

    int sensorGroupID = -1;
    std::map<SensorType, int> sensors;

    xercesc::DOMElement * docElement = doc->getDocumentElement();
    xercesc::DOMElement * nextElement;
    nextElement = docElement->getFirstElementChild();

    XMLCh * sensorGroupIDString = xercesc::XMLString::transcode("sensorGroupID");
    XMLCh * sensorString = xercesc::XMLString::transcode("sensor");
    XMLCh * sensorIDString = xercesc::XMLString::transcode("sensorID");
    XMLCh * sensorTypeString = xercesc::XMLString::transcode("sensorType");

    while(nextElement != NULL)
    {
        if(xercesc::XMLString::compareIString(nextElement->getTagName(), sensorGroupIDString) == 0)
        {
            temp = xercesc::XMLString::transcode(nextElement->getTextContent());
            sensorGroupID = boost::lexical_cast<int>(temp);
            xercesc::XMLString::release(&temp);

        }
        if(xercesc::XMLString::compareIString(nextElement->getTagName(), sensorString) == 0)
        {
            std::pair<SensorType, int> sensor;
            xercesc::DOMElement * child;
            child = nextElement->getFirstElementChild();

            if(xercesc::XMLString::compareIString(child->getTagName(), sensorIDString) == 0)
            {
                temp = xercesc::XMLString::transcode(child->getTextContent());
                sensor.second = boost::lexical_cast<int>(temp);
                xercesc::XMLString::release(&temp);

            }
            else
            {
                throw InvalidWSXML();
            }

            child = child->getNextElementSibling();

            if(xercesc::XMLString::compareIString(child->getTagName(), sensorTypeString) == 0)
            {
                temp = xercesc::XMLString::transcode(child->getTextContent());
                std::cout << "sensorType in XML from WS: " << temp << std::endl;
                sensor.first = stringToSensorType(temp);

                xercesc::XMLString::release(&temp);
            }
            else
            {
                throw InvalidWSXML();
            }
            std::cout << "adding sensor to vector" << std::endl;
            sensors.insert(sensor);

        }

        nextElement = nextElement->getNextElementSibling();
    }


    // Add these sensors to the right node in the database

    for(auto it = sensors.begin(); it != sensors.end(); ++it)
    {
        db->updateSensorsInNode(sensorGroupID, it->first, it->second);

    }

    // Send a LibelAddNodePacket
    std::string zigbee64BitAddressString = db->getNodeAddress(sensorGroupID);
    std::cout << "Zigbee64BitAddress for add node packet (from string)" << std::endl << zigbee64BitAddressString << std::endl;
    std::vector <unsigned char> zigbee64BitAddress = convertStringToVector(zigbee64BitAddressString);


    std::cout << "Zigbee64BitAddress for add node packet (from vector)" << std::endl;
    for(auto it = zigbee64BitAddress.begin(); it < zigbee64BitAddress.end(); ++it)
    {
        std::cout << std::uppercase << std::setw(2) << std::setfill('0') << std::hex  << (int) (*it) << " ";
    }

    std::map<SensorType,int> sensorsFromDB = db->getSensorsFromNode(sensorGroupID);
    std::vector<SensorType> sensorTypes;
    for(auto it = sensorsFromDB.begin(); it != sensorsFromDB.end(); ++it)
    {
        sensorTypes.push_back(it->first);
    }

    LibelAddNodePacket * packet =  new LibelAddNodePacket(zigbee64BitAddress, sensorTypes);

    //zbSenderQueue->addPacket(dynamic_cast<Packet *> (packet));
    localZBSenderQueue->push_back(dynamic_cast<Packet *> (packet));

    addNodeSentPackets->addPacket(packet);

    //std::lock_guard<std::mutex> lg(*zbSenderConditionVariableMutex);
    //zbSenderConditionVariable->notify_azigbeeAddressll();

}

SensorType MainClass::stringToSensorType(std::string sensorType) throw (InvalidWSXML)
{
    std::cout << "stringToSensorType" << std::endl;
    if(sensorType == "zigbeeTemp")
    {
        return TEMP;
    }
    else if (sensorType == "zigbeeHum")
    {
        return HUM;
    }
    else if (sensorType == "zigbeePres")
    {
        return PRES;
    }
    else if (sensorType == "zigbeeBat")
    {
        return BAT;
    }
    else if (sensorType == "zigbeeCO2")
    {
        return CO2;
    }
    else if (sensorType == "zigbeeAnemo")
    {
        return ANEMO;
    }
    else if (sensorType == "zigbeeVane")
    {
        return VANE;
    }
    else if (sensorType == "zigbeePluvio")
    {
        return PLUVIO;
    }
    else
    {
        throw InvalidWSXML();
    }
}

std::vector<unsigned char> MainClass::convertStringToVector(std::string input)
{
    std::vector<unsigned char> output;
    for(auto it = input.begin(); it != input.end(); it += 2)
    {
        std::string slice(it, it + 2);

        unsigned int x;
        std::stringstream ss;
        ss << std::hex << slice;
        ss >> x;
        // output it as a signed type

        output.push_back(x);
    }
    return output;
}
