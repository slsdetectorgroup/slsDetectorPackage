#pragma once
/************************************************
 * @file sls_detector_exceptions.h
 * @short exceptions defined
 ***********************************************/
/**
 *@short exceptions defined
 */

#include <iostream>
#include <stdexcept>

namespace sls{

struct RuntimeError : public std::runtime_error {
public:
	RuntimeError(): runtime_error("SLS Detector Package Failed") {}
	RuntimeError(std::string msg): runtime_error(msg) {}
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



}



