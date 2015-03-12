#ifndef UDPINTERFACE_H
#define UDPINTERFACE_H

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
#include "MySocketTCP.h"

#include "utilities.h"
#include "logger.h"


class UDPInterface {
	

	/* abstract class that defines the UDP interface of an sls detector data receiver.
	 *
	 * Use the factory method UDPInterface::create() to get an instance:
	 *
	 *      UDPInterface *udp_interface = UDPInterface::create()
	 *
	 *  supported sequence of method-calls:
	 *
	 *  initialize() : once and only once after create()
	 *
	 *  get*()       : anytime after initialize(), multiples times
	 *  set*()       : anytime after initialize(), multiple times
	 *
	 *  startReceiver(): anytime after initialize(). Will fail if state already is 'running'
	 *
	 *  abort(),
	 *  stopReceiver() : anytime after initialize(). Will do nothing if state already is idle.
	 *
	 *  getStatus() returns the actual state of the data receiver - running or idle. All other
	 *  get*() and set*() methods access the local cache of configuration values only and *do not* modify the data receiver settings.
	 *
	 *  Only startReceiver() does change the data receiver configuration, it does pass the whole configuration cache to the data receiver.
	 *
	 *  get- and set-methods that return a char array (char *) allocate a new array at each call. The caller is responsible to free the allocated space:
	 *
	 *      char *c = receiver->getFileName();
	 *          ....
	 *      delete[] c;
	 *
	 *  always: 1:YES       0:NO      for int as bool-like arguments
	 *
	 */
	
 public:
	
	/**
	 * Destructor
	 */
	virtual ~UDPInterface() {};
	
	/**
	 * Factory create method
	 */
	static UDPInterface *create(string receiver_type = "standard");

	virtual void configure(map<string, string> config_map) = 0;

	
 public:
	
	/**
	 * Initialize the Receiver
	 @param detectorHostName detector hostname
	 * you can call this function only once. You must call it before you call startReceiver() for  the first time.
	 */
	virtual void initialize(const char *detectorHostName) = 0;
	

	 /* Returns detector hostname
	    /returns hostname
	    * caller needs to deallocate the returned char array.
	    * if uninitialized, it must return NULL
	    */
 	virtual char *getDetectorHostname() const  = 0;

	/**
	 * Returns status of receiver: idle, running or error
	 */
	virtual slsReceiverDefs::runStatus getStatus() const = 0;

	/**
	 * Returns File Name
	 * caller is responsible to deallocate the returned char array.
	 */
	virtual char *getFileName() const = 0;


	/**
	 * Returns File Path
	 * caller is responsible to deallocate the returned char array
	 */
	virtual char *getFilePath() const = 0; //FIXME: Does the caller need to free() the returned pointer?


	/**
	 * Returns the number of bits per pixel
	 */
	virtual int getDynamicRange() const = 0;

	/**
	 * Returns scan tag
	 */
	virtual int getScanTag() const = 0;

	/*
	 * Returns number of frames to receive
	 * This is the number of frames to expect to receiver from the detector.
	 * The data receiver will change from running to idle when it got this number of frames
	 */
	virtual int getNumberOfFrames() const = 0;

	/**
	* Returns file write enable
	* 1: YES 0: NO
	*/
	virtual int getEnableFileWrite() const = 0;

	/**
	* Returns file over write enable
	* 1: YES 0: NO
	*/
	virtual int getEnableOverwrite() const = 0;

	/**
	 * Set File Name (without frame index, file index and extension)
	 @param c file name
	 /returns file name
	  * returns NULL on failure (like bad file name)
	  * does not check the existence of the file - we don't know which path we'll finally use, so no point to check.
	  * caller is responsible to deallocate the returned char array.
	 */
	virtual char* setFileName(const char c[]) = 0;

	/**
	 * Set File Path
	 @param c file path
	 /returns file path
	  * checks the existence of the directory. returns NULL if directory does not exist or is not readable.
	  * caller is responsible to deallocate the returned char array.
	 */
	virtual char* setFilePath(const char c[]) = 0;

	/**
	 * Returns the number of bits per pixel
	 @param dr sets dynamic range
	 /returns dynamic range
	  * returns -1 on failure
	  * FIXME: what are the allowd values - should we use an enum as argument?
	 */
	virtual int setDynamicRange(const int dr) = 0;


	/**
	 * Set scan tag
	 @param tag scan tag
	 /returns scan tag (always non-negative)
	  * FIXME: valid range - only positive? 16bit ore 32bit?
	  * returns -1 on failure
	 */
	virtual int setScanTag(const int tag) = 0;

	/**
	 * Sets number of frames
	 @param fnum number of frames
	 /returns number of frames
	 */
	virtual int setNumberOfFrames(const int fnum) = 0;

	/**
	 * Set enable file write
	 * @param i file write enable
	 /returns file write enable
	 */
	virtual int setEnableFileWrite(const int i) = 0;

	/**
	 * Set enable file overwrite
	 * @param i file overwrite enable
	 /returns file overwrite enable
	 */
	virtual int setEnableOverwrite(const int i) = 0;

	/**
	 * Starts Receiver - activate all configuration settings to the eiger receiver and start to listen for packets
	 @param message is the error message if there is an error
	 /returns 0 on success or -1 on failure
	 */
	//FIXME: success == 0 or success == 1?
	virtual int startReceiver(char *message=NULL) = 0; //FIXME: who allocates message[]?

	/**
	 * Stops Receiver - stops listening for packets
	 /returns success
	  * same as abort(). Always returns 0.
	 */
	virtual int stopReceiver() = 0;

	/**
	 * abort acquisition with minimum damage: close open files, cleanup.
	 * does nothing if state already is 'idle'
	 */
	virtual void abort() = 0;



/*******************************************************************************************************************
 ****************************************  Added by Dhanya *********************************************************
 *******************************************************************************************************************/

	/**
	 * Returns File Index
	 */
	virtual int getFileIndex() = 0;

	/**
	 * Returns Total Frames Caught for an entire acquisition (including all scans)
	 */
	virtual int getTotalFramesCaught() = 0;

	/**
	 * Returns Frames Caught for each real time acquisition (eg. for each scan)
	 */
	virtual int getFramesCaught() = 0;

	/**
	 * Returns the frame index at start of entire acquisition (including all scans)
	 */
	virtual uint32_t getStartAcquisitionIndex()=0;

	/**
	 * Returns current Frame Index Caught for an entire  acquisition (including all scans)
	 */
	virtual uint32_t getAcquisitionIndex() = 0;

	/**
	 * Returns the frame index at start of each real time acquisition (eg. for each scan)
	 */
	virtual uint32_t getStartFrameIndex() = 0;

	/** get data compression, by saving only hits
	 */
	virtual bool getDataCompression() = 0;

	/**
	 * Set receiver type
	 * @param det detector type
	 * Returns success or FAIL
	 */
	virtual int setDetectorType(slsReceiverDefs::detectorType det) = 0;

	/**
	 * Set File Index
	 * @param i file index
	 */
	virtual int setFileIndex(int i) = 0;

	/** set acquisition period if a positive number
	 */
	virtual int64_t setAcquisitionPeriod(int64_t index) = 0;

	/**
	 * Set Frame Index Needed
	 * @param i frame index needed
	 */
	virtual int setFrameIndexNeeded(int i) = 0;

	/**
	 * Set UDP Port Number
	 */
	virtual void setUDPPortNo(int p) = 0;

	/**
	 * Set UDP Port Number
	 */
	virtual void setUDPPortNo2(int p) = 0;

	/**
	 * Set Ethernet Interface or IP to listen to
	 */
	virtual void setEthernetInterface(char* c) = 0;

	/**
	 * Set short frame
	 * @param i if shortframe i=1
	 */
	virtual int setShortFrame(int i) = 0;

	/**
	 * Set the variable to send every nth frame to gui
	 * or if 0,send frame only upon gui request
	 */
	virtual int setNFrameToGui(int i) = 0;

	/**
	 * Resets the Total Frames Caught
	 * This is how the receiver differentiates between entire acquisitions
	 * Returns 0
	 */
	virtual void resetTotalFramesCaught() = 0;

	/** enabl data compression, by saving only hits
	 /returns if failed
	 */
	virtual int enableDataCompression(bool enable) = 0;

	/**
	 * enable 10Gbe
	 @param enable 1 for 10Gbe or 0 for 1 Gbe, -1 to read out
	 \returns enable for 10Gbe
	 */
	virtual int enableTenGiga(int enable = -1) = 0;

	/**
	 * Returns the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param fnum frame number for eiger as it is not in the packet
	 * @param startAcquisitionIndex is the start index of the acquisition
	 * @param startFrameIndex is the start index of the scan
	 */
	virtual void readFrame(char* c,char** raw, uint32_t &fnum, uint32_t &startAcquisitionIndex, uint32_t &startFrameIndex)=0;

	/** set status to transmitting and
	 * when fifo is empty later, sets status to run_finished
	 */
	virtual void startReadout() = 0;

	/**
	 * shuts down the  udp sockets
	 * \returns if success or fail
	 */
	virtual int shutDownUDPSockets() = 0;

	/**
	 * Closes all files
	 * @param ithr thread index, -1 for all threads
	 */
	virtual void closeFile(int ithr = -1) = 0;

	/**
	 * Call back for start acquisition
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   datasize

	   return value is
	   0 callback takes care of open,close,wrie file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything
	*/
	virtual void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg) = 0;

	/**
	 * Call back for acquisition finished
	   callback argument is
	   total frames caught
	*/
	virtual void registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg) = 0;

	/**
	 * Call back for raw data
	  args to raw data ready callback are
	  framenum
	  datapointer
	  datasize in bytes
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	virtual void registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg) = 0;
	
 protected:
	
 private:
	
};

#endif  /* #ifndef UDPINTERFACE_H */
