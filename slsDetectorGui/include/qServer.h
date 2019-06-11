#pragma once

class qDetectorMain;

class multiSlsDetector;
class ServerSocket;

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
    int DecodeFunction(ServerSocket *sock);
    void ShutDownSockets();
    void ServerThread(ServerSocket* sock);
    int GetStatus(ServerSocket* sock);
    int StartAcquisition(ServerSocket* sock);
    int StopsAcquisition(ServerSocket* sock);
    int Acquire(ServerSocket* sock);
    int ExitServer(ServerSocket* sock);

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
