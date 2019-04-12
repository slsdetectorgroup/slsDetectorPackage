#include "qClient.h"

#include "ClientSocket.h"
#include "logger.h"
#include "sls_detector_exceptions.h"

#include <iostream>
#include <sstream>

int main(int argc, char *argv[]) {
    qClient *cl = 0;
    cl = new qClient(argv[1]);
    cl->executeLine(argc - 2, argv + 2);
    delete cl;
    return 0;
}

qClient::qClient(char *h)
    : controlPort(DEFAULT_GUI_PORTNO), stopPort(DEFAULT_GUI_PORTNO + 1) {
    hostname.assign(h);
}

qClient::~qClient() {}

void qClient::executeLine(int narg, char *args[]) {

    std::string retval = "";
    std::string cmd = args[0];

    // validate command structure
    if (narg < 1) {
        throw sls::RuntimeError("No command parsed. " + printCommands());
    }

    // help
    if (cmd == "help") {
        retval = printCommands();
    }

    // file name
    else if (cmd == "status") {

        if (narg > 1) {
            std::string argument = args[1];
            // start acquisition
            if (argument == "start")
                startAcquisition();
            else if (argument == "stop")
                stopAcquisition();
            else {
                throw sls::RuntimeError("Could not parse arguments. " +
                                        printCommands());
            }
        }
        retval = getStatus();
    }

    else if (cmd == "acquire") {
        startAcquisition(true);
        retval = getStatus();
    }

    else if (cmd == "exit") {
        exitServer();
        retval.assign("Server exited successfully");
    }

    // unrecognized command
    else {
        throw sls::RuntimeError("Unrecognized command. " + printCommands());
    }

    // print result
    FILE_LOG(logINFO) << cmd << ": " << retval;
}

std::string qClient::printCommands() {
    std::ostringstream os;
    os << "\nexit \t exits servers in gui" << std::endl;
    os << "status \t gets status of acquisition in gui. - can be running or "
          "idle"
       << std::endl;
    os << "status i  starts/stops acquistion in gui-non blocking. i is start "
          "or stop"
       << std::endl;
    os << "acquire  starts acquistion in gui-blocking" << std::endl;
    return os.str();
}

std::string qClient::getStatus() {
    int fnum = qDefs::F_GUI_GET_RUN_STATUS;
    int retvals[2] = {static_cast<int>(ERROR), 0};

    auto client = sls::GuiSocket(hostname, controlPort);
    client.sendCommandThenRead(fnum, nullptr, 0, retvals, sizeof(retvals));

    runStatus status = static_cast<runStatus>(retvals[0]);
    int progress = retvals[1];
    return std::to_string(progress) + std::string("% ") +
           slsDetectorDefs::runStatusType(status);
}

void qClient::startAcquisition(bool blocking) {
    int fnum = qDefs::F_GUI_START_ACQUISITION;
    if (blocking)
        fnum = qDefs::F_GUI_START_AND_READ_ALL;

    auto client = sls::GuiSocket(hostname.c_str(), controlPort);
    client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
}

void qClient::stopAcquisition() {
    int fnum = qDefs::F_GUI_STOP_ACQUISITION;

    auto client = sls::GuiSocket(hostname, stopPort);
    client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
}

void qClient::exitServer() {
    int fnum = qDefs::F_GUI_EXIT_SERVER;
    // closes both control and stop server
    auto client = sls::GuiSocket(hostname, controlPort);
    client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
 }
