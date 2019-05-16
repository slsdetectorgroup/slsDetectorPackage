#pragma once
/********************************************//**
 * @file slsReceiverTCPIPInterface.h
 * @short interface between receiver and client
 ***********************************************/


#include "sls_detector_defs.h"
#include "receiver_defs.h"

class MySocketTCP;
class ServerInterface;
class slsReceiverImplementation;
#include "ServerSocket.h"


/**
 *@short interface between receiver and client
 */

class slsReceiverTCPIPInterface : private virtual slsDetectorDefs {
 private:
	enum numberMode {DEC, HEX};
	
 public:

	/** Destructor */
	virtual ~slsReceiverTCPIPInterface();

	/**
	 * Constructor
	 * reads config file, creates socket, assigns function table
	 * throws an exception in case of failure to construct
	 * @param pn port number (defaults to default port number)
	 */

  slsReceiverTCPIPInterface(int pn=-1);

	/**
	 * Starts listening on the TCP port for client comminication
	 \returns OK or FAIL
	 */
	int start();

	/** stop listening on the TCP & UDP port for client comminication */
	void stop();


	/** gets version */
	int64_t getReceiverVersion();

	//***callback functions***
	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is insignificant at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	void registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg);

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg);

	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * sls_receiver_header frame metadata
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes.
	 */
	void registerCallBackRawDataReady(void (*func)(char* ,
			char*, uint32_t, void*),void *arg);

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes.
     * Can be modified to the new size to be written/streamed. (only smaller value).
     */
    void registerCallBackRawDataModifyReady(void (*func)(char* ,
            char*, uint32_t &,void*),void *arg);


 private:

	/**
	 * Static function - Thread started which is a TCP server
	 * Called by start()
	 * @param this_pointer pointer to this object
	 */
	static void* startTCPServerThread(void *this_pointer);


	/**
	 * Thread started which is a TCP server
	 * Called by start()
	 */
	void startTCPServer();

	/** assigns functions to the fnum enum */
	int function_table();

	/** Decodes Function */
	int decode_function(sls::ServerInterface2 &socket);

	/** function not implemented for specific detector */
	void functionNotImplemented();

	/** mode not implemented for specific detector */
	void modeNotImplemented(std::string modename, int mode);

	/** validate and set error */
	template <typename T>
	void validate(T arg, T retval, std::string modename, numberMode hex);

	/** Unrecognized Function */
	int M_nofunc(sls::ServerInterface2 & socket);



	/** Execute command */
	int	exec_command(sls::ServerInterface2 &socket);

	/** Exit Receiver Server */
	int	exit_server(sls::ServerInterface2 &socket);

	/** Locks Receiver */
	int	lock_receiver(sls::ServerInterface2 &socket);

	/** Get Last Client IP*/
	int	get_last_client_ip(sls::ServerInterface2 &socket);

	/** Set port */
	int set_port(sls::ServerInterface2 &socket);

	/** Updates Client if different clients connect */
	int	update_client(sls::ServerInterface2 &socket);

	/** Sends the updated parameters to client */
	int send_update(sls::ServerInterface2 &socket);

	/** get version, calls get_version */
	int get_id(sls::ServerInterface2 &socket);

	/** Set detector type */
	int set_detector_type(sls::ServerInterface2 &socket);

	/** set detector hostname  */
	int set_detector_hostname(sls::ServerInterface2 &socket);

	/** set roi */
	int set_roi(sls::ServerInterface2 &socket);

	/** Set up UDP Details */
	int setup_udp(sls::ServerInterface2 &socket);

	/** set acquisition period, frame number etc */
	int set_timer(sls::ServerInterface2 &socket);

	/** set dynamic range  */
	int set_dynamic_range(sls::ServerInterface2 &socket);

	/** Sets the receiver streaming frequency */
	int set_streaming_frequency(sls::ServerInterface2 &socket);

	/** Gets receiver status */
	int	get_status(sls::ServerInterface2 &socket);

	/** Start Receiver - starts listening to udp packets from detector */
	int start_receiver(sls::ServerInterface2 &socket);

	/** Stop Receiver - stops listening to udp packets from detector*/
	int stop_receiver(sls::ServerInterface2 &socket);

	/** Set File path */
	int set_file_dir(sls::ServerInterface2 &socket);

	/** Set File name without frame index, file index and extension */
	int set_file_name(sls::ServerInterface2 &socket);

	/** Set File index */
	int set_file_index(sls::ServerInterface2 &socket);

	/** Gets frame index for each acquisition */
	int	get_frame_index(sls::ServerInterface2 &socket);

	/** Gets Total Frames Caught */
	int	get_frames_caught(sls::ServerInterface2 &socket);

	/** Resets Total Frames Caught */
	int	reset_frames_caught(sls::ServerInterface2 &socket);

	/** Enable File Write*/
	int enable_file_write(sls::ServerInterface2 &socket);

	/** Enable Master File Write */
	int enable_master_file_write(sls::ServerInterface2 &socket);

	/** enable compression */
	int enable_compression(sls::ServerInterface2 &socket);

	/** enable overwrite  */
	int enable_overwrite(sls::ServerInterface2 &socket);

	/** enable 10Gbe */
	int enable_tengiga(sls::ServerInterface2 &socket);

	/** set fifo depth */
	int set_fifo_depth(sls::ServerInterface2 &socket);

	/** activate/ deactivate */
	int set_activate(sls::ServerInterface2 &socket);

	/* Set the data stream enable */
	int set_data_stream_enable(sls::ServerInterface2 &socket);

	/** Sets the steadming timer when frequency is set to 0 */
	int set_streaming_timer(sls::ServerInterface2 &socket);

	/** enable flipped data */
	int set_flipped_data(sls::ServerInterface2 &socket);

	/** set file format */
	int set_file_format(sls::ServerInterface2 &socket);

	/** set position id */
	int set_detector_posid(sls::ServerInterface2 &socket);

	/** set multi detector size */
	int set_multi_detector_size(sls::ServerInterface2 &socket);

	/** set streaming port */
	int set_streaming_port(sls::ServerInterface2 &socket);

	/** set streaming source ip */
	int set_streaming_source_ip(sls::ServerInterface2 &socket);

	/** set silent mode */
	int set_silent_mode(sls::ServerInterface2 &socket);

	/** enable gap pixels */
	int enable_gap_pixels(sls::ServerInterface2 &socket);

	/** restream stop packet */
	int restream_stop(sls::ServerInterface2 &socket);

    /** set additional json header */
    int set_additional_json_header(sls::ServerInterface2 &socket);

    /** get additional json header */
    int get_additional_json_header(sls::ServerInterface2 &socket);

    /** set udp socket buffer size */
    int set_udp_socket_buffer_size(sls::ServerInterface2 &socket);

    /** get real udp socket buffer size */
    int get_real_udp_socket_buffer_size(sls::ServerInterface2 &socket);

    /** set frames per file */
    int set_frames_per_file(sls::ServerInterface2 &socket);

    /** check version compatibility */
    int check_version_compatibility(sls::ServerInterface2 &socket);

    /** set frame discard policy */
    int set_discard_policy(sls::ServerInterface2 &socket);

    /** set partial frame padding enable*/
    int set_padding_enable(sls::ServerInterface2 &socket);

    /** set deactivated receiver padding enable */
    int set_deactivated_padding_enable(sls::ServerInterface2 &socket);

    /** set readout flags */
    int set_readout_flags(sls::ServerInterface2 &socket);

		/** set adc mask */
		int set_adc_mask(sls::ServerInterface2 &socket);

		/** set receiver dbit list */
		int set_dbit_list(sls::ServerInterface2 &socket);

		/** get receiver dbit list */
		int get_dbit_list(sls::ServerInterface2 &socket);

		/** set dbit offset */
		int set_dbit_offset(sls::ServerInterface2 &socket);


	int LogSocketCrash();
	void NullObjectError(int& ret, char* mess);

	/** detector type */
	detectorType myDetectorType;

	/** slsReceiverBase object */
	slsReceiverImplementation *receiver;

	/** Function List */
	int (slsReceiverTCPIPInterface::*flist[NUM_REC_FUNCTIONS])(sls::ServerInterface2& socket);

	/** Message */
	char mess[MAX_STR_LENGTH] = "dummy message";

	/** success/failure */
	int ret{OK};

	/** function index */
	int fnum{-1};

	/** Lock Status if server locked to a client */
	int lockStatus{0};

	/** kill tcp server thread */
	int killTCPServerThread{0};

	/** thread for TCP server */
	pthread_t   TCPServer_thread;

	/** tcp thread created flag*/
	bool tcpThreadCreated{false};

	/** port number */
	int portNumber;

	//***callback parameters***
	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is insignificant at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	int (*startAcquisitionCallBack)(char*, char*, uint64_t, uint32_t, void*) = nullptr;
	void *pStartAcquisition{nullptr};

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * total frames caught
	 */
	void (*acquisitionFinishedCallBack)(uint64_t, void*) = nullptr;
	void *pAcquisitionFinished{nullptr};


	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * sls_receiver_header frame metadata
	 * dataPointer is the pointer to the data
	 * dataSize in bytes is the size of the data in bytes.
	 */
	void (*rawDataReadyCallBack)(char* ,
			char*, uint32_t, void*) = nullptr;

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes. Can be modified to the new size to be written/streamed. (only smaller value).
     */
    void (*rawDataModifyReadyCallBack)(char* ,
            char*, uint32_t &, void*) = nullptr;

	void *pRawDataReady{nullptr};



protected:


	std::unique_ptr<sls::ServerSocket> server{nullptr};

      private:
        int VerifyLock(int &ret, char *mess);
				int VerifyLockAndIdle(int &ret, char *mess, int fnum);
};
