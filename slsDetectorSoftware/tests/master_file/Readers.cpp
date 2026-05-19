// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Readers.h"

#include "sls/ToString.h"

namespace sls::test::master_file {

/* --scalar reads-- */

/** std::string */
void read_from_json(const Document &doc, const std::string &name,
                    std::string &out) {
    out = doc[name.c_str()].GetString();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::string &out) {
    ds.read(out, ds.getStrType());
}
#endif

/** int */
void read_from_json(const Document &doc, const std::string &name, int &out) {
    out = doc[name.c_str()].GetInt();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          int &out) {
    ds.read(&out, H5::PredType::NATIVE_INT);
}
#endif

/** uint32_t */
void read_from_json(const Document &doc, const std::string &name,
                    uint32_t &out) {
    out = doc[name.c_str()].GetUint();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          uint32_t &out) {
    ds.read(&out, H5::PredType::STD_U32LE);
}
#endif

/** uint64_t */
void read_from_json(const Document &doc, const std::string &name,
                    uint64_t &out) {
    out = doc[name.c_str()].GetUint64();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          uint64_t &out) {
    ds.read(&out, H5::PredType::STD_U64LE);
}
#endif

/** double */
void read_from_json(const Document &doc, const std::string &name, double &out) {
    out = doc[name.c_str()].GetDouble();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          double &out) {
    ds.read(&out, H5::PredType::NATIVE_DOUBLE);
}
#endif

/** vector<string> */
/** std::vector<int64_t> */
void read_from_json(const Document &doc, const std::string &name,
                    std::vector<int64_t> &out) {
    for (const auto &item : doc[name.c_str()].GetArray()) {
        out.push_back(item.GetInt64());
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::vector<int64_t> &out) {
    H5::DataSpace dataspace = ds.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    out.resize(dims[0]);
    ds.read(out.data(), H5::PredType::STD_I64LE);
}
#endif

/* ---arrays/ maps--- */
/** std::array<int, 3UL> */
void read_from_json(const Document &doc, const std::string &name,
                    std::array<int, 3UL> &out) {
    const auto &json_values = doc[name.c_str()].GetArray();
    auto len = json_values.Size();
    if (len != 3) {
        throw sls::RuntimeError("JSON array " + name + " has " +
                                std::to_string(len) +
                                " elements instead of 3.");
    }
    int index = 0;
    for (const auto &item : json_values) {
        out[index++] = item.GetInt();
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::array<int, 3UL> &out) {
    H5::DataSpace dataspace = ds.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    auto len = dims[0];
    if (len != 3) {
        throw sls::RuntimeError("HDF5 dataset " + name + " has " +
                                std::to_string(len) +
                                " elements instead of 3.");
    }
    ds.read(out.data(), H5::PredType::NATIVE_INT);
}
#endif

/* std::array<ns, 3UL> */
void read_from_json(const Document &doc, const std::string &name,
                    std::array<ns, 3UL> &out) {
    const auto &json_values = doc[name.c_str()].GetArray();
    auto len = json_values.Size();
    if (len != 3) {
        throw sls::RuntimeError("JSON array " + name + " has " +
                                std::to_string(len) +
                                " elements instead of 3.");
    }
    int index = 0;
    for (const auto &item : json_values) {
        std::string sval = item.GetString();
        out[index++] = StringTo<ns>(sval);
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::array<ns, 3UL> &out) {
    H5::DataSpace dataspace = ds.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    auto len = dims[0];
    if (len != 3) {
        throw sls::RuntimeError("HDF5 dataset " + name + " has " +
                                std::to_string(len) +
                                " elements instead of 3.");
    }
    std::vector<const char *> strValues(dims[0]);
    ds.read(strValues.data(), ds.getStrType());
    for (size_t i = 0; i < dims[0]; ++i) {
        out[i] = StringTo<ns>(strValues[i]);
    }
}
#endif

/** std::map<std::string, std::string> */
void read_from_json(const Document &doc, const std::string &name,
                    std::map<std::string, std::string> &out) {
    for (const auto &m : doc[name.c_str()].GetObject()) {
        out[m.name.GetString()] = m.value.GetString();
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::map<std::string, std::string> &out) {
    H5::DataSpace dataspace = ds.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    if (dims[0] == 0) {
        return; // empty ds
    }
    auto strType = ds.getStrType();
    H5::CompType mapType(sizeof(char *) * 2);
    mapType.insertMember("key", 0, strType);
    mapType.insertMember("value", sizeof(char *), strType);
    struct KeyValue {
        const char *key;
        const char *value;
    };
    std::vector<KeyValue> kv_vector(dims[0]);
    ds.read(kv_vector.data(), mapType);
    for (const auto &kv : kv_vector) {
        out[kv.key] = kv.value;
    }
}
#endif

/** complex types */
/* std::vector<ROI> */
void read_from_json(const Document &doc, const std::string &name,
                    std::vector<defs::ROI> &out) {
    for (const auto &item : doc[name.c_str()].GetArray()) {
        defs::ROI r{};
        r.xmin = item["xmin"].GetInt();
        r.xmax = item["xmax"].GetInt();
        r.ymin = item["ymin"].GetInt();
        r.ymax = item["ymax"].GetInt();
        out.push_back(r);
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          std::vector<defs::ROI> &out) {
    H5::DataSpace space = ds.getSpace();
    hsize_t dims[1];
    space.getSimpleExtentDims(dims);

    H5::CompType type(sizeof(defs::ROI));
    type.insertMember("xmin", HOFFSET(defs::ROI, xmin),
                      H5::PredType::NATIVE_INT);
    type.insertMember("xmax", HOFFSET(defs::ROI, xmax),
                      H5::PredType::NATIVE_INT);
    type.insertMember("ymin", HOFFSET(defs::ROI, ymin),
                      H5::PredType::NATIVE_INT);
    type.insertMember("ymax", HOFFSET(defs::ROI, ymax),
                      H5::PredType::NATIVE_INT);

    out.resize(dims[0]);
    ds.read(out.data(), type);
}
#endif

/** defs::xy */
void read_from_json(const Document &doc, const std::string &name,
                    defs::xy &out) {
    out.x = doc[name.c_str()]["x"].GetInt();
    out.y = doc[name.c_str()]["y"].GetInt();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          defs::xy &out) {
    H5::CompType type(sizeof(defs::xy));
    type.insertMember("x", HOFFSET(defs::xy, x), H5::PredType::NATIVE_INT);
    type.insertMember("y", HOFFSET(defs::xy, y), H5::PredType::NATIVE_INT);
    ds.read(&out, type);
}
#endif

/** defs::scanParameters */
void read_from_json(const Document &doc, const std::string &name,
                    defs::scanParameters &out) {
    const auto &s = doc[name.c_str()].GetObject();
    out.enable = s["enable"].GetInt();
    out.dacInd = static_cast<defs::dacIndex>(s["dacInd"].GetInt());
    out.startOffset = s["start offset"].GetInt();
    out.stopOffset = s["stop offset"].GetInt();
    out.stepSize = s["step size"].GetInt();
    out.dacSettleTime_ns = s["dac settle time ns"].GetInt64();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &ds, const std::string &name,
                          defs::scanParameters &out) {
    H5::CompType cType(sizeof(defs::scanParameters));
    cType.insertMember("enable", HOFFSET(defs::scanParameters, enable),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("dacInd", HOFFSET(defs::scanParameters, dacInd),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("startOffset",
                       HOFFSET(defs::scanParameters, startOffset),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("stopOffset", HOFFSET(defs::scanParameters, stopOffset),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("stepSize", HOFFSET(defs::scanParameters, stepSize),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("dacSettleTime_ns",
                       HOFFSET(defs::scanParameters, dacSettleTime_ns),
                       H5::PredType::STD_I64LE);
    ds.read(&out, cType);
}
#endif

} // namespace sls::test::master_file
