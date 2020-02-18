/************************************************
 * @file ThreadObject.cpp
 * @short creates/destroys a thread
 ***********************************************/



#include "ThreadObject.h"
#include "container_utils.h"

#include <iostream>
#include <syscall.h>



ThreadObject::ThreadObject(int threadIndex, std::string threadType)
	: index(threadIndex), type(threadType) {
	FILE_LOG(logDEBUG) 	<< type << " thread created: " << 	index;

	sem_init(&semaphore,1,0);

	try {
		threadObject = sls::make_unique<std::thread>(&ThreadObject::RunningThread, this);
	} catch (...) {
		throw sls::RuntimeError("Could not create " + type + " thread with index " + std::to_string(index));
	}
}


ThreadObject::~ThreadObject() {
	killThread = true;
	sem_post(&semaphore);

	threadObject->join();

	sem_destroy(&semaphore);
}


void ThreadObject::RunningThread() {
	FILE_LOG(logINFOBLUE) << "Created [ " << type << "Thread " << index << ", Tid: " << syscall(SYS_gettid) << "]";
	FILE_LOG(logDEBUG) << type << " thread " << index << " created successfully.";

	while(true)	{
		while(IsRunning()) {
			ThreadExecution();
		}
		//wait till the next acquisition
		sem_wait(&semaphore);
		if(killThread)	{
			break;
		}
	}
	
	FILE_LOG(logDEBUG) << type << " thread with index " << index << " destroyed successfully.";
	FILE_LOG(logINFOBLUE) << "Exiting [ " << type << " Thread " << index << ", Tid: " << syscall(SYS_gettid) << "]";
}


void ThreadObject::Continue() {
	sem_post(&semaphore);
}


void ThreadObject::SetThreadPriority(int priority) {
	struct sched_param param;
	param.sched_priority = priority;
	if (pthread_setschedparam(threadObject->native_handle(), SCHED_FIFO, &param) == EPERM) {
		if (index == 0) {
			FILE_LOG(logWARNING) << "Could not prioritize " << type << " thread. "
                                    "(No Root Privileges?)";
		}
	} else {
		FILE_LOG(logINFO) << "Priorities set - " << type << ": " << priority;
	}
}
