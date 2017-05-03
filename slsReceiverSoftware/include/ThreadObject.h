#pragma once
/************************************************
 * @file ThreadObject.h
 * @short creates/destroys a thread
 ***********************************************/
/**
 *@short creates/destroys a thread
 */

#include "sls_receiver_defs.h"
#include "logger.h"

#include <pthread.h>
#include <semaphore.h>
#include <string>

class ThreadObject : private virtual slsReceiverDefs {
	
 public:
	/**
	 * Constructor
	 * @param ind self index
	 */
	ThreadObject(int ind);

	/**
	 * Destructor
	 * if alive, destroys thread
	 */
	virtual ~ThreadObject();

	/**
	 * Print all member values
	 */
	void PrintMembers();


	/**
	 * Get Type
	 * @return type
	 */
	virtual std::string GetType() = 0;

	/**
	 * Returns if the thread is currently running
	 * @returns true if thread is running, else false
	 */
	virtual bool IsRunning() = 0;

	/**
	 * What is really being executed in the thread
	 */
	virtual void ThreadExecution() = 0;

	/**
	 * Post semaphore so thread can continue & start an acquisition
	 */
	void Continue();

 protected:

	/**
	 * Destroy thread, semaphore and resets alive and killThread
	 */
	void DestroyThread();

	/**
	 * Create Thread, sets semaphore, alive and killThread
	 * @return OK if successful, else FAIL
	 */
	int CreateThread();


 private:

	/**
	 * Static function using pointer from argument to call RunningThread()
	 * @param thisPointer pointer to an object of ThreadObject
	 */
	static void* StartThread(void *thisPointer);

	/**
	 * Actual Thread called:  An infinite while loop in which,
	 * semaphore starts executing its contents as long RunningMask is satisfied
	 * Then it exits the thread on its own if killThread is true
	 */
	void RunningThread();


 protected:
	/** Self Index */
	int index;

	/** Thread is alive/dead */
	volatile bool alive;

	/** Variable monitored by thread to kills itself */
	volatile bool killThread;

	/** Thread variable */
	pthread_t thread;

	/** Semaphore to synchonize starting of each run */
	sem_t semaphore;



};

