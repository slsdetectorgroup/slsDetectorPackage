/********************************************//**
 * @file slsReceiverTCPIPInterface.cpp
 * @short interface between receiver and client
 ***********************************************/

#include "slsReceiverTCPIPInterface.h"
#include "UDPInterface.h"
#include "gitInfoReceiver.h"
#include "slsReceiverUsers.h"
#include "slsReceiver.h"

#include  <signal.h>	//SIGINT
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
	if(socket) {delete socket; socket=NULL;}
	if(receiverBase) {delete receiverBase; receiverBase=NULL;}
	closeFile(0);
}

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int &success, UDPInterface* rbase, int pn, bool bot):
		myDetectorType(GOTTHARD),
		receiverBase(rbase),
		ret(OK),
		lockStatus(0),
		shortFrame(-1),
		packetsPerFrame(GOTTHARD_PACKETS_PER_FRAME),
		dynamicrange(16),
		socket(NULL),
		killTCPServerThread(0),
		tenGigaEnable(0), portNumber(DEFAULT_PORTNO+2),
		bottom(bot){

  int port_no=portNumber;


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
#ifdef VERBOSE
			cout << "Function table assigned." << endl;
#endif
			
			//Catch signal SIGINT to close files properly
			signal(SIGINT,staticCloseFile);
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
	cout << "Creating TCP Server Thread" << endl;
	killTCPServerThread = 0;
	if(pthread_create(&TCPServer_thread, NULL,startTCPServerThread, (void*) this)){
		cout << "Could not create TCP Server thread" << endl;
		return FAIL;
	}
	//#ifdef VERBOSE
	cout << "TCP Server thread created successfully." << endl;
	//#endif
	return OK;
}


void slsReceiverTCPIPInterface::stop(){

	cout << "Shutting down UDP Socket" << endl;
	if(receiverBase)
		receiverBase->shutDownUDPSockets();

	cout << "Closing Files... " << endl;
		receiverBase->closeFile();


	cout<<"Shutting down TCP Socket and TCP thread"<<endl;
	killTCPServerThread = 1;
	socket->ShutDownSocket();
	cout<<"Socket closed"<<endl;
	void* status;
	pthread_join(TCPServer_thread, &status);
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
	int v=slsReceiverDefs::OK;

	while(1) {
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
			if(receiverBase)
				receiverBase->shutDownUDPSockets();

			cout << "Closing Files... " << endl;
			receiverBase->closeFile();

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


#ifdef VERBOSE
	for (int i=0;i<numberOfFunctions;i++)
		cout << "function " << i << "located at " << flist[i] << endl;
#endif
	return OK;

}





int slsReceiverTCPIPInterface::decode_function(){
	ret = FAIL;
	int n,fnum;
#ifdef VERBOSE
	cout <<  "receive data" << endl;
#endif
	n = socket->ReceiveDataOnly(&fnum,sizeof(fnum));
	if (n <= 0) {
#ifdef VERBOSE
		cout << "ERROR reading from socket " << n << ", " << fnum << endl;
#endif
		return FAIL;
	}
#ifdef VERBOSE
	else
		cout << "size of data received " << n <<endl;
#endif

#ifdef VERBOSE
	cout <<  "calling function fnum = "<< fnum << hex << ":"<< flist[fnum] << endl;
#endif

	if (fnum<0 || fnum>numberOfFunctions-1)
		fnum = numberOfFunctions-1;
	//calling function
	(this->*flist[fnum])();
	if (ret==FAIL)
		cout <<  "Error executing the function = " << fnum << endl;

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
	stop();
	cout << "Goodbye!" << endl;
	exit(-1);
}

void slsReceiverTCPIPInterface::staticCloseFile(int p){
	slsReceiverUsers::receiver->closeFile(p);
}


int slsReceiverTCPIPInterface::set_detector_type(){
	ret=OK;
	int retval=FAIL;
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
			myDetectorType = dr;
			ret=receiverBase->setDetectorType(dr);
			retval = myDetectorType;
		}
	}
//#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "detector type" << dr << endl;
	else
		cout << mess << endl;
//#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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
	char retval[MAX_STR_LENGTH]="";
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
 	 	 else
			strcpy(retval,receiverBase->setFileName(fName));
	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "file name:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	socket->SendDataOnly(retval,MAX_STR_LENGTH);

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::set_file_dir() {
	ret=OK;
	char retval[MAX_STR_LENGTH]="";
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
		else{
			strcpy(retval,receiverBase->setFilePath(fPath));
			// if file path doesnt exist
			if(strlen(fPath))
				if (strcmp(retval,fPath)){
					strcpy(mess,"receiver file path does not exist\n");
					ret=FAIL;
				}
		}

	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "file path:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	socket->SendDataOnly(retval,MAX_STR_LENGTH);

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
		else
			retval=receiverBase->setFileIndex(index);
	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "file index:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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
		else
			retval=receiverBase->setFrameIndexNeeded(index);
	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "frame index:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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






//LEO: is the client that commands the setup, or you just need the args?
int slsReceiverTCPIPInterface::setup_udp(){
	ret=OK;
	strcpy(mess,"could not set up udp connection");
	char retval[MAX_STR_LENGTH]="";
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
		else if(receiverBase->getStatus()==RUNNING){
			ret = FAIL;
			strcpy(mess,"cannot set up udp when receiver is running\n");
		}
		else{
			//set up udp port
			 sscanf(args[1],"%d",&udpport);
			 sscanf(args[2],"%d",&udpport2);
			 receiverBase->setUDPPortNo(udpport);
			 receiverBase->setUDPPortNo2(udpport2);
			 //setup udpip
			 //get ethernet interface or IP to listen to
			 temp = genericSocket::ipToName(args[0]);
			 if(temp=="none"){
				 ret = FAIL;
				 strcpy(mess, "failed to get ethernet interface or IP to listen to\n");
			 }
			 else{
				 strcpy(eth,temp.c_str());
				 if (strchr(eth,'.')!=NULL) {
					 strcpy(eth,"");
					 ret = FAIL;
				 }
				 FILE_LOG(logDEBUG) << __FILE__ << "::" << __func__ << " " << eth;
				 receiverBase->setEthernetInterface(eth);

				 //get mac address from ethernet interface
				 if (ret != FAIL)
					temp = genericSocket::nameToMac(eth);


				 if ((temp=="00:00:00:00:00:00") || (ret == FAIL)){
					 ret = FAIL;
					 strcpy(mess,"failed to get mac adddress to listen to\n");
					 cout << "mess:" << mess << endl;
				 }
				 else{
					 strcpy(retval,temp.c_str());
					 cout<<"mac:"<<retval<<endl;
				 }
			 }
		}
	}
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	socket->SendDataOnly(retval,MAX_STR_LENGTH);

	//return ok/fail
	return ret;
}






int slsReceiverTCPIPInterface::start_receiver(){
	ret=OK;
	ret=OK;
	enum runStatus s;
	char cstatus[15];
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
	else {
		s = receiverBase->getStatus();
		switch (s) {
		case ERROR:       	strcpy(cstatus,"error");	break;
		case WAITING:    	strcpy(cstatus,"waiting");	break;
		case RUNNING:     	strcpy(cstatus,"running");	break;
		case TRANSMITTING: 	strcpy(cstatus,"data");		break;
		case RUN_FINISHED:	strcpy(cstatus,"finished");	break;
		default:       		strcpy(cstatus,"idle");		break;
		}
		if(s == IDLE)
			ret=receiverBase->startReceiver(mess);
		else{
			sprintf(mess,"Cannot start Receiver as it is in %s state\n",cstatus);
			ret=FAIL;
		}
	}

#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		socket->SendDataOnly(mess,sizeof(mess));
	}
	//return ok/fail
	return ret;


}


int slsReceiverTCPIPInterface::stop_receiver(){
	ret=OK;

	strcpy(mess,"Could not stop receiver\n");

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	if (lockStatus==1 && socket->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
		ret=FAIL;
	}
	else if(receiverBase->getStatus()!=IDLE)
		ret=receiverBase->stopReceiver();
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_status(){
	ret=OK;
	enum runStatus retval;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	retval=receiverBase->getStatus();
#endif

	if(socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	socket->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_frames_caught(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	retval=receiverBase->getTotalFramesCaught();
#endif
	if(socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	socket->SendDataOnly(&retval,sizeof(retval));
	//return ok/fail
	return ret;


}


int	slsReceiverTCPIPInterface::get_frame_index(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	retval=receiverBase->getAcquisitionIndex();
#endif

	if(socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
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
		else
			receiverBase->resetTotalFramesCaught();
	}
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));

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
		else if(receiverBase->getStatus()==RUNNING){
			strcpy(mess,"Cannot set short frame while status is running\n");
			ret=FAIL;
		}
		else{
			retval=receiverBase->setShortFrame(index);
			shortFrame = retval;
			if(shortFrame==-1)
				packetsPerFrame=GOTTHARD_PACKETS_PER_FRAME;
			else
				packetsPerFrame=GOTTHARD_SHORT_PACKETS_PER_FRAME;
		}
	}
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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




int	slsReceiverTCPIPInterface::read_frame(){
	switch(myDetectorType){
	case MOENCH:
		return moench_read_frame();
	case EIGER:
		return eiger_read_frame();
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

	char* raw 		= new char[bufferSize];

	uint32_t startAcquisitionIndex=0;
	uint32_t startFrameIndex=0;
	uint32_t index = 0,bindex = 0, offset=0;

	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	/**send garbage with -1 index to try again*/
	if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex = -1;
		cout<<"haven't caught any frame yet"<<endl;
	}

	else{
		ret = OK;
		/*startIndex=receiverBase->getStartFrameIndex();*/
		receiverBase->readFrame(fName,&raw,index,startAcquisitionIndex,startFrameIndex);

		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERBOSE
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
#ifdef VERBOSE
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
#ifdef VERBOSE
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

#ifdef VERBOSE
	cout << "fName:" << fName << endl;
	cout << "acquisitionIndex:" << acquisitionIndex << endl;
	cout << "frameIndex:" << frameIndex << endl;
	cout << "startAcquisitionIndex:" << startAcquisitionIndex << endl;
	cout << "startFrameIndex:" << startFrameIndex << endl;
#endif

#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cout << "mess:" << mess << endl;
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
	char* raw 		= new char[bufferSize];


	uint32_t index=0,index2=0;
	uint32_t pindex=0,pindex2=0;
	uint32_t bindex=0,bindex2=0;
	uint32_t startAcquisitionIndex=0;
	uint32_t startFrameIndex=0;

	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS



	/**send garbage with -1 index to try again*/
	if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex=-1;
		cout<<"haven't caught any frame yet"<<endl;
	}else{
		ret = OK;
		/*startIndex=receiverBase->getStartFrameIndex();*/
		receiverBase->readFrame(fName,&raw,index,startAcquisitionIndex,startFrameIndex);

		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERBOSE
			cout<<"data not ready for gui yet"<<endl;
#endif
		}else{
			if(shortFrame!=-1){
				bindex = (uint32_t)(*((uint32_t*)raw));
				pindex = (bindex & GOTTHARD_SHORT_PACKET_INDEX_MASK);
				index = ((bindex & GOTTHARD_SHORT_FRAME_INDEX_MASK) >> GOTTHARD_SHORT_FRAME_INDEX_OFFSET);
#ifdef VERBOSE
				cout << "index:" << hex << index << endl;
#endif
			}else{
				bindex = ((uint32_t)(*((uint32_t*)raw)))+1;
				pindex = (bindex & GOTTHARD_PACKET_INDEX_MASK);
				index = ((bindex & GOTTHARD_FRAME_INDEX_MASK) >> GOTTHARD_FRAME_INDEX_OFFSET);
				bindex2 = ((uint32_t)(*((uint32_t*)((char*)(raw+onebuffersize)))))+1;
				pindex2 =(bindex2 & GOTTHARD_PACKET_INDEX_MASK);
				index2 =((bindex2 & GOTTHARD_FRAME_INDEX_MASK) >> GOTTHARD_FRAME_INDEX_OFFSET);
#ifdef VERBOSE
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

#ifdef VERBOSE
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
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cout << "mess:" << mess << endl;
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









int	slsReceiverTCPIPInterface::eiger_read_frame(){
	ret=OK;
	char fName[MAX_STR_LENGTH]="";
	int acquisitionIndex = -1;
	int frameIndex= -1;
	int i;
	uint32_t index=0;

	int frameSize   = EIGER_ONE_GIGA_ONE_PACKET_SIZE * packetsPerFrame;
	int dataSize 	= EIGER_ONE_GIGA_ONE_DATA_SIZE * packetsPerFrame;
	if(tenGigaEnable){
		frameSize  	= EIGER_TEN_GIGA_ONE_PACKET_SIZE * packetsPerFrame;
		dataSize	= EIGER_TEN_GIGA_ONE_DATA_SIZE * packetsPerFrame;
	}
	char* raw 		= new char[frameSize];
	char* origVal 	= new char[frameSize];
	char* retval 	= new char[dataSize];
	uint32_t startAcquisitionIndex=0;
	uint32_t startFrameIndex=0;
	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS



	/**send garbage with -1 index to try again*/
	if(!receiverBase->getFramesCaught()){
		startAcquisitionIndex=-1;
#ifdef VERBOSE
		cout<<"haven't caught any frame yet"<<endl;
#endif
	}
	/** acq started */
	else{
		ret = OK;
		/** read a frame */
		receiverBase->readFrame(fName,&raw,index,startAcquisitionIndex,startFrameIndex);
#ifdef VERBOSE
		cout << "index:" << dec << index << endl;
#endif
		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			startAcquisitionIndex = -1;
#ifdef VERBOSE
			cout<<"data not ready for gui yet"<<endl;
#endif
		}

		/**proper frame*/
		else{//cout<<"**** got proper frame ******"<<endl;

			memcpy(origVal,raw,frameSize);
			raw=NULL;

			int c1=8;//first port
			int c2=(frameSize/2) + 8; //second port
			int retindex=0;
			int irow,ibytesperpacket;
			int linesperpacket = (16/dynamicrange)* 1;// 16:1 line, 8:2 lines, 4:4 lines, 32: 0.5
			int numbytesperlineperport=(EIGER_PIXELS_IN_ONE_ROW/EIGER_MAX_PORTS)*dynamicrange/8;//16:1024,8:512,4:256,32:2048
			int datapacketlength = EIGER_ONE_GIGA_ONE_DATA_SIZE;
			int total_num_bytes = 1040*(16*dynamicrange)*2;

			if(tenGigaEnable){
				linesperpacket = (16/dynamicrange)* 4;// 16:4 line, 8:8 lines, 4:16 lines, 32: 2
				datapacketlength = EIGER_TEN_GIGA_ONE_DATA_SIZE;
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
						if(dynamicrange == 32){
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
						if(dynamicrange == 32){
							c2 += 16;
							memcpy(retval+retindex ,origVal+c2 ,numbytesperlineperport);
							retindex += numbytesperlineperport;
							c2 += numbytesperlineperport;
							c2 += 16;
						}
						ibytesperpacket += numbytesperlineperport;
					}
					if(dynamicrange != 32) {
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
						if(dynamicrange == 32){
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
					if(dynamicrange != 32) {
						c1 -= 16;
						c2 -= 16;
					}
				}

			}

			//64 bit htonl cuz of endianness
			for(i=0;i<(1024*(16*dynamicrange)*2)/8;i++){
				(*(((uint64_t*)retval)+i)) = be64toh(((uint64_t)(*(((uint64_t*)retval)+i))));
			 /*
			  int64_t temp;
			  temp = ((uint64_t)(*(((uint64_t*)retval)+i)));
			  temp = ((temp << 8) & 0xFF00FF00FF00FF00ULL ) | ((temp >> 8) & 0x00FF00FF00FF00FFULL );
			  temp = ((temp << 16) & 0xFFFF0000FFFF0000ULL ) | ((temp >> 16) & 0x0000FFFF0000FFFFULL );
			  temp =  (temp << 32) | ((temp >> 32) & 0xFFFFFFFFULL);
			  (*(((uint64_t*)retval)+i)) = temp;
			  */
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

#ifdef VERBOSE
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
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cout << "mess:" << mess << endl;
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
		}/*
		else if((receiverBase->getStatus()==RUNNING) && (index >= 0)){
			ret = FAIL;
			strcpy(mess,"cannot set up receiver mode when receiver is running\n");
		}*/
		else
			retval=receiverBase->setNFrameToGui(index);
	}

#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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
		else{
			if(enable >= 0)
				receiverBase->setEnableFileWrite(enable);
			retval=receiverBase->getEnableFileWrite();
			if((enable!=-1)&&(enable!=retval))
				ret=FAIL;
		}
	}
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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



int slsReceiverTCPIPInterface::get_id(){
	ret=OK;
	int64_t retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	retval = getReceiverVersion();
#endif

	if(socket->differentClients){
		cout << "Force update" << endl;
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
	receiverBase->startReadout();
	retval = receiverBase->getStatus();
	if((retval == TRANSMITTING) || (retval == RUN_FINISHED) || (retval == IDLE))
		ret = OK;
	else
		ret = FAIL;
#endif

	if(socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
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
		else{
			if(index[0] == slsReceiverDefs::FRAME_PERIOD)
				retval=receiverBase->setAcquisitionPeriod(index[1]);
			else
				retval=receiverBase->setNumberOfFrames(index[1]);
		}
	}
#ifdef VERBOSE
	if(ret!=FAIL){
		if(index[0] == slsReceiverDefs::FRAME_PERIOD)
			cout << "acquisition period:" << retval << endl;
		else
			cout << "frame number:" << retval << endl;
	}else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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
			else if(receiverBase->getStatus()==RUNNING){
				strcpy(mess,"Cannot enable/disable compression while status is running\n");
				ret=FAIL;
			}
			else
				ret = receiverBase->enableDataCompression(enable);
		}

		retval=receiverBase->getDataCompression();
	}
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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




int slsReceiverTCPIPInterface::set_detector_hostname() {
	ret=OK;
	char retval[MAX_STR_LENGTH]="";
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
 	 	 else{
 	 		receiverBase->initialize(hostname);
			strcpy(retval,receiverBase->getDetectorHostname());
 	 	 }
	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "hostname:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	socket->SendDataOnly(retval,MAX_STR_LENGTH);

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
		else if(myDetectorType == EIGER){
			switch(dr){
				case 4:
				case 8:
				case 16:
				case 32:break;
				default:
					sprintf(mess,"This dynamic range does not exist for eiger: %d\n",dr);
					ret=FAIL;
					break;
			}
		}
		if(ret!=FAIL){
			retval=receiverBase->setDynamicRange(dr);
			dynamicrange = dr;
			 if(myDetectorType == EIGER){
				 if(!tenGigaEnable)
					 packetsPerFrame = EIGER_ONE_GIGA_CONSTANT * dynamicrange * EIGER_MAX_PORTS;
				 else
					 packetsPerFrame = EIGER_TEN_GIGA_CONSTANT * dynamicrange * EIGER_MAX_PORTS;
			 }
		}
	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "dynamic range" << dr << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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
		else{
			if(index >= 0)
				receiverBase->setEnableOverwrite(index);
			retval=receiverBase->getEnableOverwrite();
		}
	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "overwrite:" << retval << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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
		else{
			retval=receiverBase->enableTenGiga(val);
			if((val!=-1) && (val != retval))
				ret = FAIL;
			else
				tenGigaEnable = retval;
		}
	}
#ifdef VERBOSE
	if(ret!=FAIL)
		cout << "10Gbe:" << val << endl;
	else
		cout << mess << endl;
#endif
#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
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
	if (ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	else
		socket->SendDataOnly(&lockStatus,sizeof(lockStatus));

	//return ok/fail
	return ret;
}







int slsReceiverTCPIPInterface::set_port() {
	ret=OK;
	MySocketTCP* mySocket=NULL;
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
			mySocket = new MySocketTCP(p_number);
		}
		if(mySocket){
			sd = mySocket->getErrorStatus();
			if (!sd){
				ret=OK;
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
	if (ret==FAIL) {
		socket->SendDataOnly(mess,sizeof(mess));
	} else {
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
	char path[MAX_STR_LENGTH];

	socket->SendDataOnly(socket->lastClientIP,sizeof(socket->lastClientIP));

	//index
#ifdef SLS_RECEIVER_UDP_FUNCTIONS

	ind=receiverBase->getFileIndex();

	socket->SendDataOnly(&ind,sizeof(ind));
#endif

	//filepath
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	strcpy(path,receiverBase->getFilePath());
#endif
	socket->SendDataOnly(path,MAX_STR_LENGTH);


	//filename
#ifdef SLS_RECEIVER_UDP_FUNCTIONS
	strcpy(path,receiverBase->getFileName());
#endif
	socket->SendDataOnly(path,MAX_STR_LENGTH);


	if (lockStatus==0) {
		strcpy(socket->lastClientIP,socket->thisClientIP);
	}

	return ret;


}






int slsReceiverTCPIPInterface::update_client() {
	ret=OK;
	socket->SendDataOnly(&ret,sizeof(ret));

	return send_update();
}







int slsReceiverTCPIPInterface::exit_server() {
	ret=GOODBYE;
	socket->SendDataOnly(&ret,sizeof(ret));
	strcpy(mess,"closing server");
	socket->SendDataOnly(mess,sizeof(mess));
	cout << mess << endl;
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
#ifdef VERBOSE
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





