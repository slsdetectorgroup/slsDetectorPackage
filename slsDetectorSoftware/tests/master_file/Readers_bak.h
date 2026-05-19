// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Context.h"
#include "sls/sls_detector_defs.h"

#ifdef HDF5C
#include "H5Cpp.h"
#endif

#include <rapidjson/document.h>

namespace sls::test::master_file {

using ns = std::chrono::nanoseconds;

void read_from_json(const Document &doc, const std::string &name,
                    std::string &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::string &out);
#endif

/** int */
void read_from_json(const Document &doc, const std::string &name, int &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          int &out);
#endif

/** uint32_t */
void read_from_json(const Document &doc, const std::string &name,
                    uint32_t &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          uint32_t &out);
#endif

/** uint64_t */
void read_from_json(const Document &doc, const std::string &name,
                    uint64_t &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          uint64_t &out);
#endif

/** double */
void read_from_json(const Document &doc, const std::string &name, double &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          double &out);
#endif

/* ---vector reads--- */
/** int64_t */
void read_from_json(const Document &doc, const std::string &name,
                    std::vector<int64_t> &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::vector<int64_t> &out);
#endif

/* ---arrays/ maps--- */
/** std::array<int, 3UL> */
void read_from_json(const Document &doc, const std::string &name,
                    std::array<int, 3UL> &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::array<int, 3UL> &out);
#endif

/** std::array<ns, 3UL> */
void read_from_json(const Document &doc, const std::string &name,
                    std::array<ns, 3UL> &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::array<ns, 3UL> &out);
#endif

/** std::map<std::string, std::string> */
void read_from_json(const Document &doc, const std::string &name,
                    std::map<std::string, std::string> &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::map<std::string, std::string> &out);
#endif

/* complex types */
/* std::vector<ROI> */
void read_from_json(const Document &doc, const std::string &name,
                    std::vector<defs::ROI> &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::vector<defs::ROI> &out);
#endif

/** defs::xy */
void read_from_json(const Document &doc, const std::string &name,
                    defs::xy &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          defs::xy &out);
#endif

/** defs::scanParameters */
void read_from_json(const Document &doc, const std::string &name,
                    defs::scanParameters &out);
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          defs::scanParameters &out);
#endif

} // namespace sls::test::master_file
