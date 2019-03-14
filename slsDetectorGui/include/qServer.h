#pragma once


#include "sls_detector_defs.h"
#include "qDefs.h"
class qDetectorMain;

class multiSlsDetector;
class MySocketTCP;

#include <QWidget>



/**
 *@short Sets up the gui server
 */
class qServer: public QWidget, public virtual slsDetectorDefs{
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
	 * Start or Stop Gui (Control and Stop) Servers
	 * @param start is 1 to start and 0 to stop
	 */
	void StartServers(int start);

private:
	/**
	 * Assigns functions to the fnum enum
	 */
	void FunctionTable();

	/**
	 * Decodes Function
	 * @param sock control or stop socket
	 * @returns success of function
	 */
	int DecodeFunction(MySocketTCP* sock);

	/**
	 * Exit Server
	 * @returns GOODBYE
	 */
	int ExitServer();

	/**
	 * Static function - to start contol server
	 * @param this_pointer pointer to this object
	 */
	static void* ControlServerThread(void *this_pointer);

	/**
	 * Thread of control server
	 */
	void ControlServer();

	/**
	 * Static function - to start stop server
	 * @param this_pointer pointer to this object
	 */
	static void* StopServerThread(void *this_pointer);

	/**
	 * Thread of stop server
	 */
	void StopServer();

	/**
	 * Get Detector Status
	 * @returns success of operation
	 */
	int GetStatus();

	/**
	 * Starts Acquisition
	 * @returns success of operation
	 */
	int StartAcquisition();

	/**
	 * Stops Acquisition
	 * @returns success of operation
	 */
	int StopsAcquisition();

	/**
	 * Acquire - blocking
	 * @returns success of operation
	 */
	int Acquire();

	/**The measurement tab object*/
	qDetectorMain *myMainTab;

	/** tcp socket to gui client */
	MySocketTCP *controlSocket;
	/** tcp socket to gui client to stop or get status */
	MySocketTCP *stopSocket;

	/** server port number */
	int portNo;

	/** Function List */
	static const int NUMBER_OF_FUNCTIONS = 10;
	int (qServer::*flist[NUMBER_OF_FUNCTIONS])();

	/** if the gui server thread is running*/
	static int threadRunning;
	/** thread listening to gui client*/
	pthread_t controlThread;
	/** thread also listening to gui client to stop acquisition*/
	pthread_t stopThread;

	/** server started */
	int checkControlStarted;
	int checkStopStarted;

	/** Message */
	char mess[MAX_STR_LENGTH];


signals:
	void ServerStoppedSignal();

};

