#pragma once

#include "qDefs.h"
#include "ServerSocket.h"
class qDetectorMain;
class ServerInterface2;

#include <vector>
#include <future>

class qServer : public QWidget {
    Q_OBJECT

  public:
    qServer(qDetectorMain *t);
    ~qServer();
    void CreateServers();
    void DestroyServers();

  private:
    void FunctionTable();
    void DecodeFunction(sls::ServerInterface2 *socket);
    void ServerThread(bool isControlServer);
    void GetStatus(sls::ServerInterface2* socket);
    void StartAcquisition(sls::ServerInterface2* socket);
    void StopsAcquisition(sls::ServerInterface2* socket);
    void Acquire(sls::ServerInterface2* socket);
    void ExitServer(sls::ServerInterface2* socket);

    void (qServer::*flist[qDefs::QF_NUM_FUNCTIONS])(sls::ServerInterface2 &socket);
    qDetectorMain *mainTab;
    bool tcpThreadCreated{false};
    bool killTCPServerThread{false};
    std::future<void> controlStatus;
    std::future<void> stopStatus;
    int controlPort;
    int stopPort;
    std::unique_ptr<sls::ServerSocket> controlSocket{nullptr};
    std::unique_ptr<sls::ServerSocket> stopSocket{nullptr};

  signals:
    // to update the Listening to Gui check box
    void ServerStoppedSignal();
};
