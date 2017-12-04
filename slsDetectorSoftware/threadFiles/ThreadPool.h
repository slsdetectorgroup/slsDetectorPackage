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
  void wait_for_tasks_to_complete();

private:
  int m_pool_size;
  Mutex m_task_mutex;
  CondVar m_task_cond_var;
  std::vector<pthread_t> m_threads; // storage for threads
  std::deque<Task*> m_tasks;
  volatile int m_pool_state;

  Mutex m_all_tasks_mutex;
  CondVar m_all_tasks_cond_var;
  bool m_tasks_loaded;
  bool thread_started;
  int current_thread_number;

  //volatile uint64_t tasks_done_mask;
  volatile int number_of_ongoing_tasks;
};

