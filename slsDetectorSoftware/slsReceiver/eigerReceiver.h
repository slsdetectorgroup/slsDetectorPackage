#ifndef EIGERRECEIVER_H
#define EIGERRECEIVER_H
/***********************************************
 * @file eigerReceiver.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/

/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */

#include "sls_detector_defs.h"
#include "RestHelper.h"

class EigerReceiver {
	/* abstract class that defines the public interface of an eiger data receiver.
	 *
	 * Use the factory method EigerReceiver::create() to get an instance:
	 *
	 *      EigerReceiver *receiver = EigerReceiver::create()
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
	 * factory method to create instances
	 */
	static EigerReceiver *create();

	/**
	 * Destructor
	 */
	virtual ~EigerReceiver() {};

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
    virtual slsDetectorDefs::runStatus getStatus() const = 0;

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
	virtual int getEnableFileWrite() const  = 0;

	/**
	* Returns file over write enable
	* 1: YES 0: NO
	*/
	virtual int getEnableOverwrite() const  = 0;

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
	virtual int startReceiver(char message[]) = 0; //FIXME: who allocates message[]?

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

protected:

private:
	
};

#endif  /* #ifndef EIGERRECEIVER_H */
