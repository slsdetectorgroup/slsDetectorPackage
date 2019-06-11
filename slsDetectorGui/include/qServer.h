#pragma once

class qDetectorMain;

class multiSlsDetector;
class ServerSocket;
class ServerInterface;

#include <vector>

class qServer : public QWidget, public virtual slsDetectorDefs {
    Q_OBJECT

  public:
    qServer(qDetectorMain *t);
    ~qServer();
    void CreateServers();
    void DestroyServers();

  private:
    void FunctionTable();
    void DecodeFunction(ServerSocket *sock);
    void ShutDownSockets();
    void ServerThread(ServerSocket* sock);
    void GetStatus(ServerSocket* sock);
    void StartAcquisition(ServerSocket* sock);
    void StopsAcquisition(ServerSocket* sock);
    void Acquire(ServerSocket* sock);
    void ExitServer(ServerSocket* sock);

    /** function list */
    typedef int (qServer::*some_func_t)(ServerSocket*);
    typedef std::vector<some_func_t> sflist;
     bool guiServerRunning;
    bool threadStarted;

    qDetectorMain *mainTab;
    int controlPort;
    int stopPort;
    ServerSocket *controlSocket;
    ServerSocket *stopSocket;

  signals:
    // to update the Listening to Gui check box
    void ServerStoppedSignal();
};
