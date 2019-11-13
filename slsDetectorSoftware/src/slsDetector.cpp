#include "slsDetector.h"
#include "ClientSocket.h"
#include "SharedMemory.h"
#include "file_utils.h"
#include "network_utils.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"
#include "versionAPI.h"
#include "ToString.h"

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

#include <algorithm>

using namespace sls;

// create shm
slsDetector::slsDetector(detectorType type, int multi_id, int det_id,
                         bool verify)
    : detId(det_id), shm(multi_id, det_id) {

    // ensure shared memory was not created before
    if (shm.IsExisting()) {
        FILE_LOG(logWARNING) << "This shared memory should have been "
                                "deleted before! "
                             << shm.GetName() << ". Freeing it again";
        shm.RemoveSharedMemory();
    }

    initSharedMemory(type, multi_id, verify);
}

// pick up from shm
slsDetector::slsDetector(int multi_id, int det_id, bool verify)
    : detId(det_id), shm(multi_id, det_id) {

    // getDetectorType From shm will check if it was already existing
    detectorType type = getDetectorTypeFromShm(multi_id, verify);
    initSharedMemory(type, multi_id, verify);
}

slsDetector::~slsDetector() = default;

bool slsDetector::isFixedPatternSharedMemoryCompatible() {
    return (shm()->shmversion >= SLS_SHMAPIVERSION);
}

void slsDetector::checkDetectorVersionCompatibility() {
    int fnum = F_CHECK_VERSION;
    int64_t arg = 0;

    // get api version number for detector server
    switch (shm()->myDetectorType) {
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
    case MYTHEN3:
        arg = APIMYTHEN3;
        break;
    case GOTTHARD2:
        arg = APIGOTTHARD2;
        break;        
    default:
        throw NotImplementedError(
            "Check version compatibility is not implemented for this detector");
    }
    FILE_LOG(logDEBUG1)
        << "Checking version compatibility with detector with value "
        << std::hex << arg << std::dec;

    sendToDetector(fnum, arg, nullptr);
    sendToDetectorStop(fnum, arg, nullptr);
}

void slsDetector::checkReceiverVersionCompatibility() {
    // TODO! Verify that this works as intended when version don't match
    int64_t arg = APIRECEIVER;
    FILE_LOG(logDEBUG1)
        << "Checking version compatibility with receiver with value "
        << std::hex << arg << std::dec;
    sendToReceiver(F_RECEIVER_CHECK_VERSION, arg, nullptr);
}

int64_t slsDetector::getFirmwareVersion() {
    int64_t retval = -1;
    sendToDetector(F_GET_FIRMWARE_VERSION, nullptr, retval);
    FILE_LOG(logDEBUG1) << "firmware version: 0x" << std::hex << retval << std::dec;
    return retval;
}

int64_t slsDetector::getDetectorServerVersion() {
    int64_t retval = -1;
    sendToDetector(F_GET_SERVER_VERSION, nullptr, retval);
    FILE_LOG(logDEBUG1) << "firmware version: 0x" << std::hex << retval << std::dec;
    return retval;
}

int64_t slsDetector::getSerialNumber() {
    int64_t retval = -1;
    sendToDetector(F_GET_SERIAL_NUMBER, nullptr, retval);
    FILE_LOG(logDEBUG1) << "firmware version: 0x" << std::hex << retval << std::dec;
    return retval;
}


int64_t slsDetector::getReceiverSoftwareVersion() const {
    FILE_LOG(logDEBUG1) << "Getting receiver software version";
    int64_t retval = -1;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_GET_RECEIVER_VERSION, nullptr, retval);
    }
    return retval;
}

void slsDetector::sendToDetector(int fnum, const void *args, size_t args_size,
                                 void *retval, size_t retval_size) {
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    auto ret =
        client.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    client.close();
    if (ret == FORCE_UPDATE) {
        updateCachedDetectorVariables();
    }
}

template <typename Arg, typename Ret>
void slsDetector::sendToDetector(int fnum, const Arg &args, Ret &retval) {
    sendToDetector(fnum, &args, sizeof(args), &retval, sizeof(retval));
}

template <typename Arg>
void slsDetector::sendToDetector(int fnum, const Arg &args, std::nullptr_t) {
    sendToDetector(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Ret>
void slsDetector::sendToDetector(int fnum, std::nullptr_t, Ret &retval) {
    sendToDetector(fnum, nullptr, 0, &retval, sizeof(retval));
}

void slsDetector::sendToDetector(int fnum) {
    sendToDetector(fnum, nullptr, 0, nullptr, 0);
}

void slsDetector::sendToDetectorStop(int fnum, const void *args,
                                     size_t args_size, void *retval,
                                     size_t retval_size) {
    static_cast<const slsDetector &>(*this).sendToDetectorStop(
        fnum, args, args_size, retval, retval_size);
}

void slsDetector::sendToDetectorStop(int fnum, const void *args,
                                     size_t args_size, void *retval,
                                     size_t retval_size) const {
    auto stop = DetectorSocket(shm()->hostname, shm()->stopPort);
    stop.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    stop.close();
}

template <typename Arg, typename Ret>
void slsDetector::sendToDetectorStop(int fnum, const Arg &args, Ret &retval) {
    sendToDetectorStop(fnum, &args, sizeof(args), &retval, sizeof(retval));
}

template <typename Arg, typename Ret>
void slsDetector::sendToDetectorStop(int fnum, const Arg &args,
                                     Ret &retval) const {
    sendToDetectorStop(fnum, &args, sizeof(args), &retval, sizeof(retval));
}

template <typename Arg>
void slsDetector::sendToDetectorStop(int fnum, const Arg &args,
                                     std::nullptr_t) {
    sendToDetectorStop(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void slsDetector::sendToDetectorStop(int fnum, const Arg &args,
                                     std::nullptr_t) const {
    sendToDetectorStop(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Ret>
void slsDetector::sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval) {
    sendToDetectorStop(fnum, nullptr, 0, &retval, sizeof(retval));
}

template <typename Ret>
void slsDetector::sendToDetectorStop(int fnum, std::nullptr_t,
                                     Ret &retval) const {
    sendToDetectorStop(fnum, nullptr, 0, &retval, sizeof(retval));
}

void slsDetector::sendToDetectorStop(int fnum) {
    sendToDetectorStop(fnum, nullptr, 0, nullptr, 0);
}

void slsDetector::sendToDetectorStop(int fnum) const {
    sendToDetectorStop(fnum, nullptr, 0, nullptr, 0);
}

void slsDetector::sendToReceiver(int fnum, const void *args, size_t args_size,
                                 void *retval, size_t retval_size) {
    static_cast<const slsDetector &>(*this).sendToReceiver(
        fnum, args, args_size, retval, retval_size);
}

void slsDetector::sendToReceiver(int fnum, const void *args, size_t args_size,
                                 void *retval, size_t retval_size) const {
    auto receiver = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    receiver.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    receiver.close();
}

template <typename Arg, typename Ret>
void slsDetector::sendToReceiver(int fnum, const Arg &args, Ret &retval) {
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
}

template <typename Arg, typename Ret>
void slsDetector::sendToReceiver(int fnum, const Arg &args, Ret &retval) const {
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
}

template <typename Arg>
void slsDetector::sendToReceiver(int fnum, const Arg &args, std::nullptr_t) {
    sendToReceiver(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void slsDetector::sendToReceiver(int fnum, const Arg &args,
                                 std::nullptr_t) const {
    sendToReceiver(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Ret>
void slsDetector::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) {
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
}

template <typename Ret>
void slsDetector::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) const {
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
}

void slsDetector::sendToReceiver(int fnum) {
    sendToReceiver(fnum, nullptr, 0, nullptr, 0);
}

void slsDetector::sendToReceiver(int fnum) const {
    sendToReceiver(fnum, nullptr, 0, nullptr, 0);
}

void slsDetector::freeSharedMemory() {
    if (shm.IsExisting()) {
        shm.RemoveSharedMemory();
    }
}

void slsDetector::setHostname(const std::string &hostname) {
    sls::strcpy_safe(shm()->hostname, hostname.c_str());
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.close();

    FILE_LOG(logINFO) << "Checking Detector Version Compatibility";
    checkDetectorVersionCompatibility();

    FILE_LOG(logINFO) << "Detector connecting - updating!";
    updateCachedDetectorVariables();
}

std::string slsDetector::getHostname() const { return shm()->hostname; }

void slsDetector::initSharedMemory(detectorType type, int multi_id,
                                   bool verify) {
    shm = SharedMemory<sharedSlsDetector>(multi_id, detId);
    if (!shm.IsExisting()) {
        shm.CreateSharedMemory();
        initializeDetectorStructure(type);
    } else {
        shm.OpenSharedMemory();
        if (verify && shm()->shmversion != SLS_SHMVERSION) {
            std::ostringstream ss;
            ss << "Single shared memory (" << multi_id << "-" << detId
               << ":) version mismatch (expected 0x" << std::hex
               << SLS_SHMVERSION << " but got 0x" << shm()->shmversion << ")"
               << std::dec << ". Clear Shared memory to continue.";
            throw SharedMemoryError(ss.str());
        }
    }
}

void slsDetector::initializeDetectorStructure(detectorType type) {
    shm()->shmversion = SLS_SHMVERSION;
    memset(shm()->hostname, 0, MAX_STR_LENGTH);
    shm()->myDetectorType = type;
    shm()->multiSize.x = 0;
    shm()->multiSize.y = 0;
    shm()->controlPort = DEFAULT_PORTNO;
    shm()->stopPort = DEFAULT_PORTNO + 1;
    sls::strcpy_safe(shm()->settingsDir, getenv("HOME"));
    shm()->roi.xmin = -1;
    shm()->roi.xmax = -1;
    shm()->adcEnableMask = BIT32_MASK;
    shm()->roMode = ANALOG_ONLY;
    shm()->currentSettings = UNINITIALIZED;
    shm()->currentThresholdEV = -1;
    shm()->nFrames = 1;
    shm()->nTriggers = 1;
    shm()->nAddStorageCells = 0;
    shm()->deadTime = 0;
    sls::strcpy_safe(shm()->rxHostname, "none");
    shm()->rxTCPPort = DEFAULT_PORTNO + 2;
    shm()->useReceiverFlag = false;
    shm()->tenGigaEnable = 0;
    shm()->flippedDataX = 0;
    shm()->zmqport = DEFAULT_ZMQ_CL_PORTNO +
                     (detId * ((shm()->myDetectorType == EIGER) ? 2 : 1));
    shm()->rxZmqport = DEFAULT_ZMQ_RX_PORTNO +
                       (detId * ((shm()->myDetectorType == EIGER) ? 2 : 1));
    shm()->rxUpstream = false;
    shm()->rxReadFreq = 1;
    shm()->zmqip = 0u;
    shm()->rxZmqip = 0u;
    shm()->gappixels = 0u;
    memset(shm()->rxAdditionalJsonHeader, 0, MAX_STR_LENGTH);
    shm()->rxFrameDiscardMode = NO_DISCARD;
    shm()->rxFramePadding = true;
    shm()->activated = true;
    shm()->rxPadDeactivatedModules = true;
    shm()->rxSilentMode = false;
    sls::strcpy_safe(shm()->rxFilePath, "/");
    sls::strcpy_safe(shm()->rxFileName, "run");
    shm()->rxFileIndex = 0;
    shm()->rxFileFormat = BINARY;
    switch (shm()->myDetectorType) {
    case GOTTHARD:
        shm()->rxFramesPerFile = MAX_FRAMES_PER_FILE;
        break;
    case EIGER:
        shm()->rxFramesPerFile = EIGER_MAX_FRAMES_PER_FILE;
        break;
    case JUNGFRAU:
        shm()->rxFramesPerFile = JFRAU_MAX_FRAMES_PER_FILE;
        break;
    case CHIPTESTBOARD:
        shm()->rxFramesPerFile = CTB_MAX_FRAMES_PER_FILE;
        break;
    case MOENCH:
        shm()->rxFramesPerFile = MOENCH_MAX_FRAMES_PER_FILE;
        break;
    case MYTHEN3:
        shm()->rxFramesPerFile = MYTHEN3_MAX_FRAMES_PER_FILE;
        break;
    case GOTTHARD2:
        shm()->rxFramesPerFile = GOTTHARD2_MAX_FRAMES_PER_FILE;
        break;       
    default:
        break;
    }
    shm()->rxFileWrite = true;
    shm()->rxMasterFileWrite = true;
    shm()->rxFileOverWrite = true;
    shm()->rxDbitOffset = 0;
    shm()->numUDPInterfaces = 1;

    // get the detector parameters based on type
    detParameters parameters{type};
    shm()->nChan.x = parameters.nChanX;
    shm()->nChan.y = parameters.nChanY;
    shm()->nChip.x = parameters.nChipX;
    shm()->nChip.y = parameters.nChipY;
    shm()->nDacs = parameters.nDacs;
    shm()->dynamicRange = parameters.dynamicRange;
    shm()->nGappixels.x = parameters.nGappixelsX;
    shm()->nGappixels.y = parameters.nGappixelsY;

    // update #nchan, as it depends on #samples, adcmask,
    updateNumberOfChannels();
}

int slsDetector::sendModule(sls_detector_module *myMod,
                            sls::ClientSocket &client) {
    TLogLevel level = logDEBUG1;
    FILE_LOG(level) << "Sending Module";
    int ts = 0;
    int n = 0;
    n = client.Send(&(myMod->serialnumber), sizeof(myMod->serialnumber));
    ts += n;
    FILE_LOG(level) << "Serial number sent. " << n
                    << " bytes. serialno: " << myMod->serialnumber;

    n = client.Send(&(myMod->nchan), sizeof(myMod->nchan));
    ts += n;
    FILE_LOG(level) << "nchan sent. " << n
                    << " bytes. nchan: " << myMod->nchan;

    n = client.Send(&(myMod->nchip), sizeof(myMod->nchip));
    ts += n;
    FILE_LOG(level) << "nchip sent. " << n
                    << " bytes. nchip: " << myMod->nchip;

    n = client.Send(&(myMod->ndac), sizeof(myMod->ndac));
    ts += n;
    FILE_LOG(level) << "ndac sent. " << n
                    << " bytes. ndac: " << myMod->ndac;

    n = client.Send(&(myMod->reg), sizeof(myMod->reg));
    ts += n;
    FILE_LOG(level) << "reg sent. " << n << " bytes. reg: " << myMod->reg;

    n = client.Send(&(myMod->iodelay), sizeof(myMod->iodelay));
    ts += n;
    FILE_LOG(level) << "iodelay sent. " << n
                    << " bytes. iodelay: " << myMod->iodelay;

    n = client.Send(&(myMod->tau), sizeof(myMod->tau));
    ts += n;
    FILE_LOG(level) << "tau sent. " << n << " bytes. tau: " << myMod->tau;

    n = client.Send(&(myMod->eV), sizeof(myMod->eV));
    ts += n;
    FILE_LOG(level) << "ev sent. " << n << " bytes. ev: " << myMod->eV;

    n = client.Send(myMod->dacs, sizeof(int) * (myMod->ndac));
    ts += n;
    FILE_LOG(level) << "dacs sent. " << n << " bytes";

    if (shm()->myDetectorType == EIGER) {
        n = client.Send(myMod->chanregs, sizeof(int) * (myMod->nchan));
        ts += n;
        FILE_LOG(level) << "channels sent. " << n << " bytes";
    }
    return ts;
}

int slsDetector::receiveModule(sls_detector_module *myMod,
                               sls::ClientSocket &client) {
    int ts = 0;
    ts += client.Receive(&(myMod->serialnumber), sizeof(myMod->serialnumber));
    ts += client.Receive(&(myMod->nchan), sizeof(myMod->nchan));
    ts += client.Receive(&(myMod->nchip), sizeof(myMod->nchip));
    ts += client.Receive(&(myMod->ndac), sizeof(myMod->ndac));
    ts += client.Receive(&(myMod->reg), sizeof(myMod->reg));
    ts += client.Receive(&(myMod->iodelay), sizeof(myMod->iodelay));
    ts += client.Receive(&(myMod->tau), sizeof(myMod->tau));
    ts += client.Receive(&(myMod->eV), sizeof(myMod->eV));

    ts += client.Receive(myMod->dacs, sizeof(int) * (myMod->ndac));
    FILE_LOG(logDEBUG1) << "received dacs of size " << ts;
    if (shm()->myDetectorType == EIGER) {
        ts += client.Receive(myMod->chanregs, sizeof(int) * (myMod->nchan));
        FILE_LOG(logDEBUG1)
            << " nchan= " << myMod->nchan << " nchip= " << myMod->nchip
            << "received chans of size " << ts;
    }
    FILE_LOG(logDEBUG1) << "received module of size " << ts << " register "
                        << myMod->reg;
    return ts;
}

slsDetectorDefs::detectorType slsDetector::getDetectorTypeFromShm(int multi_id,
                                                                  bool verify) {
    if (!shm.IsExisting()) {
        throw SharedMemoryError("Shared memory " + shm.GetName() +
                                "does not exist.\n Corrupted Multi Shared "
                                "memory. Please free shared memory.");
    }

    shm.OpenSharedMemory();
    if (verify && shm()->shmversion != SLS_SHMVERSION) {
        std::ostringstream ss;
        ss << "Single shared memory (" << multi_id << "-" << detId
           << ":)version mismatch (expected 0x" << std::hex << SLS_SHMVERSION
           << " but got 0x" << shm()->shmversion << ")" << std::dec
           << ". Clear Shared memory to continue.";
        shm.UnmapSharedMemory();
        throw SharedMemoryError(ss.str());
    }
    auto type = shm()->myDetectorType;
    return type;
}

// static function
slsDetectorDefs::detectorType
slsDetector::getTypeFromDetector(const std::string &hostname, int cport) {
    int fnum = F_GET_DETECTOR_TYPE;
    int ret = FAIL;
    detectorType retval = GENERIC;
    FILE_LOG(logDEBUG1) << "Getting detector type ";
    sls::ClientSocket cs("Detector", hostname, cport);
    cs.Send(reinterpret_cast<char *>(&fnum), sizeof(fnum));
    cs.Receive(reinterpret_cast<char *>(&ret), sizeof(ret));
    cs.Receive(reinterpret_cast<char *>(&retval), sizeof(retval));
    FILE_LOG(logDEBUG1) << "Detector type is " << retval;
    return retval;
}

int slsDetector::setDetectorType(detectorType const type) {
    int fnum = F_GET_DETECTOR_TYPE;
    detectorType retval = GENERIC;
    FILE_LOG(logDEBUG1) << "Setting detector type to " << type;

    // if unspecified, then get from detector
    if (type == GET_DETECTOR_TYPE) {
        sendToDetector(fnum, nullptr, retval);
        shm()->myDetectorType = static_cast<detectorType>(retval);
        FILE_LOG(logDEBUG1) << "Detector Type: " << retval;
    }
    if (shm()->useReceiverFlag) {
        auto arg = static_cast<int>(shm()->myDetectorType);
        retval = GENERIC;
        FILE_LOG(logDEBUG1) << "Sending detector type to Receiver: " << arg;
        sendToReceiver(F_GET_RECEIVER_TYPE, arg, retval);
        FILE_LOG(logDEBUG1) << "Receiver Type: " << retval;
    }
    return retval;
}

slsDetectorDefs::detectorType slsDetector::getDetectorType() const {
    return shm()->myDetectorType;
}

void slsDetector::updateNumberOfChannels() {
    if (shm()->myDetectorType == CHIPTESTBOARD ||
        shm()->myDetectorType == MOENCH) {

        int nachans = 0, ndchans = 0;
        // analog channels (normal, analog/digital readout)
        if (shm()->roMode == slsDetectorDefs::ANALOG_ONLY ||
            shm()->roMode == slsDetectorDefs::ANALOG_AND_DIGITAL) {
            uint32_t mask = shm()->adcEnableMask;
            if (mask == BIT32_MASK) {
                nachans = 32;
            } else {
                for (int ich = 0; ich < 32; ++ich) {
                    if ((mask & (1 << ich)) != 0u)
                        ++nachans;
                }
            }
            FILE_LOG(logDEBUG1) << "#Analog Channels:" << nachans;
        }

        // digital channels (ctb only, digital, analog/digital readout)
        if (shm()->myDetectorType == CHIPTESTBOARD &&
            (shm()->roMode == DIGITAL_ONLY ||
             shm()->roMode == ANALOG_AND_DIGITAL)) {
            ndchans = 64;
            FILE_LOG(logDEBUG1) << "#Digital Channels:" << ndchans;
        }
        shm()->nChan.x = nachans + ndchans;
        FILE_LOG(logDEBUG1) << "# Total #Channels:" << shm()->nChan.x;
    }
}

slsDetectorDefs::xy slsDetector::getNumberOfChannels() const {
    slsDetectorDefs::xy coord;
    coord.x = (shm()->nChan.x * shm()->nChip.x +
               shm()->gappixels * shm()->nGappixels.x);
    coord.y = (shm()->nChan.y * shm()->nChip.y +
               shm()->gappixels * shm()->nGappixels.y);
    return coord;
}

bool slsDetector::getQuad() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Quad Type";
    sendToDetector(F_GET_QUAD, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Quad Type :" << retval;
    return (retval == 0 ? false : true);
}

void slsDetector::setQuad(const bool enable) {
    int value = enable ? 1 : 0;
    FILE_LOG(logDEBUG1) << "Setting Quad type to " << value;
    sendToDetector(F_SET_QUAD, value, nullptr);
    FILE_LOG(logDEBUG1) << "Setting Quad type to " << value << " in Receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_QUAD, value, nullptr);
    }
}

void slsDetector::setReadNLines(const int value) {
    FILE_LOG(logDEBUG1) << "Setting read n lines to " << value;
    sendToDetector(F_SET_READ_N_LINES, value, nullptr);
    FILE_LOG(logDEBUG1) << "Setting read n lines to " << value
                        << " in Receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_READ_N_LINES, value, nullptr);
    }
}

int slsDetector::getReadNLines() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting read n lines";
    sendToDetector(F_GET_READ_N_LINES, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Read n lines: " << retval;
    return retval;
}

void slsDetector::updateMultiSize(slsDetectorDefs::xy det) {
    shm()->multiSize = det;
    int args[2] = {shm()->multiSize.y, detId};
    sendToDetector(F_SET_POSITION, args, nullptr);
}


int slsDetector::setControlPort(int port_number) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting control port to " << port_number;
    if (port_number >= 0 && port_number != shm()->controlPort) {
        if (strlen(shm()->hostname) > 0) {
            sendToDetector(F_SET_PORT, port_number, retval);
            shm()->controlPort = retval;
            FILE_LOG(logDEBUG1) << "Control port: " << retval;
        } else {
            shm()->controlPort = port_number;
        }
    }
    return shm()->controlPort;
}

int slsDetector::setStopPort(int port_number) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting stop port to " << port_number;
    if (port_number >= 0 && port_number != shm()->stopPort) {
        if (strlen(shm()->hostname) > 0) {
            sendToDetectorStop(F_SET_PORT, port_number, retval);
            shm()->stopPort = retval;
            FILE_LOG(logDEBUG1) << "Stop port: " << retval;
        } else {
            shm()->stopPort = port_number;
        }
    }
    return shm()->stopPort;
}

int slsDetector::setReceiverPort(int port_number) {
    FILE_LOG(logDEBUG1) << "Setting reciever port to " << port_number;
    if (port_number >= 0 && port_number != shm()->rxTCPPort) {
        if (shm()->useReceiverFlag) {
            int retval = -1;
            sendToReceiver(F_SET_RECEIVER_PORT, port_number, retval);
            shm()->rxTCPPort = retval;
            FILE_LOG(logDEBUG1) << "Receiver port: " << retval;

        } else {
            shm()->rxTCPPort = port_number;
        }
    }
    return shm()->rxTCPPort;
}

int slsDetector::getReceiverPort() const { return shm()->rxTCPPort; }

int slsDetector::getControlPort() const { return shm()->controlPort; }

int slsDetector::getStopPort() const { return shm()->stopPort; }

bool slsDetector::lockServer(int lock) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting detector server lock to " << lock;
    sendToDetector(F_LOCK_SERVER, lock, retval);
    FILE_LOG(logDEBUG1) << "Lock: " << retval;
    return (retval == 1 ? true : false);
}

sls::IpAddr slsDetector::getLastClientIP() {
    sls::IpAddr retval = 0u;
    FILE_LOG(logDEBUG1) << "Getting last client ip to detector server";
    sendToDetector(F_GET_LAST_CLIENT_IP, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Last client IP to detector: " << retval;
    return retval;
}

void slsDetector::exitServer() {
    FILE_LOG(logDEBUG1) << "Sending exit command to detector server";
    sendToDetector(F_EXIT_SERVER);
    FILE_LOG(logINFO) << "Shutting down the Detector server";
}

void slsDetector::execCommand(const std::string &cmd) {
    char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    sls::strcpy_safe(arg, cmd.c_str());
    FILE_LOG(logDEBUG1) << "Sending command to detector " << arg;
    sendToDetector(F_EXEC_COMMAND, arg, retval);
    if (strlen(retval) != 0u) {
        FILE_LOG(logINFO) << "Detector " << detId << " returned:\n" << retval;
    }
}

void slsDetector::updateCachedDetectorVariables() {
    int fnum = F_UPDATE_CLIENT;
    FILE_LOG(logDEBUG1) << "Sending update client to detector server";
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    if (client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0) ==
        FORCE_UPDATE) {
        int n = 0, i32 = 0;
        int64_t i64 = 0;
        sls::IpAddr lastClientIP = 0u;
        n += client.Receive(&lastClientIP, sizeof(lastClientIP));
        FILE_LOG(logDEBUG1)
            << "Updating detector last modified by " << lastClientIP;

        // dr
        n += client.Receive(&i32, sizeof(i32));
        shm()->dynamicRange = i32;

        // settings
        if (shm()->myDetectorType == EIGER ||
            shm()->myDetectorType == JUNGFRAU || shm()->myDetectorType == GOTTHARD) {
            n += client.Receive(&i32, sizeof(i32));
            shm()->currentSettings = static_cast<detectorSettings>(i32);
        }

        // threshold
        if (shm()->myDetectorType == EIGER) {
            n += client.Receive(&i32, sizeof(i32));
            shm()->currentThresholdEV = i32;
        }

        // frame number
        n += client.Receive(&i64, sizeof(i64));
        shm()->nFrames = i64;

        if (shm()->myDetectorType == JUNGFRAU) {
            // storage cell
            n += client.Receive(&i64, sizeof(i64));
            shm()->nAddStorageCells = i64;
        }

        // triggers
        n += client.Receive(&i64, sizeof(i64));
        shm()->nTriggers = i64;

        // readout mode
        if (shm()->myDetectorType == CHIPTESTBOARD) {
            n += client.Receive(&i32, sizeof(i32));
            shm()->roMode = static_cast<readoutMode>(i32);
        }

        // roi
        if (shm()->myDetectorType == GOTTHARD) {
            n += client.Receive(&i32, sizeof(i32));
            shm()->roi.xmin = i32;
            n += client.Receive(&i32, sizeof(i32));
            shm()->roi.xmax = i32;
        }

        if (shm()->myDetectorType == CHIPTESTBOARD ||
            shm()->myDetectorType == MOENCH) {
            // adcmask
            uint32_t u32 = 0;
            n += client.Receive(&u32, sizeof(u32));
            shm()->adcEnableMask = u32;
            if (shm()->myDetectorType == MOENCH)
                setAdditionalJsonParameter("adcmask", std::to_string(u32));

            // update #nchan, as it depends on #samples, adcmask,
            updateNumberOfChannels();
        }

        // num udp interfaces
        if (shm()->myDetectorType == JUNGFRAU) {
            n += client.Receive(&i32, sizeof(i32));
            shm()->numUDPInterfaces = i32;
        }

        if (n == 0) {
            FILE_LOG(logERROR) << "Could not update detector, received 0 bytes";
        }
    }
}

std::vector<std::string> slsDetector::getConfigFileCommands() {
    std::vector<std::string> base{"hostname",    "port",       "stopport",
                                  "settingsdir", "fpath",      "lock",
                                  "zmqport",     "rx_zmqport", "zmqip",
                                  "rx_zmqip",    "rx_tcpport"};

    switch (shm()->myDetectorType) {
    case GOTTHARD:
        base.emplace_back("detectormac");
        base.emplace_back("detectorip");
        base.emplace_back("rx_udpport");
        base.emplace_back("rx_udpip");
        base.emplace_back("rx_udpmac");
        base.emplace_back("extsig");
        break;
    case EIGER:
        base.emplace_back("detectormac");
        base.emplace_back("detectorip");
        base.emplace_back("rx_udpport");
        base.emplace_back("rx_udpport2");
        base.emplace_back("rx_udpip");
        base.emplace_back("rx_udpmac");
        base.emplace_back("trimen");
        base.emplace_back("iodelay");
        base.emplace_back("tengiga");
        break;
    case JUNGFRAU:
        base.emplace_back("detectormac");
        base.emplace_back("detectormac2");
        base.emplace_back("detectorip");
        base.emplace_back("detectorip2");
        base.emplace_back("rx_udpport");
        base.emplace_back("rx_udpport2");
        base.emplace_back("rx_udpip");
        base.emplace_back("rx_udpip2");
        base.emplace_back("rx_udpmac");
        base.emplace_back("rx_udpmac2");
        base.emplace_back("powerchip");
        break;
    case CHIPTESTBOARD:
        base.emplace_back("detectormac");
        base.emplace_back("detectorip");
        base.emplace_back("rx_udpport");
        base.emplace_back("rx_udpip");
        base.emplace_back("rx_udpmac");
        break;
    case MOENCH:
        base.emplace_back("detectormac");
        base.emplace_back("detectorip");
        base.emplace_back("rx_udpport");
        base.emplace_back("rx_udpip");
        base.emplace_back("rx_udpmac");
        break;
    default:
        throw RuntimeError(
            "Write configuration file called with unknown detector: " +
            std::to_string(shm()->myDetectorType));
    }

    base.emplace_back("vhighvoltage");
    base.emplace_back("rx_hostname");
    base.emplace_back("r_readfreq");
    base.emplace_back("rx_udpsocksize");
    base.emplace_back("rx_realudpsocksize");

    std::vector<std::string> commands;
    for (const auto &cmd : base) {
        std::ostringstream os;
        os << detId << ':' << cmd;
        commands.emplace_back(os.str());
    }
    return commands;
}

slsDetectorDefs::detectorSettings slsDetector::getSettings() {
    return sendSettingsOnly(GET_SETTINGS);
}

slsDetectorDefs::detectorSettings
slsDetector::setSettings(detectorSettings isettings) {
    FILE_LOG(logDEBUG1) << "slsDetector setSettings " << isettings;

    if (isettings == -1) {
        return getSettings();
    }

    // eiger: only set shm, setting threshold loads the module data
    if (shm()->myDetectorType == EIGER) {
        switch (isettings) {
        case STANDARD:
        case HIGHGAIN:
        case LOWGAIN:
        case VERYHIGHGAIN:
        case VERYLOWGAIN:
            shm()->currentSettings = isettings;
            return shm()->currentSettings;
        default:
            std::ostringstream ss;
            ss << "Unknown settings " << ToString(isettings)
               << " for this detector!";
            throw RuntimeError(ss.str());
        }
    }

    // others: send only the settings, detector server will update dac values
    // already in server
    return sendSettingsOnly(isettings);
}

slsDetectorDefs::detectorSettings
slsDetector::sendSettingsOnly(detectorSettings isettings) {
    int arg = static_cast<int>(isettings);
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting settings to " << arg;
    sendToDetector(F_SET_SETTINGS, arg, retval);
    FILE_LOG(logDEBUG1) << "Settings: " << retval;
    shm()->currentSettings = static_cast<detectorSettings>(retval);
    return shm()->currentSettings;
}

int slsDetector::getThresholdEnergy() {
    // moench - get threshold energy from processor (due to different clients,
    // diff shm)
    if (shm()->myDetectorType == MOENCH) {
        // get json from rxr, parse for threshold and update shm
        getAdditionalJsonHeader();
        std::string result = getAdditionalJsonParameter("threshold");
        // convert to integer
        try {
            // udpate shm
            shm()->currentThresholdEV = stoi(result);
            return shm()->currentThresholdEV;
        }
        // not found or cannot scan integer
        catch (...) {
            return -1;
        }
    }

    FILE_LOG(logDEBUG1) << "Getting threshold energy";
    int retval = -1;
    sendToDetector(F_GET_THRESHOLD_ENERGY, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Threshold: " << retval;
    shm()->currentThresholdEV = retval;
    return shm()->currentThresholdEV;
}

int slsDetector::setThresholdEnergy(int e_eV, detectorSettings isettings,
                                    int tb) {

    // check as there is client processing
    if (shm()->myDetectorType == EIGER) {
        setThresholdEnergyAndSettings(e_eV, isettings, tb);
        return shm()->currentThresholdEV;
    }

    // moench - send threshold energy to processor
    else if (shm()->myDetectorType == MOENCH) {
        std::string result =
            setAdditionalJsonParameter("threshold", std::to_string(e_eV));
        if (result == std::to_string(e_eV)) {
            // update shm
            shm()->currentThresholdEV = e_eV;
            return shm()->currentThresholdEV;
        }
        return -1;
    }
    throw RuntimeError(
        "Set threshold energy not implemented for this detector");
}

void slsDetector::setThresholdEnergyAndSettings(int e_eV,
                                                detectorSettings isettings,
                                                int tb) {

    // if settings provided, use that, else use the shared memory variable
    detectorSettings is =
        ((isettings != GET_SETTINGS) ? isettings : shm()->currentSettings);

    // verify e_eV exists in trimEneregies[]
    if (shm()->trimEnergies.empty() || (e_eV < shm()->trimEnergies.front()) ||
        (e_eV > shm()->trimEnergies.back())) {
        throw RuntimeError("This energy " + std::to_string(e_eV) +
                           " not defined for this module!");
    }

    bool interpolate =
        std::all_of(shm()->trimEnergies.begin(), shm()->trimEnergies.end(),
                    [e_eV](const int &e) { return e != e_eV; });

    sls_detector_module myMod{shm()->myDetectorType};

    if (!interpolate) {
        std::string settingsfname = getTrimbitFilename(is, e_eV);
        FILE_LOG(logDEBUG1) << "Settings File is " << settingsfname;
        myMod = readSettingsFile(settingsfname, tb);
    } else {
        // find the trim values
        int trim1 = -1, trim2 = -1;
        for (size_t i = 0; i < shm()->trimEnergies.size(); ++i) {
            if (e_eV < shm()->trimEnergies[i]) {
                trim2 = shm()->trimEnergies[i];
                trim1 = shm()->trimEnergies[i - 1];
                break;
            }
        }
        std::string settingsfname1 = getTrimbitFilename(is, trim1);
        std::string settingsfname2 = getTrimbitFilename(is, trim2);
        FILE_LOG(logDEBUG1) << "Settings Files are " << settingsfname1
                            << " and " << settingsfname2;
        auto myMod1 = readSettingsFile(settingsfname1, tb);
        auto myMod2 = readSettingsFile(settingsfname2, tb);
        if (myMod1.iodelay != myMod2.iodelay) {
            throw RuntimeError("setThresholdEnergyAndSettings: Iodelays do not "
                               "match between files");
        }
        myMod = interpolateTrim(&myMod1, &myMod2, e_eV, trim1, trim2, tb);
        myMod.iodelay = myMod1.iodelay;
        myMod.tau =
            linearInterpolation(e_eV, trim1, trim2, myMod1.tau, myMod2.tau);
    }

    shm()->currentSettings = is;
    myMod.reg = shm()->currentSettings;
    myMod.eV = e_eV;
    setModule(myMod, tb);
    if (getSettings() != is) {
        throw RuntimeError("setThresholdEnergyAndSettings: Could not set "
                           "settings in detector");
    }
}

std::string slsDetector::getTrimbitFilename(detectorSettings s, int e_eV) {
    std::string ssettings;
    switch (s) {
    case STANDARD:
        ssettings = "/standard";
        break;
    case HIGHGAIN:
        ssettings = "/highgain";
        break;
    case LOWGAIN:
        ssettings = "/lowgain";
        break;
    case VERYHIGHGAIN:
        ssettings = "/veryhighgain";
        break;
    case VERYLOWGAIN:
        ssettings = "/verylowgain";
        break;
    default:
        std::ostringstream ss;
        ss << "Unknown settings " << ToString(s)
           << " for this detector!";
        throw RuntimeError(ss.str());
    }
    std::ostringstream ostfn;
    ostfn << shm()->settingsDir << ssettings << "/" << e_eV << "eV"
          << "/noise.sn" << std::setfill('0') << std::setw(3) << std::dec
          << getSerialNumber() << std::setbase(10);
    return ostfn.str();
}

std::string slsDetector::getSettingsDir() {
    return std::string(shm()->settingsDir);
}

std::string slsDetector::setSettingsDir(const std::string &dir) {
    sls::strcpy_safe(shm()->settingsDir, dir.c_str());
    return shm()->settingsDir;
}

void slsDetector::loadSettingsFile(const std::string &fname) {
    std::string fn = fname;
    std::ostringstream ostfn;
    ostfn << fname;

    // find specific file if it has detid in file name (.snxxx)
    if (shm()->myDetectorType == EIGER) {
        if (fname.find(".sn") == std::string::npos &&
            fname.find(".trim") == std::string::npos &&
            fname.find(".settings") == std::string::npos) {
            ostfn << ".sn" << std::setfill('0') << std::setw(3) << std::dec
                  << getSerialNumber();
        }
    }
    fn = ostfn.str();
    auto myMod = readSettingsFile(fn);
    setModule(myMod);
}

void slsDetector::saveSettingsFile(const std::string &fname) {
    std::string fn = fname;
    std::ostringstream ostfn;
    ostfn << fname;

    // find specific file if it has detid in file name (.snxxx)
    if (shm()->myDetectorType == EIGER) {
        ostfn << ".sn" << std::setfill('0') << std::setw(3) << std::dec
              << getSerialNumber();
    }
    fn = ostfn.str();
    sls_detector_module myMod = getModule();
    writeSettingsFile(fn, myMod);
}

slsDetectorDefs::runStatus slsDetector::getRunStatus() const {
    runStatus retval = ERROR;
    FILE_LOG(logDEBUG1) << "Getting status";
    sendToDetectorStop(F_GET_RUN_STATUS, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Detector status: " << ToString(retval);
    return retval;
}

void slsDetector::prepareAcquisition() {
    FILE_LOG(logDEBUG1) << "Preparing Detector for Acquisition";
    sendToDetector(F_PREPARE_ACQUISITION);
    FILE_LOG(logDEBUG1) << "Prepare Acquisition successful";
}

void slsDetector::startAcquisition() {
    FILE_LOG(logDEBUG1) << "Starting Acquisition";
    sendToDetector(F_START_ACQUISITION);
    FILE_LOG(logDEBUG1) << "Starting Acquisition successful";
}

void slsDetector::stopAcquisition() {
    // get status before stopping acquisition
    runStatus s = ERROR, r = ERROR;
    if (shm()->rxUpstream) {
        s = getRunStatus();
        r = getReceiverStatus();
    }
    FILE_LOG(logDEBUG1) << "Stopping Acquisition";
    sendToDetectorStop(F_STOP_ACQUISITION);
    FILE_LOG(logDEBUG1) << "Stopping Acquisition successful";
    // if rxr streaming and acquisition finished, restream dummy stop packet
    if ((shm()->rxUpstream) && (s == IDLE) && (r == IDLE)) {
        restreamStopFromReceiver();
    }
}

void slsDetector::sendSoftwareTrigger() {
    FILE_LOG(logDEBUG1) << "Sending software trigger";
    sendToDetector(F_SOFTWARE_TRIGGER);
    FILE_LOG(logDEBUG1) << "Sending software trigger successful";
}

void slsDetector::startAndReadAll() {
    FILE_LOG(logDEBUG1) << "Starting and reading all frames";
    sendToDetector(F_START_AND_READ_ALL);
    FILE_LOG(logDEBUG1) << "Detector successfully finished acquisition";
}

void slsDetector::startReadOut() {
    FILE_LOG(logDEBUG1) << "Starting readout";
    sendToDetector(F_START_READOUT);
    FILE_LOG(logDEBUG1) << "Starting detector readout successful";
}

void slsDetector::readAll() {
    FILE_LOG(logDEBUG1) << "Reading all frames";
    sendToDetector(F_READ_ALL);
    FILE_LOG(logDEBUG1) << "Detector successfully finished reading all frames";
}

void slsDetector::setStartingFrameNumber(uint64_t value) {
    FILE_LOG(logDEBUG1) << "Setting starting frame number to " << value;
    sendToDetector(F_SET_STARTING_FRAME_NUMBER, value, nullptr);
}

uint64_t slsDetector::getStartingFrameNumber() {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting starting frame number";
    sendToDetector(F_GET_STARTING_FRAME_NUMBER, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Starting frame number :" << retval;
    return retval;
}

void slsDetector::sendTotalNumFramestoReceiver() {
    if (shm()->useReceiverFlag) {
        int64_t arg = shm()->nFrames * shm()->nTriggers * (shm()->nAddStorageCells + 1);
        FILE_LOG(logDEBUG1) << "Sending total number of frames (#f x #t x #s) to Receiver: " << arg;
        sendToReceiver(F_RECEIVER_SET_NUM_FRAMES, arg, nullptr);   
    }
}

int64_t slsDetector::getNumberOfFramesFromShm() {
    return shm()->nFrames; 
}

int64_t slsDetector::getNumberOfFrames() {
    int64_t prevVal = shm()->nFrames;
    int64_t retval = -1;
    sendToDetector(F_GET_NUM_FRAMES, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of frames :" << retval;
    shm()->nFrames = retval;
    if (prevVal != retval) {
        sendTotalNumFramestoReceiver();
    }    
    return shm()->nFrames; 
}

void slsDetector::setNumberOfFrames(int64_t value) {
    FILE_LOG(logDEBUG1) << "Setting number of frames to " << value;
    sendToDetector(F_SET_NUM_FRAMES, value, nullptr);
    shm()->nFrames = value;
    sendTotalNumFramestoReceiver();
}

int64_t slsDetector::getNumberOfTriggersFromShm() {
    return shm()->nTriggers; 
}

int64_t slsDetector::getNumberOfTriggers() {
    int64_t prevVal = shm()->nTriggers;
    int64_t retval = -1;
    sendToDetector(F_GET_NUM_TRIGGERS, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of triggers :" << retval;
    shm()->nTriggers = retval;
    if (prevVal != retval) {
        sendTotalNumFramestoReceiver();
    }    
    return shm()->nTriggers; 
}

void slsDetector::setNumberOfTriggers(int64_t value) {
    FILE_LOG(logDEBUG1) << "Setting number of triggers to " << value;
    sendToDetector(F_SET_NUM_TRIGGERS, value, nullptr);
    shm()->nTriggers = value;
    sendTotalNumFramestoReceiver();
}
    
int slsDetector::getNumberOfAdditionalStorageCellsFromShm() {
    return shm()->nAddStorageCells; 
}

int slsDetector::getNumberOfAdditionalStorageCells() {
    int prevVal = shm()->nAddStorageCells;
    int retval = -1;
    sendToDetector(F_GET_NUM_ADDITIONAL_STORAGE_CELLS, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of storage cells :" << retval;
    shm()->nAddStorageCells = retval;
    if (prevVal != retval) {
        sendTotalNumFramestoReceiver();
    }    
    return shm()->nAddStorageCells; 
}
   
void slsDetector::setNumberOfAdditionalStorageCells(int value) {
    FILE_LOG(logDEBUG1) << "Setting number of storage cells to " << value;
    sendToDetector(F_SET_NUM_ADDITIONAL_STORAGE_CELLS, value, nullptr);
    shm()->nAddStorageCells = value;
    sendTotalNumFramestoReceiver();
}

int slsDetector::getNumberOfAnalogSamples() {
    int retval = -1;
    sendToDetector(F_GET_NUM_ANALOG_SAMPLES, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of analog samples :" << retval;
    return retval; 
}
    
void slsDetector::setNumberOfAnalogSamples(int value) {
    FILE_LOG(logDEBUG1) << "Setting number of analog samples to " << value;
    sendToDetector(F_SET_NUM_ANALOG_SAMPLES, value, nullptr);
    // update #nchan, as it depends on #samples, adcmask
    updateNumberOfChannels();
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1) << "Sending number of analog samples to Receiver: " << value;
        sendToReceiver(F_RECEIVER_SET_NUM_ANALOG_SAMPLES, value, nullptr);   
    }
}
   
int slsDetector::getNumberOfDigitalSamples() {
    int retval = -1;
    sendToDetector(F_GET_NUM_DIGITAL_SAMPLES, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of digital samples :" << retval;
    return retval; 
}
    
void slsDetector::setNumberOfDigitalSamples(int value) {
    FILE_LOG(logDEBUG1) << "Setting number of digital samples to " << value;
    sendToDetector(F_SET_NUM_DIGITAL_SAMPLES, value, nullptr);
    // update #nchan, as it depends on #samples, adcmask
    updateNumberOfChannels();   
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1) << "Sending number of digital samples to Receiver: " << value;
        sendToReceiver(F_RECEIVER_SET_NUM_DIGITAL_SAMPLES, value, nullptr);   
    }
}

int64_t slsDetector::getExptime() {
    int64_t retval = -1;
    sendToDetector(F_GET_EXPTIME, nullptr, retval);
    FILE_LOG(logDEBUG1) << "exptime :" << retval << "ns";
    return retval; 
}

void slsDetector::setExptime(int64_t value) {
    int64_t prevVal = value;
    if (shm()->myDetectorType == EIGER) {
        prevVal = getExptime();
    }
    FILE_LOG(logDEBUG1) << "Setting exptime to " << value << "ns";
    sendToDetector(F_SET_EXPTIME, value, nullptr);
    if (shm()->myDetectorType == EIGER && prevVal != value && shm()->dynamicRange == 16) {
        int r = getRateCorrection();
        if (r != 0) {
            setRateCorrection(r);
        }            
    }
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1) << "Sending exptime to Receiver: " << value;
        sendToReceiver(F_RECEIVER_SET_EXPTIME, value, nullptr);   
    }
}

int64_t slsDetector::getPeriod() {
    int64_t retval = -1;
    sendToDetector(F_GET_PERIOD, nullptr, retval);
    FILE_LOG(logDEBUG1) << "period :" << retval << "ns";
    return retval; 
}

void slsDetector::setPeriod(int64_t value) {
    FILE_LOG(logDEBUG1) << "Setting period to " << value << "ns";
    sendToDetector(F_SET_PERIOD, value, nullptr);
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1) << "Sending period to Receiver: " << value;
        sendToReceiver(F_RECEIVER_SET_PERIOD, value, nullptr);   
    }
}

int64_t slsDetector::getDelayAfterTrigger() {
    int64_t retval = -1;
    sendToDetector(F_GET_DELAY_AFTER_TRIGGER, nullptr, retval);
    FILE_LOG(logDEBUG1) << "delay after trigger :" << retval << "ns";
    return retval; 
}

void slsDetector::setDelayAfterTrigger(int64_t value) {
    FILE_LOG(logDEBUG1) << "Setting delay after trigger to " << value << "ns";
    sendToDetector(F_SET_DELAY_AFTER_TRIGGER, value, nullptr);
}

int64_t slsDetector::getSubExptime() {
    int64_t retval = -1;
    sendToDetector(F_GET_SUB_EXPTIME, nullptr, retval);
    FILE_LOG(logDEBUG1) << "sub exptime :" << retval << "ns";
    return retval; 
}

void slsDetector::setSubExptime(int64_t value) {
    int64_t prevVal = value;
    if (shm()->myDetectorType == EIGER) {
        prevVal = getSubExptime();
    }    
    FILE_LOG(logDEBUG1) << "Setting sub exptime to " << value << "ns";
    sendToDetector(F_SET_SUB_EXPTIME, value, nullptr);
    if (shm()->myDetectorType == EIGER && prevVal != value && shm()->dynamicRange == 32) {
        int r = getRateCorrection();
        if (r != 0) {
            setRateCorrection(r);
        }            
    }    
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1) << "Sending sub exptime to Receiver: " << value;
        sendToReceiver(F_RECEIVER_SET_SUB_EXPTIME, value, nullptr);   
    }
}
    
int64_t slsDetector::getSubDeadTime() {
    int64_t retval = -1;
    sendToDetector(F_GET_SUB_DEADTIME, nullptr, retval);
    FILE_LOG(logDEBUG1) << "sub deadtime :" << retval << "ns";
    return retval; 
}
    
void slsDetector::setSubDeadTime(int64_t value) {
    FILE_LOG(logDEBUG1) << "Setting sub deadtime to " << value << "ns";
    sendToDetector(F_SET_SUB_DEADTIME, value, nullptr);
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1) << "Sending sub deadtime to Receiver: " << value;
        sendToReceiver(F_RECEIVER_SET_SUB_DEADTIME, value, nullptr);   
    }
}
    
int64_t slsDetector::getStorageCellDelay() {
    int64_t retval = -1;
    sendToDetector(F_GET_STORAGE_CELL_DELAY, nullptr, retval);
    FILE_LOG(logDEBUG1) << "storage cell delay :" << retval;
    return retval; 
}
        
void slsDetector::setStorageCellDelay(int64_t value) {
    FILE_LOG(logDEBUG1) << "Setting storage cell delay to " << value << "ns";
    sendToDetector(F_SET_STORAGE_CELL_DELAY, value, nullptr);
}
      
int64_t slsDetector::getNumberOfFramesLeft() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_FRAMES_LEFT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of frames left :" << retval;
    return retval; 
}
    
int64_t slsDetector::getNumberOfTriggersLeft() const {
     int64_t retval = -1;
    sendToDetectorStop(F_GET_TRIGGERS_LEFT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of triggers left :" << retval;
    return retval; 
}
    
int64_t slsDetector::getDelayAfterTriggerLeft() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_DELAY_AFTER_TRIGGER_LEFT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "delay after trigger left :" << retval << "ns";
    return retval; 
}
    
int64_t slsDetector::getExptimeLeft() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_EXPTIME_LEFT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "exptime left :" << retval << "ns";
    return retval; 
}
    
int64_t slsDetector::getPeriodLeft() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_PERIOD_LEFT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "period left :" << retval << "ns";
    return retval; 
}
    
int64_t slsDetector::getMeasuredPeriod() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_MEASURED_PERIOD, nullptr, retval);
    FILE_LOG(logDEBUG1) << "measured period :" << retval << "ns";
    return retval; 
}
    
int64_t slsDetector::getMeasuredSubFramePeriod() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_MEASURED_SUBPERIOD, nullptr, retval);
    FILE_LOG(logDEBUG1) << "exptime :" << retval << "ns";
    return retval; 
}
    
int64_t slsDetector::getNumberOfFramesFromStart() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_FRAMES_FROM_START, nullptr, retval);
    FILE_LOG(logDEBUG1) << "number of frames from start :" << retval;
    return retval; 
}
    
int64_t slsDetector::getActualTime() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_ACTUAL_TIME, nullptr, retval);
    FILE_LOG(logDEBUG1) << "actual time :" << retval << "ns";
    return retval; 
}
    
int64_t slsDetector::getMeasurementTime() const {
    int64_t retval = -1;
    sendToDetectorStop(F_GET_MEASUREMENT_TIME, nullptr, retval);
    FILE_LOG(logDEBUG1) << "measurement time :" << retval << "ns";
    return retval; 
}

int slsDetector::setDynamicRange(int n) {
    // TODO! Properly handle fail
    int prevDr = shm()->dynamicRange;

    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting dynamic range to " << n;
    sendToDetector(F_SET_DYNAMIC_RANGE, n, retval);
    FILE_LOG(logDEBUG1) << "Dynamic Range: " << retval;
    shm()->dynamicRange = retval;

    if (shm()->useReceiverFlag) {
        n = shm()->dynamicRange;
        retval = -1;
        FILE_LOG(logDEBUG1) << "Sending dynamic range to receiver: " << n;
        sendToReceiver(F_SET_RECEIVER_DYNAMIC_RANGE, n, retval);
        FILE_LOG(logDEBUG1) << "Receiver Dynamic range: " << retval;
    }

    // changes in dr
    int dr = shm()->dynamicRange;
    if (prevDr != dr && shm()->myDetectorType == EIGER) {
        updateRateCorrection();
        // update speed for usability
        if (dr == 32) {
            FILE_LOG(logINFO) << "Setting Clock to Quarter Speed to cope with Dynamic Range of 32";     setClockDivider(RUN_CLOCK, 2);
        } else if (dr == 16) {
            FILE_LOG(logINFO) << "Setting Clock to Half Speed to cope with Dynamic Range of 16";     setClockDivider(RUN_CLOCK, 1);
        }
    }
    
    return shm()->dynamicRange;
}

int slsDetector::setDAC(int val, dacIndex index, int mV) {
    int args[]{static_cast<int>(index), mV, val};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting DAC " << index << " to " << val
                        << (mV != 0 ? "mV" : "dac units");
    sendToDetector(F_SET_DAC, args, retval);
    FILE_LOG(logDEBUG1) << "Dac index " << index << ": " << retval
                        << (mV != 0 ? "mV" : "dac units");
    return retval;
}

int slsDetector::getOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex) {
    int args[]{static_cast<int>(index), chipIndex}; 
    int retval = -1;
    sendToDetector(F_GET_ON_CHIP_DAC, args, retval);
    FILE_LOG(logDEBUG1) << "On chip DAC " << index << " (chip index:" << chipIndex << "): " << retval;
    return retval;
}
    
void slsDetector::setOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex, int value) {
    int args[]{static_cast<int>(index), chipIndex, value}; 
    FILE_LOG(logDEBUG1) << "Setting On chip DAC " << index << " (chip index:" << chipIndex << ") to " << value;
    sendToDetector(F_SET_ON_CHIP_DAC, args, nullptr);
}    

int slsDetector::getADC(dacIndex index) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting ADC " << index;
    sendToDetector(F_GET_ADC, static_cast<int>(index), retval);
    FILE_LOG(logDEBUG1) << "ADC (" << index << "): " << retval;
    return retval;
}

slsDetectorDefs::timingMode slsDetector::setTimingMode(timingMode pol) {
    int fnum = F_SET_TIMING_MODE;
    auto arg = static_cast<int>(pol);
    timingMode retval = GET_TIMING_MODE;
    FILE_LOG(logDEBUG1) << "Setting communication to mode " << pol;
    sendToDetector(fnum, arg, retval);
    FILE_LOG(logDEBUG1) << "Timing Mode: " << retval;
    return retval;
}

slsDetectorDefs::externalSignalFlag
slsDetector::setExternalSignalFlags(externalSignalFlag pol) {
    int fnum = F_SET_EXTERNAL_SIGNAL_FLAG;
    auto retval = GET_EXTERNAL_SIGNAL_FLAG;
    FILE_LOG(logDEBUG1) << "Setting signal flag to " << pol;
    sendToDetector(fnum, pol, retval);
    FILE_LOG(logDEBUG1) << "Ext Signal: " << retval;
    return retval;
}

void slsDetector::setParallelMode(const bool enable) {
    int arg = static_cast<int>(enable);
    FILE_LOG(logDEBUG1) << "Setting parallel mode to " << arg;
    sendToDetector(F_SET_PARALLEL_MODE, arg, nullptr);
}

bool slsDetector::getParallelMode() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting parallel mode";
    sendToDetector(F_GET_PARALLEL_MODE, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Parallel mode: " << retval;
    return static_cast<bool>(retval);
}

void slsDetector::setOverFlowMode(const bool enable) {
    int arg = static_cast<int>(enable);
    FILE_LOG(logDEBUG1) << "Setting overflow mode to " << arg;
    sendToDetector(F_SET_OVERFLOW_MODE, arg, nullptr);
}
   
bool slsDetector::getOverFlowMode() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting overflow mode";
    sendToDetector(F_GET_OVERFLOW_MODE, nullptr, retval);
    FILE_LOG(logDEBUG1) << "overflow mode: " << retval;
    return static_cast<bool>(retval);
}

void slsDetector::setStoreInRamMode(const bool enable) {
    int arg = static_cast<int>(enable);
    FILE_LOG(logDEBUG1) << "Setting store in ram mode to " << arg;
    sendToDetector(F_SET_STOREINRAM_MODE, arg, nullptr);
}

bool slsDetector::getStoreInRamMode() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting store in ram mode";
    sendToDetector(F_GET_STOREINRAM_MODE, nullptr, retval);
    FILE_LOG(logDEBUG1) << "store in ram mode: " << retval;
    return static_cast<bool>(retval);
}

void slsDetector::setReadoutMode(const slsDetectorDefs::readoutMode mode) {
    uint32_t arg = static_cast<uint32_t>(mode);
    FILE_LOG(logDEBUG1) << "Setting readout mode to " << arg;
    sendToDetector(F_SET_READOUT_MODE, arg, nullptr);
    shm()->roMode = mode;
    // update #nchan, as it depends on #samples, adcmask,
    if (shm()->myDetectorType == CHIPTESTBOARD) {
        updateNumberOfChannels();
    }
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_READOUT_MODE, mode, nullptr);
    }
}

slsDetectorDefs::readoutMode slsDetector::getReadoutMode() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting readout mode";
    sendToDetector(F_GET_READOUT_MODE, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Readout mode: " << retval;
    readoutMode oldmode = shm()->roMode;
    shm()->roMode = static_cast<readoutMode>(retval);

    if (oldmode != shm()->roMode) {
        // update #nchan, as it depends on #samples, adcmask,
        if (shm()->myDetectorType == CHIPTESTBOARD) {
            updateNumberOfChannels();
        }
        if (shm()->useReceiverFlag) {
            sendToReceiver(F_RECEIVER_SET_READOUT_MODE, shm()->roMode, nullptr);
        }
    }
    return shm()->roMode;
}

void slsDetector::setInterruptSubframe(const bool enable) {
    int arg = static_cast<int>(enable);
    FILE_LOG(logDEBUG1) << "Setting Interrupt subframe to " << arg;
    sendToDetector(F_SET_INTERRUPT_SUBFRAME, arg, nullptr);
}

bool slsDetector::getInterruptSubframe() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Interrupt subframe";
    sendToDetector(F_GET_INTERRUPT_SUBFRAME, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Interrupt subframe: " << retval;
    return static_cast<bool>(retval);
}

uint32_t slsDetector::writeRegister(uint32_t addr, uint32_t val) {
    uint32_t args[]{addr, val};
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Writing to reg 0x" << std::hex << addr << "data: 0x"
                        << std::hex << val << std::dec;
    sendToDetector(F_WRITE_REGISTER, args, retval);
    FILE_LOG(logDEBUG1) << "Reg 0x" << std::hex << addr << ": 0x" << std::hex
                        << retval << std::dec;
    return retval;
}

uint32_t slsDetector::readRegister(uint32_t addr) {
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Reading reg 0x" << std::hex << addr << std::dec;
    sendToDetector(F_READ_REGISTER, addr, retval);
    FILE_LOG(logDEBUG1) << "Reg 0x" << std::hex << addr << ": 0x" << std::hex
                        << retval << std::dec;
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

std::string slsDetector::setReceiverHostname(const std::string &receiverIP) {
    FILE_LOG(logDEBUG1) << "Setting up Receiver with " << receiverIP;
    // recieverIP is none
    if (receiverIP == "none") {
        memset(shm()->rxHostname, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(shm()->rxHostname, "none");
        shm()->useReceiverFlag = false;
        return std::string(shm()->rxHostname);
    }

    // stop acquisition if running
    if (getRunStatus() == RUNNING) {
        FILE_LOG(logWARNING) << "Acquisition already running, Stopping it.";
        stopAcquisition();
    }
    // update detector before receiver
    updateCachedDetectorVariables();

    // start updating
    sls::strcpy_safe(shm()->rxHostname, receiverIP.c_str());
    shm()->useReceiverFlag = true;
    checkReceiverVersionCompatibility();

    FILE_LOG(logDEBUG)
        << "detector type:"
        << (ToString(shm()->myDetectorType))
        << "\ndetector id:" << detId
        << "\ndetector hostname:" << shm()->hostname
        << "\nfile path:" << shm()->rxFilePath
        << "\nfile name:" << shm()->rxFileName
        << "\nfile index:" << shm()->rxFileIndex
        << "\nfile format:" << shm()->rxFileFormat
        << "\nr_framesperfile:" << shm()->rxFramesPerFile
        << "\nr_discardpolicy:" << shm()->rxFrameDiscardMode
        << "\nr_padding:" << shm()->rxFramePadding
        << "\nwrite enable:" << shm()->rxFileWrite
        << "\nmaster write enable:" << shm()->rxMasterFileWrite
        << "\noverwrite enable:" << shm()->rxFileOverWrite
        << "\ndynamic range:" << shm()->dynamicRange
        << "\nflippeddatax:" << (shm()->flippedDataX)
        << "\nactivated: " << shm()->activated
        << "\nreceiver deactivated padding: " << shm()->rxPadDeactivatedModules
        << "\nsilent Mode:" << shm()->rxSilentMode
        << "\n10GbE:" << shm()->tenGigaEnable
        << "\nGap pixels: " << shm()->gappixels
        << "\nr_readfreq:" << shm()->rxReadFreq
        << "\nrx streaming port:" << shm()->rxZmqport
        << "\nrx streaming source ip:" << shm()->rxZmqip
        << "\nrx additional json header:" << shm()->rxAdditionalJsonHeader
        << "\nrx_datastream:" << enableDataStreamingFromReceiver(-1)
        << "\nrx_dbitlistsize:" << shm()->rxDbitList.size()
        << "\nrx_DbitOffset:" << shm()->rxDbitOffset << std::endl;

    if (setDetectorType(shm()->myDetectorType) != GENERIC) {
        sendMultiDetectorSize();
        setDetectorId();
        setDetectorHostname();
        
        // setup udp
        updateRxDestinationUDPIP();
        setDestinationUDPPort(getDestinationUDPPort());
        if (shm()->myDetectorType == JUNGFRAU || shm()->myDetectorType == EIGER ) {        
            setDestinationUDPPort2(getDestinationUDPPort2());
        }
        if (shm()->myDetectorType == JUNGFRAU) {
            updateRxDestinationUDPIP2();   
            setNumberofUDPInterfaces(getNumberofUDPInterfaces());
        }
        FILE_LOG(logDEBUG1) << printReceiverConfiguration();

        setReceiverUDPSocketBufferSize(0);
        setFilePath(shm()->rxFilePath);
        setFileName(shm()->rxFileName);
        setFileIndex(shm()->rxFileIndex);
        setFileFormat(shm()->rxFileFormat);
        setFramesPerFile(shm()->rxFramesPerFile);
        setReceiverFramesDiscardPolicy(shm()->rxFrameDiscardMode);
        setPartialFramesPadding(shm()->rxFramePadding);
        setFileWrite(shm()->rxFileWrite);
        setMasterFileWrite(shm()->rxMasterFileWrite);
        setFileOverWrite(shm()->rxFileOverWrite);
        sendTotalNumFramestoReceiver();
        setExptime(getExptime());
        setPeriod(getPeriod());

        // detector specific
        switch (shm()->myDetectorType) {

        case EIGER:
            setSubExptime(getSubExptime());
            setSubDeadTime(getSubDeadTime());
            setDynamicRange(shm()->dynamicRange);
            setFlippedDataX(-1);
            activate(-1);
            setDeactivatedRxrPaddingMode(
                static_cast<int>(shm()->rxPadDeactivatedModules));
            enableGapPixels(shm()->gappixels);
            enableTenGigabitEthernet(shm()->tenGigaEnable);
            setQuad(getQuad());
            break;

        case CHIPTESTBOARD:
            setNumberOfAnalogSamples(getNumberOfAnalogSamples());
            setNumberOfDigitalSamples(getNumberOfDigitalSamples());
            enableTenGigabitEthernet(shm()->tenGigaEnable);
            setReadoutMode(shm()->roMode);
            setADCEnableMask(shm()->adcEnableMask);
            setReceiverDbitOffset(shm()->rxDbitOffset);
            setReceiverDbitList(shm()->rxDbitList);
            break;

        case MOENCH:
            setNumberOfAnalogSamples(getNumberOfAnalogSamples());
            setNumberOfDigitalSamples(getNumberOfDigitalSamples());
            enableTenGigabitEthernet(shm()->tenGigaEnable);
            setADCEnableMask(shm()->adcEnableMask);
            break;

        case GOTTHARD:
            sendROItoReceiver();
            break;

        default:
            break;
        }

        setReceiverSilentMode(static_cast<int>(shm()->rxSilentMode));
        // data streaming
        setReceiverStreamingFrequency(shm()->rxReadFreq);
        setReceiverStreamingPort(getReceiverStreamingPort());
        updateReceiverStreamingIP();
        setAdditionalJsonHeader(shm()->rxAdditionalJsonHeader);
        enableDataStreamingFromReceiver(
            static_cast<int>(enableDataStreamingFromReceiver(-1)));
    }

    return std::string(shm()->rxHostname);
}

std::string slsDetector::getReceiverHostname() const {
    return std::string(shm()->rxHostname);
}

void slsDetector::setSourceUDPMAC(const sls::MacAddr mac) {
    FILE_LOG(logDEBUG1) << "Setting source udp mac to " << mac;
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address");
    }
    sendToDetector(F_SET_SOURCE_UDP_MAC, mac, nullptr); 
}

sls::MacAddr slsDetector::getSourceUDPMAC() {
    sls::MacAddr retval(0lu);
    FILE_LOG(logDEBUG1) << "Getting source udp mac";
    sendToDetector(F_GET_SOURCE_UDP_MAC, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Source udp mac: " << retval;
    return retval;
}

void slsDetector::setSourceUDPMAC2(const sls::MacAddr mac) {
    FILE_LOG(logDEBUG1) << "Setting source udp mac2 to " << mac;
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address2");
    }
    sendToDetector(F_SET_SOURCE_UDP_MAC2, mac, nullptr); 
}

sls::MacAddr slsDetector::getSourceUDPMAC2() {
    sls::MacAddr retval(0lu);
    FILE_LOG(logDEBUG1) << "Getting source udp mac2";
    sendToDetector(F_GET_SOURCE_UDP_MAC2, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Source udp mac2: " << retval;
    return retval;
}

void slsDetector::setSourceUDPIP(const IpAddr ip) {
    FILE_LOG(logDEBUG1) << "Setting source udp ip to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address");
    }

    sendToDetector(F_SET_SOURCE_UDP_IP, ip, nullptr); 
}

sls::IpAddr slsDetector::getSourceUDPIP() {
    sls::IpAddr retval(0u);
    FILE_LOG(logDEBUG1) << "Getting source udp ip";
    sendToDetector(F_GET_SOURCE_UDP_IP, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Source udp ip: " << retval;
    return retval;
}

void slsDetector::setSourceUDPIP2(const IpAddr ip) {
    FILE_LOG(logDEBUG1) << "Setting source udp ip2 to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address2");
    }

    sendToDetector(F_SET_SOURCE_UDP_IP2, ip, nullptr); 
}

sls::IpAddr slsDetector::getSourceUDPIP2() {
    sls::IpAddr retval(0u);
    FILE_LOG(logDEBUG1) << "Getting source udp ip2";
    sendToDetector(F_GET_SOURCE_UDP_IP2, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Source udp ip2: " << retval;
    return retval;
}

void slsDetector::setDestinationUDPIP(const IpAddr ip) {
    FILE_LOG(logDEBUG1) << "Setting destination udp ip to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address");
    }
    sendToDetector(F_SET_DEST_UDP_IP, ip, nullptr); 
    if (shm()->useReceiverFlag) {
        sls::MacAddr retval(0lu);
        sendToReceiver(F_SET_RECEIVER_UDP_IP, ip, retval);
        FILE_LOG(logINFO) << "Setting destination udp mac to " << retval;
        sendToDetector(F_SET_DEST_UDP_MAC, retval, nullptr); 
    }   
}

sls::IpAddr slsDetector::getDestinationUDPIP() {
    sls::IpAddr retval(0u);
    FILE_LOG(logDEBUG1) << "Getting destination udp ip";
    sendToDetector(F_GET_DEST_UDP_IP, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Destination udp ip: " << retval;
    return retval;
}

void slsDetector::updateRxDestinationUDPIP() {
    auto ip = getDestinationUDPIP();
    if (ip == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        ip = shm()->rxHostname;
        if (ip == 0) {
            ip = HostnameToIp(shm()->rxHostname);
        }  
        FILE_LOG(logINFO) << "Setting destination default udp ip to " << ip;
    }     
    setDestinationUDPIP(ip);    
}

void slsDetector::setDestinationUDPIP2(const IpAddr ip) {
    FILE_LOG(logDEBUG1) << "Setting destination udp ip2 to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address2");
    }

    sendToDetector(F_SET_DEST_UDP_IP2, ip, nullptr); 
    if (shm()->useReceiverFlag) {
        sls::MacAddr retval(0lu);
        sendToReceiver(F_SET_RECEIVER_UDP_IP2, ip, retval);
        FILE_LOG(logINFO) << "Setting destination udp mac2 to " << retval;
        sendToDetector(F_SET_DEST_UDP_MAC2, retval, nullptr); 
    }   
}

sls::IpAddr slsDetector::getDestinationUDPIP2() {
    sls::IpAddr retval(0u);
    FILE_LOG(logDEBUG1) << "Getting destination udp ip2";
    sendToDetector(F_GET_DEST_UDP_IP2, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Destination udp ip2: " << retval;
    return retval;
}

void slsDetector::updateRxDestinationUDPIP2() {
    auto ip = getDestinationUDPIP2();
    if (ip == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        ip = shm()->rxHostname;
        if (ip == 0) {
            ip = HostnameToIp(shm()->rxHostname);
        }  
        FILE_LOG(logINFO) << "Setting destination default udp ip2 to " << ip;    
    }     
    setDestinationUDPIP2(ip);       
}

void slsDetector::setDestinationUDPMAC(const MacAddr mac) {
    FILE_LOG(logDEBUG1) << "Setting destination udp mac to " << mac;
    if (mac == 0) {
        throw RuntimeError("Invalid destination udp mac address");
    }

    sendToDetector(F_SET_DEST_UDP_MAC, mac, nullptr); 
}

sls::MacAddr slsDetector::getDestinationUDPMAC() {
    sls::MacAddr retval(0lu);
    FILE_LOG(logDEBUG1) << "Getting destination udp mac";
    sendToDetector(F_GET_DEST_UDP_MAC, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Destination udp mac: " << retval;
    return retval;
}

void slsDetector::setDestinationUDPMAC2(const MacAddr mac) {
    FILE_LOG(logDEBUG1) << "Setting destination udp mac2 to " << mac;
    if (mac == 0) {
        throw RuntimeError("Invalid desinaion udp mac address2");
    }

    sendToDetector(F_SET_DEST_UDP_MAC2, mac, nullptr); 
}

sls::MacAddr slsDetector::getDestinationUDPMAC2() {
    sls::MacAddr retval(0lu);
    FILE_LOG(logDEBUG1) << "Getting destination udp mac2";
    sendToDetector(F_GET_DEST_UDP_MAC2, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Destination udp mac2: " << retval;
    return retval;
}

void slsDetector::setDestinationUDPPort(const int port) {
    FILE_LOG(logDEBUG1) << "Setting destination udp port to " << port;
    sendToDetector(F_SET_DEST_UDP_PORT, port, nullptr); 
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_UDP_PORT, port, nullptr); 
    }   
}

int slsDetector::getDestinationUDPPort() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting destination udp port";
    sendToDetector(F_GET_DEST_UDP_PORT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Destination udp port: " << retval;

    return retval;
}

void slsDetector::setDestinationUDPPort2(const int port) {
    FILE_LOG(logDEBUG1) << "Setting destination udp port2 to " << port;
    sendToDetector(F_SET_DEST_UDP_PORT2, port, nullptr); 
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_UDP_PORT2, port, nullptr); 
    }   
}

int slsDetector::getDestinationUDPPort2() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting destination udp port2";
    sendToDetector(F_GET_DEST_UDP_PORT2, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Destination udp port2: " << retval;
    return retval;
}

void slsDetector::setNumberofUDPInterfaces(int n) {
    FILE_LOG(logDEBUG1) << "Setting number of udp interfaces to " << n;
    sendToDetector(F_SET_NUM_INTERFACES, n, nullptr);
    shm()->numUDPInterfaces = n;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_INTERFACES, n, nullptr); 
    }  
}

int slsDetector::getNumberofUDPInterfacesFromShm() {
    return shm()->numUDPInterfaces;
} 


int slsDetector::getNumberofUDPInterfaces() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting number of udp interfaces";
    sendToDetector(F_GET_NUM_INTERFACES, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Number of udp interfaces: " << retval;
    shm()->numUDPInterfaces = retval;
    return shm()->numUDPInterfaces;
}

void slsDetector::selectUDPInterface(int n) {
    FILE_LOG(logDEBUG1) << "Setting selected udp interface to " << n;
    sendToDetector(F_SET_INTERFACE_SEL, n, nullptr); 
}

int slsDetector::getSelectedUDPInterface() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting selected udp interface";
    sendToDetector(F_GET_INTERFACE_SEL, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Selected udp interface: " << retval;
    return retval;
}

void slsDetector::setClientStreamingPort(int port) { shm()->zmqport = port; }

int slsDetector::getClientStreamingPort() { return shm()->zmqport; }

void slsDetector::setReceiverStreamingPort(int port) {
    int fnum = F_SET_RECEIVER_STREAMING_PORT;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending receiver streaming port to receiver: "
                        << port;
    if (shm()->useReceiverFlag) {
        sendToReceiver(fnum, port, retval);
        FILE_LOG(logDEBUG1) << "Receiver streaming port: " << retval;
        shm()->rxZmqport = retval;
    } else {
        shm()->rxZmqport = port;
    }
}

int slsDetector::getReceiverStreamingPort() { return shm()->rxZmqport; }

void slsDetector::setClientStreamingIP(const sls::IpAddr ip) {
    FILE_LOG(logDEBUG1) << "Setting client zmq ip to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid client zmq ip address");
    } 
    shm()->zmqip = ip;  
}

sls::IpAddr slsDetector::getClientStreamingIP() { return shm()->zmqip; }

void slsDetector::setReceiverStreamingIP(const sls::IpAddr ip) {
    FILE_LOG(logDEBUG1) << "Setting rx zmq ip to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid receiver zmq ip address");
    }
    
    // if zmqip is empty, update it
    if (shm()->zmqip == 0) {
        shm()->zmqip = ip;
    }

    // send to receiver
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1)
            << "Sending receiver streaming IP to receiver: " << ip;
        sendToReceiver(F_RECEIVER_STREAMING_SRC_IP, ip, nullptr);
    } else {
        shm()->rxZmqip = ip;
    }
}

sls::IpAddr slsDetector::getReceiverStreamingIP() {
    return shm()->rxZmqip;
}

void slsDetector::updateReceiverStreamingIP() {
    auto ip = getReceiverStreamingIP();
    if (ip == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        ip = shm()->rxHostname;
        if (ip == 0) {
            ip = HostnameToIp(shm()->rxHostname);
        }  
        FILE_LOG(logINFO) << "Setting default receiver streaming zmq ip to " << ip;
    }     
    setReceiverStreamingIP(ip);   
}


bool slsDetector::getTenGigaFlowControl() {
    int retval = -1;
    sendToDetector(F_GET_TEN_GIGA_FLOW_CONTROL, nullptr, retval);
    FILE_LOG(logDEBUG1) << "ten giga flow control :" << retval;
    return retval == 1 ? true : false;
}

void slsDetector::setTenGigaFlowControl(bool enable) {
    int arg = static_cast<int>(enable);
    FILE_LOG(logDEBUG1) << "Setting ten giga flow control to " << arg;
    sendToDetector(F_SET_TEN_GIGA_FLOW_CONTROL, arg, nullptr);
}

int slsDetector::getTransmissionDelayFrame() {
    int retval = -1;
    sendToDetector(F_GET_TRANSMISSION_DELAY_FRAME, nullptr, retval);
    FILE_LOG(logDEBUG1) << "transmission delay frame :" << retval;
    return retval; 
}

void slsDetector::setTransmissionDelayFrame(int value) {
    FILE_LOG(logDEBUG1) << "Setting transmission delay frame to " << value;
    sendToDetector(F_SET_TRANSMISSION_DELAY_FRAME, value, nullptr);
}

int slsDetector::getTransmissionDelayLeft() {
    int retval = -1;
    sendToDetector(F_GET_TRANSMISSION_DELAY_LEFT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "transmission delay left :" << retval;
    return retval; 
}

void slsDetector::setTransmissionDelayLeft(int value) {
    FILE_LOG(logDEBUG1) << "Setting transmission delay left to " << value;
    sendToDetector(F_SET_TRANSMISSION_DELAY_LEFT, value, nullptr);
}

int slsDetector::getTransmissionDelayRight() {
    int retval = -1;
    sendToDetector(F_GET_TRANSMISSION_DELAY_RIGHT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "transmission delay right :" << retval;
    return retval; 
}

void slsDetector::setTransmissionDelayRight(int value) {
    FILE_LOG(logDEBUG1) << "Setting transmission delay right to " << value;
    sendToDetector(F_SET_TRANSMISSION_DELAY_RIGHT, value, nullptr);
}


std::string
slsDetector::setAdditionalJsonHeader(const std::string &jsonheader) {
    int fnum = F_ADDITIONAL_JSON_HEADER;
    char args[MAX_STR_LENGTH]{};
    char retvals[MAX_STR_LENGTH]{};
    sls::strcpy_safe(args, jsonheader.c_str());
    FILE_LOG(logDEBUG1) << "Sending additional json header " << args;

    if (shm()->useReceiverFlag) {
        sendToReceiver(fnum, args, retvals);
        FILE_LOG(logDEBUG1) << "Additional json header: " << retvals;
        memset(shm()->rxAdditionalJsonHeader, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(shm()->rxAdditionalJsonHeader, retvals);
    } else {
        sls::strcpy_safe(shm()->rxAdditionalJsonHeader, jsonheader.c_str());
    }
    return shm()->rxAdditionalJsonHeader;
}

std::string slsDetector::getAdditionalJsonHeader() {
    int fnum = F_GET_ADDITIONAL_JSON_HEADER;
    char retvals[MAX_STR_LENGTH]{};
    FILE_LOG(logDEBUG1) << "Getting additional json header ";
    if (shm()->useReceiverFlag) {
        sendToReceiver(fnum, nullptr, retvals);
        FILE_LOG(logDEBUG1) << "Additional json header: " << retvals;
        memset(shm()->rxAdditionalJsonHeader, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(shm()->rxAdditionalJsonHeader, retvals);
    }
    return std::string(shm()->rxAdditionalJsonHeader);
}

std::string slsDetector::setAdditionalJsonParameter(const std::string &key,
                                                    const std::string &value) {
    if (key.empty() || value.empty()) {
        throw RuntimeError(
            "Could not set additional json header parameter as the key or "
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

    std::string header(shm()->rxAdditionalJsonHeader);
    size_t keyPos = header.find(keyLiteral);

    // if key found, replace value
    if (keyPos != std::string::npos) {
        size_t valueStartPos = header.find(std::string(":"), keyPos) + 1;
        size_t valueEndPos = header.find(std::string(","), valueStartPos) - 1;
        // if valueEndPos doesnt find comma (end of string), it goes anyway to
        // end of line
        header.replace(valueStartPos, valueEndPos - valueStartPos + 1,
                       valueLiteral);
    }

    // key not found, append key value pair
    else {
        if (header.length() != 0u) {
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
    if (strlen(shm()->rxAdditionalJsonHeader) == 0u)
        return std::string();

    // add quotations before and after the key value
    std::string keyLiteral = key;
    keyLiteral.insert(0, "\"");
    keyLiteral.append("\"");

    // loop through the parameters
    for (const auto &parameter :
         sls::split(shm()->rxAdditionalJsonHeader, ',')) {
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
    return std::string();
}

int64_t slsDetector::setReceiverUDPSocketBufferSize(int64_t udpsockbufsize) {
    FILE_LOG(logDEBUG1) << "Sending UDP Socket Buffer size to receiver: "
                        << udpsockbufsize;
    int64_t retval = -1;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_UDP_SOCK_BUF_SIZE, udpsockbufsize, retval);
        FILE_LOG(logDEBUG1) << "Receiver UDP Socket Buffer size: " << retval;
    }
    return retval;
}

int64_t slsDetector::getReceiverUDPSocketBufferSize() {
    return setReceiverUDPSocketBufferSize();
}

int64_t slsDetector::getReceiverRealUDPSocketBufferSize() const {
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting real UDP Socket Buffer size from receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE, nullptr, retval);
        FILE_LOG(logDEBUG1)
            << "Real Receiver UDP Socket Buffer size: " << retval;
    }
    return retval;
}

void slsDetector::executeFirmwareTest() {
    FILE_LOG(logDEBUG1) << "Executing firmware test";
    sendToDetector(F_SET_FIRMWARE_TEST);
}

void slsDetector::executeBusTest() {
    FILE_LOG(logDEBUG1) << "Executing bus test";
    sendToDetector(F_SET_BUS_TEST);
}

int slsDetector::getImageTestMode() {
    int retval = -1;
    sendToDetector(F_GET_IMAGE_TEST_MODE, nullptr, retval);
    FILE_LOG(logDEBUG1) << "image test mode: " << retval;
    return retval;
}

void slsDetector::setImageTestMode(const int value) {
    FILE_LOG(logDEBUG1) << "Sending image test mode " << value;
    sendToDetector(F_SET_IMAGE_TEST_MODE, value, nullptr);
}

std::array<int, 2> slsDetector::getInjectChannel() {
    std::array<int, 2> retvals{};
    sendToDetector(F_GET_INJECT_CHANNEL, nullptr, retvals);
    FILE_LOG(logDEBUG1) << "Inject Channel: [offset: " << retvals[0] << ", increment: " << retvals[1] << ']';
    return retvals; 
}

void slsDetector::setInjectChannel(int offsetChannel, int incrementChannel) {
    int args[]{offsetChannel, incrementChannel};
    FILE_LOG(logDEBUG1) << "Setting inject channels [offset: " << offsetChannel << ", increment: " << incrementChannel << ']';
    sendToDetector(F_SET_INJECT_CHANNEL, args, nullptr);
}

int slsDetector::setCounterBit(int cb) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending counter bit " << cb;
    sendToDetector(F_SET_COUNTER_BIT, cb, retval);
    FILE_LOG(logDEBUG1) << "Counter bit: " << retval;
    return retval;
}

void slsDetector::clearROI() {
    FILE_LOG(logDEBUG1) << "Clearing ROI";
    slsDetectorDefs::ROI arg;
    arg.xmin = -1;
    arg.xmax = -1;
    setROI(arg);
}

void slsDetector::setROI(slsDetectorDefs::ROI arg) {
    int fnum = F_SET_ROI;
    int ret = FAIL;
    if (arg.xmin < 0 || arg.xmax >= getNumberOfChannels().x) {
        arg.xmin = -1;
        arg.xmax = -1;
    }
    FILE_LOG(logDEBUG) << "Sending ROI to detector [" << arg.xmin << ", "
                       << arg.xmax << "]";
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(&fnum, sizeof(fnum));
    client.Send(&arg.xmin, sizeof(int));
    client.Send(&arg.xmax, sizeof(int));
    client.Receive(&ret, sizeof(ret));
    // handle ret
    if (ret == FAIL) {
        char mess[MAX_STR_LENGTH]{};
        client.Receive(mess, MAX_STR_LENGTH);
        throw RuntimeError("Detector " + std::to_string(detId) +
                           " returned error: " + std::string(mess));
    } else {
        memcpy(&shm()->roi, &arg, sizeof(ROI));
        if (ret == FORCE_UPDATE) {
            updateCachedDetectorVariables();
        }
    }

    sendROItoReceiver();
}

void slsDetector::sendROItoReceiver() {
    // update roi in receiver
    if (shm()->useReceiverFlag) {
        FILE_LOG(logDEBUG1) << "Sending ROI to receiver";
        sendToReceiver(F_RECEIVER_SET_ROI, shm()->roi, nullptr);
    }
}

slsDetectorDefs::ROI slsDetector::getROI() {
    int fnum = F_GET_ROI;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Getting ROI from detector";
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(&fnum, sizeof(fnum));
    client.Receive(&ret, sizeof(ret));
    // handle ret
    if (ret == FAIL) {
        char mess[MAX_STR_LENGTH]{};
        client.Receive(mess, MAX_STR_LENGTH);
        throw RuntimeError("Detector " + std::to_string(detId) +
                           " returned error: " + std::string(mess));
    } else {
        ROI retval;
        client.Receive(&retval.xmin, sizeof(int));
        client.Receive(&retval.xmax, sizeof(int));
        FILE_LOG(logDEBUG1)
            << "ROI retval [" << retval.xmin << "," << retval.xmax << "]";
        if (ret == FORCE_UPDATE) {
            updateCachedDetectorVariables();
        }
        // if different from shm, update and send to receiver
        if (shm()->roi.xmin != retval.xmin || shm()->roi.xmax != retval.xmax) {
            memcpy(&shm()->roi, &retval, sizeof(ROI));
            sendROItoReceiver();
        }
    }

    return shm()->roi;
}

void slsDetector::setADCEnableMask(uint32_t mask) {
    uint32_t arg = mask;
    FILE_LOG(logDEBUG1) << "Setting ADC Enable mask to 0x" << std::hex << arg
                        << std::dec;
    sendToDetector(F_SET_ADC_ENABLE_MASK, &arg, sizeof(arg), nullptr, 0);
    shm()->adcEnableMask = mask;

    // update #nchan, as it depends on #samples, adcmask,
    updateNumberOfChannels();

    // send to processor
    if (shm()->myDetectorType == MOENCH)
        setAdditionalJsonParameter("adcmask",
                                   std::to_string(shm()->adcEnableMask));

    if (shm()->useReceiverFlag) {
        int fnum = F_RECEIVER_SET_ADC_MASK;
        int retval = -1;
        mask = shm()->adcEnableMask;
        FILE_LOG(logDEBUG1) << "Setting ADC Enable mask to 0x" << std::hex
                            << mask << std::dec << " in receiver";
        sendToReceiver(fnum, mask, retval);
    }
}

uint32_t slsDetector::getADCEnableMask() {
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting ADC Enable mask";
    sendToDetector(F_GET_ADC_ENABLE_MASK, nullptr, 0, &retval, sizeof(retval));
    shm()->adcEnableMask = retval;
    FILE_LOG(logDEBUG1) << "ADC Enable Mask: 0x" << std::hex << retval
                        << std::dec;
    return shm()->adcEnableMask;
}

void slsDetector::setADCInvert(uint32_t value) {
    FILE_LOG(logDEBUG1) << "Setting ADC Invert to 0x" << std::hex << value
                        << std::dec;
    sendToDetector(F_SET_ADC_INVERT, value, nullptr);
}

uint32_t slsDetector::getADCInvert() {
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting ADC Invert";
    sendToDetector(F_GET_ADC_INVERT, nullptr, retval);
    FILE_LOG(logDEBUG1) << "ADC Invert: 0x" << std::hex << retval << std::dec;
    return retval;
}

int slsDetector::setExternalSamplingSource(int value) {
    int arg = value;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting External Sampling Source to " << arg;
    sendToDetector(F_EXTERNAL_SAMPLING_SOURCE, arg, retval);
    FILE_LOG(logDEBUG1) << "External Sampling source: " << retval;
    return retval;
}

int slsDetector::getExternalSamplingSource() {
    return setExternalSamplingSource(-1);
}

int slsDetector::setExternalSampling(int value) {
    int arg = value;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting External Sampling to " << arg;
    sendToDetector(F_EXTERNAL_SAMPLING, arg, retval);
    FILE_LOG(logDEBUG1) << "External Sampling: " << retval;
    return retval;
}

int slsDetector::getExternalSampling() { return setExternalSampling(-1); }

void slsDetector::setReceiverDbitList(std::vector<int> list) {
    FILE_LOG(logDEBUG1) << "Setting Receiver Dbit List";

    if (list.size() > 64) {
        throw sls::RuntimeError("Dbit list size cannot be greater than 64\n");
    }
    for (auto &it : list) {
        if (it < 0 || it > 63) {
            throw sls::RuntimeError(
                "Dbit list value must be between 0 and 63\n");
        }
    }
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> arg = list;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_DBIT_LIST, arg, nullptr);
    } else {
        shm()->rxDbitList = list;
    }
}

std::vector<int> slsDetector::getReceiverDbitList() const {
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> retval;
    FILE_LOG(logDEBUG1) << "Getting Receiver Dbit List";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_GET_RECEIVER_DBIT_LIST, nullptr, retval);
        shm()->rxDbitList = retval;
    }
    return shm()->rxDbitList;
}

int slsDetector::setReceiverDbitOffset(int value) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting digital bit offset in receiver to "
                        << value;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_DBIT_OFFSET, value, retval);
        FILE_LOG(logDEBUG1) << "Receiver digital bit offset: " << retval;
    } else {
        shm()->rxDbitOffset = value;
    }
    return shm()->rxDbitOffset;
}

int slsDetector::getReceiverDbitOffset() { return shm()->rxDbitOffset; }

void slsDetector::writeAdcRegister(uint32_t addr, uint32_t val) {
    uint32_t args[]{addr, val};
    FILE_LOG(logDEBUG1) << "Writing to ADC register 0x" << std::hex << addr
                        << "data: 0x" << std::hex << val << std::dec;
    sendToDetector(F_WRITE_ADC_REG, args, nullptr);
}

int slsDetector::activate(int enable) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting activate flag to " << enable;
    sendToDetector(F_ACTIVATE, enable, retval);
    FILE_LOG(logDEBUG1) << "Activate: " << retval;
    shm()->activated = static_cast<bool>(retval);
    if (shm()->useReceiverFlag) {
        int fnum = F_RECEIVER_ACTIVATE;
        enable = static_cast<int>(shm()->activated);
        retval = -1;
        FILE_LOG(logDEBUG1)
            << "Setting activate flag " << enable << " to receiver";
        sendToReceiver(fnum, enable, retval);
    }
    return static_cast<int>(shm()->activated);
}

bool slsDetector::setDeactivatedRxrPaddingMode(int padding) {
    int fnum = F_RECEIVER_DEACTIVATED_PADDING_ENABLE;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable: " << padding;
    if (shm()->useReceiverFlag) {
        sendToReceiver(fnum, padding, retval);
        FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable:" << retval;
        shm()->rxPadDeactivatedModules = static_cast<bool>(retval);
    } else {
        shm()->rxPadDeactivatedModules = padding;
    }
    return shm()->rxPadDeactivatedModules;
}

int slsDetector::getFlippedDataX() const { return shm()->flippedDataX; }

int slsDetector::setFlippedDataX(int value) {
    // replace get with shm value (write to shm right away as it is a det value,
    // not rx value)
    if (value > -1) {
        shm()->flippedDataX = (value > 0) ? 1 : 0;
    }
    int retval = -1;
    int arg = shm()->flippedDataX;
    FILE_LOG(logDEBUG1) << "Setting flipped data across x axis with value: "
                        << arg;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_FLIPPED_DATA_RECEIVER, arg, retval);
        FILE_LOG(logDEBUG1) << "Flipped data:" << retval;
    }
    return shm()->flippedDataX;
}

int slsDetector::setAllTrimbits(int val) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting all trimbits to " << val;
    sendToDetector(F_SET_ALL_TRIMBITS, val, retval);
    FILE_LOG(logDEBUG1) << "All trimbit value: " << retval;
    return retval;
}

int slsDetector::enableGapPixels(int val) {
    if (val >= 0) {
        if (shm()->myDetectorType != EIGER) {
          throw NotImplementedError(
                "Function (enableGapPixels) not implemented for this detector");  
        }
        int fnum = F_ENABLE_GAPPIXELS_IN_RECEIVER;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Sending gap pixels enable to receiver: " << val;
        if (shm()->useReceiverFlag) {
            sendToReceiver(fnum, val, retval);
            FILE_LOG(logDEBUG1) << "Gap pixels enable to receiver:" << retval;
            shm()->gappixels = retval;
        }
    }
    return shm()->gappixels;
}

int slsDetector::setTrimEn(std::vector<int> energies) {
    if (shm()->myDetectorType != EIGER) {
         throw RuntimeError("Not implemented for this detector.");
    }
    if (energies.size() > MAX_TRIMEN) {
        std::ostringstream os;
        os << "Size of trim energies: " << energies.size()
           << " exceeds what can be stored in shared memory: " << MAX_TRIMEN
           << "\n";
        throw RuntimeError(os.str());
    }
    shm()->trimEnergies = energies;
    return shm()->trimEnergies.size();
}

std::vector<int> slsDetector::getTrimEn() {
    if (shm()->myDetectorType != EIGER) {
         throw RuntimeError("Not implemented for this detector.");
    }
    return std::vector<int>(shm()->trimEnergies.begin(),
                            shm()->trimEnergies.end());
}

void slsDetector::pulsePixel(int n, int x, int y) {
    int args[]{n, x, y};
    FILE_LOG(logDEBUG1) << "Pulsing pixel " << n << " number of times at (" << x
                        << "," << y << ")";
    sendToDetector(F_PULSE_PIXEL, args, nullptr);
}

void slsDetector::pulsePixelNMove(int n, int x, int y) {
    int args[]{n, x, y};
    FILE_LOG(logDEBUG1) << "Pulsing pixel " << n
                        << " number of times and move by delta (" << x << ","
                        << y << ")";
    sendToDetector(F_PULSE_PIXEL_AND_MOVE, args, nullptr);
}

void slsDetector::pulseChip(int n_pulses) {
    FILE_LOG(logDEBUG1) << "Pulsing chip " << n_pulses << " number of times";
    sendToDetector(F_PULSE_CHIP, n_pulses, nullptr);
}

int slsDetector::setThresholdTemperature(int val) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting threshold temperature to " << val;
    sendToDetectorStop(F_THRESHOLD_TEMP, val, retval);
    FILE_LOG(logDEBUG1) << "Threshold temperature: " << retval;
    return retval;
}

int slsDetector::setTemperatureControl(int val) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting temperature control to " << val;
    sendToDetectorStop(F_TEMP_CONTROL, val, retval);
    FILE_LOG(logDEBUG1) << "Temperature control: " << retval;
    return retval;
}

int slsDetector::setTemperatureEvent(int val) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting temperature event to " << val;
    sendToDetectorStop(F_TEMP_EVENT, val, retval);
    FILE_LOG(logDEBUG1) << "Temperature event: " << retval;
    return retval;
}

int slsDetector::setStoragecellStart(int pos) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting storage cell start to " << pos;
    sendToDetector(F_STORAGE_CELL_START, pos, retval);
    FILE_LOG(logDEBUG1) << "Storage cell start: " << retval;
    return retval;
}

void slsDetector::programFPGA(std::vector<char> buffer) {
    // validate type
    switch (shm()->myDetectorType) {
    case JUNGFRAU:
    case CHIPTESTBOARD:
    case MOENCH:
        break;
    default:
        throw RuntimeError("Program FPGA is not implemented for this detector");
    }

    size_t filesize = buffer.size();

    // send program from memory to detector
    int fnum = F_PROGRAM_FPGA;
    int ret = FAIL;
    char mess[MAX_STR_LENGTH] = {0};
    FILE_LOG(logINFO) << "Sending programming binary to detector " << detId
                      << " (" << shm()->hostname << ")";

    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(&fnum, sizeof(fnum));
    client.Send(&filesize, sizeof(filesize));
    client.Receive(&ret, sizeof(ret));
    // error in detector at opening file pointer to flash
    if (ret == FAIL) {
        client.Receive(mess, sizeof(mess));
        std::ostringstream os;
        os << "Detector " << detId << " (" << shm()->hostname << ")"
           << " returned error: " << mess;
        throw RuntimeError(os.str());
    }

    // erasing flash
    if (ret != FAIL) {
        FILE_LOG(logINFO) << "Erasing Flash for detector " << detId << " ("
                          << shm()->hostname << ")";
        printf("%d%%\r", 0);
        std::cout << std::flush;
        // erasing takes 65 seconds, printing here (otherwise need threads
        // in server-unnecessary)
        const int ERASE_TIME = 65;
        int count = ERASE_TIME + 1;
        while (count > 0) {
            usleep(1 * 1000 * 1000);
            --count;
            printf("%d%%\r",
                   static_cast<int>(
                       (static_cast<double>(ERASE_TIME - count) / ERASE_TIME) *
                       100));
            std::cout << std::flush;
        }
        printf("\n");
        FILE_LOG(logINFO) << "Writing to Flash to detector " << detId << " ("
                          << shm()->hostname << ")";
        printf("%d%%\r", 0);
        std::cout << std::flush;
    }

    // sending program in parts of 2mb each
    size_t unitprogramsize = 0;
    int currentPointer = 0;
    size_t totalsize = filesize;
    while (filesize > 0) {
        unitprogramsize = MAX_FPGAPROGRAMSIZE; // 2mb
        if (unitprogramsize > filesize) {      // less than 2mb
            unitprogramsize = filesize;
        }
        FILE_LOG(logDEBUG1) << "unitprogramsize:" << unitprogramsize
                            << "\t filesize:" << filesize;

        client.Send(&buffer[currentPointer], unitprogramsize);
        client.Receive(&ret, sizeof(ret));
        if (ret == FAIL) {
            printf("\n");
            client.Receive(mess, sizeof(mess));
            std::ostringstream os;
            os << "Detector " << detId << " (" << shm()->hostname << ")"
               << " returned error: " << mess;
            throw RuntimeError(os.str());
        } else {
            filesize -= unitprogramsize;
            currentPointer += unitprogramsize;

            // print progress
            printf("%d%%\r",
                   static_cast<int>(
                       (static_cast<double>(totalsize - filesize) / totalsize) *
                       100));
            std::cout << std::flush;
        }
    }
    printf("\n");
    rebootController();
}

void slsDetector::resetFPGA() {
    FILE_LOG(logDEBUG1) << "Sending reset FPGA";
    return sendToDetector(F_RESET_FPGA);
}

void slsDetector::copyDetectorServer(const std::string &fname,
                                     const std::string &hostname) {
    char args[2][MAX_STR_LENGTH]{};
    sls::strcpy_safe(args[0], fname.c_str());
    sls::strcpy_safe(args[1], hostname.c_str());
    FILE_LOG(logINFO) << "Sending detector server " << args[0] << " from host "
                      << args[1];
    sendToDetector(F_COPY_DET_SERVER, args, nullptr);
}

void slsDetector::rebootController() {
    if (shm()->myDetectorType == EIGER) {
        throw RuntimeError(
            "Reboot controller not implemented for this detector");
    }
    int fnum = F_REBOOT_CONTROLLER;
    FILE_LOG(logINFO) << "Sending reboot controller to detector " << detId
                      << " (" << shm()->hostname << ")";
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(&fnum, sizeof(fnum));
}

int slsDetector::powerChip(int ival) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting power chip to " << ival;
    sendToDetector(F_POWER_CHIP, ival, retval);
    FILE_LOG(logDEBUG1) << "Power chip: " << retval;
    return retval;
}

int slsDetector::setAutoComparatorDisableMode(int ival) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting auto comp disable mode to " << ival;
    sendToDetector(F_AUTO_COMP_DISABLE, ival, retval);
    FILE_LOG(logDEBUG1) << "Auto comp disable: " << retval;
    return retval;
}

void slsDetector::setModule(sls_detector_module &module, int tb) {
    int fnum = F_SET_MODULE;
    int ret = FAIL;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting module with tb:" << tb;
    // to exclude trimbits
    if (tb == 0) {
        module.nchan = 0;
        module.nchip = 0;
    }
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(&fnum, sizeof(fnum));
    sendModule(&module, client);
    client.Receive(&ret, sizeof(ret));
    if (ret == FAIL) {
        char mess[MAX_STR_LENGTH] = {0};
        client.Receive(mess, sizeof(mess));
        throw RuntimeError("Detector " + std::to_string(detId) +
                           " returned error: " + mess);
    }
    client.Receive(&retval, sizeof(retval));
    FILE_LOG(logDEBUG1) << "Set Module returned: " << retval;
    if (ret == FORCE_UPDATE) {
        updateCachedDetectorVariables();
    }
    // update client structure
    if (module.eV != -1) {
        shm()->currentThresholdEV = module.eV;
    }
}

sls_detector_module slsDetector::getModule() {
    int fnum = F_GET_MODULE;
    int ret = FAIL;
    FILE_LOG(logDEBUG1) << "Getting module";
    sls_detector_module myMod{shm()->myDetectorType};
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    ret = client.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
    receiveModule(&myMod, client);
    if (ret == FORCE_UPDATE) {
        updateCachedDetectorVariables();
    }
    if (myMod.eV != -1) {
        shm()->currentThresholdEV = myMod.eV;
    }
    return myMod;
}

void slsDetector::setDefaultRateCorrection() {
    FILE_LOG(logDEBUG1) << "Setting Default Rate Correction";
    int64_t arg = -1;
    sendToDetector(F_SET_RATE_CORRECT, arg, nullptr);
    shm()->deadTime = -1;
}

void slsDetector::setRateCorrection(int64_t t) {
    FILE_LOG(logDEBUG1) << "Setting Rate Correction to " << t;
    sendToDetector(F_SET_RATE_CORRECT, t, nullptr);
    shm()->deadTime = t;
}

int64_t slsDetector::getRateCorrection() {
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting rate correction";
    sendToDetector(F_GET_RATE_CORRECT, nullptr, retval);
    shm()->deadTime = retval;
    FILE_LOG(logDEBUG1) << "Rate correction: " << retval;
    return retval;
}

void slsDetector::updateRateCorrection() {
    if (shm()->deadTime != 0) {
        switch (shm()->dynamicRange) {
        case 16:
        case 32:
            setRateCorrection(shm()->deadTime);
            break;
        default:
            setRateCorrection(0);
            throw sls::RuntimeError(
                "Rate correction Deactivated, must be in 32 or 16 bit mode");
        }
    }
}

std::string slsDetector::printReceiverConfiguration() {
    std::ostringstream os;
    os << "\n\nDetector " << detId << "\nReceiver Hostname:\t"
       << getReceiverHostname();
    
    if (shm()->myDetectorType == JUNGFRAU) {  
        os << "\nNumber of Interfaces:\t" << getNumberofUDPInterfaces() 
        << "\nSelected Interface:\t" << getSelectedUDPInterface();
    }
        
    os << "\nDetector UDP IP:\t"
       << getSourceUDPIP() << "\nDetector UDP MAC:\t" 
       << getSourceUDPMAC() << "\nReceiver UDP IP:\t" 
       << getDestinationUDPIP() << "\nReceiver UDP MAC:\t" << getDestinationUDPMAC();

    if (shm()->myDetectorType == JUNGFRAU) {
        os << "\nDetector UDP IP2:\t" << getSourceUDPIP2() 
       << "\nDetector UDP MAC2:\t" << getSourceUDPMAC2()
       << "\nReceiver UDP IP2:\t" << getDestinationUDPIP2()      
       << "\nReceiver UDP MAC2:\t" << getDestinationUDPMAC2();
    }
    os << "\nReceiver UDP Port:\t" << getDestinationUDPPort();
    if (shm()->myDetectorType == JUNGFRAU || shm()->myDetectorType == EIGER) {
       os << "\nReceiver UDP Port2:\t" << getDestinationUDPPort2();
    }
    os << "\n";
    return os.str();
}

bool slsDetector::getUseReceiverFlag() const { return shm()->useReceiverFlag; }

int slsDetector::lockReceiver(int lock) {
    FILE_LOG(logDEBUG1) << "Setting receiver server lock to " << lock;
    int retval = -1;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_LOCK_RECEIVER, lock, retval);
        FILE_LOG(logDEBUG1) << "Receiver Lock: " << retval;
    }
    return retval;
}

sls::IpAddr slsDetector::getReceiverLastClientIP() const {
    sls::IpAddr retval = 0u;
    FILE_LOG(logDEBUG1) << "Getting last client ip to receiver server";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_GET_LAST_RECEIVER_CLIENT_IP, nullptr, retval);
        FILE_LOG(logDEBUG1) << "Last client IP from receiver: " << retval;
    }
    return retval;
}

void slsDetector::exitReceiver() {
    FILE_LOG(logDEBUG1) << "Sending exit command to receiver server";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_EXIT_RECEIVER);
    }
}

void slsDetector::execReceiverCommand(const std::string &cmd) {
    char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    sls::strcpy_safe(arg, cmd.c_str());
    FILE_LOG(logDEBUG1) << "Sending command to receiver: " << arg;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_EXEC_RECEIVER_COMMAND, arg, retval);
        FILE_LOG(logINFO) << "Receiver " << detId << " returned:\n" << retval;
    }
}

void slsDetector::updateCachedReceiverVariables() const {
    int fnum = F_UPDATE_RECEIVER_CLIENT;
    FILE_LOG(logDEBUG1) << "Sending update client to receiver server";

    if (shm()->useReceiverFlag) {
        auto receiver =
            sls::ClientSocket("Receiver", shm()->rxHostname, shm()->rxTCPPort);
        receiver.sendCommandThenRead(fnum, nullptr, 0, nullptr, 0);
        int n = 0, i32 = 0;
        int64_t i64 = 0;
        char cstring[MAX_STR_LENGTH]{};
        IpAddr ip = 0u;

        n += receiver.Receive(&ip, sizeof(ip));
        FILE_LOG(logDEBUG1)
            << "Updating receiver last modified by " << ip;

        // filepath
        n += receiver.Receive(cstring, sizeof(cstring));
        sls::strcpy_safe(shm()->rxFilePath, cstring);

        // filename
        n += receiver.Receive(cstring, sizeof(cstring));
        sls::strcpy_safe(shm()->rxFileName, cstring);

        // index
        n += receiver.Receive(&i64, sizeof(i64));
        shm()->rxFileIndex = i64;

        // file format
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxFileFormat = static_cast<fileFormat>(i32);

        // frames per file
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxFramesPerFile = i32;

        // frame discard policy
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxFrameDiscardMode = static_cast<frameDiscardPolicy>(i32);

        // frame padding
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxFramePadding = static_cast<bool>(i32);

        // file write enable
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxFileWrite = static_cast<bool>(i32);

        // master file write enable
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxMasterFileWrite = static_cast<bool>(i32);

        // file overwrite enable
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxFileOverWrite = static_cast<bool>(i32);

        // gap pixels
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->gappixels = i32;

        // receiver read frequency
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxReadFreq = i32;

        // receiver streaming port
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxZmqport = i32;

        // streaming source ip
        n += receiver.Receive(&ip, sizeof(ip));
        shm()->rxZmqip = ip;

        // additional json header
        n += receiver.Receive(cstring, sizeof(cstring));
        sls::strcpy_safe(shm()->rxAdditionalJsonHeader, cstring);

        // receiver streaming enable
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxUpstream = static_cast<bool>(i32);

        // activate
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->activated = static_cast<bool>(i32);

        // deactivated padding enable
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxPadDeactivatedModules = static_cast<bool>(i32);

        // silent mode
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxSilentMode = static_cast<bool>(i32);

        // dbit list
        {
            sls::FixedCapacityContainer<int, MAX_RX_DBIT> temp;
            n += receiver.Receive(&temp, sizeof(temp));
            shm()->rxDbitList = temp;
        }

        // dbit offset
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxDbitOffset = i32;

        if (n == 0) {
            throw RuntimeError(
                "Could not update receiver: " + std::string(shm()->rxHostname) +
                ", received 0 bytes\n");
        }
    }
}

void slsDetector::sendMultiDetectorSize() {
    int args[]{shm()->multiSize.x, shm()->multiSize.y};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending multi detector size to receiver: ("
                        << shm()->multiSize.x << "," << shm()->multiSize.y
                        << ")";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SEND_RECEIVER_MULTIDETSIZE, args, retval);
        FILE_LOG(logDEBUG1) << "Receiver multi size returned: " << retval;
    }
}

void slsDetector::setDetectorId() {
    FILE_LOG(logDEBUG1) << "Sending detector pos id to receiver: " << detId;
    if (shm()->useReceiverFlag) {
        int retval = -1;
        sendToReceiver(F_SEND_RECEIVER_DETPOSID, detId, retval);
        FILE_LOG(logDEBUG1) << "Receiver Position Id returned: " << retval;
    }
}

void slsDetector::setDetectorHostname() {
    char args[MAX_STR_LENGTH]{};
    char retvals[MAX_STR_LENGTH]{};
    sls::strcpy_safe(args, shm()->hostname);
    FILE_LOG(logDEBUG1) << "Sending detector hostname to receiver: " << args;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SEND_RECEIVER_DETHOSTNAME, args, retvals);
        FILE_LOG(logDEBUG1) << "Receiver set detector hostname: " << retvals;
    }
}

std::string slsDetector::getFilePath() { return shm()->rxFilePath; }

std::string slsDetector::setFilePath(const std::string &path) {
    if (!path.empty()) {
        char args[MAX_STR_LENGTH]{};
        char retvals[MAX_STR_LENGTH]{};
        sls::strcpy_safe(args, path.c_str());
        FILE_LOG(logDEBUG1) << "Sending file path to receiver: " << args;
        if (shm()->useReceiverFlag) {
            sendToReceiver(F_SET_RECEIVER_FILE_PATH, args, retvals);
            FILE_LOG(logDEBUG1) << "Receiver file path: " << retvals;
            sls::strcpy_safe(shm()->rxFilePath, retvals);
        } else {
            sls::strcpy_safe(shm()->rxFilePath, args);
        }
    }
    return shm()->rxFilePath;
}

std::string slsDetector::getFileName() { return shm()->rxFileName; }

std::string slsDetector::setFileName(const std::string &fname) {
    if (!fname.empty()) {
        char args[MAX_STR_LENGTH]{};
        char retvals[MAX_STR_LENGTH]{};
        sls::strcpy_safe(args, fname.c_str());
        FILE_LOG(logDEBUG1) << "Sending file name to receiver: " << args;
        if (shm()->useReceiverFlag) {
            sendToReceiver(F_SET_RECEIVER_FILE_NAME, args, retvals);
            FILE_LOG(logDEBUG1) << "Receiver file name: " << retvals;
            sls::strcpy_safe(shm()->rxFileName, retvals);
        } else {
            sls::strcpy_safe(shm()->rxFileName, args);
        }
    }
    return shm()->rxFileName;
}

int slsDetector::setFramesPerFile(int n_frames) {
    if (n_frames >= 0) {
        FILE_LOG(logDEBUG1)
            << "Setting receiver frames per file to " << n_frames;
        if (shm()->useReceiverFlag) {
            int retval = -1;
            sendToReceiver(F_SET_RECEIVER_FRAMES_PER_FILE, n_frames, retval);
            FILE_LOG(logDEBUG1) << "Receiver frames per file: " << retval;
            shm()->rxFramesPerFile = retval;
        } else {
            shm()->rxFramesPerFile = n_frames;
        }
    }
    return getFramesPerFile();
}

int slsDetector::getFramesPerFile() const { return shm()->rxFramesPerFile; }

slsDetectorDefs::frameDiscardPolicy
slsDetector::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
    int arg = static_cast<int>(f);
    FILE_LOG(logDEBUG1) << "Setting receiver frames discard policy to " << arg;
    if (shm()->useReceiverFlag) {
        auto retval = static_cast<frameDiscardPolicy>(-1);
        sendToReceiver(F_RECEIVER_DISCARD_POLICY, arg, retval);
        FILE_LOG(logDEBUG1) << "Receiver frames discard policy: " << retval;
        shm()->rxFrameDiscardMode = retval;
    } else {
        shm()->rxFrameDiscardMode = f;
    }
    return shm()->rxFrameDiscardMode;
}

bool slsDetector::setPartialFramesPadding(bool padding) {
    int arg = static_cast<int>(padding);
    int retval = static_cast<int>(padding);
    FILE_LOG(logDEBUG1) << "Setting receiver partial frames enable to " << arg;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_PADDING_ENABLE, arg, retval);
        FILE_LOG(logDEBUG1) << "Receiver partial frames enable: " << retval;
        shm()->rxFramePadding = static_cast<bool>(retval);
    } else {
        shm()->rxFramePadding = padding;
    }
    return getPartialFramesPadding();
}

bool slsDetector::getPartialFramesPadding() const {
    return shm()->rxFramePadding;
}

slsDetectorDefs::fileFormat slsDetector::setFileFormat(fileFormat f) {
    if (f != GET_FILE_FORMAT) {
        auto arg = static_cast<int>(f);
        auto retval = static_cast<fileFormat>(-1);
        FILE_LOG(logDEBUG1) << "Setting receiver file format to " << arg;
        if (shm()->useReceiverFlag) {
            sendToReceiver(F_SET_RECEIVER_FILE_FORMAT, arg, retval);
            FILE_LOG(logDEBUG1) << "Receiver file format: " << retval;
            shm()->rxFileFormat = retval;
        } else {
            shm()->rxFileFormat = f;
        }
    }
    return getFileFormat();
}

slsDetectorDefs::fileFormat slsDetector::getFileFormat() const {
    return shm()->rxFileFormat;
}

int64_t slsDetector::setFileIndex(int64_t file_index) {
    if (file_index >= 0) {
        int64_t retval = -1;
        FILE_LOG(logDEBUG1) << "Setting file index to " << file_index;
        if (shm()->useReceiverFlag) {
            sendToReceiver(F_SET_RECEIVER_FILE_INDEX, file_index, retval);
            FILE_LOG(logDEBUG1) << "Receiver file index: " << retval;
            shm()->rxFileIndex = retval;
        } else {
            shm()->rxFileIndex = file_index;
        }
    }
    return getFileIndex();
}

int64_t slsDetector::getFileIndex() const { return shm()->rxFileIndex; }

int64_t slsDetector::incrementFileIndex() {
    if (shm()->rxFileWrite) {
        return setFileIndex(shm()->rxFileIndex + 1);
    }
    return shm()->rxFileIndex;
}

void slsDetector::startReceiver() {
    FILE_LOG(logDEBUG1) << "Starting Receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_START_RECEIVER);
    }
}

void slsDetector::stopReceiver() {
    FILE_LOG(logDEBUG1) << "Stopping Receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_STOP_RECEIVER);
    }
}

slsDetectorDefs::runStatus slsDetector::getReceiverStatus() const {
    runStatus retval = ERROR;
    FILE_LOG(logDEBUG1) << "Getting Receiver Status";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_GET_RECEIVER_STATUS, nullptr, retval);
        FILE_LOG(logDEBUG1) << "Receiver Status: " << ToString(retval);
    }
    return retval;
}

int64_t slsDetector::getFramesCaughtByReceiver() const {
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Frames Caught by Receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_GET_RECEIVER_FRAMES_CAUGHT, nullptr, retval);
        FILE_LOG(logDEBUG1) << "Frames Caught by Receiver: " << retval;
    }
    return retval;
}

uint64_t slsDetector::getReceiverCurrentFrameIndex() const {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Current Frame Index of Receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_GET_RECEIVER_FRAME_INDEX, nullptr, retval);
        FILE_LOG(logDEBUG1) << "Current Frame Index of Receiver: " << retval;
    }
    return retval;
}

bool slsDetector::setFileWrite(bool value) {
    int arg = static_cast<int>(value);
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending enable file write to receiver: " << arg;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_ENABLE_RECEIVER_FILE_WRITE, arg, retval);
        FILE_LOG(logDEBUG1) << "Receiver file write enable: " << retval;
        shm()->rxFileWrite = static_cast<bool>(retval);
    } else {
        shm()->rxFileWrite = value;
    }
    return getFileWrite();
}

bool slsDetector::getFileWrite() const { return shm()->rxFileWrite; }

bool slsDetector::setMasterFileWrite(bool value) {
    int arg = static_cast<int>(value);
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending enable master file write to receiver: "
                        << arg;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_ENABLE_RECEIVER_MASTER_FILE_WRITE, arg, retval);
        FILE_LOG(logDEBUG1) << "Receiver master file write enable: " << retval;
        shm()->rxMasterFileWrite = static_cast<bool>(retval);
    } else {
        shm()->rxMasterFileWrite = value;
    }
    return getMasterFileWrite();
}

bool slsDetector::getMasterFileWrite() const {
    return shm()->rxMasterFileWrite;
}

bool slsDetector::setFileOverWrite(bool value) {
    int arg = static_cast<int>(value);
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending enable file overwrite to receiver: " << arg;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_ENABLE_RECEIVER_OVERWRITE, arg, retval);
        FILE_LOG(logDEBUG1) << "Receiver file overwrite enable: " << retval;
        shm()->rxFileOverWrite = static_cast<bool>(retval);
    } else {
        shm()->rxFileOverWrite = value;
    }
    return getFileOverWrite();
}

bool slsDetector::getFileOverWrite() const { return shm()->rxFileOverWrite; }

int slsDetector::setReceiverStreamingFrequency(int freq) {
    if (freq >= 0) {
        FILE_LOG(logDEBUG1) << "Sending read frequency to receiver: " << freq;
        if (shm()->useReceiverFlag) {
            int retval = -1;
            sendToReceiver(F_RECEIVER_STREAMING_FREQUENCY, freq, retval);
            FILE_LOG(logDEBUG1) << "Receiver read frequency: " << retval;
            shm()->rxReadFreq = retval;
        } else {
            shm()->rxReadFreq = freq;
        }
    }
    return shm()->rxReadFreq;
}

int slsDetector::setReceiverStreamingTimer(int time_in_ms) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending read timer to receiver: " << time_in_ms;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_STREAMING_TIMER, time_in_ms, retval);
        FILE_LOG(logDEBUG1) << "Receiver read timer: " << retval;
    }
    return retval;
}

bool slsDetector::enableDataStreamingFromReceiver(int enable) {
    if (enable >= 0) {
        FILE_LOG(logDEBUG1) << "Sending Data Streaming to receiver: " << enable;
        if (shm()->useReceiverFlag) {
            int retval = -1;
            sendToReceiver(F_STREAM_DATA_FROM_RECEIVER, enable, retval);
            FILE_LOG(logDEBUG1) << "Receiver Data Streaming: " << retval;
            shm()->rxUpstream = static_cast<bool>(retval);
        } else {
            shm()->rxUpstream = enable;
        }
    }
    return shm()->rxUpstream;
}

int slsDetector::enableTenGigabitEthernet(int value) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Enabling / Disabling 10Gbe: " << value;
    sendToDetector(F_ENABLE_TEN_GIGA, value, retval);
    FILE_LOG(logDEBUG1) << "10Gbe: " << retval;
    shm()->tenGigaEnable = retval;
    if (shm()->useReceiverFlag) {
        retval = -1;
        value = shm()->tenGigaEnable;
        FILE_LOG(logDEBUG1) << "Sending 10Gbe enable to receiver: " << value;
        sendToReceiver(F_ENABLE_RECEIVER_TEN_GIGA, value, retval);
        FILE_LOG(logDEBUG1) << "Receiver 10Gbe enable: " << retval;
    }
    return shm()->tenGigaEnable;
}

int slsDetector::setReceiverFifoDepth(int n_frames) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending Receiver Fifo Depth: " << n_frames;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_FIFO_DEPTH, n_frames, retval);
        FILE_LOG(logDEBUG1) << "Receiver Fifo Depth: " << retval;
    }
    return retval;
}

bool slsDetector::setReceiverSilentMode(int value) {
    FILE_LOG(logDEBUG1) << "Sending Receiver Silent Mode: " << value;
    if (shm()->useReceiverFlag) {
        int retval = -1;
        sendToReceiver(F_SET_RECEIVER_SILENT_MODE, value, retval);
        FILE_LOG(logDEBUG1) << "Receiver Data Streaming: " << retval;
        shm()->rxSilentMode = static_cast<bool>(retval);
    } else {
        shm()->rxSilentMode = value;
    }
    return shm()->rxSilentMode;
}

void slsDetector::restreamStopFromReceiver() {
    FILE_LOG(logDEBUG1) << "Restream stop dummy from Receiver via zmq";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RESTREAM_STOP_FROM_RECEIVER);
    }
}

void slsDetector::setPattern(const std::string &fname) {
    uint64_t word;
    uint64_t addr = 0;
    FILE *fd = fopen(fname.c_str(), "r");
    if (fd != nullptr) {
        while (fread(&word, sizeof(word), 1, fd) != 0u) {
            setPatternWord(addr, word); // TODO! (Erik) do we need to send
                                        // pattern in 64bit chunks?
            ++addr;
        }
        fclose(fd);
    } else {
        throw RuntimeError("Could not open file to set pattern");
    }
}

uint64_t slsDetector::setPatternIOControl(uint64_t word) {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Setting Pattern IO Control, word: 0x" << std::hex
                        << word << std::dec;
    sendToDetector(F_SET_PATTERN_IO_CONTROL, word, retval);
    FILE_LOG(logDEBUG1) << "Set Pattern IO Control: " << retval;
    return retval;
}

uint64_t slsDetector::setPatternClockControl(uint64_t word) {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Setting Pattern Clock Control, word: 0x" << std::hex
                        << word << std::dec;
    sendToDetector(F_SET_PATTERN_CLOCK_CONTROL, word, retval);
    FILE_LOG(logDEBUG1) << "Set Pattern Clock Control: " << retval;
    return retval;
}

uint64_t slsDetector::setPatternWord(int addr, uint64_t word) {
    uint64_t args[]{static_cast<uint64_t>(addr), word};
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Setting Pattern word, addr: 0x" << std::hex << addr
                        << ", word: 0x" << word << std::dec;
    sendToDetector(F_SET_PATTERN_WORD, args, retval);
    FILE_LOG(logDEBUG1) << "Set Pattern word: " << retval;
    return retval;
}

std::array<int, 2> slsDetector::setPatternLoopAddresses(int level, int start, int stop) {
    int args[]{level, start, stop};
    std::array<int, 2> retvals{};
    FILE_LOG(logDEBUG1) << "Setting Pat Loop Addresses, level: " << level
                        << ", start: " << start << ", stop: " << stop;
    sendToDetector(F_SET_PATTERN_LOOP_ADDRESSES, args, retvals);
    FILE_LOG(logDEBUG1) << "Set Pat Loop Addresses: " << retvals[0] << ", " << retvals[1];
    return retvals;
}

int slsDetector::setPatternLoopCycles(int level, int n) {
    int args[]{level, n};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting Pat Loop cycles, level: " << level
                        << ",nloops: " << n;
    sendToDetector(F_SET_PATTERN_LOOP_CYCLES, args, retval);
    FILE_LOG(logDEBUG1) << "Set Pat Loop Cycles: " << retval;
    return retval;
}

int slsDetector::setPatternWaitAddr(int level, int addr) {
    int retval = -1;
    int args[]{level, addr};
    FILE_LOG(logDEBUG1) << "Setting Pat Wait Addr, level: " << level
                        << ", addr: 0x" << std::hex << addr << std::dec;
    sendToDetector(F_SET_PATTERN_WAIT_ADDR, args, retval);
    FILE_LOG(logDEBUG1) << "Set Pat Wait Addr: " << retval;
    return retval;
}

uint64_t slsDetector::setPatternWaitTime(int level, uint64_t t) {
    uint64_t retval = -1;
    uint64_t args[]{static_cast<uint64_t>(level), t};
    FILE_LOG(logDEBUG1) << "Setting Pat Wait Time, level: " << level
                        << ", t: " << t;
    sendToDetector(F_SET_PATTERN_WAIT_TIME, args, retval);
    FILE_LOG(logDEBUG1) << "Set Pat Wait Time: " << retval;
    return retval;
}

void slsDetector::setPatternMask(uint64_t mask) {
    FILE_LOG(logDEBUG1) << "Setting Pattern Mask " << std::hex << mask
                        << std::dec;
    sendToDetector(F_SET_PATTERN_MASK, mask, nullptr);
    FILE_LOG(logDEBUG1) << "Pattern Mask successful";
}

uint64_t slsDetector::getPatternMask() {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Pattern Mask ";

    sendToDetector(F_GET_PATTERN_MASK, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Pattern Mask:" << retval;
    return retval;
}

void slsDetector::setPatternBitMask(uint64_t mask) {
    FILE_LOG(logDEBUG1) << "Setting Pattern Bit Mask " << std::hex << mask
                        << std::dec;
    sendToDetector(F_SET_PATTERN_BIT_MASK, mask, nullptr);
    FILE_LOG(logDEBUG1) << "Pattern Bit Mask successful";
}

uint64_t slsDetector::getPatternBitMask() {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Pattern Bit Mask ";
    sendToDetector(F_GET_PATTERN_BIT_MASK, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Pattern Bit Mask:" << retval;
    return retval;
}

int slsDetector::setLEDEnable(int enable) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending LED Enable: " << enable;
    sendToDetector(F_LED, enable, retval);
    FILE_LOG(logDEBUG1) << "LED Enable: " << retval;
    return retval;
}

void slsDetector::setDigitalIODelay(uint64_t pinMask, int delay) {
    uint64_t args[]{pinMask, static_cast<uint64_t>(delay)};
    FILE_LOG(logDEBUG1) << "Sending Digital IO Delay, pin mask: " << std::hex
                        << args[0] << ", delay: " << std::dec << args[1]
                        << " ps";
    sendToDetector(F_DIGITAL_IO_DELAY, args, nullptr);
    FILE_LOG(logDEBUG1) << "Digital IO Delay successful";
}

int slsDetector::getClockFrequency(int clkIndex) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Clock " << clkIndex << " frequency";
    sendToDetector(F_GET_CLOCK_FREQUENCY, clkIndex, retval);
    FILE_LOG(logDEBUG1) << "Clock " << clkIndex << " frequency: " << retval;
    return retval;
}

void slsDetector::setClockFrequency(int clkIndex, int value) {
    int args[]{clkIndex, value};
    FILE_LOG(logDEBUG1) << "Setting Clock " << clkIndex << " frequency to " << value;
    sendToDetector(F_SET_CLOCK_FREQUENCY, args, nullptr);
}

int slsDetector::getClockPhase(int clkIndex, bool inDegrees) {
    int args[]{clkIndex, static_cast<int>(inDegrees)};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Clock " << clkIndex << " phase " << (inDegrees ? "in degrees" : "");
    sendToDetector(F_GET_CLOCK_PHASE, args, retval);
    FILE_LOG(logDEBUG1) << "Clock " << clkIndex << " frequency: " << retval << (inDegrees ? "degrees" : "");
    return retval;
}

void slsDetector::setClockPhase(int clkIndex, int value, bool inDegrees) {
    int args[]{clkIndex, value, static_cast<int>(inDegrees)};
    FILE_LOG(logDEBUG1) << "Setting Clock " << clkIndex << " phase to " << value << (inDegrees ? "degrees" : "");
    sendToDetector(F_SET_CLOCK_PHASE, args, nullptr);
}

int slsDetector::getMaxClockPhaseShift(int clkIndex) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Max Phase Shift for Clock " << clkIndex;
    sendToDetector(F_GET_MAX_CLOCK_PHASE_SHIFT, clkIndex, retval);
    FILE_LOG(logDEBUG1) << "Max Phase Shift for Clock " << clkIndex << ": " << retval;
    return retval;
}

int slsDetector::getClockDivider(int clkIndex) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Clock " << clkIndex << " divider";
    sendToDetector(F_GET_CLOCK_DIVIDER, clkIndex, retval);
    FILE_LOG(logDEBUG1) << "Clock " << clkIndex << " divider: " << retval;
    return retval;
}

void slsDetector::setClockDivider(int clkIndex, int value) {
    int args[]{clkIndex, value};
    FILE_LOG(logDEBUG1) << "Setting Clock " << clkIndex << " divider to " << value;
    sendToDetector(F_SET_CLOCK_DIVIDER, args, nullptr);
}

int slsDetector::getPipeline(int clkIndex) {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting Clock " << clkIndex << " pipeline";
    sendToDetector(F_GET_PIPELINE, clkIndex, retval);
    FILE_LOG(logDEBUG1) << "Clock " << clkIndex << " pipeline: " << retval;
    return retval;
}

void slsDetector::setPipeline(int clkIndex, int value) {
    int args[]{clkIndex, value};
    FILE_LOG(logDEBUG1) << "Setting Clock " << clkIndex << " pipeline to " << value;
    sendToDetector(F_SET_PIPELINE, args, nullptr);
}


sls_detector_module slsDetector::interpolateTrim(sls_detector_module *a,
                                                 sls_detector_module *b,
                                                 const int energy, const int e1,
                                                 const int e2, int tb) {

    // only implemented for eiger currently (in terms of which dacs)
    if (shm()->myDetectorType != EIGER) {
        throw NotImplementedError(
            "Interpolation of Trim values not implemented for this detector!");
    }

    sls_detector_module myMod{shm()->myDetectorType};
    enum eiger_DacIndex {
        E_SVP,
        E_VTR,
        E_VRF,
        E_VRS,
        E_SVN,
        E_VTGSTV,
        E_VCMP_LL,
        E_VCMP_LR,
        E_CAL,
        E_VCMP_RL,
        E_RXB_RB,
        E_RXB_LB,
        E_VCMP_RR,
        E_VCP,
        E_VCN,
        E_VIS
    };

    // Copy other dacs
    int dacs_to_copy[] = {E_SVP, E_VTR, E_SVN, E_VTGSTV, E_RXB_RB, E_RXB_LB, E_VCN, E_VIS};
    int num_dacs_to_copy = sizeof(dacs_to_copy) / sizeof(dacs_to_copy[0]);
    for (int i = 0; i < num_dacs_to_copy; ++i) {
        if (a->dacs[dacs_to_copy[i]] != b->dacs[dacs_to_copy[i]]) {
            throw RuntimeError("Interpolate module: dacs different");
        }
        myMod.dacs[dacs_to_copy[i]] = a->dacs[dacs_to_copy[i]];
    }

    // Copy irrelevant dacs (without failing): CAL
    if (a->dacs[E_CAL] != b->dacs[E_CAL]) {
        FILE_LOG(logWARNING)
            << "DAC CAL differs in both energies (" << a->dacs[E_CAL] << ","
            << b->dacs[E_CAL] << ")!\nTaking first: " << a->dacs[E_CAL];
    }
    myMod.dacs[E_CAL] = a->dacs[E_CAL];

    // Interpolate vrf, vcmp, vcp
    int dacs_to_interpolate[] = {E_VRF,     E_VCMP_LL, E_VCMP_LR, E_VCMP_RL,
                                 E_VCMP_RR, E_VCP,     E_VRS};
    int num_dacs_to_interpolate =
        sizeof(dacs_to_interpolate) / sizeof(dacs_to_interpolate[0]);
    for (int i = 0; i < num_dacs_to_interpolate; ++i) {
        myMod.dacs[dacs_to_interpolate[i]] =
            linearInterpolation(energy, e1, e2, a->dacs[dacs_to_interpolate[i]],
                                b->dacs[dacs_to_interpolate[i]]);
    }
    // Interpolate all trimbits
    if (tb != 0) {
        for (int i = 0; i < myMod.nchan; ++i) {
            myMod.chanregs[i] = linearInterpolation(
                energy, e1, e2, a->chanregs[i], b->chanregs[i]);
        }
    }
    return myMod;
}

sls_detector_module slsDetector::readSettingsFile(const std::string &fname,
                                                  int tb) {
    FILE_LOG(logDEBUG1) << "Read settings file " << fname;
    sls_detector_module myMod(shm()->myDetectorType);
    auto names = getSettingsFileDacNames();
    // open file
    std::ifstream infile;
    if (shm()->myDetectorType == EIGER) {
        infile.open(fname.c_str(), std::ifstream::binary);
    } else {
        infile.open(fname.c_str(), std::ios_base::in);
    }
    if (!infile.is_open()) {
        throw RuntimeError("Could not open settings file: " + fname);
    }

    // eiger
    if (shm()->myDetectorType == EIGER) {
        bool allread = false;
        infile.read(reinterpret_cast<char *>(myMod.dacs),
                    sizeof(int) * (myMod.ndac));
        if (infile.good()) {
            infile.read(reinterpret_cast<char *>(&myMod.iodelay),
                        sizeof(myMod.iodelay));
            if (infile.good()) {
                infile.read(reinterpret_cast<char *>(&myMod.tau),
                            sizeof(myMod.tau));
                if (tb != 0) {
                    if (infile.good()) {
                        infile.read(reinterpret_cast<char *>(myMod.chanregs),
                                    sizeof(int) * (myMod.nchan));
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
            infile.close();
            throw RuntimeError("readSettingsFile: Could not load all values "
                               "for settings for " +
                               fname);
        }
        for (int i = 0; i < myMod.ndac; ++i) {
            FILE_LOG(logDEBUG1) << "dac " << i << ":" << myMod.dacs[i];
        }
        FILE_LOG(logDEBUG1) << "iodelay:" << myMod.iodelay;
        FILE_LOG(logDEBUG1) << "tau:" << myMod.tau;
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
                    myMod.dacs[i] = ival;
                    found = true;
                    FILE_LOG(logDEBUG1)
                        << names[i] << "(" << i << "): " << ival;
                    ++idac;
                }
            }
            if (!found) {
                throw RuntimeError("readSettingsFile: Unknown dac: " +
                                   sargname);
                infile.close();
            }
        }
        // not all read
        if (idac != names.size()) {
            infile.close();
            throw RuntimeError("Could read only " + std::to_string(idac) +
                               " dacs. Expected " +
                               std::to_string(names.size()) + " dacs");
        }
    }
    infile.close();
    FILE_LOG(logINFO) << "Settings file loaded: " << fname.c_str();
    return myMod;
}

void slsDetector::writeSettingsFile(const std::string &fname,
                                    sls_detector_module &mod) {
    FILE_LOG(logDEBUG1) << "Write settings file " << fname;
    auto names = getSettingsFileDacNames();
    std::ofstream outfile;
    if (shm()->myDetectorType == EIGER) {
        outfile.open(fname.c_str(), std::ofstream::binary);
    } else {
        outfile.open(fname.c_str(), std::ios_base::out);
    }
    if (!outfile.is_open()) {
        throw RuntimeError("Could not open settings file for writing: " +
                           fname);
    }
    if (shm()->myDetectorType == EIGER) {
        for (int i = 0; i < mod.ndac; ++i) {
            FILE_LOG(logINFO) << "dac " << i << ":" << mod.dacs[i];
        }
        FILE_LOG(logINFO) << "iodelay: " << mod.iodelay;
        FILE_LOG(logINFO) << "tau: " << mod.tau;

        outfile.write(reinterpret_cast<char *>(mod.dacs),
                      sizeof(int) * (mod.ndac));
        outfile.write(reinterpret_cast<char *>(&mod.iodelay),
                      sizeof(mod.iodelay));
        outfile.write(reinterpret_cast<char *>(&mod.tau), sizeof(mod.tau));
        outfile.write(reinterpret_cast<char *>(mod.chanregs),
                      sizeof(int) * (mod.nchan));
    }
    // gotthard, jungfrau
    else {
        for (int i = 0; i < mod.ndac; ++i) {
            FILE_LOG(logDEBUG1) << "dac " << i << ": " << mod.dacs[i];
            outfile << names[i] << " " << mod.dacs[i] << std::endl;
        }
    }
    outfile.close();
}

std::vector<std::string> slsDetector::getSettingsFileDacNames() {
    switch (shm()->myDetectorType) {
    case GOTTHARD:
        return {"Vref",  "VcascN", "VcascP",    "Vout",
                "Vcasc", "Vin",    "Vref_comp", "Vib_test"};
        break;
    case EIGER:
        break;
    case JUNGFRAU:
        return {"VDAC0",  "VDAC1",  "VDAC2",  "VDAC3", "VDAC4",  "VDAC5",
                "VDAC6",  "VDAC7",  "VDAC8",  "VDAC9", "VDAC10", "VDAC11",
                "VDAC12", "VDAC13", "VDAC14", "VDAC15"};
        break;
    default:
        throw RuntimeError(
            "Unknown detector type - unknown format for settings file");
    }
    return {};
}
