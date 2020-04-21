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

#include <memory>
#include <atomic>
#include "ThreadObject.h"
#include "UdpRxSocket.h"

class GeneralData;
class Fifo;

class Listener : private virtual slsDetectorDefs, public ThreadObject {
	
 public:
	/**
	 * Constructor
	 * Calls Base Class CreateThread(), sets ErrorMask if error and increments NumberofListerners
	 * @param ind self index
	 * @param dtype detector type
	 * @param f address of Fifo pointer
	 * @param s pointer to receiver status
	 * @param portno pointer to udp port number
	 * @param e ethernet interface
	 * @param nf pointer to number of images to catch
	 * @param dr pointer to dynamic range
	 * @param us pointer to udp socket buffer size
	 * @param as pointer to actual udp socket buffer size
	 * @param fpf pointer to frames per file
	 * @param fdp frame discard policy
	 * @param act pointer to activated
	 * @param depaden pointer to deactivated padding enable
	 * @param sm pointer to silent mode
	 */
	Listener(int ind, detectorType dtype, Fifo* f, std::atomic<runStatus>* s,
	        uint32_t* portno, std::string* e, uint64_t* nf, uint32_t* dr,
	        int64_t* us, int64_t* as, uint32_t* fpf,
			frameDiscardPolicy* fdp, bool* act, bool* depaden, bool* sm);

	/**
	 * Destructor
	 * Calls Base Class DestroyThread() and decrements NumberofListerners
	 */
	~Listener();

	/**
	 * Get Packets caught 
	 * @return Packets caught 
	 */
	uint64_t GetPacketsCaught() const;

	/**
	 * Get Last Frame index caught
	 * @return last frame index caught
	 */
	uint64_t GetLastFrameIndexCaught() const;

	/** Get  number of missing packets */
	uint64_t GetNumMissingPacket(bool stoppedFlag, uint64_t numPackets) const;

	/**
	 * Set Fifo pointer to the one given
	 * @param f address of Fifo pointer
	 */
	void SetFifo(Fifo* f);

	/**
	 * Reset parameters for new acquisition 
	 */
	void ResetParametersforNewAcquisition();

	/**
	 * Set GeneralData pointer to the one given
	 * @param g address of GeneralData (Detector Data) pointer
	 */
	void SetGeneralData(GeneralData* g);

	/**
	 * Creates UDP Sockets
	 */
	void CreateUDPSockets();

	/**
	 * Shuts down and deletes UDP Sockets
	 */
	void ShutDownUDPSocket();

    /**
     * Create & closes a dummy UDP socket
     * to set & get actual buffer size
     * @param s UDP socket buffer size to be set
     */
    void CreateDummySocketForUDPSocketBufferSize(int64_t s);

    /**
     * Set hard coded (calculated but not from detector) row and column
     * r is in row index if detector has not send them yet in firmware,
     * c is in col index for jungfrau and eiger (for missing packets/deactivated eiger)
     * c when used is in 2d
     */
    void SetHardCodedPosition(uint16_t r, uint16_t c);


 private:

	/**
	 * Record First Acquisition Index
	 * @param fnum frame index to record
	 */
	void RecordFirstIndex(uint64_t fnum);

	/**
	 * Thread Execution for Listener Class
	 * Pop free addresses, listen to udp socket,
	 * write to memory & push the address into fifo
	 */
	void ThreadExecution() override;

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
	 * @returns number of bytes of relevant data, can be image size or 0 (stop acquisition)
	 * or -1 to discard image
	 */
	uint32_t ListenToAnImage(char* buf);

	/**
	 * Print Fifo Statistics
	 */
	void PrintFifoStatistics();

	/** type of thread */
	static const std::string TypeName;

	/** GeneralData (Detector Data) object */
	GeneralData* generalData{nullptr};

	/** Fifo structure */
	Fifo* fifo;

	// individual members
	/** Detector Type */
	detectorType myDetectorType;

	/** Receiver Status */
	std::atomic<runStatus>* status;

	/** UDP Socket - Detector to Receiver */
	std::unique_ptr<sls::UdpRxSocket> udpSocket{nullptr};

	/** UDP Port Number */
	uint32_t* udpPortNumber;

	/** ethernet interface */
	std::string* eth;

	/** Number of Images to catch */
	uint64_t* numImages;

	/** Dynamic Range */
	uint32_t* dynamicRange;

	/** UDP Socket Buffer Size */
	int64_t* udpSocketBufferSize;

	/** actual UDP Socket Buffer Size (double due to kernel bookkeeping) */
	int64_t* actualUDPSocketBufferSize;

	/** frames per file */
	uint32_t* framesPerFile;

	/** frame discard policy */
	frameDiscardPolicy* frameDiscardMode;

	/** Activated/Deactivated */
	bool* activated;

	/** Deactivated padding enable */
	bool* deactivatedPaddingEnable;

    /** Silent Mode */
    bool* silentMode;

	/** row hardcoded as 1D or 2d,
	 * if detector does not send them yet or
	 * missing packets/deactivated (eiger/jungfrau sends 2d pos) **/
	uint16_t row{0};

	/** column hardcoded as 2D,
	 * deactivated eiger/missing packets (eiger/jungfrau sends 2d pos) **/
	uint16_t column{0};

	// acquisition start
	/** Aquisition Started flag */
	std::atomic<bool> startedFlag{false};

	/** Frame Number of First Frame  */
	uint64_t firstIndex{0};

	// for acquisition summary
	/** Number of complete Packets caught */
	std::atomic<uint64_t> numPacketsCaught{0};

	/** Last Frame Index caught  from udp network */
	std::atomic<uint64_t> lastCaughtFrameIndex{0};

	// parameters to acquire image
	/** Current Frame Index, default value is 0
	 * ( always check startedFlag for validity first)
	 */
	uint64_t currentFrameIndex{0};

	/** True if there is a packet carry over from previous Image */
	bool carryOverFlag{false};

	/** Carry over packet buffer */
	std::unique_ptr<char []> carryOverPacket;

	/** Listening buffer for one packet - might be removed when we can peek and eiger fnum is in header */
	std::unique_ptr<char []> listeningPacket;

	/** if the udp socket is connected */
	std::atomic<bool> udpSocketAlive{false};

	// for print progress during acquisition
	/** number of packets for statistic */
	uint32_t numPacketsStatistic{0};

	/** number of images for statistic */
	uint32_t numFramesStatistic{0};

    /**
     * starting packet number is odd or even, accordingly increment frame number
     * to get first packet number as 0
     * (pecific to gotthard, can vary between modules, hence defined here) */
    bool oddStartingPacket{true};
};

