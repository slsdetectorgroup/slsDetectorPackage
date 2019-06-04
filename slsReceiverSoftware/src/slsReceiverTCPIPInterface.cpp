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
using sls::RuntimeError;
using Interface = sls::ServerInterface2;

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
                      << portNumber << '\n';
    server = sls::make_unique<sls::ServerSocket>(portNumber);
    while (true) {
		FILE_LOG(logDEBUG1) << "Start accept loop";
        try {
            auto socket = server->accept();
			// constexpr int time_us = 5000000;
			// socket.setReceiveTimeout(time_us);
			try{
				VerifyLock();
            	ret = decode_function(socket);
			}catch(const RuntimeError& e){
				//We had an error needs to be sent to client
				int r = FAIL;
        		strcpy(mess, e.what());
				socket.write(&r, sizeof(r));
				socket.write(mess, sizeof(mess));
			}
			

            // if tcp command was to exit server
            if (ret == GOODBYE) {
                FILE_LOG(logINFO) << "Shutting down UDP Socket";
                if (receiver) {
                    impl()->shutDownUDPSockets();
                }
                FILE_LOG(logINFOBLUE)
                    << "Exiting [ TCP server Tid: " << syscall(SYS_gettid)
                    << "]";
                pthread_exit(nullptr);
            }
        }catch(const RuntimeError& e){
			std::cout << "Accept failed\n";
		}

        // if user entered exit
        if (killTCPServerThread) {
            if (ret != GOODBYE) {
                if (receiver) {
                    impl()->shutDownUDPSockets();
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

int slsReceiverTCPIPInterface::decode_function(Interface &socket) {
    ret = FAIL;
	socket.receiveArg(fnum);
    if (fnum <= NUM_DET_FUNCTIONS || fnum >= NUM_REC_FUNCTIONS) {
        throw RuntimeError("Unrecognized Function enum " + 
							std::to_string(fnum) + "\n");
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
	char message[MAX_STR_LENGTH];
	sprintf(message, "Function (%s) is not implemented for this detector\n",
			getFunctionNameFromEnum((enum detFuncs)fnum));
	throw RuntimeError(mess);
}

void slsReceiverTCPIPInterface::modeNotImplemented(std::string modename, int mode) {
	char message[MAX_STR_LENGTH];
	sprintf(message, "%s (%d) is not implemented for this detector\n", modename.c_str(), mode);
	throw RuntimeError(message);
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
		throw RuntimeError(mess);
	}
}

void slsReceiverTCPIPInterface::VerifyLock() {
    if (lockStatus && server->getThisClient() != server->getLockedBy()) {
        strcpy(mess, "Receiver locked\n");
		throw sls::SocketError(mess);
    }
}

int slsReceiverTCPIPInterface::VerifyIdle(sls::ServerInterface2& socket) {
    if (impl()->getStatus() != IDLE) {
        sprintf(mess, "Can not execute %s when receiver is not idle\n",
                getFunctionNameFromEnum((enum detFuncs)fnum));
		throw sls::SocketError(mess);
    }
	return OK;
}

int slsReceiverTCPIPInterface::exec_command(Interface &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char cmd[MAX_STR_LENGTH]{};
	char retval[MAX_STR_LENGTH]{};
	socket.receiveArg( cmd, MAX_STR_LENGTH);
	FILE_LOG(logINFO) << "Executing command (" << cmd << ")";

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
	return socket.sendResult(ret, retval, MAX_STR_LENGTH, mess);
}



int slsReceiverTCPIPInterface::exit_server(Interface &socket) {
	FILE_LOG(logINFO) << "Closing server";
	ret = OK;
	memset(mess, 0, sizeof(mess));
	socket.sendResult(ret, nullptr, 0, nullptr);
	return GOODBYE;
}

void slsReceiverTCPIPInterface::ThrowNullObjectError(Interface &socket){
	int r = FAIL;
	strcpy(mess, "Receiver not set up. Please use rx_hostname first.\n"); 
	socket.write(&r, sizeof(r));
	socket.write(mess, sizeof(mess));
	throw sls::SocketError(mess);
}

void slsReceiverTCPIPInterface::NullObjectError(int& ret, char* mess){
	ret=FAIL;
	strcpy(mess,"Receiver not set up. Please use rx_hostname first.\n");
	FILE_LOG(logERROR) << mess;
}

int slsReceiverTCPIPInterface::lock_receiver(Interface &socket) {
    auto lock = socket.receive<int>();
    FILE_LOG(logDEBUG1) << "Locking Server to " << lock;
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
    return socket.sendResult(lockStatus);
}

int slsReceiverTCPIPInterface::get_last_client_ip(Interface &socket) {
	ret = OK;
	memset(mess, 0, sizeof(mess));
	char ip[INET_ADDRSTRLEN]{};
	sls::strcpy_safe(ip, server->getLastClient().str().c_str());
	return socket.sendResult(ret, &ip, sizeof(ip));
}


int slsReceiverTCPIPInterface::set_port(Interface &socket) {
	int p_number = -1;
	socket.receiveArg(p_number);
	if (p_number < 1024)
		throw RuntimeError("Port Number: " + std::to_string(p_number)+ " is too low (<1024)");
	
	FILE_LOG(logINFO) << "set port to " << p_number <<std::endl;
	auto new_server = sls::make_unique<sls::ServerSocket>(p_number);
	new_server->setLockedBy(server->getLockedBy());
	new_server->setLastClient(server->getThisClient());
	server = std::move(new_server);
	socket.sendResult(p_number);
	return OK;
}

int slsReceiverTCPIPInterface::update_client(Interface &socket) {
	if(receiver == nullptr)
		ThrowNullObjectError(socket);
	socket.sendResult(OK);
	return send_update(socket);
}



int slsReceiverTCPIPInterface::send_update(Interface &socket) {
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

int slsReceiverTCPIPInterface::get_id(Interface &socket){
	return socket.sendResult(getReceiverVersion());
}

int slsReceiverTCPIPInterface::set_detector_type(Interface &socket){
	memset(mess, 0, sizeof(mess));
	detectorType arg = GENERIC;
	detectorType retval = GENERIC;
	socket.receiveArg(arg);

	// set
	if (arg >= 0) {
		// if object exists, verify unlocked and idle, else only verify lock (connecting first time)
		if (receiver != nullptr){
			VerifyIdle(socket);
		}
		switch(arg) {
		case GOTTHARD:
		case EIGER:
		case CHIPTESTBOARD:
		case MOENCH:
		case JUNGFRAU:
			break;
		default:
			throw RuntimeError("Unknown detector type: " + std::to_string(arg));
			break;
		}
		
		if(receiver == nullptr){
			receiver = sls::make_unique<slsReceiverImplementation>();
		}
		myDetectorType = arg;
		impl()->setDetectorType(myDetectorType);
		retval = myDetectorType;

		// callbacks after (in setdetectortype, the object is reinitialized)
		if(startAcquisitionCallBack)
			impl()->registerCallBackStartAcquisition(startAcquisitionCallBack,pStartAcquisition);
		if(acquisitionFinishedCallBack)
			impl()->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,pAcquisitionFinished);
		if(rawDataReadyCallBack)
			impl()->registerCallBackRawDataReady(rawDataReadyCallBack,pRawDataReady);
		if(rawDataModifyReadyCallBack)
			impl()->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,pRawDataReady);
	
	}
	//get
	retval = myDetectorType;
	validate((int)arg, (int)retval, std::string("set detector type"), DEC);
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_detector_hostname(Interface &socket) {
	char hostname[MAX_STR_LENGTH]{};
	char retval[MAX_STR_LENGTH]{};

	socket.receiveArg(hostname);
	if (strlen(hostname)) {
		VerifyIdle(socket);
		impl()->setDetectorHostname(hostname);
	}
	auto s = impl()->getDetectorHostname();
	strcpy(retval, s.c_str());
	if (s.empty()) {
		throw RuntimeError("Hostname not set");
	}
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::LogSocketCrash(){
	FILE_LOG(logERROR) << "Reading from socket failed. Possible socket crash";
	return FAIL;
}

int slsReceiverTCPIPInterface::set_roi(Interface &socket) {
	static_assert(sizeof(ROI) == 4*sizeof(int), "ROI not packed");
	auto narg = socket.receive<int>();

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

	VerifyIdle(socket);
	if (impl()->setROI(arg) == FAIL)
		throw RuntimeError("Could not set ROI");
	return socket.sendResult(OK);
}

int slsReceiverTCPIPInterface::setup_udp(Interface &socket){
	ret = OK;
	char args[5][MAX_STR_LENGTH]{};
	char retvals[2][MAX_STR_LENGTH]{};
	socket.receiveArg(args);
	VerifyIdle(socket);

	//setup interfaces count
	int numInterfaces = atoi(args[0]) > 1 ? 2 : 1;
	char* ip1 = args[1];
	char* ip2 = args[2];
	uint32_t port1 = atoi(args[3]);
	uint32_t port2 = atoi(args[4]);

	// 1st interface
	impl()->setUDPPortNumber(port1);
	if (myDetectorType == EIGER) {
		impl()->setUDPPortNumber2(port2);
	}
	FILE_LOG(logINFO) << "Receiver UDP IP: " << ip1;
	// get eth
	std::string temp = sls::IpToInterfaceName(ip1);
	if (temp == "none"){
		ret = FAIL;
		strcpy(mess, "Failed to get ethernet interface or IP \n");
		FILE_LOG(logERROR) << mess;
	} else {
		char eth[MAX_STR_LENGTH]{};
		strcpy(eth, temp.c_str());
		// if there is a dot in eth name
		if (strchr(eth, '.') != nullptr) {
			strcpy(eth, "");
			ret = FAIL;
			sprintf(mess, "Failed to get ethernet interface from IP. Got %s\n", temp.c_str());
			FILE_LOG(logERROR) << mess;
		}
		impl()->setEthernetInterface(eth);
		if (myDetectorType == EIGER) {
			impl()->setEthernetInterface2(eth);
		}
		//get mac address
		if (ret != FAIL) {
			temp = sls::InterfaceNameToMac(eth).str();
			if (temp=="00:00:00:00:00:00") {
				ret = FAIL;
				strcpy(mess,"failed to get mac adddress to listen to\n");
				FILE_LOG(logERROR) << mess;
			} else {
				strcpy(retvals[0],temp.c_str());
				FILE_LOG(logINFO) << "Receiver MAC Address: " << retvals[0];
			}
		}
	}

	// 2nd interface
	if (myDetectorType == JUNGFRAU && numInterfaces == 2) {
		impl()->setUDPPortNumber2(port2);
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
			impl()->setEthernetInterface2(eth);

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
	if (myDetectorType == JUNGFRAU && impl()->setNumberofUDPInterfaces(numInterfaces) == FAIL) {
		ret = FAIL;
		sprintf(mess, "Failed to set number of interfaces\n");
		FILE_LOG(logERROR) << mess;
	}
	return socket.sendResult(ret, retvals, sizeof(retvals), mess);
}

int slsReceiverTCPIPInterface::set_timer(Interface &socket) {
	memset(mess, 0, sizeof(mess));
	int64_t index[2] = {-1, -1};
	int64_t retval = -1;
	socket.receiveArg(index);
	if (index[1] >= 0) {
		FILE_LOG(logDEBUG1) << "Setting timer index " << index[0] << " to " << index[1];
		switch (index[0]) {
		case ACQUISITION_TIME:
				ret = impl()->setAcquisitionTime(index[1]);
			break;
		case FRAME_PERIOD:
			ret = impl()->setAcquisitionPeriod(index[1]);
			break;
		case FRAME_NUMBER:
		case CYCLES_NUMBER:
		case STORAGE_CELL_NUMBER:
			impl()->setNumberOfFrames(index[1]);
			break;
		case SUBFRAME_ACQUISITION_TIME:
			impl()->setSubExpTime(index[1]);
			break;
		case SUBFRAME_DEADTIME:
			impl()->setSubPeriod(index[1] + impl()->getSubExpTime());
			break;
		case ANALOG_SAMPLES:
			if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
				modeNotImplemented("(Analog Samples) Timer index", (int)index[0]);
				break;
			}
			impl()->setNumberofAnalogSamples(index[1]);
			break;
		case DIGITAL_SAMPLES:
			if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
				modeNotImplemented("(Digital Samples) Timer index", (int)index[0]);
				break;
			}
			impl()->setNumberofDigitalSamples(index[1]);
			break;
		default:
			modeNotImplemented("Timer index", (int)index[0]);
			break;
		}

	}
	// get
	switch (index[0]) {
	case ACQUISITION_TIME:
		retval=impl()->getAcquisitionTime();
		break;
	case FRAME_PERIOD:
		retval=impl()->getAcquisitionPeriod();
		break;
	case FRAME_NUMBER:
	case CYCLES_NUMBER:
	case STORAGE_CELL_NUMBER:
		retval=impl()->getNumberOfFrames();
		break;
	case SUBFRAME_ACQUISITION_TIME:
		retval=impl()->getSubExpTime();
		break;
	case SUBFRAME_DEADTIME:
		retval=(impl()->getSubPeriod() - impl()->getSubExpTime());
		break;
	case ANALOG_SAMPLES:
		if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
			sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
			throw RuntimeError(mess);
			break;
		}
		retval=impl()->getNumberofAnalogSamples();
		break;
	case DIGITAL_SAMPLES:
		if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
			sprintf(mess,"This timer mode (%lld) does not exist for this receiver type\n", (long long int)index[0]);
			throw RuntimeError(mess);
			break;
		}
		retval=impl()->getNumberofDigitalSamples();
		break;
	default:
		modeNotImplemented("Timer index", (int)index[0]);
		break;
	}
	validate((int)index[1], (int)retval, std::string("set timer"), DEC);
	FILE_LOG(logDEBUG1) << slsDetectorDefs::getTimerType((timerIndex)(index[0])) << ":" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_dynamic_range(Interface &socket) {
	auto dr = socket.receive<int>();
	if (dr >= 0) {
		VerifyIdle(socket);
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
		if (!exists) {
			modeNotImplemented("Dynamic range", dr);
		}
		else {
			ret = impl()->setDynamicRange(dr);
			if(ret == FAIL) {
				throw RuntimeError("Could not allocate memory for fifo or could not start listening/writing threads");
			}
		}
		
	}
	int retval = impl()->getDynamicRange();
	validate(dr, retval, std::string("set dynamic range"), DEC);
	FILE_LOG(logDEBUG1) << "dynamic range: " << retval;
	return socket.sendResult(retval);
}



int slsReceiverTCPIPInterface::set_streaming_frequency(Interface &socket) {
	auto index = socket.receive<int>();
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting streaming frequency: " << index;
		ret = impl()->setStreamingFrequency(index);
		if(ret == FAIL) {
			throw RuntimeError("Could not allocate memory for listening fifo");
		}
	}
	int retval = impl()->getStreamingFrequency();
	validate(index, retval, std::string("set streaming frequency"), DEC);
	return socket.sendResult(retval);
}

int	slsReceiverTCPIPInterface::get_status(Interface &socket){
	auto retval = impl()->getStatus();
	FILE_LOG(logDEBUG1) << "Status:" << runStatusType(retval);
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::start_receiver(Interface &socket){
	runStatus status = impl()->getStatus();
	if (status != IDLE) {
		throw RuntimeError("Cannot start Receiver as it is: " +runStatusType(status));
	}else {
		FILE_LOG(logDEBUG1) << "Starting Receiver";
		ret = impl()->startReceiver(mess);
		if (ret == FAIL) {
			throw RuntimeError(mess);
		}
	}
	return socket.sendData(OK);
}

int slsReceiverTCPIPInterface::stop_receiver(Interface &socket){
	if(impl()->getStatus() != IDLE) {
		FILE_LOG(logDEBUG1) << "Stopping Receiver";
		impl()->stopReceiver();
	}
	auto s = impl()->getStatus();
	if (s != IDLE)
		throw RuntimeError("Could not stop receiver. It as it is: " + runStatusType(s));

	return socket.sendData(OK);
}

int slsReceiverTCPIPInterface::set_file_dir(Interface &socket) {
	char fPath[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
	socket.receiveArg(fPath);

	if (strlen(fPath)) {
		FILE_LOG(logDEBUG1) << "Setting file path: " << fPath;
		impl()->setFilePath(fPath);
	}
	std::string s = impl()->getFilePath();
	strcpy(retval, s.c_str());
	if ((!s.length()) || (strlen(fPath) && strcasecmp(fPath, retval)))
		throw RuntimeError("Receiver file path does not exist");
	else
		FILE_LOG(logDEBUG1) << "file path:" << retval;
	
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_file_name(Interface &socket) {
	char fName[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
	socket.receiveArg(fName);
	if (strlen(fName)) {
		FILE_LOG(logDEBUG1) << "Setting file name: " << fName;
		impl()->setFileName(fName);
	}
	std::string s = impl()->getFileName();
	if (s.empty())
		throw RuntimeError("file name is empty");

	strcpy(retval, s.c_str());
	FILE_LOG(logDEBUG1) << "file name:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_file_index(Interface &socket) {
	int index = -1;
	socket.receiveArg(index);
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting file index: " << index;
		impl()->setFileIndex(index);
	}
	int retval = impl()->getFileIndex();
	validate(index, retval, std::string("set file index"), DEC);
	FILE_LOG(logDEBUG1) << "file index:" << retval;
	return socket.sendResult(retval);
}

int	slsReceiverTCPIPInterface::get_frame_index(Interface &socket){
	uint64_t retval = impl()->getAcquisitionIndex();
	FILE_LOG(logDEBUG1) << "frame index:" << retval;
	return socket.sendResult(retval);
}

int	slsReceiverTCPIPInterface::get_frames_caught(Interface &socket){
	int retval = impl()->getTotalFramesCaught();
	FILE_LOG(logDEBUG1) << "frames caught:" << retval;
	return socket.sendResult(retval);
}

int	slsReceiverTCPIPInterface::reset_frames_caught(Interface &socket){
	FILE_LOG(logDEBUG1) << "Reset frames caught";
	impl()->resetAcquisitionCount();
	return socket.sendData(OK);
}

int slsReceiverTCPIPInterface::enable_file_write(Interface &socket){
	int enable = -1;
	socket.receiveArg(enable);
	// set
	if (enable >= 0) {
		// verify if receiver is unlocked and idle
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting File write enable:" << enable;
		impl()->setFileWriteEnable(enable);
	}
	// get
	int retval = impl()->getFileWriteEnable();
	validate(enable, retval, std::string("set file write enable"), DEC);
	FILE_LOG(logDEBUG1) << "file write enable:" << retval;
	return socket.sendResult(retval);
}


int slsReceiverTCPIPInterface::enable_master_file_write(Interface &socket){
	int enable = -1;
	socket.receiveArg(enable);
	// set
	if (enable >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting Master File write enable:" << enable;
		impl()->setMasterFileWriteEnable(enable);	
	}
	// get
	int retval = impl()->getMasterFileWriteEnable();
	validate(enable, retval, std::string("set master file write enable"), DEC);
	FILE_LOG(logDEBUG1) << "master file write enable:" << retval;
	return socket.sendResult(retval);
}


int slsReceiverTCPIPInterface::enable_overwrite(Interface &socket) {
	int index = -1;
	socket.receiveArg(index);
	// set
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting File overwrite enable:" << index;
		impl()->setOverwriteEnable(index);
	}
	// get
	int retval = impl()->getOverwriteEnable();
	validate(index, retval, std::string("set file overwrite enable"), DEC);
	FILE_LOG(logDEBUG1) << "file overwrite enable:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::enable_tengiga(Interface &socket) {
	int val = -1;
	socket.receiveArg(val);
	if (myDetectorType != EIGER && myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH)
		functionNotImplemented();

	if (val >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting 10GbE:" << val;
		ret = impl()->setTenGigaEnable(val);

	}
	int retval = impl()->getTenGigaEnable();
	validate(val, retval, std::string("set 10GbE"), DEC);
	FILE_LOG(logDEBUG1) << "10Gbe:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_fifo_depth(Interface &socket) {
	auto value = socket.receive<int>();
	if (value >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting fifo depth:" << value;
		impl()->setFifoDepth(value);
	}
	int retval = impl()->getFifoDepth();
	validate(value, retval, std::string("set fifo depth"), DEC);
	FILE_LOG(logDEBUG1) << "fifo depth:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_activate(Interface &socket) {
	int enable = -1;
	socket.receiveArg(enable);
	if (myDetectorType != EIGER)
		functionNotImplemented();

	if (enable >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting activate:" << enable;
		impl()->setActivate(enable > 0 ? true : false);
	}
	auto retval = static_cast<int>(impl()->getActivate());
	validate(enable, retval, std::string("set activate"), DEC);
	FILE_LOG(logDEBUG1) << "Activate: " << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_data_stream_enable(Interface &socket){
	int index = -1;
	socket.receiveArg(index);
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting data stream enable:" << index;
		ret = impl()->setDataStreamEnable(index);
	}
	int retval = impl()->getDataStreamEnable();
	validate(index, retval, std::string("set data stream enable"), DEC);
	FILE_LOG(logDEBUG1) << "data streaming enable:" << retval;
	return socket.sendResult(retval);
}



int slsReceiverTCPIPInterface::set_streaming_timer(Interface &socket){
	auto index = socket.receive<int>();
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting streaming timer:" << index;
		impl()->setStreamingTimer(index);
	}
	int retval=impl()->getStreamingTimer();
	validate(index, retval, std::string("set data stream timer"), DEC);
	FILE_LOG(logDEBUG1) << "Streaming timer:" << retval;
	return socket.sendResult(retval);
}



int slsReceiverTCPIPInterface::set_flipped_data(Interface &socket){
	//TODO! Why 2 args?
	memset(mess, 0, sizeof(mess));
	int args[2]{0,-1};
	socket.receiveArg(args);

	if (myDetectorType != EIGER)
		functionNotImplemented();

	if (args[1] >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting flipped data:" << args[1];
		impl()->setFlippedData(args[0],args[1]);
	}
	int retval = impl()->getFlippedData(args[0]);
	validate(args[1], retval, std::string("set flipped data"), DEC);
	FILE_LOG(logDEBUG1) << "Flipped Data:" << retval;
	return socket.sendResult(retval);
}



int slsReceiverTCPIPInterface::set_file_format(Interface &socket) {
	fileFormat f = GET_FILE_FORMAT;
	socket.receiveArg(f);
	if (f >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting file format:" << f;
		impl()->setFileFormat(f);
	}
	auto retval = impl()->getFileFormat();
	validate(f, retval, std::string("set file format"), DEC);
	FILE_LOG(logDEBUG1) << "File Format: " << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_detector_posid(Interface &socket) {
	auto arg = socket.receive<int>();
	if (arg >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting detector position id:" << arg;
		impl()->setDetectorPositionId(arg);
	}
	auto retval = impl()->getDetectorPositionId();
	validate(arg, retval, std::string("set detector position id"), DEC);
	FILE_LOG(logDEBUG1) << "Position Id:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_multi_detector_size(Interface &socket) {
	int arg[]{-1, -1};
	socket.receiveArg(arg);
	if((arg[0] > 0) && (arg[1] > 0)) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting multi detector size:" << arg[0] << "," << arg[1];
		impl()->setMultiDetectorSize(arg);
		
	}
	int* temp = impl()->getMultiDetectorSize(); //TODO! return by value!
	int retval = -1;
	for (int i = 0; i < MAX_DIMENSIONS; ++i) {
		if (i == 0)
			retval = temp[i];
		else
			retval *= temp[i];
	}
	FILE_LOG(logDEBUG1) << "Multi Detector Size:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_streaming_port(Interface &socket) {
	auto port = socket.receive<int>();
	if (port >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting streaming port:" << port;
		impl()->setStreamingPort(port);
	}
	int retval = impl()->getStreamingPort();
	validate(port, retval, std::string("set streaming port"), DEC);
	FILE_LOG(logDEBUG1) << "streaming port:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_streaming_source_ip(Interface &socket) {
	char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
	socket.receiveArg(arg);
	VerifyIdle(socket);
	FILE_LOG(logDEBUG1) << "Setting streaming source ip:" << arg;
	impl()->setStreamingSourceIP(arg);
	strcpy(retval, impl()->getStreamingSourceIP().c_str());
	FILE_LOG(logDEBUG1) << "streaming source ip:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_silent_mode(Interface &socket) {
	auto value = socket.receive<int>();
	if (value >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting silent mode:" << value;
		impl()->setSilentMode(value);
	}
	auto retval = static_cast<int>(impl()->getSilentMode());
	validate(value, retval, std::string("set silent mode"), DEC);
	FILE_LOG(logDEBUG1) << "silent mode:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::enable_gap_pixels(Interface &socket) {
	auto enable = socket.receive<int>();
	if (myDetectorType != EIGER)
		functionNotImplemented();

	if (enable >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting gap pixels enable:" << enable;
		impl()->setGapPixelsEnable(enable);
	}
	int retval = impl()->getGapPixelsEnable();
	validate(enable, retval, std::string("set gap pixels enable"), DEC);
	FILE_LOG(logDEBUG1) << "Gap Pixels Enable: " << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::restream_stop(Interface &socket){
	VerifyIdle(socket);
	if (impl()->getDataStreamEnable() == false) {
		throw RuntimeError("Could not restream stop packet as data Streaming is disabled");
	} else {
		FILE_LOG(logDEBUG1) << "Restreaming stop";
		ret = impl()->restreamStop();
		if (ret == FAIL)
			throw RuntimeError("Could not restream stop packet");
	}
	return socket.sendResult(OK);
}

int slsReceiverTCPIPInterface::set_additional_json_header(Interface &socket) {
    memset(mess, 0, sizeof(mess));
    char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
	socket.receiveArg(arg);
	if (VerifyIdle(socket) == OK) {
		FILE_LOG(logDEBUG1) << "Setting additional json header: " << arg;
		impl()->setAdditionalJsonHeader(arg);
	}
	strcpy(retval, impl()->getAdditionalJsonHeader().c_str());
	FILE_LOG(logDEBUG1) << "additional json header:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::get_additional_json_header(Interface &socket) {
    char retval[MAX_STR_LENGTH]{};
	strcpy(retval, impl()->getAdditionalJsonHeader().c_str());
	FILE_LOG(logDEBUG1) << "additional json header:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_udp_socket_buffer_size(Interface &socket) {
	auto index = socket.receive<int64_t>();
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting UDP Socket Buffer size: " << index;
		if (impl()->setUDPSocketBufferSize(index) == FAIL) {
			throw RuntimeError("Could not create dummy UDP Socket to test buffer size");
		}
	}
	int64_t retval = impl()->getUDPSocketBufferSize();
	if (index != 0)
		validate(index, retval, std::string("set udp socket buffer size (No CAP_NET_ADMIN privileges?)"), DEC);
	FILE_LOG(logDEBUG1) << "UDP Socket Buffer Size:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size(Interface &socket){
	return socket.sendResult(impl()->getActualUDPSocketBufferSize());
}

int slsReceiverTCPIPInterface::set_frames_per_file(Interface &socket) {
	auto index = socket.receive<int>();
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting frames per file: " << index;
		impl()->setFramesPerFile(index);
	}
	auto retval = static_cast<int>(impl()->getFramesPerFile());
	validate(index, retval, std::string("set frames per file"), DEC);
	FILE_LOG(logDEBUG1) << "frames per file:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::check_version_compatibility(Interface &socket) {
	auto arg =  socket.receive<int64_t>();
	FILE_LOG(logDEBUG1) << "Checking versioning compatibility with value " << arg;
	int64_t client_requiredVersion = arg;
	int64_t rx_apiVersion = APIRECEIVER;
	int64_t rx_version = getReceiverVersion();

	if (rx_apiVersion > client_requiredVersion) {
		// old client
		ret = FAIL;
		sprintf(mess,"This client is incompatible.\n"
				"Client's receiver API Version: (0x%llx). Receiver API Version: (0x%llx).\n"
				"Incompatible, update client!\n",
				(long long unsigned int)client_requiredVersion,
				(long long unsigned int)rx_apiVersion);
		throw RuntimeError(mess);
	}else if (client_requiredVersion > rx_version) {
		// old software
		ret = FAIL;
		sprintf(mess,"This receiver is incompatible.\n"
				"Receiver Version: (0x%llx). Client's receiver API Version: (0x%llx).\n"
				"Incompatible, update receiver!\n",
				(long long unsigned int)rx_version,
				(long long unsigned int)client_requiredVersion);
		throw RuntimeError(mess);
	}
	else{
		FILE_LOG(logINFO) << "Compatibility with Client: Successful";
	} 
	return socket.sendResult(OK);
}

int slsReceiverTCPIPInterface::set_discard_policy(Interface &socket) {
	auto index = socket.receive<int>();
	if (index >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting frames discard policy: " << index;
		impl()->setFrameDiscardPolicy((frameDiscardPolicy)index);
	}
	int retval = impl()->getFrameDiscardPolicy();
	validate(index, retval, std::string("set discard policy"), DEC);
	FILE_LOG(logDEBUG1) << "frame discard policy:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_padding_enable(Interface &socket) {
	auto index = socket.receive<int>();
	if (index >= 0) {
		VerifyIdle(socket);
		index = (index == 0) ? 0 : 1;
		FILE_LOG(logDEBUG1) << "Setting frames padding enable: " << index;
		impl()->setFramePaddingEnable(index);
	}
	auto retval = static_cast<int>(impl()->getFramePaddingEnable());
	validate(index, retval, std::string("set frame padding enable"), DEC);
	FILE_LOG(logDEBUG1) << "Frame Padding Enable:" << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_deactivated_padding_enable(Interface &socket) {
	auto enable =  socket.receive<int>();
	if (myDetectorType != EIGER)
		functionNotImplemented();

	if (enable >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting deactivated padding enable: " << enable;
		impl()->setDeactivatedPadding(enable > 0 ? true : false);
	}
	auto retval = static_cast<int>(impl()->getDeactivatedPadding());
	validate(enable, retval, std::string("set deactivated padding enable"), DEC);
	FILE_LOG(logDEBUG1) << "Deactivated Padding Enable: " << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_readout_flags(Interface &socket) {
	auto arg = GET_READOUT_FLAGS;
	socket.receiveArg(arg);

	if (myDetectorType == JUNGFRAU || myDetectorType == GOTTHARD || myDetectorType == MOENCH)
		functionNotImplemented();

	if (arg >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting readout flag: " << arg;
		ret = impl()->setReadOutFlags(arg);
	}
	auto retval = impl()->getReadOutFlags();
	validate((int)arg, (int)(retval & arg), std::string("set readout flags"), HEX);
	FILE_LOG(logDEBUG1) << "Readout flags: " << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_adc_mask(Interface &socket) {
	auto arg = socket.receive<uint32_t>();
	VerifyIdle(socket);
	FILE_LOG(logDEBUG1) << "Setting ADC enable mask: " << arg;
	impl()->setADCEnableMask(arg);
	auto retval = impl()->getADCEnableMask();
	if (retval != arg) {
		sprintf(mess, "Could not ADC enable mask. Set 0x%x, but read 0x%x\n", arg, retval);
		throw RuntimeError(mess);
	}
	FILE_LOG(logDEBUG1) << "ADC enable mask retval: " << retval;
	return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_dbit_list(Interface &socket) {
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> args;
    socket.receiveArg(args);
	FILE_LOG(logDEBUG1) << "Setting DBIT list";
	for (auto &it : args) {
		FILE_LOG(logDEBUG1) << it << " ";
	}
	FILE_LOG(logDEBUG1) << "\n";
	VerifyIdle(socket);
	if (args.size() > 64) {
		ret = FAIL;
		sprintf(mess, "Could not set dbit list as size is > 64\n");
		FILE_LOG(logERROR) << mess;
	} else
		impl()->setDbitList(args);
	
    return socket.sendResult(OK);
}

int slsReceiverTCPIPInterface::get_dbit_list(Interface &socket) {
	sls::FixedCapacityContainer<int, MAX_RX_DBIT> retval;
	retval = impl()->getDbitList();
	FILE_LOG(logDEBUG1) << "Dbit list size retval:" << retval.size();
	return socket.sendResult(ret, &retval, sizeof(retval), mess);
}

int slsReceiverTCPIPInterface::set_dbit_offset(Interface &socket) {
	auto arg = socket.receive<int>();
	if (arg >= 0) {
		VerifyIdle(socket);
		FILE_LOG(logDEBUG1) << "Setting Dbit offset: " << arg;
		impl()->setDbitOffset(arg);
	}
	int retval = impl()->getDbitOffset();
	validate(arg, retval, std::string("set dbit offset"), DEC);
	FILE_LOG(logDEBUG1) << "Dbit offset retval: " << retval;
	return socket.sendResult(retval);
}
