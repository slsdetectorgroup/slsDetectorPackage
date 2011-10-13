#include "sls_detector_defs.h"
#include "server_funcs.h"
//#include "server_defs.h"

// Global variables

int (*flist[256])(int);




const enum detectorType myDetectorType=GOTTHARD;
char mess[1000]; 
char stringname[100]="what i want it to be"; 


int init_detector(int b) {
  printf("This is a GOTTHARD detector\n");
  sprintf(mess,"dummy message");
  return OK;
}


int decode_function() {
  int fnum,n;
  int retval=FAIL;
#ifdef VERBOSE
  printf( "receive data\n");
#endif 
  n = receiveDataOnly(&fnum,sizeof(fnum));
  if (n <= 0) {
    printf("ERROR reading from socket %d", n);
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
  retval=(*flist[fnum])(fnum);
  if (retval==FAIL)
    printf( "Error executing the function = %d \n",fnum);    
  return retval;
}






int function_table(int b) {
  int i;
  for (i=0;i<256;i++){
    flist[i]=&M_nofunc;
  }
  flist[F_EXIT_SERVER]=&exit_server;
  flist[F_GET_GOTTHARD]=&getGotthard;
  flist[F_SET_GOTTHARD]=&setGotthard;
  return OK;
}


int  M_nofunc(int fnum){
  
  int retval=FAIL;
  sprintf(mess,"Unrecognized Function %d\n",fnum);
  printf(mess);
  sendDataOnly(&retval,sizeof(retval));
  sendDataOnly(mess,sizeof(mess));
  return GOODBYE;
}



int exit_server(int fnum) {
  int retval=FAIL;
  sendDataOnly(&retval,sizeof(retval));
  printf("closing server.");
  sprintf(mess,"closing server");
  sendDataOnly(mess,sizeof(mess));
  return GOODBYE;
}





int getGotthard(int fnum) {

  int retval=OK;
  int ret=0;
  char val[100];
  int n=0;
  printf("in getgotthard fn\n");
 /* receive arguments
  n = receiveDataOnly(val,sizeof(val));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    retval=FAIL;
  }
 */
  /* execute action if the arguments correctly arrived*/
  if (retval==OK) {

    printf("getting stringname %s\n", stringname);

  }

   /* send ret */
  n = sendDataOnly(&ret,sizeof(ret));
  if (n < 0) {
    sprintf(mess,"Error writing to socket");
    retval=FAIL;
  }
 
  /* send answer */
  n = sendDataOnly(stringname,sizeof(stringname));
  if (n < 0) {
    sprintf(mess,"Error writing to socket");
    retval=FAIL;
  }

  if(retval==FAIL)
    sendDataOnly(mess,sizeof(mess));
 


  /*return ok/fail*/
  return retval; 
 
}

int setGotthard(int fnum) {
 int retval=OK;
  int ret=0;
  char val[100];
  int n=0;
  printf("in setgotthard fn\n");
 /* receive arguments */
  n = receiveDataOnly(val,sizeof(val));
  if (n < 0) {
    sprintf(mess,"Error reading from socket\n");
    retval=FAIL;
  }

  /* execute action if the arguments correctly arrived*/
  if (retval==OK) {

    printf("setting stringname %s\n", val);

  }

  strcpy(stringname, val);

   /* send ret */
  n = sendDataOnly(&ret,sizeof(ret));
  if (n < 0) {
    sprintf(mess,"Error writing to socket");
    retval=FAIL;
  }
 
  /* send answer */
  n = sendDataOnly(stringname,sizeof(stringname));
  if (n < 0) {
    sprintf(mess,"Error writing to socket");
    retval=FAIL;
  }

  if(retval==FAIL)
    sendDataOnly(mess,sizeof(mess));

  /*return ok/fail*/
  return retval; 
  
}

