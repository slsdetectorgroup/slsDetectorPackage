/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"
#include "logger.h"
#include "communication_funcs.h"
#include "slsDetectorServer_funcs.h"
#include "slsDetectorServer_defs.h"

#include <signal.h>
#include <string.h>


// Global variables from  communication_funcs
extern int isControlServer;
extern int ret;

// Global variables from slsDetectorServer_funcs
extern int sockfd;
extern int debugflag;

// Global variables from slsDetectorFunctionList
#ifdef GOTTHARDD
extern int phaseShift;
#endif

void error(char *msg){
	perror(msg);
}

int main(int argc, char *argv[]){
	int  portno = DEFAULT_PORTNO;
	int retval = OK;
	int fd = 0;

	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	signal(SIGPIPE, SIG_IGN);


    // circumvent the basic tests
	{
		int i;
		for (i = 1; i < argc; ++i) {
			if(!strcasecmp(argv[i],"-stopserver")) {
				FILE_LOG(logINFO, ("Detected stop server\n"));
				isControlServer = 0;
			}
            else if(!strcasecmp(argv[i],"-devel")){
                FILE_LOG(logINFO, ("Detected developer mode\n"));
                debugflag = 1;
            }
#ifdef GOTTHARDD
			else if(!strcasecmp(argv[i],"-phaseshift")){
			    if ((i + 1) >= argc) {
			        FILE_LOG(logERROR, ("no phase shift value given. Exiting.\n"));
			        return -1;
			    }
			    if (sscanf(argv[i + 1], "%d", &phaseShift) == 0) {
			        FILE_LOG(logERROR, ("cannot decode phase shift value %s. Exiting.\n", argv[i + 1]));
			        return -1;
			    }
				FILE_LOG(logINFO, ("Detected phase shift of %d\n", phaseShift));
			}
#endif
#ifdef JUNGFRAUD
			else if(!strcasecmp(argv[i],"-update")){
				FILE_LOG(logINFO, ("Detected update mode\n"));
				debugflag = PROGRAMMING_MODE;
			}
#endif
			else if(strchr(argv[i],'-') != NULL) {
				FILE_LOG(logERROR, ("cannot scan program argument %s\n", argv[1]));
				return -1;
			}
		}
	}

#ifdef STOP_SERVER
	char cmd[100];
	memset(cmd, 0, 100);
#endif
	if (isControlServer) {
		portno = DEFAULT_PORTNO;
		FILE_LOG(logINFO, ("Opening control server on port %d \n", portno));
#ifdef STOP_SERVER
		{
			int i;
			for (i = 0; i < argc; ++i)
				sprintf(cmd, "%s %s", cmd, argv[i]);
			sprintf(cmd,"%s -stopserver&", cmd);
			FILE_LOG(logDEBUG1, ("Command to start stop server:%s\n", cmd));
			system(cmd);
		}
#endif
	} else {
		portno = DEFAULT_PORTNO + 1;
		FILE_LOG(logINFO,("Opening stop server on port %d \n", portno));
	}

	init_detector();

	{	// bind socket
		sockfd = bindSocket(portno);
		if (ret == FAIL)
			return -1;
	}

	// assign function table
	function_table();

	if (isControlServer) {
		FILE_LOG(logINFOBLUE, ("Control Server Ready...\n\n"));
	} else {
		FILE_LOG(logINFO, ("Stop Server Ready...\n\n"));
	}

	// waits for connection
	while(retval != GOODBYE) {
		fd = acceptConnection(sockfd);
		if (fd > 0) {
			retval = decode_function(fd);
			closeConnection(fd);
		}
	}

	exitServer(sockfd);
	FILE_LOG(logINFO,("Goodbye!\n"));

	return 0;
}

