/*
 * qServer.cpp
 *
 *  Created on: Feb 27, 2013
 *      Author: Dhanya Maliakal
 */
// Qt Project Class Headers
#include "qServer.h"
#include "qTabMeasurement.h"
#include "qDrawPlot.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "MySocketTCP.h"
// C++ Include Headers
#include <iostream>
#include <string>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qServer::gui_server_thread_running(0);


//-------------------------------------------------------------------------------------------------------------------------------------------------


qServer::qServer(multiSlsDetector*& detector, qTabMeasurement* m, qDrawPlot *d):
		myDet(detector),tab_measurement(m),myPlot(d),mySocket(NULL),port_no(DEFAULT_GUI_PORTNO),lockStatus(0){

	function_table();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qServer::~qServer(){
	delete myDet;
	delete tab_measurement;
	delete myPlot;
	if(mySocket) delete mySocket;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qServer::function_table(){

	for (int i=0;i<NUMBER_OF_FUNCTIONS;i++)
			flist[i]=&qServer::M_nofunc;

	flist[F_GET_RUN_STATUS]		=	&qServer::get_status;
	flist[F_START_ACQUISITION]	=	&qServer::get_status;
	flist[F_STOP_ACQUISITION]	=	&qServer::get_status;
	flist[F_START_AND_READ_ALL]	=	&qServer::get_status;
	flist[F_EXIT_SERVER]		=	&qServer::get_status;

	return qDefs::OK;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::decode_function(){
	int ret = qDefs::FAIL;
	int n,fnum;
#ifdef VERYVERBOSE
	cout <<  "receive data" << endl;
#endif
	n = mySocket->ReceiveDataOnly(&fnum,sizeof(fnum));
	if (n <= 0) {
#ifdef VERYVERBOSE
		cout << "ERROR reading from socket " << n << ", " << fnum << endl;
#endif
		return qDefs::FAIL;
	}
#ifdef VERYVERBOSE
	else
		cout << "size of data received " << n <<endl;
#endif

#ifdef VERYVERBOSE
	cout <<  "calling function fnum = "<< fnum << hex << ":"<< flist[fnum] << endl;
#endif

	if (fnum<0 || fnum>NUMBER_OF_FUNCTIONS-1)
		fnum = NUMBER_OF_FUNCTIONS-1;
	//calling function
	(this->*flist[fnum])();
	if (ret==qDefs::FAIL)
		cout <<  "Error executing the function = " << fnum << endl;

	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::M_nofunc(){

	int ret = qDefs::FAIL;
	sprintf(mess,"Unrecognized Function\n");
	cout << mess << endl;

	mySocket->SendDataOnly(&ret,sizeof(ret));
	mySocket->SendDataOnly(mess,sizeof(mess));

	return GOODBYE;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qServer::StartStopServer(int start){

	//start server
	if(start){
#ifdef VERBOSE
		cout << endl << "Starting Gui Server" << endl;
#endif

		if(!gui_server_thread_running){
#ifdef VERBOSE
			cout << "Starting gui server thread ...." << endl;
#endif
			gui_server_thread_running=1;
			checkStarted=1;
			//error creating thread
			if (pthread_create(&gui_server_thread, NULL,StartServerThread, (void*) this)){
				gui_server_thread_running=0;
				qDefs::Message(qDefs::WARNING,"Can't create gui server thread", "Server");
				return 0;
			}
			while(checkStarted);
			checkStarted=0;
#ifdef VERBOSE
			if(gui_server_thread_running)
				cout << "Server thread created successfully." << endl;
#endif
		}
	}

	//stop server
	else{
#ifdef VERBOSE
		cout << "Stopping Gui Server" << endl;
#endif

		if(gui_server_thread_running){
#ifdef VERBOSE
			cout << "Stopping gui server thread ...." << endl;
#endif
			gui_server_thread_running=0;
			mySocket->Disconnect();
			mySocket->ShutDownSocket();
			pthread_join(gui_server_thread,NULL);
			delete mySocket;
			mySocket = NULL;
		}
#ifdef VERBOSE
		cout << "Server stopped successfully." << endl;
#endif
	}


	return gui_server_thread_running;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void* qServer::StartServerThread(void* this_pointer){
	((qServer*)this_pointer)->StartServer();
	return this_pointer;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



int qServer::StartServer(){
#ifdef VERYVERBOSE
	cout << "In StartServer()\n");
#endif
	int ret = qDefs::OK;

	mySocket = new MySocketTCP(port_no);
	if (mySocket->getErrorStatus()){
		gui_server_thread_running = 0;
		qDefs::Message(qDefs::WARNING,"Could not start gui server socket","Server");
	}
	checkStarted = 0;

	while ((gui_server_thread_running) && (ret!=GOODBYE)) {
#ifdef VERBOSE
		cout<< endl;
#endif
#ifdef VERYVERBOSE
		cout << "Waiting for client call" << endl;
#endif
		if(mySocket->Connect()>=0){
#ifdef VERYVERBOSE
			cout << "Conenction accepted" << endl;
#endif
			ret = decode_function();
#ifdef VERYVERBOSE
			cout << "function executed" << endl;
#endif
			mySocket->Disconnect();
#ifdef VERYVERBOSE
			cout << "connection closed" << endl;
#endif
		}
	}
#ifdef VERBOSE
	cout << "Stopped gui server thread" << endl;
#endif
	gui_server_thread_running = 0;
	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qServer::get_status(){

	int ret = qDefs::OK;
	enum slsDetectorDefs::runStatus retval;

	// execute action if the arguments correctly arrived
	if(myPlot->isRunning())
		retval = slsDetectorDefs::RUNNING;
	else
		retval = slsDetectorDefs::IDLE;

	// send answer
	mySocket->SendDataOnly(&ret,sizeof(ret));
	mySocket->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------

