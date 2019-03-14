#include "qServer.h"
#include "qDetectorMain.h"

#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "MySocketTCP.h"

#include <iostream>
#include <string>


int qServer::threadRunning(0);


qServer::qServer(qDetectorMain *t):
		  myMainTab(t), controlSocket(0), stopSocket(0), portNo(DEFAULT_GUI_PORTNO), checkControlStarted(0), checkStopStarted(0){
	strcpy(mess,"");
	FunctionTable();
    FILE_LOG(logDEBUG) << "Client Server ready";
}


qServer::~qServer(){
	delete myMainTab;
	if(controlSocket) delete controlSocket;
	if(stopSocket) delete stopSocket;
}


void qServer::FunctionTable(){
	flist[F_GET_RUN_STATUS]		=	&qServer::GetStatus;
	flist[F_START_ACQUISITION]	=	&qServer::StartAcquisition;
	flist[F_STOP_ACQUISITION]	=	&qServer::StopsAcquisition;
	flist[F_START_AND_READ_ALL]	=	&qServer::Acquire;
	flist[F_EXIT_SERVER]		=	&qServer::ExitServer;
}


int qServer::DecodeFunction(MySocketTCP* sock){
	int ret = qDefs::FAIL;

	int fnum = 0;
	int n = sock->ReceiveDataOnly(&fnum,sizeof(fnum));
	if (n <= 0) {
		FILE_LOG(logDEBUG3) << "Could not read socket. "
				"Received " << n << " bytes," <<
				"fnum:" << fnum;
		return FAIL;
	} else {
		FILE_LOG(logDEBUG3) << "Received " << n << " bytes";
	}

	// validate
	if (((sock == stopSocket) && ((fnum == F_GET_RUN_STATUS) || (fnum == F_STOP_ACQUISITION) || (fnum == F_EXIT_SERVER))) ||
			((sock == controlSocket) && ((fnum == F_START_ACQUISITION) || (fnum == F_START_AND_READ_ALL))))
		;
	//unrecognized functions exit guis
	else{
		ret = qDefs::FAIL;
		sprintf(mess,"Unrecognized Function in Gui Server\n");
		FILE_LOG(logERROR) << mess;

		if (controlSocket)
			controlSocket->ShutDownSocket();

		stopSocket->SendDataOnly(&ret,sizeof(ret));
		stopSocket->SendDataOnly(mess,sizeof(mess));
		return GOODBYE;
	}


	//calling function
	FILE_LOG(logDEBUG1) <<  "calling function fnum: "<< fnum << " "
			"located at " << flist[fnum];
	ret = (this->*flist[fnum])();

	if (ret == qDefs::FAIL)
		FILE_LOG(logERROR) <<  "Error executing the function = " << fnum;

	return ret;
}


int qServer::ExitServer(){

	int ret = OK;
	strcpy(mess," Gui Server closed successfully\n");
	FILE_LOG(logINFO) << mess;

	if(controlSocket)
		controlSocket->ShutDownSocket();

	stopSocket->SendDataOnly(&ret,sizeof(ret));
	stopSocket->SendDataOnly(mess,sizeof(mess));
	cout << mess << endl;

	return GOODBYE;
}


void qServer::StartServers(int start){

	//start server
	if(start){
		FILE_LOG(logINFO) << "Starting Gui Server";

		if(!threadRunning){
			threadRunning=1;


			// start control server
			checkControlStarted=1;
			if (pthread_create(&controlThread, NULL,ControlServerThread, (void*) this)) {
				threadRunning=0;
				qDefs::Message(qDefs::WARNING,"Can't create gui server thread", "qServer::StartServers");
				FILE_LOG(logERROR) << "Can't create gui server thread";
				return;
			}
			while(checkControlStarted);
			checkControlStarted = 0;
			if(threadRunning)
				FILE_LOG(logDEBUG) << "Server thread created successfully.";


			// start stop server
			checkStopStarted=1;
			if (pthread_create(&stopThread, NULL,StopServerThread, (void*) this)) {
				threadRunning=0;
				qDefs::Message(qDefs::WARNING,"Can't create gui stop server thread", "qServer::StartServers");
				FILE_LOG(logINFO) << "Can't create gui stop server thread";
				return;
			}
			while(checkStopStarted);
			checkStopStarted=0;
			if(threadRunning)
				FILE_LOG(logDEBUG) << "Server Stop thread created successfully.";
		}
	}


	//stop server
	else{
		FILE_LOG(logINFO) << "Stopping Gui Server";

		if(threadRunning){
			threadRunning=0;

			// kill control server
			if(controlSocket)
				controlSocket->ShutDownSocket();
			pthread_join(controlThread,NULL);
			if(controlSocket){
				delete controlSocket;
				controlSocket = 0;
			}

			// kill stop server
			if(stopSocket)
				stopSocket->ShutDownSocket();
			pthread_join(stopThread,NULL);
			if(stopSocket){
				delete stopSocket;
				stopSocket = 0;
			}
		}
		FILE_LOG(logDEBUG) << "Server threads stopped successfully.";
	}

}


void* qServer::StopServerThread(void* this_pointer){
	((qServer*)this_pointer)->StopServer();
	return this_pointer;
}


void qServer::StopServer(){
	FILE_LOG(logDEBUG) << "Starting StopServer()";
	int ret = qDefs::OK;

	try {
		MySocketTCP* s = new MySocketTCP(portNo+1);
		stopSocket = s;
	} catch(...) {
		threadRunning = 0;
		qDefs::Message(qDefs::WARNING,"Could not start gui stop server socket","qServer::StopServer");
		FILE_LOG(logWARNING) << "Could not start gui stop server socket";
	}
	checkStopStarted = 0;

	while ((threadRunning) && (ret!=GOODBYE)) {
		FILE_LOG(logDEBUG3) << "Waiting for client call";
		if(stopSocket->Connect()>=0){
			FILE_LOG(logDEBUG3) << "Conenction accepted";
			ret = DecodeFunction(stopSocket);
			FILE_LOG(logDEBUG3) << "function executed";
			stopSocket->Disconnect();
			FILE_LOG(logDEBUG3) << "connection closed";
		}
	}
	FILE_LOG(logDEBUG) << "Stopped gui stop server thread";

	threadRunning = 0;
	//delete socket(via exit server)
	if(stopSocket){
		delete stopSocket;
		stopSocket = 0;
	}

	if(!threadRunning)
		emit ServerStoppedSignal();

}


void* qServer::ControlServerThread(void* this_pointer){
	((qServer*)this_pointer)->ControlServer();
	return this_pointer;
}


void qServer::ControlServer(){
	FILE_LOG(logDEBUG) << "Starting ControlServer()";
	int ret = qDefs::OK;

	try {
		MySocketTCP* s = new MySocketTCP(portNo);
		controlSocket = s;
	} catch(...) {
		threadRunning = 0;
		qDefs::Message(qDefs::WARNING,"Could not start gui server socket","qServer::ControlServer");
		FILE_LOG(logWARNING) << "Could not start gui start server socket";
	}
	checkControlStarted = 0;

	while ((threadRunning) && (ret!=GOODBYE)) {
		FILE_LOG(logDEBUG3) << "Waiting for client call";
		if(controlSocket->Connect()>=0){
			FILE_LOG(logDEBUG3) << "Conenction accepted";
			ret = DecodeFunction(controlSocket);
			FILE_LOG(logDEBUG3) << "function executed";
			controlSocket->Disconnect();
			FILE_LOG(logDEBUG3) << "connection closed";
		}
	}
	FILE_LOG(logDEBUG) << "Stopped gui server thread";

	threadRunning = 0;
	//delete socket(via exit server)
	if(controlSocket){
		delete controlSocket;
		controlSocket = 0;
	}

	if(!threadRunning)
		emit ServerStoppedSignal();
}


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
	stopSocket->SendDataOnly(&ret,sizeof(ret));
	stopSocket->SendDataOnly(&retval,sizeof(retval));
	stopSocket->SendDataOnly(&progress,sizeof(progress));
	//return ok/fail
	return ret;
}


int qServer::StartAcquisition(){

	strcpy(mess,"Could not start acquisition in gui. \n")

	int ret = myMainTab->StartStopAcquisitionFromClient(true);
	controlSocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL) {
		FILE_LOG(logERROR) << mess;
		controlSocket->SendDataOnly(mess,sizeof(mess));
	}

	return ret;
}


int qServer::StopsAcquisition(){

	strcpy(mess,"Could not stop acquisition in gui. \n");

	int ret = myMainTab->StartStopAcquisitionFromClient(false);
	stopSocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL) {
		FILE_LOG(logERROR) << mess;
		controlSocket->SendDataOnly(mess,sizeof(mess));
	}
	return ret;
}


int qServer::Acquire(){

	strcpy(mess,"Could not start blocking acquisition in gui. \n");

	int ret = myMainTab->StartStopAcquisitionFromClient(true);

	usleep(5000);
	while(myMainTab->isPlotRunning());

	controlSocket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL) {
		FILE_LOG(logERROR) << mess;
		controlSocket->SendDataOnly(mess,sizeof(mess));
	}

	return ret;
}
