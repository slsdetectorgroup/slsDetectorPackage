/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_detector_defs.h"
#include "slsReceiverUsers.h"
#include "logger.h"

#include <iostream>
#include <string.h>
#include <signal.h>	//SIGINT
#include <cstdlib>		//system
#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <unistd.h> 	//usleep
#include <syscall.h>
#include <map>
#include <getopt.h>

bool keeprunning;
int ctbOffset = 0;

void sigInterruptHandler(int p){
	keeprunning = false;
}


#ifdef MYTHEN302
void GetData(char* metadata, char* datapointer, uint32_t& datasize, void* p) {
	// remove the offset in datasize
    int offset = ctbOffset * sizeof(uint64_t);
    datasize -= offset;

    constexpr int dynamicRange = 24;
    constexpr int numSamples = 32 * 3;          // 32 channels * 3 counters = 96
    constexpr int numCounters = numSamples * 2; // 2 strips
    // validate datasize
    {
        FILE_LOG(logDEBUG) << "Datasize after removing offset:" << datasize;
        const double dataNumSamples =
            ((double)datasize / (double)sizeof(uint64_t)) / (double)dynamicRange; // 2304 / 24 = 96
        if (dataNumSamples - numSamples) {
            FILE_LOG(logERROR) << "Number of samples do not match, Expected "
                               << numSamples << ", got " << dataNumSamples;
        }
    }
	// source
    uint64_t* source = (uint64_t*)datapointer;
	// remove the offset from source
    source += offset;
    // destination
    auto result = new int[numCounters];
    auto destStrip0 = result;
    auto destStrip1 = result + numSamples;
	constexpr int bit_index0 = 6;
    constexpr int bit_index1 = 17;
    constexpr int mask0 = (1 << bit_index0);
    constexpr int mask1 = (1 << bit_index1);

    for (int j = 0; j < numSamples; ++j) {
        for (int i = 0; i < dynamicRange; ++i) {
            int bit0 = (*source & mask0) >> bit_index0;
            int bit1 = (*source++ & mask1) >> bit_index1;
            *destStrip0 |= bit0 << i;
            *destStrip1 |= bit1 << i;
        }
        destStrip0++;
        destStrip1++;
    }

	// update the size to be written to file & overwrite data in memory
    datasize = numCounters * sizeof(int);
    memcpy(datapointer, (char*)result, datasize);
    FILE_LOG(logDEBUG) << "Modified Size: " << datasize;
}
#endif


int main(int argc, char *argv[]) {

	// options
	std::map<std::string, std::string> configuration_map;
	//parse command line for config
	static struct option long_options[] = {
		{"ctb_offset",	required_argument,	nullptr,	'o'},
		{nullptr, 		0, 					nullptr, 	0}
	};
	//initialize global optind variable (required when instantiating multiple receivers in the same process)
	optind = 1;
	// getopt_long stores the option index here.
	int option_index = 0;
	int c = 0;
	while ( c != -1 ) {
		c = getopt_long (argc, argv, "hvf:t:o:", long_options, &option_index);
		// Detect the end of the options.
		if (c == -1)
			break;
		switch(c) {
			case 'o':
				sscanf(optarg, "%d", &ctbOffset);
                break;
            default:
                break;
        }
    }

#ifdef MYTHEN302
    FILE_LOG(logINFOGREEN) << "Mythen 302 Receiver";
    FILE_LOG(logINFO) << "CTB Offset: " << ctbOffset;
#endif

    keeprunning = true;
    FILE_LOG(logINFOBLUE) << "Created [ Tid: " << syscall(SYS_gettid) << " ]";

    // Catch signal SIGINT to close files and call destructors properly
    struct sigaction sa;
    sa.sa_flags = 0;                     // no flags
    sa.sa_handler = sigInterruptHandler; // handler function
    sigemptyset(&sa.sa_mask); // dont block additional signals during invocation
                              // of handler
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        FILE_LOG(logERROR) << "Could not set handler function for SIGINT";
	}


	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, nullptr) == -1) {
		FILE_LOG(logERROR) << "Could not set handler function for SIGPIPE";
	}

	int ret = slsDetectorDefs::OK;
	slsReceiverUsers *receiver = new slsReceiverUsers(argc, argv, ret);
	if(ret==slsDetectorDefs::FAIL){
		delete receiver;
		FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
		exit(EXIT_FAILURE);
	}


	//register callbacks
	receiver->registerCallBackRawDataModifyReady(GetData, NULL);

        //start tcp server thread
	if (receiver->start() == slsDetectorDefs::FAIL){
		delete receiver;
		FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
		exit(EXIT_FAILURE);
	}

	FILE_LOG(logINFO) << "Ready ... ";
	FILE_LOG(logINFO) << "[ Press \'Ctrl+c\' to exit ]";
	while(keeprunning)
		pause();

	delete receiver;
	FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
	FILE_LOG(logINFO) << "Exiting Receiver";
	return 0;
}

