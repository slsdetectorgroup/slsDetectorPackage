//#ifdef SLS_RECEIVER_UDP_FUNCTIONS
/********************************************//**
 * @file slsReceiverUDPFunctions.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/




#include <signal.h>  		// SIGINT
#include <sys/stat.h> 		// stat
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), shut down
#include <arpa/inet.h>		// sock_addr_in, htonl, INADDR_ANY
#include <stdlib.h>			// exit()
#include <iomanip>			//set precision
#include <sys/mman.h>		//munmap

#include <string.h>
#include <iostream>
using namespace std;

#include "UDPInterface.h"
#include "UDPBaseImplementation.h"

#include "moench02ModuleData.h"
#include "gotthardModuleData.h"
#include "gotthardShortModuleData.h"


using namespace std;

UDPInterface * UDPInterface::create(string receiver_type){
	
	if (receiver_type == "standard")
		return new UDPBaseImplementation();
	else{
		cout << "[ERROR] UDP interface not supported, using standard implementation" << endl;
		return new UDPBaseImplementation();
	}
}


//#endif
