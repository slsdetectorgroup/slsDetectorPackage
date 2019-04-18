#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"
#include "mythenFileIO.h"

namespace py = pybind11;

PYBIND11_MODULE(_sls_detector, m) {
    m.doc() = R"pbdoc(
        C/C++ API
        -----------------------
        .. warning ::

            This is the compiled c extension. You probably want to look at the
            interface provided by sls instead.

    )pbdoc";

    py::class_<Detector> DetectorApi(m, "DetectorApi", R"pbdoc(
    Interface to the multiSlsDetector class through Detector.h These functions
    are used by the python classes Eiger and Jungfrau and normally it is better
    to use them than to directly access functions here.

    However it is possible to access these functions...

    ::

        #Using the python class
        from sls_detector import Eiger
        d = Eiger()
        d._api.getThresholdEnergy()

        #creating a DetectorApi object (remember to set online flags)
        from _sls_detector import DetectorApi
        api = DetectorApi(0)
        api.setOnline(True)
        api.setReceiverOnline(True)
        api.getNumberOfFrames()

        #But the Pythonic way is almost alway simpler
        d = Eiger()
        d.n_frames
        >> 10

    )pbdoc");
    DetectorApi.def(py::init<int>())
        .def("freeSharedMemory", &Detector::freeSharedMemory)
        .def("getMultiDetectorId", &Detector::getMultiDetectorId)
        .def("acq", &Detector::acquire)
        .def("getAcquiringFlag", &Detector::getAcquiringFlag)
        .def("setAcquiringFlag", &Detector::setAcquiringFlag)

        .def("setAllTrimbits", &Detector::setAllTrimbits)
        .def("getAllTrimbits", &Detector::getAllTrimbits)
        .def("setCounterBit", &Detector::setCounterBit)
        .def("getCounterBit", &Detector::getCounterBit)

        .def("getAdc", &Detector::getAdc)
        .def("getDac", &Detector::getDac)
        .def("getDac_mV", &Detector::getDac_mV)
        .def("setDac", &Detector::setDac)
        .def("setDac_mV", &Detector::setDac_mV)
        .def("getDacFromIndex", &Detector::getDacFromIndex)
        .def("setDacFromIndex", &Detector::setDacFromIndex)

        .def("getDbitPipeline", &Detector::getDbitPipeline)
        .def("setDbitPipeline", &Detector::setDbitPipeline)
        .def("getDbitPhase", &Detector::getDbitPhase)
        .def("setDbitPhase", &Detector::setDbitPhase)
        .def("getDbitClock", &Detector::getDbitClock)
        .def("setDbitClock", &Detector::setDbitClock)

        .def("setThresholdEnergy", &Detector::setThresholdEnergy)
        .def("getThresholdEnergy", &Detector::getThresholdEnergy)

        .def("getSettings", &Detector::getSettings)
        .def("setSettings", &Detector::setSettings)
        .def("getSettingsDir", &Detector::getSettingsDir)
        .def("setSettingsDir", &Detector::setSettingsDir)

        .def("loadTrimbitFile", &Detector::loadTrimbitFile)
        .def("setTrimEnergies", &Detector::setTrimEnergies)
        .def("getTrimEnergies", &Detector::getTrimEnergies)

        .def("pulseChip", &Detector::pulseChip)
        .def("pulseAllPixels", &Detector::pulseAllPixels)
        .def("pulseDiagonal", &Detector::pulseDiagonal)
        .def("getRunStatus", &Detector::getRunStatus)
        .def("readConfigurationFile", &Detector::readConfigurationFile)
        .def("readParametersFile", &Detector::readParametersFile)
        .def("checkOnline", &Detector::checkOnline)
        .def("setReadoutClockSpeed", &Detector::setReadoutClockSpeed)
        .def("getReadoutClockSpeed", &Detector::getReadoutClockSpeed)
        .def("getHostname", &Detector::getHostname)
        .def("setHostname", &Detector::setHostname)

        .def("getOnline", &Detector::getOnline)
        .def("setOnline", &Detector::setOnline)
        .def("getReceiverOnline", &Detector::getReceiverOnline)
        .def("setReceiverOnline", &Detector::setReceiverOnline)

        .def("getReceiverPort", &Detector::getReceiverPort)
        .def("setReceiverPort", &Detector::setReceiverPort)

        .def("isChipPowered", &Detector::isChipPowered)
        .def("powerChip", &Detector::powerChip)

        .def("readRegister", &Detector::readRegister)
        .def("writeRegister", &Detector::writeRegister)
        .def("writeAdcRegister", &Detector::writeAdcRegister)
        .def("setBitInRegister", &Detector::setBitInRegister)
        .def("clearBitInRegister", &Detector::clearBitInRegister)

        .def("setDynamicRange", &Detector::setDynamicRange)
        .def("getDynamicRange", &Detector::getDynamicRange)
        .def("getFirmwareVersion", &Detector::getFirmwareVersion)
        .def("getServerVersion", &Detector::getServerVersion)
        .def("getClientVersion", &Detector::getClientVersion)
        .def("getReceiverVersion", &Detector::getReceiverVersion)
        .def("getDetectorNumber", &Detector::getDetectorNumber)
        .def("getRateCorrection", &Detector::getRateCorrection)
        .def("setRateCorrection", &Detector::setRateCorrection)

        .def("startAcquisition", &Detector::startAcquisition)
        .def("stopAcquisition", &Detector::stopAcquisition)
        .def("startReceiver", &Detector::startReceiver)
        .def("stopReceiver", &Detector::stopReceiver)

        .def("getFilePath",
             (std::string(Detector::*)()) & Detector::getFilePath,
             "Using multiSlsDetector")
        .def("getFilePath",
             (std::string(Detector::*)(int)) & Detector::getFilePath,
             "File path for individual detector")
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
        .def("setNumberOfMeasurements", &Detector::setNumberOfMeasurements)
        .def("getNumberOfMeasurements", &Detector::getNumberOfMeasurements)
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
        .def("getReceiverHostname", &Detector::getReceiverHostname,
             py::arg("det_id") = -1)
        .def("setReceiverHostname", &Detector::setReceiverHostname,
             py::arg("hostname"), py::arg("det_id") = -1)
        .def("getReceiverStreamingPort", &Detector::getReceiverStreamingPort)
        .def("setReceiverStreamingPort", &Detector::setReceiverStreamingPort)
        .def("getReceiverUDPPort", &Detector::getReceiverUDPPort)
        .def("getReceiverUDPPort2", &Detector::getReceiverUDPPort2)
        .def("setReceiverUDPPort", &Detector::setReceiverUDPPort)
        .def("setReceiverUDPPort2", &Detector::setReceiverUDPPort2)
        .def("setReceiverUDPIP", &Detector::setReceiverUDPIP)
        .def("getReceiverUDPIP", &Detector::getReceiverUDPIP)
        .def("getReceiverUDPMAC", &Detector::getReceiverUDPMAC)
        .def("setReceiverUDPMAC", &Detector::setReceiverUDPMAC)

        .def("getReceiverPort", &Detector::getReceiverPort)
        .def("setReceiverPort", &Detector::setReceiverPort)

        .def("configureNetworkParameters",
             &Detector::configureNetworkParameters)
        .def("getDelayFrame", &Detector::getDelayFrame)
        .def("setDelayFrame", &Detector::setDelayFrame)
        .def("getDelayLeft", &Detector::getDelayLeft)
        .def("setDelayLeft", &Detector::setDelayLeft)
        .def("getDelayRight", &Detector::getDelayRight)
        .def("setDelayRight", &Detector::setDelayRight)
        .def("getLastClientIP", &Detector::getLastClientIP)
        .def("getReceiverLastClientIP", &Detector::getReceiverLastClientIP)

        .def("setFramesPerFile", &Detector::setFramesPerFile)
        .def("getFramesPerFile", &Detector::getFramesPerFile)
        .def("setReceiverFifoDepth", &Detector::setReceiverFifoDepth)
        .def("getReceiverFifoDepth", &Detector::getReceiverFifoDepth)

        .def("getReceiverFrameDiscardPolicy",
             &Detector::getReceiverFrameDiscardPolicy)
        .def("setReceiverFramesDiscardPolicy",
             &Detector::setReceiverFramesDiscardPolicy)
        .def("setPartialFramesPadding", &Detector::setPartialFramesPadding)
        .def("getPartialFramesPadding", &Detector::getPartialFramesPadding)

        .def("getUserDetails", &Detector::getUserDetails)
        .def("isClientAndDetectorCompatible",
             &Detector::isClientAndDetectorCompatible)
        .def("isClientAndReceiverCompatible",
             &Detector::isClientAndReceiverCompatible)
        .def("getMeasuredPeriod", &Detector::getMeasuredPeriod)
        .def("getMeasuredSubPeriod", &Detector::getMeasuredSubPeriod)

        .def("setFileWrite", &Detector::setFileWrite)
        .def("getFileWrite", &Detector::getFileWrite)
        .def("setFileOverWrite", &Detector::setFileOverWrite)
        .def("getFileOverWrite", &Detector::getFileOverWrite)
        .def("getDacVthreshold", &Detector::getDacVthreshold)
        .def("setDacVthreshold", &Detector::setDacVthreshold)
        .def("setNumberOfFrames", &Detector::setNumberOfFrames)
        .def("getNumberOfFrames", &Detector::getNumberOfFrames)

        // Overloaded calls
        .def("getFramesCaughtByReceiver",
             (int (Detector::*)()) & Detector::getFramesCaughtByReceiver)
        .def("getFramesCaughtByReceiver",
             (int (Detector::*)(int)) & Detector::getFramesCaughtByReceiver)

        .def("resetFramesCaught", &Detector::resetFramesCaught)
        .def("getReceiverCurrentFrameIndex",
             &Detector::getReceiverCurrentFrameIndex)
        .def("getGapPixels", &Detector::getGapPixels)
        .def("setGapPixels", &Detector::setGapPixels)
        .def("getFlippedDataX", &Detector::getFlippedDataX)
        .def("getFlippedDataY", &Detector::getFlippedDataY)
        .def("setFlippedDataX", &Detector::setFlippedDataX)
        .def("setFlippedDataY", &Detector::setFlippedDataY)

        .def("getServerLock", &Detector::getServerLock)
        .def("setServerLock", &Detector::setServerLock)
        .def("getReceiverLock", &Detector::getReceiverLock)
        .def("setReceiverLock", &Detector::setReceiverLock)

        .def("getReadoutFlags", &Detector::getReadoutFlags)
        .def("setReadoutFlag", &Detector::setReadoutFlag)

        .def("setFileFormat", &Detector::setFileFormat)
        .def("getFileFormat", &Detector::getFileFormat)

        .def("getActive", &Detector::getActive)
        .def("setActive", &Detector::setActive)

        .def("getTenGigabitEthernet", &Detector::getTenGigabitEthernet)
        .def("setTenGigabitEthernet", &Detector::setTenGigabitEthernet)

        .def("getPatternLoops", &Detector::getPatternLoops, py::arg("level"),
             py::arg("det_id") = -1)
        .def("setPatternLoops", &Detector::setPatternLoops, py::arg("level"),
             py::arg("start"), py::arg("stop"), py::arg("n"),
             py::arg("det_id") = -1)
        .def("setPatternWord", &Detector::setPatternWord, py::arg("addr"),
             py::arg("word"), py::arg("det_id") = -1)
        .def("getPatternWord", &Detector::getPatternWord, py::arg("addr"),
             py::arg("det_id") = -1)

        .def("setPatternWaitAddr", &Detector::setPatternWaitAddr,
             py::arg("level"), py::arg("addr"), py::arg("det_id") = -1)
        .def("getPatternWaitAddr", &Detector::getPatternWaitAddr,
             py::arg("level"), py::arg("det_id") = -1)

        .def("setPatternWaitTime", &Detector::setPatternWaitTime,
             py::arg("level"), py::arg("duration"), py::arg("det_id") = -1)

        .def("getPatternWaitTime", &Detector::getPatternWaitTime,
             py::arg("level"), py::arg("det_id") = -1)

        .def("getImageSize", &Detector::getImageSize)
        .def("setImageSize", &Detector::setImageSize)
        .def("getNumberOfDetectors", &Detector::getNumberOfDetectors)
        .def("getDetectorGeometry", &Detector::getDetectorGeometry);

    // Experimental API to use the multi directly and inherit from to reduce
    // code duplication need to investigate how to handle documentation
    py::class_<multiSlsDetector> multiDetectorApi(m, "multiDetectorApi");
    multiDetectorApi.def(py::init<int>())
        .def("acquire", &multiSlsDetector::acquire)

        .def_property("online", 
        py::cpp_function(&multiSlsDetector::setOnline, py::arg(), py::arg()=-1, py::arg("det_id")=-1),
        py::cpp_function(&multiSlsDetector::setOnline, py::arg(), py::arg("flag"), py::arg("det_id")=-1)
        )
        // .def("_setOnline", &multiSlsDetector::setOnline, py::arg("flag") = -1,
        //      py::arg("det_id") = -1)

        .def_property_readonly(
            "hostname", py::cpp_function(&multiSlsDetector::getHostname,
                                         py::arg(), py::arg("det_id") = -1))
        .def_property("busy",
                      py::cpp_function(&multiSlsDetector::getAcquiringFlag),
                      py::cpp_function(&multiSlsDetector::setAcquiringFlag))
        .def_property_readonly(
            "rx_tcpport", py::cpp_function(&multiSlsDetector::getReceiverPort))
        .def_property_readonly(
            "detectornumber",
            py::cpp_function(&multiSlsDetector::getDetectorNumber))
        .def_property("rx_udpip",
                      py::cpp_function(&multiSlsDetector::getReceiverUDPIP,
                                       py::arg(), py::arg("det_id") = -1),
                      py::cpp_function(&multiSlsDetector::setReceiverUDPIP,
                                       py::arg(), py::arg("ip"), py::arg("det_id") = -1) )
        .def("_getReceiverUDPIP", &multiSlsDetector::getReceiverUDPIP)
        .def("_setReceiverUDPIP", &multiSlsDetector::setReceiverUDPIP)
        .def("getPatternLoops", &multiSlsDetector::getPatternLoops,
             py::arg("level"), py::arg("det_id") = -1)
        .def("setPatternLoops", &multiSlsDetector::setPatternLoops)
        .def("setPatternWord", &multiSlsDetector::setPatternWord,
             py::arg("addr"), py::arg("word"), py::arg("det_id") = -1);

    py::module io = m.def_submodule("io", "Submodule for io");
    io.def("read_my302_file", &read_my302_file, "some");

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
