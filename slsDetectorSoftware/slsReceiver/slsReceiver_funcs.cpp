/********************************************//**
 * @file slsReceiver_funcs.h
 * @short interface between receiver and client
 ***********************************************/

#include "slsReceiver_funcs.h"
#include "slsReceiverFunctionList.h"
#include "svnInfoReceiver.h"
#include "slsReceiverUsers.h"
#include "slsDetectorBase.h"

#include  <signal.h>	//SIGINT
#include  <stdlib.h>	//EXIT

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
using namespace std;


int slsReceiverFuncs::file_des(-1);
int slsReceiverFuncs::socketDescriptor(-1);


slsReceiverFuncs::~slsReceiverFuncs() {
	if(socket) delete socket;
	closeFile(0);
}


slsReceiverFuncs::slsReceiverFuncs(int argc, char *argv[], int &success):
		myDetectorType(GOTTHARD),
		ret(OK),
		lockStatus(0),
		shortFrame(-1),
		packetsPerFrame(GOTTHARD_PACKETS_PER_FRAME),
		socket(NULL){

	int port_no = DEFAULT_PORTNO+2;
	ifstream infile;
	string sLine,sargname;
	int iline = 0;


	success=OK;
	string fname = "";

	//parse command line for config
	for(int iarg=1;iarg<argc;iarg++){
		if((!strcasecmp(argv[iarg],"-config"))||(!strcasecmp(argv[iarg],"-f"))){
			if(iarg+1==argc){
				cout << "no config file name given. Exiting." << endl;
				success=FAIL;
			}else
				fname.assign(argv[iarg+1]);
		}
	}
	if((!fname.empty()) && (success == OK)){
#ifdef VERBOSE
		std::cout<< "config file name "<< fname << std::endl;
#endif
		infile.open(fname.c_str(), ios_base::in);
		if (infile.is_open()) {
			while(infile.good()){
				getline(infile,sLine);
				iline++;
#ifdef VERBOSE
				cout <<  sLine << endl;
#endif
				if(sLine.find('#')!=string::npos){
#ifdef VERBOSE
					cout << "Line is a comment " << endl;
#endif
					continue;
				}else if(sLine.length()<2){
#ifdef VERBOSE
					cout << "Empty line " << endl;
#endif
					continue;
				}else{
					istringstream sstr(sLine);
					//parameter name
					if(sstr.good())
						sstr >> sargname;

					//tcp port
					if(sargname=="rx_tcpport"){
						if(sstr.good()) {
							sstr >> sargname;
							if(sscanf(sargname.c_str(),"%d",&port_no))
								cout<<"dataport:"<<port_no<<endl;
							else{
								cout << "could not decode port in config file. Exiting." << endl;
								success=FAIL;
							}
						}
					}
				}
			}
			infile.close();
		}else {
			cout << "Error opening configuration file " << fname << endl;
			success = FAIL;
		}
#ifdef VERBOSE
		cout << "Read configuration file of " << iline << " lines" << endl;
#endif

	}



	//parse command line for port etc.. more priority
	if(success == OK){
		for(int iarg=1;iarg<argc;iarg++){

			//tcp port
			if(!strcasecmp(argv[iarg],"-rx_tcpport")){
				if(iarg+1==argc){
					cout << "no port given after -rx_tcpport in command line. Exiting." << endl;
					success=FAIL;
				}else{
					if(sscanf(argv[iarg+1],"%d",&port_no)){
						cout<<"dataport:"<<port_no<<endl;
						iarg++;
					}else{
						cout << "could not decode port in command line. \n\nExiting." << endl;
						success=FAIL;
					}
				}
			}
			else{
				cout << "Unknown argument:" << argv[iarg] << endl;
				success=FAIL;
			}
		}
	}


	//help
	if(success != OK){
		cout << "Help Commands " << endl;
		cout << "rx_tcpport:\t TCP Communication Port with the client. Default:1954. " << endl << endl;
	}



	//create socket
	if(success == OK){
		socket = new MySocketTCP(port_no);
		if (socket->getErrorStatus()) {
			success = FAIL;
			delete socket;
			socket=NULL;
		}	else {
			//initialize variables
			strcpy(socket->lastClientIP,"none");
			strcpy(socket->thisClientIP,"none1");
			strcpy(mess,"dummy message");

			function_table();
#ifdef VERBOSE
			cout << "Function table assigned." << endl;
#endif
			slsReceiverList =  new slsReceiverFunctionList();

			//Catch signal SIGINT to close files properly
			signal(SIGINT,staticCloseFile);


			file_des=socket->getFileDes();
			socketDescriptor=socket->getsocketDescriptor();

			//success = OK;
		}
	}

}

void slsReceiverFuncs::start(){

  int v=slsDetectorDefs::OK;

	while(v!=GOODBYE) {
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
	}
	


}


int slsReceiverFuncs::function_table(){

	for (int i=0;i<numberOfFunctions;i++)
		flist[i]=&slsReceiverFuncs::M_nofunc;


	flist[F_GET_DETECTOR_TYPE]		=	&slsReceiverFuncs::set_detector_type;
	flist[F_SET_FILE_NAME]			=	&slsReceiverFuncs::set_file_name;
	flist[F_SET_FILE_PATH]			=	&slsReceiverFuncs::set_file_dir;
	flist[F_SET_FILE_INDEX]		 	=	&slsReceiverFuncs::set_file_index;
	flist[F_SET_FRAME_INDEX]	 	=	&slsReceiverFuncs::set_frame_index;
	flist[F_SETUP_UDP]				=	&slsReceiverFuncs::setup_udp;
	flist[F_START_RECEIVER]			=	&slsReceiverFuncs::start_receiver;
	flist[F_STOP_RECEIVER]			=	&slsReceiverFuncs::stop_receiver;
	flist[F_GET_RECEIVER_STATUS]	=	&slsReceiverFuncs::get_status;
	flist[F_GET_FRAMES_CAUGHT]		=	&slsReceiverFuncs::get_frames_caught;
	flist[F_GET_FRAME_INDEX]		=	&slsReceiverFuncs::get_frame_index;
	flist[F_RESET_FRAMES_CAUGHT]	=	&slsReceiverFuncs::reset_frames_caught;
	flist[F_READ_FRAME]				=	&slsReceiverFuncs::read_frame;
	flist[F_READ_RECEIVER_FREQUENCY]= 	&slsReceiverFuncs::set_read_frequency;
	flist[F_ENABLE_FILE_WRITE]		=	&slsReceiverFuncs::enable_file_write;
	flist[F_GET_ID]					=	&slsReceiverFuncs::get_version;
	flist[F_CONFIGURE_MAC]			=	&slsReceiverFuncs::set_short_frame;
	flist[F_START_READOUT]			= 	&slsReceiverFuncs::start_readout;
	flist[F_SET_TIMER]				= 	&slsReceiverFuncs::set_timer;
	flist[F_ENABLE_COMPRESSION]		= 	&slsReceiverFuncs::enable_compression;
	flist[F_SET_DETECTOR_HOSTNAME]	= 	&slsReceiverFuncs::set_detector_hostname;
	flist[F_SET_DYNAMIC_RANGE]		= 	&slsReceiverFuncs::set_dynamic_range;
	flist[F_ENABLE_OVERWRITE]		= 	&slsReceiverFuncs::enable_overwrite;



	//General Functions
	flist[F_LOCK_SERVER]			=	&slsReceiverFuncs::lock_receiver;
	flist[F_SET_PORT]				=	&slsReceiverFuncs::set_port;
	flist[F_GET_LAST_CLIENT_IP]		=	&slsReceiverFuncs::get_last_client_ip;
	flist[F_UPDATE_CLIENT]			=	&slsReceiverFuncs::update_client;
	flist[F_EXIT_SERVER]			=	&slsReceiverFuncs::exit_server;		//not implemented in client
	flist[F_EXEC_COMMAND]			=	&slsReceiverFuncs::exec_command;	//not implemented in client


#ifdef VERBOSE
	for (i=0;i<numberOfFunctions;i++)
		cout << "function " << i << "located at " << flist[i] << endl;
#endif
	return OK;

}





int slsReceiverFuncs::decode_function(){
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






int slsReceiverFuncs::M_nofunc(){

	ret=FAIL;
	sprintf(mess,"Unrecognized Function\n");
	cout << mess << endl;

	socket->SendDataOnly(&ret,sizeof(ret));
	socket->SendDataOnly(mess,sizeof(mess));

	return GOODBYE;
}




void slsReceiverFuncs::closeFile(int p){
	cout<<"Closing Files... "<<endl;
	slsReceiverList->closeFile();
	cout << "Goodbye!" << endl;
	exit(-1);
}

void slsReceiverFuncs::staticCloseFile(int p){
	slsReceiverUsers::receiver->closeFile(p);
}


int slsReceiverFuncs::set_detector_type(){
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else{
			myDetectorType = dr;
			ret=slsReceiverList->setDetectorType(dr);
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







int slsReceiverFuncs::set_file_name() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {

		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
 	 	 else
			strcpy(retval,slsReceiverList->setFileName(fName));
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






int slsReceiverFuncs::set_file_dir() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}/*
		else if((strlen(fPath))&&(slsReceiverList->getStatus()==RUNNING)){
			strcpy(mess,"Can not set file path while receiver running\n");
			ret = FAIL;
		}*/
		else{
			strcpy(retval,slsReceiverList->setFilePath(fPath));
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






int slsReceiverFuncs::set_file_index() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else
			retval=slsReceiverList->setFileIndex(index);
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








int slsReceiverFuncs::set_frame_index() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else
			retval=slsReceiverList->setFrameIndexNeeded(index);
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







int slsReceiverFuncs::setup_udp(){
	ret=OK;
	strcpy(mess,"could not set up udp connection");
	char retval[MAX_STR_LENGTH]="";
	char args[2][MAX_STR_LENGTH];

	string temp;
	int udpport;
	char eth[MAX_STR_LENGTH];


	// receive arguments

	if(socket->ReceiveDataOnly(args,sizeof(args)) < 0 ){
		strcpy(mess,"Error reading from socket\n");
		ret = FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if(slsReceiverList->getStatus()==RUNNING){
			ret = FAIL;
			strcpy(mess,"cannot set up udp when receiver is running\n");
		}
		else{
			//set up udp port
			 sscanf(args[1],"%d",&udpport);
			 slsReceiverList->setUDPPortNo(udpport);

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
				 cout<<"eth:"<<eth<<endl;
				 slsReceiverList->setEthernetInterface(eth);

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






int slsReceiverFuncs::start_receiver(){
	ret=OK;
	ret=OK;
	enum runStatus s;
	char cstatus[15];
	strcpy(mess,"Could not start receiver\n");

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (lockStatus==1 && socket->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
		ret=FAIL;
	}
	/*
	else if(!strlen(slsReceiverList->getFilePath())){
		strcpy(mess,"receiver not set up. set receiver ip again.\n");
		ret = FAIL;
	}
	*/
	else {
		s = slsReceiverList->getStatus();
		strcpy(cstatus, slsDetectorBase::runStatusType(s).c_str());
		if(s == IDLE)
			ret=slsReceiverList->startReceiver(mess);
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
	if(ret==FAIL)
		socket->SendDataOnly(mess,sizeof(mess));
	//return ok/fail
	return ret;


}


int slsReceiverFuncs::stop_receiver(){
	ret=OK;

	strcpy(mess,"Could not stop receiver\n");

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (lockStatus==1 && socket->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
		ret=FAIL;
	}
	else if(slsReceiverList->getStatus()!=IDLE)
		ret=slsReceiverList->stopReceiver();
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


int	slsReceiverFuncs::get_status(){
	ret=OK;
	enum runStatus retval;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	retval=slsReceiverList->getStatus();
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


int	slsReceiverFuncs::get_frames_caught(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	retval=slsReceiverList->getTotalFramesCaught();
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


int	slsReceiverFuncs::get_frame_index(){
	ret=OK;
	int retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	retval=slsReceiverList->getAcquisitionIndex();
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


int	slsReceiverFuncs::reset_frames_caught(){
	ret=OK;

	strcpy(mess,"Could not reset frames caught\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else
			slsReceiverList->resetTotalFramesCaught();
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






int slsReceiverFuncs::set_short_frame() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else if(slsReceiverList->getStatus()==RUNNING){
			strcpy(mess,"Cannot set short frame while status is running\n");
			ret=FAIL;
		}
		else{
			retval=slsReceiverList->setShortFrame(index);
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




int	slsReceiverFuncs::read_frame(){
	switch(myDetectorType){
	case MOENCH:
		return moench_read_frame();
	default:
		return gotthard_read_frame();
	}
}



int	slsReceiverFuncs::moench_read_frame(){
	ret=OK;
	char fName[MAX_STR_LENGTH]="";
	int arg = -1,i;


	int bufferSize = MOENCH_BUFFER_SIZE;
	int rnel 		= bufferSize/(sizeof(int));
	int* retval 	= new int[rnel];
	int* origVal 	= new int[rnel];
	//all initialized to 0
	for(i=0;i<rnel;i++)	retval[i]=0;
	for(i=0;i<rnel;i++)	origVal[i]=0;

	char* raw 		= new char[bufferSize];

	uint32_t startIndex=0;
	int index = 0,bindex = 0, offset=0;

	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST

	/**send garbage with -1 index to try again*/
	if(!slsReceiverList->getFramesCaught()){
		arg = -1;
		cout<<"haven't caught any frame yet"<<endl;
	}

	else{
		ret = OK;
		startIndex=slsReceiverList->getStartFrameIndex();
		slsReceiverList->readFrame(fName,&raw);

		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			arg = -1;
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
					index = startIndex - 1;
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

			arg = index - startIndex;
		}

	}

#ifdef VERBOSE
	cout << "\nstartIndex:" << startIndex << endl;
	cout << "fName:" << fName << endl;
	cout << "index:" << arg << endl;
	if(retval==NULL)cout<<"retval is null"<<endl;
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
		socket->SendDataOnly(&arg,sizeof(arg));
		socket->SendDataOnly(retval,MOENCH_DATA_BYTES);
	}
	//return ok/fail


	delete [] retval;
	delete [] origVal;
	delete [] raw;

	return ret;

}




int	slsReceiverFuncs::gotthard_read_frame(){
	ret=OK;
	char fName[MAX_STR_LENGTH]="";
	int arg = -1,i;


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
	uint32_t startIndex=0;

	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST



	/**send garbage with -1 index to try again*/
	if(!slsReceiverList->getFramesCaught()){
		arg=-1;
		cout<<"haven't caught any frame yet"<<endl;
	}else{
		ret = OK;
		startIndex=slsReceiverList->getStartFrameIndex();
		slsReceiverList->readFrame(fName,&raw);

		/**send garbage with -1 index to try again*/
		if (raw == NULL){
			arg = -1;
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
					index = startIndex - 1;
					cout << "Missing Packet,Not sending to gui" << endl;
				}
			}
			//all adc
			else{
				//ignore if half frame is missing
				if ((bindex != 0xFFFFFFFF) && (bindex2 != 0xFFFFFFFF)){

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
				}
				else{
					index = startIndex - 1;
					cout << "Missing Packet,Not sending to gui" << endl;
				}
			}

			arg = (index - startIndex);
		}
	}

#ifdef VERBOSE
	if(arg!=-1){
		cout << "fName:" << fName << endl;
		cout << "findex:" << arg << endl;
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
		socket->SendDataOnly(&arg,sizeof(arg));
		socket->SendDataOnly(retval,GOTTHARD_DATA_BYTES);
	}

	delete [] retval;
	delete [] origVal;
	delete [] raw;

	return ret;
}




int slsReceiverFuncs::set_read_frequency(){
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}/*
		else if((slsReceiverList->getStatus()==RUNNING) && (index >= 0)){
			ret = FAIL;
			strcpy(mess,"cannot set up receiver mode when receiver is running\n");
		}*/
		else
			retval=slsReceiverList->setNFrameToGui(index);
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






int slsReceiverFuncs::enable_file_write(){
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else{
			retval=slsReceiverList->setEnableFileWrite(enable);
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



int slsReceiverFuncs::get_version(){
	ret=OK;
	int64_t retval=-1;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	retval= SVNREV;
	retval= (retval <<32) | SVNDATE;
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




int	slsReceiverFuncs::start_readout(){
	ret=OK;
	enum runStatus retval;

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	slsReceiverList->startReadout();
	retval = slsReceiverList->getStatus();
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




int slsReceiverFuncs::set_timer() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else{
			if(index[0] == slsDetectorDefs::FRAME_PERIOD)
				retval=slsReceiverList->setAcquisitionPeriod(index[1]);
			else
				retval=slsReceiverList->setNumberOfFrames(index[1]);
		}
	}
#ifdef VERBOSE
	if(ret!=FAIL){
		if(index[0] == slsDetectorDefs::FRAME_PERIOD)
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






int slsReceiverFuncs::enable_compression() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if(enable >= 0){
			if (lockStatus==1 && socket->differentClients==1){
				sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
				ret=FAIL;
			}
			else if(slsReceiverList->getStatus()==RUNNING){
				strcpy(mess,"Cannot enable/disable compression while status is running\n");
				ret=FAIL;
			}
			else
				ret = slsReceiverList->enableDataCompression(enable);
		}

		retval=slsReceiverList->getDataCompression();
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




int slsReceiverFuncs::set_detector_hostname() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {

		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
 	 	 else
			strcpy(retval,slsReceiverList->setDetectorHostname(hostname));
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







int slsReceiverFuncs::set_dynamic_range() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else
			retval=slsReceiverList->setDynamicRange(dr);
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







int slsReceiverFuncs::enable_overwrite() {
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
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else
			retval=slsReceiverList->enableOverwrite(index);
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









































int slsReceiverFuncs::lock_receiver() {
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







int slsReceiverFuncs::set_port() {
	ret=OK;
	MySocketTCP* mySocket=NULL;
	int sd=-1;
	enum portType p_type; /** data? control? stop? Unused! */
	int p_number; /** new port number */

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
			file_des=socket->getFileDes();
		}
	}

	//return ok/fail
	return ret;
}






int slsReceiverFuncs::get_last_client_ip() {
	ret=OK;

	if (socket->differentClients )
		ret=FORCE_UPDATE;

	socket->SendDataOnly(&ret,sizeof(ret));
	socket->SendDataOnly(socket->lastClientIP,sizeof(socket->lastClientIP));

	return ret;
}







int slsReceiverFuncs::send_update() {
	ret=OK;
	int ind;
	char path[MAX_STR_LENGTH];

	socket->SendDataOnly(socket->lastClientIP,sizeof(socket->lastClientIP));

	//index
#ifdef SLS_RECEIVER_FUNCTION_LIST

	/*if(myDetectorType != EIGER)*/
		ind=slsReceiverList->getFileIndex();

	socket->SendDataOnly(&ind,sizeof(ind));
#endif

	//filepath
#ifdef SLS_RECEIVER_FUNCTION_LIST
	strcpy(path,slsReceiverList->getFilePath());
#endif
	socket->SendDataOnly(path,MAX_STR_LENGTH);


	//filename
#ifdef SLS_RECEIVER_FUNCTION_LIST
	strcpy(path,slsReceiverList->getFileName());
#endif
	socket->SendDataOnly(path,MAX_STR_LENGTH);


	if (lockStatus==0) {
		strcpy(socket->lastClientIP,socket->thisClientIP);
	}

	return ret;


}






int slsReceiverFuncs::update_client() {
	ret=OK;
	socket->SendDataOnly(&ret,sizeof(ret));

	return send_update();
}







int slsReceiverFuncs::exit_server() {
	ret=GOODBYE;
	socket->SendDataOnly(&ret,sizeof(ret));
	strcpy(mess,"closing server");
	socket->SendDataOnly(mess,sizeof(mess));
	cout << mess << endl;
	return ret;
}





int slsReceiverFuncs::exec_command() {
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





