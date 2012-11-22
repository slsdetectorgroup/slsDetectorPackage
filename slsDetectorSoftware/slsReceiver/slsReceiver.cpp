/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "receiver_defs.h"
#include "MySocketTCP.h"
#include "slsReceiver_funcs.h"

#include <iostream>
using namespace std;



int main(int argc, char *argv[])
{
	int ret = slsDetectorDefs::OK;
	MySocketTCP *socket = NULL;
	string fname = "";
	bool shortfname = false;

	//parse command line for config
	for(int iarg=1;iarg<argc;iarg++){
		if(!strcasecmp(argv[iarg],"-config")){
			if(iarg+1==argc){
				cout << "no config file name given. Exiting." << endl;
				return -1;
			}
			fname.assign(argv[iarg]);
		}
		if(!strcasecmp(argv[iarg],"-shortfname"))
			shortfname = true;
	}



	//reads config file, creates socket, assigns function table
	slsReceiverFuncs *receiver = new slsReceiverFuncs(socket,fname,ret, shortfname);
	if(ret==slsDetectorDefs::FAIL)
		return -1;

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
		if(socket->Connect()>=0){
#ifdef VERY_VERBOSE
		cout << "Conenction accepted" << endl;
#endif
		ret = receiver->decode_function();
#ifdef VERY_VERBOSE
		cout << "function executed" << endl;
#endif
		socket->Disconnect();
#ifdef VERY_VERBOSE
			cout << "connection closed" << endl;
#endif
		}
	}

	delete socket;
	cout << "Goodbye!" << endl;

	return 0;
}

