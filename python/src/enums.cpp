#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls_detector_defs.h"
namespace py = pybind11;
void init_enums(py::module &m) {
    py::class_<slsDetectorDefs> Defs(m, "slsDetectorDefs");
    py::enum_<slsDetectorDefs::runStatus>(Defs, "runStatus")
        .value("IDLE", slsDetectorDefs::runStatus::IDLE)
        .value("ERROR", slsDetectorDefs::runStatus::ERROR)
        .value("WAITING", slsDetectorDefs::runStatus::WAITING)
        .value("RUN_FINISHED", slsDetectorDefs::runStatus::RUN_FINISHED)
        .value("TRANSMITTING", slsDetectorDefs::runStatus::TRANSMITTING)
        .value("RUNNING", slsDetectorDefs::runStatus::RUNNING)
        .value("STOPPED", slsDetectorDefs::runStatus::STOPPED)
        .export_values();
}
