#include "Implementation.h"
#include "DataProcessor.h"
#include "DataStreamer.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "Listener.h"
#include "ZmqSocket.h" //just for the zmq port define
#include "file_utils.h"
#include "ToString.h"

#include <cerrno>  //eperm
#include <cstdlib> //system
#include <cstring>
#include <cstring> //strcpy
#include <fstream>
#include <iostream>
#include <sys/stat.h> // stat

/** cosntructor & destructor */

Implementation::Implementation(const detectorType d) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    InitializeMembers();
    setDetectorType(d);
}

Implementation::~Implementation() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    DeleteMembers();
}

void Implementation::DeleteMembers() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (generalData) {
        delete generalData;
        generalData = nullptr;
    }

    listener.clear();
    dataProcessor.clear();
    dataStreamer.clear();
    fifo.clear();
    eth.clear();
    udpPortNum.clear();
    ctbDbitList.clear();
}

void Implementation::InitializeMembers() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    // config parameters
    numThreads = 1;
    myDetectorType = GENERIC;
    for (int i = 0; i < MAX_DIMENSIONS; ++i)
        numDet[i] = 0;
    detID = 0;
    detHostname = "";
    silentMode = false;
    fifoDepth = 0;
    frameDiscardMode = NO_DISCARD;
    framePadding = false;

    // file parameters
    fileFormatType = BINARY;
    filePath = "";
    fileName = "run";
    fileIndex = 0;
    fileWriteEnable = true;
    masterFileWriteEnable = true;
    overwriteEnable = true;
    framesPerFile = 0;

    // acquisition
    status = IDLE;
    stoppedFlag = false;

    // network configuration (UDP)
    numUDPInterfaces = 1;
    eth.resize(MAX_NUMBER_OF_LISTENING_THREADS);
    udpPortNum.resize(MAX_NUMBER_OF_LISTENING_THREADS);
    for (int i = 0; i < MAX_NUMBER_OF_LISTENING_THREADS; ++i) {
        eth[i] = "";
        udpPortNum[i] =  DEFAULT_UDP_PORTNO + i;
    }
    udpSocketBufferSize = 0;
    actualUDPSocketBufferSize = 0;

    // zmq parameters
    dataStreamEnable = false;
    streamingFrequency = 1;
    streamingTimerInMs = DEFAULT_STREAMING_TIMER_IN_MS;
    streamingPort = 0;
    streamingSrcIP = 0u;
    additionalJsonHeader = "";

    // detector parameters
    numberOfFrames = 0;
    acquisitionPeriod = SAMPLE_TIME_IN_NS;
    acquisitionTime = 0;
    subExpTime = 0;
    subPeriod = 0;
    numberOfAnalogSamples = 0;
    numberOfDigitalSamples = 0;
    numberOfCounters = 0;
    dynamicRange = 16;
    roi.xmin = -1;
    roi.xmax = -1;
    tengigaEnable = false;
    flippedDataX = 0;
    gapPixelsEnable = false;
    quadEnable = false;
    activated = true;
    deactivatedPaddingEnable = true;
    numLinesReadout = MAX_EIGER_ROWS_PER_READOUT;
    readoutType = ANALOG_ONLY;
    adcEnableMaskOneGiga = BIT32_MASK;
    adcEnableMaskTenGiga = BIT32_MASK;

    ctbDbitOffset = 0;
    ctbAnalogDataBytes = 0;

    // callbacks
    startAcquisitionCallBack = nullptr;
    pStartAcquisition = nullptr;
    acquisitionFinishedCallBack = nullptr;
    pAcquisitionFinished = nullptr;
    rawDataReadyCallBack = nullptr;
    rawDataModifyReadyCallBack = nullptr;
    pRawDataReady = nullptr;

    // class objects
    generalData = nullptr;
}

void Implementation::SetLocalNetworkParameters() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    // to increase Max length of input packet queue
    int max_back_log;
    const char *proc_file_name = "/proc/sys/net/core/netdev_max_backlog";
    {
        std::ifstream proc_file(proc_file_name);
        proc_file >> max_back_log;
    }

    if (max_back_log < MAX_SOCKET_INPUT_PACKET_QUEUE) {
        std::ofstream proc_file(proc_file_name);
        if (proc_file.good()) {
            proc_file << MAX_SOCKET_INPUT_PACKET_QUEUE << std::endl;
            FILE_LOG(logINFOBLUE)
                << "Max length of input packet queue "
                   "[/proc/sys/net/core/netdev_max_backlog] modified to "
                << MAX_SOCKET_INPUT_PACKET_QUEUE;
        } else {
            FILE_LOG(logWARNING)
                << "Could not change max length of "
                   "input packet queue [net.core.netdev_max_backlog]. (No Root "
                   "Privileges?)";
        }
    }
}

void Implementation::SetThreadPriorities() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : listener) {
        it->SetThreadPriority(LISTENER_PRIORITY);
    }
}

void Implementation::SetupFifoStructure() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    fifo.clear();
    for (int i = 0; i < numThreads; ++i) {

        // create fifo structure
        try {
            fifo.push_back(sls::make_unique<Fifo>(
                i,
                (generalData->imageSize) + (generalData->fifoBufferHeaderSize),
                fifoDepth));
        } catch (...) {
            fifo.clear();
            throw sls::RuntimeError("Could not allocate memory for fifo structure " + std::to_string(i));
        }
        // set the listener & dataprocessor threads to point to the right fifo
        if (listener.size())
            listener[i]->SetFifo(fifo[i].get());
        if (dataProcessor.size())
            dataProcessor[i]->SetFifo(fifo[i].get());
        if (dataStreamer.size())
            dataStreamer[i]->SetFifo(fifo[i].get());
    }

    FILE_LOG(logINFO) << "Memory Allocated Per Fifo: "
                      << (((generalData->imageSize) +
                           (generalData->fifoBufferHeaderSize)) *
                          fifoDepth)
                      << " bytes";
    FILE_LOG(logINFO) << numThreads << " Fifo structure(s) reconstructed";
}



/**************************************************
 *                                                 *
 *   Configuration Parameters                      *
 *                                                 *
 * ************************************************/

void Implementation::setDetectorType(const detectorType d) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    myDetectorType = d;
    switch (myDetectorType) {
    case GOTTHARD:
    case EIGER:
    case JUNGFRAU:
    case CHIPTESTBOARD:
    case MOENCH:
    case MYTHEN3:
        FILE_LOG(logINFO) << " ***** " << sls::ToString(d)
                          << " Receiver *****";
        break;
    default:
        throw sls::RuntimeError("This is an unknown receiver type "  + std::to_string(static_cast<int>(d)));
    }

    // set detector specific variables
    switch (myDetectorType) {
    case GOTTHARD:
        generalData = new GotthardData();
        break;
    case EIGER:
        generalData = new EigerData();
        break;
    case JUNGFRAU:
        generalData = new JungfrauData();
        break;
    case CHIPTESTBOARD:
        generalData = new ChipTestBoardData();
        break;
    case MOENCH:
        generalData = new MoenchData();
        break;
    case MYTHEN3:
        generalData = new Mythen3Data();
        break;        
    default:
        break;
    }
    numThreads = generalData->threadsPerReceiver;
    fifoDepth = generalData->defaultFifoDepth;
    udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;
    framesPerFile = generalData->maxFramesPerFile;

    SetLocalNetworkParameters();
    SetupFifoStructure();

    // create threads
    for (int i = 0; i < numThreads; ++i) {

        try {
            auto fifo_ptr = fifo[i].get();
            listener.push_back(sls::make_unique<Listener>(
                i, myDetectorType, fifo_ptr, &status, &udpPortNum[i], &eth[i],
                &numberOfFrames, &dynamicRange, &udpSocketBufferSize,
                &actualUDPSocketBufferSize, &framesPerFile, &frameDiscardMode,
                &activated, &deactivatedPaddingEnable, &silentMode));
            dataProcessor.push_back(sls::make_unique<DataProcessor>(
                i, myDetectorType, fifo_ptr, &fileFormatType, fileWriteEnable,
                &masterFileWriteEnable, &dataStreamEnable, &gapPixelsEnable,
                &dynamicRange, &streamingFrequency, &streamingTimerInMs,
                &framePadding, &activated, &deactivatedPaddingEnable,
                &silentMode, &quadEnable, &ctbDbitList, &ctbDbitOffset,
                &ctbAnalogDataBytes));
        } catch (...) {
            listener.clear();
            dataProcessor.clear();
            throw sls::RuntimeError("Could not create listener/dataprocessor threads (index:" + std::to_string(i) + ")");
        }
    }

    // set up writer and callbacks

    for (const auto &it : listener)
        it->SetGeneralData(generalData);
    for (const auto &it : dataProcessor)
        it->SetGeneralData(generalData);
    SetThreadPriorities();

    FILE_LOG(logDEBUG) << " Detector type set to " << sls::ToString(d);
}

int *Implementation::getMultiDetectorSize() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return (int *)numDet;
}

void Implementation::setMultiDetectorSize(const int *size) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    std::string log_message = "Detector Size (ports): (";
    for (int i = 0; i < MAX_DIMENSIONS; ++i) {
        // x dir (colums) each udp port
        if (myDetectorType == EIGER && i == X)
            numDet[i] = size[i] * 2;
        // y dir (rows) each udp port
        else if (numUDPInterfaces == 2 && i == Y)
            numDet[i] = size[i] * 2;
        else
            numDet[i] = size[i];
        log_message += std::to_string(numDet[i]);
        if (i < MAX_DIMENSIONS - 1)
            log_message += ", ";
    }
    log_message += ")";

    int nd[2] = {numDet[0], numDet[1]};
	if (quadEnable) {
		nd[0] = 1;
		nd[1] = 2;
	}
	for (const auto &it : dataStreamer) {
		it->SetNumberofDetectors(nd);
	}

    FILE_LOG(logINFO) << log_message;
}

int Implementation::getDetectorPositionId() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return detID;
}

void Implementation::setDetectorPositionId(const int id) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    detID = id;
    FILE_LOG(logINFO) << "Detector Position Id:" << detID;
    for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
        dataProcessor[i]->SetupFileWriter(
            fileWriteEnable, (int *)numDet, &framesPerFile, &fileName, &filePath,
            &fileIndex, &overwriteEnable, &detID, &numThreads, &numberOfFrames,
            &dynamicRange, &udpPortNum[i], generalData);
    }
    assert(numDet[1] != 0);
    for (unsigned int i = 0; i < listener.size(); ++i) {
        uint16_t row = 0, col = 0;
        row = (detID % numDet[1]) * ((numUDPInterfaces == 2) ? 2 : 1); // row
        col = (detID / numDet[1]) * ((myDetectorType == EIGER) ? 2 : 1) +
              i; // col for horiz. udp ports
        listener[i]->SetHardCodedPosition(row, col);
    }
}

std::string Implementation::getDetectorHostname() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return detHostname;
}

void Implementation::setDetectorHostname(const std::string& c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty())
        detHostname = c;
    FILE_LOG(logINFO) << "Detector Hostname: " << detHostname;
}

bool Implementation::getSilentMode() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return silentMode;
}

void Implementation::setSilentMode(const bool i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    silentMode = i;
    FILE_LOG(logINFO) << "Silent Mode: " << i;
}

uint32_t Implementation::getFifoDepth() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fifoDepth;
}

void Implementation::setFifoDepth(const uint32_t i) {
    if (fifoDepth != i) {
        fifoDepth = i;
        SetupFifoStructure();
    }
    FILE_LOG(logINFO) << "Fifo Depth: " << i;
}

slsDetectorDefs::frameDiscardPolicy
Implementation::getFrameDiscardPolicy() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return frameDiscardMode;
}

void Implementation::setFrameDiscardPolicy(
    const frameDiscardPolicy i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (i >= 0 && i < NUM_DISCARD_POLICIES)
        frameDiscardMode = i;

    FILE_LOG(logINFO) << "Frame Discard Policy: "
                      << sls::ToString(frameDiscardMode);
}

bool Implementation::getFramePaddingEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framePadding;
}

void Implementation::setFramePaddingEnable(const bool i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framePadding = i;
    FILE_LOG(logINFO) << "Frame Padding: " << framePadding;
}


/**************************************************
 *                                                 *
 *   File Parameters                               *
 *                                                 *
 * ************************************************/
slsDetectorDefs::fileFormat Implementation::getFileFormat() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileFormatType;
}

void Implementation::setFileFormat(const fileFormat f) {
    switch (f) {
#ifdef HDF5C
    case HDF5:
        fileFormatType = HDF5;
        break;
#endif
    default:
        fileFormatType = BINARY;
        break;
    }

    for (const auto &it : dataProcessor)
        it->SetFileFormat(f);

    FILE_LOG(logINFO) << "File Format: " << sls::ToString(fileFormatType);
}

std::string Implementation::getFilePath() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return filePath;
}

void Implementation::setFilePath(const std::string& c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty()) {
        mkdir_p(c); //throws if it can't create
        filePath = c;
    }
    FILE_LOG(logINFO) << "File path: " << filePath;
}

std::string Implementation::getFileName() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileName;
}

void Implementation::setFileName(const std::string& c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty())
       fileName = c;
    FILE_LOG(logINFO) << "File name: " << fileName;
}

uint64_t Implementation::getFileIndex() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileIndex;
}

void Implementation::setFileIndex(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    fileIndex = i;
    FILE_LOG(logINFO) << "File Index: " << fileIndex;
}

bool Implementation::getFileWriteEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileWriteEnable;
}

void Implementation::setFileWriteEnable(const bool b) {
    if (fileWriteEnable != b) {
        fileWriteEnable = b;
        for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
            dataProcessor[i]->SetupFileWriter(
                fileWriteEnable, (int *)numDet, &framesPerFile, &fileName,
                &filePath, &fileIndex, &overwriteEnable, &detID, &numThreads,
                &numberOfFrames, &dynamicRange, &udpPortNum[i], generalData);
        }
    }

    FILE_LOG(logINFO) << "File Write Enable: " << (fileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getMasterFileWriteEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return masterFileWriteEnable;
}

void Implementation::setMasterFileWriteEnable(const bool b) {
    masterFileWriteEnable = b;

    FILE_LOG(logINFO) << "Master File Write Enable: "
                      << (masterFileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getOverwriteEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return overwriteEnable;
}

void Implementation::setOverwriteEnable(const bool b) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    overwriteEnable = b;
    FILE_LOG(logINFO) << "Overwrite Enable: " << (overwriteEnable ? "enabled" : "disabled");
}

uint32_t Implementation::getFramesPerFile() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framesPerFile;
}

void Implementation::setFramesPerFile(const uint32_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framesPerFile = i;
    FILE_LOG(logINFO) << "Frames per file: " << framesPerFile;
}


/**************************************************
 *                                                 *
 *   Acquisition                                   *
 *                                                 *
 * ************************************************/
slsDetectorDefs::runStatus Implementation::getStatus() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return status;
}

uint64_t Implementation::getFramesCaught() const {
    uint64_t min = -1;
    uint32_t flagsum = 0;

    for (const auto &it : dataProcessor) {
        flagsum += it->GetStartedFlag();
        uint64_t curr = it->GetNumFramesCaught();
        min = curr < min ? curr : min;        
    }
    // no data processed
    if (flagsum != dataProcessor.size())
        return 0;

    return min;
}

uint64_t Implementation::getAcquisitionIndex() const {
    uint64_t min = -1;
    uint32_t flagsum = 0;

    for (const auto &it : dataProcessor) {
        flagsum += it->GetStartedFlag();
        uint64_t curr = it->GetCurrentFrameIndex();
        min = curr < min ? curr : min;
    }
    // no data processed
    if (flagsum != dataProcessor.size())
        return 0;

    return min;
}

std::vector<uint64_t> Implementation::getNumMissingPackets() const {
    std::vector<uint64_t> mp(numThreads);
    for (int i = 0; i < numThreads; i++) {
        int np = generalData->packetsPerFrame;
        uint64_t totnp = np;
        // partial readout
        if (numLinesReadout != MAX_EIGER_ROWS_PER_READOUT) {
            totnp = ((numLinesReadout * np) / MAX_EIGER_ROWS_PER_READOUT);
        }     
        totnp *= numberOfFrames;
        mp[i] = listener[i]->GetNumMissingPacket(stoppedFlag, totnp);
    }
    return mp;
}

void Implementation::startReceiver() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    FILE_LOG(logINFO) << "Starting Receiver";
    stoppedFlag = false;
    ResetParametersforNewAcquisition();

    // listener
    CreateUDPSockets();

    // callbacks
    if (startAcquisitionCallBack) {
        startAcquisitionCallBack(filePath, fileName, fileIndex,
                                 (generalData->imageSize) +
                                     (generalData->fifoBufferHeaderSize),
                                 pStartAcquisition);
        if (rawDataReadyCallBack != nullptr) {
            FILE_LOG(logINFO) << "Data Write has been defined externally";
        }
    }

    // processor->writer
    if (fileWriteEnable) {
        SetupWriter();
    } else
        FILE_LOG(logINFO) << "File Write Disabled";

    FILE_LOG(logINFO) << "Ready ...";

    // status
    status = RUNNING;

    // Let Threads continue to be ready for acquisition
    StartRunning();

    FILE_LOG(logINFO) << "Receiver Started";
    FILE_LOG(logINFO) << "Status: " << sls::ToString(status);
}

void Implementation::setStoppedFlag(bool stopped) {
    stoppedFlag = stopped;
}

void Implementation::stopReceiver() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    FILE_LOG(logINFO) << "Stopping Receiver";

    // set status to transmitting
    startReadout();

    // wait for the processes (Listener and DataProcessor) to be done
    bool running = true;
    while (running) {
        running = false;
        for (const auto &it : listener)
            if (it->IsRunning())
                running = true;

        for (const auto &it : dataProcessor)
            if (it->IsRunning())
                running = true;
        usleep(5000);
    }

    // create virtual file
    if (fileWriteEnable && fileFormatType == HDF5) {
        uint64_t maxIndexCaught = 0;
        bool anycaught = false;
        for (const auto &it : dataProcessor) {
            maxIndexCaught =
                std::max(maxIndexCaught, it->GetProcessedIndex());
            if (it->GetStartedFlag())
                anycaught = true;
        }
        // to create virtual file & set files/acquisition to 0 (only hdf5 at the
        // moment)
        dataProcessor[0]->EndofAcquisition(anycaught, maxIndexCaught);
    }

    // wait for the processes (dataStreamer) to be done
    running = true;
    while (running) {
        running = false;
        for (const auto &it : dataStreamer)
            if (it->IsRunning())
                running = true;
        usleep(5000);
    }

    status = RUN_FINISHED;
    FILE_LOG(logINFO) << "Status: " << sls::ToString(status);

    { // statistics
        std::vector<uint64_t> mp = getNumMissingPackets();
        uint64_t tot = 0;
        for (int i = 0; i < numThreads; i++) {
            int nf = dataProcessor[i]->GetNumFramesCaught();
            tot += nf;

            TLogLevel lev =
                (((int64_t)mp[i]) > 0) ? logINFORED : logINFOGREEN;
            FILE_LOG(lev) <<
                // udp port number could be the second if selected interface is
                // 2 for jungfrau
                "Summary of Port " << udpPortNum[i]
                          << "\n\tMissing Packets\t\t: " << mp[i]
                          << "\n\tComplete Frames\t\t: " << nf
                          << "\n\tLast Frame Caught\t: "
                          << listener[i]->GetLastFrameIndexCaught();
        }
        if (!activated) {
            FILE_LOG(logINFORED) << "Deactivated Receiver";
        }
        // callback
        if (acquisitionFinishedCallBack)
            acquisitionFinishedCallBack((tot / numThreads),
                                        pAcquisitionFinished);
    }

    // change status
    status = IDLE;

    FILE_LOG(logINFO) << "Receiver Stopped";
    FILE_LOG(logINFO) << "Status: " << sls::ToString(status);
}

void Implementation::startReadout() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (status == RUNNING) {
        // wait for incoming delayed packets
        int totalPacketsReceived = 0;
        int previousValue = -1;
        for (const auto &it : listener)
            totalPacketsReceived += it->GetPacketsCaught();

        // wait for all packets
        const int numPacketsToReceive =
            numberOfFrames * generalData->packetsPerFrame * listener.size();
        if (totalPacketsReceived != numPacketsToReceive) {
            while (totalPacketsReceived != previousValue) {
                FILE_LOG(logDEBUG3)
                    << "waiting for all packets, previousValue:"
                    << previousValue
                    << " totalPacketsReceived: " << totalPacketsReceived;
                usleep(5 * 1000); /* TODO! Need to find optimal time **/
                previousValue = totalPacketsReceived;
                totalPacketsReceived = 0;
                for (const auto &it : listener)
                    totalPacketsReceived += it->GetPacketsCaught();

                FILE_LOG(logDEBUG3) << "\tupdated:  totalPacketsReceived:"
                                    << totalPacketsReceived;
            }
        }
        status = TRANSMITTING;
        FILE_LOG(logINFO) << "Status: Transmitting";
    }
    // shut down udp sockets to make listeners push dummy (end) packets for
    // processors
    shutDownUDPSockets();
}

void Implementation::shutDownUDPSockets() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : listener)
        it->ShutDownUDPSocket();
}

void Implementation::closeFiles() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    uint64_t maxIndexCaught = 0;
    bool anycaught = false;
    for (const auto &it : dataProcessor) {
        it->CloseFiles();
        maxIndexCaught =
            std::max(maxIndexCaught, it->GetProcessedIndex());
        if (it->GetStartedFlag())
            anycaught = true;
    }
    // to create virtual file & set files/acquisition to 0 (only hdf5 at the
    // moment)
    dataProcessor[0]->EndofAcquisition(anycaught, maxIndexCaught);
}

void Implementation::restreamStop() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : dataStreamer) {
        it->RestreamStop();
    }
    FILE_LOG(logINFO) << "Restreaming Dummy Header via ZMQ successful";
}

void Implementation::ResetParametersforNewAcquisition() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    for (const auto &it : listener)
        it->ResetParametersforNewAcquisition();
    for (const auto &it : dataProcessor)
        it->ResetParametersforNewAcquisition();

    if (dataStreamEnable) {
        std::ostringstream os;
        os << filePath << '/' << fileName;
        std::string fnametostream = os.str();
        for (const auto &it : dataStreamer)
            it->ResetParametersforNewAcquisition(fnametostream);
    }
}

void Implementation::CreateUDPSockets() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    try{
        for (unsigned int i = 0; i < listener.size(); ++i) {
            listener[i]->CreateUDPSockets();
        }
    } catch(const sls::RuntimeError &e) {
        shutDownUDPSockets();
        throw sls::RuntimeError("Could not create UDP Socket(s).");
    }

    FILE_LOG(logDEBUG) << "UDP socket(s) created successfully.";
}

void Implementation::SetupWriter() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
	masterAttributes attr;
	attr.detectorType = myDetectorType;
	attr.dynamicRange = dynamicRange;
	attr.tenGiga = tengigaEnable;
	attr.imageSize = generalData->imageSize;
	attr.nPixelsX = generalData->nPixelsX;
	attr.nPixelsY = generalData->nPixelsY;
	attr.maxFramesPerFile = framesPerFile;
	attr.totalFrames = numberOfFrames;
	attr.exptimeNs = acquisitionTime;
	attr.subExptimeNs = subExpTime;
	attr.subPeriodNs = subPeriod;
	attr.periodNs = acquisitionPeriod;
	attr.gapPixelsEnable = gapPixelsEnable;
    attr.quadEnable = quadEnable;
    attr.analogFlag = (readoutType == ANALOG_ONLY || readoutType == ANALOG_AND_DIGITAL) ? 1 : 0;
    attr.digitalFlag = (readoutType == DIGITAL_ONLY || readoutType == ANALOG_AND_DIGITAL) ? 1 : 0;
    attr.adcmask = tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga;
    attr.dbitoffset = ctbDbitOffset;
    attr.dbitlist = 0;
    attr.roiXmin = roi.xmin;
    attr.roiXmax = roi.xmax;
    for (auto &i : ctbDbitList) {
        attr.dbitlist |= (1 << i);
    }

    try {
        for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
            dataProcessor[i]->CreateNewFile(attr);
        }
    } catch(const sls::RuntimeError &e) {
        shutDownUDPSockets();
        closeFiles();
        throw sls::RuntimeError("Could not create file.");
    }
}

void Implementation::StartRunning() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    // set running mask and post semaphore to start the inner loop in execution
    // thread
    for (const auto &it : listener) {
        it->StartRunning();
        it->Continue();
    }
    for (const auto &it : dataProcessor) {
        it->StartRunning();
        it->Continue();
    }
    for (const auto &it : dataStreamer) {
        it->StartRunning();
        it->Continue();
    }
}


/**************************************************
 *                                                 *
 *   Network Configuration (UDP)                   *
 *                                                 *
 * ************************************************/
int Implementation::getNumberofUDPInterfaces() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numUDPInterfaces;
}

void Implementation::setNumberofUDPInterfaces(const int n) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (numUDPInterfaces != n) {

        // reduce number of detectors in y dir (rows) if it had 2 interfaces
        // before
        if (numUDPInterfaces == 2)
            numDet[Y] /= 2;

        numUDPInterfaces = n;

        // clear all threads and fifos
        listener.clear();
        dataProcessor.clear();
        dataStreamer.clear();
        fifo.clear();

        // set local variables
        generalData->SetNumberofInterfaces(n);
        numThreads = generalData->threadsPerReceiver;
        udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;

        // fifo
        SetupFifoStructure();

        // create threads
        for (int i = 0; i < numThreads; ++i) {
            // listener and dataprocessor threads
            try {
                auto fifo_ptr = fifo[i].get();
                listener.push_back(sls::make_unique<Listener>(
                    i, myDetectorType, fifo_ptr, &status, &udpPortNum[i],
                    &eth[i], &numberOfFrames, &dynamicRange,
                    &udpSocketBufferSize, &actualUDPSocketBufferSize,
                    &framesPerFile, &frameDiscardMode, &activated,
                    &deactivatedPaddingEnable, &silentMode));
                listener[i]->SetGeneralData(generalData);

                dataProcessor.push_back(sls::make_unique<DataProcessor>(
                    i, myDetectorType, fifo_ptr, &fileFormatType,
                    fileWriteEnable, &masterFileWriteEnable, &dataStreamEnable,
                    &gapPixelsEnable, &dynamicRange, &streamingFrequency,
                    &streamingTimerInMs, &framePadding, &activated,
                    &deactivatedPaddingEnable, &silentMode, &quadEnable, &ctbDbitList,
                    &ctbDbitOffset, &ctbAnalogDataBytes));
                dataProcessor[i]->SetGeneralData(generalData);
            } catch (...) {
                listener.clear();
                dataProcessor.clear();
                throw sls::RuntimeError("Could not create listener/dataprocessor threads (index:" + std::to_string(i) + ")");
            }
            // streamer threads
            if (dataStreamEnable) {
                try {
                    int fd = flippedDataX;
                    int nd[2] = {numDet[0], numDet[1]};
                    if (quadEnable) {
                        fd = i;
                        nd[0] = 1;
                        nd[1] = 2;
                    }
                    dataStreamer.push_back(sls::make_unique<DataStreamer>(
                        i, fifo[i].get(), &dynamicRange, &roi, &fileIndex,
                        fd, &additionalJsonHeader, (int*)nd, &gapPixelsEnable, &quadEnable));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numThreads, streamingPort, streamingSrcIP);

                } catch (...) {
                    if (dataStreamEnable) {
                        dataStreamer.clear();
                        dataStreamEnable = false;
                    }
                    throw sls::RuntimeError("Could not create datastreamer threads (index:" + std::to_string(i) + ")");
                }
            }
        }

        SetThreadPriorities();

        // update (from 1 to 2 interface) & also for printout
        setMultiDetectorSize(numDet);
        // update row and column in dataprocessor
        setDetectorPositionId(detID);

        // update call backs
        if (rawDataReadyCallBack) {
            for (const auto &it : dataProcessor)
                it->registerCallBackRawDataReady(rawDataReadyCallBack,
                                                 pRawDataReady);
        }
        if (rawDataModifyReadyCallBack) {
            for (const auto &it : dataProcessor)
                it->registerCallBackRawDataModifyReady(
                    rawDataModifyReadyCallBack, pRawDataReady);
        }

        // test socket buffer size with current set up
        setUDPSocketBufferSize(0);
    }

    FILE_LOG(logINFO) << "Number of Interfaces: " << numUDPInterfaces;
}

std::string Implementation::getEthernetInterface() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return eth[0];
}

void Implementation::setEthernetInterface(const std::string &c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    eth[0] = c;
    FILE_LOG(logINFO) << "Ethernet Interface: " << eth[0];
}

std::string Implementation::getEthernetInterface2() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return eth[1];
}

void Implementation::setEthernetInterface2(const std::string &c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    eth[1] = c;
    FILE_LOG(logINFO) << "Ethernet Interface 2: " << eth[1];
}

uint32_t Implementation::getUDPPortNumber() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpPortNum[0];
}

void Implementation::setUDPPortNumber(const uint32_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    udpPortNum[0] = i;
    FILE_LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum[0];
}

uint32_t Implementation::getUDPPortNumber2() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpPortNum[1];
}

void Implementation::setUDPPortNumber2(const uint32_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    udpPortNum[1] = i;
    FILE_LOG(logINFO) << "UDP Port Number[1]: " << udpPortNum[1];
}

int64_t Implementation::getUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpSocketBufferSize;
}

void Implementation::setUDPSocketBufferSize(const int64_t s) {
    int64_t size = (s == 0) ? udpSocketBufferSize : s;
    size_t listSize = listener.size();

    if (myDetectorType == JUNGFRAU && (int)listSize != numUDPInterfaces) {
        throw sls::RuntimeError("Number of Interfaces " + std::to_string(numUDPInterfaces) + " do not match listener size " + std::to_string(listSize));
    }

    for (unsigned int i = 0; i < listSize; ++i) {
        listener[i]->CreateDummySocketForUDPSocketBufferSize(size);
    }
}

int64_t Implementation::getActualUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return actualUDPSocketBufferSize;
}


/**************************************************
 *                                                 *
 *   ZMQ Streaming Parameters (ZMQ)                *
 *                                                 *
 * ************************************************/
bool Implementation::getDataStreamEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return dataStreamEnable;
}

void Implementation::setDataStreamEnable(const bool enable) {

    if (dataStreamEnable != enable) {
        dataStreamEnable = enable;

        // data sockets have to be created again as the client ones are
        dataStreamer.clear();

        if (enable) {
            for (int i = 0; i < numThreads; ++i) {
                try {
                    int fd = flippedDataX;
                    int nd[2] = {numDet[0], numDet[1]};
                    if (quadEnable) {
                        fd = i;
                        nd[0] = 1;
                        nd[1] = 2;
                    }
                    dataStreamer.push_back(sls::make_unique<DataStreamer>(
                        i, fifo[i].get(), &dynamicRange, &roi, &fileIndex,
                        fd, &additionalJsonHeader, (int*)nd, &gapPixelsEnable, &quadEnable));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numThreads, streamingPort, streamingSrcIP);
                } catch (...) {
                    dataStreamer.clear();
                    dataStreamEnable = false;
                    throw sls::RuntimeError("Could not set data stream enable.");
                }
            }
            SetThreadPriorities();
        }
    }
    FILE_LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;
}

uint32_t Implementation::getStreamingFrequency() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingFrequency;
}

void Implementation::setStreamingFrequency(const uint32_t freq) {
    if (streamingFrequency != freq) {
        streamingFrequency = freq;
    }
    FILE_LOG(logINFO) << "Streaming Frequency: " << streamingFrequency;
}

uint32_t Implementation::getStreamingTimer() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingTimerInMs;
}

void Implementation::setStreamingTimer(const uint32_t time_in_ms) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    streamingTimerInMs = time_in_ms;
    FILE_LOG(logINFO) << "Streamer Timer: " << streamingTimerInMs;
}

uint32_t Implementation::getStreamingPort() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingPort;
}

void Implementation::setStreamingPort(const uint32_t i) {
    streamingPort = i;

    FILE_LOG(logINFO) << "Streaming Port: " << streamingPort;
}

sls::IpAddr Implementation::getStreamingSourceIP() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingSrcIP;
}

void Implementation::setStreamingSourceIP(const sls::IpAddr ip) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    streamingSrcIP = ip;
    FILE_LOG(logINFO) << "Streaming Source IP: " << streamingSrcIP;
}

std::string Implementation::getAdditionalJsonHeader() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return additionalJsonHeader;
}

void Implementation::setAdditionalJsonHeader(const std::string& c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    additionalJsonHeader = c;
    FILE_LOG(logINFO) << "Additional JSON Header: " << additionalJsonHeader;
}


/**************************************************
 *                                                 *
 *   Detector Parameters                           *
 *                                                 *
 * ************************************************/
uint64_t Implementation::getNumberOfFrames() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfFrames;
}

void Implementation::setNumberOfFrames(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    numberOfFrames = i;
    FILE_LOG(logINFO) << "Number of Frames: " << numberOfFrames;
}

uint64_t Implementation::getAcquisitionPeriod() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionPeriod;
}

void Implementation::setAcquisitionPeriod(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionPeriod = i;
    FILE_LOG(logINFO) << "Acquisition Period: "
                      << (double)acquisitionPeriod / (1E9) << "s";
}

uint64_t Implementation::getAcquisitionTime() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionTime;
}

void Implementation::setAcquisitionTime(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionTime = i;
    FILE_LOG(logINFO) << "Acquisition Time: " << (double)acquisitionTime / (1E9)
                      << "s";
}

uint64_t Implementation::getSubExpTime() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subExpTime;
}

void Implementation::setSubExpTime(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subExpTime = i;
    FILE_LOG(logINFO) << "Sub Exposure Time: " << (double)subExpTime / (1E9)
                      << "s";
}

uint64_t Implementation::getSubPeriod() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subPeriod;
}

void Implementation::setSubPeriod(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subPeriod = i;
    FILE_LOG(logINFO) << "Sub Exposure Period: " << (double)subPeriod / (1E9)
                      << "s";
}

uint32_t Implementation::getNumberofAnalogSamples() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfAnalogSamples;
}

void Implementation::setNumberofAnalogSamples(const uint32_t i) {
    if (numberOfAnalogSamples != i) {
        numberOfAnalogSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }
    FILE_LOG(logINFO) << "Number of Analog Samples: " << numberOfAnalogSamples;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

uint32_t Implementation::getNumberofDigitalSamples() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfDigitalSamples;
}

void Implementation::setNumberofDigitalSamples(const uint32_t i) {
    if (numberOfDigitalSamples != i) {
        numberOfDigitalSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }
    FILE_LOG(logINFO) << "Number of Digital Samples: "
                      << numberOfDigitalSamples;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

int Implementation::getNumberofCounters() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfCounters;
}

void Implementation::setNumberofCounters(const int i) {
    if (numberOfCounters != i) {
        numberOfCounters = i;

        if (myDetectorType == MYTHEN3) {
            generalData->SetDynamicRange(i, tengigaEnable);
            // to update npixelsx, npixelsy in file writer
            for (const auto &it : dataProcessor)
                it->SetPixelDimension();
            SetupFifoStructure();
        }
    }
    FILE_LOG(logINFO) << "Number of Counters: " << numberOfCounters;
}

uint32_t Implementation::getDynamicRange() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return dynamicRange;
}

void Implementation::setDynamicRange(const uint32_t i) {

    if (dynamicRange != i) {
        dynamicRange = i;

        if (myDetectorType == EIGER || myDetectorType == MYTHEN3) {
            generalData->SetDynamicRange(i, tengigaEnable);
            if (myDetectorType == EIGER) {
                generalData->SetGapPixelsEnable(gapPixelsEnable, dynamicRange, quadEnable);
            }
            // to update npixelsx, npixelsy in file writer
            for (const auto &it : dataProcessor)
                it->SetPixelDimension();
            SetupFifoStructure();
        }
    }
    FILE_LOG(logINFO) << "Dynamic Range: " << dynamicRange;
}

slsDetectorDefs::ROI Implementation::getROI() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return roi;
}

void Implementation::setROI(slsDetectorDefs::ROI arg) {
    if (roi.xmin != arg.xmin || roi.xmax != arg.xmax) {
        roi.xmin = arg.xmin;
        roi.xmax = arg.xmax;

        // only for gotthard
        generalData->SetROI(arg);
        framesPerFile = generalData->maxFramesPerFile;
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }

    FILE_LOG(logINFO) << "ROI: [" << roi.xmin << ", " << roi.xmax << "]";;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

bool Implementation::getTenGigaEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return tengigaEnable;
}

void Implementation::setTenGigaEnable(const bool b) {
    if (tengigaEnable != b) {
        tengigaEnable = b;
        // side effects
        switch (myDetectorType) {
        case EIGER:
            generalData->SetTenGigaEnable(b, dynamicRange);
            generalData->SetGapPixelsEnable(gapPixelsEnable, dynamicRange, quadEnable);
            break;
        case MOENCH:
        case CHIPTESTBOARD:
            ctbAnalogDataBytes = generalData->setImageSize(
                tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
                numberOfAnalogSamples, numberOfDigitalSamples,
                tengigaEnable, readoutType);
            break;
        default:
            break;
        }

        SetupFifoStructure();
    }
    FILE_LOG(logINFO) << "Ten Giga: " << (tengigaEnable ? "enabled" : "disabled");
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

int Implementation::getFlippedDataX() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return flippedDataX;
}

void Implementation::setFlippedDataX(int enable) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    flippedDataX = (enable == 0) ? 0 : 1;

	if (!quadEnable) {
		for (const auto &it : dataStreamer) {
			it->SetFlippedDataX(flippedDataX);
		}	
	} 
	else {
		if (dataStreamer.size() == 2) {
			dataStreamer[0]->SetFlippedDataX(0);
			dataStreamer[1]->SetFlippedDataX(1);	
		}
	}

    FILE_LOG(logINFO) << "Flipped Data X: " << flippedDataX;
}

bool Implementation::getGapPixelsEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return gapPixelsEnable;
}

void Implementation::setGapPixelsEnable(const bool b) {
    if (gapPixelsEnable != b) {
        gapPixelsEnable = b;

        // side effects
        generalData->SetGapPixelsEnable(b, dynamicRange, quadEnable);
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }
    FILE_LOG(logINFO) << "Gap Pixels Enable: " << gapPixelsEnable;
}

bool Implementation::getQuad() const {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	return quadEnable;
}

void Implementation::setQuad(const bool b) {
	if (quadEnable != b) {
		quadEnable = b;

		generalData->SetGapPixelsEnable(gapPixelsEnable, dynamicRange, b);
		// to update npixelsx, npixelsy in file writer
        for (const auto &it : dataProcessor)
		    it->SetPixelDimension();
		SetupFifoStructure();

		if (!quadEnable) {
			for (const auto &it : dataStreamer) {
				it->SetNumberofDetectors(numDet);
				it->SetFlippedDataX(flippedDataX);
			}
		} else {
			int size[2] = {1, 2};
			for (const auto &it : dataStreamer) {
			    it->SetNumberofDetectors(size);
			}
			if (dataStreamer.size() == 2) {
				dataStreamer[0]->SetFlippedDataX(0);
				dataStreamer[1]->SetFlippedDataX(1);	
			}	
		}
	}
	FILE_LOG(logINFO)  << "Quad Enable: " << quadEnable;
}

bool Implementation::getActivate() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return activated;
}

bool Implementation::setActivate(bool enable) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    activated = enable;
    FILE_LOG(logINFO) << "Activation: " << (activated ? "enabled" : "disabled");
    return activated;
}

bool Implementation::getDeactivatedPadding() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return deactivatedPaddingEnable;
}

bool Implementation::setDeactivatedPadding(bool enable) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    deactivatedPaddingEnable = enable;
    FILE_LOG(logINFO) << "Deactivated Padding Enable: "
                      << (deactivatedPaddingEnable ? "enabled" : "disabled");
    return deactivatedPaddingEnable;
}

int Implementation::getReadNLines() const {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	return numLinesReadout;
}

void Implementation::setReadNLines(const int value) {
    numLinesReadout = value;
	FILE_LOG(logINFO)  << "Number of Lines to readout: " << numLinesReadout;
}

slsDetectorDefs::readoutMode
Implementation::getReadoutMode() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return readoutType;
}

void Implementation::setReadoutMode(const readoutMode f) {
    if (readoutType != f) {
        readoutType = f;

        // side effects
        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }

    FILE_LOG(logINFO) << "Readout Mode: " << sls::ToString(f);
    FILE_LOG(logINFO) << "Packets per Frame: "
                          << (generalData->packetsPerFrame);
}

uint32_t Implementation::getADCEnableMask() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return adcEnableMaskOneGiga;
}

void Implementation::setADCEnableMask(uint32_t mask) {
    if (adcEnableMaskOneGiga != mask) {
        adcEnableMaskOneGiga = mask;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);

        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }

    FILE_LOG(logINFO) << "ADC Enable Mask for 1Gb mode: 0x" << std::hex << adcEnableMaskOneGiga
                      << std::dec;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

uint32_t Implementation::getTenGigaADCEnableMask() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return adcEnableMaskTenGiga;
}

void Implementation::setTenGigaADCEnableMask(uint32_t mask) {
    if (adcEnableMaskTenGiga != mask) {
        adcEnableMaskTenGiga = mask;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);

        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }

    FILE_LOG(logINFO) << "ADC Enable Mask for 10Gb mode: 0x" << std::hex << adcEnableMaskTenGiga
                      << std::dec;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

std::vector<int> Implementation::getDbitList() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitList;
}

void Implementation::setDbitList(const std::vector<int> v) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitList = v;
}

int Implementation::getDbitOffset() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitOffset;
}

void Implementation::setDbitOffset(const int s) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitOffset = s;
}


/**************************************************
 *                                                *
 *    Callbacks                                   *
 *                                                *
 * ************************************************/
void Implementation::registerCallBackStartAcquisition(
    int (*func)(std::string, std::string, uint64_t, uint32_t, void *), void *arg) {
    startAcquisitionCallBack = func;
    pStartAcquisition = arg;
}

void Implementation::registerCallBackAcquisitionFinished(
    void (*func)(uint64_t, void *), void *arg) {
    acquisitionFinishedCallBack = func;
    pAcquisitionFinished = arg;
}

void Implementation::registerCallBackRawDataReady(
    void (*func)(char *, char *, uint32_t, void *), void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
    for (const auto &it : dataProcessor)
        it->registerCallBackRawDataReady(rawDataReadyCallBack, pRawDataReady);
}

void Implementation::registerCallBackRawDataModifyReady(
    void (*func)(char *, char *, uint32_t &, void *), void *arg) {
    rawDataModifyReadyCallBack = func;
    pRawDataReady = arg;
    for (const auto &it : dataProcessor)
        it->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,
                                               pRawDataReady);
}

