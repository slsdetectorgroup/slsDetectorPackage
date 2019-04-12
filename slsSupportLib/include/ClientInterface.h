#pragma once


#include "sls_detector_defs.h"
#include "MySocketTCP.h"
#include "ClientSocket.h"


/**
 * @short the ClientInterface class is the interface between the client and the server
 */
// Do not overload to make it easier for manual comparison between client and server functions

class ClientInterface: public virtual slsDetectorDefs{

public:

	/**
	 * (default) constructor
	 * @param socket tcp socket between client and receiver
	 * @param n for debugging purposes (useful only for client side)
	 * @param t string to identify type (Detector, Receiver) for printouts (useful only for client side)
	 */
	ClientInterface(sls::ClientSocket* socket, int n);

	/**
	 * destructor
	 */
	virtual ~ClientInterface() = default;

	void SetSocket(sls::ClientSocket *socket){
		socket_ = socket;
	}

	/**
	 * Receive ret, mess or retval from Server
	 * @param ret result of operation
	 * @param mess pointer to message
	 * @param retval pointer to retval
	 * @param sizeOfRetval size of retval
	 */
	void Client_Receive(int& ret, char* mess, void* retval, int sizeOfRetval);

	/**
	 * Send Arguments to server and receives result back
	 * @param fnum function enum to determine what parameter
	 * @param args pointer to arguments
	 * @param sizeOfArgs argument size
	 * @param retval pointer to return value
	 * @param sizeOfRetval return value size
	 * @param mess pointer to message if message required externally
	 * @returns success of operation
	 */
	int Client_Send(int fnum,
			void* args, int sizeOfArgs,
			void* retval, int sizeOfRetval,
			char* mess = 0);




private:

	/**
	 * socket for data acquisition
	 */
	sls::ClientSocket* socket_;

};


