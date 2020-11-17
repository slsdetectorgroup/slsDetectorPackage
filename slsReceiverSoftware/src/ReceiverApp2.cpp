#include <sls/Receiver.h>

#include <csignal> //SIGINT
#include <iostream>
#include <semaphore.h>
#include <sys/syscall.h>
#include <unistd.h>

sem_t semaphore;

void sigInterruptHandler(int p) { sem_post(&semaphore); }

/**
 * Start Acquisition Call back
 * slsReceiver writes data if file write enabled.
 * Users get data to write using call back if registerCallBackRawDataReady is
 * registered.
 * @param filepath file path
 * @param filename file name
 * @param fileindex file index
 * @param datasize data size in bytes
 * @param p pointer to object
 * \returns ignored
 */
int StartAcq(std::string filepath, std::string filename, uint64_t fileindex,
             uint32_t datasize, void *p) {
    std::cout << "#### StartAcq:  filepath:" << filepath
              << "  filename:" << filename << " fileindex:" << fileindex
              << "  datasize:" << datasize << " ####";

    // throw std::runtime_error(
    //     "Throwing exception from start acquisition call back");
    return 0;
}

/**
 * Acquisition Finished Call back
 * @param frames Number of frames caught
 * @param p pointer to object
 */
void AcquisitionFinished(uint64_t frames, void *p) {
    std::cout << "#### AcquisitionFinished: frames:" << frames << " ####";

    /*throw std::runtime_error(
        "Throwing exception from acquisition finished call back");*/
}

/**
 * Get Receiver Data Call back and prints the header for each image call back.
 * @param metadata sls_receiver_header metadata
 * @param datapointer pointer to data
 * @param datasize data size in bytes.
 * @param p pointer to object
 */
void GetData(char *metadata, char *datapointer, uint32_t datasize, void *p) {
    slsDetectorDefs::sls_receiver_header *header =
        (slsDetectorDefs::sls_receiver_header *)metadata;
    slsDetectorDefs::sls_detector_header detectorHeader = header->detHeader;

    std::cout << "#### " << detectorHeader.row << " GetData: ####\n"
              << "frameNumber: " << detectorHeader.frameNumber
              << "\t\texpLength: " << detectorHeader.expLength
              << "\t\tpacketNumber: " << detectorHeader.packetNumber
              << "\t\tbunchId: " << detectorHeader.bunchId
              << "\t\ttimestamp: " << detectorHeader.timestamp
              << "\t\tmodId: " << detectorHeader.modId
              << "\t\trow: " << detectorHeader.row
              << "\t\tcolumn: " << detectorHeader.column
              << "\t\treserved: " << detectorHeader.reserved
              << "\t\tdebug: " << detectorHeader.debug
              << "\t\troundRNumber: " << detectorHeader.roundRNumber
              << "\t\tdetType: " << detectorHeader.detType << "\t\tversion: "
              << detectorHeader.version
              //<< "\t\tpacketsMask: " << header->packetsMask.to_string()
              << "\t\tfirstbytedata: " << std::hex << "0x"
              << ((uint8_t)(*((uint8_t *)(datapointer))))
              << "\t\tdatsize: " << datasize << "\n\n";

    if (detectorHeader.frameNumber % 2 == 0) {
        throw std::runtime_error("Throwing exception from Get Data call back");
    }
}

/**
 * Get Receiver Data Call back (modified) and prints headers for each image call
 * back.
 * @param metadata sls_receiver_header metadata
 * @param datapointer pointer to data
 * @param revDatasize new data size in bytes after the callback.
 * This will be the size written/streamed. (only smaller value is allowed).
 * @param p pointer to object
 */
void GetData(char *metadata, char *datapointer, uint32_t &revDatasize,
             void *p) {
    slsDetectorDefs::sls_receiver_header *header =
        (slsDetectorDefs::sls_receiver_header *)metadata;
    slsDetectorDefs::sls_detector_header detectorHeader = header->detHeader;

    std::cout << "#### " << detectorHeader.row << " GetData: ####\n"
              << "frameNumber: " << detectorHeader.frameNumber
              << "\t\texpLength: " << detectorHeader.expLength
              << "\t\tpacketNumber: " << detectorHeader.packetNumber
              << "\t\tbunchId: " << detectorHeader.bunchId
              << "\t\ttimestamp: " << detectorHeader.timestamp
              << "\t\tmodId: " << detectorHeader.modId
              << "\t\trow: " << detectorHeader.row
              << "\t\tcolumn: " << detectorHeader.column
              << "\t\treserved: " << detectorHeader.reserved
              << "\t\tdebug: " << detectorHeader.debug
              << "\t\troundRNumber: " << detectorHeader.roundRNumber
              << "\t\tdetType: " << detectorHeader.detType << "\t\tversion: "
              << detectorHeader.version
              //<< "\t\tpacketsMask: " << header->packetsMask.to_string()
              << "\t\tfirstbytedata: " << std::hex << "0x"
              << ((uint8_t)(*((uint8_t *)(datapointer))))
              << "\t\tdatsize: " << revDatasize << "\n\n";

    // if data is modified, eg ROI and size is reduced
    revDatasize = 26000;
}

int main(int argc, char *argv[]) {

    sem_init(&semaphore, 1, 0);

    std::cout << "Created [ Tid: " << syscall(SYS_gettid) << " ]";

    // Catch signal SIGINT to close files and call destructors properly
    struct sigaction sa;
    sa.sa_flags = 0;                     // no flags
    sa.sa_handler = sigInterruptHandler; // handler function
    sigemptyset(&sa.sa_mask); // dont block additional signals during invocation
                              // of handler
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        std::cout << "Could not set handler function for SIGINT";
    }

    // if socket crash, ignores SISPIPE, prevents global signal handler
    // subsequent read/write to socket gives error - must handle locally
    struct sigaction asa;
    asa.sa_flags = 0;          // no flags
    asa.sa_handler = SIG_IGN;  // handler function
    sigemptyset(&asa.sa_mask); // dont block additional signals during
                               // invocation of handler
    if (sigaction(SIGPIPE, &asa, nullptr) == -1) {
        std::cout << "Could not set handler function for SIGPIPE";
    }

    try {
        sls::Receiver r(argc, argv);

        // register call backs
        /** - Call back for start acquisition */
        std::cout << "Registering 	StartAcq()";
        r.registerCallBackStartAcquisition(StartAcq, nullptr);

        /** - Call back for acquisition finished */
        std::cout << "Registering 	AcquisitionFinished()";
        r.registerCallBackAcquisitionFinished(AcquisitionFinished, nullptr);

        /* 	- Call back for raw data */
        std::cout << "Registering GetData()";
        r.registerCallBackRawDataReady(GetData, nullptr);

        std::cout << "[ Press \'Ctrl+c\' to exit ]";
        sem_wait(&semaphore);
        sem_destroy(&semaphore);
    } catch (...) {
        // pass
    }
    std::cout << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
    std::cout << "Exiting Receiver";
    return 0;
}
