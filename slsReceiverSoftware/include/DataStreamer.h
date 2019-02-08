#pragma once
/************************************************
 * @file DataStreamer.h
 * @short streams data from receiver via ZMQ
 ***********************************************/
/**
 *@short creates & manages a data streamer thread each
 */

#include "ThreadObject.h"

class GeneralData;
class Fifo;
class DataStreamer;
class ZmqSocket;

#include <vector>

class DataStreamer : private virtual slsReceiverDefs, public ThreadObject {
	
 public:
	/**
	 * Constructor
	 * Calls Base Class CreateThread(), sets ErrorMask if error and increments NumberofDataStreamers
     * @param ind self index
	 * @param f address of Fifo pointer
	 * @param dr pointer to dynamic range
	 * @param r roi
	 * @param fi pointer to file index
	 * @param fd flipped data enable for x and y dimensions
	 * @param ajh additional json header
	 * @param sm pointer to silent mode
	 */
	DataStreamer(int ind, Fifo*& f, uint32_t* dr, std::vector<ROI>* r,
			uint64_t* fi, int* fd, char* ajh, bool* sm);

	/**
	 * Destructor
	 * Calls Base Class DestroyThread() and decrements NumberofDataStreamers
	 */
	~DataStreamer();

	//*** getters ***
    /**
     * Returns if the thread is currently running
     * @returns true if thread is running, else false
     */
    bool IsRunning();


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
	void ResetParametersforNewMeasurement(char* fname);

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
	 * Creates Zmq Sockets
	 * (throws an exception if it couldnt create zmq sockets)
	 * @param nunits pointer to number of theads/ units per detector
	 * @param port streaming port start index
	 * @param srcip streaming source ip
	 */
	void CreateZmqSockets(int* nunits, uint32_t port, const char* srcip);

	/**
	 * Shuts down and deletes Zmq Sockets
	 */
	void CloseZmqSocket();

	/**
	 * Restream stop dummy packet
	 * @return OK or FAIL
	 */
	int RestreamStop();


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
	 * Thread Exeution for DataStreamer Class
	 * Stream an image via zmq
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
	 * Create and send Json Header
	 * @param rheader header of image
	 * @param size data size (could have been modified in call back)
	 * @param nx number of pixels in x dim
	 * @param ny number of pixels in y dim
	 * @param dummy true if its a dummy header
	 * @returns 0 if error, else 1
	 */
	int SendHeader(sls_receiver_header* rheader, uint32_t size = 0, uint32_t nx = 0, uint32_t ny = 0, bool dummy = true);

	/** type of thread */
	static const std::string TypeName;

    /** Object running status */
    bool runningFlag;

	/** GeneralData (Detector Data) object */
	const GeneralData* generalData;

	/** Fifo structure */
	Fifo* fifo;



	/** ZMQ Socket - Receiver to Client */
	ZmqSocket* zmqSocket;

	/** Pointer to dynamic range */
	uint32_t* dynamicRange;

	/** ROI */
	std::vector<ROI>* roi;

	/** adc Configured */
	int adcConfigured;

	/** Pointer to file index */
	uint64_t* fileIndex;

	/** flipped data across both dimensions enable */
	int* flippedData;

	/** additional json header */
	char* additionJsonHeader;

	/** Aquisition Started flag */
	bool acquisitionStartedFlag;

	/** Measurement Started flag */
	bool measurementStartedFlag;

	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t firstAcquisitionIndex;

	/** Frame Number of First Frame for each real time acquisition (eg. for each scan) */
	uint64_t firstMeasurementIndex;

	/* File name to stream */
	char fileNametoStream[MAX_STR_LENGTH];

	/** Complete buffer used for roi, eg. shortGotthard */
	char* completeBuffer;

};

