/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include "communication_funcs.h"
#include "slsDetectorServer_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

extern int sockfd;


void error(char *msg){
	perror(msg);
}

int main(int argc, char *argv[]){
	int  portno = DEFAULT_PORTNO;
	int retval = OK;
	int sd, fd;
	int debugflag = 0;
	int controlserver = 1;

	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	signal(SIGPIPE, SIG_IGN);

    // circumvent the basic tests
	{
		int i;
		for (i = 1; i < argc; ++i) {
			if(!strcasecmp(argv[i],"-stopserver")) {
				cprintf(BLUE,"Detected stop server\n");
				controlserver = 0;
			}
			else if(!strcasecmp(argv[i],"-devel")){
				cprintf(BLUE,"Detected developer mode\n");
				debugflag = 1;
			}
#ifdef JUNGFRAUD
			else if(!strcasecmp(argv[i],"-update")){
				cprintf(BLUE,"Detected update mode\n");
				debugflag = PROGRAMMING_MODE;
			}
#endif
			else if(strchr(argv[i],'-') != NULL) {
				cprintf(RED,"cannot scan program argument %s\n", argv[1]);
				return -1;
			}
		}
	}

#ifdef STOP_SERVER
	char cmd[100];
	memset(cmd, 0, 100);
#endif
	if (controlserver) {
		portno = DEFAULT_PORTNO;
		cprintf(BLUE,
		"********************************************************\n"
		"********* opening control server on port %d **********\n"
		"********************************************************\n\n"
		, portno);
#ifdef STOP_SERVER
		{
			int i;
			for (i = 0; i < argc; ++i)
				sprintf(cmd, "%s %s", cmd, argv[i]);
			sprintf(cmd,"%s -stopserver&", cmd);
			cprintf(BLUE,"cmd:%s\n", cmd);
			system(cmd);
		}
#endif
	} else {
		portno = DEFAULT_PORTNO+1;
		cprintf(BLUE,
		"********************************************************\n"
		"*********** opening stop server on port %d ***********\n"
		"********************************************************\n\n"
		, portno);
	}

	setModeFlag(debugflag); //defined in slsDetectorServer_funcs
	init_detector(controlserver); //defined in slsDetectorServer_funcs

	sd=bindSocket(portno); //defined in communication_funcs
	sockfd=sd;
	if (getServerError(sd)) {  //defined in communication_funcs
		printf("server error!\n");
		return -1;
	}

	/* assign function table */
	function_table();  //defined in slsDetectorServer_funcs
#ifdef VERBOSE
	printf("function table assigned \n");
#endif

	if (controlserver)
	    printf("\nControl Server Ready...\n\n");
	else
	    printf("\nStop Server Ready...\n\n");

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
			retval=decode_function(fd);   //defined in slsDetectorServer_funcs
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

