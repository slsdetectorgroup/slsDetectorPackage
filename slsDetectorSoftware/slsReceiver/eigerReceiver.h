#ifdef EIGER_RECEIVER_H
/********************************************//**
 * @file eigerReceiver.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/




/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */

class eigerReceiver{ /** public slsDetectorDefs Its an enum unders slsDetectorsPackage/slsDetectorSoftware/commonFiles/slsDetectorDefs.h

public:
	/**
	 * Constructor
	 */
	eigerReceiver();

	/**
	 * Destructor
	 */
	virtual ~eigerReceiver();

	/**
	 * Initialize the Receiver
	 @param detectorHostName detector hostname
	 */
	void initialize(char* detectorHostName);

	/**
	 * Returns status of receiver: idle, running or error
	 */
	runStatus getStatus();

	/**
	 * Returns File Name
	 */
	char* getFileName();

	/**
	 * Returns File Path
	 */
	char* getFilePath();

	/**
	 * Set File Name (without frame index, file index and extension)
	 * @param c file name
	 */
	char* setFileName(char c[]);

	/**
	 * Set File Path
	 * @param c file path
	 */
	char* setFilePath(char c[]);

	/**
	 * Set enable file write
	 * @param i file write enable
	 * Returns file write enable
	 */
	int setEnableFileWrite(int i);

	/**
	 * Starts Receiver - starts to listen for packets
	 * @param message is the error message if there is an error
	 * Returns success
	 */
	int startReceiver(char message[]);

	/**
	 * Stops Receiver - stops listening for packets
	 * Returns success
	 */
	int stopReceiver();

	/**
	 * abort program with minimum damage
	 */
	void abort();


private:
	
};

#endif

