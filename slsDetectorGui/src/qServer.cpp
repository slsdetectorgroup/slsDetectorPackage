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


qServer::qServer(qDetectorMain *t):
		  myMainTab(t), mySocket(NULL),myStopSocket(NULL),port_no(DEFAULT_GUI_PORTNO),lockStatus(0),checkStarted(0),checkStopStarted(0){
	strcpy(mess,"");
	FunctionTable();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qServer::~qServer(){
	delete myMainTab;
	if(mySocket) delete mySocket;
	if(myStopSocket) delete myStopSocket;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qServer::FunctionTable(){

	flist[F_GET_RUN_STATUS]		=	&qServer::GetStatus;
	flist[F_START_ACQUISITION]	=	&qServer::StartAcquisition;
	flist[F_STOP_ACQUISITION]	=	&qServer::StopsAcquisition;
	flist[F_START_AND_READ_ALL]	=	&qServer::Acquire;
	flist[F_EXIT_SERVER]		=	&qServer::ExitServer;

	return qDefs::OK;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::DecodeFunction(MySocketTCP* sock){
	int ret = qDefs::FAIL;
	int n,fnum;
#ifdef VERYVERBOSE
	cout <<  "receive data" << endl;
#endif
	n = sock->ReceiveDataOnly(&fnum,sizeof(fnum));
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



	if (((sock == myStopSocket) && ((fnum == F_GET_RUN_STATUS) || (fnum == F_STOP_ACQUISITION) || (fnum == F_EXIT_SERVER))) ||
			((sock == mySocket) && ((fnum == F_START_ACQUISITION) || (fnum == F_START_AND_READ_ALL))))
		;
	//unrecognized functions exit guis
	else{
		ret = qDefs::FAIL;
		sprintf(mess,"Unrecognized Function\n");
		cout << mess << endl;

		if (mySocket)
			mySocket->ShutDownSocket();

		myStopSocket->SendDataOnly(&ret,sizeof(ret));
		myStopSocket->SendDataOnly(mess,sizeof(mess));
		return GOODBYE;
	}


	//calling function
	ret = (this->*flist[fnum])();
	if (ret==qDefs::FAIL)
		cout <<  "Error executing the function = " << fnum << endl;

	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::ExitServer(){

	int ret = OK;
	strcpy(mess," Gui Server closed successfully\n");

	if(mySocket)
		mySocket->ShutDownSocket();

	myStopSocket->SendDataOnly(&ret,sizeof(ret));
	myStopSocket->SendDataOnly(mess,sizeof(mess));
	cout << mess << endl;

	return GOODBYE;
}


//------------------------------------------------------------------------------------------------------------------------------------------



int qServer::StartStopServer(int start){

	//start server
	if(start){
#ifdef VERBOSE
		cout << endl << "Starting Gui Server" << endl;
#endif

		if(!gui_server_thread_running){
			gui_server_thread_running=1;


			//error creating thread
			checkStarted=1;
			if (pthread_create(&gui_server_thread, NULL,StartServerThread, (void*) this)){
				gui_server_thread_running=0;
				qDefs::Message(qDefs::WARNING,"Can't create gui server thread", "qServer::StartStopServer");
				cout << "ERROR: Can't create gui server thread" << endl;
				return FAIL;
			}
			while(checkStarted);
			checkStarted = 0;
#ifdef VERBOSE
			if(gui_server_thread_running)
				cout << "Server thread created successfully." << endl;
#endif


			//error creating thread
			checkStopStarted=1;
			if (pthread_create(&gui_stop_server_thread, NULL,StopServerThread, (void*) this)){
				gui_server_thread_running=0;
				qDefs::Message(qDefs::WARNING,"Can't create gui stop server thread", "qServer::StartStopServer");
				cout << "ERROR: Can't create gui stop server thread" << endl;
				return FAIL;
			}
			while(checkStopStarted);
			checkStopStarted=0;
#ifdef VERBOSE
			if(gui_server_thread_running)
				cout << "Server Stop thread created successfully." << endl;
#endif
		}
	}


	//stop server
	else{
#ifdef VERBOSE
		cout << "Stopping Gui Server" << endl;
#endif

		if(gui_server_thread_running){
			gui_server_thread_running=0;

			if(mySocket)
				mySocket->ShutDownSocket();
			pthread_join(gui_server_thread,NULL);
			if(mySocket){
				delete mySocket;
				mySocket = NULL;
			}

			if(myStopSocket)
				myStopSocket->ShutDownSocket();
			pthread_join(gui_stop_server_thread,NULL);
			if(myStopSocket){
				delete myStopSocket;
				myStopSocket = NULL;
			}
		}
#ifdef VERBOSE
		cout << "Server threads stopped successfully." << endl;
#endif
	}

	return gui_server_thread_running;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void* qServer::StopServerThread(void* this_pointer){
	((qServer*)this_pointer)->StopServer();
	return this_pointer;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



int qServer::StopServer(){
#ifdef VERYVERBOSE
	cout << "In StopServer()" <<  endl;
#endif
	int ret = qDefs::OK;

	myStopSocket = new MySocketTCP(port_no+1);
	if (myStopSocket->getErrorStatus()){
		gui_server_thread_running = 0;
		qDefs::Message(qDefs::WARNING,"Could not start gui stop server socket","qServer::StopServer");
	}
	checkStopStarted = 0;

	while ((gui_server_thread_running) && (ret!=GOODBYE)) {
#ifdef VERBOSE
		cout<< endl;
#endif
#ifdef VERYVERBOSE
		cout << "Waiting for client call" << endl;
#endif
		if(myStopSocket->Connect()>=0){
#ifdef VERYVERBOSE
			cout << "Conenction accepted" << endl;
#endif
			ret = DecodeFunction(myStopSocket);
#ifdef VERYVERBOSE
			cout << "function executed" << endl;
#endif
			myStopSocket->Disconnect();
#ifdef VERYVERBOSE
			cout << "connection closed" << endl;
#endif
		}
	}
#ifdef VERBOSE
	cout << "Stopped gui stop server thread" << endl;
#endif
	gui_server_thread_running = 0;
	//delete socket(via exit server)
	if(myStopSocket){
		delete myStopSocket;
		myStopSocket = NULL;
	}

	if(!gui_server_thread_running)
		emit ServerStoppedSignal();

	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



void* qServer::StartServerThread(void* this_pointer){
	((qServer*)this_pointer)->StartServer();
	return this_pointer;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



int qServer::StartServer(){
#ifdef VERYVERBOSE
	cout << "In StartServer()" << endl;
#endif
	int ret = qDefs::OK;

	mySocket = new MySocketTCP(port_no);
	if (mySocket->getErrorStatus()){
		gui_server_thread_running = 0;
		qDefs::Message(qDefs::WARNING,"Could not start gui server socket","qServer::StartServer");
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
			ret = DecodeFunction(mySocket);
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

	if(!gui_server_thread_running)
		emit ServerStoppedSignal();

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
	myStopSocket->SendDataOnly(&ret,sizeof(ret));
	myStopSocket->SendDataOnly(&retval,sizeof(retval));
	myStopSocket->SendDataOnly(&progress,sizeof(progress));
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
	myStopSocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		myStopSocket->SendDataOnly(mess,sizeof(mess));

	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------


int qServer::Acquire(){

	strcpy(mess,"Could not start blocking acquisition in gui. \n");

	int ret = myMainTab->StartStopAcquisitionFromClient(true);

	usleep(5000);
	while(myMainTab->isPlotRunning());

	mySocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		mySocket->SendDataOnly(mess,sizeof(mess));

	return ret;
}


//------------------------------------------------------------------------------------------------------------------------------------------

