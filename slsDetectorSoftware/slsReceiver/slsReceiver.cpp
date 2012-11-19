/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "receiver_defs.h"
#include "MySocketTCP.h"
#include "slsReceiver_funcs.h"

#include <iostream>
using namespace std;



int main(int argc, char *argv[])
{
	int  portno = DEFAULT_PORTNO+2;
	int retval = slsDetectorDefs::OK;

	MySocketTCP *socket = new MySocketTCP(portno);
	if (socket->getErrorStatus())
		return -1;

	//assign function table
	slsReceiverFuncs *receiver = new slsReceiverFuncs(socket);
#ifdef VERBOSE
	cout << "Function table assigned." << endl;
#endif

	cout << " Ready..." << endl;
	//waits for connection
	while(retval!=GOODBYE) {
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
		retval = receiver->decode_function();
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

