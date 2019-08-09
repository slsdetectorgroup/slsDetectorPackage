#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"
#include "sls_detector_defs.h"
#include "typecaster.h"
namespace py = pybind11;
void init_experimental(py::module &m) {
    // Experimental API to use the multi directly and inherit from to reduce
    // code duplication need to investigate how to handle documentation
    using sls::Detector;
    using sls::Positions;
    py::class_<Detector> multiDetectorApi(m, "multiDetectorApi");
    multiDetectorApi
        .def(py::init<int>())

        // Acq related
        .def("acquire", &Detector::acquire)
        .def("startReceiver", &Detector::startReceiver, py::arg() = Positions{})
        .def("stopReceiver", &Detector::stopReceiver, py::arg() = Positions{})
        .def("getAcquiringFlag", &Detector::getAcquiringFlag)
        .def("setAcquiringFlag", &Detector::setAcquiringFlag)
        .def("getReceiverStatus", &Detector::getReceiverStatus,
             py::arg() = Positions{})

        // Configuration
        .def("free", &Detector::freeSharedMemory)
        .def("setConfig", &Detector::setConfig)
        .def("getHostname", &Detector::getHostname, py::arg() = Positions{})

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
        .def("getFileName", &Detector::getFileName)
        .def("setFileName", &Detector::setFileName, py::arg(),py::arg() = Positions{})
        .def("getFilePath", &Detector::getFilePath)
        .def("setFilePath", &Detector::setFilePath, py::arg(),py::arg() = Positions{})
        .def("setFileWrite", &Detector::setFileWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFileWrite", &Detector::getFileWrite, py::arg() = Positions{})
        .def("setFileOverWrite", &Detector::setFileOverWrite, py::arg(),
             py::arg() = Positions{})
        .def("getFileOverWrite", &Detector::getFileOverWrite,
             py::arg() = Positions{})

        // Time
        .def("setExptime", &Detector::setExptime, py::arg(),
             py::arg() = Positions{})
        .def("getExptime", &Detector::getExptime, py::arg() = Positions{})
        .def("setPeriod", &Detector::setPeriod, py::arg(),
             py::arg() = Positions{})
        .def("getPeriod", &Detector::getPeriod, py::arg() = Positions{})
        .def("setSubExptime", &Detector::setSubExptime, py::arg(),
             py::arg() = Positions{})
        .def("getSubExptime", &Detector::getSubExptime,
             py::arg() = Positions{});
}
