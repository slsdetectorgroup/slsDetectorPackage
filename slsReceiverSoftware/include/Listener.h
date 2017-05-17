#pragma once
/************************************************
 * @file Listener.h
 * @short creates the listener thread that
 * listens to udp sockets, writes data to memory
 * & puts pointers to their memory addresses into fifos
 ***********************************************/
/**
 *@short creates & manages a listener thread each
 */

#include "ThreadObject.h"

class GeneralData;
class Fifo;
class genericSocket;

class Listener : private virtual slsReceiverDefs, public ThreadObject {
	
 public:
	/**
	 * Constructor
	 * Calls Base Class CreateThread(), sets ErrorMask if error and increments NumberofListerners
	 * @param dtype detector type
	 * @param f address of Fifo pointer
	 * @param s pointer to receiver status
	 * @param portno pointer to udp port number
	 * @param e ethernet interface
	 * @param act pointer to activated
	 * @param nf pointer to number of images to catch
	 * @param dr pointer to dynamic range
	 */
	Listener(detectorType dtype, Fifo*& f, runStatus* s, uint32_t* portno, char* e, int* act, uint64_t* nf, uint32_t* dr);

	/**
	 * Destructor
	 * Calls Base Class DestroyThread() and decrements NumberofListerners
	 */
	~Listener();


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
	 * Get Packets caught in a real time acquisition (start and stop of receiver)
	 * @return Packets caught in a real time acquisition
	 */
	uint64_t GetPacketsCaught();

	/**
	 * Get Last Frame index caught
	 * @return last frame index caught
	 */
	uint64_t GetLastFrameIndexCaught();


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
	void SetGeneralData(GeneralData*& g);

	/**
	 * Set thread priority
	 * @priority priority
	 * @returns OK or FAIL
	 */
	int SetThreadPriority(int priority);

	/**
	 * Creates UDP Sockets
	 * @return OK or FAIL
	 */
	int CreateUDPSockets();

	/**
	 * Shuts down and deletes UDP Sockets
	 */
	void ShutDownUDPSocket();




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
	 * Thread Exeution for Listener Class
	 * Pop free addresses, listen to udp socket,
	 * write to memory & push the address into fifo
	 */
	void ThreadExecution();

	/**
	 * Pushes non empty buffers into fifo/ frees empty buffer,
	 * pushes dummy buffer into fifo
	 * and reset running mask by calling StopRunning()
	 * @param buf address of buffer
	 */
	void StopListening(char* buf);

	/**
	 * Listen to the UDP Socket for an image,
	 * place them in the right order
	 * @param buffer
	 * @returns number of bytes of relevant data, can be image size or 0
	 */
	uint32_t ListenToAnImage(char* buf);

	/**
	 * Create an image (for deactivated detectors),
	 * @param buffer
	 * @returns image size or 0
	 */
	uint32_t CreateAnImage(char* buf);



	/** type of thread */
	static const std::string TypeName;

	/** Total Number of Listener Objects */
	static int NumberofListeners;

	/** Mask of errors on any object eg.thread creation */
	static uint64_t ErrorMask;

	/** Mask of all listener objects running */
	static uint64_t RunningMask;

	/** Mutex to update static items among objects (threads)*/
	static pthread_mutex_t Mutex;

	/** GeneralData (Detector Data) object */
	const GeneralData* generalData;

	/** Fifo structure */
	Fifo* fifo;


	// individual members
	/** Detector Type */
	detectorType myDetectorType;

	/** Receiver Status */
	runStatus* status;

	/** UDP Socket - Detector to Receiver */
	genericSocket* udpSocket;

	/** UDP Port Number */
	uint32_t* udpPortNumber;

	/** ethernet interface */
	char* eth;

	/** if the detector is activated */
	int* activated;

	/** Number of Images to catch */
	uint64_t* numImages;

	/** Dynamic Range */
	uint32_t* dynamicRange;


	// acquisition start
	/** Aquisition Started flag */
	bool acquisitionStartedFlag;

	/** Measurement Started flag */
	bool measurementStartedFlag;

	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t firstAcquisitionIndex;

	/** Frame Number of First Frame for each real time acquisition (eg. for each scan) */
	uint64_t firstMeasurementIndex;


	// for statistics
	/** Number of complete Packets caught for each real time acquisition (eg. for each scan (start& stop of receiver)) */
	volatile uint64_t numPacketsCaught;

	/** Last Frame Index caught  from udp network */
	uint64_t lastCaughtFrameIndex;


	// parameters to acquire image
	/** Current Frame Index, default value is 0
	 * ( always check acquisitionStartedFlag for validity first)
	 */
	uint64_t currentFrameIndex;

	/** True if there is a packet carry over from previous Image */
	bool carryOverFlag;

	/** Carry over packet buffer */
	char* carryOverPacket;

	/** Listening buffer for one packet - might be removed when we can peek and eiger fnum is in header */
	char* listeningPacket;


};

