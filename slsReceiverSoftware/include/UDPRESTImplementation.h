//#ifdef REST
#ifndef UDP_REST_IMPLEMENTATION_H
#define UDP_REST_IMPLEMENTATION_H
/********************************************//**
 * @file UDPRESTImplementation.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "UDPBaseImplementation.h"
#include "RestHelper.h"

#include <string.h>
#include <stdio.h>

/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */

class UDPRESTImplementation : protected virtual slsReceiverDefs, public UDPBaseImplementation {
public:
	/*************************************************************************
	 * Constructor & Destructor **********************************************
	 *************************************************************************/
	/**
	 * Constructor
	 */
	UDPRESTImplementation();

	/**
	 * Destructor
	 */
	virtual ~UDPRESTImplementation();


 protected:

	/*************************************************************************
	 * Getters ***************************************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/
	/**
	 * Get Rest State
	 */
	int get_rest_state(RestHelper * rest, string *rest_state);


	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/
	/**
	 * Initialize REST
	 */
	void initialize_REST();




 public:
	/*************************************************************************
	 * Getters ***************************************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/


	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/

	/**
	 * Overridden method
	 * Configure command line parameters
	 * @param config_map mapping of config parameters passed from command line arguments
	 */
	void configure(map<string, string> config_map);	


	/*************************************************************************
	 * Behavioral functions***************************************************
	 * They may modify the status of the receiver ****************************
	 *************************************************************************/

	/**
	 * Overridden method
	 * Start Listening for Packets by activating all configuration settings to receiver
	 * When this function returns, it has status RUNNING(upon SUCCESS) or IDLE (upon failure)
	 * @param c error message if FAIL
	 * @return OK or FAIL
	 */
	int startReceiver(char *c=NULL);

	/**
	 * Overridden method
	 * Stop Listening for Packets
	 * Calls startReadout(), which stops listening and sets status to Transmitting
	 * When it has read every frame in buffer, the status changes to Run_Finished
	 * When this function returns, receiver has status IDLE
	 * Pre: status is running, semaphores have been instantiated,
	 * Post: udp sockets shut down, status is idle, semaphores destroyed
	 */
	void stopReceiver();

	/**
	 * Overridden method
	 * Stop Listening to Packets
	 * and sets status to Transmitting
	 * Next step would be to get all data and stop receiver completely and return with idle state
	 * Pre: status is running, udp sockets have been initialized, stop receiver initiated
	 * Post:udp sockets closed, status is transmitting,
	 */
	void startReadout();

	/**
	 * Overridden method
	 * Shuts down and deletes UDP Sockets
	 * TCPIPInterface can also call this in case of illegal shutdown of receiver
	 * @return OK or FAIL
	 */
	int shutDownUDPSockets();

	/**
	 * Overridden method
	 * Get the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param startAcq start index of the acquisition
	 * @param startFrame start index of the scan
	 */
	void readFrame(char* c,char** raw, uint64_t &startAcq, uint64_t &startFrame);

	/**
	 * Overridden method
	 * Closes file / all files(data compression involves multiple files)
	 * TCPIPInterface can also call this in case of illegal shutdown of receiver
	 * @param i thread index valid for datacompression using root files, -1 for all threads
	 */
	void closeFile(int i = -1);

private:
	
	bool isInitialized;
	RestHelper * rest ;
	int rest_port;  		// receiver backend port
	string rest_hostname;  	// receiver hostname
	
};


#endif

//#endif /*REST*/
