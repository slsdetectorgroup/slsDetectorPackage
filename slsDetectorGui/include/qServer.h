#pragma once

#include "qDefs.h"
#include "sls_detector_defs.h"
class qDetectorMain;

class multiSlsDetector;
class ServerSocket;

#include <QWidget>

#include <vector>

/**
 *@short Sets up the gui server
 */
class qServer : public QWidget, public virtual slsDetectorDefs {
    Q_OBJECT

  public:
    /**
     * The constructor
     */
    qServer(qDetectorMain *t);
    /**
     * Destructor
     */
    ~qServer();

    /**
     * Create (Control and Stop) Gui Servers
     */
    void CreateServers();

    /**
     * Destroy (Control and Stop) Gui Servers
     */
    void DestroyServers();

  private:
    /**
     * Assigns functions to the fnum enum
     */
    void FunctionTable();

    /**
     * Decodes Function
     * @param sock control or stop socket
     * @returns OK or FAIL
     */
    int DecodeFunction(ServerSocket *sock);

    /** 
     * Shut down Sockets
     */
    void ShutDownSockets();

    /**
     * Server thread
     * @param pointer to control or stop socket
     */
    void ServerThread(ServerSocket* sock);

    /**
     * Get Detector Status
     * @returns success of operation
     */
    int GetStatus(ServerSocket* sock);

    /**
     * Starts Acquisition
     * @returns success of operation
     */
    int StartAcquisition(ServerSocket* sock);

    /**
     * Stops Acquisition
     * @returns success of operation
     */
    int StopsAcquisition(ServerSocket* sock);

    /**
     * Acquire - blocking
     * @returns success of operation
     */
    int Acquire(ServerSocket* sock);

    /**
     * Exit Server
     * @returns GOODBYE
     */
    int ExitServer(ServerSocket* sock);

    /** function list */
    typedef int (qServer::*some_func_t)(ServerSocket*);
    typedef std::vector<some_func_t> sflist;

    /** if the gui server thread is running*/
    bool threadRunning;

    /** if thread started */
    bool threadStarted;

    /**The measurement tab object*/
    qDetectorMain *mainTab;

    /** control port */
    int controlPort;

    /** stop port */
    int stopPort;

    /** control socket */
    ServerSocket *controlSocket;

    /** stop socket */
    ServerSocket *stopSocket;

  signals:
    // to update the Listening to Gui check box
    void ServerStoppedSignal();
};
