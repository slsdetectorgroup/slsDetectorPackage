/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"
#include "clogger.h"
#include "communication_funcs.h"
#include "slsDetectorServer_funcs.h"
#include "slsDetectorServer_defs.h"
#include "versionAPI.h"

#include <signal.h>
#include <string.h>
#include <unistd.h>

// Global variables from  communication_funcs
extern int isControlServer;
extern int ret;

// Global variables from slsDetectorServer_funcs
extern int sockfd;
extern int debugflag;
extern int checkModuleFlag;


// Global variables from slsDetectorFunctionList
#ifdef GOTTHARDD
extern int phaseShift;
#endif

void error(char *msg){
	perror(msg);
}

int main(int argc, char *argv[]){
	// print version
	if (argc > 1 && !strcasecmp(argv[1], "-version")) {
        int version = 0;
#ifdef GOTTHARDD
        version = APIGOTTHARD;
#elif EIGERD
		version = APIEIGER;
#elif JUNGFRAUD
		version = APIJUNGFRAU;
#elif CHIPTESTBOARDD
		version = APICTB;
#elif MOENCHD
		version = APIMOENCH;
#endif
		LOG(logINFO, ("SLS Detector Server %s (0x%x)\n", GITBRANCH, version));
	}

    int portno = DEFAULT_PORTNO;
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
				LOG(logINFO, ("Detected stop server\n"));
				isControlServer = 0;
			}
            else if(!strcasecmp(argv[i],"-devel")){
                LOG(logINFO, ("Detected developer mode\n"));
                debugflag = 1;
            }
            else if(!strcasecmp(argv[i],"-nomodule")){
                LOG(logINFO, ("Detected No Module mode\n"));
                checkModuleFlag = 0;
            }	
			else if(!strcasecmp(argv[i],"-port")){
			    if ((i + 1) >= argc) {
			        LOG(logERROR, ("no port value given. Exiting.\n"));
			        return -1;
			    }
			    if (sscanf(argv[i + 1], "%d", &portno) == 0) {
			        LOG(logERROR, ("cannot decode port value %s. Exiting.\n", argv[i + 1]));
			        return -1;
			    }
				LOG(logINFO, ("Detected port: %d\n", portno));
            }
#ifdef GOTTHARDD
			else if(!strcasecmp(argv[i],"-phaseshift")){
			    if ((i + 1) >= argc) {
			        LOG(logERROR, ("no phase shift value given. Exiting.\n"));
			        return -1;
			    }
			    if (sscanf(argv[i + 1], "%d", &phaseShift) == 0) {
			        LOG(logERROR, ("cannot decode phase shift value %s. Exiting.\n", argv[i + 1]));
			        return -1;
			    }
				LOG(logINFO, ("Detected phase shift of %d\n", phaseShift));
			}
#endif
		}
	}

#ifdef STOP_SERVER
	char cmd[MAX_STR_LENGTH];
	memset(cmd, 0, MAX_STR_LENGTH);
#endif
	if (isControlServer) {
		LOG(logINFO, ("Opening control server on port %d \n", portno));
#ifdef STOP_SERVER
		{
			int i;
			for (i = 0; i < argc; ++i) {
				if (i > 0) {
					strcat(cmd, " ");
				}
				strcat(cmd, argv[i]);
			}
			char temp[50];
			memset(temp, 0, sizeof(temp));
			sprintf(temp, " -stopserver -port %d &", portno + 1);
			strcat(cmd, temp);
			
			LOG(logDEBUG1, ("Command to start stop server:%s\n", cmd));
			system(cmd);
		}
#endif
	} else {
		LOG(logINFO,("Opening stop server on port %d \n", portno));
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
		LOG(logINFOBLUE, ("Control Server Ready...\n\n"));
	} else {
		LOG(logINFO, ("Stop Server Ready...\n\n"));
	}

	// waits for connection
	while(retval != GOODBYE && retval != REBOOT) {
		fd = acceptConnection(sockfd);
		if (fd > 0) {
			retval = decode_function(fd);
			closeConnection(fd);
		}
	}

	exitServer(sockfd);

	if (retval == REBOOT) {
		LOG(logINFORED,("Rebooting!\n"));
		fflush(stdout);
#if defined(MYTHEN3D) || defined(GOTTHARD2D)
		rebootNiosControllerAndFPGA();
#else
		system("reboot");
#endif
	}
	LOG(logINFO,("Goodbye!\n"));
	return 0;
}
