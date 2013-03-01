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
class qDrawPlot;
class qTabMeasurement;

/** Project Class Headers */
class multiSlsDetector;
class MySocketTCP;
/** C++ Include Headers */


/**
 *@short Sets up the gui server
 */
class qServer: public virtual slsDetectorDefs{


public:
	/** \short The constructor	 */
	qServer(multiSlsDetector*& detector, qTabMeasurement* m, qDrawPlot *d);
	/** Destructor	 */
	~qServer();

	/** Start or Stop Gui Server
	 * @param start is 1 to start and 0 to stop
	 */
	int StartStopServer(int start);

private:
	/** assigns functions to the fnum enum */
	int function_table();

	/** Decodes Function */
	int decode_function();

	/** Unrecognized Function */
	int M_nofunc();


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

	/** Get Detector Status */
	int get_status();




	/** The multi detector object */
	multiSlsDetector *myDet;
	/**The measurement tab object*/
	qTabMeasurement *tab_measurement;
	/**The plot widget object*/
	qDrawPlot 		*myPlot;


	/** tcp socket to gui client */
	MySocketTCP 	*mySocket;
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

	/** server started */
	int checkStarted;

	/** Message */
	char mess[MAX_STR_LENGTH];

};



#endif /* QSERVER_H */
