#include "postProcessing.h"

#include <time.h>


static void* startProcessData(void *n){
   postProcessing *myDet=(postProcessing*)n;
   myDet->processData();
   pthread_exit(NULL);
   
};



int postProcessing::kbhit(){
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
}


postProcessing::postProcessing():
		threadedProcessing(0),
		jointhread(0),
		acquiringDone(0),
		fdata(0),
		thisData(0),
		dataReady(0),
		pCallbackArg(0)
		{
  pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER;
  mp=mp1;
  pthread_mutex_init(&mp, NULL);  
  mg=mp1;
  pthread_mutex_init(&mg, NULL);
  ms=mp1;
  pthread_mutex_init(&ms, NULL);

}





postProcessing::~postProcessing(){
}


void* postProcessing::processData() {
	if(setReceiverOnline()==OFFLINE_FLAG){
		return 0;
	} //receiver 
	else{
 		//cprintf(RED,"In post processing threads\n");

 		 if(dataReady) {
 			readFrameFromReceiver();
 		 }
 		//only update progress
 		else{
 			int caught = -1;
 			char c;
 			int ifp;
			while(true){

				// set only in startThread
				if (*threadedProcessing==0)
					setTotalProgress();

				// to exit acquire by typing q
				ifp=kbhit();
				if (ifp!=0){
					c=fgetc(stdin);
					if (c=='q') {
						std::cout<<"Caught the command to stop acquisition"<<std::endl;
						stopAcquisition();
					}
				}


 				//get progress
 				if(setReceiverOnline() == ONLINE_FLAG){
 					pthread_mutex_lock(&mg);
 					caught = getFramesCaughtByAnyReceiver();
 					pthread_mutex_unlock(&mg);
 				}


 				//updating progress
 				if(caught!= -1){
 					setCurrentProgress(caught);
 #ifdef VERY_VERY_DEBUG
 				std::cout << "caught:" << caught << std::endl;
 #endif
 				}

 				// exiting loop
 				if (*threadedProcessing==0)
 					break;
 				if (checkJoinThread()){
 					break;
 				}

 				usleep(100 * 1000); //20ms need this else connecting error to receiver (too fast)
 			}
 		}
	}

	return 0;
}



int postProcessing::checkJoinThread() {
  int retval;
  pthread_mutex_lock(&mp);
  retval=jointhread;
  pthread_mutex_unlock(&mp);
  return retval;
}

void postProcessing::setJoinThread( int v) {
  pthread_mutex_lock(&mp);
  jointhread=v;
  pthread_mutex_unlock(&mp);
}



void postProcessing::startThread() {


  setTotalProgress();

#ifdef VERBOSE
  std::cout << "start thread stuff"  << std::endl;
#endif

  pthread_attr_t tattr;
  int ret;
  sched_param param, mparam;
  int policy= SCHED_OTHER;

  // set the priority; others are unchanged
  //newprio = 30;
  mparam.sched_priority =1;
  param.sched_priority =1;   


   /* Initialize and set thread detached attribute */
   pthread_attr_init(&tattr);
   pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);

  ret = pthread_setschedparam(pthread_self(), policy, &mparam);


    ret = pthread_create(&dataProcessingThread, &tattr,startProcessData, (void*)this);

  if (ret)
 	  printf("ret %d\n", ret);

  pthread_attr_destroy(&tattr);

  // scheduling parameters of target thread
  ret = pthread_setschedparam(dataProcessingThread, policy, &param);
  
}



