// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <datetime.h>
#include <pybind11/pybind11.h>

#include "DurationWrapper.h"
#include "sls/Result.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;
namespace pybind11 {
namespace detail {
template <typename Type, typename Alloc>
struct type_caster<sls::Result<Type, Alloc>>
    : list_caster<sls::Result<Type, Alloc>, Type> {};

// Based on the typecaster in pybind11/chrono.h
template <> struct type_caster<std::chrono::nanoseconds> {
  public:
    PYBIND11_TYPE_CASTER(std::chrono::nanoseconds,
                         const_name("DurationWrapper"));

    // signed 25 bits required by the standard.
    using days = std::chrono::duration<int_least32_t, std::ratio<86400>>;

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into
     * std::chrono::nanoseconds try datetime.timedelta, floats and our
     * DurationWrapper wrapper
     */

    bool load(handle src, bool) {
        using namespace std::chrono;

        // Lazy initialise the PyDateTime import
        if (!PyDateTimeAPI) {
            PyDateTime_IMPORT;
        }

        if (!src) {
            return false;
        }
        // If invoked with datetime.delta object, same as in chrono.h
        if (PyDelta_Check(src.ptr())) {
            value = duration_cast<nanoseconds>(
                days(PyDateTime_DELTA_GET_DAYS(src.ptr())) +
                seconds(PyDateTime_DELTA_GET_SECONDS(src.ptr())) +
                microseconds(PyDateTime_DELTA_GET_MICROSECONDS(src.ptr()))

            );
            return true;
        }
        // If invoked with a float we assume it is seconds and convert, same as
        // in chrono.h
        if (PyFloat_Check(src.ptr())) {
            value = duration_cast<nanoseconds>(
                duration<double>(PyFloat_AsDouble(src.ptr())));
            return true;
        }
        // If invoked with an int we assume it is nanoseconds and convert, same
        // as in chrono.h
        if (PyLong_Check(src.ptr())) {
            value = duration_cast<nanoseconds>(
                duration<int64_t>(PyLong_AsLongLong(src.ptr())));
            return true;
        }

        // Lastly if we were actually called with a DurationWrapper object we
        // get the number of nanoseconds and create a std::chrono::nanoseconds
        // from it
        py::object py_cls =
            py::module::import("slsdet._slsdet").attr("DurationWrapper");
        if (py::isinstance(src, py_cls)) {
            sls::DurationWrapper *cls = src.cast<sls::DurationWrapper *>();
            value = nanoseconds(cls->count());
            return true;
        }

        return false;
    }

    /**
     * Conversion part 2 (C++ -> Python)
     * import the module to get a handle to the wrapped class
     * Default construct an object of (wrapped) DurationWrapper
     * set the count from chrono::nanoseconds and return
     */
    static handle cast(std::chrono::nanoseconds src,
                       return_value_policy /* policy */, handle /* parent */) {
        py::object py_cls =
            py::module::import("slsdet._slsdet").attr("DurationWrapper");
        py::object *obj = new py::object;
        *obj = py_cls();
        sls::DurationWrapper *dur = obj->cast<sls::DurationWrapper *>();
        dur->set_count(src.count());
        return *obj;
    }
};

// Type caster for sls::defs::ROI from tuple
template <> struct type_caster<sls::defs::ROI> {
    PYBIND11_TYPE_CASTER(sls::defs::ROI, _("Sequence[int, int, int, int] or "
                                           "Sequence[int, int]"));

    // convert c++ ROI to python tuple
    static handle cast(const sls::defs::ROI &roi, return_value_policy, handle) {
        return py::make_tuple(roi.xmin, roi.xmax, roi.ymin, roi.ymax).release();
    }

    // convert from python to c++ ROI
    bool load(handle roi, bool /*allow implicit conversion*/) {

        // accept tuple, list, numpy array any sequence
        py::sequence seq;
        try {
            seq = py::reinterpret_borrow<py::sequence>(roi);
        } catch (...) {
            return false;
        }

        if (seq.size() != 4 && seq.size() != 2)
            return false;
        // Check if each element is an int
        for (auto item : seq) {
            if (!py::isinstance<py::int_>(item)) {
                return false;
            }
        }

        value.xmin = seq[0].cast<int>();
        value.xmax = seq[1].cast<int>();

        if (seq.size() == 4) {
            value.ymin = seq[2].cast<int>();
            value.ymax = seq[3].cast<int>();
        }

        return true;
    }
};

} // namespace detail
} // namespace pybind11