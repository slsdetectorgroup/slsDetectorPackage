#include "slsDetector.h"
#include "ClientInterface.h"
#include "ClientSocket.h"
#include "MySocketTCP.h"
#include "ServerInterface.h"
#include "SharedMemory.h"
#include "file_utils.h"
#include "gitInfoLib.h"
#include "multiSlsDetector.h"
#include "network_utils.h"
#include "slsDetectorCommand.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"
#include "versionAPI.h"
#include <arpa/inet.h>
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace sls;

#define DEFAULT_HOSTNAME "localhost"

slsDetector::slsDetector(detectorType type, int multi_id, int det_id, bool verify)
    : detId(det_id), detector_shm(multi_id, det_id) {
    /* called from put hostname command,
     * so sls shared memory will be created */

    // ensure shared memory was not created before
    if (detector_shm.IsExisting()) {
        FILE_LOG(logWARNING) << "This shared memory should have been "
                                "deleted before! "
                             << detector_shm.GetName() << ". Freeing it again";
        freeSharedMemory(multi_id, det_id);
    }

    initSharedMemory(type, multi_id, verify);
}

slsDetector::slsDetector(int multi_id, int det_id, bool verify)
    : detId(det_id), detector_shm(multi_id, det_id) {
    /* called from multi constructor to populate structure,
     * so sls shared memory will be opened, not created */

    // getDetectorType From shm will check if it was already existing
    detectorType type = getDetectorTypeFromShm(multi_id, verify);
    initSharedMemory(type, multi_id, verify);
}

slsDetector::~slsDetector() = default;

int slsDetector::checkDetectorVersionCompatibility() {
    // TODO! Verify that this works as intended when version don't match
    int fnum = F_CHECK_VERSION;
    int ret = FAIL;
    int64_t arg = 0;

    // get api version number for detector server
    switch (detector_shm()->myDetectorType) {
    case EIGER:
        arg = APIEIGER;
        break;
    case JUNGFRAU:
        arg = APIJUNGFRAU;
        break;
    case GOTTHARD:
        arg = APIGOTTHARD;
        break;
    case CHIPTESTBOARD:
        arg = APICTB;
        break;
    case MOENCH:
        arg = APIMOENCH;
        break;
    default:
        throw NotImplementedError(
            "Check version compatibility is not implemented for this detector");
    }
    FILE_LOG(logDEBUG1) << "Checking version compatibility with detector with value " << std::hex
                        << arg << std::dec;

    // control server
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        // in case it throws
        detector_shm()->detectorControlAPIVersion = 0;
        detector_shm()->detectorStopAPIVersion = 0;
        detector_shm()->onlineFlag = OFFLINE_FLAG;

        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);

        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        ret = stop.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);

        // success
        detector_shm()->detectorControlAPIVersion = arg;
        detector_shm()->detectorStopAPIVersion = arg;
        detector_shm()->onlineFlag = ONLINE_FLAG;
    }
    return ret;
}

int slsDetector::checkReceiverVersionCompatibility() {
    // TODO! Verify that this works as intended when version don't match
    int fnum = F_RECEIVER_CHECK_VERSION;
    int ret = FAIL;
    int64_t arg = APIRECEIVER;

    FILE_LOG(logDEBUG1) << "Checking version compatibility with receiver with value " << std::hex
                        << arg << std::dec;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        // in case it throws
        detector_shm()->receiverAPIVersion = 0;
        detector_shm()->receiverOnlineFlag = OFFLINE_FLAG;

        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);

        // success
        detector_shm()->receiverAPIVersion = arg;
        detector_shm()->receiverOnlineFlag = ONLINE_FLAG;
    }
    return ret;
}

int64_t slsDetector::getId(idMode mode) {
    int arg = static_cast<int>(mode);
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting id type " << mode;

    // These should not go to detector...
    assert(mode != THIS_SOFTWARE_VERSION);
    assert(mode != RECEIVER_VERSION);
    assert(mode != CLIENT_SOFTWARE_API_VERSION);
    assert(mode != CLIENT_RECEIVER_API_VERSION);

    int fnum = F_GET_ID;
    int ret = FAIL;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
    }
    FILE_LOG(logDEBUG1) << "Id (" << mode << "): 0x" << std::hex << retval << std::dec;
    if (ret == FORCE_UPDATE) {
        updateDetector();
    }
    return retval;
}

int64_t slsDetector::getReceiverSoftwareVersion() const {
    int ret = FAIL;
    int arg = RECEIVER_VERSION;
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting id type " << arg;
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        int fnum = F_GET_RECEIVER_ID;
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return retval;
}

void slsDetector::freeSharedMemory(int multi_id, int slsId) {
    SharedMemory<sharedSlsDetector> shm(multi_id, slsId);
    if (shm.IsExisting()) {
        shm.RemoveSharedMemory();
    }
}

void slsDetector::freeSharedMemory() {
    if (detector_shm.IsExisting()) {
        detector_shm.RemoveSharedMemory();
    }
}

void slsDetector::setHostname(const std::string &hostname) {
    sls::strcpy_safe(detector_shm()->hostname, hostname.c_str());
    updateDetector();
}

std::string slsDetector::getHostname() const { return detector_shm()->hostname; }

void slsDetector::initSharedMemory(detectorType type, int multi_id, bool verify) {

    detector_shm = SharedMemory<sharedSlsDetector>(multi_id, detId);
    if (!detector_shm.IsExisting()) {
        detector_shm.CreateSharedMemory();
        initializeDetectorStructure(type);
    } else {
        detector_shm.OpenSharedMemory();
        if (verify && detector_shm()->shmversion != SLS_SHMVERSION) {
            std::ostringstream ss;
            ss << "Single shared memory (" << multi_id << "-" << detId
               << ":) version mismatch (expected 0x" << std::hex << SLS_SHMVERSION << " but got 0x"
               << detector_shm()->shmversion << ")" << std::dec;
            throw SharedMemoryError(ss.str());
        }
    }
}

void slsDetector::setDetectorSpecificParameters(detectorType type, detParameterList &list) {
    switch (type) {
    case GOTTHARD:
        list.nChanX = 128;
        list.nChanY = 1;
        list.nChipX = 10;
        list.nChipY = 1;
        list.nDacs = 8;
        list.dynamicRange = 16;
        list.nGappixelsX = 0;
        list.nGappixelsY = 0;
        break;
    case JUNGFRAU:
        list.nChanX = 256;
        list.nChanY = 256;
        list.nChipX = 4;
        list.nChipY = 2;
        list.nDacs = 8;
        list.dynamicRange = 16;
        list.nGappixelsX = 0;
        list.nGappixelsY = 0;
        break;
    case CHIPTESTBOARD:
        list.nChanX = 36;
        list.nChanY = 1;
        list.nChipX = 1;
        list.nChipY = 1;
        list.nDacs = 24;
        list.dynamicRange = 16;
        list.nGappixelsX = 0;
        list.nGappixelsY = 0;
        break;
    case MOENCH:
        list.nChanX = 32;
        list.nChanY = 1;
        list.nChipX = 1;
        list.nChipY = 1;
        list.nDacs = 8;
        list.dynamicRange = 16;
        list.nGappixelsX = 0;
        list.nGappixelsY = 0;
        break;
    case EIGER:
        list.nChanX = 256;
        list.nChanY = 256;
        list.nChipX = 4;
        list.nChipY = 1;
        list.nDacs = 16;
        list.dynamicRange = 16;
        list.nGappixelsX = 6;
        list.nGappixelsY = 1;
        break;
    default:
        throw RuntimeError("Unknown detector type! " + slsDetectorDefs::detectorTypeToString(type));
    }
}

void slsDetector::initializeDetectorStructure(detectorType type) {
    detector_shm()->shmversion = SLS_SHMVERSION;
    detector_shm()->onlineFlag = OFFLINE_FLAG;
    detector_shm()->stoppedFlag = 0;
    sls::strcpy_safe(detector_shm()->hostname, DEFAULT_HOSTNAME);
    detector_shm()->myDetectorType = type;
    detector_shm()->offset[X] = 0;
    detector_shm()->offset[Y] = 0;
    detector_shm()->multiSize[X] = 0;
    detector_shm()->multiSize[Y] = 0;
    detector_shm()->controlPort = DEFAULT_PORTNO;
    detector_shm()->stopPort = DEFAULT_PORTNO + 1;
    sls::strcpy_safe(detector_shm()->settingsDir, getenv("HOME"));
    detector_shm()->nTrimEn = 0;
    for (int &trimEnergie : detector_shm()->trimEnergies) {
        trimEnergie = 0;
    }
    detector_shm()->nROI = 0;
    memset(detector_shm()->roiLimits, 0, MAX_ROIS * sizeof(ROI));
    detector_shm()->roFlags = NORMAL_READOUT;
    detector_shm()->currentSettings = UNINITIALIZED;
    detector_shm()->currentThresholdEV = -1;
    detector_shm()->timerValue[FRAME_NUMBER] = 1;
    detector_shm()->timerValue[ACQUISITION_TIME] = 0;
    detector_shm()->timerValue[FRAME_PERIOD] = 0;
    detector_shm()->timerValue[DELAY_AFTER_TRIGGER] = 0;
    detector_shm()->timerValue[GATES_NUMBER] = 0;
    detector_shm()->timerValue[CYCLES_NUMBER] = 1;
    detector_shm()->timerValue[ACTUAL_TIME] = 0;
    detector_shm()->timerValue[MEASUREMENT_TIME] = 0;
    detector_shm()->timerValue[PROGRESS] = 0;
    detector_shm()->timerValue[MEASUREMENTS_NUMBER] = 1;
    detector_shm()->timerValue[FRAMES_FROM_START] = 0;
    detector_shm()->timerValue[FRAMES_FROM_START_PG] = 0;
    detector_shm()->timerValue[SAMPLES] = 1;
    detector_shm()->timerValue[SUBFRAME_ACQUISITION_TIME] = 0;
    detector_shm()->timerValue[STORAGE_CELL_NUMBER] = 0;
    detector_shm()->timerValue[SUBFRAME_DEADTIME] = 0;
    sls::strcpy_safe(detector_shm()->receiver_hostname, "none");
    detector_shm()->receiverTCPPort = DEFAULT_PORTNO + 2;
    detector_shm()->receiverUDPPort = DEFAULT_UDP_PORTNO;
    detector_shm()->receiverUDPPort2 = DEFAULT_UDP_PORTNO + 1;

    detector_shm()->receiverUDPIP = 0u;
    detector_shm()->receiverUDPIP2 = 0u;
    detector_shm()->receiverUDPMAC = 0ul;
    detector_shm()->receiverUDPMAC2 = 0ul;

    detector_shm()->detectorMAC = DEFAULT_DET_MAC;
    detector_shm()->detectorMAC2 = DEFAULT_DET_MAC2;
    detector_shm()->detectorIP = DEFAULT_DET_MAC;
    detector_shm()->detectorIP2 = DEFAULT_DET_MAC2;

    detector_shm()->numUDPInterfaces = 1;
    detector_shm()->selectedUDPInterface = 1;
    detector_shm()->receiverOnlineFlag = OFFLINE_FLAG;
    detector_shm()->tenGigaEnable = 0;
    detector_shm()->flippedData[X] = 0;
    detector_shm()->flippedData[Y] = 0;
    detector_shm()->zmqport =
        DEFAULT_ZMQ_CL_PORTNO + (detId * ((detector_shm()->myDetectorType == EIGER) ? 2 : 1));
    detector_shm()->receiver_zmqport =
        DEFAULT_ZMQ_RX_PORTNO + (detId * ((detector_shm()->myDetectorType == EIGER) ? 2 : 1));
    detector_shm()->receiver_upstream = false;
    detector_shm()->receiver_read_freq = 0;
    memset(detector_shm()->zmqip, 0, MAX_STR_LENGTH);
    memset(detector_shm()->receiver_zmqip, 0, MAX_STR_LENGTH);
    detector_shm()->gappixels = 0;
    memset(detector_shm()->receiver_additionalJsonHeader, 0, MAX_STR_LENGTH);
    detector_shm()->detectorControlAPIVersion = 0;
    detector_shm()->detectorStopAPIVersion = 0;
    detector_shm()->receiverAPIVersion = 0;
    detector_shm()->receiver_frameDiscardMode = NO_DISCARD;
    detector_shm()->receiver_framePadding = true;
    detector_shm()->activated = true;
    detector_shm()->receiver_deactivatedPaddingEnable = true;
    detector_shm()->receiver_silentMode = false;
    sls::strcpy_safe(detector_shm()->receiver_filePath, "/");
    sls::strcpy_safe(detector_shm()->receiver_fileName, "run");
    detector_shm()->receiver_fileIndex = 0;
    detector_shm()->receiver_fileFormatType = BINARY;
    switch (detector_shm()->myDetectorType) {
    case GOTTHARD:
        detector_shm()->receiver_framesPerFile = MAX_FRAMES_PER_FILE;
        break;
    case EIGER:
        detector_shm()->receiver_framesPerFile = EIGER_MAX_FRAMES_PER_FILE;
        break;
    case JUNGFRAU:
        detector_shm()->receiver_framesPerFile = JFRAU_MAX_FRAMES_PER_FILE;
        break;
    case CHIPTESTBOARD:
        detector_shm()->receiver_framesPerFile = CTB_MAX_FRAMES_PER_FILE;
        break;
    case MOENCH:
        detector_shm()->receiver_framesPerFile = MOENCH_MAX_FRAMES_PER_FILE;
        break;
    default:
        break;
    }
    detector_shm()->receiver_fileWriteEnable = true;
    detector_shm()->receiver_overWriteEnable = true;

    // get the detector parameters based on type
    detParameterList detlist;
    setDetectorSpecificParameters(type, detlist);
    detector_shm()->nChan[X] = detlist.nChanX;
    detector_shm()->nChan[Y] = detlist.nChanY;
    detector_shm()->nChip[X] = detlist.nChipX;
    detector_shm()->nChip[Y] = detlist.nChipY;
    detector_shm()->nDacs = detlist.nDacs;
    detector_shm()->dynamicRange = detlist.dynamicRange;
    detector_shm()->nGappixels[X] = detlist.nGappixelsX;
    detector_shm()->nGappixels[Y] = detlist.nGappixelsY;

    // derived parameters
    detector_shm()->nChans = detector_shm()->nChan[X] * detector_shm()->nChan[Y];
    detector_shm()->nChips = detector_shm()->nChip[X] * detector_shm()->nChip[Y];

    // calculating databytes
    detector_shm()->dataBytes =
        detector_shm()->nChips * detector_shm()->nChans * detector_shm()->dynamicRange / 8;
    detector_shm()->dataBytesInclGapPixels =
        (detector_shm()->nChip[X] * detector_shm()->nChan[X] +
         detector_shm()->gappixels * detector_shm()->nGappixels[X]) *
        (detector_shm()->nChip[Y] * detector_shm()->nChan[Y] +
         detector_shm()->gappixels * detector_shm()->nGappixels[Y]) *
        detector_shm()->dynamicRange / 8;

    // update #nchans and databytes, as it depends on #samples, roi,
    // readoutflags (ctb only)
    if (detector_shm()->myDetectorType == CHIPTESTBOARD ||
        detector_shm()->myDetectorType == MOENCH) {
        updateTotalNumberOfChannels();
    }
}

slsDetectorDefs::sls_detector_module *slsDetector::createModule() {
    return createModule(detector_shm()->myDetectorType);
}

slsDetectorDefs::sls_detector_module *slsDetector::createModule(detectorType type) {
    // get the detector parameters based on type
    detParameterList detlist;
    int nch = 0, nc = 0, nd = 0;
    try {
        setDetectorSpecificParameters(type, detlist);
        nch = detlist.nChanX * detlist.nChanY;
        nc = detlist.nChipX * detlist.nChipY;
        nd = detlist.nDacs;
    } catch (...) {
        return nullptr;
    }
    int *dacs = new int[nd];
    int *chanregs = new int[nch * nc];

    auto *myMod = new sls_detector_module;
    myMod->ndac = nd;
    myMod->nchip = nc;
    myMod->nchan = nch * nc;
    myMod->dacs = dacs;
    myMod->chanregs = chanregs;
    return myMod;
}

void slsDetector::deleteModule(sls_detector_module *myMod) {
    if (myMod != nullptr) {
        delete[] myMod->dacs;
        delete[] myMod->chanregs;
        delete myMod;
    }
}

int slsDetector::sendModule(sls_detector_module *myMod, sls::ClientSocket &client) {
    TLogLevel level = logDEBUG1;
    FILE_LOG(level) << "Sending Module";
    int ts = 0;
    int n = 0;
    n = client.sendData(&(myMod->serialnumber), sizeof(myMod->serialnumber));
    ts += n;
    FILE_LOG(level) << "Serial number sent. " << n << " bytes. serialno: " << myMod->serialnumber;

    n = client.sendData(&(myMod->nchan), sizeof(myMod->nchan));
    ts += n;
    FILE_LOG(level) << "nchan sent. " << n << " bytes. serialno: " << myMod->nchan;

    n = client.sendData(&(myMod->nchip), sizeof(myMod->nchip));
    ts += n;
    FILE_LOG(level) << "nchip sent. " << n << " bytes. serialno: " << myMod->nchip;

    n = client.sendData(&(myMod->ndac), sizeof(myMod->ndac));
    ts += n;
    FILE_LOG(level) << "ndac sent. " << n << " bytes. serialno: " << myMod->ndac;

    n = client.sendData(&(myMod->reg), sizeof(myMod->reg));
    ts += n;
    FILE_LOG(level) << "reg sent. " << n << " bytes. serialno: " << myMod->reg;

    n = client.sendData(&(myMod->iodelay), sizeof(myMod->iodelay));
    ts += n;
    FILE_LOG(level) << "iodelay sent. " << n << " bytes. serialno: " << myMod->iodelay;

    n = client.sendData(&(myMod->tau), sizeof(myMod->tau));
    ts += n;
    FILE_LOG(level) << "tau sent. " << n << " bytes. serialno: " << myMod->tau;

    n = client.sendData(&(myMod->eV), sizeof(myMod->eV));
    ts += n;
    FILE_LOG(level) << "ev sent. " << n << " bytes. serialno: " << myMod->eV;

    n = client.sendData(myMod->dacs, sizeof(int) * (myMod->ndac));
    ts += n;
    FILE_LOG(level) << "dacs sent. " << n << " bytes";

    if (detector_shm()->myDetectorType == EIGER) {
        n = client.sendData(myMod->chanregs, sizeof(int) * (myMod->nchan));
        ts += n;
        FILE_LOG(level) << "channels sent. " << n << " bytes";
    }
    return ts;
}

int slsDetector::receiveModule(sls_detector_module *myMod, sls::ClientSocket &client) {
    int ts = 0;
    ts += client.receiveData(&(myMod->serialnumber), sizeof(myMod->serialnumber));
    ts += client.receiveData(&(myMod->nchan), sizeof(myMod->nchan));
    ts += client.receiveData(&(myMod->nchip), sizeof(myMod->nchip));
    ts += client.receiveData(&(myMod->ndac), sizeof(myMod->ndac));
    ts += client.receiveData(&(myMod->reg), sizeof(myMod->reg));
    ts += client.receiveData(&(myMod->iodelay), sizeof(myMod->iodelay));
    ts += client.receiveData(&(myMod->tau), sizeof(myMod->tau));
    ts += client.receiveData(&(myMod->eV), sizeof(myMod->eV));

    ts += client.receiveData(myMod->dacs, sizeof(int) * (myMod->ndac));
    FILE_LOG(logDEBUG1) << "received dacs of size " << ts;
    if (detector_shm()->myDetectorType == EIGER) {
        ts += client.receiveData(myMod->chanregs, sizeof(int) * (myMod->nchan));
        FILE_LOG(logDEBUG1) << "nchans= " << detector_shm()->nChans
                            << " nchips= " << detector_shm()->nChips
                            << "mod - nchans= " << myMod->nchan << " nchips= " << myMod->nchip
                            << "received chans of size " << ts;
    }
    FILE_LOG(logDEBUG1) << "received module of size " << ts << " register " << myMod->reg;
    return ts;
}

slsDetectorDefs::detectorType slsDetector::getDetectorTypeFromShm(int multi_id, bool verify) {
    if (!detector_shm.IsExisting()) {
        throw SharedMemoryError("Shared memory " + detector_shm.GetName() +
                                "does not exist.\n Corrupted Multi Shared "
                                "memory. Please free shared memory.");
    }

    detector_shm.OpenSharedMemory();
    if (verify && detector_shm()->shmversion != SLS_SHMVERSION) {
        std::ostringstream ss;
        ss << "Single shared memory (" << multi_id << "-" << detId
           << ":)version mismatch (expected 0x" << std::hex << SLS_SHMVERSION << " but got 0x"
           << detector_shm()->shmversion << ")" << std::dec;
        detector_shm.UnmapSharedMemory();
        throw SharedMemoryError(ss.str());
    }
    auto type = detector_shm()->myDetectorType;
    return type;
}

// static function
slsDetectorDefs::detectorType slsDetector::getTypeFromDetector(const std::string &hostname,
                                                               int cport) {
    int fnum = F_GET_DETECTOR_TYPE;
    int ret = FAIL;
    detectorType retval = GENERIC;
    FILE_LOG(logDEBUG1) << "Getting detector type ";
    sls::ClientSocket cs(false, hostname, cport);
    cs.sendData(reinterpret_cast<char *>(&fnum), sizeof(fnum));
    cs.receiveData(reinterpret_cast<char *>(&ret), sizeof(ret));
    cs.receiveData(reinterpret_cast<char *>(&retval), sizeof(retval));
    FILE_LOG(logDEBUG1) << "Detector type is " << retval;
    return retval;
}

int slsDetector::setDetectorType(detectorType const type) {
    int fnum = F_GET_DETECTOR_TYPE;
    int ret = FAIL;
    detectorType retval = GENERIC;
    FILE_LOG(logDEBUG1) << "Setting detector type to " << type;

    // if unspecified, then get from detector
    if (type == GET_DETECTOR_TYPE) {
        if (detector_shm()->onlineFlag == ONLINE_FLAG) {
            auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
            ret = client.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
            detector_shm()->myDetectorType = static_cast<detectorType>(retval);
            FILE_LOG(logDEBUG1) << "Detector Type: " << retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateDetector();
        }
    } else {
        ret = OK;
    }

    // receiver
    if ((detector_shm()->receiverOnlineFlag == ONLINE_FLAG) && ret == OK) {
        fnum = F_GET_RECEIVER_TYPE;
        ret = FAIL;
        int arg = (int)detector_shm()->myDetectorType;
        retval = GENERIC;
        FILE_LOG(logDEBUG1) << "Sending detector type to Receiver: "
                            << (int)detector_shm()->myDetectorType;
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver Type: " << retval;
        if (ret == FORCE_UPDATE) {
            receiver.close(); // TODO! Should find a better solution
            ret = updateCachedReceiverVariables();
        }
    }
    return retval;
}

slsDetectorDefs::detectorType slsDetector::getDetectorTypeAsEnum() const {
    return detector_shm()->myDetectorType;
}

std::string slsDetector::getDetectorTypeAsString() const {
    return slsDetectorDefs::detectorTypeToString(getDetectorTypeAsEnum());
}

int slsDetector::getTotalNumberOfChannels() const {
    return detector_shm()->nChans * detector_shm()->nChips;
}

void slsDetector::updateTotalNumberOfChannels() {
    if (detector_shm()->myDetectorType == CHIPTESTBOARD ||
        detector_shm()->myDetectorType == MOENCH) {

        // default number of channels
        detector_shm()->nChan[X] = 32;

        // if roi, recalculate #nchanX
        if (detector_shm()->nROI > 0) {
            detector_shm()->nChan[X] = 0;
            for (int iroi = 0; iroi < detector_shm()->nROI; ++iroi) {
                detector_shm()->nChan[X] += (detector_shm()->roiLimits[iroi].xmax -
                                             detector_shm()->roiLimits[iroi].xmin + 1);
            }
        }

        // add digital signals depending on readout flags
        if (detector_shm()->myDetectorType == CHIPTESTBOARD &&
            (detector_shm()->roFlags & DIGITAL_ONLY ||
             detector_shm()->roFlags & ANALOG_AND_DIGITAL)) {
            detector_shm()->nChan[X] += 4;
        }

        // recalculate derived parameters chans and databytes
        detector_shm()->nChans = detector_shm()->nChan[X];
        detector_shm()->dataBytes = detector_shm()->nChans * detector_shm()->nChips *
                                    (detector_shm()->dynamicRange / 8) *
                                    detector_shm()->timerValue[SAMPLES];
        FILE_LOG(logDEBUG1) << "Number of Channels:" << detector_shm()->nChans
                            << " Databytes: " << detector_shm()->dataBytes;
    }
}

int slsDetector::getTotalNumberOfChannels(dimension d) const {
    return detector_shm()->nChan[d] * detector_shm()->nChip[d];
}

int slsDetector::getTotalNumberOfChannelsInclGapPixels(dimension d) const {
    return (detector_shm()->nChan[d] * detector_shm()->nChip[d] +
            detector_shm()->gappixels * detector_shm()->nGappixels[d]);
}

int slsDetector::getNChans() const { return detector_shm()->nChans; }

int slsDetector::getNChans(dimension d) const { return detector_shm()->nChan[d]; }

int slsDetector::getNChips() const { return detector_shm()->nChips; }

int slsDetector::getNChips(dimension d) const { return detector_shm()->nChip[d]; }

int slsDetector::getDetectorOffset(dimension d) const { return detector_shm()->offset[d]; }

void slsDetector::setDetectorOffset(dimension d, int off) {
    if (off >= 0) {
        detector_shm()->offset[d] = off;
    }
}

void slsDetector::updateMultiSize(int detx, int dety) {
    detector_shm()->multiSize[0] = detx;
    detector_shm()->multiSize[1] = dety;
}

int slsDetector::setOnline(int value) {
    if (value != GET_ONLINE_FLAG) {
        int old_flag = detector_shm()->onlineFlag;
        detector_shm()->onlineFlag = OFFLINE_FLAG;

        if (value == ONLINE_FLAG) {

            auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
            client.close();
            detector_shm()->onlineFlag = ONLINE_FLAG;

            if (old_flag == OFFLINE_FLAG) {

                // check version compatibility (first time)
                if ((detector_shm()->detectorControlAPIVersion == 0) ||
                    (detector_shm()->detectorStopAPIVersion == 0)) {
                    checkDetectorVersionCompatibility();
                }

                FILE_LOG(logINFO) << "Detector connecting - updating!";
                updateDetector();
            }
        }
    }
    return detector_shm()->onlineFlag;
}

int slsDetector::getOnlineFlag() const { return detector_shm()->onlineFlag; }

std::string slsDetector::checkOnline() {
    std::string retval;
    try {
        // Need both control and stop socket to work!
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        detector_shm()->onlineFlag = ONLINE_FLAG;
    } catch (...) {
        detector_shm()->onlineFlag = OFFLINE_FLAG;
        retval = detector_shm()->hostname;
    }
    return retval;
}

int slsDetector::setControlPort(int port_number) {
    int fnum = F_SET_PORT;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting control port to " << port_number;

    if (port_number >= 0 && port_number != detector_shm()->controlPort) {
        if (detector_shm()->onlineFlag == ONLINE_FLAG) {
            auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
            ret = client.sendCommandThenRead(fnum, &port_number, sizeof(port_number), &retval,
                                             sizeof(retval));
            detector_shm()->controlPort = retval;
            FILE_LOG(logDEBUG1) << "Control port: " << retval;
        } else {
            detector_shm()->controlPort = port_number;
        }
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return detector_shm()->controlPort;
}

int slsDetector::setStopPort(int port_number) {
    int fnum = F_SET_PORT;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting stop port "
                        << " to " << port_number;

    if (port_number >= 0 && port_number != detector_shm()->stopPort) {
        if (detector_shm()->onlineFlag == ONLINE_FLAG) {
            auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
            ret = stop.sendCommandThenRead(fnum, &port_number, sizeof(port_number), &retval,
                                           sizeof(retval));
            detector_shm()->stopPort = retval;
            FILE_LOG(logDEBUG1) << "Stop port: " << retval;
        } else {
            detector_shm()->stopPort = port_number;
        }
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return detector_shm()->stopPort;
}

int slsDetector::setReceiverPort(int port_number) {
    int fnum = F_SET_RECEIVER_PORT;
    int ret = FAIL;
    int retval = -1;

    FILE_LOG(logDEBUG1) << "Setting reciever port to " << port_number;

    if (port_number >= 0 && port_number != detector_shm()->receiverTCPPort) {
        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto stop =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = stop.sendCommandThenRead(fnum, &port_number, sizeof(port_number), &retval,
                                           sizeof(retval));
            detector_shm()->receiverTCPPort = retval;
            FILE_LOG(logDEBUG1) << "Receiver port: " << retval;

        } else {
            detector_shm()->receiverTCPPort = port_number;
        }
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return detector_shm()->receiverTCPPort;
}

int slsDetector::getReceiverPort() const { return detector_shm()->receiverTCPPort; }

int slsDetector::getControlPort() const { return detector_shm()->controlPort; }

int slsDetector::getStopPort() const { return detector_shm()->stopPort; }

int slsDetector::lockServer(int lock) {
    int fnum = F_LOCK_SERVER;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting detector server lock to " << lock;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &lock, sizeof(lock), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Lock: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

std::string slsDetector::getLastClientIP() {
    int fnum = F_GET_LAST_CLIENT_IP;
    int ret = FAIL;
    char retval[INET_ADDRSTRLEN] = {0};
    FILE_LOG(logDEBUG1) << "Getting last client ip to detector server";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Last client IP to detector: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return std::string(retval);
}

int slsDetector::exitServer() {
    int fnum = F_EXIT_SERVER;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Sending exit command to detector server";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        FILE_LOG(logINFO) << "Shutting down the Detector server";
    }
    return ret;
}

int slsDetector::execCommand(const std::string &cmd) {
    int fnum = F_EXEC_COMMAND;
    int ret = FAIL;
    char arg[MAX_STR_LENGTH] = {};
    char retval[MAX_STR_LENGTH] = {};
    sls::strcpy_safe(arg, cmd.c_str());
    FILE_LOG(logDEBUG1) << "Sending command to detector " << arg;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, arg, sizeof(arg), retval, sizeof(retval));
        FILE_LOG(logINFO) << "Detector " << detId << " returned:\n" << retval;
    }
    return ret;
}

int slsDetector::updateDetectorNoWait(sls::ClientSocket &client) {
    int n = 0, i32 = 0;
    int64_t i64 = 0;
    char lastClientIP[INET_ADDRSTRLEN] = {0};
    n += client.receiveData(lastClientIP, sizeof(lastClientIP));
    FILE_LOG(logDEBUG1) << "Updating detector last modified by " << lastClientIP;

    // dr
    n += client.receiveData(&i32, sizeof(i32));
    detector_shm()->dynamicRange = i32;

    // databytes
    n += client.receiveData(&i32, sizeof(i32));
    detector_shm()->dataBytes = i32;

    // settings
    if ((detector_shm()->myDetectorType != CHIPTESTBOARD) &&
        (detector_shm()->myDetectorType != MOENCH)) {
        n += client.receiveData(&i32, sizeof(i32));
        detector_shm()->currentSettings = (detectorSettings)i32;
    }

    // threshold
    if (detector_shm()->myDetectorType == EIGER) {
        n += client.receiveData(&i32, sizeof(i32));
        detector_shm()->currentThresholdEV = i32;
    }

    // frame number
    n += client.receiveData(&i64, sizeof(i64));
    detector_shm()->timerValue[FRAME_NUMBER] = i64;

    // exptime
    n += client.receiveData(&i64, sizeof(i64));
    detector_shm()->timerValue[ACQUISITION_TIME] = i64;

    // subexptime, subdeadtime
    if (detector_shm()->myDetectorType == EIGER) {
        n += client.receiveData(&i64, sizeof(i64));
        detector_shm()->timerValue[SUBFRAME_ACQUISITION_TIME] = i64;

        n += client.receiveData(&i64, sizeof(i64));
        detector_shm()->timerValue[SUBFRAME_DEADTIME] = i64;
    }

    // period
    n += client.receiveData(&i64, sizeof(i64));
    detector_shm()->timerValue[FRAME_PERIOD] = i64;

    // delay
    if (detector_shm()->myDetectorType != EIGER) {
        n += client.receiveData(&i64, sizeof(i64));
        detector_shm()->timerValue[DELAY_AFTER_TRIGGER] = i64;
    }

    if (detector_shm()->myDetectorType == JUNGFRAU) {
        // storage cell
        n += client.receiveData(&i64, sizeof(i64));
        detector_shm()->timerValue[STORAGE_CELL_NUMBER] = i64;

        // storage cell delay
        n += client.receiveData(&i64, sizeof(i64));
        detector_shm()->timerValue[STORAGE_CELL_DELAY] = i64;
    }

    // cycles
    n += client.receiveData(&i64, sizeof(i64));
    detector_shm()->timerValue[CYCLES_NUMBER] = i64;

    // readout flags
    if (detector_shm()->myDetectorType == EIGER ||
        detector_shm()->myDetectorType == CHIPTESTBOARD) {
        n += client.receiveData(&i32, sizeof(i32));
        detector_shm()->roFlags = (readOutFlags)i32;
    }

    // samples
    if (detector_shm()->myDetectorType == CHIPTESTBOARD ||
        detector_shm()->myDetectorType == MOENCH) {
        n += client.receiveData(&i64, sizeof(i64));
        if (i64 >= 0) {
            detector_shm()->timerValue[SAMPLES] = i64;
        }
    }

    // roi
    if (detector_shm()->myDetectorType == CHIPTESTBOARD ||
        detector_shm()->myDetectorType == MOENCH || detector_shm()->myDetectorType == GOTTHARD) {
        n += client.receiveData(&i32, sizeof(i32));
        detector_shm()->nROI = i32;
        for (int i = 0; i < detector_shm()->nROI; ++i) {
            n += client.receiveData(&i32, sizeof(i32));
            detector_shm()->roiLimits[i].xmin = i32;
            n += client.receiveData(&i32, sizeof(i32));
            detector_shm()->roiLimits[i].xmax = i32;
            n += client.receiveData(&i32, sizeof(i32));
            detector_shm()->roiLimits[i].ymin = i32;
            n += client.receiveData(&i32, sizeof(i32));
            detector_shm()->roiLimits[i].xmax = i32;
        }
        // moench (send to processor)
        if (detector_shm()->myDetectorType == MOENCH) {
            sendROIToProcessor();
        }
    }

    // update #nchans and databytes, as it depends on #samples, roi,
    // readoutflags (ctb only)
    if (detector_shm()->myDetectorType == CHIPTESTBOARD ||
        detector_shm()->myDetectorType == MOENCH) {
        updateTotalNumberOfChannels();
    }

    if (!n) {
        FILE_LOG(logERROR) << "Could not update detector, received 0 bytes";
    }

    return OK;
}

int slsDetector::updateDetector() {
    int fnum = F_UPDATE_CLIENT;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Sending update client to detector server";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        // if it returns ok (jungfrau in programming mode), dont update
        if (ret == FORCE_UPDATE) {
            ret = updateDetectorNoWait(client);
        }
    }
    return ret;
}

int slsDetector::writeConfigurationFile(const std::string &fname, multiSlsDetector *m) {
    int iline = 0;
    std::ofstream outfile;
    outfile.open(fname.c_str(), std::ios_base::out);
    if (outfile.is_open()) {
        iline = writeConfigurationFile(outfile, m);
        outfile.close();
    } else {
        throw RuntimeError("Could not open configuration file for writing");
    }
    FILE_LOG(logINFO) << iline << " lines written to configuration file";
    return OK;
}

int slsDetector::writeConfigurationFile(std::ofstream &outfile, multiSlsDetector *m) {

    FILE_LOG(logDEBUG1) << "Write configuration file";

    std::vector<std::string> names;
    // common config
    names.emplace_back("hostname");
    names.emplace_back("port");
    names.emplace_back("stopport");
    names.emplace_back("settingsdir");
    names.emplace_back("ffdir");
    names.emplace_back("outdir");
    names.emplace_back("lock");
    // receiver config
    names.emplace_back("zmqport");
    names.emplace_back("rx_zmqport");
    names.emplace_back("zmqip");
    names.emplace_back("rx_zmqip");
    names.emplace_back("rx_tcpport");

    // detector specific config
    switch (detector_shm()->myDetectorType) {
    case GOTTHARD:
        names.emplace_back("detectormac");
        names.emplace_back("detectorip");
        names.emplace_back("rx_udpport");
        names.emplace_back("rx_udpip");
        names.emplace_back("rx_udpmac");
        names.emplace_back("rx_hostname");

        names.emplace_back("extsig:0");
        names.emplace_back("vhighvoltage");
        break;
    case EIGER:
        names.emplace_back("detectormac");
        names.emplace_back("detectorip");
        names.emplace_back("rx_udpport");
        names.emplace_back("rx_udpport2");
        names.emplace_back("rx_udpip");
        names.emplace_back("rx_udpmac");
        names.emplace_back("rx_hostname");

        names.emplace_back("vhighvoltage");
        names.emplace_back("trimen");
        names.emplace_back("iodelay");
        names.emplace_back("tengiga");
        break;
    case JUNGFRAU:
        names.emplace_back("detectormac");
        names.emplace_back("detectormac2");
        names.emplace_back("detectorip");
        names.emplace_back("detectorip2");
        names.emplace_back("rx_udpport");
        names.emplace_back("rx_udpport2");
        names.emplace_back("rx_udpip");
        names.emplace_back("rx_udpip2");
        names.emplace_back("rx_udpmac");
        names.emplace_back("rx_udpmac2");
        names.emplace_back("rx_hostname");

        names.emplace_back("powerchip");
        names.emplace_back("vhighvoltage");
        break;
    case CHIPTESTBOARD:
        names.emplace_back("detectormac");
        names.emplace_back("detectorip");
        names.emplace_back("rx_udpport");
        names.emplace_back("rx_udpip");
        names.emplace_back("rx_udpmac");
        names.emplace_back("rx_hostname");

        names.emplace_back("vhighvoltage");
        break;
    case MOENCH:
        names.emplace_back("detectormac");
        names.emplace_back("detectorip");
        names.emplace_back("rx_udpport");
        names.emplace_back("rx_udpip");
        names.emplace_back("rx_udpmac");
        names.emplace_back("rx_hostname");

        names.emplace_back("powerchip");
        names.emplace_back("vhighvoltage");
        break;
    default:
        throw RuntimeError("Write configuration file called with unknown detector: " +
                           std::to_string(detector_shm()->myDetectorType));
    }

    names.emplace_back("r_readfreq");
    names.emplace_back("rx_udpsocksize");
    names.emplace_back("rx_realudpsocksize");

    auto cmd = slsDetectorCommand(m);
    for (auto &name : names) {
        char *args[] = {(char *)name.c_str()};
        outfile << detId << ":";
        outfile << name << " " << cmd.executeLine(1, args, GET_ACTION) << std::endl;
    }
    return OK;
}

slsDetectorDefs::detectorSettings slsDetector::getSettings() {
    return sendSettingsOnly(GET_SETTINGS);
}

slsDetectorDefs::detectorSettings slsDetector::setSettings(detectorSettings isettings) {
    FILE_LOG(logDEBUG1) << "slsDetector setSettings " << isettings;

    if (isettings == -1) {
        return getSettings();
    }

    // eiger: only set shm, setting threshold loads the module data
    if (detector_shm()->myDetectorType == EIGER) {
        switch (isettings) {
        case STANDARD:
        case HIGHGAIN:
        case LOWGAIN:
        case VERYHIGHGAIN:
        case VERYLOWGAIN:
            detector_shm()->currentSettings = isettings;
            return detector_shm()->currentSettings;
        default:
            std::ostringstream ss;
            ss << "Unknown settings " << getDetectorSettings(isettings) << " for this detector!";
            throw RuntimeError(ss.str());
        }
    }

    // others: send only the settings, detector server will update dac values
    // already in server
    return sendSettingsOnly(isettings);
}

slsDetectorDefs::detectorSettings slsDetector::sendSettingsOnly(detectorSettings isettings) {
    int fnum = F_SET_SETTINGS;
    int ret = FAIL;
    int arg = (int)isettings;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting settings to " << arg;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Settings: " << retval;
        detector_shm()->currentSettings = (detectorSettings)retval;
    }
    if (ret == FORCE_UPDATE) {
        updateDetector();
    }
    return detector_shm()->currentSettings;
}

int slsDetector::getThresholdEnergy() {
    // moench - get threshold energy from processor (due to different clients,
    // diff shm)
    if (detector_shm()->myDetectorType == MOENCH) {
        // get json from rxr, parse for threshold and update shm
        getAdditionalJsonHeader();
        std::string result = getAdditionalJsonParameter("threshold");
        // convert to integer
        try {
            // udpate shm
            detector_shm()->currentThresholdEV = stoi(result);
            return detector_shm()->currentThresholdEV;
        }
        // not found or cannot scan integer
        catch (...) {
            return -1;
        }
    }

    int fnum = F_GET_THRESHOLD_ENERGY;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting threshold energy";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Threshold: " << retval;
        detector_shm()->currentThresholdEV = retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return detector_shm()->currentThresholdEV;
}

int slsDetector::setThresholdEnergy(int e_eV, detectorSettings isettings, int tb) {

    // check as there is client processing
    if (detector_shm()->myDetectorType == EIGER) {
        setThresholdEnergyAndSettings(e_eV, isettings, tb);
        return detector_shm()->currentThresholdEV;
    }

    // moench - send threshold energy to processor
    else if (detector_shm()->myDetectorType == MOENCH) {
        std::string result = setAdditionalJsonParameter("threshold", std::to_string(e_eV));
        if (result == std::to_string(e_eV)) {
            // update shm
            detector_shm()->currentThresholdEV = e_eV;
            return detector_shm()->currentThresholdEV;
        }
        return -1;
    }
    throw RuntimeError("Set threshold energy not implemented for this detector");
}

int slsDetector::setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings, int tb) {

    // if settings provided, use that, else use the shared memory variable
    detectorSettings is =
        ((isettings != GET_SETTINGS) ? isettings : detector_shm()->currentSettings);
    std::string ssettings;
    switch (is) {
    case STANDARD:
        ssettings = "/standard";
        detector_shm()->currentSettings = STANDARD;
        break;
    case HIGHGAIN:
        ssettings = "/highgain";
        detector_shm()->currentSettings = HIGHGAIN;
        break;
    case LOWGAIN:
        ssettings = "/lowgain";
        detector_shm()->currentSettings = LOWGAIN;
        break;
    case VERYHIGHGAIN:
        ssettings = "/veryhighgain";
        detector_shm()->currentSettings = VERYHIGHGAIN;
        break;
    case VERYLOWGAIN:
        ssettings = "/verylowgain";
        detector_shm()->currentSettings = VERYLOWGAIN;
        break;
    default:
        std::ostringstream ss;
        ss << "Unknown settings " << getDetectorSettings(is) << " for this detector!";
        throw RuntimeError(ss.str());
    }

    // verify e_eV exists in trimEneregies[]
    if (!detector_shm()->nTrimEn || (e_eV < detector_shm()->trimEnergies[0]) ||
        (e_eV > detector_shm()->trimEnergies[detector_shm()->nTrimEn - 1])) {
        throw RuntimeError("This energy " + std::to_string(e_eV) + " not defined for this module!");
    }

    // find if interpolation required
    bool interpolate = true;
    for (int i = 0; i < detector_shm()->nTrimEn; ++i) {
        if (detector_shm()->trimEnergies[i] == e_eV) {
            interpolate = false;
            break;
        }
    }

    // fill detector module structure
    sls_detector_module *myMod = nullptr;

    // normal
    if (!interpolate) {
        // find their directory names
        std::ostringstream ostfn;
        ostfn << detector_shm()->settingsDir << ssettings << "/" << e_eV << "eV"
              << "/noise.sn" << std::setfill('0') << std::setw(3) << std::dec
              << getId(DETECTOR_SERIAL_NUMBER) << std::setbase(10);
        std::string settingsfname = ostfn.str();
        FILE_LOG(logDEBUG1) << "Settings File is " << settingsfname;

        // read the files
        // myMod = createModule(); // readSettings also checks if create module
        // is null
        if (nullptr == readSettingsFile(settingsfname, myMod, tb)) {
            if (myMod) {
                deleteModule(myMod);
            }
            return FAIL;
        }
    }

    // interpolate
    else {
        // find the trim values
        int trim1 = -1, trim2 = -1;
        for (int i = 0; i < detector_shm()->nTrimEn; ++i) {
            if (e_eV < detector_shm()->trimEnergies[i]) {
                trim2 = detector_shm()->trimEnergies[i];
                trim1 = detector_shm()->trimEnergies[i - 1];
                break;
            }
        }
        // find their directory names
        std::ostringstream ostfn;
        ostfn << detector_shm()->settingsDir << ssettings << "/" << trim1 << "eV"
              << "/noise.sn" << std::setfill('0') << std::setw(3) << std::dec
              << getId(DETECTOR_SERIAL_NUMBER) << std::setbase(10);
        std::string settingsfname1 = ostfn.str();
        ostfn.str("");
        ostfn.clear();
        ostfn << detector_shm()->settingsDir << ssettings << "/" << trim2 << "eV"
              << "/noise.sn" << std::setfill('0') << std::setw(3) << std::dec
              << getId(DETECTOR_SERIAL_NUMBER) << std::setbase(10);
        std::string settingsfname2 = ostfn.str();
        // read the files
        FILE_LOG(logDEBUG1) << "Settings Files are " << settingsfname1 << " and " << settingsfname2;
        sls_detector_module *myMod1 = createModule();
        sls_detector_module *myMod2 = createModule();
        if (nullptr == readSettingsFile(settingsfname1, myMod1, tb)) {

            deleteModule(myMod1);
            deleteModule(myMod2);
            throw RuntimeError("setThresholdEnergyAndSettings: Could not open settings file");
        }
        if (nullptr == readSettingsFile(settingsfname2, myMod2, tb)) {
            deleteModule(myMod1);
            deleteModule(myMod2);
            throw RuntimeError("setThresholdEnergyAndSettings: Could not open settings file");
        }
        if (myMod1->iodelay != myMod2->iodelay) {
            deleteModule(myMod1);
            deleteModule(myMod2);
            throw RuntimeError("setThresholdEnergyAndSettings: Iodelays do not "
                               "match between files");
        }

        // interpolate  module
        myMod = interpolateTrim(myMod1, myMod2, e_eV, trim1, trim2, tb);
        if (myMod == nullptr) {
            deleteModule(myMod1);
            deleteModule(myMod2);
            throw RuntimeError("setThresholdEnergyAndSettings: Could not "
                               "interpolate, different "
                               "dac values in files");
        }
        // interpolate tau
        myMod->iodelay = myMod1->iodelay;
        myMod->tau = linearInterpolation(e_eV, trim1, trim2, myMod1->tau, myMod2->tau);
        // printf("new tau:%d\n",tau);

        deleteModule(myMod1);
        deleteModule(myMod2);
    }

    myMod->reg = detector_shm()->currentSettings;
    myMod->eV = e_eV;
    setModule(*myMod, tb);
    deleteModule(myMod);
    if (getSettings() != is) {
        throw RuntimeError("setThresholdEnergyAndSettings: Could not set "
                           "settings in detector");
    }
    return OK;
}

std::string slsDetector::getSettingsDir() { return std::string(detector_shm()->settingsDir); }

std::string slsDetector::setSettingsDir(const std::string &dir) {
    sls::strcpy_safe(detector_shm()->settingsDir, dir.c_str());
    return detector_shm()->settingsDir;
}

int slsDetector::loadSettingsFile(const std::string &fname) {
    std::string fn = fname;
    std::ostringstream ostfn;
    ostfn << fname;

    // find specific file if it has detid in file name (.snxxx)
    if (detector_shm()->myDetectorType == EIGER) {
        if (fname.find(".sn") == std::string::npos && fname.find(".trim") == std::string::npos &&
            fname.find(".settings") == std::string::npos) {
            ostfn << ".sn" << std::setfill('0') << std::setw(3) << std::dec
                  << getId(DETECTOR_SERIAL_NUMBER);
        }
    }
    fn = ostfn.str();

    // read settings file
    sls_detector_module *myMod = nullptr;
    myMod = readSettingsFile(fn, myMod);

    // set module
    int ret = FAIL;
    if (myMod != nullptr) {
        myMod->reg = -1;
        myMod->eV = -1;
        ret = setModule(*myMod);
        deleteModule(myMod);
    }
    return ret;
}

int slsDetector::saveSettingsFile(const std::string &fname) {
    std::string fn = fname;
    std::ostringstream ostfn;
    ostfn << fname;

    // find specific file if it has detid in file name (.snxxx)
    if (detector_shm()->myDetectorType == EIGER) {
        ostfn << ".sn" << std::setfill('0') << std::setw(3) << std::dec
              << getId(DETECTOR_SERIAL_NUMBER);
    }
    fn = ostfn.str();

    // get module
    int ret = FAIL;
    sls_detector_module *myMod = getModule();
    if (myMod != nullptr) {
        ret = writeSettingsFile(fn, *myMod);
        deleteModule(myMod);
    }
    return ret;
}

slsDetectorDefs::runStatus slsDetector::getRunStatus() {
    int fnum = F_GET_RUN_STATUS;
    int ret = FAIL;
    runStatus retval = ERROR;
    FILE_LOG(logDEBUG1) << "Getting status";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        ret = stop.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Detector status: " << runStatusType(retval);
        if (ret == FORCE_UPDATE) {
            updateDetector();
        }
    }
    return retval;
}

int slsDetector::prepareAcquisition() {
    int fnum = F_PREPARE_ACQUISITION;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Preparing Detector for Acquisition";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        FILE_LOG(logDEBUG1) << "Prepare Acquisition successful";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::startAcquisition() {
    int fnum = F_START_ACQUISITION;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Starting Acquisition";

    detector_shm()->stoppedFlag = 0;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        FILE_LOG(logDEBUG1) << "Starting Acquisition successful";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::stopAcquisition() {
    // get status before stopping acquisition
    runStatus s = ERROR, r = ERROR;
    if (detector_shm()->receiver_upstream) {
        s = getRunStatus();
        r = getReceiverStatus();
    }
    int fnum = F_STOP_ACQUISITION;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Stopping Acquisition";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        ret = stop.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        FILE_LOG(logDEBUG1) << "Stopping Acquisition successful";
        if (ret == FORCE_UPDATE) {
            updateDetector();
        }
    }
    detector_shm()->stoppedFlag = 1;
    // if rxr streaming and acquisition finished, restream dummy stop packet
    if ((detector_shm()->receiver_upstream) && (s == IDLE) && (r == IDLE)) {
        restreamStopFromReceiver();
    }
    return ret;
}

int slsDetector::sendSoftwareTrigger() {
    int fnum = F_SOFTWARE_TRIGGER;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Sending software trigger";

    detector_shm()->stoppedFlag = 0;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        FILE_LOG(logDEBUG1) << "Sending software trigger successful";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::startAndReadAll() {
    int fnum = F_START_AND_READ_ALL;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Starting and reading all frames";

    detector_shm()->stoppedFlag = 0;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        // TODO! how to we hande this? ret == FAIL -->
        // detector_shm()->stoppedFlag = 1;
        FILE_LOG(logDEBUG1) << "Detector successfully finished acquisition";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::startReadOut() {
    int fnum = F_START_READOUT;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Starting readout";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        FILE_LOG(logDEBUG1) << "Starting detector readout successful";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::readAll() {
    int fnum = F_READ_ALL;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Reading all frames";
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        // TODO! how to we hande this? ret == FAIL -->
        // detector_shm()->stoppedFlag = 1;
        FILE_LOG(logDEBUG1) << "Detector successfully finished reading all frames";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::configureMAC() {
    int fnum = F_CONFIGURE_MAC;
    int ret = FAIL;
    const size_t array_size = 50;
    const size_t n_args = 14;
    const size_t n_retvals = 2;
    char args[n_args][array_size]{};
    char retvals[n_retvals][array_size]{};
    FILE_LOG(logDEBUG1) << "Configuring MAC";
    if (detector_shm()->receiverUDPIP == 0) {
        // If hostname is valid ip use that, oterwise lookup hostname
        detector_shm()->receiverUDPIP = IpStringToUint(detector_shm()->receiver_hostname);
        if (detector_shm()->receiverUDPIP == 0) {
            detector_shm()->receiverUDPIP = HostnameToIp(detector_shm()->receiver_hostname);
        }
    }

    if (detector_shm()->receiverUDPMAC == 0) {
        throw RuntimeError("configureMAC: Error. Receiver UDP MAC Addresses not set");
    }
    FILE_LOG(logDEBUG1) << "rx_hostname and rx_udpmac are valid ";

    // Jungfrau second interface
    if (detector_shm()->numUDPInterfaces == 2) {
        if (detector_shm()->receiverUDPIP2 == 0) {
            detector_shm()->receiverUDPIP2 = detector_shm()->receiverUDPIP;
        }
        if (detector_shm()->receiverUDPMAC2 == 0) {
            throw RuntimeError("configureMAC: Error. Receiver UDP MAC Addresses 2 not set");
        }
        FILE_LOG(logDEBUG1) << "rx_udpmac2 is valid ";
    }

    // copy to args and convert to hex
    snprintf(args[0], array_size, "%x", detector_shm()->receiverUDPPort);
    // snprintf(args[1], array_size, "%x", __builtin_bswap32(detector_shm()->receiverUDPIP));
    sls::strcpy_safe(args[1], getReceiverUDPIP().str());
    sls::strcpy_safe(args[2], getReceiverUDPMAC().hex().c_str());
    // sls::removeChar(args[2], ':');
    sls::strcpy_safe(args[3], getDetectorIP().hex().c_str());
    sls::strcpy_safe(args[4], getDetectorMAC().hex().c_str());
    // sls::removeChar(args[4], ':');
    snprintf(args[5], array_size, "%x", detector_shm()->receiverUDPPort2);
    // snprintf(args[6], array_size, "%x", __builtin_bswap32(detector_shm()->receiverUDPIP2));
    sls::strcpy_safe(args[6], getReceiverUDPIP2().str());
    sls::strcpy_safe(args[7], getReceiverUDPMAC2().hex().c_str());
    // sls::removeChar(args[7], ':');
    sls::strcpy_safe(args[8], getDetectorIP2().hex().c_str());
    sls::strcpy_safe(args[9], getDetectorMAC2().hex().c_str());
    // sls::removeChar(args[9], ':');

    // number of interfaces and which one
    snprintf(args[10], array_size, "%x", detector_shm()->numUDPInterfaces);
    snprintf(args[11], array_size, "%x", detector_shm()->selectedUDPInterface);

    // 2d positions to detector to put into udp header
    {
        int pos[2] = {0, 0};
        int max = detector_shm()->multiSize[1] * (detector_shm()->numUDPInterfaces);
        // row
        pos[0] = (detId % max);
        // col for horiz. udp ports
        pos[1] = (detId / max) * ((detector_shm()->myDetectorType == EIGER) ? 2 : 1);
        // pos[2] (z is reserved)
        FILE_LOG(logDEBUG1) << "Detector [" << detId << "] - (" << pos[0] << "," << pos[1] << ")";
        snprintf(args[12], array_size, "%x", pos[0]);
        snprintf(args[13], array_size, "%x", pos[1]);
    }

    FILE_LOG(logDEBUG1) << "receiver udp port:" << std::dec << args[0] << "-";
    FILE_LOG(logDEBUG1) << "receiver udp ip:" << args[1] << "-";
    FILE_LOG(logDEBUG1) << "receiver udp mac:" << args[2] << "-";
    FILE_LOG(logDEBUG1) << "detecotor udp ip:" << args[3] << "-";
    FILE_LOG(logDEBUG1) << "detector udp mac:" << args[4] << "-";
    FILE_LOG(logDEBUG1) << "receiver udp port2:" << std::dec << args[5] << "-";
    FILE_LOG(logDEBUG1) << "receiver udp ip2:" << args[6] << "-";
    FILE_LOG(logDEBUG1) << "receiver udp mac2:" << args[7] << "-";
    FILE_LOG(logDEBUG1) << "detecotor udp ip2:" << args[8] << "-";
    FILE_LOG(logDEBUG1) << "detector udp mac2:" << args[9] << "-";
    FILE_LOG(logDEBUG1) << "number of udp interfaces:" << std::dec << args[10] << "-";
    FILE_LOG(logDEBUG1) << "selected udp interface:" << std::dec << args[11] << "-";
    FILE_LOG(logDEBUG1) << "row:" << args[12] << "-";
    FILE_LOG(logDEBUG1) << "col:" << args[13] << "-";

    // send to server
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));

        uint64_t detector_mac = 0;
        uint32_t detector_ip = 0;
        sscanf(retvals[0], "%lx", &detector_mac); // TODO! (Erik) send mac and ip as int
        sscanf(retvals[1], "%x", &detector_ip);
        detector_ip = __builtin_bswap32(detector_ip);

        if (detector_shm()->detectorMAC != detector_mac) {
            detector_shm()->detectorMAC = detector_mac;
            FILE_LOG(logINFO) << detId << ": Detector MAC updated to " << getDetectorMAC();
        }

        if (detector_shm()->detectorIP != detector_ip) {
            detector_shm()->detectorIP = detector_ip;
            FILE_LOG(logINFO) << detId << ": Detector IP updated to " << getDetectorIP();
        }
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int64_t slsDetector::setTimer(timerIndex index, int64_t t) {
    int fnum = F_SET_TIMER;
    int ret = FAIL;
    int64_t args[2] = {(int64_t)index, t};
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Setting " << getTimerType(index) << " to " << t << " ns/value";

    // meausurement is only shm level
    if (index == MEASUREMENTS_NUMBER) {
        if (t >= 0) {
            detector_shm()->timerValue[index] = t;
            FILE_LOG(logDEBUG1) << getTimerType(index) << ": " << t;
        }
        return detector_shm()->timerValue[index];
    }

    // send to detector
    int64_t oldtimer = detector_shm()->timerValue[index];
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << getTimerType(index) << ": " << retval;
        detector_shm()->timerValue[index] = retval;
        // update #nchans and databytes, as it depends on #samples, roi,
        // readoutflags (ctb only)
        if (index == SAMPLES && (detector_shm()->myDetectorType == CHIPTESTBOARD ||
                                 detector_shm()->myDetectorType == MOENCH)) {
            updateTotalNumberOfChannels();
        }
        if (ret == FORCE_UPDATE) {
            client.close();
            ret = updateDetector();
        }
    }

    // setting timers consequences (eiger (ratecorr) )
    // (a get can also change timer value, hence check difference)
    if (oldtimer != detector_shm()->timerValue[index]) {
        // eiger: change exptime/subexptime, set rate correction to update table
        if (detector_shm()->myDetectorType == EIGER) {
            int dr = detector_shm()->dynamicRange;
            if ((dr == 32 && index == SUBFRAME_ACQUISITION_TIME) ||
                (dr == 16 && index == ACQUISITION_TIME)) {
                int r = getRateCorrection();
                if (r) {
                    setRateCorrection(r);
                }
            }
        }
    }

    // send to reciever
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG && ret == OK) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        // char mess[MAX_STR_LENGTH]{};
        switch (index) {
        case FRAME_NUMBER:
        case FRAME_PERIOD:
        case CYCLES_NUMBER:
        case ACQUISITION_TIME:
        case SUBFRAME_ACQUISITION_TIME:
        case SUBFRAME_DEADTIME:
        case SAMPLES:
        case STORAGE_CELL_NUMBER:
            // send
            fnum = F_SET_RECEIVER_TIMER;
            ret = FAIL;
            args[1] = detector_shm()->timerValue[index]; // to the value given by detector
            retval = -1;

            // rewrite args
            if ((index == FRAME_NUMBER) || (index == CYCLES_NUMBER) ||
                (index == STORAGE_CELL_NUMBER)) {
                args[1] = detector_shm()->timerValue[FRAME_NUMBER] *
                          ((detector_shm()->timerValue[CYCLES_NUMBER] > 0)
                               ? (detector_shm()->timerValue[CYCLES_NUMBER])
                               : 1) *
                          ((detector_shm()->timerValue[STORAGE_CELL_NUMBER] > 0)
                               ? (detector_shm()->timerValue[STORAGE_CELL_NUMBER]) + 1
                               : 1);
            }
            FILE_LOG(logDEBUG1) << "Sending "
                                << (((index == FRAME_NUMBER) || (index == CYCLES_NUMBER) ||
                                     (index == STORAGE_CELL_NUMBER))
                                        ? "(#Frames) * (#cycles) * (#storage cells)"
                                        : getTimerType(index))
                                << " to receiver: " << args[1];
            ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
            if (ret == FORCE_UPDATE) {
                receiver.close();
                ret = updateCachedReceiverVariables();
            }
            break;
        default:
            break;
        }
    }
    return detector_shm()->timerValue[index];
}

int64_t slsDetector::getTimeLeft(timerIndex index) {
    int fnum = F_GET_TIME_LEFT;
    int ret = FAIL;
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting " << getTimerType(index) << " left";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        ret = stop.sendCommandThenRead(fnum, &index, sizeof(index), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << getTimerType(index) << " left: " << retval;
        if (ret == FORCE_UPDATE) {
            ret = updateDetector();
        }
    }
    return retval;
}

int slsDetector::setSpeed(speedVariable sp, int value, int mode) {
    int fnum = F_SET_SPEED;
    int ret = FAIL;
    int args[3] = {(int)sp, value, mode};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting speed index " << sp << " to " << value << " mode: " << mode;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Speed index " << sp << ": " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setDynamicRange(int n) {
    // TODO! Properly handle fail
    int fnum = F_SET_DYNAMIC_RANGE;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting dynamic range to " << n;

    // send to detector
    int olddr = detector_shm()->dynamicRange;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        // char mess[MAX_STR_LENGTH] = {};
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &n, sizeof(n), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Dynamic Range: " << retval;
        detector_shm()->dynamicRange = retval;
        if (ret == FORCE_UPDATE) {
            client.close();
            ret = updateDetector();
        }
    }

    // only for eiger
    // setting dr consequences on databytes shm
    // (a get can also change timer value, hence check difference)
    if (olddr != detector_shm()->dynamicRange) {
        detector_shm()->dataBytes = detector_shm()->nChips * detector_shm()->nChans * retval / 8;
        detector_shm()->dataBytesInclGapPixels =
            (detector_shm()->nChip[X] * detector_shm()->nChan[X] +
             detector_shm()->gappixels * detector_shm()->nGappixels[X]) *
            (detector_shm()->nChip[Y] * detector_shm()->nChan[Y] +
             detector_shm()->gappixels * detector_shm()->nGappixels[Y]) *
            retval / 8;
        FILE_LOG(logDEBUG1) << "Data bytes " << detector_shm()->dataBytes;
        FILE_LOG(logDEBUG1) << "Data bytes including gap pixels"
                            << detector_shm()->dataBytesInclGapPixels;
    }

    // send to receiver
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG && ret == OK) {
        fnum = F_SET_RECEIVER_DYNAMIC_RANGE;
        ret = FAIL;
        n = detector_shm()->dynamicRange;
        retval = -1;
        FILE_LOG(logDEBUG1) << "Sending dynamic range to receiver: " << n;
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &n, sizeof(n), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver Dynamic range: " << retval;
        if (ret == FORCE_UPDATE) {
            receiver.close();
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->dynamicRange;
}

int slsDetector::getDataBytes() { return detector_shm()->dataBytes; }

int slsDetector::getDataBytesInclGapPixels() { return detector_shm()->dataBytesInclGapPixels; }

int slsDetector::setDAC(int val, dacIndex index, int mV) {
    int fnum = F_SET_DAC;
    int ret = FAIL;
    int args[3] = {(int)index, mV, val};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting DAC " << index << " to " << val << (mV ? "mV" : "dac units");

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Dac index " << index << ": " << retval << (mV ? "mV" : "dac units");
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::getADC(dacIndex index) {
    int fnum = F_GET_ADC;
    int arg = static_cast<int>(index);
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting ADC " << index;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "ADC (" << index << "): " << retval;
    }
    return retval;
}

slsDetectorDefs::externalCommunicationMode
slsDetector::setExternalCommunicationMode(externalCommunicationMode pol) {
    int fnum = F_SET_EXTERNAL_COMMUNICATION_MODE;
    int ret = FAIL;
    int arg = (int)pol;
    externalCommunicationMode retval = GET_EXTERNAL_COMMUNICATION_MODE;
    FILE_LOG(logDEBUG1) << "Setting communication to mode " << pol;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Timing Mode: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

slsDetectorDefs::externalSignalFlag slsDetector::setExternalSignalFlags(externalSignalFlag pol,
                                                                        int signalindex) {
    int fnum = F_SET_EXTERNAL_SIGNAL_FLAG;
    int ret = FAIL;
    int args[2] = {signalindex, pol};
    externalSignalFlag retval = GET_EXTERNAL_SIGNAL_FLAG;
    FILE_LOG(logDEBUG1) << "Setting signal " << signalindex << " to flag " << pol;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Ext Signal (" << signalindex << "): " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setReadOutFlags(readOutFlags flag) {
    int fnum = F_SET_READOUT_FLAGS;
    int ret = FAIL;
    int arg = (int)flag;
    readOutFlags retval = GET_READOUT_FLAGS;
    FILE_LOG(logDEBUG1) << "Setting readout flags to " << flag;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Readout flag: " << retval;
        detector_shm()->roFlags = (readOutFlags)retval;
        // update #nchans and databytes, as it depends on #samples, roi,
        // readoutflags (ctb only)
        if (detector_shm()->myDetectorType == CHIPTESTBOARD) {
            updateTotalNumberOfChannels();
        }
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }

    // sending to receiver
    if (ret != FAIL) {
        fnum = F_RECEIVER_SET_READOUT_FLAGS;
        ret = FAIL;
        arg = detector_shm()->roFlags;
        retval = (readOutFlags)-1;
        FILE_LOG(logDEBUG1) << "Setting receiver readout flags to " << arg;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver readout flag: " << retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }

    return detector_shm()->roFlags;
}

uint32_t slsDetector::writeRegister(uint32_t addr, uint32_t val) {
    int fnum = F_WRITE_REGISTER;
    int ret = FAIL;
    uint32_t args[2] = {addr, val};
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Writing to register 0x" << std::hex << addr << "data: 0x" << std::hex
                        << val << std::dec;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Register 0x" << std::hex << addr << ": 0x" << std::hex << retval
                            << std::dec;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

uint32_t slsDetector::readRegister(uint32_t addr) {
    int fnum = F_READ_REGISTER;
    int ret = FAIL;
    uint32_t arg = addr;
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Reading register 0x" << std::hex << addr << std::dec;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Register 0x" << std::hex << addr << ": 0x" << std::hex << retval
                            << std::dec;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

uint32_t slsDetector::setBit(uint32_t addr, int n) {
    if (n < 0 || n > 31) {
        throw RuntimeError("Bit number " + std::to_string(n) + " out of Range");
    } else {
        uint32_t val = readRegister(addr);
        return writeRegister(addr, val | 1 << n);
    }
}

uint32_t slsDetector::clearBit(uint32_t addr, int n) {
    if (n < 0 || n > 31) {
        throw RuntimeError("Bit number " + std::to_string(n) + " out of Range");
    } else {
        uint32_t val = readRegister(addr);
        return writeRegister(addr, val & ~(1 << n));
    }
}

std::string slsDetector::setDetectorMAC(const std::string &address) {
    auto addr = MacAddr(address);
    if (addr == 0) {
        throw RuntimeError("server MAC Address should be in xx:xx:xx:xx:xx:xx format");
    } else {
        detector_shm()->detectorMAC = addr;
        if (!strcmp(detector_shm()->receiver_hostname, "none")) {
            FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
        } else if (setUDPConnection() == FAIL) {
            FILE_LOG(logWARNING) << "UDP connection set up failed";
        }
    }
    return getDetectorMAC().str();
}

MacAddr slsDetector::getDetectorMAC() { return detector_shm()->detectorMAC; }

std::string slsDetector::setDetectorMAC2(const std::string &address) {
    auto addr = MacAddr(address);
    if (addr == 0) {
        throw RuntimeError("server MAC Address 2 should be in xx:xx:xx:xx:xx:xx format");
    } else {
        detector_shm()->detectorMAC2 = addr;
        if (!strcmp(detector_shm()->receiver_hostname, "none")) {
            FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
        } else if (setUDPConnection() == FAIL) {
            FILE_LOG(logWARNING) << "UDP connection set up failed";
        }
    }
    return getDetectorMAC2().str();
}

MacAddr slsDetector::getDetectorMAC2() { return detector_shm()->detectorMAC2; }

std::string slsDetector::setDetectorIP(const std::string &ip) {
    auto addr = IpAddr(ip);
    if (addr != 0) {
        detector_shm()->detectorIP = ip;
        if (!strcmp(detector_shm()->receiver_hostname, "none")) {
            FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
        } else if (setUDPConnection() == FAIL) {
            FILE_LOG(logWARNING) << "UDP connection set up failed";
        }
    } else {
        throw RuntimeError("setDetectorIP: IP Address should be VALID and "
                           "in xxx.xxx.xxx.xxx format");
    }
    return getDetectorIP().str();
}

IpAddr slsDetector::getDetectorIP() const { return detector_shm()->detectorIP; }

std::string slsDetector::setDetectorIP2(const std::string &ip) {
    auto addr = IpAddr(ip);
    if (addr != 0) {
        detector_shm()->detectorIP2 = ip;
        if (!strcmp(detector_shm()->receiver_hostname, "none")) {
            FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
        } else if (setUDPConnection() == FAIL) {
            FILE_LOG(logWARNING) << "UDP connection set up failed";
        }
    } else {
        throw RuntimeError("setDetectorIP: IP2 Address should be VALID and "
                           "in xxx.xxx.xxx.xxx format");
    }
    return getDetectorIP().str();
}

IpAddr slsDetector::getDetectorIP2() const { return detector_shm()->detectorIP2; }

std::string slsDetector::setReceiverHostname(const std::string &receiverIP) {
    FILE_LOG(logDEBUG1) << "Setting up Receiver with " << receiverIP;
    // recieverIP is none
    if (receiverIP == "none") {
        memset(detector_shm()->receiver_hostname, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(detector_shm()->receiver_hostname, "none");
        detector_shm()->receiverOnlineFlag = OFFLINE_FLAG;
        return std::string(detector_shm()->receiver_hostname);
    }
    // stop acquisition if running
    if (getRunStatus() == RUNNING) {
        FILE_LOG(logWARNING) << "Acquisition already running, Stopping it.";
        stopAcquisition();
    }
    // update detector before receiver
    updateDetector();

    // start updating
    sls::strcpy_safe(detector_shm()->receiver_hostname, receiverIP.c_str());

    if (setReceiverOnline(ONLINE_FLAG) == ONLINE_FLAG) {
        FILE_LOG(logDEBUG1)
            << "detector type:"
            << (slsDetectorDefs::detectorTypeToString(detector_shm()->myDetectorType))
            << "\ndetector id:" << detId << "\ndetector hostname:" << detector_shm()->hostname
            << "\nfile path:" << detector_shm()->receiver_filePath
            << "\nfile name:" << detector_shm()->receiver_fileName
            << "\nfile index:" << detector_shm()->receiver_fileIndex
            << "\nfile format:" << detector_shm()->receiver_fileFormatType
            << "\nr_framesperfile:" << detector_shm()->receiver_framesPerFile
            << "\nr_discardpolicy:" << detector_shm()->receiver_frameDiscardMode
            << "\nr_padding:" << detector_shm()->receiver_framePadding
            << "\nwrite enable:" << detector_shm()->receiver_fileWriteEnable
            << "\noverwrite enable:" << detector_shm()->receiver_overWriteEnable
            << "\nframe index needed:"
            << ((detector_shm()->timerValue[FRAME_NUMBER] *
                 detector_shm()->timerValue[CYCLES_NUMBER]) > 1)
            << "\nframe period:" << (detector_shm()->timerValue[FRAME_PERIOD])
            << "\nframe number:" << (detector_shm()->timerValue[FRAME_NUMBER])
            << "\nsub exp time:" << (detector_shm()->timerValue[SUBFRAME_ACQUISITION_TIME])
            << "\nsub dead time:" << (detector_shm()->timerValue[SUBFRAME_DEADTIME])
            << "\nsamples:" << (detector_shm()->timerValue[SAMPLES])
            << "\ndynamic range:" << detector_shm()->dynamicRange
            << "\nflippeddatax:" << (detector_shm()->flippedData[X])
            << "\nactivated: " << detector_shm()->activated << "\nreceiver deactivated padding: "
            << detector_shm()->receiver_deactivatedPaddingEnable
            << "\nsilent Mode:" << detector_shm()->receiver_silentMode
            << "\n10GbE:" << detector_shm()->tenGigaEnable
            << "\nGap pixels: " << detector_shm()->gappixels
            << "\nr_readfreq:" << detector_shm()->receiver_read_freq
            << "\nrx streaming port:" << detector_shm()->receiver_zmqport
            << "\nrx streaming source ip:" << detector_shm()->receiver_zmqip
            << "\nrx additional json header:" << detector_shm()->receiver_additionalJsonHeader
            << "\nrx_datastream:" << enableDataStreamingFromReceiver(-1) << std::endl;

        if (setDetectorType(detector_shm()->myDetectorType) != GENERIC) {
            sendMultiDetectorSize();
            setDetectorId();
            setDetectorHostname();
            setUDPConnection();
            setReceiverUDPSocketBufferSize(0);
            setFilePath(detector_shm()->receiver_filePath);
            setFileName(detector_shm()->receiver_fileName);
            setFileIndex(detector_shm()->receiver_fileIndex);
            setFileFormat(detector_shm()->receiver_fileFormatType);
            setReceiverFramesPerFile(detector_shm()->receiver_framesPerFile);
            setReceiverFramesDiscardPolicy(detector_shm()->receiver_frameDiscardMode);
            setReceiverPartialFramesPadding(detector_shm()->receiver_framePadding);
            enableWriteToFile(detector_shm()->receiver_fileWriteEnable);
            overwriteFile(detector_shm()->receiver_overWriteEnable);
            setTimer(FRAME_PERIOD, detector_shm()->timerValue[FRAME_PERIOD]);
            setTimer(FRAME_NUMBER, detector_shm()->timerValue[FRAME_NUMBER]);
            setTimer(ACQUISITION_TIME, detector_shm()->timerValue[ACQUISITION_TIME]);

            // detector specific
            switch (detector_shm()->myDetectorType) {

            case EIGER:
                setTimer(SUBFRAME_ACQUISITION_TIME,
                         detector_shm()->timerValue[SUBFRAME_ACQUISITION_TIME]);
                setTimer(SUBFRAME_DEADTIME, detector_shm()->timerValue[SUBFRAME_DEADTIME]);
                setDynamicRange(detector_shm()->dynamicRange);
                setFlippedData(X, -1);
                activate(-1);
                setDeactivatedRxrPaddingMode(detector_shm()->receiver_deactivatedPaddingEnable);
                enableGapPixels(detector_shm()->gappixels);
                enableTenGigabitEthernet(detector_shm()->tenGigaEnable);
                setReadOutFlags(GET_READOUT_FLAGS);
                break;

            case CHIPTESTBOARD:
                setTimer(SAMPLES, detector_shm()->timerValue[SAMPLES]);
                enableTenGigabitEthernet(detector_shm()->tenGigaEnable);
                setReadOutFlags(GET_READOUT_FLAGS);
                break;

            case MOENCH:
                setTimer(SAMPLES, detector_shm()->timerValue[SAMPLES]);
                enableTenGigabitEthernet(detector_shm()->tenGigaEnable);
                break;

            default:
                break;
            }

            setReceiverSilentMode(detector_shm()->receiver_silentMode);
            // data streaming
            setReceiverStreamingFrequency(detector_shm()->receiver_read_freq);
            setReceiverStreamingPort(getReceiverStreamingPort());
            setReceiverStreamingIP(getReceiverStreamingIP());
            setAdditionalJsonHeader(detector_shm()->receiver_additionalJsonHeader);
            enableDataStreamingFromReceiver(enableDataStreamingFromReceiver(-1));
            if (detector_shm()->myDetectorType == GOTTHARD ||
                detector_shm()->myDetectorType == CHIPTESTBOARD ||
                detector_shm()->myDetectorType == MOENCH) {
                sendROI(-1, nullptr);
            }
        }
    }
    return std::string(detector_shm()->receiver_hostname);
}

std::string slsDetector::getReceiverHostname() const {
    return std::string(detector_shm()->receiver_hostname);
}

std::string slsDetector::setReceiverUDPIP(const std::string &udpip) {

    if (udpip.length() && udpip.length() < 16) {
        auto ip = IpStringToUint(udpip.c_str());
        if (ip == 0) {
            throw ReceiverError("setReceiverUDPIP: UDP IP Address should be "
                                "VALID and in xxx.xxx.xxx.xxx format");
        } else {
            detector_shm()->receiverUDPIP = ip;
            if (!strcmp(detector_shm()->receiver_hostname, "none")) {
                FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
            } else if (setUDPConnection() == FAIL) {
                FILE_LOG(logWARNING) << "UDP connection set up failed";
            }
        }
    }
    return getReceiverUDPIP().str();
}

sls::IpAddr slsDetector::getReceiverUDPIP() const { return detector_shm()->receiverUDPIP; }

std::string slsDetector::setReceiverUDPIP2(const std::string &udpip) {
    if (udpip.length() && udpip.length() < 16) {
        auto ip = IpStringToUint(udpip.c_str());
        if (ip == 0) {
            throw ReceiverError("setReceiverUDPIP: UDP IP Address 2 should be "
                                "VALID and in xxx.xxx.xxx.xxx format");
        } else {
            detector_shm()->receiverUDPIP2 = ip;
            if (!strcmp(detector_shm()->receiver_hostname, "none")) {
                FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
            } else if (setUDPConnection() == FAIL) {
                FILE_LOG(logWARNING) << "UDP connection set up failed";
            }
        }
    }
    return getReceiverUDPIP2().str();
}

sls::IpAddr slsDetector::getReceiverUDPIP2() const { return detector_shm()->receiverUDPIP2; }

std::string slsDetector::setReceiverUDPMAC(const std::string &udpmac) {
    auto mac = MacStringToUint(udpmac);
    if (mac == 0) {
        throw ReceiverError("Could not decode UDPMAC from: " + udpmac);
    }
    detector_shm()->receiverUDPMAC = mac;
    return getReceiverUDPMAC().str();
}

MacAddr slsDetector::getReceiverUDPMAC() const { return detector_shm()->receiverUDPMAC; }

std::string slsDetector::setReceiverUDPMAC2(const std::string &udpmac) {
    auto mac = MacStringToUint(udpmac);
    if (mac == 0) {
        throw ReceiverError("Could not decode UDPMA2C from: " + udpmac);
    }
    detector_shm()->receiverUDPMAC2 = mac;
    return getReceiverUDPMAC2().str();
}

MacAddr slsDetector::getReceiverUDPMAC2() const { return detector_shm()->receiverUDPMAC2; }

int slsDetector::setReceiverUDPPort(int udpport) {
    detector_shm()->receiverUDPPort = udpport;
    if (!strcmp(detector_shm()->receiver_hostname, "none")) {
        FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
    } else if (setUDPConnection() == FAIL) {
        FILE_LOG(logWARNING) << "UDP connection set up failed";
    }
    return detector_shm()->receiverUDPPort;
}

int slsDetector::getReceiverUDPPort() const { return detector_shm()->receiverUDPPort; }

int slsDetector::setReceiverUDPPort2(int udpport) {
    detector_shm()->receiverUDPPort2 = udpport;
    if (!strcmp(detector_shm()->receiver_hostname, "none")) {
        FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
    } else if (setUDPConnection() == FAIL) {
        FILE_LOG(logWARNING) << "UDP connection set up failed";
    }
    return detector_shm()->receiverUDPPort2;
}

int slsDetector::getReceiverUDPPort2() const { return detector_shm()->receiverUDPPort2; }

int slsDetector::setNumberofUDPInterfaces(int n) {
    if (detector_shm()->myDetectorType != JUNGFRAU) {
        throw RuntimeError("Cannot choose number of interfaces for this detector");
    }
    detector_shm()->numUDPInterfaces = (n > 1 ? 2 : 1);
    if (!strcmp(detector_shm()->receiver_hostname, "none")) {
        FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
    } else if (setUDPConnection() == FAIL) {
        FILE_LOG(logWARNING) << "UDP connection set up failed";
    }
    return detector_shm()->numUDPInterfaces;
}

int slsDetector::getNumberofUDPInterfaces() const { return detector_shm()->numUDPInterfaces; }

int slsDetector::selectUDPInterface(int n) {
    if (detector_shm()->myDetectorType != JUNGFRAU) {
        throw RuntimeError("Cannot select an interface for this detector");
    }
    detector_shm()->selectedUDPInterface = (n > 1 ? 2 : 1);
    if (!strcmp(detector_shm()->receiver_hostname, "none")) {
        FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
    } else if (setUDPConnection() == FAIL) {
        FILE_LOG(logWARNING) << "UDP connection set up failed";
    }
    return detector_shm()->selectedUDPInterface;
}

int slsDetector::getSelectedUDPInterface() const { return detector_shm()->selectedUDPInterface; }

void slsDetector::setClientStreamingPort(int port) { detector_shm()->zmqport = port; }

int slsDetector::getClientStreamingPort() { return detector_shm()->zmqport; }

void slsDetector::setReceiverStreamingPort(int port) {
    // copy now else it is lost if rx_hostname not set yet
    detector_shm()->receiver_zmqport = port;

    int fnum = F_SET_RECEIVER_STREAMING_PORT;
    int ret = FAIL;
    int arg = detector_shm()->receiver_zmqport;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending receiver streaming port to receiver: " << arg;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver streaming port: " << retval;
        detector_shm()->receiver_zmqport = retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
}

int slsDetector::getReceiverStreamingPort() { return detector_shm()->receiver_zmqport; }

void slsDetector::setClientStreamingIP(const std::string &sourceIP) {
    struct addrinfo *result;
    // on failure to convert to a valid ip
    if (sls::ConvertHostnameToInternetAddress(sourceIP.c_str(), &result)) {
        throw RuntimeError("Could not convert zmqip into a valid IP" + sourceIP);
    }
    // on success put IP as std::string into arg
    memset(detector_shm()->zmqip, 0, MAX_STR_LENGTH);
    sls::ConvertInternetAddresstoIpString(result, detector_shm()->zmqip, MAX_STR_LENGTH);
}

std::string slsDetector::getClientStreamingIP() { return std::string(detector_shm()->zmqip); }

void slsDetector::setReceiverStreamingIP(std::string sourceIP) {
    int fnum = F_RECEIVER_STREAMING_SRC_IP;
    int ret = FAIL;
    char args[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};
    FILE_LOG(logDEBUG1) << "Sending receiver streaming IP to receiver: " << sourceIP;

    // if empty, give rx_hostname
    if (sourceIP.empty()) {
        if (!strcmp(detector_shm()->receiver_hostname, "none")) {
            throw RuntimeError("Receiver hostname not set yet. Cannot create "
                               "rx_zmqip from none");
        }
        sourceIP.assign(detector_shm()->receiver_hostname);
    }

    // verify the ip
    {
        struct addrinfo *result;
        // on failure to convert to a valid ip
        if (sls::ConvertHostnameToInternetAddress(sourceIP.c_str(), &result)) {
            throw RuntimeError("Could not convert rx_zmqip into a valid IP" + sourceIP);
        }
        // on success put IP as std::string into arg
        sls::ConvertInternetAddresstoIpString(result, args, sizeof(args));
    }

    // set it anyway, else it is lost if rx_hostname is not set yet
    memset(detector_shm()->receiver_zmqip, 0, MAX_STR_LENGTH);
    sls::strcpy_safe(detector_shm()->receiver_zmqip, args);
    // if zmqip is empty, update it
    if (!strlen(detector_shm()->zmqip)) {
        sls::strcpy_safe(detector_shm()->zmqip, args);
    }
    FILE_LOG(logDEBUG1) << "Sending receiver streaming IP to receiver: " << args;

    // send to receiver
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));
        FILE_LOG(logDEBUG1) << "Receiver streaming port: " << retvals;
        memset(detector_shm()->receiver_zmqip, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(detector_shm()->receiver_zmqip, retvals);
        if (ret == FORCE_UPDATE) {
            receiver.close();
            ret = updateCachedReceiverVariables();
        }
    }
}

std::string slsDetector::getReceiverStreamingIP() {
    return std::string(detector_shm()->receiver_zmqip);
}

int slsDetector::setDetectorNetworkParameter(networkParameter index, int delay) {
    int fnum = F_SET_NETWORK_PARAMETER;
    int ret = FAIL;
    int args[2] = {(int)index, delay};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting network parameter index " << index << " to " << delay;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Network Parameter (" << index << "): " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

std::string slsDetector::setAdditionalJsonHeader(const std::string &jsonheader) {
    int fnum = F_ADDITIONAL_JSON_HEADER;
    int ret = FAIL;
    char args[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};
    sls::strcpy_safe(args, jsonheader.c_str());
    FILE_LOG(logDEBUG1) << "Sending additional json header " << args;

    if (detector_shm()->receiverOnlineFlag != ONLINE_FLAG) {
        sls::strcpy_safe(detector_shm()->receiver_additionalJsonHeader, jsonheader.c_str());
    } else {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));
        FILE_LOG(logDEBUG1) << "Additional json header: " << retvals;
        memset(detector_shm()->receiver_additionalJsonHeader, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(detector_shm()->receiver_additionalJsonHeader, retvals);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return std::string(detector_shm()->receiver_additionalJsonHeader);
}

std::string slsDetector::getAdditionalJsonHeader() {
    int fnum = F_GET_ADDITIONAL_JSON_HEADER;
    int ret = FAIL;
    char retvals[MAX_STR_LENGTH] = {0};
    FILE_LOG(logDEBUG1) << "Getting additional json header ";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, retvals, sizeof(retvals));
        FILE_LOG(logDEBUG1) << "Additional json header: " << retvals;
        memset(detector_shm()->receiver_additionalJsonHeader, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(detector_shm()->receiver_additionalJsonHeader, retvals);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return std::string(detector_shm()->receiver_additionalJsonHeader);
}

std::string slsDetector::setAdditionalJsonParameter(const std::string &key,
                                                    const std::string &value) {
    // validation (value or key is empty)
    if (!key.length() || !value.length()) {
        throw("Could not set additional json header parameter as the key or "
              "value is empty");
    }

    // validation (ignore if key or value has , : ")
    if (key.find_first_of(",\":") != std::string::npos ||
        value.find_first_of(",\":") != std::string::npos) {
        throw RuntimeError("Could not set additional json header parameter as "
                           "the key or value has "
                           "illegal characters (,\":)");
    }

    // create actual key to search for and actual value to put, (key has
    // additional ':' as value could exist the same way)
    std::string keyLiteral(std::string("\"") + key + std::string("\":"));
    std::string valueLiteral(value);
    // add quotations to value only if it is a string
    try {
        stoi(valueLiteral);
    } catch (...) {
        // add quotations if it failed to convert to integer, otherwise nothing
        valueLiteral.insert(0, "\"");
        valueLiteral.append("\"");
    }

    std::string header(detector_shm()->receiver_additionalJsonHeader);
    size_t keyPos = header.find(keyLiteral);

    // if key found, replace value
    if (keyPos != std::string::npos) {
        size_t valueStartPos = header.find(std::string(":"), keyPos) + 1;
        size_t valueEndPos = header.find(std::string(","), valueStartPos) - 1;
        // if valueEndPos doesnt find comma (end of string), it goes anyway to
        // end of line
        header.replace(valueStartPos, valueEndPos - valueStartPos + 1, valueLiteral);
    }

    // key not found, append key value pair
    else {
        if (header.length()) {
            header.append(",");
        }
        header.append(keyLiteral + valueLiteral);
    }

    // update additional json header
    setAdditionalJsonHeader(header);
    return getAdditionalJsonParameter(key);
}

std::string slsDetector::getAdditionalJsonParameter(const std::string &key) {
    // additional json header is empty
    if (!strlen(detector_shm()->receiver_additionalJsonHeader))
        return std::string("");

    // add quotations before and after the key value
    std::string keyLiteral = key;
    keyLiteral.insert(0, "\"");
    keyLiteral.append("\"");

    // loop through the parameters
    for (const auto &parameter : sls::split(detector_shm()->receiver_additionalJsonHeader, ',')) {
        // get a vector of key value pair for each parameter
        const auto &pairs = sls::split(parameter, ':');
        // match for key
        if (pairs[0] == keyLiteral) {
            // return value without quotations (if it has any)
            if (pairs[1][0] == '\"')
                return pairs[1].substr(1, pairs[1].length() - 2);
            else
                return pairs[1];
        }
    }

    // return empty string as no match found with key
    return std::string("");
}

int64_t slsDetector::setReceiverUDPSocketBufferSize(int64_t udpsockbufsize) {
    int fnum = F_RECEIVER_UDP_SOCK_BUF_SIZE;
    int ret = FAIL;
    int64_t arg = udpsockbufsize;
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Sending UDP Socket Buffer size to receiver: " << arg;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver UDP Socket Buffer size: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return retval;
}

int64_t slsDetector::getReceiverUDPSocketBufferSize() { return setReceiverUDPSocketBufferSize(); }

int64_t slsDetector::getReceiverRealUDPSocketBufferSize() {
    int fnum = F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE;
    int ret = FAIL;
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting real UDP Socket Buffer size to receiver";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Real Receiver UDP Socket Buffer size: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return retval;
}

int slsDetector::setUDPConnection() {
    int fnum = F_SETUP_RECEIVER_UDP;
    int ret = FAIL;
    char args[6][MAX_STR_LENGTH] = {{}, {}, {}, {}, {}, {}};
    char retvals[2][MAX_STR_LENGTH] = {{}, {}};
    FILE_LOG(logDEBUG1) << "Setting UDP Connection";

    // called before set up
    if (!strcmp(detector_shm()->receiver_hostname, "none")) {
        FILE_LOG(logDEBUG1) << "Receiver hostname not set yet.";
        return FAIL;
    }

    if (detector_shm()->receiverUDPIP == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        detector_shm()->receiverUDPIP = IpStringToUint(detector_shm()->receiver_hostname);
        if (detector_shm()->receiverUDPIP == 0) {
            detector_shm()->receiverUDPIP = HostnameToIp(detector_shm()->receiver_hostname);
        }
    }
    // jungfrau 2 interfaces or (1 interface and 2nd interface), copy udpip if
    // udpip2 empty
    if (detector_shm()->numUDPInterfaces == 2 || detector_shm()->selectedUDPInterface == 2) {
        if (detector_shm()->receiverUDPIP2 == 0) {
            detector_shm()->receiverUDPIP2 = detector_shm()->receiverUDPIP;
        }
    }

    // copy arguments to args[][]
    snprintf(args[0], sizeof(args[0]), "%d", detector_shm()->numUDPInterfaces);
    snprintf(args[1], sizeof(args[1]), "%d", detector_shm()->selectedUDPInterface);
    sls::strcpy_safe(args[2], getReceiverUDPIP().str());
    sls::strcpy_safe(args[3], getReceiverUDPIP2().str());
    snprintf(args[4], sizeof(args[4]), "%d", detector_shm()->receiverUDPPort);
    snprintf(args[5], sizeof(args[5]), "%d", detector_shm()->receiverUDPPort2);
    FILE_LOG(logDEBUG1) << "Receiver Number of UDP Interfaces: "
                        << detector_shm()->numUDPInterfaces;
    FILE_LOG(logDEBUG1) << "Receiver Selected Interface: " << detector_shm()->selectedUDPInterface;
    FILE_LOG(logDEBUG1) << "Receiver udp ip address: " << detector_shm()->receiverUDPIP;
    FILE_LOG(logDEBUG1) << "Receiver udp ip address2: " << detector_shm()->receiverUDPIP2;
    FILE_LOG(logDEBUG1) << "Receiver udp port: " << detector_shm()->receiverUDPPort;
    FILE_LOG(logDEBUG1) << "Receiver udp port2: " << detector_shm()->receiverUDPPort2;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));
        if (strlen(retvals[0])) {
            FILE_LOG(logDEBUG1) << "Receiver UDP MAC returned : " << retvals[0];
            detector_shm()->receiverUDPMAC = MacStringToUint(retvals[0]);
        }
        if (strlen(retvals[1])) {
            FILE_LOG(logDEBUG1) << "Receiver UDP MAC2 returned : " << retvals[1];
            detector_shm()->receiverUDPMAC2 = MacStringToUint(retvals[1]);
        }
        if (ret == FORCE_UPDATE) {
            receiver.close();
            ret = updateCachedReceiverVariables();
        }
        // configure detector with udp details
        if (configureMAC() == FAIL) {
            setReceiverOnline(OFFLINE_FLAG);
        }
    } else {
        throw ReceiverError("setUDPConnection: Receiver is OFFLINE");
    }

    printReceiverConfiguration(logDEBUG1);
    return ret;
}

int slsDetector::digitalTest(digitalTestMode mode, int ival) {
    int fnum = F_DIGITAL_TEST;
    int ret = FAIL;
    int args[2] = {(int)mode, ival};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending digital test of mode " << mode << ", ival " << ival;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Digital Test returned: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::loadImageToDetector(imageType index, const std::string &fname) {
    int nChan = getTotalNumberOfChannels();
    int16_t args[nChan];
    FILE_LOG(logDEBUG1) << "Loading " << (!index ? "Dark" : "Gain") << "image from file " << fname;

    if (readDataFile(fname, args, nChan)) {
        return sendImageToDetector(index, args);
    } else {
        throw RuntimeError("slsDetector::loadImageToDetector: Could not open file: " + fname);
    }
}

int slsDetector::sendImageToDetector(imageType index, int16_t imageVals[]) {
    int fnum = F_LOAD_IMAGE;
    int ret = FAIL;
    int nChan = getTotalNumberOfChannels();
    int args[2] = {(int)index, nChan};
    FILE_LOG(logDEBUG1) << "Sending image to detector";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        client.sendData(&fnum, sizeof(fnum));
        client.sendData(args, sizeof(args));
        client.sendData(imageVals, nChan * sizeof(int16_t));
        client.receiveData(&ret, sizeof(ret));
        if (ret == FAIL) {
            char mess[MAX_STR_LENGTH]{};
            client.receiveData(mess, MAX_STR_LENGTH);
            throw DetectorError("Detector " + std::to_string(detId) +
                                " returned error: " + std::string(mess));
        }
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::writeCounterBlockFile(const std::string &fname, int startACQ) {
    int ret = FAIL;
    int nChan = getTotalNumberOfChannels();
    short int retvals[nChan];
    FILE_LOG(logDEBUG1) << "Reading Counter to " << fname
                        << (startACQ ? " and Restarting Acquisition" : "\n");

    ret = getCounterBlock(retvals, startACQ);
    if (ret != FAIL) {
        return writeDataFile(fname, nChan, retvals);
    } else {
        throw RuntimeError("slsDetector::writeCounterBlockFile: getCounterBlock failed");
    }
}

int slsDetector::getCounterBlock(int16_t image[], int startACQ) {
    int fnum = F_READ_COUNTER_BLOCK;
    int ret = FAIL;
    int nChan = getTotalNumberOfChannels();
    int args[2] = {startACQ, nChan};
    FILE_LOG(logDEBUG1) << "Reading Counter block with startacq: " << startACQ;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), image, nChan * sizeof(int16_t));
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::resetCounterBlock(int startACQ) {
    int fnum = F_RESET_COUNTER_BLOCK;
    int ret = FAIL;
    int arg = startACQ;
    FILE_LOG(logDEBUG1) << "Resetting Counter with startacq: " << startACQ;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::setCounterBit(int i) {
    int fnum = F_SET_COUNTER_BIT;
    int ret = FAIL;
    int arg = i;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending counter bit " << arg;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Counter bit: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::sendROIToProcessor() {
    std::ostringstream os;
    os << "[" << detector_shm()->roiLimits[0].xmin << ", " << detector_shm()->roiLimits[0].xmax
       << ", " << detector_shm()->roiLimits[0].ymin << ", " << detector_shm()->roiLimits[0].ymax
       << "]";
    std::string sroi = os.str();
    std::string result = setAdditionalJsonParameter("roi", sroi);
    if (result == sroi)
        return OK;
    return FAIL;
}

int slsDetector::setROI(int n, ROI roiLimits[]) {
    // sort ascending order
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (roiLimits[j].xmin < roiLimits[i].xmin) {
                int temp = roiLimits[i].xmin;
                roiLimits[i].xmin = roiLimits[j].xmin;
                roiLimits[j].xmin = temp;
                temp = roiLimits[i].xmax;
                roiLimits[i].xmax = roiLimits[j].xmax;
                roiLimits[j].xmax = temp;
                temp = roiLimits[i].ymin;
                roiLimits[i].ymin = roiLimits[j].ymin;
                roiLimits[j].ymin = temp;
                temp = roiLimits[i].ymax;
                roiLimits[i].ymax = roiLimits[j].ymax;
                roiLimits[j].ymax = temp;
            }
        }
    }

    int ret = sendROI(n, roiLimits);
    // moench (send to processor)
    if (detector_shm()->myDetectorType == MOENCH) {
        sendROIToProcessor();
    }
    // update #nchans and databytes, as it depends on #samples, roi,
    // readoutflags (ctb only)
    if (detector_shm()->myDetectorType == CHIPTESTBOARD ||
        detector_shm()->myDetectorType == MOENCH) {
        updateTotalNumberOfChannels();
    }
    return ret;
}

slsDetectorDefs::ROI *slsDetector::getROI(int &n) {
    sendROI(-1, nullptr);
    n = detector_shm()->nROI;
    // moench - get json header(due to different clients, diff shm) (get roi is
    // from detector: updated anyway)
    if (detector_shm()->myDetectorType == MOENCH) {
        getAdditionalJsonHeader();
    }
    // update #nchans and databytes, as it depends on #samples, roi,
    // readoutflags (ctb only)
    if (detector_shm()->myDetectorType == CHIPTESTBOARD ||
        detector_shm()->myDetectorType == MOENCH) {
        updateTotalNumberOfChannels();
    }
    return detector_shm()->roiLimits;
}

int slsDetector::getNRoi() { return detector_shm()->nROI; }

int slsDetector::sendROI(int n, ROI roiLimits[]) {
    int fnum = F_SET_ROI;
    int ret = FAIL;
    int narg = n;
    // send roiLimits if given, else from shm
    ROI *arg = (roiLimits != nullptr) ? roiLimits : detector_shm()->roiLimits;
    int nretval = 0;
    ROI retval[MAX_ROIS];
    FILE_LOG(logDEBUG1) << "Sending ROI to detector" << narg;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        client.sendData(&fnum, sizeof(fnum));
        client.sendData(&narg, sizeof(narg));
        if (narg != -1) {
            for (int i = 0; i < narg; ++i) {
                client.sendData(&arg[i].xmin, sizeof(int));
                client.sendData(&arg[i].xmax, sizeof(int));
                client.sendData(&arg[i].ymin, sizeof(int));
                client.sendData(&arg[i].ymax, sizeof(int));
            }
        }
        client.receiveData(&ret, sizeof(ret));

        // handle ret
        if (ret == FAIL) {
            char mess[MAX_STR_LENGTH]{};
            client.receiveData(mess, MAX_STR_LENGTH);
            throw RuntimeError("Detector " + std::to_string(detId) +
                               " returned error: " + std::string(mess));
        } else {
            client.receiveData(&nretval, sizeof(nretval));
            int nrec = 0;
            for (int i = 0; i < nretval; ++i) {
                nrec += client.receiveData(&retval[i].xmin, sizeof(int));
                nrec += client.receiveData(&retval[i].xmax, sizeof(int));
                nrec += client.receiveData(&retval[i].ymin, sizeof(int));
                nrec += client.receiveData(&retval[i].ymax, sizeof(int));
            }
            detector_shm()->nROI = nretval;
            FILE_LOG(logDEBUG1) << "nRoi: " << nretval;
            for (int i = 0; i < nretval; ++i) {
                detector_shm()->roiLimits[i] = retval[i];
                FILE_LOG(logDEBUG1)
                    << "ROI [" << i << "] (" << detector_shm()->roiLimits[i].xmin << ","
                    << detector_shm()->roiLimits[i].xmax << "," << detector_shm()->roiLimits[i].ymin
                    << "," << detector_shm()->roiLimits[i].ymax << ")";
            }
        }
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    // old firmware requires configuremac after setting roi
    if (detector_shm()->myDetectorType == GOTTHARD && n != -1) {
        ret = configureMAC();
    }

    // update roi in receiver
    if (ret == OK && detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        fnum = F_RECEIVER_SET_ROI;
        ret = FAIL;
        narg = detector_shm()->nROI;
        arg = detector_shm()->roiLimits;
        FILE_LOG(logDEBUG1) << "Sending ROI to receiver: " << detector_shm()->nROI;

        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        receiver.sendData(&fnum, sizeof(fnum));
        receiver.sendData(&narg, sizeof(narg));
        if (narg != -1) {
            for (int i = 0; i < narg; ++i) {
                receiver.sendData(&arg[i].xmin, sizeof(int));
                receiver.sendData(&arg[i].xmax, sizeof(int));
                receiver.sendData(&arg[i].ymin, sizeof(int));
                receiver.sendData(&arg[i].ymax, sizeof(int));
            }
        }
        receiver.receiveData(&ret, sizeof(ret));

        if (ret == FAIL) {
            char mess[MAX_STR_LENGTH]{};
            receiver.receiveData(mess, MAX_STR_LENGTH);
            throw ReceiverError("Receiver " + std::to_string(detId) +
                                " returned error: " + std::string(mess));
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return ret;
}

int slsDetector::writeAdcRegister(int addr, int val) {
    int fnum = F_WRITE_ADC_REG;
    int ret = FAIL;
    uint32_t args[2] = {(uint32_t)addr, (uint32_t)val};
    FILE_LOG(logDEBUG1) << "Writing to ADC register 0x" << std::hex << addr << "data: 0x"
                        << std::hex << val << std::dec;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::activate(int const enable) {
    int fnum = F_ACTIVATE;
    int ret = FAIL;
    int arg = enable;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting activate flag to " << arg;

    // detector
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Activate: " << retval;
        detector_shm()->activated = retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }

    // receiver
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG && ret == OK) {
        fnum = F_RECEIVER_ACTIVATE;
        ret = FAIL;
        arg = detector_shm()->activated;
        retval = -1;
        FILE_LOG(logDEBUG1) << "Setting activate flag " << arg << " to receiver";

        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        if (ret == FORCE_UPDATE) {
            receiver.close();
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->activated;
}

int slsDetector::setDeactivatedRxrPaddingMode(int padding) {
    int fnum = F_RECEIVER_DEACTIVATED_PADDING_ENABLE;
    int ret = OK;
    int arg = padding;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable: " << arg;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable:" << retval;
        detector_shm()->receiver_deactivatedPaddingEnable = retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return detector_shm()->receiver_deactivatedPaddingEnable;
}

int slsDetector::getFlippedData(dimension d) const { return detector_shm()->flippedData[d]; }

int slsDetector::setFlippedData(dimension d, int value) {
    int fnum = F_SET_FLIPPED_DATA_RECEIVER;
    int ret = OK;
    int args[2] = {(int)d, value};
    int retval = -1;

    // flipped across y
    if (d == Y) {
        throw RuntimeError("Flipped across Y axis is not implemented");
    }

    // replace get with shm value (write to shm right away as it is a det value,
    // not rx value)
    if (value > -1) {
        detector_shm()->flippedData[d] = (value > 0) ? 1 : 0;
    }
    args[1] = detector_shm()->flippedData[d];
    FILE_LOG(logDEBUG1) << "Setting flipped data across axis " << d << " with value: " << value;
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Flipped data:" << retval << " ret: " << ret;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return detector_shm()->flippedData[d];
}

int slsDetector::setAllTrimbits(int val) {
    int fnum = F_SET_ALL_TRIMBITS;
    int ret = FAIL;
    int arg = val;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting all trimbits to " << val;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "All trimbit value: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::enableGapPixels(int val) {
    if (val >= 0) {
        int fnum = F_ENABLE_GAPPIXELS_IN_RECEIVER;
        int ret = OK;
        int arg = val;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Sending gap pixels enable to receiver: " << arg;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Gap pixels enable to receiver:" << retval;
            detector_shm()->gappixels = retval;

            // update databytes
            detector_shm()->dataBytesInclGapPixels = 0;
            if (detector_shm()->dynamicRange != 4) {
                detector_shm()->dataBytesInclGapPixels =
                    (detector_shm()->nChip[X] * detector_shm()->nChan[X] +
                     detector_shm()->gappixels * detector_shm()->nGappixels[X]) *
                    (detector_shm()->nChip[Y] * detector_shm()->nChan[Y] +
                     detector_shm()->gappixels * detector_shm()->nGappixels[Y]) *
                    detector_shm()->dynamicRange / 8;
            }
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->gappixels;
}

int slsDetector::setTrimEn(int nen, int *en) {
    if (en) {
        for (int ien = 0; ien < nen; ++ien) {
            detector_shm()->trimEnergies[ien] = en[ien];
        }
        detector_shm()->nTrimEn = nen;
    }
    return (detector_shm()->nTrimEn);
}

int slsDetector::getTrimEn(int *en) {
    if (en) {
        for (int ien = 0; ien < detector_shm()->nTrimEn; ++ien) {
            en[ien] = detector_shm()->trimEnergies[ien];
        }
    }
    return (detector_shm()->nTrimEn);
}

int slsDetector::pulsePixel(int n, int x, int y) {
    int fnum = F_PULSE_PIXEL;
    int ret = FAIL;
    int args[3] = {n, x, y};
    FILE_LOG(logDEBUG1) << "Pulsing pixel " << n << " number of times at (" << x << "," << y << ")";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::pulsePixelNMove(int n, int x, int y) {
    int fnum = F_PULSE_PIXEL_AND_MOVE;
    int ret = FAIL;
    int args[3] = {n, x, y};
    FILE_LOG(logDEBUG1) << "Pulsing pixel " << n << " number of times and move by delta (" << x
                        << "," << y << ")";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::pulseChip(int n) {
    int fnum = F_PULSE_CHIP;
    int ret = FAIL;
    int arg = n;
    FILE_LOG(logDEBUG1) << "Pulsing chip " << n << " number of times";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::setThresholdTemperature(int val) {
    int fnum = F_THRESHOLD_TEMP;
    int arg = val;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting threshold temperature to " << val;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        stop.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Threshold temperature: " << retval;
        // no updateDetector as it is stop server
    }
    return retval;
}

int slsDetector::setTemperatureControl(int val) {
    int fnum = F_TEMP_CONTROL;
    int arg = val;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting temperature control to " << val;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        stop.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Temperature control: " << retval;
        // no updateDetector as it is stop server
    }
    return retval;
}

int slsDetector::setTemperatureEvent(int val) {
    int fnum = F_TEMP_EVENT;
    int arg = val;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting temperature event to " << val;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
        stop.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Temperature event: " << retval;
        // no updateDetector as it is stop server
    }
    return retval;
}

int slsDetector::setStoragecellStart(int pos) {
    int fnum = F_STORAGE_CELL_START;
    int ret = FAIL;
    int arg = pos;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting storage cell start to " << pos;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Storage cell start: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::programFPGA(const std::string &fname) {
    // TODO! make exception safe!
    // now malloced memory can leak
    // only jungfrau implemented (client processing, so check now)
    if (detector_shm()->myDetectorType != JUNGFRAU &&
        detector_shm()->myDetectorType != CHIPTESTBOARD &&
        detector_shm()->myDetectorType != MOENCH) {
        throw RuntimeError("Program FPGA is not implemented for this detector");
    }
    FILE_LOG(logDEBUG1) << "Programming FPGA with file name:" << fname;
    size_t filesize = 0;
    char *fpgasrc = nullptr;

    // check if it exists
    {
        struct stat st;
        if (stat(fname.c_str(), &st)) {
            throw RuntimeError("Program FPGA: Programming file does not exist");
        }
    }

    {
        // open src
        FILE *src = fopen(fname.c_str(), "rb");
        if (src == nullptr) {
            throw RuntimeError("Program FPGA: Could not open source file for programming: " +
                               fname);
        }

        // create temp destination file
        char destfname[] = "/tmp/SLS_DET_MCB.XXXXXX";
        int dst = mkstemp(destfname); // create temporary file and open it in r/w
        if (dst == -1) {
            fclose(src);
            throw RuntimeError(std::string("Could not create destination file "
                                           "in /tmp for programming: ") +
                               destfname);
        }

        // convert src to dst rawbin
        FILE_LOG(logDEBUG1) << "Converting " << fname << " to " << destfname;
        {
            int filepos, x, y, i;
            // Remove header (0...11C)
            for (filepos = 0; filepos < 0x11C; ++filepos) {
                fgetc(src);
            }
            // Write 0x80 times 0xFF (0...7F)
            {
                char c = 0xFF;
                for (filepos = 0; filepos < 0x80; ++filepos) {
                    write(dst, &c, 1);
                }
            }
            // Swap bits and write to file
            for (filepos = 0x80; filepos < 0x1000000; ++filepos) {
                x = fgetc(src);
                if (x < 0) {
                    break;
                }
                y = 0;
                for (i = 0; i < 8; ++i) {
                    y = y | (((x & (1 << i)) >> i) << (7 - i)); // This swaps the bits
                }
                write(dst, &y, 1);
            }
            if (filepos < 0x1000000) {
                throw RuntimeError("Could not convert programming file. EOF "
                                   "before end of flash");
            }
        }
        if (fclose(src)) {
            throw RuntimeError("Program FPGA: Could not close source file");
        }
        if (close(dst)) {
            throw RuntimeError("Program FPGA: Could not close destination file");
        }
        FILE_LOG(logDEBUG1) << "File has been converted to " << destfname;

        // loading dst file to memory
        FILE *fp = fopen(destfname, "r");
        if (fp == nullptr) {
            throw RuntimeError("Program FPGA: Could not open rawbin file");
        }
        if (fseek(fp, 0, SEEK_END)) {
            throw RuntimeError("Program FPGA: Seek error in rawbin file");
        }
        filesize = ftell(fp);
        if (filesize <= 0) {
            throw RuntimeError("Program FPGA: Could not get length of rawbin file");
        }
        rewind(fp);
        fpgasrc = (char *)malloc(filesize + 1); //<------------------- MALLOC!
        if (fpgasrc == nullptr) {
            throw RuntimeError("Program FPGA: Could not allocate size of program");
        }
        if (fread(fpgasrc, sizeof(char), filesize, fp) != filesize) {
            free(fpgasrc);
            throw RuntimeError("Program FPGA: Could not read rawbin file");
        }

        if (fclose(fp)) {
            free(fpgasrc);
            throw RuntimeError("Program FPGA: Could not close destination file "
                               "after converting");
        }
        unlink(destfname); // delete temporary file
        FILE_LOG(logDEBUG1) << "Successfully loaded the rawbin file to program memory";
    }

    // send program from memory to detector
    int fnum = F_PROGRAM_FPGA;
    int ret = FAIL;
    char mess[MAX_STR_LENGTH] = {0};
    FILE_LOG(logDEBUG1) << "Sending programming binary to detector";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        client.sendData(&fnum, sizeof(fnum));
        client.sendData(&filesize, sizeof(filesize));
        client.receiveData(&ret, sizeof(ret));
        // opening error
        if (ret == FAIL) {
            client.receiveData(mess, sizeof(mess));
            free(fpgasrc);
            throw RuntimeError("Detector " + std::to_string(detId) +
                               " returned error: " + std::string(mess));
        }

        // erasing flash
        if (ret != FAIL) {
            FILE_LOG(logINFO) << "This can take awhile. Please be patient...";
            FILE_LOG(logINFO) << "Erasing Flash:";
            printf("%d%%\r", 0);
            std::cout << std::flush;
            // erasing takes 65 seconds, printing here (otherwise need threads
            // in server-unnecessary)
            const int ERASE_TIME = 65;
            int count = ERASE_TIME + 1;
            while (count > 0) {
                usleep(1 * 1000 * 1000);
                --count;
                printf("%d%%\r", (int)(((double)(ERASE_TIME - count) / ERASE_TIME) * 100));
                std::cout << std::flush;
            }
            printf("\n");
            FILE_LOG(logINFO) << "Writing to Flash:";
            printf("%d%%\r", 0);
            std::cout << std::flush;
        }

        // sending program in parts of 2mb each
        size_t unitprogramsize = 0;
        int currentPointer = 0;
        size_t totalsize = filesize;
        while (ret != FAIL && (filesize > 0)) {

            unitprogramsize = MAX_FPGAPROGRAMSIZE; // 2mb
            if (unitprogramsize > filesize) {      // less than 2mb
                unitprogramsize = filesize;
            }
            FILE_LOG(logDEBUG1) << "unitprogramsize:" << unitprogramsize
                                << "\t filesize:" << filesize;

            client.sendData(fpgasrc + currentPointer, unitprogramsize);
            client.receiveData(&ret, sizeof(ret));
            if (ret != FAIL) {
                filesize -= unitprogramsize;
                currentPointer += unitprogramsize;

                // print progress
                printf("%d%%\r", (int)(((double)(totalsize - filesize) / totalsize) * 100));
                std::cout << std::flush;
            } else {
                printf("\n");
                client.receiveData(mess, sizeof(mess));
                free(fpgasrc);
                throw RuntimeError("Detector returned error: " + std::string(mess));
            }
        }
        printf("\n");

        // check ending error
        if ((ret == FAIL) && (strstr(mess, "not implemented") == nullptr) &&
            (strstr(mess, "locked") == nullptr) && (strstr(mess, "-update") == nullptr)) {
            client.receiveData(&ret, sizeof(ret));
            if (ret == FAIL) {
                client.receiveData(mess, sizeof(mess));
                free(fpgasrc);
                throw RuntimeError("Detector returned error: " + std::string(mess));
            }
        }

        if (ret == FORCE_UPDATE) {
            updateDetector();
        }

        // remapping stop server
        if ((ret == FAIL) && (strstr(mess, "not implemented") == nullptr) &&
            (strstr(mess, "locked") == nullptr) && (strstr(mess, "-update") == nullptr)) {
            fnum = F_RESET_FPGA;
            int stopret = FAIL;
            auto stop = DetectorSocket(detector_shm()->hostname, detector_shm()->stopPort);
            stop.sendData(&fnum, sizeof(fnum));
            stop.receiveData(&stopret, sizeof(stopret));
            if (stopret == FAIL) {
                client.receiveData(mess, sizeof(mess));
                free(fpgasrc);
                throw RuntimeError("Detector returned error: " + std::string(mess));
            }
        }
    }
    FILE_LOG(logINFO) << "You can now restart the detector servers in normal mode.";
    free(fpgasrc);
    return ret;
}

int slsDetector::resetFPGA() {
    int fnum = F_RESET_FPGA;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Sending reset FPGA";
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::powerChip(int ival) {
    int fnum = F_POWER_CHIP;
    int ret = FAIL;
    int arg = ival;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting power chip to " << ival;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Power chip: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setAutoComparatorDisableMode(int ival) {
    int fnum = F_AUTO_COMP_DISABLE;
    int ret = FAIL;
    int arg = ival;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting auto comp disable mode to " << ival;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Auto comp disable: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::getChanRegs(double *retval) {
    int n = getTotalNumberOfChannels();
    // update chanregs
    sls_detector_module *myMod = getModule();

    if (myMod != nullptr) {
        // the original array has 0 initialized
        if (myMod->chanregs) {
            for (int i = 0; i < n; ++i) {
                retval[i] = (double)(myMod->chanregs[i] & TRIMBITMASK);
            }
        }
        deleteModule(myMod);
    }
    return n;
}

int slsDetector::setModule(sls_detector_module module, int tb) {
    int fnum = F_SET_MODULE;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting module with tb:" << tb;
    // to exclude trimbits
    if (!tb) {
        module.nchan = 0;
        module.nchip = 0;
    }

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        client.sendData(&fnum, sizeof(fnum));
        sendModule(&module, client);
        client.receiveData(&ret, sizeof(ret));
        if (ret == FAIL) {
            char mess[MAX_STR_LENGTH] = {0};
            client.receiveData(mess, sizeof(mess));
            throw RuntimeError("Detector " + std::to_string(detId) + " returned error: " + mess);
        }
        client.receiveData(&retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Set Module returned: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }

    // update client structure
    if (ret == OK) {
        if (module.eV != -1) {
            detector_shm()->currentThresholdEV = module.eV;
        }
    }
    return ret;
}

slsDetectorDefs::sls_detector_module *slsDetector::getModule() {
    int fnum = F_GET_MODULE;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Getting module";

    sls_detector_module *myMod = createModule();
    if (myMod == nullptr) {
        throw RuntimeError("Could not create module");
    }
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        receiveModule(myMod, client);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    if (ret == OK) {
        if (myMod->eV != -1) {
            detector_shm()->currentThresholdEV = myMod->eV;
        }
    }
    return myMod;
}

int slsDetector::setRateCorrection(int64_t t) {
    int fnum = F_SET_RATE_CORRECT;
    int ret = FAIL;
    int64_t arg = t;
    FILE_LOG(logDEBUG1) << "Setting Rate Correction to " << arg;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int64_t slsDetector::getRateCorrection() {
    int fnum = F_GET_RATE_CORRECT;
    int ret = FAIL;
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting rate correction";

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Rate correction: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

void slsDetector::printReceiverConfiguration(TLogLevel level) {
    FILE_LOG(level) << "#Detector " << detId << ":\n Receiver Hostname:\t" << getReceiverHostname()
                    << "\nDetector UDP IP (Source):\t\t" << getDetectorIP()
                    << "\nDetector UDP IP2 (Source):\t\t" << getDetectorIP2()
                    << "\nDetector UDP MAC:\t\t" << getDetectorMAC() << "\nDetector UDP MAC2:\t\t"
                    << getDetectorMAC2() << "\nReceiver UDP IP:\t" << getReceiverUDPIP()
                    << "\nReceiver UDP IP2:\t" << getReceiverUDPIP2() << "\nReceiver UDP MAC:\t"
                    << getReceiverUDPMAC() << "\nReceiver UDP MAC2:\t" << getReceiverUDPMAC2()
                    << "\nReceiver UDP Port:\t" << getReceiverUDPPort() << "\nReceiver UDP Port2:\t"
                    << getReceiverUDPPort2();
}

int slsDetector::setReceiverOnline(int value) {
    if (value != GET_ONLINE_FLAG) {
        // no receiver
        if (!strcmp(detector_shm()->receiver_hostname, "none")) {
            detector_shm()->receiverOnlineFlag = OFFLINE_FLAG;
        } else {
            detector_shm()->receiverOnlineFlag = OFFLINE_FLAG;

            // set online
            if (value == ONLINE_FLAG) {

                // connect and set offline flag
                auto receiver = ReceiverSocket(detector_shm()->receiver_hostname,
                                               detector_shm()->receiverTCPPort);
                receiver.close();
                detector_shm()->receiverOnlineFlag = ONLINE_FLAG;
                // check for version compatibility
                if (detector_shm()->receiverAPIVersion == 0) {
                    checkReceiverVersionCompatibility();
                }
            }
        }
    }
    return detector_shm()->receiverOnlineFlag;
}

int slsDetector::getReceiverOnline() const { return detector_shm()->receiverOnlineFlag; }

std::string slsDetector::checkReceiverOnline() {
    std::string retval;
    try {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        detector_shm()->receiverOnlineFlag = ONLINE_FLAG;
    } catch (...) {
        detector_shm()->receiverOnlineFlag = OFFLINE_FLAG;
        retval = detector_shm()->receiver_hostname;
    }
    return retval;
}

int slsDetector::lockReceiver(int lock) {
    int fnum = F_LOCK_RECEIVER;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting receiver server lock to " << lock;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &lock, sizeof(lock), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver Lock: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return retval;
}

std::string slsDetector::getReceiverLastClientIP() {
    int fnum = F_GET_LAST_RECEIVER_CLIENT_IP;
    int ret = FAIL;
    char retval[INET_ADDRSTRLEN] = {};
    FILE_LOG(logDEBUG1) << "Getting last client ip to receiver server";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Last client IP to receiver: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return retval;
}

int slsDetector::exitReceiver() {
    int fnum = F_EXIT_RECEIVER;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Sending exit command to receiver server";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        // no ret handling as ret never fail
        FILE_LOG(logINFO) << "Shutting down the receiver server";
    }
    return ret;
}

int slsDetector::execReceiverCommand(const std::string &cmd) {
    int fnum = F_EXEC_RECEIVER_COMMAND;
    int ret = FAIL;
    char arg[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};
    sls::strcpy_safe(arg, cmd.c_str());
    FILE_LOG(logDEBUG1) << "Sending command to receiver: " << arg;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, arg, sizeof(arg), retval, sizeof(retval));
        FILE_LOG(logINFO) << "Receiver " << detId << " returned:\n" << retval;
    }
    return ret;
}

int slsDetector::updateCachedReceiverVariables() const {
    int fnum = F_UPDATE_RECEIVER_CLIENT;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Sending update client to receiver server";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver = sls::ClientSocket(true, detector_shm()->receiver_hostname,
                                          detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        if (ret == FAIL) {
            std::string msg =
                "Could not update receiver: " + std::string(detector_shm()->receiver_hostname);
            throw RuntimeError(msg);
        } else {
            int n = 0, i32 = 0;
            char cstring[MAX_STR_LENGTH] = {};
            char lastClientIP[INET_ADDRSTRLEN] = {};

            n += receiver.receiveData(lastClientIP, sizeof(lastClientIP));
            FILE_LOG(logDEBUG1) << "Updating receiver last modified by " << lastClientIP;

            // filepath
            n += receiver.receiveData(cstring, sizeof(cstring));
            sls::strcpy_safe(detector_shm()->receiver_filePath, cstring);

            // filename
            n += receiver.receiveData(cstring, sizeof(cstring));
            sls::strcpy_safe(detector_shm()->receiver_fileName, cstring);

            // index
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_fileIndex = i32;

            // file format
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_fileFormatType = (fileFormat)i32;

            // frames per file
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_framesPerFile = i32;

            // frame discard policy
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_frameDiscardMode = (frameDiscardPolicy)i32;

            // frame padding
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_framePadding = i32;

            // file write enable
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_fileWriteEnable = i32;

            // file overwrite enable
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_overWriteEnable = i32;

            // gap pixels
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->gappixels = i32;

            // receiver read frequency
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_read_freq = i32;

            // receiver streaming port
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_zmqport = i32;

            // streaming source ip
            n += receiver.receiveData(cstring, sizeof(cstring));
            sls::strcpy_safe(detector_shm()->receiver_zmqip, cstring);

            // additional json header
            n += receiver.receiveData(cstring, sizeof(cstring));
            sls::strcpy_safe(detector_shm()->receiver_additionalJsonHeader, cstring);

            // receiver streaming enable
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_upstream = i32;

            // activate
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->activated = i32;

            // deactivated padding enable
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_deactivatedPaddingEnable = i32;

            // silent mode
            n += receiver.receiveData(&i32, sizeof(i32));
            detector_shm()->receiver_silentMode = i32;

            if (!n) {
                throw RuntimeError(
                    "Could not update receiver: " + std::string(detector_shm()->receiver_hostname) +
                    ", received 0 bytes\n");
            }
            return OK;
        }
    }
    return ret;
}

void slsDetector::sendMultiDetectorSize() {
    int fnum = F_SEND_RECEIVER_MULTIDETSIZE;
    int ret = FAIL;
    int args[2] = {detector_shm()->multiSize[0], detector_shm()->multiSize[1]};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending multi detector size to receiver: ("
                        << detector_shm()->multiSize[0] << "," << detector_shm()->multiSize[1]
                        << ")";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver multi size returned: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
}

void slsDetector::setDetectorId() {
    int fnum = F_SEND_RECEIVER_DETPOSID;
    int ret = FAIL;
    int arg = detId;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending detector pos id to receiver: " << detId;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver Position Id returned: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
}

void slsDetector::setDetectorHostname() {
    int fnum = F_SEND_RECEIVER_DETHOSTNAME;
    int ret = FAIL;
    char args[MAX_STR_LENGTH] = {};
    char retvals[MAX_STR_LENGTH] = {};
    sls::strcpy_safe(args, detector_shm()->hostname);
    FILE_LOG(logDEBUG1) << "Sending detector hostname to receiver: " << args;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));
        FILE_LOG(logDEBUG1) << "Receiver set detector hostname: " << retvals;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
}

std::string slsDetector::getFilePath() { return detector_shm()->receiver_filePath; }

std::string slsDetector::setFilePath(const std::string &path) {
    if (!path.empty()) {
        int fnum = F_SET_RECEIVER_FILE_PATH;
        int ret = FAIL;
        char args[MAX_STR_LENGTH] = {};
        char retvals[MAX_STR_LENGTH] = {};
        sls::strcpy_safe(args, path.c_str());
        FILE_LOG(logDEBUG1) << "Sending file path to receiver: " << args;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));
            FILE_LOG(logDEBUG1) << "Receiver file path: " << retvals;
            sls::strcpy_safe(detector_shm()->receiver_filePath, retvals);
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_filePath;
}

std::string slsDetector::getFileName() { return detector_shm()->receiver_fileName; }

std::string slsDetector::setFileName(const std::string &fname) {
    if (!fname.empty()) {
        int fnum = F_SET_RECEIVER_FILE_NAME;
        int ret = FAIL;
        char args[MAX_STR_LENGTH]{};
        char retvals[MAX_STR_LENGTH]{};
        sls::strcpy_safe(args, fname.c_str());
        FILE_LOG(logDEBUG1) << "Sending file name to receiver: " << args;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));
            FILE_LOG(logDEBUG1) << "Receiver file name: " << retvals;
            sls::strcpy_safe(detector_shm()->receiver_fileName, retvals);
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_fileName;
}

int slsDetector::setReceiverFramesPerFile(int f) {
    if (f >= 0) {
        int fnum = F_SET_RECEIVER_FRAMES_PER_FILE;
        int ret = FAIL;
        int arg = f;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Setting receiver frames per file to " << arg;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver frames per file: " << retval;
            detector_shm()->receiver_framesPerFile = retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_framesPerFile;
}

slsDetectorDefs::frameDiscardPolicy
slsDetector::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
    int fnum = F_RECEIVER_DISCARD_POLICY;
    int ret = FAIL;
    int arg = static_cast<int>(f);
    auto retval = static_cast<frameDiscardPolicy>(-1);
    FILE_LOG(logDEBUG1) << "Setting receiver frames discard policy to " << arg;
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver frames discard policy: " << retval;
        detector_shm()->receiver_frameDiscardMode = retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return detector_shm()->receiver_frameDiscardMode;
}

int slsDetector::setReceiverPartialFramesPadding(int f) {
    int fnum = F_RECEIVER_PADDING_ENABLE;
    int ret = FAIL;
    int arg = f;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting receiver partial frames enable to " << arg;
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver partial frames enable: " << retval;
        detector_shm()->receiver_framePadding = retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return detector_shm()->receiver_framePadding;
}

slsDetectorDefs::fileFormat slsDetector::setFileFormat(fileFormat f) {
    if (f != GET_FILE_FORMAT) {
        int fnum = F_SET_RECEIVER_FILE_FORMAT;
        int ret = FAIL;
        int arg = f;
        auto retval = (fileFormat)-1;
        FILE_LOG(logDEBUG1) << "Setting receiver file format to " << arg;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver file format: " << retval;
            detector_shm()->receiver_fileFormatType = retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return getFileFormat();
}

slsDetectorDefs::fileFormat slsDetector::getFileFormat() {
    return detector_shm()->receiver_fileFormatType;
}

int slsDetector::getFileIndex() { return detector_shm()->receiver_fileIndex; }

int slsDetector::setFileIndex(int i) {
    if (i >= 0) {
        int fnum = F_SET_RECEIVER_FILE_INDEX;
        int ret = FAIL;
        int arg = i;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Setting file index to " << arg;
        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver file index: " << retval;
            detector_shm()->receiver_fileIndex = retval;
        }
        if (ret == FORCE_UPDATE) {
            updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_fileIndex;
}

int slsDetector::incrementFileIndex() {
    if (detector_shm()->receiver_fileWriteEnable) {
        return setFileIndex(detector_shm()->receiver_fileIndex + 1);
    }
    return detector_shm()->receiver_fileIndex;
}

int slsDetector::startReceiver() {
    int fnum = F_START_RECEIVER;
    int ret = FAIL;
    // char mess[MAX_STR_LENGTH]{};
    FILE_LOG(logDEBUG1) << "Starting Receiver";
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        updateCachedReceiverVariables();
    }
    return ret;
}

int slsDetector::stopReceiver() {
    int fnum = F_STOP_RECEIVER;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Stopping Receiver";
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return ret;
}

slsDetectorDefs::runStatus slsDetector::getReceiverStatus() {
    int fnum = F_GET_RECEIVER_STATUS;
    int ret = FAIL;
    runStatus retval = ERROR;
    FILE_LOG(logDEBUG1) << "Getting Receiver Status";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver Status: " << runStatusType(retval);
    }
    if (ret == FORCE_UPDATE) {
        updateCachedReceiverVariables(); // Do we need to handle this ret?
    }
    return retval;
}

int slsDetector::getFramesCaughtByReceiver() {
    int fnum = F_GET_RECEIVER_FRAMES_CAUGHT;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Frames Caught by Receiver";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Frames Caught by Receiver: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        updateCachedReceiverVariables();
    }
    return retval;
}

int slsDetector::getReceiverCurrentFrameIndex() {
    int fnum = F_GET_RECEIVER_FRAME_INDEX;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Current Frame Index of Receiver";
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Current Frame Index of Receiver: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        updateCachedReceiverVariables();
    }
    return retval;
}

int slsDetector::resetFramesCaught() {
    int fnum = F_RESET_RECEIVER_FRAMES_CAUGHT;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Reset Frames Caught by Receiver";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        updateCachedReceiverVariables();
    }
    return ret;
}

int slsDetector::enableWriteToFile(int enable) {
    if (enable >= 0) {
        int fnum = F_ENABLE_RECEIVER_FILE_WRITE;
        int ret = FAIL;
        int arg = enable;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Sending enable file write to receiver: " << arg;
        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver file write enable: " << retval;
            detector_shm()->receiver_fileWriteEnable = retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_fileWriteEnable;
}

int slsDetector::overwriteFile(int enable) {
    if (enable >= 0) {
        int fnum = F_ENABLE_RECEIVER_OVERWRITE;
        int ret = FAIL;
        int arg = enable;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Sending enable file overwrite to receiver: " << arg;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver file overwrite enable: " << retval;
            detector_shm()->receiver_overWriteEnable = retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_overWriteEnable;
}

int slsDetector::setReceiverStreamingFrequency(int freq) {
    if (freq >= 0) {
        int fnum = F_RECEIVER_STREAMING_FREQUENCY;
        int ret = FAIL;
        int arg = freq;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Sending read frequency to receiver: " << arg;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver read frequency: " << retval;
            detector_shm()->receiver_read_freq = retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_read_freq;
}

int slsDetector::setReceiverStreamingTimer(int time_in_ms) {
    int fnum = F_RECEIVER_STREAMING_TIMER;
    int ret = FAIL;
    int arg = time_in_ms;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending read timer to receiver: " << arg;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver read timer: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return retval;
}

int slsDetector::enableDataStreamingFromReceiver(int enable) {
    if (enable >= 0) {
        int fnum = F_STREAM_DATA_FROM_RECEIVER;
        int ret = FAIL;
        int arg = enable;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Sending Data Streaming to receiver: " << arg;

        if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
            auto receiver =
                ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
            ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Receiver Data Streaming: " << retval;
            detector_shm()->receiver_upstream = retval;
        }
        if (ret == FORCE_UPDATE) {
            ret = updateCachedReceiverVariables();
        }
    }
    return detector_shm()->receiver_upstream;
}

int slsDetector::enableTenGigabitEthernet(int i) {
    int fnum = F_ENABLE_TEN_GIGA;
    int ret = FAIL;
    int arg = i;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Enabling / Disabling 10Gbe: " << arg;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "10Gbe: " << retval;
        detector_shm()->tenGigaEnable = retval;
        client.close();
        if (ret == FORCE_UPDATE) {
            ret = updateDetector();
        }
        ret = configureMAC();
    }

    // receiver
    if ((detector_shm()->receiverOnlineFlag == ONLINE_FLAG) && ret == OK) {
        fnum = F_ENABLE_RECEIVER_TEN_GIGA;
        ret = FAIL;
        arg = detector_shm()->tenGigaEnable;
        retval = -1;
        FILE_LOG(logDEBUG1) << "Sending 10Gbe enable to receiver: " << arg;
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver 10Gbe enable: " << retval;
        if (ret == FORCE_UPDATE) {
            receiver.close();
            updateCachedReceiverVariables();
        }
    }
    return detector_shm()->tenGigaEnable;
}

int slsDetector::setReceiverFifoDepth(int i) {
    int fnum = F_SET_RECEIVER_FIFO_DEPTH;
    int ret = FAIL;
    int arg = i;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending Receiver Fifo Depth: " << arg;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver Fifo Depth: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        updateCachedReceiverVariables();
    }
    return retval;
}

int slsDetector::setReceiverSilentMode(int i) {
    int fnum = F_SET_RECEIVER_SILENT_MODE;
    int ret = FAIL;
    int arg = i;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending Receiver Silent Mode: " << arg;

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver Data Streaming: " << retval;
        detector_shm()->receiver_silentMode = retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return detector_shm()->receiver_silentMode;
}

int slsDetector::restreamStopFromReceiver() {
    int fnum = F_RESTREAM_STOP_FROM_RECEIVER;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Restream stop dummy from Receiver via zmq";
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto receiver =
            ReceiverSocket(detector_shm()->receiver_hostname, detector_shm()->receiverTCPPort);
        ret = receiver.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
    }
    if (ret == FORCE_UPDATE) {
        ret = updateCachedReceiverVariables();
    }
    return ret;
}

int slsDetector::setPattern(const std::string &fname) {
    uint64_t word;
    int addr = 0;
    FILE *fd = fopen(fname.c_str(), "r");
    if (fd != nullptr) {
        while (fread(&word, sizeof(word), 1, fd)) {
            setPatternWord(addr, word); // TODO! (Erik) do we need to send
                                        // pattern in 64bit chunks?
            ++addr;
        }
        fclose(fd);
    } else {
        return -1;
    }
    return addr;
}

uint64_t slsDetector::setPatternWord(int addr, uint64_t word) {
    int fnum = F_SET_PATTERN;
    int ret = FAIL;
    int mode = 0; // sets word
    uint64_t args[3] = {(uint64_t)mode, (uint64_t)addr, word};
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Setting Pattern word, addr: 0x" << std::hex << addr << ", word: 0x"
                        << word << std::dec;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Set Pattern word: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setPatternLoops(int level, int &start, int &stop, int &n) {
    int fnum = F_SET_PATTERN;
    int ret = FAIL;
    int mode = 1; // sets loop
    uint64_t args[5] = {(uint64_t)mode, (uint64_t)level, (uint64_t)start, (uint64_t)stop,
                        (uint64_t)n};
    int retvals[3] = {0, 0, 0};
    FILE_LOG(logDEBUG1) << "Setting Pat Loops, level: " << level << ", start: " << start
                        << ", stop: " << stop << ", n: " << n;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), retvals, sizeof(retvals));
        FILE_LOG(logDEBUG1) << "Set Pat Loops: " << retvals[0] << ", " << retvals[1] << ", "
                            << retvals[2];
        start = retvals[0];
        stop = retvals[1];
        n = retvals[2];
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

int slsDetector::setPatternWaitAddr(uint64_t level, uint64_t addr) {
    int fnum = F_SET_PATTERN;
    int ret = FAIL;
    uint64_t mode = 2; // sets loop
    int retval = -1;
    std::array<uint64_t, 3> args{mode, level, addr};
    FILE_LOG(logDEBUG1) << "Setting Pat Wait Addr, "
                           "level: "
                        << level << ", addr: 0x" << std::hex << addr << std::dec;

    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args.data(), sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Set Pat Wait Addr: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

uint64_t slsDetector::setPatternWaitTime(uint64_t level, uint64_t t) {
    int fnum = F_SET_PATTERN;
    int ret = FAIL;
    uint64_t mode = 3;    // sets loop
    uint64_t retval = -1; // TODO! is this what we want?
    std::array<uint64_t, 3> args{mode, level, t};
    FILE_LOG(logDEBUG1) << "Setting Pat Wait Time, level: " << level << ", t: " << t;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args.data(), sizeof(args), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Set Pat Wait Time: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setPatternMask(uint64_t mask) {
    int fnum = F_SET_PATTERN_MASK;
    int ret = FAIL;
    uint64_t arg = mask;
    FILE_LOG(logDEBUG1) << "Setting Pattern Mask " << std::hex << mask << std::dec;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);
        FILE_LOG(logDEBUG1) << "Pattern Mask successful";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

uint64_t slsDetector::getPatternMask() {
    int fnum = F_GET_PATTERN_MASK;
    int ret = FAIL;
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Pattern Mask ";
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Pattern Mask:" << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setPatternBitMask(uint64_t mask) {
    int fnum = F_SET_PATTERN_BIT_MASK;
    int ret = FAIL;
    uint64_t arg = mask;
    FILE_LOG(logDEBUG1) << "Setting Pattern Bit Mask " << std::hex << mask << std::dec;
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), nullptr, 0);
        FILE_LOG(logDEBUG1) << "Pattern Bit Mask successful";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

uint64_t slsDetector::getPatternBitMask() {
    int fnum = F_GET_PATTERN_BIT_MASK;
    int ret = FAIL;
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Pattern Bit Mask ";
    if (detector_shm()->onlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, nullptr, 0, &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Pattern Bit Mask:" << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setLEDEnable(int enable) {
    int fnum = F_LED;
    int ret = FAIL;
    int arg = enable;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending LED Enable: " << arg;
    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "LED Enable: " << retval;
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return retval;
}

int slsDetector::setDigitalIODelay(uint64_t pinMask, int delay) {
    int fnum = F_DIGITAL_IO_DELAY;
    int ret = FAIL;
    uint64_t args[2] = {pinMask, (uint64_t)delay};
    FILE_LOG(logDEBUG1) << "Sending Digital IO Delay, pin mask: " << std::hex << args[0]
                        << ", delay: " << std::dec << args[1] << " ps";

    if (detector_shm()->receiverOnlineFlag == ONLINE_FLAG) {
        auto client = DetectorSocket(detector_shm()->hostname, detector_shm()->controlPort);
        ret = client.sendCommandThenRead(fnum, args, sizeof(args), nullptr, 0);
        FILE_LOG(logDEBUG1) << "Digital IO Delay successful";
    }
    if (ret == FORCE_UPDATE) {
        ret = updateDetector();
    }
    return ret;
}

slsDetectorDefs::sls_detector_module *slsDetector::interpolateTrim(sls_detector_module *a,
                                                                   sls_detector_module *b,
                                                                   const int energy, const int e1,
                                                                   const int e2, int tb) {

    // only implemented for eiger currently (in terms of which dacs)
    if (detector_shm()->myDetectorType != EIGER) {
        throw NotImplementedError(
            "Interpolation of Trim values not implemented for this detector!");
    }

    sls_detector_module *myMod = createModule(detector_shm()->myDetectorType);
    if (myMod == nullptr) {
        throw RuntimeError("Could not create module");
    }
    enum eiger_DacIndex {
        SVP,
        VTR,
        VRF,
        VRS,
        SVN,
        VTGSTV,
        VCMP_LL,
        VCMP_LR,
        CAL,
        VCMP_RL,
        RXB_RB,
        RXB_LB,
        VCMP_RR,
        VCP,
        VCN,
        VIS
    };

    // Copy other dacs
    int dacs_to_copy[] = {SVP, VTR, SVN, VTGSTV, RXB_RB, RXB_LB, VCN, VIS};
    int num_dacs_to_copy = sizeof(dacs_to_copy) / sizeof(dacs_to_copy[0]);
    for (int i = 0; i < num_dacs_to_copy; ++i) {
        if (a->dacs[dacs_to_copy[i]] != b->dacs[dacs_to_copy[i]]) {
            deleteModule(myMod);
            return nullptr;
        }
        myMod->dacs[dacs_to_copy[i]] = a->dacs[dacs_to_copy[i]];
    }

    // Copy irrelevant dacs (without failing): CAL
    if (a->dacs[CAL] != b->dacs[CAL]) {
        FILE_LOG(logWARNING) << "DAC CAL differs in both energies "
                                "("
                             << a->dacs[CAL] << "," << b->dacs[CAL]
                             << ")!\n"
                                "Taking first: "
                             << a->dacs[CAL];
    }
    myMod->dacs[CAL] = a->dacs[CAL];

    // Interpolate vrf, vcmp, vcp
    int dacs_to_interpolate[] = {VRF, VCMP_LL, VCMP_LR, VCMP_RL, VCMP_RR, VCP, VRS};
    int num_dacs_to_interpolate = sizeof(dacs_to_interpolate) / sizeof(dacs_to_interpolate[0]);
    for (int i = 0; i < num_dacs_to_interpolate; ++i) {
        myMod->dacs[dacs_to_interpolate[i]] = linearInterpolation(
            energy, e1, e2, a->dacs[dacs_to_interpolate[i]], b->dacs[dacs_to_interpolate[i]]);
    }

    // Interpolate all trimbits
    if (tb) {
        for (int i = 0; i < myMod->nchan; ++i) {
            myMod->chanregs[i] =
                linearInterpolation(energy, e1, e2, a->chanregs[i], b->chanregs[i]);
        }
    }
    return myMod;
}

slsDetectorDefs::sls_detector_module *
slsDetector::readSettingsFile(const std::string &fname, sls_detector_module *myMod, int tb) {

    FILE_LOG(logDEBUG1) << "Read settings file " << fname;
    bool modCreated = false; // If we create a module it must be deleted, TODO! usre RAII
    if (myMod == nullptr) {
        myMod = createModule(detector_shm()->myDetectorType);
        if (myMod == nullptr) {
            throw RuntimeError("Could not create module");
        }
        modCreated = true;
    }

    std::vector<std::string> names;
    switch (detector_shm()->myDetectorType) {
    case GOTTHARD:
        names.emplace_back("Vref");
        names.emplace_back("VcascN");
        names.emplace_back("VcascP");
        names.emplace_back("Vout");
        names.emplace_back("Vcasc");
        names.emplace_back("Vin");
        names.emplace_back("Vref_comp");
        names.emplace_back("Vib_test");
        break;
    case EIGER:
        break;
    case JUNGFRAU:
        names.emplace_back("VDAC0");
        names.emplace_back("VDAC1");
        names.emplace_back("VDAC2");
        names.emplace_back("VDAC3");
        names.emplace_back("VDAC4");
        names.emplace_back("VDAC5");
        names.emplace_back("VDAC6");
        names.emplace_back("VDAC7");
        break;
    default:
        if (modCreated) {
            if (myMod) {
                deleteModule(myMod);
            }
        }
        throw RuntimeError("Unknown detector type - unknown format for settings file");
    }

    // open file
    std::ifstream infile;
    if (detector_shm()->myDetectorType == EIGER) {
        infile.open(fname.c_str(), std::ifstream::binary);
    } else {
        infile.open(fname.c_str(), std::ios_base::in);
    }
    if (!infile.is_open()) {
        if (modCreated) {
            deleteModule(myMod);
        }
        throw RuntimeError("Could not open settings file for reading: " + fname);
    }

    // eiger
    if (detector_shm()->myDetectorType == EIGER) {
        bool allread = false;
        infile.read((char *)myMod->dacs, sizeof(int) * (myMod->ndac));
        if (infile.good()) {
            infile.read((char *)&myMod->iodelay, sizeof(myMod->iodelay));
            if (infile.good()) {
                infile.read((char *)&myMod->tau, sizeof(myMod->tau));
                if (tb) {
                    if (infile.good()) {
                        infile.read((char *)myMod->chanregs, sizeof(int) * (myMod->nchan));
                        if (infile) {
                            allread = true;
                        }
                    }
                } else if (infile) {
                    allread = true;
                }
            }
        }
        if (!allread) {
            if (modCreated) {
                deleteModule(myMod);
            }
            infile.close();
            throw RuntimeError("readSettingsFile: Could not load all values "
                               "for settings for " +
                               fname);
        }
        for (int i = 0; i < myMod->ndac; ++i) {
            FILE_LOG(logDEBUG1) << "dac " << i << ":" << myMod->dacs[i];
        }
        FILE_LOG(logDEBUG1) << "iodelay:" << myMod->iodelay;
        FILE_LOG(logDEBUG1) << "tau:" << myMod->tau;
    }

    // gotthard, jungfrau
    else {
        size_t idac = 0;
        std::string str;
        while (infile.good()) {
            getline(infile, str);
            if (str.empty()) {
                break;
            }
            FILE_LOG(logDEBUG1) << str;
            std::string sargname;
            int ival = 0;
            std::istringstream ssstr(str);
            ssstr >> sargname >> ival;
            bool found = false;
            for (size_t i = 0; i < names.size(); ++i) {
                if (sargname == names[i]) {
                    myMod->dacs[i] = ival;
                    found = true;
                    FILE_LOG(logDEBUG1) << names[i] << "(" << i << "): " << ival;
                    ++idac;
                }
            }
            if (!found) {
                throw RuntimeError("readSettingsFile: Unknown dac: " + sargname);
                if (modCreated) {
                    deleteModule(myMod);
                }
                infile.close();
            }
        }
        // not all read
        if (idac != names.size()) {
            if (modCreated) {
                deleteModule(myMod);
            }
            infile.close();
            throw RuntimeError("Could read only " + std::to_string(idac) + " dacs. Expected " +
                               std::to_string(names.size()) + " dacs");
        }
    }

    infile.close();
    FILE_LOG(logINFO) << "Settings file loaded: " << fname.c_str();
    return myMod;
}

int slsDetector::writeSettingsFile(const std::string &fname, sls_detector_module &mod) {

    FILE_LOG(logDEBUG1) << "Write settings file " << fname;

    std::vector<std::string> names;
    switch (detector_shm()->myDetectorType) {
    case GOTTHARD:
        names.emplace_back("Vref");
        names.emplace_back("VcascN");
        names.emplace_back("VcascP");
        names.emplace_back("Vout");
        names.emplace_back("Vcasc");
        names.emplace_back("Vin");
        names.emplace_back("Vref_comp");
        names.emplace_back("Vib_test");
        break;
    case EIGER:
        break;
    case JUNGFRAU:
        names.emplace_back("VDAC0");
        names.emplace_back("VDAC1");
        names.emplace_back("VDAC2");
        names.emplace_back("VDAC3");
        names.emplace_back("VDAC4");
        names.emplace_back("VDAC5");
        names.emplace_back("VDAC6");
        names.emplace_back("VDAC7");
        names.emplace_back("VDAC8");
        names.emplace_back("VDAC9");
        names.emplace_back("VDAC10");
        names.emplace_back("VDAC11");
        names.emplace_back("VDAC12");
        names.emplace_back("VDAC13");
        names.emplace_back("VDAC14");
        names.emplace_back("VDAC15");
        break;
    default:
        throw RuntimeError("Unknown detector type - unknown format for settings file");
    }

    // open file
    std::ofstream outfile;
    if (detector_shm()->myDetectorType == EIGER) {
        outfile.open(fname.c_str(), std::ofstream::binary);
    } else {
        outfile.open(fname.c_str(), std::ios_base::out);
    }
    if (!outfile.is_open()) {
        throw RuntimeError("Could not open settings file for writing: " + fname);
    }

    // eiger
    if (detector_shm()->myDetectorType == EIGER) {
        for (int i = 0; i < mod.ndac; ++i) {
            FILE_LOG(logINFO) << "dac " << i << ":" << mod.dacs[i];
        }
        FILE_LOG(logINFO) << "iodelay: " << mod.iodelay;
        FILE_LOG(logINFO) << "tau: " << mod.tau;

        outfile.write(reinterpret_cast<char *>(mod.dacs), sizeof(int) * (mod.ndac));
        outfile.write(reinterpret_cast<char *>(&mod.iodelay), sizeof(mod.iodelay));
        outfile.write(reinterpret_cast<char *>(&mod.tau), sizeof(mod.tau));
        outfile.write(reinterpret_cast<char *>(mod.chanregs), sizeof(int) * (mod.nchan));
    }

    // gotthard, jungfrau
    else {
        for (int i = 0; i < mod.ndac; ++i) {
            FILE_LOG(logDEBUG1) << "dac " << i << ": " << mod.dacs[i];
            outfile << names[i] << " " << mod.dacs[i] << std::endl;
        }
    }

    outfile.close();
    return OK;
}
