//#ifdef UDP_BASE_IMPLEMENTATION
#ifndef UDP_STANDARD_IMPLEMENTATION_H
#define UDP_STANDARD_IMPLEMENTATION_H
/********************************************//**
 * @file UDPBaseImplementation.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/

#include "UDPBaseImplementation.h"

#include "genericSocket.h"
#include "circularFifo.h"
#include "singlePhotonDetector.h"
#include "slsReceiverData.h"
#include "moenchCommonMode.h"


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


	/*************************************************************************
	 * Constructor & Destructor **********************************************
	 *************************************************************************/
	/**
	 * Constructor
	 */
	UDPStandardImplementation();

	/**
	 * Destructor
	 */
	virtual ~UDPStandardImplementation();


	/*************************************************************************
	 * Getters ***************************************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/


	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/

	//**initial parameters***

	/**
	 * Overridden method
	 * Configure command line parameters
	 * @param config_map mapping of config parameters passed from command line arguments
	 */
	void configure(map<string, string> config_map);

	//*** file parameters***
	/**
	 * Set File Name Prefix (without frame index, file index and extension (_d0_f000000000000_8.raw))
	 * Does not check for file existence since it is created only at startReceiver
	 * @param c file name (max of 1000 characters)
	 */
	void setFileName(const char c[]);

	/**
	 * Overridden method
	 * Set data compression, by saving only hits (so far implemented only for Moench and Gotthard)
	 * @param b true for data compression enable, else false
	 * @return OK or FAIL
	 */
	int setDataCompressionEnable(const bool b);

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

	//***acquisition parameters***
	/**
	 * Overridden method
	 * Set Short Frame Enabled, later will be moved to getROI (so far only for gotthard)
	 * @param i index of adc enabled, else -1 if all enabled
	 */
	void setShortFrameEnable(const int i);

	/**
	 * Set the Frequency of Frames Sent to GUI
	 * @param freq 0 for random frame requests, n for nth frame frequency
	 * @return OK or FAIL
	 */
	int setFrameToGuiFrequency(const uint32_t freq);

	/**
	 * Set the data stream enable
	 * @param enable 0 to disable, 1 to enable
	 * @return OK or FAIL
	 */
	uint32_t setDataStreamEnable(const uint32_t enable);

	/**
	 * Overridden method
	 * Set Acquisition Period
	 * @param i acquisition period
	 * @return OK or FAIL
	 */
	int setAcquisitionPeriod(const uint64_t i);

	/**
	 * Overridden method
	 * Set Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames
	 * @param i number of frames expected
	 * @return OK or FAIL
	 */
	int setNumberOfFrames(const uint64_t i);

	/**
	 * Overridden method
	 * Set Dynamic Range or Number of Bits Per Pixel
	 * @param i dynamic range that is 4, 8, 16 or 32
	 * @return OK or FAIL
	 */
	int setDynamicRange(const uint32_t i);

	/**
	 * Overridden method
	 * Set Ten Giga Enable
	 * @param b true if 10Giga enabled, else false (1G enabled)
	 * @return OK or FAIL
	 */
	int setTenGigaEnable(const bool b);


	/**
	 * Overridden method
	 * Set Fifo Depth
	 * @param i fifo depth value
	 * @return OK or FAIL
	 */
	int setFifoDepth(const uint32_t i);

	/*************************************************************************
	 * Behavioral functions***************************************************
	 * They may modify the status of the receiver ****************************
	 *************************************************************************/

	//***initial functions***
	/**
	 * Overridden method
	 * Set receiver type (and corresponding detector variables in derived STANDARD class)
	 * It is the first function called by the client when connecting to receiver
	 * @param d detector type
	 * @return OK or FAIL
	 */
	int setDetectorType(const detectorType d);

	//***acquisition functions***
	/**
	 * Overridden method
	 * Reset acquisition parameters such as total frames caught for an entire acquisition (including all scans)
	 */
	void resetAcquisitionCount();

	/**
	 * Overridden method
	 * Start Listening for Packets by activating all configuration settings to receiver
	 * When this function returns, it has status RUNNING(upon SUCCESS) or IDLE (upon failure)
	 * @param c error message if FAIL
	 * @return OK or FAIL
	 */
	int startReceiver(char *c=NULL);

	/**
	 * Overridden method
	 * Stop Listening for Packets
	 * Calls startReadout(), which stops listening and sets status to Transmitting
	 * When it has read every frame in buffer, the status changes to Run_Finished
	 * When this function returns, receiver has status IDLE
	 * Pre: status is running, semaphores have been instantiated,
	 * Post: udp sockets shut down, status is idle, semaphores destroyed
	 */
	void stopReceiver();

	/**
	 * Overridden method
	 * Stop Listening to Packets
	 * and sets status to Transmitting
	 * Next step would be to get all data and stop receiver completely and return with idle state
	 * Pre: status is running, udp sockets have been initialized, stop receiver initiated
	 * Post:udp sockets closed, status is transmitting
	 */
	void startReadout();

	/**
	 * Overridden method
	 * Shuts down and deletes UDP Sockets
	 * TCPIPInterface can also call this in case of illegal shutdown of receiver
	 * @return OK or FAIL
	 */
	int shutDownUDPSockets();

	/**
	 * Overridden method
	 * Get the buffer-current frame read by receiver
	 * @param ithread writer thread
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param startAcq start index of the acquisition
	 * @param startFrame start index of the scan
	 */
	void readFrame(int ithread, char* c,char** raw, int64_t &startAcq, int64_t &startFrame);


	void resetGuiPointer(int ithread);
	/**
	 * Overridden method
	 * Closes file / all files(data compression involves multiple files)
	 * TCPIPInterface can also call this in case of illegal shutdown of receiver
	 * @param ithread writer thread index
	 */
	void closeFile(int ithread = 0);

private:
	/*************************************************************************
	 * Getters ***************************************************************
	 * They access local cache of configuration or detector parameters *******
	 *************************************************************************/

/*
	uint64_t (*getFrameNumber)();
	uint64_t eigerGetFrameNumber();
	uint64_t generalGetFrameNumber();
	getframenumber = &generalgetframenumber;
	if(dettpe == eiger) getframenumber = &eigerGerFramenumber;

	call using getframenumber();
*/

	//**initial parameters***

    /**
	 * Delete and free member parameters
	 */
    void deleteMembers();

	/**
	 * Deletes all the filter objects for single photon data
	 * Deals with data compression
	 */
	void deleteFilter();

	/**
	 * Initialize base member parameters
	 */
	void initializeBaseMembers();

	/**
	 * Initialize member parameters
	 */
	void initializeMembers();

	/**
	 * Sets up all the filter objects for single photon data
	 * Deals with data compression
	 */
	void initializeFilter();

	/**
	 * Set up the Fifo Structure for processing buffers
	 * between listening and writer threads
	 * When the parameters ahve been determined and if fifostructure needs to be changes,
	 * the listerning and writing threads are also destroyed together with this
	 * @return OK or FAIL
	 */
	int setupFifoStructure();



	/*************************************************************************
	 * Listening and Writing Threads *****************************************
	 *************************************************************************/
	/**
	 * Create Data Call Back Threads
	 * @param destroy is true to destroy all the threads
	 * @return OK or FAIL
	 */
	int createDataCallbackThreads(bool destroy = false);

	/**
	 *  Create Listening Threads
	 * @param destroy is true to destroy all the threads
	 */
	int createListeningThreads(bool destroy = false);

	/**
	 * Create Writer Threads
	 * @param destroy is true to destroy all the threads
	 * @return OK or FAIL
	 */
	int createWriterThreads(bool destroy = false);




	/**
	 * Set Thread Priorities
	 */
	void setThreadPriorities();

	/**
	 * Creates UDP Sockets
	 * @return OK or FAIL
	 */
	int createUDPSockets();

	/**
	 * Initializes writer variables and creates the first file
	 * also does the startAcquisitionCallBack
	 * @return OK or FAIL
	 */
	int setupWriter();

	/**
	 * Creates new file and reset some parameters
	 * @param ithread writer thread index
	 * @return OK or FAIL
	 */
	int createNewFile(int ithread);

	/**
	 * Creates new tree and file for compression
	 * @param ithread thread number
	 * @param iframe frame number
	 * @return OK or FAIL
	 */
	int createCompressionFile(int ithread, int iframe);

	/**
	 * Static function - Starts Data Callback Thread of this object
	 * @param this_pointer pointer to this object
	 */
	static void* startDataCallbackThread(void *this_pointer);

	/**
	 * Static function - Starts Listening Thread of this object
	 * @param this_pointer pointer to this object
	 */
	static void* startListeningThread(void *this_pointer);

	/**
	 * Static function - Starts Writing Thread of this object
	 * @param this_pointer pointer to this object
	 */
	static void* startWritingThread(void *this_pointer);

	/**
	 * Thread that sends data packets to client
	 */
	void startDataCallback();

	/**
	 * Thread that listens to packets
	 * It pops the fifofree for free addresses, listens to packets and pushes them into the fifo
	 * This is continuously looped for each buffer in a nested loop, which is again looped for each acquisition
	 * Exits only for changing dynamic range, 10G parameters etc and recreated
	 *
	 */
	void startListening();

	/**
	 * Called by startListening
	 * Listens to buffer, until  packet(s) received or shutdownUDPsocket called by client
	 * Also copies carryovers from previous frame in front of buffer (gotthard and moench)
	 * For eiger, it ignores packets less than onePacketSize
	 * @param ithread listening thread index
	 * @param cSize number of bytes carried on from previous buffer
	 * @param temp temporary storage of previous buffer
	 * @return the number of bytes actually received
	 */
	int prepareAndListenBuffer(int ithread, int cSize, char* temp);

	/**
	 * Called by startListening
	 * Its called for the first packet of a scan or acquistion
	 * Sets the startframeindices and the variables to know if acquisition started
	 * @param ithread listening thread number
	 */
	void startFrameIndices(int ithread);

	/**
	 * Called by prepareAndListenBuffer
	 * This is called when udp socket is shut down by client
	 * It pushes ffff instead of packet number into fifo
	 * to inform writers about the end of listening session
	 * Then sets the listening mask so that it stops listening and wait for next acquisition trigger
	 * @param ithread listening thread number
	 * @param numbytes number of bytes received
	 */
	void stopListening(int ithread, int numbytes);

	/*
	 * Called by startListening for gotthard and moench to handle split frames
	 * It processes listening thread buffers by ensuring split frames are in the same buffer
	 * @param ithread listening thread index
	 * @param cSize number of bytes carried over to the next buffer to reunite with split frame
	 * @param temp temporary buffer to store the split frame
	 * @param rc number of bytes received
	 * @return packet count
	 */
	uint32_t processListeningBuffer(int ithread, int &cSize,char* temp, int rc);

	/**
	 * Thread started which writes packets to file.
	 * It calls popAndCheckEndofAcquisition to  pop fifo and check if it is a dummy end buffer
	 * It then calls a function to process and write packets to file and pushes the addresses into the fifoFree
	 * This is continuously looped for each buffer in a nested loop, which is again looped for each acquisition
	 * Exits only for changing dynamic range, 10G parameters etc and recreated
	 *
	 */
	void startWriting();

	/**
	 * Called by processWritingBuffer and processWritingBufferPacketByPacket
	 * When dummy-end buffers are popped from all FIFOs (acquisition over), this is called
	 * It frees the FIFO addresses, closes all files
	 * For data compression, it waits for all threads to be done
	 * Changes the status to RUN_FINISHED and prints statistics
	 * @param ithread writing thread index
	 * @param wbuffer writing buffer popped out from FIFO
	 */
	void stopWriting(int ithread, char* wbuffer);

	/**
	 * Called by processWritingBuffer and processWritingBufferPacketByPacket
	 * Updates parameters, (writes headers for eiger) and writes to file when not a dummy frame
	 * Copies data for gui display and frees addresses popped from FIFOs
	 * @param ithread writing thread index
	 * @param wbuffer writing buffer popped out from FIFO
	 * @param npackets number of packets
	 */
	void handleWithoutDataCompression(int ithread, char* wbuffer,uint32_t npackets);

	/**
	 * Called by processWritingBuffer  for jungfrau
	 * writes to dummy file, doesnt need to read packet numbers
	 * Copies data for gui display and frees addresses popped from FIFOs
	 * @param ithread writing thread index
	 * @param wbuffer writing buffer popped out from FIFO
	 * @param npackets number of packets
	 */
	void handleWithoutMissingPackets(int ithread, char* wbuffer,uint32_t npackets);

	/**
	 * Calle by handleWithoutDataCompression
	 * Creating headers Writing to file without compression
	 * @param ithread writer thread index
	 * @param wbuffer is the address of buffer popped out of FIFO
	 * @param numpackets is the number of packets
	 */
	void writeFileWithoutCompression(int ithread, char* wbuffer,uint32_t numpackets);

	/**
	 * Called by writeToFileWithoutCompression
	 * Create headers for file writing (at the moment, this is eiger specific)
	 * @param wbuffer writing buffer popped from FIFOs
	 */
	void createHeaders(char* wbuffer);

	/**
	 * Updates the file header char aray, each time the corresp parameter is changed
	 * @param ithread writer thread index
	 */
	void updateFileHeader(int ithread);

	/**
	 * Called by handleWithoutDataCompression and handleWithCompression after writing to file
	 * Copy frames for GUI and updates appropriate parameters for frequency frames to gui
	 * Uses semaphore for nth frame mode
	 * @param ithread writer thread index
	 * @param buffer buffer to copy
	 * @param numpackets number of packets to copy
	 */
	void copyFrameToGui(int ithread, char* buffer, uint32_t numpackets);

	void waitWritingBufferForNextAcquisition(int ithread);

	/**
	 * Called by processWritingBuffer
	 * Processing fifo popped buffers for data compression
	 * Updates parameters and writes to file
	 * Copies data for gui display and frees addresses popped from FIFOs
	 * @param ithread writing thread number
	 * @param wbuffer writer buffer
	 * @param nf number of frames
	 */
	void handleDataCompression(int ithread, char* wbuffer, uint64_t &nf);


	/**
	 * Get Frame Number
	 * @param ithread writer thread index
	 * @param wbuffer writer buffer
	 * @param framenumber reference to the frame number
	 * @param packetnumber reference to the packet number
	 * @param subframenumber reference to the subframe number
	 * @return OK or FAIL
	 */
	int getFrameandPacketNumber(int ithread, char* wbuffer, uint64_t &framenumber, uint32_t &packetnumber, uint32_t &subframenumber);

	/**
	 * Find offset upto this frame number and write it to file
	 * @param ithread writer thread index
	 * @param wbuffer writer buffer
	 * @param offset reference of offset to look from and replaces offset to starting of nextframenumber
	 * @param nextFrameNumber frame number up to which data written
	 * @param numpackets number of packets in buffer
	 * @param numPacketsWritten number of packets written to file
	 */
	int writeUptoFrameNumber(int ithread, char* wbuffer, int &offset, uint64_t nextFrameNumber, uint32_t numpackets, int &numPacketsWritten);

	/*************************************************************************
	 * Class Members *********************************************************
	 *************************************************************************/

	/** Maximum Number of Writer Threads */

#ifdef DCOMPRESS
	/**** most likely not used ***/
	const static int MAX_NUMBER_OF_WRITER_THREADS = 15;
#else
	const static int MAX_NUMBER_OF_WRITER_THREADS = 2;
#endif

	//**detector parameters***
	/*Detector Readout ID*/
	int detID;

	/** Size of 1 buffer processed at a time */
	int bufferSize;

	/** One Packet Size including headers */
	int onePacketSize;

	/** One Packet Size without headers */
	int oneDataSize;

	/** Frame Index Mask */
	uint64_t frameIndexMask;

	/** Frame Index Offset */
	int frameIndexOffset;

	/** Packet Index Mask */
	uint64_t packetIndexMask;

	/** Footer offset from start of Packet*/
	int footerOffset;


	//***File parameters***
#ifdef MYROOT1
	/** Tree where the hits are stored */
	TTree *myTree[MAX_NUMBER_OF_WRITER_THREADS];

	/** File where the tree is saved */
	TFile *myFile[MAX_NUMBER_OF_WRITER_THREADS];
#endif

	/** Complete File name */
	char completeFileName[MAX_NUMBER_OF_WRITER_THREADS][MAX_STR_LENGTH];

	/** File Name without frame index, file index and extension (_d0_f000000000000_8.raw)*/
	char fileNamePerThread[MAX_NUMBER_OF_WRITER_THREADS][MAX_STR_LENGTH];

		/** Maximum Frames Per File **/
	uint64_t maxFramesPerFile;

	/** If file created successfully for all Writer Threads */
	bool fileCreateSuccess;

	const static int FILE_HEADER_SIZE = 400;

	char fileHeader[MAX_NUMBER_OF_WRITER_THREADS][FILE_HEADER_SIZE];




	//***acquisition indices/count parameters***
	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t startAcquisitionIndex;

	/** Frame index at start of each real time acquisition (eg. for each scan) */
	uint64_t startFrameIndex;

	/** Actual current frame index of each time acquisition (eg. for each scan) */
	uint64_t frameIndex[MAX_NUMBER_OF_WRITER_THREADS];

	/** Current Frame Number */
	uint64_t currentFrameNumber[MAX_NUMBER_OF_WRITER_THREADS];

	/** Previous Frame number from buffer to calculate loss */
	int64_t frameNumberInPreviousFile[MAX_NUMBER_OF_WRITER_THREADS];

	/** Previous Frame number from last check to calculate loss */
	int64_t frameNumberInPreviousCheck[MAX_NUMBER_OF_WRITER_THREADS];
	/** total packet count from last check */
	int64_t totalWritingPacketCountFromLastCheck[MAX_NUMBER_OF_WRITER_THREADS];


	/* Acquisition started */
	bool acqStarted;

	/* Measurement started - for each thread to get progress print outs*/
	bool measurementStarted[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Total packet Count listened to by listening threads */
	int totalListeningPacketCount[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Total packet Count ignored by listening threads */
	int totalIgnoredPacketCount[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Pckets currently in current file, starts new file when it reaches max */
	int64_t lastFrameNumberInFile[MAX_NUMBER_OF_WRITER_THREADS];

	/** packets in current file */
	uint64_t totalPacketsInFile[MAX_NUMBER_OF_WRITER_THREADS];

	/**Total packet count written by each writing thread */
	uint64_t totalWritingPacketCount[MAX_NUMBER_OF_LISTENING_THREADS];




	//***receiver parameters***
	/** Receiver Buffer */
	char *buffer[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Memory allocated */
	char *mem0[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Circular fifo to point to addresses of data listened to */
	CircularFifo<char>* fifo[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Circular fifo to point to address already written and freed, to be reused */
	CircularFifo<char>* fifoFree[MAX_NUMBER_OF_LISTENING_THREADS];

	/** UDP Sockets - Detector to Receiver */
	genericSocket* udpSocket[MAX_NUMBER_OF_LISTENING_THREADS];

	/** File Descriptor */
	FILE *sfilefd[MAX_NUMBER_OF_WRITER_THREADS];

	/** Number of Jobs Per Buffer */
	int numberofJobsPerBuffer;

	/** Total fifo size */
	uint32_t fifoSize;

	/** fifo buffer header size */
	uint32_t fifoBufferHeaderSize;

	/** Missing Packet  */
	int missingPacketinFile;

	/** Dummy Packet identifier value */
	const static uint32_t dummyPacketValue = 0xFFFFFFFF;



	//***receiver to GUI parameters***
	/** Current Frame copied for GUI */
	char* latestData[MAX_NUMBER_OF_WRITER_THREADS];

	/** Pointer to file name to be sent to GUI */
	char guiFileName[MAX_NUMBER_OF_WRITER_THREADS][MAX_STR_LENGTH];

	/** Number of packets copied to be sent to gui (others padded) */
	int guiNumPackets[MAX_NUMBER_OF_WRITER_THREADS];

	/** Semaphore to synchronize Writer and GuiReader threads*/
	sem_t writerGuiSemaphore[MAX_NUMBER_OF_WRITER_THREADS]; //datacompression, only first thread sends to gui

	/** Semaphore to synchronize Writer and GuiReader threads*/
	sem_t dataCallbackWriterSemaphore[MAX_NUMBER_OF_WRITER_THREADS]; //datacompression, only first thread sends to gui

	/** counter for nth frame to gui */
	int frametoGuiCounter[MAX_NUMBER_OF_WRITER_THREADS];



	//***data call back thread parameters***
	/** Ensures if zmq threads created successfully */
	bool zmqThreadStarted;

	/** Number of data callback Threads */
	int numberofDataCallbackThreads;

	/** Data Callback Threads */
	pthread_t dataCallbackThreads[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Semaphores Synchronizing DataCallback Threads */
	sem_t dataCallbackSemaphore[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Mask with each bit indicating status of each data callback thread  */
	volatile uint32_t dataCallbackThreadsMask;

	/** Set to self-terminate data callback threads waiting for semaphores */
	bool killAllDataCallbackThreads;



	//***general and listening thread parameters***
	/** Ensures if threads created successfully */
	bool threadStarted;

	/** Current Thread Index*/
	int currentThreadIndex;

	/** Number of Listening Threads */
	int numberofListeningThreads;

	/** Listening Threads */
	pthread_t listeningThreads[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Semaphores Synchronizing Listening Threads */
	sem_t listenSemaphore[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Mask with each bit indicating status of each listening thread  */
	volatile uint32_t listeningThreadsMask;

	/** Set to self-terminate listening threads waiting for semaphores */
	bool killAllListeningThreads;



	//***writer thread parameters***
	/** Number of Writer Threads */
	int numberofWriterThreads;

	/** Writer Threads */
	pthread_t writingThreads[MAX_NUMBER_OF_WRITER_THREADS];

	/** Semaphores Synchronizing Writer Threads */
	sem_t writerSemaphore[MAX_NUMBER_OF_WRITER_THREADS];

	/** Mask with each bit indicating status of each writer thread */
	volatile uint32_t writerThreadsMask;

	/** Mask with each bit indicating file created for each writer thread*/
	volatile uint32_t createFileMask;

	/** Set to self-terminate writer threads waiting for semaphores */
	bool killAllWritingThreads;


	//***deactivated parameters***
	uint64_t deactivated_framenumber[MAX_NUMBER_OF_LISTENING_THREADS];
	uint32_t deactivated_packetnumber[MAX_NUMBER_OF_LISTENING_THREADS];

	//***deactivated parameters***
	uint64_t deactivatedFrameNumber[MAX_NUMBER_OF_LISTENING_THREADS];
	int deactivatedFrameIncrement;



	//***filter parameters***
	/** Common Mode Subtraction Enable FIXME: Always false, only moench uses, Ask Anna */
	bool commonModeSubtractionEnable;

	/** Moench Common Mode Subtraction */
	moenchCommonMode *moenchCommonModeSubtraction;

	/** Single Photon Detector Object for each writer thread */
	singlePhotonDetector<uint16_t> *singlePhotonDetectorObject[MAX_NUMBER_OF_WRITER_THREADS];

	/** Receiver Data Object for each writer thread */
	slsReceiverData<uint16_t>  *receiverData[MAX_NUMBER_OF_WRITER_THREADS];




	//***mutex***
	/** Status mutex */
	pthread_mutex_t statusMutex;

	/** Writing mutex */
	pthread_mutex_t writeMutex;

	/** GuiDataReady Mutex */
	pthread_mutex_t  dataReadyMutex;

	/** Progress (currentFrameNumber) Mutex  */
	pthread_mutex_t progressMutex;


	//***callback***
	/** The action which decides what the user and default responsibilities to save data are
	 * 0 raw data ready callback takes care of open,close,write file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything */
	int cbAction;

};


#endif

//#endif
