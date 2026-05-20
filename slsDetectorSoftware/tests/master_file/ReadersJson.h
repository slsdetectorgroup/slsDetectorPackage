// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Readers.h"

#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

#include <rapidjson/document.h>

namespace sls::test::master_file {
using ns = std::chrono::nanoseconds;

/* --scalar reads-- */
template <> struct Reader<JsonContext, int> {
    static int read(const JsonContext &ctx, const std::string &name,
                    AccessType access) {
        return ctx.doc[name.c_str()].GetInt();
    }
};

template <> struct Reader<JsonContext, uint32_t> {
    static uint32_t read(const JsonContext &ctx, const std::string &name,
                         AccessType access) {
        return ctx.doc[name.c_str()].GetUint();
    }
};

template <> struct Reader<JsonContext, uint64_t> {
    static uint64_t read(const JsonContext &ctx, const std::string &name,
                         AccessType access) {
        return ctx.doc[name.c_str()].GetUint64();
    }
};

template <> struct Reader<JsonContext, double> {
    static double read(const JsonContext &ctx, const std::string &name,
                       AccessType access) {
        return ctx.doc[name.c_str()].GetDouble();
    }
};

template <> struct Reader<JsonContext, std::string> {
    static std::string read(const JsonContext &ctx, const std::string &name,
                            AccessType access) {
        return ctx.doc[name.c_str()].GetString();
    }
};

/** complex types */
template <> struct Reader<JsonContext, defs::xy> {
    static defs::xy read(const JsonContext &ctx, const std::string &name,
                         AccessType access) {
        defs::xy out{};
        out.x = ctx.doc[name.c_str()]["x"].GetInt();
        out.y = ctx.doc[name.c_str()]["y"].GetInt();
        return out;
    }
};

template <> struct Reader<JsonContext, std::vector<defs::ROI>> {
    static std::vector<defs::ROI>
    read(const JsonContext &ctx, const std::string &name, AccessType access) {
        std::vector<defs::ROI> out{};
        for (const auto &item : ctx.doc[name.c_str()].GetArray()) {
            defs::ROI r{};
            r.xmin = item["xmin"].GetInt();
            r.xmax = item["xmax"].GetInt();
            r.ymin = item["ymin"].GetInt();
            r.ymax = item["ymax"].GetInt();
            out.push_back(r);
        }
        return out;
    }
};

template <> struct Reader<JsonContext, defs::scanParameters> {
    static defs::scanParameters
    read(const JsonContext &ctx, const std::string &name, AccessType access) {
        defs::scanParameters out{};
        const auto &s = ctx.doc[name.c_str()].GetObject();
        out.enable = s["enable"].GetInt();
        out.dacInd = static_cast<defs::dacIndex>(s["dacInd"].GetInt());
        out.startOffset = s["start offset"].GetInt();
        out.stopOffset = s["stop offset"].GetInt();
        out.stepSize = s["step size"].GetInt();
        out.dacSettleTime_ns = s["dac settle time ns"].GetInt64();
        return out;
    }
};

/** arrays/vectors/maps */
template <> struct Reader<JsonContext, std::array<int, 3UL>> {
    static std::array<int, 3UL>
    read(const JsonContext &ctx, const std::string &name, AccessType access) {
        const auto &arr = ctx.doc[name.c_str()].GetArray();
        check_size(arr.Size(), 3, name, "JSON");

        std::array<int, 3UL> out{};
        for (size_t i = 0; i < 3; ++i) {
            out[i] = arr[i].GetInt();
        }
        return out;
    }
};

template <> struct Reader<JsonContext, std::array<ns, 3UL>> {
    static std::array<ns, 3UL>
    read(const JsonContext &ctx, const std::string &name, AccessType access) {
        const auto &arr = ctx.doc[name.c_str()].GetArray();
        check_size(arr.Size(), 3, name, "JSON");

        std::array<ns, 3UL> out{};
        for (size_t i = 0; i < 3; ++i) {
            std::string sval = arr[i].GetString();
            out[i] = StringTo<ns>(sval);
        }
        return out;
    }
};

template <> struct Reader<JsonContext, std::vector<int64_t>> {
    static std::vector<int64_t>
    read(const JsonContext &ctx, const std::string &name, AccessType access) {
        std::vector<int64_t> out{};
        for (const auto &item : ctx.doc[name.c_str()].GetArray()) {
            out.push_back(item.GetInt64());
        }
        return out;
    }
};

template <> struct Reader<JsonContext, std::map<std::string, std::string>> {
    static std::map<std::string, std::string>
    read(const JsonContext &ctx, const std::string &name, AccessType access) {
        std::map<std::string, std::string> out{};
        for (const auto &m : ctx.doc[name.c_str()].GetObject()) {
            out[m.name.GetString()] = m.value.GetString();
        }
        return out;
    }
};

} // namespace sls::test::master_file