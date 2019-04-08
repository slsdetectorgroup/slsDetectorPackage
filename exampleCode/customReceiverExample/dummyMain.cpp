/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "dummyUDPInterface.h"
#include "slsReceiverTCPIPInterface.h"
#include "ansi.h"

#include <iostream>
#include <string.h>
#include <signal.h>	//SIGINT
#include <cstdlib>	//system
#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <syscall.h>
using namespace std;

bool keeprunning;

void sigInterruptHandler(int p){
	keeprunning = false;
}

int main(int argc, char *argv[]) {

	keeprunning = true;
	bprintf(BLUE,"Created [ Tid: %ld ]\n", (long)syscall(SYS_gettid));

	// Catch signal SIGINT to close files and call destructors properly
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SIGINT\n");
	}


	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, NULL) == -1) {
		bprintf(RED, "Could not set handler function for SIGCHILD\n");
	}


	int ret = slsReceiverDefs::OK;
	int tcpip_port_no = 1954;
	dummyUDPInterface *udp_interface = new dummyUDPInterface();
	slsReceiverTCPIPInterface *tcpipInterface = new slsReceiverTCPIPInterface(ret, udp_interface, tcpip_port_no);
	if(ret==slsReceiverDefs::FAIL){
		delete tcpipInterface;
		bprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
		exit(EXIT_FAILURE);
	}


	//start tcp server thread
	if (tcpipInterface->start() == slsReceiverDefs::FAIL){
		delete tcpipInterface;
		bprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
		exit(EXIT_FAILURE);
	}

	FILE_LOG(logINFO) << "Ready ... ";
	bprintf(GRAY, "\n[ Press \'Ctrl+c\' to exit ]\n");
	while(keeprunning)
		usleep(5 * 1000 * 1000);

	delete tcpipInterface;
	bprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
	FILE_LOG(logINFO) << "Goodbye!";
	return 0;
}


