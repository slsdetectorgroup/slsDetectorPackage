/********************************************//**
 * @file slsReceiverTCPIPInterface.cpp
 * @short interface between receiver and client
 ***********************************************/


#include "FixedCapacityContainer.h"
#include "ServerSocket.h"
#include "slsReceiver.h"
#include "slsReceiverImplementation.h"
#include "slsReceiverTCPIPInterface.h"
#include "slsReceiverUsers.h"
#include "versionAPI.h"
#include "string_utils.h"
#include "sls_detector_exceptions.h"

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <syscall.h>
#include <vector>

using sls::SocketError;

slsReceiverTCPIPInterface::~slsReceiverTCPIPInterface() {
	stop();
}

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int pn):
				myDetectorType(GOTTHARD),
				portNumber(pn > 0 ? pn : DEFAULT_PORTNO + 2)
{
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
	FILE_LOG(logDEBUG) << "TCP Server thread created successfully.";
	return OK;
}


void slsReceiverTCPIPInterface::stop(){
	if (tcpThreadCreated) {
		FILE_LOG(logINFO) << "Shutting down TCP Socket on port " << portNumber;
		killTCPServerThread = 1;
		if(server)
			server->shutDownSocket();
		FILE_LOG(logDEBUG) << "TCP Socket closed on port " << portNumber;
		pthread_join(TCPServer_thread, nullptr);
		tcpThreadCreated = false;
		killTCPServerThread = 0;
		FILE_LOG(logDEBUG) << "Exiting TCP Server Thread on port " << portNumber;
	}
}



int64_t slsReceiverTCPIPInterface::getReceiverVersion(){
	return APIRECEIVER;
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

void slsReceiverTCPIPInterface::startTCPServer() {
    FILE_LOG(logINFOBLUE) << "Created [ TCP server Tid: " 
						  << syscall(SYS_gettid) << "]";
    FILE_LOG(logINFO) << "SLS Receiver starting TCP Server on port "
                      << portNumber << std::endl;
    int ret = OK;
    server = sls::make_unique<sls::ServerSocket>(portNumber);
    while (true) {
        try {
            auto socket = server->accept();
			socket.setReceiveTimeout(static_cast<int>(5E6));
            ret = decode_function(socket);

            // if tcp command was to exit server
            if (ret == GOODBYE) {
                FILE_LOG(logINFO) << "Shutting down UDP Socket";
                if (receiver) {
                    receiver->shutDownUDPSockets();
                }
                FILE_LOG(logINFOBLUE)
                    << "Exiting [ TCP server Tid: " << syscall(SYS_gettid)
                    << "]";
                pthread_exit(nullptr);
            }
        }catch(const sls::SocketError& e){
			std::cout << "Accept failed\n";
		}

        // if user entered exit
        if (killTCPServerThread) {
            if (ret != GOODBYE) {
                if (receiver) {
                    receiver->shutDownUDPSockets();
                }
            }
            FILE_LOG(logINFOBLUE)
                << "Exiting [ TCP server Tid: " << syscall(SYS_gettid) << "]";
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
	flist[F_ENABLE_RECEIVER_MASTER_FILE_WRITE]	=	&slsReceiverTCPIPInterface::enable_master_file_write;
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
	flist[F_RECEIVER_DEACTIVATED_PADDING_ENABLE] = &slsReceiverTCPIPInterface::set_deactivated_padding_enable;
	flist[F_RECEIVER_SET_READOUT_FLAGS] 	= 	&slsReceiverTCPIPInterface::set_readout_flags;
	flist[F_RECEIVER_SET_ADC_MASK]			=	&slsReceiverTCPIPInterface::set_adc_mask;
	flist[F_SET_RECEIVER_DBIT_LIST]			=	&slsReceiverTCPIPInterface::set_dbit_list;
	flist[F_GET_RECEIVER_DBIT_LIST]			=	&slsReceiverTCPIPInterface::get_dbit_list;
	flist[F_RECEIVER_DBIT_OFFSET]			= 	&slsReceiverTCPIPInterface::set_dbit_offset;

	for (int i = NUM_DET_FUNCTIONS + 1; i < NUM_REC_FUNCTIONS ; i++) {
		FILE_LOG(logDEBUG1) << "function fnum: " << i << " (" <<
				getFunctionNameFromEnum((enum detFuncs)i) << ") located at " << flist[i];
	}

	return OK;
}

int slsReceiverTCPIPInterface::decode_function(sls::ServerInterface2 &socket) {
    ret = FAIL;
    int n = socket.read(&fnum, sizeof(fnum));
    if (n <= 0) {
        FILE_LOG(logDEBUG3)
            << "Could not read socket. Received " << n
            << " bytes, fnum:" << fnum << " ("
            << getFunctionNameFromEnum((enum detFuncs)fnum) << ")";
        return FAIL;
    } else {
        FILE_LOG(logDEBUG3) << "Received " << n << " bytes";
    }
    if (fnum <= NUM_DET_FUNCTIONS || fnum >= NUM_REC_FUNCTIONS) {
        FILE_LOG(logERROR) << "Unknown function enum " << fnum;
        ret = (this->M_nofunc)(socket);
    } else {
        FILE_LOG(logDEBUG1) << "calling function fnum: " << fnum << " ("
                            << getFunctionNameFromEnum((enum detFuncs)fnum)
                            << ") located at " << flist[fnum];
        ret = (this->*flist[fnum])(socket);
        if (ret == FAIL) {
            FILE_LOG(logDEBUG1)
                << "Failed to execute function = " << fnum << " ("
                << getFunctionNameFromEnum((enum detFuncs)fnum) << ")";
        } else
            FILE_LOG(logDEBUG1)
                << "Function " << getFunctionNameFromEnum((enum detFuncs)fnum)
                << " executed OK";
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

int slsReceiverTCPIPInterface::M_nofunc(sls::ServerInterface2 &socket){
	ret = FAIL;
	memset(mess, 0, sizeof(mess));

	int n = 1;
	while (n > 0)
		n = socket.read(mess, MAX_STR_LENGTH);

	sprintf(mess,"Unrecognized Function enum %d. Please do not proceed.\n", fnum);
	FILE_LOG(logERROR) << mess;
	return socket.sendResult(false, ret, nullptr, 0, mess);
}

int slsReceiverTCPIPInterface::VerifyLock(int &ret, char *mess) {
    if (server->getThisClient() != server->getLockedBy()  && lockStatus) {
        ret = FAIL;
        sprintf(mess, "Receiver locked\n");
        FILE_LOG(logERROR) << mess;
    }
    return ret;
}

int slsReceiverTCPIPInterface::VerifyLockAndIdle(int &ret, char *mess, int fnum) {
    VerifyLock(ret, mess);
    if (ret == FAIL)
        return ret;
    if (receiver->getStatus() != IDLE) {
        sprintf(mess, "Can not execute %s when receiver is not idle\n",
                getFunctionNameFromEnum((enum detFuncs)fnum));
    }
	ret = OK;
	return ret;
}

int slsReceiverTCPIPInterface::exec_command(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char cmd[MAX_STR_LENGTH]{};
	char retval[MAX_STR_LENGTH]{};

	// get args, return if socket crashed
	if (socket.receiveArg(ret, mess, cmd, MAX_STR_LENGTH) == FAIL)
		return FAIL;
	FILE_LOG(logINFO) << "Executing command (" << cmd << ")";

	// verify if receiver is unlocked
	if (VerifyLock(ret, mess) == OK) {
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
	return socket.sendResult(false, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::exit_server(sls::ServerInterface2 &socket) {
	FILE_LOG(logINFO) << "Closing server";
	ret = OK;
	memset(mess, 0, sizeof(mess));
	socket.sendResult(false, ret, nullptr, 0, nullptr);
	return GOODBYE;
}

int slsReceiverTCPIPInterface::lock_receiver(sls::ServerInterface2 &socket) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int lock = 0;

    // get args, return if socket crashed
    if (socket.receiveArg(ret, mess, &lock, sizeof(lock)) == FAIL)
        return FAIL;
    FILE_LOG(logDEBUG1) << "Locking Server to " << lock;

    // execute action
    if (lock >= 0) {
        if (!lockStatus || (server->getLockedBy() == server->getThisClient())) {
            lockStatus = lock;
            lock ? server->setLockedBy(server->getThisClient())
                 : server->setLockedBy(sls::IpAddr{});
        } else{
			sprintf(mess, "Receiver locked\n");
        	FILE_LOG(logERROR) << mess;
		}
            
    }
    return socket.sendResult(true, ret, &lockStatus,
                                        sizeof(lockStatus), mess);
}

int slsReceiverTCPIPInterface::get_last_client_ip(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char ip[INET_ADDRSTRLEN]{};
	sls::strcpy_safe(ip, server->getLastClient().str().c_str());
	return socket.sendResult(true, ret, &ip, sizeof(ip));
}



int slsReceiverTCPIPInterface::set_port(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int p_number = -1;
	if (socket.receiveArg(ret, mess, &p_number, sizeof(p_number)) == FAIL)
		return FAIL;

	if (VerifyLock(ret, mess) == OK) {
		if (p_number < 1024) {
			ret = FAIL;
			sprintf(mess,"Port Number (%d) too low\n", p_number);
			FILE_LOG(logERROR) << mess;
		} else {
			FILE_LOG(logINFO) << "set port to " << p_number <<std::endl;
			try {
				auto new_server = sls::make_unique<sls::ServerSocket>(p_number);
				new_server->setLockedBy(server->getLockedBy());
				new_server->setLastClient(server->getLastClient());
				server = std::move(new_server);
			} catch(SocketError &e) {
				ret = FAIL;
				sprintf(mess, "%s", e.what());
				FILE_LOG(logERROR) << mess;
			} catch (...) {
				ret = FAIL;
				sprintf(mess, "Could not set port %d.\n", p_number);
				FILE_LOG(logERROR) << mess;
			}
		}
	}

	socket.sendResult(true, ret, &p_number,sizeof(p_number), mess);
	return ret;
}



int slsReceiverTCPIPInterface::update_client(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr)
		NullObjectError(ret, mess);
	socket.sendResult(false, ret, nullptr, 0, mess);

	if (ret == FAIL)
		return ret;

	// update
	return send_update(socket);
}



int slsReceiverTCPIPInterface::send_update(sls::ServerInterface2 &socket) {
	int n = 0;
	int i32 = -1;
	char cstring[MAX_STR_LENGTH]{};

	char ip[INET_ADDRSTRLEN]{};
	sls::strcpy_safe(ip, server->getLastClient().str().c_str());
	n += socket.sendData(ip,sizeof(ip));

	// filepath
	strcpy(cstring, receiver->getFilePath().c_str());
    n += socket.sendData(cstring, sizeof(cstring));

	// filename
    strcpy(cstring, receiver->getFileName().c_str());
    n += socket.sendData(cstring, sizeof(cstring));

	// index
	i32=receiver->getFileIndex();
	n += socket.sendData(&i32, sizeof(i32));

	//file format
	i32=(int)receiver->getFileFormat();
	n += socket.sendData(&i32, sizeof(i32));

	//frames per file
	i32=(int)receiver->getFramesPerFile();
	n += socket.sendData(&i32, sizeof(i32));

	//frame discard policy
	i32=(int)receiver->getFrameDiscardPolicy();
	n += socket.sendData(&i32, sizeof(i32));

	//frame padding
	i32=(int)receiver->getFramePaddingEnable();
	n += socket.sendData(&i32, sizeof(i32));

	// file write enable
	i32=(int)receiver->getFileWriteEnable();
	n += socket.sendData(&i32, sizeof(i32));

	// master file write enable
	i32=(int)receiver->getMasterFileWriteEnable();
	n += socket.sendData(&i32, sizeof(i32));

	// file overwrite enable
	i32=(int)receiver->getOverwriteEnable();
	n += socket.sendData(&i32, sizeof(i32));

	// gap pixels
	i32=(int)receiver->getGapPixelsEnable();
	n += socket.sendData(&i32, sizeof(i32));

	// streaming frequency
	i32=(int)receiver->getStreamingFrequency();
	n += socket.sendData(&i32, sizeof(i32));

	// streaming port
	i32=(int)receiver->getStreamingPort();
	n += socket.sendData(&i32, sizeof(i32));

	// streaming source ip
    strcpy(cstring, receiver->getStreamingSourceIP().c_str());
    n += socket.sendData(cstring, sizeof(cstring));

    // additional json header
    strcpy(cstring, receiver->getAdditionalJsonHeader().c_str());
    n += socket.sendData(cstring, sizeof(cstring));

	// data streaming enable
	i32=(int)receiver->getDataStreamEnable();
	n += socket.sendData(&i32, sizeof(i32));

	// activate
	i32=(int)receiver->getActivate();
	n += socket.sendData(&i32, sizeof(i32));

	// deactivated padding enable
	i32=(int)receiver->getDeactivatedPadding();
	n += socket.sendData(&i32, sizeof(i32));

	// silent mode
	i32=(int)receiver->getSilentMode();
	n += socket.sendData(&i32, sizeof(i32));

	// dbit list
	{
		std::vector <int> list = receiver->getDbitList();
		int retvalsize = list.size();
		int retval[retvalsize];
		std::copy(std::begin(list), std::end(list), retval);
		socket.sendData(&retvalsize, sizeof(retvalsize));
		socket.sendData(retval, sizeof(retval));
	}

	// dbit offset
	i32=receiver->getDbitOffset();
	n += socket.sendData(&i32, sizeof(i32));

	return OK;
}



int slsReceiverTCPIPInterface::get_id(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t retval = getReceiverVersion();
	return socket.sendResult(true, ret, &retval, sizeof(retval));
}



int slsReceiverTCPIPInterface::set_detector_type(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	detectorType arg = GENERIC;
	detectorType retval = GENERIC;

	// get args, return if socket crashed
	if (socket.receiveArg(ret, mess, &arg, sizeof(arg)) == FAIL)
		return FAIL;

	// set
	if (arg >= 0) {
		// if object exists, verify unlocked and idle, else only verify lock (connecting first time)
		if (receiver == nullptr){
			VerifyLock(ret, mess);
		}
		else{
			VerifyLockAndIdle(ret, mess, fnum);
		}
			
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
					receiver = sls::make_unique<slsReceiverImplementation>();
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
			}

		}
	}
	//get
	retval = myDetectorType;
	validate((int)arg, (int)retval, std::string("set detector type"), DEC);
	return socket.sendResult(false, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_detector_hostname(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char hostname[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, hostname, MAX_STR_LENGTH);
	if(server == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}
		
	// base object not null
	if (ret == OK) {
		// set
		if (strlen(hostname)) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK)
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

	return socket.sendResult(true, ret,	retval, MAX_STR_LENGTH, mess);
}

int slsReceiverTCPIPInterface::LogSocketCrash(){
	FILE_LOG(logERROR) << "Reading from socket failed. Possible socket crash";
	return FAIL;
}

void slsReceiverTCPIPInterface::NullObjectError(int& ret, char* mess){
	ret=FAIL;
	strcpy(mess,"Receiver not set up. Please use rx_hostname first.\n");
	FILE_LOG(logERROR) << mess;
}



int slsReceiverTCPIPInterface::set_roi(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	static_assert(sizeof(ROI) == 4*sizeof(int), "ROI not packed");
	int narg = -1;
	socket.read(&narg,sizeof(narg));

	std::vector <ROI> arg;
	for (int iloop = 0; iloop < narg; ++iloop) {
		ROI temp{};
		socket.read(&temp, sizeof(temp));
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
		NullObjectError(ret, mess);
	else {
		if (VerifyLockAndIdle(ret, mess, fnum) == OK)
			ret = receiver->setROI(arg);
	}
	return socket.sendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::setup_udp(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char args[6][MAX_STR_LENGTH]{};
	char retvals[2][MAX_STR_LENGTH]{};

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, args, sizeof(args));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (VerifyLockAndIdle(ret, mess, fnum) == OK) {

			//setup interfaces count
			int numInterfaces = atoi(args[0]) > 1 ? 2 : 1;
			int selInterface = atoi(args[1]) > 1 ? 2 : 1;

			char* ip1 = args[2];
			char* ip2 = args[3];
			uint32_t port1 = atoi(args[4]);
			uint32_t port2 = atoi(args[5]);

			// using the 2nd interface only
			if (myDetectorType == JUNGFRAU && numInterfaces == 1 && selInterface == 2) {
				ip1 = ip2;
				port1 = port2;
			}

			// 1st interface
			receiver->setUDPPortNumber(port1);
			if (myDetectorType == EIGER) {
				receiver->setUDPPortNumber2(port2);
			}
			FILE_LOG(logINFO) << "Receiver UDP IP: " << ip1;
			// get eth
			std::string temp = sls::IpToInterfaceName(ip1);
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
				if (myDetectorType == EIGER) {
					receiver->setEthernetInterface2(eth);
				}
				//get mac address
				if (ret != FAIL) {
					temp = sls::InterfaceNameToMac(eth).str();
					if (temp=="00:00:00:00:00:00") {
						ret = FAIL;
						strcpy(mess,"failed to get mac adddress to listen to\n");
						FILE_LOG(logERROR) << mess;
					} else {
						// using the 2nd interface only
						if (myDetectorType == JUNGFRAU && numInterfaces == 1 && selInterface == 2) {
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
			if (myDetectorType == JUNGFRAU && numInterfaces == 2) {
				receiver->setUDPPortNumber2(port2);
				FILE_LOG(logINFO) << "Receiver UDP IP 2: " << ip2;
				// get eth
				temp = sls::IpToInterfaceName(ip2);
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
						temp = sls::InterfaceNameToMac(eth).str();
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
			if (myDetectorType == JUNGFRAU && receiver->setNumberofUDPInterfaces(numInterfaces) == FAIL) {
				ret = FAIL;
				sprintf(mess, "Failed to set number of interfaces\n");
				FILE_LOG(logERROR) << mess;
			}
		}
	}
	return socket.sendResult(true, ret, retvals, sizeof(retvals), mess);
}



int slsReceiverTCPIPInterface::set_timer(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t index[2] = {-1, -1};
	int64_t retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Setting timer index " << index[0] << " to " << index[1];

		// set
		if (index[1] >= 0) {
			// verify if receiver is unlocked
			if (VerifyLock(ret, mess) == OK) {
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
				case ANALOG_SAMPLES:
					if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
						modeNotImplemented("(Analog Samples) Timer index", (int)index[0]);
						break;
					}
					receiver->setNumberofAnalogSamples(index[1]);
					break;
				case DIGITAL_SAMPLES:
					if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
						modeNotImplemented("(Digital Samples) Timer index", (int)index[0]);
						break;
					}
					receiver->setNumberofDigitalSamples(index[1]);
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
		case ANALOG_SAMPLES:
			if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
				ret = FAIL;
				sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
				FILE_LOG(logERROR) << "Warning: " << mess;
				break;
			}
			retval=receiver->getNumberofAnalogSamples();
			break;
		case DIGITAL_SAMPLES:
			if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
				ret = FAIL;
				sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
				FILE_LOG(logERROR) << "Warning: " << mess;
				break;
			}
			retval=receiver->getNumberofDigitalSamples();
			break;
		default:
			modeNotImplemented("Timer index", (int)index[0]);
			break;
		}
		validate((int)index[1], (int)retval, std::string("set timer"), DEC);
		FILE_LOG(logDEBUG1) << slsDetectorDefs::getTimerType((timerIndex)(index[0])) << ":" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_dynamic_range(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int dr = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &dr, sizeof(dr));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (dr >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
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
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_frequency(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
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
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_status(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	enum runStatus retval = ERROR;

	// no arg, check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting Status";
		std::cout << "kalas\n";
		retval = receiver->getStatus();
		std::cout << "puff\n";
		FILE_LOG(logDEBUG1) << "Status:" << runStatusType(retval);
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::start_receiver(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}
	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (VerifyLock(ret, mess) == OK) {
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
	return socket.sendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::stop_receiver(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}
	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (VerifyLock(ret, mess) == OK) {
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
	return socket.sendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::set_file_dir(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char fPath[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, fPath, sizeof(fPath));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

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
    return socket.sendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_file_name(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char fName[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, fName, sizeof(fName));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

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
	 return socket.sendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_file_index(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting file index: " << index;
				receiver->setFileIndex(index);
			}
		}
		// get
		retval=receiver->getFileIndex();
		validate(index, retval, std::string("set file index"), DEC);
		FILE_LOG(logDEBUG1) << "file index:" << retval;
	}
	return socket.sendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_frame_index(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	// no arg, check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting frame index";
		retval = receiver->getAcquisitionIndex();
		FILE_LOG(logDEBUG1) << "frame index:" << retval;
	}
	return socket.sendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::get_frames_caught(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int retval = -1;

	// no arg, check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting frames caught";
		retval = receiver->getTotalFramesCaught();
		FILE_LOG(logDEBUG1) << "frames caught:" << retval;
	}
	return socket.sendResult(true, ret, &retval,sizeof(retval), mess);
}



int	slsReceiverTCPIPInterface::reset_frames_caught(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked
		if (VerifyLock(ret, mess) == OK) {
			FILE_LOG(logDEBUG1) << "Reset frames caught";
			receiver->resetAcquisitionCount();
		}
	}
	return socket.sendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::enable_file_write(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &enable, sizeof(enable));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting File write enable:" << enable;
				receiver->setFileWriteEnable(enable);
			}
		}
		// get
		retval = receiver->getFileWriteEnable();
		validate(enable, retval, std::string("set file write enable"), DEC);
		FILE_LOG(logDEBUG1) << "file write enable:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::enable_master_file_write(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &enable, sizeof(enable));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting Master File write enable:" << enable;
				receiver->setMasterFileWriteEnable(enable);
			}
		}
		// get
		retval = receiver->getMasterFileWriteEnable();
		validate(enable, retval, std::string("set master file write enable"), DEC);
		FILE_LOG(logDEBUG1) << "master file write enable:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}


int slsReceiverTCPIPInterface::enable_overwrite(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting File overwrite enable:" << index;
				receiver->setOverwriteEnable(index);
			}
		}
		// get
		retval = receiver->getOverwriteEnable();
		validate(index, retval, std::string("set file overwrite enable"), DEC);
		FILE_LOG(logDEBUG1) << "file overwrite enable:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::enable_tengiga(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int val = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &val, sizeof(val));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	if (myDetectorType != EIGER && myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (val >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting 10GbE:" << val;
				ret = receiver->setTenGigaEnable(val);
			}
		}
		// get
		retval = receiver->getTenGigaEnable();
		validate(val, retval, std::string("set 10GbE"), DEC);
		FILE_LOG(logDEBUG1) << "10Gbe:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_fifo_depth(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int value = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &value, sizeof(value));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (value >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
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
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_activate(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &enable, sizeof(enable));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting activate:" << enable;
				receiver->setActivate(enable > 0 ? true : false);
			}
		}
		// get
		retval = (int)receiver->getActivate();
		validate(enable, retval, std::string("set activate"), DEC);
		FILE_LOG(logDEBUG1) << "Activate: " << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_data_stream_enable(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}
	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting data stream enable:" << index;
				ret = receiver->setDataStreamEnable(index);
			}
		}
		// get
		retval = receiver->getDataStreamEnable();
		validate(index, retval, std::string("set data stream enable"), DEC);
		FILE_LOG(logDEBUG1) << "data streaming enable:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_timer(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting streaming timer:" << index;
				receiver->setStreamingTimer(index);
			}
		}
		// get
		retval=receiver->getStreamingTimer();
		validate(index, retval, std::string("set data stream timer"), DEC);
		FILE_LOG(logDEBUG1) << "Streaming timer:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_flipped_data(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int args[2] = {0,-1};
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, args, sizeof(args));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (args[1] >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting flipped data:" << args[1];
				receiver->setFlippedData(args[0],args[1]);
			}
		}
		// get
		retval=receiver->getFlippedData(args[0]);
		validate(args[1], retval, std::string("set flipped data"), DEC);
		FILE_LOG(logDEBUG1) << "Flipped Data:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_file_format(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	fileFormat retval = GET_FILE_FORMAT;
	fileFormat f = GET_FILE_FORMAT;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &f, sizeof(f));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (f >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting file format:" << f;
				receiver->setFileFormat(f);
			}
		}
		// get
		retval = receiver->getFileFormat();
		validate(f, retval, std::string("set file format"), DEC);
		FILE_LOG(logDEBUG1) << "File Format: " << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_detector_posid(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &arg, sizeof(arg));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (arg >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting detector position id:" << arg;
				receiver->setDetectorPositionId(arg);
			}
		}
		// get
		retval = receiver->getDetectorPositionId();
		validate(arg, retval, std::string("set detector position id"), DEC);
		FILE_LOG(logDEBUG1) << "Position Id:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_multi_detector_size(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg[]{-1, -1};
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, arg, sizeof(arg));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if((arg[0] > 0) && (arg[1] > 0)) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
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
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_port(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int port = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &port, sizeof(port));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (port >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting streaming port:" << port;
				receiver->setStreamingPort(port);
			}
		}
		// get
		retval = receiver->getStreamingPort();
		validate(port, retval, std::string("set streaming port"), DEC);
		FILE_LOG(logDEBUG1) << "streaming port:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_streaming_source_ip(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char arg[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, arg, MAX_STR_LENGTH);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
			FILE_LOG(logDEBUG1) << "Setting streaming source ip:" << arg;
			receiver->setStreamingSourceIP(arg);
		}
		// get
		strcpy(retval, receiver->getStreamingSourceIP().c_str());
		FILE_LOG(logDEBUG1) << "streaming source ip:" << retval;
	}
	return socket.sendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_silent_mode(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int value = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &value, sizeof(value));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (value >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting silent mode:" << value;
				receiver->setSilentMode(value);
			}
		}
		// get
		retval = (int)receiver->getSilentMode();
		validate(value, retval, std::string("set silent mode"), DEC);
		FILE_LOG(logDEBUG1) << "silent mode:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::enable_gap_pixels(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &enable, sizeof(enable));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (enable >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting gap pixels enable:" << enable;
				receiver->setGapPixelsEnable(enable);
			}
		}
		// get
		retval = receiver->getGapPixelsEnable();
		validate(enable, retval, std::string("set gap pixels enable"), DEC);
		FILE_LOG(logDEBUG1) << "Gap Pixels Enable: " << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::restream_stop(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));

	// no arg, and check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}

	// receiver is not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
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
	return socket.sendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::set_additional_json_header(sls::ServerInterface2 &socket) {
	ret = OK;
    memset(mess, 0, sizeof(mess));
    char arg[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, arg, sizeof(arg));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// only set
		// verify if receiver is unlocked and idle
		if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
			FILE_LOG(logDEBUG1) << "Setting additional json header: " << arg;
			receiver->setAdditionalJsonHeader(arg);
		}
		// get
		strcpy(retval, receiver->getAdditionalJsonHeader().c_str());
		FILE_LOG(logDEBUG1) << "additional json header:" << retval;
	}
	 return socket.sendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::get_additional_json_header(sls::ServerInterface2 &socket) {
	ret = OK;
    memset(mess, 0, sizeof(mess));
    char retval[MAX_STR_LENGTH] = {0};

	// no arg, check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}

	// base object not null
	if (ret == OK) {
		// get
		strcpy(retval, receiver->getAdditionalJsonHeader().c_str());
		FILE_LOG(logDEBUG1) << "additional json header:" << retval;
	}
	 return socket.sendResult(true, ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::set_udp_socket_buffer_size(sls::ServerInterface2 &socket) {
	ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t index = -1;
    int64_t retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
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
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size(sls::ServerInterface2 &socket){
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t retval = -1;

	// no arg, check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}

	if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Getting actual UDP buffer size";
		retval = receiver->getActualUDPSocketBufferSize();
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_frames_per_file(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting frames per file: " << index;
				receiver->setFramesPerFile(index);
			}
		}
		// get
		retval = receiver->getFramesPerFile();
		validate(index, retval, std::string("set frames per file"), DEC);
		FILE_LOG(logDEBUG1) << "frames per file:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::check_version_compatibility(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int64_t arg = -1;
	// get args, return if socket crashed
	socket.receiveArg(ret, mess, &arg, sizeof(arg));
	if (ret == FAIL)
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
	return socket.sendResult(true, ret, nullptr, 0, mess);
}



int slsReceiverTCPIPInterface::set_discard_policy(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting frames discard policy: " << index;
				receiver->setFrameDiscardPolicy((frameDiscardPolicy)index);
			}
		}
		// get
		retval = receiver->getFrameDiscardPolicy();
		validate(index, retval, std::string("set discard policy"), DEC);
		FILE_LOG(logDEBUG1) << "frame discard policy:" << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_padding_enable(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int index = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &index, sizeof(index));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		// set
		if (index >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
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
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_deactivated_padding_enable(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int enable = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &enable, sizeof(enable));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	if (myDetectorType != EIGER)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (enable >= 0) {
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting deactivated padding enable: " << enable;
				receiver->setDeactivatedPadding(enable > 0 ? true : false);
			}
		}
		// get
		retval = (int)receiver->getDeactivatedPadding();
		validate(enable, retval, std::string("set deactivated padding enable"), DEC);
		FILE_LOG(logDEBUG1) << "Deactivated Padding Enable: " << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}


int slsReceiverTCPIPInterface::set_readout_flags(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	readOutFlags arg = GET_READOUT_FLAGS;
	readOutFlags retval = GET_READOUT_FLAGS;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &arg, sizeof(arg));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	if (myDetectorType == JUNGFRAU || myDetectorType == GOTTHARD || myDetectorType == MOENCH)
		functionNotImplemented();

	// base object not null
	else if (ret == OK) {
		// set
		if (arg >= 0) {
			// verify if receiver is unlocked and idle
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting readout flag: " << arg;
				ret = receiver->setReadOutFlags(arg);
			}
		}
		// get
		retval = receiver->getReadOutFlags();
		validate((int)arg, (int)(retval & arg), std::string("set readout flags"), HEX);
		FILE_LOG(logDEBUG1) << "Readout flags: " << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}



int slsReceiverTCPIPInterface::set_adc_mask(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	uint32_t arg = -1;
	uint32_t retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	socket.receiveArg(ret, mess, &arg, sizeof(arg));
	if(receiver == nullptr){
		NullObjectError(ret, mess);
		return FAIL;
	}

	// base object not null
	if (ret == OK) {
		if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
			FILE_LOG(logDEBUG1) << "Setting ADC enable mask: " << arg;
			receiver->setADCEnableMask(arg);
		}
		retval = receiver->getADCEnableMask();
		if (ret == OK && retval != arg) {
			ret = FAIL;
			sprintf(mess, "Could not ADC enable mask. Set 0x%x, but read 0x%x\n", arg, retval);
			FILE_LOG(logERROR) << mess;
		}
		FILE_LOG(logDEBUG1) << "ADC enable mask retval: " << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}

int slsReceiverTCPIPInterface::set_dbit_list(sls::ServerInterface2 &socket) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> args;
    if (socket.receiveArg(ret, mess, &args, sizeof(args)) == FAIL) {
        if(receiver == nullptr){
			NullObjectError(ret, mess);
		}
		return FAIL;
	
    } else if (ret == OK) {
		FILE_LOG(logDEBUG1) << "Setting DBIT list";
        for (auto &it : args) {
            FILE_LOG(logDEBUG1) << it << " ";
        }
        FILE_LOG(logDEBUG1) << "\n";
        if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
            if (args.size() > 64) {
                ret = FAIL;
                sprintf(mess, "Could not set dbit list as size is > 64\n");
                FILE_LOG(logERROR) << mess;
            } else
                receiver->setDbitList(args);
        }
    }
    return socket.sendResult(true, ret, nullptr, 0, mess);
}

int slsReceiverTCPIPInterface::get_dbit_list(sls::ServerInterface2 &socket) {
	ret = OK;
    memset(mess, 0, sizeof(mess));
	sls::FixedCapacityContainer<int, MAX_RX_DBIT> retval;

	// no arg, check receiver is null
	socket.receiveArg(ret, mess, nullptr, 0);
	if(receiver == nullptr){
		NullObjectError(ret, mess);
	}
	// base object not null
	if (ret == OK) {
		retval = receiver->getDbitList();
		FILE_LOG(logDEBUG1) << "Dbit list size retval:" << retval.size();
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}


int slsReceiverTCPIPInterface::set_dbit_offset(sls::ServerInterface2 &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	int arg = -1;
	int retval = -1;

	// get args, return if socket crashed, ret is fail if receiver is not null
	if (socket.receiveArg(ret, mess, &arg, sizeof(arg)) == FAIL){
		if(receiver == nullptr){
			NullObjectError(ret, mess);
		}
		return FAIL;
	}
		

	// base object not null
	else if (ret == OK) {
		if (arg >= 0) {
			if (VerifyLockAndIdle(ret, mess, fnum) == OK) {
				FILE_LOG(logDEBUG1) << "Setting Dbit offset: " << arg;
				receiver->setDbitOffset(arg);
			}
		}
		retval = receiver->getDbitOffset();
		validate(arg, retval, std::string("set dbit offset"), DEC);
		FILE_LOG(logDEBUG1) << "Dbit offset retval: " << retval;
	}
	return socket.sendResult(true, ret, &retval, sizeof(retval), mess);
}