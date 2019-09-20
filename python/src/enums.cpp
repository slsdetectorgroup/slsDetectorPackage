#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls_detector_defs.h"
namespace py = pybind11;
void init_enums(py::module &m) {
    py::class_<slsDetectorDefs> Defs(m, "slsDetectorDefs");
    py::class_<slsDetectorDefs::xy> xy(m, "xy");
    // xy.def(py::init())
    xy.def_readwrite("x", &slsDetectorDefs::xy::x);
    xy.def_readwrite("y", &slsDetectorDefs::xy::y);

    py::enum_<slsDetectorDefs::runStatus>(Defs, "runStatus")
        .value("IDLE", slsDetectorDefs::runStatus::IDLE)
        .value("ERROR", slsDetectorDefs::runStatus::ERROR)
        .value("WAITING", slsDetectorDefs::runStatus::WAITING)
        .value("RUN_FINISHED", slsDetectorDefs::runStatus::RUN_FINISHED)
        .value("TRANSMITTING", slsDetectorDefs::runStatus::TRANSMITTING)
        .value("RUNNING", slsDetectorDefs::runStatus::RUNNING)
        .value("STOPPED", slsDetectorDefs::runStatus::STOPPED)
        .export_values();

    py::enum_<slsDetectorDefs::detectorType>(Defs, "detectorType")
        .value("GENERIC", slsDetectorDefs::detectorType::GENERIC)
        .value("EIGER", slsDetectorDefs::detectorType::EIGER)
        .value("GOTTHARD", slsDetectorDefs::detectorType::GOTTHARD)
        .value("JUNGFRAU", slsDetectorDefs::detectorType::JUNGFRAU)
        .value("CHIPTESTBOARD", slsDetectorDefs::detectorType::CHIPTESTBOARD)
        .value("MOENCH", slsDetectorDefs::detectorType::MOENCH)
        .value("MYTHEN3", slsDetectorDefs::detectorType::MYTHEN3)
        .value("GOTTHARD2", slsDetectorDefs::detectorType::GOTTHARD2)
        .export_values();

        py::enum_<slsDetectorDefs::detectorSettings>(Defs, "detectorSettings")
        .value("GET_SETTINGS", slsDetectorDefs::detectorSettings::GET_SETTINGS)
        .value("STANDARD", slsDetectorDefs::detectorSettings::STANDARD)
        .value("FAST", slsDetectorDefs::detectorSettings::FAST)
        .value("HIGHGAIN", slsDetectorDefs::detectorSettings::HIGHGAIN)
        .value("DYNAMICGAIN", slsDetectorDefs::detectorSettings::DYNAMICGAIN)
        .value("LOWGAIN", slsDetectorDefs::detectorSettings::LOWGAIN)
        .value("MEDIUMGAIN", slsDetectorDefs::detectorSettings::MEDIUMGAIN)
        .value("VERYHIGHGAIN", slsDetectorDefs::detectorSettings::VERYHIGHGAIN)
        .value("DYNAMICHG0", slsDetectorDefs::detectorSettings::DYNAMICHG0)
        .value("FIXGAIN1", slsDetectorDefs::detectorSettings::FIXGAIN1)
        .value("FIXGAIN2", slsDetectorDefs::detectorSettings::FIXGAIN2)
        .value("FORCESWITCHG1", slsDetectorDefs::detectorSettings::FORCESWITCHG1)
        .value("FORCESWITCHG2", slsDetectorDefs::detectorSettings::FORCESWITCHG2)
        .value("VERYLOWGAIN", slsDetectorDefs::detectorSettings::VERYLOWGAIN)
        .value("UNDEFINED", slsDetectorDefs::detectorSettings::UNDEFINED)
        .value("UNINITIALIZED", slsDetectorDefs::detectorSettings::UNINITIALIZED)
        .export_values();
}
