#pragma once
/************************************************
 * @file sls_detector_exceptions.h
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
