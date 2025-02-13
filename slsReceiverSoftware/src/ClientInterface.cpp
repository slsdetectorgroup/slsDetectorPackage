// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "ClientInterface.h"
#include "sls/ServerSocket.h"
#include "sls/StaticVector.h"
#include "sls/ToString.h"

#include "sls/sls_detector_exceptions.h"
#include "sls/string_utils.h"
#include "sls/versionAPI.h"

#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace sls {

using ns = std::chrono::nanoseconds;
using Interface = ServerInterface;

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

std::mutex ClientInterface::callbackMutex;

ClientInterface::~ClientInterface() {
    killTcpThread = true;
    LOG(logINFO) << "Shutting down TCP Socket on port " << portNumber;
    server.shutdown();
    LOG(logDEBUG) << "TCP Socket closed on port " << portNumber;
    tcpThread->join();
}

ClientInterface::ClientInterface(uint16_t portNumber)
    : detType(GOTTHARD), portNumber(portNumber), server(portNumber) {
    validatePortNumber(portNumber);
    functionTable();
    parentThreadId = gettid();
    tcpThread =
        make_unique<std::thread>(&ClientInterface::startTCPServer, this);
}

std::string ClientInterface::getReceiverVersion() { return APIRECEIVER; }

/***callback functions***/
void ClientInterface::registerCallBackStartAcquisition(
    int (*func)(const startCallbackHeader, void *), void *arg) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    startAcquisitionCallBack = func;
    pStartAcquisition = arg;
}

void ClientInterface::registerCallBackAcquisitionFinished(
    void (*func)(const endCallbackHeader, void *), void *arg) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    acquisitionFinishedCallBack = func;
    pAcquisitionFinished = arg;
}

void ClientInterface::registerCallBackRawDataReady(
    void (*func)(sls_receiver_header &, dataCallbackHeader, char *, size_t &,
                 void *),
    void *arg) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
}

void ClientInterface::startTCPServer() {
    tcpThreadId = gettid();
    LOG(logINFOBLUE) << "Created [ TCP server Tid: " << tcpThreadId << "]";
    LOG(logINFO) << "SLS Receiver starting TCP Server on port " << portNumber
                 << '\n';

    while (!killTcpThread) {
        LOG(logDEBUG1) << "Start accept loop";
        try {
            auto socket = server.accept();
            try {
                verifyLock(); // lock should be checked only for set (not get),
                              // Move it back?
                ret = decodeFunction(socket);
            } catch (const RuntimeError &e) {
                // We had an error needs to be sent to client
                char mess[MAX_STR_LENGTH]{};
                strcpy_safe(mess, e.what());
                socket.Send(FAIL);
                socket.Send(mess);
            }
            // if tcp command was to exit server
            if (ret == GOODBYE) {
                break;
            }
        } catch (const RuntimeError &e) {
            LOG(logERROR) << "Accept failed";
        }
    }

    if (receiver) {
        receiver->shutDownUDPSockets();
    }
    LOG(logINFOBLUE) << "Exiting [ TCP server Tid: " << tcpThreadId << "]";
}

// clang-format off
int ClientInterface::functionTable(){
	flist[F_LOCK_RECEIVER]					=	&ClientInterface::lock_receiver;
	flist[F_GET_LAST_RECEIVER_CLIENT_IP]	=	&ClientInterface::get_last_client_ip;
	flist[F_GET_RECEIVER_VERSION]			=	&ClientInterface::get_version;
	flist[F_SETUP_RECEIVER]				    =	&ClientInterface::setup_receiver;
	flist[F_RECEIVER_SET_DETECTOR_ROI]		=	&ClientInterface::set_detector_roi;
	flist[F_RECEIVER_SET_NUM_FRAMES]        =   &ClientInterface::set_num_frames;  
	flist[F_SET_RECEIVER_NUM_TRIGGERS]      =   &ClientInterface::set_num_triggers;           
	flist[F_SET_RECEIVER_NUM_BURSTS]        =   &ClientInterface::set_num_bursts;         
	flist[F_SET_RECEIVER_NUM_ADD_STORAGE_CELLS] = &ClientInterface::set_num_add_storage_cells;                 
	flist[F_SET_RECEIVER_TIMING_MODE]       =   &ClientInterface::set_timing_mode;          
	flist[F_SET_RECEIVER_BURST_MODE]        =   &ClientInterface::set_burst_mode;  
	flist[F_RECEIVER_SET_NUM_ANALOG_SAMPLES]=   &ClientInterface::set_num_analog_samples;            
	flist[F_RECEIVER_SET_NUM_DIGITAL_SAMPLES]=  &ClientInterface::set_num_digital_samples;           
	flist[F_RECEIVER_SET_EXPTIME]           =   &ClientInterface::set_exptime;
	flist[F_RECEIVER_SET_PERIOD]            =   &ClientInterface::set_period;
	flist[F_RECEIVER_SET_SUB_EXPTIME]       =   &ClientInterface::set_subexptime;    
	flist[F_RECEIVER_SET_SUB_DEADTIME]      =   &ClientInterface::set_subdeadtime;    
	flist[F_SET_RECEIVER_DYNAMIC_RANGE]		= 	&ClientInterface::set_dynamic_range;
	flist[F_SET_RECEIVER_STREAMING_FREQUENCY] = 	&ClientInterface::set_streaming_frequency;
	flist[F_GET_RECEIVER_STREAMING_FREQUENCY] = 	&ClientInterface::get_streaming_frequency;
	flist[F_GET_RECEIVER_STATUS]			=	&ClientInterface::get_status;
	flist[F_START_RECEIVER]					=	&ClientInterface::start_receiver;
	flist[F_STOP_RECEIVER]					=	&ClientInterface::stop_receiver;
	flist[F_SET_RECEIVER_FILE_PATH]			=	&ClientInterface::set_file_dir;
	flist[F_GET_RECEIVER_FILE_PATH]			=	&ClientInterface::get_file_dir;
	flist[F_SET_RECEIVER_FILE_NAME]			=	&ClientInterface::set_file_name;
	flist[F_GET_RECEIVER_FILE_NAME]			=	&ClientInterface::get_file_name;
	flist[F_SET_RECEIVER_FILE_INDEX]		=	&ClientInterface::set_file_index;
	flist[F_GET_RECEIVER_FILE_INDEX]		=	&ClientInterface::get_file_index;
	flist[F_GET_RECEIVER_FRAME_INDEX]		=	&ClientInterface::get_frame_index;    
	flist[F_GET_RECEIVER_FRAMES_CAUGHT]		=	&ClientInterface::get_frames_caught;
    flist[F_GET_NUM_MISSING_PACKETS]		=	&ClientInterface::get_missing_packets;
	flist[F_SET_RECEIVER_FILE_WRITE]		=	&ClientInterface::set_file_write;
	flist[F_GET_RECEIVER_FILE_WRITE]		=	&ClientInterface::get_file_write;
	flist[F_SET_RECEIVER_MASTER_FILE_WRITE]	=	&ClientInterface::set_master_file_write;
	flist[F_GET_RECEIVER_MASTER_FILE_WRITE]	=	&ClientInterface::get_master_file_write;
	flist[F_SET_RECEIVER_OVERWRITE]		    = 	&ClientInterface::set_overwrite;
	flist[F_GET_RECEIVER_OVERWRITE]		    = 	&ClientInterface::get_overwrite;
	flist[F_ENABLE_RECEIVER_TEN_GIGA]		= 	&ClientInterface::enable_tengiga;
	flist[F_SET_RECEIVER_FIFO_DEPTH]		= 	&ClientInterface::set_fifo_depth;
	flist[F_RECEIVER_ACTIVATE]				= 	&ClientInterface::set_activate;
	flist[F_SET_RECEIVER_STREAMING]		    = 	&ClientInterface::set_streaming;
	flist[F_GET_RECEIVER_STREAMING]		    = 	&ClientInterface::get_streaming;
	flist[F_RECEIVER_STREAMING_TIMER]		= 	&ClientInterface::set_streaming_timer;
	flist[F_GET_FLIP_ROWS_RECEIVER]		    = 	&ClientInterface::get_flip_rows;
	flist[F_SET_FLIP_ROWS_RECEIVER]		    = 	&ClientInterface::set_flip_rows;
	flist[F_SET_RECEIVER_FILE_FORMAT]		= 	&ClientInterface::set_file_format;
	flist[F_GET_RECEIVER_FILE_FORMAT]		= 	&ClientInterface::get_file_format;
	flist[F_SET_RECEIVER_STREAMING_PORT]	= 	&ClientInterface::set_streaming_port;
	flist[F_GET_RECEIVER_STREAMING_PORT]	= 	&ClientInterface::get_streaming_port;
	flist[F_SET_RECEIVER_SILENT_MODE]		= 	&ClientInterface::set_silent_mode;
	flist[F_GET_RECEIVER_SILENT_MODE]		= 	&ClientInterface::get_silent_mode;
	flist[F_RESTREAM_STOP_FROM_RECEIVER]	= 	&ClientInterface::restream_stop;
	flist[F_SET_ADDITIONAL_JSON_HEADER]     =   &ClientInterface::set_additional_json_header;
	flist[F_GET_ADDITIONAL_JSON_HEADER]     =   &ClientInterface::get_additional_json_header;
    flist[F_RECEIVER_UDP_SOCK_BUF_SIZE]     =   &ClientInterface::set_udp_socket_buffer_size;
    flist[F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE]=   &ClientInterface::get_real_udp_socket_buffer_size;
    flist[F_SET_RECEIVER_FRAMES_PER_FILE]	=   &ClientInterface::set_frames_per_file;
    flist[F_GET_RECEIVER_FRAMES_PER_FILE]	=   &ClientInterface::get_frames_per_file;
    flist[F_SET_RECEIVER_DISCARD_POLICY]	=   &ClientInterface::set_discard_policy;
    flist[F_GET_RECEIVER_DISCARD_POLICY]	=   &ClientInterface::get_discard_policy;
	flist[F_SET_RECEIVER_PADDING]		    =   &ClientInterface::set_padding_enable;
	flist[F_GET_RECEIVER_PADDING]		    =   &ClientInterface::get_padding_enable;
	flist[F_RECEIVER_SET_READOUT_MODE] 	    = 	&ClientInterface::set_readout_mode;
	flist[F_RECEIVER_SET_ADC_MASK]			=	&ClientInterface::set_adc_mask;
	flist[F_SET_RECEIVER_DBIT_LIST]			=	&ClientInterface::set_dbit_list;
	flist[F_GET_RECEIVER_DBIT_LIST]			=	&ClientInterface::get_dbit_list;
	flist[F_SET_RECEIVER_DBIT_OFFSET]		= 	&ClientInterface::set_dbit_offset;
	flist[F_GET_RECEIVER_DBIT_OFFSET]		= 	&ClientInterface::get_dbit_offset;
    flist[F_SET_RECEIVER_QUAD]			    = 	&ClientInterface::set_quad_type;
    flist[F_SET_RECEIVER_READ_N_ROWS]       =   &ClientInterface::set_read_n_rows;
    flist[F_SET_RECEIVER_UDP_IP]            =   &ClientInterface::set_udp_ip;
	flist[F_SET_RECEIVER_UDP_IP2]           =   &ClientInterface::set_udp_ip2;
	flist[F_SET_RECEIVER_UDP_PORT]          =   &ClientInterface::set_udp_port;
	flist[F_SET_RECEIVER_UDP_PORT2]         =   &ClientInterface::set_udp_port2;
	flist[F_SET_RECEIVER_NUM_INTERFACES]    =   &ClientInterface::set_num_interfaces;
	flist[F_RECEIVER_SET_ADC_MASK_10G]		=	&ClientInterface::set_adc_mask_10g;
    flist[F_RECEIVER_SET_COUNTER_MASK]      =   &ClientInterface::set_counter_mask;
    flist[F_INCREMENT_FILE_INDEX]           =   &ClientInterface::increment_file_index;
    flist[F_SET_ADDITIONAL_JSON_PARAMETER]  =   &ClientInterface::set_additional_json_parameter;
	flist[F_GET_ADDITIONAL_JSON_PARAMETER]  =   &ClientInterface::get_additional_json_parameter;
	flist[F_GET_RECEIVER_PROGRESS]          =   &ClientInterface::get_progress;
    flist[F_SET_RECEIVER_NUM_GATES]         =   &ClientInterface::set_num_gates;    
    flist[F_SET_RECEIVER_GATE_DELAY]        =   &ClientInterface::set_gate_delay;        
    flist[F_GET_RECEIVER_THREAD_IDS]        =   &ClientInterface::get_thread_ids;
    flist[F_GET_RECEIVER_STREAMING_START_FNUM] = &ClientInterface::get_streaming_start_fnum;
    flist[F_SET_RECEIVER_STREAMING_START_FNUM] = &ClientInterface::set_streaming_start_fnum;
    flist[F_SET_RECEIVER_RATE_CORRECT]      =   &ClientInterface::set_rate_correct;
    flist[F_SET_RECEIVER_SCAN]              =   &ClientInterface::set_scan;
    flist[F_RECEIVER_SET_THRESHOLD]         =   &ClientInterface::set_threshold;
    flist[F_GET_RECEIVER_STREAMING_HWM]     =   &ClientInterface::get_streaming_hwm;
    flist[F_SET_RECEIVER_STREAMING_HWM]     =   &ClientInterface::set_streaming_hwm;
    flist[F_RECEIVER_SET_ALL_THRESHOLD]     =   &ClientInterface::set_all_threshold;
    flist[F_RECEIVER_SET_DATASTREAM]        =   &ClientInterface::set_detector_datastream;
    flist[F_GET_RECEIVER_ARPING]            =   &ClientInterface::get_arping;
    flist[F_SET_RECEIVER_ARPING]            =   &ClientInterface::set_arping;
    flist[F_RECEIVER_GET_RECEIVER_ROI]      =   &ClientInterface::get_receiver_roi;
    flist[F_RECEIVER_SET_RECEIVER_ROI]      =   &ClientInterface::set_receiver_roi;
    flist[F_RECEIVER_SET_RECEIVER_ROI_METADATA] =   &ClientInterface::set_receiver_roi_metadata;
    flist[F_RECEIVER_SET_NUM_TRANSCEIVER_SAMPLES] = &ClientInterface::set_num_transceiver_samples;
    flist[F_RECEIVER_SET_TRANSCEIVER_MASK]  =   &ClientInterface::set_transceiver_mask;
    flist[F_RECEIVER_SET_ROW]               =   &ClientInterface::set_row;
    flist[F_RECEIVER_SET_COLUMN]            =   &ClientInterface::set_column;    


	for (int i = NUM_DET_FUNCTIONS + 1; i < NUM_REC_FUNCTIONS ; i++) {
		LOG(logDEBUG1) << "function fnum: " << i << " (" <<
				getFunctionNameFromEnum((enum detFuncs)i) << ") located at " << flist[i];
	}

	return OK;
}
// clang-format on
int ClientInterface::decodeFunction(Interface &socket) {
    ret = FAIL;
    socket.Receive(fnum);
    socket.setFnum(fnum);
    if (fnum <= NUM_DET_FUNCTIONS || fnum >= NUM_REC_FUNCTIONS) {
        throw RuntimeError(UNRECOGNIZED_FNUM_ENUM + std::to_string(fnum));
    } else {
        LOG(logDEBUG1) << "calling function fnum: " << fnum << " ("
                       << getFunctionNameFromEnum((enum detFuncs)fnum) << ")";
        ret = (this->*flist[fnum])(socket);
        LOG(logDEBUG1) << "Function "
                       << getFunctionNameFromEnum((enum detFuncs)fnum)
                       << " finished";
    }
    return ret;
}

void ClientInterface::functionNotImplemented() {
    std::ostringstream os;
    os << "Function: " << getFunctionNameFromEnum((enum detFuncs)fnum)
       << " is not implemented for this detector";
    throw RuntimeError(os.str());
}

void ClientInterface::modeNotImplemented(const std::string &modename,
                                         int mode) {
    std::ostringstream os;
    os << modename << " (" << mode << ") is not implemented for this detector";
    throw RuntimeError(os.str());
}

template <typename T>
void ClientInterface::validate(T arg, T retval, const std::string &modename,
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

void ClientInterface::verifyLock() {
    if (lockedByClient && server.getThisClient() != server.getLockedBy()) {
        throw SocketError("Receiver locked\n");
    }
}

void ClientInterface::verifyIdle(Interface &socket) {
    if (impl()->getStatus() != IDLE) {
        std::ostringstream oss;
        oss << "Can not execute "
            << getFunctionNameFromEnum((enum detFuncs)fnum)
            << " when receiver is not idle";
        throw SocketError(oss.str());
    }
}

int ClientInterface::lock_receiver(Interface &socket) {
    auto lock = socket.Receive<int>();
    LOG(logDEBUG1) << "Locking Server to " << lock;
    if (lock >= 0) {
        if (!lockedByClient ||
            (server.getLockedBy() == server.getThisClient())) {
            lockedByClient = lock;
            lock ? server.setLockedBy(server.getThisClient())
                 : server.setLockedBy(IpAddr{});
        } else {
            throw RuntimeError("Receiver locked\n");
        }
    }
    return socket.sendResult(lockedByClient);
}

int ClientInterface::get_last_client_ip(Interface &socket) {
    return socket.sendResult(server.getLastClient());
}

int ClientInterface::get_version(Interface &socket) {
    auto version = getReceiverVersion();
    version.resize(MAX_STR_LENGTH);
    return socket.sendResult(version);
}

int ClientInterface::setup_receiver(Interface &socket) {
    auto arg = socket.Receive<rxParameters>();
    LOG(logDEBUG) << ToString(arg);

    MacAddr retvals[2];
    try {
        // if object exists, verify unlocked and idle, else only verify lock
        // (connecting first time)
        if (receiver != nullptr) {
            verifyIdle(socket);
        }

        // basic setup
        setDetectorType(arg.detType);
        impl()->setDetectorSize(arg.numberOfModule);
        impl()->setModulePositionId(arg.moduleIndex);
        impl()->setDetectorHostname(arg.hostname);

        // udp setup
        // update retvals only if detmac is not the same as in detector
        if (arg.udp_dstip != 0) {
            MacAddr r = setUdpIp(IpAddr(arg.udp_dstip));
            MacAddr detMac{arg.udp_dstmac};
            if (detMac != r) {
                retvals[0] = r;
            }
        }
        if (arg.udp_dstip2 != 0) {
            MacAddr r = setUdpIp2(IpAddr(arg.udp_dstip2));
            MacAddr detMac{arg.udp_dstmac2};
            if (detMac != r) {
                retvals[1] = r;
            }
        }

        impl()->setUDPPortNumber(arg.udp_dstport);
        impl()->setUDPPortNumber2(arg.udp_dstport2);
        if (detType == JUNGFRAU || detType == MOENCH || detType == GOTTHARD2) {
            impl()->setNumberofUDPInterfaces(arg.udpInterfaces);
        }
        impl()->setUDPSocketBufferSize(0);

        // acquisition parameters
        impl()->setNumberOfFrames(arg.frames);
        impl()->setNumberOfTriggers(arg.triggers);
        if (detType == GOTTHARD2) {
            impl()->setNumberOfBursts(arg.bursts);
        }
        if (detType == JUNGFRAU) {
            impl()->setNumberOfAdditionalStorageCells(
                arg.additionalStorageCells);
        }

        if (detType == CHIPTESTBOARD || detType == XILINX_CHIPTESTBOARD) {
            impl()->setNumberofAnalogSamples(arg.analogSamples);
            impl()->setNumberofDigitalSamples(arg.digitalSamples);
            impl()->setNumberofTransceiverSamples(arg.transceiverSamples);
        }
        if (detType != MYTHEN3) {
            impl()->setAcquisitionTime(std::chrono::nanoseconds(arg.expTimeNs));
        }
        impl()->setAcquisitionPeriod(std::chrono::nanoseconds(arg.periodNs));
        if (detType == EIGER) {
            impl()->setSubExpTime(std::chrono::nanoseconds(arg.subExpTimeNs));
            impl()->setSubPeriod(std::chrono::nanoseconds(arg.subExpTimeNs) +
                                 std::chrono::nanoseconds(arg.subDeadTimeNs));
            impl()->setActivate(static_cast<bool>(arg.activate));
            impl()->setDetectorDataStream(LEFT, arg.dataStreamLeft);
            impl()->setDetectorDataStream(RIGHT, arg.dataStreamRight);
            impl()->setQuad(arg.quad == 0 ? false : true);
            impl()->setThresholdEnergy(arg.thresholdEnergyeV[0]);
        }
        if (detType == EIGER || detType == JUNGFRAU || detType == MOENCH) {
            impl()->setReadNRows(arg.readNRows);
        }
        if (detType == MYTHEN3) {
            std::array<int, 3> val;
            for (int i = 0; i < 3; ++i) {
                val[i] = arg.thresholdEnergyeV[i];
            }
            impl()->setThresholdEnergy(val);
        }
        if (detType == EIGER || detType == MYTHEN3) {
            impl()->setDynamicRange(arg.dynamicRange);
        }
        impl()->setTimingMode(arg.timMode);
        if (detType == EIGER || detType == CHIPTESTBOARD ||
            detType == MYTHEN3) {
            impl()->setTenGigaEnable(arg.tenGiga);
        }
        if (detType == CHIPTESTBOARD || detType == XILINX_CHIPTESTBOARD) {
            impl()->setReadoutMode(arg.roMode);
            impl()->setTenGigaADCEnableMask(arg.adc10gMask);
            impl()->setTransceiverEnableMask(arg.transceiverMask);
        }
        if (detType == CHIPTESTBOARD) {
            impl()->setADCEnableMask(arg.adcMask);
        }
        if (detType == GOTTHARD) {
            impl()->setDetectorROI(arg.roi);
        }
        if (detType == MYTHEN3) {
            impl()->setCounterMask(arg.countermask);
            impl()->setAcquisitionTime1(
                std::chrono::nanoseconds(arg.expTime1Ns));
            impl()->setAcquisitionTime2(
                std::chrono::nanoseconds(arg.expTime2Ns));
            impl()->setAcquisitionTime3(
                std::chrono::nanoseconds(arg.expTime3Ns));
            impl()->setGateDelay1(std::chrono::nanoseconds(arg.gateDelay1Ns));
            impl()->setGateDelay2(std::chrono::nanoseconds(arg.gateDelay2Ns));
            impl()->setGateDelay3(std::chrono::nanoseconds(arg.gateDelay3Ns));
            impl()->setNumberOfGates(arg.gates);
        }
        if (detType == GOTTHARD2) {
            impl()->setBurstMode(arg.burstType);
        }
        impl()->setScan(arg.scanParams);
    } catch (std::exception &e) {
        throw RuntimeError("Could not setup receiver [" +
                           std::string(e.what()) + ']');
    }

    return socket.sendResult(retvals);
}

void ClientInterface::setDetectorType(detectorType arg) {
    switch (arg) {
    case GOTTHARD:
    case EIGER:
    case CHIPTESTBOARD:
    case XILINX_CHIPTESTBOARD:
    case JUNGFRAU:
    case MOENCH:
    case MYTHEN3:
    case GOTTHARD2:
        break;
    default:
        throw RuntimeError("Unknown detector type: " + std::to_string(arg));
        break;
    }

    try {
        detType = GENERIC;
        receiver = make_unique<Implementation>(arg);
        detType = arg;
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set detector type in the receiver. [" +
                           std::string(e.what()) + ']');
    }
    // callbacks after (in setdetectortype, the object is reinitialized)
    {
        std::lock_guard<std::mutex> lock(callbackMutex);
        if (startAcquisitionCallBack != nullptr)
            impl()->registerCallBackStartAcquisition(startAcquisitionCallBack,
                                                     pStartAcquisition);
        if (acquisitionFinishedCallBack != nullptr)
            impl()->registerCallBackAcquisitionFinished(
                acquisitionFinishedCallBack, pAcquisitionFinished);
        if (rawDataReadyCallBack != nullptr)
            impl()->registerCallBackRawDataReady(rawDataReadyCallBack,
                                                 pRawDataReady);
    }

    impl()->setThreadIds(parentThreadId, tcpThreadId);
}

int ClientInterface::set_detector_roi(Interface &socket) {
    auto arg = socket.Receive<ROI>();
    LOG(logDEBUG1) << "Set Detector ROI: " << ToString(arg);

    if (detType != GOTTHARD)
        functionNotImplemented();

    verifyIdle(socket);
    try {
        impl()->setDetectorROI(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set ROI [" + std::string(e.what()) + ']');
    }
    return socket.Send(OK);
}

int ClientInterface::set_num_frames(Interface &socket) {
    auto value = socket.Receive<int64_t>();
    if (value <= 0) {
        throw RuntimeError("Invalid number of frames " + std::to_string(value));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting num frames to " << value;
    impl()->setNumberOfFrames(value);
    return socket.Send(OK);
}

int ClientInterface::set_num_triggers(Interface &socket) {
    auto value = socket.Receive<int64_t>();
    if (value <= 0) {
        throw RuntimeError("Invalid number of triggers " +
                           std::to_string(value));
    }
    verifyIdle(socket);
    impl()->setNumberOfTriggers(value);
    return socket.Send(OK);
}

int ClientInterface::set_num_bursts(Interface &socket) {
    auto value = socket.Receive<int64_t>();
    if (value <= 0) {
        throw RuntimeError("Invalid number of bursts " + std::to_string(value));
    }
    verifyIdle(socket);
    impl()->setNumberOfBursts(value);
    return socket.Send(OK);
}

int ClientInterface::set_num_add_storage_cells(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value < 0) {
        throw RuntimeError("Invalid number of additional storage cells " +
                           std::to_string(value));
    }
    // allowing this to be done even when receiver not idle
    LOG(logDEBUG1) << "Setting num additional storage cells to " << value;
    impl()->setNumberOfAdditionalStorageCells(value);
    return socket.Send(OK);
}

int ClientInterface::set_timing_mode(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value < 0 || value >= NUM_TIMING_MODES) {
        throw RuntimeError("Invalid timing mode " + std::to_string(value));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting timing mode to " << value;
    impl()->setTimingMode(static_cast<timingMode>(value));
    return socket.Send(OK);
}

int ClientInterface::set_burst_mode(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value < 0 || value >= NUM_BURST_MODES) {
        throw RuntimeError("Invalid burst mode " + std::to_string(value));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting burst mode to " << value;
    impl()->setBurstMode(static_cast<burstMode>(value));
    return socket.Send(OK);
}

int ClientInterface::set_num_analog_samples(Interface &socket) {
    auto value = socket.Receive<int>();
    LOG(logDEBUG1) << "Setting num analog samples to " << value;
    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD) {
        functionNotImplemented();
    }
    try {
        impl()->setNumberofAnalogSamples(value);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set number of analog samples to " +
                           std::to_string(value) + " [" +
                           std::string(e.what()) + ']');
    }
    return socket.Send(OK);
}

int ClientInterface::set_num_digital_samples(Interface &socket) {
    auto value = socket.Receive<int>();
    LOG(logDEBUG1) << "Setting num digital samples to " << value;
    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD) {
        functionNotImplemented();
    }
    try {
        impl()->setNumberofDigitalSamples(value);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set number of digital samples to " +
                           std::to_string(value) + " [" +
                           std::string(e.what()) + ']');
    }

    return socket.Send(OK);
}

int ClientInterface::set_exptime(Interface &socket) {
    int64_t args[2]{-1, -1};
    socket.Receive(args);
    int gateIndex = static_cast<int>(args[0]);
    ns value = std::chrono::nanoseconds(args[1]);
    LOG(logDEBUG1) << "Setting exptime to " << ToString(value)
                   << " (gateIndex: " << gateIndex << ")";
    switch (gateIndex) {
    case -1:
        if (detType == MYTHEN3) {
            impl()->setAcquisitionTime1(value);
            impl()->setAcquisitionTime2(value);
            impl()->setAcquisitionTime3(value);
        } else {
            impl()->setAcquisitionTime(value);
        }
        break;
    case 0:
        if (detType != MYTHEN3) {
            functionNotImplemented();
        }
        impl()->setAcquisitionTime1(value);
        break;
    case 1:
        if (detType != MYTHEN3) {
            functionNotImplemented();
        }
        impl()->setAcquisitionTime2(value);
        break;
    case 2:
        if (detType != MYTHEN3) {
            functionNotImplemented();
        }
        impl()->setAcquisitionTime3(value);
        break;
    default:
        throw RuntimeError("Unknown gate index for exptime " +
                           std::to_string(gateIndex));
    }
    return socket.Send(OK);
}

int ClientInterface::set_period(Interface &socket) {
    auto value = std::chrono::nanoseconds(socket.Receive<int64_t>());
    LOG(logDEBUG1) << "Setting period to " << ToString(value);
    impl()->setAcquisitionPeriod(value);
    return socket.Send(OK);
}

int ClientInterface::set_subexptime(Interface &socket) {
    auto value = std::chrono::nanoseconds(socket.Receive<int64_t>());
    LOG(logDEBUG1) << "Setting period to " << ToString(value);
    ns subdeadtime = impl()->getSubPeriod() - impl()->getSubExpTime();
    impl()->setSubExpTime(value);
    impl()->setSubPeriod(impl()->getSubExpTime() + subdeadtime);
    return socket.Send(OK);
}

int ClientInterface::set_subdeadtime(Interface &socket) {
    auto value = std::chrono::nanoseconds(socket.Receive<int64_t>());
    LOG(logDEBUG1) << "Setting sub deadtime to " << ToString(value);
    impl()->setSubPeriod(value + impl()->getSubExpTime());
    LOG(logDEBUG1) << "Setting sub period to "
                   << ToString(impl()->getSubPeriod());
    return socket.Send(OK);
}

int ClientInterface::set_dynamic_range(Interface &socket) {
    auto dr = socket.Receive<int>();
    if (dr >= 0) {
        verifyIdle(socket);
        LOG(logDEBUG1) << "Setting dynamic range: " << dr;
        bool exists = false;
        switch (dr) {
        case 16:
            exists = true;
            break;
        /*case 1: //TODO: Not yet implemented in firmware
            if (detType == MYTHEN3) {
                exists = true;
            }
            break;
        */
        case 4:
        case 12:
            if (detType == EIGER) {
                exists = true;
            }
            break;
        case 8:
        case 32:
            if (detType == EIGER || detType == MYTHEN3) {
                exists = true;
            }
            break;
        default:
            break;
        }
        if (!exists) {
            modeNotImplemented("Dynamic range", dr);
        } else {
            try {
                impl()->setDynamicRange(dr);
            } catch (const std::exception &e) {
                throw RuntimeError("Could not set dynamic range [" +
                                   std::string(e.what()) + ']');
            }
        }
    }
    int retval = impl()->getDynamicRange();
    validate(dr, retval, "set dynamic range", DEC);
    LOG(logDEBUG1) << "dynamic range: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_streaming_frequency(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index < 0) {
        throw RuntimeError("Invalid streaming frequency: " +
                           std::to_string(index));
    }
    verifyIdle(socket);
    impl()->setStreamingFrequency(index);
    return socket.Send(OK);
}

int ClientInterface::get_streaming_frequency(Interface &socket) {
    int retval = impl()->getStreamingFrequency();
    LOG(logDEBUG1) << "streaming freq:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::get_status(Interface &socket) {
    auto retval = impl()->getStatus();
    LOG(logDEBUG1) << "Status:" << ToString(retval);
    return socket.sendResult(retval);
}

int ClientInterface::start_receiver(Interface &socket) {
    if (impl()->getStatus() == IDLE) {
        LOG(logDEBUG1) << "Starting Receiver";
        try {
            impl()->startReceiver();
        } catch (const std::exception &e) {
            throw RuntimeError("Could not start reciever [" +
                               std::string(e.what()) + ']');
        }
    }
    return socket.Send(OK);
}

int ClientInterface::stop_receiver(Interface &socket) {
    auto arg = socket.Receive<int>();
    if (impl()->getStatus() == RUNNING) {
        LOG(logDEBUG1) << "Stopping Receiver";
        impl()->setStoppedFlag(static_cast<bool>(arg));
        try {
            impl()->stopReceiver();
        } catch (const std::exception &e) {
            throw RuntimeError("Could not stop receiver [" +
                               std::string(e.what()) + ']');
        }
    }
    auto s = impl()->getStatus();
    if (s != IDLE)
        throw RuntimeError("Could not stop receiver. Status: " + ToString(s));

    return socket.Send(OK);
}

int ClientInterface::set_file_dir(Interface &socket) {
    std::string fpath = socket.Receive(MAX_STR_LENGTH);

    if (fpath.empty()) {
        throw RuntimeError("Cannot set empty file path");
    }
    if (fpath[0] != '/')
        throw RuntimeError("Receiver path needs to be absolute path");

    LOG(logDEBUG1) << "Setting file path: " << fpath;
    try {
        impl()->setFilePath(fpath);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set file path [" + std::string(e.what()) +
                           ']');
    }
    return socket.Send(OK);
}

int ClientInterface::get_file_dir(Interface &socket) {
    auto fpath = impl()->getFilePath();
    LOG(logDEBUG1) << "file path:" << fpath;
    fpath.resize(MAX_STR_LENGTH);
    return socket.sendResult(fpath);
}

int ClientInterface::set_file_name(Interface &socket) {
    std::string fname = socket.Receive(MAX_STR_LENGTH);
    if (fname.empty()) {
        throw RuntimeError("Cannot set empty file name");
    }
    LOG(logDEBUG1) << "Setting file name: " << fname;
    impl()->setFileName(fname);
    return socket.Send(OK);
}

int ClientInterface::get_file_name(Interface &socket) {
    auto fname = impl()->getFileName();
    LOG(logDEBUG1) << "file name:" << fname;
    fname.resize(MAX_STR_LENGTH);
    return socket.sendResult(fname);
}

int ClientInterface::set_file_index(Interface &socket) {
    auto index = socket.Receive<int64_t>();
    if (index < 0) {
        throw RuntimeError("Invalid file index: " + std::to_string(index));
    }
    verifyIdle(socket);
    impl()->setFileIndex(index);
    return socket.Send(OK);
}

int ClientInterface::get_file_index(Interface &socket) {
    int64_t retval = impl()->getFileIndex();
    LOG(logDEBUG1) << "file index:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::get_frame_index(Interface &socket) {
    auto retval = impl()->getCurrentFrameIndex();
    LOG(logDEBUG1) << "frames index:" << ToString(retval);
    auto size = static_cast<int>(retval.size());
    socket.Send(OK);
    socket.Send(size);
    socket.Send(retval);
    return OK;
}

int ClientInterface::get_missing_packets(Interface &socket) {
    auto missing_packets = impl()->getNumMissingPackets();
    LOG(logDEBUG1) << "missing packets:" << ToString(missing_packets);
    auto size = static_cast<int>(missing_packets.size());
    socket.Send(OK);
    socket.Send(size);
    socket.Send(missing_packets);
    return OK;
}

int ClientInterface::get_frames_caught(Interface &socket) {
    auto retval = impl()->getFramesCaught();
    LOG(logDEBUG1) << "frames caught:" << ToString(retval);
    auto size = static_cast<int>(retval.size());
    socket.Send(OK);
    socket.Send(size);
    socket.Send(retval);
    return OK;
}

int ClientInterface::set_file_write(Interface &socket) {
    auto enable = socket.Receive<int>();
    if (enable < 0) {
        throw RuntimeError("Invalid file write enable: " +
                           std::to_string(enable));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting File write enable:" << enable;
    try {
        impl()->setFileWriteEnable(enable);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not enable/disable file write [" +
                           std::string(e.what()) + ']');
    }
    return socket.Send(OK);
}

int ClientInterface::get_file_write(Interface &socket) {
    int retval = impl()->getFileWriteEnable();
    LOG(logDEBUG1) << "file write enable:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_master_file_write(Interface &socket) {
    auto enable = socket.Receive<int>();
    if (enable < 0) {
        throw RuntimeError("Invalid master file write enable: " +
                           std::to_string(enable));
    }
    verifyIdle(socket);
    impl()->setMasterFileWriteEnable(enable);
    return socket.Send(OK);
}

int ClientInterface::get_master_file_write(Interface &socket) {
    int retval = impl()->getMasterFileWriteEnable();
    LOG(logDEBUG1) << "master file write enable:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_overwrite(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index < 0) {
        throw RuntimeError("Invalid over write enable: " +
                           std::to_string(index));
    }
    verifyIdle(socket);
    impl()->setOverwriteEnable(index);
    return socket.Send(OK);
}

int ClientInterface::get_overwrite(Interface &socket) {
    int retval = impl()->getOverwriteEnable();
    LOG(logDEBUG1) << "file overwrite enable:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::enable_tengiga(Interface &socket) {
    auto val = socket.Receive<int>();
    if (detType != EIGER && detType != CHIPTESTBOARD && detType != MYTHEN3)
        functionNotImplemented();

    if (val >= 0) {
        verifyIdle(socket);
        LOG(logDEBUG1) << "Setting 10GbE:" << val;
        try {
            impl()->setTenGigaEnable(val);
        } catch (const std::exception &e) {
            throw RuntimeError("Could not set 10GbE. [" +
                               std::string(e.what()) + ']');
        }
    }
    int retval = impl()->getTenGigaEnable();
    validate(val, retval, "set 10GbE", DEC);
    LOG(logDEBUG1) << "10Gbe:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_fifo_depth(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value >= 0) {
        verifyIdle(socket);
        LOG(logDEBUG1) << "Setting fifo depth:" << value;
        try {
            impl()->setFifoDepth(value);
        } catch (const std::exception &e) {
            throw RuntimeError("Could not set fifo depth [" +
                               std::string(e.what()) + ']');
        }
    }
    int retval = impl()->getFifoDepth();
    validate(value, retval, std::string("set fifo depth"), DEC);
    LOG(logDEBUG1) << "fifo depth:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_activate(Interface &socket) {
    auto enable = socket.Receive<int>();
    if (detType != EIGER)
        functionNotImplemented();

    if (enable >= 0) {
        verifyIdle(socket);
        LOG(logDEBUG1) << "Setting activate:" << enable;
        impl()->setActivate(static_cast<bool>(enable));
    }
    auto retval = static_cast<int>(impl()->getActivate());
    validate(enable, retval, "set activate", DEC);
    LOG(logDEBUG1) << "Activate: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_streaming(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index < 0) {
        throw RuntimeError("Invalid streaming enable: " +
                           std::to_string(index));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting data stream enable:" << index;
    try {
        impl()->setDataStreamEnable(index);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set data stream enable to " +
                           std::to_string(index) + " [" +
                           std::string(e.what()) + ']');
    }

    return socket.Send(OK);
}

int ClientInterface::get_streaming(Interface &socket) {
    auto retval = static_cast<int>(impl()->getDataStreamEnable());
    LOG(logDEBUG1) << "data streaming enable:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_streaming_timer(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index >= 0) {
        verifyIdle(socket);
        LOG(logDEBUG1) << "Setting streaming timer:" << index;
        impl()->setStreamingTimer(index);
    }
    int retval = impl()->getStreamingTimer();
    validate(index, retval, "set data stream timer", DEC);
    LOG(logDEBUG1) << "Streaming timer:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::get_flip_rows(Interface &socket) {
    if (detType != EIGER)
        functionNotImplemented();

    int retval = impl()->getFlipRows();
    LOG(logDEBUG1) << "Flip rows:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_flip_rows(Interface &socket) {
    auto arg = socket.Receive<int>();

    if (detType != EIGER)
        functionNotImplemented();

    if (arg != 0 && arg != 1) {
        throw RuntimeError("Could not set flip rows. Invalid argument: " +
                           std::to_string(arg));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting flip rows:" << arg;
    impl()->setFlipRows(static_cast<bool>(arg));

    int retval = impl()->getFlipRows();
    validate(arg, retval, std::string("set flip rows"), DEC);
    LOG(logDEBUG1) << "Flip rows:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_file_format(Interface &socket) {
    auto f = socket.Receive<fileFormat>();
    if (f < 0 || f > NUM_FILE_FORMATS) {
        throw RuntimeError("Invalid file format: " + std::to_string(f));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting file format:" << f;
    try {
        impl()->setFileFormat(f);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set file format to " + ToString(f) +
                           " [" + std::string(e.what()) + ']');
    }

    auto retval = impl()->getFileFormat();
    validate(f, retval, "set file format", DEC);
    LOG(logDEBUG1) << "File Format: " << retval;
    return socket.Send(OK);
}

int ClientInterface::get_file_format(Interface &socket) {
    auto retval = impl()->getFileFormat();
    LOG(logDEBUG1) << "File Format: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_streaming_port(Interface &socket) {
    auto port = socket.Receive<uint16_t>();
    try {
        validatePortNumber(port);
    } catch (...) {
        throw RuntimeError(
            "Could not set streaming (zmq) port number. Invalid value.");
    }
    verifyIdle(socket);
    impl()->setStreamingPort(port);
    return socket.Send(OK);
}

int ClientInterface::get_streaming_port(Interface &socket) {
    uint16_t retval = impl()->getStreamingPort();
    LOG(logDEBUG1) << "streaming port:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_silent_mode(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value < 0) {
        throw RuntimeError("Invalid silent mode: " + std::to_string(value));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting silent mode:" << value;
    impl()->setSilentMode(value);
    return socket.Send(OK);
}

int ClientInterface::get_silent_mode(Interface &socket) {
    auto retval = static_cast<int>(impl()->getSilentMode());
    LOG(logDEBUG1) << "silent mode:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::restream_stop(Interface &socket) {
    verifyIdle(socket);
    if (!impl()->getDataStreamEnable()) {
        throw RuntimeError(
            "Could not restream stop packet as data Streaming is disabled");
    } else {
        LOG(logDEBUG1) << "Restreaming stop";
        impl()->restreamStop();
    }
    return socket.Send(OK);
}

int ClientInterface::set_additional_json_header(Interface &socket) {
    std::map<std::string, std::string> json;
    auto size = socket.Receive<int>();
    if (size > 0) {
        std::string buff(size, '\0');
        socket.Receive(&buff[0], buff.size());
        std::istringstream iss(buff);
        std::string key, value;
        while (iss >> key) {
            iss >> value;
            json[key] = value;
        }
    }
    // verifyIdle(socket); allowing it to be set on the fly
    LOG(logDEBUG1) << "Setting additional json header: " << ToString(json);
    impl()->setAdditionalJsonHeader(json);
    return socket.Send(OK);
}

int ClientInterface::get_additional_json_header(Interface &socket) {
    std::map<std::string, std::string> json = impl()->getAdditionalJsonHeader();
    LOG(logDEBUG1) << "additional json header:" << ToString(json);
    std::ostringstream oss;
    for (auto &it : json) {
        oss << it.first << ' ' << it.second << ' ';
    }
    auto buff = oss.str();
    auto size = static_cast<int>(buff.size());
    socket.sendResult(size);
    if (size > 0)
        socket.Send(buff);
    return OK;
}

int ClientInterface::set_udp_socket_buffer_size(Interface &socket) {
    auto size = socket.Receive<int>();
    if (size == 0) {
        throw RuntimeError(
            "Receiver socket buffer size must be greater than 0.");
    }
    if (size > 0) {
        verifyIdle(socket);
        if (size > INT_MAX / 2) {
            throw RuntimeError(
                "Receiver socket buffer size exceeded max (INT_MAX/2)");
        }
        LOG(logDEBUG1) << "Setting UDP Socket Buffer size: " << size;
        try {
            impl()->setUDPSocketBufferSize(size);
        } catch (const std::exception &e) {
            throw RuntimeError("Could not set udp socket buffer size to " +
                               std::to_string(size) + " [" +
                               std::string(e.what()) + ']');
        }
    }
    int retval = impl()->getUDPSocketBufferSize();
    if (size != 0)
        validate(size, retval,
                 "set udp socket buffer size (No CAP_NET_ADMIN privileges?)",
                 DEC);
    LOG(logDEBUG1) << "UDP Socket Buffer Size:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::get_real_udp_socket_buffer_size(Interface &socket) {
    auto size = impl()->getActualUDPSocketBufferSize();
    LOG(logDEBUG1) << "Actual UDP socket size :" << size;
    return socket.sendResult(size);
}

int ClientInterface::set_frames_per_file(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index < 0) {
        throw RuntimeError("Invalid frames per file: " + std::to_string(index));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting frames per file: " << index;
    impl()->setFramesPerFile(index);
    return socket.Send(OK);
}

int ClientInterface::get_frames_per_file(Interface &socket) {
    auto retval = static_cast<int>(impl()->getFramesPerFile());
    LOG(logDEBUG1) << "frames per file:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_discard_policy(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index < 0 || index > NUM_DISCARD_POLICIES) {
        throw RuntimeError("Invalid discard policy " + std::to_string(index));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting frames discard policy: " << index;
    impl()->setFrameDiscardPolicy(static_cast<frameDiscardPolicy>(index));
    return socket.Send(OK);
}

int ClientInterface::get_discard_policy(Interface &socket) {
    int retval = impl()->getFrameDiscardPolicy();
    LOG(logDEBUG1) << "frame discard policy:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_padding_enable(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index < 0) {
        throw RuntimeError("Invalid padding enable: " + std::to_string(index));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting frames padding enable: " << index;
    impl()->setFramePaddingEnable(static_cast<bool>(index));
    return socket.Send(OK);
}

int ClientInterface::get_padding_enable(Interface &socket) {
    auto retval = static_cast<int>(impl()->getFramePaddingEnable());
    LOG(logDEBUG1) << "Frame Padding Enable:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_readout_mode(Interface &socket) {
    auto arg = socket.Receive<readoutMode>();

    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD)
        functionNotImplemented();

    if (arg >= 0) {
        verifyIdle(socket);
        LOG(logDEBUG1) << "Setting readout mode: " << arg;
        try {
            impl()->setReadoutMode(arg);
        } catch (const std::exception &e) {
            throw RuntimeError("Could not set read out mode [" +
                               std::string(e.what()) + ']');
        }
    }
    auto retval = impl()->getReadoutMode();
    validate(static_cast<int>(arg), static_cast<int>(retval),
             "set readout mode", DEC);
    LOG(logDEBUG1) << "Readout mode: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_adc_mask(Interface &socket) {
    auto arg = socket.Receive<uint32_t>();
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting 1Gb ADC enable mask: " << arg;
    try {
        impl()->setADCEnableMask(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set adc enable mask [" +
                           std::string(e.what()) + ']');
    }

    auto retval = impl()->getADCEnableMask();
    if (retval != arg) {
        std::ostringstream os;
        os << "Could not set 1Gb ADC enable mask. Set 0x" << std::hex << arg
           << " but read 0x" << std::hex << retval;
        throw RuntimeError(os.str());
    }
    LOG(logDEBUG1) << "1Gb ADC enable mask retval: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_dbit_list(Interface &socket) {
    StaticVector<int, MAX_RX_DBIT> args;
    socket.Receive(args);
    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD)
        functionNotImplemented();
    LOG(logDEBUG1) << "Setting DBIT list";
    for (auto &it : args) {
        LOG(logDEBUG1) << it << " ";
    }
    LOG(logDEBUG1) << '\n';
    verifyIdle(socket);
    impl()->setDbitList(args);
    return socket.Send(OK);
}

int ClientInterface::get_dbit_list(Interface &socket) {
    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD)
        functionNotImplemented();
    StaticVector<int, MAX_RX_DBIT> retval;
    retval = impl()->getDbitList();
    LOG(logDEBUG1) << "Dbit list size retval:" << retval.size();
    return socket.sendResult(retval);
}

int ClientInterface::set_dbit_offset(Interface &socket) {
    auto arg = socket.Receive<int>();
    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD)
        functionNotImplemented();
    if (arg < 0) {
        throw RuntimeError("Invalid dbit offset: " + std::to_string(arg));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting Dbit offset: " << arg;
    impl()->setDbitOffset(arg);
    return socket.Send(OK);
}

int ClientInterface::get_dbit_offset(Interface &socket) {
    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD)
        functionNotImplemented();
    int retval = impl()->getDbitOffset();
    LOG(logDEBUG1) << "Dbit offset retval: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_quad_type(Interface &socket) {
    auto quadEnable = socket.Receive<int>();
    if (quadEnable >= 0) {
        verifyIdle(socket);
        LOG(logDEBUG1) << "Setting quad:" << quadEnable;
        try {
            impl()->setQuad(quadEnable == 0 ? false : true);
        } catch (const std::exception &e) {
            throw RuntimeError("Could not set quad to " +
                               std::to_string(quadEnable) + " [" +
                               std::string(e.what()) + ']');
        }
    }
    int retval = impl()->getQuad() ? 1 : 0;
    validate(quadEnable, retval, "set quad", DEC);
    LOG(logDEBUG1) << "quad retval:" << retval;
    return socket.Send(OK);
}

int ClientInterface::set_read_n_rows(Interface &socket) {
    auto arg = socket.Receive<int>();
    if (arg >= 0) {
        verifyIdle(socket);
        if (detType != EIGER && detType != JUNGFRAU && detType != MOENCH) {
            throw RuntimeError("Could not set number of rows. Not implemented "
                               "for this detector");
        }
        LOG(logDEBUG1) << "Setting number of rows:" << arg;
        impl()->setReadNRows(arg);
    }
    int retval = impl()->getReadNRows();
    validate(arg, retval, "set number of rows", DEC);
    LOG(logDEBUG1) << "read number of rows:" << retval;
    return socket.Send(OK);
}

MacAddr ClientInterface::setUdpIp(IpAddr arg) {
    LOG(logINFO) << "Received UDP IP: " << arg;
    // getting eth
    std::string eth = IpToInterfaceName(arg.str());
    if (eth == "none") {
        throw RuntimeError("Failed to get udp ethernet interface from IP " +
                           arg.str());
    }
    if (eth.find('.') != std::string::npos) {
        eth = "";
        LOG(logERROR) << "Failed to get udp ethernet interface from IP " << arg
                      << ". Got " << eth;
    }
    impl()->setEthernetInterface(eth);
    if (detType == EIGER) {
        impl()->setEthernetInterface2(eth);
    }

    // update locally to use for arping
    udpips[0] = arg.str();

    // get mac address
    auto retval = InterfaceNameToMac(eth);
    if (retval == 0 && arg.str() != LOCALHOST_IP) {
        throw RuntimeError("Failed to get udp mac adddress to listen to (eth:" +
                           eth + ", ip:" + arg.str() + ")\n");
    }
    LOG(logINFO) << "Receiver MAC Address: " << retval;
    return retval;
}

int ClientInterface::set_udp_ip(Interface &socket) {
    auto arg = socket.Receive<IpAddr>();
    verifyIdle(socket);
    auto retval = setUdpIp(arg);
    return socket.sendResult(retval);
}

MacAddr ClientInterface::setUdpIp2(IpAddr arg) {
    LOG(logINFO) << "Received UDP IP2: " << arg;
    // getting eth
    std::string eth = IpToInterfaceName(arg.str());
    if (eth == "none") {
        throw RuntimeError("Failed to get udp ethernet interface2 from IP " +
                           arg.str());
    }
    if (eth.find('.') != std::string::npos) {
        eth = "";
        LOG(logERROR) << "Failed to get udp ethernet interface2 from IP " << arg
                      << ". Got " << eth;
    }
    impl()->setEthernetInterface2(eth);

    // update locally to use for arping
    udpips[1] = arg.str();

    // get mac address
    auto retval = InterfaceNameToMac(eth);
    if (retval == 0 && arg.str() != LOCALHOST_IP) {
        throw RuntimeError(
            "Failed to get udp mac adddress2 to listen to (eth:" + eth +
            ", ip:" + arg.str() + ")\n");
    }
    LOG(logINFO) << "Receiver MAC Address2: " << retval;
    return retval;
}

int ClientInterface::set_udp_ip2(Interface &socket) {
    auto arg = socket.Receive<IpAddr>();
    verifyIdle(socket);
    if (detType != JUNGFRAU && detType != MOENCH && detType != GOTTHARD2) {
        throw RuntimeError(
            "UDP Destination IP2 not implemented for this detector");
    }
    auto retval = setUdpIp2(arg);
    return socket.sendResult(retval);
}

int ClientInterface::set_udp_port(Interface &socket) {
    auto arg = socket.Receive<uint16_t>();
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting UDP Port:" << arg;
    impl()->setUDPPortNumber(arg);
    return socket.Send(OK);
}

int ClientInterface::set_udp_port2(Interface &socket) {
    auto arg = socket.Receive<uint16_t>();
    verifyIdle(socket);
    if (detType != JUNGFRAU && detType != MOENCH && detType != EIGER &&
        detType != GOTTHARD2) {
        throw RuntimeError(
            "UDP Destination Port2 not implemented for this detector");
    }
    LOG(logDEBUG1) << "Setting UDP Port:" << arg;
    impl()->setUDPPortNumber2(arg);
    return socket.Send(OK);
}

int ClientInterface::set_num_interfaces(Interface &socket) {
    auto arg = socket.Receive<int>();
    arg = (arg > 1 ? 2 : 1);
    verifyIdle(socket);
    if (detType != JUNGFRAU && detType != MOENCH && detType != GOTTHARD2) {
        throw RuntimeError(
            "Number of interfaces not implemented for this detector");
    }
    LOG(logDEBUG1) << "Setting Number of UDP Interfaces:" << arg;
    try {
        impl()->setNumberofUDPInterfaces(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set number of interfaces to " +
                           std::to_string(arg) + " [" + std::string(e.what()) +
                           ']');
    }

    return socket.Send(OK);
}

int ClientInterface::set_adc_mask_10g(Interface &socket) {
    auto arg = socket.Receive<uint32_t>();
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting 10Gb ADC enable mask: " << arg;
    try {
        impl()->setTenGigaADCEnableMask(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set 10Gb adc enable mask [" +
                           std::string(e.what()) + ']');
    }

    auto retval = impl()->getTenGigaADCEnableMask();
    if (retval != arg) {
        std::ostringstream os;
        os << "Could not 10gb ADC enable mask. Set 0x" << std::hex << arg
           << " but read 0x" << std::hex << retval;
        throw RuntimeError(os.str());
    }
    LOG(logDEBUG1) << "10Gb ADC enable mask retval: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_counter_mask(Interface &socket) {
    auto arg = socket.Receive<uint32_t>();
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting counters: " << arg;
    try {
        impl()->setCounterMask(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set counter mask [" +
                           std::string(e.what()) + ']');
    }
    return socket.Send(OK);
}

int ClientInterface::increment_file_index(Interface &socket) {
    verifyIdle(socket);
    if (impl()->getFileWriteEnable()) {
        LOG(logDEBUG1) << "Incrementing file index";
        impl()->setFileIndex(impl()->getFileIndex() + 1);
    }
    return socket.Send(OK);
}

int ClientInterface::set_additional_json_parameter(Interface &socket) {
    char args[2][SHORT_STR_LENGTH]{};
    socket.Receive(args);
    // verifyIdle(socket); allowing it to be set on the fly
    LOG(logDEBUG1) << "Setting additional json parameter (" << args[0]
                   << "): " << args[1];
    impl()->setAdditionalJsonParameter(args[0], args[1]);
    return socket.Send(OK);
}

int ClientInterface::get_additional_json_parameter(Interface &socket) {
    std::string key = socket.Receive(SHORT_STR_LENGTH);
    std::string value = impl()->getAdditionalJsonParameter(key);
    value.resize(SHORT_STR_LENGTH);
    return socket.sendResult(value);
}

int ClientInterface::get_progress(Interface &socket) {
    double retval = impl()->getProgress();
    LOG(logDEBUG1) << "progress retval: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_num_gates(Interface &socket) {
    auto value = socket.Receive<int>();
    LOG(logDEBUG1) << "Setting num gates to " << value;
    if (detType != MYTHEN3) {
        functionNotImplemented();
    }
    impl()->setNumberOfGates(value);
    return socket.Send(OK);
}

int ClientInterface::set_gate_delay(Interface &socket) {
    int64_t args[2]{-1, -1};
    socket.Receive(args);
    int gateIndex = static_cast<int>(args[0]);
    auto value = std::chrono::nanoseconds(args[1]);
    LOG(logDEBUG1) << "Setting gate delay to " << ToString(value)
                   << " (gateIndex: " << gateIndex << ")";
    if (detType != MYTHEN3) {
        functionNotImplemented();
    }
    switch (gateIndex) {
    case -1:
        impl()->setGateDelay1(value);
        impl()->setGateDelay2(value);
        impl()->setGateDelay3(value);
        break;
    case 0:
        impl()->setGateDelay1(value);
        break;
    case 1:
        impl()->setGateDelay2(value);
        break;
    case 2:
        impl()->setGateDelay3(value);
        break;
    default:
        throw RuntimeError("Unknown gate index for gate delay " +
                           std::to_string(gateIndex));
    }
    return socket.Send(OK);
}

int ClientInterface::get_thread_ids(Interface &socket) {
    auto retval = impl()->getThreadIds();
    LOG(logDEBUG1) << "thread ids retval: " << ToString(retval);
    return socket.sendResult(retval);
}

int ClientInterface::get_streaming_start_fnum(Interface &socket) {
    int retval = impl()->getStreamingStartingFrameNumber();
    LOG(logDEBUG1) << "streaming start fnum:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_streaming_start_fnum(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index < 0) {
        throw RuntimeError("Invalid streaming start frame number: " +
                           std::to_string(index));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting streaming start fnum: " << index;
    impl()->setStreamingStartingFrameNumber(index);
    return socket.Send(OK);
}

int ClientInterface::set_rate_correct(Interface &socket) {
    auto index = socket.Receive<int>();
    if (index <= 0) {
        throw RuntimeError("Invalid number of rate correction values: " +
                           std::to_string(index));
    }
    LOG(logDEBUG) << "Number of detectors for rate correction: " << index;
    std::vector<int64_t> t(index);
    socket.Receive(t);
    verifyIdle(socket);
    LOG(logINFO) << "Setting rate corrections[" << index << ']';
    impl()->setRateCorrections(t);
    return socket.Send(OK);
}

int ClientInterface::set_scan(Interface &socket) {
    auto arg = socket.Receive<scanParameters>();
    LOG(logDEBUG) << "Scan Mode: " << ToString(arg);
    verifyIdle(socket);
    impl()->setScan(arg);
    return socket.Send(OK);
}

int ClientInterface::set_threshold(Interface &socket) {
    auto arg = socket.Receive<int>();
    LOG(logDEBUG) << "Threshold: " << arg << " eV";
    if (detType != EIGER)
        functionNotImplemented();
    verifyIdle(socket);
    impl()->setThresholdEnergy(arg);
    return socket.Send(OK);
}

int ClientInterface::get_streaming_hwm(Interface &socket) {
    int retval = impl()->getStreamingHwm();
    LOG(logDEBUG1) << "zmq send hwm limit:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_streaming_hwm(Interface &socket) {
    auto limit = socket.Receive<int>();
    if (limit < -1) {
        throw RuntimeError("Invalid zmq send hwm limit " +
                           std::to_string(limit));
    }
    verifyIdle(socket);
    impl()->setStreamingHwm(limit);
    return socket.Send(OK);
}

int ClientInterface::set_all_threshold(Interface &socket) {
    auto eVs = socket.Receive<std::array<int, 3>>();
    LOG(logDEBUG) << "Threshold:" << ToString(eVs);
    if (detType != MYTHEN3)
        functionNotImplemented();
    verifyIdle(socket);
    impl()->setThresholdEnergy(eVs);
    return socket.Send(OK);
}

int ClientInterface::set_detector_datastream(Interface &socket) {
    int args[2]{-1, -1};
    socket.Receive(args);
    portPosition port = static_cast<portPosition>(args[0]);
    switch (port) {
    case LEFT:
    case RIGHT:
        break;
    default:
        throw RuntimeError("Invalid port type");
    }
    bool enable = static_cast<int>(args[1]);
    LOG(logDEBUG1) << "Setting datastream (" << ToString(port) << ") to "
                   << ToString(enable);
    if (detType != EIGER)
        functionNotImplemented();
    verifyIdle(socket);
    impl()->setDetectorDataStream(port, enable);
    return socket.Send(OK);
}

int ClientInterface::get_arping(Interface &socket) {
    auto retval = static_cast<int>(impl()->getArping());
    LOG(logDEBUG1) << "arping thread status:" << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_arping(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value < 0) {
        throw RuntimeError("Invalid arping value: " + std::to_string(value));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Starting/ Killing arping thread:" << value;
    try {
        impl()->setArping(value, udpips);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not start/kill arping thread [" +
                           std::string(e.what()) + ']');
    }
    return socket.Send(OK);
}

int ClientInterface::get_receiver_roi(Interface &socket) {
    auto retval = impl()->getReceiverROI();
    LOG(logDEBUG1) << "Receiver roi retval:" << ToString(retval);
    return socket.sendResult(retval);
}

int ClientInterface::set_receiver_roi(Interface &socket) {
    auto arg = socket.Receive<ROI>();
    if (detType == CHIPTESTBOARD || detType == XILINX_CHIPTESTBOARD)
        functionNotImplemented();
    LOG(logDEBUG1) << "Set Receiver ROI: " << ToString(arg);
    verifyIdle(socket);
    try {
        impl()->setReceiverROI(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set Receiver ROI [" +
                           std::string(e.what()) + ']');
    }

    return socket.Send(OK);
}

int ClientInterface::set_receiver_roi_metadata(Interface &socket) {
    auto arg = socket.Receive<ROI>();
    if (detType == CHIPTESTBOARD || detType == XILINX_CHIPTESTBOARD)
        functionNotImplemented();
    LOG(logDEBUG1) << "Set Receiver ROI Metadata: " << ToString(arg);
    verifyIdle(socket);
    try {
        impl()->setReceiverROIMetadata(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set ReceiverROI metadata [" +
                           std::string(e.what()) + ']');
    }

    return socket.Send(OK);
}

int ClientInterface::set_num_transceiver_samples(Interface &socket) {
    auto value = socket.Receive<int>();
    LOG(logDEBUG1) << "Setting num transceiver samples to " << value;
    if (detType != CHIPTESTBOARD && detType != XILINX_CHIPTESTBOARD) {
        functionNotImplemented();
    }
    try {
        impl()->setNumberofTransceiverSamples(value);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set number of transceiver samples to " +
                           std::to_string(value) + " [" +
                           std::string(e.what()) + ']');
    }

    return socket.Send(OK);
}

int ClientInterface::set_transceiver_mask(Interface &socket) {
    auto arg = socket.Receive<uint32_t>();
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting Transceiver enable mask: " << arg;
    try {
        impl()->setTransceiverEnableMask(arg);
    } catch (const std::exception &e) {
        throw RuntimeError("Could not set transceiver enable mask [" +
                           std::string(e.what()) + ']');
    }

    auto retval = impl()->getTransceiverEnableMask();
    if (retval != arg) {
        std::ostringstream os;
        os << "Could not set Transceiver enable mask. Set 0x" << std::hex << arg
           << " but read 0x" << std::hex << retval;
        throw RuntimeError(os.str());
    }
    LOG(logDEBUG1) << "Transceiver enable mask retval: " << retval;
    return socket.sendResult(retval);
}

int ClientInterface::set_row(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value < 0) {
        throw RuntimeError("Invalid row " + std::to_string(value));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting num rows to " << value;
    impl()->setRow(value);
    return socket.Send(OK);
}

int ClientInterface::set_column(Interface &socket) {
    auto value = socket.Receive<int>();
    if (value < 0) {
        throw RuntimeError("Invalid column " + std::to_string(value));
    }
    verifyIdle(socket);
    LOG(logDEBUG1) << "Setting column to " << value;
    impl()->setColumn(value);
    return socket.Send(OK);
}

} // namespace sls
