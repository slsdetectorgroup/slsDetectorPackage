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


struct SharedMemoryException : public std::exception {
public:
    SharedMemoryException() {}
    std::string GetMessage() const { return "Shared Memory Failed";};
};

struct ThreadpoolException : public std::exception {
public:
	ThreadpoolException() {}
	std::string GetMessage() const { return "Threadpool Failed";};
};

struct SocketException : public std::exception {
public:
	SocketException() {}
	std::string GetMessage() const { return "Socket Failed";};
};


struct SamePortSocketException : public SocketException {
public:
	SamePortSocketException() {}
	std::string GetMessage() const { return "Socket Failed";};
};

