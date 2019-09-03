/**********************************************
 * @file
 *slsReceiverTCPIPInterface.cpp
 * @short interface between
 *receiver and client
 ***********************************************/

#include "slsReceiverTCPIPInterface.h"
#include "FixedCapacityContainer.h"
#include "ServerSocket.h"
#include "slsReceiver.h"
#include "slsReceiverImplementation.h"
#include "slsReceiverUsers.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"
#include "versionAPI.h"

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <syscall.h>
#include <vector>

using sls::RuntimeError;
using sls::SocketError;
using Interface = sls::ServerInterface2;

slsReceiverTCPIPInterface::~slsReceiverTCPIPInterface() { stop(); }

slsReceiverTCPIPInterface::slsReceiverTCPIPInterface(int pn)
    : myDetectorType(GOTTHARD), portNumber(pn > 0 ? pn : DEFAULT_PORTNO + 2) {
    function_table();
}

int slsReceiverTCPIPInterface::start() {
    FILE_LOG(logDEBUG) << "Creating TCP Server Thread";
    killTCPServerThread = 0;
    if (pthread_create(&TCPServer_thread, nullptr, startTCPServerThread,
                       (void *)this)) {
        FILE_LOG(logERROR) << "Could not create TCP Server thread";
        return FAIL;
    }
    tcpThreadCreated = true;
    FILE_LOG(logDEBUG) << "TCP Server thread created successfully.";
    return OK;
}

void slsReceiverTCPIPInterface::stop() {
    if (tcpThreadCreated) {
        FILE_LOG(logINFO) << "Shutting down TCP Socket on port " << portNumber;
        killTCPServerThread = 1;
        if (server)
            server->shutDownSocket();
        FILE_LOG(logDEBUG) << "TCP Socket closed on port " << portNumber;
        pthread_join(TCPServer_thread, nullptr);
        tcpThreadCreated = false;
        killTCPServerThread = 0;
        FILE_LOG(logDEBUG) << "Exiting TCP Server Thread on port "
                           << portNumber;
    }
}

int64_t slsReceiverTCPIPInterface::getReceiverVersion() { return APIRECEIVER; }

/***callback functions***/
void slsReceiverTCPIPInterface::registerCallBackStartAcquisition(
    int (*func)(char *, char *, uint64_t, uint32_t, void *), void *arg) {
    startAcquisitionCallBack = func;
    pStartAcquisition = arg;
}

void slsReceiverTCPIPInterface::registerCallBackAcquisitionFinished(
    void (*func)(uint64_t, void *), void *arg) {
    acquisitionFinishedCallBack = func;
    pAcquisitionFinished = arg;
}

void slsReceiverTCPIPInterface::registerCallBackRawDataReady(
    void (*func)(char *, char *, uint32_t, void *), void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
}

void slsReceiverTCPIPInterface::registerCallBackRawDataModifyReady(
    void (*func)(char *, char *, uint32_t &, void *), void *arg) {
    rawDataModifyReadyCallBack = func;
    pRawDataReady = arg;
}

void *slsReceiverTCPIPInterface::startTCPServerThread(void *this_pointer) {
    ((slsReceiverTCPIPInterface *)this_pointer)->startTCPServer();
    return this_pointer;
}

void slsReceiverTCPIPInterface::startTCPServer() {
    FILE_LOG(logINFOBLUE) << "Created [ TCP server Tid: " << syscall(SYS_gettid)
                          << "]";
    FILE_LOG(logINFO) << "SLS Receiver starting TCP Server on port "
                      << portNumber << '\n';
    server = sls::make_unique<sls::ServerSocket>(portNumber);
    while (true) {
        FILE_LOG(logDEBUG1) << "Start accept loop";
        try {
            auto socket = server->accept();
            try {
                VerifyLock();
                ret = decode_function(socket);
            } catch (const RuntimeError &e) {
                // We had an error needs to be sent to client
                sls::strcpy_safe(mess, e.what());
                socket.Send(FAIL);
                socket.Send(mess);
            }

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
        } catch (const RuntimeError &e) {
            FILE_LOG(logERROR) << "Accept failed";
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
// clang-format off
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
	flist[F_RECEIVER_SET_READOUT_MODE] 	    = 	&slsReceiverTCPIPInterface::set_readout_mode;
	flist[F_RECEIVER_SET_ADC_MASK]			=	&slsReceiverTCPIPInterface::set_adc_mask;
	flist[F_SET_RECEIVER_DBIT_LIST]			=	&slsReceiverTCPIPInterface::set_dbit_list;
	flist[F_GET_RECEIVER_DBIT_LIST]			=	&slsReceiverTCPIPInterface::get_dbit_list;
	flist[F_RECEIVER_DBIT_OFFSET]			= 	&slsReceiverTCPIPInterface::set_dbit_offset;
    flist[F_SET_RECEIVER_QUAD]			    = 	&slsReceiverTCPIPInterface::set_quad_type;
    flist[F_SET_RECEIVER_READ_N_LINES]      =   &slsReceiverTCPIPInterface::set_read_n_lines;
    flist[F_SET_RECEIVER_UDP_IP]            =   &slsReceiverTCPIPInterface::set_udp_ip;
	flist[F_SET_RECEIVER_UDP_IP2]           =   &slsReceiverTCPIPInterface::set_udp_ip2;
	flist[F_SET_RECEIVER_UDP_PORT]          =   &slsReceiverTCPIPInterface::set_udp_port;
	flist[F_SET_RECEIVER_UDP_PORT2]         =   &slsReceiverTCPIPInterface::set_udp_port2;
	flist[F_SET_RECEIVER_NUM_INTERFACES]    =   &slsReceiverTCPIPInterface::set_num_interfaces;

	for (int i = NUM_DET_FUNCTIONS + 1; i < NUM_REC_FUNCTIONS ; i++) {
		FILE_LOG(logDEBUG1) << "function fnum: " << i << " (" <<
				getFunctionNameFromEnum((enum detFuncs)i) << ") located at " << flist[i];
	}

	return OK;
}
// clang-format on
int slsReceiverTCPIPInterface::decode_function(Interface &socket) {
    ret = FAIL;
    socket.Receive(fnum);
    if (fnum <= NUM_DET_FUNCTIONS || fnum >= NUM_REC_FUNCTIONS) {
        throw RuntimeError("Unrecognized Function enum " +
                           std::to_string(fnum) + "\n");
    } else {
        FILE_LOG(logDEBUG1) << "calling function fnum: " << fnum << " ("
                            << getFunctionNameFromEnum((enum detFuncs)fnum)
                            << ")";
        ret = (this->*flist[fnum])(socket);
        FILE_LOG(logDEBUG1)
            << "Function " << getFunctionNameFromEnum((enum detFuncs)fnum)
            << " finished";
    }
    return ret;
}

void slsReceiverTCPIPInterface::functionNotImplemented() {
    std::ostringstream os;
    os << "Function: " << getFunctionNameFromEnum((enum detFuncs)fnum)
       << ", is is not implemented for this detector";
    throw RuntimeError(os.str());
}

void slsReceiverTCPIPInterface::modeNotImplemented(const std::string &modename,
                                                   int mode) {
    std::ostringstream os;
    os << modename << " (" << mode << ") is not implemented for this detector";
    throw RuntimeError(os.str());
}

template <typename T>
void slsReceiverTCPIPInterface::validate(T arg, T retval, std::string modename,
                                         numberMode hex) {
    if (ret == OK && arg != -1 && retval != arg) {
        auto format = (hex == HEX) ? std::hex : std::dec;
        auto prefix = (hex == HEX) ? "0x" : "";
        std::ostringstream os;
        os << "Could not " << modename << ". Set " << prefix << format << arg
           << ", but read " << prefix << retval << '\n';
        throw RuntimeError(os.str());
    }
}

void slsReceiverTCPIPInterface::VerifyLock() {
    if (lockStatus && server->getThisClient() != server->getLockedBy()) {
        throw sls::SocketError("Receiver locked\n");
    }
}

void slsReceiverTCPIPInterface::VerifyIdle(Interface &socket) {
    if (impl()->getStatus() != IDLE) {
        sprintf(mess, "Can not execute %s when receiver is not idle\n",
                getFunctionNameFromEnum((enum detFuncs)fnum));
        throw sls::SocketError(mess);
    }
}

int slsReceiverTCPIPInterface::exec_command(Interface &socket) {
    char cmd[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    socket.Receive(cmd);
    FILE_LOG(logINFO) << "Executing command (" << cmd << ")";
    const size_t tempsize = 256;
    std::array<char, tempsize> temp{};
    std::string sresult;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw RuntimeError("Executing Command failed\n");
    } else {
        while (!feof(pipe.get())) {
            if (fgets(temp.data(), tempsize, pipe.get()) != nullptr)
                sresult += temp.data();
        }
        strncpy(retval, sresult.c_str(), MAX_STR_LENGTH);
        FILE_LOG(logINFO) << "Result of cmd (" << cmd << "):\n" << retval;
    }
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::exit_server(Interface &socket) {
    FILE_LOG(logINFO) << "Closing server";
    socket.Send(OK);
    return GOODBYE;
}

int slsReceiverTCPIPInterface::lock_receiver(Interface &socket) {
    auto lock = socket.Receive<int>();
    FILE_LOG(logDEBUG1) << "Locking Server to " << lock;
    if (lock >= 0) {
        if (!lockStatus || (server->getLockedBy() == server->getThisClient())) {
            lockStatus = lock;
            lock ? server->setLockedBy(server->getThisClient())
                 : server->setLockedBy(sls::IpAddr{});
        } else {
            throw RuntimeError("Receiver locked\n");
        }
    }
    return socket.sendResult(lockStatus);
}

int slsReceiverTCPIPInterface::get_last_client_ip(Interface &socket) {
    return socket.sendResult(server->getLastClient().arr());
}

int slsReceiverTCPIPInterface::set_port(Interface &socket) {
    auto p_number = socket.Receive<int>();
    if (p_number < 1024)
        throw RuntimeError("Port Number: " + std::to_string(p_number) +
                           " is too low (<1024)");

    FILE_LOG(logINFO) << "set port to " << p_number << std::endl;
    auto new_server = sls::make_unique<sls::ServerSocket>(p_number);
    new_server->setLockedBy(server->getLockedBy());
    new_server->setLastClient(server->getThisClient());
    server = std::move(new_server);
    socket.sendResult(p_number);
    return OK;
}

int slsReceiverTCPIPInterface::update_client(Interface &socket) {
    if (receiver == nullptr)
        throw sls::SocketError(
            "Receiver not set up. Please use rx_hostname first.\n");
    socket.Send(OK);
    return send_update(socket);
}

int slsReceiverTCPIPInterface::send_update(Interface &socket) {
    int n = 0;
    int i32 = -1;
    char cstring[MAX_STR_LENGTH]{};

    char ip[INET_ADDRSTRLEN]{};
    sls::strcpy_safe(ip, server->getLastClient().str().c_str());
    n += socket.Send(ip, sizeof(ip));

    // filepath
    sls::strcpy_safe(cstring, receiver->getFilePath().c_str());
    n += socket.Send(cstring, sizeof(cstring));

    // filename
    sls::strcpy_safe(cstring, receiver->getFileName().c_str());
    n += socket.Send(cstring, sizeof(cstring));

    // index
    i32 = receiver->getFileIndex();
    n += socket.Send(&i32, sizeof(i32));

    // file format
    i32 = (int)receiver->getFileFormat();
    n += socket.Send(&i32, sizeof(i32));

    // frames per file
    i32 = (int)receiver->getFramesPerFile();
    n += socket.Send(&i32, sizeof(i32));

    // frame discard policy
    i32 = (int)receiver->getFrameDiscardPolicy();
    n += socket.Send(&i32, sizeof(i32));

    // frame padding
    i32 = (int)receiver->getFramePaddingEnable();
    n += socket.Send(&i32, sizeof(i32));

    // file write enable
    i32 = (int)receiver->getFileWriteEnable();
    n += socket.Send(&i32, sizeof(i32));

    // master file write enable
    i32 = (int)receiver->getMasterFileWriteEnable();
    n += socket.Send(&i32, sizeof(i32));

    // file overwrite enable
    i32 = (int)receiver->getOverwriteEnable();
    n += socket.Send(&i32, sizeof(i32));

    // gap pixels
    i32 = (int)receiver->getGapPixelsEnable();
    n += socket.Send(&i32, sizeof(i32));

    // streaming frequency
    i32 = (int)receiver->getStreamingFrequency();
    n += socket.Send(&i32, sizeof(i32));

    // streaming port
    i32 = (int)receiver->getStreamingPort();
    n += socket.Send(&i32, sizeof(i32));

    // streaming source ip
    sls::strcpy_safe(cstring, receiver->getStreamingSourceIP().c_str());
    n += socket.Send(cstring, sizeof(cstring));

    // additional json header
    sls::strcpy_safe(cstring, receiver->getAdditionalJsonHeader().c_str());
    n += socket.Send(cstring, sizeof(cstring));

    // data streaming enable
    i32 = (int)receiver->getDataStreamEnable();
    n += socket.Send(&i32, sizeof(i32));

    // activate
    i32 = (int)receiver->getActivate();
    n += socket.Send(&i32, sizeof(i32));

    // deactivated padding enable
    i32 = (int)receiver->getDeactivatedPadding();
    n += socket.Send(&i32, sizeof(i32));

    // silent mode
    i32 = (int)receiver->getSilentMode();
    n += socket.Send(&i32, sizeof(i32));

    // dbit list
    {
        std::vector<int> list = receiver->getDbitList();
        int retvalsize = list.size();
        int retval[retvalsize];
        std::copy(std::begin(list), std::end(list), retval);
        socket.Send(&retvalsize, sizeof(retvalsize));
        socket.Send(retval, sizeof(retval));
    }

    // dbit offset
    i32 = receiver->getDbitOffset();
    n += socket.Send(&i32, sizeof(i32));

    return OK;
}

int slsReceiverTCPIPInterface::get_id(Interface &socket) {
    return socket.sendResult(getReceiverVersion());
}

int slsReceiverTCPIPInterface::set_detector_type(Interface &socket) {
    auto arg = socket.Receive<detectorType>();
    // set
    if (arg >= 0) {
        // if object exists, verify unlocked and idle, else only verify lock
        // (connecting first time)
        if (receiver != nullptr) {
            VerifyIdle(socket);
        }
        switch (arg) {
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

        if (receiver == nullptr) {
            receiver = sls::make_unique<slsReceiverImplementation>();
        }
        myDetectorType = arg;
        if (impl()->setDetectorType(myDetectorType) == FAIL) {
            throw RuntimeError("Could not set detector type");
        }

        // callbacks after (in setdetectortype, the object is reinitialized)
        if (startAcquisitionCallBack != nullptr)
            impl()->registerCallBackStartAcquisition(startAcquisitionCallBack,
                                                     pStartAcquisition);
        if (acquisitionFinishedCallBack != nullptr)
            impl()->registerCallBackAcquisitionFinished(
                acquisitionFinishedCallBack, pAcquisitionFinished);
        if (rawDataReadyCallBack != nullptr)
            impl()->registerCallBackRawDataReady(rawDataReadyCallBack,
                                                 pRawDataReady);
        if (rawDataModifyReadyCallBack != nullptr)
            impl()->registerCallBackRawDataModifyReady(
                rawDataModifyReadyCallBack, pRawDataReady);
    }
    return socket.sendResult(myDetectorType);
}

int slsReceiverTCPIPInterface::set_detector_hostname(Interface &socket) {
    char hostname[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    socket.Receive(hostname);

    if (strlen(hostname) != 0) {
        VerifyIdle(socket);
        impl()->setDetectorHostname(hostname);
    }
    auto s = impl()->getDetectorHostname();
    sls::strcpy_safe(retval, s.c_str());
    if (s.empty()) {
        throw RuntimeError("Hostname not set");
    }
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_roi(Interface &socket) {
    static_assert(sizeof(ROI) == 2 * sizeof(int), "ROI not packed");
    ROI arg;
    socket.Receive(arg);
    FILE_LOG(logDEBUG1) << "Set ROI: [" << arg.xmin << ", " << arg.xmax << "]";

    if (myDetectorType != GOTTHARD)
        functionNotImplemented();

    VerifyIdle(socket);
    if (impl()->setROI(arg) == FAIL)
        throw RuntimeError("Could not set ROI");
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::set_timer(Interface &socket) {
    auto index = socket.Receive<int64_t>();
    auto value = socket.Receive<int64_t>();
    if (value >= 0) {
        FILE_LOG(logDEBUG1)
            << "Setting timer index " << index << " to " << value;
        switch (index) {
        case ACQUISITION_TIME:
            ret = impl()->setAcquisitionTime(value);
            break;
        case FRAME_PERIOD:
            ret = impl()->setAcquisitionPeriod(value);
            break;
        case FRAME_NUMBER:
        case CYCLES_NUMBER:
        case STORAGE_CELL_NUMBER:
            impl()->setNumberOfFrames(value);
            break;
        case SUBFRAME_ACQUISITION_TIME:
            impl()->setSubExpTime(value);
            break;
        case SUBFRAME_DEADTIME:
            impl()->setSubPeriod(value + impl()->getSubExpTime());
            break;
        case ANALOG_SAMPLES:
            if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
                modeNotImplemented("(Analog Samples) Timer index",
                                   static_cast<int>(index));
                break;
            }
            impl()->setNumberofAnalogSamples(value);
            break;
        case DIGITAL_SAMPLES:
            if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
                modeNotImplemented("(Digital Samples) Timer index",
                                   static_cast<int>(index));
                break;
            }
            impl()->setNumberofDigitalSamples(value);
            break;
        default:
            modeNotImplemented("Timer index", static_cast<int>(value));
            break;
        }
    }
    // get
    int64_t retval = -1;
    switch (index) {
    case ACQUISITION_TIME:
        retval = impl()->getAcquisitionTime();
        break;
    case FRAME_PERIOD:
        retval = impl()->getAcquisitionPeriod();
        break;
    case FRAME_NUMBER:
    case CYCLES_NUMBER:
    case STORAGE_CELL_NUMBER:
        retval = impl()->getNumberOfFrames();
        break;
    case SUBFRAME_ACQUISITION_TIME:
        retval = impl()->getSubExpTime();
        break;
    case SUBFRAME_DEADTIME:
        retval = impl()->getSubPeriod() - impl()->getSubExpTime();
        break;
    case ANALOG_SAMPLES:
        if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
            throw RuntimeError("This timer mode (" + std::to_string(index) +
                               ") does not exist for this receiver type");
        }
        retval = impl()->getNumberofAnalogSamples();
        break;
    case DIGITAL_SAMPLES:
        if (myDetectorType != CHIPTESTBOARD && myDetectorType != MOENCH) {
            throw RuntimeError("This timer mode (" + std::to_string(index) +
                               ") does not exist for this receiver type");
        }
        retval = impl()->getNumberofDigitalSamples();
        break;
    default:
        modeNotImplemented("Timer index", static_cast<int>(index));
        break;
    }
    validate(value, retval, "set timer", DEC);
    FILE_LOG(logDEBUG1) << slsDetectorDefs::getTimerType((timerIndex)(index))
                        << ":" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_dynamic_range(Interface &socket) {
    auto dr = socket.Receive<int>();
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
        } else {
            ret = impl()->setDynamicRange(dr);
            if (ret == FAIL) {
                throw RuntimeError("Could not allocate memory for fifo or "
                                   "could not start listening/writing threads");
            }
        }
    }
    int retval = impl()->getDynamicRange();
    validate(dr, retval, "set dynamic range", DEC);
    FILE_LOG(logDEBUG1) << "dynamic range: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_streaming_frequency(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting streaming frequency: " << index;
        ret = impl()->setStreamingFrequency(index);
        if (ret == FAIL) {
            throw RuntimeError("Could not allocate memory for listening fifo");
        }
    }
    int retval = impl()->getStreamingFrequency();
    validate(index, retval, "set streaming frequency", DEC);
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::get_status(Interface &socket) {
    auto retval = impl()->getStatus();
    FILE_LOG(logDEBUG1) << "Status:" << runStatusType(retval);
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::start_receiver(Interface &socket) {
    runStatus status = impl()->getStatus();
    if (status != IDLE) {
        throw RuntimeError("Cannot start Receiver as it is: " +
                           runStatusType(status));
    } else {
        FILE_LOG(logDEBUG1) << "Starting Receiver";
        ret = impl()->startReceiver(mess);
        if (ret == FAIL) {
            throw RuntimeError(mess);
        }
    }
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::stop_receiver(Interface &socket) {
    if (impl()->getStatus() != IDLE) {
        FILE_LOG(logDEBUG1) << "Stopping Receiver";
        impl()->stopReceiver();
    }
    auto s = impl()->getStatus();
    if (s != IDLE)
        throw RuntimeError("Could not stop receiver. It as it is: " +
                           runStatusType(s));

    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::set_file_dir(Interface &socket) {
    char fPath[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    socket.Receive(fPath);

    if (strlen(fPath) != 0) {
        FILE_LOG(logDEBUG1) << "Setting file path: " << fPath;
        if(fPath[0] != '/')
            throw RuntimeError("Receiver path needs to be absolute path");
        impl()->setFilePath(fPath);
    }
    std::string s = impl()->getFilePath();
    sls::strcpy_safe(retval, s.c_str());
    if ((s.empty()) || (strlen(fPath) && strcasecmp(fPath, retval)))
        throw RuntimeError("Receiver file path does not exist");
    else
        FILE_LOG(logDEBUG1) << "file path:" << retval;

    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_file_name(Interface &socket) {
    char fName[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    socket.Receive(fName);
    if (strlen(fName) != 0) {
        FILE_LOG(logDEBUG1) << "Setting file name: " << fName;
        impl()->setFileName(fName);
    }
    std::string s = impl()->getFileName();
    if (s.empty())
        throw RuntimeError("file name is empty");

    sls::strcpy_safe(retval, s.c_str());
    FILE_LOG(logDEBUG1) << "file name:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_file_index(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting file index: " << index;
        impl()->setFileIndex(index);
    }
    int retval = impl()->getFileIndex();
    validate(index, retval, "set file index", DEC);
    FILE_LOG(logDEBUG1) << "file index:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::get_frame_index(Interface &socket) {
    uint64_t retval = impl()->getAcquisitionIndex();
    FILE_LOG(logDEBUG1) << "frame index:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::get_frames_caught(Interface &socket) {
    int retval = impl()->getTotalFramesCaught();
    FILE_LOG(logDEBUG1) << "frames caught:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::reset_frames_caught(Interface &socket) {
    FILE_LOG(logDEBUG1) << "Reset frames caught";
    impl()->resetAcquisitionCount();
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::enable_file_write(Interface &socket) {
    auto enable = socket.Receive<int>();
    if (enable >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting File write enable:" << enable;
        impl()->setFileWriteEnable(enable);
    }
    int retval = impl()->getFileWriteEnable();
    validate(enable, retval, "set file write enable", DEC);
    FILE_LOG(logDEBUG1) << "file write enable:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::enable_master_file_write(Interface &socket) {
    auto enable = socket.Receive<int>();
    if (enable >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting Master File write enable:" << enable;
        impl()->setMasterFileWriteEnable(enable);
    }
    int retval = impl()->getMasterFileWriteEnable();
    validate(enable, retval, "set master file write enable", DEC);
    FILE_LOG(logDEBUG1) << "master file write enable:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::enable_overwrite(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting File overwrite enable:" << index;
        impl()->setOverwriteEnable(index);
    }
    int retval = impl()->getOverwriteEnable();
    validate(index, retval, "set file overwrite enable", DEC);
    FILE_LOG(logDEBUG1) << "file overwrite enable:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::enable_tengiga(Interface &socket) {
    auto val = socket.Receive<int>();
    if (myDetectorType != EIGER && myDetectorType != CHIPTESTBOARD &&
        myDetectorType != MOENCH)
        functionNotImplemented();

    if (val >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting 10GbE:" << val;
        ret = impl()->setTenGigaEnable(val);
    }
    int retval = impl()->getTenGigaEnable();
    validate(val, retval, "set 10GbE", DEC);
    FILE_LOG(logDEBUG1) << "10Gbe:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_fifo_depth(Interface &socket) {
    auto value = socket.Receive<int>();
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
    auto enable = socket.Receive<int>();
    if (myDetectorType != EIGER)
        functionNotImplemented();

    if (enable >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting activate:" << enable;
        impl()->setActivate(static_cast<bool>(enable));
    }
    auto retval = static_cast<int>(impl()->getActivate());
    validate(enable, retval, "set activate", DEC);
    FILE_LOG(logDEBUG1) << "Activate: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_data_stream_enable(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting data stream enable:" << index;
        impl()->setDataStreamEnable(index);
    }
    auto retval = static_cast<int>(impl()->getDataStreamEnable());
    validate(index, retval, "set data stream enable", DEC);
    FILE_LOG(logDEBUG1) << "data streaming enable:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_streaming_timer(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting streaming timer:" << index;
        impl()->setStreamingTimer(index);
    }
    int retval = impl()->getStreamingTimer();
    validate(index, retval, "set data stream timer", DEC);
    FILE_LOG(logDEBUG1) << "Streaming timer:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_flipped_data(Interface &socket) {
    // TODO! Why 2 args?
    memset(mess, 0, sizeof(mess));
    auto arg = socket.Receive<int>();

    if (myDetectorType != EIGER)
        functionNotImplemented();

    if (arg >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting flipped data:" << arg;
        impl()->setFlippedDataX(arg);
    }
    int retval = impl()->getFlippedDataX();
    validate(arg, retval, std::string("set flipped data"), DEC);
    FILE_LOG(logDEBUG1) << "Flipped Data:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_file_format(Interface &socket) {
    fileFormat f = GET_FILE_FORMAT;
    socket.Receive(f);
    if (f >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting file format:" << f;
        impl()->setFileFormat(f);
    }
    auto retval = impl()->getFileFormat();
    validate(f, retval, "set file format", DEC);
    FILE_LOG(logDEBUG1) << "File Format: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_detector_posid(Interface &socket) {
    auto arg = socket.Receive<int>();
    if (arg >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting detector position id:" << arg;
        impl()->setDetectorPositionId(arg);
    }
    auto retval = impl()->getDetectorPositionId();
    validate(arg, retval, "set detector position id", DEC);
    FILE_LOG(logDEBUG1) << "Position Id:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_multi_detector_size(Interface &socket) {
    int arg[]{-1, -1};
    socket.Receive(arg);
    if ((arg[0] > 0) && (arg[1] > 0)) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1)
            << "Setting multi detector size:" << arg[0] << "," << arg[1];
        impl()->setMultiDetectorSize(arg);
    }
    int *temp = impl()->getMultiDetectorSize(); // TODO! return by value!
    int retval = temp[0] * temp[1];
    FILE_LOG(logDEBUG1) << "Multi Detector Size:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_streaming_port(Interface &socket) {
    auto port = socket.Receive<int>();
    if (port >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting streaming port:" << port;
        impl()->setStreamingPort(port);
    }
    int retval = impl()->getStreamingPort();
    validate(port, retval, "set streaming port", DEC);
    FILE_LOG(logDEBUG1) << "streaming port:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_streaming_source_ip(Interface &socket) {
    char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    socket.Receive(arg);
    VerifyIdle(socket);
    FILE_LOG(logDEBUG1) << "Setting streaming source ip:" << arg;
    impl()->setStreamingSourceIP(arg);
    sls::strcpy_safe(retval, impl()->getStreamingSourceIP().c_str());
    FILE_LOG(logDEBUG1) << "streaming source ip:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_silent_mode(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting silent mode:" << value;
        impl()->setSilentMode(value);
    }
    auto retval = static_cast<int>(impl()->getSilentMode());
    validate(value, retval, "set silent mode", DEC);
    FILE_LOG(logDEBUG1) << "silent mode:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::enable_gap_pixels(Interface &socket) {
    auto enable = socket.Receive<int>();
    if (myDetectorType != EIGER)
        functionNotImplemented();

    if (enable >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting gap pixels enable:" << enable;
        impl()->setGapPixelsEnable(static_cast<bool>(enable));
    }
    auto retval = static_cast<int>(impl()->getGapPixelsEnable());
    validate(enable, retval, "set gap pixels enable", DEC);
    FILE_LOG(logDEBUG1) << "Gap Pixels Enable: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::restream_stop(Interface &socket) {
    VerifyIdle(socket);
    if (!impl()->getDataStreamEnable()) {
        throw RuntimeError(
            "Could not restream stop packet as data Streaming is disabled");
    } else {
        FILE_LOG(logDEBUG1) << "Restreaming stop";
        impl()->restreamStop();
    }
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::set_additional_json_header(Interface &socket) {
    memset(mess, 0, sizeof(mess));
    char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    socket.Receive(arg);
    VerifyIdle(socket);
    FILE_LOG(logDEBUG1) << "Setting additional json header: " << arg;
    impl()->setAdditionalJsonHeader(arg);
    sls::strcpy_safe(retval, impl()->getAdditionalJsonHeader().c_str());
    FILE_LOG(logDEBUG1) << "additional json header:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::get_additional_json_header(Interface &socket) {
    char retval[MAX_STR_LENGTH]{};
    sls::strcpy_safe(retval, impl()->getAdditionalJsonHeader().c_str());
    FILE_LOG(logDEBUG1) << "additional json header:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_udp_socket_buffer_size(Interface &socket) {
    auto index = socket.Receive<int64_t>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting UDP Socket Buffer size: " << index;
        if (impl()->setUDPSocketBufferSize(index) == FAIL) {
            throw RuntimeError(
                "Could not create dummy UDP Socket to test buffer size");
        }
    }
    int64_t retval = impl()->getUDPSocketBufferSize();
    if (index != 0)
        validate(index, retval,
                 "set udp socket buffer size (No CAP_NET_ADMIN privileges?)",
                 DEC);
    FILE_LOG(logDEBUG1) << "UDP Socket Buffer Size:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::get_real_udp_socket_buffer_size(
    Interface &socket) {
    auto size = impl()->getActualUDPSocketBufferSize();
    FILE_LOG(logDEBUG1) << "Actual UDP socket size :" << size;
    return socket.sendResult(size);
}

int slsReceiverTCPIPInterface::set_frames_per_file(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting frames per file: " << index;
        impl()->setFramesPerFile(index);
    }
    auto retval = static_cast<int>(impl()->getFramesPerFile());
    validate(index, retval, "set frames per file", DEC);
    FILE_LOG(logDEBUG1) << "frames per file:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::check_version_compatibility(Interface &socket) {
    auto arg = socket.Receive<int64_t>();
    FILE_LOG(logDEBUG1) << "Checking versioning compatibility with value "
                        << arg;
    int64_t client_requiredVersion = arg;
    int64_t rx_apiVersion = APIRECEIVER;
    int64_t rx_version = getReceiverVersion();

    if (rx_apiVersion > client_requiredVersion) {
        std::ostringstream os;
        os << "Incompatible versions.\n Client's receiver API Version: (0x"
           << std::hex << client_requiredVersion
           << "). Receiver API Version: (0x" << std::hex
           << ").\n Please update the client!\n";
        throw RuntimeError(os.str());
    } else if (client_requiredVersion > rx_version) {
        std::ostringstream os;
        os << "This receiver is incompatible.\n Receiver Version: (0x"
           << std::hex << rx_version << "). Client's receiver API Version: (0x"
           << std::hex << client_requiredVersion
           << ").\n Please update the receiver";
        throw RuntimeError(os.str());
    } else {
        FILE_LOG(logINFO) << "Compatibility with Client: Successful";
    }
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::set_discard_policy(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting frames discard policy: " << index;
        impl()->setFrameDiscardPolicy(static_cast<frameDiscardPolicy>(index));
    }
    int retval = impl()->getFrameDiscardPolicy();
    validate(index, retval, "set discard policy", DEC);
    FILE_LOG(logDEBUG1) << "frame discard policy:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_padding_enable(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting frames padding enable: " << index;
        impl()->setFramePaddingEnable(static_cast<bool>(index));
    }
    auto retval = static_cast<int>(impl()->getFramePaddingEnable());
    validate(index, retval, "set frame padding enable", DEC);
    FILE_LOG(logDEBUG1) << "Frame Padding Enable:" << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_deactivated_padding_enable(
    Interface &socket) {
    auto enable = socket.Receive<int>();
    if (myDetectorType != EIGER)
        functionNotImplemented();

    if (enable >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting deactivated padding enable: " << enable;
        impl()->setDeactivatedPadding(enable > 0);
    }
    auto retval = static_cast<int>(impl()->getDeactivatedPadding());
    validate(enable, retval, "set deactivated padding enable", DEC);
    FILE_LOG(logDEBUG1) << "Deactivated Padding Enable: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_readout_mode(Interface &socket) {
    auto arg = socket.Receive<readoutMode>();

    if (myDetectorType != CHIPTESTBOARD)
        functionNotImplemented();

    if (arg >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting readout mode: " << arg;
        impl()->setReadoutMode(arg);
    }
    auto retval = impl()->getReadoutMode();
    validate(static_cast<int>(arg), static_cast<int>(retval),
             "set readout mode", DEC);
    FILE_LOG(logDEBUG1) << "Readout mode: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_adc_mask(Interface &socket) {
    auto arg = socket.Receive<uint32_t>();
    VerifyIdle(socket);
    FILE_LOG(logDEBUG1) << "Setting ADC enable mask: " << arg;
    impl()->setADCEnableMask(arg);
    auto retval = impl()->getADCEnableMask();
    if (retval != arg) {
        std::ostringstream os;
        os << "Could not ADC enable mask. Set 0x" << std::hex << arg
           << " but read 0x" << std::hex << retval;
        throw RuntimeError(os.str());
    }
    FILE_LOG(logDEBUG1) << "ADC enable mask retval: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_dbit_list(Interface &socket) {
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> args;
    socket.Receive(args);
    FILE_LOG(logDEBUG1) << "Setting DBIT list";
    for (auto &it : args) {
        FILE_LOG(logDEBUG1) << it << " ";
    }
    FILE_LOG(logDEBUG1) << "\n";
    VerifyIdle(socket);
    impl()->setDbitList(args);
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::get_dbit_list(Interface &socket) {
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> retval;
    retval = impl()->getDbitList();
    FILE_LOG(logDEBUG1) << "Dbit list size retval:" << retval.size();
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_dbit_offset(Interface &socket) {
    auto arg = socket.Receive<int>();
    if (arg >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting Dbit offset: " << arg;
        impl()->setDbitOffset(arg);
    }
    int retval = impl()->getDbitOffset();
    validate(arg, retval, "set dbit offset", DEC);
    FILE_LOG(logDEBUG1) << "Dbit offset retval: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_quad_type(Interface &socket) {
    auto quadEnable = socket.Receive<int>();
    if (quadEnable >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting quad:" << quadEnable;
        ret = impl()->setQuad(quadEnable == 0 ? false : true);
        if (ret == FAIL) {
            throw RuntimeError("Could not set Quad due to fifo structure");
        } 
    }
    int retval = impl()->getQuad() ? 1 : 0;
    validate(quadEnable, retval, "set quad", DEC);
    FILE_LOG(logDEBUG1) << "quad retval:" << retval;
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::set_read_n_lines(Interface &socket) {
    auto arg = socket.Receive<int>();
    if (arg >= 0) {
        VerifyIdle(socket);
        FILE_LOG(logDEBUG1) << "Setting Read N Lines:" << arg;
        impl()->setReadNLines(arg);
    }
    int retval = impl()->getReadNLines();
    validate(arg, retval, "set read n lines", DEC);
    FILE_LOG(logDEBUG1) << "read n lines retval:" << retval;
    return socket.Send(OK);
}


int slsReceiverTCPIPInterface::set_udp_ip(Interface &socket) {
    auto arg = socket.Receive<sls::IpAddr>();
    VerifyIdle(socket);
    FILE_LOG(logINFO) << "Received UDP IP: " << arg;
    // getting eth
    std::string eth = sls::IpToInterfaceName(arg.str());
    if (eth == "none") {
        throw RuntimeError("Failed to get udp ethernet interface from IP " + arg.str());
    }   
    if (eth.find('.') == std::string::npos) {
        eth = "";
        FILE_LOG(logERROR) << "Failed to get udp ethernet interface from IP " << arg << ". Got " << eth;
    }   
    impl()->setEthernetInterface(eth);
    if (myDetectorType == EIGER) {
        impl()->setEthernetInterface2(eth);
    }
    // get mac address
    auto retval = sls::InterfaceNameToMac(eth);
    if (retval == 0) {
        throw RuntimeError("Failed to get udp mac adddress to listen to\n");
    }
    FILE_LOG(logINFO) << "Receiver MAC Address: " << retval;
    return socket.sendResult(retval);
}


int slsReceiverTCPIPInterface::set_udp_ip2(Interface &socket) {
    auto arg = socket.Receive<sls::IpAddr>();
    VerifyIdle(socket);
    if (myDetectorType != JUNGFRAU) {
        throw RuntimeError("UDP Destination IP2 not implemented for this detector");
    }
    FILE_LOG(logINFO) << "Received UDP IP2: " << arg;
    // getting eth
    std::string eth = sls::IpToInterfaceName(arg.str());
    if (eth == "none") {
        throw RuntimeError("Failed to get udp ethernet interface2 from IP " + arg.str());
    }   
    if (eth.find('.') == std::string::npos) {
        eth = "";
        FILE_LOG(logERROR) << "Failed to get udp ethernet interface2 from IP " << arg << ". Got " << eth;
    }   
    impl()->setEthernetInterface2(eth);

    // get mac address
    auto retval = sls::InterfaceNameToMac(eth);
    if (retval == 0) {
        throw RuntimeError("Failed to get udp mac adddress2 to listen to\n");
    }
    FILE_LOG(logINFO) << "Receiver MAC Address2: " << retval;
    return socket.sendResult(retval);
}

int slsReceiverTCPIPInterface::set_udp_port(Interface &socket) {
    auto arg = socket.Receive<int>();
    VerifyIdle(socket);
    FILE_LOG(logDEBUG1) << "Setting UDP Port:" << arg;
    impl()->setUDPPortNumber(arg);
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::set_udp_port2(Interface &socket) {
    auto arg = socket.Receive<int>();
    VerifyIdle(socket);
    if (myDetectorType != JUNGFRAU && myDetectorType != EIGER) {
        throw RuntimeError("UDP Destination Port2 not implemented for this detector");
    }    
    FILE_LOG(logDEBUG1) << "Setting UDP Port:" << arg;
    impl()->setUDPPortNumber2(arg);
    return socket.Send(OK);
}

int slsReceiverTCPIPInterface::set_num_interfaces(Interface &socket) {
    auto arg = socket.Receive<int>();
    arg = (arg > 1 ? 2 : 1);
    VerifyIdle(socket);
    if (myDetectorType != JUNGFRAU) {
        throw RuntimeError("Number of interfaces not implemented for this detector");
    }    
    FILE_LOG(logDEBUG1) << "Setting Number of UDP Interfaces:" << arg;
    if (impl()->setNumberofUDPInterfaces(arg) == FAIL) {
        throw RuntimeError("Failed to set number of interfaces");
    }
    return socket.Send(OK);
}