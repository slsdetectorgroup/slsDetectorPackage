/************************************************
 * @file ThreadObject.cpp
 * @short creates/destroys a thread
 ***********************************************/



#include "ThreadObject.h"

#include <iostream>
#include <syscall.h>



ThreadObject::ThreadObject(int ind):
		index(ind),
		alive(false),
		killThread(false),
		thread(0)
{
	PrintMembers();
}


ThreadObject::~ThreadObject() {
	DestroyThread();
}


void ThreadObject::PrintMembers() {
	FILE_LOG(logDEBUG) 	<< "Index : " << 	index
						<< "\nalive: " <<	alive
						<< "\nkillThread: " << killThread
						<< "\npthread: " << thread;
}


void  ThreadObject::DestroyThread() {
	if(alive){
		killThread = true;
		sem_post(&semaphore);
		pthread_join(thread,nullptr);
		sem_destroy(&semaphore);
		killThread = false;
		alive = false;
		FILE_LOG(logDEBUG) << GetType() << " thread with index " << index << " destroyed successfully.";
	}
}


void ThreadObject::CreateThread() {
	if (alive) {
		throw sls::RuntimeError("Cannot create " + GetType() + " thread " + std::to_string(index) + ". Already alive");
	}
	sem_init(&semaphore,1,0);
	killThread = false;

	if (pthread_create(&thread, nullptr,StartThread, (void*) this)){
		throw sls::RuntimeError("Could not create " + GetType() + " thread with index " + std::to_string(index));
	}
	alive = true;
	FILE_LOG(logDEBUG) << GetType() << " thread " << index << " created successfully.";
}


void* ThreadObject::StartThread(void* thisPointer) {
	((ThreadObject*)thisPointer)->RunningThread();
	return thisPointer;
}


void ThreadObject::RunningThread() {
	FILE_LOG(logINFOBLUE) << "Created [ " << GetType() << "Thread " << index << ", "
			"Tid: " << syscall(SYS_gettid) << "]";
	while(true)	{

		while(IsRunning()) {

			ThreadExecution();

		}//end of inner loop


		//wait till the next acquisition
		sem_wait(&semaphore);

		if(killThread)	{
			FILE_LOG(logINFOBLUE) << "Exiting [ " << GetType() <<
					" Thread " << index << ", Tid: " << syscall(SYS_gettid) << "]";
			pthread_exit(nullptr);
		}

	}//end of outer loop
}




void ThreadObject::Continue() {
	sem_post(&semaphore);
}



