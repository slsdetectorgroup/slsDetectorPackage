/********************************************//**
 * @file slsReceiver.cpp
 * @short creates the UDP and TCP class objects
 ***********************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>

#include <getopt.h>

#include "slsReceiver.h"
#include "UDPInterface.h"

#include "utilities.h"

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
	
	//creating base receiver
	int tcpip_port_no = 1984;
	success=OK;
	string fname = "";

	//parse command line for config
	static struct option long_options[] = {
		/* These options set a flag. */
		//{"verbose", no_argument,       &verbose_flag, 1},
		/* These options don’t set a flag.
		   We distinguish them by their indices. */
		{"config",     required_argument,       0, 'f'},
		{"rx_tcpport",  required_argument,       0, 'b'},
		{"help",  no_argument,       0, 'h'},
		{0, 0, 0, 0}
        };
	/* getopt_long stores the option index here. */
	int option_index = 0;
	int c;

	while ( c != -1 ){
		c = getopt_long (argc, argv, "bfh", long_options, &option_index);
		
		/* Detect the end of the options. */
		if (c == -1)
			break;
	
		switch(c){
		case 'f':
			fname = optarg;
			cout << long_options[option_index].name << " " << optarg << endl;
			break;

		case 'b':
			sscanf(optarg,"%d",&tcpip_port_no);
			cout << long_options[option_index].name << " " << optarg << endl;
			break;

		case 'h':
			string help_message = """\nSLS Receiver Server\n\n""";
			help_message += """usage: slsReceiver --config config_fname [--rx_tcpport port]\n\n""";
			help_message += """\t--config:\t configuration filename for SLS Detector receiver\n""";
			help_message += """\t--rx_tcpport:\t TCP Communication Port with the client. Default: 1954.\n\n""";
			cout << help_message << endl;
			break;
		       
		}
	}

	// if required fname parameter not available, fail
	if (fname == "")
		success = FAIL;

	if((!fname.empty()) && (success == OK)){
		VERBOSE_PRINT("config file name " + fname );
		success = read_config_file(fname, &tcpip_port_no);
		VERBOSE_PRINT("Read configuration file of " + iline + " lines");
	}
	else {
		cout << "Error opening configuration file " << fname << endl;
		success = FAIL;
	}


	if(success != OK){
		cout << "Failed: see output above for more information " << endl;
	}

	if (success==OK){
		cout << "SLS Receiver starting" << endl;
		udp_interface = UDPInterface::create("standard");
		tcpipInterface = new slsReceiverTCPIPInterface(success, udp_interface, tcpip_port_no);
		//tcp ip interface
	}
}


slsReceiver::~slsReceiver() {if(udp_interface) delete udp_interface; if(tcpipInterface) delete tcpipInterface;}


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
	tcpipInterface->getReceiverVersion();
}


void slsReceiver::registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){
  //tcpipInterface
	udp_interface->registerCallBackStartAcquisition(func,arg);
}



void slsReceiver::registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){
  //tcpipInterface
	udp_interface->registerCallBackAcquisitionFinished(func,arg);
}


void slsReceiver::registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){
	//tcpipInterface
	udp_interface->registerCallBackRawDataReady(func,arg);
}


