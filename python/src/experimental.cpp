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
        .def("configureMAC", &Detector::configureMAC, py::arg() = Positions{})
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
        .def("getFileNamePrefix", &Detector::getFileNamePrefix)
        .def("setFileNamePrefix", &Detector::setFileNamePrefix, py::arg(),
             py::arg() = Positions{})
        .def("getFilePath", &Detector::getFilePath)
        .def("setFilePath", &Detector::setFilePath, py::arg(),
             py::arg() = Positions{})
        .def("setFileWrite", &Detector::setFileWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFileWrite", &Detector::getFileWrite, py::arg() = Positions{})
        .def("setFileOverWrite", &Detector::setFileOverWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFileOverWrite", &Detector::getFileOverWrite,
             py::arg() = Positions{})

        // Time

        .def("setSubExptime", &Detector::setSubExptime, py::arg(),
             py::arg() = Positions{})
        .def("getSubExptime", &Detector::getSubExptime,
             py::arg() = Positions{});
}
