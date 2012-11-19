/********************************************//**
 * @file slsReceiver_funcs.h
 * @short interface between receiver and client
 ***********************************************/

#include "slsReceiver_funcs.h"
#include "slsReceiverFunctionList.h"

#include <iostream>
using namespace std;


slsReceiverFuncs::slsReceiverFuncs(MySocketTCP *socket):
		socket(socket),
		ret(OK),
		lockStatus(0){
	//initialize variables
	strcpy(mess,"dummy message");
	strcpy(socket->lastClientIP,"none");
	strcpy(socket->thisClientIP,"none1");

	function_table();
	slsReceiverList =  new slsReceiverFunctionList();
}


int slsReceiverFuncs::function_table(){

	for (int i=0;i<numberOfFunctions;i++)
		flist[i]=&slsReceiverFuncs::M_nofunc;

	flist[F_SET_FILE_NAME]		=	&slsReceiverFuncs::set_file_name;
	flist[F_SET_FILE_PATH]		=	&slsReceiverFuncs::set_file_dir;
	flist[F_SET_FILE_INDEX]		=	&slsReceiverFuncs::set_file_index;
	flist[F_START_RECEIVER]		=	&slsReceiverFuncs::start_receiver;
	flist[F_STOP_RECEIVER]		=	&slsReceiverFuncs::stop_receiver;
	flist[F_GET_RECEIVER_STATUS]=	&slsReceiverFuncs::get_status;
	flist[F_GET_FRAMES_CAUGHT]	=	&slsReceiverFuncs::get_frames_caught;
	flist[F_GET_FRAME_INDEX]	=	&slsReceiverFuncs::get_frame_index;
	flist[F_RESET_FRAMES_CAUGHT]=	&slsReceiverFuncs::reset_frames_caught;
	flist[F_READ_FRAME]			=	&slsReceiverFuncs::read_frame;

	//General Functions
	flist[F_LOCK_SERVER]		=	&slsReceiverFuncs::lock_receiver;
	flist[F_SET_PORT]			=	&slsReceiverFuncs::set_port;
	flist[F_GET_LAST_CLIENT_IP]	=	&slsReceiverFuncs::get_last_client_ip;
	flist[F_UPDATE_CLIENT]		=	&slsReceiverFuncs::update_client;
	flist[F_EXIT_SERVER]		=	&slsReceiverFuncs::exit_server;		//not implemented in client
	flist[F_EXEC_COMMAND]		=	&slsReceiverFuncs::exec_command;	//not implemented in client


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
	cout <<  "calling function fnum = "<< fnum << hex << flist[fnum] << endl;
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
		}
		else if((strlen(fPath))&&(slsReceiverList->getStatus()==RUNNING)){
			strcpy(mess,"Can not set file path while receiver running\n");
			ret = FAIL;
		}
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
		if (lockStatus==1 && socket->differentClients==1){//necessary???
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





int slsReceiverFuncs::start_receiver(){
	ret=OK;

	strcpy(mess,"Could not start receiver\n");

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (lockStatus==1 && socket->differentClients==1){
		sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
		ret=FAIL;
	}
	else if(!strlen(slsReceiverList->setFilePath(""))){
		strcpy(mess,"receiver not set up. set receiver ip again.\n");
		ret = FAIL;
	}
	else if(slsReceiverList->getStatus()!=RUNNING)
		ret=slsReceiverList->startReceiver();
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

	if(ret==OK && socket->differentClients){
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

	if(ret==OK && socket->differentClients){
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

	if(ret==OK && socket->differentClients){
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
	int retval=-1;
	int index=-1;

	strcpy(mess,"Could not reset frames caught\n");


	// receive arguments
	if(socket->ReceiveDataOnly(&index,sizeof(index)) < 0) {;
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (ret==OK) {
		if (lockStatus==1 && socket->differentClients==1){
			sprintf(mess,"Receiver locked by %s\n", socket->lastClientIP);
			ret=FAIL;
		}
		else
			retval=slsReceiverList->resetTotalFramesCaught(index);
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
	ret=OK;
	int i,startIndex=-1;

	char* retval=NULL;
	char buffer[1286*2];
	char fName[MAX_STR_LENGTH];
	int arg[2];
	arg[1]=1;//do not flip
	int index=-1,index2=-1,fIndex=-1;;


	strcpy(mess,"Could not read frame\n");


	// execute action if the arguments correctly arrived
#ifdef SLS_RECEIVER_FUNCTION_LIST
	//wait till you get first frame 1. to get index(from startindex 2. filename corresponds to buffer value
	if(startIndex==-1){
		ret=FAIL;
		strcpy(mess,"did not start index\n");
		for(i=0;i<10;i++){
			startIndex=slsReceiverList->getStartFrameIndex();
			if(startIndex==-1)
				usleep(1000000);
			else {
				ret=OK;
				break;
			}
		}
	}
	//got atleast first frame, read buffer
	if(ret==OK){
		int count=0;
		do{
			if(count>0){
				cout << endl << "unmatching: index:" << index <<" index2:" << index2 << endl;
				if(count>10){
					strcpy(mess,"unmatching index. could not read frame.\n");
					ret=FAIL;
					break;
				}
			}
			retval=slsReceiverList->readFrame(fName);
			index=(int)(*((int*)retval));
			char* retval2= retval+1286;
			index2= (int)(*((int*)retval2));
			count++;

		}while((index%2)==(index2%2));

		fIndex=((int)(*((int*)retval)) - startIndex)/2;
		arg[0]=fIndex-1;
		arg[1]=(index%2);

#ifdef VERBOSE
		cout << "\nstartIndex:" << startIndex << endl;
		cout << "fName:" << fName << endl;
		if((index%2)==0)
			cout << "\nEven Index, must flip:" << index << endl;
#endif
	}


#endif

	if(ret==OK && socket->differentClients){
		cout << "Force update" << endl;
		ret=FORCE_UPDATE;
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if(ret==FAIL){
		cout<<"mess:"<<mess<<endl;
		socket->SendDataOnly(mess,sizeof(mess));
	}
	else{
		socket->SendDataOnly(fName,MAX_STR_LENGTH);
		socket->SendDataOnly(arg,sizeof(arg));
		socket->SendDataOnly(retval,sizeof(buffer));
	}
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
	int sd=-1;
	enum portType p_type; /** data? control? stop? Unused! */
	int p_number; /** new port number */

	// receive arguments
	if(socket->ReceiveDataOnly(&p_type,sizeof(p_type)) < 0 ){
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (ptype)\n");
		ret=FAIL;
	}

	if(socket->ReceiveDataOnly(&p_number,sizeof(p_number)) < 0 ){
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (pnum)\n");
		ret=FAIL;
	}

	// execute action if the arguments correctly arrived
	if (ret==OK) {
		if (socket->differentClients==1 && lockStatus==1 ) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",socket->lastClientIP);
		}  else {
			if (p_number<1024) {
				sprintf(mess,"Too low port number %d\n", p_number);
				cout << endl;
				ret=FAIL;
			}
			cout << "set port " << p_type << " to " << p_number <<endl;
			socket = new MySocketTCP(p_number);
		}
		if (!socket->getErrorStatus()){
			ret=OK;
			if (socket->differentClients)
				ret=FORCE_UPDATE;
		} else {
			ret=FAIL;
			sprintf(mess,"Could not bind port %d\n", p_number);
			cout << "Could not bind port " << p_number << endl;
			if (socket->getErrorStatus()==-10) {
				sprintf(mess,"Port %d already set\n", p_number);
				cout << "Port " << p_number << " already set" << endl;
			}
		}
	}

	// send answer
	socket->SendDataOnly(&ret,sizeof(ret));
	if (ret==FAIL) {
		socket->SendDataOnly(mess,sizeof(mess));
	} else {
		socket->SendDataOnly(&p_number,sizeof(p_number));
		socket->Disconnect();
		delete socket;
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
	ind=slsReceiverList->getFileIndex();
#endif
	socket->SendDataOnly(&ind,sizeof(ind));


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
	ret=FAIL;
	socket->SendDataOnly(&ret,sizeof(ret));
	strcpy(mess,"closing server");
	cout << mess << endl;
	socket->SendDataOnly(mess,sizeof(mess));
	return GOODBYE;
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





