//#ifdef UDP_BASE_IMPLEMENTATION
#ifndef UDP_STANDARD_IMPLEMENTATION_H
#define UDP_STANDARD_IMPLEMENTATION_H
/********************************************//**
 * @file UDPBaseImplementation.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/

#include "UDPBaseImplementation.h"

//#include "sls_receiver_defs.h"
//#include "receiver_defs.h"
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
	 * They access local cache of configuration or detector parameters *******
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

	//***acquisition count parameters***


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
	 */
	void setDataCompressionEnable(const bool b);

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
	int setDetectorType(const slsReceiverDefs::detectorType d);

	//***acquisition functions***
	/**
	 * Overridden method
	 * Reset acquisition parameters such as total frames caught for an entire acquisition (including all scans)
	 */
	void resetAcquisitionCount();

	/**
	 * Overridden method
	 * Start Listening for Packets by activating all configuration settings to receiver
	 * @param c error message if FAIL
	 * @return OK or FAIL
	 */
	int startReceiver(char *c=NULL);



private:

	/*************************************************************************
	 * Setters ***************************************************************
	 * They modify the local cache of configuration or detector parameters ***
	 *************************************************************************/
	//**initial parameters***

	/**
	 * Delete and free base member parameters
	 */
    void deleteBaseMembers();

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
	 * Set up the Fifo Structure for processing buffers
	 * between listening and writer threads
	 * @return OK or FAIL
	 */
	int setupFifoStructure();

	/**
	 * Creates UDP Sockets
	 * @return OK or FAIL
	 */
	int createUDPSockets();



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
	/** Maximum Packets Per File **/
	int maxPacketsPerFile;



	//***acquisition indices parameters***
	/** Frame Number of First Frame of an Acquisition */
	uint64_t startAcquisitionIndex;

	/** Frame index at start of each real time acquisition (eg. for each scan) */
	uint64_t startFrameIndex;

	/** Current Frame Number */
	uint64_t currentFrameNumber;

	/* Acquisition started */
	bool acqStarted;

	/* Measurement started */
	bool measurementStarted;

	/** Total Frame Count listened to by listening threads */
	int totalListeningFrameCount[MAX_NUMBER_OF_LISTENING_THREADS];




	//***receiver parameters***
	/** Receiver Buffer */
	char *buffer[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Memory allocated */
	char *mem0[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Circular fifo to point to addresses of data listened to */
	CircularFifo<char>* fifo[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Circular fifo to point to address already written and freed, to be reused */
	CircularFifo<char>* fifoFree[MAX_NUMBER_OF_LISTENING_THREADS];

	/** Number of Jobs Per Buffer */
	int numberofJobsPerBuffer;

	/** Fifo Depth */
	uint32_t fifoSize;

	/** Current Frame copied for Gui */
	char* latestData;



	//***general and listening thread parameters***
	/** Ensures if threads created successfully */
	bool threadStarted;

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

	/** Current Writer Thread Index*/
	int currentWriterThreadIndex;

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
	/** mutex for status */
	pthread_mutex_t status_mutex;


























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
	 * Returns if acquisition started
	 */
	//bool getAcquistionStarted();

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
	void setDetectorHostname(const char *detectorHostName);



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
	 * @param startAcquisitionIndex is the start index of the acquisition
	 * @param startFrameIndex is the start index of the scan
	 */
	void readFrame(char* c,char** raw, uint32_t &startAcquisitionIndex, uint32_t &startFrameIndex);

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
	 * Copy frames to gui
	 * uses semaphore for nth frame mode
	 */
	void copyFrameToGui(char* startbuf[], char* buf=NULL);


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
	void writeToFile_withoutCompression(char* buf[],int numpackets, uint32_t framenum);

	/**
	 * Its called for the first packet of a scan or acquistion
	 * Sets the startframeindices and the variables to know if acquisition started
	 * @param ithread listening thread number
	 * @param numbytes number of bytes it listened to
	 */
	void startFrameIndices(int ithread, int numbytes);

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
	 */
	void stopWriting(int ithread, char* wbuffer[]);

	/**
	 * updates parameters and writes to file when not a dummy frame
	 * Also calls writeToFile_withoutCompression or handleDataCompression
	 * Called by startWriting()
	 * @param ithread writing thread number
	 * @param wbuffer writer buffer
	 * @param npackets number of packets
	 */
	void handleWithoutDataCompression(int ithread, char* wbuffer[],int npackets);

	/**
	 * data compression for each fifo output
	 * @param ithread writing thread number
	 * @param wbuffer writer buffer
	 * @param data pointer to the next packet start
	 * @param xmax max pixels in x direction
	 * @param ymax max pixels in y direction
	 * @param nf nf
	 */
	void handleDataCompression(int ithread, char* wbuffer[], char* data, int xmax, int ymax, int &nf);


























	/** missing packet identifier value */
	const static uint16_t missingPacketValue = 0xFFFF;


	/** UDP Socket between Receiver and Detector */
	genericSocket* udpSocket[MAX_NUM_LISTENING_THREADS];

	/** Complete File name */
	char savefilename[MAX_STR_LENGTH];

	/** Actual current frame index of each time acquisition (eg. for each scan) */
	uint32_t frameIndex;

	/** Pckets currently in current file, starts new file when it reaches max */
	uint32_t packetsInFile;

	/** Number of missing packets in acquisition*/
	uint32_t numTotMissingPackets;

	/** Number of missing packets in file (sometimes packetsinFile is incorrect due to padded packets for eiger)*/
	uint32_t numTotMissingPacketsInFile;

	/** Number of missing packets per buffer*/
	uint32_t numMissingPackets;

	/** Previous Frame number from buffer */
	int prevframenum;


	/** gui data ready */
	int guiDataReady;

	/** points to the data to send to gui */
	char* guiData;

	/** points to the filename to send to gui */
	char* guiFileName;

/** OK if file created was successful */
	int ret_createfile;

	// TODO: not properly sure where to put these...
	/** structure of an eiger image header*/




//semaphores
	/** semaphore to synchronize  writer and guireader threads */
	sem_t smp;

//mutex
	/** guiDataReady mutex */
	pthread_mutex_t  dataReadyMutex;

	/** mutex for progress variable currframenum */
	pthread_mutex_t progress_mutex;

	/** mutex for writing data to file */
	pthread_mutex_t write_mutex;

	/** File Descriptor */
	FILE *sfilefd;

	//filter


#ifdef MYROOT1
	/** Tree where the hits are stored */
	TTree *myTree[MAX_NUM_WRITER_THREADS];

	/** File where the tree is saved */
	TFile *myFile[MAX_NUM_WRITER_THREADS];
#endif


	/** The action which decides what the user and default responsibilites to save data are
	 * 0 raw data ready callback takes care of open,close,write file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything */
	int cbAction;


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
