#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"
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

        // Acq related
        .def("acquire", &Detector::acquire)
        .def("clearAcquiringFlag", &Detector::clearAcquiringFlag)
        .def("getReceiverStatus", &Detector::getReceiverStatus,
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
