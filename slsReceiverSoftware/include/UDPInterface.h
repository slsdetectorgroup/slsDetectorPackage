#pragma once
/***********************************************
 * @file UDPInterface.h
 * @short Base class with all the functions for the UDP inteface of the receiver
 ***********************************************/
/**
 * \mainpage Base class with all the functions for the UDP inteface of the receiver
 */

/**
 * @short Base class with all the functions for the UDP inteface of the receiver
 */

#include <exception>

#include "sls_receiver_defs.h"
#include "receiver_defs.h"
#include "utilities.h"
#include "logger.h"


class UDPInterface {
	

	/*  abstract class that defines the UDP interface of an sls detector data receiver.
	 *
	 *  Use the factory method UDPInterface::create() to get an instance:
	 *
	 *  UDPInterface *udp_interface = UDPInterface::create()
	 *
	 *
	 *  supported sequence of method-calls:
	 *
	 *  initialize() : once and only once after create() //FIXME: only once functionality implemented in the derived REST class, so not mention here?
	 *
	 *	get*()       : anytime after initialize(), multiples times
	 *
	 *  set*()       : anytime after initialize(), multiple times
	 *
	 *  startReceiver(): anytime after initialize(). Will fail in TCPIP itself if state already is 'running':
	 *
	 *  Only startReceiver() does change the data receiver configuration, it does pass the whole configuration cache to the data receiver.
	 *
	 *  abort(), //FIXME: needed?
	 *
	 *  stopReceiver() : anytime after initialize(). Will do nothing if state already is idle.
	 *  				 Otherwise, sets status to transmitting when shutting down sockets
	 *  				 then to run_finished when all data obtained
	 *  				 then to idle when returning from this function
	 *
	 *
	 *  getStatus() returns the actual state of the data receiver - idle, running or error, enum defined in include/sls_receiver_defs.h
	 *
	 *
	 *
	 *  get*() and set*() methods access the local cache of configuration values only and *do not* modify the data receiver settings.
	 *
	 *	set methods return nothing, use get methods to validate a set method success
	 *
	 *  get-methods that return a char array (char *) allocate a new array at each call. The caller is responsible to free the allocated space:
	 *
	 *      char *c = receiver->getFileName();
	 *       Or
	 *      FIXME: so that the pointers are not shared external to the class, do the following way in the calling method?
	 *      char *c = new char[MAX_STR_LENGTH];
	 *      strcpy(c,receiver->getFileName());
	 *      ....
	 *
	 *      delete[] c;
	 *
	 *  All pointers passed in externally will be allocated and freed by the calling function
	 *
	 *  OK and FAIL are defined in include/sls_receiver_defs.h for functions implementing behavior
	 *
	 */
	
 public:
	
	/*************************************************************************
	 * Constructor & Destructor **********************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/
	/**
	 * Constructor
	 * Only non virtual function implemented in this class
	 * Factory create method to create a standard or REST object
	 * @param [in] receiver_type type can be standard or REST
	 * @return a UDPInterface reference to object depending on receiver type
	 */
	static UDPInterface *create(string receiver_type = "standard");

	/**
	 * Destructor
	 */
	virtual ~UDPInterface() {};
	


	/*************************************************************************
	 * Getters ***************************************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/

	//**initial/detector parameters***

	/*
	 * Get multi detector size
	 * @return pointer to array of multi detector size in every dimension
	 */
	virtual int* getMultiDetectorSize() const  = 0;

	/*
	 * Get detector position id
	 * @return detector position id
	 */
	virtual int getDetectorPositionId() const  = 0;

	/*
	 * Get detector hostname
	 * @return hostname or NULL if uninitialized, must be released by calling function  (max of 1000 characters)
	 */
	virtual char *getDetectorHostname() const  = 0;

	/*
	 * Get flipped data across 'axis'
	 * @return if data is flipped across 'axis'
	 */
	virtual int getFlippedData(int axis=0) const = 0;


	//***file parameters***
	/**
	 * Get File Format
	 * @return file format
	 */
	virtual slsReceiverDefs::fileFormat getFileFormat() const = 0;

	/**
	 * Get File Name Prefix (without frame index, file index and extension (_d0_f000000000000_8.raw))
	 * @return NULL or pointer to file name prefix, must be released by calling function (max of 1000 characters)
	 */
	virtual char *getFileName() const = 0;

	/**
	 * Get File Path
	 * @return NULL or pointer to file path, must be released by calling function  (max of 1000 characters)
	 */
	virtual char *getFilePath() const = 0;

	/**
	 * Get File Index
	 * @return NULL or file index of acquisition
	 */
	virtual uint64_t getFileIndex() const = 0;

	/**
	 * Get Scan Tag
	 * @return scan tag //FIXME: needed? (unsigned integer?)
	 */
	virtual int getScanTag() const = 0;

	/**
	 * Get if Frame Index is enabled (acquisition of more than 1 frame adds '_f000000000000' to file name )
	 * @return true if frame index needed, else false
	 */
	virtual bool getFrameIndexEnable() const = 0;

	/**
	 * Get File Write Enable
	 * @return true if file write enabled, else false
	 */
	virtual bool getFileWriteEnable() const = 0;

	/**
	 * Get File Over Write Enable
	 * @return true if file over write enabled, else false
	 */
	virtual bool getOverwriteEnable() const = 0;

	/**
	 * Get data compression, by saving only hits (so far implemented only for Moench and Gotthard)
	 * @return true if data compression enabled, else false
	 */
	virtual bool getDataCompressionEnable() const = 0;


	//***acquisition count parameters***
	/**
	 * Get Total Frames Caught for an entire acquisition (including all scans)
	 * @return total number of frames caught for entire acquisition
	 */
	virtual uint64_t getTotalFramesCaught() const = 0;

	/**
	 * Get Frames Caught for each real time acquisition (eg. for each scan)
	 * @return number of frames caught for each scan
	 */
	virtual uint64_t getFramesCaught() const = 0;

	/**
	 * Get Current Frame Index for an entire  acquisition (including all scans)
	 * @return -1 if no frames have been caught, else  current frame index (represents all scans too) or -1 if no packets caught
	 */
	virtual int64_t getAcquisitionIndex() const = 0;


	//***connection parameters***
	/**
	 * Get UDP Port Number
	 * @return udp port number
	 */
	virtual uint32_t getUDPPortNumber() const = 0;

	/**
	 * Get Second UDP Port Number (eiger specific)
	 * @return second udp port number
	 */
	virtual uint32_t getUDPPortNumber2() const = 0;

	/**
	 * Get Ehernet Interface
	 * @return ethernet interface. eg. eth0 (max of 1000 characters)
	 */
	virtual char *getEthernetInterface() const = 0;


	//***acquisition parameters***
	/**
	 * Get Short Frame Enabled, later will be moved to getROI (so far only for gotthard)
	 * @return index of adc enabled, else -1 if all enabled
	 */
	virtual int getShortFrameEnable() const = 0;

	/**
	 * Get the Frequency of Frames Sent to GUI
	 * @return 0 for random frame requests, n for nth frame frequency
	 */
	virtual uint32_t getFrameToGuiFrequency() const = 0;

	/**
	 * Gets the timer between frames streamed when frequency is set to 0
	 * @return timer between frames streamed
	 */
	virtual uint32_t getFrameToGuiTimer() const = 0;


	/**
	 * Get the data stream enable
	 * @return data stream enable
	 */
	virtual bool getDataStreamEnable() const = 0;

	/**
	 * Get Acquisition Period
	 * @return acquisition period
	 */
	virtual uint64_t getAcquisitionPeriod() const = 0;

	/**
	 * Get Acquisition Time
	 * @return acquisition time
	 */
	virtual uint64_t getAcquisitionTime() const = 0;

	/*
	 * Get Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames FIXME: (Not implemented)
	 * @return number of frames expected
	 */
	virtual uint64_t getNumberOfFrames() const = 0;

	/**
	 * Get Dynamic Range or Number of Bits Per Pixel
	 * @return dynamic range that is 4, 8, 16 or 32
	 */
	virtual uint32_t getDynamicRange() const = 0;

	/**
	 * Get Ten Giga Enable
	 * @return true if 10Giga enabled, else false (1G enabled)
	 */
	virtual bool getTenGigaEnable() const = 0;

	/**
	 * Get Fifo Depth
	 * @return fifo depth
	 */
	virtual uint32_t getFifoDepth() const = 0;

	//***receiver status***
	/**
	 * Get Listening Status of Receiver
	 * @return can be idle, listening or error depending on if the receiver is listening or not
	 */
	virtual slsReceiverDefs::runStatus getStatus() const = 0;

	/**
	 * Get activate
	 * If deactivated, receiver will write dummy packets 0xFF
	 * (as it will receive nothing from detector)
	 * @return 0 for deactivated, 1 for activated
	 */
	virtual int getActivate() const = 0;


	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/

	//**initial parameters***
	/**
	 * Configure command line parameters
	 * @param config_map mapping of config parameters passed from command line arguments
	 */
	virtual void configure(map<string, string> config_map) = 0;

	/*
	 * Set multi detector size
	 * @param pointer to array of multi detector size in every dimension
	 */
	virtual void setMultiDetectorSize(const int* size) = 0;

	/*
	 * Get flipped data across 'axis'
	 * @return if data is flipped across 'axis'
	 */
	virtual void setFlippedData(int axis=0, int enable=-1) = 0;


	//***file parameters***
	/**
	 * Set File Format
	 * @param f fileformat binary or hdf5
	 */
	virtual void setFileFormat(slsReceiverDefs::fileFormat f) = 0;

	/**
	 * Set File Name Prefix (without frame index, file index and extension (_d0_f000000000000_8.raw))
	 * Does not check for file existence since it is created only at startReceiver
	 * @param c file name (max of 1000 characters)
	 */
	virtual void setFileName(const char c[]) = 0;

	/**
	 * Set File Path
	 * Checks for file directory existence before setting file path
	 * @param c file path (max of 1000 characters)
	 */
	virtual void setFilePath(const char c[]) = 0;

	/**
	 * Set File Index of acquisition
	 * @param i file index of acquisition
	 */
	virtual void setFileIndex(const uint64_t i) = 0;

	/**
	 * Set Scan Tag
	 * @param i scan tag //FIXME: needed? (unsigned integer?)
	 */
	virtual void setScanTag(const int i) = 0;

	/**
	 * Set Frame Index Enable (acquisition of more than 1 frame adds '_f000000000000' to file name )
	 * @param b true for frame index enable, else false
	 */
	virtual void setFrameIndexEnable(const bool b) = 0;

	/**
	 * Set File Write Enable
	 * @param b true for file write enable, else false
	 */
	virtual void setFileWriteEnable(const bool b) = 0;

	/**
	 * Set File Overwrite Enable
	 * @param b true for file overwrite enable, else false
	 */
	virtual void setOverwriteEnable(const bool b) = 0;

	/**
	 * Set data compression, by saving only hits (so far implemented only for Moench and Gotthard)
	 * @param b true for data compression enable, else false
	 * @return OK or FAIL
	 */
	virtual int setDataCompressionEnable(const bool b) = 0;

	//***connection parameters***
	/**
	 * Set UDP Port Number
	 * @param i udp port number
	 */
	virtual void setUDPPortNumber(const uint32_t i) = 0;

	/**
	 * Set Second UDP Port Number (eiger specific)
	 * @return second udp port number
	 */
	virtual void setUDPPortNumber2(const uint32_t i) = 0;

	/**
	 * Set Ethernet Interface to listen to
	 * @param c ethernet inerface eg. eth0 (max of 1000 characters)
	 */
	virtual void setEthernetInterface(const char* c) = 0;


	//***acquisition parameters***
	/**
	 * Set Short Frame Enabled, later will be moved to getROI (so far only for gotthard)
	 * @param i index of adc enabled, else -1 if all enabled
	 * @return OK or FAIL
	 */
	virtual int setShortFrameEnable(const int i) = 0;

	/**
	 * Set the Frequency of Frames Sent to GUI
	 * @param freq 0 for random frame requests, n for nth frame frequency
	 * @return OK or FAIL
	 */
	virtual int setFrameToGuiFrequency(const uint32_t freq) = 0;

	/**
	 * Sets the timer between frames streamed when frequency is set to 0
	 * @param time_in_ms timer between frames streamed
	 */
	virtual void setFrameToGuiTimer(const uint32_t time_in_ms) = 0;

	/**
	 * Set the data stream enable
	 * @param enable data stream enable
	 * @return OK or FAIL
	 */
	virtual int setDataStreamEnable(const bool enable) = 0;

	/**
	 * Set Acquisition Period
	 * @param i acquisition period
	 * @return OK or FAIL
	 */
	virtual int setAcquisitionPeriod(const uint64_t i) = 0;

	/**
	 * Set Acquisition Time
	 * @param i acquisition time
	 * @return OK or FAIL
	 */
	virtual int setAcquisitionTime(const uint64_t i) = 0;

	/**
	 * Set Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames FIXME: (Not implemented)
	 * @param i number of frames expected
	 * @return OK or FAIL
	 */
	virtual int setNumberOfFrames(const uint64_t i) = 0;

	/**
	 * Set Dynamic Range or Number of Bits Per Pixel
	 * @param i dynamic range that is 4, 8, 16 or 32
	 * @return OK or FAIL
	 */
	virtual int setDynamicRange(const uint32_t i) = 0;

	/**
	 * Set Ten Giga Enable
	 * @param b true if 10Giga enabled, else false (1G enabled)
	 * @return OK or FAIL
	 */
	virtual int setTenGigaEnable(const bool b) = 0;

	/**
	 * Set Fifo Depth
	 * @param i fifo depth value
	 * @return OK or FAIL
	 */
	virtual int setFifoDepth(const uint32_t i) = 0;


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
	virtual int setDetectorType(const slsReceiverDefs::detectorType d) = 0;

	/**
	 * Set detector position id
	 * @param i position id
	 */
	virtual void setDetectorPositionId(const int i) = 0;

	/**
	 * Sets detector hostname (and corresponding detector variables in derived REST class)
	 * It is second function called by the client when connecting to receiver.
	 * you can call this function only once. //FIXME: is this still valid, this implemented in derived REST class?
	 * @param c detector hostname
	 */
	virtual void initialize(const char *c) = 0;


	//***acquisition functions***
	/**
	 * Reset acquisition parameters such as total frames caught for an entire acquisition (including all scans)
	 */
	virtual void resetAcquisitionCount() = 0;

	/**
	 * Start Listening for Packets by activating all configuration settings to receiver
	 * @param c error message if FAIL
	 * @return OK or FAIL
	 */
	virtual int startReceiver(char *c=NULL) = 0;

	/**
	 * Stop Listening for Packets
	 * Calls startReadout(), which stops listening and sets status to Transmitting
	 * When it has read every frame in buffer,it returns with the status Run_Finished
	 */
	virtual void stopReceiver() = 0;

	/**
	 * Stop Listening to Packets
	 * and sets status to Transmitting
	 */
	virtual void startReadout() = 0;

	/**
	 * Shuts down and deletes UDP Sockets
	 */
	virtual void shutDownUDPSockets() = 0;

	/**
	 * Get the buffer-current frame read by receiver
	 * @param ithread port thread index
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param startAcq start index of the acquisition
	 * @param startFrame start index of the scan
	 */
	virtual void readFrame(int ithread, char* c,char** raw, int64_t &startAcq, int64_t &startFrame)=0;

	/**
	 * abort acquisition with minimum damage: close open files, cleanup.
	 * does nothing if state already is 'idle'
	 */
	virtual void abort() = 0;  //FIXME: needed, isnt stopReceiver enough?

	/**
	 * Closes all files
	 */
	virtual void closeFiles() = 0;

	/**
	 * Activate / Deactivate Receiver
	 * If deactivated, receiver will write dummy packets 0xFF
	 * (as it will receive nothing from detector)
	 */
	virtual int setActivate(int enable = -1) = 0;


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
	virtual void registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg) = 0;

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	virtual void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg) = 0;

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
	virtual void registerCallBackRawDataReady(void (*func)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
			char*, uint32_t, void*),void *arg) = 0;


 protected:
 private:
	
};
