/*
 *	Created by Roel Storms on 28/01/2013  
 *
 *	TODO: change all getters to return const references
 *	TODO: rewrite packet classes so that information is not stored 2ce, once in encoded packet and once in fields such as networkAddress, frameID etc. Make getters and setters that manipulate the encoded packet directly
 *	TODO: clean up main
 *	TODO: add thread with webservice
 *	TODO: add queue that sends packets from webservice to main + condition variable
 *	TODO: Adding a sensorGroup and the necessary sensors to ipsum in 1 function
 *	TODO: Saving url and XML of packet that should be sent to ipsum to the sql database whenever ipsum is down
 *	TODO: Create function to add new nodes to sql database
 *	TODO: create a system so that an installer can add nodes
 *	TODO: Change frequency of sampling of certain sensors and connect it to packages from the webservice 
 *	TODO: When data comes in we need to upload this data to each corresponding sensor in ipsum. Some how you should have knowledge about which sensor to upload to.
 *
 */


#include "mainclass.h"


int main(int argc, char* argv[])
{
    std::cout << "Main" << std::endl;


    MainClass mainClass(argc, argv, 60);
    mainClass();

    /*
    std::vector<unsigned char> zigbeeAddress64bit{0x00, 0X13, 0xA2, 0X00, 0x40, 0X69, 0x73, 0x6C};
    std::vector<SensorType> sensors{TEMP,PRES,BAT};

    LibelAddNodePacket libelAddNodePacket(zigbeeAddress64bit, sensors);
    std::cout << libelAddNodePacket << std::endl;

    Connection con;
    int cd = con.openPort(atoi(argv[1]), 9600);
    while (true)
    {
        write(cd, libelAddNodePacket.getEncodedPacket().data(), libelAddNodePacket.getEncodedPacket().size());
        sleep(3);
    }
    */

	return 0;
}

