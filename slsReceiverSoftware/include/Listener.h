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

class Fifo;
class genericSocket;

class Listener : private virtual slsReceiverDefs, public ThreadObject {
	
 public:
	/**
	 * Constructor
	 * Calls Base Class CreateThread(), sets ErrorMask if error and increments NumberofListerners
	 * @param f address of Fifo pointer
	 */
	Listener(Fifo*& f);

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
	 * @param portnumber udp port number
	 * @param packetSize size of one packet
	 * @param eth ethernet interface or null
	 * @param headerPacketSize size of a header packet
	 * @return OK or FAIL
	 */
	int CreateUDPSockets(uint32_t portnumber, uint32_t packetSize, const char* eth, uint32_t headerPacketSize);

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

	/** Fifo structure */
	Fifo* fifo;


	// individual members
	/** Aquisition Started flag */
	static bool acquisitionStartedFlag;

	/** Measurement Started flag */
	static bool measurementStartedFlag;

	/**Number of complete Packets caught for an entire acquisition (including all scans) */
	uint64_t numTotalPacketsCaught;

	/** Number of complete Packets caught for each real time acquisition (eg. for each scan) */
	uint64_t numPacketsCaught;

	/** Frame Number of First Frame of an entire Acquisition (including all scans) */
	uint64_t firstAcquisitionIndex;

	/** Frame Number of First Frame for each real time acquisition (eg. for each scan) */
	uint64_t firstMeasurementIndex;

	/** UDP Sockets - Detector to Receiver */
	genericSocket* udpSocket;
};

#endif
