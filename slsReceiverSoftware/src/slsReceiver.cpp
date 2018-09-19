/********************************************//**
 * @file slsReceiver.cpp
 * @short creates the UDP and TCP class objects
 ***********************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <getopt.h>

#include "slsReceiver.h"
#include "gitInfoReceiver.h"




slsReceiver::slsReceiver(int argc, char *argv[]):
		tcpipInterface (0) {

	// options
	std::map<std::string, std::string> configuration_map;
	int tcpip_port_no = 1954;
	std::string fname = "";
	int64_t tempval = 0;

	//parse command line for config
	static struct option long_options[] = {
			// These options set a flag.
			//{"verbose", no_argument,       &verbose_flag, 1},
			// These options don’t set a flag. We distinguish them by their indices.
			{"config",     	required_argument,  0, 'f'},
			{"rx_tcpport",  required_argument,  0, 't'},
			{"version",  	no_argument,  		0, 'v'},
			{"help",  		no_argument,       	0, 'h'},
			{0, 			0, 					0, 	0}
	};

	//initialize global optind variable (required when instantiating multiple receivers in the same process)
	optind = 1;
	// getopt_long stores the option index here.
	int option_index = 0;
	int c = 0;

	while ( c != -1 ){
		c = getopt_long (argc, argv, "hvf:t:", long_options, &option_index);

		// Detect the end of the options.
		if (c == -1)
			break;

		switch(c){

		case 'f':
			fname = optarg;
#ifdef VERYVERBOSE
			FILE_LOG(logDEBUG) << long_options[option_index].name << " " << optarg << endl;
#endif
			break;

		case 't':
			sscanf(optarg, "%d", &tcpip_port_no);
			break;

		case 'v':
			tempval = GITREV;
			tempval = (tempval <<32) | GITDATE;
			std::cout << "SLS Receiver " << GITBRANCH << " (0x" << std::hex << tempval << ")" << std::endl;
			throw std::exception();

		case 'h':
		default:
			std::string help_message = "\n"
					+ std::string(argv[0]) + "\n"
					+ "Usage: " + std::string(argv[0]) + " [arguments]\n"
					+ "Possible arguments are:\n"
					+ "\t-f, --config <fname>    : Loads config from file\n"
					+ "\t-t, --rx_tcpport <port> : TCP Communication Port with client. \n"
					+ "\t                          Default: 1954. Required for multiple \n"
					+ "\t                          receivers\n\n";

			FILE_LOG(logINFO) << help_message << std::endl;
			throw std::exception();

		}
	}

	if( !fname.empty() && read_config_file(fname, &tcpip_port_no, &configuration_map) == FAIL) {
		throw std::exception();
	}

	// might throw an exception
	tcpipInterface = new slsReceiverTCPIPInterface(tcpip_port_no);

}


slsReceiver::~slsReceiver() {
	if(tcpipInterface)
		delete tcpipInterface;
}


int slsReceiver::start() {
	return tcpipInterface->start();
}


void slsReceiver::stop() {
	tcpipInterface->stop();
}


int64_t slsReceiver::getReceiverVersion(){
	return tcpipInterface->getReceiverVersion();
}


void slsReceiver::registerCallBackStartAcquisition(int (*func)(
		char*, char*, uint64_t, uint32_t, void*),void *arg){
	tcpipInterface->registerCallBackStartAcquisition(func,arg);
}



void slsReceiver::registerCallBackAcquisitionFinished(
		void (*func)(uint64_t, void*),void *arg){
	tcpipInterface->registerCallBackAcquisitionFinished(func,arg);
}


void slsReceiver::registerCallBackRawDataReady(void (*func)(char*,
		char*, uint32_t, void*),void *arg){
	tcpipInterface->registerCallBackRawDataReady(func,arg);
}


void slsReceiver::registerCallBackRawDataModifyReady(void (*func)(char*,
        char*, uint32_t &, void*),void *arg){
	tcpipInterface->registerCallBackRawDataModifyReady(func,arg);
}
