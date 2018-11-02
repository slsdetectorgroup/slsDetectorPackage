#pragma once
/************************************************
 * @file sls_receiver_exceptions.h
 * @short exceptions defined
 ***********************************************/
/**
 *@short exceptions defined
 */

#include <iostream>
#include <exception>

struct SlsDetectorPackageExceptions : public std::exception {
public:
	SlsDetectorPackageExceptions() {}
    std::string GetMessage() const { return "SLS Detector Package Failed";};
};

struct SharedMemoryException : public SlsDetectorPackageExceptions {
public:
    SharedMemoryException() {}
    std::string GetMessage() const { return "Shared Memory Failed";};
};

struct ThreadpoolException : public SlsDetectorPackageExceptions {
public:
	ThreadpoolException() {}
	std::string GetMessage() const { return "Threadpool Failed";};
};

struct SocketException : public SlsDetectorPackageExceptions {
public:
	SocketException() {}
	std::string GetMessage() const { return "Socket Failed";};
};

struct SamePortSocketException : public SocketException {
public:
	SamePortSocketException() {}
	std::string GetMessage() const { return "Socket Failed";};
};

