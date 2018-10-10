#pragma once


#include "sls_detector_defs.h"
#include "MySocketTCP.h"


/**
 * @short the ClientInterface class is the interface between the client and the server
 */


class ClientInterface{

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
	void SetSocket(MySocketTCP *socket){mySocket=socket;};

	/**
	 * Send a string to server
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg value to send
	 * \returns success of operation
	 */
	int SendString(int fnum, char retval[], char arg[]);

	/**
	 * Send a string to server
	 * @param fnum function enum to send udp ip and udp port
	 * @param retval return value server mac
	 * @param arg value to send
	 * \returns success of operation
	 */
	int SendUDPDetails(int fnum, char retval[], char arg[3][MAX_STR_LENGTH]);

	/**
	 * Send an integer to server
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg value to send
	 * \returns success of operation
	 */
	int SendInt(int fnum, int &retval, int arg);

	/**
	 * Get an integer value from server
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * \returns success of operation
	 */
	int GetInt(int fnum, int &retval);

	/**
	 * Send an integer to server
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg value to send
	 * \returns success of operation
	 */
	int SendInt(int fnum, int64_t &retval, int64_t arg);

	/**
	 * Send an integer to server
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg values to send
	 * @param mess message returned
	 * \returns success of operation
	 */
	int SendIntArray(int fnum, int64_t &retval, int64_t arg[2],char mess[]);

	/**
	 * Send an integer to server
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * @param arg values to send
	 * \returns success of operation
	 */
	int SendIntArray(int fnum, int &retval, int arg[2]);

	/**
	 * Get an integer value from server
	 * @param fnum function enum to determine what parameter
	 * @param retval return value
	 * \returns success of operation
	 */
	int GetInt(int fnum, int64_t &retval);

	/**
	 * Get last client ip connected to server
	 * @param fnum function enum to get last client up
	 * @param retval return value
	 * \returns success of operation
	 */
	int GetLastClientIP(int fnum, char retval[]);

	/**
	 * Send a function number to execute function
	 * @param fnum function enum to determine which function to execute
	 * @param mess return error message
	 * \returns success of operation
	 */
	int ExecuteFunction(int fnum,char mess[]);

	/**
	 * Send an integer to server
	 * @param fnum function enum to determine what parameter
	 * @param n number of ROIs to send
	 * @param roiLimits ROI structure
	 * \returns success of operation
	 */
	int SendROI(int fnum, int n, slsReceiverDefs::ROI roiLimits[]);


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


