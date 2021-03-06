/*
 *	Created by Roel Storms
 *
 *	Does all the IPSUM connection and uses the XML class to generate and interprete the XML
 *  The Ipsum thread uses this class to handle it's packets.
 *
 *  Several functions in this class are no longer used nor tested. They are still in here
 *  might someone need similar code so they can reuse it.
 */

#ifndef HTTP_H
#define HTTP_H

#define CURL_STATICLIB
extern "C" {
#include <curl/curl.h>
}

#include <string.h>

#include <stdio.h>
#include <openssl/sha.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <iterator>
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include "ipsumuploadpacket.h"
#include "ipsumchangeinusepacket.h"
#include "ipsumchangefreqpacket.h"

#include "../XML/XML.h"
#include "../errors.h"
#include "config.h"

class Http
{
	private:
		CURL *curl;	
		std::string urlBase;
		std::string personalKey;
		std::string token;
		boost::posix_time::ptime tokenExpireTime;
		std::string curlReply;
		int httpError;
		XML xmlParser;
        std::string PersonalKey;
	public: 
        Http();

		~Http();

		static size_t standardReplyWrapper(void *buffer, size_t size, size_t nmemb, void *obj);
		size_t write_data(void *buffer, size_t size, size_t nmemb);
		static size_t headerHandlerWrapper(void *buffer, size_t size, size_t nmemb, void *obj);
		size_t headerHandler(void *buffer, size_t size, size_t nmemb);

		std::string sendGet(std::string urlAddition, size_t (*callback) (void*, size_t, size_t, void*)) throw (HttpError);
		std::string sendPost(std::string urlAddition, std::string data, size_t (*callback) (void *, size_t, size_t, void *)) throw (HttpError);
		std::string generateCode(std::string url);
		std::string calculateDestination(int userID, int installationID = -1, int sensorGroupID = -1, int sensorID = -1);
		std::string toBase64(std::string input);
        void uploadData(IpsumUploadPacket * packet) throw (HttpError);
        void uploadData(std::string aSensorType, std::string destinationBase64, std::vector<std::pair<std::string, double> > input) throw (HttpError);      
		bool login() throw (HttpError, InvalidLogin);       
		void setUserRights(std::string entity, int userID, int rights) throw (HttpError);       
		std::string getEntity(std::string destinationBase64) throw (HttpError);	        
		std::string getChildren(std::string destinationBase64) throw (HttpError);	     
        void setToken(std::string token);
		std::string selectData(std::string destinationBase64, std::vector<std::string> fields) throw (HttpError);
		std::string ipsumInfo() throw (HttpError);
        //std::string createNewSensor(std::string sensorGroupIDValue, std::string nameValue, std::string dataNameValue, std::string descriptionValue, std::string inuseValue) throw (HttpError);
        //std::string createNewType(std::string aName, std::vector<std::pair<std::string, std::string> > aListOfFields) throw (HttpError);
        std::string changeSensorGroup(std::string newXML);
        std::string changeSensor(std::string newXML);

        void changeInUse(IpsumChangeInUsePacket * packet) throw(HttpError, InvalidXMLError);
        void changeFreq(IpsumChangeFreqPacket * packet) throw (HttpError, InvalidXMLError);

		//std::string createNewSensorGroup(const std::string& installationIDValue, const std::string& nameValue, const std::string& descriptionValue, const std::string& inuseValue) throw (HttpError);
		
};


#endif	
