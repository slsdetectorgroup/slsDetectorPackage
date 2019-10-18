/********************************************/ /**
                                                * @file
                                                *slsReceiverImplementation.cpp
                                                * @short does all the functions
                                                *for a receiver, set/get
                                                *parameters, start/stop etc.
                                                ***********************************************/

#include "slsReceiverImplementation.h"
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

slsReceiverImplementation::slsReceiverImplementation() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    InitializeMembers();
}

slsReceiverImplementation::~slsReceiverImplementation() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    DeleteMembers();
}

void slsReceiverImplementation::DeleteMembers() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (generalData) {
        delete generalData;
        generalData = nullptr;
    }

    listener.clear();
    dataProcessor.clear();
    dataStreamer.clear();
    fifo.clear();
    ctbDbitList.clear();
}

void slsReceiverImplementation::InitializeMembers() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    //**detector parameters***
    myDetectorType = GENERIC;
    for (int i = 0; i < MAX_DIMENSIONS; ++i)
        numDet[i] = 0;
    detID = 0;
    strcpy(detHostname, "");
    acquisitionPeriod = SAMPLE_TIME_IN_NS;
    acquisitionTime = 0;
    subExpTime = 0;
    subPeriod = 0;
    numberOfFrames = 0;
    numberOfAnalogSamples = 0;
    numberOfDigitalSamples = 0;
    dynamicRange = 16;
    tengigaEnable = false;
    fifoDepth = 0;
    flippedDataX = 0;
    gapPixelsEnable = false;
    quadEnable = false;
    numLinesReadout = MAX_EIGER_ROWS_PER_READOUT;
    readoutType = ANALOG_ONLY;

    //*** receiver parameters ***
    numThreads = 1;
    status = IDLE;
    activated = true;
    deactivatedPaddingEnable = true;
    frameDiscardMode = NO_DISCARD;
    framePadding = false;
    silentMode = false;
    ctbDbitOffset = 0;
    ctbAnalogDataBytes = 0;

    //***connection parameters***
    numUDPInterfaces = 1;
    eth.resize(MAX_NUMBER_OF_LISTENING_THREADS);
    for (int i = 0; i < MAX_NUMBER_OF_LISTENING_THREADS; i++) {
        udpPortNum[i] = DEFAULT_UDP_PORTNO + i;
    }
    udpSocketBufferSize = 0;
    actualUDPSocketBufferSize = 0;

    //***file parameters***
    fileFormatType = BINARY;
    strcpy(fileName, "run");
    strcpy(filePath, "");
    fileIndex = 0;
    framesPerFile = 0;
    fileWriteEnable = true;
    masterFileWriteEnable = true;
    overwriteEnable = true;

    //***acquisition parameters***
    roi.xmin = -1;
    roi.xmax = -1;
    adcEnableMask = BIT32_MASK;
    streamingFrequency = 1;
    streamingTimerInMs = DEFAULT_STREAMING_TIMER_IN_MS;
    dataStreamEnable = false;
    streamingPort = 0;
    streamingSrcIP = 0u;
    memset(additionalJsonHeader, 0, sizeof(additionalJsonHeader));

    //** class objects ***
    generalData = nullptr;

    //***callback parameters***
    startAcquisitionCallBack = nullptr;
    pStartAcquisition = nullptr;
    acquisitionFinishedCallBack = nullptr;
    pAcquisitionFinished = nullptr;
    rawDataReadyCallBack = nullptr;
    rawDataModifyReadyCallBack = nullptr;
    pRawDataReady = nullptr;
}

/*************************************************************************
 * Getters ***************************************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

/**initial parameters***/
int *slsReceiverImplementation::getMultiDetectorSize() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return (int *)numDet;
}

int slsReceiverImplementation::getDetectorPositionId() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return detID;
}

std::string slsReceiverImplementation::getDetectorHostname() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return std::string(detHostname);
}

int slsReceiverImplementation::getFlippedDataX() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return flippedDataX;
}

bool slsReceiverImplementation::getGapPixelsEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return gapPixelsEnable;
}

bool slsReceiverImplementation::getQuad() const {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	return quadEnable;
}

int slsReceiverImplementation::getReadNLines() const {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	return numLinesReadout;
}

slsDetectorDefs::readoutMode
slsReceiverImplementation::getReadoutMode() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return readoutType;
}

/***file parameters***/
slsDetectorDefs::fileFormat slsReceiverImplementation::getFileFormat() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileFormatType;
}

std::string slsReceiverImplementation::getFileName() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return std::string(fileName);
}

std::string slsReceiverImplementation::getFilePath() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return std::string(filePath);
}

uint64_t slsReceiverImplementation::getFileIndex() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileIndex;
}

uint32_t slsReceiverImplementation::getFramesPerFile() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framesPerFile;
}

slsDetectorDefs::frameDiscardPolicy
slsReceiverImplementation::getFrameDiscardPolicy() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return frameDiscardMode;
}

bool slsReceiverImplementation::getFramePaddingEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framePadding;
}

bool slsReceiverImplementation::getFileWriteEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileWriteEnable;
}

bool slsReceiverImplementation::getMasterFileWriteEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return masterFileWriteEnable;
}

bool slsReceiverImplementation::getOverwriteEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return overwriteEnable;
}

/***acquisition count parameters***/
uint64_t slsReceiverImplementation::getTotalFramesCaught() const {
    uint64_t sum = 0;
    uint32_t flagsum = 0;

    for (const auto &it : dataProcessor) {
        flagsum += it->GetMeasurementStartedFlag();
        sum += it->GetNumTotalFramesCaught();
    }
    // no data processed
    if (flagsum != dataProcessor.size())
        return 0;

    return (sum / dataProcessor.size());
}

uint64_t slsReceiverImplementation::getFramesCaught() const {
    uint64_t sum = 0;
    uint32_t flagsum = 0;

    for (const auto &it : dataProcessor) {
        flagsum += it->GetMeasurementStartedFlag();
        sum += it->GetNumFramesCaught();
    }
    // no data processed
    if (flagsum != dataProcessor.size())
        return 0;

    return (sum / dataProcessor.size());
}

uint64_t slsReceiverImplementation::getAcquisitionIndex() const {
    uint64_t sum = 0;
    uint32_t flagsum = 0;

    for (const auto &it : dataProcessor) {
        flagsum += it->GetMeasurementStartedFlag();
        sum += it->GetActualProcessedAcquisitionIndex();
    }
    // no data processed
    if (flagsum != dataProcessor.size())
        return 0;

    return (sum / dataProcessor.size());
}

/***connection parameters***/
uint32_t slsReceiverImplementation::getUDPPortNumber() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpPortNum[0];
}

uint32_t slsReceiverImplementation::getUDPPortNumber2() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpPortNum[1];
}

std::string slsReceiverImplementation::getEthernetInterface() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return eth[0];
}

std::string slsReceiverImplementation::getEthernetInterface2() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return eth[1];
}

int slsReceiverImplementation::getNumberofUDPInterfaces() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numUDPInterfaces;
}

/***acquisition parameters***/
slsDetectorDefs::ROI slsReceiverImplementation::getROI() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return roi;
}

uint32_t slsReceiverImplementation::getADCEnableMask() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return adcEnableMask;
}

uint32_t slsReceiverImplementation::getStreamingFrequency() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingFrequency;
}

uint32_t slsReceiverImplementation::getStreamingTimer() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingTimerInMs;
}

bool slsReceiverImplementation::getDataStreamEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return dataStreamEnable;
}

uint64_t slsReceiverImplementation::getAcquisitionPeriod() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionPeriod;
}

uint64_t slsReceiverImplementation::getAcquisitionTime() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionTime;
}

uint64_t slsReceiverImplementation::getSubExpTime() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subExpTime;
}

uint64_t slsReceiverImplementation::getSubPeriod() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subPeriod;
}

uint64_t slsReceiverImplementation::getNumberOfFrames() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfFrames;
}

uint64_t slsReceiverImplementation::getNumberofAnalogSamples() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfAnalogSamples;
}

uint64_t slsReceiverImplementation::getNumberofDigitalSamples() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfDigitalSamples;
}

uint32_t slsReceiverImplementation::getDynamicRange() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return dynamicRange;
}

bool slsReceiverImplementation::getTenGigaEnable() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return tengigaEnable;
}

uint32_t slsReceiverImplementation::getFifoDepth() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fifoDepth;
}

/***receiver status***/
slsDetectorDefs::runStatus slsReceiverImplementation::getStatus() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return status;
}

bool slsReceiverImplementation::getSilentMode() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return silentMode;
}

std::vector<int> slsReceiverImplementation::getDbitList() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitList;
}

int slsReceiverImplementation::getDbitOffset() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitOffset;
}

bool slsReceiverImplementation::getActivate() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return activated;
}

bool slsReceiverImplementation::getDeactivatedPadding() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return deactivatedPaddingEnable;
}

uint32_t slsReceiverImplementation::getStreamingPort() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingPort;
}

sls::IpAddr slsReceiverImplementation::getStreamingSourceIP() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingSrcIP;
}

std::string slsReceiverImplementation::getAdditionalJsonHeader() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return std::string(additionalJsonHeader);
}

int64_t slsReceiverImplementation::getUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpSocketBufferSize;
}

int64_t slsReceiverImplementation::getActualUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return actualUDPSocketBufferSize;
}

/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/**initial parameters***/

void slsReceiverImplementation::setDetectorHostname(const char *c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (strlen(c))
        strcpy(detHostname, c);
    FILE_LOG(logINFO) << "Detector Hostname: " << detHostname;
}

void slsReceiverImplementation::setMultiDetectorSize(const int *size) {
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

void slsReceiverImplementation::setFlippedDataX(int enable) {
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

int slsReceiverImplementation::setGapPixelsEnable(const bool b) {
    if (gapPixelsEnable != b) {
        gapPixelsEnable = b;

        // side effects
        generalData->SetGapPixelsEnable(b, dynamicRange, quadEnable);
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }
    FILE_LOG(logINFO) << "Gap Pixels Enable: " << gapPixelsEnable;
    return OK;
}


int slsReceiverImplementation::setQuad(const bool b) {
	if (quadEnable != b) {
		quadEnable = b;

		generalData->SetGapPixelsEnable(gapPixelsEnable, dynamicRange, b);
		// to update npixelsx, npixelsy in file writer
        for (const auto &it : dataProcessor)
		    it->SetPixelDimension();
		if (SetupFifoStructure() == FAIL)
			return FAIL;	

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
    return OK;
}

void slsReceiverImplementation::setReadNLines(const int value) {
    numLinesReadout = value;
	FILE_LOG(logINFO)  << "Number of Lines to readout: " << numLinesReadout;
}

int slsReceiverImplementation::setReadoutMode(const readoutMode f) {
    if (readoutType != f) {
        readoutType = f;

        // side effects
        ctbAnalogDataBytes = generalData->setImageSize(
            adcEnableMask, numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }

    FILE_LOG(logINFO) << "Readout Mode: " << sls::ToString(f);
    FILE_LOG(logINFO) << "Packets per Frame: "
                          << (generalData->packetsPerFrame);
    return OK;
}

void slsReceiverImplementation::setFileFormat(const fileFormat f) {
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

void slsReceiverImplementation::setFileName(const char c[]) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (strlen(c))
        strcpy(fileName, c);
    FILE_LOG(logINFO) << "File name: " << fileName;
}

void slsReceiverImplementation::setFilePath(const char c[]) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (strlen(c)) {
        mkdir_p(c); //throws if it can't create
        strcpy(filePath, c);
    }
    FILE_LOG(logINFO) << "File path: " << filePath;
}

void slsReceiverImplementation::setFileIndex(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    fileIndex = i;
    FILE_LOG(logINFO) << "File Index: " << fileIndex;
}

void slsReceiverImplementation::setFramesPerFile(const uint32_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framesPerFile = i;
    FILE_LOG(logINFO) << "Frames per file: " << framesPerFile;
}

void slsReceiverImplementation::setFrameDiscardPolicy(
    const frameDiscardPolicy i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (i >= 0 && i < NUM_DISCARD_POLICIES)
        frameDiscardMode = i;

    FILE_LOG(logINFO) << "Frame Discard Policy: "
                      << sls::ToString(frameDiscardMode);
}

void slsReceiverImplementation::setFramePaddingEnable(const bool i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framePadding = i;
    FILE_LOG(logINFO) << "Frame Padding: " << framePadding;
}

void slsReceiverImplementation::setFileWriteEnable(const bool b) {
    if (fileWriteEnable != b) {
        fileWriteEnable = b;
        for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
            dataProcessor[i]->SetupFileWriter(
                fileWriteEnable, (int *)numDet, &framesPerFile, fileName,
                filePath, &fileIndex, &overwriteEnable, &detID, &numThreads,
                &numberOfFrames, &dynamicRange, &udpPortNum[i], generalData);
        }
    }

    FILE_LOG(logINFO) << "File Write Enable: " << stringEnable(fileWriteEnable);
}

void slsReceiverImplementation::setMasterFileWriteEnable(const bool b) {
    masterFileWriteEnable = b;

    FILE_LOG(logINFO) << "Master File Write Enable: "
                      << stringEnable(masterFileWriteEnable);
}

void slsReceiverImplementation::setOverwriteEnable(const bool b) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    overwriteEnable = b;
    FILE_LOG(logINFO) << "Overwrite Enable: " << stringEnable(overwriteEnable);
}

/***connection parameters***/
void slsReceiverImplementation::setUDPPortNumber(const uint32_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    udpPortNum[0] = i;
    FILE_LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum[0];
}

void slsReceiverImplementation::setUDPPortNumber2(const uint32_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    udpPortNum[1] = i;
    FILE_LOG(logINFO) << "UDP Port Number[1]: " << udpPortNum[1];
}

void slsReceiverImplementation::setEthernetInterface(const std::string &c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    eth[0] = c;
    FILE_LOG(logINFO) << "Ethernet Interface: " << eth[0];
}

void slsReceiverImplementation::setEthernetInterface2(const std::string &c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    eth[1] = c;
    FILE_LOG(logINFO) << "Ethernet Interface 2: " << eth[1];
}

int slsReceiverImplementation::setNumberofUDPInterfaces(const int n) {
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
        if (SetupFifoStructure() == FAIL)
            return FAIL;

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
                FILE_LOG(logERROR)
                    << "Could not create listener/dataprocessor threads (index:"
                    << i << ")";
                listener.clear();
                dataProcessor.clear();
                return FAIL;
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
                        fd, additionalJsonHeader, (int*)nd, &gapPixelsEnable, &quadEnable));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numThreads, streamingPort, streamingSrcIP);

                } catch (...) {
                    FILE_LOG(logERROR)
                        << "Could not create datastreamer threads (index:" << i
                        << ")";
                    if (dataStreamEnable) {
                        dataStreamer.clear();
                        dataStreamEnable = false;
                    }
                    return FAIL;
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
        if (setUDPSocketBufferSize(0) == FAIL) {
            return FAIL;
        }
    }

    FILE_LOG(logINFO) << "Number of Interfaces: " << numUDPInterfaces;
    return OK;
}

int slsReceiverImplementation::setUDPSocketBufferSize(const int64_t s) {
    int64_t size = (s == 0) ? udpSocketBufferSize : s;
    size_t listSize = listener.size();

    if (myDetectorType == JUNGFRAU && (int)listSize != numUDPInterfaces) {
        FILE_LOG(logERROR) << "Number of Interfaces " << numUDPInterfaces
                           << " do not match listener size " << listSize;
        return FAIL;
    }

    for (unsigned int i = 0; i < listSize; ++i) {
        if (listener[i]->CreateDummySocketForUDPSocketBufferSize(size) == FAIL)
            return FAIL;
    }

    return OK;
}

/***acquisition parameters***/
int slsReceiverImplementation::setROI(slsDetectorDefs::ROI arg) {
    if (roi.xmin != arg.xmin || roi.xmax != arg.xmax) {
        roi.xmin = arg.xmin;
        roi.xmax = arg.xmax;

        // only for gotthard
        generalData->SetROI(arg);
        framesPerFile = generalData->maxFramesPerFile;
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }

    FILE_LOG(logINFO) << "ROI: [" << roi.xmin << ", " << roi.xmax << "]";;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
    return OK;
}

int slsReceiverImplementation::setADCEnableMask(uint32_t mask) {
    if (adcEnableMask != mask) {
        adcEnableMask = mask;

        ctbAnalogDataBytes = generalData->setImageSize(
            mask, numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);

        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }

    FILE_LOG(logINFO) << "ADC Enable Mask: 0x" << std::hex << adcEnableMask
                      << std::dec;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
    return OK;
}

int slsReceiverImplementation::setStreamingFrequency(const uint32_t freq) {
    if (streamingFrequency != freq) {
        streamingFrequency = freq;
    }
    FILE_LOG(logINFO) << "Streaming Frequency: " << streamingFrequency;
    return OK;
}

void slsReceiverImplementation::setStreamingTimer(const uint32_t time_in_ms) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    streamingTimerInMs = time_in_ms;
    FILE_LOG(logINFO) << "Streamer Timer: " << streamingTimerInMs;
}

int slsReceiverImplementation::setDataStreamEnable(const bool enable) {

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
                        fd, additionalJsonHeader, (int*)nd, &gapPixelsEnable, &quadEnable));
                    dataStreamer[i]->SetGeneralData(generalData);
                    dataStreamer[i]->CreateZmqSockets(
                        &numThreads, streamingPort, streamingSrcIP);
                } catch (...) {
                    dataStreamer.clear();
                    dataStreamEnable = false;
                    return FAIL;
                }
            }
            SetThreadPriorities();
        }
    }
    FILE_LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;
    return OK;
}

void slsReceiverImplementation::setStreamingPort(const uint32_t i) {
    streamingPort = i;

    FILE_LOG(logINFO) << "Streaming Port: " << streamingPort;
}

void slsReceiverImplementation::setStreamingSourceIP(const sls::IpAddr ip) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    streamingSrcIP = ip;
    FILE_LOG(logINFO) << "Streaming Source IP: " << streamingSrcIP;
}

void slsReceiverImplementation::setAdditionalJsonHeader(const char c[]) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    strcpy(additionalJsonHeader, c);
    FILE_LOG(logINFO) << "Additional JSON Header: " << additionalJsonHeader;
}

int slsReceiverImplementation::setAcquisitionPeriod(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionPeriod = i;
    FILE_LOG(logINFO) << "Acquisition Period: "
                      << (double)acquisitionPeriod / (1E9) << "s";

    return OK;
}

int slsReceiverImplementation::setAcquisitionTime(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionTime = i;
    FILE_LOG(logINFO) << "Acquisition Time: " << (double)acquisitionTime / (1E9)
                      << "s";

    return OK;
}

void slsReceiverImplementation::setSubExpTime(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subExpTime = i;
    FILE_LOG(logINFO) << "Sub Exposure Time: " << (double)subExpTime / (1E9)
                      << "s";
}

void slsReceiverImplementation::setSubPeriod(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subPeriod = i;
    FILE_LOG(logINFO) << "Sub Exposure Period: " << (double)subPeriod / (1E9)
                      << "s";
}

void slsReceiverImplementation::setNumberOfFrames(const uint64_t i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    numberOfFrames = i;
    FILE_LOG(logINFO) << "Number of Frames: " << numberOfFrames;
}

int slsReceiverImplementation::setNumberofAnalogSamples(const uint64_t i) {
    if (numberOfAnalogSamples != i) {
        numberOfAnalogSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            adcEnableMask, numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }
    FILE_LOG(logINFO) << "Number of Analog Samples: " << numberOfAnalogSamples;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
    return OK;
}

int slsReceiverImplementation::setNumberofDigitalSamples(const uint64_t i) {
    if (numberOfDigitalSamples != i) {
        numberOfDigitalSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            adcEnableMask, numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }
    FILE_LOG(logINFO) << "Number of Digital Samples: "
                      << numberOfDigitalSamples;
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
    return OK;
}

int slsReceiverImplementation::setDynamicRange(const uint32_t i) {
    // only eiger
    if (dynamicRange != i) {
        dynamicRange = i;
        generalData->SetDynamicRange(i, tengigaEnable);
        generalData->SetGapPixelsEnable(gapPixelsEnable, dynamicRange, quadEnable);
        // to update npixelsx, npixelsy in file writer
        for (const auto &it : dataProcessor)
            it->SetPixelDimension();
        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }
    FILE_LOG(logINFO) << "Dynamic Range: " << dynamicRange;
    return OK;
}

int slsReceiverImplementation::setTenGigaEnable(const bool b) {
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
                adcEnableMask, numberOfAnalogSamples, numberOfDigitalSamples,
                tengigaEnable, readoutType);
            break;
        default:
            break;
        }

        if (SetupFifoStructure() == FAIL)
            return FAIL;
    }
    FILE_LOG(logINFO) << "Ten Giga: " << stringEnable(tengigaEnable);
    FILE_LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
    return OK;
}

int slsReceiverImplementation::setFifoDepth(const uint32_t i) {
    if (fifoDepth != i) {
        fifoDepth = i;
        if (SetupFifoStructure() == FAIL)
            throw sls::RuntimeError("Failed to setup fifo structure");
    }
    FILE_LOG(logINFO) << "Fifo Depth: " << i;
    return OK;
}

/***receiver parameters***/
bool slsReceiverImplementation::setActivate(bool enable) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    activated = enable;
    FILE_LOG(logINFO) << "Activation: " << stringEnable(activated);
    return activated;
}

bool slsReceiverImplementation::setDeactivatedPadding(bool enable) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    deactivatedPaddingEnable = enable;
    FILE_LOG(logINFO) << "Deactivated Padding Enable: "
                      << stringEnable(deactivatedPaddingEnable);
    return deactivatedPaddingEnable;
}

void slsReceiverImplementation::setSilentMode(const bool i) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    silentMode = i;
    FILE_LOG(logINFO) << "Silent Mode: " << i;
}

void slsReceiverImplementation::setDbitList(const std::vector<int> v) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitList = v;
}

void slsReceiverImplementation::setDbitOffset(const int s) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitOffset = s;
}

/*************************************************************************
 * Behavioral functions***************************************************
 * They may modify the status of the receiver ****************************
 *************************************************************************/

/***initial functions***/
int slsReceiverImplementation::setDetectorType(const detectorType d) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    DeleteMembers();
    InitializeMembers();
    myDetectorType = d;
    switch (myDetectorType) {
    case GOTTHARD:
    case EIGER:
    case JUNGFRAU:
    case CHIPTESTBOARD:
    case MOENCH:
        FILE_LOG(logINFO) << " ***** " << sls::ToString(d)
                          << " Receiver *****";
        break;
    default:
        FILE_LOG(logERROR) << "This is an unknown receiver type " << (int)d;
        return FAIL;
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
    default:
        break;
    }
    numThreads = generalData->threadsPerReceiver;
    fifoDepth = generalData->defaultFifoDepth;
    udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;
    framesPerFile = generalData->maxFramesPerFile;

    SetLocalNetworkParameters();
    if (SetupFifoStructure() == FAIL) {
        FILE_LOG(logERROR) << "Could not allocate memory for fifo structure";
        return FAIL;
    }

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
            FILE_LOG(logERROR)
                << "Could not create listener/dataprocessor threads (index:"
                << i << ")";
            listener.clear();
            dataProcessor.clear();
            return FAIL;
        }
    }

    // set up writer and callbacks

    for (const auto &it : listener)
        it->SetGeneralData(generalData);
    for (const auto &it : dataProcessor)
        it->SetGeneralData(generalData);
    SetThreadPriorities();

    FILE_LOG(logDEBUG) << " Detector type set to " << sls::ToString(d);
    return OK;
}

void slsReceiverImplementation::setDetectorPositionId(const int id) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    detID = id;
    FILE_LOG(logINFO) << "Detector Position Id:" << detID;
    for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
        dataProcessor[i]->SetupFileWriter(
            fileWriteEnable, (int *)numDet, &framesPerFile, fileName, filePath,
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

/***acquisition functions***/
void slsReceiverImplementation::resetAcquisitionCount() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : listener)
        it->ResetParametersforNewAcquisition();
    for (const auto &it : dataProcessor)
        it->ResetParametersforNewAcquisition();
    for (const auto &it : dataStreamer)
        it->ResetParametersforNewAcquisition();

    FILE_LOG(logINFO) << "Acquisition Count has been reset";
}

int slsReceiverImplementation::startReceiver(char *c) {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    FILE_LOG(logINFO) << "Starting Receiver";
    ResetParametersforNewMeasurement();

    // listener
    if (CreateUDPSockets() == FAIL) {
        strcpy(c, "Could not create UDP Socket(s).");
        FILE_LOG(logERROR) << c;
        return FAIL;
    }

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
        if (SetupWriter() == FAIL) {
            strcpy(c, "Could not create file.\n");
            FILE_LOG(logERROR) << c;
            return FAIL;
        }
    } else
        FILE_LOG(logINFO) << "File Write Disabled";

    FILE_LOG(logINFO) << "Ready ...";

    // status
    status = RUNNING;

    // Let Threads continue to be ready for acquisition
    StartRunning();

    FILE_LOG(logINFO) << "Receiver Started";
    FILE_LOG(logINFO) << "Status: " << sls::ToString(status);
    return OK;
}

void slsReceiverImplementation::stopReceiver() {
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
                std::max(maxIndexCaught, it->GetProcessedMeasurementIndex());
            if (it->GetMeasurementStartedFlag())
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
        uint64_t tot = 0;
        for (int i = 0; i < numThreads; i++) {
            tot += dataProcessor[i]->GetNumFramesCaught();
            int64_t missingpackets =
                numberOfFrames * generalData->packetsPerFrame -
                listener[i]->GetPacketsCaught();

            // partial readout
            if (numLinesReadout != MAX_EIGER_ROWS_PER_READOUT) {
                int maxnp = generalData->packetsPerFrame;
                int np = ((numLinesReadout * maxnp) / MAX_EIGER_ROWS_PER_READOUT);
                missingpackets = numberOfFrames * np - listener[i]->GetPacketsCaught();
            }

            TLogLevel lev =
                (((int64_t)missingpackets) > 0) ? logINFORED : logINFOGREEN;
            FILE_LOG(lev) <<
                // udp port number could be the second if selected interface is
                // 2 for jungfrau
                "Summary of Port " << udpPortNum[i]
                          << "\n\tMissing Packets\t\t: " << missingpackets
                          << "\n\tComplete Frames\t\t: "
                          << dataProcessor[i]->GetNumFramesCaught()
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

void slsReceiverImplementation::startReadout() {
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

void slsReceiverImplementation::shutDownUDPSockets() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : listener)
        it->ShutDownUDPSocket();
}

void slsReceiverImplementation::closeFiles() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    uint64_t maxIndexCaught = 0;
    bool anycaught = false;
    for (const auto &it : dataProcessor) {
        it->CloseFiles();
        maxIndexCaught =
            std::max(maxIndexCaught, it->GetProcessedMeasurementIndex());
        if (it->GetMeasurementStartedFlag())
            anycaught = true;
    }
    // to create virtual file & set files/acquisition to 0 (only hdf5 at the
    // moment)
    dataProcessor[0]->EndofAcquisition(anycaught, maxIndexCaught);
}

int slsReceiverImplementation::restreamStop() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    for (const auto &it : dataStreamer) {
        if (it->RestreamStop() == FAIL)
            throw sls::RuntimeError("Could not restream stop packet");
    }
    FILE_LOG(logINFO) << "Restreaming Dummy Header via ZMQ successful";
    return OK;
}

/***callback functions***/
void slsReceiverImplementation::registerCallBackStartAcquisition(
    int (*func)(char *, char *, uint64_t, uint32_t, void *), void *arg) {
    startAcquisitionCallBack = func;
    pStartAcquisition = arg;
}

void slsReceiverImplementation::registerCallBackAcquisitionFinished(
    void (*func)(uint64_t, void *), void *arg) {
    acquisitionFinishedCallBack = func;
    pAcquisitionFinished = arg;
}

void slsReceiverImplementation::registerCallBackRawDataReady(
    void (*func)(char *, char *, uint32_t, void *), void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
    for (const auto &it : dataProcessor)
        it->registerCallBackRawDataReady(rawDataReadyCallBack, pRawDataReady);
}

void slsReceiverImplementation::registerCallBackRawDataModifyReady(
    void (*func)(char *, char *, uint32_t &, void *), void *arg) {
    rawDataModifyReadyCallBack = func;
    pRawDataReady = arg;
    for (const auto &it : dataProcessor)
        it->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,
                                               pRawDataReady);
}

void slsReceiverImplementation::SetLocalNetworkParameters() {
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

void slsReceiverImplementation::SetThreadPriorities() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    for (const auto &it : listener) {
        if (it->SetThreadPriority(LISTENER_PRIORITY) == FAIL) {
            FILE_LOG(logWARNING) << "Could not prioritize listener threads. "
                                    "(No Root Privileges?)";
            return;
        }
    }
    std::ostringstream osfn;
    osfn << "Priorities set - "
            "Listener:"
         << LISTENER_PRIORITY;

    FILE_LOG(logINFO) << osfn.str();
}

int slsReceiverImplementation::SetupFifoStructure() {
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
            FILE_LOG(logERROR)
                << "Could not allocate memory for fifo structure of index "
                << i;
            fifo.clear();
            return FAIL;
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
    return OK;
}

void slsReceiverImplementation::ResetParametersforNewMeasurement() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

    for (const auto &it : listener)
        it->ResetParametersforNewMeasurement();
    for (const auto &it : dataProcessor)
        it->ResetParametersforNewMeasurement();

    if (dataStreamEnable) {
        char fnametostream[MAX_STR_LENGTH * 2];
        snprintf(fnametostream, MAX_STR_LENGTH * 2, "%s/%s", filePath,
                 fileName);
        for (const auto &it : dataStreamer)
            it->ResetParametersforNewMeasurement(fnametostream);
    }
}

int slsReceiverImplementation::CreateUDPSockets() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    bool error = false;

    for (unsigned int i = 0; i < listener.size(); ++i) {
        if (listener[i]->CreateUDPSockets() == FAIL) {
            error = true;
            break;
        }
    }

    if (error) {
        shutDownUDPSockets();
        return FAIL;
    }

    FILE_LOG(logDEBUG) << "UDP socket(s) created successfully.";
    return OK;
}

int slsReceiverImplementation::SetupWriter() {
    FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
    bool error = false;
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
    attr.adcmask = adcEnableMask;
    attr.dbitoffset = ctbDbitOffset;
    attr.dbitlist = 0;
    attr.roiXmin = roi.xmin;
    attr.roiXmax = roi.xmax;
    for (auto &i : ctbDbitList) {
        attr.dbitlist |= (1 << i);
    }
    for (unsigned int i = 0; i < dataProcessor.size(); ++i)
        if (dataProcessor[i]->CreateNewFile(attr) == FAIL) {
            error = true;
            break;
        }
    if (error) {
        shutDownUDPSockets();
        closeFiles();
        return FAIL;
    }

    return OK;
}

void slsReceiverImplementation::StartRunning() {
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
