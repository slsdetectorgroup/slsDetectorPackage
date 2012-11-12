


#ifndef SLS_RECEIVER_INTERFACE_H
#define SLS_RECEIVER_INTERFACE_H

#include "MySocketTCP.h"



/**
 * 
 * @short the slsReceiverInterface class is the interface between the sls detector and the sls receiver
 * @author Dhanya Maliakal
 * @version 0.1alpha
 */


/**
   @short
 */
class receiverInterface{

public:

	/**
	 * (default) constructor
	 * @param socket tcp socket between client and receiver
	 */
	receiverInterface(MySocketTCP *socket);


	/**
	 * destructor
	 */
	virtual ~receiverInterface();

	/**
	 * Set the datasocket
	 * @param socket the data socket
	 */
	void setSocket(MySocketTCP *socket){dataSocket=socket;};


	/**
	 * Send a string to receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg value to send
	 * \returns success of operation
	 */
	int sendString(int fnum, char retval[], char arg[]);


	/**
	 * Send an integer to receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg value to send
	 * \returns success of operation
	 */
	int sendInt(int fnum, int &retval, int arg);


	/**
	 * Get an integer value from receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * \returns success of operation
	 */
	int getInt(int fnum, int &retval);


	/**
	 * Get last client ip connected to receiver
	 * @param fnum function enum to get last client up
	 * @param retval return value
	 * \returns success of operation
	 */
	int getLastClientIP(int fnum, char retval[]);


	/**
	 * Send a function number to execute function
	 * @param fnum function enum to determine which function to execute
	 * \returns success of operation
	 */
	int executeFunction(int fnum);



private:

	/**
	 * socket for data acquisition
	 */
	MySocketTCP *dataSocket;

};

#endif
