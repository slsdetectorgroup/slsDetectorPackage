/********************************************//**
 * @file slsReceiver.cpp
 * @short creates the UDP and TCP class objects
 ***********************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>

#include "slsReceiver.h"
//#include "slsReceiverUDPFunctions.h"
//#include "eigerReceiver.h"

#include "UDPInterface.h"
//#include "UDPBaseImplementation.h"


#include "utilities.h"

using namespace std;

slsReceiver::slsReceiver(int argc, char *argv[], int &success){
	
	//creating base receiver
	int tcpip_port_no=-1;

	ifstream infile;
	string sLine,sargname;
	int iline = 0;

	success=OK;

	string fname = "";

	//parse command line for config
	for(int iarg=1;iarg<argc;iarg++){
		if((!strcasecmp(argv[iarg],"-config"))||(!strcasecmp(argv[iarg],"-f"))){
			if(iarg+1==argc){
				cout << "no config file name given. Exiting." << endl;
				success=FAIL;
			}
			else
				fname.assign(argv[iarg+1]);
		}
	}
	if((!fname.empty()) && (success == OK)){

		VERBOSE_PRINT("config file name " + fname );
		
		infile.open(fname.c_str(), ios_base::in);
		if (infile.is_open()) {
			while(infile.good()){
				getline(infile,sLine);
				iline++;

				VERBOSE_PRINT(sLine);

				if(sLine.find('#')!=string::npos){
					VERBOSE_PRINT( "Line is a comment ");
					continue;
				}
				else if(sLine.length()<2){
					VERBOSE_PRINT("Empty line ");
					continue;
				}
				else{
					istringstream sstr(sLine);
					
					//parameter name
					if(sstr.good())
						sstr >> sargname;

					//tcp port
					if(sargname=="rx_tcpport"){
						if(sstr.good()) {
							sstr >> sargname;
							if(sscanf(sargname.c_str(),"%d",&tcpip_port_no))
								cout<<"dataport:"<<tcpip_port_no<<endl;
							else{
								cout << "could not decode port in config file. Exiting." << endl;
								success=FAIL;
							}
						}
					}
				}
			}
			infile.close();
		}
		else {
			cout << "Error opening configuration file " << fname << endl;
			success = FAIL;
		}

		VERBOSE_PRINT("Read configuration file of " + iline + " lines");
	}



	//parse command line for port etc.. more priority
	if(success == OK){
		for(int iarg=1;iarg<argc;iarg++){

			//tcp port
			if(!strcasecmp(argv[iarg],"-rx_tcpport")){
				if(iarg+1==argc){
					cout << "no port given after -rx_tcpport in command line. Exiting." << endl;
					success=FAIL;
				}else{
					if(sscanf(argv[iarg+1],"%d",&tcpip_port_no)){
						cout<<"dataport:"<<tcpip_port_no<<endl;
						iarg++;
					}else{
						cout << "could not decode port in command line. \n\nExiting." << endl;
						success=FAIL;
					}
				}
			}
			else{
				cout << "Unknown argument:" << argv[iarg] << endl;
				success=FAIL;
			}
		}
	}


	//help
	if(success != OK){
		cout << "Help Commands " << endl;
		cout << "rx_tcpport:\t TCP Communication Port with the client. Default:1954. " << endl << endl;
	}


	if (success==OK){
		cout << "SLS Receiver" << endl;
		udp_interface = UDPInterface::create("stasndard");
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


