// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "py_headers.h"

#include "sls/sls_detector_defs.h"
#include <sstream>
namespace py = pybind11;
void init_scan(py::module &m) {

    using sp = slsDetectorDefs::scanParameters;
    py::class_<sp> scanParameters(m, "scanParameters");

    scanParameters.def(py::init());

    scanParameters.def(py::init<slsDetectorDefs::dacIndex, int, int, int>());
    scanParameters.def(py::init<slsDetectorDefs::dacIndex, int, int, int,
                                std::chrono::nanoseconds>());
    scanParameters.def_readwrite("enable", &sp::enable);
    scanParameters.def_readwrite("dacInd", &sp::dacInd);
    scanParameters.def_readwrite("startOffset", &sp::startOffset);
    scanParameters.def_readwrite("stopOffset", &sp::stopOffset);
    scanParameters.def_readwrite("stepSize", &sp::stepSize);
    scanParameters.def_readwrite("dacSettleTime_ns", &sp::dacSettleTime_ns);
    scanParameters.def("__repr__", [](const sp &a) {
        std::ostringstream oss;
        auto indent = "  ";
        oss << "<scanParameters>\n";
        oss << indent << "enable: " << a.enable << '\n';
        oss << indent << "dacInd: " << a.dacInd << '\n';
        oss << indent << "startOffset: " << a.startOffset << '\n';
        oss << indent << "stopOffset: " << a.stopOffset << '\n';
        oss << indent << "stepSize: " << a.stepSize << '\n';
        oss << indent << "dacSettleTime_ns: " << a.dacSettleTime_ns;
        return oss.str();
    });
}
