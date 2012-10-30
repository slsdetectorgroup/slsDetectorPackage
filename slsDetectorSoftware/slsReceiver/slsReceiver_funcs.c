//#include "sls_detector_defs.h"
#include "slsReceiver_funcs.h"
#include "slsReceiverFunctionList.h"

#include "sls_detector_defs.h"
#include "sls_detector_funcs.h"

#include <string.h>
#include <stdlib.h>

// Global variables

int (*flist[256])(int);


//defined in the detector specific file
#ifdef MYTHEND
const enum detectorType myDetectorType=MYTHEN;
#elif GOTTHARDD
const enum detectorType myDetectorType=GOTTHARD;
#elif EIGERD
const enum detectorType myDetectorType=EIGER;
#elif PICASSOD
const enum detectorType myDetectorType=PICASSO;
#else
const enum detectorType myDetectorType=GENERIC;
#endif


//define in communication_funcs
extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;



/* global variables for optimized readout */

char *dataretval=NULL;
int dataret;
char mess[1000];
int dataBytes;



/*
int init_receiver() {


	initializeReceiver();

	strcpy(mess,"dummy message");
	strcpy(lastClientIP,"none");
	strcpy(thisClientIP,"none1");
	lockStatus=0;
	return OK;
}
 */

int decode_function(int file_des) {
	int fnum,n;
	int retval=FAIL;
#ifdef VERBOSE
	printf( "receive data\n");
#endif
	n = receiveDataOnly(file_des,&fnum,sizeof(fnum));
	if (n <= 0) {
#ifdef VERBOSE
		printf("ERROR reading from socket %d, %d %d\n", n, fnum, file_des);
#endif
		return FAIL;
	}
#ifdef VERBOSE
	else
		printf("size of data received %d\n",n);
#endif

#ifdef VERBOSE
	printf( "calling function fnum = %d %x\n",fnum,flist[fnum]);
#endif
	if (fnum<0 || fnum>255)
		fnum=255;
	retval=(*flist[fnum])(file_des);
	if (retval==FAIL)
		printf( "Error executing the function = %d \n",fnum);
	return retval;
}


int function_table() {
	int i;
	for (i=0;i<256;i++){
		flist[i]=&M_nofunc;
	}
	flist[F_EXIT_SERVER]=&exit_server;		//not implemented in client
	flist[F_EXEC_COMMAND]=&exec_command;	//not implemented in client


	flist[F_SET_FILE_NAME]=&set_file_name;
	flist[F_SET_FILE_PATH]=&set_file_dir;
	flist[F_SET_FILE_INDEX]=&set_file_index;
	flist[F_START_RECEIVER]=&start_receiver;
	flist[F_STOP_RECEIVER]=&stop_receiver;
	flist[F_GET_RECEIVER_STATUS]=&get_receiver_status;
	flist[F_GET_FRAMES_CAUGHT]=&get_frames_caught;
	flist[F_GET_FRAME_INDEX]=&get_frame_index;

	flist[F_LOCK_RECEIVER]=&lock_receiver;
	flist[F_SET_PORT]=&set_port;
	flist[F_GET_LAST_CLIENT_IP]=&get_last_client_ip;
	flist[F_UPDATE_CLIENT]=&update_client;


#ifdef VERBOSE
	/*  for (i=0;i<256;i++){
    printf("function %d located at %x\n",i,flist[i]);
    }*/
#endif
	return OK;
}






int set_file_name(int file_des) {
	int ret=OK;
	int n=0;
	char fName[MAX_STR_LENGTH],retval[MAX_STR_LENGTH]="";

	strcpy(mess,"Could not set file name");


	/* receive arguments */
	n = receiveDataOnly(file_des,fName,MAX_STR_LENGTH);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	/* execute action if the arguments correctly arrived*/
	if (ret==OK) {
#ifdef SLS_RECEIVER_FUNCTION_LIST

		if (lockStatus==1 && differentClients==1){//necessary???
			sprintf(mess,"Receiver locked by %s\n", lastClientIP);
			ret=FAIL;
		}
		else if((strlen(fName))&&(getReceiverStatus()==RUNNING)){
			strcpy(mess,"Can not set file name while receiver running");
			ret = FAIL;
		}
		else
			strcpy(retval,setFileName(fName));
	}

#endif
#ifdef VERBOSE
	if(ret!=FAIL)
		printf("file name:%s\n",retval);
	else
		printf("%s",mess);
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	n = sendDataOnly(file_des,retval,MAX_STR_LENGTH);

	/*return ok/fail*/
	return ret;
}







int set_file_dir(int file_des) {
	int ret=OK;
	int n=0;
	char fPath[MAX_STR_LENGTH],retval[MAX_STR_LENGTH]="";

	strcpy(mess,"Could not set file path\n");


	/* receive arguments */
	n = receiveDataOnly(file_des,fPath,MAX_STR_LENGTH);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	/* execute action if the arguments correctly arrived*/
	if (ret==OK) {
#ifdef SLS_RECEIVER_FUNCTION_LIST

		if (lockStatus==1 && differentClients==1){//necessary???
			sprintf(mess,"Receiver locked by %s\n", lastClientIP);
			ret=FAIL;
		}
		else if((strlen(fPath))&&(getReceiverStatus()==RUNNING)){
			strcpy(mess,"Can not set file path while receiver running\n");
			ret = FAIL;
		}
		else{
			strcpy(retval,setFilePath(fPath));
			/* if file path doesnt exist*/
			if(strlen(fPath))
				if (strcmp(retval,fPath)){
					strcpy(mess,"receiver file path does not exist\n");
					ret=FAIL;
				}
		}

	}
#endif
#ifdef VERBOSE
	if(ret!=FAIL)
		printf("file path:%s\n",retval);
	else
		printf("%s",mess);
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	n = sendDataOnly(file_des,retval,MAX_STR_LENGTH);

	/*return ok/fail*/
	return ret;
}







int set_file_index(int file_des) {
	int ret=OK;
	int n=0;
	int index,retval=-1;

	strcpy(mess,"Could not set file index\n");


	/* receive arguments */
	n = receiveDataOnly(file_des,&index,sizeof(index));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	/* execute action if the arguments correctly arrived*/
	if (ret==OK) {
#ifdef SLS_RECEIVER_FUNCTION_LIST

		if (lockStatus==1 && differentClients==1){//necessary???
			sprintf(mess,"Receiver locked by %s\n", lastClientIP);
			ret=FAIL;
		}
		else if((index>=0)&&(getReceiverStatus()==RUNNING)){
			strcpy(mess,"Can not set file index while receiver running\n");
			ret = FAIL;
		}
		else
			retval=setFileIndex(index);
	}

#endif
#ifdef VERBOSE
	if(ret!=FAIL)
		printf("file index:%d\n",retval);
	else
		printf("%s",mess);
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	n = sendDataOnly(file_des,&retval,sizeof(retval));

	/*return ok/fail*/
	return ret;
}








int start_receiver(int file_des) {
	int ret=OK;
	int n=0;

	strcpy(mess,"Could not start receiver\n");

	/* execute action if the arguments correctly arrived*/
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (lockStatus==1 && differentClients==1){//necessary???
		sprintf(mess,"Receiver locked by %s\n", lastClientIP);
		ret=FAIL;
	}
	else if(!strlen(setFilePath(""))){
		strcpy(mess,"receiver not set up. set receiver ip again.\n");
		ret = FAIL;
	}
	else if(getReceiverStatus()!=RUNNING)
		ret=startReceiver();
#endif


	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	/*return ok/fail*/
	return ret;
}






int stop_receiver(int file_des) {
	int ret=OK;
	int n=0;

	strcpy(mess,"Could not stop receiver\n");

	/* execute action if the arguments correctly arrived*/
#ifdef SLS_RECEIVER_FUNCTION_LIST
	if (lockStatus==1 && differentClients==1){//necessary???
		sprintf(mess,"Receiver locked by %s\n", lastClientIP);
		ret=FAIL;
	}
	else if(getReceiverStatus()!=IDLE)
		ret=stopReceiver();
#endif


	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	/*return ok/fail*/
	return ret;
}






int get_receiver_status(int file_des) {
	int ret=OK;
	int n=0;
	enum runStatus retval;

	/* execute action if the arguments correctly arrived*/
#ifdef SLS_RECEIVER_FUNCTION_LIST
	retval=getReceiverStatus();
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	n = sendDataOnly(file_des,&retval,sizeof(retval));
	/*return ok/fail*/
	return ret;
}








int get_frames_caught(int file_des) {
	int ret=OK;
	int n=0;
	int retval=-1;

	/* execute action if the arguments correctly arrived*/
#ifdef SLS_RECEIVER_FUNCTION_LIST
	retval=getFramesCaught();
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	n = sendDataOnly(file_des,&retval,sizeof(retval));
	/*return ok/fail*/
	return ret;
}










int get_frame_index(int file_des) {
	int ret=OK;
	int n=0;
	int retval=-1;

	/* execute action if the arguments correctly arrived*/
#ifdef SLS_RECEIVER_FUNCTION_LIST
	retval=getFrameIndex();
#endif

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	n = sendDataOnly(file_des,&retval,sizeof(retval));
	/*return ok/fail*/
	return ret;
}



























int  M_nofunc(int file_des){

	int retval=FAIL;
	sprintf(mess,"Unrecognized Function\n");
	printf(mess);
	sendDataOnly(file_des,&retval,sizeof(retval));
	sendDataOnly(file_des,mess,sizeof(mess));
	return GOODBYE;
}






int exit_server(int file_des) {
	int retval=FAIL;
	sendDataOnly(file_des,&retval,sizeof(retval));
	printf("closing server.");
	sprintf(mess,"closing server");
	sendDataOnly(file_des,mess,sizeof(mess));
	return GOODBYE;
}

int exec_command(int file_des) {
	char cmd[MAX_STR_LENGTH];
	char answer[MAX_STR_LENGTH];
	int retval=OK;
	int sysret=0;
	int n=0;

	// receive arguments
	n = receiveDataOnly(file_des,cmd,MAX_STR_LENGTH);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		retval=FAIL;
	}

	// execute action if the arguments correctly arrived
	if (retval==OK) {
		//#ifdef VERBOSE
		printf("executing command %s\n", cmd);
		//#endif
		if (lockStatus==0 || differentClients==0)
			sysret=system(cmd);

		//should be replaced by popen
		if (sysret==0) {
			sprintf(answer,"Succeeded\n");
			if (lockStatus==1 && differentClients==1)
				sprintf(answer,"Detector locked by %s\n", lastClientIP);
		} else {
			sprintf(answer,"Failed\n");
			retval=FAIL;
		}
	} else {
		sprintf(answer,"Could not receive the command\n");
	}

	// send answer
	n = sendDataOnly(file_des,&retval,sizeof(retval));
	n = sendDataOnly(file_des,answer,MAX_STR_LENGTH);
	if (n < 0) {
		sprintf(mess,"Error writing to socket");
		retval=FAIL;
	}


	//return ok/fail
	return retval;

}

int lock_receiver(int file_des) {


  int n;
  int ret=OK;

  int lock;
  n = receiveDataOnly(file_des,&lock,sizeof(lock));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    printf("Error reading from socket (lock)\n");
    ret=FAIL;
  }
  if (lock>=0) {
    if (lockStatus==0 || strcmp(lastClientIP,thisClientIP)==0 || strcmp(lastClientIP,"none")==0) {
      lockStatus=lock;
      strcpy(lastClientIP,thisClientIP);
    }   else {
      ret=FAIL;
      sprintf(mess,"Receiver already locked by %s\n", lastClientIP);
    }
  }
 if (differentClients && ret==OK)
   ret=FORCE_UPDATE;

  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n = sendDataOnly(file_des,mess,sizeof(mess));
  }  else
    n = sendDataOnly(file_des,&lockStatus,sizeof(lockStatus));

  return ret;

}



int set_port(int file_des) {
  int n;
  int ret=OK;
  int sd=-1;

  enum portType p_type; /** data? control? stop? Unused! */
  int p_number; /** new port number */

  n = receiveDataOnly(file_des,&p_type,sizeof(p_type));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    printf("Error reading from socket (ptype)\n");
    ret=FAIL;
  }

  n = receiveDataOnly(file_des,&p_number,sizeof(p_number));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    printf("Error reading from socket (pnum)\n");
    ret=FAIL;
  }
  if (differentClients==1 && lockStatus==1 ) {
    ret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);
  }  else {
    if (p_number<1024) {
      sprintf(mess,"Too low port number %d\n", p_number);
      printf("\n");
      ret=FAIL;
    }

    printf("set port %d to %d\n",p_type, p_number);

    sd=bindSocket(p_number);
  }
    if (sd>=0) {
      ret=OK;
      if (differentClients )
	ret=FORCE_UPDATE;
    } else {
      ret=FAIL;
      sprintf(mess,"Could not bind port %d\n", p_number);
      printf("Could not bind port %d\n", p_number);
      if (sd==-10) {
      sprintf(mess,"Port %d already set\n", p_number);
      printf("Port %d already set\n", p_number);

      }
    }

    n = sendDataOnly(file_des,&ret,sizeof(ret));
    if (ret==FAIL) {
      n = sendDataOnly(file_des,mess,sizeof(mess));
    } else {
      n = sendDataOnly(file_des,&p_number,sizeof(p_number));
      closeConnection(file_des);
      exitServer(sockfd);
      sockfd=sd;

    }

  return ret;

}

int get_last_client_ip(int file_des) {
  int ret=OK;
  int n;
 if (differentClients )
   ret=FORCE_UPDATE;
  n = sendDataOnly(file_des,&ret,sizeof(ret));
  n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));

  return ret;

}


int send_update(int file_des) {

  int ret=OK;
  int n,ind;
  char path[MAX_STR_LENGTH];


  n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));
  //index
  ind=getFileIndex(-1);
  n = sendDataOnly(file_des,&ind,sizeof(ind));
  //filepath
  strcpy(path,getFilePath(""));
  n = sendDataOnly(file_des,path,MAX_STR_LENGTH);
  //filename
  strcpy(path,getFileName(""));
  n = sendDataOnly(file_des,path,MAX_STR_LENGTH);

  if (lockStatus==0) {
    strcpy(lastClientIP,thisClientIP);
  }

  return ret;


}


int update_client(int file_des) {

  int ret=OK;
  sendDataOnly(file_des,&ret,sizeof(ret));
  return send_update(file_des);



}

/*
int set_master(int file_des) {

  enum masterFlags retval=GET_MASTER;
  enum masterFlags arg;
  int n;
  int ret=OK;
  // int regret=OK;


  sprintf(mess,"can't set master flags\n");


  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }


#ifdef VERBOSE
  printf("setting master flags  to %d\n",arg);
#endif

  if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
    ret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);
  }  else {
    retval=setMaster(arg);

  }
  if (retval==GET_MASTER) {
    ret=FAIL;
  }
  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n = sendDataOnly(file_des,mess,sizeof(mess));
  } else {
    n = sendDataOnly(file_des,&retval,sizeof(retval));
  }
  return ret;
}






int set_synchronization(int file_des) {

  enum synchronizationMode retval=GET_MASTER;
  enum synchronizationMode arg;
  int n;
  int ret=OK;
  //int regret=OK;


  sprintf(mess,"can't set synchronization mode\n");


  n = receiveDataOnly(file_des,&arg,sizeof(arg));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    ret=FAIL;
  }
#ifdef VERBOSE
  printf("setting master flags  to %d\n",arg);
#endif

  if (differentClients==1 && lockStatus==1 && arg!=GET_READOUT_FLAGS) {
    ret=FAIL;
    sprintf(mess,"Detector locked by %s\n",lastClientIP);
  }  else {
    retval=setSynchronization(arg);
  }
  if (retval==GET_SYNCHRONIZATION_MODE) {
    ret=FAIL;
  }
  n = sendDataOnly(file_des,&ret,sizeof(ret));
  if (ret==FAIL) {
    n = sendDataOnly(file_des,mess,sizeof(mess));
  } else {
    n = sendDataOnly(file_des,&retval,sizeof(retval));
  }
  return ret;
}
*/
