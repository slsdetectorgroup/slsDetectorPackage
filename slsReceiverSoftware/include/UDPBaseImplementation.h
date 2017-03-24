#pragma once
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
	 * Get multi detector size
	 * @return pointer to array of multi detector size in every dimension
	 */
	int* getMultiDetectorSize() const;


	/*
	 * Get detector position id
	 * @return detector position id
	 */
	int getDetectorPositionId() const;

	/*
	 * Get detector hostname
	 * @return NULL or hostname or NULL if uninitialized (max of 1000 characters)
	 */
	char *getDetectorHostname() const;

	/*
	 * Get flipped data across 'axis'
	 * @return if data is flipped across 'axis'
	 */
	int getFlippedData(int axis=0) const;


	//***file parameters***
	/**
	 * Get File Format
	 * @return file format
	 */
	fileFormat getFileFormat() const;
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
	 * @return -1 if no frames have been caught, else current frame index (represents all scans too)
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
	 * Gets the timer between frames streamed when frequency is set to 0
	 * @return timer between frames streamed
	 */
	uint32_t getFrameToGuiTimer() const;

	/**
	 * Get the data stream enable
	 * @return data stream enable
	 */
	bool getDataStreamEnable() const;


	/**
	 * Get Acquisition Period
	 * @return acquisition period
	 */
	uint64_t getAcquisitionPeriod() const;

	/**
	 * Get Acquisition Time
	 * @return acquisition time
	 */
	uint64_t getAcquisitionTime() const;

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

	/**
	 * Get activate
	 * If deactivated, receiver will write dummy packets 0xFF
	 * (as it will receive nothing from detector)
	 * @return 0 for deactivated, 1 for activated
	 */
	int getActivate() const;



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

	/*
	 * Set multi detector size
	 * @param pointer to array of multi detector size in every dimension
	 */
	void setMultiDetectorSize(const int* size);

	/*
	 * Get flipped data across 'axis'
	 * @return if data is flipped across 'axis'
	 */
	void setFlippedData(int axis=0, int enable=-1);


	//***file parameters***
	/**
	 * Set File Format
	 * @param f fileformat binary or hdf5
	 */
	void setFileFormat(slsReceiverDefs::fileFormat f);

	/**
	 * Set File Name Prefix (without frame index, file index and extension (_d0_f000000000000_8.raw))
	 * Does not check for file existence since it is created only at startReceiver
	 * @param c file name (max of 1000 characters)
	 */
	void setFileName(const char c[]);

	/**
	 * Set File Path
	 * Checks for file directory existence before setting file path,
	 * If it exists, it sets it
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
	 * @return OK or FAIL
	 */
	int setShortFrameEnable(const int i);

	/**
	 * Set the Frequency of Frames Sent to GUI
	 * @param freq 0 for random frame requests, n for nth frame frequency
	 * @return OK or FAIL
	 */
	int setFrameToGuiFrequency(const uint32_t freq);

	/**
	 * Sets the timer between frames streamed when frequency is set to 0
	 * @param time_in_ms timer between frames streamed
	 */
	void setFrameToGuiTimer(const uint32_t time_in_ms);

	/**
	 * Set the data stream enable
	 * @param enable data stream enable
	 * @return OK or FAIL
	 */
	int setDataStreamEnable(const bool enable);

	/**
	 * Set Acquisition Period
	 * @param i acquisition period
	 * @return OK or FAIL
	 */
	int setAcquisitionPeriod(const uint64_t i);

	/**
	 * Set Acquisition Time
	 * @param i acquisition time
	 * @return OK or FAIL
	 */
	int setAcquisitionTime(const uint64_t i);

	/**
	 * Set Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames
	 * @param i number of frames expected
	 * @return OK or FAIL
	 */
	int setNumberOfFrames(const uint64_t i);

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
	 * Set detector position id
	 * @param i position id
	 */
	void setDetectorPositionId(const int i);

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
	 */
	void shutDownUDPSockets();

	/**
	 * Get the buffer-current frame read by receiver
	 * @param ithread port thread index
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param startAcq start index of the acquisition
	 * @param startFrame start index of the scan
	 */
	void readFrame(int ithread, char* c,char** raw, int64_t &startAcq, int64_t &startFrame);

	/**
	 * abort acquisition with minimum damage: close open files, cleanup.
	 * does nothing if state already is 'idle'
	 */
	void abort();  //FIXME: needed, isn't stopReceiver enough?

	/**
	 * Closes all files
	 */
	void closeFiles();

	/**
	 * Activate / Deactivate Receiver
	 * If deactivated, receiver will write dummy packets 0xFF
	 * (as it will receive nothing from detector)
	 */
	int setActivate(int enable = -1);

	//***callback functions***
	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is insignificant at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	void registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg);

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg);

	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * frameNumber is the frame number
	 * expLength is the subframe number (32 bit eiger) or real time exposure time in 100ns (others)
	 * packetNumber is the packet number
	 * bunchId is the bunch id from beamline
	 * timestamp is the time stamp with 10 MHz clock
	 * modId is the unique module id (unique even for left, right, top, bottom)
	 * xCoord is the x coordinate in the complete detector system
	 * yCoord is the y coordinate in the complete detector system
	 * zCoord is the z coordinate in the complete detector system
	 * debug is for debugging purposes
	 * roundRNumber is the round robin set number
	 * detType is the detector type see :: detectorType
	 * version is the version number of this structure format
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes
	 */
	void registerCallBackRawDataReady(void (*func)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
			char*, uint32_t, void*),void *arg);




 protected:

	/*************************************************************************
	 * Class Members *********************************************************
	 *************************************************************************/
	//**detector parameters***
	/** detector type */
	detectorType myDetectorType;
	/** Number of Detectors in each dimension direction */
	int numDet[MAX_DIMENSIONS];
	/*Detector Readout ID*/
	int detID;
	/** detector hostname */
	char detHostname[MAX_STR_LENGTH];
	/** Acquisition Period */
	uint64_t acquisitionPeriod;
	/** Acquisition Time */
	uint64_t acquisitionTime;
	/** Frame Number */
	uint64_t numberOfFrames;
	/** Dynamic Range */
	uint32_t dynamicRange;
	/** Ten Giga Enable*/
	bool tengigaEnable;
	/** Fifo Depth */
	uint32_t fifoDepth;
	/** enable for flipping data across both axes */
	int flippedData[2];

	//***receiver parameters***
	/** Maximum Number of Listening Threads/ UDP Ports */
	const static int MAX_NUMBER_OF_LISTENING_THREADS = 2;
	/** Receiver Status */
	runStatus status;
	/** Activated/Deactivated */
	int activated;

	//***connection parameters***
	/** Ethernet Interface */
	char eth[MAX_STR_LENGTH];
	/** Server UDP Port Number*/
	uint32_t udpPortNum[MAX_NUMBER_OF_LISTENING_THREADS];

	//***file parameters***
	/** File format */
	fileFormat fileFormatType;
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

	//***acquisition parameters***
	/* Short Frame Enable or index of adc enabled, else -1 if all enabled (gotthard specific) TODO: move to setROI */
	int shortFrameEnable;
	/** Frequency of Frames sent to GUI */
	uint32_t frameToGuiFrequency;
	/** Timer of Frames sent to GUI when frequency is 0 */
	uint32_t frameToGuiTimerinMS;
	/** Data Stream Enable from Receiver */
	bool dataStreamEnable;
	static const int DEFAULT_STREAMING_TIMER = 500;


	//***callback parameters***
	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is insignificant at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	int (*startAcquisitionCallBack)(char*, char*, uint64_t, uint32_t, void*);
	void *pStartAcquisition;

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void (*acquisitionFinishedCallBack)(uint64_t, void*);
	void *pAcquisitionFinished;


	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * frameNumber is the frame number
	 * expLength is the subframe number (32 bit eiger) or real time exposure time in 100ns (others)
	 * packetNumber is the packet number
	 * bunchId is the bunch id from beamline
	 * timestamp is the time stamp with 10 MHz clock
	 * modId is the unique module id (unique even for left, right, top, bottom)
	 * xCoord is the x coordinate in the complete detector system
	 * yCoord is the y coordinate in the complete detector system
	 * zCoord is the z coordinate in the complete detector system
	 * debug is for debugging purposes
	 * roundRNumber is the round robin set number
	 * detType is the detector type see :: detectorType
	 * version is the version number of this structure format
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes
	 */
	void (*rawDataReadyCallBack)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
			char*, uint32_t, void*);
	void *pRawDataReady;



private:

};
