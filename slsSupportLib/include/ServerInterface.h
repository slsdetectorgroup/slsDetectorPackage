#pragma once


#include "sls_detector_defs.h"
#include "MySocketTCP.h"


/**
 * @short the ServerInterface class is the interface between the client and the server
 */
// Do not overload to make it easier for manual comparison between client and server functions

class ServerInterface: public virtual slsDetectorDefs{

public:

	/**
	 * (default) constructor
	 * @param socket tcp socket between client and receiver
	 * @param n for debugging purposes (useful only for client side)
	 * @param t string to identify type (Detector, Receiver) for printouts (useful only for client side)
	 */
	ServerInterface(MySocketTCP *socket, int n=-1, std::string t="");

	/**
	 * destructor
	 */
	virtual ~ServerInterface() = default;

	/**
	 * Set the datasocket
	 * @param socket the data socket
	 */
	void SetSocket(MySocketTCP *socket);

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


	/** only Receiver
	 * Server sends result to client (also set ret to force_update if different clients)
	 * @param update true if one must update if different clients, else false
	 * @param ret success of operation
	 * @param retval pointer to result
	 * @param retvalSize size of result
	 * @param mess message
	 * @returns success of operation
	 */
	int Server_SendResult(bool update, int ret, void* retval, int retvalSize, char* mess = 0);

	/** only Receiver
	 * Server receives arguments and checks if base object is null (if checkbase is true)
	 * checking base object is null (for receiver only when it has not been configured yet)
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
	 * @param ret pointer to success
	 * @param mess message
	 * @param lockstatus status of lock
	 * @returns success of operaton
	 */
	int Server_VerifyLock(int& ret, char* mess, int lockstatus);

	/** only Receiver
	 * Server verifies if it is unlocked and idle,
	 * sets and prints appropriate message if it is locked and different clients
	 * @param ret pointer to success
	 * @param mess message
	 * @param lockstatus status of lock
	 * @param status status of server
	 * @param fnum function number for error message
	 * @returns success of operaton
	 */
	int Server_VerifyLockAndIdle(int& ret, char* mess, int lockstatus, slsDetectorDefs::runStatus status, int fnum);

	/** only Receiver
	 * Server sets and prints error message for null object error (receiver only)
	 * @param ret pointer to success that will be set to FAIL
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
	 * @param ret pointer to success that will be set to FAIL
	 * @param mess message
	 * @returns success of operaton
	 */
	int Server_LockedError(int& ret, char* mess);

	/** only Receiver
	 * Servers sets and prints error message for server not being idle
	 * @param ret pointer to success that will be set to FAIL
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


