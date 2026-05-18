// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/sls_detector_defs.h"

#ifdef HDF5C
#include "H5Cpp.h"
#endif

#include <array>
#include <map>
#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace sls::test::master_file {
using ns = std::chrono::nanoseconds;
using rapidjson::Document;

/*
struct JsonDoc {
    const rapidjson::Document &doc;
};
#ifdef HDF5C
struct H5Doc {
    const H5::DataSet& ds;
};
#endif

template <typename doc, typename T>
T read(const doc& b, const std::string& name);
*/

/** ---scalar reads--- */
/** std::string */
/*
template <>
inline std::string read<JsonDoc, std::string>(
    const JsonDoc& b,
    const std::string& name)
{
        return b.doc[name.c_str()].GetString();
}
#ifdef HDF5C
template <>
inline std::string read<H5Doc, std::string>(
    const H5Doc& b,
    const std::string& name)
{
    std::string out;
    b.ds.read(out, ds.getStrType());
    return out;
}
#endif
*/
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
