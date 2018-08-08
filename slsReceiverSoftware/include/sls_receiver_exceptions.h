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
using namespace std;


struct SharedMemoryException : public exception {
public:
    SharedMemoryException() {}
    string GetMessage() const { return "Shared Memory Failed";};
};

struct ThreadpoolException : public exception {
public:
	ThreadpoolException() {}
    string GetMessage() const { return "Threadpool Failed";};
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

