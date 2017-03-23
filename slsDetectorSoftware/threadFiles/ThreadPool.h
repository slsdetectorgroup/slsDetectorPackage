#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <vector>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "Mutex.h"
#include "Task.h"
#include "CondVar.h"
#include "Global.h"
#include <semaphore.h>
using namespace std;


class ThreadPool
{
public:
  ThreadPool(int pool_size);
  ~ThreadPool();
  int initialize_threadpool();
  int destroy_threadpool();
  void* execute_thread();
  int add_task(Task* task);
  void startExecuting();
  void wait_for_tasks_to_complete();
  void setzeromqThread();

private:
  int m_pool_size;
  Mutex m_task_mutex;
  CondVar m_task_cond_var;
  std::vector<pthread_t> m_threads; // storage for threads
  std::deque<Task*> m_tasks;
  volatile int m_pool_state;

  volatile bool m_tasks_loaded;
  volatile bool thread_started;
  int current_thread_number;

  //volatile uint64_t tasks_done_mask;
  volatile int number_of_ongoing_tasks;
  volatile int number_of_total_tasks;

 sem_t semStart;
 sem_t semDone;
 bool zmqthreadpool;
};

