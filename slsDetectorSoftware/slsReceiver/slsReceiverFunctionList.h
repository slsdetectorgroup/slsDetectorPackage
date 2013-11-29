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
#include "singlePhotonFilter.h"


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
	int getFramesCaught(){return framesCaught;};

	/**
	 * Returns Total Frames Caught for an entire acquisition (including all scans)
	 */
	int getTotalFramesCaught(){return totalFramesCaught;};

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

	/**
	 * Static function - Thread started which listens to packets.
	 * Called by startReceiver()
	 * @param this_pointer pointer to this object
	 */
	static void* startListeningThread(void *this_pointer);

	/**
	 * Thread started which listens to packets.
	 * Called by startReceiver()
	 *
	 */
	int startListening();

	/**
	 * Static function - Thread started which writes packets to file.
	 * Called by startReceiver()
	 * @param this_pointer pointer to this object
	 */
	static void* startWritingThread(void *this_pointer);

	/**
	 * Thread started which writes packets to file.
	 * Called by startReceiver()
	 *
	 */
	int startWriting();

	/**
	 * Creates new file
	 *\returns OK for succces or FAIL for failure
	 */
	int createNewFile();

	/**
	 * Copy frames to gui
	 * uses semaphore for nth frame mode
	 */
	void copyFrameToGui(char* startbuf);

	/**
	 * Returns the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 */
	void readFrame(char* c,char** raw);

	/**
	 * Set short frame
	 * @param i if shortframe i=1
	 */
	int setShortFrame(int i);

	/** set status to transmitting and
	 * when fifo is empty later, sets status to run_finished */
	void startReadout();

	/** enabl data compression, by saving only hits */
	void enableDataCompression(bool enable){dataCompression = enable;if(filter)filter->enableCompression(enable);};

	/** get data compression, by saving only hits */
	bool getDataCompression(){ return dataCompression;};

	/**
	 * Set the variable to send every nth frame to gui
	 * or if 0,send frame only upon gui request
	 */
	int setNFrameToGui(int i);

	/** set acquisition period if a positive number */
	int64_t setAcquisitionPeriod(int64_t index);

	/** set up fifo according to the new numjobsperthread */
	void setupFifoStructure ();

	/** free fifo buffer, called back from single photon filter */
	static void freeFifoBufferCallBack (char* fbuffer, void *this_pointer){((slsReceiverFunctionList*)this_pointer)->freeFifoBuffer(fbuffer);};
	void freeFifoBuffer(char* fbuffer){/*cout<< "fifo freed:"<<(void*)fbuffer<<endl;*/fifofree->push(fbuffer);};



private:

	/** detector type */
	detectorType myDetectorType;

	/** max frames per file **/
	int maxFramesPerFile;

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

	/** Frames Caught for each real time acquisition (eg. for each scan) */
	int framesCaught;

	/* Acquisition started */
	bool acqStarted;

	/* Measurement started */
	bool measurementStarted;

	/** Frame index at start of each real time acquisition (eg. for each scan) */
	uint32_t startFrameIndex;

	/** Actual current frame index of each time acquisition (eg. for each scan) */
	uint32_t frameIndex;

	/** Total Frames Caught for an entire acquisition (including all scans) */
	int totalFramesCaught;

	/** Total packets caught for an entire acquisition (including all scans) */
	int totalPacketsCaught;

	/** Frames currently in current file, starts new file when it reaches max */
	int framesInFile;

	/** Frame index at start of an entire acquisition (including all scans) */
	uint32_t startAcquisitionIndex;

	/** Actual current frame index of an entire acquisition (including all scans) */
	uint32_t acquisitionIndex;

	/** Previous Frame number from buffer */
	uint32_t prevframenum;

	/** thread listening to packets */
	pthread_t   listening_thread;

	/** thread writing packets */
	pthread_t   writing_thread;

	/** mutex for locking variable used by different threads */
	pthread_mutex_t status_mutex;

	/** listening thread running */
	int listening_thread_running;

	/** writing thread running */
	int writing_thread_running;

	/** status of receiver */
	runStatus status;

	/** Receiver buffer */
	char* buffer;

	/** Receiver buffer */
	char *mem0, *memfull;

	/** latest data */
	char* latestData;

	/** UDP Socket between Receiver and Detector */
	genericSocket* udpSocket;

	/** Server UDP Port*/
	int server_port;

	/** ethernet interface or IP to listen to */
	char *eth;

	/** Element structure to put inside a fifo */
	struct dataStruct {
	char* buffer;
	int rc;
	};

	/** circular fifo to read and write data*/
	CircularFifo<char>* fifo;

	/** circular fifo to read and write data*/
	CircularFifo<char>* fifofree;

	/** fifo size */
	unsigned int fifosize;

	/** short frames */
	int shortFrame;

	/** buffer size can be 1286*2 or 518 or 1286*40 */
	int bufferSize;

	/** number of packets per frame*/
	int packetsPerFrame;
	
	/** gui data ready */
	int guiDataReady;

	/** points to the data to send to gui */
	char* guiData;

	/** points to the filename to send to gui */
	char* guiFileName;

	/** current frame number */
	uint32_t currframenum;

	/** send every nth frame to gui or only upon gui request*/
	int nFrameToGui;

	/** frame index mask */
	int frameIndexMask;

	/** frame index offset */
	int frameIndexOffset;

	/** datacompression - save only hits */
	bool dataCompression;

	/** single photon filter */
	singlePhotonFilter *filter;

	/** oen buffer size */
	int oneBufferSize;

	/** semaphore to synchronize  writer and guireader threads */
	sem_t smp;

	/** guiDataReady mutex */
	pthread_mutex_t  dataReadyMutex;

	/** Number of jobs per thread for data compression */
	int numJobsPerThread;

	/** acquisition period */
	int64_t acquisitionPeriod;

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
	/** File Descriptor */
	static FILE *sfilefd;

	/** if the receiver threads are running*/
	static int receiver_threads_running;


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
