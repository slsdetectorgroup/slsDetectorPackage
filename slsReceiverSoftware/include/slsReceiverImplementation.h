#pragma once
/********************************************//**
 * @file slsReceiverImplementation.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/
/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */
// #include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "logger.h"
#include "container_utils.h"
class GeneralData;
class Listener;
class DataProcessor;
class DataStreamer;
class Fifo;
class slsDetectorDefs;

#include <exception>
#include <vector>
#include <memory>

class slsReceiverImplementation: private virtual slsDetectorDefs {
 public:


	//*** cosntructor & destructor ***
	/**
	 * Constructor
	 */
	slsReceiverImplementation();

	/**
	 * Destructor
	 */
	virtual ~slsReceiverImplementation();


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
	 * @return hostname  (max of 1000 characters)
	 */
	std::string getDetectorHostname() const;

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

	/**
	 * Get readout flags (Eiger, chiptestboard, moench)
	 * @return readout flags
	 */
	readOutFlags getReadOutFlags() const;


	//***file parameters***
	/**
	 * Get File Format
	 * @return file format
	 */
	fileFormat getFileFormat() const;
	/**
	 * Get File Name Prefix (without frame index, file index and extension (_d0_f000000000000_8.raw))
	 * @return file name prefix
	 */
	std::string getFileName() const;

	/**
	 * Get File Path
	 * @return file path
	 */
	std::string getFilePath() const;

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
	 * Get File Write Enable
	 * @return true if file write enabled, else false
	 */
	bool getFileWriteEnable() const;

	/**
	 * Get Master File Write Enable
	 * @return true if Master file write enabled, else false
	 */
	bool getMasterFileWriteEnable() const;

	/**
	 * Get File Over Write Enable
	 * @return true if file over write enabled, else false
	 */
	bool getOverwriteEnable() const;


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
	 * Get Second UDP Port Number (eiger/jungfrau specific)
	 * @return second udp port number
	 */
	uint32_t getUDPPortNumber2() const;

	/**
	 * Get Ehernet Interface
	 * @return ethernet interface. eg. eth0 or "" if listening to all
	 */
	std::string getEthernetInterface() const;

	/**
	 * Get Ehernet Interface 2 (jungfrau specific)
	 * @return ethernet interface 2. eg. eth0 or "" if listening to all
	 */
	std::string getEthernetInterface2() const;

	/**
	 * Get number of UDP Interfaces (jungfrau specific)
	 * @return number of udp interfaces. Options (1-2)
	 */
	int getNumberofUDPInterfaces() const;


	//***acquisition parameters***
	/**
	 * Get ROI
	 * @return index of adc enabled, else -1 if all enabled
	 */
	std::vector<ROI> getROI() const;

	/**
	 * Get ADC Enable Mask
	 * @return ADC Enable Mask
	 */
	uint32_t getADCEnableMask() const;

	/**
	 * Get the streaming frequency
	 * @return 0 for timer, n for nth frame frequency
	 */
	uint32_t getStreamingFrequency() const;

	/**
	 * Gets the timer between frames streamed when frequency is set to 0
	 * @return timer between frames streamed
	 */
	uint32_t getStreamingTimer() const;

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
	std::string getStreamingSourceIP() const;

    /**
     * Get additional json header
     * @return additional json header
     */
	std::string getAdditionalJsonHeader() const;

    /** (not saved in client shared memory)
     * Get UDP Socket Buffer Size
     * @return UDP Socket Buffer Size
     */
    int64_t getUDPSocketBufferSize() const;


    /** (not saved in client shared memory)
     * Get actual UDP Socket Buffer Size
     * @return actual UDP Socket Buffer Size
     */
    int64_t getActualUDPSocketBufferSize() const;

	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/

	//**initial parameters***
	/**
	 * Sets detector hostname
	 * @param c detector hostname
	 */
	void setDetectorHostname(const char *c);

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

	/**
	 * Set readout flags (eiger, chiptestboard, moench)
	 * @param f readout flag
	 * @return OK or FAIL
	 */
	int setReadOutFlags(const readOutFlags f);


	//***file parameters***
	/**
	 * Set File Format
	 * @param f fileformat binary or hdf5
	 */
	void setFileFormat(slsDetectorDefs::fileFormat f);

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
	 * Set File Write Enable
	 * @param b true for file write enable, else false
	 */
	void setFileWriteEnable(const bool b);

		/**
	 * Set Master File Write Enable
	 * @param b true for Master file write enable, else false
	 */
	void setMasterFileWriteEnable(const bool b);

	/**
	 * Set File Overwrite Enable
	 * @param b true for file overwrite enable, else false
	 */
	void setOverwriteEnable(const bool b);

	//***connection parameters***
	/**
	 * Set UDP Port Number
	 * @param i udp port number
	 */
	void setUDPPortNumber(const uint32_t i);

	/**
	 * Set Second UDP Port Number (eiger/jungfrau specific)
	 * @return second udp port number
	 */
	void setUDPPortNumber2(const uint32_t i);

	/**
	 * Set Ethernet Interface to listen to
	 * @param c ethernet inerface eg. eth0 (max of 1000 characters)
	 */
	void setEthernetInterface(const char* c);

	/**
	 * Set second Ethernet Interface to listen to (jungfrau specific)
	 * @param c second ethernet inerface eg. eth0 (max of 1000 characters)
	 */
	void setEthernetInterface2(const char* c);

	/**
	 * Set number of UDP Interfaces (jungfrau specific)
	 * @param n number of udp interfaces. Options (1-2)
	 * @return OK or FAIL for fifo structure creation
	 */
	int setNumberofUDPInterfaces(const int n);

    /** (not saved in client shared memory)
     * Set UDP Socket Buffer Size
     * @param s UDP Socket Buffer Size
     * @return OK or FAIL if dummy socket could be created
     */
    int setUDPSocketBufferSize(const int64_t s);


	//***acquisition parameters***
	/**
	 * Set ROI
	 * @param i ROI
	 * @return OK or FAIL
	 */
    int setROI(const std::vector<ROI> new_roi);

	/**
	 * Set ADC Enable Mask
	 * @param mask ADC Enable Mask
	 * @return OK or FAIL
	 */
    int setADCEnableMask(const uint32_t mask);

    /**
     * Set the streaming frequency
     * @param freq 0 for timer, n for nth frame frequency
     * @return OK or FAIL
     */
    int setStreamingFrequency(const uint32_t freq);

    /**
     * Sets the timer between frames streamed when frequency is set to 0
     * @param time_in_ms timer between frames streamed
     */
    void setStreamingTimer(const uint32_t time_in_ms);
    /**
     * Set the data stream enable
     * @param enable data stream enable
     * @return OK or FAIL
     */
    int setDataStreamEnable(const bool enable);

    /**
     * Set streaming port
     * @param i streaming port
     */
    void setStreamingPort(const uint32_t i);

    /**
     * Set streaming source ip
     * @param c streaming source ip
     */
    void setStreamingSourceIP(const char *c);

    /**
     * Set additional json header
     */
    void setAdditionalJsonHeader(const char* c);

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
	 * @param b true if 10GbE enabled, else false (1G enabled)
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
	 * Set detector position id and construct filewriter
	 * @param id position id
	 */
	void setDetectorPositionId(const int id);

	//***acquisition functions***
	/**
	 * Reset acquisition parameters such as total frames caught for an entire acquisition (including all scans)
	 */
	void resetAcquisitionCount();

	/**
	 * Start Listening for Packets by activating all configuration settings to receiver
	 * When this function returns, it has status RUNNING(upon SUCCESS) or IDLE (upon failure)
	 * @param c error message if FAIL
	 * @return OK or FAIL
	 */
	int startReceiver(char *c=NULL);

	/**
	 * Stop Listening for Packets
	 * Calls startReadout(), which stops listening and sets status to Transmitting
	 * When it has read every frame in buffer, the status changes to Run_Finished
	 * When this function returns, receiver has status IDLE
	 * Pre: status is running, semaphores have been instantiated,
	 * Post: udp sockets shut down, status is idle, semaphores destroyed
	 */
	void stopReceiver();

	/**
	 * Stop Listening to Packets
	 * and sets status to Transmitting
	 * Next step would be to get all data and stop receiver completely and return with idle state
	 * Pre: status is running, udp sockets have been initialized, stop receiver initiated
	 * Post:udp sockets closed, status is transmitting
	 */
	void startReadout();

	/**
	 * Shuts down and deletes UDP Sockets
	 * also called in case of illegal shutdown of receiver
	 */
	void shutDownUDPSockets();

	/**
	 * Closes file / all files(data compression involves multiple files)
	 */
	void closeFiles();

	/**
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

private:

    /**
	 * Delete and free member parameters
	 */
    void DeleteMembers();

	/**
	 * Initialize member parameters
	 */
	void InitializeMembers();

	/**
	 * Sets local network parameters, but requires permissions
	 */
	void SetLocalNetworkParameters();

	/**
	 * Set Thread Priorities
	 */
	void SetThreadPriorities();

	/**
	 * Set up the Fifo Structure for processing buffers
	 * between listening and dataprocessor threads
	 * @return OK or FAIL
	 */
	int SetupFifoStructure();

	/**
	 * Reset parameters for new measurement (eg. for each scan)
	 */
	void ResetParametersforNewMeasurement();

	/**
	 * Creates UDP Sockets
	 * @return OK or FAIL
	 */
	int CreateUDPSockets();

	/**
	 * Creates the first file
	 * also does the startAcquisitionCallBack
	 * @return OK or FAIL
	 */
	int SetupWriter();

	/**
	 * Start Running
	 * Set running mask and post semaphore of the threads
	 * to start the inner loop in execution thread
	 */
	void StartRunning();



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
	/** readout flags*/
	readOutFlags readoutFlags;

	//*** receiver parameters ***
	/** Number of Threads */
	int numThreads;
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
	/** silent mode */
	bool silentMode;

	//***connection parameters***
	/** Number of UDP Interfaces */
	int numUDPInterfaces;
	/** Ethernet Interface */
	char eth[MAX_NUMBER_OF_LISTENING_THREADS][MAX_STR_LENGTH];
	/** Server UDP Port Number*/
	uint32_t udpPortNum[MAX_NUMBER_OF_LISTENING_THREADS];
	/** udp socket buffer size */
	int64_t udpSocketBufferSize;
    /** actual UDP Socket Buffer Size (halved due to kernel bookkeeping) */
    int64_t actualUDPSocketBufferSize;

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
	/** File Write enable */
	bool fileWriteEnable;
	/** MasterFile Write enable */
	bool masterFileWriteEnable;	
	/** Overwrite enable */
	bool overwriteEnable;

	//***acquisition parameters***
	/* ROI */
	std::vector<ROI> roi;
	/** ADC Enable Mask */
	uint32_t adcEnableMask;
	/** streaming frequency */
	uint32_t streamingFrequency;
	/** Streaming timer when frequency is 0 */
	uint32_t streamingTimerInMs;
	/** Data Stream Enable from Receiver */
	bool dataStreamEnable;
	/** streaming port */
	uint32_t streamingPort;
	/** streaming port */
	char streamingSrcIP[MAX_STR_LENGTH];
	/** additional json header */
	char additionalJsonHeader[MAX_STR_LENGTH];

	//** class objects ***
	/** General Data Properties */
	GeneralData* generalData;
	/** Listener Objects that listen to UDP and push into fifo */
	std::vector<std::unique_ptr<Listener>> listener;
	/** DataProcessor Objects that pull from fifo and process data */
	std::vector<std::unique_ptr<DataProcessor>> dataProcessor;
	/** DataStreamer Objects that stream data via ZMQ */
	std::vector<std::unique_ptr<DataStreamer>> dataStreamer;
	/** Fifo Structure to store addresses of memory writes */
	std::vector<std::unique_ptr<Fifo>> fifo;

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


};

