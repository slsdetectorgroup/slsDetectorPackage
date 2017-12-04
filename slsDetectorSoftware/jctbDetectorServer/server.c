/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"

#include <stdlib.h>
#include "communication_funcs.h"
#include "server_funcs.h"
#include <string.h>



extern int sockfd;
extern int phase_shift;



void error(char *msg)
{
    perror(msg);
}

int main(int argc, char *argv[])
{
	int  portno, b;
	char cmd[500];
	int retval=OK;
	int sd, fd;
	int iarg;
	int checkType = 1;


	for(iarg=1; iarg<argc; iarg++){

		if(!strcasecmp(argv[iarg],"-phaseshift")){
			if(argc==iarg+1){
				printf("No phaseshift given. Exiting.\n");
				return 1;
			}
			if ( sscanf(argv[iarg+1],"%d",&phase_shift)==0) {
				printf("could not decode phase shift\n");
				return 1;
			}
		}

		else if(!strcasecmp(argv[iarg],"-test")){
			if(argc==iarg+1){
				printf("No test condition given. Exiting.\n");
				return 1;
			}
			if(!strcasecmp(argv[iarg+1],"with_gotthard")){
				checkType = 0;
			}else{
				printf("could not decode test condition. Possible arguments: with_gotthard. Exiting\n");
				return 1;
			}

		}
	}


	//stop server
	if ((argc > 2) && (!strcasecmp(argv[2],"stopserver"))){
		portno = DEFAULT_PORTNO+1;
		if ( sscanf(argv[1],"%d",&portno) ==0) {
			printf("could not open stop server: unknown port\n");
			return 1;
		}
		b=0;
		printf("\n\nStop Server\nOpening stop server on port %d\n",portno);
		checkType=0;

	}

	//control server
	else {
		portno = DEFAULT_PORTNO;
		if(checkType)
			sprintf(cmd,"%s %d stopserver &",argv[0],DEFAULT_PORTNO+1);
		else
			sprintf(cmd,"%s %d stopserver -test with_gotthard &",argv[0],DEFAULT_PORTNO+1);
		printf("\n\nControl Server\nOpening control server on port %d\n",portno );

		//printf("\n\ncmd:%s\n",cmd);
		system(cmd);
		b=1;
		checkType=1;
	
	}





	init_detector(b, checkType);


	sd=bindSocket(portno);
	sockfd=sd;
	if (getServerError(sd)) {
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
		fd=acceptConnection(sockfd);
#ifdef VERY_VERBOSE
		printf("Conenction accepted\n");
#endif
		retval=decode_function(fd);
#ifdef VERY_VERBOSE
		printf("function executed\n");
#endif
		closeConnection(fd);
#ifdef VERY_VERBOSE
		printf("connection closed\n");
#endif
	}

	exitServer(sockfd);
	printf("Goodbye!\n");

	return 0;
}

