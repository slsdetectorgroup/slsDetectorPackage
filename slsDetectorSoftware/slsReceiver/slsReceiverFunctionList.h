#ifdef SLS_RECEIVER_FUNCTION_LIST

#ifndef SLS_RECEIVER_FUNCTION_LIST_H
#define SLS_RECEIVER_FUNCTION_LIST_H
/********************************************//**
 * @file slsReceiverFunctionList.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "genericSocket.h"
#include "circularFifo.h"

#include "singlePhotonDetector.h"
#include "moench02ModuleData.h"

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

class slsReceiverFunctionList : private virtual slsDetectorDefs  {

public:
	/**
	 * Constructor
	 */
	slsReceiverFunctionList(detectorType det);

	/**
	 * Destructor
	 */
	virtual ~slsReceiverFunctionList();

	/**
	 * Set UDP Port Number
	 */
	void setUDPPortNo(int p){server_port = p;};

	/**
	 * Set Ethernet Interface or IP to listen to
	 */
	void setEthernetInterface(char* c);

	/**
	 * Returns status of receiver: idle, running or error
	 */
	runStatus getStatus(){ return status;};

	/**
	 * Returns File Name
	 */
	char* getFileName(){return fileName;};

	/**
	 * Returns File Path
	 */
	char* getFilePath(){return filePath;};

	/**
	 * Returns File Index
	 */
	int getFileIndex(){return fileIndex;};

	/**
	 * Returns Frames Caught for each real time acquisition (eg. for each scan)
	 */
	int getFramesCaught(){return (packetsCaught/packetsPerFrame);};

	/**
	 * Returns Total Frames Caught for an entire acquisition (including all scans)
	 */
	int getTotalFramesCaught(){return (totalPacketsCaught/packetsPerFrame);};

	/**
	 * Returns the frame index at start of each real time acquisition (eg. for each scan)
	 */
	uint32_t getStartFrameIndex(){return startFrameIndex;};

	/**
	 * Returns current Frame Index for each real time acquisition (eg. for each scan)
	 */
	uint32_t getFrameIndex();

	/**
	 * Returns current Frame Index Caught for an entire  acquisition (including all scans)
	 */
	uint32_t getAcquisitionIndex();


	/**
	 * Returns if acquisition started
	 */
	bool getAcquistionStarted(){return acqStarted;};


	/**
	 * Returns if measurement started
	 */
	bool getMeasurementStarted(){return measurementStarted;};

	/**
	 * Set File Name (without frame index, file index and extension)
	 * @param c file name
	 */
	char* setFileName(char c[]);

	/**
	 * Set File Path
	 * @param c file path
	 */
	char* setFilePath(char c[]);

	/**
	 * Set File Index
	 * @param i file index
	 */
	int setFileIndex(int i);

	/**
	 * Set Frame Index Needed
	 * @param i frame index needed
	 */
	int setFrameIndexNeeded(int i){frameIndexNeeded = i; return frameIndexNeeded;};

	/**
	 * Set enable file write
	 * @param i file write enable
	 * Returns file write enable
	 */
	int setEnableFileWrite(int i);

	/**
	 * Resets the Total Frames Caught
	 * This is how the receiver differentiates between entire acquisitions
	 * Returns 0
	 */
	void resetTotalFramesCaught();

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

	/** enabl data compression, by saving only hits
	 */
	void enableDataCompression(bool enable);

	/** get data compression, by saving only hits
	 */
	bool getDataCompression(){ return dataCompression;};

	/** set status to transmitting and
	 * when fifo is empty later, sets status to run_finished
	 */
	void startReadout();

	/**
	 * Returns the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 */
	void readFrame(char* c,char** raw);


	/** free fifo buffer, called back from single photon filter
	 */
	static void freeFifoBufferCallBack (char* fbuffer, void *this_pointer);

	/**
	 * Call back from single photon filter to free writingfifo
	 * called from freeFifoBufferCallBack
	 */
	void freeFifoBuffer(char* fbuffer);

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


private:
	/**
	 * Constructs the filter for single photon data
	 */
	void setupFilter();

	/**
	 * Copy frames to gui
	 * uses semaphore for nth frame mode
	 */
	void copyFrameToGui(char* startbuf);

	/**
	 * set up fifo according to the new numjobsperthread
	 */
	void setupFifoStructure ();

	/**
	 * creates udp socket
	 * \returns if success or fail
	 */
	int createUDPSocket();

	/**
	 * create listening thread and many writer threads at class construction
	 * @param destroy is true to kill all threads and start again
	 */
	int createThreads(bool destroy = false);

	/**
	 * initializes variables and creates the first file
	 * also does the startAcquisitionCallBack
	 * \returns FAIL or OK
	 */
	int setupWriter();

	/**
	 * Creates new tree and file for compression
	 *\returns OK for succces or FAIL for failure
	 */
	int createCompressionFile();

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
	 * @param num
	 */
	void writeToFile_withoutCompression(char* buf,int numpackets);







	/** detector type */
	detectorType myDetectorType;

	/** status of receiver */
	runStatus status;

	/** UDP Socket between Receiver and Detector */
	genericSocket* udpSocket;

	/** Server UDP Port*/
	int server_port;

	/** ethernet interface or IP to listen to */
	char *eth;

	/** max packets per file **/
	int maxPacketsPerFile;

	/** File write enable */
	int enableFileWrite;

	/** Complete File name */
	char savefilename[MAX_STR_LENGTH];

	/** File Name without frame index, file index and extension*/
	char fileName[MAX_STR_LENGTH];

	/** File Path */
	char filePath[MAX_STR_LENGTH];

	/** File Index */
	int fileIndex;

	/** if frame index required in file name */
	int frameIndexNeeded;

	/* Acquisition started */
	bool acqStarted;

	/* Measurement started */
	bool measurementStarted;

	/** Frame index at start of each real time acquisition (eg. for each scan) */
	uint32_t startFrameIndex;

	/** Actual current frame index of each time acquisition (eg. for each scan) */
	uint32_t frameIndex;

	/** Frames Caught for each real time acquisition (eg. for each scan) */
	int packetsCaught;

	/** Total packets caught for an entire acquisition (including all scans) */
	int totalPacketsCaught;

	/** Pckets currently in current file, starts new file when it reaches max */
	int packetsInFile;

	/** Frame index at start of an entire acquisition (including all scans) */
	uint32_t startAcquisitionIndex;

	/** Actual current frame index of an entire acquisition (including all scans) */
	uint32_t acquisitionIndex;

	/** number of packets per frame*/
	int packetsPerFrame;

	/** frame index mask */
	uint32_t frameIndexMask;

	/** packet index mask */
	uint32_t packetIndexMask;

	/** frame index offset */
	int frameIndexOffset;

	/** acquisition period */
	int64_t acquisitionPeriod;

	/** short frames */
	int shortFrame;

	/** current frame number */
	uint32_t currframenum;

	/** Previous Frame number from buffer */
	uint32_t prevframenum;

	/** buffer size can be 1286*2 or 518 or 1286*40 */
	int bufferSize;

	/** oen buffer size */
	int onePacketSize;

	/** latest data */
	char* latestData;

	/** gui data ready */
	int guiDataReady;

	/** points to the data to send to gui */
	char* guiData;

	/** points to the filename to send to gui */
	char* guiFileName;

	/** send every nth frame to gui or only upon gui request*/
	int nFrameToGui;

	/** fifo size */
	unsigned int fifosize;

	/** number of jobs per thread for data compression */
	int numJobsPerThread;

	/** memory allocated for the buffer */
	char *mem0;

	/** datacompression - save only hits */
	bool dataCompression;

	/** circular fifo to store addresses of data read */
	CircularFifo<char>* fifo;

	/** circular fifo to store addresses of data already written and ready to be resued*/
	CircularFifo<char>* fifoFree;

	/** Receiver buffer */
	char *buffer;

	/** max number of writer threads */
	const static int MAX_NUM_WRITER_THREADS = 15;

	/** number of writer threads */
	int numWriterThreads;

	/** to know if listening and writer threads created properly */
	int thread_started;

	/** mask showing which threads are running */
	volatile int32_t writerthreads_mask;

	/** current writer thread index*/
	int currentWriterThreadIndex;

	/** thread listening to packets */
	pthread_t   listening_thread;

	/** thread writing packets */
	pthread_t   writing_thread[MAX_NUM_WRITER_THREADS];

	/** total frame count the listening thread has listened to */
	int totalListeningFrameCount;





//semaphores
	/** semaphore to synchronize  writer and guireader threads */
	sem_t smp;
	/** semaphore to synchronize  listener thread */
	sem_t listensmp;
	/** semaphore to synchronize  writer threads */
	sem_t writersmp[MAX_NUM_WRITER_THREADS];

//mutex
	/** guiDataReady mutex */
	pthread_mutex_t  dataReadyMutex;

	/** mutex for status */
	pthread_mutex_t status_mutex;

	/** mutex for progress variable currframenum */
	pthread_mutex_t progress_mutex;

	/** mutex for writing data to file */
	pthread_mutex_t write_mutex;


//filter
	singlePhotonDetector<uint16_t> *singlePhotonDet;

	moench02ModuleData *mdecoder;

	bool commonModeSubtractionEnable;

	int iFrame;

	/**
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   data size
	   
	   return value is 
	   0 callback takes care of open,close,write file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything

	*/
	int (*startAcquisitionCallBack)(char*, char*,int, int, void*);
	void *pStartAcquisition;

	/**
	   args to acquisition finished callback
	   total frames caught
	   
	*/
	void (*acquisitionFinishedCallBack)(int, void*);
	void *pAcquisitionFinished;


	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  datasize in bytes
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	void (*rawDataReadyCallBack)(int, char*, int, FILE*, char*, void*);
	void *pRawDataReady;

	/** The action which decides what the user and default responsibilites to save data are
	 * 0 raw data ready callback takes care of open,close,write file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything */
	int cbAction;



public:

#ifdef MYROOT1
	/** Tree where the hits are stored */
	static TTree *myTree;

	/** File where the tree is saved */
	static TFile *myFile;
#endif


	/** File Descriptor */
	static FILE *sfilefd;

	/** if the receiver threads are running*/
	static int receiver_threads_running;

	/** 0 if receiver is idle, 1 otherwise */
	static int running;

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

#endif
