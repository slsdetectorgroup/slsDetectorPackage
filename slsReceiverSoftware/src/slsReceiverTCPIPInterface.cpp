/********************************************//**
 * @file slsReceiverTCPIPInterface.cpp
 * @short interface between receiver and client
 ***********************************************/

#include "slsReceiverTCPIPInterface.h"
#include "UDPInterface.h"
#include "gitInfoReceiver.h"
#include "slsReceiverUsers.h"
#include "slsReceiver.h"

#include  <stdlib.h>	//EXIT
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <byteswap.h> //linux5
#define be64toh(x) __bswap_64 (x) //linux5
//#include <endian.h> //linux6
using namespace std;



slsReceiverTCPIPInterface::~slsReceiverTCPIPInterface() {
	stop();
	if(mySock) {delete mySock; mySock=NULL;}
}

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int &success, UDPInterface* rbase, int pn):
				myDetectorType(GOTTHARD),
				receiverBase(rbase),
				ret(OK),
				lockStatus(0),
				killTCPServerThread(0),
				tenGigaEnable(0),
				portNumber(DEFAULT_PORTNO+2),
				mySock(NULL){

	strcpy(SET_RECEIVER_ERR_MESSAGE,"Receiver not set up. Please use rx_hostname first.\n");

	//***callback parameters***
	startAcquisitionCallBack = NULL;
	pStartAcquisition = NULL;
	acquisitionFinishedCallBack = NULL;
	pAcquisitionFinished = NULL;
	rawDataReadyCallBack = NULL;
	pRawDataReady = NULL;

	unsigned short int port_no=portNumber;
	if(receiverBase == NULL) receiverBase = 0;

	if (pn>0)
		port_no = pn;

	success=OK;

	//create socket
	if(success == OK){
		mySock = new MySocketTCP(port_no);
		if (mySock->getErrorStatus()) {
			success = FAIL;
			delete mySock;
			mySock=NULL;
		}	else {
			portNumber=port_no;
			//initialize variables
			strcpy(mySock->lastClientIP,"none");
			strcpy(mySock->thisClientIP,"none1");
			strcpy(mess,"dummy message");
			function_table();
#ifdef VERYVERBOSE
			cout << "Function table assigned." << endl;
#endif
		}
	}

}


int slsReceiverTCPIPInterface::setPortNumber(int pn){
	int p_number;

	MySocketTCP *oldsocket=NULL;;
	int sd=0;

	if (pn>0) {
		p_number = pn;

		if (p_number<1024) {
			sprintf(mess,"Too low port number %d\n", p_number);
			cout << mess << endl;
		} else {

			oldsocket=mySock;
			mySock = new MySocketTCP(p_number);
			if(mySock){
				sd = mySock->getErrorStatus();
				if (!sd){
					portNumber=p_number;
					strcpy(mySock->lastClientIP,oldsocket->lastClientIP);
					delete oldsocket;
				} else {
					cout << "Could not bind port " << p_number << endl;
					if (sd==-10) {

						cout << "Port "<< p_number << " already set" << endl;
					} else {
						delete mySock;
						mySock=oldsocket;
					}
				}

			} else {
				mySock=oldsocket;
			}
		}
	}

	return portNumber;
}



int slsReceiverTCPIPInterface::start(){
	FILE_LOG(logDEBUG) << "Creating TCP Server Thread" << endl;
	killTCPServerThread = 0;
	if(pthread_create(&TCPServer_thread, NULL,startTCPServerThread, (void*) this)){
		cout << "Could not create TCP Server thread" << endl;
		return FAIL;
	}
	//#ifdef VERYVERBOSE
	FILE_LOG(logDEBUG) << "TCP Server thread created successfully." << endl;
	//#endif
	return OK;
}


void slsReceiverTCPIPInterface::stop(){
	cout << "Shutting down UDP Socket" << endl;
	killTCPServerThread = 1;
	if(mySock)	mySock->ShutDownSocket();
	cout<<"Socket closed"<<endl;
	pthread_join(TCPServer_thread, NULL);
	killTCPServerThread = 0;
	cout<<"Threads joined"<<endl;
}





void* slsReceiverTCPIPInterface::startTCPServerThread(void *this_pointer){
	((slsReceiverTCPIPInterface*)this_pointer)->startTCPServer();
	return this_pointer;
}


void slsReceiverTCPIPInterface::startTCPServer(){


#ifdef VERYVERBOSE
	cout << "Starting Receiver TCP Server" << endl;
#endif
	int v=OK;

	while(1) {
#ifdef VERYVERBOSE
		cout<< endl;
#endif
#ifdef VERY_VERBOSE
		cout << "Waiting for client call" << endl;
#endif
		if(mySock->Connect()>=0){
#ifdef VERY_VERBOSE
			cout << "Conenction accepted" << endl;
#endif
			v = decode_function();
#ifdef VERY_VERBOSE
			cout << "function executed" << endl;
#endif
			mySock->Disconnect();
#ifdef VERY_VERBOSE
			cout << "connection closed" << endl;
#endif
		}

		//if tcp command was to exit server
		if(v==GOODBYE){
			cout << "Shutting down UDP Socket" << endl;
			if(receiverBase){
				receiverBase->shutDownUDPSockets();

				cout << "Closing Files... " << endl;
				receiverBase->closeFiles();
			}

			mySock->exitServer();
			pthread_exit(NULL);
		}

		//if user entered exit
		if(killTCPServerThread)
			pthread_exit(NULL);

	}
}





int slsReceiverTCPIPInterface::function_table(){

	for (int i=0;i<numberOfFunctions;i++)
		flist[i]=&slsReceiverTCPIPInterface::M_nofunc;

	//General functions
	flist[F_EXEC_RECEIVER_COMMAND]			=	&slsReceiverTCPIPInterface::exec_command;	//not implemented in client
	flist[F_EXIT_RECEIVER]					=	&slsReceiverTCPIPInterface::exit_server;
	flist[F_LOCK_RECEIVER]					=	&slsReceiverTCPIPInterface::lock_receiver;
	flist[F_GET_LAST_RECEIVER_CLIENT_IP]	=	&slsReceiverTCPIPInterface::get_last_client_ip;
	flist[F_SET_RECEIVER_PORT]				=	&slsReceiverTCPIPInterface::set_port;
	flist[F_UPDATE_RECEIVER_CLIENT]			=	&slsReceiverTCPIPInterface::update_client;

	// Identification
	flist[F_GET_RECEIVER_ID]				=	&slsReceiverTCPIPInterface::get_id;
	flist[F_GET_RECEIVER_TYPE]				=	&slsReceiverTCPIPInterface::set_detector_type;
	flist[F_SEND_RECEIVER_DETHOSTNAME]		= 	&slsReceiverTCPIPInterface::set_detector_hostname;

	//network functions
	flist[F_RECEIVER_SHORT_FRAME]			=	&slsReceiverTCPIPInterface::set_short_frame;
	flist[F_SETUP_RECEIVER_UDP]				=	&slsReceiverTCPIPInterface::setup_udp;

	//Acquisition setup functions
	flist[F_SET_RECEIVER_TIMER]				= 	&slsReceiverTCPIPInterface::set_timer;
	flist[F_SET_RECEIVER_DYNAMIC_RANGE]		= 	&slsReceiverTCPIPInterface::set_dynamic_range;
	flist[F_READ_RECEIVER_FREQUENCY]		= 	&slsReceiverTCPIPInterface::set_read_frequency;

	// Acquisition functions
	flist[F_GET_RECEIVER_STATUS]			=	&slsReceiverTCPIPInterface::get_status;
	flist[F_START_RECEIVER]					=	&slsReceiverTCPIPInterface::start_receiver;
	flist[F_STOP_RECEIVER]					=	&slsReceiverTCPIPInterface::stop_receiver;
	flist[F_START_RECEIVER_READOUT]			= 	&slsReceiverTCPIPInterface::start_readout;
	flist[F_READ_RECEIVER_FRAME]			=	&slsReceiverTCPIPInterface::read_frame;

	//file functions
	flist[F_SET_RECEIVER_FILE_PATH]			=	&slsReceiverTCPIPInterface::set_file_dir;
	flist[F_SET_RECEIVER_FILE_NAME]			=	&slsReceiverTCPIPInterface::set_file_name;
	flist[F_SET_RECEIVER_FILE_INDEX]		=	&slsReceiverTCPIPInterface::set_file_index;
	flist[F_SET_RECEIVER_FRAME_INDEX]		=	&slsReceiverTCPIPInterface::set_frame_index;
	flist[F_GET_RECEIVER_FRAME_INDEX]		=	&slsReceiverTCPIPInterface::get_frame_index;
	flist[F_GET_RECEIVER_FRAMES_CAUGHT]		=	&slsReceiverTCPIPInterface::get_frames_caught;
	flist[F_RESET_RECEIVER_FRAMES_CAUGHT]	=	&slsReceiverTCPIPInterface::reset_frames_caught;
	flist[F_ENABLE_RECEIVER_FILE_WRITE]		=	&slsReceiverTCPIPInterface::enable_file_write;
	flist[F_ENABLE_RECEIVER_COMPRESSION]	= 	&slsReceiverTCPIPInterface::enable_compression;
	flist[F_ENABLE_RECEIVER_OVERWRITE]		= 	&slsReceiverTCPIPInterface::enable_overwrite;

	flist[F_ENABLE_RECEIVER_TEN_GIGA]		= 	&slsReceiverTCPIPInterface::enable_tengiga;
	flist[F_SET_RECEIVER_FIFO_DEPTH]		= 	&slsReceiverTCPIPInterface::set_fifo_depth;
	flist[F_ACTIVATE]						= 	&slsReceiverTCPIPInterface::set_activate;
	flist[F_STREAM_DATA_FROM_RECEIVER]		= 	&slsReceiverTCPIPInterface::set_data_stream_enable;
	flist[F_READ_RECEIVER_TIMER]			= 	&slsReceiverTCPIPInterface::set_read_receiver_timer;
	flist[F_SET_FLIPPED_DATA_RECEIVER]		= 	&slsReceiverTCPIPInterface::set_flipped_data;
	flist[F_SET_RECEIVER_FILE_FORMAT]		= 	&slsReceiverTCPIPInterface::set_file_format;
	flist[F_SEND_RECEIVER_DETPOSID]			= 	&slsReceiverTCPIPInterface::set_detector_posid;
	flist[F_SEND_RECEIVER_MULTIDETSIZE]		= 	&slsReceiverTCPIPInterface::set_multi_detector_size;


#ifdef VERYVERBOSE
	for (int i=0;i<numberOfFunctions;i++)
		cout << "function " << i << "located at " << flist[i] << endl;
#endif
	return OK;

}





int slsReceiverTCPIPInterface::decode_function(){
	ret = FAIL;
	int n,fnum;
#ifdef VERYVERBOSE
	cout <<  "receive data" << endl;
#endif
	n = mySock->ReceiveDataOnly(&fnum,sizeof(fnum));
	if (n <= 0) {
#ifdef VERYVERBOSE
		cout << "ERROR reading from socket " << n << ", " << fnum << endl;
#endif
		return FAIL;
	}
#ifdef VERYVERBOSE
	else
		cout << "size of data received " << n <<endl;
#endif

#ifdef VERYVERBOSE
	cout <<  "calling function fnum = "<< fnum << dec << ":"<< flist[fnum] << endl;
#endif

	if (fnum<0 || fnum>numberOfFunctions-1)
		fnum = numberOfFunctions-1;
	//calling function
	(this->*flist[fnum])();
	if (ret==FAIL)
		cprintf(RED, "Error executing the function = %d\n",fnum);

	return ret;
}






int slsReceiverTCPIPInterface::M_nofunc(){

	ret=FAIL;
	sprintf(mess,"Unrecognized Function\n");
	cout << mess << endl;

	mySock->SendDataOnly(&ret,sizeof(ret));
	mySock->SendDataOnly(mess,sizeof(mess));

	return GOODBYE;
}




void slsReceiverTCPIPInterface::closeFile(int p){
	receiverBase->closeFiles();
}


int slsReceiverTCPIPInterface::set_detector_type(){
	ret=OK;
	detectorType retval=GENERIC;
	detectorType dr;
	strcpy(mess,"Could not set detector type range\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&dr,sizeof(dr)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if((receiverBase)&&(receiverBase->getStatus()!= IDLE)){
			strcpy(mess,"Can not set detector type while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{

			switch(dr){
			case GOTTHARD:
			case PROPIX:
			case MOENCH:
			case EIGER:
			case JUNGFRAUCTB:
			case JUNGFRAU:
				break;
			default:
				sprintf(mess,"Unknown detector type: %d\n", dr);
				ret = FAIL;
				break;
			}
			if(ret != FAIL){
#ifndef REST
				if(receiverBase == NULL){
					receiverBase = UDPInterface::create("standard");
					if(startAcquisitionCallBack)
						receiverBase->registerCallBackStartAcquisition(startAcquisitionCallBack,pStartAcquisition);
					if(acquisitionFinishedCallBack)
						receiverBase->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,pAcquisitionFinished);
					if(rawDataReadyCallBack)
						receiverBase->registerCallBackRawDataReady(rawDataReadyCallBack,pRawDataReady);
				}
#endif
			  myDetectorType = dr;
			  ret=receiverBase->setDetectorType(myDetectorType);
			  retval = myDetectorType;
			}
			
		}
	}
	//#ifdef VERYVERBOSE
	if(ret!=FAIL)
		FILE_LOG(logDEBUG) << "detector type " << dr;
	else
		cprintf(RED, "%s\n", mess);
	//#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		mySock->SendDataOnly(mess,sizeof(mess));
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}







int slsReceiverTCPIPInterface::set_file_name() {
	ret=OK;
	char* retval = NULL;
	char defaultVal[MAX_STR_LENGTH] = "";
	char fName[MAX_STR_LENGTH];
	strcpy(mess,"Could not set file name");

	// receive arguments
	if(mySock->ReceiveDataOnly(fName,MAX_STR_LENGTH) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {

		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set file name while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			receiverBase->setFileName(fName);
			retval = receiverBase->getFileName();
			if(retval == NULL)
				ret = FAIL;
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "file name:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif


	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	if(retval == NULL)
		mySock->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		mySock->SendDataOnly(retval,MAX_STR_LENGTH);
		delete[] retval;
	}


	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::set_file_dir() {
	ret=OK;
	char* retval=NULL;
	char defaultVal[MAX_STR_LENGTH] = "";
	char fPath[MAX_STR_LENGTH];
	strcpy(mess,"Could not set file path\n");

	// receive arguments
	if(mySock->ReceiveDataOnly(fPath,MAX_STR_LENGTH) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set file path while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			receiverBase->setFilePath(fPath);
			retval = receiverBase->getFilePath();
			if (retval == NULL || (strlen(fPath) && strcasecmp(fPath, retval))) {
				ret = FAIL;
				strcpy(mess,"receiver file path does not exist\n");
			}
		}

	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "file path:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	if(retval == NULL)
		mySock->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		mySock->SendDataOnly(retval,MAX_STR_LENGTH);
		delete[] retval;
	}

	//return ok/fail
	return ret;
}





// LEO: do we need it in the base class?
int slsReceiverTCPIPInterface::set_file_index() {
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not set file index\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set file index while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if(index >= 0)
				receiverBase->setFileIndex(index);
			retval=receiverBase->getFileIndex();
			if(index>=0 && retval!=index)
				ret = FAIL;
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "file index:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}








int slsReceiverTCPIPInterface::set_frame_index() {
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not set frame index\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set frame index while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			//client sets to 0, but for receiver it is just an enable
			//client uses this value for other detectors not using receiver,
			//so implement the interface here

			switch(index){
			case -1: 	index=0; break;
			default: 	index=1; break; //value is 0
			}
			receiverBase->setFrameIndexEnable(index);
			retval=receiverBase->getFrameIndexEnable();
			switch(retval){
			case 0: 	retval=-1; break;
			case 1: 	retval=0; break;
			}
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "frame index:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






//LEO: is the client that commands the setup, or you just need the args?
int slsReceiverTCPIPInterface::setup_udp(){
	ret=OK;
	strcpy(mess,"could not set up udp connection");
	char retval[MAX_STR_LENGTH] = "";
	char args[3][MAX_STR_LENGTH];

	string temp;
	int udpport,udpport2;
	char eth[MAX_STR_LENGTH];


	// receive arguments

	if(mySock->ReceiveDataOnly(args,sizeof(args)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set up udp while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			//set up udp port
			sscanf(args[1],"%d",&udpport);
			sscanf(args[2],"%d",&udpport2);
			receiverBase->setUDPPortNumber(udpport);
			if (myDetectorType == EIGER)
				receiverBase->setUDPPortNumber2(udpport2);
			//setup udpip
			//get ethernet interface or IP to listen to
			FILE_LOG(logINFO) << "Receiver UDP IP: " << args[0];
			temp = genericSocket::ipToName(args[0]);
			if(temp=="none"){
				ret = FAIL;
				strcpy(mess, "Failed to get ethernet interface or IP\n");
				FILE_LOG(logERROR) << mess;
			}
			else{
				strcpy(eth,temp.c_str());
				if (strchr(eth,'.')!=NULL) {
					strcpy(eth,"");
					ret = FAIL;
				}
				receiverBase->setEthernetInterface(eth);

				//get mac address from ethernet interface
				if (ret != FAIL)
					temp = genericSocket::nameToMac(eth);


				if ((temp=="00:00:00:00:00:00") || (ret == FAIL)){
					ret = FAIL;
					strcpy(mess,"failed to get mac adddress to listen to\n");
				}
				else{
					strcpy(retval,temp.c_str());
					FILE_LOG(logINFO) << "Reciever MAC Address: " << retval;
				}
			}
		}
	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		FILE_LOG(logERROR) << mess;
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(retval,MAX_STR_LENGTH);

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::start_receiver(){
	ret=OK;
	ret=OK;
	enum runStatus s = ERROR;
	strcpy(mess,"Could not start receiver\n");

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (lockStatus==1 && mySock->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
		ret=FAIL;
	}
	/*
	else if(!strlen(receiverBase->getFilePath())){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE");
		ret = FAIL;
	}
	 */
	else if (receiverBase == NULL){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
		ret=FAIL;
	}
	else {
		s = receiverBase->getStatus();
		if(s == IDLE)
			ret=receiverBase->startReceiver(mess);
		else{
			sprintf(mess,"Cannot start Receiver as it is in %s state\n",runStatusType(s).c_str());
			cprintf(RED,"%s",mess);
			ret=FAIL;
		}
	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "Error:%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	//return ok/fail
	return ret;


}


int slsReceiverTCPIPInterface::stop_receiver(){
	ret=OK;
	enum runStatus s = ERROR;
	strcpy(mess,"Could not stop receiver\n");

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (lockStatus==1 && mySock->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
		ret=FAIL;
	}
	else if (receiverBase == NULL){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
		ret=FAIL;
	}
	else{
		if(receiverBase->getStatus()!= IDLE){
			receiverBase->stopReceiver();
		}
		s = receiverBase->getStatus();
		if(s==IDLE)
			ret = OK;
		else{
			sprintf(mess,"Could not stop receiver. It is in %s state\n",runStatusType(s).c_str());
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_status(){
	ret=OK;
	int retval=-1;
	enum runStatus s=ERROR;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
		ret=FAIL;
	}else s=receiverBase->getStatus();
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	retval = (runStatus(s));
	mySock->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_frames_caught(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
		ret=FAIL;
	}else retval=receiverBase->getTotalFramesCaught();
#endif
	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_frame_index(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
		ret=FAIL;
	}else
		retval=receiverBase->getAcquisitionIndex();
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::reset_frames_caught(){
	ret=OK;

	strcpy(mess,"Could not reset frames caught\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()==RUNNING){
			strcpy(mess,"Cannot reset frames caught while status is running\n");
			ret=FAIL;
		}
		else
			receiverBase->resetAcquisitionCount();
	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	//return ok/fail
	return ret;


}






int slsReceiverTCPIPInterface::set_short_frame() {
	ret=OK;
	int index=0;
	int retval=-100;
	strcpy(mess,"Could not set/reset short frame for receiver\n");

	//does not exist for moench
	if(myDetectorType==MOENCH){
		strcpy(mess,"can not set short frame for moench\n");
		ret = FAIL;
	}

	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Cannot set short frame while status is running\n");
			cprintf(RED,"%s",mess);
			ret=FAIL;
		}
		else{
			receiverBase->setShortFrameEnable(index);
			retval = receiverBase->getShortFrameEnable();
		}
	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}




int	slsReceiverTCPIPInterface::read_frame(){
	switch(myDetectorType){
	case MOENCH:
		return moench_read_frame();
	case EIGER:
		return eiger_read_frame();
	case PROPIX:
		return propix_read_frame();
	case JUNGFRAU:
		return jungfrau_read_frame();
	default:
		return gotthard_read_frame();
	}
}



int	slsReceiverTCPIPInterface::moench_read_frame(){	return FAIL;}




int	slsReceiverTCPIPInterface::gotthard_read_frame(){ return FAIL;}







int	slsReceiverTCPIPInterface::propix_read_frame(){ return FAIL;}










int	slsReceiverTCPIPInterface::eiger_read_frame(){ return FAIL;}







int	slsReceiverTCPIPInterface::jungfrau_read_frame(){ return FAIL;}






int slsReceiverTCPIPInterface::set_read_frequency(){
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not set receiver read frequency\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set receiver frequency mode while receiver not idle\n");
			cprintf(RED,"%s\n",mess);
			ret = FAIL;
		}
		else{
			if(index >= 0 ){
				ret = receiverBase->setFrameToGuiFrequency(index);
				if(ret == FAIL){
					strcpy(mess, "Could not allocate memory for listening fifo\n");
					cprintf(RED,"%s\n",mess);
				}
			}
			retval=receiverBase->getFrameToGuiFrequency();
			if(index>=0 && retval!=index){
				strcpy(mess,"Could not set frame to gui frequency");
				cprintf(RED,"%s\n",mess);
				ret = FAIL;
			}
		}
	}

#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::set_read_receiver_timer(){
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not set receiver read timer\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set receiver frequency mode while receiver not idle\n");
			cprintf(RED,"%s\n",mess);
			ret = FAIL;
		}
		else{
			if(index >= 0 ){
				receiverBase->setFrameToGuiTimer(index);
			}
			retval=receiverBase->getFrameToGuiTimer();
			if(index>=0 && retval!=index){
				strcpy(mess,"Could not set datastream timer");
				cprintf(RED,"%s\n",mess);
				ret = FAIL;
			}
		}
	}

#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}




int slsReceiverTCPIPInterface::set_data_stream_enable(){
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not set data stream enable\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if((index >= 0) && (receiverBase->getStatus()!= IDLE)){
			strcpy(mess,"Can not set data stream enable while receiver not idle\n");
			cprintf(RED,"%s\n",mess);
			ret = FAIL;
		}
		else{
			if(index >= 0 )
				ret = receiverBase->setDataStreamEnable(index);
			retval=receiverBase->getDataStreamEnable();
			if(index>=0 && retval!=index){
				strcpy(mess,"Could not set data stream enable");
				cprintf(RED,"%s\n",mess);
				ret = FAIL;
			}
		}
	}

#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}





int slsReceiverTCPIPInterface::enable_file_write(){
	ret=OK;
	int retval=-1;
	int enable;
	strcpy(mess,"Could not set/get enable file write\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&enable,sizeof(enable)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set file write mode while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if(enable >= 0)
				receiverBase->setFileWriteEnable(enable);
			retval=receiverBase->getFileWriteEnable();
			if(enable>=0 && enable!=retval)
				ret=FAIL;
		}
	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}



int slsReceiverTCPIPInterface::get_id(){
	ret=OK;
	int64_t retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	retval = getReceiverVersion();
#endif

	if(mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}



int64_t slsReceiverTCPIPInterface::getReceiverVersion(){
	int64_t retval = SVNREV;
	retval= (retval <<32) | SVNDATE;
	return retval;
}




int	slsReceiverTCPIPInterface::start_readout(){cprintf(BLUE,"In start readout!\n");
	ret=OK;
	enum runStatus retval;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
		ret=FAIL;
	}
	/*else if(receiverBase->getStatus()!= IDLE){
		strcpy(mess,"Can not start receiver readout while receiver not idle\n");
		ret = FAIL;
	}*/
	else{
		receiverBase->startReadout();
		retval = receiverBase->getStatus();
		if((retval == TRANSMITTING) || (retval == RUN_FINISHED) || (retval == IDLE))
			ret = OK;
		else
			ret = FAIL;
	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}




int slsReceiverTCPIPInterface::set_timer() {
	ret=OK;
	int64_t retval = -1;
	int64_t index[2];
	index[1] = -1;
	strcpy(mess,"Could not set acquisition period or frame number in receiver\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set timer while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if(index[0] == ACQUISITION_TIME){
				if(index[1]>=0){
					ret = receiverBase->setAcquisitionTime(index[1]);
					if(ret == FAIL)
						strcpy(mess,"Could not allocate memory for listening fifo\n");
				}
				retval=receiverBase->getAcquisitionTime();
			}else if(index[0] == FRAME_PERIOD){
				if(index[1]>=0){
					ret = receiverBase->setAcquisitionPeriod(index[1]);
					if(ret == FAIL)
						strcpy(mess,"Could not allocate memory for listening fifo\n");
				}
				retval=receiverBase->getAcquisitionPeriod();
			}else{
				if(index[1]>=0)
					receiverBase->setNumberOfFrames(index[1]);
				retval=receiverBase->getNumberOfFrames();
			}
			if(index[1]>=0 && retval!=index[1])
				ret = FAIL;
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL){
		if(index[0] == ACQUISITION_TIME)
			cout << "acquisition time:" << retval << endl;
		else if(index[0] == FRAME_PERIOD)
			cout << "acquisition period:" << retval << endl;
		else
			cout << "frame number:" << retval << endl;
	}else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::enable_compression() {
	ret=OK;
	int enable=-1;
	int retval=-100;
	strcpy(mess,"Could not enable/disable compression for receiver\n");

	// receive arguments
	if(mySock->ReceiveDataOnly(&enable,sizeof(enable)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if(enable >= 0){
			if (lockStatus==1 && mySock->differentClients==1){
				sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
				ret=FAIL;
			}
			else if (receiverBase == NULL){
				strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
				ret=FAIL;
			}
			else if(receiverBase->getStatus()!= IDLE){
				strcpy(mess,"Cannot enable/disable compression while status is running\n");
				cprintf(RED,"%s",mess);
				ret=FAIL;
			}
			else{
				if(enable >= 0)
					ret = receiverBase->setDataCompressionEnable(enable);
			}
		}

		if(ret != FAIL){
			if (receiverBase == NULL){
				strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
				ret=FAIL;
			}else{
				retval = receiverBase->getDataCompressionEnable();
				if(enable >= 0 && retval != enable)
					ret = FAIL;
			}
		}

	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}




int slsReceiverTCPIPInterface::set_detector_hostname() {
	ret=OK;
	char* retval = NULL;
	char defaultVal[MAX_STR_LENGTH] = "";
	char hostname[MAX_STR_LENGTH]="";
	strcpy(mess,"Could not set detector hostname");

	// receive arguments
	if(mySock->ReceiveDataOnly(hostname,MAX_STR_LENGTH) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {

		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set detector hostname while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			receiverBase->initialize(hostname);
			retval = receiverBase->getDetectorHostname();
			if(retval == NULL){
				ret = FAIL;
			}
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "hostname:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	if(retval == NULL)
		mySock->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		mySock->SendDataOnly(retval,MAX_STR_LENGTH);
		delete[] retval;
	}

	//return ok/fail
	return ret;
}






//LEO: why the receiver should set the dynamic range?
int slsReceiverTCPIPInterface::set_dynamic_range() {
	ret=OK;
	int retval=-1;
	int dr;
	strcpy(mess,"Could not set dynamic range\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&dr,sizeof(dr)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (dr>0) {
			switch(dr){
			case 4:
			case 8:
			case 16:
			case 32:break;
			default:
				sprintf(mess,"This dynamic range does not exist: %d\n",dr);
				cprintf(RED,"%s", mess);
				ret=FAIL;
				break;
			}
		}
		if(ret!=FAIL){
			if (receiverBase == NULL){
				strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
				ret=FAIL;
			}
			else if(receiverBase->getStatus()!= IDLE){
				strcpy(mess,"Can not set dynamic range while receiver not idle\n");
				cprintf(RED,"%s",mess);
				ret = FAIL;
			}
			else{
				if(dr > 0){
					ret = receiverBase->setDynamicRange(dr);
					if(ret == FAIL)
						strcpy(mess, "Could not allocate memory for fifo or could not start listening/writing threads\n");
				}
				retval = receiverBase->getDynamicRange();
				if(dr > 0 && retval != dr)
					ret = FAIL;
			}
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "dynamic range" << dr << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}







int slsReceiverTCPIPInterface::enable_overwrite() {
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not enable/disable overwrite\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set overwrite mode while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if(index >= 0)
				receiverBase->setOverwriteEnable(index);
			retval=receiverBase->getOverwriteEnable();
			if(index >=0 && retval != index)
				ret = FAIL;
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "overwrite:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}







int slsReceiverTCPIPInterface::enable_tengiga() {
	ret=OK;
	int retval=-1;
	int val;
	strcpy(mess,"Could not enable/disable 10Gbe\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&val,sizeof(val)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set up 1Giga/10Giga mode while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if(val >= 0)
				ret = receiverBase->setTenGigaEnable(val);
			retval=receiverBase->getTenGigaEnable();
			if((val >= 0) && (val != retval))
				ret = FAIL;
			else
				tenGigaEnable = retval;
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "10Gbe:" << val << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::set_fifo_depth() {
	ret=OK;
	int value=-1;
	int retval=-100;
	strcpy(mess,"Could not set/get fifo depth for receiver\n");

	// receive arguments
	if(mySock->ReceiveDataOnly(&value,sizeof(value)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if(value >= 0){
			if (lockStatus==1 && mySock->differentClients==1){
				sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
				ret=FAIL;
			}
			else if (receiverBase == NULL){
				strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
				ret=FAIL;
			}
			else if(receiverBase->getStatus()!= IDLE){
				strcpy(mess,"Cannot set/get fifo depth while status is running\n");
				cprintf(RED,"%s",mess);
				ret=FAIL;
			}
			else{
				if(value >= 0){
					ret = receiverBase->setFifoDepth(value);
				}
			}
		}


		if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}else{
			retval = receiverBase->getFifoDepth();
			/*if(value >= 0 && retval != value)
				ret = FAIL;*/
		}

	}
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}





int slsReceiverTCPIPInterface::set_activate() {
	ret=OK;
	int retval=-1;
	int enable;
	strcpy(mess,"Could not activate/deactivate\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&enable,sizeof(enable)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		cprintf(RED,"%s",mess);
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}

		if(ret!=FAIL){
			if (receiverBase == NULL){
				strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
				cprintf(RED,"%s",mess);
				ret=FAIL;
			}else if(receiverBase->getStatus()==RUNNING){
				strcpy(mess,"Cannot activate/deactivate while status is running\n");
				cprintf(RED,"%s",mess);
				ret=FAIL;
			}else{
				if(enable != -1)
					receiverBase->setActivate(enable);
				retval = receiverBase->getActivate();
				if(enable >= 0 && retval != enable){
					sprintf(mess,"Tried to set activate to %d, but returned %d\n",enable,retval);
					ret = FAIL;
					cprintf(RED,"%s",mess);
				}
			}
		}
	}
#endif
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "Activate: " << retval << endl;
	else
		cout << mess << endl;
#endif


	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}





int slsReceiverTCPIPInterface::set_flipped_data(){
	ret=OK;
	int retval = -1;
	int args[2]={0,-1};
	strcpy(mess,"Could not set flipped data in receiver\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(args,sizeof(args)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set flipped data while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if(args[1] > -1)
				receiverBase->setFlippedData(args[0],args[1]);
			retval=receiverBase->getFlippedData(args[0]);
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL){
		cout << "Flipped Data:" << retval << endl;
	}else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}




int slsReceiverTCPIPInterface::set_file_format() {
	ret=OK;
	fileFormat retval = GET_FILE_FORMAT;
	fileFormat f = GET_FILE_FORMAT;
	strcpy(mess,"Could not set/get file format\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&f,sizeof(f)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		cprintf(RED,"%s",mess);
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}

		if(ret!=FAIL){
			if (receiverBase == NULL){
				strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
				cprintf(RED,"%s",mess);
				ret=FAIL;
			}else if(receiverBase->getStatus()==RUNNING && (f>=0)){
				strcpy(mess,"Cannot set file format while status is running\n");
				cprintf(RED,"%s",mess);
				ret=FAIL;
			}else{
				if(f != -1)
					receiverBase->setFileFormat(f);
				retval = receiverBase->getFileFormat();
				if(f >= 0 && retval != f){
					sprintf(mess,"Tried to set file format to %d, but returned %d\n",f,retval);
					ret = FAIL;
					cprintf(RED,"%s",mess);
				}
			}
		}
	}
#endif
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "File Format: " << retval << endl;
	else
		cout << mess << endl;
#endif


	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}




int slsReceiverTCPIPInterface::set_detector_posid() {
	ret=OK;
	int retval=-1;
	int arg=-1;
	strcpy(mess,"Could not set detector position id\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(&arg,sizeof(arg)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set position file id while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if(arg >= 0)
				receiverBase->setDetectorPositionId(arg);
			retval=receiverBase->getDetectorPositionId();
			if(arg>=0 && retval!=arg)
				ret = FAIL;
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "Position Id:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::set_multi_detector_size() {
	ret=OK;
	int retval=-1;
	int arg[2];
	arg[0]=-1;
	arg[1]=-1;
	strcpy(mess,"Could not set multi detector size\n");


	// receive arguments
	if(mySock->ReceiveDataOnly(arg,sizeof(arg)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && mySock->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", mySock->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
			ret=FAIL;
			cprintf(RED, "%s", mess);
		}
		else if(receiverBase->getStatus()!= IDLE){
			strcpy(mess,"Can not set position file id while receiver not idle\n");
			cprintf(RED,"%s",mess);
			ret = FAIL;
		}
		else{
			if((arg[0] > 0) && (arg[1] > 0))
				receiverBase->setMultiDetectorSize(arg);
		}
	}
#ifdef VERYVERBOSE
	if(ret!=FAIL)
		cout << "Multi Detector Size:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && mySock->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	mySock->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}



























int slsReceiverTCPIPInterface::lock_receiver() {
	ret=OK;
	int lock;

	// receive arguments
	if(mySock->ReceiveDataOnly(&lock,sizeof(lock)) < 0 ){
		sprintf(mess,"Error reading from socket\n");
		cout << "Error reading from socket (lock)" << endl;
		ret=FAIL;
	}
	// execute action if the arguments correctly arrived
	if(ret==OK){
		if (lock>=0) {
			if (lockStatus==0 || strcmp(mySock->lastClientIP,mySock->thisClientIP)==0 || strcmp(mySock->lastClientIP,"none")==0) {
				lockStatus=lock;
				strcpy(mySock->lastClientIP,mySock->thisClientIP);
			}   else {
				ret=FAIL;
				sprintf(mess,"Receiver already locked by %s\n", mySock->lastClientIP);
			}
		}
	}

	if (mySock->differentClients && ret==OK)
		ret=FORCE_UPDATE;

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}
	else
		mySock->SendDataOnly(&lockStatus,sizeof(lockStatus));

	//return ok/fail
	return ret;
}







int slsReceiverTCPIPInterface::set_port() {
	ret=OK;
	MySocketTCP* mySocket=NULL;
	char oldLastClientIP[INET_ADDRSTRLEN];
	int sd=-1;
	enum runStatus p_type; /* just to get the input */
	int p_number;

	// receive arguments
	if(mySock->ReceiveDataOnly(&p_type,sizeof(p_type)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		cout << mess << endl;
		ret=FAIL;
	}

	if(mySock->ReceiveDataOnly(&p_number,sizeof(p_number)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		cout << mess << endl;
		ret=FAIL;
	}

	// execute action if the arguments correctly arrived
	if (ret==OK) {
		if (mySock->differentClients==1 && lockStatus==1 ) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",mySock->lastClientIP);
		}
		else {
			if (p_number<1024) {
				sprintf(mess,"Too low port number %d\n", p_number);
				cout << mess << endl;
				ret=FAIL;
			}
			cout << "set port " << p_type << " to " << p_number <<endl;
			strcpy(oldLastClientIP, mySock->lastClientIP);
			mySocket = new MySocketTCP(p_number);
		}
		if(mySocket){
			sd = mySocket->getErrorStatus();
			if (!sd){
				ret=OK;
				strcpy(mySock->lastClientIP,oldLastClientIP);
				if (mySocket->differentClients)
					ret=FORCE_UPDATE;
			} else {
				ret=FAIL;
				sprintf(mess,"Could not bind port %d\n", p_number);
				cout << mess << endl;
				if (sd==-10) {
					sprintf(mess,"Port %d already set\n", p_number);
					cout << mess << endl;
				}
			}
		}
	}

	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
	}else {
		mySock->SendDataOnly(&p_number,sizeof(p_number));
		if(sd>=0){
			mySock->Disconnect();
			delete mySock;
			mySock = mySocket;
		}
	}

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::get_last_client_ip() {
	ret=OK;

	if (mySock->differentClients )
		ret=FORCE_UPDATE;

	mySock->SendDataOnly(&ret,sizeof(ret));
	mySock->SendDataOnly(mySock->lastClientIP,sizeof(mySock->lastClientIP));

	return ret;
}







int slsReceiverTCPIPInterface::send_update() {
	ret=OK;
	int ind;
	char defaultVal[MAX_STR_LENGTH]="";
	char* path = NULL;

	mySock->SendDataOnly(mySock->lastClientIP,sizeof(mySock->lastClientIP));

	//index
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	ind=receiverBase->getFileIndex();

	mySock->SendDataOnly(&ind,sizeof(ind));
#endif

	//filepath
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	path = receiverBase->getFilePath();
#endif
	if(path == NULL)
		mySock->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		mySock->SendDataOnly(path,MAX_STR_LENGTH);
		delete[] path;
	}


	//filename
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	path = receiverBase->getFileName();
#endif
	if(path == NULL)
		mySock->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		mySock->SendDataOnly(path,MAX_STR_LENGTH);
		delete[] path;
	}

	if (lockStatus==0) {
		strcpy(mySock->lastClientIP,mySock->thisClientIP);
	}

	return ret;


}






int slsReceiverTCPIPInterface::update_client() {
	ret=OK;
	if (receiverBase == NULL){
		strcpy(mess,SET_RECEIVER_ERR_MESSAGE);
		ret=FAIL;
	}
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		mySock->SendDataOnly(mess,sizeof(mess));
		return ret;
	}

	return send_update();
}







int slsReceiverTCPIPInterface::exit_server() {
	ret=GOODBYE;
	mySock->SendDataOnly(&ret,sizeof(ret));
	strcpy(mess,"closing server");
	mySock->SendDataOnly(mess,sizeof(mess));
	cprintf(RED,"%s\n",mess);
	return ret;
}





int slsReceiverTCPIPInterface::exec_command() {
	ret = OK;
	char cmd[MAX_STR_LENGTH];
	char answer[MAX_STR_LENGTH];
	int sysret=0;

	// receive arguments
	if(mySock->ReceiveDataOnly(cmd,MAX_STR_LENGTH) < 0 ){
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	// execute action if the arguments correctly arrived
	if (ret==OK) {
#ifdef VERYVERBOSE
		cout << "executing command " << cmd << endl;
#endif
		if (lockStatus==0 || mySock->differentClients==0)
			sysret=system(cmd);

		//should be replaced by popen
		if (sysret==0) {
			strcpy(answer,"Succeeded\n");
			if (lockStatus==1 && mySock->differentClients==1)
				sprintf(answer,"Detector locked by %s\n", mySock->lastClientIP);
		} else {
			strcpy(answer,"Failed\n");
			ret=FAIL;
		}
	} else
		strcpy(answer,"Could not receive the command\n");


	// send answer
	mySock->SendDataOnly(&ret,sizeof(ret));
	if(mySock->SendDataOnly(answer,MAX_STR_LENGTH) < 0){
		strcpy(mess,"Error writing to socket");
		ret=FAIL;
	}

	//return ok/fail
	return ret;
}



/***callback functions***/
void slsReceiverTCPIPInterface::registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg){
	startAcquisitionCallBack=func;
	pStartAcquisition=arg;
}

void slsReceiverTCPIPInterface::registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg){
	acquisitionFinishedCallBack=func;
	pAcquisitionFinished=arg;
}

void slsReceiverTCPIPInterface::registerCallBackRawDataReady(void (*func)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
		char*, uint32_t, void*),void *arg){
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}





