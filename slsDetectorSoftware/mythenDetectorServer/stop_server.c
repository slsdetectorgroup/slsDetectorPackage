/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "communication_funcs.h"
#include "firmware_funcs.h"


int sockfd;

int main(int argc, char *argv[])
{
  int  portno;
  int retval=0;

  portno = DEFAULT_PORTNO;


  bindSocket(portno);
  if (getServerError())
    return -1;



  /* waits for connection */
  while(retval!=GOODBYE) {
#ifdef VERBOSE
    printf("\n");
#endif
#ifdef VERY_VERBOSE
    printf("Stop server: waiting for client call\n");
#endif
    acceptConnection();
    retval=stopStateMachine();
    closeConnection();
  }

  exitServer();
  printf("Goodbye!\n");

  return 0; 
}

