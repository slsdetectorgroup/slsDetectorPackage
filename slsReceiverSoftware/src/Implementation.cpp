#include "Implementation.h"
#include "DataProcessor.h"
#include "DataStreamer.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "Listener.h"
#include "MasterAttributes.h"
#include "ToString.h"
#include "ZmqSocket.h" //just for the zmq port define
#include "file_utils.h"

#include <cerrno> //eperm
#include <chrono>
#include <cstdlib> //system
#include <cstring>
#include <cstring> //strcpy
#include <fstream>
#include <iostream>
#include <sys/stat.h> // stat
#include <thread>
#include <unistd.h>

/** cosntructor & destructor */

Implementation::Implementation(const detectorType d) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    InitializeMembers();
    setDetectorType(d);
}

Implementation::~Implementation() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    DeleteMembers();
}

void Implementation::DeleteMembers() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (generalData) {
        delete generalData;
        generalData = nullptr;
    }

    additionalJsonHeader.clear();
    listener.clear();
    dataProcessor.clear();
    dataStreamer.clear();
    fifo.clear();
    eth.clear();
    udpPortNum.clear();
    rateCorrections.clear();
    ctbDbitList.clear();
}

void Implementation::InitializeMembers() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    // config parameters
    numThreads = 1;
    myDetectorType = GENERIC;
    for (int i = 0; i < MAX_DIMENSIONS; ++i)
        numDet[i] = 0;
    modulePos = 0;
    detHostname = "";
    silentMode = false;
    fifoDepth = 0;
    frameDiscardMode = NO_DISCARD;
    framePadding = true;

    // file parameters
    fileFormatType = BINARY;
    filePath = "/";
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
        udpPortNum[i] = DEFAULT_UDP_PORTNO + i;
    }
    udpSocketBufferSize = 0;
    actualUDPSocketBufferSize = 0;

    // zmq parameters
    dataStreamEnable = false;
    streamingFrequency = 1;
    streamingTimerInMs = DEFAULT_STREAMING_TIMER_IN_MS;
    streamingStartFnum = 0;
    streamingPort = 0;
    streamingSrcIP = sls::IpAddr{};

    // detector parameters
    numberOfTotalFrames = 0;
    numberOfFrames = 1;
    numberOfTriggers = 1;
    numberOfBursts = 1;
    numberOfAdditionalStorageCells = 0;
    numberOfGates = 0;
    timingMode = AUTO_TIMING;
    burstMode = BURST_INTERNAL;
    acquisitionPeriod = SAMPLE_TIME_IN_NS;
    acquisitionTime = 0;
    acquisitionTime1 = 0;
    acquisitionTime2 = 0;
    acquisitionTime3 = 0;
    gateDelay1 = 0;
    gateDelay2 = 0;
    gateDelay3 = 0;
    subExpTime = 0;
    subPeriod = 0;
    numberOfAnalogSamples = 0;
    numberOfDigitalSamples = 0;
    counterMask = 0;
    dynamicRange = 16;
    roi.xmin = -1;
    roi.xmax = -1;
    tengigaEnable = false;
    flippedDataX = 0;
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
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

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
            LOG(logINFOBLUE)
                << "Max length of input packet queue "
                   "[/proc/sys/net/core/netdev_max_backlog] modified to "
                << MAX_SOCKET_INPUT_PACKET_QUEUE;
        } else {
            LOG(logWARNING)
                << "Could not change max length of "
                   "input packet queue [net.core.netdev_max_backlog]. (No Root "
                   "Privileges?)";
        }
    }
}

void Implementation::SetThreadPriorities() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : listener) {
        it->SetThreadPriority(LISTENER_PRIORITY);
    }
}

void Implementation::SetupFifoStructure() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    fifo.clear();
    for (int i = 0; i < numThreads; ++i) {
        uint32_t datasize = generalData->imageSize;
        // veto data size
        if (myDetectorType == GOTTHARD2 && i != 0) {
            datasize = generalData->vetoImageSize;
        }

        // create fifo structure
        try {
            fifo.push_back(sls::make_unique<Fifo>(
                i, datasize + (generalData->fifoBufferHeaderSize), fifoDepth));
        } catch (...) {
            fifo.clear();
            fifoDepth = 0;
            throw sls::RuntimeError(
                "Could not allocate memory for fifo structure " +
                std::to_string(i) + ". FifoDepth is now 0.");
        }
        // set the listener & dataprocessor threads to point to the right fifo
        if (listener.size())
            listener[i]->SetFifo(fifo[i].get());
        if (dataProcessor.size())
            dataProcessor[i]->SetFifo(fifo[i].get());
        if (dataStreamer.size())
            dataStreamer[i]->SetFifo(fifo[i].get());

        LOG(logINFO) << "Memory Allocated for Fifo " << i << ": "
                     << (double)(((size_t)(datasize) +
                                  (size_t)(generalData->fifoBufferHeaderSize)) *
                                 (size_t)fifoDepth) /
                            (double)(1024 * 1024)
                     << " MB";
    }

    LOG(logINFO) << numThreads << " Fifo structure(s) reconstructed";
}

/**************************************************
 *                                                 *
 *   Configuration Parameters                      *
 *                                                 *
 * ************************************************/

void Implementation::setDetectorType(const detectorType d) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    myDetectorType = d;
    switch (myDetectorType) {
    case GOTTHARD:
    case EIGER:
    case JUNGFRAU:
    case CHIPTESTBOARD:
    case MOENCH:
    case MYTHEN3:
    case GOTTHARD2:
        LOG(logINFO) << " ***** " << sls::ToString(d) << " Receiver *****";
        break;
    default:
        throw sls::RuntimeError("This is an unknown receiver type " +
                                std::to_string(static_cast<int>(d)));
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
    case GOTTHARD2:
        generalData = new Gotthard2Data();
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
                &numberOfTotalFrames, &dynamicRange, &udpSocketBufferSize,
                &actualUDPSocketBufferSize, &framesPerFile, &frameDiscardMode,
                &activated, &deactivatedPaddingEnable, &silentMode));
            dataProcessor.push_back(sls::make_unique<DataProcessor>(
                i, myDetectorType, fifo_ptr, &fileFormatType, fileWriteEnable,
                &masterFileWriteEnable, &dataStreamEnable, &dynamicRange,
                &streamingFrequency, &streamingTimerInMs, &streamingStartFnum,
                &framePadding, &activated, &deactivatedPaddingEnable,
                &silentMode, &quadEnable, &ctbDbitList, &ctbDbitOffset,
                &ctbAnalogDataBytes));
        } catch (...) {
            listener.clear();
            dataProcessor.clear();
            throw sls::RuntimeError(
                "Could not create listener/dataprocessor threads (index:" +
                std::to_string(i) + ")");
        }
    }

    // set up writer and callbacks

    for (const auto &it : listener)
        it->SetGeneralData(generalData);
    for (const auto &it : dataProcessor)
        it->SetGeneralData(generalData);
    SetThreadPriorities();

    LOG(logDEBUG) << " Detector type set to " << sls::ToString(d);
}

int *Implementation::getDetectorSize() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return (int *)numDet;
}

void Implementation::setDetectorSize(const int *size) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
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

    LOG(logINFO) << log_message;
}

int Implementation::getModulePositionId() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return modulePos;
}

void Implementation::setModulePositionId(const int id) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    modulePos = id;
    LOG(logINFO) << "Module Position Id:" << modulePos;

    // update zmq port
    streamingPort =
        DEFAULT_ZMQ_RX_PORTNO + (modulePos * (myDetectorType == EIGER ? 2 : 1));

    for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
        dataProcessor[i]->SetupFileWriter(
            fileWriteEnable, (int *)numDet, &framesPerFile, &fileName,
            &filePath, &fileIndex, &overwriteEnable, &modulePos, &numThreads,
            &numberOfTotalFrames, &dynamicRange, &udpPortNum[i], generalData);
    }
    assert(numDet[1] != 0);
    for (unsigned int i = 0; i < listener.size(); ++i) {
        uint16_t row = 0, col = 0;
        row =
            (modulePos % numDet[1]) * ((numUDPInterfaces == 2) ? 2 : 1); // row
        col = (modulePos / numDet[1]) * ((myDetectorType == EIGER) ? 2 : 1) +
              i; // col for horiz. udp ports
        listener[i]->SetHardCodedPosition(row, col);
    }
}

std::string Implementation::getDetectorHostname() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return detHostname;
}

void Implementation::setDetectorHostname(const std::string &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty())
        detHostname = c;
    LOG(logINFO) << "Detector Hostname: " << detHostname;
}

bool Implementation::getSilentMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return silentMode;
}

void Implementation::setSilentMode(const bool i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    silentMode = i;
    LOG(logINFO) << "Silent Mode: " << i;
}

uint32_t Implementation::getFifoDepth() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fifoDepth;
}

void Implementation::setFifoDepth(const uint32_t i) {
    if (fifoDepth != i) {
        fifoDepth = i;
        SetupFifoStructure();
    }
    LOG(logINFO) << "Fifo Depth: " << i;
}

slsDetectorDefs::frameDiscardPolicy
Implementation::getFrameDiscardPolicy() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return frameDiscardMode;
}

void Implementation::setFrameDiscardPolicy(const frameDiscardPolicy i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (i >= 0 && i < NUM_DISCARD_POLICIES)
        frameDiscardMode = i;

    LOG(logINFO) << "Frame Discard Policy: " << sls::ToString(frameDiscardMode);
}

bool Implementation::getFramePaddingEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framePadding;
}

void Implementation::setFramePaddingEnable(const bool i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framePadding = i;
    LOG(logINFO) << "Frame Padding: " << framePadding;
}

void Implementation::setThreadIds(const pid_t parentTid, const pid_t tcpTid) {
    parentThreadId = parentTid;
    tcpThreadId = tcpTid;
}

std::array<pid_t, NUM_RX_THREAD_IDS> Implementation::getThreadIds() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    std::array<pid_t, NUM_RX_THREAD_IDS> retval{};
    int id = 0;
    retval[id++] = parentThreadId;
    retval[id++] = tcpThreadId;
    retval[id++] = listener[0]->GetThreadId();
    retval[id++] = dataProcessor[0]->GetThreadId();
    if (dataStreamEnable) {
        retval[id++] = dataStreamer[0]->GetThreadId();
    } else {
        retval[id++] = 0;
    }
    if (numThreads == 2) {
        retval[id++] = listener[1]->GetThreadId();
        retval[id++] = dataProcessor[1]->GetThreadId();
        if (dataStreamEnable) {
            retval[id++] = dataStreamer[1]->GetThreadId();
        } else {
            retval[id++] = 0;
        }
    }
    return retval;
}

/**************************************************
 *                                                 *
 *   File Parameters                               *
 *                                                 *
 * ************************************************/
slsDetectorDefs::fileFormat Implementation::getFileFormat() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
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

    LOG(logINFO) << "File Format: " << sls::ToString(fileFormatType);
}

std::string Implementation::getFilePath() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return filePath;
}

void Implementation::setFilePath(const std::string &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty()) {
        mkdir_p(c); // throws if it can't create
        filePath = c;
    }
    LOG(logINFO) << "File path: " << filePath;
}

std::string Implementation::getFileName() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileName;
}

void Implementation::setFileName(const std::string &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty())
        fileName = c;
    LOG(logINFO) << "File name: " << fileName;
}

uint64_t Implementation::getFileIndex() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileIndex;
}

void Implementation::setFileIndex(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    fileIndex = i;
    LOG(logINFO) << "File Index: " << fileIndex;
}

bool Implementation::getFileWriteEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileWriteEnable;
}

void Implementation::setFileWriteEnable(const bool b) {
    if (fileWriteEnable != b) {
        fileWriteEnable = b;
        for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
            dataProcessor[i]->SetupFileWriter(
                fileWriteEnable, (int *)numDet, &framesPerFile, &fileName,
                &filePath, &fileIndex, &overwriteEnable, &modulePos,
                &numThreads, &numberOfTotalFrames, &dynamicRange,
                &udpPortNum[i], generalData);
        }
    }

    LOG(logINFO) << "File Write Enable: "
                 << (fileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getMasterFileWriteEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return masterFileWriteEnable;
}

void Implementation::setMasterFileWriteEnable(const bool b) {
    masterFileWriteEnable = b;

    LOG(logINFO) << "Master File Write Enable: "
                 << (masterFileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getOverwriteEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return overwriteEnable;
}

void Implementation::setOverwriteEnable(const bool b) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    overwriteEnable = b;
    LOG(logINFO) << "Overwrite Enable: "
                 << (overwriteEnable ? "enabled" : "disabled");
}

uint32_t Implementation::getFramesPerFile() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framesPerFile;
}

void Implementation::setFramesPerFile(const uint32_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framesPerFile = i;
    LOG(logINFO) << "Frames per file: " << framesPerFile;
}

/**************************************************
 *                                                 *
 *   Acquisition                                   *
 *                                                 *
 * ************************************************/
slsDetectorDefs::runStatus Implementation::getStatus() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
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

int Implementation::getProgress() const {
    // get minimum of processed frame indices
    uint64_t currentFrameIndex = -1;
    uint32_t flagsum = 0;

    for (const auto &it : dataProcessor) {
        flagsum += it->GetStartedFlag();
        uint64_t curr = it->GetProcessedIndex();
        currentFrameIndex = curr < currentFrameIndex ? curr : currentFrameIndex;
    }
    // no data processed
    if (flagsum != dataProcessor.size()) {
        currentFrameIndex = -1;
    }

    return (100.00 *
            ((double)(currentFrameIndex + 1) / (double)numberOfTotalFrames));
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
        totnp *= numberOfTotalFrames;
        mp[i] = listener[i]->GetNumMissingPacket(stoppedFlag, totnp);
    }
    return mp;
}

void Implementation::startReceiver() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    LOG(logINFO) << "Starting Receiver";
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
            LOG(logINFO) << "Data Write has been defined externally";
        }
    }

    // processor->writer
    if (fileWriteEnable) {
        SetupWriter();
    } else
        LOG(logINFO) << "File Write Disabled";

    LOG(logINFO) << "Ready ...";

    // status
    status = RUNNING;

    // Let Threads continue to be ready for acquisition
    StartRunning();

    LOG(logINFO) << "Receiver Started";
    LOG(logINFO) << "Status: " << sls::ToString(status);
}

void Implementation::setStoppedFlag(bool stopped) { stoppedFlag = stopped; }

void Implementation::stopReceiver() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    LOG(logINFO) << "Stopping Receiver";

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
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // create virtual file
    if (fileWriteEnable && fileFormatType == HDF5) {
        uint64_t maxIndexCaught = 0;
        bool anycaught = false;
        for (const auto &it : dataProcessor) {
            maxIndexCaught = std::max(maxIndexCaught, it->GetProcessedIndex());
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
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    status = RUN_FINISHED;
    LOG(logINFO) << "Status: " << sls::ToString(status);

    { // statistics
        std::vector<uint64_t> mp = getNumMissingPackets();
        uint64_t tot = 0;
        for (int i = 0; i < numThreads; i++) {
            int nf = dataProcessor[i]->GetNumFramesCaught();
            tot += nf;

            TLogLevel lev = (((int64_t)mp[i]) > 0) ? logINFORED : logINFOGREEN;
            LOG(lev) <<
                // udp port number could be the second if selected interface is
                // 2 for jungfrau
                "Summary of Port " << udpPortNum[i]
                     << "\n\tMissing Packets\t\t: " << mp[i]
                     << "\n\tComplete Frames\t\t: " << nf
                     << "\n\tLast Frame Caught\t: "
                     << listener[i]->GetLastFrameIndexCaught();
        }
        if (!activated) {
            LOG(logINFORED) << "Deactivated Receiver";
        }
        // callback
        if (acquisitionFinishedCallBack)
            acquisitionFinishedCallBack((tot / numThreads),
                                        pAcquisitionFinished);
    }

    // change status
    status = IDLE;

    LOG(logINFO) << "Receiver Stopped";
    LOG(logINFO) << "Status: " << sls::ToString(status);
}

void Implementation::startReadout() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (status == RUNNING) {
        // wait for incoming delayed packets
        int totalPacketsReceived = 0;
        int previousValue = -1;
        for (const auto &it : listener)
            totalPacketsReceived += it->GetPacketsCaught();

        // wait for all packets
        const int numPacketsToReceive = numberOfTotalFrames *
                                        generalData->packetsPerFrame *
                                        listener.size();
        if (totalPacketsReceived != numPacketsToReceive) {
            while (totalPacketsReceived != previousValue) {
                LOG(logDEBUG3)
                    << "waiting for all packets, previousValue:"
                    << previousValue
                    << " totalPacketsReceived: " << totalPacketsReceived;
                /* TODO! Need to find optimal time **/
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                previousValue = totalPacketsReceived;
                totalPacketsReceived = 0;
                for (const auto &it : listener)
                    totalPacketsReceived += it->GetPacketsCaught();

                LOG(logDEBUG3) << "\tupdated:  totalPacketsReceived:"
                               << totalPacketsReceived;
            }
        }
        status = TRANSMITTING;
        LOG(logINFO) << "Status: Transmitting";
    }
    // shut down udp sockets to make listeners push dummy (end) packets for
    // processors
    shutDownUDPSockets();
}

void Implementation::shutDownUDPSockets() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : listener)
        it->ShutDownUDPSocket();
}

void Implementation::closeFiles() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    uint64_t maxIndexCaught = 0;
    bool anycaught = false;
    for (const auto &it : dataProcessor) {
        it->CloseFiles();
        maxIndexCaught = std::max(maxIndexCaught, it->GetProcessedIndex());
        if (it->GetStartedFlag())
            anycaught = true;
    }
    // to create virtual file & set files/acquisition to 0 (only hdf5 at the
    // moment)
    dataProcessor[0]->EndofAcquisition(anycaught, maxIndexCaught);
}

void Implementation::restreamStop() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : dataStreamer) {
        it->RestreamStop();
    }
    LOG(logINFO) << "Restreaming Dummy Header via ZMQ successful";
}

void Implementation::ResetParametersforNewAcquisition() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

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
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    try {
        for (unsigned int i = 0; i < listener.size(); ++i) {
            listener[i]->CreateUDPSockets();
        }
    } catch (const sls::RuntimeError &e) {
        shutDownUDPSockets();
        throw sls::RuntimeError("Could not create UDP Socket(s).");
    }

    LOG(logDEBUG) << "UDP socket(s) created successfully.";
}

void Implementation::SetupWriter() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    std::unique_ptr<MasterAttributes> masterAttributes;
    switch (myDetectorType) {
    case GOTTHARD:
        masterAttributes = sls::make_unique<GotthardMasterAttributes>();
        break;
    case JUNGFRAU:
        masterAttributes = sls::make_unique<JungfrauMasterAttributes>();
        break;
    case EIGER:
        masterAttributes = sls::make_unique<EigerMasterAttributes>();
        break;
    case MYTHEN3:
        masterAttributes = sls::make_unique<Mythen3MasterAttributes>();
        break;
    case GOTTHARD2:
        masterAttributes = sls::make_unique<Gotthard2MasterAttributes>();
        break;
    case MOENCH:
        masterAttributes = sls::make_unique<MoenchMasterAttributes>();
        break;
    case CHIPTESTBOARD:
        masterAttributes = sls::make_unique<CtbMasterAttributes>();
        break;
    default:
        throw sls::RuntimeError(
            "Unknown detector type to set up master file attributes");
    }
    masterAttributes->detType = myDetectorType;
    masterAttributes->imageSize = generalData->imageSize;
    masterAttributes->nPixels =
        xy(generalData->nPixelsX, generalData->nPixelsY);
    masterAttributes->maxFramesPerFile = framesPerFile;
    masterAttributes->totalFrames = numberOfTotalFrames;
    masterAttributes->exptime = std::chrono::nanoseconds(acquisitionTime);
    masterAttributes->period = std::chrono::nanoseconds(acquisitionPeriod);
    masterAttributes->dynamicRange = dynamicRange;
    masterAttributes->tenGiga = tengigaEnable;
    masterAttributes->subExptime = std::chrono::nanoseconds(subExpTime);
    masterAttributes->subPeriod = std::chrono::nanoseconds(subPeriod);
    masterAttributes->quad = quadEnable;
    masterAttributes->ratecorr = rateCorrections;
    masterAttributes->adcmask =
        tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga;
    masterAttributes->analog =
        (readoutType == ANALOG_ONLY || readoutType == ANALOG_AND_DIGITAL) ? 1
                                                                          : 0;
    masterAttributes->digital =
        (readoutType == DIGITAL_ONLY || readoutType == ANALOG_AND_DIGITAL) ? 1
                                                                           : 0;
    masterAttributes->dbitoffset = ctbDbitOffset;
    masterAttributes->dbitlist = 0;
    for (auto &i : ctbDbitList) {
        masterAttributes->dbitlist |= (1 << i);
    }
    masterAttributes->roi = roi;
    masterAttributes->counterMask = counterMask;
    masterAttributes->exptime1 = std::chrono::nanoseconds(acquisitionTime1);
    masterAttributes->exptime2 = std::chrono::nanoseconds(acquisitionTime2);
    masterAttributes->exptime3 = std::chrono::nanoseconds(acquisitionTime3);
    masterAttributes->gateDelay1 = std::chrono::nanoseconds(gateDelay1);
    masterAttributes->gateDelay2 = std::chrono::nanoseconds(gateDelay2);
    masterAttributes->gateDelay3 = std::chrono::nanoseconds(gateDelay3);
    masterAttributes->gates = numberOfGates;

    try {
        for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
            dataProcessor[i]->CreateNewFile(masterAttributes.get());
        }
    } catch (const sls::RuntimeError &e) {
        shutDownUDPSockets();
        closeFiles();
        throw sls::RuntimeError("Could not create file.");
    }
}

void Implementation::StartRunning() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
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
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numUDPInterfaces;
}

void Implementation::setNumberofUDPInterfaces(const int n) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

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
                    &eth[i], &numberOfTotalFrames, &dynamicRange,
                    &udpSocketBufferSize, &actualUDPSocketBufferSize,
                    &framesPerFile, &frameDiscardMode, &activated,
                    &deactivatedPaddingEnable, &silentMode));
                listener[i]->SetGeneralData(generalData);

                dataProcessor.push_back(sls::make_unique<DataProcessor>(
                    i, myDetectorType, fifo_ptr, &fileFormatType,
                    fileWriteEnable, &masterFileWriteEnable, &dataStreamEnable,
                    &dynamicRange, &streamingFrequency, &streamingTimerInMs,
                    &streamingStartFnum, &framePadding, &activated,
                    &deactivatedPaddingEnable, &silentMode, &quadEnable,
                    &ctbDbitList, &ctbDbitOffset, &ctbAnalogDataBytes));
                dataProcessor[i]->SetGeneralData(generalData);
            } catch (...) {
                listener.clear();
                dataProcessor.clear();
                throw sls::RuntimeError(
                    "Could not create listener/dataprocessor threads (index:" +
                    std::to_string(i) + ")");
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
                        i, fifo[i].get(), &dynamicRange, &roi, &fileIndex, fd,
                        (int *)nd, &quadEnable, &numberOfTotalFrames));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numThreads, streamingPort, streamingSrcIP);
                    dataStreamer[i]->SetAdditionalJsonHeader(
                        additionalJsonHeader);

                } catch (...) {
                    if (dataStreamEnable) {
                        dataStreamer.clear();
                        dataStreamEnable = false;
                    }
                    throw sls::RuntimeError(
                        "Could not create datastreamer threads (index:" +
                        std::to_string(i) + ")");
                }
            }
        }

        SetThreadPriorities();

        // update (from 1 to 2 interface) & also for printout
        setDetectorSize(numDet);
        // update row and column in dataprocessor
        setModulePositionId(modulePos);

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

    LOG(logINFO) << "Number of Interfaces: " << numUDPInterfaces;
}

std::string Implementation::getEthernetInterface() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return eth[0];
}

void Implementation::setEthernetInterface(const std::string &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    eth[0] = c;
    LOG(logINFO) << "Ethernet Interface: " << eth[0];
}

std::string Implementation::getEthernetInterface2() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return eth[1];
}

void Implementation::setEthernetInterface2(const std::string &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    eth[1] = c;
    LOG(logINFO) << "Ethernet Interface 2: " << eth[1];
}

uint32_t Implementation::getUDPPortNumber() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpPortNum[0];
}

void Implementation::setUDPPortNumber(const uint32_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    udpPortNum[0] = i;
    LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum[0];
}

uint32_t Implementation::getUDPPortNumber2() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpPortNum[1];
}

void Implementation::setUDPPortNumber2(const uint32_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    udpPortNum[1] = i;
    LOG(logINFO) << "UDP Port Number[1]: " << udpPortNum[1];
}

int64_t Implementation::getUDPSocketBufferSize() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpSocketBufferSize;
}

void Implementation::setUDPSocketBufferSize(const int64_t s) {
    int64_t size = (s == 0) ? udpSocketBufferSize : s;
    size_t listSize = listener.size();

    if (myDetectorType == JUNGFRAU && (int)listSize != numUDPInterfaces) {
        throw sls::RuntimeError(
            "Number of Interfaces " + std::to_string(numUDPInterfaces) +
            " do not match listener size " + std::to_string(listSize));
    }

    for (unsigned int i = 0; i < listSize; ++i) {
        listener[i]->CreateDummySocketForUDPSocketBufferSize(size);
    }
}

int64_t Implementation::getActualUDPSocketBufferSize() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return actualUDPSocketBufferSize;
}

/**************************************************
 *                                                 *
 *   ZMQ Streaming Parameters (ZMQ)                *
 *                                                 *
 * ************************************************/
bool Implementation::getDataStreamEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
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
                        i, fifo[i].get(), &dynamicRange, &roi, &fileIndex, fd,
                        (int *)nd, &quadEnable, &numberOfTotalFrames));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numThreads, streamingPort, streamingSrcIP);
                    dataStreamer[i]->SetAdditionalJsonHeader(
                        additionalJsonHeader);
                } catch (...) {
                    dataStreamer.clear();
                    dataStreamEnable = false;
                    throw sls::RuntimeError(
                        "Could not set data stream enable.");
                }
            }
            SetThreadPriorities();
        }
    }
    LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;
}

uint32_t Implementation::getStreamingFrequency() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingFrequency;
}

void Implementation::setStreamingFrequency(const uint32_t freq) {
    if (streamingFrequency != freq) {
        streamingFrequency = freq;
    }
    LOG(logINFO) << "Streaming Frequency: " << streamingFrequency;
}

uint32_t Implementation::getStreamingTimer() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingTimerInMs;
}

void Implementation::setStreamingTimer(const uint32_t time_in_ms) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    streamingTimerInMs = time_in_ms;
    LOG(logINFO) << "Streamer Timer: " << streamingTimerInMs;
}

uint32_t Implementation::getStreamingStartingFrameNumber() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingStartFnum;
}

void Implementation::setStreamingStartingFrameNumber(const uint32_t fnum) {
    if (streamingStartFnum != fnum) {
        streamingStartFnum = fnum;
    }
    LOG(logINFO) << "Streaming Start Frame num: " << streamingStartFnum;
}

uint32_t Implementation::getStreamingPort() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingPort;
}

void Implementation::setStreamingPort(const uint32_t i) {
    streamingPort = i;

    LOG(logINFO) << "Streaming Port: " << streamingPort;
}

sls::IpAddr Implementation::getStreamingSourceIP() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingSrcIP;
}

void Implementation::setStreamingSourceIP(const sls::IpAddr ip) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    streamingSrcIP = ip;
    LOG(logINFO) << "Streaming Source IP: " << streamingSrcIP;
}

std::map<std::string, std::string>
Implementation::getAdditionalJsonHeader() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return additionalJsonHeader;
}

void Implementation::setAdditionalJsonHeader(
    const std::map<std::string, std::string> &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    additionalJsonHeader = c;
    for (const auto &it : dataStreamer) {
        it->SetAdditionalJsonHeader(c);
    }
    LOG(logINFO) << "Additional JSON Header: "
                 << sls::ToString(additionalJsonHeader);
}

std::string
Implementation::getAdditionalJsonParameter(const std::string &key) const {
    if (additionalJsonHeader.find(key) != additionalJsonHeader.end()) {
        return additionalJsonHeader.at(key);
    }
    throw sls::RuntimeError("No key " + key +
                            " found in additional json header");
}

void Implementation::setAdditionalJsonParameter(const std::string &key,
                                                const std::string &value) {
    auto pos = additionalJsonHeader.find(key);

    // if value is empty, delete
    if (value.empty()) {
        // doesnt exist
        if (pos == additionalJsonHeader.end()) {
            LOG(logINFO) << "Additional json parameter (" << key
                         << ") does not exist anyway";
        } else {
            LOG(logINFO) << "Deleting additional json parameter (" << key
                         << ")";
            additionalJsonHeader.erase(pos);
        }
    }
    // if found, set it
    else if (pos != additionalJsonHeader.end()) {
        additionalJsonHeader[key] = value;
        LOG(logINFO) << "Setting additional json parameter (" << key << ") to "
                     << value;
    }
    // append if not found
    else {
        additionalJsonHeader[key] = value;
        LOG(logINFO) << "Adding additional json parameter (" << key << ") to "
                     << value;
    }
    for (const auto &it : dataStreamer) {
        it->SetAdditionalJsonHeader(additionalJsonHeader);
    }
    LOG(logINFO) << "Additional JSON Header: "
                 << sls::ToString(additionalJsonHeader);
}

/**************************************************
 *                                                 *
 *   Detector Parameters                           *
 *                                                 *
 * ************************************************/
void Implementation::updateTotalNumberOfFrames() {
    int64_t repeats = numberOfTriggers;
    // gotthard2: auto mode
    // burst mode: (bursts instead of triggers)
    // non burst mode: no bursts or triggers
    if (myDetectorType == GOTTHARD2 && timingMode == AUTO_TIMING) {
        if (burstMode != BURST_OFF) {
            repeats = numberOfBursts;
        } else {
            repeats = 1;
        }
    }
    numberOfTotalFrames = numberOfFrames * repeats *
                          (int64_t)(numberOfAdditionalStorageCells + 1);
    if (numberOfTotalFrames == 0) {
        throw sls::RuntimeError("Invalid total number of frames to receive: 0");
    }
    LOG(logINFO) << "Total Number of Frames: " << numberOfTotalFrames;
}

uint64_t Implementation::getNumberOfFrames() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfFrames;
}

void Implementation::setNumberOfFrames(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfFrames = i;
    LOG(logINFO) << "Number of Frames: " << numberOfFrames;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getNumberOfTriggers() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfTriggers;
}

void Implementation::setNumberOfTriggers(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfTriggers = i;
    LOG(logINFO) << "Number of Triggers: " << numberOfTriggers;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getNumberOfBursts() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfBursts;
}

void Implementation::setNumberOfBursts(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfBursts = i;
    LOG(logINFO) << "Number of Bursts: " << numberOfBursts;
    updateTotalNumberOfFrames();
}

int Implementation::getNumberOfAdditionalStorageCells() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfAdditionalStorageCells;
}

void Implementation::setNumberOfAdditionalStorageCells(const int i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfAdditionalStorageCells = i;
    LOG(logINFO) << "Number of Additional Storage Cells: "
                 << numberOfAdditionalStorageCells;
    updateTotalNumberOfFrames();
}

void Implementation::setNumberOfGates(const int i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfGates = i;
    LOG(logINFO) << "Number of Gates: " << numberOfGates;
}

slsDetectorDefs::timingMode Implementation::getTimingMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return timingMode;
}

void Implementation::setTimingMode(const slsDetectorDefs::timingMode i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    timingMode = i;
    LOG(logINFO) << "Timing Mode: " << timingMode;
    updateTotalNumberOfFrames();
}

slsDetectorDefs::burstMode Implementation::getBurstMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return burstMode;
}

void Implementation::setBurstMode(const slsDetectorDefs::burstMode i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    burstMode = i;
    LOG(logINFO) << "Burst Mode: " << burstMode;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getAcquisitionPeriod() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionPeriod;
}

void Implementation::setAcquisitionPeriod(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionPeriod = i;
    LOG(logINFO) << "Acquisition Period: " << (double)acquisitionPeriod / (1E9)
                 << "s";
}

uint64_t Implementation::getAcquisitionTime() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionTime;
}

void Implementation::updateAcquisitionTime() {
    if (acquisitionTime1 == acquisitionTime2 &&
        acquisitionTime2 == acquisitionTime3) {
        acquisitionTime = acquisitionTime1;
    } else {
        acquisitionTime = -1;
    }
}

void Implementation::setAcquisitionTime(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionTime = i;
    LOG(logINFO) << "Acquisition Time: " << (double)acquisitionTime / (1E9)
                 << "s";
}

void Implementation::setAcquisitionTime1(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionTime1 = i;
    LOG(logINFO) << "Acquisition Time1: " << (double)acquisitionTime1 / (1E9)
                 << "s";
    updateAcquisitionTime();
}

void Implementation::setAcquisitionTime2(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionTime2 = i;
    LOG(logINFO) << "Acquisition Time2: " << (double)acquisitionTime2 / (1E9)
                 << "s";
    updateAcquisitionTime();
}

void Implementation::setAcquisitionTime3(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionTime3 = i;
    LOG(logINFO) << "Acquisition Time3: " << (double)acquisitionTime3 / (1E9)
                 << "s";
    updateAcquisitionTime();
}

void Implementation::setGateDelay1(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    gateDelay1 = i;
    LOG(logINFO) << "Gate Delay1: " << (double)gateDelay1 / (1E9) << "s";
}

void Implementation::setGateDelay2(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    gateDelay2 = i;
    LOG(logINFO) << "Gate Delay2: " << (double)gateDelay2 / (1E9) << "s";
}

void Implementation::setGateDelay3(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    gateDelay3 = i;
    LOG(logINFO) << "Gate Delay3: " << (double)gateDelay3 / (1E9) << "s";
}

uint64_t Implementation::getSubExpTime() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subExpTime;
}

void Implementation::setSubExpTime(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subExpTime = i;
    LOG(logINFO) << "Sub Exposure Time: " << (double)subExpTime / (1E9) << "s";
}

uint64_t Implementation::getSubPeriod() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subPeriod;
}

void Implementation::setSubPeriod(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subPeriod = i;
    LOG(logINFO) << "Sub Period: " << (double)subPeriod / (1E9) << "s";
}

uint32_t Implementation::getNumberofAnalogSamples() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfAnalogSamples;
}

void Implementation::setNumberofAnalogSamples(const uint32_t i) {
    if (numberOfAnalogSamples != i) {
        numberOfAnalogSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga,
            numberOfAnalogSamples, numberOfDigitalSamples, tengigaEnable,
            readoutType);

        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Analog Samples: " << numberOfAnalogSamples;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getNumberofDigitalSamples() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfDigitalSamples;
}

void Implementation::setNumberofDigitalSamples(const uint32_t i) {
    if (numberOfDigitalSamples != i) {
        numberOfDigitalSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga,
            numberOfAnalogSamples, numberOfDigitalSamples, tengigaEnable,
            readoutType);

        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Digital Samples: " << numberOfDigitalSamples;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getCounterMask() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return counterMask;
}

void Implementation::setCounterMask(const uint32_t i) {
    if (counterMask != i) {
        int ncounters = __builtin_popcount(i);
        if (ncounters < 1 || ncounters > 3) {
            throw sls::RuntimeError("Invalid number of counters " +
                                    std::to_string(ncounters) +
                                    ". Expected 1-3.");
        }
        counterMask = i;
        generalData->SetNumberofCounters(ncounters, dynamicRange,
                                         tengigaEnable);
        // to update npixelsx, npixelsy in file writer
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }
    LOG(logINFO) << "Counter mask: " << sls::ToStringHex(counterMask);
    int ncounters = __builtin_popcount(counterMask);
    LOG(logINFO) << "Number of counters: " << ncounters;
}

uint32_t Implementation::getDynamicRange() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return dynamicRange;
}

void Implementation::setDynamicRange(const uint32_t i) {

    if (dynamicRange != i) {
        dynamicRange = i;

        if (myDetectorType == EIGER || myDetectorType == MYTHEN3) {

            if (myDetectorType == EIGER) {
                generalData->SetDynamicRange(i, tengigaEnable);
            } else {
                int ncounters = __builtin_popcount(counterMask);
                generalData->SetNumberofCounters(ncounters, i, tengigaEnable);
            }

            // to update npixelsx, npixelsy in file writer
            for (const auto &it : dataProcessor)
                it->SetPixelDimension();
            fifoDepth = generalData->defaultFifoDepth;
            SetupFifoStructure();
        }
    }
    LOG(logINFO) << "Dynamic Range: " << dynamicRange;
}

slsDetectorDefs::ROI Implementation::getROI() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
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

    LOG(logINFO) << "ROI: [" << roi.xmin << ", " << roi.xmax << "]";
    ;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

bool Implementation::getTenGigaEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return tengigaEnable;
}

void Implementation::setTenGigaEnable(const bool b) {
    if (tengigaEnable != b) {
        tengigaEnable = b;
        int ncounters = __builtin_popcount(counterMask);
        // side effects
        switch (myDetectorType) {
        case EIGER:
            generalData->SetTenGigaEnable(b, dynamicRange);
            break;
        case MYTHEN3:
            generalData->SetNumberofCounters(ncounters, dynamicRange, b);
            break;
        case MOENCH:
        case CHIPTESTBOARD:
            ctbAnalogDataBytes = generalData->setImageSize(
                tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga,
                numberOfAnalogSamples, numberOfDigitalSamples, tengigaEnable,
                readoutType);
            break;
        default:
            break;
        }

        SetupFifoStructure();
    }
    LOG(logINFO) << "Ten Giga: " << (tengigaEnable ? "enabled" : "disabled");
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

int Implementation::getFlippedDataX() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return flippedDataX;
}

void Implementation::setFlippedDataX(int enable) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    flippedDataX = (enable == 0) ? 0 : 1;

    if (!quadEnable) {
        for (const auto &it : dataStreamer) {
            it->SetFlippedDataX(flippedDataX);
        }
    } else {
        if (dataStreamer.size() == 2) {
            dataStreamer[0]->SetFlippedDataX(0);
            dataStreamer[1]->SetFlippedDataX(1);
        }
    }

    LOG(logINFO) << "Flipped Data X: " << flippedDataX;
}

bool Implementation::getQuad() const {
    LOG(logDEBUG) << __AT__ << " starting";
    return quadEnable;
}

void Implementation::setQuad(const bool b) {
    if (quadEnable != b) {
        quadEnable = b;

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
    LOG(logINFO) << "Quad Enable: " << quadEnable;
}

bool Implementation::getActivate() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return activated;
}

bool Implementation::setActivate(bool enable) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    activated = enable;
    LOG(logINFO) << "Activation: " << (activated ? "enabled" : "disabled");
    return activated;
}

bool Implementation::getDeactivatedPadding() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return deactivatedPaddingEnable;
}

bool Implementation::setDeactivatedPadding(bool enable) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    deactivatedPaddingEnable = enable;
    LOG(logINFO) << "Deactivated Padding Enable: "
                 << (deactivatedPaddingEnable ? "enabled" : "disabled");
    return deactivatedPaddingEnable;
}

int Implementation::getReadNLines() const {
    LOG(logDEBUG) << __AT__ << " starting";
    return numLinesReadout;
}

void Implementation::setReadNLines(const int value) {
    numLinesReadout = value;
    LOG(logINFO) << "Number of Lines to readout: " << numLinesReadout;
}

void Implementation::setRateCorrections(const std::vector<int64_t> &t) {
    rateCorrections = t;
    LOG(logINFO) << "Rate Corrections: " << sls::ToString(rateCorrections);
}

slsDetectorDefs::readoutMode Implementation::getReadoutMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return readoutType;
}

void Implementation::setReadoutMode(const readoutMode f) {
    if (readoutType != f) {
        readoutType = f;

        // side effects
        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga,
            numberOfAnalogSamples, numberOfDigitalSamples, tengigaEnable,
            readoutType);
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }

    LOG(logINFO) << "Readout Mode: " << sls::ToString(f);
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getADCEnableMask() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return adcEnableMaskOneGiga;
}

void Implementation::setADCEnableMask(uint32_t mask) {
    if (adcEnableMaskOneGiga != mask) {
        adcEnableMaskOneGiga = mask;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga,
            numberOfAnalogSamples, numberOfDigitalSamples, tengigaEnable,
            readoutType);

        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }

    LOG(logINFO) << "ADC Enable Mask for 1Gb mode: 0x" << std::hex
                 << adcEnableMaskOneGiga << std::dec;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getTenGigaADCEnableMask() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return adcEnableMaskTenGiga;
}

void Implementation::setTenGigaADCEnableMask(uint32_t mask) {
    if (adcEnableMaskTenGiga != mask) {
        adcEnableMaskTenGiga = mask;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga,
            numberOfAnalogSamples, numberOfDigitalSamples, tengigaEnable,
            readoutType);

        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        SetupFifoStructure();
    }

    LOG(logINFO) << "ADC Enable Mask for 10Gb mode: 0x" << std::hex
                 << adcEnableMaskTenGiga << std::dec;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

std::vector<int> Implementation::getDbitList() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitList;
}

void Implementation::setDbitList(const std::vector<int> v) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitList = v;
}

int Implementation::getDbitOffset() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitOffset;
}

void Implementation::setDbitOffset(const int s) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitOffset = s;
}

/**************************************************
 *                                                *
 *    Callbacks                                   *
 *                                                *
 * ************************************************/
void Implementation::registerCallBackStartAcquisition(
    int (*func)(std::string, std::string, uint64_t, uint32_t, void *),
    void *arg) {
    startAcquisitionCallBack = func;
    pStartAcquisition = arg;
}

void Implementation::registerCallBackAcquisitionFinished(void (*func)(uint64_t,
                                                                      void *),
                                                         void *arg) {
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
