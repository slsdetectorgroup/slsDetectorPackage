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
	if(socket) {delete socket; socket=NULL;}
}

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int &success, UDPInterface* rbase, int pn, bool bot):
				myDetectorType(GOTTHARD),
				receiverBase(rbase),
				ret(OK),
				lockStatus(0),
				shortFrame(-1),
				packetsPerFrame(GOTTHARD_PACKETS_PER_FRAME),
				dynamicrange(16),
				killTCPServerThread(0),
				tenGigaEnable(0),
				portNumber(DEFAULT_PORTNO+2),
				bottom(bot),
				socket(NULL){

	//***callback parameters***
	startAcquisitionCallBack = NULL;
	pStartAcquisition = NULL;
	acquisitionFinishedCallBack = NULL;
	pAcquisitionFinished = NULL;
	rawDataReadyCallBack = NULL;
	pRawDataReady = NULL;

	int port_no=portNumber;
	if(receiverBase == NULL) receiverBase = 0;

	if (pn>0)
		port_no = pn;

	success=OK;

	//create socket
	if(success == OK){
		socket = new MySocketTCP(port_no);
		if (socket->getErrorStatus()) {
			success = FAIL;
			delete socket;
			socket=NULL;
		}	else {
			portNumber=port_no;
			//initialize variables
			strcpy(socket->lastClientIP,"none");
			strcpy(socket->thisClientIP,"none1");
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

			oldsocket=socket;
			socket = new MySocketTCP(p_number);
			if(socket){
				sd = socket->getErrorStatus();
				if (!sd){
					portNumber=p_number;
					strcpy(socket->lastClientIP,oldsocket->lastClientIP);
					delete oldsocket;
				} else {
					cout << "Could not bind port " << p_number << endl;
					if (sd==-10) {

						cout << "Port "<< p_number << " already set" << endl;
					} else {
						delete socket;
						socket=oldsocket;
					}
				}

			} else {
				socket=oldsocket;
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
	if(socket)	socket->ShutDownSocket();
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
		if(socket->Connect()>=0){
#ifdef VERY_VERBOSE
			cout << "Conenction accepted" << endl;
#endif
			v = decode_function();
#ifdef VERY_VERBOSE
			cout << "function executed" << endl;
#endif
			socket->Disconnect();
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
				receiverBase->closeFile();
			}

			socket->exitServer();
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
	n = socket->ReceiveDataOnly(&fnum,sizeof(fnum));
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
	cout <<  "calling function fnum = "<< fnum << hex << ":"<< flist[fnum] << endl;
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

	socket->SendDataOnly(&ret,sizeof(ret));
	socket->SendDataOnly(mess,sizeof(mess));

	return GOODBYE;
}




void slsReceiverTCPIPInterface::closeFile(int p){
	receiverBase->closeFile();
}


int slsReceiverTCPIPInterface::set_detector_type(){
	ret=OK;
	detectorType retval=GENERIC;
	detectorType dr;
	strcpy(mess,"Could not set detector type range\n");


	// receive arguments
	if(socket->ReceiveDataOnly(&dr,sizeof(dr)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
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
			  receiverBase = UDPInterface::create("standard");
			  if(startAcquisitionCallBack)
				  receiverBase->registerCallBackStartAcquisition(startAcquisitionCallBack,pStartAcquisition);
			  if(acquisitionFinishedCallBack)
				  receiverBase->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,pAcquisitionFinished);
			  if(rawDataReadyCallBack)
				  receiverBase->registerCallBackRawDataReady(rawDataReadyCallBack,pRawDataReady);
#endif
			  myDetectorType = dr;
			  ret=receiverBase->setDetectorType(myDetectorType);
			  retval = myDetectorType;
#ifndef REST
			  receiverBase->setBottomEnable(bottom);
#endif
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	socket->SendDataOnly(&retval,sizeof(retval));

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
	if(socket->ReceiveDataOnly(fName,MAX_STR_LENGTH) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {

		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
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


	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	if(retval == NULL)
		socket->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		socket->SendDataOnly(retval,MAX_STR_LENGTH);
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
	if(socket->ReceiveDataOnly(fPath,MAX_STR_LENGTH) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}/*
		else if((strlen(fPath))&&(receiverBase->getStatus()==RUNNING)){
			strcpy(mess,"Can not set file path while receiver running\n");
			ret = FAIL;
		}*/
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
		}
		else{
			receiverBase->setFilePath(fPath);
			retval = receiverBase->getFilePath();
			if(retval == NULL){
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	if(retval == NULL)
		socket->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		socket->SendDataOnly(retval,MAX_STR_LENGTH);
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
	if(socket->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}








int slsReceiverTCPIPInterface::set_frame_index() {
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not set frame index\n");


	// receive arguments
	if(socket->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

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

	if(socket->ReceiveDataOnly(args,sizeof(args)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
		}
		else if(receiverBase->getStatus()==RUNNING){
			ret = FAIL;
			strcpy(mess,"cannot set up udp when receiver is running\n");
		}
		else{
			//set up udp port
			sscanf(args[1],"%d",&udpport);
			sscanf(args[2],"%d",&udpport2);
			receiverBase->setUDPPortNumber(udpport);
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		FILE_LOG(logERROR) << mess;
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(retval,MAX_STR_LENGTH);

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
	if (lockStatus==1 && socket->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
		ret=FAIL;
	}
	/*
	else if(!strlen(receiverBase->getFilePath())){
		strcpy(mess,"receiver not set up. set receiver ip again.\n");
		ret = FAIL;
	}
	 */
	else if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}
	else {
		s = receiverBase->getStatus();
		if(s == IDLE)
			ret=receiverBase->startReceiver(mess);
		else{
			sprintf(mess,"Cannot start Receiver as it is in %s state\n",runStatusType(s).c_str());
			ret=FAIL;
		}
	}
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "Error:%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
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
	if (lockStatus==1 && socket->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
		ret=FAIL;
	}
	else if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}
	else{
		if(receiverBase->getStatus()!=IDLE)
			receiverBase->stopReceiver();
		s = receiverBase->getStatus();
		if(s==IDLE)
			ret = OK;
		else{
			sprintf(mess,"Could not stop receiver. It is in %s state\n",runStatusType(s).c_str());
			ret = FAIL;
		}
	}
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_status(){
	ret=OK;
	enum runStatus retval = ERROR;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}else retval=receiverBase->getStatus();
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_frames_caught(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}else retval=receiverBase->getTotalFramesCaught();
#endif
	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_frame_index(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}else
		retval=receiverBase->getAcquisitionIndex();
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::reset_frames_caught(){
	ret=OK;

	strcpy(mess,"Could not reset frames caught\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
		}
		else
			receiverBase->resetAcquisitionCount();
	}
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
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
	if(socket->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
		}
		else if(receiverBase->getStatus()==RUNNING){
			strcpy(mess,"Cannot set short frame while status is running\n");
			ret=FAIL;
		}
		else{
			receiverBase->setShortFrameEnable(index);
			retval = receiverBase->getShortFrameEnable();
			shortFrame = retval;
			if(shortFrame==-1)
				packetsPerFrame=GOTTHARD_PACKETS_PER_FRAME;
			else
				packetsPerFrame=GOTTHARD_SHORT_PACKETS_PER_FRAME;
		}
	}
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

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



int	slsReceiverTCPIPInterface::moench_read_frame(){
	ret=OK;
	char fName[MAX_STR_LENGTH]="";
	int acquisitionIndex = -1;
	int frameIndex= -1;
	int i;


	int bufferSize = MOENCH_BUFFER_SIZE;
	int rnel 		= bufferSize/(sizeof(int));
	int* retval 	= new int[rnel];
	int* origVal 	= new int[rnel];
	//all initialized to 0
	for(i=0;i<rnel;i++)	retval[i]=0;
	for(i=0;i<rnel;i++)	origVal[i]=0;

	char* raw;

	uint64_t startAcquisitionIndex=0;
	uint64_t startFrameIndex=0;
	uint32_t index = -1,bindex = 0, offset=0;

	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}
	/**send garbage with -1 index to try again*/
	else if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex = -1;
		cout<<"haven't caught any frame yet"<<endl;
	}

	else{
		ret = OK;
		receiverBase->readFrame(fName,&raw,startAcquisitionIndex,startFrameIndex);

		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERYVERBOSE
			cout<<"data not ready for gui yet"<<endl;
#endif
		}

		else{
			bindex = ((uint32_t)(*((uint32_t*)raw)));
			memcpy(origVal,raw,bufferSize);
			raw=NULL;

			//************** packet number order**********************
			index = ((bindex & (MOENCH_FRAME_INDEX_MASK)) >> MOENCH_FRAME_INDEX_OFFSET);

			uint32_t numPackets = MOENCH_PACKETS_PER_FRAME; //40
			uint32_t onePacketSize = MOENCH_DATA_BYTES / MOENCH_PACKETS_PER_FRAME; //1280*40 / 40 = 1280
			uint32_t packetDatabytes_row = onePacketSize * (MOENCH_BYTES_IN_ONE_ROW / MOENCH_BYTES_PER_ADC); //1280 * 4 = 5120
			uint32_t partsPerFrame = onePacketSize / MOENCH_BYTES_PER_ADC; // 1280 / 80 = 16
			uint32_t packetOffset = 0;
			int packetIndex,x,y;
			int iPacket = 0;
			offset = 4;


			while (iPacket < (int)numPackets){
#ifdef VERYVERBOSE
				printf("iPacket:%d\n",iPacket);cout << endl;
#endif
				//if missing packets, dont send to gui
				bindex = (*((uint32_t*)(((char*)origVal)+packetOffset)));
				if (bindex == 0xFFFFFFFF){
					cout << "Missing Packet,Not sending to gui" << endl;
					index = startAcquisitionIndex - 1;
					break;//use continue and change index above if you want to display missing packets with 0 value anyway in gui
				}

				packetIndex = bindex & MOENCH_PACKET_INDEX_MASK;
				//cout<<"packetIndex:"<<packetIndex<<endl;
				//the first packet is placed in the end
				packetIndex--;
				if(packetIndex ==-1)
					packetIndex = 39;
				//cout<<"packetIndexM:"<<packetIndex<<endl;
				//check validity
				if ((packetIndex >= 40) && (packetIndex < 0))
					cout << "cannot decode packet index:" << packetIndex << endl;
				else{

					x = packetIndex / 10;
					y = packetIndex % 10;
#ifdef VERYVERBOSE
					cout<<"x:"<<x<<" y:"<<y<<endl;
#endif
					//copy 16 times 80 bytes
					for (i = 0; i < (int)partsPerFrame; i++) {
						memcpy((((char*)retval) +
								y * packetDatabytes_row +
								i * MOENCH_BYTES_IN_ONE_ROW +
								x * MOENCH_BYTES_PER_ADC),

								(((char*) origVal) +
										iPacket * offset +
										iPacket * onePacketSize +
										i * MOENCH_BYTES_PER_ADC + 4)  ,
										MOENCH_BYTES_PER_ADC);
					}
				}

				//increment
				offset=6;
				iPacket++;
				packetOffset = packetOffset + offset + onePacketSize;

				//check if same frame number
			}

			acquisitionIndex = index-startAcquisitionIndex;
			if(acquisitionIndex ==  -1)
				startFrameIndex = -1;
			else
				frameIndex = index-startFrameIndex;
#ifdef VERY_VERY_DEBUG
			cout << "acquisitionIndex calculated is:" << acquisitionIndex << endl;
			cout << "frameIndex calculated is:" << frameIndex << endl;
			cout << "index:" << index << endl;
			cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
			cout << "startFrameIndex:" << startFrameIndex << endl;
#endif
		}

	}

#ifdef VERYVERBOSE
	cout << "fName:" << fName << endl;
	cout << "acquisitionIndex:" << acquisitionIndex << endl;
	cout << "frameIndex:" << frameIndex << endl;
	cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
	cout << "startFrameIndex:" << startFrameIndex << endl;
#endif

#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	else{
		socket->SendDataOnly(fName,MAX_STR_LENGTH);
		socket->SendDataOnly(&acquisitionIndex,sizeof(acquisitionIndex));
		socket->SendDataOnly(&frameIndex,sizeof(frameIndex));
		socket->SendDataOnly(retval,MOENCH_DATA_BYTES);
	}
	//return ok/fail


	delete [] retval;
	delete [] origVal;
	delete [] raw;

	return ret;

}




int	slsReceiverTCPIPInterface::gotthard_read_frame(){
	ret=OK;
	char fName[MAX_STR_LENGTH]="";
	int acquisitionIndex = -1;
	int frameIndex= -1;
	int i;


	//retval is a full frame
	int bufferSize  = GOTTHARD_BUFFER_SIZE;
	int rnel 		= bufferSize/(sizeof(int));
	int* retval 	= new int[rnel];
	int* origVal 	= new int[rnel];
	//all initialized to 0
	for(i=0;i<rnel;i++)	retval[i]=0;
	for(i=0;i<rnel;i++)	origVal[i]=0;

	//only for full frames
	int onebuffersize = GOTTHARD_BUFFER_SIZE/GOTTHARD_PACKETS_PER_FRAME;
	int onedatasize = GOTTHARD_DATA_BYTES/GOTTHARD_PACKETS_PER_FRAME;


	//depending on shortframe or not
	if(shortFrame!=-1)
		bufferSize=GOTTHARD_SHORT_BUFFER_SIZE;
	char* raw;


	uint32_t index=-1,index2=0;
	uint32_t pindex=0,pindex2=0;
	uint32_t bindex=0,bindex2=0;
	uint64_t startAcquisitionIndex=0;
	uint64_t startFrameIndex=0;

	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}

	/**send garbage with -1 index to try again*/
	else if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex=-1;
		cout<<"haven't caught any frame yet"<<endl;
	}else{
		ret = OK;
		receiverBase->readFrame(fName,&raw,startAcquisitionIndex,startFrameIndex);

		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERYVERBOSE
			cout<<"data not ready for gui yet"<<endl;
#endif
		}else{
			if(shortFrame!=-1){
				bindex = (uint32_t)(*((uint32_t*)raw));
				pindex = (bindex & GOTTHARD_SHORT_PACKET_INDEX_MASK);
				index = ((bindex & GOTTHARD_SHORT_FRAME_INDEX_MASK) >> GOTTHARD_SHORT_FRAME_INDEX_OFFSET);
#ifdef VERYVERBOSE
				cout << "index:" << hex << index << endl;
#endif
			}else{
				bindex = ((uint32_t)(*((uint32_t*)raw)))+1;
				pindex = (bindex & GOTTHARD_PACKET_INDEX_MASK);
				index = ((bindex & GOTTHARD_FRAME_INDEX_MASK) >> GOTTHARD_FRAME_INDEX_OFFSET);
				bindex2 = ((uint32_t)(*((uint32_t*)((char*)(raw+onebuffersize)))))+1;
				pindex2 =(bindex2 & GOTTHARD_PACKET_INDEX_MASK);
				index2 =((bindex2 & GOTTHARD_FRAME_INDEX_MASK) >> GOTTHARD_FRAME_INDEX_OFFSET);
#ifdef VERYVERBOSE
				cout << "index1:" << hex << index << endl;
				cout << "index2:" << hex << index << endl;
#endif
			}

			memcpy(origVal,raw,bufferSize);
			raw=NULL;


			//1 adc
			if(shortFrame!=-1){
				if(bindex != 0xFFFFFFFF)
					memcpy((((char*)retval)+(GOTTHARD_SHORT_DATABYTES*shortFrame)),((char*) origVal)+4, GOTTHARD_SHORT_DATABYTES);
				else{
					index = startAcquisitionIndex - 1;
					cout << "Missing Packet,Not sending to gui" << endl;
				}
			}
			//all adc
			else{
				/*//ignore if half frame is missing
				if ((bindex != 0xFFFFFFFF) && (bindex2 != 0xFFFFFFFF)){*/

				//should be same frame
				if (index == index2){
					//ideal situation (should be odd, even(index+1))
					if(!pindex){
						memcpy(retval,((char*) origVal)+4, onedatasize);
						memcpy((((char*)retval)+onedatasize), ((char*) origVal)+10+onedatasize, onedatasize);
					}
					//swap to even,odd
					else{
						memcpy((((char*)retval)+onedatasize),((char*) origVal)+4, onedatasize);
						memcpy(retval, ((char*) origVal)+10+onedatasize, onedatasize);
						index=index2;
					}
				}else
					cout << "different frames caught. frame1:"<< hex << index << ":"<<pindex<<" frame2:" << hex << index2 << ":"<<pindex2<<endl;
				/*}
				else{
					index = startIndex - 1;
					cout << "Missing Packet,Not sending to gui" << endl;
				}*/
			}

			acquisitionIndex = index-startAcquisitionIndex;
			if(acquisitionIndex ==  -1)
				startFrameIndex = -1;
			else
				frameIndex = index-startFrameIndex;

#ifdef VERY_VERY_DEBUG
			cout << "acquisitionIndex calculated is:" << acquisitionIndex << endl;
			cout << "frameIndex calculated is:" << frameIndex << endl;
			cout << "index:" << index << endl;
			cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
			cout << "startFrameIndex:" << startFrameIndex << endl;
#endif
		}
	}

#ifdef VERYVERBOSE
	if(frameIndex!=-1){
		cout << "fName:" << fName << endl;
		cout << "acquisitionIndex:" << acquisitionIndex << endl;
		cout << "frameIndex:" << frameIndex << endl;
		cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
		cout << "startFrameIndex:" << startFrameIndex << endl;
	}
#endif



#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	else{
		socket->SendDataOnly(fName,MAX_STR_LENGTH);
		socket->SendDataOnly(&acquisitionIndex,sizeof(acquisitionIndex));
		socket->SendDataOnly(&frameIndex,sizeof(frameIndex));
		socket->SendDataOnly(retval,GOTTHARD_DATA_BYTES);
	}

	delete [] retval;
	delete [] origVal;
	delete [] raw;

	return ret;
}







int	slsReceiverTCPIPInterface::propix_read_frame(){
	ret=OK;
	char fName[MAX_STR_LENGTH]="";
	int acquisitionIndex = -1;
	int frameIndex= -1;
	int i;


	//retval is a full frame
	int bufferSize  = PROPIX_BUFFER_SIZE;
	int onebuffersize = bufferSize/PROPIX_PACKETS_PER_FRAME;
	int onedatasize = PROPIX_DATA_BYTES;

	char* raw;
	int rnel 		= bufferSize/(sizeof(int));
	int* retval 	= new int[rnel];
	int* origVal 	= new int[rnel];
	//all initialized to 0
	for(i=0;i<rnel;i++)	retval[i]=0;
	for(i=0;i<rnel;i++)	origVal[i]=0;


	uint32_t index=-1,index2=0;
	uint32_t pindex=0,pindex2=0;
	uint32_t bindex=0,bindex2=0;
	uint64_t startAcquisitionIndex=0;
	uint64_t startFrameIndex=0;

	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}

	/**send garbage with -1 index to try again*/
	else if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex=-1;
		cout<<"haven't caught any frame yet"<<endl;
	}else{
		ret = OK;
		receiverBase->readFrame(fName,&raw,startAcquisitionIndex,startFrameIndex);

		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERYVERBOSE
			cout<<"data not ready for gui yet"<<endl;
#endif
		}else{
			bindex = ((uint32_t)(*((uint32_t*)raw)))+1;
			pindex = (bindex & PROPIX_PACKET_INDEX_MASK);
			index = ((bindex & PROPIX_FRAME_INDEX_MASK) >> PROPIX_FRAME_INDEX_OFFSET);
			bindex2 = ((uint32_t)(*((uint32_t*)((char*)(raw+onebuffersize)))))+1;
			pindex2 =(bindex2 & PROPIX_PACKET_INDEX_MASK);
			index2 =((bindex2 & PROPIX_FRAME_INDEX_MASK) >> PROPIX_FRAME_INDEX_OFFSET);
#ifdef VERYVERBOSE
			cout << "index1:" << hex << index << endl;
			cout << "index2:" << hex << index << endl;
#endif

			memcpy(origVal,raw,bufferSize);
			raw=NULL;

			/*//ignore if half frame is missing
				if ((bindex != 0xFFFFFFFF) && (bindex2 != 0xFFFFFFFF)){*/

			//should be same frame
			if (index == index2){
				//ideal situation (should be odd, even(index+1))
				if(!pindex){
					memcpy(retval,((char*) origVal)+4, onedatasize);
					memcpy((((char*)retval)+onedatasize), ((char*) origVal)+10+onedatasize, onedatasize);
				}
				//swap to even,odd
				else{
					memcpy((((char*)retval)+onedatasize),((char*) origVal)+4, onedatasize);
					memcpy(retval, ((char*) origVal)+10+onedatasize, onedatasize);
					index=index2;
				}
			}else
				cout << "different frames caught. frame1:"<< hex << index << ":"<<pindex<<" frame2:" << hex << index2 << ":"<<pindex2<<endl;
			/*}
				else{
					index = startIndex - 1;
					cout << "Missing Packet,Not sending to gui" << endl;
				}*/


			acquisitionIndex = index-startAcquisitionIndex;
			if(acquisitionIndex ==  -1)
				startFrameIndex = -1;
			else
				frameIndex = index-startFrameIndex;

#ifdef VERY_VERY_DEBUG
			cout << "acquisitionIndex calculated is:" << acquisitionIndex << endl;
			cout << "frameIndex calculated is:" << frameIndex << endl;
			cout << "index:" << index << endl;
			cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
			cout << "startFrameIndex:" << startFrameIndex << endl;
#endif
		}
	}

#ifdef VERYVERBOSE
	if(frameIndex!=-1){
		cout << "fName:" << fName << endl;
		cout << "acquisitionIndex:" << acquisitionIndex << endl;
		cout << "frameIndex:" << frameIndex << endl;
		cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
		cout << "startFrameIndex:" << startFrameIndex << endl;
	}
#endif



#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	else{
		socket->SendDataOnly(fName,MAX_STR_LENGTH);
		socket->SendDataOnly(&acquisitionIndex,sizeof(acquisitionIndex));
		socket->SendDataOnly(&frameIndex,sizeof(frameIndex));
		socket->SendDataOnly(retval,PROPIX_DATA_BYTES);
	}

	delete [] retval;
	delete [] origVal;
	delete [] raw;

	return ret;
}










int	slsReceiverTCPIPInterface::eiger_read_frame(){
	ret=OK;

	char fName[MAX_STR_LENGTH]="";
	int acquisitionIndex = -1;
	int frameIndex= -1;
	uint32_t index=0;
	uint32_t subframenumber=-1;

	int frameSize   = EIGER_ONE_GIGA_ONE_PACKET_SIZE * packetsPerFrame;
	int dataSize 	= EIGER_ONE_GIGA_ONE_DATA_SIZE * packetsPerFrame;
	int oneDataSize = EIGER_ONE_GIGA_ONE_DATA_SIZE;
	if(tenGigaEnable){
		frameSize  	= EIGER_TEN_GIGA_ONE_PACKET_SIZE * packetsPerFrame;
		dataSize	= EIGER_TEN_GIGA_ONE_DATA_SIZE * packetsPerFrame;
		oneDataSize = EIGER_TEN_GIGA_ONE_DATA_SIZE;
	}
	char* raw;
	char* origVal 	= new char[frameSize];
	char* retval 	= new char[dataSize];
	uint64_t startAcquisitionIndex=0;
	uint64_t startFrameIndex=0;
	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}

	//send garbage with -1 index to try again
	else if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex=-1;
#ifdef VERYVERBOSE
		cout<<"haven't caught any frame yet"<<endl;
#endif
	}


	// acq started
	else{
		ret = OK;
		//read a frame
		receiverBase->readFrame(fName,&raw,startAcquisitionIndex,startFrameIndex);
		//send garbage with -1 index to try again
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERYVERBOSE
			cout<<"data not ready for gui yet"<<endl;
#endif
		}

		//proper frame
		else{//cout<<"**** got proper frame ******"<<endl;

			eiger_packet_footer_t* wbuf_footer;
			wbuf_footer = (eiger_packet_footer_t*)(raw + oneDataSize + sizeof(eiger_packet_header_t));
			index =(uint32_t)(*( (uint64_t*) wbuf_footer));
			index += (startFrameIndex-1);
			if(dynamicrange == 32){
				eiger_packet_header_t* wbuf_header;
				wbuf_header = (eiger_packet_header_t*) raw;
				subframenumber = *( (uint32_t*) wbuf_header->subFrameNumber);
			}

#ifdef VERYVERBOSE
			cout << "index:" << dec << index << endl;
			cout << "subframenumber:" << dec << subframenumber << endl;
#endif

			memcpy(origVal,raw,frameSize);
			raw=NULL;

			int c1=8;//first port
			int c2=(frameSize/2) + 8; //second port
			int retindex=0;
			int irow,ibytesperpacket;
			int linesperpacket = (16*1/dynamicrange);// 16:1 line, 8:2 lines, 4:4 lines, 32: 0.5
			int numbytesperlineperport=(EIGER_PIXELS_IN_ONE_ROW/EIGER_MAX_PORTS)*dynamicrange/8;//16:1024,8:512,4:256,32:2048
			int datapacketlength = EIGER_ONE_GIGA_ONE_DATA_SIZE;
			int total_num_bytes = EIGER_ONE_GIGA_ONE_PACKET_SIZE *(EIGER_ONE_GIGA_CONSTANT *dynamicrange)*2;

			if(tenGigaEnable){
				linesperpacket = (16*4/dynamicrange);// 16:4 line, 8:8 lines, 4:16 lines, 32: 2
				datapacketlength = EIGER_TEN_GIGA_ONE_DATA_SIZE;
				total_num_bytes = EIGER_TEN_GIGA_ONE_PACKET_SIZE*(EIGER_TEN_GIGA_CONSTANT*dynamicrange)*2;
			}
			//if 1GbE, one line is split into two packets for 32 bit mode, so its special
			else if(dynamicrange == 32){
				numbytesperlineperport = 1024;
				linesperpacket = 1; //we repeat this twice anyway for 32 bit
			}

			if(!bottom){

				for(irow=0;irow<EIGER_PIXELS_IN_ONE_COL/linesperpacket;++irow){
					ibytesperpacket=0;
					while(ibytesperpacket<datapacketlength){
						//first port
						memcpy(retval+retindex ,origVal+c1 ,numbytesperlineperport);
						retindex += numbytesperlineperport;
						c1 += numbytesperlineperport;
						if(dynamicrange == 32 && !tenGigaEnable){
							c1 += 16;
							memcpy(retval+retindex ,origVal+c1 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							c1 += numbytesperlineperport;
							c1 += 16;
						}
						//second port
						memcpy(retval+retindex ,origVal+c2 ,numbytesperlineperport);
						retindex += numbytesperlineperport;
						c2 += numbytesperlineperport;
						if(dynamicrange == 32 && !tenGigaEnable){
							c2 += 16;
							memcpy(retval+retindex ,origVal+c2 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							c2 += numbytesperlineperport;
							c2 += 16;
						}
						ibytesperpacket += numbytesperlineperport;
					}
					if(dynamicrange != 32 || tenGigaEnable) {
						c1 += 16;
						c2 += 16;
					}
				}

			}

			//bottom half module

			else{
				c1 = (frameSize/2) - numbytesperlineperport - 8 ;
				c2 = total_num_bytes - numbytesperlineperport - 8;

				for(irow=0;irow<EIGER_PIXELS_IN_ONE_COL/linesperpacket;++irow){
					ibytesperpacket=0;
					while(ibytesperpacket<datapacketlength){
						if(dynamicrange == 32 && !tenGigaEnable){
							//first port first chip
							c1 -= (numbytesperlineperport + 16);
							memcpy(retval+retindex ,origVal+c1 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							//first port second chip
							c1 += (numbytesperlineperport+16);
							memcpy(retval+retindex ,origVal+c1 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							c1 -= (numbytesperlineperport*2+32);//1024*2+16*2
							//second port first chip
							c2 -= (numbytesperlineperport + 16);
							memcpy(retval+retindex ,origVal+c2 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							//second port second chip
							c2 += (numbytesperlineperport + 16);
							memcpy(retval+retindex ,origVal+c2 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							c2 -= (numbytesperlineperport*2+32);
						}else{
							//first port
							memcpy(retval+retindex ,origVal+c1 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							c1 -= numbytesperlineperport;
							//second port
							memcpy(retval+retindex ,origVal+c2 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							c2 -= numbytesperlineperport;
						}
						ibytesperpacket += numbytesperlineperport;
					}
					if(dynamicrange != 32 || tenGigaEnable) {
						c1 -= 16;
						c2 -= 16;
					}
				}

			}


			acquisitionIndex = index-startAcquisitionIndex;
			if(acquisitionIndex ==  -1)
				startFrameIndex = -1;
			else
				frameIndex = index-startFrameIndex;
#ifdef VERY_VERY_DEBUG
			cout << "acquisitionIndex calculated is:" << acquisitionIndex << endl;
			cout << "frameIndex calculated is:" << frameIndex << endl;
			cout << "index:" << index << endl;
			cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
			cout << "startFrameIndex:" << startFrameIndex << endl;
			cout << "subframenumber:" << subframenumber << endl;
#endif
		}
	}

#ifdef VERYVERBOSE
	if(frameIndex!=-1){
		cout << "fName:" << fName << endl;
		cout << "acquisitionIndex:" << acquisitionIndex << endl;
		cout << "frameIndex:" << frameIndex << endl;
		cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
		cout << "startFrameIndex:" << startFrameIndex << endl;
		cout << "subframenumber:" << subframenumber << endl;
	}
#endif



#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	else{
		socket->SendDataOnly(fName,MAX_STR_LENGTH);
		socket->SendDataOnly(&acquisitionIndex,sizeof(acquisitionIndex));
		socket->SendDataOnly(&frameIndex,sizeof(frameIndex));
		socket->SendDataOnly(&subframenumber,sizeof(subframenumber));
		socket->SendDataOnly(retval,dataSize);
	}

	delete [] retval;
	delete [] origVal;
	delete [] raw;

	return ret;
}







int	slsReceiverTCPIPInterface::jungfrau_read_frame(){
	ret=OK;

	char fName[MAX_STR_LENGTH]="";
	int acquisitionIndex = -1;
	int frameIndex= -1;
	int64_t currentIndex=0;
	uint64_t startAcquisitionIndex=0;
	uint64_t startFrameIndex=0;
	strcpy(mess,"Could not read frame\n");


	int frameSize   = JFRAU_ONE_PACKET_SIZE * packetsPerFrame;
	int dataSize 	= JFRAU_ONE_DATA_SIZE * packetsPerFrame;
	int oneDataSize = JFRAU_ONE_DATA_SIZE;

	char* raw;
	char* origVal 	= new char[frameSize];
	char* retval 	= new char[dataSize];
	char* blackpacket = new char[oneDataSize];

	for(int i=0;i<oneDataSize;i++)
		blackpacket[i]='0';


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}

	//send garbage with -1 currentIndex to try again
	else if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex=-1;
#ifdef VERYVERBOSE
		cout<<"haven't caught any frame yet"<<endl;
#endif
	}


	// acq started
	else{
		ret = OK;
		//read a frame
		receiverBase->readFrame(fName,&raw,startAcquisitionIndex,startFrameIndex);
		//send garbage with -1 index to try again
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERYVERBOSE
			cout<<"data not ready for gui yet"<<endl;
#endif
		}

		//proper frame
		else{
			//cout<<"**** got proper frame ******"<<endl;
			memcpy(origVal,raw,frameSize);
			raw=NULL;

			//fixed frame number
			jfrau_packet_header_t* header = (jfrau_packet_header_t*) origVal;
			currentIndex = (*( (uint32_t*) header->frameNumber))&0xffffff;
#ifdef VERYVERBOSE
			cout << "currentIndex:" << dec << currentIndex << endl;
#endif

			int64_t currentPacket = packetsPerFrame-1;
			int offsetsrc = 0;
			int offsetdest = 0;
			int64_t ifnum=-1;
			int64_t ipnum=-1;

			while(currentPacket >= 0){
				header = (jfrau_packet_header_t*) (origVal + offsetsrc);
				ifnum = (*( (uint32_t*) header->frameNumber))&0xffffff;
				ipnum = (*( (uint8_t*) header->packetNumber));
				if(ifnum != currentIndex) {
					cout << "current packet " << currentPacket << " Wrong Frame number " << ifnum << ", copying blank packet" << endl;
					memcpy(retval+offsetdest,blackpacket,oneDataSize);
					offsetdest += oneDataSize;
					//no need to increase offsetsrc as all packets will be wrong
					currentPacket--;
					continue;
				}
				if(ipnum!= currentPacket){
					cout << "current packet " << currentPacket << " Wrong packet number " << ipnum << ", copying blank packet" << endl;
					memcpy(retval+offsetdest,blackpacket,oneDataSize);
					offsetdest += oneDataSize;
					//no need to increase offsetsrc until we get the right packet
					currentPacket--;
					continue;
				}
				offsetsrc+=JFRAU_HEADER_LENGTH;
				memcpy(retval+offsetdest,origVal+offsetsrc,oneDataSize);
				offsetdest += oneDataSize;
				offsetsrc += oneDataSize;
				currentPacket--;
			}


			acquisitionIndex = (int)(currentIndex-startAcquisitionIndex);
			if(acquisitionIndex ==  -1)
				startFrameIndex = -1;
			else
				frameIndex = (int)(currentIndex-startFrameIndex);
#ifdef VERY_VERY_DEBUG
			cout << "acquisitionIndex calculated is:" << acquisitionIndex << endl;
			cout << "frameIndex calculated is:" << frameIndex << endl;
			cout << "currentIndex:" << currentIndex << endl;
			cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
			cout << "startFrameIndex:" << startFrameIndex << endl;
#endif
		}
	}

#ifdef VERYVERBOSE
	if(frameIndex!=-1){
		cout << "fName:" << fName << endl;
		cout << "acquisitionIndex:" << acquisitionIndex << endl;
		cout << "frameIndex:" << frameIndex << endl;
		cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
		cout << "startFrameIndex:" << startFrameIndex << endl;
	}
#endif



#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	else{
		socket->SendDataOnly(fName,MAX_STR_LENGTH);
		socket->SendDataOnly(&acquisitionIndex,sizeof(acquisitionIndex));
		socket->SendDataOnly(&frameIndex,sizeof(frameIndex));
		socket->SendDataOnly(retval,dataSize);
	}

	delete [] retval;
	delete [] origVal;
	delete [] raw;

	return ret;
}






int slsReceiverTCPIPInterface::set_read_frequency(){
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not set receiver read frequency\n");


	// receive arguments
	if(socket->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
		}
		/*
		else if((receiverBase->getStatus()==RUNNING) && (index >= 0)){
			ret = FAIL;
			strcpy(mess,"cannot set up receiver mode when receiver is running\n");
		}*/
		else{
			if(index >= 0){
				ret = receiverBase->setFrameToGuiFrequency(index);
				if(ret == FAIL)
					strcpy(mess, "Could not allocate memory for listening fifo\n");
			}
			retval=receiverBase->getFrameToGuiFrequency();
			if(index>=0 && retval!=index)
				ret = FAIL;
		}
	}

#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::enable_file_write(){
	ret=OK;
	int retval=-1;
	int enable;
	strcpy(mess,"Could not set/get enable file write\n");


	// receive arguments
	if(socket->ReceiveDataOnly(&enable,sizeof(enable)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

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

	if(socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}



int64_t slsReceiverTCPIPInterface::getReceiverVersion(){
	int64_t retval = SVNREV;
	retval= (retval <<32) | SVNDATE;
	return retval;
}




int	slsReceiverTCPIPInterface::start_readout(){
	ret=OK;
	enum runStatus retval;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}else{
		receiverBase->startReadout();
		retval = receiverBase->getStatus();
		if((retval == TRANSMITTING) || (retval == RUN_FINISHED) || (retval == IDLE))
			ret = OK;
		else
			ret = FAIL;
	}
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));
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
	if(socket->ReceiveDataOnly(index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
		}
		else{
			if(index[0] == FRAME_PERIOD){
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
		if(index[0] == FRAME_PERIOD)
			cout << "acquisition period:" << retval << endl;
		else
			cout << "frame number:" << retval << endl;
	}else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::enable_compression() {
	ret=OK;
	int enable=-1;
	int retval=-100;
	strcpy(mess,"Could not enable/disable compression for receiver\n");

	// receive arguments
	if(socket->ReceiveDataOnly(&enable,sizeof(enable)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if(enable >= 0){
			if (lockStatus==1 && socket->differentClients==1){
				sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
				ret=FAIL;
			}
			else if (receiverBase == NULL){
				strcpy(mess,"Receiver not set up\n");
				ret=FAIL;
			}
			else if(receiverBase->getStatus()==RUNNING){
				strcpy(mess,"Cannot enable/disable compression while status is running\n");
				ret=FAIL;
			}
			else{
				if(enable >= 0)
					ret = receiverBase->setDataCompressionEnable(enable);
			}
		}

		if(ret != FAIL){
			if (receiverBase == NULL){
				strcpy(mess,"Receiver not set up\n");
				ret=FAIL;
			}else{
				retval = receiverBase->getDataCompressionEnable();
				if(enable >= 0 && retval != enable)
					ret = FAIL;
			}
		}

	}
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

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
	if(socket->ReceiveDataOnly(hostname,MAX_STR_LENGTH) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {

		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED, "%s\n", mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	if(retval == NULL)
		socket->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		socket->SendDataOnly(retval,MAX_STR_LENGTH);
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
	if(socket->ReceiveDataOnly(&dr,sizeof(dr)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
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
				strcpy(mess,"Receiver not set up\n");
				ret=FAIL;
			}else{
				if(dr > 0){
					ret = receiverBase->setDynamicRange(dr);
					if(ret == FAIL)
						strcpy(mess, "Could not allocate memory for fifo or could not start listening/writing threads\n");
				}
				retval = receiverBase->getDynamicRange();
				if(dr > 0 && retval != dr)
					ret = FAIL;
				else{
					dynamicrange = retval;
					if(myDetectorType == EIGER){
						if(!tenGigaEnable)
							packetsPerFrame = EIGER_ONE_GIGA_CONSTANT * dynamicrange * EIGER_MAX_PORTS;
						else
							packetsPerFrame = EIGER_TEN_GIGA_CONSTANT * dynamicrange * EIGER_MAX_PORTS;
					}else if (myDetectorType == JUNGFRAU)
						packetsPerFrame = JFRAU_PACKETS_PER_FRAME;
				}
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}







int slsReceiverTCPIPInterface::enable_overwrite() {
	ret=OK;
	int retval=-1;
	int index;
	strcpy(mess,"Could not enable/disable overwrite\n");


	// receive arguments
	if(socket->ReceiveDataOnly(&index,sizeof(index)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}







int slsReceiverTCPIPInterface::enable_tengiga() {
	ret=OK;
	int retval=-1;
	int val;
	strcpy(mess,"Could not enable/disable 10Gbe\n");


	// receive arguments
	if(socket->ReceiveDataOnly(&val,sizeof(val)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
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

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::set_fifo_depth() {
	ret=OK;
	int value=-1;
	int retval=-100;
	strcpy(mess,"Could not set/get fifo depth for receiver\n");

	// receive arguments
	if(socket->ReceiveDataOnly(&value,sizeof(value)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (ret==OK) {
		if(value >= 0){
			if (lockStatus==1 && socket->differentClients==1){
				sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
				ret=FAIL;
			}
			else if (receiverBase == NULL){
				strcpy(mess,"Receiver not set up\n");
				ret=FAIL;
			}
			else if(receiverBase->getStatus()==RUNNING){
				strcpy(mess,"Cannot set/get fifo depth while status is running\n");
				ret=FAIL;
			}
			else{
				if(value >= 0){
					ret = receiverBase->setFifoDepth(value);
				}
			}
		}


		if (receiverBase == NULL){
			strcpy(mess,"Receiver not set up\n");
			ret=FAIL;
		}else{
			retval = receiverBase->getFifoDepth();
			if(value >= 0 && retval != value)
				ret = FAIL;
		}

	}
#endif

	if(ret==OK && socket->differentClients){
		FILE_LOG(logDEBUG) << "Force update";
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	socket->SendDataOnly(&retval,sizeof(retval));

	//return ok/fail
	return ret;
}





































int slsReceiverTCPIPInterface::lock_receiver() {
	ret=OK;
	int lock;

	// receive arguments
	if(socket->ReceiveDataOnly(&lock,sizeof(lock)) < 0 ){
		sprintf(mess,"Error reading from socket\n");
		cout << "Error reading from socket (lock)" << endl;
		ret=FAIL;
	}
	// execute action if the arguments correctly arrived
	if(ret==OK){
		if (lock>=0) {
			if (lockStatus==0 || strcmp(socket->lastClientIP,socket->thisClientIP)==0 || strcmp(socket->lastClientIP,"none")==0) {
				lockStatus=lock;
				strcpy(socket->lastClientIP,socket->thisClientIP);
			}   else {
				ret=FAIL;
				sprintf(mess,"Receiver already locked by %s\n", socket->lastClientIP);
			}
		}
	}

	if (socket->differentClients && ret==OK)
		ret=FORCE_UPDATE;

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}
	else
		socket->SendDataOnly(&lockStatus,sizeof(lockStatus));

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
	if(socket->ReceiveDataOnly(&p_type,sizeof(p_type)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		cout << mess << endl;
		ret=FAIL;
	}

	if(socket->ReceiveDataOnly(&p_number,sizeof(p_number)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		cout << mess << endl;
		ret=FAIL;
	}

	// execute action if the arguments correctly arrived
	if (ret==OK) {
		if (socket->differentClients==1 && lockStatus==1 ) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",socket->lastClientIP);
		}
		else {
			if (p_number<1024) {
				sprintf(mess,"Too low port number %d\n", p_number);
				cout << mess << endl;
				ret=FAIL;
			}
			cout << "set port " << p_type << " to " << p_number <<endl;
			strcpy(oldLastClientIP, socket->lastClientIP);
			mySocket = new MySocketTCP(p_number);
		}
		if(mySocket){
			sd = mySocket->getErrorStatus();
			if (!sd){
				ret=OK;
				strcpy(socket->lastClientIP,oldLastClientIP);
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
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
	}else {
		socket->SendDataOnly(&p_number,sizeof(p_number));
		if(sd>=0){
			socket->Disconnect();
			delete socket;
			socket = mySocket;
		}
	}

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::get_last_client_ip() {
	ret=OK;

	if (socket->differentClients )
		ret=FORCE_UPDATE;

	socket->SendDataOnly(&ret,sizeof(ret));
	socket->SendDataOnly(socket->lastClientIP,sizeof(socket->lastClientIP));

	return ret;
}







int slsReceiverTCPIPInterface::send_update() {
	ret=OK;
	int ind;
	char defaultVal[MAX_STR_LENGTH]="";
	char* path = NULL;

	socket->SendDataOnly(socket->lastClientIP,sizeof(socket->lastClientIP));

	//index
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	ind=receiverBase->getFileIndex();

	socket->SendDataOnly(&ind,sizeof(ind));
#endif

	//filepath
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	path = receiverBase->getFilePath();
#endif
	if(path == NULL)
		socket->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		socket->SendDataOnly(path,MAX_STR_LENGTH);
		delete[] path;
	}


	//filename
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	path = receiverBase->getFileName();
#endif
	if(path == NULL)
		socket->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else{
		socket->SendDataOnly(path,MAX_STR_LENGTH);
		delete[] path;
	}

	if (lockStatus==0) {
		strcpy(socket->lastClientIP,socket->thisClientIP);
	}

	return ret;


}






int slsReceiverTCPIPInterface::update_client() {
	ret=OK;
	if (receiverBase == NULL){
		strcpy(mess,"Receiver not set up\n");
		ret=FAIL;
	}
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cprintf(RED,"%s\n",mess);
		socket->SendDataOnly(mess,sizeof(mess));
		return ret;
	}

	return send_update();
}







int slsReceiverTCPIPInterface::exit_server() {
	ret=GOODBYE;
	socket->SendDataOnly(&ret,sizeof(ret));
	strcpy(mess,"closing server");
	socket->SendDataOnly(mess,sizeof(mess));
	cprintf(RED,"%s\n",mess);
	return ret;
}





int slsReceiverTCPIPInterface::exec_command() {
	ret = OK;
	char cmd[MAX_STR_LENGTH];
	char answer[MAX_STR_LENGTH];
	int sysret=0;

	// receive arguments
	if(socket->ReceiveDataOnly(cmd,MAX_STR_LENGTH) < 0 ){
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	// execute action if the arguments correctly arrived
	if (ret==OK) {
#ifdef VERYVERBOSE
		cout << "executing command " << cmd << endl;
#endif
		if (lockStatus==0 || socket->differentClients==0)
			sysret=system(cmd);

		//should be replaced by popen
		if (sysret==0) {
			strcpy(answer,"Succeeded\n");
			if (lockStatus==1 && socket->differentClients==1)
				sprintf(answer,"Detector locked by %s\n", socket->lastClientIP);
		} else {
			strcpy(answer,"Failed\n");
			ret=FAIL;
		}
	} else
		strcpy(answer,"Could not receive the command\n");


	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(socket->SendDataOnly(answer,MAX_STR_LENGTH) < 0){
		strcpy(mess,"Error writing to socket");
		ret=FAIL;
	}

	//return ok/fail
	return ret;
}



/***callback functions***/
void slsReceiverTCPIPInterface::registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){
	startAcquisitionCallBack=func;
	pStartAcquisition=arg;
}

void slsReceiverTCPIPInterface::registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){
	acquisitionFinishedCallBack=func;
	pAcquisitionFinished=arg;
}

void slsReceiverTCPIPInterface::registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}





