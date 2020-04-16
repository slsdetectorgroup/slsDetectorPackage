#pragma once
/************************************************
 * @file ThreadObject.h
 * @short creates/destroys a thread
 ***********************************************/
/**
 *@short creates/destroys a thread
 */

#include "sls_detector_defs.h"
#include "logger.h"


#include <semaphore.h>
#include <string>
#include <atomic>
#include <future>

class ThreadObject : private virtual slsDetectorDefs {
	
 public:
	ThreadObject(int threadIndex, std::string threadType);
	virtual ~ThreadObject();
	bool IsRunning() const;
	void StartRunning();
	void StopRunning();
	void Continue();
	void SetThreadPriority(int priority);

 protected:
 	virtual void ThreadExecution() = 0;

 private:
	/**
	 * Thread called:  An infinite while loop in which,
	 * semaphore starts executing its contents as long RunningMask is satisfied
	 * Then it exits the thread on its own if killThread is true
	 */
	void RunningThread();


 protected:
	int index{0};
	std::string type;
	std::atomic<bool> killThread{false};
	std::atomic<bool> runningFlag{false};
	std::unique_ptr<std::thread> threadObject;
	sem_t semaphore;
};

