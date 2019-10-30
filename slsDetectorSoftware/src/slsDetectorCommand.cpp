#include "slsDetectorCommand.h"
#include "multiSlsDetector.h"
#include "slsDetector.h"
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


  
    /* settings dump/retrieve */

    descrToFuncMap[i].m_pFuncName = "config";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;



    numberOfCommands = i;


}


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


std::vector<std::string> slsDetectorCommand::getAllCommands(){
    std::vector<std::string> commands;
    for (int i = 0; i!= numberOfCommands; ++i)
        commands.emplace_back(descrToFuncMap[i].m_pFuncName);
    return commands;
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
    if (myDet->Parallel(&slsDetector::getUseReceiverFlag, {}).squash(false)) {
        std::ostringstream os;
        os << "\nAcquired ";
        os << sls::ToString(myDet->Parallel(&slsDetector::getFramesCaughtByReceiver, {}));
        return os.str();
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



