#pragma once

#include <stdlib.h>
#include <string>

class qClient {

  public:
    qClient(char *h);
    virtual ~qClient();
    void executeLine(int narg, char *args[]);

  private:
    std::string printCommands();
    std::string getStatus();
    void startAcquisition(bool blocking = false);
    void stopAcquisition();
    void exitServer();

    std::string hostname;
    int controlPort;
    int stopPort;
};
