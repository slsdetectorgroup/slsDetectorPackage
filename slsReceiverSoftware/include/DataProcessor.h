/************************************************
 * @file DataProcessor.h
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/
#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H
/**
 *@short creates & manages a data processor thread each
 */

#include "ThreadObject.h"

class Fifo;

class DataProcessor : private virtual slsReceiverDefs, public ThreadObject {
	
 public:
	/**
	 * Constructor
	 * Calls Base Class CreateThread(), sets ErrorMask if error and increments NumberofDataProcessors
	 * @param f address of Fifo pointer
	 */
	DataProcessor(Fifo*& f);

	/**
	 * Destructor
	 * Calls Base Class DestroyThread() and decrements NumberofDataProcessors
	 */
	~DataProcessor();


	/**
	 * Get RunningMask
	 * @return RunningMask
	 */
	static uint64_t GetErrorMask();

	/**
	 * Get acquisition started flag
	 * @return acquisition started flag
	 */
	static bool GetAcquisitionStartedFlag();

	/**
	 * Get measurement started flag
	 * @return measurement started flag
	 */
	static bool GetMeasurementStartedFlag();

	/**
	 * Get Total Complete Frames Caught for an entire acquisition (including all scans)
	 * @return total number of frames caught for entire acquisition
	 */
	uint64_t GetNumTotalFramesCaught();

	/**
	 * Get Frames Complete Caught for each real time acquisition (eg. for each scan)
	 * @return number of frames caught for each scan
	 */
	uint64_t GetNumFramesCaught();

	/**
	 * Get Current Frame Index thats been processed for an entire  acquisition (including all scans)
	 * @return -1 if no frames have been caught, else current frame index (represents all scans too)
	 */
	uint64_t GetProcessedAcquisitionIndex();

	/**
	 * Set bit in RunningMask to allow thread to run
	 */
	void StartRunning();

	/**
	 * Reset bit in RunningMask to prevent thread from running
	 */
	void StopRunning();

	/**
	 * Set Fifo pointer to the one given
	 * @param f address of Fifo pointer
	 */
	void SetFifo(Fifo*& f);

	/**
	 * Reset parameters for new acquisition (including all scans)
	 */
	void ResetParametersforNewAcquisition();

	/**
	 * Reset parameters for new measurement (eg. for each scan)
	 */
	void ResetParametersforNewMeasurement();

	/**
	 * Create New File
	 */
	int CreateNewFile();

	/**
	 * Closes file
	 */
	void CloseFile();

 private:

	/**
	 * Get Type
	 * @return type
	 */
	std::string GetType();

	/**
	 * Returns if the thread is currently running
	 * @returns true if thread is running, else false
	 */
	bool IsRunning();

	/**
	 * Thread Exeution for DataProcessor Class
	 * Pop bound addresses, process them,
	 * write to file if needed & free the address
	 */
	void ThreadExecution();



	/** type of thread */
	static const std::string TypeName;

	/** Total Number of DataProcessor Objects */
	static int NumberofDataProcessors;

	/** Mask of errors on any object eg.thread creation */
	static uint64_t ErrorMask;

	/** Mask of all listener objects running */
	static uint64_t RunningMask;

	/** mutex to update static items among objects (threads)*/
	static pthread_mutex_t Mutex;

	/** Fifo structure */
	Fifo* fifo;


	// individual members
	/** Aquisition Started flag */
	static bool acquisitionStartedFlag;

	/** Measurement Started flag */
	static bool measurementStartedFlag;

	/**Number of complete frames caught for an entire acquisition (including all scans) */
	uint64_t numTotalFramesCaught;

	/** Number of complete frames caught for each real time acquisition (eg. for each scan) */
	uint64_t numFramesCaught;

	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t firstAcquisitionIndex;

	/** Frame Number of First Frame for each real time acquisition (eg. for each scan) */
	uint64_t firstMeasurementIndex;

	/** Frame Number of latest processed frame number of an entire Acquisition (including all scans) */
	uint64_t currentFrameIndex;

};

#endif
