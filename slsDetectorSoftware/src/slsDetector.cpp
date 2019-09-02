#include "slsDetector.h"
#include "ClientSocket.h"
#include "SharedMemory.h"
#include "file_utils.h"
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

int64_t slsDetector::getId(idMode mode) {
    // These should not go to detector...
    assert(mode != THIS_SOFTWARE_VERSION);
    assert(mode != RECEIVER_VERSION);
    assert(mode != CLIENT_SOFTWARE_API_VERSION);
    assert(mode != CLIENT_RECEIVER_API_VERSION);

    int arg = static_cast<int>(mode);
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting id type " << mode;
    sendToDetector(F_GET_ID, arg, retval);
    FILE_LOG(logDEBUG1) << "Id (" << mode << "): 0x" << std::hex << retval
                        << std::dec;
    return retval;
}

int64_t slsDetector::getReceiverSoftwareVersion() const {
    FILE_LOG(logDEBUG1) << "Getting receiver software version";
    int64_t retval = -1;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_GET_RECEIVER_ID, nullptr, retval);
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
    shm()->roFlags = NORMAL_READOUT;
    shm()->currentSettings = UNINITIALIZED;
    shm()->currentThresholdEV = -1;
    shm()->timerValue[FRAME_NUMBER] = 1;
    shm()->timerValue[ACQUISITION_TIME] = 0;
    shm()->timerValue[FRAME_PERIOD] = 0;
    shm()->timerValue[DELAY_AFTER_TRIGGER] = 0;
    shm()->timerValue[CYCLES_NUMBER] = 1;
    shm()->timerValue[ACTUAL_TIME] = 0;
    shm()->timerValue[MEASUREMENT_TIME] = 0;
    shm()->timerValue[PROGRESS] = 0;
    shm()->timerValue[FRAMES_FROM_START] = 0;
    shm()->timerValue[FRAMES_FROM_START_PG] = 0;
    shm()->timerValue[ANALOG_SAMPLES] = 1;
    shm()->timerValue[DIGITAL_SAMPLES] = 1;
    shm()->timerValue[SUBFRAME_ACQUISITION_TIME] = 0;
    shm()->timerValue[STORAGE_CELL_NUMBER] = 0;
    shm()->timerValue[SUBFRAME_DEADTIME] = 0;
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
    // readoutflags (ctb only)
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
                    << " bytes. serialno: " << myMod->nchan;

    n = client.Send(&(myMod->nchip), sizeof(myMod->nchip));
    ts += n;
    FILE_LOG(level) << "nchip sent. " << n
                    << " bytes. serialno: " << myMod->nchip;

    n = client.Send(&(myMod->ndac), sizeof(myMod->ndac));
    ts += n;
    FILE_LOG(level) << "ndac sent. " << n
                    << " bytes. serialno: " << myMod->ndac;

    n = client.Send(&(myMod->reg), sizeof(myMod->reg));
    ts += n;
    FILE_LOG(level) << "reg sent. " << n << " bytes. serialno: " << myMod->reg;

    n = client.Send(&(myMod->iodelay), sizeof(myMod->iodelay));
    ts += n;
    FILE_LOG(level) << "iodelay sent. " << n
                    << " bytes. serialno: " << myMod->iodelay;

    n = client.Send(&(myMod->tau), sizeof(myMod->tau));
    ts += n;
    FILE_LOG(level) << "tau sent. " << n << " bytes. serialno: " << myMod->tau;

    n = client.Send(&(myMod->eV), sizeof(myMod->eV));
    ts += n;
    FILE_LOG(level) << "ev sent. " << n << " bytes. serialno: " << myMod->eV;

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

slsDetectorDefs::detectorType slsDetector::getDetectorTypeAsEnum() const {
    return shm()->myDetectorType;
}

std::string slsDetector::getDetectorTypeAsString() const {
    return slsDetectorDefs::detectorTypeToString(getDetectorTypeAsEnum());
}

void slsDetector::updateNumberOfChannels() {
    if (shm()->myDetectorType == CHIPTESTBOARD ||
        shm()->myDetectorType == MOENCH) {

        int nachans = 0, ndchans = 0;
        // analog channels (normal, analog/digital readout)
        if (shm()->roFlags == slsDetectorDefs::NORMAL_READOUT ||
            ((shm()->roFlags & slsDetectorDefs::ANALOG_AND_DIGITAL) != 0)) {
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
            (((shm()->roFlags & DIGITAL_ONLY) != 0) ||
             ((shm()->roFlags & ANALOG_AND_DIGITAL) != 0))) {
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

std::string slsDetector::getLastClientIP() {
    char retval[INET_ADDRSTRLEN]{};
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
        char lastClientIP[INET_ADDRSTRLEN] = {0};
        n += client.Receive(lastClientIP, sizeof(lastClientIP));
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
        shm()->timerValue[FRAME_NUMBER] = i64;

        // exptime
        n += client.Receive(&i64, sizeof(i64));
        shm()->timerValue[ACQUISITION_TIME] = i64;

        // subexptime, subdeadtime
        if (shm()->myDetectorType == EIGER) {
            n += client.Receive(&i64, sizeof(i64));
            shm()->timerValue[SUBFRAME_ACQUISITION_TIME] = i64;

            n += client.Receive(&i64, sizeof(i64));
            shm()->timerValue[SUBFRAME_DEADTIME] = i64;
        }

        // period
        n += client.Receive(&i64, sizeof(i64));
        shm()->timerValue[FRAME_PERIOD] = i64;

        // delay
        if (shm()->myDetectorType != EIGER && shm()->myDetectorType != MYTHEN3 && shm()->myDetectorType != GOTTHARD2) {
            n += client.Receive(&i64, sizeof(i64));
            shm()->timerValue[DELAY_AFTER_TRIGGER] = i64;
        }

        if (shm()->myDetectorType == JUNGFRAU) {
            // storage cell
            n += client.Receive(&i64, sizeof(i64));
            shm()->timerValue[STORAGE_CELL_NUMBER] = i64;

            // storage cell delay
            n += client.Receive(&i64, sizeof(i64));
            shm()->timerValue[STORAGE_CELL_DELAY] = i64;
        }

        // cycles
        n += client.Receive(&i64, sizeof(i64));
        shm()->timerValue[CYCLES_NUMBER] = i64;

        // readout flags
        if (shm()->myDetectorType == EIGER ||
            shm()->myDetectorType == CHIPTESTBOARD) {
            n += client.Receive(&i32, sizeof(i32));
            shm()->roFlags = static_cast<readOutFlags>(i32);
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
            // analog samples
            n += client.Receive(&i64, sizeof(i64));
            if (i64 >= 0) {
                shm()->timerValue[ANALOG_SAMPLES] = i64;
            }

            // digital samples
            n += client.Receive(&i64, sizeof(i64));
            if (i64 >= 0) {
                shm()->timerValue[DIGITAL_SAMPLES] = i64;
            }

            // adcmask
            uint32_t u32 = 0;
            n += client.Receive(&u32, sizeof(u32));
            shm()->adcEnableMask = u32;
            if (shm()->myDetectorType == MOENCH)
                setAdditionalJsonParameter("adcmask", std::to_string(u32));

            // update #nchan, as it depends on #samples, adcmask,
            // readoutflags
            updateNumberOfChannels();
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
            ss << "Unknown settings " << getDetectorSettings(isettings)
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
        ss << "Unknown settings " << getDetectorSettings(s)
           << " for this detector!";
        throw RuntimeError(ss.str());
    }
    std::ostringstream ostfn;
    ostfn << shm()->settingsDir << ssettings << "/" << e_eV << "eV"
          << "/noise.sn" << std::setfill('0') << std::setw(3) << std::dec
          << getId(DETECTOR_SERIAL_NUMBER) << std::setbase(10);
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
                  << getId(DETECTOR_SERIAL_NUMBER);
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
              << getId(DETECTOR_SERIAL_NUMBER);
    }
    fn = ostfn.str();
    sls_detector_module myMod = getModule();
    writeSettingsFile(fn, myMod);
}

slsDetectorDefs::runStatus slsDetector::getRunStatus() const {
    runStatus retval = ERROR;
    FILE_LOG(logDEBUG1) << "Getting status";
    sendToDetectorStop(F_GET_RUN_STATUS, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Detector status: " << runStatusType(retval);
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

void slsDetector::configureMAC() {
    int fnum = F_CONFIGURE_MAC;
    const size_t array_size = 50;
    const size_t n_args = 14;
    const size_t n_retvals = 2;
    char args[n_args][array_size]{};
    char retvals[n_retvals][array_size]{};
    FILE_LOG(logDEBUG1) << "Configuring MAC";
    if (shm()->rxUDPIP == 0) {
        // If hostname is valid ip use that, oterwise lookup hostname
        shm()->rxUDPIP = shm()->rxHostname;
        if (shm()->rxUDPIP == 0) {
            shm()->rxUDPIP = HostnameToIp(shm()->rxHostname);
        }
    }

    if (shm()->rxUDPMAC == 0) {
        throw RuntimeError(
            "configureMAC: Error. Receiver UDP MAC Addresses not set");
    }
    FILE_LOG(logDEBUG1) << "rx_hostname and rx_udpmac are valid ";

    // Jungfrau second interface
    if (shm()->numUDPInterfaces == 2) {
        if (shm()->rxUDPIP2 == 0) {
            shm()->rxUDPIP2 = shm()->rxUDPIP;
        }
        if (shm()->rxUDPMAC2 == 0) {
            throw RuntimeError(
                "configureMAC: Error. Receiver UDP MAC Addresses 2 not set");
        }
        FILE_LOG(logDEBUG1) << "rx_udpmac2 is valid ";
    }

    // copy to args and convert to hex
    snprintf(args[0], array_size, "%x", shm()->rxUDPPort);
    sls::strcpy_safe(args[1], getReceiverUDPIP().hex());
    sls::strcpy_safe(args[2], getReceiverUDPMAC().hex());
    sls::strcpy_safe(args[3], getDetectorIP().hex());
    sls::strcpy_safe(args[4], getDetectorMAC().hex());
    snprintf(args[5], array_size, "%x", shm()->rxUDPPort2);
    sls::strcpy_safe(args[6], getReceiverUDPIP2().hex());
    sls::strcpy_safe(args[7], getReceiverUDPMAC2().hex());
    sls::strcpy_safe(args[8], getDetectorIP2().hex());
    sls::strcpy_safe(args[9], getDetectorMAC2().hex());
    snprintf(args[10], array_size, "%x", shm()->numUDPInterfaces);
    snprintf(args[11], array_size, "%x", shm()->selectedUDPInterface);

    // 2d positions to detector to put into udp header
    {
        int pos[2] = {0, 0};
        int max = shm()->multiSize.y * (shm()->numUDPInterfaces);
        // row
        pos[0] = (detId % max);
        // col for horiz. udp ports
        pos[1] = (detId / max) * ((shm()->myDetectorType == EIGER) ? 2 : 1);
        // pos[2] (z is reserved)
        FILE_LOG(logDEBUG) << "Detector [" << detId << "] - (" << pos[0] << ","
                           << pos[1] << ")";
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
    FILE_LOG(logDEBUG1) << "number of udp interfaces:" << std::dec << args[10]
                        << "-";
    FILE_LOG(logDEBUG1) << "selected udp interface:" << std::dec << args[11]
                        << "-";
    FILE_LOG(logDEBUG1) << "row:" << args[12] << "-";
    FILE_LOG(logDEBUG1) << "col:" << args[13] << "-";

    // send to server
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    int ret = client.sendCommandThenRead(fnum, args, sizeof(args), retvals,
                                         sizeof(retvals));

    // TODO!(Erik) Send as int already from detector
    uint64_t detector_mac = 0;
    uint32_t detector_ip = 0;
    sscanf(retvals[0], "%lx", &detector_mac);
    sscanf(retvals[1], "%x", &detector_ip);
    detector_ip = __builtin_bswap32(detector_ip);

    if (shm()->detectorMAC != detector_mac) {
        shm()->detectorMAC = detector_mac;
        FILE_LOG(logINFO) << detId << ": Detector MAC updated to "
                          << getDetectorMAC();
    }

    if (shm()->detectorIP != detector_ip) {
        shm()->detectorIP = detector_ip;
        FILE_LOG(logINFO) << detId << ": Detector IP updated to "
                          << getDetectorIP();
    }
    if (ret == FORCE_UPDATE) {
        updateCachedDetectorVariables();
    }
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

int64_t slsDetector::setTimer(timerIndex index, int64_t t) {
    int64_t args[]{static_cast<int64_t>(index), t};
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Setting " << getTimerType(index) << " to " << t
                        << " ns/value";

    // send to detector
    int64_t oldtimer = shm()->timerValue[index];
    sendToDetector(F_SET_TIMER, args, retval);
    FILE_LOG(logDEBUG1) << getTimerType(index) << ": " << retval;
    shm()->timerValue[index] = retval;
    // update #nchan, as it depends on #samples, adcmask,
    // readoutflags
    if (index == ANALOG_SAMPLES || index == DIGITAL_SAMPLES) {
        updateNumberOfChannels();
    }

    // setting timers consequences (eiger (ratecorr) )
    // (a get can also change timer value, hence check difference)
    if (oldtimer != shm()->timerValue[index]) {
        // eiger: change exptime/subexptime, set rate correction to update table
        if (shm()->myDetectorType == EIGER) {
            int dr = shm()->dynamicRange;
            if ((dr == 32 && index == SUBFRAME_ACQUISITION_TIME) ||
                (dr == 16 && index == ACQUISITION_TIME)) {
                int r = getRateCorrection();
                if (r != 0) {
                    setRateCorrection(r);
                }
            }
        }
    }

    // send to reciever
    if (shm()->useReceiverFlag) {
        timerIndex rt[]{FRAME_NUMBER,
                        FRAME_PERIOD,
                        CYCLES_NUMBER,
                        ACQUISITION_TIME,
                        SUBFRAME_ACQUISITION_TIME,
                        SUBFRAME_DEADTIME,
                        ANALOG_SAMPLES,
                        DIGITAL_SAMPLES,
                        STORAGE_CELL_NUMBER};

        // if in list (lambda)
        if (std::any_of(std::begin(rt), std::end(rt),
                        [index](timerIndex t) { return t == index; })) {
            args[1] = shm()->timerValue[index];
            retval = -1;

            // rewrite args
            if ((index == FRAME_NUMBER) || (index == CYCLES_NUMBER) ||
                (index == STORAGE_CELL_NUMBER)) {
                args[1] = shm()->timerValue[FRAME_NUMBER] *
                          ((shm()->timerValue[CYCLES_NUMBER] > 0)
                               ? (shm()->timerValue[CYCLES_NUMBER])
                               : 1) *
                          ((shm()->timerValue[STORAGE_CELL_NUMBER] > 0)
                               ? (shm()->timerValue[STORAGE_CELL_NUMBER]) + 1
                               : 1);
            }
            FILE_LOG(logDEBUG1)
                << "Sending "
                << (((index == FRAME_NUMBER) || (index == CYCLES_NUMBER) ||
                     (index == STORAGE_CELL_NUMBER))
                        ? "(#Frames) * (#cycles) * (#storage cells)"
                        : getTimerType(index))
                << " to receiver: " << args[1];

            sendToReceiver(F_SET_RECEIVER_TIMER, args, retval);
        }
    }
    return shm()->timerValue[index];
}

int64_t slsDetector::getTimeLeft(timerIndex index) const {
    int64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting " << getTimerType(index) << " left";
    sendToDetectorStop(F_GET_TIME_LEFT, index, retval);
    FILE_LOG(logDEBUG1) << getTimerType(index) << " left: " << retval;
    return retval;
}

int slsDetector::setSpeed(speedVariable sp, int value, int mode) {
    int args[]{static_cast<int>(sp), value, mode};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting speed index " << sp << " to " << value
                        << " mode: " << mode;
    sendToDetector(F_SET_SPEED, args, retval);
    FILE_LOG(logDEBUG1) << "Speed index " << sp << ": " << retval;
    return retval;
}

int slsDetector::setDynamicRange(int n) {
    // TODO! Properly handle fail
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
    return shm()->dynamicRange;
}

int slsDetector::getDynamicRangeFromShm() { return shm()->dynamicRange; }

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

int slsDetector::setReadOutFlags(readOutFlags flag) {
    auto arg = static_cast<int>(flag);
    readOutFlags retval = GET_READOUT_FLAGS;
    FILE_LOG(logDEBUG1) << "Setting readout flags to " << flag;
    sendToDetector(F_SET_READOUT_FLAGS, arg, retval);
    FILE_LOG(logDEBUG1) << "Readout flag: " << retval;
    shm()->roFlags = retval;
    // update #nchan, as it depends on #samples, adcmask,
    // readoutflags
    if (shm()->myDetectorType == CHIPTESTBOARD) {
        updateNumberOfChannels();
    }
    FILE_LOG(logDEBUG1) << "Setting receiver readout flags to " << arg;
    if (shm()->useReceiverFlag) {
        int fnum = F_RECEIVER_SET_READOUT_FLAGS;
        arg = shm()->roFlags;
        retval = static_cast<readOutFlags>(-1);
        sendToReceiver(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver readout flag: " << retval;
    }
    return shm()->roFlags;
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
        << (slsDetectorDefs::detectorTypeToString(shm()->myDetectorType))
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
        << "\nframe index needed:"
        << ((shm()->timerValue[FRAME_NUMBER] *
             shm()->timerValue[CYCLES_NUMBER]) > 1)
        << "\nframe period:" << (shm()->timerValue[FRAME_PERIOD])
        << "\nframe number:" << (shm()->timerValue[FRAME_NUMBER])
        << "\nsub exp time:" << (shm()->timerValue[SUBFRAME_ACQUISITION_TIME])
        << "\nsub dead time:" << (shm()->timerValue[SUBFRAME_DEADTIME])
        << "\nasamples:" << (shm()->timerValue[ANALOG_SAMPLES])
        << "\ndsamples:" << (shm()->timerValue[DIGITAL_SAMPLES])
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
        
        updateRxDestinationUDPIP();
        updateRxDestinationUDPIP2();        
        setDestinationUDPPort(getDestinationUDPPort());
        setDestinationUDPPort2(getDestinationUDPPort2());
        setNumberofUDPInterfaces(getNumberofUDPInterfaces());
        selectUDPInterface(getSelectedUDPInterface());
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
        setTimer(FRAME_PERIOD, shm()->timerValue[FRAME_PERIOD]);
        setTimer(FRAME_NUMBER, shm()->timerValue[FRAME_NUMBER]);
        setTimer(ACQUISITION_TIME, shm()->timerValue[ACQUISITION_TIME]);

        // detector specific
        switch (shm()->myDetectorType) {

        case EIGER:
            setTimer(SUBFRAME_ACQUISITION_TIME,
                     shm()->timerValue[SUBFRAME_ACQUISITION_TIME]);
            setTimer(SUBFRAME_DEADTIME, shm()->timerValue[SUBFRAME_DEADTIME]);
            setDynamicRange(shm()->dynamicRange);
            setFlippedDataX(-1);
            activate(-1);
            setDeactivatedRxrPaddingMode(
                static_cast<int>(shm()->rxPadDeactivatedModules));
            enableGapPixels(shm()->gappixels);
            enableTenGigabitEthernet(shm()->tenGigaEnable);
            setReadOutFlags(GET_READOUT_FLAGS);
            setQuad(getQuad());
            break;

        case CHIPTESTBOARD:
            setTimer(ANALOG_SAMPLES, shm()->timerValue[ANALOG_SAMPLES]);
            setTimer(DIGITAL_SAMPLES, shm()->timerValue[DIGITAL_SAMPLES]);
            enableTenGigabitEthernet(shm()->tenGigaEnable);
            setReadOutFlags(GET_READOUT_FLAGS);
            setADCEnableMask(shm()->adcEnableMask);
            setReceiverDbitOffset(shm()->rxDbitOffset);
            setReceiverDbitList(shm()->rxDbitList);
            break;

        case MOENCH:
            setTimer(ANALOG_SAMPLES, shm()->timerValue[ANALOG_SAMPLES]);
            setTimer(DIGITAL_SAMPLES, shm()->timerValue[DIGITAL_SAMPLES]);
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
        setReceiverStreamingIP(getReceiverStreamingIP());
        setAdditionalJsonHeader(shm()->rxAdditionalJsonHeader);
        enableDataStreamingFromReceiver(
            static_cast<int>(enableDataStreamingFromReceiver(-1)));
    }

    return std::string(shm()->rxHostname);
}

std::string slsDetector::getReceiverHostname() const {
    return std::string(shm()->rxHostname);
}

void slsDetector::setSourceUDPMAC(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address");
    }
    FILE_LOG(logDEBUG1) << "Setting source udp mac to " << mac;
    uint64_t arg = static_cast<uint64_t>(mac);
    sendToDetector(F_SET_SOURCE_UDP_MAC, arg, nullptr); 
}

MacAddr slsDetector::getSourceUDPMAC() {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting source udp mac";
    sendToDetector(F_GET_SOURCE_UDP_MAC, nullptr, retval);
    auto mac = MacAddr(retval);
    FILE_LOG(logDEBUG1) << "Source udp mac: " << mac;
    return mac;
}

void slsDetector::setSourceUDPMAC2(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address2");
    }
    FILE_LOG(logDEBUG1) << "Setting source udp mac2 to " << mac;
    uint64_t arg = static_cast<uint64_t>(mac);
    sendToDetector(F_SET_SOURCE_UDP_MAC2, arg, nullptr); 
}

MacAddr slsDetector::getSourceUDPMAC2() {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting source udp mac2";
    sendToDetector(F_GET_SOURCE_UDP_MAC2, nullptr, retval);
    auto mac = MacAddr(retval);
    FILE_LOG(logDEBUG1) << "Source udp mac2: " << mac;
    return mac;
}

void slsDetector::setSourceUDPIP(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address");
    }
    FILE_LOG(logDEBUG1) << "Setting source udp ip to " << ip;
    uint32_t arg = static_cast<uint32_t>(ip);
    sendToDetector(F_SET_SOURCE_UDP_IP, arg, nullptr); 
}

IpAddr slsDetector::getSourceUDPIP() {
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting source udp ip";
    sendToDetector(F_GET_SOURCE_UDP_IP, nullptr, retval);
    auto ip = IpAddr(retval);
    FILE_LOG(logDEBUG1) << "Source udp ip: " << ip;
    return ip;
}

void slsDetector::setSourceUDPIP2(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address2");
    }
    FILE_LOG(logDEBUG1) << "Setting source udp ip2 to " << ip;
    uint32_t arg = static_cast<uint32_t>(ip);
    sendToDetector(F_SET_SOURCE_UDP_IP2, arg, nullptr); 
}

IpAddr slsDetector::getSourceUDPIP2() {
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting source udp ip2";
    sendToDetector(F_GET_SOURCE_UDP_IP2, nullptr, retval);
    auto ip = IpAddr(retval);
    FILE_LOG(logDEBUG1) << "Source udp ip2: " << ip;
    return ip;
}

void slsDetector::setDestinationUDPMAC(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid destination udp mac address");
    }
    FILE_LOG(logDEBUG1) << "Setting destination udp mac to " << mac;
    uint64_t arg = static_cast<uint64_t>(mac);
    sendToDetector(F_SET_DEST_UDP_MAC, arg, nullptr); 
}

MacAddr slsDetector::getDestinationUDPMAC() {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting destination udp mac";
    sendToDetector(F_GET_DEST_UDP_MAC, nullptr, retval);
    auto mac = MacAddr(retval);
    FILE_LOG(logDEBUG1) << "Destination udp mac: " << mac;
    return mac;
}

void slsDetector::setDestinationUDPMAC2(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid desinaion udp mac address2");
    }
    FILE_LOG(logDEBUG1) << "Setting destination udp mac2 to " << mac;
    uint64_t arg = static_cast<uint64_t>(mac);
    sendToDetector(F_SET_DEST_UDP_MAC2, arg, nullptr); 
}

MacAddr slsDetector::getDestinationUDPMAC2() {
    uint64_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting destination udp mac2";
    sendToDetector(F_GET_DEST_UDP_MAC2, nullptr, retval);
    auto mac = MacAddr(retval);
    FILE_LOG(logDEBUG1) << "Destination udp mac2: " << mac;
    return mac;
}

void slsDetector::setDestinationUDPIP(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address");
    }
    FILE_LOG(logDEBUG1) << "Setting destination udp ip to " << ip;
    uint32_t arg = static_cast<uint32_t>(ip);
    sendToDetector(F_SET_DEST_UDP_IP, arg, nullptr); 
    if (shm()->useReceiverFlag) {
        uint64_t retval = -1;
        sendToReceiver(F_SET_RECEIVER_UDP_IP, arg, retval);
        sendToDetector(F_SET_DEST_UDP_MAC, retval, nullptr); 
    }   
}

IpAddr slsDetector::getDestinationUDPIP() {
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting destination udp ip";
    sendToDetector(F_GET_DEST_UDP_IP, nullptr, retval);
    auto ip = IpAddr(retval);
    FILE_LOG(logDEBUG1) << "Destination udp ip: " << ip;
    return ip;
}

void slsDetector::updateRxDestinationUDPIP() {
    auto ip = getDestinationUDPIP();
    if (ip == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        sls::IpAddr ip = shm()->rxHostname;
        if (ip == 0) {
            ip = HostnameToIp(shm()->rxHostname);
        }       
        FILE_LOG(logDEBUG1) << "Setting destination default udp ip to " << ip;
        setDestinationUDPIP(ip);    
    }
}

void slsDetector::setDestinationUDPIP2(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address2");
    }
    FILE_LOG(logDEBUG1) << "Setting destination udp ip2 to " << ip;
    uint32_t arg = static_cast<uint32_t>(ip);
    sendToDetector(F_SET_DEST_UDP_IP2, arg, nullptr); 
    if (shm()->useReceiverFlag) {
        uint64_t retval = -1;
        sendToReceiver(F_SET_RECEIVER_UDP_IP2, arg, retval);
        sendToDetector(F_SET_DEST_UDP_MAC2, retval, nullptr); 
    }   
}

IpAddr slsDetector::getDestinationUDPIP2() {
    uint32_t retval = -1;
    FILE_LOG(logDEBUG1) << "Getting destination udp ip2";
    sendToDetector(F_GET_DEST_UDP_IP2, nullptr, retval);
    auto ip = IpAddr(retval);
    FILE_LOG(logDEBUG1) << "Destination udp ip2: " << ip;
    return ip;
}

void slsDetector::updateRxDestinationUDPIP2() {
    auto ip = getDestinationUDPIP2();
    if (ip == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        sls::IpAddr ip = shm()->rxHostname;
        if (ip == 0) {
            ip = HostnameToIp(shm()->rxHostname);
        }       
        FILE_LOG(logDEBUG1) << "Setting destination default udp ip2 to " << ip;
        setDestinationUDPIP2(ip);    
    }
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
    FILE_LOG(logDEBUG1) << "Destination udp ip: " << retval;

    return retval;
}

void slsDetector::setDestinationUDPPort2(const int port) {
    FILE_LOG(logDEBUG1) << "Setting destination udp port2 to " << port2;
    sendToDetector(F_SET_DEST_UDP_PORT2, port, nullptr); 
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_UDP_PORT2, port, nullptr); 
    }   
}

int slsDetector::getDestinationUDPPort2() {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting destination udp port2";
    sendToDetector(F_GET_DEST_UDP_PORT2, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Destination udp ip2: " << retval;
    return retval;
}

void slsDetector::setNumberofUDPInterfaces(int n) {
    FILE_LOG(logDEBUG1) << "Setting number of udp interfaces to " << n;
    sendToDetector(F_SET_NUM_INTERFACES, n, nullptr); 
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_INTERFACES, n, nullptr); 
    }  
}

int slsDetector::getNumberofUDPInterfaces() const {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting number of udp interfaces";
    sendToDetector(F_GET_NUM_INTERFACES, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Number of udp interfaces: " << retval;
    return retval;
}

void slsDetector::selectUDPInterface(int n) {
    FILE_LOG(logDEBUG1) << "Setting selected udp interface to " << n;
    sendToDetector(F_SET_INTERFACE_SEL, n, nullptr); 
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_INTERFACES, n, nullptr); 
    }
}

int slsDetector::getSelectedUDPInterface() const {
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Getting selected udp interface";
    sendToDetector(F_GET_INTERFACE_SEL, nullptr, retval);
    FILE_LOG(logDEBUG1) << "Selected udp interface: " << retval;
    return retval;
}

void slsDetector::setClientStreamingPort(int port) { shm()->zmqport = port; }

int slsDetector::getClientStreamingPort() { return shm()->zmqport; }

void slsDetector::setReceiverStreamingPort(int port) {
    // copy now else it is lost if rx_hostname not set yet
    shm()->rxZmqport = port;
    int fnum = F_SET_RECEIVER_STREAMING_PORT;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending receiver streaming port to receiver: "
                        << port;
    if (shm()->useReceiverFlag) {
        sendToReceiver(fnum, &port, sizeof(port), &retval, sizeof(retval));
        FILE_LOG(logDEBUG1) << "Receiver streaming port: " << retval;
        shm()->rxZmqport = retval;
    }
}

int slsDetector::getReceiverStreamingPort() { return shm()->rxZmqport; }

void slsDetector::setClientStreamingIP(const std::string &sourceIP) {
    auto ip = HostnameToIp(sourceIP.c_str());
    if (ip != 0) {
        shm()->zmqip = ip;
    } else {
        throw sls::RuntimeError("Could not set zmqip");
    }
}

std::string slsDetector::getClientStreamingIP() { return shm()->zmqip.str(); }

void slsDetector::setReceiverStreamingIP(std::string sourceIP) {
    // if empty, give rx_hostname
    if (sourceIP.empty() || sourceIP == "0.0.0.0") {
        if (strcmp(shm()->rxHostname, "none") == 0) {
            throw RuntimeError("Receiver hostname not set yet. Cannot create "
                               "rx_zmqip from none");
        }
        sourceIP = shm()->rxHostname;
    }

    FILE_LOG(logDEBUG1) << "Sending receiver streaming IP to receiver: "
                        << sourceIP;
    shm()->rxZmqip = HostnameToIp(sourceIP.c_str());

    // if zmqip is empty, update it
    if (shm()->zmqip == 0) {
        shm()->zmqip = shm()->rxZmqip;
    }

    // send to receiver
    if (shm()->useReceiverFlag) {
        char retvals[MAX_STR_LENGTH]{};
        char args[MAX_STR_LENGTH]{};
        sls::strcpy_safe(args, shm()->rxZmqip.str()); // TODO send int
        FILE_LOG(logDEBUG1)
            << "Sending receiver streaming IP to receiver: " << args;
        sendToReceiver(F_RECEIVER_STREAMING_SRC_IP, args, retvals);
        FILE_LOG(logDEBUG1) << "Receiver streaming ip: " << retvals;
        shm()->rxZmqip = retvals;
    }
}

std::string slsDetector::getReceiverStreamingIP() {
    return shm()->rxZmqip.str();
}

int slsDetector::setDetectorNetworkParameter(networkParameter index,
                                             int delay) {
    int args[]{static_cast<int>(index), delay};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Setting network parameter index " << index << " to "
                        << delay;
    sendToDetector(F_SET_NETWORK_PARAMETER, args, retval);
    FILE_LOG(logDEBUG1) << "Network Parameter (" << index << "): " << retval;
    return retval;
}

std::string
slsDetector::setAdditionalJsonHeader(const std::string &jsonheader) {
    int fnum = F_ADDITIONAL_JSON_HEADER;
    char args[MAX_STR_LENGTH]{};
    char retvals[MAX_STR_LENGTH]{};
    sls::strcpy_safe(args, jsonheader.c_str());
    FILE_LOG(logDEBUG1) << "Sending additional json header " << args;

    if (!shm()->useReceiverFlag) {
        sls::strcpy_safe(shm()->rxAdditionalJsonHeader, jsonheader.c_str());
    } else {
        sendToReceiver(fnum, args, sizeof(args), retvals, sizeof(retvals));
        FILE_LOG(logDEBUG1) << "Additional json header: " << retvals;
        memset(shm()->rxAdditionalJsonHeader, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(shm()->rxAdditionalJsonHeader, retvals);
    }
    return shm()->rxAdditionalJsonHeader;
}

std::string slsDetector::getAdditionalJsonHeader() {
    int fnum = F_GET_ADDITIONAL_JSON_HEADER;
    char retvals[MAX_STR_LENGTH]{};
    FILE_LOG(logDEBUG1) << "Getting additional json header ";
    if (shm()->useReceiverFlag) {
        sendToReceiver(fnum, nullptr, 0, retvals, sizeof(retvals));
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

int slsDetector::digitalTest(digitalTestMode mode, int ival) {
    int args[]{static_cast<int>(mode), ival};
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending digital test of mode " << mode << ", ival "
                        << ival;
    sendToDetector(F_DIGITAL_TEST, args, retval);
    FILE_LOG(logDEBUG1) << "Digital Test returned: " << retval;
    return retval;
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

    // old firmware requires configuremac after setting roi
    if (shm()->myDetectorType == GOTTHARD) {
        configureMAC();
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
    // readoutflags
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
        sendToReceiver(fnum, &mask, sizeof(mask), &retval, sizeof(retval));
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
    shm()->rxDbitList = list;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_DBIT_LIST, shm()->rxDbitList, nullptr);
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
    if (value >= 0)
        shm()->rxDbitOffset = value;
    FILE_LOG(logDEBUG1) << "Setting digital bit offset in receiver to "
                        << value;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_DBIT_OFFSET, value, retval);
        FILE_LOG(logDEBUG1) << "Receiver digital bit offset: " << retval;
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
        sendToReceiver(fnum, &enable, sizeof(enable), &retval, sizeof(retval));
    }
    return static_cast<int>(shm()->activated);
}

bool slsDetector::setDeactivatedRxrPaddingMode(int padding) {
    int fnum = F_RECEIVER_DEACTIVATED_PADDING_ENABLE;
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable: " << padding;
    if (shm()->useReceiverFlag) {
        sendToReceiver(fnum, &padding, sizeof(padding), &retval,
                       sizeof(retval));
        FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable:" << retval;
        shm()->rxPadDeactivatedModules = static_cast<bool>(retval);
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
        int fnum = F_ENABLE_GAPPIXELS_IN_RECEIVER;
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Sending gap pixels enable to receiver: " << val;
        if (shm()->useReceiverFlag) {
            sendToReceiver(fnum, &val, sizeof(val), &retval, sizeof(retval));
            FILE_LOG(logDEBUG1) << "Gap pixels enable to receiver:" << retval;
            shm()->gappixels = retval;
        }
    }
    return shm()->gappixels;
}

int slsDetector::setTrimEn(std::vector<int> energies) {
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
    os << "#Detector " << detId << ":\n Receiver Hostname:\t"
       << getReceiverHostname() << "\nDetector UDP IP (Source):\t\t"
       << getDetectorIP() << "\nDetector UDP IP2 (Source):\t\t"
       << getDetectorIP2() << "\nDetector UDP MAC:\t\t" << getDetectorMAC()
       << "\nDetector UDP MAC2:\t\t" << getDetectorMAC2()
       << "\nReceiver UDP IP:\t" << getReceiverUDPIP()
       << "\nReceiver UDP IP2:\t" << getReceiverUDPIP2()
       << "\nReceiver UDP MAC:\t" << getReceiverUDPMAC()
       << "\nReceiver UDP MAC2:\t" << getReceiverUDPMAC2()
       << "\nReceiver UDP Port:\t" << getReceiverUDPPort()
       << "\nReceiver UDP Port2:\t" << getReceiverUDPPort2();
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

std::string slsDetector::getReceiverLastClientIP() const {
    char retval[INET_ADDRSTRLEN]{};
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
        char cstring[MAX_STR_LENGTH]{};
        char lastClientIP[INET_ADDRSTRLEN]{};

        n += receiver.Receive(lastClientIP, sizeof(lastClientIP));
        FILE_LOG(logDEBUG1)
            << "Updating receiver last modified by " << lastClientIP;

        // filepath
        n += receiver.Receive(cstring, sizeof(cstring));
        sls::strcpy_safe(shm()->rxFilePath, cstring);

        // filename
        n += receiver.Receive(cstring, sizeof(cstring));
        sls::strcpy_safe(shm()->rxFileName, cstring);

        // index
        n += receiver.Receive(&i32, sizeof(i32));
        shm()->rxFileIndex = i32;

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
        n += receiver.Receive(cstring, sizeof(cstring));
        shm()->rxZmqip = cstring;

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
    }
    shm()->rxFramePadding = static_cast<bool>(retval);
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
        }
    }
    return getFileFormat();
}

slsDetectorDefs::fileFormat slsDetector::getFileFormat() const {
    return shm()->rxFileFormat;
}

int slsDetector::setFileIndex(int file_index) {
    if (F_SET_RECEIVER_FILE_INDEX >= 0) {
        int retval = -1;
        FILE_LOG(logDEBUG1) << "Setting file index to " << file_index;
        if (shm()->useReceiverFlag) {
            sendToReceiver(F_SET_RECEIVER_FILE_INDEX, file_index, retval);
            FILE_LOG(logDEBUG1) << "Receiver file index: " << retval;
            shm()->rxFileIndex = retval;
        }
    }
    return getFileIndex();
}

int slsDetector::getFileIndex() const { return shm()->rxFileIndex; }

int slsDetector::incrementFileIndex() {
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
        FILE_LOG(logDEBUG1) << "Receiver Status: " << runStatusType(retval);
    }
    return retval;
}

int slsDetector::getFramesCaughtByReceiver() const {
    int retval = -1;
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

void slsDetector::resetFramesCaught() {
    FILE_LOG(logDEBUG1) << "Reset Frames Caught by Receiver";
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RESET_RECEIVER_FRAMES_CAUGHT);
    }
}

bool slsDetector::setFileWrite(bool value) {
    int arg = static_cast<int>(value);
    int retval = -1;
    FILE_LOG(logDEBUG1) << "Sending enable file write to receiver: " << arg;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_ENABLE_RECEIVER_FILE_WRITE, arg, retval);
        FILE_LOG(logDEBUG1) << "Receiver file write enable: " << retval;
        shm()->rxFileWrite = static_cast<bool>(retval);
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
    if (value >= 0) {
        configureMAC();
    }
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

std::array<int, 3> slsDetector::setPatternLoops(int level, int start, int stop,
                                                int n) {
    int args[]{level, start, stop, n};
    std::array<int, 3> retvals{};
    FILE_LOG(logDEBUG1) << "Setting Pat Loops, level: " << level
                        << ", start: " << start << ", stop: " << stop
                        << ", nloops: " << n;
    sendToDetector(F_SET_PATTERN_LOOP, args, retvals);
    FILE_LOG(logDEBUG1) << "Set Pat Loops: " << retvals[0] << ", " << retvals[1]
                        << ", " << retvals[2];
    return retvals;
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
            throw RuntimeError("Interpolate module: dacs different");
        }
        myMod.dacs[dacs_to_copy[i]] = a->dacs[dacs_to_copy[i]];
    }

    // Copy irrelevant dacs (without failing): CAL
    if (a->dacs[CAL] != b->dacs[CAL]) {
        FILE_LOG(logWARNING)
            << "DAC CAL differs in both energies (" << a->dacs[CAL] << ","
            << b->dacs[CAL] << ")!\nTaking first: " << a->dacs[CAL];
    }
    myMod.dacs[CAL] = a->dacs[CAL];

    // Interpolate vrf, vcmp, vcp
    int dacs_to_interpolate[] = {VRF,     VCMP_LL, VCMP_LR, VCMP_RL,
                                 VCMP_RR, VCP,     VRS};
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
