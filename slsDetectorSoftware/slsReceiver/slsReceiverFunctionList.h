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

#include <string.h>
#include <pthread.h>
#include <stdio.h>

/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */

class slsReceiverFunctionList : private virtual slsDetectorDefs  {

public:
	/**
	 * Constructor
	 */
	slsReceiverFunctionList();

	/**
	 * Destructor
	 */
	virtual ~slsReceiverFunctionList(){ if(latestData) delete latestData;};

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
	runStatus getStatus(){return status;};

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
	int	getFileIndex(){return fileIndex;};

	/**
	 * Returns Frames Caught for each real time acquisition (eg. for each scan)
	 */
	int getFramesCaught(){return framesCaught;};

	/**
	 * Returns Total Frames Caught for an entire acquisition (including all scans)
	 */
	int getTotalFramesCaught(){	return totalFramesCaught;};

	/**
	 * Returns the frame index at start of each real time acquisition (eg. for each scan)
	 */
	int getStartFrameIndex(){return startFrameIndex;};

	/**
	 * Returns current Frame Index for each real time acquisition (eg. for each scan)
	 */
	int getFrameIndex();

	/**
	 * Returns current Frame Index Caught for an entire  acquisition (including all scans)
	 */
	int getAcquisitionIndex();

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
	int setFrameIndexNeeded(int i){frameIndexNeeded = i;};

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
	 * Close File
	 */
	//static void closeFile(int p);

	/**
	 * Starts Receiver - starts to listen for packets
	 * Returns success
	 */
	int startReceiver();

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

	/**
	 * Register call back function to write receiver data
	 */
	void registerWriteReceiverDataCallback(int( *userCallback)(char*, int, FILE*, void*), void *pArg) {writeReceiverData = userCallback; pwriteReceiverDataArg = pArg;};

private:

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

	/** Frame index at start of each real time acquisition (eg. for each scan) */
	int startFrameIndex;

	/** Actual current frame index of each time acquisition (eg. for each scan) */
	int frameIndex;

	/** Total Frames Caught for an entire acquisition (including all scans) */
	int totalFramesCaught;

	/** Frame index at start of an entire acquisition (including all scans) */
	int startAcquisitionIndex;

	/** Actual current frame index of an entire acquisition (including all scans) */
	int acquisitionIndex;

	/** Frames currently in current file, starts new file when it reaches max */
	int framesInFile;

	/** Previous Frame number from buffer */
	int prevframenum;

	/** thread listening to packets */
	pthread_t   listening_thread;

	/** thread writing packets */
	pthread_t   writing_thread;

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
	//CircularFifo<dataStruct,FIFO_SIZE>* fifo;
	CircularFifo<char,FIFO_SIZE>* fifo;

	/** circular fifo to read and write data*/
	CircularFifo<char,FIFO_SIZE>* fifofree;

	/** short frames */
	int shortFrame;

	/** buffer size can be 1286*2 or 518 */
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
	int currframenum;

	/** register for call back to get data */
	int (*writeReceiverData)(char*,int,FILE*,void*);
	void *pwriteReceiverDataArg;

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
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	void (*rawDataReadyCallBack)(int, char*, FILE*, char*, void*);
	void *pRawDataReady;

	/** The action which decides what the user and default responsibilites to save data are
	 * 0 raw data ready callback takes care of open,close,write file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything */
	int cbAction;

public:
	/** File Descriptor */
	static FILE *sfilefd;

	/** if the listening thread is running*/
	static int listening_thread_running;

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
	int registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){acquisitionFinishedCallBack=func; pAcquisitionFinished=arg;};
	


	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	int registerCallBackRawDataReady(void (*func)(int, char*, FILE*, char*, void*),void *arg){rawDataReadyCallBack=func; pRawDataReady=arg;};




};


#endif

#endif
