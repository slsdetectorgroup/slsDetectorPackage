/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "communication_funcs.h"
#include "slsReceiver_funcs.h"
#include <stdlib.h>


extern int sockfd;


void error(char *msg)
{
    perror(msg);
}

int main(int argc, char *argv[])
{
  int  portno = DEFAULT_PORTNO+2;
  int retval = OK;
  int sd, fd;


  //init_receiver(argv[1]); //defined in slsReceiver_funcs


  sd=bindSocket(portno); //defined in communication_funcs

  sockfd=sd;


  if (getServerError(sd)) {  //defined in communication_funcs
    printf("server error!\n");
    return -1;
  }

  /* assign function table */
  function_table();  //defined in slsReceiver_funcs
#ifdef VERBOSE
  printf("function table assigned \n");
#endif

printf("Ready...\n");

  /* waits for connection */
  while(retval!=GOODBYE) {
#ifdef VERBOSE
    printf("\n");
#endif
#ifdef VERY_VERBOSE
    printf("Waiting for client call\n");
#endif
    fd=acceptConnection(sockfd);  //defined in communication_funcs
#ifdef VERY_VERBOSE
    printf("Conenction accepted\n");
#endif
    if (fd>0) {
      retval=decode_function(fd);   //defined in slsReceiverServer_funcs
#ifdef VERY_VERBOSE
      printf("function executed\n");
#endif
      closeConnection(fd);  //defined in communication_funcs
#ifdef VERY_VERBOSE
      printf("connection closed\n");
#endif
    }
  }

  exitServer(sockfd); //defined in communication_funcs
  printf("Goodbye!\n");

  return 0;
}

