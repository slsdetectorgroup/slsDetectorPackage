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
#include <array>
#include <memory>	//unique_ptr

slsReceiverTCPIPInterface::~slsReceiverTCPIPInterface() {
	stop();
	if(mySock) {
		delete mySock;
		mySock=NULL;
	}
	if (interface)
		delete interface;
	if(receiver)
		delete receiver;
}

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int pn):
				myDetectorType(GOTTHARD),
				receiver(0),
				ret(OK),
				fnum(-1),
				lockStatus(0),
				killTCPServerThread(0),
				tcpThreadCreated(false),
				portNumber(DEFAULT_PORTNO+2),
				mySock(0),
				interface(0)
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
	interface = new ClientInterface(mySock, -1, "Receiver");

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
			if(receiver){
				receiver->shutDownUDPSockets();
			}

			mySock->exitServer();
			cprintf(BLUE,"Exiting [ TCP server Tid: %ld ]\n", (long)syscall(SYS_gettid));
			pthread_exit(NULL);
		}

		//if user entered exit
		if(killTCPServerThread) {
			if (ret != GOODBYE) {
				if(receiver){
					receiver->shutDownUDPSockets();
				}
			}
			cprintf(BLUE,"Exiting [ TCP server Tid: %ld ]\n", (long)syscall(SYS_gettid));
			pthread_exit(NULL);
		}
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
				getFunctionNameFromEnum((enum detFuncs)i) << ") located at " << flist[i];
	}

	return OK;
}




int slsReceiverTCPIPInterface::decode_function(){
	ret = FAIL;
	int n = mySock->ReceiveDataOnly(&fnum,sizeof(fnum));
	if (n <= 0) {
		FILE_LOG(logDEBUG5) << "Could not read socket. "
				"Received " << n << " bytes," <<
				"fnum:" << fnum << " "
				"(" << getFunctionNameFromEnum((enum detFuncs)fnum) << ")";
		return FAIL;
	}
	else
		FILE_LOG(logDEBUG5) << "Received " << n << " bytes";

	if (fnum <= NUM_DET_FUNCTIONS || fnum >= NUM_REC_FUNCTIONS) {
		FILE_LOG(logERROR) << "Unknown function enum " << fnum;
		ret = (this->M_nofunc)();
	} else{
		FILE_LOG(logDEBUG5) <<  "calling function fnum: "<< fnum << " "
				"(" << getFunctionNameFromEnum((enum detFuncs)fnum) << ") "
				"located at " << flist[fnum];
		ret = (this->*flist[fnum])();

		if (ret == FAIL) {
			FILE_LOG(logDEBUG5) << "Failed to execute function = " << fnum << " ("
					<< getFunctionNameFromEnum((enum detFuncs)fnum) << ")";
		}
	}
	return ret;
}


void slsReceiverTCPIPInterface::functionNotImplemented() {
	ret = FAIL;
	sprintf(mess, "Function (%s) is not implemented for this detector\n",
			getFunctionNameFromEnum((enum detFuncs)fnum));
	FILE_LOG(logERROR) << mess;
}


int slsReceiverTCPIPInterface::M_nofunc(){
	ret = FAIL;
	memset(mess, 0, sizeof(mess));
	int n = 0;

	// to receive any arguments
	while (n > 0)
		n = mySock->ReceiveDataOnly(mess,MAX_STR_LENGTH);

	strcpy(mess,"Unrecognized Function. Please do not proceed.\n");
	FILE_LOG(logERROR) << mess;

	return interface->Server_SendResult(false, ret, NULL, 0, mess);
}




int slsReceiverTCPIPInterface::exec_command() {
	ret = FAIL;
	memset(mess, 0, sizeof(mess));
	char cmd[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, cmd, MAX_STR_LENGTH) == FAIL)
		return FAIL;

	// verify if receiver is unlocked
	if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {

		const size_t tempsize = 256;
		std::array<char, tempsize> temp;
		std::string sresult;
		std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
		if (!pipe)  {
			ret = FAIL;
			strcpy(mess, "Executing Command failed\n");
			FILE_LOG(logERROR) << mess;
		} else  {
			while (!feof(pipe.get())) {
				if (fgets(temp.data(), tempsize, pipe.get()) != NULL)
					sresult += temp.data();
			}
			strncpy(retval, sresult.c_str(), MAX_STR_LENGTH);
			ret = OK;
		}
	}

	return interface->Server_SendResult(false, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::exit_server() {
	cprintf(RED,"Closing server\n");
	memset(mess, 0, sizeof(mess));
	ret = OK;
	interface->Server_SendResult(false, ret, NULL, 0);
	return GOODBYE;
}



int slsReceiverTCPIPInterface::lock_receiver() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int lock = 0;

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, &lock, sizeof(lock)) == FAIL)
		return FAIL;
	FILE_LOG(logDEBUG5) << "Locking Server to " << lock;

	// execute action
	if (lock >= 0) {
		if (!lockStatus || // if it was unlocked, anyone can lock
				(!strcmp(mySock->lastClientIP,mySock->thisClientIP)) || // if it was locked, need same ip
				(!strcmp(mySock->lastClientIP,"none"))) // if it was locked, must be by "none"
			{
			lockStatus = lock;
			strcpy(mySock->lastClientIP,mySock->thisClientIP);
		}   else
			 interface->Server_LockedError(ret, mess);
	}
	return interface->Server_SendResult(true, ret, &lockStatus,sizeof(lockStatus), mess);
}



int slsReceiverTCPIPInterface::get_last_client_ip() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	return interface->Server_SendResult(true, ret,mySock->lastClientIP, sizeof(mySock->lastClientIP));
}



int slsReceiverTCPIPInterface::set_port() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int p_number = -1;
	MySocketTCP* mySocket = 0;
	char oldLastClientIP[INET_ADDRSTRLEN] = {0};

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, &p_number, sizeof(p_number)) == FAIL)
		return FAIL;

	// verify if receiver is unlocked
	if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
		// port number too low
		if (p_number < 1024) {
			ret = FAIL;
			sprintf(mess,"Port Number (%d) too low\n", p_number);
			FILE_LOG(logERROR) << mess;
		} else {
			FILE_LOG(logINFO) << "set port to " << p_number <<std::endl;
			strcpy(oldLastClientIP, mySock->lastClientIP);
			// create new socket
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

	interface->Server_SendResult(true, ret, &p_number,sizeof(p_number), mess);
	// delete old socket
	if(ret != FAIL){
		mySock->Disconnect();
		delete mySock;
		mySock = mySocket;
		interface->SetSocket(mySock);
	}

	return ret;
}



int slsReceiverTCPIPInterface::update_client() {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	interface->Server_SendResult(false, ret, NULL, 0, mess);

	if (ret == FAIL)
		return ret;

	// update
	return send_update();
}



int slsReceiverTCPIPInterface::send_update() {
	int ind = -1;
	char path[MAX_STR_LENGTH] = {0};
	int n = 0;

	n += mySock->SendDataOnly(mySock->lastClientIP,sizeof(mySock->lastClientIP));

	// filepath
	path = receiver->getFilePath();
    n += mySock->SendDataOnly(path, sizeof(path));

	// filename
	path = receiver->getFileName();
    n += mySock->SendDataOnly(path, sizeof(path));

	// index
	ind=receiver->getFileIndex();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	//file format
	ind=(int)receiver->getFileFormat();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	//frames per file
	ind=(int)receiver->getFramesPerFile();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	//frame discard policy
	ind=(int)receiver->getFrameDiscardPolicy();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	//frame padding
	ind=(int)receiver->getFramePaddingEnable();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// file write enable
	ind=(int)receiver->getFileWriteEnable();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// file overwrite enable
	ind=(int)receiver->getOverwriteEnable();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// gap pixels
	ind=(int)receiver->getGapPixelsEnable();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// streaming frequency
	ind=(int)receiver->getStreamingFrequency();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// streaming port
	ind=(int)receiver->getStreamingPort();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// streaming source ip
	path = receiver->getStreamingSourceIP();
    n += mySock->SendDataOnly(path, sizeof(path));

    // additional json header
    path = receiver->getAdditionalJsonHeader();
    n += mySock->SendDataOnly(path, sizeof(path));

	// data streaming enable
	ind=(int)receiver->getDataStreamEnable();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// activate
	ind=(int)receiver->getActivate();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// deactivated padding enable
	ind=(int)receiver->getDeactivatedPadding();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	// silent mode
	ind=(int)receiver->getSilentMode();
	n += mySock->SendDataOnly(&ind, sizeof(ind));

	if (!lockStatus)
		strcpy(mySock->lastClientIP,mySock->thisClientIP);

	return OK;
}



int slsReceiverTCPIPInterface::get_id(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t retval = getReceiverVersion();
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval));
}



int slsReceiverTCPIPInterface::set_detector_type(){
	memset(mess, 0, sizeof(mess));
	detectorType dr = GENERIC;
	detectorType retval = GENERIC;

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, &dr, sizeof(dr)) == FAIL)
		return FAIL;

	// set
	if (dr >= 0) {
		// if object exists, verify unlocked and idle, else only verify lock (connecting first time)
		if (receiver == NULL)
			interface->Server_VerifyLock(ret, mess, lockStatus);
		else
			interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum);
		if (ret == OK) {
			switch(dr) {
			case GOTTHARD:
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
				if(receiver == NULL){
					receiver = new slsReceiverImplementation();
					if(startAcquisitionCallBack)
						receiver->registerCallBackStartAcquisition(startAcquisitionCallBack,pStartAcquisition);
					if(acquisitionFinishedCallBack)
						receiver->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,pAcquisitionFinished);
					if(rawDataReadyCallBack)
						receiver->registerCallBackRawDataReady(rawDataReadyCallBack,pRawDataReady);
					if(rawDataModifyReadyCallBack)
						receiver->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,pRawDataReady);
				}
				myDetectorType = dr;
				ret = receiver->setDetectorType(myDetectorType);
				retval = myDetectorType;

				// client has started updating receiver, update ip
				if (!lockStatus)
					strcpy(mySock->lastClientIP,mySock->thisClientIP);
			}

		}
	}
	//get
	retval = myDetectorType;
	return interface->Server_SendResult(false, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_detector_hostname() {
	memset(mess, 0, sizeof(mess));
	char hostname[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, hostname,MAX_STR_LENGTH, true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (strlen(hostname)) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK)
				receiver->setDetectorHostname(hostname);
		}
		// get
		retval = receiver->getDetectorHostname();
		if (strlen(retval)) {
			ret = FAIL;
			sprintf(mess, "hostname not set\n");
			FILE_LOG(logERROR) << mess;
		}
	}

	return interface->Server_SendResult(true, ret,	retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_roi() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int nroi = 0;

	// receive arguments
	if (mySock->ReceiveDataOnly(&nroi,sizeof(nroi)) < 0 )
		return interface->Server_SocketCrash();

	std::vector <ROI> roiLimits;
	int iloop = 0;
	for (iloop = 0; iloop < nroi; iloop++) {
		ROI temp;
		if ( mySock->ReceiveDataOnly(&temp.xmin,sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		if ( mySock->ReceiveDataOnly(&temp.xmax,sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		if ( mySock->ReceiveDataOnly(&temp.ymin,sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		if ( mySock->ReceiveDataOnly(&temp.ymax,sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		roiLimits.push_back(temp);
	}

	// only for gotthard
	if (myDetectorType != GOTTHARD)
		functionNotImplemented();

	// base object not null
	else if (receiver == NULL)
		interface->Server_NullObjectError(ret, mess);
	else {
		// only set
		// verify if receiver is unlocked and idle
		if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK)
			ret = receiver->setROI(roiLimits);
	}

	interface->Server_SendResult(true, ret, NULL, 0, mess);

	roiLimits.clear();

	return ret;
}



int slsReceiverTCPIPInterface::setup_udp(){
	memset(mess, 0, sizeof(mess));
	char args[3][MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, args, sizeof(args), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
			//set up udp port
			int udpport=-1,udpport2=-1;
			sscanf(args[1],"%d",&udpport);
			sscanf(args[2],"%d",&udpport2);
			receiver->setUDPPortNumber(udpport);
			if (myDetectorType == EIGER)
				receiver->setUDPPortNumber2(udpport2);

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
				receiver->setEthernetInterface(eth);

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
	}

	return interface->Server_SendResult(true, ret, retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_timer() {
	memset(mess, 0, sizeof(mess));
	int64_t index[2] = {-1, -1};
	int64_t retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		FILE_LOG(logDEBUG5, ("Setting timer index %d to %lld ns\n", index[0], index[1]));
		char timername[100] = {0};

		// set
		if (index[1] >= 0) {
			// verify if receiver is unlocked
			if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
				switch (index[0]) {
				case ACQUISITION_TIME:
					strcpy(timername, "exptime");
					ret = receiver->setAcquisitionTime(index[1]);
					break;
				case FRAME_PERIOD:
					strcpy(timername, "period");
					ret = receiver->setAcquisitionPeriod(index[1]);
					break;
				case FRAME_NUMBER:
				case CYCLES_NUMBER:
				case STORAGE_CELL_NUMBER:
					strcpy(timername, "frames_cycles_storagecells");
					receiver->setNumberOfFrames(index[1]);
					break;
				case SUBFRAME_ACQUISITION_TIME:
					strcpy(timername, "subexptime");
					receiver->setSubExpTime(index[1]);
					break;
				case SUBFRAME_DEADTIME:
					strcpy(timername, "subdeadtime");
					receiver->setSubPeriod(index[1] + receiver->getSubExpTime());
					break;
				case SAMPLES_JCTB:
					strcpy(timername, "samples");
					if (myDetectorType != JUNGFRAUCTB) {
						ret = FAIL;
						sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
						FILE_LOG(logERROR) << "Warning: " << mess;
						break;
					}
					receiver->setNumberofSamples(index[1]);
					break;
				default:
					strcpy(timername, "unknown");
					ret = FAIL;
					sprintf(mess,"This timer mode (%lld) does not exist for receiver\n", (long long int)index[0]);
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		// get
		switch (index[0]) {
		case ACQUISITION_TIME:
			retval=receiver->getAcquisitionTime();
			break;
		case FRAME_PERIOD:
			retval=receiver->getAcquisitionPeriod();
			break;
		case FRAME_NUMBER:
		case CYCLES_NUMBER:
		case STORAGE_CELL_NUMBER:
			retval=receiver->getNumberOfFrames();
			break;
		case SUBFRAME_ACQUISITION_TIME:
			retval=receiver->getSubExpTime();
			break;
		case SUBFRAME_DEADTIME:
			retval=(receiver->getSubPeriod() - receiver->getSubExpTime());
			break;
		case SAMPLES_JCTB:
			if (myDetectorType != JUNGFRAUCTB) {
				ret = FAIL;
				sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
				FILE_LOG(logERROR) << "Warning: " << mess;
				break;
			}
			retval=receiver->getNumberofSamples();
			break;
		default:
			ret = FAIL;
			sprintf(mess,"This timer mode (%lld) does not exist for receiver\n", (long long int)index[0]);
			FILE_LOG(logERROR) << mess;
		}

		// check
		if (ret == OK && index[1] >= 0 && retval != index[1]) {
			ret = FAIL;
			strcpy(mess,"Could not set timer %s\n", timername);
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << slsDetectorDefs::getTimerType((timerIndex)(index[0])) << ":" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_dynamic_range() {
	memset(mess, 0, sizeof(mess));
	int dr = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &dr, sizeof(dr), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (dr >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				bool exists = false;
				switch (dr) {
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
				// invalid dr
				if (!exists) {
					ret = FAIL;
					sprintf(mess,"This dynamic range %d does not exist for this detector\n",dr);
					FILE_LOG(logERROR) << mess;
				}
				// valid dr
				else {
					ret = receiver->setDynamicRange(dr);
					if(ret == FAIL) {
						strcpy(mess, "Could not allocate memory for fifo or could not start listening/writing threads\n");
						FILE_LOG(logERROR) << mess;
					}
				}
			}
		}
		// get
		retval = receiver->getDynamicRange();
		if(dr > 0 && retval != dr) {
			ret = FAIL;
			strcpy(mess, "Could not set dynamic range\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "dynamic range: " << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_frequency(){
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				ret = receiver->setStreamingFrequency(index);
				if(ret == FAIL) {
					strcpy(mess, "Could not allocate memory for listening fifo\n");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		// get
		retval=receiver->getStreamingFrequency();
		// check retval for failure
		if(ret == OK && index >= 0 && retval != index){
			ret = FAIL;
			strcpy(mess,"Could not set streaming frequency");
			FILE_LOG(logERROR) << mess;
		}
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_status(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum runStatus retval = ERROR;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	if (ret == OK)
		retval = receiver->getStatus();

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::start_receiver(){
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
			// should not be idle
			enum runStatus s = receiver->getStatus();
			if (s != IDLE) {
				ret=FAIL;
				sprintf(mess,"Cannot start Receiver as it is in %s state\n",runStatusType(s).c_str());
				FILE_LOG(logERROR) << mess;
			}else {
				ret=receiver->startReceiver(mess);
				if (ret == FAIL) {
					FILE_LOG(logERROR) << mess;
				}
			}
		}
	}

	return interface->Server_SendResult(true, ret, NULL, 0, mess);
}



int slsReceiverTCPIPInterface::stop_receiver(){
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
			if(receiver->getStatus() != IDLE)
				receiver->stopReceiver();
			enum runStatus s = receiver->getStatus();
			if (s == IDLE)
				ret = OK;
			else {
				ret = FAIL;
				sprintf(mess,"Could not stop receiver. It is in %s state\n",runStatusType(s).c_str());
				FILE_LOG(logERROR) << mess;
			}
		}
	}

	return interface->Server_SendResult(true, ret, NULL, 0, mess);
}






int slsReceiverTCPIPInterface::set_file_dir() {
	memset(mess, 0, sizeof(mess));
	char fPath[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, fPath, sizeof(fPath), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (strlen(fPath)) {
			receiver->setFilePath(fPath);
		}
		// get
		retval = receiver->getFilePath();
		if ((!strlen(retval)) || (strlen(fPath) && strcasecmp(fPath, retval))) {
			ret = FAIL;
			strcpy(mess,"receiver file path does not exist\n");
			FILE_LOG(logERROR) << mess;
		} else
			FILE_LOG(logDEBUG1) << "file path:" << retval;
	}

    return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_file_name() {
	memset(mess, 0, sizeof(mess));
	char fName[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, fName, sizeof(fName), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (strlen(fName)) {
			receiver->setFileName(fName);
		}
		// get
		retval = receiver->getFileName();
		if (strlen(retval)) {
			ret = FAIL;
			strcpy(mess, "file name is empty\n");
			FILE_LOG(logERROR) << mess;
		} else
			FILE_LOG(logDEBUG1) << "file name:" << retval;
	}

	 return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_file_index() {
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setFileIndex(index);
			}
		}
		// get
		retval=receiver->getFileIndex();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set file index\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "file index:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval,sizeof(retval), mess);
}






int	slsReceiverTCPIPInterface::get_frame_index(){
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	if (ret == OK)
		retval=receiver->getAcquisitionIndex();

	return interface->Server_SendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_frames_caught(){
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	if (ret == OK)
		retval=receiver->getTotalFramesCaught();

	return interface->Server_SendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::reset_frames_caught(){
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
			receiver->resetAcquisitionCount();
		}
	}

	return interface->Server_SendResult(true, ret, NULL, 0, mess);
}



int slsReceiverTCPIPInterface::enable_file_write(){
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &enable, sizeof(enable), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setFileWriteEnable(enable);
			}
		}
		// get
		retval=receiver->getFileWriteEnable();
		if(enable >= 0 && enable != retval) {
			ret=FAIL;
			strcpy(mess,"Could not set file write enable");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "file write enable:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}





int slsReceiverTCPIPInterface::enable_overwrite() {
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setOverwriteEnable(index);
			}
		}
		// get
		retval=receiver->getOverwriteEnable();
		if(index >=0 && retval != index) {
			ret = FAIL;
			strcpy(mess,"Could not set file over write enable\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "file overwrite enable:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::enable_tengiga() {
	memset(mess, 0, sizeof(mess));
	int val = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &val, sizeof(val), true, receiver) == FAIL)
		return FAIL;

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (val >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				ret = receiver->setTenGigaEnable(val);
			}
		}
		// get
		retval=receiver->getTenGigaEnable();
		if((val >= 0) && (val != retval)) {
			ret = FAIL;
			strcpy(mess,"Could not set ten giga enable");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "10Gbe:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}




int slsReceiverTCPIPInterface::set_fifo_depth() {
	memset(mess, 0, sizeof(mess));
	int value = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &value, sizeof(value), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (value >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				ret = receiver->setFifoDepth(value);
				if (ret == FAIL) {
					strcpy(mess,"Could not set fifo depth");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		// get
		retval = receiver->getFifoDepth();
		if(value >= 0 && retval != value) {
			ret = FAIL;
			strcpy(mess, "Could not set fifo depth\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "fifo depth:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_activate() {
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &enable, sizeof(enable), true, receiver) == FAIL)
		return FAIL;

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setActivate(enable > 0 ? true : false);
			}
		}
		// get
		retval = (int)receiver->getActivate();
		if(enable >= 0 && retval != enable){
			ret = FAIL;
			sprintf(mess,"Could not set activate to %d, returned %d\n",enable,retval);
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "Activate: " << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_data_stream_enable(){
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				ret = receiver->setDataStreamEnable(index);
			}
		}
		// get
		retval = receiver->getDataStreamEnable();
		if(index >= 0 && retval != index){
			ret = FAIL;
			strcpy(mess,"Could not set data stream enable");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "data streaming enable:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_timer(){
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setStreamingTimer(index);
			}
		}
		// get
		retval=receiver->getStreamingTimer();
		if(index >= 0 && retval != index){
			ret = FAIL;
			strcpy(mess,"Could not set datastream timer");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "Streaming timer:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_flipped_data(){
	memset(mess, 0, sizeof(mess));
	int args[2] = {0,-1};
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, args, sizeof(args), true, receiver) == FAIL)
		return FAIL;

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (args[1] >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setFlippedData(args[0],args[1]);
			}
		}
		// get
		retval=receiver->getFlippedData(args[0]);
		if (args[1] > -1 && retval != args[1]) {
			ret = FAIL;
			strcpy(mess, "Could not set flipped data\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "Flipped Data:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}




int slsReceiverTCPIPInterface::set_file_format() {
	memset(mess, 0, sizeof(mess));
	fileFormat retval = GET_FILE_FORMAT;
	fileFormat f = GET_FILE_FORMAT;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &f, sizeof(f), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (f >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setFileFormat(f);
			}
		}
		// get
		retval = receiver->getFileFormat();
		if(f >= 0 && retval != f){
			ret = FAIL;
			sprintf(mess,"Could not set file format to %s, returned %s\n",
					getFileFormatType(f).c_str(),getFileFormatType(retval).c_str());
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "File Format: " << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_detector_posid() {
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &arg, sizeof(arg), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (arg >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setDetectorPositionId(arg);
			}
		}
		// get
		retval=receiver->getDetectorPositionId();
		if (arg >= 0 && retval != arg) {
			ret = FAIL;
			strcpy(mess,"Could not set detector position id");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "Position Id:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}






int slsReceiverTCPIPInterface::set_multi_detector_size() {
	memset(mess, 0, sizeof(mess));
	int arg[2] = {-1, -1};
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, arg, sizeof(arg), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if((arg[0] > 0) && (arg[1] > 0)) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setMultiDetectorSize(arg);
			}
		}
		// get
		int* temp = receiver->getMultiDetectorSize();
		for (int i = 0; i < MAX_DIMENSIONS; ++i) {
			if (!i)
				retval = temp[i];
			else
				retval *= temp[i];
		}
		FILE_LOG(logDEBUG1) << "Multi Detector Size:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}




int slsReceiverTCPIPInterface::set_streaming_port() {
	memset(mess, 0, sizeof(mess));
	int port = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &port, sizeof(port), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (port >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setStreamingPort(port);
			}
		}
		// get
		retval=receiver->getStreamingPort();
		FILE_LOG(logDEBUG1) << "streaming port:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}





int slsReceiverTCPIPInterface::set_streaming_source_ip() {
	memset(mess, 0, sizeof(mess));
	char arg[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, arg, MAX_STR_LENGTH, true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
			receiver->setStreamingSourceIP(arg);
		}
		// get
		retval = receiver->getStreamingSourceIP();
		FILE_LOG(logDEBUG1) << "streaming source ip:" << retval;
	}

	return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}







int slsReceiverTCPIPInterface::set_silent_mode() {
	memset(mess, 0, sizeof(mess));
	int value = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &value, sizeof(value), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (value >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setSilentMode(value);
			}
		}
		// get
		retval = (int)receiver->getSilentMode();
		FILE_LOG(logDEBUG1) << "silent mode:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}






int slsReceiverTCPIPInterface::enable_gap_pixels() {
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &enable, sizeof(enable), true, receiver) == FAIL)
		return FAIL;

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setGapPixelsEnable(enable);
			}
		}
		// get
		retval = receiver->getGapPixelsEnable();
		if(enable >= 0 && retval != enable){
			ret = FAIL;
			sprintf(mess,"Could not set gap pixels to %d, returned %d\n",enable,retval);
			FILE_LOG(logERROR) << "Warning: " << mess;
		}
		FILE_LOG(logDEBUG1) << "Gap Pixels Enable: " << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}




int slsReceiverTCPIPInterface::restream_stop(){
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
			if (receiver->getDataStreamEnable() == false) {
				ret = FAIL;
				sprintf(mess,"Could not restream stop packet as data Streaming is disabled.\n");
				FILE_LOG(logERROR) << mess;
			} else {
				ret = receiver->restreamStop();
				if (ret == FAIL) {
					sprintf(mess,"Could not restream stop packet.\n");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
	}

	return interface->Server_SendResult(true, ret, NULL, 0, mess);
}



int slsReceiverTCPIPInterface::set_additional_json_header() {
    memset(mess, 0, sizeof(mess));
    char arg[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, arg, sizeof(arg), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
			receiver->setAdditionalJsonHeader(arg);
		}
		// get
		retval = receiver->getAdditionalJsonHeader();
		FILE_LOG(logDEBUG1) << "additional json header:" << retval;
	}

	 return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_udp_socket_buffer_size() {
    memset(mess, 0, sizeof(mess));
    int index = -1;
    int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
                if (receiver->setUDPSocketBufferSize(index) == FAIL) {
                    ret = FAIL;
                    strcpy(mess, "Could not create dummy UDP Socket to test buffer size\n");
                    FILE_LOG(logERROR) << mess;
                }
			}
		}
		// get
        retval=receiver->getUDPSocketBufferSize();
        if(index >= 0 && ((retval != index) || ((int)receiver->getActualUDPSocketBufferSize() != (index*2)))) {
            ret = FAIL;
            strcpy(mess, "Could not set UDP Socket buffer size (No CAP_NET_ADMIN privileges?)\n");
            FILE_LOG(logERROR) << mess;
        }
        FILE_LOG(logDEBUG1) << "UDP Socket Buffer Size:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size(){
	memset(mess, 0, sizeof(mess));
    int retval = -1;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, NULL, 0, true, receiver);

	if (ret == OK)
		retval = receiver->getActualUDPSocketBufferSize();

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_frames_per_file() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setFramesPerFile(index);
			}
		}
		// get
		retval=receiver->getFramesPerFile();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set frames per file\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "frames per file:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}






int slsReceiverTCPIPInterface::check_version_compatibility() {
	memset(mess, 0, sizeof(mess));
	int64_t arg = -1;

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, &arg, sizeof(arg)) == FAIL)
		return FAIL;
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
	return interface->Server_SendResult(true, ret, NULL, 0, mess);
}




int slsReceiverTCPIPInterface::set_discard_policy() {
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setFrameDiscardPolicy((frameDiscardPolicy)index);
			}
		}
		// get
		retval=receiver->getFrameDiscardPolicy();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set frame discard policy\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "frame discard policy:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}




int slsReceiverTCPIPInterface::set_padding_enable() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				index = (index == 0) ? 0 : 1;
				receiver->setFramePaddingEnable(index);
			}
		}
		// get
		retval=(int)receiver->getFramePaddingEnable();
		if(index >= 0 && retval != index) {
			ret = FAIL;
			strcpy(mess, "Could not set frame padding enable\n");
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "Frame Padding Enable:" << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}




int slsReceiverTCPIPInterface::set_deactivated_receiver_padding_enable() {
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &enable, sizeof(enable), true, receiver) == FAIL)
		return FAIL;

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				receiver->setDeactivatedPadding(enable > 0 ? true : false);
			}
		}
		// get
		retval = (int)receiver->getDeactivatedPadding();
		if(enable >= 0 && retval != enable){
			ret = FAIL;
			sprintf(mess,"Could not set deactivated padding enable to %d, returned %d\n",enable,retval);
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "Deactivated Padding Enable: " << retval;
	}

	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}
