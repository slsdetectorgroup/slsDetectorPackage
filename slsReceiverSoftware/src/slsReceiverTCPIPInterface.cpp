/********************************************//**
 * @file slsReceiverTCPIPInterface.cpp
 * @short interface between receiver and client
 ***********************************************/

#include "slsReceiverTCPIPInterface.h"
#include "slsReceiverImplementation.h"
#include "MySocketTCP.h"
#include "ClientInterface.h"
#include "gitInfoReceiver.h"
#include "slsReceiverUsers.h"
#include "slsReceiver.h"
#include "versionAPI.h"

#include  <stdlib.h>	//EXIT
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <syscall.h>
#include <vector>



slsReceiverTCPIPInterface::~slsReceiverTCPIPInterface() {
	stop();
	if(mySock) {
		delete mySock;
		mySock=NULL;
	}
	if (clientInterface)
		delete clientInterface;
	if(receiverBase)
		delete receiverBase;
}

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int pn):
				myDetectorType(GOTTHARD),
				receiverBase(0),
				ret(OK),
				fnum(-1),
				lockStatus(0),
				killTCPServerThread(0),
				tcpThreadCreated(false),
				portNumber(DEFAULT_PORTNO+2),
				mySock(0),
				clientInterface(0)
{
	//***callback parameters***
	startAcquisitionCallBack = NULL;
	pStartAcquisition = NULL;
	acquisitionFinishedCallBack = NULL;
	pAcquisitionFinished = NULL;
	rawDataReadyCallBack = NULL;
	rawDataModifyReadyCallBack = NULL;
	pRawDataReady = NULL;

	// create socket
	portNumber = (pn > 0 ? pn : DEFAULT_PORTNO + 2);
	MySocketTCP* m = new MySocketTCP(portNumber);
	mySock = m;
	clientInterface = new ClientInterface(mySock);

	//initialize variables
	strcpy(mySock->lastClientIP,"none");
	strcpy(mySock->thisClientIP,"none1");
	memset(mess,0,sizeof(mess));
	strcpy(mess,"dummy message");

	function_table();
}


int slsReceiverTCPIPInterface::start(){
	FILE_LOG(logDEBUG) << "Creating TCP Server Thread";
	killTCPServerThread = 0;
	if(pthread_create(&TCPServer_thread, NULL,startTCPServerThread, (void*) this)){
		FILE_LOG(logERROR) << "Could not create TCP Server thread";
		return FAIL;
	}
	tcpThreadCreated = true;
	//#ifdef VERYVERBOSE
	FILE_LOG(logDEBUG) << "TCP Server thread created successfully.";
	//#endif
	return OK;
}


void slsReceiverTCPIPInterface::stop(){
	if (tcpThreadCreated) {
		FILE_LOG(logINFO) << "Shutting down TCP Socket on port " << portNumber;
		killTCPServerThread = 1;
		if(mySock)	mySock->ShutDownSocket();
		FILE_LOG(logDEBUG) << "TCP Socket closed on port " << portNumber;
		pthread_join(TCPServer_thread, NULL);
		tcpThreadCreated = false;
		killTCPServerThread = 0;
		FILE_LOG(logDEBUG) << "Exiting TCP Server Thread on port " << portNumber;
	}
}



int64_t slsReceiverTCPIPInterface::getReceiverVersion(){
	int64_t retval = GITDATE & 0xFFFFFF;
	return retval;
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

void slsReceiverTCPIPInterface::registerCallBackRawDataReady(void (*func)(char* ,
		char*, uint32_t, void*),void *arg){
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}

void slsReceiverTCPIPInterface::registerCallBackRawDataModifyReady(void (*func)(char* ,
        char*, uint32_t &,void*),void *arg){
    rawDataModifyReadyCallBack=func;
    pRawDataReady=arg;
}



void* slsReceiverTCPIPInterface::startTCPServerThread(void *this_pointer){
	((slsReceiverTCPIPInterface*)this_pointer)->startTCPServer();
	return this_pointer;
}


void slsReceiverTCPIPInterface::startTCPServer(){
	cprintf(BLUE,"Created [ TCP server Tid: %ld ]\n", (long)syscall(SYS_gettid));
	FILE_LOG(logINFO) << "SLS Receiver starting TCP Server on port " << portNumber << std::endl;
	int ret = OK;

	while(true) {
		if(mySock->Connect() >= 0){
			ret = decode_function();
			mySock->Disconnect();
		}

		//if tcp command was to exit server
		if(ret == GOODBYE){
			FILE_LOG(logINFO) << "Shutting down UDP Socket";
			if(receiverBase){
				receiverBase->shutDownUDPSockets();
			}

			mySock->exitServer();
			cprintf(BLUE,"Exiting [ TCP server Tid: %ld ]\n", (long)syscall(SYS_gettid));
			pthread_exit(NULL);
		}

		//if user entered exit
		if(killTCPServerThread) {
			if (ret != GOODBYE) {
				if(receiverBase){
					receiverBase->shutDownUDPSockets();
				}
			}
			cprintf(BLUE,"Exiting [ TCP server Tid: %ld ]\n", (long)syscall(SYS_gettid));
			pthread_exit(NULL);
		}
	}
}


const char* slsReceiverTCPIPInterface::getFunctionName(enum detFuncs func) {
	switch (func) {
	case F_EXEC_RECEIVER_COMMAND:		return "F_EXEC_RECEIVER_COMMAND";
	case F_EXIT_RECEIVER: 				return "F_EXIT_RECEIVER";
	case F_LOCK_RECEIVER: 				return "F_LOCK_RECEIVER";
	case F_GET_LAST_RECEIVER_CLIENT_IP: return "F_GET_LAST_RECEIVER_CLIENT_IP";
	case F_SET_RECEIVER_PORT: 			return "F_SET_RECEIVER_PORT";
	case F_UPDATE_RECEIVER_CLIENT: 		return "F_UPDATE_RECEIVER_CLIENT";
	case F_GET_RECEIVER_ID: 			return "F_GET_RECEIVER_ID";
	case F_GET_RECEIVER_TYPE: 			return "F_GET_RECEIVER_TYPE";
	case F_SEND_RECEIVER_DETHOSTNAME:	return "F_SEND_RECEIVER_DETHOSTNAME";
	case F_RECEIVER_SET_ROI: 			return "F_RECEIVER_SET_ROI";
	case F_SETUP_RECEIVER_UDP:			return "F_SETUP_RECEIVER_UDP";
	case F_SET_RECEIVER_TIMER:  		return "F_SET_RECEIVER_TIMER";
	case F_SET_RECEIVER_DYNAMIC_RANGE:  return "F_SET_RECEIVER_DYNAMIC_RANGE";
	case F_RECEIVER_STREAMING_FREQUENCY:return "F_RECEIVER_STREAMING_FREQUENCY";
	case F_GET_RECEIVER_STATUS:			return "F_GET_RECEIVER_STATUS";
	case F_START_RECEIVER:				return "F_START_RECEIVER";
	case F_STOP_RECEIVER:				return "F_STOP_RECEIVER";
	case F_SET_RECEIVER_FILE_PATH: 		return "F_SET_RECEIVER_FILE_PATH";
	case F_SET_RECEIVER_FILE_NAME: 		return "F_SET_RECEIVER_FILE_NAME";
	case F_SET_RECEIVER_FILE_INDEX: 	return "F_SET_RECEIVER_FILE_INDEX";
	case F_GET_RECEIVER_FRAME_INDEX:	return "F_GET_RECEIVER_FRAME_INDEX";
	case F_GET_RECEIVER_FRAMES_CAUGHT:	return "F_GET_RECEIVER_FRAMES_CAUGHT";
	case F_RESET_RECEIVER_FRAMES_CAUGHT:return "F_RESET_RECEIVER_FRAMES_CAUGHT";
	case F_ENABLE_RECEIVER_FILE_WRITE:	return "F_ENABLE_RECEIVER_FILE_WRITE";
	case F_ENABLE_RECEIVER_OVERWRITE:	return "F_ENABLE_RECEIVER_OVERWRITE";
	case F_ENABLE_RECEIVER_TEN_GIGA:	return "F_ENABLE_RECEIVER_TEN_GIGA";
	case F_SET_RECEIVER_FIFO_DEPTH:		return "F_SET_RECEIVER_FIFO_DEPTH";
	case F_RECEIVER_ACTIVATE:			return "F_RECEIVER_ACTIVATE";
	case F_STREAM_DATA_FROM_RECEIVER:	return "F_STREAM_DATA_FROM_RECEIVER";
	case F_RECEIVER_STREAMING_TIMER:	return "F_RECEIVER_STREAMING_TIMER";
	case F_SET_FLIPPED_DATA_RECEIVER:	return "F_SET_FLIPPED_DATA_RECEIVER";
	case F_SET_RECEIVER_FILE_FORMAT:	return "F_SET_RECEIVER_FILE_FORMAT";
	case F_SEND_RECEIVER_DETPOSID:		return "F_SEND_RECEIVER_DETPOSID";
	case F_SEND_RECEIVER_MULTIDETSIZE:  return "F_SEND_RECEIVER_MULTIDETSIZE";
	case F_SET_RECEIVER_STREAMING_PORT: return "F_SET_RECEIVER_STREAMING_PORT";
	case F_RECEIVER_STREAMING_SRC_IP: 	return "F_RECEIVER_STREAMING_SRC_IP";
	case F_SET_RECEIVER_SILENT_MODE:	return "F_SET_RECEIVER_SILENT_MODE";
	case F_ENABLE_GAPPIXELS_IN_RECEIVER:return "F_ENABLE_GAPPIXELS_IN_RECEIVER";
	case F_RESTREAM_STOP_FROM_RECEIVER:	return "F_RESTREAM_STOP_FROM_RECEIVER";
    case F_ADDITIONAL_JSON_HEADER:      return "F_ADDITIONAL_JSON_HEADER";
    case F_RECEIVER_UDP_SOCK_BUF_SIZE:  return "F_RECEIVER_UDP_SOCK_BUF_SIZE";
    case F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE:  return "F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE";
    case F_SET_RECEIVER_FRAMES_PER_FILE:return "F_SET_RECEIVER_FRAMES_PER_FILE";
    case F_RECEIVER_CHECK_VERSION:		return "F_RECEIVER_CHECK_VERSION";
    case F_RECEIVER_DISCARD_POLICY:		return "F_RECEIVER_DISCARD_POLICY";
    case F_RECEIVER_PADDING_ENABLE:		return "F_RECEIVER_PADDING_ENABLE";
    case F_RECEIVER_DEACTIVATED_PADDING_ENABLE: return "F_RECEIVER_DEACTIVATED_PADDING_ENABLE";

	default:							return "Unknown Function";
	}
}



int slsReceiverTCPIPInterface::function_table(){
	flist[F_EXEC_RECEIVER_COMMAND]			=	&slsReceiverTCPIPInterface::exec_command;
	flist[F_EXIT_RECEIVER]					=	&slsReceiverTCPIPInterface::exit_server;
	flist[F_LOCK_RECEIVER]					=	&slsReceiverTCPIPInterface::lock_receiver;
	flist[F_GET_LAST_RECEIVER_CLIENT_IP]	=	&slsReceiverTCPIPInterface::get_last_client_ip;
	flist[F_SET_RECEIVER_PORT]				=	&slsReceiverTCPIPInterface::set_port;
	flist[F_UPDATE_RECEIVER_CLIENT]			=	&slsReceiverTCPIPInterface::update_client;
	flist[F_GET_RECEIVER_ID]				=	&slsReceiverTCPIPInterface::get_id;
	flist[F_GET_RECEIVER_TYPE]				=	&slsReceiverTCPIPInterface::set_detector_type;
	flist[F_SEND_RECEIVER_DETHOSTNAME]		= 	&slsReceiverTCPIPInterface::set_detector_hostname;
	flist[F_RECEIVER_SET_ROI]				=	&slsReceiverTCPIPInterface::set_roi;
	flist[F_SETUP_RECEIVER_UDP]				=	&slsReceiverTCPIPInterface::setup_udp;
	flist[F_SET_RECEIVER_TIMER]				= 	&slsReceiverTCPIPInterface::set_timer;
	flist[F_SET_RECEIVER_DYNAMIC_RANGE]		= 	&slsReceiverTCPIPInterface::set_dynamic_range;
	flist[F_RECEIVER_STREAMING_FREQUENCY]	= 	&slsReceiverTCPIPInterface::set_streaming_frequency;
	flist[F_GET_RECEIVER_STATUS]			=	&slsReceiverTCPIPInterface::get_status;
	flist[F_START_RECEIVER]					=	&slsReceiverTCPIPInterface::start_receiver;
	flist[F_STOP_RECEIVER]					=	&slsReceiverTCPIPInterface::stop_receiver;
	flist[F_SET_RECEIVER_FILE_PATH]			=	&slsReceiverTCPIPInterface::set_file_dir;
	flist[F_SET_RECEIVER_FILE_NAME]			=	&slsReceiverTCPIPInterface::set_file_name;
	flist[F_SET_RECEIVER_FILE_INDEX]		=	&slsReceiverTCPIPInterface::set_file_index;
	flist[F_GET_RECEIVER_FRAME_INDEX]		=	&slsReceiverTCPIPInterface::get_frame_index;
	flist[F_GET_RECEIVER_FRAMES_CAUGHT]		=	&slsReceiverTCPIPInterface::get_frames_caught;
	flist[F_RESET_RECEIVER_FRAMES_CAUGHT]	=	&slsReceiverTCPIPInterface::reset_frames_caught;
	flist[F_ENABLE_RECEIVER_FILE_WRITE]		=	&slsReceiverTCPIPInterface::enable_file_write;
	flist[F_ENABLE_RECEIVER_OVERWRITE]		= 	&slsReceiverTCPIPInterface::enable_overwrite;
	flist[F_ENABLE_RECEIVER_TEN_GIGA]		= 	&slsReceiverTCPIPInterface::enable_tengiga;
	flist[F_SET_RECEIVER_FIFO_DEPTH]		= 	&slsReceiverTCPIPInterface::set_fifo_depth;
	flist[F_RECEIVER_ACTIVATE]				= 	&slsReceiverTCPIPInterface::set_activate;
	flist[F_STREAM_DATA_FROM_RECEIVER]		= 	&slsReceiverTCPIPInterface::set_data_stream_enable;
	flist[F_RECEIVER_STREAMING_TIMER]		= 	&slsReceiverTCPIPInterface::set_streaming_timer;
	flist[F_SET_FLIPPED_DATA_RECEIVER]		= 	&slsReceiverTCPIPInterface::set_flipped_data;
	flist[F_SET_RECEIVER_FILE_FORMAT]		= 	&slsReceiverTCPIPInterface::set_file_format;
	flist[F_SEND_RECEIVER_DETPOSID]			= 	&slsReceiverTCPIPInterface::set_detector_posid;
	flist[F_SEND_RECEIVER_MULTIDETSIZE]		= 	&slsReceiverTCPIPInterface::set_multi_detector_size;
	flist[F_SET_RECEIVER_STREAMING_PORT]	= 	&slsReceiverTCPIPInterface::set_streaming_port;
	flist[F_RECEIVER_STREAMING_SRC_IP]		= 	&slsReceiverTCPIPInterface::set_streaming_source_ip;
	flist[F_SET_RECEIVER_SILENT_MODE]		= 	&slsReceiverTCPIPInterface::set_silent_mode;
	flist[F_ENABLE_GAPPIXELS_IN_RECEIVER]	=	&slsReceiverTCPIPInterface::enable_gap_pixels;
	flist[F_RESTREAM_STOP_FROM_RECEIVER]	= 	&slsReceiverTCPIPInterface::restream_stop;
	flist[F_ADDITIONAL_JSON_HEADER]         =   &slsReceiverTCPIPInterface::set_additional_json_header;
    flist[F_RECEIVER_UDP_SOCK_BUF_SIZE]     =   &slsReceiverTCPIPInterface::set_udp_socket_buffer_size;
    flist[F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE]=   &slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size;
    flist[F_SET_RECEIVER_FRAMES_PER_FILE]	=   &slsReceiverTCPIPInterface::set_frames_per_file;
    flist[F_RECEIVER_CHECK_VERSION]			=   &slsReceiverTCPIPInterface::check_version_compatibility;
    flist[F_RECEIVER_DISCARD_POLICY]		=   &slsReceiverTCPIPInterface::set_discard_policy;
	flist[F_RECEIVER_PADDING_ENABLE]		=   &slsReceiverTCPIPInterface::set_padding_enable;
	flist[F_RECEIVER_DEACTIVATED_PADDING_ENABLE] = &slsReceiverTCPIPInterface::set_deactivated_receiver_padding_enable;

	for (int i = NUM_DET_FUNCTIONS + 1; i < NUM_REC_FUNCTIONS ; i++) {
		FILE_LOG(logDEBUG1) << "function fnum: " << i << " (" <<
				getFunctionName((enum detFuncs)i) << ") located at " << flist[i];
	}

	return OK;
}




int slsReceiverTCPIPInterface::decode_function(){
	ret = FAIL;

	FILE_LOG(logDEBUG1) <<  "waiting to receive data";
	int n = mySock->ReceiveDataOnly(&fnum,sizeof(fnum));
	if (n <= 0) {
		FILE_LOG(logDEBUG1) << "ERROR reading from socket. "
				"Received " << n << " bytes," <<
				"fnum:" << fnum << " "
				"(" << getFunctionName((enum detFuncs)fnum) << ")";
		return FAIL;
	}
	else
		FILE_LOG(logDEBUG1) << "Received " << n << " bytes";


	if (fnum <= NUM_DET_FUNCTIONS || fnum >= NUM_REC_FUNCTIONS) {
		FILE_LOG(logERROR) << "Unknown function enum " << fnum;
		ret = (this->M_nofunc)();
	} else{
		FILE_LOG(logDEBUG1) <<  "calling function fnum: "<< fnum << " "
				"(" << getFunctionName((enum detFuncs)fnum) << ") "
				"located at " << flist[fnum];
		ret = (this->*flist[fnum])();

		if (ret == FAIL) {
			FILE_LOG(logERROR) << "Failed to execute function = " << fnum << " ("
					<< getFunctionName((enum detFuncs)fnum) << ")";
		}
	}
	return ret;
}



int slsReceiverTCPIPInterface::printSocketReadError() {
	FILE_LOG(logERROR) << "Reading from socket failed. Possible socket crash";
	return FAIL;
}


void slsReceiverTCPIPInterface::invalidReceiverObject() {
	ret=FAIL;
	strcpy(mess,"Receiver not set up. Please use rx_hostname first.\n");
	FILE_LOG(logERROR) << mess;
}


void slsReceiverTCPIPInterface::receiverlocked() {
	ret = FAIL;
	sprintf(mess,"Receiver locked by %s\n",mySock->lastClientIP);
	FILE_LOG(logERROR) << mess;
}


void slsReceiverTCPIPInterface::receiverNotIdle() {
	ret = FAIL;
	sprintf(mess,"Can not execute %s when receiver is not idle\n",
			getFunctionName((enum detFuncs)fnum));
	FILE_LOG(logERROR) << mess;
}

void slsReceiverTCPIPInterface::functionNotImplemented() {
	ret = FAIL;
	sprintf(mess, "Function (%s) is not implemented for this detector\n",
			getFunctionName((enum detFuncs)fnum));
	FILE_LOG(logERROR) << mess;
}


int slsReceiverTCPIPInterface::M_nofunc(){printf("111 \n");
	ret = FAIL;
	memset(mess, 0, sizeof(mess));
	int n = 0;

	// to receive any arguments
	while (n > 0)
		n = mySock->ReceiveDataOnly(mess,MAX_STR_LENGTH);

	strcpy(mess,"Unrecognized Function. Please do not proceed.\n");
	FILE_LOG(logERROR) << mess;

	clientInterface->Server_SendResult(false, ret, NULL, 0, mess);

	return ret;
}




int slsReceiverTCPIPInterface::exec_command() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char cmd[MAX_STR_LENGTH];
	memset(cmd,0,sizeof(cmd));
	int sysret = 0;

	// receive arguments
	if (mySock->ReceiveDataOnly(cmd,MAX_STR_LENGTH) < 0)
		return printSocketReadError();

	if (mySock->differentClients && lockStatus)
		receiverlocked();
	else {
		sysret=system(cmd);
		//should be replaced by popen
		if (sysret == 0) {
			ret = OK;
		} else {
			ret = FAIL;
			sprintf(mess,"Executing Command failed\n");
			FILE_LOG(logERROR) << mess;
		}
	}

	clientInterface->Server_SendResult(false, ret, NULL, 0, mess);

	return ret;
}



int slsReceiverTCPIPInterface::exit_server() {
	cprintf(RED,"Closing receiver server\n");

	ret = OK;
	clientInterface->Server_SendResult(false, ret, NULL, 0);

	ret = GOODBYE;
	return ret;
}



int slsReceiverTCPIPInterface::lock_receiver() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int lock = 0;

	// receive arguments
	if (mySock->ReceiveDataOnly(&lock,sizeof(lock)) < 0 )
		return printSocketReadError();

	// execute action
	if (lock >= 0) {
		if (!lockStatus || // if it was unlocked, anyone can lock
				(!strcmp(mySock->lastClientIP,mySock->thisClientIP)) || // if it was locked, need same ip
				(!strcmp(mySock->lastClientIP,"none"))) //if it was locked, must be by "none"
			{
			lockStatus = lock;
			strcpy(mySock->lastClientIP,mySock->thisClientIP);
		}   else
			receiverlocked();
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &lockStatus,sizeof(lockStatus), mess);

	return ret;
}



int slsReceiverTCPIPInterface::get_last_client_ip() {
	ret = OK;
	clientInterface->Server_SendResult(mySock->differentClients,
			ret,mySock->lastClientIP, sizeof(mySock->lastClientIP));

	return ret;
}



int slsReceiverTCPIPInterface::set_port() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int p_type = 0;
	int p_number = -1;
	MySocketTCP* mySocket = 0;
	char oldLastClientIP[INET_ADDRSTRLEN];
	memset(oldLastClientIP, 0, sizeof(oldLastClientIP));

	// receive arguments
	if (mySock->ReceiveDataOnly(&p_type,sizeof(p_type)) < 0 )
		return printSocketReadError();
	if (mySock->ReceiveDataOnly(&p_number,sizeof(p_number)) < 0 )
		return printSocketReadError();

	// execute action
	if (mySock->differentClients && lockStatus) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",mySock->lastClientIP);
		FILE_LOG(logERROR) << mess;
	}
	else {
		if (p_number < 1024) {
			ret = FAIL;
			sprintf(mess,"Port Number (%d) too low\n", p_number);
			FILE_LOG(logERROR) << mess;
		} else {
			FILE_LOG(logINFO) << "set port to " << p_number <<std::endl;
			strcpy(oldLastClientIP, mySock->lastClientIP);

			try {
				mySocket = new MySocketTCP(p_number);
				strcpy(mySock->lastClientIP,oldLastClientIP);
			} catch(SamePortSocketException e) {
				ret = FAIL;
				sprintf(mess, "Could not bind port %d. It is already set\n", p_number);
				FILE_LOG(logERROR) << mess;
			} catch (...) {
				ret = FAIL;
				sprintf(mess, "Could not bind port %d.\n", p_number);
				FILE_LOG(logERROR) << mess;
			}
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &p_number,sizeof(p_number), mess);

	if(ret != FAIL){
		mySock->Disconnect();
		delete mySock;
		mySock = mySocket;
		clientInterface->SetSocket(mySock);
	}

	return ret;
}



int slsReceiverTCPIPInterface::update_client() {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	if (receiverBase == NULL)
		invalidReceiverObject();

	clientInterface->Server_SendResult(false, ret, NULL, 0, mess);

	if (ret == FAIL)
		return ret;

	// update
	return send_update();
}



int slsReceiverTCPIPInterface::send_update() {
	int ind = -1;
	char defaultVal[MAX_STR_LENGTH];
	memset(defaultVal, 0, sizeof(defaultVal));
	char* path = NULL;
	int n = 0;

	n += mySock->SendDataOnly(mySock->lastClientIP,sizeof(mySock->lastClientIP));

	// filepath
	path = receiverBase->getFilePath();
	if (path == NULL)
	    n += mySock->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else {
	    n += mySock->SendDataOnly(path,MAX_STR_LENGTH);
		delete[] path;
	}

	// filename
	path = receiverBase->getFileName();
	if(path == NULL)
	    n += mySock->SendDataOnly(defaultVal,MAX_STR_LENGTH);
	else {
	    n += mySock->SendDataOnly(path,MAX_STR_LENGTH);
		delete[] path;
	}

	// index
	ind=receiverBase->getFileIndex();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	//file format
	ind=(int)receiverBase->getFileFormat();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	//frames per file
	ind=(int)receiverBase->getFramesPerFile();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	//frame discard policy
	ind=(int)receiverBase->getFrameDiscardPolicy();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	//frame padding
	ind=(int)receiverBase->getFramePaddingEnable();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// file write enable
	ind=(int)receiverBase->getFileWriteEnable();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// file overwrite enable
	ind=(int)receiverBase->getOverwriteEnable();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// gap pixels
	ind=(int)receiverBase->getGapPixelsEnable();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// streaming frequency
	ind=(int)receiverBase->getStreamingFrequency();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// streaming port
	ind=(int)receiverBase->getStreamingPort();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// streaming source ip
	path = receiverBase->getStreamingSourceIP();
	mySock->SendDataOnly(path,MAX_STR_LENGTH);
	if (path != NULL)
		delete[] path;

    // additional json header
    path = receiverBase->getAdditionalJsonHeader();
    mySock->SendDataOnly(path,MAX_STR_LENGTH);
    if (path != NULL)
        delete[] path;

	// data streaming enable
	ind=(int)receiverBase->getDataStreamEnable();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// activate
	ind=(int)receiverBase->getActivate();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// deactivated padding enable
	ind=(int)receiverBase->getDeactivatedPadding();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	// silent mode
	ind=(int)receiverBase->getSilentMode();
	n += mySock->SendDataOnly(&ind,sizeof(ind));

	if (!lockStatus)
		strcpy(mySock->lastClientIP,mySock->thisClientIP);

	return OK;
}



int slsReceiverTCPIPInterface::get_id(){
	ret = OK;
	int64_t retval = getReceiverVersion();

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval));

	return ret;
}



int slsReceiverTCPIPInterface::set_detector_type(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	detectorType dr = GENERIC;
	detectorType retval = GENERIC;

	// receive arguments
	if (mySock->ReceiveDataOnly(&dr,sizeof(dr)) < 0 )
		return printSocketReadError();

	// execute action
	if (dr == GET_DETECTOR_TYPE)
		retval = myDetectorType;
	else if (mySock->differentClients && lockStatus)
		receiverlocked();
	else if ((receiverBase) && (receiverBase->getStatus() != IDLE))
		receiverNotIdle();
	else {
		switch(dr) {
		case GOTTHARD:
		case PROPIX:
		case MOENCH:
		case EIGER:
		case JUNGFRAUCTB:
		case JUNGFRAU:
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Unknown detector type: %d\n", dr);
			FILE_LOG(logERROR) << mess;
			break;
		}
		if(ret == OK) {
			if(receiverBase == NULL){
				receiverBase = new slsReceiverImplementation();
				if(startAcquisitionCallBack)
					receiverBase->registerCallBackStartAcquisition(startAcquisitionCallBack,pStartAcquisition);
				if(acquisitionFinishedCallBack)
					receiverBase->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,pAcquisitionFinished);
				if(rawDataReadyCallBack)
					receiverBase->registerCallBackRawDataReady(rawDataReadyCallBack,pRawDataReady);
                if(rawDataModifyReadyCallBack)
                    receiverBase->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,pRawDataReady);
			}
			myDetectorType = dr;
			ret = receiverBase->setDetectorType(myDetectorType);
			retval = myDetectorType;
		}
	}
	// client has started updating receiver, update ip
	if (!lockStatus)
		strcpy(mySock->lastClientIP,mySock->thisClientIP);

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_detector_hostname() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char hostname[MAX_STR_LENGTH];
	memset(hostname, 0, sizeof(hostname));
	char* retval = NULL;

	// receive arguments
	if (mySock->ReceiveDataOnly(hostname,MAX_STR_LENGTH) < 0 )
		return printSocketReadError();

	// execute action
	if (mySock->differentClients && lockStatus)
		receiverlocked();
	else if (receiverBase == NULL)
		invalidReceiverObject();
	else if (receiverBase->getStatus() != IDLE)
		receiverNotIdle();
	else {
		receiverBase->setDetectorHostname(hostname);
		retval = receiverBase->getDetectorHostname();
		if(retval == NULL) {
			ret = FAIL;
			cprintf(RED, "Could not set hostname to %s\n", hostname);
			FILE_LOG(logERROR) << mess;
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, retval, (retval == NULL) ? 0 : MAX_STR_LENGTH, mess);

	if(retval != NULL)
		delete[] retval;

	return ret;
}



int slsReceiverTCPIPInterface::set_roi() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int nroi = 0;

	// receive arguments
	if (mySock->ReceiveDataOnly(&nroi,sizeof(nroi)) < 0 )
		return printSocketReadError();

	std::vector <ROI> roiLimits;
	int iloop = 0;
	for (iloop = 0; iloop < nroi; iloop++) {
		ROI temp;
		if ( mySock->ReceiveDataOnly(&temp,sizeof(ROI)) < 0 )
			return printSocketReadError();
		roiLimits.push_back(temp);
	}

	//does not exist
	if (myDetectorType != GOTTHARD)
		functionNotImplemented();

	else {
		if (mySock->differentClients && lockStatus)
			receiverlocked();
		else if (receiverBase == NULL)
			invalidReceiverObject();
		else if (receiverBase->getStatus() != IDLE)
			receiverNotIdle();
		else {
			ret = receiverBase->setROI(roiLimits);
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, NULL, 0, mess);

	roiLimits.clear();

	return ret;
}



int slsReceiverTCPIPInterface::setup_udp(){
	ret = OK;
	char args[3][MAX_STR_LENGTH];
	memset(args,0,sizeof(args));
	char retval[MAX_STR_LENGTH];
	memset(retval,0,sizeof(retval));


	// receive arguments
	if (mySock->ReceiveDataOnly(args,sizeof(args)) < 0 )
		return printSocketReadError();

	// execute action
	if (mySock->differentClients && lockStatus)
		receiverlocked();
	else if (receiverBase == NULL)
		invalidReceiverObject();
	else if (receiverBase->getStatus() != IDLE)
		receiverNotIdle();
	else {
		//set up udp port
		int udpport=-1,udpport2=-1;
		sscanf(args[1],"%d",&udpport);
		sscanf(args[2],"%d",&udpport2);
		receiverBase->setUDPPortNumber(udpport);
		if (myDetectorType == EIGER)
			receiverBase->setUDPPortNumber2(udpport2);

		//setup udpip
		//get ethernet interface or IP to listen to
		FILE_LOG(logINFO) << "Receiver UDP IP: " << args[0];
		std::string temp = genericSocket::ipToName(args[0]);
		if (temp == "none"){
			ret = FAIL;
			strcpy(mess, "Failed to get ethernet interface or IP\n");
			FILE_LOG(logERROR) << mess;
		}
		else {
			char eth[MAX_STR_LENGTH];
			memset(eth,0,sizeof(eth));
			strcpy(eth,temp.c_str());
			if (strchr(eth,'.') != NULL) {
				strcpy(eth,"");
				ret = FAIL;
				strcpy(mess, "Failed to get ethernet interface\n");
				FILE_LOG(logERROR) << mess;
			}
			receiverBase->setEthernetInterface(eth);

			//get mac address from ethernet interface
			if (ret != FAIL)
				temp = genericSocket::nameToMac(eth);

			if ((temp=="00:00:00:00:00:00") || (ret == FAIL)){
				ret = FAIL;
				strcpy(mess,"failed to get mac adddress to listen to\n");
				FILE_LOG(logERROR) << mess;
			}
			else {
				strcpy(retval,temp.c_str());
				FILE_LOG(logINFO) << "Reciever MAC Address: " << retval;
			}
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_timer() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t index[2] = {-1, -1};
	int64_t retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(index,sizeof(index)) < 0 )
		return printSocketReadError();

	// execute action
	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if (index[1] >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
		//	else if (receiverBase->getStatus() != IDLE)
		//		receiverNotIdle();
			else {
				switch (index[0]) {
				case ACQUISITION_TIME:
					ret = receiverBase->setAcquisitionTime(index[1]);
					break;
				case FRAME_PERIOD:
					ret = receiverBase->setAcquisitionPeriod(index[1]);
					break;
				case FRAME_NUMBER:
				case CYCLES_NUMBER:
				case STORAGE_CELL_NUMBER:
					receiverBase->setNumberOfFrames(index[1]);
					break;
				case SUBFRAME_ACQUISITION_TIME:
					receiverBase->setSubExpTime(index[1]);
					break;
				case SUBFRAME_DEADTIME:
					receiverBase->setSubPeriod(index[1] + receiverBase->getSubExpTime());
					break;
				case SAMPLES_JCTB:
					if (myDetectorType != JUNGFRAUCTB) {
						ret = FAIL;
						sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
						FILE_LOG(logERROR) << "Warning: " << mess;
						break;
					}
					receiverBase->setNumberofSamples(index[1]);
					break;
				default:
					ret = FAIL;
					sprintf(mess,"This timer mode (%lld) does not exist for receiver\n", (long long int)index[0]);
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		// get
		switch (index[0]) {
		case ACQUISITION_TIME:
			retval=receiverBase->getAcquisitionTime();
			break;
		case FRAME_PERIOD:
			retval=receiverBase->getAcquisitionPeriod();
			break;
		case FRAME_NUMBER:
		case CYCLES_NUMBER:
		case STORAGE_CELL_NUMBER:
			retval=receiverBase->getNumberOfFrames();
			break;
		case SUBFRAME_ACQUISITION_TIME:
			retval=receiverBase->getSubExpTime();
			break;
		case SUBFRAME_DEADTIME:
			retval=(receiverBase->getSubPeriod() - receiverBase->getSubExpTime());
			break;
		case SAMPLES_JCTB:
			if (myDetectorType != JUNGFRAUCTB) {
				ret = FAIL;
				sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
				FILE_LOG(logERROR) << "Warning: " << mess;
				break;
			}
			retval=receiverBase->getNumberofSamples();
			break;
		default:
			ret = FAIL;
			sprintf(mess,"This timer mode (%lld) does not exist for receiver\n", (long long int)index[0]);
			FILE_LOG(logERROR) << mess;
		}

		// check
		if (ret == OK && index[1] >= 0 && retval != index[1]) {
			ret = FAIL;
			strcpy(mess,"Could not set timer\n");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << slsDetectorDefs::getTimerType((timerIndex)(index[0])) << ":" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_dynamic_range() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int dr = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&dr,sizeof(dr)) < 0 )
		return printSocketReadError();

	// execute action
	bool exists = false;
	switch (dr) {
	case -1:
	case 16:
		exists = true;
		break;
	case 4:
	case 8:
	case 32:
		if (myDetectorType == EIGER)
			exists = true;
		break;
	default:
		break;
	}
	if (!exists) {
		ret = FAIL;
		sprintf(mess,"This dynamic range %d does not exist for this detector\n",dr);
		FILE_LOG(logERROR) << mess;
	}


	if (ret == OK){
		if (receiverBase == NULL)
			invalidReceiverObject();
		else {
			// set
			if (dr > 0) {
				if (mySock->differentClients && lockStatus)
					receiverlocked();
				else if (receiverBase->getStatus() != IDLE)
					receiverNotIdle();
				else {
					ret = receiverBase->setDynamicRange(dr);
					if(ret == FAIL) {
						strcpy(mess, "Could not allocate memory for fifo or could not start listening/writing threads\n");
						FILE_LOG(logERROR) << mess;
					}
				}
			}
			//get
			retval = receiverBase->getDynamicRange();
			if(dr > 0 && retval != dr) {
				ret = FAIL;
				strcpy(mess, "Could not set dynamic range\n");
				FILE_LOG(logERROR) << mess;
			}
		}
	}
	FILE_LOG(logDEBUG1) << "dynamic range: " << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_streaming_frequency(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if (index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				ret = receiverBase->setStreamingFrequency(index);
				if(ret == FAIL) {
					strcpy(mess, "Could not allocate memory for listening fifo\n");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		//get
		retval=receiverBase->getStreamingFrequency();
		if(index >= 0 && retval != index){
			ret = FAIL;
			strcpy(mess,"Could not set streaming frequency");
			FILE_LOG(logERROR) << mess;
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int	slsReceiverTCPIPInterface::get_status(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum runStatus retval = ERROR;

	if (receiverBase == NULL)
		invalidReceiverObject();
	else retval = receiverBase->getStatus();

	if (ret == OK && mySock->differentClients)
		ret = FORCE_UPDATE;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::start_receiver(){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	if (receiverBase == NULL)
		invalidReceiverObject();
	else if (mySock->differentClients && lockStatus)
		receiverlocked();
	else {
		enum runStatus s = receiverBase->getStatus();
		if (s != IDLE) {
			ret=FAIL;
			sprintf(mess,"Cannot start Receiver as it is in %s state\n",runStatusType(s).c_str());
			FILE_LOG(logERROR) << mess;
		}
		else {
			ret=receiverBase->startReceiver(mess);
			if (ret == FAIL) {
				FILE_LOG(logERROR) << mess;
			}
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, NULL, 0, mess);

	return ret;


}



int slsReceiverTCPIPInterface::stop_receiver(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum runStatus s = ERROR;

	if (receiverBase == NULL)
		invalidReceiverObject();
	else if (mySock->differentClients && lockStatus)
		receiverlocked();
	else {
		if(receiverBase->getStatus() != IDLE)
			receiverBase->stopReceiver();
		s = receiverBase->getStatus();
		if (s == IDLE)
			ret = OK;
		else {
			ret = FAIL;
			sprintf(mess,"Could not stop receiver. It is in %s state\n",runStatusType(s).c_str());
			FILE_LOG(logERROR) << mess;
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, NULL, 0, mess);

	return ret;
}






int slsReceiverTCPIPInterface::set_file_dir() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char fPath[MAX_STR_LENGTH];
	memset(fPath, 0, sizeof(fPath));
	char* retval=NULL;

	// receive arguments
	if (mySock->ReceiveDataOnly(fPath,MAX_STR_LENGTH) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if (strlen(fPath)) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setFilePath(fPath);
			}
		}
		//get
		retval = receiverBase->getFilePath();
		if (retval == NULL || (strlen(fPath) && strcasecmp(fPath, retval))) {
			ret = FAIL;
			strcpy(mess,"receiver file path does not exist\n");
			FILE_LOG(logERROR) << mess;
		}
	}
	if (retval != NULL) {
		FILE_LOG(logDEBUG1) << "file path:" << retval;
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, retval, (retval == NULL) ? 0 : MAX_STR_LENGTH, mess);

	if(retval != NULL)
		delete[] retval;

	return ret;
}



int slsReceiverTCPIPInterface::set_file_name() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char fName[MAX_STR_LENGTH];
	memset(fName, 0, sizeof(fName));
	char* retval = NULL;

	// receive arguments
	if (mySock->ReceiveDataOnly(fName,MAX_STR_LENGTH) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if (strlen(fName)) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setFileName(fName);
			}
		}
		//get
		retval = receiverBase->getFileName();
		if(retval == NULL) {
			ret = FAIL;
			strcpy(mess, "file name is empty\n");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "file name:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, retval, (retval == NULL) ? 0 : MAX_STR_LENGTH, mess);

	if(retval != NULL)
		delete[] retval;

	return ret;
}



int slsReceiverTCPIPInterface::set_file_index() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setFileIndex(index);
			}
		}
		//get
		retval=receiverBase->getFileIndex();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set file index\n");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "file index:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval,sizeof(retval), mess);

	return ret;
}






int	slsReceiverTCPIPInterface::get_frame_index(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	if (receiverBase == NULL)
		invalidReceiverObject();
	else
		retval=receiverBase->getAcquisitionIndex();

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval,sizeof(retval), mess);

	return ret;
}



int	slsReceiverTCPIPInterface::get_frames_caught(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	if (receiverBase == NULL)
		invalidReceiverObject();
	else retval=receiverBase->getTotalFramesCaught();

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval,sizeof(retval), mess);

	return ret;
}



int	slsReceiverTCPIPInterface::reset_frames_caught(){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	if (receiverBase == NULL)
		invalidReceiverObject();
	else if (mySock->differentClients && lockStatus)
		receiverlocked();
	else if (receiverBase->getStatus() != IDLE)
		receiverNotIdle();
	else
		receiverBase->resetAcquisitionCount();

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, NULL, 0, mess);

	return ret;
}



int slsReceiverTCPIPInterface::enable_file_write(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&enable,sizeof(enable)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if (enable >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setFileWriteEnable(enable);
			}
		}
		//get
		retval=receiverBase->getFileWriteEnable();
		if(enable >= 0 && enable != retval) {
			ret=FAIL;
			strcpy(mess,"Could not set file write enable");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "file write enable:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}





int slsReceiverTCPIPInterface::enable_overwrite() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setOverwriteEnable(index);
			}
		}
		//get
		retval=receiverBase->getOverwriteEnable();
		if(index >=0 && retval != index) {
			ret = FAIL;
			strcpy(mess,"Could not set file over write enable\n");
			FILE_LOG(logERROR) << mess;
		}
	}

	FILE_LOG(logDEBUG1) << "file overwrite enable:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::enable_tengiga() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int val = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&val,sizeof(val)) < 0 )
		return printSocketReadError();

	if (myDetectorType != EIGER)
		functionNotImplemented();

	else {
		if (receiverBase == NULL)
			invalidReceiverObject();
		else {
			// set
			if (val >= 0) {
				if (mySock->differentClients && lockStatus)
					receiverlocked();
				else if (receiverBase->getStatus() != IDLE)
					receiverNotIdle();
				else {
					ret = receiverBase->setTenGigaEnable(val);
				}
			}
			//get
			retval=receiverBase->getTenGigaEnable();
			if((val >= 0) && (val != retval)) {
				ret = FAIL;
				strcpy(mess,"Could not set ten giga enable");
				FILE_LOG(logERROR) << mess;
			}
		}
	}
	FILE_LOG(logDEBUG1) << "10Gbe:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}




int slsReceiverTCPIPInterface::set_fifo_depth() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int value = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&value,sizeof(value)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(value >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				ret = receiverBase->setFifoDepth(value);
				if (ret == FAIL) {
					strcpy(mess,"Could not set fifo depth");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		//get
		retval = receiverBase->getFifoDepth();
		if(value >= 0 && retval != value) {
			ret = FAIL;
			strcpy(mess, "Could not set fifo depth\n");
			FILE_LOG(logERROR) << mess;
		}
	}

	FILE_LOG(logDEBUG1) << "fifo depth:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_activate() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&enable,sizeof(enable)) < 0 )
		return printSocketReadError();

	if (myDetectorType != EIGER)
		functionNotImplemented();

	else {
		if (receiverBase == NULL)
			invalidReceiverObject();
		else {
			// set
			if(enable >= 0) {
				if (mySock->differentClients && lockStatus)
					receiverlocked();
				else if (receiverBase->getStatus() != IDLE)
					receiverNotIdle();
				else {
					receiverBase->setActivate(enable > 0 ? true : false);
				}
			}
			//get
			retval = (int)receiverBase->getActivate();
			if(enable >= 0 && retval != enable){
				ret = FAIL;
				sprintf(mess,"Could not set activate to %d, returned %d\n",enable,retval);
				FILE_LOG(logERROR) << mess;
			}
		}
	}
	FILE_LOG(logDEBUG1) << "Activate: " << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_data_stream_enable(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				ret = receiverBase->setDataStreamEnable(index);
			}
		}
		//get
		retval = receiverBase->getDataStreamEnable();
		if(index >= 0 && retval != index){
			ret = FAIL;
			strcpy(mess,"Could not set data stream enable");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "data streaming enable:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_streaming_timer(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if(mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setStreamingTimer(index);
			}
		}
		//get
		retval=receiverBase->getStreamingTimer();
		if(index >= 0 && retval != index){
			ret = FAIL;
			strcpy(mess,"Could not set datastream timer");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "Streaming timer:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_flipped_data(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {0,-1};
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(args,sizeof(args)) < 0 )
		return printSocketReadError();

	if (myDetectorType != EIGER)
		functionNotImplemented();

	else {
		if (receiverBase == NULL)
			invalidReceiverObject();
		else {
			// set
			if(args[1] >= 0) {
				if (mySock->differentClients && lockStatus)
					receiverlocked();
				else if (receiverBase->getStatus() != IDLE)
					receiverNotIdle();
				else {
					receiverBase->setFlippedData(args[0],args[1]);
				}
			}
			//get
			retval=receiverBase->getFlippedData(args[0]);
			if (args[1] > -1 && retval != args[1]) {
				ret = FAIL;
				strcpy(mess, "Could not set flipped data\n");
				FILE_LOG(logERROR) << mess;
			}
		}
	}
	FILE_LOG(logDEBUG1) << "Flipped Data:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}




int slsReceiverTCPIPInterface::set_file_format() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	fileFormat retval = GET_FILE_FORMAT;
	fileFormat f = GET_FILE_FORMAT;

	// receive arguments
	if (mySock->ReceiveDataOnly(&f,sizeof(f)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(f >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setFileFormat(f);
			}
		}
		//get
		retval = receiverBase->getFileFormat();
		if(f >= 0 && retval != f){
			ret = FAIL;
			sprintf(mess,"Could not set file format to %s, returned %s\n",getFileFormatType(f).c_str(),getFileFormatType(retval).c_str());
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "File Format: " << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_detector_posid() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&arg,sizeof(arg)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(arg >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setDetectorPositionId(arg);
			}
		}
		//get
		retval=receiverBase->getDetectorPositionId();
		if (arg >= 0 && retval != arg) {
			ret = FAIL;
			strcpy(mess,"Could not set detector position id");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "Position Id:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}






int slsReceiverTCPIPInterface::set_multi_detector_size() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg[2] = {-1, -1};
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(arg,sizeof(arg)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if((arg[0] > 0) && (arg[1] > 0)) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setMultiDetectorSize(arg);
			}
		}
		//get
		int* temp = receiverBase->getMultiDetectorSize();
		for (int i = 0; i < MAX_DIMENSIONS; ++i) {
			if (!i)
				retval = temp[i];
			else
				retval *= temp[i];
		}
	}
	FILE_LOG(logDEBUG1) << "Multi Detector Size:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}




int slsReceiverTCPIPInterface::set_streaming_port() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int port = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&port,sizeof(port)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(port >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setStreamingPort(port);
			}
		}
		//get
		retval=receiverBase->getStreamingPort();
	}
	FILE_LOG(logDEBUG1) << "streaming port:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}





int slsReceiverTCPIPInterface::set_streaming_source_ip() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char arg[MAX_STR_LENGTH];
	memset(arg, 0, sizeof(arg));
	char* retval=NULL;

	// receive arguments
	if (mySock->ReceiveDataOnly(arg,MAX_STR_LENGTH) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if (mySock->differentClients && lockStatus)
			receiverlocked();
		else if (receiverBase->getStatus() != IDLE)
			receiverNotIdle();
		else {
			receiverBase->setStreamingSourceIP(arg);
		}

		//get
		retval = receiverBase->getStreamingSourceIP();
	}
	FILE_LOG(logDEBUG1) << "streaming source ip:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, retval, (retval == NULL) ? 0 : MAX_STR_LENGTH, mess);

	if(retval != NULL)
		delete[] retval;

	return ret;
}







int slsReceiverTCPIPInterface::set_silent_mode() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int value = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&value,sizeof(value)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(value >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setSilentMode(value); // no check required
			}
		}
		//get
		retval = (int)receiverBase->getSilentMode(); // no check required
	}
	FILE_LOG(logDEBUG1) << "silent mode:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}






int slsReceiverTCPIPInterface::enable_gap_pixels() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&enable,sizeof(enable)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(enable >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				if ((myDetectorType != EIGER) && (enable > 0))
					functionNotImplemented();
				else
					receiverBase->setGapPixelsEnable(enable);
			}
		}
		//get
		retval = receiverBase->getGapPixelsEnable();
		if(enable >= 0 && retval != enable){
			ret = FAIL;
			sprintf(mess,"Could not set gap pixels to %d, returned %d\n",enable,retval);
			FILE_LOG(logERROR) << "Warning: " << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "Gap Pixels Enable: " << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}




int slsReceiverTCPIPInterface::restream_stop(){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	if (receiverBase == NULL)
		invalidReceiverObject();
	else if (mySock->differentClients && lockStatus)
		receiverlocked();
	else if (receiverBase->getStatus() != IDLE)
		receiverNotIdle();
	else if (receiverBase->getDataStreamEnable() == false) {
			ret = FAIL;
			sprintf(mess,"Could not restream stop packet as data Streaming is disabled.\n");
			FILE_LOG(logERROR) << mess;
	} else {
		ret = receiverBase->restreamStop();
		if (ret == FAIL) {
			sprintf(mess,"Could not restream stop packet.\n");
			FILE_LOG(logERROR) << mess;
		}
	}

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, NULL, 0, mess);

	return ret;
}



int slsReceiverTCPIPInterface::set_additional_json_header() {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    char arg[MAX_STR_LENGTH];
    memset(arg, 0, sizeof(arg));
    char* retval=NULL;

    // receive arguments
    if (mySock->ReceiveDataOnly(arg,MAX_STR_LENGTH) < 0 )
        return printSocketReadError();

    if (receiverBase == NULL)
        invalidReceiverObject();
    else {
        // set
        if (mySock->differentClients && lockStatus)
            receiverlocked();
        else if (receiverBase->getStatus() != IDLE)
            receiverNotIdle();
        else {
                receiverBase->setAdditionalJsonHeader(arg);
        }

        //get
        retval = receiverBase->getAdditionalJsonHeader();
    }
    FILE_LOG(logDEBUG1) << "additional json header:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, retval, MAX_STR_LENGTH, mess);

	if (retval != NULL)
		delete[] retval;

     return ret;
}



int slsReceiverTCPIPInterface::set_udp_socket_buffer_size() {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int index = -1;
    int retval = -1;

    // receive arguments
    if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
        return printSocketReadError();

    if (receiverBase == NULL)
        invalidReceiverObject();
    else {
        // set
        if(index >= 0) {
            if (mySock->differentClients && lockStatus)
                receiverlocked();
            else if (receiverBase->getStatus() != IDLE)
                receiverNotIdle();
            else {
                if (receiverBase->setUDPSocketBufferSize(index) == FAIL) {
                    ret = FAIL;
                    strcpy(mess, "Could not create dummy UDP Socket to test buffer size\n");
                    FILE_LOG(logERROR) << mess;
                }
            }
        }
        //get
        retval=receiverBase->getUDPSocketBufferSize();
        if(index >= 0 && ((retval != index) || ((int)receiverBase->getActualUDPSocketBufferSize() != (index*2)))) {
            ret = FAIL;
            strcpy(mess, "Could not set UDP Socket buffer size (No CAP_NET_ADMIN privileges?)\n");
            FILE_LOG(logERROR) << mess;
        }
    }
    FILE_LOG(logDEBUG1) << "UDP Socket Buffer Size:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

    return ret;
}



int slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size(){
    ret = OK;
	memset(mess, 0, sizeof(mess));
    int retval = -1;

    if (receiverBase == NULL)
        invalidReceiverObject();
    else retval = receiverBase->getActualUDPSocketBufferSize();

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

    return ret;
}



int slsReceiverTCPIPInterface::set_frames_per_file() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setFramesPerFile(index);
			}
		}
		//get
		retval=receiverBase->getFramesPerFile();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set frames per file\n");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "frames per file:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}






int slsReceiverTCPIPInterface::check_version_compatibility() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t arg = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&arg,sizeof(arg)) < 0 )
		return printSocketReadError();


	// execute action
	FILE_LOG(logDEBUG1) << "Checking versioning compatibility with value " << arg;

	int64_t client_requiredVersion = arg;
	int64_t rx_apiVersion = APIRECEIVER;
	int64_t rx_version = getReceiverVersion();

	// old client
	if (rx_apiVersion > client_requiredVersion) {
		ret = FAIL;
		sprintf(mess,"This client is incompatible.\n"
				"Client's receiver API Version: (0x%llx). Receiver API Version: (0x%llx).\n"
				"Incompatible, update client!\n",
				(long long unsigned int)client_requiredVersion,
				(long long unsigned int)rx_apiVersion);
		FILE_LOG(logERROR) << mess;
	}

	// old software
	else if (client_requiredVersion > rx_version) {
		ret = FAIL;
		sprintf(mess,"This receiver is incompatible.\n"
				"Receiver Version: (0x%llx). Client's receiver API Version: (0x%llx).\n"
				"Incompatible, update receiver!\n",
				(long long unsigned int)rx_version,
				(long long unsigned int)client_requiredVersion);
		FILE_LOG(logERROR) << mess;
	}
	else FILE_LOG(logINFO) << "Compatibility with Client: Successful";

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, NULL, 0, mess);

	return ret;
}




int slsReceiverTCPIPInterface::set_discard_policy() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				receiverBase->setFrameDiscardPolicy((frameDiscardPolicy)index);
			}
		}
		//get
		retval=receiverBase->getFrameDiscardPolicy();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set frame discard policy\n");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "frame discard policy:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}




int slsReceiverTCPIPInterface::set_padding_enable() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&index,sizeof(index)) < 0 )
		return printSocketReadError();

	if (receiverBase == NULL)
		invalidReceiverObject();
	else {
		// set
		if(index >= 0) {
			if (mySock->differentClients && lockStatus)
				receiverlocked();
			else if (receiverBase->getStatus() != IDLE)
				receiverNotIdle();
			else {
				index = (index == 0) ? 0 : 1;
				receiverBase->setFramePaddingEnable(index);
			}
		}
		//get
		retval=(int)receiverBase->getFramePaddingEnable();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set frame padding enable\n");
			FILE_LOG(logERROR) << mess;
		}
	}
	FILE_LOG(logDEBUG1) << "Frame Padding Enable:" << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}




int slsReceiverTCPIPInterface::set_deactivated_receiver_padding_enable() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// receive arguments
	if (mySock->ReceiveDataOnly(&enable,sizeof(enable)) < 0 )
		return printSocketReadError();

	if (myDetectorType != EIGER)
		functionNotImplemented();

	else {
		if (receiverBase == NULL)
			invalidReceiverObject();
		else {
			// set
			if(enable >= 0) {
				if (mySock->differentClients && lockStatus)
					receiverlocked();
				else if (receiverBase->getStatus() != IDLE)
					receiverNotIdle();
				else {
					receiverBase->setDeactivatedPadding(enable > 0 ? true : false);
				}
			}
			//get
			retval = (int)receiverBase->getDeactivatedPadding();
			if(enable >= 0 && retval != enable){
				ret = FAIL;
				sprintf(mess,"Could not set deactivated padding enable to %d, returned %d\n",enable,retval);
				FILE_LOG(logERROR) << mess;
			}
		}
	}
	FILE_LOG(logDEBUG1) << "Deactivated Padding Enable: " << retval;

	clientInterface->Server_SendResult(mySock->differentClients,
			ret, &retval, sizeof(retval), mess);

	return ret;
}
