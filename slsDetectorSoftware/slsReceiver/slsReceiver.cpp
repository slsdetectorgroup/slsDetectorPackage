/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "MySocketTCP.h"
#include "slsReceiver_funcs.h"


#include <signal.h>	//SIGINT
#include <cstdlib>	//EXIT

#include <iostream>
using namespace std;

//static MySocketTCP *mysocket = NULL;

void closeFile(int p){cout<<"in closefile in receiver"<<endl;
	slsReceiverFuncs::closeFile(p);
	//mysocket->Disconnect();
	exit(0);
}

int main(int argc, char *argv[])
{
	int ret = slsDetectorDefs::OK;
	MySocketTCP *mysocket = NULL;
	string fname = "";
	bool shortfname = false;

	//parse command line for config
	for(int iarg=1;iarg<argc;iarg++){
		if(!strcasecmp(argv[iarg],"-config")){
			if(iarg+1==argc){
				cout << "no config file name given. Exiting." << endl;
				return -1;
			}
			fname.assign(argv[iarg+1]);
		}
		if(!strcasecmp(argv[iarg],"-shortfname"))
			shortfname = true;
	}



	//reads config file, creates socket, assigns function table
	slsReceiverFuncs *receiver = new slsReceiverFuncs(mysocket,fname,ret, shortfname);
	if(ret==slsDetectorDefs::FAIL)
		return -1;


	//Catch signal SIGINT to close files properly
	signal(SIGINT,closeFile);


#ifdef VERBOSE
	cout << "Function table assigned." << endl;
#endif

	cout << " Ready..." << endl;
	//waits for connection
	while(ret!=GOODBYE) {
#ifdef VERBOSE
		cout<< endl;
#endif
#ifdef VERY_VERBOSE
		cout << "Waiting for client call" << endl;
#endif
		if(mysocket->Connect()>=0){
#ifdef VERY_VERBOSE
		cout << "Conenction accepted" << endl;
#endif
		ret = receiver->decode_function();
#ifdef VERY_VERBOSE
		cout << "function executed" << endl;
#endif
		mysocket->Disconnect();
#ifdef VERY_VERBOSE
			cout << "connection closed" << endl;
#endif
		}
	}


	slsReceiverFuncs::closeFile(0);
	cout << "Goodbye!" << endl;
	delete mysocket;

	return 0;
}

