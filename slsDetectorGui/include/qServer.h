/*
 * qServer.h.h
 *
 *  Created on: Feb 27, 2013
 *      Author: Dhanya Maliakal
 */
#ifndef QSERVER_H
#define QSERVER_H


/** Qt Project Class Headers */
#include "sls_detector_defs.h"
#include "qDefs.h"
class qDetectorMain;
/** Project Class Headers */
class multiSlsDetector;
class MySocketTCP;
/** Qt Include Headers */
#include <QWidget>
/** C++ Include Headers */


/**
 *@short Sets up the gui server
 */
class qServer: public QWidget, public virtual slsDetectorDefs{
		Q_OBJECT


public:
	/** \short The constructor	 */
	qServer(qDetectorMain *t);
	/** Destructor	 */
	~qServer();

	/** Start or Stop Gui Server
	 * @param start is 1 to start and 0 to stop
	 */
	int StartStopServer(int start);

private:
	/** assigns functions to the fnum enum */
	int FunctionTable();

	/** Decodes Function */
	int DecodeFunction(MySocketTCP* sock);

	/** Exit Server */
	int ExitServer();


	/**
	 * Static function - Thread started which listens to client gui.
	 * Called by StartStopServer()
	 * @param this_pointer pointer to this object
	 */
	static void* StartServerThread(void *this_pointer);

	/**
	 * Thread started which listens to client gui.
	 * Called by startServerThread()
	 *
	 */
	int StartServer();

	/**
	 * Static function - Thread started which listens to client gui to stop acquisition
	 * Called by StartStopServer()
	 * @param this_pointer pointer to this object
	 */
	static void* StopServerThread(void *this_pointer);

	/**
	 * Thread started which listens to client gui to stop acquisition.
	 * Called by startServerThread()
	 *
	 */
	int StopServer();



	/** Get Detector Status */
	int GetStatus();

	/** Starts Acquisition */
	int StartAcquisition();

	/** Stops Acquisition */
	int StopsAcquisition();

	/** Acquire - blocking */
	int Acquire();



	/**The measurement tab object*/
	qDetectorMain *myMainTab;

	/** tcp socket to gui client */
	MySocketTCP 	*mySocket;
	/** tcp socket to gui client to stop or get status */
	MySocketTCP 	*myStopSocket;

	/** server port number*/
	int port_no;

	/** Lock Status if server locked to a client */
	int lockStatus;

	/** Function List */
	static const int NUMBER_OF_FUNCTIONS = 256;
	int (qServer::*flist[NUMBER_OF_FUNCTIONS])();


	/** if the gui server thread is running*/
	static int gui_server_thread_running;
	/** thread listening to gui client*/
	pthread_t   gui_server_thread;
	/** thread also listening to gui client to stop acquisition*/
	pthread_t   gui_stop_server_thread;

	/** server started */
	int checkStarted;
	int checkStopStarted;

	/** Message */
	char mess[MAX_STR_LENGTH];


signals:
	void ServerStoppedSignal();

};



#endif /* QSERVER_H */
