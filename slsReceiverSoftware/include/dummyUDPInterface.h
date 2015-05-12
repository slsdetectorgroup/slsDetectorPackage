#ifndef DUMMYUDPINTERFACE_H
#define DUMMYUDPINTERFACE_H

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

#include "UDPInterface.h"
#include "sls_receiver_defs.h"


class dummyUDPInterface : public UDPInterface {
	

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
  dummyUDPInterface() :  UDPInterface(), dynamicRange(16), scanTag(1000), nFrames(100), fWrite(1), fOverwrite(1), fIndex(0), fCaught(0), totfCaught(0), startAcqIndex(0), startFrameIndex(0), acqIndex(0), dataCompression(false), period(0), type(slsReceiverDefs::GENERIC), framesNeeded(100), udpPort1(1900), udpPort2(1901),  shortFrame(0), nFramesToGui(0), e10G(0) {strcpy(detHostname,"none"); strcpy(fName,"run"); strcpy(fPath,"/scratch/"); strcpy(eth,"eth0"); cout << "New dummy UDP Interface" << endl;};

    ~dummyUDPInterface() {cout << "Destroying  dummy UDP Interface" << endl;};
	

    void del(){cout << "Destroying  dummy UDP Interface" << endl;};
	
  virtual void configure(map<string, string> config_map) {};
	/**
	 * Initialize the Receiver
	 @param detectorHostName detector hostname
	 * you can call this function only once. You must call it before you call startReceiver() for  the first time.
	 */
  virtual void initialize(const char *detectorHostName){ cout << "set detector hostname to" << detHostname << endl; strcpy(detHostname,detectorHostName);};
	

	 /* Returns detector hostname
	    /returns hostname
	    * caller needs to deallocate the returned char array.
	    * if uninitialized, it must return NULL
	    */
  virtual char *getDetectorHostname() const { cout << "get detector hostname " << detHostname << endl; return (char*) detHostname;};

	/**
	 * Returns status of receiver: idle, running or error
	 */
	virtual slsReceiverDefs::runStatus getStatus() const { cout << "get dsummy status IDLE "  << endl; return slsReceiverDefs::IDLE;};;

	/**
	 * Returns File Name
	 * caller is responsible to deallocate the returned char array.
	 */
	virtual char *getFileName() const  { cout << "get file name " << fName << endl; return (char*) fName;};


	/**
	 * Returns File Path
	 * caller is responsible to deallocate the returned char array
	 */
	virtual char *getFilePath() const  { cout << "get file path " << fPath << endl; return  (char*) fPath;};;


	/**
	 * Returns the number of bits per pixel
	 */
	virtual int getDynamicRange() const { cout << "get dynamic range " << dynamicRange << endl; return dynamicRange;};;

	/**
	 * Returns scan tag
	 */
	virtual int getScanTag() const  { cout << "get scan tag " << scanTag << endl; return scanTag;};

	/*
	 * Returns number of frames to receive
	 * This is the number of frames to expect to receiver from the detector.
	 * The data receiver will change from running to idle when it got this number of frames
	 */
	virtual int getNumberOfFrames() const  { cout << "get number of frames " << nFrames << endl; return nFrames;};

	/**
	* Returns file write enable
	* 1: YES 0: NO
	*/
	virtual int getEnableFileWrite() const  { cout << "get enable file write " << fWrite << endl; return fWrite;};

	/**
	* Returns file over write enable
	* 1: YES 0: NO
	*/
	virtual int getEnableOverwrite() const  { cout << "get enable file overwrite " << fOverwrite << endl; return fOverwrite;};

	/**
	 * Set File Name (without frame index, file index and extension)
	 @param c file name
	 /returns file name
	  * returns NULL on failure (like bad file name)
	  * does not check the existence of the file - we don't know which path we'll finally use, so no point to check.
	  * caller is responsible to deallocate the returned char array.
	 */
	virtual char* setFileName(const char c[])  { strcpy(fName,c); cout << "set file name " << fName << endl; return fName; };

	/**
	 * Set File Path
	 @param c file path
	 /returns file path
	  * checks the existence of the directory. returns NULL if directory does not exist or is not readable.
	  * caller is responsible to deallocate the returned char array.
	 */
	virtual char* setFilePath(const char c[]) { strcpy(fPath,c); cout << "set file path " << fPath << endl; return fPath; };

	/**
	 * Returns the number of bits per pixel
	 @param dr sets dynamic range
	 /returns dynamic range
	  * returns -1 on failure
	  * FIXME: what are the allowd values - should we use an enum as argument?
	 */
	virtual int setDynamicRange(const int dr)  {dynamicRange=dr; cout << "set dynamic range " << dynamicRange << endl; return dynamicRange; };


	/**
	 * Set scan tag
	 @param tag scan tag
	 /returns scan tag (always non-negative)
	  * FIXME: valid range - only positive? 16bit ore 32bit?
	  * returns -1 on failure
	 */
	virtual int setScanTag(const int tag)  {scanTag=tag; cout << "set scan tag " << scanTag << endl; return scanTag; };


	/**
	 * Sets number of frames
	 @param fnum number of frames
	 /returns number of frames
	 */
	virtual int setNumberOfFrames(const int fnum) {nFrames=fnum; cout << "set number of frames " << nFrames << endl; return nFrames; };


	/**
	 * Set enable file write
	 * @param i file write enable
	 /returns file write enable
	 */
	virtual int setEnableFileWrite(const int i)  {fWrite=i; cout << "set enable file write " << fWrite << endl; return fWrite; };


	/**
	 * Set enable file overwrite
	 * @param i file overwrite enable
	 /returns file overwrite enable
	 */
	virtual int setEnableOverwrite(const int i)  {fOverwrite=i; cout << "set enable file overwrite " << fOverwrite << endl; return fOverwrite; };


	/**
	 * Starts Receiver - activate all configuration settings to the eiger receiver and start to listen for packets
	 @param message is the error message if there is an error
	 /returns 0 on success or -1 on failure
	 */
	//FIXME: success == 0 or success == 1?
	virtual int startReceiver(char *message=NULL) {cout << "dummy start receiver" << endl; return 0;};

	/**
	 * Stops Receiver - stops listening for packets
	 /returns success
	  * same as abort(). Always returns 0.
	 */
	virtual int stopReceiver()  {cout << "dummy stop receiver" << endl; return 0;};

	/**
	 * abort acquisition with minimum damage: close open files, cleanup.
	 * does nothing if state already is 'idle'
	 */
	virtual void abort()   {cout << "Aborting receiver" << endl; };



/*******************************************************************************************************************
 ****************************************  Added by Dhanya *********************************************************
 *******************************************************************************************************************/

	/**
	 * Returns File Index
	 */
	virtual int getFileIndex() {cout << "get file index " << fIndex << endl; return fIndex;};

	/**
	 * Returns Total Frames Caught for an entire acquisition (including all scans)
	 */
	virtual int getTotalFramesCaught() {cout << "get total frames caught " << totfCaught << endl ; return  totfCaught;};
	
	/**
	 * Returns Frames Caught for each real time acquisition (eg. for each scan)
	 */
	virtual int getFramesCaught()  {cout << "get frames caught " << fCaught << endl; return   fCaught;};


	/**
	 * Returns the frame index at start of entire acquisition (including all scans)
	 */
	virtual uint32_t getStartAcquisitionIndex(){ cout << "get start acquisition index " << startAcqIndex << endl; return startAcqIndex; };
	
	/**
	 * Returns current Frame Index Caught for an entire  acquisition (including all scans)
	 */
	virtual uint32_t getAcquisitionIndex(){ cout << "get acquisition index " << acqIndex << endl; return acqIndex; };
	
	
	/**
	 * Returns the frame index at start of each real time acquisition (eg. for each scan)
	 */
	virtual uint32_t getStartFrameIndex() { cout << "get start frame index " << startFrameIndex << endl; return startFrameIndex; };
	
	
	/** get data compression, by saving only hits
	 */
	virtual bool getDataCompression() { cout << "get data compression " << dataCompression << endl; return dataCompression;};

	/**
	 * Set receiver type
	 * @param det detector type
	 * Returns success or FAIL
	 */
	virtual int setDetectorType(slsReceiverDefs::detectorType det) {type=det; cout << "set detector type " << det << endl; return slsReceiverDefs::OK;};
	
	/**
	 * Set File Index
	 * @param i file index
	 */
	virtual int setFileIndex(int i) {fIndex=i; cout << "get file index " << fIndex << endl; return fIndex;};
	
	/** set acquisition period if a positive number
	 */
	virtual int64_t setAcquisitionPeriod(int64_t index) {if (index>=0) {period=index; cout << "set period " << period << endl;} else { cout << "get period " << period << endl;} return period;};

	/**
	 * Set Frame Index Needed
	 * @param i frame index needed
	 */
	virtual int setFrameIndexNeeded(int i) {framesNeeded=i;  cout << "set frame index needed " << period << endl; return framesNeeded;};

	/**
	 * Set UDP Port Number
	 */
	virtual void setUDPPortNo(int p){udpPort1=p;  cout << "set UDP port 1 " << udpPort1 << endl; };


	/**
	 * Set UDP Port Number
	 */
	virtual void setUDPPortNo2(int p) {udpPort2=p;  cout << "set UDP port 2 " << udpPort2 << endl; };

	/**
	 * Set Ethernet Interface or IP to listen to
	 */
	virtual void setEthernetInterface(char* c){strcpy(eth,c); cout << "set eth " << c;};

	/**
	 * Set short frame
	 * @param i if shortframe i=1
	 */
	virtual int setShortFrame(int i){shortFrame=i; cout << " set short frame" << shortFrame << endl; return shortFrame;};

	/**
	 * Set the variable to send every nth frame to gui
	 * or if 0,send frame only upon gui request
	 */
	virtual int setNFrameToGui(int i) {nFramesToGui=i; cout << "set nframes to gui " << nFramesToGui << endl; return nFramesToGui;};

	/**
	 * Resets the Total Frames Caught
	 * This is how the receiver differentiates between entire acquisitions
	 * Returns 0
	 */
	virtual void resetTotalFramesCaught() {totfCaught=0; cout << "total frames caugh reset " << totfCaught << endl;};

	/** enabl data compression, by saving only hits
	 /returns if failed
	 */
	virtual int enableDataCompression(bool enable) {dataCompression=enable; cout << "set data compression " << dataCompression<< endl; return dataCompression;};

	/**
	 * enable 10Gbe
	 @param enable 1 for 10Gbe or 0 for 1 Gbe, -1 to read out
	 \returns enable for 10Gbe
	 */
	virtual int enableTenGiga(int enable = -1) {if (enable>=0) {e10G=enable; cout << "set 10Gb "<< e10G << endl;} else cout << "get 10Gb "<< e10G << endl; return e10G;};

	/**
	 * Returns the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param fnum frame number for eiger as it is not in the packet
	 * @param startAcquisitionIndex is the start index of the acquisition
	 * @param startFrameIndex is the start index of the scan
	 */
	virtual void readFrame(char* c,char** raw, uint32_t &fnum, uint32_t &startAcquisitionIndex, uint32_t &startFrameIndex){cout << "dummy read frame" << endl; };

	/** set status to transmitting and
	 * when fifo is empty later, sets status to run_finished
	 */
	virtual void startReadout(){cout << "dummy start readout" << endl; };

	/**
	 * shuts down the  udp sockets
	 * \returns if success or fail
	 */
	virtual int shutDownUDPSockets(){cout << "dummy shut down udp sockets" << endl; return slsReceiverDefs::OK;};

	/**
	 * Closes all files
	 * @param ithr thread index, -1 for all threads
	 */
	virtual void closeFile(int ithr = -1){cout << "dummy close file" << ithr << endl; };

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
	virtual void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){cout << "dummy register callback start acquisition" << endl; };

	/**
	 * Call back for acquisition finished
	   callback argument is
	   total frames caught
	*/
	virtual void registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){cout << "dummy register callback acquisition finished" << endl; }; 

	/**
	 * Call back for raw data
	  args to raw data ready callback are
	  framenum
	  datapointer
	  datasize in bytes
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
virtual void registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){cout << "dummy register callback get raw data" << endl; }; 
	
 protected:
	
 private:
 char detHostname[1000];
	char fName[10000];
	char fPath[10000];
	int dynamicRange;
	int scanTag;
	int nFrames;
	int fWrite;
	int fOverwrite;

	int fIndex;
	int fCaught;
int totfCaught;
int startAcqIndex;
int startFrameIndex;
int acqIndex;
bool dataCompression;
 int64_t period;
 slsReceiverDefs::detectorType type;
 int framesNeeded;
 int udpPort1;
 int udpPort2;
 char eth[1000];
 int shortFrame;
 int nFramesToGui;
 int e10G;
};

#endif  /* #ifndef DUMMYUDPINTERFACE_H */
