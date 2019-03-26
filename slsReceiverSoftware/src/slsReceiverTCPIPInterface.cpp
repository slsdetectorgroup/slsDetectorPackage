/********************************************//**
 * @file slsReceiverTCPIPInterface.cpp
 * @short interface between receiver and client
 ***********************************************/

#include "slsReceiverTCPIPInterface.h"
#include "slsReceiverImplementation.h"
#include "MySocketTCP.h"
#include "ServerInterface.h"
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
		mySock=nullptr;
	}
	if (interface)
		delete interface;
	if(receiver)
		delete receiver;
}

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int pn):
				myDetectorType(GOTTHARD),
				receiver(nullptr),
				ret(OK),
				fnum(-1),
				lockStatus(0),
				killTCPServerThread(0),
				tcpThreadCreated(false),
				portNumber(DEFAULT_PORTNO+2),
				mySock(nullptr),
				interface(nullptr)
{
	//***callback parameters***
	startAcquisitionCallBack = nullptr;
	pStartAcquisition = nullptr;
	acquisitionFinishedCallBack = nullptr;
	pAcquisitionFinished = nullptr;
	rawDataReadyCallBack = nullptr;
	rawDataModifyReadyCallBack = nullptr;
	pRawDataReady = nullptr;

	// create socket
	portNumber = (pn > 0 ? pn : DEFAULT_PORTNO + 2);
	MySocketTCP* m = new MySocketTCP(portNumber);
	mySock = m;
	interface = new ServerInterface(mySock, -1, "Receiver");

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
	if(pthread_create(&TCPServer_thread, nullptr,startTCPServerThread, (void*) this)){
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
		pthread_join(TCPServer_thread, nullptr);
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
	FILE_LOG(logINFOBLUE) << "Created [ TCP server Tid: " << syscall(SYS_gettid) << "]";;
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
			FILE_LOG(logINFOBLUE) << "Exiting [ TCP server Tid: " << syscall(SYS_gettid) <<"]";
			pthread_exit(nullptr);
		}

		//if user entered exit
		if(killTCPServerThread) {
			if (ret != GOODBYE) {
				if(receiver){
					receiver->shutDownUDPSockets();
				}
			}
			FILE_LOG(logINFOBLUE) << "Exiting [ TCP server Tid: " << syscall(SYS_gettid) <<"]";
			pthread_exit(nullptr);
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
	flist[F_GET_ADDITIONAL_JSON_HEADER]     =   &slsReceiverTCPIPInterface::get_additional_json_header;
    flist[F_RECEIVER_UDP_SOCK_BUF_SIZE]     =   &slsReceiverTCPIPInterface::set_udp_socket_buffer_size;
    flist[F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE]=   &slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size;
    flist[F_SET_RECEIVER_FRAMES_PER_FILE]	=   &slsReceiverTCPIPInterface::set_frames_per_file;
    flist[F_RECEIVER_CHECK_VERSION]			=   &slsReceiverTCPIPInterface::check_version_compatibility;
    flist[F_RECEIVER_DISCARD_POLICY]		=   &slsReceiverTCPIPInterface::set_discard_policy;
	flist[F_RECEIVER_PADDING_ENABLE]		=   &slsReceiverTCPIPInterface::set_padding_enable;
	flist[F_RECEIVER_DEACTIVATED_PADDING_ENABLE] = &slsReceiverTCPIPInterface::set_deactivated_receiver_padding_enable;
	flist[F_RECEIVER_SET_READOUT_FLAGS] 	= 	&slsReceiverTCPIPInterface::set_readout_flags;

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
		FILE_LOG(logDEBUG3) << "Could not read socket. "
				"Received " << n << " bytes," <<
				"fnum:" << fnum << " "
				"(" << getFunctionNameFromEnum((enum detFuncs)fnum) << ")";
		return FAIL;
	}
	else
		FILE_LOG(logDEBUG3) << "Received " << n << " bytes";

	if (fnum <= NUM_DET_FUNCTIONS || fnum >= NUM_REC_FUNCTIONS) {
		FILE_LOG(logERROR) << "Unknown function enum " << fnum;
		ret = (this->M_nofunc)();
	} else{
		FILE_LOG(logDEBUG1) <<  "calling function fnum: "<< fnum << " "
				"(" << getFunctionNameFromEnum((enum detFuncs)fnum) << ") "
				"located at " << flist[fnum];
		ret = (this->*flist[fnum])();

		if (ret == FAIL) {
			FILE_LOG(logDEBUG1) << "Failed to execute function = " << fnum << " ("
					<< getFunctionNameFromEnum((enum detFuncs)fnum) << ")";
		} else FILE_LOG(logDEBUG1) << "Function " <<
		getFunctionNameFromEnum((enum detFuncs)fnum) << " executed OK";
	}
	return ret;
}


void slsReceiverTCPIPInterface::functionNotImplemented() {
	ret = FAIL;
	sprintf(mess, "Function (%s) is not implemented for this detector\n",
			getFunctionNameFromEnum((enum detFuncs)fnum));
	FILE_LOG(logERROR) << mess;
}

void slsReceiverTCPIPInterface::modeNotImplemented(std::string modename, int mode) {
	ret = FAIL;
	sprintf(mess, "%s (%d) is not implemented for this detector\n", modename.c_str(), mode);
	FILE_LOG(logERROR) << mess;
}

template <typename T>
void slsReceiverTCPIPInterface::validate(T arg, T retval, std::string modename, numberMode hex) {
	if (ret == OK && arg != -1 && retval != arg) {
		ret = FAIL;
		if (hex)
			sprintf(mess, "Could not %s. Set 0x%x, but read 0x%x\n",
				modename.c_str(), (unsigned int) arg, (unsigned int) retval);
		else
			sprintf(mess, "Could not %s. Set %d, but read %d\n",
				modename.c_str(), (unsigned int) arg, (unsigned int) retval);
		FILE_LOG(logERROR) << mess;
	}
}

int slsReceiverTCPIPInterface::M_nofunc(){
	ret = FAIL;
	memset(mess, 0, sizeof(mess));

	// to receive any arguments
	int n = 1;
	while (n > 0)
		n = mySock->ReceiveDataOnly(mess, MAX_STR_LENGTH);

	sprintf(mess,"Unrecognized Function enum %d. Please do not proceed.\n", fnum);
	FILE_LOG(logERROR) << mess;
	return interface->Server_SendResult(false, ret, nullptr, 0, mess);
}




int slsReceiverTCPIPInterface::exec_command() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char cmd[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, cmd, MAX_STR_LENGTH) == FAIL)
		return FAIL;
	FILE_LOG(logINFO) << "Executing command (" << cmd << ")";

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
				if (fgets(temp.data(), tempsize, pipe.get()) != nullptr)
					sresult += temp.data();
			}
			strncpy(retval, sresult.c_str(), MAX_STR_LENGTH);
			ret = OK;
			FILE_LOG(logINFO) << "Result of cmd (" << cmd << "):\n" << retval;
		}
	}
	return interface->Server_SendResult(false, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::exit_server() {
	FILE_LOG(logINFO) << "Closing server";
	ret = OK;
	memset(mess, 0, sizeof(mess));
	interface->Server_SendResult(false, ret, nullptr, 0);
	return GOODBYE;
}



int slsReceiverTCPIPInterface::lock_receiver() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int lock = 0;

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, &lock, sizeof(lock)) == FAIL)
		return FAIL;
	FILE_LOG(logDEBUG1) << "Locking Server to " << lock;

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
	MySocketTCP* mySocket = nullptr;
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
			} catch(SocketError &e) {
				ret = FAIL;
				// same socket, could not bind port
				sprintf(mess, "%s", e.what());
				FILE_LOG(logERROR) << mess;
			} catch (...) {
				ret = FAIL;
				sprintf(mess, "Could not set port %d.\n", p_number);
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
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	interface->Server_SendResult(false, ret, nullptr, 0, mess);

	if (ret == FAIL)
		return ret;

	// update
	return send_update();
}



int slsReceiverTCPIPInterface::send_update() {
	int n = 0;
	int i32 = -1;
	char cstring[MAX_STR_LENGTH] = {0};


	n += mySock->SendDataOnly(mySock->lastClientIP,sizeof(mySock->lastClientIP));

	// filepath
	strcpy(cstring, receiver->getFilePath().c_str());
    n += mySock->SendDataOnly(cstring, sizeof(cstring));

	// filename
    strcpy(cstring, receiver->getFileName().c_str());
    n += mySock->SendDataOnly(cstring, sizeof(cstring));

	// index
	i32=receiver->getFileIndex();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	//file format
	i32=(int)receiver->getFileFormat();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	//frames per file
	i32=(int)receiver->getFramesPerFile();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	//frame discard policy
	i32=(int)receiver->getFrameDiscardPolicy();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	//frame padding
	i32=(int)receiver->getFramePaddingEnable();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// file write enable
	i32=(int)receiver->getFileWriteEnable();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// file overwrite enable
	i32=(int)receiver->getOverwriteEnable();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// gap pixels
	i32=(int)receiver->getGapPixelsEnable();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// streaming frequency
	i32=(int)receiver->getStreamingFrequency();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// streaming port
	i32=(int)receiver->getStreamingPort();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// streaming source ip
    strcpy(cstring, receiver->getStreamingSourceIP().c_str());
    n += mySock->SendDataOnly(cstring, sizeof(cstring));

    // additional json header
    strcpy(cstring, receiver->getAdditionalJsonHeader().c_str());
    n += mySock->SendDataOnly(cstring, sizeof(cstring));

	// data streaming enable
	i32=(int)receiver->getDataStreamEnable();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// activate
	i32=(int)receiver->getActivate();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// deactivated padding enable
	i32=(int)receiver->getDeactivatedPadding();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	// silent mode
	i32=(int)receiver->getSilentMode();
	n += mySock->SendDataOnly(&i32, sizeof(i32));

	if (!lockStatus)
		strcpy(mySock->lastClientIP, mySock->thisClientIP);

	return OK;
}



int slsReceiverTCPIPInterface::get_id(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t retval = getReceiverVersion();
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval));
}



int slsReceiverTCPIPInterface::set_detector_type(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	detectorType arg = GENERIC;
	detectorType retval = GENERIC;

	// get args, return if socket crashed
	if (interface->Server_ReceiveArg(ret, mess, &arg, sizeof(arg)) == FAIL)
		return FAIL;

	// set
	if (arg >= 0) {
		// if object exists, verify unlocked and idle, else only verify lock (connecting first time)
		if (receiver == nullptr)
			interface->Server_VerifyLock(ret, mess, lockStatus);
		else
			interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum);
		if (ret == OK) {
			switch(arg) {
			case GOTTHARD:
			case EIGER:
			case CHIPTESTBOARD:
			case MOENCH:
			case JUNGFRAU:
				break;
			default:
				ret = FAIL;
				sprintf(mess,"Unknown detector type: %d\n", arg);
				FILE_LOG(logERROR) << mess;
				break;
			}
			if(ret == OK) {
				if(receiver == nullptr){
					receiver = new slsReceiverImplementation();
				}
				myDetectorType = arg;
				ret = receiver->setDetectorType(myDetectorType);
				retval = myDetectorType;

				// callbacks after (in setdetectortype, the object is reinitialized)
				if(startAcquisitionCallBack)
					receiver->registerCallBackStartAcquisition(startAcquisitionCallBack,pStartAcquisition);
				if(acquisitionFinishedCallBack)
					receiver->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,pAcquisitionFinished);
				if(rawDataReadyCallBack)
					receiver->registerCallBackRawDataReady(rawDataReadyCallBack,pRawDataReady);
				if(rawDataModifyReadyCallBack)
					receiver->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,pRawDataReady);

				// client has started updating receiver, update ip
				if (!lockStatus)
					strcpy(mySock->lastClientIP, mySock->thisClientIP);
			}

		}
	}
	//get
	retval = myDetectorType;
	validate((int)arg, (int)retval, std::string("set detector type"), DEC);
	return interface->Server_SendResult(false, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_detector_hostname() {
	ret = OK;
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
		std::string s = receiver->getDetectorHostname();
		strcpy(retval, s.c_str());
		if (!s.length()) {
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
	int narg = -1;
	std::vector <ROI> arg;

	// receive arguments
	if (mySock->ReceiveDataOnly(&narg,sizeof(narg)) < 0 )
		return interface->Server_SocketCrash();
	for (int iloop = 0; iloop < narg; ++iloop) {
		ROI temp;
		if ( mySock->ReceiveDataOnly(&temp.xmin, sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		if ( mySock->ReceiveDataOnly(&temp.xmax, sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		if ( mySock->ReceiveDataOnly(&temp.ymin, sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		if ( mySock->ReceiveDataOnly(&temp.ymax, sizeof(int)) < 0 )
			return interface->Server_SocketCrash();
		arg.push_back(temp);
	}
	FILE_LOG(logDEBUG1) << "Set ROI narg: " << narg;
	for (int iloop = 0; iloop < narg; ++iloop) {
		FILE_LOG(logDEBUG1) << "(" << arg[iloop].xmin << ", " <<
				arg[iloop].xmax << ", " << arg[iloop].ymin << ", " <<
				arg[iloop].ymax << ")";
	}

	if (myDetectorType == EIGER || myDetectorType == JUNGFRAU)
		functionNotImplemented();

	// base object not null
	else if (receiver == nullptr)
		interface->Server_NullObjectError(ret, mess);
	else {
		// only set
		// verify if receiver is unlocked and idle
		if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK)
			ret = receiver->setROI(arg);
	}
	arg.clear();
	return interface->Server_SendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::setup_udp(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char args[6][MAX_STR_LENGTH] = {{""}, {""}, {""}, {""}, {""}, {""}};
	char retvals[2][MAX_STR_LENGTH] = {{""}, {""}};

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, args, sizeof(args), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {

			//setup interfaces count
			int numInterfaces = atoi(args[0]) > 1 ? 2 : 1;
			int selInterface = atoi(args[1]) > 1 ? 2 : 1;

			char* ip1 = args[2];
			char* ip2 = args[3];
			uint32_t port1 = atoi(args[4]);
			uint32_t port2 = atoi(args[5]);

			// using the 2nd interface only
			if (numInterfaces == 1 && selInterface == 2) {
				ip1 = ip2;
				port1 = port2;
			}

			// 1st interface
			receiver->setUDPPortNumber(port1);
			FILE_LOG(logINFO) << "Receiver UDP IP: " << ip1;
			// get eth
			std::string temp = genericSocket::ipToName(ip1);
			if (temp == "none"){
				ret = FAIL;
				strcpy(mess, "Failed to get ethernet interface or IP \n");
				FILE_LOG(logERROR) << mess;
			} else {
				char eth[MAX_STR_LENGTH] = {""};
				memset(eth, 0, MAX_STR_LENGTH);
				strcpy(eth, temp.c_str());
				// if there is a dot in eth name
				if (strchr(eth, '.') != nullptr) {
					strcpy(eth, "");
					ret = FAIL;
					sprintf(mess, "Failed to get ethernet interface from IP. Got %s\n", temp.c_str());
					FILE_LOG(logERROR) << mess;
				}
				receiver->setEthernetInterface(eth);

				//get mac address
				if (ret != FAIL) {
					temp = genericSocket::nameToMac(eth);
					if (temp=="00:00:00:00:00:00") {
						ret = FAIL;
						strcpy(mess,"failed to get mac adddress to listen to\n");
						FILE_LOG(logERROR) << mess;
					} else {
						// using the 2nd interface only
						if (numInterfaces == 1 && selInterface == 2) {
							strcpy(retvals[1],temp.c_str());
							FILE_LOG(logINFO) << "Receiver MAC Address: " << retvals[1];
						}
						else {
							strcpy(retvals[0],temp.c_str());
							FILE_LOG(logINFO) << "Receiver MAC Address: " << retvals[0];
						}
					}
				}
			}

			// 2nd interface
			if (numInterfaces == 2) {
				receiver->setUDPPortNumber2(port2);
				FILE_LOG(logINFO) << "Receiver UDP IP 2: " << ip2;
				// get eth
				std::string temp = genericSocket::ipToName(ip2);
				if (temp == "none"){
					ret = FAIL;
					strcpy(mess, "Failed to get 2nd ethernet interface or IP \n");
					FILE_LOG(logERROR) << mess;
				} else {
					char eth[MAX_STR_LENGTH] = {""};
					memset(eth, 0, MAX_STR_LENGTH);
					strcpy(eth, temp.c_str());
					// if there is a dot in eth name
					if (strchr(eth, '.') != nullptr) {
						strcpy(eth, "");
						ret = FAIL;
						sprintf(mess, "Failed to get 2nd ethernet interface from IP. Got %s\n", temp.c_str());
						FILE_LOG(logERROR) << mess;
					}
					receiver->setEthernetInterface2(eth);

					//get mac address
					if (ret != FAIL) {
						temp = genericSocket::nameToMac(eth);
						if (temp=="00:00:00:00:00:00") {
							ret = FAIL;
							strcpy(mess,"failed to get 2nd mac adddress to listen to\n");
							FILE_LOG(logERROR) << mess;
						} else {
							strcpy(retvals[1],temp.c_str());
							FILE_LOG(logINFO) << "Receiver MAC Address 2: " << retvals[1];
						}
					}
				}
			}

			// set the number of udp interfaces (changes number of threads and many others)
			if (receiver->setNumberofUDPInterfaces(numInterfaces) == FAIL) {
				ret = FAIL;
				sprintf(mess, "Failed to set number of interfaces\n");
				FILE_LOG(logERROR) << mess;
			}
		}
	}
	return interface->Server_SendResult(true, ret, retvals, sizeof(retvals), mess);
}



int slsReceiverTCPIPInterface::set_timer() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t index[2] = {-1, -1};
	int64_t retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Setting timer index " << index[0] << " to " << index[1];

		// set
		if (index[1] >= 0) {
			// verify if receiver is unlocked
			if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
				switch (index[0]) {
				case ACQUISITION_TIME:
						ret = receiver->setAcquisitionTime(index[1]);
					break;
				case FRAME_PERIOD:
					ret = receiver->setAcquisitionPeriod(index[1]);
					break;
				case FRAME_NUMBER:
				case CYCLES_NUMBER:
				case STORAGE_CELL_NUMBER:
					receiver->setNumberOfFrames(index[1]);
					break;
				case SUBFRAME_ACQUISITION_TIME:
					receiver->setSubExpTime(index[1]);
					break;
				case SUBFRAME_DEADTIME:
					receiver->setSubPeriod(index[1] + receiver->getSubExpTime());
					break;
				case SAMPLES:
					if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
						modeNotImplemented("(Samples) Timer index", (int)index[0]);
						break;
					}
					receiver->setNumberofSamples(index[1]);
					break;
				default:
					modeNotImplemented("Timer index", (int)index[0]);
					break;
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
		case SAMPLES:
			if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
				ret = FAIL;
				sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
				FILE_LOG(logERROR) << "Warning: " << mess;
				break;
			}
			retval=receiver->getNumberofSamples();
			break;
		default:
			modeNotImplemented("Timer index", (int)index[0]);
			break;
		}
		validate((int)index[1], (int)retval, std::string("set timer"), DEC);
		FILE_LOG(logDEBUG1) << slsDetectorDefs::getTimerType((timerIndex)(index[0])) << ":" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_dynamic_range() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting dynamic range: " << dr;
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
					modeNotImplemented("Dynamic range", dr);
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
		validate(dr, retval, std::string("set dynamic range"), DEC);
		FILE_LOG(logDEBUG1) << "dynamic range: " << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_frequency() {
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
				FILE_LOG(logDEBUG1) << "Setting streaming frequency: " << index;
				ret = receiver->setStreamingFrequency(index);
				if(ret == FAIL) {
					strcpy(mess, "Could not allocate memory for listening fifo\n");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		// get
		retval = receiver->getStreamingFrequency();
		validate(index, retval, std::string("set streaming frequency"), DEC);
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_status(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum runStatus retval = ERROR;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting Status";
		retval = receiver->getStatus();
		FILE_LOG(logDEBUG1) << "Status:" << runStatusType(retval);
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::start_receiver(){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

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
				FILE_LOG(logDEBUG1) << "Starting Receiver";
				ret = receiver->startReceiver(mess);
				if (ret == FAIL) {
					FILE_LOG(logERROR) << mess;
				}
			}
		}
	}
	return interface->Server_SendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::stop_receiver(){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
			if(receiver->getStatus() != IDLE) {
				FILE_LOG(logDEBUG1) << "Stopping Receiver";
				receiver->stopReceiver();
			}
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
	return interface->Server_SendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::set_file_dir() {
	ret = OK;
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
			FILE_LOG(logDEBUG1) << "Setting file path: " << fPath;
			receiver->setFilePath(fPath);
		}
		// get
		std::string s = receiver->getFilePath();
		strcpy(retval, s.c_str());
		if ((!s.length()) || (strlen(fPath) && strcasecmp(fPath, retval))) {
			ret = FAIL;
			strcpy(mess,"receiver file path does not exist\n");
			FILE_LOG(logERROR) << mess;
		} else
			FILE_LOG(logDEBUG1) << "file path:" << retval;
	}
    return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_file_name() {
	ret = OK;
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
			FILE_LOG(logDEBUG1) << "Setting file name: " << fName;
			receiver->setFileName(fName);
		}
		// get
		std::string s = receiver->getFileName();
		strcpy(retval, s.c_str());
		if (!s.length()) {
			ret = FAIL;
			strcpy(mess, "file name is empty\n");
			FILE_LOG(logERROR) << mess;
		} else
			FILE_LOG(logDEBUG1) << "file name:" << retval;
	}
	 return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_file_index() {
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
				FILE_LOG(logDEBUG1) << "Setting file index: " << index;
				receiver->setFileIndex(index);
			}
		}
		// get
		retval=receiver->getFileIndex();
		validate(index, retval, std::string("set file index"), DEC);
		FILE_LOG(logDEBUG1) << "file index:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_frame_index(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting frame index";
		retval = receiver->getAcquisitionIndex();
		FILE_LOG(logDEBUG1) << "frame index:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_frames_caught(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting frames caught";
		retval = receiver->getTotalFramesCaught();
		FILE_LOG(logDEBUG1) << "frames caught:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::reset_frames_caught(){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (interface->Server_VerifyLock(ret, mess, lockStatus) == OK) {
			FILE_LOG(logDEBUG1) << "Reset frames caught";
			receiver->resetAcquisitionCount();
		}
	}
	return interface->Server_SendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::enable_file_write(){
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting File write enable:" << enable;
				receiver->setFileWriteEnable(enable);
			}
		}
		// get
		retval = receiver->getFileWriteEnable();
		validate(enable, retval, std::string("set file write enable"), DEC);
		FILE_LOG(logDEBUG1) << "file write enable:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::enable_overwrite() {
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
				FILE_LOG(logDEBUG1) << "Setting File overwrite enable:" << index;
				receiver->setOverwriteEnable(index);
			}
		}
		// get
		retval = receiver->getOverwriteEnable();
		validate(index, retval, std::string("set file overwrite enable"), DEC);
		FILE_LOG(logDEBUG1) << "file overwrite enable:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::enable_tengiga() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int val = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &val, sizeof(val), true, receiver) == FAIL)
		return FAIL;

	if (myDetectorType != EIGER && myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (val >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting 10GbE:" << val;
				ret = receiver->setTenGigaEnable(val);
			}
		}
		// get
		retval = receiver->getTenGigaEnable();
		validate(val, retval, std::string("set 10GbE"), DEC);
		FILE_LOG(logDEBUG1) << "10Gbe:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_fifo_depth() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting fifo depth:" << value;
				ret = receiver->setFifoDepth(value);
				if (ret == FAIL) {
					strcpy(mess,"Could not set fifo depth");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
		// get
		retval = receiver->getFifoDepth();
		validate(value, retval, std::string("set fifo depth"), DEC);
		FILE_LOG(logDEBUG1) << "fifo depth:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_activate() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting activate:" << enable;
				receiver->setActivate(enable > 0 ? true : false);
			}
		}
		// get
		retval = (int)receiver->getActivate();
		validate(enable, retval, std::string("set activate"), DEC);
		FILE_LOG(logDEBUG1) << "Activate: " << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_data_stream_enable(){
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
				FILE_LOG(logDEBUG1) << "Setting data stream enable:" << index;
				ret = receiver->setDataStreamEnable(index);
			}
		}
		// get
		retval = receiver->getDataStreamEnable();
		validate(index, retval, std::string("set data stream enable"), DEC);
		FILE_LOG(logDEBUG1) << "data streaming enable:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_timer(){
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
				FILE_LOG(logDEBUG1) << "Setting streaming timer:" << index;
				receiver->setStreamingTimer(index);
			}
		}
		// get
		retval=receiver->getStreamingTimer();
		validate(index, retval, std::string("set data stream timer"), DEC);
		FILE_LOG(logDEBUG1) << "Streaming timer:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_flipped_data(){
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting flipped data:" << args[1];
				receiver->setFlippedData(args[0],args[1]);
			}
		}
		// get
		retval=receiver->getFlippedData(args[0]);
		validate(args[1], retval, std::string("set flipped data"), DEC);
		FILE_LOG(logDEBUG1) << "Flipped Data:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_file_format() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting file format:" << f;
				receiver->setFileFormat(f);
			}
		}
		// get
		retval = receiver->getFileFormat();
		validate(f, retval, std::string("set file format"), DEC);
		FILE_LOG(logDEBUG1) << "File Format: " << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_detector_posid() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting detector position id:" << arg;
				receiver->setDetectorPositionId(arg);
			}
		}
		// get
		retval = receiver->getDetectorPositionId();
		validate(arg, retval, std::string("set detector position id"), DEC);
		FILE_LOG(logDEBUG1) << "Position Id:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_multi_detector_size() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting multi detector size:" << arg[0] << "," << arg[1];
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
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting streaming port:" << port;
				receiver->setStreamingPort(port);
			}
		}
		// get
		retval = receiver->getStreamingPort();
		validate(port, retval, std::string("set streaming port"), DEC);
		FILE_LOG(logDEBUG1) << "streaming port:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_source_ip() {
	ret = OK;
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
			FILE_LOG(logDEBUG1) << "Setting streaming source ip:" << arg;
			receiver->setStreamingSourceIP(arg);
		}
		// get
		strcpy(retval, receiver->getStreamingSourceIP().c_str());
		FILE_LOG(logDEBUG1) << "streaming source ip:" << retval;
	}
	return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_silent_mode() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting silent mode:" << value;
				receiver->setSilentMode(value);
			}
		}
		// get
		retval = (int)receiver->getSilentMode();
		validate(value, retval, std::string("set silent mode"), DEC);
		FILE_LOG(logDEBUG1) << "silent mode:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::enable_gap_pixels() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting gap pixels enable:" << enable;
				receiver->setGapPixelsEnable(enable);
			}
		}
		// get
		retval = receiver->getGapPixelsEnable();
		validate(enable, retval, std::string("set gap pixels enable"), DEC);
		FILE_LOG(logDEBUG1) << "Gap Pixels Enable: " << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::restream_stop(){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

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
				FILE_LOG(logDEBUG1) << "Restreaming stop";
				ret = receiver->restreamStop();
				if (ret == FAIL) {
					sprintf(mess,"Could not restream stop packet.\n");
					FILE_LOG(logERROR) << mess;
				}
			}
		}
	}
	return interface->Server_SendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::set_additional_json_header() {
	ret = OK;
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
			FILE_LOG(logDEBUG1) << "Setting additional json header: " << arg;
			receiver->setAdditionalJsonHeader(arg);
		}
		// get
		strcpy(retval, receiver->getAdditionalJsonHeader().c_str());
		FILE_LOG(logDEBUG1) << "additional json header:" << retval;
	}
	 return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::get_additional_json_header() {
	ret = OK;
    memset(mess, 0, sizeof(mess));
    char retval[MAX_STR_LENGTH] = {0};

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	// base object not null
	if (ret == OK) {
		// get
		strcpy(retval, receiver->getAdditionalJsonHeader().c_str());
		FILE_LOG(logDEBUG1) << "additional json header:" << retval;
	}
	 return interface->Server_SendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_udp_socket_buffer_size() {
	ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t index = -1;
    int64_t retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &index, sizeof(index), true, receiver) == FAIL)
		return FAIL;

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting UDP Socket Buffer size: " << index;
				if (receiver->setUDPSocketBufferSize(index) == FAIL) {
                    ret = FAIL;
                    strcpy(mess, "Could not create dummy UDP Socket to test buffer size\n");
                    FILE_LOG(logERROR) << mess;
                }
			}
		}
		// get
        retval = receiver->getUDPSocketBufferSize();
        if (index != 0)
        	validate(index, retval, std::string("set udp socket buffer size (No CAP_NET_ADMIN privileges?)"), DEC);
        FILE_LOG(logDEBUG1) << "UDP Socket Buffer Size:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size(){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t retval = -1;

	// no arg, check receiver is null
	interface->Server_ReceiveArg(ret, mess, nullptr, 0, true, receiver);

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting actual UDP buffer size";
		retval = receiver->getActualUDPSocketBufferSize();
	}
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
				FILE_LOG(logDEBUG1) << "Setting frames per file: " << index;
				receiver->setFramesPerFile(index);
			}
		}
		// get
		retval = receiver->getFramesPerFile();
		validate(index, retval, std::string("set frames per file"), DEC);
		FILE_LOG(logDEBUG1) << "frames per file:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::check_version_compatibility() {
	ret = OK;
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
	return interface->Server_SendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::set_discard_policy() {
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
				FILE_LOG(logDEBUG1) << "Setting frames discard policy: " << index;
				receiver->setFrameDiscardPolicy((frameDiscardPolicy)index);
			}
		}
		// get
		retval = receiver->getFrameDiscardPolicy();
		validate(index, retval, std::string("set discard policy"), DEC);
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
				FILE_LOG(logDEBUG1) << "Setting frames padding enable: " << index;
				receiver->setFramePaddingEnable(index);
			}
		}
		// get
		retval = (int)receiver->getFramePaddingEnable();
		validate(index, retval, std::string("set frame padding enable"), DEC);
		FILE_LOG(logDEBUG1) << "Frame Padding Enable:" << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_deactivated_receiver_padding_enable() {
	ret = OK;
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
				FILE_LOG(logDEBUG1) << "Setting deactivated padding enable: " << enable;
				receiver->setDeactivatedPadding(enable > 0 ? true : false);
			}
		}
		// get
		retval = (int)receiver->getDeactivatedPadding();
		validate(enable, retval, std::string("set deactivated padding enable"), DEC);
		FILE_LOG(logDEBUG1) << "Deactivated Padding Enable: " << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}


int slsReceiverTCPIPInterface::set_readout_flags() {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	readOutFlags arg = GET_READOUT_FLAGS;
	readOutFlags retval = GET_READOUT_FLAGS;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (interface->Server_ReceiveArg(ret, mess, &arg, sizeof(arg), true, receiver) == FAIL)
		return FAIL;

	if (myDetectorType == JUNGFRAU || myDetectorType == GOTTHARD || myDetectorType == MOENCH)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (arg >= 0) {
			// verify if receiver is unlocked and idle
			if (interface->Server_VerifyLockAndIdle(ret, mess, lockStatus,	receiver->getStatus(), fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting readout flag: " << arg;
				ret = receiver->setReadOutFlags(arg);
			}
		}
		// get
		retval = receiver->getReadOutFlags();
		validate((int)arg, (int)(retval & arg), std::string("set readout flags"), HEX);
		FILE_LOG(logDEBUG1) << "Readout flags: " << retval;
	}
	return interface->Server_SendResult(true, ret, &retval, sizeof(retval), mess);
}
