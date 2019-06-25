#include "qServer.h"
#include "qDetectorMain.h"

#include "ServerInterface2.h"
#include "string_utils.h"
#include "container_utils.h"

#include <iostream>
#include <string>
#include <future>

qServer::qServer(qDetectorMain *t)
    : mainTab(t), controlPort(DEFAULT_GUI_PORTNO), stopPort(DEFAULT_GUI_PORTNO + 1), 
    controlSocket(nullptr), stopSocket(nullptr) {
    FILE_LOG(logDEBUG) << "Client Server ready";
}

qServer::~qServer() {}

void qServer::FunctionTable() {
    flist[qDefs::QF_GET_DETECTOR_STATUS] = &qServer::GetStatus;
    flist[qDefs::QF_START_ACQUISITION] = &qServer::StartAcquisition;
    flist[qDefs::QF_STOP_ACQUISITION] = &qServer::StopsAcquisition;
    flist[qDefs::QF_START_AND_READ_ALL] = &qServer::Acquire;
    flist[qDefs::QF_EXIT_SERVER] = &qServer::ExitServer;
}

void qServer::DecodeFunction(sls::ServerInterface2 &socket) {
    qDefs::qFuncNames fnum;
    socket.Receive(fnum);

    if (fnum < 0 || fnum >= qDefs::QF_NUM_FUNCTIONS) {
        throw sls::RuntimeError("Unrecognized Function enum " + std::to_string(fnum) + "\n");  
    }

    FILE_LOG(logDEBUG1) << "calling function fnum: " << fnum << " (" 
        << qDefs::getQFunctionNameFromEnum(fnum) << ")";
    (this->*flist[static_cast<int>(fnum)])(socket);   
    FILE_LOG(logDEBUG1) << "Function " << qDefs::getQFunctionNameFromEnum(fnum) << " finished";
}

void qServer::CreateServers() {
    if (!tcpThreadCreated) {
        FILE_LOG(logINFO) << "Starting Gui Servers";
        tcpThreadCreated = true;
        try {
            // start control server
            controlStatus = std::async(std::launch::async, &qServer::ServerThread, this, true);
            FILE_LOG(logDEBUG) << "Gui control server thread created successfully.";
            // start stop server
            stopStatus = std::async(std::launch::async, &qServer::ServerThread, this, false);
            FILE_LOG(logDEBUG) << "Gui stop server thread created successfully.";    
         
        } catch (...) {
            std::string mess = "Could not create Gui TCP servers";
            FILE_LOG(logERROR) << mess;
            qDefs::Message(qDefs::WARNING, mess, "qServer::CreateServers");
            DestroyServers();
        }
    }
}

void qServer::DestroyServers() {
    if (tcpThreadCreated) {
        FILE_LOG(logINFO) << "Shutting down Gui TCP Sockets";
        killTCPServerThread = true;
        if (controlSocket)
            controlSocket->shutDownSocket();
        if (stopSocket)
            stopSocket->shutDownSocket();     
        controlStatus.wait();
        stopStatus.wait();
        tcpThreadCreated = false;
        killTCPServerThread = false;
        FILE_LOG(logDEBUG) << "Server threads stopped successfully.";
    }
}

void qServer::ServerThread(bool isControlServer) {
    if (isControlServer) {
        FILE_LOG(logDEBUG) << "Starting Gui Server (Control port: " << controlPort << ")";
        controlSocket = sls::make_unique<sls::ServerSocket>(controlPort);
    } else {
        FILE_LOG(logDEBUG) << "Starting Gui Server (Stop port: " << stopPort << ")";   
        stopSocket = sls::make_unique<sls::ServerSocket>(stopPort);
    }

    while (true) {
        try{
            auto socket = (isControlServer ? controlSocket->accept() : stopSocket->accept());
            try{
                DecodeFunction(socket);
            } catch(const sls::RuntimeError &e) {

                if (strstr(e.what(), "exit")) {
                    FILE_LOG(logINFO) << "Exiting " << (isControlServer ? "Control" : "Stop") << "Server"; 
                    break;
                }
                char mess[MAX_STR_LENGTH];
                sls::strcpy_safe(mess, e.what());
                socket.Send(qDefs::FAIL);
                socket.Send(mess);
            }
        } catch (const sls::RuntimeError &e) {
            FILE_LOG(logERROR) << "Accept failed";
        }

        // Destroy server 
        if (killTCPServerThread) {
            FILE_LOG(logINFO) << "Exiting " << (isControlServer ? "Control" : "Stop") << "Server"; 
            break;
        }
    }
    FILE_LOG(logDEBUG) << "Stopped gui server thread";
}

void qServer::GetStatus(sls::ServerInterface2 &socket) {
    slsDetectorDefs::runStatus status = slsDetectorDefs::ERROR;
    int progress = 0;
    if (mainTab->isPlotRunning())
        status = slsDetectorDefs::RUNNING;
    else
        status = slsDetectorDefs::IDLE;

    progress = mainTab->GetProgress();

    int retvals[2] = {static_cast<int>(status), progress};
    socket.sendResult(retvals); 
}

void qServer::StartAcquisition(sls::ServerInterface2 &socket) {
    if (mainTab->StartStopAcquisitionFromClient(true) == slsDetectorDefs::FAIL) {
        throw sls::RuntimeError("Could not start acquistion in Gui");
    }
    socket.Send(slsDetectorDefs::OK);
}

void qServer::StopsAcquisition(sls::ServerInterface2 &socket) {
    if (mainTab->StartStopAcquisitionFromClient(false) == slsDetectorDefs::FAIL) {
        throw sls::RuntimeError("Could not stop acquistion in Gui");
    }
    socket.Send(slsDetectorDefs::OK);
}

void qServer::Acquire(sls::ServerInterface2 &socket) {
    if (mainTab->StartStopAcquisitionFromClient(true) == slsDetectorDefs::FAIL) {
        throw sls::RuntimeError("Could not start blocking acquistion in Gui");
    }
    //  blocking
    usleep(5000);
     while (mainTab->isPlotRunning()) {
        usleep(5000);
     }
    socket.Send(slsDetectorDefs::OK);
}

void qServer::ExitServer(sls::ServerInterface2 &socket) {
    throw sls::RuntimeError("Server exited");
}