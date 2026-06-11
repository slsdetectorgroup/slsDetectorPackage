// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Readers.h"
#include "sls/sls_detector_defs.h"
#include <iostream>

#ifdef HDF5C
#include "H5Cpp.h"

namespace sls::test::master_file {
using ns = std::chrono::nanoseconds;

static const std::string HDF5_GROUP = "/entry/instrument/detector/";

/* --safety checks-- */
inline void require_dataset(const H5Context &ctx, const std::string &name) {
    const std::string full_name = HDF5_GROUP + name;
    if (H5Lexists(ctx.file.getId(), full_name.c_str(), H5P_DEFAULT) <= 0) {
        throw sls::RuntimeError("Missing HDF5 dataset: " + full_name);
    }
}

inline void require_file_attribute(const H5Context &ctx,
                                   const std::string &name) {
    if (!H5Aexists_by_name(ctx.file.getId(), "/", name.c_str(), H5P_DEFAULT)) {
        throw sls::RuntimeError("Missing HDF5 attribute: " + name);
    }
}

/* --scalar reads-- */
template <> struct Reader<H5Context, int> {
    static int read(const H5Context &ctx, const std::string &name,
                    AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError("int attribute access not supported for HDF5");
        }
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        int out{};
        ds.read(&out, H5::PredType::NATIVE_INT);
        return out;
    }
};

template <> struct Reader<H5Context, uint32_t> {
    static uint32_t read(const H5Context &ctx, const std::string &name,
                         AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError(
                "uint32_t attribute access not supported for HDF5");
        }
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        uint32_t out{};
        ds.read(&out, H5::PredType::STD_U32LE);
        return out;
    }
};

template <> struct Reader<H5Context, uint64_t> {
    static uint64_t read(const H5Context &ctx, const std::string &name,
                         AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError(
                "'uint64_t' attribute access not supported for HDF5");
        }
        uint64_t out{};
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        ds.read(&out, H5::PredType::STD_U64LE);
        return out;
    }
};

template <> struct Reader<H5Context, double> {
    static double read(const H5Context &ctx, const std::string &name,
                       AccessType access) {
        double out{};
        if (access == AccessType::Attribute) {
            require_file_attribute(ctx, name);
            auto attr = ctx.file.openAttribute(name);
            attr.read(attr.getDataType(), &out);
            return out;
        }
        // dataset
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        ds.read(&out, H5::PredType::NATIVE_DOUBLE);
        return out;
    }
};

template <> struct Reader<H5Context, std::string> {
    static std::string read(const H5Context &ctx, const std::string &name,
                            AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError(
                "string attribute access not supported for HDF5");
        }
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        std::string out{};
        ds.read(out, ds.getStrType());
        return out;
    }
};

/** complex types */
template <> struct Reader<H5Context, defs::xy> {
    static defs::xy read(const H5Context &ctx, const std::string &name,
                         AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError(
                "'defs::xy' attribute access not supported for HDF5");
        }
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        // define type
        H5::CompType type(sizeof(defs::xy));
        type.insertMember("x", HOFFSET(defs::xy, x), H5::PredType::NATIVE_INT);
        type.insertMember("y", HOFFSET(defs::xy, y), H5::PredType::NATIVE_INT);
        // read
        defs::xy out{};
        ds.read(&out, type);
        return out;
    }
};

inline hsize_t get_1d_size(const H5::DataSet &ds) {
    H5::DataSpace space = ds.getSpace();
    hsize_t dims[1];
    space.getSimpleExtentDims(dims);
    return dims[0];
}

template <> struct Reader<H5Context, std::vector<defs::ROI>> {
    static std::vector<defs::ROI>
    read(const H5Context &ctx, const std::string &name, AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError("'std::vector<defs::ROI>' attribute access not "
                               "supported for HDF5");
        }
        require_dataset(ctx, name);
        // define type
        H5::CompType type(sizeof(defs::ROI));
        type.insertMember("xmin", HOFFSET(defs::ROI, xmin),
                          H5::PredType::NATIVE_INT);
        type.insertMember("xmax", HOFFSET(defs::ROI, xmax),
                          H5::PredType::NATIVE_INT);
        type.insertMember("ymin", HOFFSET(defs::ROI, ymin),
                          H5::PredType::NATIVE_INT);
        type.insertMember("ymax", HOFFSET(defs::ROI, ymax),
                          H5::PredType::NATIVE_INT);
        // read
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        auto len = get_1d_size(ds);
        std::vector<defs::ROI> out{};
        out.resize(len);
        ds.read(out.data(), type);
        return out;
    }
};

template <> struct Reader<H5Context, defs::scanParameters> {
    static defs::scanParameters
    read(const H5Context &ctx, const std::string &name, AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError("'defs::scanParameters' attribute access not "
                               "supported for HDF5");
        }
        require_dataset(ctx, name);
        // define type
        H5::CompType type(sizeof(defs::scanParameters));
        type.insertMember("enable", HOFFSET(defs::scanParameters, enable),
                          H5::PredType::NATIVE_INT);
        type.insertMember("dacInd", HOFFSET(defs::scanParameters, dacInd),
                          H5::PredType::NATIVE_INT);
        type.insertMember("startOffset",
                          HOFFSET(defs::scanParameters, startOffset),
                          H5::PredType::NATIVE_INT);
        type.insertMember("stopOffset",
                          HOFFSET(defs::scanParameters, stopOffset),
                          H5::PredType::NATIVE_INT);
        type.insertMember("stepSize", HOFFSET(defs::scanParameters, stepSize),
                          H5::PredType::NATIVE_INT);
        type.insertMember("dacSettleTime_ns",
                          HOFFSET(defs::scanParameters, dacSettleTime_ns),
                          H5::PredType::STD_I64LE);
        // read
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        defs::scanParameters out{};
        ds.read(&out, type);
        return out;
    }
};

/** arrays/vectors/maps */
template <> struct Reader<H5Context, std::array<int, 3UL>> {
    static std::array<int, 3UL>
    read(const H5Context &ctx, const std::string &name, AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError("'std::array<int, 3UL>' attribute access not "
                               "supported for HDF5");
        }
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        auto len = get_1d_size(ds);
        check_size(len, 3, name, "HDF5");
        std::array<int, 3UL> out{};
        ds.read(out.data(), H5::PredType::NATIVE_INT);
        return out;
    }
};

template <> struct Reader<H5Context, std::array<ns, 3UL>> {
    static std::array<ns, 3UL>
    read(const H5Context &ctx, const std::string &name, AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError("'std::array<ns, 3UL>' attribute access not "
                               "supported for HDF5");
        }
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        auto len = get_1d_size(ds);
        check_size(len, 3, name, "HDF5");

        // read int raw char buffer
        std::vector<const char *> raw(len);
        ds.read(raw.data(), ds.getStrType());

        std::array<ns, 3UL> out{};
        for (size_t i = 0; i != len; ++i) {
            out[i] = StringTo<ns>(raw[i]);
        }
        return out;
    }
};

template <> struct Reader<H5Context, std::vector<int64_t>> {
    static std::vector<int64_t>
    read(const H5Context &ctx, const std::string &name, AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError("'std::vector<int64_t>' attribute access not "
                               "supported for HDF5");
        }
        require_dataset(ctx, name);
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        auto len = get_1d_size(ds);
        std::vector<int64_t> out{};
        out.resize(len);
        ds.read(out.data(), H5::PredType::STD_I64LE);
        return out;
    }
};

template <> struct Reader<H5Context, std::map<std::string, std::string>> {
    static std::map<std::string, std::string>
    read(const H5Context &ctx, const std::string &name, AccessType access) {
        if (access == AccessType::Attribute) {
            throw RuntimeError("'std::map<std::string, std::string>' attribute "
                               "access not supported for HDF5");
        }
        require_dataset(ctx, name);
        std::map<std::string, std::string> out{};
        // dim
        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        auto len = get_1d_size(ds);
        // empty ds
        if (len == 0) {
            return out;
        }
        // define type
        auto strType = ds.getStrType();
        H5::CompType mapType(sizeof(char *) * 2);
        mapType.insertMember("key", 0, strType);
        mapType.insertMember("value", sizeof(char *), strType);
        struct KeyValue {
            const char *key;
            const char *value;
        };
        // read
        std::vector<KeyValue> kv_vector(len);
        ds.read(kv_vector.data(), mapType);
        for (const auto &kv : kv_vector) {
            out[kv.key] = kv.value;
        }
        return out;
    }
};

} // namespace sls::test::master_file

#endif