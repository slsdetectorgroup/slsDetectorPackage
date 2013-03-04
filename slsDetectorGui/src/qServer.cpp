/*
 * qServer.cpp
 *
 *  Created on: Feb 27, 2013
 *      Author: Dhanya Maliakal
 */
// Qt Project Class Headers
#include "qServer.h"
#include "qDetectorMain.h"
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


qServer::qServer(multiSlsDetector*& detector, qDetectorMain *t):
		myDet(detector), myMainTab(t), mySocket(NULL),port_no(DEFAULT_GUI_PORTNO),lockStatus(0){

	FunctionTable();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qServer::~qServer(){
	delete myDet;
	delete myMainTab;
	if(mySocket) delete mySocket;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qServer::FunctionTable(){

	for (int i=0;i<NUMBER_OF_FUNCTIONS;i++)
			flist[i]=&qServer::M_nofunc;

	flist[F_GET_RUN_STATUS]		=	&qServer::GetStatus;
	flist[F_START_ACQUISITION]	=	&qServer::StartAcquisition;
	flist[F_STOP_ACQUISITION]	=	&qServer::StopsAcquisition;
	flist[F_START_AND_READ_ALL]	=	&qServer::Acquire;
	flist[F_EXIT_SERVER]		=	&qServer::ExitServer;

	return qDefs::OK;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::DecodeFunction(){
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
	ret = (this->*flist[fnum])();
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
			if(mySocket)
				mySocket->ShutDownSocket();
			pthread_join(gui_server_thread,NULL);
			if(mySocket){
				delete mySocket;
				mySocket = NULL;
			}
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
			ret = DecodeFunction();
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
	//delete socket(via exit server)
	if(mySocket){
		delete mySocket;
		mySocket = NULL;
	}
	//uncheck the server in modes(via exit server)
	myMainTab->GuiServerExited();
	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qServer::GetStatus(){

	int ret = qDefs::OK;
	enum slsDetectorDefs::runStatus retval;
	int progress = 0;

	// execute action if the arguments correctly arrived
	if(myMainTab->isPlotRunning())
		retval = slsDetectorDefs::RUNNING;
	else
		retval = slsDetectorDefs::IDLE;

	progress = myMainTab->GetProgress();

	// send answer
	mySocket->SendDataOnly(&ret,sizeof(ret));
	mySocket->SendDataOnly(&retval,sizeof(retval));
	mySocket->SendDataOnly(&progress,sizeof(progress));
	//return ok/fail
	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::StartAcquisition(){

	strcpy(mess,"Could not start acquisition in gui. \n");

	int ret = myMainTab->StartStopAcquisitionFromClient(true);
	mySocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		mySocket->SendDataOnly(mess,sizeof(mess));

	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::StopsAcquisition(){

	strcpy(mess,"Could not stop acquisition in gui. \n");

	int ret = myMainTab->StartStopAcquisitionFromClient(false);
	mySocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		mySocket->SendDataOnly(mess,sizeof(mess));

	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::Acquire(){

	strcpy(mess,"Could not start blocking acquisition in gui. \n");

	int ret = myMainTab->StartStopAcquisitionFromClient(true);

	if(ret == OK)
		while(myMainTab->isPlotRunning());

	mySocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		mySocket->SendDataOnly(mess,sizeof(mess));

	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::ExitServer(){

	int ret = OK;
	strcpy(mess,"closing gui server");

	mySocket->SendDataOnly(&ret,sizeof(ret));
	mySocket->SendDataOnly(mess,sizeof(mess));
	cout << mess << endl;

	return GOODBYE;
}


//------------------------------------------------------------------------------------------------------------------------------------------
