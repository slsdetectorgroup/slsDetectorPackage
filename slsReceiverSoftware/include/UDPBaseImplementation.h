//#ifdef UDP_BASE_IMPLEMENTATION
#ifndef UDP_BASE_IMPLEMENTATION_H
#define UDP_BASE_IMPLEMENTATION_H
/********************************************//**
 * @file UDPBaseImplementation.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


//#include "sls_receiver_defs.h"
#include "UDPInterface.h"
//#include <stdio.h>

/**
 * @short does all the base functions for a receiver, set/get parameters, start/stop etc.
 */

class UDPBaseImplementation : protected virtual slsReceiverDefs, public UDPInterface {
	
 public:

	/*************************************************************************
	 * Constructor & Destructor **********************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/
	/**
	 * Constructor
	 */
	UDPBaseImplementation();

	/**
	 * Destructor
	 */
	virtual ~UDPBaseImplementation();

	/*
	 * Initialize class members
	 */
	void initializeMembers();

	/*************************************************************************
	 * Getters ***************************************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/

	//**initial parameters***
	/*
	 * Get detector hostname
	 * @return NULL or hostname or NULL if uninitialized (max of 1000 characters)
	 */
	char *getDetectorHostname() const;


	//***file parameters***
	/**
	 * Get File Name Prefix (without frame index, file index and extension (_d0_f000000000000_8.raw))
	 * @return NULL or file name prefix (max of 1000 characters)
	 */
	char *getFileName() const;

	/**
	 * Get File Path
	 * @return NULL or file path (max of 1000 characters)
	 */
	char *getFilePath() const;

	/**
	 * Get File Index
	 * @return file index of acquisition
	 */
	uint64_t getFileIndex() const;

	/**
	 * Get Scan Tag
	 * @return scan tag //FIXME: needed? (unsigned integer?)
	 */
	int getScanTag() const;

	/**
	 * Get if Frame Index is enabled (acquisition of more than 1 frame adds '_f000000000000' to file name )
	 * @return true if frame index needed, else false
	 */
	bool getFrameIndexEnable() const;

	/**
	 * Get File Write Enable
	 * @return true if file write enabled, else false
	 */
	bool getFileWriteEnable() const;

	/**
	 * Get File Over Write Enable
	 * @return true if file over write enabled, else false
	 */
	bool getOverwriteEnable() const;

	/**
	 * Get data compression, by saving only hits (so far implemented only for Moench and Gotthard)
	 * @return true if data compression enabled, else false
	 */
	bool getDataCompressionEnable() const;


	//***acquisition count parameters***
	/**
	 * Get Total Frames Caught for an entire acquisition (including all scans)
	 * @return total number of frames caught for entire acquisition
	 */
	uint64_t getTotalFramesCaught() const;

	/**
	 * Get Frames Caught for each real time acquisition (eg. for each scan)
	 * @return number of frames caught for each scan
	 */
	uint64_t getFramesCaught() const;

	/**
	 * Get Current Frame Index for an entire  acquisition (including all scans)
	 * @return current frame index (represents all scans too)
	 */
	int64_t getAcquisitionIndex() const;


	//***connection parameters***
	/**
	 * Get UDP Port Number
	 * @return udp port number
	 */
	uint32_t getUDPPortNumber() const;

	/**
	 * Get Second UDP Port Number (eiger specific)
	 * @return second udp port number
	 */
	uint32_t getUDPPortNumber2() const;

	/**
	 * Get Ehernet Interface
	 * @ethernet interface. eg. eth0 or "" if listening to all (max of 1000 characters)
	 */
	char *getEthernetInterface() const;


	//***acquisition parameters***
	/**
	 * Get Short Frame Enabled, later will be moved to getROI (so far only for gotthard)
	 * @return index of adc enabled, else -1 if all enabled
	 */
	int getShortFrameEnable() const;

	/**
	 * Get the Frequency of Frames Sent to GUI
	 * @return 0 for random frame requests, n for nth frame frequency
	 */
	uint32_t getFrameToGuiFrequency() const;

	/**
	 * Get Acquisition Period
	 * @return acquisition period
	 */
	uint64_t getAcquisitionPeriod() const;

	/*
	 * Get Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames FIXME: (Not implemented)
	 * @return number of frames expected
	 */
	uint64_t getNumberOfFrames() const;

	/**
	 * Get Dynamic Range or Number of Bits Per Pixel
	 * @return dynamic range that is 4, 8, 16 or 32
	 */
	uint32_t getDynamicRange() const;

	/**
	 * Get Ten Giga Enable
	 * @return true if 10Giga enabled, else false (1G enabled)
	 */
	bool getTenGigaEnable() const;

	/**
	 * Get Fifo Depth
	 * @return fifo depth
	 */
	uint32_t getFifoDepth() const;


	//***receiver status***
	/**
	 * Get Listening Status of Receiver
	 * @return can be idle, listening or error depending on if the receiver is listening or not
	 */
	runStatus getStatus() const;




	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/

	//**initial parameters***
	/**
	 * Configure command line parameters
	 * @param config_map mapping of config parameters passed from command line arguments
	 */
	void configure(map<string, string> config_map);

	/**
	 * Set Bottom Enable  (eiger specific, should be moved to configure, and later from client via TCPIP)
	 * @param b is true for bottom enabled or false for bottom disabled
	 */
	void setBottomEnable(const bool b);


	//***file parameters***
	/**
	 * Set File Name Prefix (without frame index, file index and extension (_d0_f000000000000_8.raw))
	 * Does not check for file existence since it is created only at startReceiver
	 * @param c file name (max of 1000 characters)
	 */
	void setFileName(const char c[]);

	/**
	 * Set File Path
	 * Checks for file directory existence before setting file path,
	 * If it doesn't exist, it will set it blank
	 * @param c file path (max of 1000 characters)
	 */
	void setFilePath(const char c[]);

	/**
	 * Set File Index of acquisition
	 * @param i file index of acquisition
	 */
	void setFileIndex(const uint64_t i);

	/**
	 * Set Scan Tag
	 * @param i scan tag //FIXME: needed? (unsigned integer?)
	 */
	void setScanTag(const int i);

	/**
	 * Set Frame Index Enable (acquisition of more than 1 frame adds '_f000000000000' to file name )
	 * @param b true for frame index enable, else false
	 */
	void setFrameIndexEnable(const bool b);

	/**
	 * Set File Write Enable
	 * @param b true for file write enable, else false
	 */
	void setFileWriteEnable(const bool b);

	/**
	 * Set File Overwrite Enable
	 * @param b true for file overwrite enable, else false
	 */
	void setOverwriteEnable(const bool b);

	/**
	 * Set data compression, by saving only hits (so far implemented only for Moench and Gotthard)
	 * @param b true for data compression enable, else false
	 * @return OK or FAIL
	 */
	int setDataCompressionEnable(const bool b);


	//***connection parameters***
	/**
	 * Set UDP Port Number
	 * @param i udp port number
	 */
	void setUDPPortNumber(const uint32_t i);

	/**
	 * Set Second UDP Port Number (eiger specific)
	 * @return second udp port number
	 */
	void setUDPPortNumber2(const uint32_t i);

	/**
	 * Set Ethernet Interface to listen to
	 * @param c ethernet inerface eg. eth0 (max of 1000 characters)
	 */
	void setEthernetInterface(const char* c);


	//***acquisition parameters***
	/**
	 * Set Short Frame Enabled, later will be moved to getROI (so far only for gotthard)
	 * @param i index of adc enabled, else -1 if all enabled
	 */
	void setShortFrameEnable(const int i);

	/**
	 * Set the Frequency of Frames Sent to GUI
	 * @param i 0 for random frame requests, n for nth frame frequency
	 * @return OK or FAIL
	 */
	int setFrameToGuiFrequency(const uint32_t i);

	/**
	 * Set Acquisition Period
	 * @param i acquisition period
	 * @return OK or FAIL
	 */
	int setAcquisitionPeriod(const uint64_t i);

	/**
	 * Set Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames FIXME: (Not implemented)
	 * @param i number of frames expected
	 */
	void setNumberOfFrames(const uint64_t i);

	/**
	 * Set Dynamic Range or Number of Bits Per Pixel
	 * @param i dynamic range that is 4, 8, 16 or 32
	 * @return OK or FAIL
	 */
	int setDynamicRange(const uint32_t i);

	/**
	 * Set Ten Giga Enable
	 * @param b true if 10Giga enabled, else false (1G enabled)
	 * @return OK or FAIL
	 */
	int setTenGigaEnable(const bool b);

	/**
	 * Set Fifo Depth
	 * @param i fifo depth value
	 * @return OK or FAIL
	 */
	int setFifoDepth(const uint32_t i);


	/*************************************************************************
	 * Behavioral functions***************************************************
	 * They may modify the status of the receiver ****************************
	 *************************************************************************/


	//***initial functions***
	/**
	 * Set receiver type (and corresponding detector variables in derived STANDARD class)
	 * It is the first function called by the client when connecting to receiver
	 * @param d detector type
	 * @return OK or FAIL
	 */
	int setDetectorType(const detectorType d);

	/**
	 * Sets detector hostname (and corresponding detector variables in derived REST class)
	 * It is second function called by the client when connecting to receiver.
	 * you can call this function only once. //FIXME: is this still valid, this implemented in derived REST class?
	 * @param c detector hostname
	 */
	void initialize(const char *c);


	//***acquisition functions***
	/**
	 * Reset acquisition parameters such as total frames caught for an entire acquisition (including all scans)
	 */
	void resetAcquisitionCount();

	/**
	 * Start Listening for Packets by activating all configuration settings to receiver
	 * @param c error message if FAIL
	 * @return OK or FAIL
	 */
	int startReceiver(char *c=NULL);

	/**
	 * Stop Listening for Packets
	 * Calls startReadout(), which stops listening and sets status to Transmitting
	 * When it has read every frame in buffer,it returns with the status Run_Finished
	 */
	void stopReceiver();

	/**
	 * Stop Listening to Packets
	 * and sets status to Transmitting
	 */
	void startReadout();

	/**
	 * Shuts down and deletes UDP Sockets
	 * @return OK or FAIL
	 */
	int shutDownUDPSockets();

	/**
	 * Get the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param startAcq start index of the acquisition
	 * @param startFrame start index of the scan
	 */
	void readFrame(char* c,char** raw, uint64_t &startAcq, uint64_t &startFrame);

	/**
	 * abort acquisition with minimum damage: close open files, cleanup.
	 * does nothing if state already is 'idle'
	 */
	void abort();  //FIXME: needed, isn't stopReceiver enough?

	/**
	 * Closes file / all files(if multiple files)
	 * @param i thread index (if multiple files used  eg. root files) -1 for all threads
	 */
	void closeFile(int i = -1);


	//***callback functions***
	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is the action which decides what the user and default responsibilities to save data are
	 * 0 callback takes care of open,close,wrie file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything
	 */
	void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg);

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);

	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * framenum
	 * datapointer
	 * datasize in bytes
	 * file descriptor
	 * guidatapointer (NULL, no data required)
	 */
	void registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg);




 protected:

	/*************************************************************************
	 * Class Members *********************************************************
	 *************************************************************************/
	//**detector parameters***
	/** detector type */
	detectorType myDetectorType;
	/** detector hostname */
	char detHostname[MAX_STR_LENGTH];
	/** Number of Packets per Frame*/
	uint32_t packetsPerFrame;
	/** Acquisition Period */
	int64_t acquisitionPeriod;
	/** Frame Number */
	int64_t numberOfFrames;
	/** Dynamic Range */
	uint32_t dynamicRange;
	/** Ten Giga Enable*/
	bool tengigaEnable;
	/** Fifo Depth */
	uint32_t fifoDepth;
	/** Bottom Half Module Enable */
	bool bottomEnable;

	//***receiver parameters***
	/** Maximum Number of Listening Threads/ UDP Ports */
	const static int MAX_NUMBER_OF_LISTENING_THREADS = 2;
	/** Receiver Status */
	runStatus status;

	//***connection parameters***
	/** Ethernet Interface */
	char eth[MAX_STR_LENGTH];
	/** Server UDP Port Number*/
	uint32_t udpPortNum[MAX_NUMBER_OF_LISTENING_THREADS];

	//***file parameters***
	/** File Name without frame index, file index and extension (_d0_f000000000000_8.raw)*/
	char fileName[MAX_STR_LENGTH];
	/** File Path */
	char filePath[MAX_STR_LENGTH];
	/** File Index */
	uint64_t fileIndex;
	/** Scan Tag */
	int scanTag;
	/** Frame Index Enable */
	bool frameIndexEnable;
	/** File Write enable */
	bool fileWriteEnable;
	/** Overwrite enable */
	bool overwriteEnable;
	/** Data Compression Enable - save only hits */
	bool dataCompressionEnable;

	//***acquisition count parameters***
	/** Total packets caught for an entire acquisition (including all scans) */
	uint64_t totalPacketsCaught;
	/** Frames Caught for each real time acquisition (eg. for each scan) */
	uint64_t packetsCaught;

	//***acquisition indices parameters***
	/** Actual current frame index of an entire acquisition (including all scans) */
	uint64_t acquisitionIndex;

	//***acquisition parameters***
	/* Short Frame Enable or index of adc enabled, else -1 if all enabled (gotthard specific) TODO: move to setROI */
	int shortFrameEnable;
	/** Frequency of Frames sent to GUI */
	uint32_t FrameToGuiFrequency;



	//***callback parameters***
	/**
	 * function being called back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is
	 * 0 callback takes care of open,close,wrie file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything
	 */
	int (*startAcquisitionCallBack)(char*, char*,int, int, void*);
	void *pStartAcquisition;

	/**
	 * function being called back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void (*acquisitionFinishedCallBack)(int, void*);
	void *pAcquisitionFinished;


	/**
	 * function being called back for raw data
	 * args to raw data ready callback are
	 * framenum
	 * datapointer
	 * datasize in bytes
	 * file descriptor
	 * guidatapointer (NULL, no data required)
	 */
	void (*rawDataReadyCallBack)(int, char*, int, FILE*, char*, void*);
	void *pRawDataReady;


private:

};


#endif

//#endif
