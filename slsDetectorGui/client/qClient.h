#pragma once

#include "qDefs.h"

#include "sls_detector_defs.h"

#include <stdlib.h>
#include <string>

/**
 *@short Sets up the gui server
 */
class qClient : public virtual slsDetectorDefs {

  public:
    /**
     * The constructor
     * @param h hostname
     */
    qClient(char *h);
    /**
     * Destructor
     */
    virtual ~qClient();

    /**
     * Execute command
     * @param narg number of arguments
     * @param args argument list
     */
    void executeLine(int narg, char *args[]);

  private:
    /**
     * Print list of commands
     * @returns string of result
     */
    std::string printCommands();

    /**
     * Gets run status
     * @returns status
     */
    std::string getStatus();

    /**
     * Start Acquisition
     * @param blocking true if its a blocking acquistion
     */
    void startAcquisition(bool blocking = false);

    /**
     * Stops Acquisition
     */
    void stopAcquisition();

    /**
     * Exits Server
     */
    void exitServer();

    /** hostname */
    std::string hostname;

    /** control port */
    int controlPort;

    /** stop port */
    int stopPort;
};
