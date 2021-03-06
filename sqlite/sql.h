#ifndef SQL_H
#define SQL_H
#include "sqlite3.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "../enums.h"
#include "../errors.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "mainclass.h"

#define BOOST_DISABLE_ASSERTS 	// If you do not define this then the boost assert macro will be included and its name collides with my TestClass::assert function 
#include <boost/lexical_cast.hpp>
#include <typeinfo>
#include "../SerialCom/packets/libeliopacket.h"



class Sql
{
	private:
	sqlite3 *db;	
	std::vector<std::map<std::string, std::string>> selectReturn;
	public:
    Sql();
	~Sql();
	static int callbackWrapper(void *thisPointer, int argc, char **argv, char **azColName);
	int callback(int argc, char **argv, char **azColName);
	std::vector<std::map<std::string, std::string>> executeQuery(std::string aQuery);

    void addMeasurement(LibelIOPacket * packet);

	void addIpsumPacket(const std::string& url, const std::string& XML);
	std::vector<std::map<std::string, std::string>> retrieveIpsumPacket();
	void removeIpsumPacket(int id);

	
	//Table nodes: 	nodeID (int), zigbee64bitaddress(text), zigbee16bitaddress(text), temperatureID(int), humidityID(int), pressureID(int),
	//		batteryID (int), co2ID(int), anemoID(int), pluvioID(int)
    bool makeNewNode(int installationID, int nodeID, std::string zigbee64BitAddress);
    bool deleteNode(std::string zigbee64BitAddress);

    // void keepSensors(std::vector<SensorType>, int nodeID);
    std::string updateSensorsInNode(int nodeID, SensorType sensorType, int sensorID);
    std::string getNodeAddress(int nodeID) throw (SqlError);
    int getNodeID(std::string zigbeeAddress64Bit) throw (SqlError);
    int getInstallationID(std::string zigbeeAddress64Bit) throw (SqlError);
    std::map<SensorType, int> getSensorsFromNode(int nodeID) throw (SqlError);

};

#endif
