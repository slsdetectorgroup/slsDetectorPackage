/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "communication_funcs.h"

#include "slsDetectorFunctionList.h"/*#include "slsDetector_firmware.h" for the time being*/
#include "slsDetectorServer_defs.h"

#include <stdio.h>
#include <stdlib.h>

int sockfd;

int main(int argc, char *argv[])
{
  int  portno;
  int retval=0;
  int sd,fd;

  portno = DEFAULT_PORTNO;


 sd=bindSocket(portno); //defined in communication_funcs
  if (getServerError(sd)) //defined in communication_funcs
    return -1;



  /* waits for connection */
  while(retval!=GOODBYE) {
#ifdef VERBOSE
    printf("\n");
#endif
#ifdef VERY_VERBOSE
    printf("Stop server: waiting for client call\n");
#endif
    fd=acceptConnection(sd);  //defined in communication_funcs
    retval=stopStateMachine();//defined in slsDetectorFirmare_funcs
    closeConnection(fd);	//defined in communication_funcs
  }

  exitServer(sd); //defined in communication_funcs
  printf("Goodbye!\n");

  return 0; 
}

