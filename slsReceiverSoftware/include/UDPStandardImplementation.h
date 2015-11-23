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
	 * Overridden method
	 * Set data compression, by saving only hits (so far implemented only for Moench and Gotthard)
	 * @param b true for data compression enable, else false
	 * @return OK or FAIL
	 */
	int setDataCompressionEnable(const bool b);

	//***acquisition parameters***
	/**
	 * Overridden method
	 * Set Short Frame Enabled, later will be moved to getROI (so far only for gotthard)
	 * @param i index of adc enabled, else -1 if all enabled
	 */
	void setShortFrameEnable(const int i);

	/**
	 * Overridden method
	 * Set the Frequency of Frames Sent to GUI
	 * @param i 0 for random frame requests, n for nth frame frequency
	 * @return OK or FAIL
	 */
	int setFrameToGuiFrequency(const uint32_t i);

	/**
	 * Overridden method
	 * Set Acquisition Period
	 * @param i acquisition period
	 * @return OK or FAIL
	 */
	int setAcquisitionPeriod(const uint64_t i);

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
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 * @param startAcq start index of the acquisition
	 * @param startFrame start index of the scan
	 */
	void readFrame(char* c,char** raw, uint64_t &startAcq, uint64_t &startFrame);

	/**
	 * Overridden method
	 * Closes file / all files(data compression involves multiple files)
	 * TCPIPInterface can also call this in case of illegal shutdown of receiver
	 * @param i thread index valid for datacompression using root files, -1 for all threads
	 */
	void closeFile(int i = -1);

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
	 * @return OK or FAIL
	 */
	int setupFifoStructure();



	/*************************************************************************
	 * Listening and Writing Threads *****************************************
	 *************************************************************************/

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
	 * @return OK or FAIL
	 */
	int createNewFile();

	/**
	 * Creates new tree and file for compression
	 * @param ithread thread number
	 * @param iframe frame number
	 * @return OK or FAIL
	 */
	int createCompressionFile(int ithread, int iframe);

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
	 * @param lSize number of bytes to listen to
	 * @param cSize number of bytes carried on from previous buffer
	 * @param temp temporary storage of previous buffer
	 * @return the number of bytes actually received
	 */
	int prepareAndListenBuffer(int ithread, int lSize, int cSize, char* temp);

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
	 * @return packet count
	 */
	uint32_t processListeningBuffer(int ithread, int cSize,char* temp);

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
	 * Pops buffer from all the FIFOs and checks for dummy frames and end of acquisition
	 * @param ithread current thread index
	 * @param wbuffer the buffer array that is popped from all the FIFOs
	 * @param ready if that FIFO is allowed to pop (depends on if dummy buffer already popped/ waiting for other FIFO to finish a frame(eiger))
	 * @param nP number of packets in the buffer popped out
	 * @param fifoTempFree circular fifo to save addresses of packets adding upto a frame before pushing into fifofree (eiger specific)
	 * @return true if end of acquisition else false
	 */
	bool popAndCheckEndofAcquisition(int ithread, char* wbuffer[], bool ready[], uint32_t nP[],CircularFifo<char>* fifoTempFree[]);

	/**
	 * Called by processWritingBuffer and processWritingBufferPacketByPacket
	 * When dummy-end buffers are popped from all FIFOs (acquisition over), this is called
	 * It frees the FIFO addresses, closes all files
	 * For data compression, it waits for all threads to be done
	 * Changes the status to RUN_FINISHED and prints statistics
	 * @param ithread writing thread index
	 * @param wbuffer writing buffer popped out from FIFO
	 */
	void stopWriting(int ithread, char* wbuffer[]);

	/**
	 * Called by processWritingBuffer and processWritingBufferPacketByPacket
	 * Updates parameters, (writes headers for eiger) and writes to file when not a dummy frame
	 * Copies data for gui display and frees addresses popped from FIFOs
	 * @param ithread writing thread index
	 * @param wbuffer writing buffer popped out from FIFO
	 * @param npackets number of packets
	 */
	void handleWithoutDataCompression(int ithread, char* wbuffer[],uint32_t npackets);

	/**
	 * Calle by handleWithoutDataCompression
	 * Creating headers Writing to file without compression
	 * @param wbuffer is the address of buffer popped out of FIFO
	 * @param numpackets is the number of packets
	 */
	void writeFileWithoutCompression(char* wbuffer[],uint32_t numpackets);

	/**
	 * Called by writeToFileWithoutCompression
	 * Create headers for file writing (at the moment, this is eiger specific)
	 * @param wbuffer writing buffer popped from FIFOs
	 */
	void createHeaders(char* wbuffer[]);

	/**
	 * Called by handleWithoutDataCompression and handleWithCompression after writing to file
	 * Copy frames for GUI and updates appropriate parameters for frequency frames to gui
	 * Uses semaphore for nth frame mode
	 * @param buffer buffer to copy
	 */
	void copyFrameToGui(char* buffer[]);

	void processWritingBuffer(int ithread);

	void processWritingBufferPacketByPacket(int ithread);

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
	void handleDataCompression(int ithread, char* wbuffer[], uint64_t &nf);



	/*************************************************************************
	 * Class Members *********************************************************
	 *************************************************************************/

	//**detector parameters***
	/**
	 * structure of an eiger packet header
	 * subframenum subframe number for 32 bit mode (already written by firmware)
	 * missingpacket explicitly put to 0xFF to recognize it in file read (written by software)
	 * portnum 0 for the first port and 1 for the second port (written by software to file)
	 * dynamicrange dynamic range or bits per pixel (written by software to file)
	 */
	typedef struct {
		unsigned char subFameNumber[4];
		unsigned char missingPacket[2];
		unsigned char portIndex[1];
		unsigned char dynamicRange[1];
	} eiger_packet_header_t;
	/**
	 * structure of an eiger packet footer
	 * framenum 48 bit frame number (already written by firmware)
	 * packetnum packet number (already written by firmware)
	 */
	typedef struct	{
		unsigned char frameNumber[6];
		unsigned char packetNumber[2];
	} eiger_packet_footer_t;

	/** Size of 1 Frame including headers */
	int frameSize;

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
	char completeFileName[MAX_STR_LENGTH];

		/** Maximum Packets Per File **/
	int maxPacketsPerFile;

	/** If file created successfully for all Writer Threads */
	bool fileCreateSuccess;




	//***acquisition indices/count parameters***
	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t startAcquisitionIndex;

	/** Frame index at start of each real time acquisition (eg. for each scan) */
	uint64_t startFrameIndex;

	/** Actual current frame index of each time acquisition (eg. for each scan) */
	uint64_t frameIndex;

	/** Current Frame Number */
	uint64_t currentFrameNumber;

	/** Previous Frame number from buffer to calculate loss */
	uint64_t previousFrameNumber;

	/* Acquisition started */
	bool acqStarted;

	/* Measurement started */
	bool measurementStarted;

	/** Total Frame Count listened to by listening threads */
	int totalListeningFrameCount[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Pckets currently in current file, starts new file when it reaches max */
	uint32_t packetsInFile;

	/** Number of Missing Packets per buffer*/
	uint32_t numMissingPackets;

	/** Total Number of Missing Packets in acquisition*/
	uint32_t numTotMissingPackets;

	/** Number of Missing Packets in file */
	uint32_t numTotMissingPacketsInFile;





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
	FILE *sfilefd;

	/** Number of Jobs Per Buffer */
	int numberofJobsPerBuffer;

	/** Total fifo size */
	uint32_t fifoSize;

	/** Missing Packet identifier value */
	const static uint16_t missingPacketValue = 0xFFFF;

	/** Dummy Packet identifier value */
	const static uint32_t dummyPacketValue = 0xFFFFFFFF;

	//***receiver to GUI parameters***
	/** Current Frame copied for GUI */
	char* latestData;

	/** If Data to be sent to GUI is ready */
	bool guiDataReady;

	/** Pointer to data to be sent to GUI */
	char* guiData;

	/** Pointer to file name to be sent to GUI */
	char guiFileName[MAX_STR_LENGTH];

	/** Semaphore to synchronize Writer and GuiReader threads*/
	sem_t writerGuiSemaphore;





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

	/** Current Listening Thread Index*/
	int currentListeningThreadIndex;

	/** Mask with each bit indicating status of each listening thread  */
	volatile uint32_t listeningThreadsMask;

	/** Set to self-terminate listening threads waiting for semaphores */
	bool killAllListeningThreads;



	//***writer thread parameters***
	/** Maximum Number of Writer Threads */
	const static int MAX_NUMBER_OF_WRITER_THREADS = 15;

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
