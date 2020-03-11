#pragma once

#include "logger.h"
#include <iostream>
#include <stdexcept>

namespace sls{

struct RuntimeError : public std::runtime_error {
public:
	RuntimeError(): runtime_error("SLS Detector Package Failed") {
		LOG(logERROR) << "SLS Detector Package Failed";
	}
	RuntimeError(const std::string& msg): runtime_error(msg) {
		LOG(logERROR) << msg;
	}
	RuntimeError(const char* msg): runtime_error(msg) {
		LOG(logERROR) << msg;
	}
};

struct SharedMemoryError : public RuntimeError {
public:
    SharedMemoryError(const std::string& msg):RuntimeError(msg) {}
};

struct SocketError : public RuntimeError {
public:
	SocketError(const std::string& msg):RuntimeError(msg) {}
};

struct ZmqSocketError : public RuntimeError {
public:
	ZmqSocketError(const std::string& msg):RuntimeError(msg) {}
};

struct NotImplementedError : public RuntimeError {
public:
	NotImplementedError(const std::string& msg):RuntimeError(msg) {}
};

struct DetectorError : public RuntimeError {
public:
	DetectorError(const std::string& msg):RuntimeError(msg) {}
};

struct ReceiverError : public RuntimeError {
public:
	ReceiverError(const std::string& msg):RuntimeError(msg) {}
};

struct GuiError : public RuntimeError {
public:
	GuiError(const std::string& msg):RuntimeError(msg) {}
};

} //  namespace sls



