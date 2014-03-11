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
	 * Returns the number of bits per pixel
	 */
	int getDynamicRange();

	/**
	 * Returns scan tag
	 */
	int getScanTag();

	/**
	 * Returns number of frames
	 */
	int getNumberOfFrames();

	/**
	 * Returns file write enable
	 */
	int getEnableFileWrite();

	/**
	 * Set File Name (without frame index, file index and extension)
	 @param c file name
	 /returns file name
	 */
	char* setFileName(char c[]);

	/**
	 * Set File Path
	 @param c file path
	 /returns file path
	 */
	char* setFilePath(char c[]);

	/**
	 * Returns the number of bits per pixel
	 @param dr sets dynamic range
	 /returns dynamic range
	 */
	int setDynamicRange(int dr);

	/**
	 * Set scan tag
	 @param tag scan tag
	 /returns scan tag
	 */
	int setScanTag(int tag);

	/**
	 * Sets number of frames
	 @param fnum number of frames
	 /returns number of frames
	 */
	int setNumberOfFrames(int fnum);

	/**
	 * Set enable file write
	 * @param i file write enable
	 /returns file write enable
	 */
	int setEnableFileWrite(int i);

	/**
	 * Starts Receiver - starts to listen for packets
	 @param message is the error message if there is an error
	 /returns success
	 */
	int startReceiver(char message[]);

	/**
	 * Stops Receiver - stops listening for packets
	 /returns success
	 */
	int stopReceiver();

	/**
	 * abort program with minimum damage
	 */
	void abort();


private:
	
};

#endif

