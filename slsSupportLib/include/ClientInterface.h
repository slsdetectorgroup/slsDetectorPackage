#pragma once


#include "sls_detector_defs.h"
#include "MySocketTCP.h"


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
	 * Get message from server
	 * Print appropriate message
	 * Check for Unrecognized function in message and return fail if it does
	 * to prevent getting retval from the server afterwards
	 * @param mess message
	 * @returns FAIL if unrecognized function found in message, else OK
	 */
	int Client_GetMesage(char* mess = 0);

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


	/** only Receiver
	 * Server sends result to client (also set ret to force_update if different clients)
	 * @param update true if one must update if different clients, else false
	 * @param ret success of operation
	 * @param retval pointer to result
	 * @param retvalSize size of result
	 * @param mess message
	 */
	void Server_SendResult(bool update, int ret, void* retval, int retvalSize, char* mess = 0);

	/** only Receiver
	 * Server receives arguments and checks if base object is null (if checkbase is true)
	 * checking base object is null (for reciever only when it has not been configured yet)
	 * @param ret pointer to success of operation
	 * @param mess message
	 * @param arg pointer to argument
	 * @param sizeofArg size of argument
	 * @param checkbase if true, checks if base object is null and sets ret and mess accordingly
	 * @param base pointer to base object
	 * @returns fail if socket crashes while reading arguments, else fail
	 */
	int Server_ReceiveArg(int& ret, char* mess, void* arg, int sizeofArg,bool checkbase=false, void* base=NULL);

	/** only Receiver
	 * Server verifies if it is unlocked,
	 * sets and prints appropriate message if it is locked and different clients
	 * @param ret pointer to sucess
	 * @param mess message
	 * @param lockstatus status of lock
	 * @returns success of operaton
	 */
	int Server_VerifyLock(int& ret, char* mess, int lockstatus);

	/** only Receiver
	 * Server verifies if it is unlocked and idle,
	 * sets and prints appropriate message if it is locked and different clients
	 * @param ret pointer to sucess
	 * @param mess message
	 * @param lockstatus status of lock
	 * @param staus status of server
	 * @param fnum function number for error message
	 * @returns success of operaton
	 */
	int Server_VerifyLockAndIdle(int& ret, char* mess, int lockstatus, slsDetectorDefs::runStatus status, int fnum);

	/** only Receiver
	 * Server sets and prints error message for null object error (receiver only)
	 * @param ret pointer to sucess that will be set to FAIL
	 * @param mess message
	 */
	void Server_NullObjectError(int& ret, char* mess);

	/** only Receiver
	 * Servers prints error message for socket crash when reading
	 * @returns always FAIL
	 */
	int Server_SocketCrash();

	/** only Receiver
	 * Servers sets and prints error message for locked server
	 * @param ret pointer to sucess that will be set to FAIL
	 * @param mess message
	 * @returns success of operaton
	 */
	int Server_LockedError(int& ret, char* mess);

	/** only Receiver
	 * Servers sets and prints error message for server not being idle
	 * @param ret pointer to sucess that will be set to FAIL
	 * @param mess message
	 * @param fnum function number for error message
	 * @returns success of operaton
	 */
	int Server_NotIdleError(int& ret, char* mess, int fnum);

private:

	/**
	 * socket for data acquisition
	 */
	MySocketTCP *mySocket;

	/** index for client debugging purposes */
	int index;

	/** string for type to differentiate between Detector & Receiver in printouts */
	std::string type;

};


