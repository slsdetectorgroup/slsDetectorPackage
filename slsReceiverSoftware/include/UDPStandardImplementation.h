//#ifdef UDP_BASE_IMPLEMENTATION
#ifndef UDP_STANDARD_IMPLEMENTATION_H
#define UDP_STANDARD_IMPLEMENTATION_H
/********************************************//**
 * @file UDPBaseImplementation.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "sls_receiver_defs.h"
#include "receiver_defs.h"
#include "genericSocket.h"
#include "circularFifo.h"
#include "singlePhotonDetector.h"
#include "slsReceiverData.h"
#include "moenchCommonMode.h"

//#include "UDPInterface.h"
#include "UDPBaseImplementation.h"

#ifdef MYROOT1
#include <TTree.h>
#include <TFile.h>
#endif


#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>


/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */


class UDPStandardImplementation: private virtual slsReceiverDefs, public UDPBaseImplementation {
 public:
        /**
	 * Constructor
	 */
	UDPStandardImplementation();

	/**
	 * Destructor
	 */
	virtual ~UDPStandardImplementation();

	void configure(map<string, string> config_map);

	/**
	 * delete and free member parameters
	 */
        void deleteMembers();

	/**
	 * initialize member parameters
	 */
	void initializeMembers();

	/**
	 * Set receiver type
	 * @param det detector type
	 * Returns success or FAIL
	 */
        int setDetectorType(detectorType det);


	//Frame indices and numbers caught
    /**
     * Returns the frame index at start of entire acquisition (including all scans)
     */
    //uint32_t getStartAcquisitionIndex();

	/**
	 * Returns current Frame Index Caught for an entire  acquisition (including all scans)
	 */
	//uint32_t getAcquisitionIndex();

	/**
	 * Returns if acquisition started
	 */
	//bool getAcquistionStarted();

	/**
	 * Returns Frames Caught for each real time acquisition (eg. for each scan)
	 */
	//int getFramesCaught();

	/**
	 * Returns Total Frames Caught for an entire acquisition (including all scans)
	 */
	//int getTotalFramesCaught();

	/**
	 * Returns the frame index at start of each real time acquisition (eg. for each scan)
	 */
	//uint32_t getStartFrameIndex();

	/**
	 * Returns current Frame Index for each real time acquisition (eg. for each scan)
	 */
	//uint32_t getFrameIndex();

	/**
	 * Returns if measurement started
	 */
	//bool getMeasurementStarted();

	/**
	 * Resets the Total Frames Caught
	 * This is how the receiver differentiates between entire acquisitions
	 * Returns 0
	 */
        //void resetTotalFramesCaught();


	//file parameters
	/**
	 * Returns File Path
	 */
	//char* getFilePath() const;

	/**
	 * Set File Path
	 * @param c file path
	 */
        //char* setFilePath(const char c[]);

	/**
	 * Returns File Name
	 */
	//char* getFileName() const;

	/**
	 * Set File Name (without frame index, file index and extension)
	 * @param c file name
	 */
	//char* setFileName(const char c[]);

	/**
	 * Returns File Index
	 */
	//int getFileIndex();

	/**
	 * Set File Index
	 * @param i file index
	 */
	//int setFileIndex(int i);

	/**
	 * Set Frame Index Needed
	 * @param i frame index needed
	 */
	//int setFrameIndexNeeded(int i);

	/**
	 * Set enable file write
	 * @param i file write enable
	 * Returns file write enable
	 */
	//int setEnableFileWrite(int i);

	/**
	 * Enable/disable overwrite
	 * @param i enable
	 * Returns enable over write
	 */
	//int setEnableOverwrite(int i);

	/**
	* Returns file write enable
	* 1: YES 0: NO
	*/
	//int getEnableFileWrite() const;

	/**
	* Returns file over write enable
	* 1: YES 0: NO
	*/
	//int getEnableOverwrite() const;

//other parameters

	/**
	 * abort acquisition with minimum damage: close open files, cleanup.
	 * does nothing if state already is 'idle'
	 */
	void abort() {};

	/**
	 * Returns status of receiver: idle, running or error
	 */
	runStatus getStatus() const;

	/**
	 * Set detector hostname
	 * @param c hostname
	 */
	void initialize(const char *detectorHostName);

	 /* Returns detector hostname
	 /returns hostname
	  * caller needs to deallocate the returned char array.
	  * if uninitialized, it must return NULL
	 */
	char *getDetectorHostname() const;

	/**
	 * Set Ethernet Interface or IP to listen to
	 */
	void setEthernetInterface(char* c);

	/**
	 * Set UDP Port Number
	 */
	void setUDPPortNo(int p);
	/**
	 * Set UDP Port Number
	 */
	void setUDPPortNo2(int p);

	/*
	 * Returns number of frames to receive
	 * This is the number of frames to expect to receiver from the detector.
	 * The data receiver will change from running to idle when it got this number of frames
	 */
	int getNumberOfFrames() const;

	/**
	 * set frame number if a positive number
	 */
	int32_t setNumberOfFrames(int32_t fnum);

	/**
	 * Returns scan tag
	 */
	int getScanTag() const;

	/**
	 * set scan tag if its is a positive number
	 */
	int32_t setScanTag(int32_t stag);

	/**
	 * Returns the number of bits per pixel
	 */
	int getDynamicRange() const;

	/**
	 * set dynamic range if its is a positive number
	 */
	int32_t setDynamicRange(int32_t dr);

	/**
	 * Set short frame
	 * @param i if shortframe i=1
	 */
	int setShortFrame(int i);

	/**
	 * Set the variable to send every nth frame to gui
	 * or if 0,send frame only upon gui request
	 */
	int setNFrameToGui(int i);

	/** set acquisition period if a positive number
	 */
	int64_t setAcquisitionPeriod(int64_t index);

	/** get data compression, by saving only hits
	 */
	bool getDataCompression();

	/** enabl data compression, by saving only hits
	 /returns if failed
	 */
	int enableDataCompression(bool enable);

	/**
	 * enable 10Gbe
	 @param enable 1 for 10Gbe or 0 for 1 Gbe, -1 to read out
	 \returns enable for 10Gbe
	 */
	int enableTenGiga(int enable = -1);



//other functions

	/**
	 * Returns the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param fnum frame number for eiger as it is not in the packet
	 * @param startAcquisitionIndex is the start index of the acquisition
	 * @param startFrameIndex is the start index of the scan
	 */
	void readFrame(char* c,char** raw, uint32_t &fnum, uint32_t &startAcquisitionIndex, uint32_t &startFrameIndex);

	/**
	 * Closes all files
	 * @param ithr thread index
	 */
	void closeFile(int ithr = -1);

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

	/** set status to transmitting and
	 * when fifo is empty later, sets status to run_finished
	 */
	void startReadout();

	/**
	 * shuts down the  udp sockets
	 * \returns if success or fail
	 */
	int shutDownUDPSockets();

private:
	
	/*
	void not_implemented(string method_name){
		std::cout << "[WARNING] Method " << method_name << " not implemented!" << std::endl;
	};
	*/
	/**
	 * Deletes all the filter objects for single photon data
	 */
	void deleteFilter();

	/**
	 * Constructs the filter for single photon data
	 */
	void setupFilter();

	/**
	 * set up fifo according to the new numjobsperthread
	 */
	void setupFifoStructure ();

	/**
	 * Copy frames to gui
	 * uses semaphore for nth frame mode
	 */
	void copyFrameToGui(char* startbuf[], uint32_t fnum=-1, char* buf=NULL);

	/**
	 * creates udp sockets
	 * \returns if success or fail
	 */
	int createUDPSockets();

	/**
	 * create listening thread
	 * @param destroy is true to kill all threads and start again
	 */
	int createListeningThreads(bool destroy = false);

	/**
	 * create writer threads
	 * @param destroy is true to kill all threads and start again
	 */
	int createWriterThreads(bool destroy = false);

	/**
	 * set thread priorities
	 */
	void setThreadPriorities();

	/**
	 * initializes variables and creates the first file
	 * also does the startAcquisitionCallBack
	 * \returns FAIL or OK
	 */
	int setupWriter();

	/**
	 * Creates new tree and file for compression
	 * @param ithr thread number
	 * @param iframe frame number
	 *\returns OK for succces or FAIL for failure
	 */
	int createCompressionFile(int ithr, int iframe);

	/**
	 * Creates new file
	 *\returns OK for succces or FAIL for failure
	 */
	int createNewFile();

	/**
	 * Static function - Thread started which listens to packets.
	 * Called by startReceiver()
	 * @param this_pointer pointer to this object
	 */
	static void* startListeningThread(void *this_pointer);

	/**
	 * Static function - Thread started which writes packets to file.
	 * Called by startReceiver()
	 * @param this_pointer pointer to this object
	 */
	static void* startWritingThread(void *this_pointer);

	/**
	 * Thread started which listens to packets.
	 * Called by startReceiver()
	 *
	 */
	int startListening();

	/**
	 * Thread started which writes packets to file.
	 * Called by startReceiver()
	 *
	 */
	int startWriting();

	/**
	 * Writing to file without compression
	 * @param buf is the address of buffer popped out of fifo
	 * @param numpackets is the number of packets
	 * @param framenum current frame number
	 */
	void writeToFile_withoutCompression(char* buf,int numpackets, uint32_t framenum);

	/**
	 * Its called for the first packet of a scan or acquistion
	 * Sets the startframeindices and the variables to know if acquisition started
	 * @param ithread listening thread number
	 */
	void startFrameIndices(int ithread);

	/**
	 * This is called when udp socket is shut down
	 * It pops ffff instead of packet number into fifo
	 * to inform writers about the end of listening session
	 * @param ithread listening thread number
	 * @param rc number of bytes received
	 * @param pc packet count
	 * @param t total packets listened to
	 */
	void stopListening(int ithread, int rc, int &pc, int &t);

	/**
	 * When acquisition is over, this is called
	 * @param ithread listening thread number
	 * @param wbuffer writer buffer
	 */
	void stopWriting(int ithread, char* wbuffer[]);


	/**
	 * data compression for each fifo output
	 * @param ithread listening thread number
	 * @param wbuffer writer buffer
	 * @param npackets number of packets from the fifo
	 * @param data pointer to the next packet start
	 * @param xmax max pixels in x direction
	 * @param ymax max pixels in y direction
	 * @param nf nf
	 */
	void handleDataCompression(int ithread, char* wbuffer[], int &npackets, char* data, int xmax, int ymax, int &nf);




public:


	/**
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
	void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){startAcquisitionCallBack=func; pStartAcquisition=arg;};

	/**
	   callback argument is
	   toatal frames caught
	*/
	void registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){acquisitionFinishedCallBack=func; pAcquisitionFinished=arg;};
	
	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  datasize in bytes
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	void registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){rawDataReadyCallBack=func; pRawDataReady=arg;};
};


#endif

//#endif
