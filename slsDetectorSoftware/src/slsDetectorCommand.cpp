#include "slsDetectorCommand.h"
#include "multiSlsDetector.h"
#include "string_utils.h"

#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <iomanip>


/*! \page CLI Command line interface


This program is intended to control the SLS detectors via command line interface.
This is the only way to access all possible functionality of the detectors, however it is often recommendable to avoid changing the most advanced settings, rather leaving the task to configuration files, as when using the GUI or the API provided.

The command line interface consists in four main functions:

- \b sls_detector_acquire to acquire data from the detector
- \b sls_detector_put to set detector parameters
- \b sls_detector_get to retrieve detector parameters
- \b sls_detector_help to get help concerning the text commands
Additionally the program slsReceiver should be started on the machine expected to receive the data from the detector.


If you need control a single detector, the use of the command line interface does not need any additional arguments.

For commands addressing a single controller of your detector, the command  cmd should be called with the index i of the controller:


<b>sls_detector_clnt i:cmd</b>


where \b sls_detector_clnt is the text client (put, get, acquire, help).

In case more than one detector is configured on the control PC, the command  cmd should be called with their respective index  j:


<b>sls_detector_clnt j-cmd</b>


where \b sls_detector_clnt is the text client (put, get, acquire, help).

To address a specific controller i of detector j use:

<b>sls_detector_clnt j-i:cmd</b>


To use different shared memory segements for different detectors on the same
client pc, one can use environment variable <b>SLSDETNAME</b> set to any string to
different strings to make the shared memory segments unique. One can then use
the same multi detector id for both detectors as they have a different shared memory names.

For additional questions concerning the indexing of the detector, please refer to the SLS Detectors FAQ documentation.

The commands are sudivided into different pages depending on their functionalities:
 - \subpage acquisition "Acquisition": commands to start/stop the acquisition and retrieve data
 - \subpage config "Configuration": commands to configure the detector
 - \subpage timing "Timing": commands to configure the detector timing
 - \subpage data "Data postprocessing": commands to process the data
 - \subpage settings "Settings": commands to define detector settings/threshold.
 - \subpage output "Output": commands to define output file destination and format
 - \subpage network "Network": commands to setup the network between client, detector and receiver
 - \subpage receiver "Receiver": commands to configure the receiver
 - \subpage prototype "Chip Test Board / Moench": commands specific for the chiptest board or moench
 - \subpage test "Developer": commands to be used only for software debugging. Avoid using them!
 
 */

slsDetectorCommand::slsDetectorCommand(multiSlsDetector *det) {

    myDet = det;

    int i = 0;

    cmd = std::string("none");

    /*! \page test Developer
    Commands to be used only for software debugging. Avoid using them!
    - \b test returns an error
	 */

    descrToFuncMap[i].m_pFuncName = "test";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdUnderDevelopment;
    ++i;

    /*! \page test
   - <b>help</b> Returns a list of possible commands.
	 */
    descrToFuncMap[i].m_pFuncName = "help";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHelp;
    ++i;

    /*! \page test
   - <b>exitserver</b> Shuts down all the detector servers. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "exitserver";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>rx_exit</b> Shuts down all the receivers. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "rx_exit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>rx_execcommand</b> Executes a command on the receiver server. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "rx_execcommand";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /* digital test and debugging */



    /* Acquisition and status commands */
    /*! \page acquisition Acquition commands
   Commands to control the acquisition
	 */
    /*! \page acquisition
   - \b acquire blocking acquisition (like calling sls_detector_acquire). Starts receiver and detector, writes and processes the data, stops detector. Only get!
     \c Returns (string)\c "acquire failed" if fails, else \c "Acquired (int)", where int is number of frames caught.
	 */
    descrToFuncMap[i].m_pFuncName = "acquire";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAcquire;
    ++i;

    /*! \page acquisition
   - \b data gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup (Eigerr store in ram only). Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "data";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdData;
    ++i;


    /*! \page config
   - \b free Free shared memory on the control PC
	 */
    descrToFuncMap[i].m_pFuncName = "free";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFree;
    ++i;


    /*! \page config
   - <b>checkdetversion</b> Checks the version compatibility with detector server (if hostname is in shared memory). Only get! Only for Eiger, Jungfrau & Gotthard. \c Returns \c ("compatible", "incompatible")
	 */
    descrToFuncMap[i].m_pFuncName = "checkdetversion";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>rx_checkversion</b> Checks the version compatibility with receiver server (if rx_hostname is in shared memory). Only get! Only for Eiger, Jungfrau & Gotthard. \c Returns \c ("compatible", "incompatible")
	 */
    descrToFuncMap[i].m_pFuncName = "rx_checkversion";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

  
    /* settings dump/retrieve */

    descrToFuncMap[i].m_pFuncName = "config";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;


    /*! \page receiver
   - <b>resetframescaught [i]</b> resets the number of frames caught to 0. i can be any number. Use this if using status start, instead of acquire (this command is included). Only put! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "resetframescaught";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;


    /* pattern generator */


    numberOfCommands = i;

    // #ifdef VERBOSE
    //   std::cout << "Number of commands is " << numberOfCommands << std::endl;
    // #endif
}

//-----------------------------------------------------------

/*!
 */

//-----------------------------------------------------------

std::string slsDetectorCommand::executeLine(int narg, const char * const args[], int action, int detPos) {
    if (action == READOUT_ACTION)
        return cmdAcquire(narg, args, action, detPos);

    size_t s = std::string(args[0]).find(':');
    std::string key = std::string(args[0]).substr(0, s); // truncate at :

    if (action == PUT_ACTION && narg < 1)
        action = HELP_ACTION;

    for (int i = 0; i < numberOfCommands; ++i) {

        /* this works only if the command completely matches the key */
        /* otherwise one could try if truncated key is unique */

        if (key == descrToFuncMap[i].m_pFuncName) {
#ifdef VERBOSE
            std::cout << i << " command=" << descrToFuncMap[i].m_pFuncName << " key=" << key << std::endl;
#endif
            cmd = descrToFuncMap[i].m_pFuncName;

            MemFuncGetter memFunc = descrToFuncMap[i].m_pFuncPtr;
            std::string dResult = (this->*memFunc)(narg, args, action, detPos);

            return dResult;
        }
    }
    return cmdUnknown(narg, args, action, detPos);
}

std::string slsDetectorCommand::cmdUnknown(int narg, const char * const args[], int action, int detPos) {
    return std::string("Unknown command, use list to list all commands ");
}
std::string slsDetectorCommand::cmdUnderDevelopment(int narg, const char * const args[], int action, int detPos) {
    return std::string("Must still develop ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
}

std::vector<std::string> slsDetectorCommand::getAllCommands(){
    std::vector<std::string> commands;
    for (int i = 0; i!= numberOfCommands; ++i)
        commands.emplace_back(descrToFuncMap[i].m_pFuncName);
    return commands;
}

std::string slsDetectorCommand::helpLine(int narg, const char * const args[], int action, int detPos) {

    std::ostringstream os;

    if (action == READOUT_ACTION) {
        return helpAcquire(HELP_ACTION);
    }

    if (narg == 0) {
        os << "Command can be: " << std::endl;
        for (int i = 0; i < numberOfCommands; ++i) {
            os << descrToFuncMap[i].m_pFuncName << "\n";
        }
        os << std::endl;
        return os.str();
    }
    return executeLine(narg, args, HELP_ACTION, detPos);
}

std::string slsDetectorCommand::cmdAcquire(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpAcquire(HELP_ACTION);
    }
    if (!myDet->size()) {
        FILE_LOG(logERROR) << "This shared memory has no detectors added. Aborting.";
        return std::string("acquire failed");
    }
    if (detPos >= 0) {
        FILE_LOG(logERROR) << "Individual detectors not allowed for readout. Aborting.";
        return std::string("acquire failed");
    }

    if (myDet->acquire() == FAIL)
        return std::string("acquire failed");
    if (myDet->getUseReceiverFlag(detPos)) {
        char answer[100];
        sprintf(answer, "\nAcquired %d", myDet->getFramesCaughtByReceiver(detPos));
        return std::string(answer);
    }

    return std::string();
}

std::string slsDetectorCommand::helpAcquire(int action) {

    if (action == PUT_ACTION)
        return std::string("");
    std::ostringstream os;
    os << "Usage is " << std::endl
       << "sls_detector_acquire  id " << std::endl;
    os << "where id is the id of the detector " << std::endl;
    os << "the detector will be started, the data acquired, processed and written to file according to the preferences configured " << std::endl;
    return os.str();
}

std::string slsDetectorCommand::cmdData(int narg, const char * const args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    //int b;
    if (action == PUT_ACTION) {
        return std::string("cannot set");
    } else if (action == HELP_ACTION) {
        return helpData(HELP_ACTION);
    } else {
        // b=myDet->setThreadedProcessing(-1);
        // myDet->setThreadedProcessing(0);
        // myDet->readAll(detPos);
        // //processdata in receiver is useful only for gui purposes
        // if(myDet->getUseReceiverFlag(detPos)==OFFLINE_FLAG)
        // 	myDet->processData();
        // myDet->setThreadedProcessing(b);
        return std::string("");
    }
}

std::string slsDetectorCommand::helpData(int action) {

    if (action == PUT_ACTION)
        return std::string("");
    else
        return std::string("data \t gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup\n");
}


std::string slsDetectorCommand::cmdFree(int narg, const char * const args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpFree(HELP_ACTION);
    }

    return ("Error: Should have been freed before creating constructor\n");
}

std::string slsDetectorCommand::helpFree(int action) {
    return std::string("free \t frees the shared memory\n");
}


std::string slsDetectorCommand::cmdHelp(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    std::cout << narg << std::endl;

    if (narg >= 1)
        return helpLine(narg - 1, args, action, detPos);
    else
        return helpLine(0, args, action, detPos);
}

std::string slsDetectorCommand::cmdExitServer(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpExitServer(action);
    }

    if (action == PUT_ACTION) {
        if (cmd == "exitserver") {
            myDet->exitServer(detPos);
            return std::string("Server shut down.");
        } else if (cmd == "rx_exit") {

            myDet->exitReceiver(detPos);
            return std::string("Receiver shut down\n");
        } else if (cmd == "rx_execcommand") {

            myDet->execReceiverCommand(std::string(args[1]), detPos);
            return std::string("Command executed successfully\n");
         } else
            return ("cannot decode command\n");
    } else
        return ("cannot get");
}

std::string slsDetectorCommand::helpExitServer(int action) {
    std::ostringstream os;
    os << std::string("exitserver \t shuts down all the detector servers. Don't use it!!!!\n");
    os << std::string("rx_exit \t shuts down all the receiver servers.\n");
    os << std::string("rx_execcommand \t executes command in receiver server. Don't use it if you do not know what you are doing.\n");
    return os.str();
}




// std::string slsDetectorCommand::cmdThreaded(int narg, const char * const args[], int action, int detPos){
// 	int ival;
// 	char answer[1000];

// 	if (action==HELP_ACTION)
// 		return helpThreaded(action);

// 	if (action==PUT_ACTION) {
// 		if (sscanf(args[1],"%d",&ival))
// 			myDet->setThreadedProcessing(ival);
// 	}
// 	sprintf(answer,"%d",myDet->setThreadedProcessing());
// 	return std::string(answer);

// }

std::string slsDetectorCommand::helpThreaded(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("threaded \t  returns wether the data processing is threaded. \n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("threaded t \t  sets the threading flag ( 1sets, 0 unsets).\n");

    return os.str();
}





std::string slsDetectorCommand::cmdSN(int narg, const char * const args[], int action, int detPos) {

    if (action == PUT_ACTION)
        return std::string("cannot set");

    if (action == HELP_ACTION)
        return helpSN(action);


    if (cmd == "checkdetversion") {
        myDet->checkDetectorVersionCompatibility(detPos);
        return std::string("compatible");
    }

    if (cmd == "rx_checkversion") {
        myDet->checkReceiverVersionCompatibility(detPos);
        return std::string("compatible");
    }

    return std::string("unknown id mode ") + cmd;
}

std::string slsDetectorCommand::helpSN(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "checkdetversion \n gets the version compatibility with detector server (if hostname is in shared memory). Only for Eiger, Jungfrau & Gotthard. Prints compatible/ incompatible." << std::endl;
        os << "rx_checkversion \n gets the version compatibility with receiver server (if rx_hostname is in shared memory). Only for Eiger, Jungfrau & Gotthard. Prints compatible/ incompatible." << std::endl;
    }
    return os.str();
}






std::string slsDetectorCommand::cmdConfiguration(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpConfiguration(action);

    if (cmd == "config") {
        if (action == PUT_ACTION) {
            myDet->readConfigurationFile(std::string(args[1]));
        } 
        return std::string("success");
    } 
    return std::string("could not decode conf mode");
}

std::string slsDetectorCommand::helpConfiguration(int action) {

    std::ostringstream os;
    return os.str();
}

std::string slsDetectorCommand::cmdReceiver(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpReceiver(action);

    if (cmd == "resetframescaught") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        else {
            myDet->resetFramesCaught(detPos);
            return std::string("successful");
        }
    }



    return std::string("could not decode command");
}

std::string slsDetectorCommand::helpReceiver(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "resetframescaught [any value] \t resets frames caught by receiver" << std::endl;

    }
    return os.str();
}




