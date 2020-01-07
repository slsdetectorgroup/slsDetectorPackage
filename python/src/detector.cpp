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
    using sls::Detector;
    using sls::Positions;

    py::class_<Detector> CppDetectorApi(m, "CppDetectorApi");
    CppDetectorApi
        .def(py::init<int>())

        .def("freeSharedMemory", &Detector::freeSharedMemory)
        .def("loadConfig", &Detector::loadConfig, py::arg())
        .def("loadParameters", &Detector::loadParameters, py::arg())
        .def("getHostname", &Detector::getHostname, py::arg() = Positions{})
        .def("setHostname", &Detector::setHostname, py::arg())
        .def("setVirtualDetectorServers", &Detector::setVirtualDetectorServers,
             py::arg(), py::arg())
        .def("getShmId", &Detector::getShmId)
        .def("getPackageVersion", &Detector::getPackageVersion)
        .def("getClientVersion", &Detector::getClientVersion)
        .def("getFirmwareVersion", &Detector::getFirmwareVersion,
             py::arg() = Positions{})
        .def("getDetectorServerVersion", &Detector::getDetectorServerVersion,
             py::arg() = Positions{})
        .def("getSerialNumber", &Detector::getSerialNumber,
             py::arg() = Positions{})
        .def("getReceiverVersion", &Detector::getReceiverVersion,
             py::arg() = Positions{})
        .def("getDetectorType", &Detector::getDetectorType,
             py::arg() = Positions{})
        .def("size", &Detector::size)
        .def("empty", &Detector::empty)
        .def("getModuleGeometry", &Detector::getModuleGeometry)
        .def("getModuleSize", &Detector::getModuleSize, py::arg() = Positions{})
        .def("getDetectorSize", &Detector::getDetectorSize)
        .def("setDetectorSize", &Detector::setDetectorSize, py::arg())
        .def("getSettings", &Detector::getSettings, py::arg() = Positions{})
        .def("setSettings", &Detector::setSettings, py::arg(),
             py::arg() = Positions{})
        .def("registerAcquisitionFinishedCallback",
             &Detector::registerAcquisitionFinishedCallback, py::arg(),
             py::arg())
        .def("registerDataCallback", &Detector::registerDataCallback, py::arg(),
             py::arg())
        .def("getNumberOfFrames", &Detector::getNumberOfFrames,
             py::arg() = Positions{})
        .def("setNumberOfFrames", &Detector::setNumberOfFrames, py::arg())
        .def("getNumberOfTriggers", &Detector::getNumberOfTriggers,
             py::arg() = Positions{})
        .def("setNumberOfTriggers", &Detector::setNumberOfTriggers, py::arg())
        .def("getExptime", &Detector::getExptime, py::arg() = Positions{})
        .def("setExptime", &Detector::setExptime, py::arg(),
             py::arg() = Positions{})
        .def("getPeriod", &Detector::getPeriod, py::arg() = Positions{})
        .def("setPeriod", &Detector::setPeriod, py::arg(),
             py::arg() = Positions{})
        .def("getDelayAfterTrigger", &Detector::getDelayAfterTrigger,
             py::arg() = Positions{})
        .def("setDelayAfterTrigger", &Detector::setDelayAfterTrigger, py::arg(),
             py::arg() = Positions{})
        .def("getNumberOfFramesLeft", &Detector::getNumberOfFramesLeft,
             py::arg() = Positions{})
        .def("getNumberOfTriggersLeft", &Detector::getNumberOfTriggersLeft,
             py::arg() = Positions{})
        .def("getDelayAfterTriggerLeft", &Detector::getDelayAfterTriggerLeft,
             py::arg() = Positions{})
        .def("getPeriodLeft", &Detector::getPeriodLeft, py::arg() = Positions{})
        .def("getSpeed", &Detector::getSpeed, py::arg() = Positions{})
        .def("setSpeed", &Detector::setSpeed, py::arg(),
             py::arg() = Positions{})
        .def("getADCPhase", &Detector::getADCPhase, py::arg() = Positions{})
        .def("setADCPhase", &Detector::setADCPhase, py::arg(),
             py::arg() = Positions{})
        .def("getMaxADCPhaseShift", &Detector::getMaxADCPhaseShift,
             py::arg() = Positions{})
        .def("getADCPhaseInDegrees", &Detector::getADCPhaseInDegrees,
             py::arg() = Positions{})
        .def("setADCPhaseInDegrees", &Detector::setADCPhaseInDegrees, py::arg(),
             py::arg() = Positions{})
        .def("getClockFrequency", &Detector::getClockFrequency, py::arg(),
             py::arg() = Positions{})
        .def("setClockFrequency", &Detector::setClockFrequency, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getClockPhase", &Detector::getClockPhase, py::arg(),
             py::arg() = Positions{})
        .def("setClockPhase", &Detector::setClockPhase, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("getMaxClockPhaseShift", &Detector::getMaxClockPhaseShift,
             py::arg(), py::arg() = Positions{})
        .def("getClockPhaseinDegrees", &Detector::getClockPhaseinDegrees,
             py::arg(), py::arg() = Positions{})
        .def("setClockPhaseinDegrees", &Detector::setClockPhaseinDegrees,
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getClockDivider", &Detector::getClockDivider, py::arg(),
             py::arg() = Positions{})
        .def("setClockDivider", &Detector::setClockDivider, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getHighVoltage", &Detector::getHighVoltage,
             py::arg() = Positions{})
        .def("setHighVoltage", &Detector::setHighVoltage, py::arg(),
             py::arg() = Positions{})
        .def("getTemperature", &Detector::getTemperature, py::arg(),
             py::arg() = Positions{})
        .def("getDAC", &Detector::getDAC, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("setDAC", &Detector::setDAC, py::arg(), py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("getOnChipDAC", &Detector::getOnChipDAC, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("setOnChipDAC", &Detector::setOnChipDAC, py::arg(), py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getTimingMode", &Detector::getTimingMode, py::arg() = Positions{})
        .def("setTimingMode", &Detector::setTimingMode, py::arg(),
             py::arg() = Positions{})
        .def("acquire", &Detector::acquire)
        .def("clearAcquiringFlag", &Detector::clearAcquiringFlag)
        .def("startReceiver", &Detector::startReceiver)
        .def("stopReceiver", &Detector::stopReceiver)
        .def("startDetector", &Detector::startDetector)
        .def("stopDetector", &Detector::stopDetector)
        .def("getDetectorStatus", &Detector::getDetectorStatus,
             py::arg() = Positions{})
        .def("getReceiverStatus", &Detector::getReceiverStatus,
             py::arg() = Positions{})
        .def("getFramesCaught", &Detector::getFramesCaught,
             py::arg() = Positions{})
        .def("getNumMissingPackets", &Detector::getNumMissingPackets,
             py::arg() = Positions{})
        .def("getStartingFrameNumber", &Detector::getStartingFrameNumber,
             py::arg() = Positions{})
        .def("setStartingFrameNumber", &Detector::setStartingFrameNumber,
             py::arg(), py::arg() = Positions{})
        .def("sendSoftwareTrigger", &Detector::sendSoftwareTrigger,
             py::arg() = Positions{})
        .def("getNumberofUDPInterfaces", &Detector::getNumberofUDPInterfaces,
             py::arg() = Positions{})
        .def("setNumberofUDPInterfaces", &Detector::setNumberofUDPInterfaces,
             py::arg(), py::arg() = Positions{})
        .def("getSelectedUDPInterface", &Detector::getSelectedUDPInterface,
             py::arg() = Positions{})
        .def("selectUDPInterface", &Detector::selectUDPInterface, py::arg(),
             py::arg() = Positions{})
        .def("getSourceUDPIP", &Detector::getSourceUDPIP,
             py::arg() = Positions{})
        .def("setSourceUDPIP", &Detector::setSourceUDPIP, py::arg(),
             py::arg() = Positions{})
        .def("getSourceUDPIP2", &Detector::getSourceUDPIP2,
             py::arg() = Positions{})
        .def("setSourceUDPIP2", &Detector::setSourceUDPIP2, py::arg(),
             py::arg() = Positions{})
        .def("getSourceUDPMAC", &Detector::getSourceUDPMAC,
             py::arg() = Positions{})
        .def("setSourceUDPMAC", &Detector::setSourceUDPMAC, py::arg(),
             py::arg() = Positions{})
        .def("getSourceUDPMAC2", &Detector::getSourceUDPMAC2,
             py::arg() = Positions{})
        .def("setSourceUDPMAC2", &Detector::setSourceUDPMAC2, py::arg(),
             py::arg() = Positions{})
        .def("getDestinationUDPIP", &Detector::getDestinationUDPIP,
             py::arg() = Positions{})
        .def("setDestinationUDPIP", &Detector::setDestinationUDPIP, py::arg(),
             py::arg() = Positions{})
        .def("getDestinationUDPIP2", &Detector::getDestinationUDPIP2,
             py::arg() = Positions{})
        .def("setDestinationUDPIP2", &Detector::setDestinationUDPIP2, py::arg(),
             py::arg() = Positions{})
        .def("getDestinationUDPMAC", &Detector::getDestinationUDPMAC,
             py::arg() = Positions{})
        .def("setDestinationUDPMAC", &Detector::setDestinationUDPMAC, py::arg(),
             py::arg() = Positions{})
        .def("getDestinationUDPMAC2", &Detector::getDestinationUDPMAC2,
             py::arg() = Positions{})
        .def("setDestinationUDPMAC2", &Detector::setDestinationUDPMAC2,
             py::arg(), py::arg() = Positions{})
        .def("getDestinationUDPPort", &Detector::getDestinationUDPPort,
             py::arg() = Positions{})
        .def("setDestinationUDPPort", &Detector::setDestinationUDPPort,
             py::arg(), py::arg())
        .def("getDestinationUDPPort2", &Detector::getDestinationUDPPort2,
             py::arg() = Positions{})
        .def("setDestinationUDPPort2", &Detector::setDestinationUDPPort2,
             py::arg(), py::arg())
        .def("printRxConfiguration", &Detector::printRxConfiguration,
             py::arg() = Positions{})
        .def("getTenGiga", &Detector::getTenGiga, py::arg() = Positions{})
        .def("setTenGiga", &Detector::setTenGiga, py::arg(),
             py::arg() = Positions{})
        .def("getTenGigaFlowControl", &Detector::getTenGigaFlowControl,
             py::arg() = Positions{})
        .def("setTenGigaFlowControl", &Detector::setTenGigaFlowControl,
             py::arg(), py::arg() = Positions{})
        .def("getTransmissionDelayFrame", &Detector::getTransmissionDelayFrame,
             py::arg() = Positions{})
        .def("setTransmissionDelayFrame", &Detector::setTransmissionDelayFrame,
             py::arg(), py::arg() = Positions{})
        .def("getTransmissionDelayLeft", &Detector::getTransmissionDelayLeft,
             py::arg() = Positions{})
        .def("setTransmissionDelayLeft", &Detector::setTransmissionDelayLeft,
             py::arg(), py::arg() = Positions{})
        .def("getTransmissionDelayRight", &Detector::getTransmissionDelayRight,
             py::arg() = Positions{})
        .def("setTransmissionDelayRight", &Detector::setTransmissionDelayRight,
             py::arg(), py::arg() = Positions{})
        .def("getUseReceiverFlag", &Detector::getUseReceiverFlag,
             py::arg() = Positions{})
        .def("getRxHostname", &Detector::getRxHostname, py::arg() = Positions{})
        .def("setRxHostname", &Detector::setRxHostname, py::arg(),
             py::arg() = Positions{})
        .def("getRxPort", &Detector::getRxPort, py::arg() = Positions{})
        .def("setRxPort", &Detector::setRxPort, py::arg(), py::arg())
        .def("getRxFifoDepth", &Detector::getRxFifoDepth,
             py::arg() = Positions{})
        .def("setRxFifoDepth", &Detector::setRxFifoDepth, py::arg(),
             py::arg() = Positions{})
        .def("getRxSilentMode", &Detector::getRxSilentMode,
             py::arg() = Positions{})
        .def("setRxSilentMode", &Detector::setRxSilentMode, py::arg(),
             py::arg() = Positions{})
        .def("getRxFrameDiscardPolicy", &Detector::getRxFrameDiscardPolicy,
             py::arg() = Positions{})
        .def("setRxFrameDiscardPolicy", &Detector::setRxFrameDiscardPolicy,
             py::arg(), py::arg() = Positions{})
        .def("getPartialFramesPadding", &Detector::getPartialFramesPadding,
             py::arg() = Positions{})
        .def("setPartialFramesPadding", &Detector::setPartialFramesPadding,
             py::arg(), py::arg() = Positions{})
        .def("getRxUDPSocketBufferSize", &Detector::getRxUDPSocketBufferSize,
             py::arg() = Positions{})
        .def("setRxUDPSocketBufferSize", &Detector::setRxUDPSocketBufferSize,
             py::arg(), py::arg() = Positions{})
        .def("getRxRealUDPSocketBufferSize",
             &Detector::getRxRealUDPSocketBufferSize, py::arg() = Positions{})
        .def("getRxLock", &Detector::getRxLock, py::arg() = Positions{})
        .def("setRxLock", &Detector::setRxLock, py::arg(),
             py::arg() = Positions{})
        .def("getRxLastClientIP", &Detector::getRxLastClientIP,
             py::arg() = Positions{})
        .def("getFileFormat", &Detector::getFileFormat, py::arg() = Positions{})
        .def("setFileFormat", &Detector::setFileFormat, py::arg(),
             py::arg() = Positions{})
        .def("getFilePath", &Detector::getFilePath, py::arg() = Positions{})
        .def("setFilePath", &Detector::setFilePath, py::arg(),
             py::arg() = Positions{})
        .def("getFileNamePrefix", &Detector::getFileNamePrefix,
             py::arg() = Positions{})
        .def("setFileNamePrefix", &Detector::setFileNamePrefix, py::arg(),
             py::arg() = Positions{})
        .def("getAcquisitionIndex", &Detector::getAcquisitionIndex,
             py::arg() = Positions{})
        .def("setAcquisitionIndex", &Detector::setAcquisitionIndex, py::arg(),
             py::arg() = Positions{})
        .def("getFileWrite", &Detector::getFileWrite, py::arg() = Positions{})
        .def("setFileWrite", &Detector::setFileWrite, py::arg(),
             py::arg() = Positions{})
        .def("getMasterFileWrite", &Detector::getMasterFileWrite,
             py::arg() = Positions{})
        .def("setMasterFileWrite", &Detector::setMasterFileWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFileOverWrite", &Detector::getFileOverWrite,
             py::arg() = Positions{})
        .def("setFileOverWrite", &Detector::setFileOverWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFramesPerFile", &Detector::getFramesPerFile,
             py::arg() = Positions{})
        .def("setFramesPerFile", &Detector::setFramesPerFile, py::arg(),
             py::arg() = Positions{})
        .def("getRxZmqDataStream", &Detector::getRxZmqDataStream,
             py::arg() = Positions{})
        .def("setRxZmqDataStream", &Detector::setRxZmqDataStream, py::arg(),
             py::arg() = Positions{})
        .def("getRxZmqFrequency", &Detector::getRxZmqFrequency,
             py::arg() = Positions{})
        .def("setRxZmqFrequency", &Detector::setRxZmqFrequency, py::arg(),
             py::arg() = Positions{})
        .def("getRxZmqTimer", &Detector::getRxZmqTimer, py::arg() = Positions{})
        .def("setRxZmqTimer", &Detector::setRxZmqTimer, py::arg(),
             py::arg() = Positions{})
        .def("getRxZmqPort", &Detector::getRxZmqPort, py::arg() = Positions{})
        .def("setRxZmqPort", &Detector::setRxZmqPort, py::arg(), py::arg())
        .def("getRxZmqIP", &Detector::getRxZmqIP, py::arg() = Positions{})
        .def("setRxZmqIP", &Detector::setRxZmqIP, py::arg(),
             py::arg() = Positions{})
        .def("getClientZmqPort", &Detector::getClientZmqPort,
             py::arg() = Positions{})
        .def("setClientZmqPort", &Detector::setClientZmqPort, py::arg(),
             py::arg())
        .def("getClientZmqIp", &Detector::getClientZmqIp,
             py::arg() = Positions{})
        .def("setClientZmqIp", &Detector::setClientZmqIp, py::arg(),
             py::arg() = Positions{})
        .def("getDynamicRange", &Detector::getDynamicRange,
             py::arg() = Positions{})
        .def("setDynamicRange", &Detector::setDynamicRange, py::arg())
        .def("getSubExptime", &Detector::getSubExptime, py::arg() = Positions{})
        .def("setSubExptime", &Detector::setSubExptime, py::arg(),
             py::arg() = Positions{})
        .def("getSubDeadTime", &Detector::getSubDeadTime,
             py::arg() = Positions{})
        .def("setSubDeadTime", &Detector::setSubDeadTime, py::arg(),
             py::arg() = Positions{})
        .def("getThresholdEnergy", &Detector::getThresholdEnergy,
             py::arg() = Positions{})
        .def("setThresholdEnergy", &Detector::setThresholdEnergy, py::arg(),
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("getSettingsPath", &Detector::getSettingsPath,
             py::arg() = Positions{})
        .def("setSettingsPath", &Detector::setSettingsPath, py::arg(),
             py::arg() = Positions{})
        .def("loadTrimbits", &Detector::loadTrimbits, py::arg(),
             py::arg() = Positions{})
        .def("getRxAddGapPixels", &Detector::getRxAddGapPixels,
             py::arg() = Positions{})
        .def("setRxAddGapPixels", &Detector::setRxAddGapPixels, py::arg())
        .def("getParallelMode", &Detector::getParallelMode,
             py::arg() = Positions{})
        .def("setParallelMode", &Detector::setParallelMode, py::arg(),
             py::arg() = Positions{})
        .def("getOverFlowMode", &Detector::getOverFlowMode,
             py::arg() = Positions{})
        .def("setOverFlowMode", &Detector::setOverFlowMode, py::arg(),
             py::arg() = Positions{})
        .def("getStoreInRamMode", &Detector::getStoreInRamMode,
             py::arg() = Positions{})
        .def("setStoreInRamMode", &Detector::setStoreInRamMode, py::arg(),
             py::arg() = Positions{})
        .def("getBottom", &Detector::getBottom, py::arg() = Positions{})
        .def("setBottom", &Detector::setBottom, py::arg(),
             py::arg() = Positions{})
        .def("getAllTrimbits", &Detector::getAllTrimbits,
             py::arg() = Positions{})
        .def("setAllTrimbits", &Detector::setAllTrimbits, py::arg(),
             py::arg() = Positions{})
        .def("getTrimEnergies", &Detector::getTrimEnergies,
             py::arg() = Positions{})
        .def("setTrimEnergies", &Detector::setTrimEnergies, py::arg(),
             py::arg() = Positions{})
        .def("getRateCorrection", &Detector::getRateCorrection,
             py::arg() = Positions{})
        .def("setDefaultRateCorrection", &Detector::setDefaultRateCorrection,
             py::arg() = Positions{})
        .def("setRateCorrection", &Detector::setRateCorrection, py::arg(),
             py::arg() = Positions{})
        .def("getPartialReadout", &Detector::getPartialReadout,
             py::arg() = Positions{})
        .def("setPartialReadout", &Detector::setPartialReadout, py::arg(),
             py::arg() = Positions{})
        .def("getInterruptSubframe", &Detector::getInterruptSubframe,
             py::arg() = Positions{})
        .def("setInterruptSubframe", &Detector::setInterruptSubframe, py::arg(),
             py::arg() = Positions{})
        .def("getMeasuredPeriod", &Detector::getMeasuredPeriod,
             py::arg() = Positions{})
        .def("getMeasuredSubFramePeriod", &Detector::getMeasuredSubFramePeriod,
             py::arg() = Positions{})
        .def("getActive", &Detector::getActive, py::arg() = Positions{})
        .def("setActive", &Detector::setActive, py::arg(),
             py::arg() = Positions{})
        .def("getRxPadDeactivatedMode", &Detector::getRxPadDeactivatedMode,
             py::arg() = Positions{})
        .def("setRxPadDeactivatedMode", &Detector::setRxPadDeactivatedMode,
             py::arg(), py::arg() = Positions{})
        .def("getPartialReset", &Detector::getPartialReset,
             py::arg() = Positions{})
        .def("setPartialReset", &Detector::setPartialReset, py::arg(),
             py::arg() = Positions{})
        .def("pulsePixel", &Detector::pulsePixel, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("pulsePixelNMove", &Detector::pulsePixelNMove, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("pulseChip", &Detector::pulseChip, py::arg(),
             py::arg() = Positions{})
        .def("getQuad", &Detector::getQuad, py::arg() = Positions{})
        .def("setQuad", &Detector::setQuad, py::arg())
        .def("getThresholdTemperature", &Detector::getThresholdTemperature,
             py::arg() = Positions{})
        .def("setThresholdTemperature", &Detector::setThresholdTemperature,
             py::arg(), py::arg() = Positions{})
        .def("getTemperatureControl", &Detector::getTemperatureControl,
             py::arg() = Positions{})
        .def("setTemperatureControl", &Detector::setTemperatureControl,
             py::arg(), py::arg() = Positions{})
        .def("getTemperatureEvent", &Detector::getTemperatureEvent,
             py::arg() = Positions{})
        .def("resetTemperatureEvent", &Detector::resetTemperatureEvent,
             py::arg() = Positions{})
        .def("getPowerChip", &Detector::getPowerChip, py::arg() = Positions{})
        .def("setPowerChip", &Detector::setPowerChip, py::arg(),
             py::arg() = Positions{})
        .def("getAutoCompDisable", &Detector::getAutoCompDisable,
             py::arg() = Positions{})
        .def("setAutoCompDisable", &Detector::setAutoCompDisable, py::arg(),
             py::arg() = Positions{})
        .def("getNumberOfAdditionalStorageCells",
             &Detector::getNumberOfAdditionalStorageCells,
             py::arg() = Positions{})
        .def("setNumberOfAdditionalStorageCells",
             &Detector::setNumberOfAdditionalStorageCells, py::arg())
        .def("getStorageCellStart", &Detector::getStorageCellStart,
             py::arg() = Positions{})
        .def("setStoragecellStart", &Detector::setStoragecellStart, py::arg(),
             py::arg() = Positions{})
        .def("getStorageCellDelay", &Detector::getStorageCellDelay,
             py::arg() = Positions{})
        .def("setStorageCellDelay", &Detector::setStorageCellDelay, py::arg(),
             py::arg() = Positions{})
        .def("getROI", &Detector::getROI, py::arg() = Positions{})
        .def("setROI", &Detector::setROI, py::arg(), py::arg())
        .def("clearROI", &Detector::clearROI, py::arg() = Positions{})
        .def("getExptimeLeft", &Detector::getExptimeLeft,
             py::arg() = Positions{})
        .def("getExternalSignalFlags", &Detector::getExternalSignalFlags,
             py::arg() = Positions{})
        .def("setExternalSignalFlags", &Detector::setExternalSignalFlags,
             py::arg(), py::arg() = Positions{})
        .def("getImageTestMode", &Detector::getImageTestMode,
             py::arg() = Positions{})
        .def("setImageTestMode", &Detector::setImageTestMode, py::arg(),
             py::arg() = Positions{})
        .def("getInjectChannel", &Detector::getInjectChannel,
             py::arg() = Positions{})
        .def("setInjectChannel", &Detector::setInjectChannel, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getVetoPhoton", &Detector::getVetoPhoton, py::arg(),
             py::arg() = Positions{})
        .def("setVetoPhoton", &Detector::setVetoPhoton, py::arg(), py::arg(),
             py::arg(), py::arg(), py::arg() = Positions{})
        .def("setVetoReference", &Detector::setVetoReference, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("setBurstMode", &Detector::setBurstMode, py::arg(),
             py::arg() = Positions{})
        .def("getBurstMode", &Detector::getBurstMode, py::arg() = Positions{})
        .def("getNumberOfAnalogSamples", &Detector::getNumberOfAnalogSamples,
             py::arg() = Positions{})
        .def("setNumberOfAnalogSamples", &Detector::setNumberOfAnalogSamples,
             py::arg(), py::arg() = Positions{})
        .def("getNumberOfDigitalSamples", &Detector::getNumberOfDigitalSamples,
             py::arg() = Positions{})
        .def("setNumberOfDigitalSamples", &Detector::setNumberOfDigitalSamples,
             py::arg(), py::arg() = Positions{})
        .def("getReadoutMode", &Detector::getReadoutMode,
             py::arg() = Positions{})
        .def("setReadoutMode", &Detector::setReadoutMode, py::arg(),
             py::arg() = Positions{})
        .def("getDBITPhase", &Detector::getDBITPhase, py::arg() = Positions{})
        .def("setDBITPhase", &Detector::setDBITPhase, py::arg(),
             py::arg() = Positions{})
        .def("getMaxDBITPhaseShift", &Detector::getMaxDBITPhaseShift,
             py::arg() = Positions{})
        .def("getDBITPhaseInDegrees", &Detector::getDBITPhaseInDegrees,
             py::arg() = Positions{})
        .def("setDBITPhaseInDegrees", &Detector::setDBITPhaseInDegrees,
             py::arg(), py::arg() = Positions{})
        .def("getADCClock", &Detector::getADCClock, py::arg() = Positions{})
        .def("setADCClock", &Detector::setADCClock, py::arg(),
             py::arg() = Positions{})
        .def("getDBITClock", &Detector::getDBITClock, py::arg() = Positions{})
        .def("setDBITClock", &Detector::setDBITClock, py::arg(),
             py::arg() = Positions{})
        .def("getRUNClock", &Detector::getRUNClock, py::arg() = Positions{})
        .def("setRUNClock", &Detector::setRUNClock, py::arg(),
             py::arg() = Positions{})
        .def("getSYNCClock", &Detector::getSYNCClock, py::arg() = Positions{})
        .def("getADCPipeline", &Detector::getADCPipeline,
             py::arg() = Positions{})
        .def("setADCPipeline", &Detector::setADCPipeline, py::arg(),
             py::arg() = Positions{})
        .def("getDBITPipeline", &Detector::getDBITPipeline,
             py::arg() = Positions{})
        .def("setDBITPipeline", &Detector::setDBITPipeline, py::arg(),
             py::arg() = Positions{})
        .def("getVoltage", &Detector::getVoltage, py::arg(),
             py::arg() = Positions{})
        .def("setVoltage", &Detector::setVoltage, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("getMeasuredVoltage", &Detector::getMeasuredVoltage, py::arg(),
             py::arg() = Positions{})
        .def("getMeasuredCurrent", &Detector::getMeasuredCurrent, py::arg(),
             py::arg() = Positions{})
        .def("getSlowADC", &Detector::getSlowADC, py::arg(),
             py::arg() = Positions{})
        .def("getADCEnableMask", &Detector::getADCEnableMask,
             py::arg() = Positions{})
        .def("setADCEnableMask", &Detector::setADCEnableMask, py::arg(),
             py::arg() = Positions{})
        .def("getTenGigaADCEnableMask", &Detector::getTenGigaADCEnableMask,
             py::arg() = Positions{})
        .def("setTenGigaADCEnableMask", &Detector::setTenGigaADCEnableMask,
             py::arg(), py::arg() = Positions{})
        .def("getADCInvert", &Detector::getADCInvert, py::arg() = Positions{})
        .def("setADCInvert", &Detector::setADCInvert, py::arg(),
             py::arg() = Positions{})
        .def("getExternalSamplingSource", &Detector::getExternalSamplingSource,
             py::arg() = Positions{})
        .def("setExternalSamplingSource", &Detector::setExternalSamplingSource,
             py::arg(), py::arg() = Positions{})
        .def("getExternalSampling", &Detector::getExternalSampling,
             py::arg() = Positions{})
        .def("setExternalSampling", &Detector::setExternalSampling, py::arg(),
             py::arg() = Positions{})
        .def("getRxDbitList", &Detector::getRxDbitList, py::arg() = Positions{})
        .def("setRxDbitList", &Detector::setRxDbitList, py::arg(),
             py::arg() = Positions{})
        .def("getRxDbitOffset", &Detector::getRxDbitOffset,
             py::arg() = Positions{})
        .def("setRxDbitOffset", &Detector::setRxDbitOffset, py::arg(),
             py::arg() = Positions{})
        .def("setDigitalIODelay", &Detector::setDigitalIODelay, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getLEDEnable", &Detector::getLEDEnable, py::arg() = Positions{})
        .def("setLEDEnable", &Detector::setLEDEnable, py::arg(),
             py::arg() = Positions{})
        .def("setPattern", &Detector::setPattern, py::arg(),
             py::arg() = Positions{})
        .def("savePattern", &Detector::savePattern, py::arg())
        .def("getPatternIOControl", &Detector::getPatternIOControl,
             py::arg() = Positions{})
        .def("setPatternIOControl", &Detector::setPatternIOControl, py::arg(),
             py::arg() = Positions{})
        .def("getPatternClockControl", &Detector::getPatternClockControl,
             py::arg() = Positions{})
        .def("setPatternClockControl", &Detector::setPatternClockControl,
             py::arg(), py::arg() = Positions{})
        .def("getPatternWord", &Detector::getPatternWord, py::arg(),
             py::arg() = Positions{})
        .def("setPatternWord", &Detector::setPatternWord, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("getPatternLoopAddresses", &Detector::getPatternLoopAddresses,
             py::arg(), py::arg() = Positions{})
        .def("setPatternLoopAddresses", &Detector::setPatternLoopAddresses,
             py::arg(), py::arg(), py::arg(), py::arg() = Positions{})
        .def("getPatternLoopCycles", &Detector::getPatternLoopCycles, py::arg(),
             py::arg() = Positions{})
        .def("setPatternLoopCycles", &Detector::setPatternLoopCycles, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getPatternWaitAddr", &Detector::getPatternWaitAddr, py::arg(),
             py::arg() = Positions{})
        .def("setPatternWaitAddr", &Detector::setPatternWaitAddr, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getPatternWaitTime", &Detector::getPatternWaitTime, py::arg(),
             py::arg() = Positions{})
        .def("setPatternWaitTime", &Detector::setPatternWaitTime, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getPatternMask", &Detector::getPatternMask,
             py::arg() = Positions{})
        .def("setPatternMask", &Detector::setPatternMask, py::arg(),
             py::arg() = Positions{})
        .def("getPatternBitMask", &Detector::getPatternBitMask,
             py::arg() = Positions{})
        .def("setPatternBitMask", &Detector::setPatternBitMask, py::arg(),
             py::arg() = Positions{})
        .def("getAdditionalJsonHeader", &Detector::getAdditionalJsonHeader,
             py::arg() = Positions{})
        .def("setAdditionalJsonHeader", &Detector::setAdditionalJsonHeader,
             py::arg(), py::arg() = Positions{})
        .def("getAdditionalJsonParameter",
             &Detector::getAdditionalJsonParameter, py::arg(),
             py::arg() = Positions{})
        .def("setAdditionalJsonParameter",
             &Detector::setAdditionalJsonParameter, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("getDetectorMinMaxEnergyThreshold",
             &Detector::getDetectorMinMaxEnergyThreshold, py::arg(),
             py::arg() = Positions{})
        .def("setDetectorMinMaxEnergyThreshold",
             &Detector::setDetectorMinMaxEnergyThreshold, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("getFrameMode", &Detector::getFrameMode, py::arg() = Positions{})
        .def("setFrameMode", &Detector::setFrameMode, py::arg(),
             py::arg() = Positions{})
        .def("getDetectorMode", &Detector::getDetectorMode,
             py::arg() = Positions{})
        .def("setDetectorMode", &Detector::setDetectorMode, py::arg(),
             py::arg() = Positions{})
        .def("programFPGA", &Detector::programFPGA, py::arg(),
             py::arg() = Positions{})
        .def("resetFPGA", &Detector::resetFPGA, py::arg() = Positions{})
        .def("copyDetectorServer", &Detector::copyDetectorServer, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("rebootController", &Detector::rebootController,
             py::arg() = Positions{})
        .def("updateFirmwareAndServer", &Detector::updateFirmwareAndServer,
             py::arg(), py::arg(), py::arg(), py::arg() = Positions{})
        .def("readRegister", &Detector::readRegister, py::arg(),
             py::arg() = Positions{})
        .def("writeRegister", &Detector::writeRegister, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("setBit", &Detector::setBit, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("clearBit", &Detector::clearBit, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("executeFirmwareTest", &Detector::executeFirmwareTest,
             py::arg() = Positions{})
        .def("executeBusTest", &Detector::executeBusTest,
             py::arg() = Positions{})
        .def("writeAdcRegister", &Detector::writeAdcRegister, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("getControlPort", &Detector::getControlPort,
             py::arg() = Positions{})
        .def("setControlPort", &Detector::setControlPort, py::arg(),
             py::arg() = Positions{})
        .def("getStopPort", &Detector::getStopPort, py::arg() = Positions{})
        .def("setStopPort", &Detector::setStopPort, py::arg(),
             py::arg() = Positions{})
        .def("getDetectorLock", &Detector::getDetectorLock,
             py::arg() = Positions{})
        .def("setDetectorLock", &Detector::setDetectorLock, py::arg(),
             py::arg() = Positions{})
        .def("getLastClientIP", &Detector::getLastClientIP,
             py::arg() = Positions{})
        .def("executeCommand", &Detector::executeCommand, py::arg(),
             py::arg() = Positions{})
        .def("getNumberOfFramesFromStart",
             &Detector::getNumberOfFramesFromStart, py::arg() = Positions{})
        .def("getActualTime", &Detector::getActualTime, py::arg() = Positions{})
        .def("getMeasurementTime", &Detector::getMeasurementTime,
             py::arg() = Positions{})
        .def("getUserDetails", &Detector::getUserDetails)
        .def("getRxCurrentFrameIndex", &Detector::getRxCurrentFrameIndex,
             py::arg() = Positions{});
}
