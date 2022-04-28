// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Implementation.h"
#include "DataProcessor.h"
#include "DataStreamer.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "Listener.h"
#include "MasterAttributes.h"
#include "sls/ToString.h"
#include "sls/ZmqSocket.h" //just for the zmq port define
#include "sls/file_utils.h"

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

Implementation::Implementation(const detectorType d) { setDetectorType(d); }

Implementation::~Implementation() {
    delete generalData;
    generalData = nullptr;
}

void Implementation::SetLocalNetworkParameters() {
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
    for (const auto &it : listener)
        it->SetThreadPriority(LISTENER_PRIORITY);
}

void Implementation::SetupFifoStructure() {
    fifo.clear();
    for (int i = 0; i < numUDPInterfaces; ++i) {
        uint32_t datasize = generalData->imageSize;
        // veto data size
        if (detType == GOTTHARD2 && i != 0) {
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
    LOG(logINFO) << numUDPInterfaces << " Fifo structure(s) reconstructed";
}

/**************************************************
 *                                                 *
 *   Configuration Parameters                      *
 *                                                 *
 * ************************************************/

void Implementation::setDetectorType(const detectorType d) {

    detType = d;
    switch (detType) {
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

    delete generalData;
    generalData = nullptr;

    // set detector specific variables
    switch (detType) {
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

    framesPerFile = generalData->maxFramesPerFile;
    fifoDepth = generalData->defaultFifoDepth;
    numUDPInterfaces = generalData->numUDPInterfaces;
    udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;
    dynamicRange = generalData->dynamicRange;
    tengigaEnable = generalData->tengigaEnable;
    numberOfAnalogSamples = generalData->nAnalogSamples;
    numberOfDigitalSamples = generalData->nDigitalSamples;
    readoutType = generalData->readoutType;
    adcEnableMaskOneGiga = generalData->adcEnableMaskOneGiga;
    adcEnableMaskTenGiga = generalData->adcEnableMaskTenGiga;
    roi = generalData->roi;
    counterMask = generalData->counterMask;

    SetLocalNetworkParameters();
    SetupFifoStructure();

    // create threads
    for (int i = 0; i < numUDPInterfaces; ++i) {

        try {
            auto fifo_ptr = fifo[i].get();
            listener.push_back(sls::make_unique<Listener>(
                i, detType, fifo_ptr, &status, &udpPortNum[i], &eth[i],
                &udpSocketBufferSize, &actualUDPSocketBufferSize,
                &framesPerFile, &frameDiscardMode, &activated,
                &detectorDataStream[i], &silentMode));
            int ctbAnalogDataBytes = 0;
            if (detType == CHIPTESTBOARD) {
                ctbAnalogDataBytes = generalData->GetNumberOfAnalogDatabytes();
            }
            dataProcessor.push_back(sls::make_unique<DataProcessor>(
                i, detType, fifo_ptr, &activated, &dataStreamEnable,
                &streamingFrequency, &streamingTimerInMs, &streamingStartFnum,
                &framePadding, &ctbDbitList, &ctbDbitOffset,
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

slsDetectorDefs::xy Implementation::getDetectorSize() const {
    return numModules;
}

slsDetectorDefs::xy Implementation::GetPortGeometry() {
    xy portGeometry{1, 1};
    if (detType == EIGER)
        portGeometry.x = numUDPInterfaces;
    else if (detType == JUNGFRAU)
        portGeometry.y = numUDPInterfaces;
    return portGeometry;
}

void Implementation::setDetectorSize(const slsDetectorDefs::xy size) {
    xy portGeometry = GetPortGeometry();

    std::string log_message = "Detector Size (ports): (";
    numModules = size;
    numPorts.x = portGeometry.x * size.x;
    numPorts.y = portGeometry.y * size.y;
    if (quadEnable) {
        numPorts.x = 1;
        numPorts.y = 2;
    }
    for (const auto &it : dataStreamer) {
        it->SetNumberofPorts(numPorts);
    }

    LOG(logINFO) << "Detector Size (ports): " << sls::ToString(numPorts);
}

int Implementation::getModulePositionId() const { return modulePos; }

void Implementation::setModulePositionId(const int id) {
    modulePos = id;
    LOG(logINFO) << "Module Position Id:" << modulePos;

    // update zmq port
    xy portGeometry = GetPortGeometry();
    streamingPort = DEFAULT_ZMQ_RX_PORTNO + modulePos * portGeometry.x;

    assert(numModules.y != 0);
    for (unsigned int i = 0; i < listener.size(); ++i) {
        uint16_t row = 0, col = 0;
        row = (modulePos % numModules.y) * portGeometry.y;
        col = (modulePos / numModules.y) * portGeometry.x;
        if (portGeometry.y == 2) {
            row += i;
        }
        if (portGeometry.x == 2) {
            col += i;
        }
        LOG(logDEBUG1) << i << ":numModules:" << numModules.x << ","
                       << numModules.y << " portGeometry:" << portGeometry.x
                       << "," << portGeometry.y;
        listener[i]->SetHardCodedPosition(row, col);
    }
}

std::string Implementation::getDetectorHostname() const { return detHostname; }

void Implementation::setDetectorHostname(const std::string &c) {
    if (!c.empty())
        detHostname = c;
    LOG(logINFO) << "Detector Hostname: " << detHostname;
}

bool Implementation::getSilentMode() const { return silentMode; }

void Implementation::setSilentMode(const bool i) {
    silentMode = i;
    LOG(logINFO) << "Silent Mode: " << i;
}

uint32_t Implementation::getFifoDepth() const { return fifoDepth; }

void Implementation::setFifoDepth(const uint32_t i) {
    if (fifoDepth != i) {
        fifoDepth = i;
        SetupFifoStructure();
    }
    LOG(logINFO) << "Fifo Depth: " << i;
}

slsDetectorDefs::frameDiscardPolicy
Implementation::getFrameDiscardPolicy() const {
    return frameDiscardMode;
}

void Implementation::setFrameDiscardPolicy(const frameDiscardPolicy i) {
    frameDiscardMode = i;
    LOG(logINFO) << "Frame Discard Policy: " << sls::ToString(frameDiscardMode);
}

bool Implementation::getFramePaddingEnable() const { return framePadding; }

void Implementation::setFramePaddingEnable(const bool i) {
    framePadding = i;
    LOG(logINFO) << "Frame Padding: " << framePadding;
}

void Implementation::setThreadIds(const pid_t parentTid, const pid_t tcpTid) {
    parentThreadId = parentTid;
    tcpThreadId = tcpTid;
}

std::array<pid_t, NUM_RX_THREAD_IDS> Implementation::getThreadIds() const {
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
    if (numUDPInterfaces == 2) {
        retval[id++] = listener[1]->GetThreadId();
        retval[id++] = dataProcessor[1]->GetThreadId();
        if (dataStreamEnable) {
            retval[id++] = dataStreamer[1]->GetThreadId();
        } else {
            retval[id++] = 0;
        }
    }
    retval[NUM_RX_THREAD_IDS - 1] = arping.GetThreadId();
    return retval;
}

bool Implementation::getArping() const { return arping.IsRunning(); }

pid_t Implementation::getArpingThreadId() const { return arping.GetThreadId(); }

void Implementation::setArping(const bool i,
                               const std::vector<std::string> ips) {
    if (i != arping.IsRunning()) {
        if (!i) {
            arping.StopThread();
        } else {
            // setup interface
            for (int i = 0; i != numUDPInterfaces; ++i) {
                // ignore eiger with 2 interfaces (only udp port)
                if (i == 1 && (numUDPInterfaces == 1 || detType == EIGER)) {
                    break;
                }
                arping.SetInterfacesAndIps(i, eth[i], ips[i]);
            }
            arping.StartThread();
        }
    }
}

/**************************************************
 *                                                 *
 *   File Parameters                               *
 *                                                 *
 * ************************************************/
slsDetectorDefs::fileFormat Implementation::getFileFormat() const {
    return fileFormatType;
}

void Implementation::setFileFormat(const fileFormat f) {
    if (f != fileFormatType) {
        switch (f) {
#ifdef HDF5C
        case HDF5:
            fileFormatType = HDF5;
            break;
#endif
        case BINARY:
            fileFormatType = BINARY;
            break;
        default:
            throw sls::RuntimeError("Unknown file format");
        }
        for (const auto &it : dataProcessor)
            it->SetupFileWriter(fileWriteEnable, fileFormatType, &hdf5LibMutex);
    }

    LOG(logINFO) << "File Format: " << sls::ToString(fileFormatType);
}

std::string Implementation::getFilePath() const { return filePath; }

void Implementation::setFilePath(const std::string &c) {
    if (!c.empty()) {
        sls::mkdir_p(c); // throws if it can't create
        filePath = c;
    }
    LOG(logINFO) << "File path: " << filePath;
}

std::string Implementation::getFileName() const { return fileName; }

void Implementation::setFileName(const std::string &c) {
    fileName = c;
    LOG(logINFO) << "File name: " << fileName;
}

uint64_t Implementation::getFileIndex() const { return fileIndex; }

void Implementation::setFileIndex(const uint64_t i) {
    fileIndex = i;
    LOG(logINFO) << "File Index: " << fileIndex;
}

bool Implementation::getFileWriteEnable() const { return fileWriteEnable; }

void Implementation::setFileWriteEnable(const bool b) {
    if (fileWriteEnable != b) {
        fileWriteEnable = b;
        for (const auto &it : dataProcessor)
            it->SetupFileWriter(fileWriteEnable, fileFormatType, &hdf5LibMutex);
    }
    LOG(logINFO) << "File Write Enable: "
                 << (fileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getMasterFileWriteEnable() const {
    return masterFileWriteEnable;
}

void Implementation::setMasterFileWriteEnable(const bool b) {
    if (masterFileWriteEnable != b) {
        masterFileWriteEnable = b;
    }
    LOG(logINFO) << "Master File Write Enable: "
                 << (masterFileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getOverwriteEnable() const { return overwriteEnable; }

void Implementation::setOverwriteEnable(const bool b) {
    overwriteEnable = b;
    LOG(logINFO) << "Overwrite Enable: "
                 << (overwriteEnable ? "enabled" : "disabled");
}

uint32_t Implementation::getFramesPerFile() const { return framesPerFile; }

void Implementation::setFramesPerFile(const uint32_t i) {
    framesPerFile = i;
    LOG(logINFO) << "Frames per file: " << framesPerFile;
}

/**************************************************
 *                                                 *
 *   Acquisition                                   *
 *                                                 *
 * ************************************************/
slsDetectorDefs::runStatus Implementation::getStatus() const { return status; }

std::vector<int64_t> Implementation::getFramesCaught() const {
    std::vector<int64_t> numFramesCaught(numUDPInterfaces);
    int index = 0;
    for (const auto &it : listener) {
        if (it->GetStartedFlag()) {
            numFramesCaught[index] = it->GetNumCompleteFramesCaught();
        }
        ++index;
    }
    return numFramesCaught;
}

std::vector<int64_t> Implementation::getCurrentFrameIndex() const {
    std::vector<int64_t> frameIndex(numUDPInterfaces);
    int index = 0;
    for (const auto &it : listener) {
        if (it->GetStartedFlag()) {
            frameIndex[index] = it->GetCurrentFrameIndex();
        }
        ++index;
    }
    return frameIndex;
}

double Implementation::getProgress() const {
    if (!activated || (!detectorDataStream[0] && !detectorDataStream[1])) {
        return 100.00;
    }

    // if disabled, considering only 1 port
    double totalFrames = (double)(numberOfTotalFrames * listener.size());
    if (!detectorDataStream[0] || !detectorDataStream[1]) {
        totalFrames /= 2;
    }

    double progress = 0;
    int index = 0;
    for (const auto &it : listener) {
        if (detectorDataStream[index] && it->GetStartedFlag()) {
            progress += (it->GetListenedIndex() + 1) / totalFrames;
        }
        ++index;
    }
    progress *= 100;
    return progress;
}

std::vector<int64_t> Implementation::getNumMissingPackets() const {
    std::vector<int64_t> mp(numUDPInterfaces);
    for (int i = 0; i < numUDPInterfaces; ++i) {
        int np = generalData->packetsPerFrame;
        uint64_t totnp = np;
        // ReadNRows
        if (readNRows != (int)generalData->maxRowsPerReadout) {
            totnp = ((readNRows * np) / generalData->maxRowsPerReadout);
        }
        totnp *= numberOfTotalFrames;
        mp[i] = listener[i]->GetNumMissingPacket(stoppedFlag, totnp);
    }
    return mp;
}

void Implementation::setScan(slsDetectorDefs::scanParameters s) {
    scanParams = s;
    LOG(logINFO) << "Scan parameters: " << sls::ToString(scanParams);
}

void Implementation::startReceiver() {
    LOG(logINFO) << "Starting Receiver";
    stoppedFlag = false;
    ResetParametersforNewAcquisition();

    // listener
    CreateUDPSockets();

    // callbacks
    if (startAcquisitionCallBack) {
        try {
            std::size_t imageSize =
                static_cast<uint32_t>(generalData->imageSize);
            startAcquisitionCallBack(filePath, fileName, fileIndex, imageSize,
                                     pStartAcquisition);
        } catch (const std::exception &e) {
            throw sls::RuntimeError("Start Acquisition Callback Error: " +
                                    std::string(e.what()));
        }
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

    if (fileWriteEnable && modulePos == 0) {
        // master and virtual file (hdf5)
        StartMasterWriter();
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
        auto mp = getNumMissingPackets();
        // print summary
        uint64_t tot = 0;
        for (int i = 0; i < numUDPInterfaces; i++) {
            int nf = listener[i]->GetNumCompleteFramesCaught();
            tot += nf;
            std::string mpMessage = std::to_string(mp[i]);
            if (mp[i] < 0) {
                mpMessage =
                    std::to_string(std::abs(mp[i])) + std::string(" (Extra)");
            }

            std::string summary;
            if (!activated) {
                summary = "\n\tDeactivated Receiver";
            } else if (!detectorDataStream[i]) {
                summary = (i == 0 ? "\n\tDeactivated Left Port"
                                  : "\n\tDeactivated Right Port");
            } else {
                std::ostringstream os;
                os << "\n\tMissing Packets\t\t: " << mpMessage
                   << "\n\tComplete Frames\t\t: " << nf
                   << "\n\tLast Frame Caught\t: "
                   << listener[i]->GetLastFrameIndexCaught();
                summary = os.str();
            }

            TLogLevel lev = ((mp[i]) > 0) ? logINFORED : logINFOGREEN;
            LOG(lev) << "Summary of Port " << udpPortNum[i] << summary;
        }

        // callback
        if (acquisitionFinishedCallBack) {
            try {
                acquisitionFinishedCallBack((tot / numUDPInterfaces),
                                            pAcquisitionFinished);
            } catch (const std::exception &e) {
                // change status
                status = IDLE;
                LOG(logINFO) << "Receiver Stopped";
                LOG(logINFO) << "Status: " << sls::ToString(status);
                throw sls::RuntimeError(
                    "Acquisition Finished Callback Error: " +
                    std::string(e.what()));
            }
        }
    }

    // change status
    status = IDLE;
    LOG(logINFO) << "Receiver Stopped";
    LOG(logINFO) << "Status: " << sls::ToString(status);
}

void Implementation::startReadout() {
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
    for (const auto &it : listener)
        it->ShutDownUDPSocket();
}

void Implementation::restreamStop() {
    for (const auto &it : dataStreamer)
        it->RestreamStop();
    LOG(logINFO) << "Restreaming Dummy Header via ZMQ successful";
}

void Implementation::ResetParametersforNewAcquisition() {
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
    try {
        for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
            dataProcessor[i]->CreateFirstFiles(
                filePath, fileName, fileIndex, overwriteEnable, silentMode,
                modulePos, numUDPInterfaces, udpPortNum[i], framesPerFile,
                numberOfTotalFrames, dynamicRange, detectorDataStream[i]);
        }
    } catch (const sls::RuntimeError &e) {
        shutDownUDPSockets();
        for (const auto &it : dataProcessor)
            it->CloseFiles();
        throw sls::RuntimeError("Could not create first data file.");
    }
}

void Implementation::StartMasterWriter() {
    try {
        std::string masterFileName;
        // master file
        if (masterFileWriteEnable) {
            MasterAttributes masterAttributes;
            masterAttributes.detType = detType;
            masterAttributes.timingMode = timingMode;
            masterAttributes.geometry = numPorts;
            masterAttributes.imageSize = generalData->imageSize;
            masterAttributes.nPixels =
                xy(generalData->nPixelsX, generalData->nPixelsY);
            masterAttributes.maxFramesPerFile = framesPerFile;
            masterAttributes.frameDiscardMode = frameDiscardMode;
            masterAttributes.framePadding = framePadding;
            masterAttributes.scanParams = scanParams;
            masterAttributes.totalFrames = numberOfTotalFrames;
            masterAttributes.exptime = acquisitionTime;
            masterAttributes.period = acquisitionPeriod;
            masterAttributes.burstMode = burstMode;
            masterAttributes.numUDPInterfaces = numUDPInterfaces;
            masterAttributes.dynamicRange = dynamicRange;
            masterAttributes.tenGiga = tengigaEnable;
            masterAttributes.thresholdEnergyeV = thresholdEnergyeV;
            masterAttributes.thresholdAllEnergyeV = thresholdAllEnergyeV;
            masterAttributes.subExptime = subExpTime;
            masterAttributes.subPeriod = subPeriod;
            masterAttributes.quad = quadEnable;
            masterAttributes.readNRows = readNRows;
            masterAttributes.ratecorr = rateCorrections;
            masterAttributes.adcmask =
                tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga;
            masterAttributes.analog = (readoutType == ANALOG_ONLY ||
                                       readoutType == ANALOG_AND_DIGITAL)
                                          ? 1
                                          : 0;
            masterAttributes.analogSamples = numberOfAnalogSamples;
            masterAttributes.digital = (readoutType == DIGITAL_ONLY ||
                                        readoutType == ANALOG_AND_DIGITAL)
                                           ? 1
                                           : 0;
            masterAttributes.digitalSamples = numberOfDigitalSamples;
            masterAttributes.dbitoffset = ctbDbitOffset;
            masterAttributes.dbitlist = 0;
            for (auto &i : ctbDbitList) {
                masterAttributes.dbitlist |= (1 << i);
            }
            masterAttributes.roi = roi;
            masterAttributes.counterMask = counterMask;
            masterAttributes.exptimeArray[0] = acquisitionTime1;
            masterAttributes.exptimeArray[1] = acquisitionTime2;
            masterAttributes.exptimeArray[2] = acquisitionTime3;
            masterAttributes.gateDelayArray[0] = gateDelay1;
            masterAttributes.gateDelayArray[1] = gateDelay2;
            masterAttributes.gateDelayArray[2] = gateDelay3;
            masterAttributes.gates = numberOfGates;
            masterAttributes.additionalJsonHeader = additionalJsonHeader;

            // create master file
            masterFileName = dataProcessor[0]->CreateMasterFile(
                filePath, fileName, fileIndex, overwriteEnable, silentMode,
                fileFormatType, &masterAttributes, &hdf5LibMutex);
        }
#ifdef HDF5C
        if (fileFormatType == HDF5) {
            std::array<std::string, 2> virtualFileAndDatasetNames;
            // create virtual hdf5 file (if multiple files)
            if (dataProcessor[0]->GetFilesInAcquisition() > 1 ||
                (numPorts.x * numPorts.y) > 1) {
                virtualFileAndDatasetNames =
                    dataProcessor[0]->CreateVirtualFile(
                        filePath, fileName, fileIndex, overwriteEnable,
                        silentMode, modulePos, numUDPInterfaces, framesPerFile,
                        numberOfTotalFrames, numPorts.x, numPorts.y,
                        dynamicRange, &hdf5LibMutex);
            }
            // link file in master
            if (masterFileWriteEnable) {
                dataProcessor[0]->LinkFileInMaster(
                    masterFileName, virtualFileAndDatasetNames[0],
                    virtualFileAndDatasetNames[1], silentMode, &hdf5LibMutex);
            }
        }
#endif
    } catch (...) {
        ; // ignore it and just print it
    }
}

void Implementation::StartRunning() {

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
    return numUDPInterfaces;
}

// not Eiger
void Implementation::setNumberofUDPInterfaces(const int n) {
    if (detType == EIGER) {
        throw sls::RuntimeError(
            "Cannot set number of UDP interfaces for Eiger");
    }

    if (numUDPInterfaces != n) {
        // clear all threads and fifos
        listener.clear();
        dataProcessor.clear();
        dataStreamer.clear();
        fifo.clear();

        // set local variables
        generalData->SetNumberofInterfaces(n);
        numUDPInterfaces = n;

        // fifo
        udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;
        SetupFifoStructure();

        // create threads
        for (int i = 0; i < numUDPInterfaces; ++i) {
            // listener and dataprocessor threads
            try {
                auto fifo_ptr = fifo[i].get();
                listener.push_back(sls::make_unique<Listener>(
                    i, detType, fifo_ptr, &status, &udpPortNum[i], &eth[i],
                    &udpSocketBufferSize, &actualUDPSocketBufferSize,
                    &framesPerFile, &frameDiscardMode, &activated,
                    &detectorDataStream[i], &silentMode));
                listener[i]->SetGeneralData(generalData);

                int ctbAnalogDataBytes = 0;
                if (detType == CHIPTESTBOARD) {
                    ctbAnalogDataBytes =
                        generalData->GetNumberOfAnalogDatabytes();
                }
                dataProcessor.push_back(sls::make_unique<DataProcessor>(
                    i, detType, fifo_ptr, &activated, &dataStreamEnable,
                    &streamingFrequency, &streamingTimerInMs,
                    &streamingStartFnum, &framePadding, &ctbDbitList,
                    &ctbDbitOffset, &ctbAnalogDataBytes));
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
                    bool flip = flipRows;
                    if (quadEnable) {
                        flip = (i == 1 ? true : false);
                    }
                    dataStreamer.push_back(sls::make_unique<DataStreamer>(
                        i, fifo[i].get(), &dynamicRange, &roi, &fileIndex, flip,
                        numPorts, &quadEnable, &numberOfTotalFrames));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numUDPInterfaces, streamingPort, streamingSrcIP,
                        streamingHwm);
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
        setDetectorSize(numModules);
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

std::string Implementation::getEthernetInterface() const { return eth[0]; }

void Implementation::setEthernetInterface(const std::string &c) {
    eth[0] = c;
    LOG(logINFO) << "Ethernet Interface: " << eth[0];
}

std::string Implementation::getEthernetInterface2() const { return eth[1]; }

void Implementation::setEthernetInterface2(const std::string &c) {
    eth[1] = c;
    LOG(logINFO) << "Ethernet Interface 2: " << eth[1];
}

uint32_t Implementation::getUDPPortNumber() const { return udpPortNum[0]; }

void Implementation::setUDPPortNumber(const uint32_t i) {
    udpPortNum[0] = i;
    LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum[0];
}

uint32_t Implementation::getUDPPortNumber2() const { return udpPortNum[1]; }

void Implementation::setUDPPortNumber2(const uint32_t i) {
    udpPortNum[1] = i;
    LOG(logINFO) << "UDP Port Number[1]: " << udpPortNum[1];
}

int Implementation::getUDPSocketBufferSize() const {
    return udpSocketBufferSize;
}

void Implementation::setUDPSocketBufferSize(const int s) {
    // custom setup is not 0 (must complain if set up didnt work)
    // testing default setup at startup, argument is 0 to use default values
    int size = (s == 0) ? udpSocketBufferSize : s;
    size_t listSize = listener.size();
    if ((detType == JUNGFRAU || detType == GOTTHARD2) &&
        (int)listSize != numUDPInterfaces) {
        throw sls::RuntimeError(
            "Number of Interfaces " + std::to_string(numUDPInterfaces) +
            " do not match listener size " + std::to_string(listSize));
    }

    for (auto &l : listener) {
        l->CreateDummySocketForUDPSocketBufferSize(size);
    }
    // custom and didnt set, throw error
    if (s != 0 && udpSocketBufferSize != s) {
        throw sls::RuntimeError("Could not set udp socket buffer size. (No "
                                "CAP_NET_ADMIN privileges?)");
    }
}

int Implementation::getActualUDPSocketBufferSize() const {
    return actualUDPSocketBufferSize;
}

/**************************************************
 *                                                 *
 *   ZMQ Streaming Parameters (ZMQ)                *
 *                                                 *
 * ************************************************/
bool Implementation::getDataStreamEnable() const { return dataStreamEnable; }

void Implementation::setDataStreamEnable(const bool enable) {
    if (dataStreamEnable != enable) {
        dataStreamEnable = enable;

        // data sockets have to be created again as the client ones are
        dataStreamer.clear();

        if (enable) {
            for (int i = 0; i < numUDPInterfaces; ++i) {
                try {
                    bool flip = flipRows;
                    if (quadEnable) {
                        flip = (i == 1 ? true : false);
                    }
                    dataStreamer.push_back(sls::make_unique<DataStreamer>(
                        i, fifo[i].get(), &dynamicRange, &roi, &fileIndex, flip,
                        numPorts, &quadEnable, &numberOfTotalFrames));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numUDPInterfaces, streamingPort, streamingSrcIP,
                        streamingHwm);
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
    return streamingFrequency;
}

void Implementation::setStreamingFrequency(const uint32_t freq) {
    streamingFrequency = freq;
    LOG(logINFO) << "Streaming Frequency: " << streamingFrequency;
}

uint32_t Implementation::getStreamingTimer() const {
    return streamingTimerInMs;
}

void Implementation::setStreamingTimer(const uint32_t time_in_ms) {
    streamingTimerInMs = time_in_ms;
    LOG(logINFO) << "Streamer Timer: " << streamingTimerInMs;
}

uint32_t Implementation::getStreamingStartingFrameNumber() const {
    return streamingStartFnum;
}

void Implementation::setStreamingStartingFrameNumber(const uint32_t fnum) {
    streamingStartFnum = fnum;
    LOG(logINFO) << "Streaming Start Frame num: " << streamingStartFnum;
}

uint32_t Implementation::getStreamingPort() const { return streamingPort; }

void Implementation::setStreamingPort(const uint32_t i) {
    streamingPort = i;
    LOG(logINFO) << "Streaming Port: " << streamingPort;
}

sls::IpAddr Implementation::getStreamingSourceIP() const {
    return streamingSrcIP;
}

void Implementation::setStreamingSourceIP(const sls::IpAddr ip) {
    streamingSrcIP = ip;
    LOG(logINFO) << "Streaming Source IP: " << streamingSrcIP;
}

int Implementation::getStreamingHwm() const { return streamingHwm; }

void Implementation::setStreamingHwm(const int i) {
    streamingHwm = i;
    LOG(logINFO) << "Streaming Hwm: "
                 << (i == -1 ? "Default (-1)" : std::to_string(streamingHwm));
}

std::map<std::string, std::string>
Implementation::getAdditionalJsonHeader() const {
    return additionalJsonHeader;
}

void Implementation::setAdditionalJsonHeader(
    const std::map<std::string, std::string> &c) {

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
    int64_t numFrames = numberOfFrames;
    // gotthard2
    if (detType == GOTTHARD2) {
        // auto
        if (timingMode == AUTO_TIMING) {
            // burst mode, repeats = #bursts
            if (burstMode == BURST_INTERNAL || burstMode == BURST_EXTERNAL) {
                repeats = numberOfBursts;
            }
            // continuous, repeats = 1 (no trigger as well)
            else {
                repeats = 1;
            }
        }
        // trigger
        else {
            // continuous, numFrames is limited
            if (burstMode == CONTINUOUS_INTERNAL ||
                burstMode == CONTINUOUS_EXTERNAL) {
                numFrames = 1;
            }
        }
    }
    numberOfTotalFrames =
        numFrames * repeats * (int64_t)(numberOfAdditionalStorageCells + 1);
    if (numberOfTotalFrames == 0) {
        throw sls::RuntimeError("Invalid total number of frames to receive: 0");
    }
    LOG(logINFO) << "Total Number of Frames: " << numberOfTotalFrames;
}

uint64_t Implementation::getNumberOfFrames() const { return numberOfFrames; }

void Implementation::setNumberOfFrames(const uint64_t i) {
    numberOfFrames = i;
    LOG(logINFO) << "Number of Frames: " << numberOfFrames;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getNumberOfTriggers() const {
    return numberOfTriggers;
}

void Implementation::setNumberOfTriggers(const uint64_t i) {
    numberOfTriggers = i;
    LOG(logINFO) << "Number of Triggers: " << numberOfTriggers;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getNumberOfBursts() const { return numberOfBursts; }

void Implementation::setNumberOfBursts(const uint64_t i) {
    numberOfBursts = i;
    LOG(logINFO) << "Number of Bursts: " << numberOfBursts;
    updateTotalNumberOfFrames();
}

int Implementation::getNumberOfAdditionalStorageCells() const {
    return numberOfAdditionalStorageCells;
}

void Implementation::setNumberOfAdditionalStorageCells(const int i) {
    numberOfAdditionalStorageCells = i;
    LOG(logINFO) << "Number of Additional Storage Cells: "
                 << numberOfAdditionalStorageCells;
    updateTotalNumberOfFrames();
}

void Implementation::setNumberOfGates(const int i) {
    numberOfGates = i;
    LOG(logINFO) << "Number of Gates: " << numberOfGates;
}

slsDetectorDefs::timingMode Implementation::getTimingMode() const {
    return timingMode;
}

void Implementation::setTimingMode(const slsDetectorDefs::timingMode i) {
    timingMode = i;
    LOG(logINFO) << "Timing Mode: " << timingMode;
    updateTotalNumberOfFrames();
}

slsDetectorDefs::burstMode Implementation::getBurstMode() const {
    return burstMode;
}

void Implementation::setBurstMode(const slsDetectorDefs::burstMode i) {
    burstMode = i;
    LOG(logINFO) << "Burst Mode: " << burstMode;
    updateTotalNumberOfFrames();
}

ns Implementation::getAcquisitionPeriod() const { return acquisitionPeriod; }

void Implementation::setAcquisitionPeriod(const ns i) {
    acquisitionPeriod = i;
    LOG(logINFO) << "Acquisition Period: " << sls::ToString(acquisitionPeriod);
}

ns Implementation::getAcquisitionTime() const { return acquisitionTime; }

void Implementation::updateAcquisitionTime() {
    if (acquisitionTime1 == acquisitionTime2 &&
        acquisitionTime2 == acquisitionTime3) {
        acquisitionTime = acquisitionTime1;
    } else {
        acquisitionTime = std::chrono::nanoseconds(0);
    }
}

void Implementation::setAcquisitionTime(const ns i) {
    acquisitionTime = i;
    LOG(logINFO) << "Acquisition Time: " << sls::ToString(acquisitionTime);
}

void Implementation::setAcquisitionTime1(const ns i) {
    acquisitionTime1 = i;
    LOG(logINFO) << "Acquisition Time1: " << sls::ToString(acquisitionTime1);
    updateAcquisitionTime();
}

void Implementation::setAcquisitionTime2(const ns i) {
    acquisitionTime2 = i;
    LOG(logINFO) << "Acquisition Time2: " << sls::ToString(acquisitionTime2);
    updateAcquisitionTime();
}

void Implementation::setAcquisitionTime3(const ns i) {
    acquisitionTime3 = i;
    LOG(logINFO) << "Acquisition Time3: " << sls::ToString(acquisitionTime3);
    updateAcquisitionTime();
}

void Implementation::setGateDelay1(const ns i) {
    gateDelay1 = i;
    LOG(logINFO) << "Gate Delay1: " << sls::ToString(gateDelay1);
}

void Implementation::setGateDelay2(const ns i) {
    gateDelay2 = i;
    LOG(logINFO) << "Gate Delay2: " << sls::ToString(gateDelay2);
}

void Implementation::setGateDelay3(const ns i) {
    gateDelay3 = i;
    LOG(logINFO) << "Gate Delay3: " << sls::ToString(gateDelay3);
}

ns Implementation::getSubExpTime() const { return subExpTime; }

void Implementation::setSubExpTime(const ns i) {
    subExpTime = i;
    LOG(logINFO) << "Sub Exposure Time: " << sls::ToString(subExpTime);
}

ns Implementation::getSubPeriod() const { return subPeriod; }

void Implementation::setSubPeriod(const ns i) {
    subPeriod = i;
    LOG(logINFO) << "Sub Period: " << sls::ToString(subPeriod);
}

uint32_t Implementation::getNumberofAnalogSamples() const {
    return numberOfAnalogSamples;
}

void Implementation::setNumberofAnalogSamples(const uint32_t i) {
    if (numberOfAnalogSamples != i) {
        numberOfAnalogSamples = i;

        generalData->SetNumberOfAnalogSamples(i);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Analog Samples: " << numberOfAnalogSamples;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getNumberofDigitalSamples() const {
    return numberOfDigitalSamples;
}

void Implementation::setNumberofDigitalSamples(const uint32_t i) {
    if (numberOfDigitalSamples != i) {
        numberOfDigitalSamples = i;

        generalData->SetNumberOfDigitalSamples(i);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Digital Samples: " << numberOfDigitalSamples;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getCounterMask() const { return counterMask; }

void Implementation::setCounterMask(const uint32_t i) {
    if (counterMask != i) {
        generalData->SetCounterMask(i);
        counterMask = i;
        SetupFifoStructure();
    }
    LOG(logINFO) << "Counter mask: " << sls::ToStringHex(counterMask);
    int ncounters = __builtin_popcount(counterMask);
    LOG(logINFO) << "Number of counters: " << ncounters;
}

uint32_t Implementation::getDynamicRange() const { return dynamicRange; }

void Implementation::setDynamicRange(const uint32_t i) {
    if (dynamicRange != i) {
        dynamicRange = i;

        if (detType == EIGER || detType == MYTHEN3) {
            generalData->SetDynamicRange(i);
            fifoDepth = generalData->defaultFifoDepth;
            SetupFifoStructure();
        }
    }
    LOG(logINFO) << "Dynamic Range: " << dynamicRange;
}

slsDetectorDefs::ROI Implementation::getROI() const { return roi; }

void Implementation::setROI(slsDetectorDefs::ROI arg) {
    if (roi.xmin != arg.xmin || roi.xmax != arg.xmax) {
        roi.xmin = arg.xmin;
        roi.xmax = arg.xmax;

        // only for gotthard
        generalData->SetROI(arg);
        framesPerFile = generalData->maxFramesPerFile;
        SetupFifoStructure();
    }

    LOG(logINFO) << "ROI: [" << roi.xmin << ", " << roi.xmax << "]";
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

bool Implementation::getTenGigaEnable() const { return tengigaEnable; }

void Implementation::setTenGigaEnable(const bool b) {
    if (tengigaEnable != b) {
        tengigaEnable = b;

        generalData->SetTenGigaEnable(b);
        SetupFifoStructure();

        // datastream can be disabled/enabled only for Eiger 10GbE
        if (detType == EIGER) {
            if (!b) {
                detectorDataStream[LEFT] = 1;
                detectorDataStream[RIGHT] = 1;
            } else {
                detectorDataStream[LEFT] = detectorDataStream10GbE[LEFT];
                detectorDataStream[RIGHT] = detectorDataStream10GbE[RIGHT];
            }
            LOG(logDEBUG) << "Detector datastream updated [Left: "
                          << sls::ToString(detectorDataStream[LEFT])
                          << ", Right: "
                          << sls::ToString(detectorDataStream[RIGHT]) << "]";
        }
    }
    LOG(logINFO) << "Ten Giga: " << (tengigaEnable ? "enabled" : "disabled");
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

bool Implementation::getFlipRows() const { return flipRows; }

void Implementation::setFlipRows(bool enable) {
    flipRows = enable;

    if (!quadEnable) {
        for (const auto &it : dataStreamer) {
            it->SetFlipRows(flipRows);
        }
    }
    // quad
    else {
        if (dataStreamer.size() == 2) {
            dataStreamer[0]->SetFlipRows(false);
            dataStreamer[1]->SetFlipRows(true);
        }
    }
    LOG(logINFO) << "Flip Rows: " << flipRows;
}

bool Implementation::getQuad() const { return quadEnable; }

void Implementation::setQuad(const bool b) {
    if (quadEnable != b) {
        quadEnable = b;
        setDetectorSize(numModules);
        if (!quadEnable) {
            for (const auto &it : dataStreamer) {
                it->SetFlipRows(flipRows);
            }
        } else {
            if (dataStreamer.size() == 2) {
                dataStreamer[0]->SetFlipRows(false);
                dataStreamer[1]->SetFlipRows(true);
            }
        }
    }
    LOG(logINFO) << "Quad Enable: " << quadEnable;
}

bool Implementation::getActivate() const { return activated; }

void Implementation::setActivate(bool enable) {
    activated = enable;
    LOG(logINFO) << "Activation: " << (activated ? "enabled" : "disabled");
}

bool Implementation::getDetectorDataStream(const portPosition port) const {
    int index = (port == LEFT ? 0 : 1);
    return detectorDataStream[index];
}

void Implementation::setDetectorDataStream(const portPosition port,
                                           const bool enable) {
    int index = (port == LEFT ? 0 : 1);
    detectorDataStream10GbE[index] = enable;
    LOG(logINFO) << "Detector 10GbE datastream (" << sls::ToString(port)
                 << " Port): " << sls::ToString(detectorDataStream10GbE[index]);
    // update datastream for 10g
    if (tengigaEnable) {
        detectorDataStream[index] = detectorDataStream10GbE[index];
        LOG(logDEBUG) << "Detector datastream updated ["
                      << (index == 0 ? "Left" : "Right")
                      << "] : " << sls::ToString(detectorDataStream[index]);
    }
}

int Implementation::getReadNRows() const { return readNRows; }

void Implementation::setReadNRows(const int value) {
    readNRows = value;
    LOG(logINFO) << "Number of rows: " << readNRows;
}

void Implementation::setThresholdEnergy(const int value) {
    thresholdEnergyeV = value;
    LOG(logINFO) << "Threshold Energy: " << thresholdEnergyeV << " eV";
}

void Implementation::setThresholdEnergy(const std::array<int, 3> value) {
    thresholdAllEnergyeV = value;
    LOG(logINFO) << "Threshold Energy (eV): "
                 << sls::ToString(thresholdAllEnergyeV);
}

void Implementation::setRateCorrections(const std::vector<int64_t> &t) {
    rateCorrections = t;
    LOG(logINFO) << "Rate Corrections: " << sls::ToString(rateCorrections);
}

slsDetectorDefs::readoutMode Implementation::getReadoutMode() const {
    return readoutType;
}

void Implementation::setReadoutMode(const readoutMode f) {
    if (readoutType != f) {
        readoutType = f;

        generalData->SetReadoutMode(f);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Readout Mode: " << sls::ToString(f);
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getADCEnableMask() const {
    return adcEnableMaskOneGiga;
}

void Implementation::setADCEnableMask(uint32_t mask) {
    if (adcEnableMaskOneGiga != mask) {
        adcEnableMaskOneGiga = mask;

        generalData->SetOneGigaAdcEnableMask(mask);
        SetupFifoStructure();
    }
    LOG(logINFO) << "ADC Enable Mask for 1Gb mode: 0x" << std::hex
                 << adcEnableMaskOneGiga << std::dec;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getTenGigaADCEnableMask() const {
    return adcEnableMaskTenGiga;
}

void Implementation::setTenGigaADCEnableMask(uint32_t mask) {
    if (adcEnableMaskTenGiga != mask) {
        adcEnableMaskTenGiga = mask;

        generalData->SetTenGigaAdcEnableMask(mask);
        SetupFifoStructure();
    }
    LOG(logINFO) << "ADC Enable Mask for 10Gb mode: 0x" << std::hex
                 << adcEnableMaskTenGiga << std::dec;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

std::vector<int> Implementation::getDbitList() const { return ctbDbitList; }

void Implementation::setDbitList(const std::vector<int> &v) { ctbDbitList = v; }

int Implementation::getDbitOffset() const { return ctbDbitOffset; }

void Implementation::setDbitOffset(const int s) { ctbDbitOffset = s; }

/**************************************************
 *                                                *
 *    Callbacks                                   *
 *                                                *
 * ************************************************/
void Implementation::registerCallBackStartAcquisition(
    int (*func)(const std::string &, const std::string &, uint64_t, size_t,
                void *),
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
    void (*func)(sls_receiver_header *, char *, size_t, void *), void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
    for (const auto &it : dataProcessor)
        it->registerCallBackRawDataReady(rawDataReadyCallBack, pRawDataReady);
}

void Implementation::registerCallBackRawDataModifyReady(
    void (*func)(sls_receiver_header *, char *, size_t &, void *), void *arg) {
    rawDataModifyReadyCallBack = func;
    pRawDataReady = arg;
    for (const auto &it : dataProcessor)
        it->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,
                                               pRawDataReady);
}
