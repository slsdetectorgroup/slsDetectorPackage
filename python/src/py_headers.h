// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

/* 
Single common header file to make sure the pybind includes are the 
same and ordered in the same way in all files. Needed to avoid
ODR warnings
*/
#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "typecaster.h"
