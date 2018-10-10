#pragma once


#include "sls_detector_defs.h"
#include "MySocketTCP.h"


/**
 * @short the ClientInterface class is the interface between the client and the receiver/detector
 */


class ClientInterface{

public:

	/**
	 * (default) constructor
	 * @param socket tcp socket between client and receiver
	 */
	ClientInterface(MySocketTCP *socket);


	/**
	 * destructor
	 */
	virtual ~ClientInterface();

	/**
	 * Set the datasocket
	 * @param socket the data socket
	 */
	void SetSocket(MySocketTCP *socket){dataSocket=socket;};


	/**
	 * Send a string to receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg value to send
	 * \returns success of operation
	 */
	int SendString(int fnum, char retval[], char arg[]);

	/**
	 * Send a string to receiver
	 * @param fnum function enum to send udp ip and udp port
	 * @param retval return value receiver mac
	 * @param arg value to send
	 * \returns success of operation
	 */
	int sendUDPDetails(int fnum, char retval[], char arg[3][MAX_STR_LENGTH]);


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
	 * Send an integer to receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg value to send
	 * \returns success of operation
	 */
	int sendInt(int fnum, int64_t &retval, int64_t arg);


	/**
	 * Send an integer to receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg values to send
	 * @param mess message returned
	 * \returns success of operation
	 */
	int sendIntArray(int fnum, int64_t &retval, int64_t arg[2],char mess[]);


	/**
	 * Send an integer to receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg values to send
	 * \returns success of operation
	 */
	int sendIntArray(int fnum, int &retval, int arg[2]);

	/**
	 * Get an integer value from receiver
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * \returns success of operation
	 */
	int getInt(int fnum, int64_t &retval);

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
	 * @param mess return error message
	 * \returns success of operation
	 */
	int executeFunction(int fnum,char mess[]);

	/**
	 * Send an integer to receiver
	 * @param fnum function enum to determine what parameter
	 * @param n number of ROIs to send
	 * @param roiLimits ROI structure
	 * \returns success of operation
	 */
	int sendROI(int fnum, int n, slsReceiverDefs::ROI roiLimits[]);



private:

	/**
	 * socket for data acquisition
	 */
	MySocketTCP *dataSocket;

};


