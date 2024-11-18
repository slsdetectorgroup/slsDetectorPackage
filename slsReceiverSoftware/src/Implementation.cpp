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

namespace sls {

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
    for (int i = 0; i < generalData->numUDPInterfaces; ++i) {
        size_t datasize = generalData->imageSize;
        // veto data size
        if (generalData->detType == GOTTHARD2 && i != 0) {
            datasize = generalData->vetoImageSize;
        }
        datasize += IMAGE_STRUCTURE_HEADER_SIZE;

        // create fifo structure
        try {
            fifo.push_back(
                sls::make_unique<Fifo>(i, datasize, generalData->fifoDepth));
        } catch (const std::exception &e) {
            fifo.clear();
            generalData->fifoDepth = 0;
            std::ostringstream oss;
            oss << e.what() << ". Fifo depth is now 0";
            throw RuntimeError(oss.str());
        }
        // set the listener & dataprocessor threads to point to the right fifo
        if (listener.size())
            listener[i]->SetFifo(fifo[i].get());
        if (dataProcessor.size())
            dataProcessor[i]->SetFifo(fifo[i].get());
        if (dataStreamer.size())
            dataStreamer[i]->SetFifo(fifo[i].get());

        LOG(logINFO) << "Memory Allocated for Fifo " << i << ": "
                     << (double)(datasize * (size_t)generalData->fifoDepth) /
                            (double)(1024 * 1024)
                     << " MB";
    }
    LOG(logINFO) << generalData->numUDPInterfaces
                 << " Fifo structure(s) reconstructed";
}

/**************************************************
 *                                                 *
 *   Configuration Parameters                      *
 *                                                 *
 * ************************************************/

void Implementation::setDetectorType(const detectorType d) {

    switch (d) {
    case GOTTHARD:
    case EIGER:
    case JUNGFRAU:
    case MOENCH:
    case CHIPTESTBOARD:
    case XILINX_CHIPTESTBOARD:
    case MYTHEN3:
    case GOTTHARD2:
        LOG(logINFO) << " ***** " << ToString(d) << " Receiver *****";
        break;
    default:
        throw RuntimeError("This is an unknown receiver type " +
                           std::to_string(static_cast<int>(d)));
    }

    delete generalData;
    generalData = nullptr;

    // set detector specific variables
    switch (d) {
    case GOTTHARD:
        generalData = new GotthardData();
        break;
    case EIGER:
        generalData = new EigerData();
        break;
    case JUNGFRAU:
        generalData = new JungfrauData();
        break;
    case MOENCH:
        generalData = new MoenchData();
        break;
    case CHIPTESTBOARD:
        generalData = new ChipTestBoardData();
        break;
    case XILINX_CHIPTESTBOARD:
        generalData = new XilinxChipTestBoardData();
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

    SetLocalNetworkParameters();
    SetupFifoStructure();

    // create threads
    for (int i = 0; i < generalData->numUDPInterfaces; ++i) {

        try {
            listener.push_back(sls::make_unique<Listener>(i, &status));
            SetupListener(i);
            dataProcessor.push_back(sls::make_unique<DataProcessor>(i));
            SetupDataProcessor(i);
        } catch (const std::exception &e) {
            listener.clear();
            dataProcessor.clear();
            throw;
        }
    }

    SetThreadPriorities();

    LOG(logDEBUG) << " Detector type set to " << ToString(d);
}

void Implementation::SetupListener(int i) {
    listener[i]->SetFifo(fifo[i].get());
    listener[i]->SetGeneralData(generalData);
    listener[i]->SetUdpPortNumber(udpPortNum[i]);
    listener[i]->SetEthernetInterface(eth[i]);
    listener[i]->SetActivate(activated);
    listener[i]->SetNoRoi(portRois[i].noRoi());
    listener[i]->SetDetectorDatastream(detectorDataStream[i]);
    listener[i]->SetSilentMode(silentMode);
}

void Implementation::SetupDataProcessor(int i) {
    dataProcessor[i]->SetFifo(fifo[i].get());
    dataProcessor[i]->SetGeneralData(generalData);
    dataProcessor[i]->SetUdpPortNumber(udpPortNum[i]);
    dataProcessor[i]->SetActivate(activated);
    dataProcessor[i]->SetReceiverROI(portRois[i]);
    dataProcessor[i]->SetDataStreamEnable(dataStreamEnable);
    dataProcessor[i]->SetStreamingFrequency(streamingFrequency);
    dataProcessor[i]->SetStreamingTimerInMs(streamingTimerInMs);
    dataProcessor[i]->SetStreamingStartFnum(streamingStartFnum);
    dataProcessor[i]->SetFramePadding(framePadding);
    dataProcessor[i]->SetCtbDbitList(ctbDbitList);
    dataProcessor[i]->SetCtbDbitOffset(ctbDbitOffset);
    dataProcessor[i]->SetQuadEnable(quadEnable);
    dataProcessor[i]->SetFlipRows(flipRows);
    dataProcessor[i]->SetNumberofTotalFrames(numberOfTotalFrames);
    dataProcessor[i]->SetAdditionalJsonHeader(additionalJsonHeader);
}

void Implementation::SetupDataStreamer(int i) {
    dataStreamer[i]->SetFifo(fifo[i].get());
    dataStreamer[i]->SetGeneralData(generalData);
    dataStreamer[i]->CreateZmqSockets(streamingPort, streamingHwm);
    dataStreamer[i]->SetAdditionalJsonHeader(additionalJsonHeader);
    dataStreamer[i]->SetFileIndex(fileIndex);
    dataStreamer[i]->SetQuadEnable(quadEnable);
    dataStreamer[i]->SetFlipRows(flipRows);
    dataStreamer[i]->SetNumberofPorts(numPorts);
    dataStreamer[i]->SetNumberofTotalFrames(numberOfTotalFrames);
    dataStreamer[i]->SetReceiverROI(
        portRois[i].completeRoi() ? GetMaxROIPerPort() : portRois[i]);
}

slsDetectorDefs::xy Implementation::getDetectorSize() const {
    return numModules;
}

const slsDetectorDefs::xy Implementation::GetPortGeometry() const {
    xy portGeometry{1, 1};
    if (generalData->detType == EIGER)
        portGeometry.x = generalData->numUDPInterfaces;
    else if (generalData->detType == JUNGFRAU || generalData->detType == MOENCH)
        portGeometry.y = generalData->numUDPInterfaces;
    return portGeometry;
}

const slsDetectorDefs::ROI Implementation::GetMaxROIPerPort() const {
    return slsDetectorDefs::ROI{0, (int)generalData->nPixelsX - 1, 0,
                                (int)generalData->nPixelsY - 1};
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

    LOG(logINFO) << "Detector Size (ports): " << ToString(numPorts);
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

void Implementation::setRow(const int value) {
    for (unsigned int i = 0; i < listener.size(); ++i) {
        int col = listener[i]->GetHardCodedPosition().second;
        listener[i]->SetHardCodedPosition(value, col);
    }
}

void Implementation::setColumn(const int value) {
    for (unsigned int i = 0; i < listener.size(); ++i) {
        int row = listener[i]->GetHardCodedPosition().first;
        listener[i]->SetHardCodedPosition(row, value);
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
    for (const auto &it : listener)
        it->SetSilentMode(silentMode);
    LOG(logINFO) << "Silent Mode: " << i;
}

uint32_t Implementation::getFifoDepth() const { return generalData->fifoDepth; }

void Implementation::setFifoDepth(const uint32_t i) {
    if (generalData->fifoDepth != i) {
        generalData->fifoDepth = i;
        SetupFifoStructure();
    }
    LOG(logINFO) << "Fifo Depth: " << i;
}

slsDetectorDefs::frameDiscardPolicy
Implementation::getFrameDiscardPolicy() const {
    return generalData->frameDiscardMode;
}

void Implementation::setFrameDiscardPolicy(const frameDiscardPolicy i) {
    generalData->frameDiscardMode = i;
    LOG(logINFO) << "Frame Discard Policy: "
                 << ToString(generalData->frameDiscardMode);
}

bool Implementation::getFramePaddingEnable() const { return framePadding; }

void Implementation::setFramePaddingEnable(const bool i) {
    framePadding = i;
    for (const auto &it : dataProcessor)
        it->SetFramePadding(framePadding);
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
    if (generalData->numUDPInterfaces == 2) {
        retval[id++] = listener[1]->GetThreadId();
        retval[id++] = dataProcessor[1]->GetThreadId();
        if (dataStreamEnable) {
            retval[id++] = dataStreamer[1]->GetThreadId();
        } else {
            retval[id++] = 0;
        }
    }
    retval[NUM_RX_THREAD_IDS - 1] = arping.GetProcessId();
    return retval;
}

bool Implementation::getArping() const { return arping.IsRunning(); }

pid_t Implementation::getArpingProcessId() const {
    return arping.GetProcessId();
}

void Implementation::setArping(const bool i,
                               const std::vector<std::string> ips) {
    if (i != arping.IsRunning()) {
        if (!i) {
            arping.StopProcess();
        } else {
            // setup interface
            for (int i = 0; i != generalData->numUDPInterfaces; ++i) {
                // ignore eiger with 2 interfaces (only udp port)
                if (i == 1 && (generalData->numUDPInterfaces == 1 ||
                               generalData->detType == EIGER)) {
                    break;
                }
                arping.SetInterfacesAndIps(i, eth[i], ips[i]);
            }
            arping.StartProcess();
        }
    }
}

slsDetectorDefs::ROI Implementation::getReceiverROI() const {
    return receiverRoi;
}

void Implementation::setReceiverROI(const slsDetectorDefs::ROI arg) {
    receiverRoi = arg;

    if (generalData->numUDPInterfaces == 1 ||
        generalData->detType == slsDetectorDefs::GOTTHARD2) {
        portRois[0] = arg;
    } else {
        slsDetectorDefs::xy nPortDim(generalData->nPixelsX,
                                     generalData->nPixelsY);

        for (int iPort = 0; iPort != generalData->numUDPInterfaces; ++iPort) {
            // default init = complete roi
            slsDetectorDefs::ROI portRoi{};

            // no roi
            if (arg.noRoi()) {
                portRoi.setNoRoi();
            }

            // incomplete roi
            else if (!arg.completeRoi()) {
                // get port limits
                slsDetectorDefs::ROI portFullRoi{0, nPortDim.x - 1, 0,
                                                 nPortDim.y - 1};
                if (iPort == 1) {
                    // left right (eiger)
                    if (GetPortGeometry().x == 2) {
                        portFullRoi.xmin += nPortDim.x;
                        portFullRoi.xmax += nPortDim.x;
                    }
                    // top bottom (jungfrau or moench)
                    else {
                        portFullRoi.ymin += nPortDim.y;
                        portFullRoi.ymax += nPortDim.y;
                    }
                }
                LOG(logDEBUG)
                    << iPort << ": portfullroi:" << ToString(portFullRoi);

                // no roi
                if (arg.xmin > portFullRoi.xmax ||
                    arg.xmax < portFullRoi.xmin ||
                    arg.ymin > portFullRoi.ymax ||
                    arg.ymax < portFullRoi.ymin) {
                    portRoi.setNoRoi();
                }

                // incomplete module roi
                else if (arg.xmin > portFullRoi.xmin ||
                         arg.xmax < portFullRoi.xmax ||
                         arg.ymin > portFullRoi.ymin ||
                         arg.ymax < portFullRoi.ymax) {
                    portRoi.xmin = (arg.xmin <= portFullRoi.xmin)
                                       ? 0
                                       : (arg.xmin % nPortDim.x);
                    portRoi.xmax = (arg.xmax >= portFullRoi.xmax)
                                       ? nPortDim.x - 1
                                       : (arg.xmax % nPortDim.x);
                    portRoi.ymin = (arg.ymin <= portFullRoi.ymin)
                                       ? 0
                                       : (arg.ymin % nPortDim.y);
                    portRoi.ymax = (arg.ymax >= portFullRoi.ymax)
                                       ? nPortDim.y - 1
                                       : (arg.ymax % nPortDim.y);
                }
            }
            portRois[iPort] = portRoi;
        }
    }
    for (size_t i = 0; i != listener.size(); ++i)
        listener[i]->SetNoRoi(portRois[i].noRoi());
    for (size_t i = 0; i != dataProcessor.size(); ++i)
        dataProcessor[i]->SetReceiverROI(portRois[i]);
    for (size_t i = 0; i != dataStreamer.size(); ++i) {
        dataStreamer[i]->SetReceiverROI(
            portRois[i].completeRoi() ? GetMaxROIPerPort() : portRois[i]);
    }
    LOG(logINFO) << "receiver roi: " << ToString(receiverRoi);
    if (generalData->numUDPInterfaces == 2 &&
        generalData->detType != slsDetectorDefs::GOTTHARD2) {
        LOG(logINFO) << "port rois: " << ToString(portRois);
    }
}

void Implementation::setReceiverROIMetadata(const ROI arg) {
    receiverRoiMetadata = arg;
    LOG(logINFO) << "receiver roi Metadata: " << ToString(receiverRoiMetadata);
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
            throw RuntimeError("Unknown file format");
        }
        for (const auto &it : dataProcessor)
            it->SetupFileWriter(fileWriteEnable, fileFormatType, &hdf5LibMutex);
    }

    LOG(logINFO) << "File Format: " << ToString(fileFormatType);
}

std::string Implementation::getFilePath() const { return filePath; }

void Implementation::setFilePath(const std::string &c) {
    filePath = c;
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
    for (const auto &it : dataStreamer)
        it->SetFileIndex(fileIndex);
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

uint32_t Implementation::getFramesPerFile() const {
    return generalData->framesPerFile;
}

void Implementation::setFramesPerFile(const uint32_t i) {
    generalData->framesPerFile = i;
    LOG(logINFO) << "Frames per file: " << generalData->framesPerFile;
}

/**************************************************
 *                                                 *
 *   Acquisition                                   *
 *                                                 *
 * ************************************************/
slsDetectorDefs::runStatus Implementation::getStatus() const { return status; }

std::vector<int64_t> Implementation::getFramesCaught() const {
    std::vector<int64_t> numFramesCaught(generalData->numUDPInterfaces);
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
    std::vector<int64_t> frameIndex(generalData->numUDPInterfaces);
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
    std::vector<bool> disabledPort;
    for (auto &it : listener) {
        disabledPort.push_back(it->isPortDisabled());
    }

    // all ports disabled
    if (allEqualTo<bool>(disabledPort, true)) {
        return 100.00;
    }

    // any disabled
    double totalFrames = (double)(numberOfTotalFrames * listener.size());
    if (anyEqualTo<bool>(disabledPort, true)) {
        for (auto it : disabledPort) {
            if (it) {
                totalFrames /= 2;
            }
        }
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
    std::vector<int64_t> mp(generalData->numUDPInterfaces);
    for (int i = 0; i < generalData->numUDPInterfaces; ++i) {
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
    LOG(logINFO) << "Scan parameters: " << ToString(scanParams);
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
            std::vector<uint32_t> udpPort;
            for (size_t i = 0; i != listener.size(); ++i) {
                udpPort.push_back(udpPortNum[i]);
            }
            startCallbackHeader callbackHeader = {
                udpPort,
                generalData->dynamicRange,
                numPorts,
                static_cast<size_t>(generalData->imageSize),
                filePath,
                fileName,
                fileIndex,
                quadEnable,
                additionalJsonHeader};
            startAcquisitionCallBack(callbackHeader, pStartAcquisition);
        } catch (const std::exception &e) {
            std::ostringstream oss;
            oss << "Start Acquisition Callback Error: " << e.what();
            throw RuntimeError(oss.str());
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
    LOG(logINFO) << "Status: " << ToString(status);
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

    // delete the udp sockets
    for (const auto &it : listener)
        it->DeleteUDPSocket();

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
    LOG(logINFO) << "Status: " << ToString(status);

    { // statistics
        auto mp = getNumMissingPackets();
        // print summary
        uint64_t tot = 0;
        for (int i = 0; i < generalData->numUDPInterfaces; i++) {
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
            } else if (portRois[i].noRoi()) {
                summary = (i == 0 ? "\n\tNo Roi on Left Port"
                                  : "\n\tNo Roi on Right Port");
            } else {
                std::ostringstream os;
                os << "\n\tMissing Packets\t\t: " << mpMessage
                   << "\n\tComplete Frames\t\t: " << nf
                   << "\n\tLast Frame Caught\t: "
                   << listener[i]->GetLastFrameIndexCaught();
                summary = os.str();
            }

            TLogLevel lev = ((mp[i]) > 0) ? logINFORED : logINFOGREEN;
            LOG(lev) << "Summary of Port " << udpPortNum[i] << " (" << eth[i]
                     << ')' << summary;
        }

        // callback
        if (acquisitionFinishedCallBack) {
            try {
                std::vector<uint32_t> udpPort;
                std::vector<uint64_t> completeFramesCaught;
                std::vector<uint64_t> lastFrameIndexCaught;
                for (size_t i = 0; i != listener.size(); ++i) {
                    udpPort.push_back(udpPortNum[i]);
                    completeFramesCaught.push_back(
                        listener[i]->GetNumCompleteFramesCaught());
                    lastFrameIndexCaught.push_back(
                        listener[i]->GetLastFrameIndexCaught());
                }
                endCallbackHeader callHeader = {udpPort, completeFramesCaught,
                                                lastFrameIndexCaught};
                acquisitionFinishedCallBack(callHeader, pAcquisitionFinished);
            } catch (const std::exception &e) {
                // change status
                status = IDLE;
                LOG(logINFO) << "Receiver Stopped";
                LOG(logINFO) << "Status: " << ToString(status);
                std::ostringstream oss;
                oss << "Acquisition Finished Callback Error: " << e.what();
                throw RuntimeError(oss.str());
            }
        }
    }

    // change status
    status = IDLE;
    LOG(logINFO) << "Receiver Stopped";
    LOG(logINFO) << "Status: " << ToString(status);
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
            listener[i]->CreateUDPSocket(actualUDPSocketBufferSize);
        }
    } catch (const RuntimeError &e) {
        shutDownUDPSockets();
        throw;
    }
    LOG(logDEBUG) << "UDP socket(s) created successfully.";
}

void Implementation::SetupWriter() {
    try {
        // check if folder exists and throw if it cant create
        mkdir_p(filePath);
        // create first files
        for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
            std::ostringstream os;
            os << filePath << "/" << fileName << "_d"
               << (modulePos * generalData->numUDPInterfaces + i);
            std::string fileNamePrefix = os.str();
            dataProcessor[i]->CreateFirstFiles(fileNamePrefix, fileIndex,
                                               overwriteEnable, silentMode,
                                               detectorDataStream[i]);
        }
    } catch (const RuntimeError &e) {
        shutDownUDPSockets();
        for (const auto &it : dataProcessor)
            it->CloseFiles();
        std::ostringstream oss;
        oss << "Could not set up writer: " << e.what();
        throw RuntimeError(oss.str());
    }
}

void Implementation::StartMasterWriter() {
    try {
        std::string masterFileName;
        // master file
        if (masterFileWriteEnable) {
            MasterAttributes masterAttributes;
            masterAttributes.detType = generalData->detType;
            masterAttributes.timingMode = timingMode;
            masterAttributes.geometry = numPorts;
            masterAttributes.imageSize = generalData->imageSize;
            masterAttributes.nPixels =
                xy(generalData->nPixelsX, generalData->nPixelsY);
            masterAttributes.maxFramesPerFile = generalData->framesPerFile;
            masterAttributes.frameDiscardMode = generalData->frameDiscardMode;
            masterAttributes.framePadding = framePadding;
            masterAttributes.scanParams = scanParams;
            masterAttributes.totalFrames = numberOfTotalFrames;
            masterAttributes.receiverRoi = receiverRoiMetadata;
            masterAttributes.exptime = acquisitionTime;
            masterAttributes.period = acquisitionPeriod;
            masterAttributes.burstMode = burstMode;
            masterAttributes.numUDPInterfaces = generalData->numUDPInterfaces;
            masterAttributes.dynamicRange = generalData->dynamicRange;
            masterAttributes.tenGiga = generalData->tengigaEnable;
            masterAttributes.thresholdEnergyeV = thresholdEnergyeV;
            masterAttributes.thresholdAllEnergyeV = thresholdAllEnergyeV;
            masterAttributes.subExptime = subExpTime;
            masterAttributes.subPeriod = subPeriod;
            masterAttributes.quad = quadEnable;
            masterAttributes.readNRows = readNRows;
            masterAttributes.ratecorr = rateCorrections;
            masterAttributes.adcmask = generalData->tengigaEnable
                                           ? generalData->adcEnableMaskTenGiga
                                           : generalData->adcEnableMaskOneGiga;
            masterAttributes.analog =
                (generalData->readoutType == ANALOG_ONLY ||
                 generalData->readoutType == ANALOG_AND_DIGITAL)
                    ? 1
                    : 0;
            masterAttributes.analogSamples = generalData->nAnalogSamples;
            masterAttributes.digital =
                (generalData->readoutType == DIGITAL_ONLY ||
                 generalData->readoutType == ANALOG_AND_DIGITAL ||
                 generalData->readoutType == DIGITAL_AND_TRANSCEIVER)
                    ? 1
                    : 0;
            masterAttributes.digitalSamples = generalData->nDigitalSamples;
            masterAttributes.dbitoffset = ctbDbitOffset;
            masterAttributes.dbitlist = 0;
            for (auto &i : ctbDbitList) {
                masterAttributes.dbitlist |= (static_cast<uint64_t>(1) << i);
            }
            masterAttributes.transceiverSamples =
                generalData->nTransceiverSamples;
            masterAttributes.transceiverMask = generalData->transceiverMask;
            masterAttributes.transceiver =
                (generalData->readoutType == TRANSCEIVER_ONLY ||
                 generalData->readoutType == DIGITAL_AND_TRANSCEIVER)
                    ? 1
                    : 0;
            masterAttributes.detectorRoi = generalData->detectorRoi;
            masterAttributes.counterMask = generalData->counterMask;
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
            std::string virtualFileName;
            // create virtual hdf5 file (if multiple files)
            if (dataProcessor[0]->GetFilesInAcquisition() > 1 ||
                (numPorts.x * numPorts.y) > 1) {
                virtualFileName = dataProcessor[0]->CreateVirtualFile(
                    filePath, fileName, fileIndex, overwriteEnable, silentMode,
                    modulePos, numPorts.x, numPorts.y, &hdf5LibMutex);
            }
            // link file in master
            if (masterFileWriteEnable) {
                dataProcessor[0]->LinkFileInMaster(
                    masterFileName, virtualFileName, silentMode, &hdf5LibMutex);
            }
        }
#endif
    } catch (const std::exception &e) {
        // ignore it and just print it
        LOG(logWARNING) << "Error creating master/virtualfiles: " << e.what();
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
    return generalData->numUDPInterfaces;
}

// not Eiger
void Implementation::setNumberofUDPInterfaces(const int n) {
    if (generalData->detType == EIGER) {
        throw RuntimeError("Cannot set number of UDP interfaces for Eiger");
    }

    if (generalData->numUDPInterfaces != n) {
        // clear all threads and fifos
        listener.clear();
        dataProcessor.clear();
        dataStreamer.clear();
        fifo.clear();

        // set local variables
        generalData->SetNumberofInterfaces(n);
        generalData->numUDPInterfaces = n;

        // fifo
        SetupFifoStructure();
        // recalculate port rois
        setReceiverROI(receiverRoi);

        // create threads
        for (int i = 0; i < generalData->numUDPInterfaces; ++i) {
            // listener and dataprocessor threads
            try {
                listener.push_back(sls::make_unique<Listener>(i, &status));
                SetupListener(i);
                dataProcessor.push_back(sls::make_unique<DataProcessor>(i));
                SetupDataProcessor(i);
            } catch (const std::exception &e) {
                listener.clear();
                dataProcessor.clear();
                throw;
            }

            // streamer threads
            if (dataStreamEnable) {
                try {
                    dataStreamer.push_back(sls::make_unique<DataStreamer>(i));
                    SetupDataStreamer(i);
                } catch (const std::exception &e) {
                    if (dataStreamEnable) {
                        dataStreamer.clear();
                        dataStreamEnable = false;
                        for (const auto &it : dataProcessor)
                            it->SetDataStreamEnable(dataStreamEnable);
                    }
                    throw;
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

        // test socket buffer size with current set up
        setUDPSocketBufferSize(0);
    }

    LOG(logINFO) << "Number of Interfaces: " << generalData->numUDPInterfaces;
}

std::string Implementation::getEthernetInterface() const { return eth[0]; }

void Implementation::setEthernetInterface(const std::string &c) {
    eth[0] = c;
    listener[0]->SetEthernetInterface(c);
    LOG(logINFO) << "Ethernet Interface: " << eth[0];
}

std::string Implementation::getEthernetInterface2() const { return eth[1]; }

void Implementation::setEthernetInterface2(const std::string &c) {
    eth[1] = c;
    if (listener.size() > 1) {
        listener[1]->SetEthernetInterface(c);
    }
    LOG(logINFO) << "Ethernet Interface 2: " << eth[1];
}

uint16_t Implementation::getUDPPortNumber() const { return udpPortNum[0]; }

void Implementation::setUDPPortNumber(const uint16_t i) {
    udpPortNum[0] = i;
    listener[0]->SetUdpPortNumber(i);
    dataProcessor[0]->SetUdpPortNumber(i);
    LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum[0];
}

uint16_t Implementation::getUDPPortNumber2() const { return udpPortNum[1]; }

void Implementation::setUDPPortNumber2(const uint16_t i) {
    udpPortNum[1] = i;
    if (listener.size() > 1) {
        listener[1]->SetUdpPortNumber(i);
        dataProcessor[1]->SetUdpPortNumber(i);
    }
    LOG(logINFO) << "UDP Port Number[1]: " << udpPortNum[1];
}

int Implementation::getUDPSocketBufferSize() const {
    return generalData->udpSocketBufferSize;
}

void Implementation::setUDPSocketBufferSize(const int s) {
    size_t listSize = listener.size();
    if ((generalData->detType == JUNGFRAU || generalData->detType == MOENCH ||
         generalData->detType == GOTTHARD2) &&
        (int)listSize != generalData->numUDPInterfaces) {
        throw RuntimeError("Number of Interfaces " +
                           std::to_string(generalData->numUDPInterfaces) +
                           " do not match listener size " +
                           std::to_string(listSize));
    }

    for (auto &l : listener) {
        l->CreateDummySocketForUDPSocketBufferSize(s,
                                                   actualUDPSocketBufferSize);
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
            for (int i = 0; i < generalData->numUDPInterfaces; ++i) {
                try {
                    dataStreamer.push_back(sls::make_unique<DataStreamer>(i));
                    SetupDataStreamer(i);
                } catch (const std::exception &e) {
                    dataStreamer.clear();
                    dataStreamEnable = false;
                    for (const auto &it : dataProcessor)
                        it->SetDataStreamEnable(dataStreamEnable);
                    throw;
                }
            }
            SetThreadPriorities();
        }
        for (const auto &it : dataProcessor)
            it->SetDataStreamEnable(dataStreamEnable);
    }
    LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;
}

uint32_t Implementation::getStreamingFrequency() const {
    return streamingFrequency;
}

void Implementation::setStreamingFrequency(const uint32_t freq) {
    streamingFrequency = freq;
    for (const auto &it : dataProcessor)
        it->SetStreamingFrequency(streamingFrequency);
    LOG(logINFO) << "Streaming Frequency: " << streamingFrequency;
}

uint32_t Implementation::getStreamingTimer() const {
    return streamingTimerInMs;
}

void Implementation::setStreamingTimer(const uint32_t time_in_ms) {
    streamingTimerInMs = time_in_ms;
    for (const auto &it : dataProcessor)
        it->SetStreamingTimerInMs(streamingTimerInMs);
    LOG(logINFO) << "Streamer Timer: " << streamingTimerInMs;
}

uint32_t Implementation::getStreamingStartingFrameNumber() const {
    return streamingStartFnum;
}

void Implementation::setStreamingStartingFrameNumber(const uint32_t fnum) {
    streamingStartFnum = fnum;
    for (const auto &it : dataProcessor)
        it->SetStreamingStartFnum(streamingStartFnum);
    LOG(logINFO) << "Streaming Start Frame num: " << streamingStartFnum;
}

uint16_t Implementation::getStreamingPort() const { return streamingPort; }

void Implementation::setStreamingPort(const uint16_t i) {
    streamingPort = i;
    LOG(logINFO) << "Streaming Port: " << streamingPort;
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
    for (const auto &it : dataProcessor) {
        it->SetAdditionalJsonHeader(c);
    }
    for (const auto &it : dataStreamer) {
        it->SetAdditionalJsonHeader(c);
    }
    LOG(logINFO) << "Additional JSON Header: "
                 << ToString(additionalJsonHeader);
}

std::string
Implementation::getAdditionalJsonParameter(const std::string &key) const {
    if (additionalJsonHeader.find(key) != additionalJsonHeader.end()) {
        return additionalJsonHeader.at(key);
    }
    throw RuntimeError("No key " + key + " found in additional json header");
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
    for (const auto &it : dataProcessor) {
        it->SetAdditionalJsonHeader(additionalJsonHeader);
    }
    for (const auto &it : dataStreamer) {
        it->SetAdditionalJsonHeader(additionalJsonHeader);
    }
    LOG(logINFO) << "Additional JSON Header: "
                 << ToString(additionalJsonHeader);
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
    if (generalData->detType == GOTTHARD2) {
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
    for (const auto &it : dataProcessor)
        it->SetNumberofTotalFrames(numberOfTotalFrames);
    for (const auto &it : dataStreamer)
        it->SetNumberofTotalFrames(numberOfTotalFrames);
    if (numberOfTotalFrames == 0) {
        throw RuntimeError("Invalid total number of frames to receive: 0");
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
    LOG(logINFO) << "Acquisition Period: " << ToString(acquisitionPeriod);
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
    LOG(logINFO) << "Acquisition Time: " << ToString(acquisitionTime);
}

void Implementation::setAcquisitionTime1(const ns i) {
    acquisitionTime1 = i;
    LOG(logINFO) << "Acquisition Time1: " << ToString(acquisitionTime1);
    updateAcquisitionTime();
}

void Implementation::setAcquisitionTime2(const ns i) {
    acquisitionTime2 = i;
    LOG(logINFO) << "Acquisition Time2: " << ToString(acquisitionTime2);
    updateAcquisitionTime();
}

void Implementation::setAcquisitionTime3(const ns i) {
    acquisitionTime3 = i;
    LOG(logINFO) << "Acquisition Time3: " << ToString(acquisitionTime3);
    updateAcquisitionTime();
}

void Implementation::setGateDelay1(const ns i) {
    gateDelay1 = i;
    LOG(logINFO) << "Gate Delay1: " << ToString(gateDelay1);
}

void Implementation::setGateDelay2(const ns i) {
    gateDelay2 = i;
    LOG(logINFO) << "Gate Delay2: " << ToString(gateDelay2);
}

void Implementation::setGateDelay3(const ns i) {
    gateDelay3 = i;
    LOG(logINFO) << "Gate Delay3: " << ToString(gateDelay3);
}

ns Implementation::getSubExpTime() const { return subExpTime; }

void Implementation::setSubExpTime(const ns i) {
    subExpTime = i;
    LOG(logINFO) << "Sub Exposure Time: " << ToString(subExpTime);
}

ns Implementation::getSubPeriod() const { return subPeriod; }

void Implementation::setSubPeriod(const ns i) {
    subPeriod = i;
    LOG(logINFO) << "Sub Period: " << ToString(subPeriod);
}

uint32_t Implementation::getNumberofAnalogSamples() const {
    return generalData->nAnalogSamples;
}

void Implementation::setNumberofAnalogSamples(const uint32_t i) {
    if (generalData->nAnalogSamples != i) {
        generalData->SetNumberOfAnalogSamples(i);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Analog Samples: " << generalData->nAnalogSamples;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getNumberofDigitalSamples() const {
    return generalData->nDigitalSamples;
}

void Implementation::setNumberofDigitalSamples(const uint32_t i) {
    if (generalData->nDigitalSamples != i) {
        generalData->SetNumberOfDigitalSamples(i);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Digital Samples: "
                 << generalData->nDigitalSamples;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getNumberofTransceiverSamples() const {
    return generalData->nTransceiverSamples;
}

void Implementation::setNumberofTransceiverSamples(const uint32_t i) {
    if (generalData->nTransceiverSamples != i) {
        generalData->SetNumberOfTransceiverSamples(i);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Transceiver Samples: "
                 << generalData->nTransceiverSamples;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getCounterMask() const {
    return generalData->counterMask;
}

void Implementation::setCounterMask(const uint32_t i) {
    if (generalData->counterMask != i) {
        generalData->SetCounterMask(i);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Counter mask: " << ToStringHex(generalData->counterMask);
    int ncounters = __builtin_popcount(generalData->counterMask);
    LOG(logINFO) << "Number of counters: " << ncounters;
}

uint32_t Implementation::getDynamicRange() const {
    return generalData->dynamicRange;
}

void Implementation::setDynamicRange(const uint32_t i) {
    if (generalData->dynamicRange != i) {
        if (generalData->detType == EIGER || generalData->detType == MYTHEN3) {
            generalData->SetDynamicRange(i);
            SetupFifoStructure();
        }
    }
    LOG(logINFO) << "Dynamic Range: " << generalData->dynamicRange;
}

slsDetectorDefs::ROI Implementation::getROI() const {
    return generalData->detectorRoi;
}

void Implementation::setDetectorROI(slsDetectorDefs::ROI arg) {
    if (generalData->detectorRoi.xmin != arg.xmin ||
        generalData->detectorRoi.xmax != arg.xmax) {
        // only for gotthard
        generalData->SetDetectorROI(arg);
        SetupFifoStructure();
    }

    LOG(logINFO) << "Detector ROI: " << ToString(generalData->detectorRoi);
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

bool Implementation::getTenGigaEnable() const {
    return generalData->tengigaEnable;
}

void Implementation::setTenGigaEnable(const bool b) {
    if (generalData->tengigaEnable != b) {
        generalData->SetTenGigaEnable(b);
        SetupFifoStructure();

        // datastream can be disabled/enabled only for Eiger 10GbE
        if (generalData->detType == EIGER) {
            if (!b) {
                detectorDataStream[LEFT] = 1;
                detectorDataStream[RIGHT] = 1;
            } else {
                detectorDataStream[LEFT] = detectorDataStream10GbE[LEFT];
                detectorDataStream[RIGHT] = detectorDataStream10GbE[RIGHT];
            }
            LOG(logDEBUG) << "Detector datastream updated [Left: "
                          << ToString(detectorDataStream[LEFT])
                          << ", Right: " << ToString(detectorDataStream[RIGHT])
                          << "]";
        }
    }
    LOG(logINFO) << "Ten Giga: "
                 << (generalData->tengigaEnable ? "enabled" : "disabled");
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

bool Implementation::getFlipRows() const { return flipRows; }

void Implementation::setFlipRows(bool enable) {
    flipRows = enable;
    for (const auto &it : dataProcessor)
        it->SetFlipRows(flipRows);
    for (const auto &it : dataStreamer)
        it->SetFlipRows(flipRows);
    LOG(logINFO) << "Flip Rows: " << flipRows;
}

bool Implementation::getQuad() const { return quadEnable; }

void Implementation::setQuad(const bool b) {
    if (quadEnable != b) {
        quadEnable = b;
        setDetectorSize(numModules);
        for (const auto &it : dataProcessor) {
            it->SetQuadEnable(quadEnable);
            it->SetFlipRows(flipRows);
        }
        for (const auto &it : dataStreamer) {
            it->SetQuadEnable(quadEnable);
            it->SetFlipRows(flipRows);
        }
    }
    LOG(logINFO) << "Quad Enable: " << quadEnable;
}

bool Implementation::getActivate() const { return activated; }

void Implementation::setActivate(bool enable) {
    activated = enable;
    for (const auto &it : listener)
        it->SetActivate(enable);
    for (const auto &it : dataProcessor)
        it->SetActivate(enable);

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
    LOG(logINFO) << "Detector 10GbE datastream (" << ToString(port)
                 << " Port): " << ToString(detectorDataStream10GbE[index]);
    // update datastream for 10g
    if (generalData->tengigaEnable) {
        detectorDataStream[index] = detectorDataStream10GbE[index];
        LOG(logDEBUG) << "Detector datastream updated ["
                      << (index == 0 ? "Left" : "Right")
                      << "] : " << ToString(detectorDataStream[index]);
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
    LOG(logINFO) << "Threshold Energy (eV): " << ToString(thresholdAllEnergyeV);
}

void Implementation::setRateCorrections(const std::vector<int64_t> &t) {
    rateCorrections = t;
    LOG(logINFO) << "Rate Corrections: " << ToString(rateCorrections);
}

slsDetectorDefs::readoutMode Implementation::getReadoutMode() const {
    return generalData->readoutType;
}

void Implementation::setReadoutMode(const readoutMode f) {
    if (generalData->readoutType != f) {
        generalData->SetReadoutMode(f);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Readout Mode: " << ToString(generalData->readoutType);
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getADCEnableMask() const {
    return generalData->adcEnableMaskOneGiga;
}

void Implementation::setADCEnableMask(uint32_t mask) {
    if (generalData->adcEnableMaskOneGiga != mask) {
        generalData->SetOneGigaAdcEnableMask(mask);
        SetupFifoStructure();
    }
    LOG(logINFO) << "ADC Enable Mask for 1Gb mode: 0x" << std::hex
                 << generalData->adcEnableMaskOneGiga << std::dec;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

uint32_t Implementation::getTenGigaADCEnableMask() const {
    return generalData->adcEnableMaskTenGiga;
}

void Implementation::setTenGigaADCEnableMask(uint32_t mask) {
    if (generalData->adcEnableMaskTenGiga != mask) {
        generalData->SetTenGigaAdcEnableMask(mask);
        SetupFifoStructure();
    }
    LOG(logINFO) << "ADC Enable Mask for 10Gb mode: 0x" << std::hex
                 << generalData->adcEnableMaskTenGiga << std::dec;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

std::vector<int> Implementation::getDbitList() const { return ctbDbitList; }

void Implementation::setDbitList(const std::vector<int> &v) {
    ctbDbitList = v;
    for (const auto &it : dataProcessor)
        it->SetCtbDbitList(ctbDbitList);
    LOG(logINFO) << "Dbit list: " << ToString(ctbDbitList);
}

int Implementation::getDbitOffset() const { return ctbDbitOffset; }

void Implementation::setDbitOffset(const int s) {
    ctbDbitOffset = s;
    for (const auto &it : dataProcessor)
        it->SetCtbDbitOffset(ctbDbitOffset);
    LOG(logINFO) << "Dbit offset: " << ctbDbitOffset;
}

uint32_t Implementation::getTransceiverEnableMask() const {
    return generalData->transceiverMask;
}

void Implementation::setTransceiverEnableMask(uint32_t mask) {
    if (generalData->transceiverMask != mask) {
        generalData->SetTransceiverEnableMask(mask);
        SetupFifoStructure();
    }
    LOG(logINFO) << "Transceiver Enable Mask: 0x" << std::hex
                 << generalData->transceiverMask << std::dec;
    LOG(logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
}

/**************************************************
 *                                                *
 *    Callbacks                                   *
 *                                                *
 * ************************************************/
void Implementation::registerCallBackStartAcquisition(
    int (*func)(const startCallbackHeader, void *), void *arg) {
    startAcquisitionCallBack = func;
    pStartAcquisition = arg;
}

void Implementation::registerCallBackAcquisitionFinished(
    void (*func)(const endCallbackHeader, void *), void *arg) {
    acquisitionFinishedCallBack = func;
    pAcquisitionFinished = arg;
}

void Implementation::registerCallBackRawDataReady(
    void (*func)(sls_receiver_header &, dataCallbackHeader, char *, size_t &,
                 void *),
    void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
    for (const auto &it : dataProcessor)
        it->registerCallBackRawDataReady(rawDataReadyCallBack, pRawDataReady);
}

} // namespace sls
