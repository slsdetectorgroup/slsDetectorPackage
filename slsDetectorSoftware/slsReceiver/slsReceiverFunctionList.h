#ifdef SLS_RECEIVER_FUNCTION_LIST
/********************************************//**
 * @file slsReceiverFunctionList.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "genericSocket.h"

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
	virtual ~slsReceiverFunctionList(){};

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
	 * Resets the Total Frames Caught
	 * This is how the receiver differentiates between entire acquisitions
	 * @param i true if frame index in file name is required, else false
	 * Returns frames needed in file name
	 */
	bool resetTotalFramesCaught(bool i);

	/**
	 * Close File
	 */
	static void closeFile(int p);

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
	 * Returns the buffer-current frame read by receiver
	 */
	char* readFrame(char* c);



private:
	/** Complete File name */
	char savefilename[MAX_STR_LENGTH];

	/** File Name without frame index, file index and extension*/
	char fileName[MAX_STR_LENGTH];

	/** File Path */
	char filePath[MAX_STR_LENGTH];

	/** File Index */
	int fileIndex;

	/** if frame index required in file name */
	bool frameIndexNeeded;

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

	/** if the listening thread is running*/
	static int listening_thread_running;

	/** thread listening to packets */
	pthread_t   listening_thread;

	/** status of receiver */
	runStatus status;

	/** File Descriptor */
	static FILE *sfilefd;

	/** Receiver buffer */
	char buffer[BUFFER_SIZE];

	/** UDP Socket between Receiver and Detector */
	genericSocket* udpSocket;

	/** Server UDP Port*/
	int server_port;

};
/*

//int setUDPPortNumber(int p=-1); //sets/gets port number to listen to for data from the detector
//int setTCPPortNumber(int p=-1); //sets/get port number for communication to client
*/

#endif
