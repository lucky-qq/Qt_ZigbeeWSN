TEMPLATE = app
CONFIG += console
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11   -g
DEFINES +=IPSUM_DEBUG HTTP_DEBUG DEBUG_MAIN DEBUG   PACKET_DEBUG SENTPACKETS_DEBUG WS_DEBUG PACKET_LOGGING    ZBSENDER_DEBUG
#  XML_DEBUG SQL_DEBUG CONFIG_DEBUG
SOURCES += \
    packetqueue.cpp \
    packet.cpp \
    mainclass.cpp \
    main.cpp \
    SerialCom/packets/dataIOPacket.cpp \
    SerialCom/packets/incomingpacket.cpp \
    SerialCom/packets/libeladdnodepacket.cpp \
    SerialCom/packets/libeladdnoderesponse.cpp \
    SerialCom/packets/libelchangefreqpacket.cpp \
    SerialCom/packets/libelchangefreqresponse.cpp \
    SerialCom/packets/libelchangenodefreqpacket.cpp \
    SerialCom/packets/libelchangenodefreqresponse.cpp \
    SerialCom/packets/libeliopacket.cpp \
    SerialCom/packets/libelmaskrequest.cpp \
    SerialCom/packets/libelmaskresponse.cpp \
    SerialCom/packets/libelrequestiopacket.cpp \
    SerialCom/packets/outgoingpacket.cpp \
    SerialCom/packets/receivepacket.cpp \
    SerialCom/packets/transmitrequestpacket.cpp \
    SerialCom/packets/ZBPacket.cpp \
    SerialCom/ZBSender.cpp \
    SerialCom/ZBReceiver.cpp \
    SerialCom/connection.cpp \
    HTTP/ipsumuploadpacket.cpp \
    HTTP/ipsumpacket.cpp \
    HTTP/ipsumchangeinusepacket.cpp \
    HTTP/ipsumchangefreqpacket.cpp \
    HTTP/ipsum.cpp \
    HTTP/http.cpp \
    sqlite/sql.cpp \
    XML/XML.cpp \
    webservice/wspacket.cpp \
    webservice/webservice.cpp \
    webservice/mongoose.c \
    webservice/wsaddnodepacket.cpp \
    webservice/wsaddsensorspacket.cpp \
    webservice/wsrequestdatapacket.cpp \
    webservice/wschangefrequencypacket.cpp \
    sqlite/sqlite3.c \
    #SerialCom/packet_test.cpp \
    testclass.cpp \
    SerialCom/packets/transmitstatuspacket.cpp \
    SerialCom/packets/libelerrorpacket.cpp \
    config.cpp
    #config.cpp

HEADERS += \
    mainclass.h \
    packetqueue.h \
    packet.h \
    testclass.h \
    enums.h \
    errors.h \
    SerialCom/packets/dataIOPacket.h \
    SerialCom/packets/incomingpacket.h \
    SerialCom/packets/libeladdnodepacket.h \
    SerialCom/packets/libeladdnoderesponse.h \
    SerialCom/packets/libelchangefreqpacket.h \
    SerialCom/packets/libelchangefreqresponse.h \
    SerialCom/packets/libelchangenodefreqpacket.h \
    SerialCom/packets/libelchangenodefreqresponse.h \
    SerialCom/packets/libeliopacket.h \
    SerialCom/packets/libelmaskrequest.h \
    SerialCom/packets/libelmaskresponse.h \
    SerialCom/packets/libelpacket.h \
    SerialCom/packets/libelrequestiopacket.h \
    SerialCom/packets/outgoingpacket.h \
    SerialCom/packets/receivepacket.h \
    SerialCom/packets/transmitrequestpacket.h \
    SerialCom/packets/ZBPacket.h \
    SerialCom/ZBSender.h \
    SerialCom/ZBReceiver.h \
    SerialCom/connection.h \
    HTTP/ipsumuploadpacket.h \
    HTTP/ipsumpacket.h \
    HTTP/ipsumchangeinusepacket.h \
    HTTP/ipsumchangefreqpacket.h \
    HTTP/ipsum.h \
    HTTP/http.h \
    sqlite/sql.h \
    XML/XML.h \
    webservice/wspacket.h \
    webservice/webservice.h \
    webservice/mongoose.h \
    sentpackets.h \
    webservice/wsaddnodepacket.h \
    webservice/wsaddsensorspacket.h \
    webservice/wsrequestdatapacket.h \
    webservice/wschangefrequencypacket.h \
    sqlite/sqlite3.h \
    SerialCom/packets/transmitstatuspacket.h \
    SerialCom/packets/libelerrorpacket.h \
    config.h
    #config.h

unix:!macx:!symbian: LIBS += -lcrypto -lcurl -lxerces-c -lboost_thread -lpthread -lboost_date_time -ldl

OTHER_FILES += \
    sqlite/sqlite3.o
