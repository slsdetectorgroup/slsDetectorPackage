#include "qServer.h"
#include "qDetectorMain.h"

#include "ServerSocket.h"
#include "multiSlsDetector.h"
#include "string_utils.h"

#include <iostream>
#include <string>
#include <future>

qServer::qServer(qDetectorMain *t)
    : threadRunning(false), threadStarted(false), mainTab(t),
      controlPort(DEFAULT_GUI_PORTNO), stopPort(DEFAULT_GUI_PORTNO + 1),
      controlSocket(nullptr), stopSocket(nullptr) {
    FILE_LOG(logDEBUG) << "Client Server ready";
}

qServer::~qServer() {}

void qServer::FunctionTable() {
    flist.push_back(qServer::GetStatus);
    flist.push_back(qServer::StartAcquisition);
    flist.push_back(qServer::StopsAcquisition);
    flist.push_back(qServer::Acquire);
    flist.push_back(qServer::ExitServer);
}

int qServer::DecodeFunction(ServerSocket *sock) {
    int ret = qDefs::FAIL;
    int fnum = 0;
    int n = sock->ReceiveDataOnly(&fnum, sizeof(fnum));
    if (n <= 0) {
        FILE_LOG(logDEBUG3) << "Received " << n << " bytes";
        throw sls::RuntimeError("Could not read socket");
    }

    // unrecognized function
    if (fnum < 0 && fnum >= qDefs::NUM_GUI_FUNCS) {
        ret = qDefs::FAIL;
        char mess[MAX_STR_LENGTH] = {};
        sls::strcpy_safe(mess, "Unrecognized function");
        // will throw an exception
        sock->SendResult(ret, nullptr, 0, mess); 
    }

    // calling function
    FILE_LOG(logDEBUG1) << "calling function fnum: " << fnum;
    ret = (this->*flist[fnum])();

    return ret;
}

void qServer::ShutDownSockets() {
    threadRunning = false;
    if (controlSocket) {
        controlSocket->shutDownSocket();
        delete controlSocket;
        controlSocket = nullptr;
    }
    if (stopSocket) {
        stopSocket->shutDownSocket();
        delete stopSocket;
        stopSocket = nullptr;
    }
}

void qServer::CreateServers() {
    if (!threadRunning) {
        FILE_LOG(logINFO) << "Starting Gui Servers";
        threadRunning = true;
        
        try {
            // start control server
            controlSocket = new ServerSocket(controlPort);
            std::async(std::launch::async, ServerThread, controlSocket);
            FILE_LOG(logDEBUG)
                << "Gui control server thread created successfully.";
            // start stop server
            stopSocket = new ServerSocket(stopPort);
            std::async(std::launch::async, ServerThread, stopSocket);
            FILE_LOG(logDEBUG)
                << "Gui stop server thread created successfully.";    
        } catch (...) {
            ShutDownSockets();
            std::string message = "Can't create gui control server thread";
            FILE_LOG(logERROR) << message;
            qDefs::Message(qDefs::WARNING, message, "qServer::CreateServers");
        }
    }
}

void qServer::DestroyServers() {
    if (threadRunning) {
        FILE_LOG(logINFO) << "Stopping Gui Servers";
        ShutDownSockets();
        FILE_LOG(logDEBUG) << "Server threads stopped successfully.";
    }
}

void qServer::ServerThread(ServerSocket* sock) {
    FILE_LOG(logDEBUG) << "Starting Gui Server at port " << sock->getPort(); 

    while (threadRunning)) {
        try{
            sock->accept();
            if (DecodeFunction(sock) ==  GOODBYE) {
                threadRunning = false;
            }
            sock->close();
        } 
        // any fails will throw an exception, which will be displayed at client side. Ignore here
        catch (...) {}
    }
    FILE_LOG(logDEBUG) << "Stopped gui server thread";

    // stop port is closed last
    if (sock->getPort() == stopPort)
        emit ServerStoppedSignal();
}

int qServer::GetStatus(ServerSock* sock) {
    slsDetectorDefs::runStatus status = slsDetectorDefs::ERROR;
    int progress = 0;
    if (myMainTab->isPlotRunning())
        status = slsDetectorDefs::RUNNING;
    else
        status = slsDetectorDefs::IDLE;

    progress = myMainTab->GetProgress();

    int ret = qDefs::OK
    int retvals[2] = {static_cast<int>(retval), progress};
    sock->SendResult(ret, retvals, sizeof(retvals), nullptr); 
    return ret;
}

int qServer::StartAcquisition(ServerSock* sock) {
    char mess[MAX_STR_LENGTH] = {};
    sls::strcpy_safe(mess, "Could not start acquistion in Gui");
    int ret = myMainTab->StartStopAcquisitionFromClient(true);
    sock->SendResult(ret, nullptr, 0, mess); 
    return ret;
}

int qServer::StopsAcquisition(ServerSock* sock) {
    char mess[MAX_STR_LENGTH] = {};
    sls::strcpy_safe(mess, "Could not stop acquistion in Gui");
    int ret = myMainTab->StartStopAcquisitionFromClient(false);
    sock->SendResult(ret, nullptr, 0, mess); 
    return ret;
}

int qServer::Acquire(ServerSock* sock) {
    char mess[MAX_STR_LENGTH] = {};
    sls::strcpy_safe(mess, "Could not start blocking acquistion in Gui");
    int ret = myMainTab->StartStopAcquisitionFromClient(true);
    //  blocking
    usleep(5000);
    while (myMainTab->isPlotRunning())
        ;
    sock->SendResult(ret, nullptr, 0, mess); 
    return ret;
}

int qServer::ExitServer(ServerSock* sock) {
    DestroyServers();
    int ret = qDefs::OK;
    sock->SendResult(ret, nullptr, 0, mess); 
    return GOODBYE;
}