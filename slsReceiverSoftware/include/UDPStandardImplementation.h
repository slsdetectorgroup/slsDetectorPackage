//#ifdef UDP_BASE_IMPLEMENTATION
#ifndef UDP_STANDARD_IMPLEMENTATION_H
#define UDP_STANDARD_IMPLEMENTATION_H
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
class FileWriter;

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

	//*** initial parameters (behavioral)***
	/**
	 * Set receiver type (and corresponding detector variables in derived STANDARD class)
	 * It is the first function called by the client when connecting to receiver
	 * @param d detector type
	 * @return OK or FAIL
	 */
	int setDetectorType(const detectorType d);


	//*** Getters ***
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



	//*** Setters ***

	//*** file parameters ***
	/**
	 * Set File Name Prefix (without frame index, file index and extension (_f000000000000_8.raw))
	 * Does not check for file existence since it is created only at startReceiver
	 * @param c file name (max of 1000 characters)
	 */
	void setFileName(const char c[]);

	//*** acquisition parameters ***
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

	//*** Behavioral functions ***



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
	 * Set up the Fifo Structure for processing buffers
	 * between listening and dataprocessor threads
	 * @return OK or FAIL
	 */
	int SetupFifoStructure();


	//*** Class Members ***


	//*** detector parameters ***
	/*Detector Readout ID*/
	int detID;

	//*** receiver parameters ***
	/** Number of Threads */
	int numThreads;

	/** Number of Jobs */
	int numberofJobs;


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

	/** File writer implemented as binary or hdf5 filewriter */
	std::vector <FileWriter*> fileWriter;
};


#endif

//#endif
