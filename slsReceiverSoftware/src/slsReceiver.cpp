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
//#include "UDPInterface.h"

using namespace std;



slsReceiver::slsReceiver(int argc, char *argv[], int &success){

	/** 
	 * Constructor method to start up a Receiver server. Reads configuration file, options, and 
	 * assembles a Receiver using TCP and UDP detector interfaces
	 * 
	 * @param iarg 
	 * 
	 * @return 
	 */
	
	udp_interface = NULL;
	tcpipInterface = NULL;

	//creating base receiver
	map<string, string> configuration_map;
	int tcpip_port_no = 1954;
	success=OK;
	string fname = "";
	string udp_interface_type = "standard";
	string rest_hostname = "localhost:8081";
	udp_interface = NULL;

	bool bottom = false; //TODO: properly set new parameter -> mode?
	//parse command line for config
	static struct option long_options[] = {
		/* These options set a flag. */
		//{"verbose", no_argument,       &verbose_flag, 1},
		/* These options don’t set a flag.
		   We distinguish them by their indices. */
		{"mode",     required_argument,       0, 'm'},
		{"type",     required_argument,       0, 't'},
		{"config",     required_argument,       0, 'f'},
		{"rx_tcpport",  required_argument,       0, 'b'},
		{"rest_hostname",  required_argument,       0, 'r'},
		{"help",  no_argument,       0, 'h'},
		{0, 0, 0, 0}
        };
	/* getopt_long stores the option index here. */
	int option_index = 0;
	int c=0;
	optind = 1;

	while ( c != -1 ){
		c = getopt_long (argc, argv, "mbfhtr", long_options, &option_index);
		
		/* Detect the end of the options. */
		if (c == -1)
			break;
	
		switch(c){

		case 'm':
			int b;
			sscanf(optarg, "%d", &b);
			bottom = b != 0;
			configuration_map["mode"] = optarg;
			break;

		case 'f':
			fname = optarg;
			//cout << long_options[option_index].name << " " << optarg << endl;
			break;

		case 'b':
			sscanf(optarg, "%d", &tcpip_port_no);
			break;
			
		case 't':
			udp_interface_type = optarg;
			break;

		case 'r':
			rest_hostname = optarg;
			break;

		case 'h':
			string help_message = """\nSLS Receiver Server\n\n""";
			help_message += """usage: slsReceiver --config config_fname [--rx_tcpport port]\n\n""";
			help_message += """\t--config:\t configuration filename for SLS Detector receiver\n""";
			help_message += """\t--mode:\t 1 for bottom and 0 for top\n""";
			help_message += """\t--rx_tcpport:\t TCP Communication Port with the client. Default: 1954.\n\n""";
			help_message += """\t--rest_hostname:\t Receiver hostname:port. It applies only to REST receivers, and indicates the hostname of the REST backend. Default: localhost:8081.\n\n""";

			help_message += """\t--type:\t Type of the receiver. Possible arguments are: standard, REST. Default: standard.\n\n""";

			cout << help_message << endl;
			break;
		       
		}
	}

	// if required fname parameter not available, fail
	//if (fname == "")
	//	success = FAIL;

	if( !fname.empty() ){
		try{
			FILE_LOG(logINFO) << "config file name " << fname;
			success = read_config_file(fname, &tcpip_port_no, &configuration_map);
			//VERBOSE_PRINT("Read configuration file of " + iline + " lines");
		}
		catch(...){
			FILE_LOG(logERROR) << "Error opening configuration file " << fname ;
		success = FAIL;
		}
	}


	if(success != OK){
		FILE_LOG(logERROR) << "Failed: see output above for more information " ;
	}

	if (success==OK){
		FILE_LOG(logINFO) << "SLS Receiver starting " << udp_interface_type << " on port " << tcpip_port_no << " with mode " << bottom << endl;
#ifdef REST
		udp_interface = UDPInterface::create(udp_interface_type);
		udp_interface->configure(configuration_map);
#endif
		tcpipInterface = new slsReceiverTCPIPInterface(success, udp_interface, tcpip_port_no, bottom);
	}
}


slsReceiver::~slsReceiver() {
	if(udp_interface) 
		delete udp_interface; 
	if(tcpipInterface) 
		delete tcpipInterface;
}


int slsReceiver::start() {
	return tcpipInterface->start();
}


void slsReceiver::stop() {
	tcpipInterface->stop();
}


void slsReceiver::closeFile(int p) {
	tcpipInterface->closeFile(p);
}


int64_t slsReceiver::getReceiverVersion(){
	return tcpipInterface->getReceiverVersion();
}


void slsReceiver::registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){
  //tcpipInterface
	if(udp_interface)
		udp_interface->registerCallBackStartAcquisition(func,arg);
	else
		tcpipInterface->registerCallBackStartAcquisition(func,arg);
}



void slsReceiver::registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){
  //tcpipInterface
	if(udp_interface)
		udp_interface->registerCallBackAcquisitionFinished(func,arg);
	else
		tcpipInterface->registerCallBackAcquisitionFinished(func,arg);
}


void slsReceiver::registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){
	//tcpipInterface
	if(udp_interface)
		udp_interface->registerCallBackRawDataReady(func,arg);
	else
		tcpipInterface->registerCallBackRawDataReady(func,arg);
}


