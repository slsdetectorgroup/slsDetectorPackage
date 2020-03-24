/* WARINING This file is auto generated any edits might be overwritten without
 * warning */

#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"
#include "ToString.h"
#include "network_utils.h"
#include "sls_detector_defs.h"
#include "typecaster.h"
namespace py = pybind11;
void init_det(py::module &m) {
    using sls::defs;
    using sls::Detector;
    using sls::Positions;
    using sls::Result;

    py::class_<Detector> CppDetectorApi(m, "CppDetectorApi");
    CppDetectorApi
        .def(py::init<int>())

        .def("freeSharedMemory",
             (void (Detector::*)()) & Detector::freeSharedMemory)
        .def("loadConfig",
             (void (Detector::*)(const std::string &)) & Detector::loadConfig,
             py::arg())
        .def("loadParameters",
             (void (Detector::*)(const std::string &)) &
                 Detector::loadParameters,
             py::arg())
        .def("getHostname",
             (Result<std::string>(Detector::*)(sls::Positions)) &
                 Detector::getHostname,
             py::arg() = Positions{})
        .def("setHostname",
             (void (Detector::*)(const std::vector<std::string> &)) &
                 Detector::setHostname,
             py::arg())
        .def("setVirtualDetectorServers",
             (void (Detector::*)(int, int)) &
                 Detector::setVirtualDetectorServers,
             py::arg(), py::arg())
        .def("getShmId", (int (Detector::*)()) & Detector::getShmId)
        .def("getPackageVersion",
             (std::string(Detector::*)()) & Detector::getPackageVersion)
        .def("getClientVersion",
             (int64_t(Detector::*)()) & Detector::getClientVersion)
        .def("getFirmwareVersion",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getFirmwareVersion,
             py::arg() = Positions{})
        .def("getDetectorServerVersion",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getDetectorServerVersion,
             py::arg() = Positions{})
        .def("getSerialNumber",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getSerialNumber,
             py::arg() = Positions{})
        .def("getReceiverVersion",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getReceiverVersion,
             py::arg() = Positions{})
        .def("getDetectorType",
             (Result<defs::detectorType>(Detector::*)(sls::Positions)) &
                 Detector::getDetectorType,
             py::arg() = Positions{})
        .def("size", (int (Detector::*)()) & Detector::size)
        .def("empty", (bool (Detector::*)()) & Detector::empty)
        .def("getModuleGeometry",
             (defs::xy(Detector::*)()) & Detector::getModuleGeometry)
        .def("getModuleSize",
             (Result<defs::xy>(Detector::*)(sls::Positions)) &
                 Detector::getModuleSize,
             py::arg() = Positions{})
        .def("getDetectorSize",
             (defs::xy(Detector::*)()) & Detector::getDetectorSize)
        .def("setDetectorSize",
             (void (Detector::*)(const defs::xy)) & Detector::setDetectorSize,
             py::arg())
        .def("getSettings",
             (Result<defs::detectorSettings>(Detector::*)(sls::Positions)) &
                 Detector::getSettings,
             py::arg() = Positions{})
        .def("setSettings",
             (void (Detector::*)(defs::detectorSettings, sls::Positions)) &
                 Detector::setSettings,
             py::arg(), py::arg() = Positions{})
        .def("registerAcquisitionFinishedCallback",
             (void (Detector::*)(void (*)(double, int, void *), void *)) &
                 Detector::registerAcquisitionFinishedCallback,
             py::arg(), py::arg())
        .def(
            "registerDataCallback",
            (void (Detector::*)(
                void (*)(detectorData *, uint64_t, uint32_t, void *), void *)) &
                Detector::registerDataCallback,
            py::arg(), py::arg())
        .def("getNumberOfFrames",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfFrames,
             py::arg() = Positions{})
        .def("setNumberOfFrames",
             (void (Detector::*)(int64_t)) & Detector::setNumberOfFrames,
             py::arg())
        .def("getNumberOfTriggers",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfTriggers,
             py::arg() = Positions{})
        .def("setNumberOfTriggers",
             (void (Detector::*)(int64_t)) & Detector::setNumberOfTriggers,
             py::arg())
        .def("getExptime",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getExptime,
             py::arg() = Positions{})
        .def("setExptime",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setExptime,
             py::arg(), py::arg() = Positions{})
        .def("getPeriod",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getPeriod,
             py::arg() = Positions{})
        .def("setPeriod",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setPeriod,
             py::arg(), py::arg() = Positions{})
        .def("getDelayAfterTrigger",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getDelayAfterTrigger,
             py::arg() = Positions{})
        .def("setDelayAfterTrigger",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setDelayAfterTrigger,
             py::arg(), py::arg() = Positions{})
        .def("getNumberOfFramesLeft",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfFramesLeft,
             py::arg() = Positions{})
        .def("getNumberOfTriggersLeft",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfTriggersLeft,
             py::arg() = Positions{})
        .def("getPeriodLeft",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getPeriodLeft,
             py::arg() = Positions{})
        .def("getDelayAfterTriggerLeft",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getDelayAfterTriggerLeft,
             py::arg() = Positions{})
        .def("getTimingMode",
             (Result<defs::timingMode>(Detector::*)(sls::Positions)) &
                 Detector::getTimingMode,
             py::arg() = Positions{})
        .def("setTimingMode",
             (void (Detector::*)(defs::timingMode, sls::Positions)) &
                 Detector::setTimingMode,
             py::arg(), py::arg() = Positions{})
        .def("getSpeed",
             (Result<defs::speedLevel>(Detector::*)(sls::Positions)) &
                 Detector::getSpeed,
             py::arg() = Positions{})
        .def("setSpeed",
             (void (Detector::*)(defs::speedLevel, sls::Positions)) &
                 Detector::setSpeed,
             py::arg(), py::arg() = Positions{})
        .def("getADCPhase",
             (Result<int>(Detector::*)(sls::Positions)) & Detector::getADCPhase,
             py::arg() = Positions{})
        .def("setADCPhase",
             (void (Detector::*)(int, sls::Positions)) & Detector::setADCPhase,
             py::arg(), py::arg() = Positions{})
        .def("getMaxADCPhaseShift",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getMaxADCPhaseShift,
             py::arg() = Positions{})
        .def("getADCPhaseInDegrees",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getADCPhaseInDegrees,
             py::arg() = Positions{})
        .def("setADCPhaseInDegrees",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setADCPhaseInDegrees,
             py::arg(), py::arg() = Positions{})
        .def("getDBITPhase",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getDBITPhase,
             py::arg() = Positions{})
        .def("setDBITPhase",
             (void (Detector::*)(int, sls::Positions)) & Detector::setDBITPhase,
             py::arg(), py::arg() = Positions{})
        .def("getMaxDBITPhaseShift",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getMaxDBITPhaseShift,
             py::arg() = Positions{})
        .def("getDBITPhaseInDegrees",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getDBITPhaseInDegrees,
             py::arg() = Positions{})
        .def("setDBITPhaseInDegrees",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setDBITPhaseInDegrees,
             py::arg(), py::arg() = Positions{})
        .def("getClockFrequency",
             (Result<int>(Detector::*)(int, sls::Positions)) &
                 Detector::getClockFrequency,
             py::arg(), py::arg() = Positions{})
        .def("setClockFrequency",
             (void (Detector::*)(int, int, sls::Positions)) &
                 Detector::setClockFrequency,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getClockPhase",
             (Result<int>(Detector::*)(int, sls::Positions)) &
                 Detector::getClockPhase,
             py::arg(), py::arg() = Positions{})
        .def("setClockPhase",
             (void (Detector::*)(int, int, sls::Positions)) &
                 Detector::setClockPhase,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getMaxClockPhaseShift",
             (Result<int>(Detector::*)(int, sls::Positions)) &
                 Detector::getMaxClockPhaseShift,
             py::arg(), py::arg() = Positions{})
        .def("getClockPhaseinDegrees",
             (Result<int>(Detector::*)(int, sls::Positions)) &
                 Detector::getClockPhaseinDegrees,
             py::arg(), py::arg() = Positions{})
        .def("setClockPhaseinDegrees",
             (void (Detector::*)(int, int, sls::Positions)) &
                 Detector::setClockPhaseinDegrees,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getClockDivider",
             (Result<int>(Detector::*)(int, sls::Positions)) &
                 Detector::getClockDivider,
             py::arg(), py::arg() = Positions{})
        .def("setClockDivider",
             (void (Detector::*)(int, int, sls::Positions)) &
                 Detector::setClockDivider,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getHighVoltage",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getHighVoltage,
             py::arg() = Positions{})
        .def("getPowerChip",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getPowerChip,
             py::arg() = Positions{})
        .def("setPowerChip",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setPowerChip,
             py::arg(), py::arg() = Positions{})
        .def("setHighVoltage",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setHighVoltage,
             py::arg(), py::arg() = Positions{})
        .def("getTemperature",
             (Result<int>(Detector::*)(defs::dacIndex, sls::Positions)) &
                 Detector::getTemperature,
             py::arg(), py::arg() = Positions{})
        .def("getDAC",
             (Result<int>(Detector::*)(defs::dacIndex, bool, sls::Positions)) &
                 Detector::getDAC,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("setDAC",
             (void (Detector::*)(defs::dacIndex, int, bool, sls::Positions)) &
                 Detector::setDAC,
             py::arg(), py::arg(), py::arg(), py::arg() = Positions{})
        .def("getOnChipDAC",
             (Result<int>(Detector::*)(defs::dacIndex, int, sls::Positions)) &
                 Detector::getOnChipDAC,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("setOnChipDAC",
             (void (Detector::*)(defs::dacIndex, int, int, sls::Positions)) &
                 Detector::setOnChipDAC,
             py::arg(), py::arg(), py::arg(), py::arg() = Positions{})
        .def("acquire", (void (Detector::*)()) & Detector::acquire)
        .def("clearAcquiringFlag",
             (void (Detector::*)()) & Detector::clearAcquiringFlag)
        .def("startReceiver", (void (Detector::*)()) & Detector::startReceiver)
        .def("stopReceiver", (void (Detector::*)()) & Detector::stopReceiver)
        .def("startDetector", (void (Detector::*)()) & Detector::startDetector)
        .def("stopDetector", (void (Detector::*)()) & Detector::stopDetector)
        .def("getDetectorStatus",
             (Result<defs::runStatus>(Detector::*)(sls::Positions)) &
                 Detector::getDetectorStatus,
             py::arg() = Positions{})
        .def("getReceiverStatus",
             (Result<defs::runStatus>(Detector::*)(sls::Positions)) &
                 Detector::getReceiverStatus,
             py::arg() = Positions{})
        .def("getFramesCaught",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getFramesCaught,
             py::arg() = Positions{})
        .def("getNumMissingPackets",
             (Result<std::vector<uint64_t>>(Detector::*)(sls::Positions)) &
                 Detector::getNumMissingPackets,
             py::arg() = Positions{})
        .def("getStartingFrameNumber",
             (Result<uint64_t>(Detector::*)(sls::Positions)) &
                 Detector::getStartingFrameNumber,
             py::arg() = Positions{})
        .def("setStartingFrameNumber",
             (void (Detector::*)(uint64_t, sls::Positions)) &
                 Detector::setStartingFrameNumber,
             py::arg(), py::arg() = Positions{})
        .def("sendSoftwareTrigger",
             (void (Detector::*)(sls::Positions)) &
                 Detector::sendSoftwareTrigger,
             py::arg() = Positions{})
        .def("getNumberofUDPInterfaces",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getNumberofUDPInterfaces,
             py::arg() = Positions{})
        .def("setNumberofUDPInterfaces",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setNumberofUDPInterfaces,
             py::arg(), py::arg() = Positions{})
        .def("getSelectedUDPInterface",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getSelectedUDPInterface,
             py::arg() = Positions{})
        .def("selectUDPInterface",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::selectUDPInterface,
             py::arg(), py::arg() = Positions{})
        .def("getSourceUDPIP",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getSourceUDPIP,
             py::arg() = Positions{})
        .def("setSourceUDPIP",
             (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                 Detector::setSourceUDPIP,
             py::arg(), py::arg() = Positions{})
        .def("getSourceUDPIP2",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getSourceUDPIP2,
             py::arg() = Positions{})
        .def("setSourceUDPIP2",
             (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                 Detector::setSourceUDPIP2,
             py::arg(), py::arg() = Positions{})
        .def("getSourceUDPMAC",
             (Result<sls::MacAddr>(Detector::*)(sls::Positions)) &
                 Detector::getSourceUDPMAC,
             py::arg() = Positions{})
        .def("setSourceUDPMAC",
             (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
                 Detector::setSourceUDPMAC,
             py::arg(), py::arg() = Positions{})
        .def("getSourceUDPMAC2",
             (Result<sls::MacAddr>(Detector::*)(sls::Positions)) &
                 Detector::getSourceUDPMAC2,
             py::arg() = Positions{})
        .def("setSourceUDPMAC2",
             (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
                 Detector::setSourceUDPMAC2,
             py::arg(), py::arg() = Positions{})
        .def("getDestinationUDPIP",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getDestinationUDPIP,
             py::arg() = Positions{})
        .def("setDestinationUDPIP",
             (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                 Detector::setDestinationUDPIP,
             py::arg(), py::arg() = Positions{})
        .def("getDestinationUDPIP2",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getDestinationUDPIP2,
             py::arg() = Positions{})
        .def("setDestinationUDPIP2",
             (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                 Detector::setDestinationUDPIP2,
             py::arg(), py::arg() = Positions{})
        .def("getDestinationUDPMAC",
             (Result<sls::MacAddr>(Detector::*)(sls::Positions)) &
                 Detector::getDestinationUDPMAC,
             py::arg() = Positions{})
        .def("setDestinationUDPMAC",
             (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
                 Detector::setDestinationUDPMAC,
             py::arg(), py::arg() = Positions{})
        .def("getDestinationUDPMAC2",
             (Result<sls::MacAddr>(Detector::*)(sls::Positions)) &
                 Detector::getDestinationUDPMAC2,
             py::arg() = Positions{})
        .def("setDestinationUDPMAC2",
             (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
                 Detector::setDestinationUDPMAC2,
             py::arg(), py::arg() = Positions{})
        .def("getDestinationUDPPort",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getDestinationUDPPort,
             py::arg() = Positions{})
        .def("setDestinationUDPPort",
             (void (Detector::*)(int, int)) & Detector::setDestinationUDPPort,
             py::arg(), py::arg())
        .def("getDestinationUDPPort2",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getDestinationUDPPort2,
             py::arg() = Positions{})
        .def("setDestinationUDPPort2",
             (void (Detector::*)(int, int)) & Detector::setDestinationUDPPort2,
             py::arg(), py::arg())
        .def("printRxConfiguration",
             (Result<std::string>(Detector::*)(sls::Positions)) &
                 Detector::printRxConfiguration,
             py::arg() = Positions{})
        .def("getTenGiga",
             (Result<bool>(Detector::*)(sls::Positions)) & Detector::getTenGiga,
             py::arg() = Positions{})
        .def("setTenGiga",
             (void (Detector::*)(bool, sls::Positions)) & Detector::setTenGiga,
             py::arg(), py::arg() = Positions{})
        .def("getTenGigaFlowControl",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getTenGigaFlowControl,
             py::arg() = Positions{})
        .def("setTenGigaFlowControl",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setTenGigaFlowControl,
             py::arg(), py::arg() = Positions{})
        .def("getTransmissionDelayFrame",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getTransmissionDelayFrame,
             py::arg() = Positions{})
        .def("setTransmissionDelayFrame",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setTransmissionDelayFrame,
             py::arg(), py::arg() = Positions{})
        .def("getTransmissionDelayLeft",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getTransmissionDelayLeft,
             py::arg() = Positions{})
        .def("setTransmissionDelayLeft",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setTransmissionDelayLeft,
             py::arg(), py::arg() = Positions{})
        .def("getTransmissionDelayRight",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getTransmissionDelayRight,
             py::arg() = Positions{})
        .def("setTransmissionDelayRight",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setTransmissionDelayRight,
             py::arg(), py::arg() = Positions{})
        .def("getUseReceiverFlag",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getUseReceiverFlag,
             py::arg() = Positions{})
        .def("getRxHostname",
             (Result<std::string>(Detector::*)(sls::Positions)) &
                 Detector::getRxHostname,
             py::arg() = Positions{})
        .def("setRxHostname",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::setRxHostname,
             py::arg(), py::arg() = Positions{})
        .def("setRxHostname",
             (void (Detector::*)(const std::vector<std::string> &)) &
                 Detector::setRxHostname,
             py::arg())
        .def("getRxPort",
             (Result<int>(Detector::*)(sls::Positions)) & Detector::getRxPort,
             py::arg() = Positions{})
        .def("setRxPort", (void (Detector::*)(int, int)) & Detector::setRxPort,
             py::arg(), py::arg())
        .def("getRxFifoDepth",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getRxFifoDepth,
             py::arg() = Positions{})
        .def("setRxFifoDepth",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setRxFifoDepth,
             py::arg(), py::arg() = Positions{})
        .def("getRxSilentMode",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getRxSilentMode,
             py::arg() = Positions{})
        .def("setRxSilentMode",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setRxSilentMode,
             py::arg(), py::arg() = Positions{})
        .def("getRxFrameDiscardPolicy",
             (Result<defs::frameDiscardPolicy>(Detector::*)(sls::Positions)) &
                 Detector::getRxFrameDiscardPolicy,
             py::arg() = Positions{})
        .def("setRxFrameDiscardPolicy",
             (void (Detector::*)(defs::frameDiscardPolicy, sls::Positions)) &
                 Detector::setRxFrameDiscardPolicy,
             py::arg(), py::arg() = Positions{})
        .def("getPartialFramesPadding",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getPartialFramesPadding,
             py::arg() = Positions{})
        .def("setPartialFramesPadding",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setPartialFramesPadding,
             py::arg(), py::arg() = Positions{})
        .def("getRxUDPSocketBufferSize",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getRxUDPSocketBufferSize,
             py::arg() = Positions{})
        .def("setRxUDPSocketBufferSize",
             (void (Detector::*)(int64_t, sls::Positions)) &
                 Detector::setRxUDPSocketBufferSize,
             py::arg(), py::arg() = Positions{})
        .def("getRxRealUDPSocketBufferSize",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getRxRealUDPSocketBufferSize,
             py::arg() = Positions{})
        .def("getRxLock",
             (Result<bool>(Detector::*)(sls::Positions)) & Detector::getRxLock,
             py::arg() = Positions{})
        .def("setRxLock",
             (void (Detector::*)(bool, sls::Positions)) & Detector::setRxLock,
             py::arg(), py::arg() = Positions{})
        .def("getRxLastClientIP",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getRxLastClientIP,
             py::arg() = Positions{})
        .def("getFileFormat",
             (Result<defs::fileFormat>(Detector::*)(sls::Positions)) &
                 Detector::getFileFormat,
             py::arg() = Positions{})
        .def("setFileFormat",
             (void (Detector::*)(defs::fileFormat, sls::Positions)) &
                 Detector::setFileFormat,
             py::arg(), py::arg() = Positions{})
        .def("getFilePath",
             (Result<std::string>(Detector::*)(sls::Positions)) &
                 Detector::getFilePath,
             py::arg() = Positions{})
        .def("setFilePath",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::setFilePath,
             py::arg(), py::arg() = Positions{})
        .def("getFileNamePrefix",
             (Result<std::string>(Detector::*)(sls::Positions)) &
                 Detector::getFileNamePrefix,
             py::arg() = Positions{})
        .def("setFileNamePrefix",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::setFileNamePrefix,
             py::arg(), py::arg() = Positions{})
        .def("getAcquisitionIndex",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getAcquisitionIndex,
             py::arg() = Positions{})
        .def("setAcquisitionIndex",
             (void (Detector::*)(int64_t, sls::Positions)) &
                 Detector::setAcquisitionIndex,
             py::arg(), py::arg() = Positions{})
        .def("getFileWrite",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getFileWrite,
             py::arg() = Positions{})
        .def("setFileWrite",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setFileWrite,
             py::arg(), py::arg() = Positions{})
        .def("getMasterFileWrite",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getMasterFileWrite,
             py::arg() = Positions{})
        .def("setMasterFileWrite",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setMasterFileWrite,
             py::arg(), py::arg() = Positions{})
        .def("getFileOverWrite",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getFileOverWrite,
             py::arg() = Positions{})
        .def("setFileOverWrite",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setFileOverWrite,
             py::arg(), py::arg() = Positions{})
        .def("getFramesPerFile",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getFramesPerFile,
             py::arg() = Positions{})
        .def("setFramesPerFile",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setFramesPerFile,
             py::arg(), py::arg() = Positions{})
        .def("getRxZmqDataStream",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getRxZmqDataStream,
             py::arg() = Positions{})
        .def("setRxZmqDataStream",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setRxZmqDataStream,
             py::arg(), py::arg() = Positions{})
        .def("getRxZmqFrequency",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getRxZmqFrequency,
             py::arg() = Positions{})
        .def("setRxZmqFrequency",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setRxZmqFrequency,
             py::arg(), py::arg() = Positions{})
        .def("getRxZmqTimer",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getRxZmqTimer,
             py::arg() = Positions{})
        .def("setRxZmqTimer",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setRxZmqTimer,
             py::arg(), py::arg() = Positions{})
        .def("getRxZmqPort",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getRxZmqPort,
             py::arg() = Positions{})
        .def("setRxZmqPort",
             (void (Detector::*)(int, int)) & Detector::setRxZmqPort, py::arg(),
             py::arg())
        .def("getRxZmqIP",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getRxZmqIP,
             py::arg() = Positions{})
        .def("setRxZmqIP",
             (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                 Detector::setRxZmqIP,
             py::arg(), py::arg() = Positions{})
        .def("getClientZmqPort",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getClientZmqPort,
             py::arg() = Positions{})
        .def("setClientZmqPort",
             (void (Detector::*)(int, int)) & Detector::setClientZmqPort,
             py::arg(), py::arg())
        .def("getClientZmqIp",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getClientZmqIp,
             py::arg() = Positions{})
        .def("setClientZmqIp",
             (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                 Detector::setClientZmqIp,
             py::arg(), py::arg() = Positions{})
        .def("getDynamicRange",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getDynamicRange,
             py::arg() = Positions{})
        .def("setDynamicRange",
             (void (Detector::*)(int)) & Detector::setDynamicRange, py::arg())
        .def("getSubExptime",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getSubExptime,
             py::arg() = Positions{})
        .def("setSubExptime",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setSubExptime,
             py::arg(), py::arg() = Positions{})
        .def("getSubDeadTime",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getSubDeadTime,
             py::arg() = Positions{})
        .def("setSubDeadTime",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setSubDeadTime,
             py::arg(), py::arg() = Positions{})
        .def("getThresholdEnergy",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getThresholdEnergy,
             py::arg() = Positions{})
        .def("setThresholdEnergy",
             (void (Detector::*)(int, defs::detectorSettings, bool,
                                 sls::Positions)) &
                 Detector::setThresholdEnergy,
             py::arg(), py::arg(), py::arg(), py::arg() = Positions{})
        .def("getSettingsPath",
             (Result<std::string>(Detector::*)(sls::Positions)) &
                 Detector::getSettingsPath,
             py::arg() = Positions{})
        .def("setSettingsPath",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::setSettingsPath,
             py::arg(), py::arg() = Positions{})
        .def("loadTrimbits",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::loadTrimbits,
             py::arg(), py::arg() = Positions{})
        .def("getRxAddGapPixels",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getRxAddGapPixels,
             py::arg() = Positions{})
        .def("setRxAddGapPixels",
             (void (Detector::*)(bool)) & Detector::setRxAddGapPixels,
             py::arg())
        .def("getParallelMode",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getParallelMode,
             py::arg() = Positions{})
        .def("setParallelMode",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setParallelMode,
             py::arg(), py::arg() = Positions{})
        .def("getOverFlowMode",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getOverFlowMode,
             py::arg() = Positions{})
        .def("setOverFlowMode",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setOverFlowMode,
             py::arg(), py::arg() = Positions{})
        .def("getStoreInRamMode",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getStoreInRamMode,
             py::arg() = Positions{})
        .def("setStoreInRamMode",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setStoreInRamMode,
             py::arg(), py::arg() = Positions{})
        .def("getBottom",
             (Result<bool>(Detector::*)(sls::Positions)) & Detector::getBottom,
             py::arg() = Positions{})
        .def("setBottom",
             (void (Detector::*)(bool, sls::Positions)) & Detector::setBottom,
             py::arg(), py::arg() = Positions{})
        .def("getAllTrimbits",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getAllTrimbits,
             py::arg() = Positions{})
        .def("setAllTrimbits",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setAllTrimbits,
             py::arg(), py::arg() = Positions{})
        .def("getTrimEnergies",
             (Result<std::vector<int>>(Detector::*)(sls::Positions)) &
                 Detector::getTrimEnergies,
             py::arg() = Positions{})
        .def("setTrimEnergies",
             (void (Detector::*)(std::vector<int>, sls::Positions)) &
                 Detector::setTrimEnergies,
             py::arg(), py::arg() = Positions{})
        .def("getRateCorrection",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getRateCorrection,
             py::arg() = Positions{})
        .def("setDefaultRateCorrection",
             (void (Detector::*)(sls::Positions)) &
                 Detector::setDefaultRateCorrection,
             py::arg() = Positions{})
        .def("setRateCorrection",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setRateCorrection,
             py::arg(), py::arg() = Positions{})
        .def("getPartialReadout",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getPartialReadout,
             py::arg() = Positions{})
        .def("setPartialReadout",
             (void (Detector::*)(const int, sls::Positions)) &
                 Detector::setPartialReadout,
             py::arg(), py::arg() = Positions{})
        .def("getInterruptSubframe",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getInterruptSubframe,
             py::arg() = Positions{})
        .def("setInterruptSubframe",
             (void (Detector::*)(const bool, sls::Positions)) &
                 Detector::setInterruptSubframe,
             py::arg(), py::arg() = Positions{})
        .def("getMeasuredPeriod",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getMeasuredPeriod,
             py::arg() = Positions{})
        .def("getMeasuredSubFramePeriod",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getMeasuredSubFramePeriod,
             py::arg() = Positions{})
        .def("getActive",
             (Result<bool>(Detector::*)(sls::Positions)) & Detector::getActive,
             py::arg() = Positions{})
        .def("setActive",
             (void (Detector::*)(bool, sls::Positions)) & Detector::setActive,
             py::arg(), py::arg() = Positions{})
        .def("getRxPadDeactivatedMode",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getRxPadDeactivatedMode,
             py::arg() = Positions{})
        .def("setRxPadDeactivatedMode",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setRxPadDeactivatedMode,
             py::arg(), py::arg() = Positions{})
        .def("getPartialReset",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getPartialReset,
             py::arg() = Positions{})
        .def("setPartialReset",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setPartialReset,
             py::arg(), py::arg() = Positions{})
        .def("pulsePixel",
             (void (Detector::*)(int, defs::xy, sls::Positions)) &
                 Detector::pulsePixel,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("pulsePixelNMove",
             (void (Detector::*)(int, defs::xy, sls::Positions)) &
                 Detector::pulsePixelNMove,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("pulseChip",
             (void (Detector::*)(int, sls::Positions)) & Detector::pulseChip,
             py::arg(), py::arg() = Positions{})
        .def("getQuad",
             (Result<bool>(Detector::*)(sls::Positions)) & Detector::getQuad,
             py::arg() = Positions{})
        .def("setQuad", (void (Detector::*)(const bool)) & Detector::setQuad,
             py::arg())
        .def("getThresholdTemperature",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getThresholdTemperature,
             py::arg() = Positions{})
        .def("setThresholdTemperature",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setThresholdTemperature,
             py::arg(), py::arg() = Positions{})
        .def("getTemperatureControl",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getTemperatureControl,
             py::arg() = Positions{})
        .def("setTemperatureControl",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setTemperatureControl,
             py::arg(), py::arg() = Positions{})
        .def("getTemperatureEvent",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getTemperatureEvent,
             py::arg() = Positions{})
        .def("resetTemperatureEvent",
             (void (Detector::*)(sls::Positions)) &
                 Detector::resetTemperatureEvent,
             py::arg() = Positions{})
        .def("getAutoCompDisable",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getAutoCompDisable,
             py::arg() = Positions{})
        .def("setAutoCompDisable",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setAutoCompDisable,
             py::arg(), py::arg() = Positions{})
        .def("getNumberOfAdditionalStorageCells",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfAdditionalStorageCells,
             py::arg() = Positions{})
        .def("setNumberOfAdditionalStorageCells",
             (void (Detector::*)(int)) &
                 Detector::setNumberOfAdditionalStorageCells,
             py::arg())
        .def("getStorageCellStart",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getStorageCellStart,
             py::arg() = Positions{})
        .def("setStoragecellStart",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setStoragecellStart,
             py::arg(), py::arg() = Positions{})
        .def("getStorageCellDelay",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getStorageCellDelay,
             py::arg() = Positions{})
        .def("setStorageCellDelay",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setStorageCellDelay,
             py::arg(), py::arg() = Positions{})
        .def("getROI",
             (Result<defs::ROI>(Detector::*)(sls::Positions)) &
                 Detector::getROI,
             py::arg() = Positions{})
        .def("setROI", (void (Detector::*)(defs::ROI, int)) & Detector::setROI,
             py::arg(), py::arg())
        .def("clearROI",
             (void (Detector::*)(sls::Positions)) & Detector::clearROI,
             py::arg() = Positions{})
        .def("getExptimeLeft",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getExptimeLeft,
             py::arg() = Positions{})
        .def("getExternalSignalFlags",
             (Result<defs::externalSignalFlag>(Detector::*)(sls::Positions)) &
                 Detector::getExternalSignalFlags,
             py::arg() = Positions{})
        .def("setExternalSignalFlags",
             (void (Detector::*)(defs::externalSignalFlag, sls::Positions)) &
                 Detector::setExternalSignalFlags,
             py::arg(), py::arg() = Positions{})
        .def("getImageTestMode",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getImageTestMode,
             py::arg() = Positions{})
        .def("setImageTestMode",
             (void (Detector::*)(const int, sls::Positions)) &
                 Detector::setImageTestMode,
             py::arg(), py::arg() = Positions{})
        .def("getNumberOfBursts",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfBursts,
             py::arg() = Positions{})
        .def("setNumberOfBursts",
             (void (Detector::*)(int64_t)) & Detector::setNumberOfBursts,
             py::arg())
        .def("getBurstPeriod",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getBurstPeriod,
             py::arg() = Positions{})
        .def("setBurstPeriod",
             (void (Detector::*)(sls::ns, sls::Positions)) &
                 Detector::setBurstPeriod,
             py::arg(), py::arg() = Positions{})
        .def("getInjectChannel",
             (Result<std::array<int, 2>>(Detector::*)(sls::Positions)) &
                 Detector::getInjectChannel,
             py::arg() = Positions{})
        .def("setInjectChannel",
             (void (Detector::*)(const int, const int, sls::Positions)) &
                 Detector::setInjectChannel,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def(
            "getVetoPhoton",
            (Result<std::vector<int>>(Detector::*)(const int, sls::Positions)) &
                Detector::getVetoPhoton,
            py::arg(), py::arg() = Positions{})
        .def("setVetoPhoton",
             (void (Detector::*)(const int, const int, const int,
                                 const std::string &, sls::Positions)) &
                 Detector::setVetoPhoton,
             py::arg(), py::arg(), py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("setVetoReference",
             (void (Detector::*)(const int, const int, sls::Positions)) &
                 Detector::setVetoReference,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getBurstMode",
             (Result<defs::burstMode>(Detector::*)(sls::Positions)) &
                 Detector::getBurstMode,
             py::arg() = Positions{})
        .def("setBurstMode",
             (void (Detector::*)(defs::burstMode, sls::Positions)) &
                 Detector::setBurstMode,
             py::arg(), py::arg() = Positions{})
        .def("getCurrentSource",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getCurrentSource,
             py::arg() = Positions{})
        .def("setCurrentSource",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setCurrentSource,
             py::arg(), py::arg() = Positions{})
        .def("getTimingSource",
             (Result<defs::timingSourceType>(Detector::*)(sls::Positions)) &
                 Detector::getTimingSource,
             py::arg() = Positions{})
        .def("setTimingSource",
             (void (Detector::*)(defs::timingSourceType, sls::Positions)) &
                 Detector::setTimingSource,
             py::arg(), py::arg() = Positions{})
        .def("getCounterMask",
             (Result<uint32_t>(Detector::*)(sls::Positions)) &
                 Detector::getCounterMask,
             py::arg() = Positions{})
        .def("setCounterMask",
             (void (Detector::*)(uint32_t, sls::Positions)) &
                 Detector::setCounterMask,
             py::arg(), py::arg() = Positions{})
        .def("getNumberOfAnalogSamples",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfAnalogSamples,
             py::arg() = Positions{})
        .def("setNumberOfAnalogSamples",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setNumberOfAnalogSamples,
             py::arg(), py::arg() = Positions{})
        .def("getADCClock",
             (Result<int>(Detector::*)(sls::Positions)) & Detector::getADCClock,
             py::arg() = Positions{})
        .def("setADCClock",
             (void (Detector::*)(int, sls::Positions)) & Detector::setADCClock,
             py::arg(), py::arg() = Positions{})
        .def("getRUNClock",
             (Result<int>(Detector::*)(sls::Positions)) & Detector::getRUNClock,
             py::arg() = Positions{})
        .def("setRUNClock",
             (void (Detector::*)(int, sls::Positions)) & Detector::setRUNClock,
             py::arg(), py::arg() = Positions{})
        .def("getSYNCClock",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getSYNCClock,
             py::arg() = Positions{})
        .def("getADCPipeline",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getADCPipeline,
             py::arg() = Positions{})
        .def("setADCPipeline",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setADCPipeline,
             py::arg(), py::arg() = Positions{})
        .def("getVoltage",
             (Result<int>(Detector::*)(defs::dacIndex, sls::Positions)) &
                 Detector::getVoltage,
             py::arg(), py::arg() = Positions{})
        .def("setVoltage",
             (void (Detector::*)(defs::dacIndex, int, sls::Positions)) &
                 Detector::setVoltage,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getADCEnableMask",
             (Result<uint32_t>(Detector::*)(sls::Positions)) &
                 Detector::getADCEnableMask,
             py::arg() = Positions{})
        .def("setADCEnableMask",
             (void (Detector::*)(uint32_t, sls::Positions)) &
                 Detector::setADCEnableMask,
             py::arg(), py::arg() = Positions{})
        .def("getTenGigaADCEnableMask",
             (Result<uint32_t>(Detector::*)(sls::Positions)) &
                 Detector::getTenGigaADCEnableMask,
             py::arg() = Positions{})
        .def("setTenGigaADCEnableMask",
             (void (Detector::*)(uint32_t, sls::Positions)) &
                 Detector::setTenGigaADCEnableMask,
             py::arg(), py::arg() = Positions{})
        .def("getNumberOfDigitalSamples",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfDigitalSamples,
             py::arg() = Positions{})
        .def("setNumberOfDigitalSamples",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setNumberOfDigitalSamples,
             py::arg(), py::arg() = Positions{})
        .def("getReadoutMode",
             (Result<defs::readoutMode>(Detector::*)(sls::Positions)) &
                 Detector::getReadoutMode,
             py::arg() = Positions{})
        .def("setReadoutMode",
             (void (Detector::*)(defs::readoutMode, sls::Positions)) &
                 Detector::setReadoutMode,
             py::arg(), py::arg() = Positions{})
        .def("getDBITClock",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getDBITClock,
             py::arg() = Positions{})
        .def("setDBITClock",
             (void (Detector::*)(int, sls::Positions)) & Detector::setDBITClock,
             py::arg(), py::arg() = Positions{})
        .def("getDBITPipeline",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getDBITPipeline,
             py::arg() = Positions{})
        .def("setDBITPipeline",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setDBITPipeline,
             py::arg(), py::arg() = Positions{})
        .def("getMeasuredVoltage",
             (Result<int>(Detector::*)(defs::dacIndex, sls::Positions)) &
                 Detector::getMeasuredVoltage,
             py::arg(), py::arg() = Positions{})
        .def("getMeasuredCurrent",
             (Result<int>(Detector::*)(defs::dacIndex, sls::Positions)) &
                 Detector::getMeasuredCurrent,
             py::arg(), py::arg() = Positions{})
        .def("getSlowADC",
             (Result<int>(Detector::*)(defs::dacIndex, sls::Positions)) &
                 Detector::getSlowADC,
             py::arg(), py::arg() = Positions{})
        .def("getExternalSamplingSource",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getExternalSamplingSource,
             py::arg() = Positions{})
        .def("setExternalSamplingSource",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setExternalSamplingSource,
             py::arg(), py::arg() = Positions{})
        .def("getExternalSampling",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getExternalSampling,
             py::arg() = Positions{})
        .def("setExternalSampling",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setExternalSampling,
             py::arg(), py::arg() = Positions{})
        .def("getRxDbitList",
             (Result<std::vector<int>>(Detector::*)(sls::Positions)) &
                 Detector::getRxDbitList,
             py::arg() = Positions{})
        .def("setRxDbitList",
             (void (Detector::*)(const std::vector<int> &, sls::Positions)) &
                 Detector::setRxDbitList,
             py::arg(), py::arg() = Positions{})
        .def("getRxDbitOffset",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getRxDbitOffset,
             py::arg() = Positions{})
        .def("setRxDbitOffset",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setRxDbitOffset,
             py::arg(), py::arg() = Positions{})
        .def("setDigitalIODelay",
             (void (Detector::*)(uint64_t, int, sls::Positions)) &
                 Detector::setDigitalIODelay,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getLEDEnable",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getLEDEnable,
             py::arg() = Positions{})
        .def("setLEDEnable",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setLEDEnable,
             py::arg(), py::arg() = Positions{})
        .def("setPattern",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::setPattern,
             py::arg(), py::arg() = Positions{})
        .def("savePattern",
             (void (Detector::*)(const std::string &)) & Detector::savePattern,
             py::arg())
        .def("getPatternIOControl",
             (Result<uint64_t>(Detector::*)(sls::Positions)) &
                 Detector::getPatternIOControl,
             py::arg() = Positions{})
        .def("setPatternIOControl",
             (void (Detector::*)(uint64_t, sls::Positions)) &
                 Detector::setPatternIOControl,
             py::arg(), py::arg() = Positions{})
        .def("getPatternClockControl",
             (Result<uint64_t>(Detector::*)(sls::Positions)) &
                 Detector::getPatternClockControl,
             py::arg() = Positions{})
        .def("setPatternClockControl",
             (void (Detector::*)(uint64_t, sls::Positions)) &
                 Detector::setPatternClockControl,
             py::arg(), py::arg() = Positions{})
        .def("getPatternWord",
             (Result<uint64_t>(Detector::*)(int, sls::Positions)) &
                 Detector::getPatternWord,
             py::arg(), py::arg() = Positions{})
        .def("setPatternWord",
             (void (Detector::*)(int, uint64_t, sls::Positions)) &
                 Detector::setPatternWord,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getPatternLoopAddresses",
             (Result<std::array<int, 2>>(Detector::*)(int, sls::Positions)) &
                 Detector::getPatternLoopAddresses,
             py::arg(), py::arg() = Positions{})
        .def("setPatternLoopAddresses",
             (void (Detector::*)(int, int, int, sls::Positions)) &
                 Detector::setPatternLoopAddresses,
             py::arg(), py::arg(), py::arg(), py::arg() = Positions{})
        .def("getPatternLoopCycles",
             (Result<int>(Detector::*)(int, sls::Positions)) &
                 Detector::getPatternLoopCycles,
             py::arg(), py::arg() = Positions{})
        .def("setPatternLoopCycles",
             (void (Detector::*)(int, int, sls::Positions)) &
                 Detector::setPatternLoopCycles,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getPatternWaitAddr",
             (Result<int>(Detector::*)(int, sls::Positions)) &
                 Detector::getPatternWaitAddr,
             py::arg(), py::arg() = Positions{})
        .def("setPatternWaitAddr",
             (void (Detector::*)(int, int, sls::Positions)) &
                 Detector::setPatternWaitAddr,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getPatternWaitTime",
             (Result<uint64_t>(Detector::*)(int, sls::Positions)) &
                 Detector::getPatternWaitTime,
             py::arg(), py::arg() = Positions{})
        .def("setPatternWaitTime",
             (void (Detector::*)(int, uint64_t, sls::Positions)) &
                 Detector::setPatternWaitTime,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getPatternMask",
             (Result<uint64_t>(Detector::*)(sls::Positions)) &
                 Detector::getPatternMask,
             py::arg() = Positions{})
        .def("setPatternMask",
             (void (Detector::*)(uint64_t, sls::Positions)) &
                 Detector::setPatternMask,
             py::arg(), py::arg() = Positions{})
        .def("getPatternBitMask",
             (Result<uint64_t>(Detector::*)(sls::Positions)) &
                 Detector::getPatternBitMask,
             py::arg() = Positions{})
        .def("setPatternBitMask",
             (void (Detector::*)(uint64_t, sls::Positions)) &
                 Detector::setPatternBitMask,
             py::arg(), py::arg() = Positions{})
        .def("getAdditionalJsonHeader",
             (Result<std::string>(Detector::*)(sls::Positions)) &
                 Detector::getAdditionalJsonHeader,
             py::arg() = Positions{})
        .def("setAdditionalJsonHeader",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::setAdditionalJsonHeader,
             py::arg(), py::arg() = Positions{})
        .def("getAdditionalJsonParameter",
             (Result<std::string>(Detector::*)(const std::string &,
                                               sls::Positions)) &
                 Detector::getAdditionalJsonParameter,
             py::arg(), py::arg() = Positions{})
        .def("setAdditionalJsonParameter",
             (void (Detector::*)(const std::string &, const std::string &,
                                 sls::Positions)) &
                 Detector::setAdditionalJsonParameter,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getDetectorMinMaxEnergyThreshold",
             (Result<int>(Detector::*)(const bool, sls::Positions)) &
                 Detector::getDetectorMinMaxEnergyThreshold,
             py::arg(), py::arg() = Positions{})
        .def("setDetectorMinMaxEnergyThreshold",
             (void (Detector::*)(const bool, const int, sls::Positions)) &
                 Detector::setDetectorMinMaxEnergyThreshold,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getFrameMode",
             (Result<defs::frameModeType>(Detector::*)(sls::Positions)) &
                 Detector::getFrameMode,
             py::arg() = Positions{})
        .def("setFrameMode",
             (void (Detector::*)(defs::frameModeType, sls::Positions)) &
                 Detector::setFrameMode,
             py::arg(), py::arg() = Positions{})
        .def("getDetectorMode",
             (Result<defs::detectorModeType>(Detector::*)(sls::Positions)) &
                 Detector::getDetectorMode,
             py::arg() = Positions{})
        .def("setDetectorMode",
             (void (Detector::*)(defs::detectorModeType, sls::Positions)) &
                 Detector::setDetectorMode,
             py::arg(), py::arg() = Positions{})
        .def("programFPGA",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::programFPGA,
             py::arg(), py::arg() = Positions{})
        .def("resetFPGA",
             (void (Detector::*)(sls::Positions)) & Detector::resetFPGA,
             py::arg() = Positions{})
        .def("copyDetectorServer",
             (void (Detector::*)(const std::string &, const std::string &,
                                 sls::Positions)) &
                 Detector::copyDetectorServer,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("rebootController",
             (void (Detector::*)(sls::Positions)) & Detector::rebootController,
             py::arg() = Positions{})
        .def("updateFirmwareAndServer",
             (void (Detector::*)(const std::string &, const std::string &,
                                 const std::string &, sls::Positions)) &
                 Detector::updateFirmwareAndServer,
             py::arg(), py::arg(), py::arg(), py::arg() = Positions{})
        .def("readRegister",
             (Result<uint32_t>(Detector::*)(uint32_t, sls::Positions)) &
                 Detector::readRegister,
             py::arg(), py::arg() = Positions{})
        .def("writeRegister",
             (void (Detector::*)(uint32_t, uint32_t, sls::Positions)) &
                 Detector::writeRegister,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("setBit",
             (void (Detector::*)(uint32_t, int, sls::Positions)) &
                 Detector::setBit,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("clearBit",
             (void (Detector::*)(uint32_t, int, sls::Positions)) &
                 Detector::clearBit,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("executeFirmwareTest",
             (void (Detector::*)(sls::Positions)) &
                 Detector::executeFirmwareTest,
             py::arg() = Positions{})
        .def("executeBusTest",
             (void (Detector::*)(sls::Positions)) & Detector::executeBusTest,
             py::arg() = Positions{})
        .def("writeAdcRegister",
             (void (Detector::*)(uint32_t, uint32_t, sls::Positions)) &
                 Detector::writeAdcRegister,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getInitialChecks",
             (bool (Detector::*)()) & Detector::getInitialChecks)
        .def("setInitialChecks",
             (void (Detector::*)(const bool)) & Detector::setInitialChecks,
             py::arg())
        .def("getADCInvert",
             (Result<uint32_t>(Detector::*)(sls::Positions)) &
                 Detector::getADCInvert,
             py::arg() = Positions{})
        .def("setADCInvert",
             (void (Detector::*)(uint32_t, sls::Positions)) &
                 Detector::setADCInvert,
             py::arg(), py::arg() = Positions{})
        .def("getControlPort",
             (Result<int>(Detector::*)(sls::Positions)) &
                 Detector::getControlPort,
             py::arg() = Positions{})
        .def("setControlPort",
             (void (Detector::*)(int, sls::Positions)) &
                 Detector::setControlPort,
             py::arg(), py::arg() = Positions{})
        .def("getStopPort",
             (Result<int>(Detector::*)(sls::Positions)) & Detector::getStopPort,
             py::arg() = Positions{})
        .def("setStopPort",
             (void (Detector::*)(int, sls::Positions)) & Detector::setStopPort,
             py::arg(), py::arg() = Positions{})
        .def("getDetectorLock",
             (Result<bool>(Detector::*)(sls::Positions)) &
                 Detector::getDetectorLock,
             py::arg() = Positions{})
        .def("setDetectorLock",
             (void (Detector::*)(bool, sls::Positions)) &
                 Detector::setDetectorLock,
             py::arg(), py::arg() = Positions{})
        .def("getLastClientIP",
             (Result<sls::IpAddr>(Detector::*)(sls::Positions)) &
                 Detector::getLastClientIP,
             py::arg() = Positions{})
        .def("executeCommand",
             (void (Detector::*)(const std::string &, sls::Positions)) &
                 Detector::executeCommand,
             py::arg(), py::arg() = Positions{})
        .def("getNumberOfFramesFromStart",
             (Result<int64_t>(Detector::*)(sls::Positions)) &
                 Detector::getNumberOfFramesFromStart,
             py::arg() = Positions{})
        .def("getActualTime",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getActualTime,
             py::arg() = Positions{})
        .def("getMeasurementTime",
             (Result<sls::ns>(Detector::*)(sls::Positions)) &
                 Detector::getMeasurementTime,
             py::arg() = Positions{})
        .def("getUserDetails",
             (std::string(Detector::*)()) & Detector::getUserDetails)
        .def("getRxCurrentFrameIndex",
             (Result<uint64_t>(Detector::*)(sls::Positions)) &
                 Detector::getRxCurrentFrameIndex,
             py::arg() = Positions{});
}
