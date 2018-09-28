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

	/**
	 * Get Gap Pixels Enable (eiger specific)
	 * @return true if gap pixels enabled, else false
	 */
	bool getGapPixelsEnable() const;


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
	 * Get Frames per File (0 means infinite)
	 * @return Frames per File
	 */
	uint32_t getFramesPerFile() const;

	/**
	 * Get Frame Discard Policy
	 * @return Frame Discard Policy
	 */
	frameDiscardPolicy getFrameDiscardPolicy() const;

	/**
	 * Get Partial Frame Padding Enable
	 * @return Partial Frame Padding Enable
	 */
	bool getFramePaddingEnable() const;

	/**
	 * Get Scan Tag
	 * @return scan tag //FIXME: needed? (unsigned integer?)
	 */
	int getScanTag() const;

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
	 * Get ROI
	 * @return index of adc enabled, else -1 if all enabled
	 */
	std::vector<ROI> getROI() const;

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

	/**
	 * Get Sub Exposure Time
	 * @return Sub Exposure Time
	 */
	uint64_t getSubExpTime() const;

	/**
	 * Get Sub Period
	 * @return Sub Period
	 */
	uint64_t getSubPeriod() const;

	/*
	 * Get Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames FIXME: (Not implemented)
	 * @return number of frames expected
	 */
	uint64_t getNumberOfFrames() const;

	/*
	 * Get Number of Samples expected by receiver from detector (for chip test board only)
	 * @return number of samples expected
	 */
	uint64_t getNumberofSamples() const;

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
	 * Get Silent Mode
	 * @return silent mode
	 */
	bool getSilentMode() const;

	/**
	 * Get activate
	 * If deactivated, receiver will create dummy data if deactivated padding is enabled
	 * (as it will receive nothing from detector)
	 * @return false for deactivated, true for activated
	 */
	bool getActivate() const;

	/**
	 * Get deactivated padding enable
	 * If enabled, receiver will create dummy packets (0xFF), else it will create nothing
	 * (as it will receive nothing from detector)
	 * @return 0 for disabled, 1 for enabled
	 */
	bool getDeactivatedPadding() const;

	/**
	 * Get Streaming Port
	 * @return streaming port
	 */
	uint32_t getStreamingPort() const;

	/**
	 * Get streaming source ip
	 * @return streaming source ip
	 */
	char *getStreamingSourceIP() const;

    /**
     * Get additional json header
     * @return additional json header
     */
    char *getAdditionalJsonHeader() const;

    /** (not saved in client shared memory)
     * Get UDP Socket Buffer Size
     * @return UDP Socket Buffer Size
     */
    uint32_t getUDPSocketBufferSize() const;


    /** (not saved in client shared memory)
     * Get actual UDP Socket Buffer Size
     * @return actual UDP Socket Buffer Size
     */
    uint32_t getActualUDPSocketBufferSize() const;

	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/

	//**initial parameters***
	/**
	 * Configure command line parameters
	 * @param config_map mapping of config parameters passed from command line arguments
	 */
	void configure(std::map<std::string, std::string> config_map);

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

	/**
	 * Set Gap Pixels Enable (eiger specific)
	 * @param b true for gap pixels enable, else false
	 * @return OK or FAIL
	 */
	int setGapPixelsEnable(const bool b);


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
	 * Set Frames per File (0 means infinite)
	 * @param i Frames per File
	 */
	void setFramesPerFile(const uint32_t i);

	/**
	 * Set Frame Discard Policy
	 * @param i Frame Discard Policy
	 */
	void setFrameDiscardPolicy(const frameDiscardPolicy i);

	/**
	 * Set Partial Frame Padding Enable
	 * @param i Partial Frame Padding Enable
	 */
	void setFramePaddingEnable(const bool i);

	/**
	 * Set Scan Tag
	 * @param i scan tag //FIXME: needed? (unsigned integer?)
	 */
	void setScanTag(const int i);

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
	 * Set ROI
	 * @param i ROI
	 * @return OK or FAIL
	 */
	int setROI(const std::vector<ROI> i);

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
	 * Set Sub Exposure Time
	 * @param i Sub Exposure Time
	 * @return OK or FAIL
	 */
	void setSubExpTime(const uint64_t i);

	/**
	 * Set Sub Period
	 * @param i Period
	 * @return OK or FAIL
	 */
	void setSubPeriod(const uint64_t i);

	/**
	 * Set Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames
	 * @param i number of frames expected
	 */
	int setNumberOfFrames(const uint64_t i);

	/**
	 * Set Number of Samples expected by receiver from detector
	 * @param i number of Samples expected
	 * @return OK or FAIL
	 */
	int setNumberofSamples(const uint64_t i);

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

	//***receiver parameters***
	/**
	 * Set Silent Mode
	 * @param i silent mode. true sets, false unsets
	 */
	void setSilentMode(const bool i);

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
	 * Sets detector hostname
	 * It is second function called by the client when connecting to receiver.
	 * you can call this function only once.
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
	 * abort acquisition with minimum damage: close open files, cleanup.
	 * does nothing if state already is 'idle'
	 */
	void abort();  //FIXME: needed, isn't stopReceiver enough?

	/**
	 * Activate / Deactivate Receiver
	 * If deactivated, receiver will create dummy data if deactivated padding is enabled
	 * (as it will receive nothing from detector)
	 * @param enable enable
	 * @return false for disabled, true for enabled
	 */
	bool setActivate(const bool enable);

	/**
	 * Set deactivated padding enable
	 * If enabled, receiver will create dummy packets (0xFF), else it will create nothing
	 * (as it will receive nothing from detector)
	 * @param enable enable
	 * @return false for disabled, true for enabled
	 */
	bool setDeactivatedPadding(const bool enable);

	/**
	 * Set streaming port
	 * @param i streaming port
	 */
	void setStreamingPort(const uint32_t i);

	/**
	 * Set streaming source ip
	 * @param c streaming source ip
	 */
	void setStreamingSourceIP(const char* c);

    /**
     * Set additional json header
     */
    void setAdditionalJsonHeader(const char* c);

    /** (not saved in client shared memory)
     * Set UDP Socket Buffer Size
     * @param s UDP Socket Buffer Size
     * @return OK or FAIL if dummy socket could be created
     */
    int setUDPSocketBufferSize(const uint32_t s);

	/*
	 * Restream stop dummy packet from receiver
	 * @return OK or FAIL
	 */
	int restreamStop();

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
	 * sls_receiver_header frame metadata
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes.
	 */
	void registerCallBackRawDataReady(void (*func)(char* ,
			char*, uint32_t, void*),void *arg);

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes.
     * Can be modified to the new size to be written/streamed. (only smaller value).
     */
    void registerCallBackRawDataModifyReady(void (*func)(char* ,
            char*, uint32_t &,void*),void *arg);


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
	/** Sub Exposure Time */
	uint64_t subExpTime;
	/** Sub Period */
	uint64_t subPeriod;
	/** Frame Number */
	uint64_t numberOfFrames;
	/** Samples Number */
	uint64_t numberOfSamples;
	/** Dynamic Range */
	uint32_t dynamicRange;
	/** Ten Giga Enable*/
	bool tengigaEnable;
	/** Fifo Depth */
	uint32_t fifoDepth;
	/** enable for flipping data across both axes */
	int flippedData[2];
	/** gap pixels enable */
	bool gapPixelsEnable;

	//***receiver parameters***
	/** Maximum Number of Listening Threads/ UDP Ports */
	const static int MAX_NUMBER_OF_LISTENING_THREADS = 2;
	/** Receiver Status */
	runStatus status;
	/** Activated/Deactivated */
	bool activated;
	/** Deactivated padding enable */
	bool deactivatedPaddingEnable;
	/** frame discard policy */
	frameDiscardPolicy frameDiscardMode;
	/** frame padding */
	bool framePadding;

	//***connection parameters***
	/** Ethernet Interface */
	char eth[MAX_STR_LENGTH];
	/** Server UDP Port Number*/
	uint32_t udpPortNum[MAX_NUMBER_OF_LISTENING_THREADS];
	/** udp socket buffer size */
	uint32_t udpSocketBufferSize;
    /** actual UDP Socket Buffer Size (halved due to kernel bookkeeping) */
    uint32_t actualUDPSocketBufferSize;

	//***file parameters***
	/** File format */
	fileFormat fileFormatType;
	/** File Name without frame index, file index and extension (_d0_f000000000000_8.raw)*/
	char fileName[MAX_STR_LENGTH];
	/** File Path */
	char filePath[MAX_STR_LENGTH];
	/** File Index */
	uint64_t fileIndex;
	/** Frames per file  (0 means infinite) */
	uint32_t framesPerFile;
	/** Scan Tag */
	int scanTag;
	/** File Write enable */
	bool fileWriteEnable;
	/** Overwrite enable */
	bool overwriteEnable;
	/** Data Compression Enable - save only hits */
	bool dataCompressionEnable;

	//***acquisition parameters***
	/* ROI */
	std::vector<ROI> roi;
	/** Frequency of Frames sent to GUI */
	uint32_t frameToGuiFrequency;
	/** Timer of Frames sent to GUI when frequency is 0 */
	uint32_t frameToGuiTimerinMS;
	/** Data Stream Enable from Receiver */
	bool dataStreamEnable;
	/** streaming port */
	uint32_t streamingPort;
	/** streaming port */
	char streamingSrcIP[MAX_STR_LENGTH];
	/** additional json header */
	char additionalJsonHeader[MAX_STR_LENGTH];

	//***receiver parameters***
	bool silentMode;



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
	 * sls_receiver_header frame metadata
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes.
	 */
	void (*rawDataReadyCallBack)(char* ,
			char*, uint32_t, void*);

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes. Can be modified to the new size to be written/streamed. (only smaller value).
     */
    void (*rawDataModifyReadyCallBack)(char* ,
            char*, uint32_t &, void*);

	void *pRawDataReady;



private:

};
