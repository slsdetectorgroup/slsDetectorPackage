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
		ostringstream os;
		os << "Starting " << receiver_type;
		string message(os.str());	FILE_LOG(logINFO, message);
		return new UDPStandardImplementation();
	}
#ifdef REST
	else if (receiver_type == "REST"){
		ostringstream os;
		os << "Starting " << receiver_type;
		string message(os.str());	FILE_LOG(logINFO, message);
		return new UDPRESTImplementation();
	}
#endif
	else{
		ostringstream os;
		os << "[ERROR] UDP interface not supported, using standard implementation";
		string message(os.str());	FILE_LOG(logWARNING, message);
		return new UDPBaseImplementation();
	}
}


//#endif
