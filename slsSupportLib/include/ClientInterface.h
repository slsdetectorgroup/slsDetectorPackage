#pragma once


#include "sls_receiver_defs.h"
#include "MySocketTCP.h"


/**
 * @short the ClientInterface class is the interface between the client and the server
 */
// Do not overload to make it easier for manual comparison between client and server functions

class ClientInterface: public virtual slsReceiverDefs{

public:

	/**
	 * (default) constructor
	 * @param socket tcp socket between client and receiver
	 * @param n for debugging purposes (useful only for client side)
	 * @param t string to identify type (Detector, Receiver) for printouts (useful only for client side)
	 */
	ClientInterface(MySocketTCP *socket, int n=-1, std::string t="");

	/**
	 * destructor
	 */
	virtual ~ClientInterface();

	/**
	 * Set the datasocket
	 * @param socket the data socket
	 */
	void SetSocket(MySocketTCP *socket);

	/**
	 * Print socket read error in Server
	 */
	int PrintSocketReadError();

	/**
	 * Server sends result to client
	 * @param ret success of operation
	 * @param retval pointer to result
	 * @param retvalSize size of result
	 */
	void Server_SendResult(int ret, void* retval, int retvalSize);

	/**
	 * Get message from server
	 * Print appropriate message
	 * Check for Unrecognized function in message and return fail if it does
	 * to prevent getting retval from the server afterwards
	 * @param mess message
	 * @returns FAIL if unrecognized function found in message, else OK
	 */
	int Client_GetMesage(char* mess=0);

	/**
	 * Send Arguments to server and get result back
	 * @param fnum function enum to determine what parameter
	 * @param args pointer to arguments
	 * @param sizeOfArgs argument size
	 * @param retval pointer to return value
	 * @param sizeOfRetval return value size
	 * @param mess pointer to message if message required externally
	 */
	int Client_Send(int fnum,
			void* args, int sizeOfArgs,
			void* retval, int sizeOfRetval,
			char* mess = 0);

	/**
	 * Send Arguments (second set) to server and get result back
	 * @param fnum function enum to determine what parameter
	 * @param args pointer to arguments
	 * @param sizeOfArgs argument size
	 * @param args2 pointer to arguments 2
	 * @param sizeOfArgs2 argument size 2
	 * @param retval pointer to return value
	 * @param sizeOfRetval return value size
	 * @param mess pointer to message if message required externally
	 */
	int Client_Send(int fnum,
			void* args, int sizeOfArgs,
			void* args2, int sizeOfArgs2,
			void* retval, int sizeOfRetval,
			char* mess = 0);


private:

	/**
	 * socket for data acquisition
	 */
	MySocketTCP *mySocket;

	/** index for debugging purposes */
	int index;

	/** string for type to differentiate between Detector & Receiver in printouts */
	std::string type;

};


