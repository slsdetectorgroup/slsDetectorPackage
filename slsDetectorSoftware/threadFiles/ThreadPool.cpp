#include "ThreadPool.h"


ThreadPool::ThreadPool(int pool_size) : m_pool_size(pool_size){
#ifdef VERBOSE
	cout << "Constructed ThreadPool of size " << m_pool_size << endl;
#endif
	m_tasks_loaded = false;
	thread_started = false;
	zmqthreadpool = false;
	current_thread_number = -1;
	number_of_ongoing_tasks = 0;
	number_of_total_tasks = 0;
}

ThreadPool::~ThreadPool(){
	// Release resources
	if (m_pool_state != STOPPED) {
		destroy_threadpool();
	}
}


extern "C"
void* start_thread(void* arg)
{
	ThreadPool* tp = (ThreadPool*) arg;
	tp->execute_thread();
	return NULL;
}

int ThreadPool::initialize_threadpool(){
	if(m_pool_size == 1)
		return m_pool_size;

	m_pool_state = STARTED;
	int ret = -1;
	sem_init(&semStart,1,0);
	sem_init(&semDone,1,0);
	for (int i = 0; i < m_pool_size; i++) {
		pthread_t tid;
		thread_started = false;
		current_thread_number = i;
		ret = pthread_create(&tid, NULL, start_thread, (void*) this);
		if (ret != 0) {
			cerr << "pthread_create() failed: " << ret << endl;
			return 0;
		}
		m_threads.push_back(tid);
		while(!thread_started);
	}
#ifdef VERBOSE
	cout << m_pool_size << " threads created by the thread pool" << endl;
#endif
	return m_pool_size;
}

int ThreadPool::destroy_threadpool(){
	if(m_pool_size == 1)
		return 0;
	/*cout << "in destroying threadpool" << endl;*/
	// thread communication- modified m_pool_state may not show up
	//to other threads until its modified in a lock!
	m_task_mutex.lock();
	m_pool_state = STOPPED;
	m_task_mutex.unlock();
	/*cout << "Broadcasting STOP signal to all threads..." << endl;*/
	m_task_cond_var.broadcast(); // notify all threads we are shttung down

//	int ret = -1;
	for (int i = 0; i < m_pool_size; i++) {
		void* result;
		sem_post(&semStart);
		sem_post(&semDone);
		//ret =
		pthread_join(m_threads[i], &result);
		/*cout << "pthread_join() returned " << ret << ": " << strerror(errno) << endl;*/
		m_task_cond_var.broadcast(); // try waking up a bunch of threads that are still waiting
	}
	sem_destroy(&semStart);
	sem_destroy(&semDone);
	number_of_ongoing_tasks = 0;
	number_of_total_tasks = 0;
	/* cout << m_pool_size << " threads exited from the thread pool" << endl;*/
	return 0;
}

void* ThreadPool::execute_thread(){
//for debugging seting ithread value
//	int ithread = current_thread_number;
	thread_started = true;
	Task* task = NULL;
	m_tasks_loaded = false;
	/*cout << "Starting thread " << pthread_self() << endl;*/
	while(true) {
		// Try to pick a task
		/*cout << "Locking: " << pthread_self() << endl;*/
		m_task_mutex.lock();

		while ((m_pool_state != STOPPED) && (m_tasks.empty())) {
			// Wait until there is a task in the queue
			// Unlock mutex while wait, then lock it back when signaled
			/* cout << "Unlocking and waiting: " << pthread_self() << endl;*/
			m_task_cond_var.wait(m_task_mutex.get_mutex_ptr());
			/* cout << "Signaled and locking: " << pthread_self() << endl;*/
		}

		// If the thread was woken up to notify process shutdown, return from here
		if (m_pool_state == STOPPED) {
			/* cout << "Unlocking and exiting: " << pthread_self() << endl;*/
			m_task_mutex.unlock();
			pthread_exit(NULL);
		}

		task = m_tasks.front();
		m_tasks.pop_front();
		/*cout << "Unlocking: " << pthread_self() << endl;*/
		m_task_mutex.unlock();

		//if(zmqthreadpool) cout<<"***"<<ithread<<" semaphore start address wait:"<<&semStart<<endl;
		sem_wait(&semStart);

		//cout<<"***"<<ithread<<" checking out semaphore done address:"<<&semDone<<endl;

		/*cout << ithread <<" Executing thread " << pthread_self() << endl;*/
		// execute the task
		(*task)(); // could also do task->run(arg);
		/*cout << ithread <<" Done executing thread " << pthread_self() << endl;*/

		delete task;
		/*cout << ithread << " task deleted" << endl;*/

		m_task_mutex.lock();
		number_of_ongoing_tasks--;
		m_task_mutex.unlock();

		//if(zmqthreadpool) cout<<ithread <<" task  done: "<<number_of_ongoing_tasks<<endl;
		//last task and check m_tasks_loaded to ensure done only once
		if((!number_of_ongoing_tasks) && m_tasks_loaded){
			//if(zmqthreadpool) cout << ithread << " all tasks done."<<endl;
			m_tasks_loaded = false;
		}
		//if(zmqthreadpool) cout<<"***"<<ithread<<" semaphore done address post:"<<&semDone<<endl;


		sem_post(&semDone);
		//removed deleteing task to earlier
	}
	return NULL;
}

int ThreadPool::add_task(Task* task){
	if(m_pool_size == 1){
		(*task)();
		return 0;
	}
	m_task_mutex.lock();
	// TODO: put a limit on how many tasks can be added at most
	m_tasks.push_back(task);
	number_of_ongoing_tasks++;
	number_of_total_tasks++;
	m_task_cond_var.signal(); // wake up one thread that is waiting for a task to be available
	m_task_mutex.unlock();
	return 0;
}

void ThreadPool::startExecuting(){
	if(m_pool_size == 1)
			return;

		/*cout << "waiting for tasks: locked. gonna wait" << endl;*/
		m_tasks_loaded = true;

		//giving all threads permission to start as all tasks have been added
		//if(zmqthreadpool) cout<<"*** semaphore start address post:"<<&semStart<<endl;
		for(int i=0;i<number_of_total_tasks;i++)
			sem_post(&semStart);

}

void ThreadPool::wait_for_tasks_to_complete(){
	if(m_pool_size == 1)
		return;
	//if(zmqthreadpool) cout<<"waiting for all tasks to be done "<<endl;
	//if(zmqthreadpool) cout<<"*** semaphore done address wait:"<<&semDone<<endl;
	for(int i=0;i<number_of_total_tasks;i++)
		sem_wait(&semDone);
	number_of_total_tasks = 0;
	//if(zmqthreadpool) cout<<"complete"<<endl<<endl;
}

void ThreadPool::setzeromqThread(){
	zmqthreadpool = true;
}
