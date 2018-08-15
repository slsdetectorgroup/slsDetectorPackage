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
     * @param ind self index
     * @param dtype detector type
	 * @param f address of Fifo pointer
	 * @param ftype pointer to file format type
	 * @param fwenable file writer enable
	 * @param dsEnable pointer to data stream enable
	 * @param gpEnable pointer to gap pixels enable
	 * @param dr pointer to dynamic range
	 * @param freq pointer to streaming frequency
	 * @param timer pointer to timer if streaming frequency is random
	 * @param fp pointer to frame padding enable
	 * @param act pointer to activated
	 * @param depaden pointer to deactivated padding enable
	 * @param sm pointer to silent mode
	 * @param dataReadycb pointer to data ready call back function
	 * @param dataModifyReadycb pointer to data ready call back function with modified
	 * @param pDataReadycb pointer to arguments of data ready call back function. To write/stream a smaller size of processed data, change this value (only smaller value is allowed).
	 */
	DataProcessor(int ind, detectorType dtype, Fifo*& f, fileFormat* ftype,
			bool fwenable, bool* dsEnable, bool* gpEnable, uint32_t* dr,
						uint32_t* freq, uint32_t* timer,
						bool* fp, bool* act, bool* depaden, bool* sm,
						void (*dataReadycb)(char*, char*, uint32_t, void*),
				        void (*dataModifyReadycb)(char*, char*, uint32_t &, void*),
						void *pDataReadycb);

	/**
	 * Destructor
	 * Calls Base Class DestroyThread() and decrements NumberofDataProcessors
	 */
	~DataProcessor();


	//*** getters ***
    /**
     * Returns if the thread is currently running
     * @returns true if thread is running, else false
     */
    bool IsRunning();

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
	 * Gets Actual Current Frame Index (that has not been subtracted from firstAcquisitionIndex) thats been processed for an entire  acquisition (including all scans)
	 * @return -1 if no frames have been caught, else current frame index (represents all scans too)
	 */
	uint64_t GetActualProcessedAcquisitionIndex();

	/**
	 * Get Current Frame Index thats been processed for an entire  acquisition (including all scans)
	 * @return -1 if no frames have been caught, else current frame index (represents all scans too)
	 */
	uint64_t GetProcessedAcquisitionIndex();

	/**
	 * Get Current Frame Index thats been processed for each real time acquisition (eg. for each scan)
	 * @return -1 if no frames have been caught, else current frame index
	 */
	uint64_t GetProcessedMeasurementIndex();

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
	 * @param fwe file write enable
	 * @param nd pointer to number of detectors in each dimension
	 * @param maxf pointer to max frames per file
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of threads/ units per detector
	 * @param nf pointer to number of images in acquisition
	 * @param dr pointer to dynamic range
	 * @param portno pointer to udp port number
	 * @param g address of GeneralData (Detector Data) pointer
	 */
	void SetupFileWriter(bool fwe, int* nd, uint32_t* maxf, char* fname,
			char* fpath, uint64_t* findex,
			 bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr,
			 uint32_t* portno, GeneralData* g = 0);

	/**
	 * Create New File
	 * @param en ten giga enable
	 * @param nf number of frames
	 * @param at acquisition time
	 * @param st sub exposure time
	 * @param sp sub period
	 * @param ap acquisition period
	 * @returns OK or FAIL
	 */
	int CreateNewFile(bool en, uint64_t nf, uint64_t at, uint64_t st,
			uint64_t sp, uint64_t ap);

	/**
	 * Closes files
	 */
	void CloseFiles();

	/**
	 * End of Acquisition
	 * @param anyPacketsCaught true if any packets are caught, else false
	 * @param numf number of images caught
	 */
	void EndofAcquisition(bool anyPacketsCaught, uint64_t numf);

	/**
	 * Update pixel dimensions in file writer
	 */
	void SetPixelDimension();




 private:

	/**
	 * Get Type
	 * @return type
	 */
	std::string GetType();

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

	/**
	 * Calls CheckTimer and CheckCount for streaming frequency and timer
	 * and determines if the current image should be sent to streamer
	 * @returns true if it should to streamer, else false
	 */
	bool SendToStreamer();

	/**
	 * This function should be called only in random frequency mode
	 * Checks if timer is done and ready to send to stream
	 * @returns true if ready to send to stream, else false
	 */
	bool CheckTimer();

	/**
	 * This function should be called only in non random frequency mode
	 * Checks if count is done and ready to send to stream
	 * @returns true if ready to send to stream, else false
	 */
	bool CheckCount();

	/**
	 * Pad Missing Packets from the bit mask
	 * @param buf buffer
	 */
	void PadMissingPackets(char* buf);

	/**
	 * Processing Function (inserting gap pixels) eiger specific
	 * @param buf pointer to image
	 * @param dr dynamic range
	 */
	void InsertGapPixels(char* buf, uint32_t dr);

	/** type of thread */
	static const std::string TypeName;

    /** Object running status */
    bool runningFlag;

	/** GeneralData (Detector Data) object */
	const GeneralData* generalData;

	/** Fifo structure */
	Fifo* fifo;


	//individual members
	/** Detector Type */
	detectorType myDetectorType;

	/** File writer implemented as binary or hdf5 File */
	File* file;

	/** Data Stream Enable */
	bool* dataStreamEnable;

	/** File Format Type */
	fileFormat* fileFormatType;

	/** File Write Enable */
	bool fileWriteEnable;

	/** Gap Pixels Enable */
	bool* gapPixelsEnable;


	/** Dynamic Range */
	uint32_t* dynamicRange;

	/** Pointer to Streaming frequency, if 0, sending random images with a timer */
	uint32_t* streamingFrequency;

	/** Pointer to the timer if Streaming frequency is random */
	uint32_t* streamingTimerInMs;

	/** Current frequency count */
	uint32_t currentFreqCount;

	/** timer beginning stamp for random streaming */
	struct timespec timerBegin;

	/** temporary buffer for processing */
	char* tempBuffer;

	/** Activated/Deactivated */
	bool* activated;

	/** Deactivated padding enable */
	bool* deactivatedPaddingEnable;

    /** Silent Mode */
    bool* silentMode;

	/** frame padding */
	bool* framePadding;

	//acquisition start
	/** Aquisition Started flag */
	bool acquisitionStartedFlag;

	/** Measurement Started flag */
	bool measurementStartedFlag;

	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t firstAcquisitionIndex;

	/** Frame Number of First Frame for each real time acquisition (eg. for each scan) */
	uint64_t firstMeasurementIndex;


	//for statistics
	/**Number of complete frames caught for an entire acquisition (including all scans) */
	uint64_t numTotalFramesCaught;

	/** Number of complete frames caught for each real time acquisition (eg. for each scan) */
	uint64_t numFramesCaught;

	/** Frame Number of latest processed frame number of an entire Acquisition (including all scans) */
	uint64_t currentFrameIndex;


	//call back
    /**
     * Call back for raw data
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * dataSize in bytes is the size of the data in bytes.
     */
    void (*rawDataReadyCallBack)(char*,
            char*, uint32_t, void*);

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes. Can be modified to the new size to be written/streamed. (only smaller value).
     */
    void (*rawDataModifyReadyCallBack)(char*,
            char*, uint32_t &, void*);

	void *pRawDataReady;





};

