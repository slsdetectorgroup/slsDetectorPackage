#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"
#include "Result.h"
#include "mythenFileIO.h"
#include <chrono>
#include <vector>

#include "typecaster.h"


using ds = std::chrono::duration<double>;

namespace py = pybind11;
void init_enums(py::module &);
void init_experimental(py::module &);
void init_det(py::module &);
void init_network(py::module &);
PYBIND11_MODULE(_sls_detector, m) {
    m.doc() = R"pbdoc(
        C/C++ API
        -----------------------
        .. warning ::

            This is the compiled c extension. You probably want to look at the
            interface provided by sls instead.

    )pbdoc";

     init_enums(m);
     init_det(m);
     init_network(m);
    //  init_experimental(m);
    

    py::module io = m.def_submodule("io", "Submodule for io");
    io.def("read_my302_file", &read_my302_file, "some");

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
