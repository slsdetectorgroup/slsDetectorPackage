#pragma once
/************************************************
 * @file DataProcessor.h
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/
/**
 *@short creates & manages a data processor thread each
 */

#include "ThreadObject.h"

class GeneralData;
class Fifo;
class File;
class DataStreamer;

#include <vector>

class DataProcessor : private virtual slsReceiverDefs, public ThreadObject {
	
 public:
	/**
	 * Constructor
	 * Calls Base Class CreateThread(), sets ErrorMask if error and increments NumberofDataProcessors
	 * @param f address of Fifo pointer
	 * @param ftype pointer to file format type
	 * @param fwenable pointer to file writer enable
	 * @param dsEnable pointer to data stream enable
	 * @param cbaction pointer to call back action
	 * @param dataReadycb pointer to data ready call back function
	 * @param pDataReadycb pointer to arguments of data ready call back function
	 */
	DataProcessor(Fifo*& f, fileFormat* ftype, bool* fwenable, bool* dsEnable,
						int* cbaction,
						void (*dataReadycb)(int, char*, int, FILE*, char*, void*),
						void *pDataReadycb);

	/**
	 * Destructor
	 * Calls Base Class DestroyThread() and decrements NumberofDataProcessors
	 */
	~DataProcessor();


	//*** static functions ***
	/**
	 * Get ErrorMask
	 * @return ErrorMask
	 */
	static uint64_t GetErrorMask();

	/**
	 * Get RunningMask
	 * @return RunningMask
	 */
	static uint64_t GetRunningMask();

	/**
	 * Reset RunningMask
	 */
	static void ResetRunningMask();

	//*** non static functions ***
	//*** getters ***
	/**
	 * Get acquisition started flag
	 * @return acquisition started flag
	 */
	bool GetAcquisitionStartedFlag();

	/**
	 * Get measurement started flag
	 * @return measurement started flag
	 */
	bool GetMeasurementStartedFlag();

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


	//*** setters ***
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
	 * Set GeneralData pointer to the one given
	 * @param g address of GeneralData (Detector Data) pointer
	 */
	void SetGeneralData(GeneralData* g);

	/**
	 * Set thread priority
	 * @priority priority
	 * @returns OK or FAIL
	 */
	int SetThreadPriority(int priority);

	/**
	 * Set File Format
	 * @param f file format
	 */
	void SetFileFormat(const fileFormat fs);

	/**
	 * Set up file writer object and call backs
	 * @param nd pointer to number of detectors in each dimension
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param frindexenable pointer to frame index enable
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of images in acquisition
	 * @param dr dynamic range
	 * @param g address of GeneralData (Detector Data) pointer
	 */
	void SetupFileWriter(int* nd, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, GeneralData* g = 0);


	/**
	 * Create New File
	 * @param en ten giga enable
	 * @param nf number of frames
	 * @param at acquisition time
	 * @param ap acquisition period
	 * @returns OK or FAIL
	 */
	int CreateNewFile(bool en, uint64_t nf, uint64_t at, uint64_t ap);

	/**
	 * Closes files
	 */
	void CloseFiles();



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
	 * Record First Indices (firstAcquisitionIndex, firstMeasurementIndex)
	 * @param fnum frame index to record
	 */
	void RecordFirstIndices(uint64_t fnum);

	/**
	 * Destroy file writer object
	 * @return OK or FAIL
	 */
	void DestroyFileWriter();

	/**
	 * Thread Exeution for DataProcessor Class
	 * Pop bound addresses, process them,
	 * write to file if needed & free the address
	 */
	void ThreadExecution();

	/**
	 * Frees dummy buffer,
	 * reset running mask by calling StopRunning()
	 * @param buf address of pointer
	 */
	void StopProcessing(char* buf);

	/**
	 * Process an image popped from fifo,
	 * write to file if fw enabled & update parameters
	 * @param buffer
	 */
	void ProcessAnImage(char* buf);



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

	/** GeneralData (Detector Data) object */
	const GeneralData* generalData;

	/** Fifo structure */
	Fifo* fifo;

	/** Data Stream Enable */
	bool* dataStreamEnable;

	/** Aquisition Started flag */
	bool acquisitionStartedFlag;

	/** Measurement Started flag */
	bool measurementStartedFlag;

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

	/** File writer implemented as binary or hdf5 File */
	File* file;

	/** File Format Type */
	fileFormat* fileFormatType;

	/** File Write Enable */
	bool* fileWriteEnable;


	//***callback parameters***
	/** Pointer to the action which decides what the user and default responsibilities to save data are
	 * 0 raw data ready callback takes care of open,close,write file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything */
	int* callbackAction;

	/**
	 * function being called back for raw data
	 * args to raw data ready callback are
	 * framenum
	 * datapointer
	 * datasize in bytes
	 * file descriptor
	 * guidatapointer (NULL, no data required)
	 */
	void (*rawDataReadyCallBack)(int, char*, int, FILE*, char*, void*);
	void *pRawDataReady;




};

