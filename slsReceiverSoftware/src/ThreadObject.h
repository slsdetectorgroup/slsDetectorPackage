#pragma once
/************************************************
 * @file ThreadObject.h
 * @short creates/destroys a thread
 ***********************************************/
/**
 *@short creates/destroys a thread
 */

#include "logger.h"
#include "sls_detector_defs.h"

#include <atomic>
#include <future>
#include <semaphore.h>
#include <string>

class ThreadObject : private virtual slsDetectorDefs {
  protected:
    int index{0};

  private:
    std::atomic<bool> killThread{false};
    std::atomic<bool> runningFlag{false};
	std::thread threadObject;
    sem_t semaphore;
    std::string type;

  public:
    ThreadObject(int threadIndex, std::string threadType);
    virtual ~ThreadObject();
    bool IsRunning() const;
    void StartRunning();
    void StopRunning();
    void Continue();
    void SetThreadPriority(int priority);

  private:
    virtual void ThreadExecution() = 0;
    /**
     * Thread called:  An infinite while loop in which,
     * semaphore starts executing its contents as long RunningMask is satisfied
     * Then it exits the thread on its own if killThread is true
     */
    void RunningThread();
};
