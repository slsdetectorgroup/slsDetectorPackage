//#ifdef SLS_RECEIVER_UDP_FUNCTIONS
/********************************************//**
 * @file slsReceiverUDPFunctions.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include <iostream>
#include <string.h>
using namespace std;


#include "UDPInterface.h"
#include "UDPBaseImplementation.h"
#include "UDPStandardImplementation.h"
#ifdef REST
#include "UDPRESTImplementation.h"
#endif


using namespace std;

UDPInterface * UDPInterface::create(string receiver_type){
	
	if (receiver_type == "standard"){
		FILE_LOG(logINFO) << "Starting " << receiver_type;
		return new UDPStandardImplementation();
	}
#ifdef REST
	else if (receiver_type == "REST"){
		FILE_LOG(logINFO) << "Starting " << receiver_type;
		return new UDPRESTImplementation();
	}
#endif
	else{
		FILE_LOG(logWARNING) << "[ERROR] UDP interface not supported, using standard implementation";
		return new UDPBaseImplementation();
	}
}


//#endif
