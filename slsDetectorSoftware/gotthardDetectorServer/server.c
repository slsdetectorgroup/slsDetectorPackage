/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "communication_funcs.h"
#include "server_funcs.h"
#include <stdlib.h>


int sockfd;


void error(char *msg)
{
    perror(msg);
}

int main(int argc, char *argv[])
{
  int  portno, b;
  char cmd[100];
  int retval=OK;
 
  if (argc==1) {
    portno = DEFAULT_PORTNO;
    sprintf(cmd,"%s %d &",argv[0],DEFAULT_PORTNO+1);
    printf("opening control server on port %d\n",portno );
    system(cmd);
    b=1;
  } else {
    portno = DEFAULT_PORTNO+1;
    if ( sscanf(argv[1],"%d",&portno) ==0) {
      printf("could not open stop server: unknown port\n");
      return 1;
    }
    b=0;
    printf("opening stop server on port %d\n",portno);
  }
  init_detector(b); 


  bindSocket(portno);
  if (getServerError()) {
    printf("server error!\n");
    return -1;
  }

  /* assign function table */
  function_table();
#ifdef VERBOSE
  printf("function table assigned \n");
#endif


  /* waits for connection */
  while(retval!=GOODBYE) {
#ifdef VERBOSE
    printf("\n");
#endif
#ifdef VERY_VERBOSE
    printf("Waiting for client call\n");
#endif
    acceptConnection();
#ifdef VERY_VERBOSE
    printf("Conenction accepted\n");
#endif
    retval=decode_function();
#ifdef VERY_VERBOSE
    printf("function executed\n");
#endif
    closeConnection();
#ifdef VERY_VERBOSE
    printf("connection closed\n");
#endif
  }

  exitServer();
  printf("Goodbye!\n");

  return 0; 
}

