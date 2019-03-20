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

struct SharedMemoryError : public RuntimeError {
public:
    SharedMemoryError(std::string msg):RuntimeError(msg) {}

};

struct SocketError : public RuntimeError {
public:
	SocketError(std::string msg):RuntimeError(msg) {}

};

struct ZmqSocketError : public RuntimeError {
public:
	ZmqSocketError(std::string msg):RuntimeError(msg) {}

};

struct NotImplementedError : public RuntimeError {
public:
	NotImplementedError(std::string msg):RuntimeError(msg) {}

};

struct DetectorError : public RuntimeError {
public:
	DetectorError(std::string msg):RuntimeError(msg) {}

};

struct ReceiverError : public RuntimeError {
public:
	ReceiverError(std::string msg):RuntimeError(msg) {}

};


}



