#pragma once
/************************************************
 * @file sls_detector_exceptions.h
 * @short exceptions defined
 ***********************************************/
/**
 *@short exceptions defined
 */

#include "logger.h"

#include <iostream>
#include <stdexcept>

namespace sls{

struct RuntimeError : public std::runtime_error {
public:
	RuntimeError(): runtime_error("SLS Detector Package Failed") {
		FILE_LOG(logERROR) << "SLS Detector Package Failed";
	}
	RuntimeError(std::string msg): runtime_error(msg) {
		FILE_LOG(logERROR) << msg;
	}
	RuntimeError(const char* msg): runtime_error(msg) {
		FILE_LOG(logERROR) << msg;
	}
};

struct CriticalError : public RuntimeError {
public:
    CriticalError(std::string msg):RuntimeError(msg) {}
};

struct SharedMemoryError : public CriticalError {
public:
    SharedMemoryError(std::string msg):CriticalError(msg) {}
};

struct SocketError : public CriticalError {
public:
	SocketError(std::string msg):CriticalError(msg) {}
};

struct ZmqSocketError : public CriticalError {
public:
	ZmqSocketError(std::string msg):CriticalError(msg) {}
};

struct NonCriticalError : public RuntimeError {
public:
    NonCriticalError(std::string msg):RuntimeError(msg) {}
};

struct NotImplementedError : public NonCriticalError {
public:
	NotImplementedError(std::string msg):NonCriticalError(msg) {}
};

struct DetectorError : public NonCriticalError {
public:
	DetectorError(std::string msg):NonCriticalError(msg) {}
};

struct ReceiverError : public NonCriticalError {
public:
	ReceiverError(std::string msg):NonCriticalError(msg) {}
};


}



