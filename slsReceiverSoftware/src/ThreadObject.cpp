/************************************************
 * @file ThreadObject.cpp
 * @short creates/destroys a thread
 ***********************************************/



#include "ThreadObject.h"

#include <iostream>
using namespace std;



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
	FILE_LOG (logDEBUG) 	<< "Index : " << 	index
						<< "\nalive: " <<	alive
						<< "\nkillThread: " << killThread
						<< "\npthread: " << thread;
}


void  ThreadObject::DestroyThread() {
	if(alive){
		killThread = true;
		sem_post(&semaphore);
		pthread_join(thread,NULL);
		sem_destroy(&semaphore);
		killThread = false;
		alive = false;
		FILE_LOG (logDEBUG) << GetType() << " thread with index " << index << " destroyed successfully.";
	}
}


int ThreadObject::CreateThread() {
	if(alive){
		FILE_LOG (logERROR) << "Cannot create thread " << index << ". Already alive";
		return FAIL;
	}
	sem_init(&semaphore,1,0);
	killThread = false;

	if(pthread_create(&thread, NULL,StartThread, (void*) this)){
		FILE_LOG (logERROR) << "Could not create "  << GetType() << " thread with index " << index;
		return FAIL;
	}
	alive = true;
	FILE_LOG (logDEBUG) << GetType() << " thread " << index << " created successfully.";

	return OK;
}


void* ThreadObject::StartThread(void* thisPointer) {
	((ThreadObject*)thisPointer)->RunningThread();
	return thisPointer;
}


void ThreadObject::RunningThread() {
	while(true)	{

		while(IsRunning()) {

			ThreadExecution();

		}//end of inner loop


		//wait till the next acquisition
		sem_wait(&semaphore);

		if(killThread)	{
			cprintf(BLUE,"%s Thread %d: Goodbye\n",GetType().c_str(),index);
			pthread_exit(NULL);
		}

	}//end of outer loop
}




void ThreadObject::Continue() {
	sem_post(&semaphore);
}



