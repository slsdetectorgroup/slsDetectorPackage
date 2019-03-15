#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"

namespace py = pybind11;

PYBIND11_MODULE(_sls_detector, m)
{
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
    DetectorApi
        .def(py::init<int>())
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

        .def("getFilePath", (std::string(Detector::*)()) & Detector::getFilePath, "Using multiSlsDetector")
        .def("getFilePath", (std::string(Detector::*)(int)) & Detector::getFilePath, "File path for individual detector")
        .def("setFilePath", (void (Detector::*)(std::string)) & Detector::setFilePath)
        .def("setFilePath", (void (Detector::*)(std::string, int)) & Detector::setFilePath)

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

        //Network stuff
        .def("getReceiverStreamingPort", &Detector::getReceiverStreamingPort)
        .def("setReceiverStreamingPort", &Detector::setReceiverStreamingPort)
        .def("getReceiverUDPPort", &Detector::getReceiverUDPPort)
        .def("getReceiverUDPPort2", &Detector::getReceiverUDPPort2)
        
        .def("getReceiverPort", &Detector::getReceiverPort)
        .def("setReceiverPort", &Detector::setReceiverPort)

        .def("configureNetworkParameters", &Detector::configureNetworkParameters)
        .def("getDelayFrame", &Detector::getDelayFrame)
        .def("setDelayFrame", &Detector::setDelayFrame)
        .def("getDelayLeft", &Detector::getDelayLeft)
        .def("setDelayLeft", &Detector::setDelayLeft)
        .def("getDelayRight", &Detector::getDelayRight)
        .def("setDelayRight", &Detector::setDelayRight)
        .def("getLastClientIP", &Detector::getLastClientIP)
        .def("getReceiverLastClientIP", &Detector::getReceiverLastClientIP)

        .def("setReceiverFramesPerFile", &Detector::setReceiverFramesPerFile)
        .def("getReceiverFramesPerFile", &Detector::getReceiverFramesPerFile)
        .def("setReceiverFifoDepth", &Detector::setReceiverFifoDepth)
        .def("getReceiverFifoDepth", &Detector::getReceiverFifoDepth)

        .def("getReceiverFrameDiscardPolicy", &Detector::getReceiverFrameDiscardPolicy)
        .def("setReceiverFramesDiscardPolicy", &Detector::setReceiverFramesDiscardPolicy)
        .def("setReceiverPartialFramesPadding", &Detector::setReceiverPartialFramesPadding)
        .def("getReceiverPartialFramesPadding", &Detector::getReceiverPartialFramesPadding)

        .def("getUserDetails", &Detector::getUserDetails)
        .def("isClientAndDetecorCompatible", &Detector::isClientAndDetecorCompatible)
        .def("isClientAndReceiverCompatible", &Detector::isClientAndReceiverCompatible)
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

        //Overloaded calls
        .def("getFramesCaughtByReceiver", (int (Detector::*)() ) & Detector::getFramesCaughtByReceiver)
        .def("getFramesCaughtByReceiver", (int (Detector::*)(int)) & Detector::getFramesCaughtByReceiver)


        .def("resetFramesCaught", &Detector::resetFramesCaught)
        .def("getReceiverCurrentFrameIndex", &Detector::getReceiverCurrentFrameIndex)
        .def("getGapPixels", &Detector::getGapPixels)
        .def("setGapPixels", &Detector::setGapPixels)

        .def("clearErrorMask", &Detector::clearErrorMask)
        .def("getErrorMask", &Detector::getErrorMask)
        .def("setErrorMask", &Detector::setErrorMask)
        .def("getErrorMessage", &Detector::getErrorMessage)

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

        .def("getImageSize", &Detector::getImageSize)
        .def("setImageSize", &Detector::setImageSize)
        .def("getNumberOfDetectors", &Detector::getNumberOfDetectors)
        .def("getDetectorGeometry", &Detector::getDetectorGeometry);



//Experimental API to use the multi directly and inherit from to reduce
//code duplication need to investigate how to handle documentation
py::class_<multiSlsDetector> multiDetectorApi(m, "multiDetectorApi");
    multiDetectorApi
        .def(py::init<int>())
        .def_property("busy", 
            py::cpp_function(&multiSlsDetector::getAcquiringFlag),
            py::cpp_function(&multiSlsDetector::setAcquiringFlag))
         .def_property_readonly("rx_tcpport", 
            py::cpp_function(&multiSlsDetector::getReceiverPort))          
            
            ;


#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
