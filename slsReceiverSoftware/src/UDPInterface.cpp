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
	
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	sprintf(cstreambuf, "Starting %s ", receiver_type.c_str());

	if (receiver_type == "standard"){
		FILE_LOG(logINFO, cstreambuf);
		return new UDPStandardImplementation();
	}
#ifdef REST
	else if (receiver_type == "REST"){
		FILE_LOG(logINFO, cstreambuf);
		return new UDPRESTImplementation();
	}
#endif
	else{
		FILE_LOG(logWARNING, "[ERROR] UDP interface not supported, using standard implementation");
		return new UDPBaseImplementation();
	}
}


//#endif
