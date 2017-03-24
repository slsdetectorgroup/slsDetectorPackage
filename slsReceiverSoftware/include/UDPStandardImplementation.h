#pragma once
/********************************************//**
 * @file UDPBaseImplementation.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/
/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */
#include "UDPBaseImplementation.h"

class GeneralData;
class Listener;
class DataProcessor;
class DataStreamer;
class Fifo;

#include <vector>


class UDPStandardImplementation: private virtual slsReceiverDefs, public UDPBaseImplementation {
 public:


	//*** cosntructor & destructor ***
	/**
	 * Constructor
	 */
	UDPStandardImplementation();

	/**
	 * Destructor
	 */
	virtual ~UDPStandardImplementation();



	//*** Overloaded Functions called by TCP Interface ***
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

	/**
	 * Get Current Frame Index for an entire  acquisition (including all scans)
	 * @return -1 if no frames have been caught, else current frame index (represents all scans too)
	 */
	int64_t getAcquisitionIndex() const;

	/**
	 * Set File Format
	 * @param f fileformat binary or hdf5
	 */
	void setFileFormat(slsReceiverDefs::fileFormat f);

	/**
	 * Set Short Frame Enabled, later will be moved to getROI (so far only for gotthard)
	 * @param i index of adc enabled, else -1 if all enabled
	 * @return OK or FAIL
	 */
	int setShortFrameEnable(const int i);

	/**
	 * Set the Frequency of Frames Sent to GUI
	 * @param freq 0 for random frame requests, n for nth frame frequency
	 * @return OK or FAIL
	 */
	int setFrameToGuiFrequency(const uint32_t freq);

	/**
	 * Set the data stream enable
	 * @param enable data stream enable
	 * @return OK or FAIL
	 */
	int setDataStreamEnable(const bool enable);

	/**
	 * Set Acquisition Period
	 * @param i acquisition period
	 * @return OK or FAIL
	 */
	int setAcquisitionPeriod(const uint64_t i);

	/**
	 * Set Acquisition Time
	 * @param i acquisition time
	 * @return OK or FAIL
	 */
	int setAcquisitionTime(const uint64_t i);

	/**
	 * Set Number of Frames expected by receiver from detector
	 * The data receiver status will change from running to idle when it gets this number of frames
	 * @param i number of frames expected
	 * @return OK or FAIL
	 */
	int setNumberOfFrames(const uint64_t i);

	/**
	 * Set Dynamic Range or Number of Bits Per Pixel
	 * @param i dynamic range that is 4, 8, 16 or 32
	 * @return OK or FAIL
	 */
	int setDynamicRange(const uint32_t i);

	/**
	 * Set Ten Giga Enable
	 * @param b true if 10GbE enabled, else false (1G enabled)
	 * @return OK or FAIL
	 */
	int setTenGigaEnable(const bool b);

	/**
	 * Set Fifo Depth
	 * @param i fifo depth value
	 * @return OK or FAIL
	 */
	int setFifoDepth(const uint32_t i);

	/**
	 * Set receiver type (and corresponding detector variables in derived STANDARD class)
	 * It is the first function called by the client when connecting to receiver
	 * @param d detector type
	 * @return OK or FAIL
	 */
	int setDetectorType(const detectorType d);

	/**
	 * Set detector position id and construct filewriter
	 * @param i position id
	 */
	void setDetectorPositionId(const int i);

	/**
	 * Reset acquisition parameters such as total frames caught for an entire acquisition (including all scans)
	 */
	void resetAcquisitionCount();

	/**
	 * Start Listening for Packets by activating all configuration settings to receiver
	 * When this function returns, it has status RUNNING(upon SUCCESS) or IDLE (upon failure)
	 * @param c error message if FAIL
	 * @return OK or FAIL
	 */
	int startReceiver(char *c=NULL);

	/**
	 * Stop Listening for Packets
	 * Calls startReadout(), which stops listening and sets status to Transmitting
	 * When it has read every frame in buffer, the status changes to Run_Finished
	 * When this function returns, receiver has status IDLE
	 * Pre: status is running, semaphores have been instantiated,
	 * Post: udp sockets shut down, status is idle, semaphores destroyed
	 */
	void stopReceiver();

	/**
	 * Stop Listening to Packets
	 * and sets status to Transmitting
	 * Next step would be to get all data and stop receiver completely and return with idle state
	 * Pre: status is running, udp sockets have been initialized, stop receiver initiated
	 * Post:udp sockets closed, status is transmitting
	 */
	void startReadout();

	/**
	 * Shuts down and deletes UDP Sockets
	 * also called in case of illegal shutdown of receiver
	 */
	void shutDownUDPSockets();

	/**
	 * Closes file / all files(data compression involves multiple files)
	 * TCPIPInterface can also call this in case of illegal shutdown of receiver
	 */
	void closeFiles();


private:

    /**
	 * Delete and free member parameters
	 */
    void DeleteMembers();

	/**
	 * Initialize member parameters
	 */
	void InitializeMembers();

	/**
	 * Sets local network parameters, but requires permissions
	 */
	void SetLocalNetworkParameters();

	/**
	 * Set Thread Priorities
	 */
	void SetThreadPriorities();

	/**
	 * Set up the Fifo Structure for processing buffers
	 * between listening and dataprocessor threads
	 * @return OK or FAIL
	 */
	int SetupFifoStructure();

	/**
	 * Reset parameters for new measurement (eg. for each scan)
	 */
	void ResetParametersforNewMeasurement();

	/**
	 * Creates UDP Sockets
	 * @return OK or FAIL
	 */
	int CreateUDPSockets();

	/**
	 * Creates the first file
	 * also does the startAcquisitionCallBack
	 * @return OK or FAIL
	 */
	int SetupWriter();

	/**
	 * Start Running
	 * Set running mask and post semaphore of the threads
	 * to start the inner loop in execution thread
	 */
	void StartRunning();



	//*** Class Members ***


	//*** receiver parameters ***
	/** Number of Threads */
	int numThreads;

	/** Number of Jobs */
	int numberofJobs;

	//*** mutex ***
	/** Status mutex */
	pthread_mutex_t statusMutex;


	//** class objects ***
	/** General Data Properties */
	GeneralData* generalData;

	/** Listener Objects that listen to UDP and push into fifo */
	std::vector <Listener*> listener;

	/** DataProcessor Objects that pull from fifo and process data */
	std::vector <DataProcessor*> dataProcessor;

	/** DataStreamer Objects that stream data via ZMQ */
	std::vector <DataStreamer*> dataStreamer;

	/** Fifo Structure to store addresses of memory writes */
	std::vector <Fifo*> fifo;

};

