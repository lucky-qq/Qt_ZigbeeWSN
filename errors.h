/*
 *  Created by Roel Storms
 *
 *  All errors that can be thrown are declared here. Every error is a child of std::exception
 *
 */

#ifndef ERRORS_H
#define ERRORS_H


#define ERROR(name, message) \
class name : public std::exception \
{ \
public: \
	const char* what() const noexcept \
	{ \
		return message; \
	}; \
};

#include <exception>


ERROR(StartupError, "Failed to start gateway")

ERROR(HttpError, "HTTP error using libcurl")
ERROR(InvalidLogin, "Login to ipsum database failed, check password and username")

ERROR(XercesError, "An error in xerces occured")
ERROR(InvalidXMLError, "XML passed was invalid")
ERROR(IpsumError, "error occured in connection with ipsum")

ERROR(SerialError, "Couldn't open serial port, check if the device is connected correctly and if the port number is right")
ERROR(UnknownPacketType, "this type of packet can not be decoded or does not exist, check the datasheet for more information. If the packet type doesn't exist there must have been an undetected transmission error")
ERROR(UnknownDataType, "The datatype you chose for one of the fields is not recognized.")
ERROR(WebserviceInvalidCommand, "Command specified by the URL has not been recognized")
ERROR(DataNotAvailable, "Sensordata requested from this package is not available. Probably this node does not have such a sensor or hasn't transmitted that data.")
ERROR(InvalidPacketType, "Invalid packet type")
ERROR(ZbCorruptedFrameData, "The framedata in this packet is invalid")
ERROR(ZbCorruptedPacket, "The packet structure is not what is expected.")

ERROR(CorruptedPacket, "Packet used to construct an object of type Incoming Packet is invalid (length, checksum, startbyte)");
ERROR(SqlError, "An unexpected error happend in connection with the sql database.");

ERROR(InvalidWSXML, "The XML supplied by the webservice request was invalid.");

#endif
