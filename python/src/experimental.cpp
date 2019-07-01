#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls_detector_defs.h"
#include "Detector.h"
#include "typecaster.h"
namespace py = pybind11;
void init_experimental(py::module &m) {
// Experimental API to use the multi directly and inherit from to reduce
    // code duplication need to investigate how to handle documentation
    using sls::Detector;
    py::class_<Detector> multiDetectorApi(m, "multiDetectorApi");
    multiDetectorApi
        .def(py::init<int>())


        // Acq related
        .def("acquire", &Detector::acquire)
        .def("startReceiver", &Detector::startReceiver,
             py::arg() = std::vector<int>{})
        .def("stopReceiver", &Detector::stopReceiver,
             py::arg() = std::vector<int>{})
        .def("getReceiverStatus", &Detector::getReceiverStatus,
             py::arg() = std::vector<int>{})
        // Configuration
        .def("free", &Detector::freeSharedMemory)
        .def("setConfig", &Detector::setConfig)
        .def("getHostname", &Detector::getHostname,
             py::arg() = std::vector<int>{})

        .def("setBit", &Detector::setBit, py::arg(), py::arg(),
             py::arg() = std::vector<int>{})
        .def("clearBit", &Detector::clearBit, py::arg(), py::arg(),
             py::arg() = std::vector<int>{})
        .def("getRegister", &Detector::getRegister, py::arg(),
             py::arg() = std::vector<int>{})

        .def("getStartingFrameNumber", &Detector::getStartingFrameNumber,
             py::arg() = std::vector<int>{})
        .def("setStartingFrameNumber", &Detector::setStartingFrameNumber,
             py::arg(), py::arg() = std::vector<int>{})

        // File
        .def("setFname", &Detector::setFname, py::arg())
        .def("getFname", &Detector::getFname)
        .def("setFwrite", &Detector::setFwrite, py::arg(),
             py::arg() = std::vector<int>{})
        .def("getFwrite", &Detector::getFwrite, py::arg() = std::vector<int>{})

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
}
