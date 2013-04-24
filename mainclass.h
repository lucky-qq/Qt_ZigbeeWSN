#ifndef MAINCLASS_H
#define MAINCLASS_H

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <boost/thread.hpp>
#include "./SerialCom/ZBReceiver.h"
#include "./SerialCom/ZBSender.h"
#include "./SerialCom/connection.h"
#include "./SerialCom/packets/libeliopacket.h"
#include "./SerialCom/packets/libelmaskrequest.h"
#include "./SerialCom/packets/libelmaskresponse.h"
#include "./SerialCom/packets/libeladdnoderesponse.h"
#include "./SerialCom/packets/libeladdnodepacket.h"
#include "./SerialCom/packets/libelchangefreqpacket.h"
#include "./SerialCom/packets/libelchangefreqresponse.h"
#include "./SerialCom/packets/libelchangenodefreqpacket.h"
#include "./SerialCom/packets/libelchangenodefreqresponse.h"

#include "./HTTP/http.h"
#include "./HTTP/ipsum.h"
#include "./HTTP/ipsumchangefreqpacket.h"
#include "./HTTP/ipsumchangeinusepacket.h"
#include "./HTTP/ipsumuploadpacket.h"

#include "packetqueue.h"
#include "sentpackets.h"

#include "./webservice/webservice.h"
#include <thread>
#include <ctime>
#include <typeinfo>

#include "sqlite/sql.h"
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

class MainClass
{
	private:
    int packetExpirationTime;

	Http * socket;
	
    SentPackets<LibelAddNodePacket *, LibelAddNodeResponse *>  * addNodeSentPackets;

    PacketQueue * zbReceiveQueue, * zbSenderQueue, * wsQueue, * ipsumSendQueue, * ipsumReceiveQueue;
    std::queue<Packet *> * localZBReceiveQueue, * localWSQueue, * localIpsumSendQueue, * localIpsumReceiveQueue, * sentZBPackets;

	std::condition_variable * mainConditionVariable, * ipsumConditionVariable, * zbSenderConditionVariable;
	std::mutex * conditionVariableMutex, * ipsumConditionVariableMutex, * zbSenderConditionVariableMutex;

	Connection * con;
	Sql * db;

	boost::thread * zbReceiverThread, * zbSenderThread, * ipsumThread;
	ZBReceiver * zbReceiver;
	ZBSender * zbSender;
	Webservice * webService;
	Ipsum * ipsum;


	public:
    MainClass(int argc, char * argv[], int packetExpirationTime);
	~MainClass();
	void operator () ();

    void checkExpiredPackets();
    std::string ucharVectToString(const std::vector<unsigned char> &ucharVect);

	void libelIOHandler(Packet * packet);
	void libelMaskResponseHandler(Packet * packet);
    void libelChangeFreqResponseHandler(Packet * packet);
	void libelChangeNodeFreqResponseHandler(Packet * packet);
	void libelAddNodeResponseHandler(Packet * packet);
		
	void webserviceHandler(Packet * packet);
    void requestIOHandler(WSPacket * wsPacket) throw (InvalidWSXML);
    void changeFrequencyHandler(WSPacket * wsPacket) throw (InvalidWSXML);
    void addNodeHandler(WSPacket * wsPacket) throw (InvalidWSXML);
    void addSensorHandler(WSPacket * wsPacket) throw (InvalidWSXML);


    SensorType stringToSensorType(std::string sensorType) throw (InvalidWSXML) ;
};

#endif
