#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"
#include "DetectorPythonInterface.h"
#include "mythenFileIO.h"
#include <chrono>
#include <vector>

using ds = std::chrono::duration<double>;

namespace py = pybind11;

PYBIND11_MODULE(_sls_detector, m) {
    m.doc() = R"pbdoc(
        C/C++ API
        -----------------------
        .. warning ::

            This is the compiled c extension. You probably want to look at the
            interface provided by sls instead.

    )pbdoc";

    py::class_<DetectorPythonInterface> DetectorApi(m, "DetectorApi", R"pbdoc(
    Interface to the multiSlsDetector class through Detector.h These functions
    are used by the python classes Eiger and Jungfrau and normally it is better
    to use them than to directly access functions here.

    However it is possible to access these functions...

    ::

        #Using the python class
        from sls_detector import Eiger
        d = Eiger()
        d._api.getThresholdEnergy()

        from _sls_detector import DetectorApi
        api = DetectorApi(0)
        api.getNumberOfFrames()

        #But the Pythonic way is almost alway simpler
        d = Eiger()
        d.n_frames
        >> 10

    )pbdoc");
    DetectorApi.def(py::init<int>())
        .def("freeSharedMemory", &DetectorPythonInterface::freeSharedMemory)
        .def("getMultiDetectorId", &DetectorPythonInterface::getMultiDetectorId)
        .def("acq", &DetectorPythonInterface::acquire)
        .def("getAcquiringFlag", &DetectorPythonInterface::getAcquiringFlag)
        .def("setAcquiringFlag", &DetectorPythonInterface::setAcquiringFlag)

        .def("setAllTrimbits", &DetectorPythonInterface::setAllTrimbits)
        .def("getAllTrimbits", &DetectorPythonInterface::getAllTrimbits)
        .def("setCounterBit", &DetectorPythonInterface::setCounterBit)
        .def("getCounterBit", &DetectorPythonInterface::getCounterBit)

        .def("getAdc", &DetectorPythonInterface::getAdc)
        .def("getDac", &DetectorPythonInterface::getDac)
        .def("getDac_mV", &DetectorPythonInterface::getDac_mV)
        .def("setDac", &DetectorPythonInterface::setDac)
        .def("setDac_mV", &DetectorPythonInterface::setDac_mV)
        .def("getDacFromIndex", &DetectorPythonInterface::getDacFromIndex)
        .def("setDacFromIndex", &DetectorPythonInterface::setDacFromIndex)

        .def("getDbitPipeline", &DetectorPythonInterface::getDbitPipeline)
        .def("setDbitPipeline", &DetectorPythonInterface::setDbitPipeline)
        .def("getDbitPhase", &DetectorPythonInterface::getDbitPhase)
        .def("setDbitPhase", &DetectorPythonInterface::setDbitPhase)
        .def("getDbitClock", &DetectorPythonInterface::getDbitClock)
        .def("setDbitClock", &DetectorPythonInterface::setDbitClock)

        .def("setThresholdEnergy", &DetectorPythonInterface::setThresholdEnergy)
        .def("getThresholdEnergy", &DetectorPythonInterface::getThresholdEnergy)

        .def("getSettings", &DetectorPythonInterface::getSettings)
        .def("setSettings", &DetectorPythonInterface::setSettings)
        .def("getSettingsDir", &DetectorPythonInterface::getSettingsDir)
        .def("setSettingsDir", &DetectorPythonInterface::setSettingsDir)

        .def("loadTrimbitFile", &DetectorPythonInterface::loadTrimbitFile)
        .def("setTrimEnergies", &DetectorPythonInterface::setTrimEnergies)
        .def("getTrimEnergies", &DetectorPythonInterface::getTrimEnergies)

        .def("pulseChip", &DetectorPythonInterface::pulseChip)
        .def("pulseAllPixels", &DetectorPythonInterface::pulseAllPixels)
        .def("pulseDiagonal", &DetectorPythonInterface::pulseDiagonal)
        .def("getRunStatus", &DetectorPythonInterface::getRunStatus)
        .def("readConfigurationFile",
             &DetectorPythonInterface::readConfigurationFile)
        .def("readParametersFile", &DetectorPythonInterface::readParametersFile)
        .def("checkOnline", &DetectorPythonInterface::checkOnline)
        .def("setReadoutClockSpeed",
             &DetectorPythonInterface::setReadoutClockSpeed)
        .def("getReadoutClockSpeed",
             &DetectorPythonInterface::getReadoutClockSpeed)
        .def("getSyncClkSpeed", &DetectorPythonInterface::getSyncClkSpeed)
        .def("getHostname", &DetectorPythonInterface::getHostname)
        .def("setHostname", &DetectorPythonInterface::setHostname)

        .def("getOnline", &DetectorPythonInterface::getOnline)
        .def("setOnline", &DetectorPythonInterface::setOnline)
        .def("getReceiverOnline", &DetectorPythonInterface::getReceiverOnline)
        .def("setReceiverOnline", &DetectorPythonInterface::setReceiverOnline)

        .def("getReceiverPort", &DetectorPythonInterface::getReceiverPort)
        .def("setReceiverPort", &DetectorPythonInterface::setReceiverPort)

        .def("isChipPowered", &DetectorPythonInterface::isChipPowered)
        .def("powerChip", &DetectorPythonInterface::powerChip)

        .def("readRegister", &DetectorPythonInterface::readRegister)
        .def("writeRegister", &DetectorPythonInterface::writeRegister)
        .def("writeAdcRegister", &DetectorPythonInterface::writeAdcRegister)
        .def("setBitInRegister", &DetectorPythonInterface::setBitInRegister)
        .def("clearBitInRegister", &DetectorPythonInterface::clearBitInRegister)

        .def("setDynamicRange", &DetectorPythonInterface::setDynamicRange)
        .def("getDynamicRange", &DetectorPythonInterface::getDynamicRange)
        .def("getFirmwareVersion", &DetectorPythonInterface::getFirmwareVersion)
        .def("getServerVersion", &DetectorPythonInterface::getServerVersion)
        .def("getClientVersion", &DetectorPythonInterface::getClientVersion)
        .def("getReceiverVersion", &DetectorPythonInterface::getReceiverVersion)
        .def("getDetectorNumber", &DetectorPythonInterface::getDetectorNumber)
        .def("getRateCorrection", &DetectorPythonInterface::getRateCorrection)
        .def("setRateCorrection", &DetectorPythonInterface::setRateCorrection)

        .def("startAcquisition", &DetectorPythonInterface::startAcquisition)
        .def("stopAcquisition", &DetectorPythonInterface::stopAcquisition)
        .def("startReceiver", &DetectorPythonInterface::startReceiver)
        .def("stopReceiver", &DetectorPythonInterface::stopReceiver)

        .def("getFilePath",
             (std::string(DetectorPythonInterface::*)()) &
                 DetectorPythonInterface::getFilePath,
             "Using multiSlsDetector")
        .def("getFilePath",
             (std::string(DetectorPythonInterface::*)(int)) &
                 DetectorPythonInterface::getFilePath,
             "File path for individual detector")
        .def("setFilePath", (void (DetectorPythonInterface::*)(std::string)) &
                                DetectorPythonInterface::setFilePath)
        .def("setFilePath",
             (void (Detector::*)(std::string)) & Detector::setFilePath)
        .def("setFilePath",
             (void (Detector::*)(std::string, int)) & Detector::setFilePath)

        .def("setFileName", &Detector::setFileName)
        .def("getFileName", &Detector::getFileName)
        .def("setFileIndex", &Detector::setFileIndex)
        .def("getFileIndex", &Detector::getFileIndex)

        .def("setExposureTime", &Detector::setExposureTime)
        .def("getExposureTime", &Detector::getExposureTime)
        .def("setSubExposureTime", &Detector::setSubExposureTime)
        .def("getSubExposureTime", &Detector::getSubExposureTime)
        .def("setPeriod", &Detector::setPeriod)
        .def("getPeriod", &Detector::getPeriod)
        .def("setSubExposureDeadTime", &Detector::setSubExposureDeadTime)
        .def("getSubExposureDeadTime", &Detector::getSubExposureDeadTime)

        .def("getCycles", &Detector::getCycles)
        .def("setCycles", &Detector::setCycles)
        .def("getNumberOfGates", &Detector::getNumberOfGates)
        .def("setNumberOfGates", &Detector::setNumberOfGates)
        .def("getDelay", &Detector::getDelay)
        .def("setDelay", &Detector::setDelay)

        .def("setStoragecellStart", &Detector::setStoragecellStart)
        .def("getStoragecellStart", &Detector::getStoragecellStart)
        .def("setNumberOfStorageCells", &Detector::setNumberOfStorageCells)
        .def("getNumberOfStorageCells", &Detector::getNumberOfStorageCells)

        .def("getTimingMode", &Detector::getTimingMode)
        .def("setTimingMode", &Detector::setTimingMode)

        .def("getDetectorType", &Detector::getDetectorType)

        .def("setThresholdTemperature", &Detector::setThresholdTemperature)
        .def("getThresholdTemperature", &Detector::getThresholdTemperature)
        .def("setTemperatureControl", &Detector::setTemperatureControl)
        .def("getTemperatureControl", &Detector::getTemperatureControl)
        .def("getTemperatureEvent", &Detector::getTemperatureEvent)
        .def("resetTemperatureEvent", &Detector::resetTemperatureEvent)

        .def("getRxDataStreamStatus", &Detector::getRxDataStreamStatus)
        .def("setRxDataStreamStatus", &Detector::setRxDataStreamStatus)

        // Network stuff
        .def("getReceiverHostname",
             &DetectorPythonInterface::getReceiverHostname,
             py::arg("det_id") = -1)
        .def("setReceiverHostname",
             &DetectorPythonInterface::setReceiverHostname, py::arg("hostname"),
             py::arg("det_id") = -1)
        .def("getReceiverStreamingPort",
             &DetectorPythonInterface::getReceiverStreamingPort)
        .def("setReceiverStreamingPort",
             &DetectorPythonInterface::setReceiverStreamingPort)
        .def("getReceiverUDPPort", &DetectorPythonInterface::getReceiverUDPPort)
        .def("getReceiverUDPPort2",
             &DetectorPythonInterface::getReceiverUDPPort2)
        .def("setReceiverUDPPort", &DetectorPythonInterface::setReceiverUDPPort)
        .def("setReceiverUDPPort2",
             &DetectorPythonInterface::setReceiverUDPPort2)
        .def("setReceiverUDPIP", &DetectorPythonInterface::setReceiverUDPIP)
        .def("getReceiverUDPIP", &DetectorPythonInterface::getReceiverUDPIP)
        .def("getReceiverUDPMAC", &DetectorPythonInterface::getReceiverUDPMAC)
        .def("setReceiverUDPMAC", &DetectorPythonInterface::setReceiverUDPMAC)

        .def("getReceiverPort", &DetectorPythonInterface::getReceiverPort)
        .def("setReceiverPort", &DetectorPythonInterface::setReceiverPort)

        .def("configureNetworkParameters",
             &DetectorPythonInterface::configureNetworkParameters)
        .def("getDelayFrame", &DetectorPythonInterface::getDelayFrame)
        .def("setDelayFrame", &DetectorPythonInterface::setDelayFrame)
        .def("getDelayLeft", &DetectorPythonInterface::getDelayLeft)
        .def("setDelayLeft", &DetectorPythonInterface::setDelayLeft)
        .def("getDelayRight", &DetectorPythonInterface::getDelayRight)
        .def("setDelayRight", &DetectorPythonInterface::setDelayRight)
        .def("getLastClientIP", &DetectorPythonInterface::getLastClientIP)
        .def("getReceiverLastClientIP",
             &DetectorPythonInterface::getReceiverLastClientIP)

        .def("setFramesPerFile", &DetectorPythonInterface::setFramesPerFile)
        .def("getFramesPerFile", &DetectorPythonInterface::getFramesPerFile)
        .def("setReceiverFifoDepth",
             &DetectorPythonInterface::setReceiverFifoDepth)
        .def("getReceiverFifoDepth",
             &DetectorPythonInterface::getReceiverFifoDepth)

        .def("getReceiverFrameDiscardPolicy",
             &DetectorPythonInterface::getReceiverFrameDiscardPolicy)
        .def("setReceiverFramesDiscardPolicy",
             &DetectorPythonInterface::setReceiverFramesDiscardPolicy)
        .def("setPartialFramesPadding",
             &DetectorPythonInterface::setPartialFramesPadding)
        .def("getPartialFramesPadding",
             &DetectorPythonInterface::getPartialFramesPadding)

        .def("getUserDetails", &DetectorPythonInterface::getUserDetails)
        .def("isClientAndDetectorCompatible",
             &DetectorPythonInterface::isClientAndDetectorCompatible)
        .def("isClientAndReceiverCompatible",
             &DetectorPythonInterface::isClientAndReceiverCompatible)
        .def("getMeasuredPeriod", &DetectorPythonInterface::getMeasuredPeriod)
        .def("getMeasuredSubPeriod",
             &DetectorPythonInterface::getMeasuredSubPeriod)

        .def("setFileWrite", &DetectorPythonInterface::setFileWrite)
        .def("getFileWrite", &DetectorPythonInterface::getFileWrite)
        .def("setFileOverWrite", &DetectorPythonInterface::setFileOverWrite)
        .def("getFileOverWrite", &DetectorPythonInterface::getFileOverWrite)
        .def("getDacVthreshold", &DetectorPythonInterface::getDacVthreshold)
        .def("setDacVthreshold", &DetectorPythonInterface::setDacVthreshold)
        .def("setNumberOfFrames", &DetectorPythonInterface::setNumberOfFrames)
        .def("getNumberOfFrames", &DetectorPythonInterface::getNumberOfFrames)

        // Overloaded calls
        .def("getFramesCaughtByReceiver",
             (int (DetectorPythonInterface::*)()) &
                 DetectorPythonInterface::getFramesCaughtByReceiver)
        .def("getFramesCaughtByReceiver",
             (int (DetectorPythonInterface::*)(int)) &
                 DetectorPythonInterface::getFramesCaughtByReceiver)

        .def("resetFramesCaught", &DetectorPythonInterface::resetFramesCaught)
        .def("getReceiverCurrentFrameIndex",
             &DetectorPythonInterface::getReceiverCurrentFrameIndex)
        .def("getGapPixels", &DetectorPythonInterface::getGapPixels)
        .def("setGapPixels", &DetectorPythonInterface::setGapPixels)
        .def("getFlippedDataX", &DetectorPythonInterface::getFlippedDataX)
        .def("getFlippedDataY", &DetectorPythonInterface::getFlippedDataY)
        .def("setFlippedDataX", &DetectorPythonInterface::setFlippedDataX)
        .def("setFlippedDataY", &DetectorPythonInterface::setFlippedDataY)

        .def("getServerLock", &DetectorPythonInterface::getServerLock)
        .def("setServerLock", &DetectorPythonInterface::setServerLock)
        .def("getReceiverLock", &DetectorPythonInterface::getReceiverLock)
        .def("setReceiverLock", &DetectorPythonInterface::setReceiverLock)

        .def("getReadoutFlags", &DetectorPythonInterface::getReadoutFlags)
        .def("setReadoutFlag", &DetectorPythonInterface::setReadoutFlag)

        .def("setFileFormat", &DetectorPythonInterface::setFileFormat)
        .def("getFileFormat", &DetectorPythonInterface::getFileFormat)

        .def("getActive", &DetectorPythonInterface::getActive)
        .def("setActive", &DetectorPythonInterface::setActive)

        .def("getTenGigabitEthernet",
             &DetectorPythonInterface::getTenGigabitEthernet)
        .def("setTenGigabitEthernet",
             &DetectorPythonInterface::setTenGigabitEthernet)

        .def("getPatternLoops", &DetectorPythonInterface::getPatternLoops,
             py::arg("level"), py::arg("det_id") = -1)
        .def("setPatternLoops", &DetectorPythonInterface::setPatternLoops,
             py::arg("level"), py::arg("start"), py::arg("stop"), py::arg("n"),
             py::arg("det_id") = -1)
        .def("setPatternWord", &DetectorPythonInterface::setPatternWord,
             py::arg("addr"), py::arg("word"), py::arg("det_id") = -1)
        .def("getPatternWord", &DetectorPythonInterface::getPatternWord,
             py::arg("addr"), py::arg("det_id") = -1)

        .def("setPatternIOControl",
             &DetectorPythonInterface::setPatternIOControl, py::arg("word"),
             py::arg("det_id") = -1)
        .def("setPatternClockControl",
             &DetectorPythonInterface::setPatternClockControl, py::arg("word"),
             py::arg("det_id") = -1)

        .def("setPatternWaitAddr", &DetectorPythonInterface::setPatternWaitAddr,
             py::arg("level"), py::arg("addr"), py::arg("det_id") = -1)
        .def("getPatternWaitAddr", &DetectorPythonInterface::getPatternWaitAddr,
             py::arg("level"), py::arg("det_id") = -1)

        .def("setPatternWaitTime", &DetectorPythonInterface::setPatternWaitTime,
             py::arg("level"), py::arg("duration"), py::arg("det_id") = -1)

        .def("getPatternWaitTime", &DetectorPythonInterface::getPatternWaitTime,
             py::arg("level"), py::arg("det_id") = -1)

        .def("getImageSize", &DetectorPythonInterface::getImageSize)
        .def("setImageSize", &DetectorPythonInterface::setImageSize)
        .def("getNumberOfDetectors",
             &DetectorPythonInterface::getNumberOfDetectors)
        .def("getDetectorGeometry",
             &DetectorPythonInterface::getDetectorGeometry);

    // Experimental API to use the multi directly and inherit from to reduce
    // code duplication need to investigate how to handle documentation
    using sls::Detector;
    py::class_<Detector> multiDetectorApi(m, "multiDetectorApi");
    multiDetectorApi.def(py::init<int>())
        .def("acquire", &Detector::acquire)

        // File
        .def("setFname", &Detector::setFname, py::arg())
        .def("getFname", &Detector::getFname)
        .def("setFwrite", &Detector::setFwrite, py::arg(),
             py::arg() = std::vector<int>{})
        .def("getFwrite", &Detector::getFwrite,
             py::arg() = std::vector<int>{})

        // Time
        .def("setExptime", &Detector::setExptime, py::arg(),
             py::arg() = std::vector<int>{})
        .def("getExptime", &Detector::getExptime,
             py::arg() = std::vector<int>{})
        .def("setPeriod", &Detector::setPeriod, py::arg(),
             py::arg() = std::vector<int>{})
        .def("getPeriod", &Detector::getPeriod, py::arg() = std::vector<int>{})
        .def("setSubExptime", &Detector::setSubExptime, py::arg(),
             py::arg() = std::vector<int>{})
        .def("getSubExptime", &Detector::getSubExptime,
             py::arg() = std::vector<int>{});

    py::module io = m.def_submodule("io", "Submodule for io");
    io.def("read_my302_file", &read_my302_file, "some");

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
