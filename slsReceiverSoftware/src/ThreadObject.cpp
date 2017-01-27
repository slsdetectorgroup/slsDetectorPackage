/************************************************
 * @file ThreadObject.h
 * @short creates/destroys a thread
 ***********************************************/



#include "ThreadObject.h"

#include <iostream>
using namespace std;



ThreadObject::ThreadObject(int ind):
		index(ind),
		alive(false),
		killThread(false),
		thread(0) {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	PrintMembers();
}


ThreadObject::~ThreadObject() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	DestroyThread();
}


void ThreadObject::PrintMembers() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	FILE_LOG(logDEBUG) 	<< "Index : " << 	index
						<< "\nalive: " <<	alive
						<< "\nkillThread: " << killThread
						<< "\npthread: " << thread;
}


void  ThreadObject::DestroyThread() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	if(alive){
		killThread = true;
		sem_post(&semaphore);
		pthread_join(thread,NULL);
		sem_destroy(&semaphore);
		killThread = false;
		alive = false;
		FILE_LOG(logDEBUG) << GetType() << " thread with index " << index << " destroyed successfully.";
	}
}


int ThreadObject::CreateThread() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	if(alive){
		FILE_LOG(logERROR) << "Cannot create thread " << index << ". Already alive";
		return FAIL;
	}
	sem_init(&semaphore,1,0);
	killThread = false;

	if(pthread_create(&thread, NULL,StartThread, (void*) this)){
		FILE_LOG(logERROR) << "Could not create "  << GetType() << " thread with index " << index;
		return FAIL;
	}
	alive = true;
	FILE_LOG(logINFO) << GetType() << " thread " << index << " created successfully.";

	return OK;
}


void* ThreadObject::StartThread(void* thisPointer) {
		FILE_LOG(logDEBUG) << __AT__ << " called";
		((ThreadObject*)thisPointer)->RunningThread();
		return thisPointer;
}


void ThreadObject::RunningThread() {
	FILE_LOG(logDEBUG) << __AT__ << " called";


	while(true)	{

		while(IsRunning()) {

			ThreadExecution();

		}/*--end of inner loop */


		//wait till the next acquisition
		sem_wait(&semaphore);

		if(killThread)	{
			cprintf(BLUE,"%s Thread %d: Goodbye\n",GetType().c_str(),index);
			pthread_exit(NULL);
		}

	}/*--end of loop for each acquisition (outer loop) */
}




void ThreadObject::Continue() {
	sem_post(&semaphore);
}



