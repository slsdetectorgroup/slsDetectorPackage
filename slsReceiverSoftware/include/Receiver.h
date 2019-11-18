#pragma once
#include "ClientInterface.h"
#include "sls_detector_defs.h"

#include <memory>

class Receiver : private virtual slsDetectorDefs {
	
 public:
	/**
	 * Constructor
	 * Starts up a Receiver server. Reads configuration file, options, and
	 * assembles a Receiver using TCP and UDP detector interfaces
	 * throws an exception in case of failure
	 * @param argc from command line
	 * @param argv from command line
	 */
	Receiver(int argc, char *argv[]);
	
	/**
	 * Constructor
	 * Starts up a Receiver server. Reads configuration file, options, and
	 * assembles a Receiver using TCP and UDP detector interfaces
	 * throws an exception in case of failure
	 * @param tcpip_port_no TCP/IP port number
	 */
	Receiver(int tcpip_port_no = 1954);

	/**
	 * get get Receiver Version
	 \returns id
	 */
	int64_t getReceiverVersion();

	/**
	 * Call back for start acquisition
	 * callback arguments are
	 * filepath
	 * filename
	 * fileindex
	 * datasize
	 *
	 * return value is undefined at the moment
	 * we write depending on file write enable
	 * users get data to write depending on call backs registered
	 */
	void registerCallBackStartAcquisition(int (*func)(std::string, std::string, uint64_t, uint32_t, void*),void *arg);

	/**
	 * Call back for acquisition finished
	 * callback argument is
	 * @param total frames caught
	 */
	void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg);

	/**
	 * Call back for raw data
	 * args to raw data ready callback are
	 * @param sls_receiver_header frame metadata
	 * @param dataPointer is the pointer to the data
	 * @param dataSize in bytes is the size of the data in bytes.
	 */
	void registerCallBackRawDataReady(void (*func)(char* ,
			char*, uint32_t, void*),void *arg);

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * @param sls_receiver_header frame metadata
     * @param dataPointer is the pointer to the data
     * @param revDatasize is the reference of data size in bytes.
     * Can be modified to the new size to be written/streamed. (only smaller value).
     */
    void registerCallBackRawDataModifyReady(void (*func)(char* ,
            char*, uint32_t &,void*),void *arg);


 private:
	std::unique_ptr<ClientInterface> tcpipInterface;
};
