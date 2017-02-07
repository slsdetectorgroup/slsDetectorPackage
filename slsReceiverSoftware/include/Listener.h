/************************************************
 * @file Listener.h
 * @short creates the listener thread that
 * listens to udp sockets, writes data to memory
 * & puts pointers to their memory addresses into fifos
 ***********************************************/
#ifndef LISTENER_H
#define LISTENER_H
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
	 * @param f address of Fifo pointer
	 * @param s pointer to receiver status
	 * @param portno pointer to udp port number
	 */
	Listener(Fifo*& f, runStatus* s, uint32_t* portno);

	/**
	 * Destructor
	 * Calls Base Class DestroyThread() and decrements NumberofListerners
	 */
	~Listener();


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
	 * Set GeneralData pointer to the one given
	 * @param g address of GeneralData (Detector Data) pointer
	 */
	static void SetGeneralData(GeneralData*& g);

	/**
	 * Get Total Packets caught in an acquisition
	 * @return Total Packets caught in an acquisition
	 */
	uint64_t GetTotalPacketsCaught();

	/**
	 * Get number of bytes currently received in udp buffer
	 */
	uint64_t GetNumReceivedinUDPBuffer();

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
	 * Creates UDP Sockets
	 * @param eth ethernet interface or null
	 * @return OK or FAIL
	 */
	int CreateUDPSockets(const char* eth);

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
	static const GeneralData* generalData;

	/** Fifo structure */
	Fifo* fifo;


	// individual members
	/** Aquisition Started flag */
	static bool acquisitionStartedFlag;

	/** Measurement Started flag */
	static bool measurementStartedFlag;

	/** Receiver Status */
	runStatus* status;

	/** UDP Sockets - Detector to Receiver */
	genericSocket* udpSocket;

	/** UDP Port Number */
	uint32_t* udpPortNumber;

	/**Number of complete Packets caught for an entire acquisition (including all scans) */
	uint64_t numTotalPacketsCaught;

	/** Number of complete Packets caught for each real time acquisition (eg. for each scan) */
	uint64_t numPacketsCaught;

	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t firstAcquisitionIndex;

	/** Frame Number of First Frame for each real time acquisition (eg. for each scan) */
	uint64_t firstMeasurementIndex;

};

#endif
