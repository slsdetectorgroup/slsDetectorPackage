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
void init_experimental(py::module &m) {
    using sls::Detector;
    using sls::Positions;
    using defs = slsDetectorDefs;

    py::class_<Detector> CppDetectorApi(m, "CppDetectorApi");
    CppDetectorApi
        .def(py::init<int>())

        // Configuration
        .def("freeSharedMemory", &Detector::freeSharedMemory)
        .def("loadConfig", &Detector::loadConfig)
        .def("loadParameters", &Detector::loadParameters)
        .def("setHostname", &Detector::setHostname)
        .def("getHostname", &Detector::getHostname, py::arg() = Positions{})
        .def("getShmId", &Detector::getShmId)
        .def("getFirmwareVersion", &Detector::getFirmwareVersion,
             py::arg() = Positions{})
        .def("getDetectorServerVersion", &Detector::getDetectorServerVersion,
             py::arg() = Positions{})
        .def("getSerialNumber", &Detector::getSerialNumber,
             py::arg() = Positions{})
        .def("getClientVersion", &Detector::getClientVersion)
        .def("getReceiverVersion", &Detector::getReceiverVersion,
             py::arg() = Positions{})
        .def("getDetectorType", &Detector::getDetectorType,
             py::arg() = Positions{})
        .def("size", &Detector::size)
        .def("getModuleGeometry", &Detector::getModuleGeometry)
        .def("getModuleSize", &Detector::getModuleSize, py::arg() = Positions{})
        .def("getDetectorSize", &Detector::getDetectorSize)
        .def("setDetectorSize", &Detector::setDetectorSize)
        .def("getSettings", &Detector::getSettings, py::arg() = Positions{})
        .def("setSettings", &Detector::setSettings, py::arg(),
             py::arg() = Positions{})

        // TODO! Python funcs for callbacks?

        // Acquisition Parameters
        .def("getNumberOfFrames", &Detector::getNumberOfFrames)
        .def("setNumberOfFrames", &Detector::setNumberOfFrames)
        .def("getNumberOfTriggers", &Detector::getNumberOfTriggers)
        .def("setNumberOfTriggers", &Detector::setNumberOfTriggers)
        .def("setExptime", &Detector::setExptime, py::arg(),
             py::arg() = Positions{})
        .def("getExptime", &Detector::getExptime, py::arg() = Positions{})
        .def("setPeriod", &Detector::setPeriod, py::arg(),
             py::arg() = Positions{})
        .def("getPeriod", &Detector::getPeriod, py::arg() = Positions{})
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
        .def("getSpeed", &Detector::getSpeed, py::arg() = Positions{})
        .def("setSpeed", &Detector::setSpeed, py::arg(),
             py::arg() = Positions{})
        .def("getADCPhase", &Detector::getADCPhase, py::arg() = Positions{})

        .def("setADCPhase", &Detector::setADCPhase, py::arg(),
             py::arg() = Positions{})
        .def("getADCPhaseInDegrees", &Detector::getADCPhaseInDegrees,
             py::arg() = Positions{})
        .def("setADCPhaseInDegrees", &Detector::setADCPhaseInDegrees, py::arg(),
             py::arg() = Positions{})
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

        .def("getTimingMode", &Detector::getTimingMode, py::arg() = Positions{})

        .def("setTimingMode", &Detector::setTimingMode, py::arg(),
             py::arg() = Positions{})

        // ACQUISITION
        .def("acquire", &Detector::acquire)
        .def("startAcquisition", &Detector::startAcquisition)
        .def("stopAcquisition", &Detector::stopAcquisition)
        .def("clearAcquiringFlag", &Detector::clearAcquiringFlag)
        .def("getDetectorStatus", &Detector::getDetectorStatus,
             py::arg() = Positions{})
        .def("getReceiverStatus", &Detector::getReceiverStatus,
             py::arg() = Positions{})

        .def("getFramesCaught", &Detector::getFramesCaught,
             py::arg() = Positions{})

        .def("getStartingFrameNumber", &Detector::getStartingFrameNumber,
             py::arg() = Positions{})
        .def("setStartingFrameNumber", &Detector::setStartingFrameNumber,
             py::arg(), py::arg() = Positions{})

        .def("sendSoftwareTrigger", &Detector::sendSoftwareTrigger,
             py::arg() = Positions{})

        // Network Configuration (Detector<->Receiver)
        .def("getNumberofUDPInterfaces", &Detector::getNumberofUDPInterfaces,
             py::arg() = Positions{})

        .def("setNumberofUDPInterfaces", &Detector::setNumberofUDPInterfaces,
             py::arg(), py::arg() = Positions{})
        .def("getSelectedUDPInterface", &Detector::getSelectedUDPInterface,
             py::arg() = Positions{})

        .def("selectUDPInterface", &Detector::selectUDPInterface, py::arg(),
             py::arg() = Positions{})

        .def("getSourceUDPIP",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getSourceUDPIP())
                     res.push_back(s.str());
                 return res;
             })
        .def("setSourceUDPIP", &Detector::setSourceUDPIP, py::arg(),
             py::arg() = Positions{})

        .def("getSourceUDPIP2",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getSourceUDPIP2())
                     res.push_back(s.str());
                 return res;
             })
        .def("setSourceUDPIP2", &Detector::setSourceUDPIP2, py::arg(),
             py::arg() = Positions{})

        .def("getSourceUDPMAC",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getSourceUDPMAC())
                     res.push_back(s.str());
                 return res;
             })
        .def("setSourceUDPMAC", &Detector::setSourceUDPMAC, py::arg(),
             py::arg() = Positions{})

        .def("getSourceUDPMAC2",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getSourceUDPMAC2())
                     res.push_back(s.str());
                 return res;
             })
        .def("setSourceUDPMAC2", &Detector::setSourceUDPMAC2, py::arg(),
             py::arg() = Positions{})

        .def("getDestinationUDPIP",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getDestinationUDPIP())
                     res.push_back(s.str());
                 return res;
             })
        .def("setDestinationUDPIP", &Detector::setDestinationUDPIP, py::arg(),
             py::arg() = Positions{})

        .def("getDestinationUDPIP2",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getDestinationUDPIP2())
                     res.push_back(s.str());
                 return res;
             })
        .def("setDestinationUDPIP2", &Detector::setDestinationUDPIP2, py::arg(),
             py::arg() = Positions{})

        .def("getDestinationUDPMAC",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getDestinationUDPMAC())
                     res.push_back(s.str());
                 return res;
             })
        .def("setDestinationUDPMAC", &Detector::setDestinationUDPMAC, py::arg(),
             py::arg() = Positions{})

        .def("getDestinationUDPMAC2",
             [](const Detector &d) {
                 std::vector<std::string> res;
                 for (const auto &s : d.getDestinationUDPMAC2())
                     res.push_back(s.str());
                 return res;
             })
        .def("setDestinationUDPMAC2", &Detector::setDestinationUDPMAC2,
             py::arg(), py::arg() = Positions{})

        .def("getDestinationUDPPort", &Detector::getDestinationUDPPort,
             py::arg() = Positions{})
        .def("setDestinationUDPPort", &Detector::setDestinationUDPPort,
             py::arg(), py::arg() = Positions{})
        .def("getDestinationUDPPort2", &Detector::getDestinationUDPPort2,
             py::arg() = Positions{})
        .def("setDestinationUDPPort2", &Detector::setDestinationUDPPort2,
             py::arg(), py::arg() = Positions{})

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

        /**************************************************
         *                                                *
         *    RECEIVER CONFIG                             *
         *                                                *
         * ************************************************/

        .def("getUseReceiverFlag", &Detector::getUseReceiverFlag,
             py::arg() = Positions{})
        .def("getRxHostname", &Detector::getRxHostname, py::arg() = Positions{})
        .def("setRxHostname", &Detector::setRxHostname, py::arg(),
             py::arg() = Positions{})
        .def("getRxPort", &Detector::getRxPort, py::arg() = Positions{})
        .def("setRxPort", &Detector::setRxPort, py::arg(),
             py::arg() = Positions{})
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

        /**************************************************
         *                                                *
         *    FILE                                        *
         *                                                *
         * ************************************************/
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
        .def("getFilePath", &Detector::getFilePath)
        .def("setFilePath", &Detector::setFilePath, py::arg(),
             py::arg() = Positions{})

        .def("getAcquisitionIndex", &Detector::getAcquisitionIndex,
             py::arg() = Positions{})
        .def("setAcquisitionIndex", &Detector::setAcquisitionIndex, py::arg(),
             py::arg() = Positions{})
        .def("setFileWrite", &Detector::setFileWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFileWrite", &Detector::getFileWrite, py::arg() = Positions{})
        .def("setFileOverWrite", &Detector::setFileOverWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFileOverWrite", &Detector::getFileOverWrite,
             py::arg() = Positions{})
        .def("setMasterFileWrite", &Detector::setMasterFileWrite, py::arg(),
             py::arg() = Positions{})
        .def("getMasterFileWrite", &Detector::getMasterFileWrite,
             py::arg() = Positions{})
        .def("setFramesPerFile", &Detector::setFramesPerFile, py::arg(),
             py::arg() = Positions{})
        .def("getFramesPerFile", &Detector::getFramesPerFile,
             py::arg() = Positions{})

        /**************************************************
         *                                                *
         *    ZMQ Streaming Parameters (Receiver<->Client)*
         *                                                *
         * ************************************************/

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
        .def("setRxZmqPort", &Detector::setRxZmqPort, py::arg(),
             py::arg() = Positions{})
        .def("getRxZmqIP", &Detector::getRxZmqIP, py::arg() = Positions{})
        .def("setRxZmqIP", &Detector::setRxZmqIP, py::arg(),
             py::arg() = Positions{})
        .def("getClientZmqPort", &Detector::getClientZmqPort,
             py::arg() = Positions{})
        .def("setClientZmqPort", &Detector::setClientZmqPort, py::arg(),
             py::arg() = -1)
        .def("getClientZmqIp", &Detector::getClientZmqIp,
             py::arg() = Positions{})
        .def("setClientZmqIp", &Detector::setClientZmqIp, py::arg(),
             py::arg() = Positions{})

        /**************************************************
         *                                                *
         *    Eiger Specific                              *
         *                                                *
         * ************************************************/

        .def("getDynamicRange", &Detector::getDynamicRange,
             py::arg() = Positions{})
        .def("setDynamicRange", &Detector::setDynamicRange)
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
             py::arg() = defs::STANDARD, py::arg() = true,
             py::arg() = Positions{})
        .def("getSettingsDir", &Detector::getSettingsDir,
             py::arg() = Positions{})
        .def("setSettingsDir", &Detector::setSettingsDir, py::arg(),
             py::arg() = Positions{})
        .def("loadTrimbits", &Detector::setSettingsDir, py::arg(),
             py::arg() = Positions{})
        .def("getRxAddGapPixels", &Detector::getRxAddGapPixels,
             py::arg() = Positions{})
        .def("setRxAddGapPixels", &Detector::setRxAddGapPixels)
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
        .def("setRateCorrection", &Detector::setRateCorrection, py::arg(),
             py::arg() = Positions{})
        .def("setDefaultRateCorrection", &Detector::setDefaultRateCorrection,
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
        .def("setRxPadDeactivatedMode", &Detector::setRxPadDeactivatedMode, py::arg(),
             py::arg() = Positions{})
        .def("getPartialReset", &Detector::getPartialReset, py::arg() = Positions{})
        .def("setPartialReset", &Detector::setPartialReset, py::arg(),
             py::arg() = Positions{})

        .def("pulsePixel", &Detector::pulsePixel, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("pulsePixelNMove", &Detector::pulsePixelNMove, py::arg(),
             py::arg(), py::arg() = Positions{})
        .def("pulseChip", &Detector::pulseChip, py::arg(),
             py::arg() = Positions{})
        .def("getQuad", &Detector::getQuad, py::arg() = Positions{})
        .def("setQuad", &Detector::setQuad)

        /**************************************************
         *                                                *
         *    Jungfrau Specific                           *
         *                                                *
         * ************************************************/
        .def("getThresholdTemperature", &Detector::getThresholdTemperature, py::arg() = Positions{})
        .def("setThresholdTemperature", &Detector::setThresholdTemperature, py::arg(),
             py::arg() = Positions{})


        // Bits and registers
        .def("setBit", &Detector::setBit, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("clearBit", &Detector::clearBit, py::arg(), py::arg(),
             py::arg() = Positions{})
        .def("readRegister", &Detector::readRegister, py::arg(),
             py::arg() = Positions{})

        .def("getStartingFrameNumber", &Detector::getStartingFrameNumber,
             py::arg() = Positions{})
        .def("setStartingFrameNumber", &Detector::setStartingFrameNumber,
             py::arg(), py::arg() = Positions{})

        // File

        // Time

        .def("setSubExptime", &Detector::setSubExptime, py::arg(),
             py::arg() = Positions{})
        .def("getSubExptime", &Detector::getSubExptime,
             py::arg() = Positions{});
}
