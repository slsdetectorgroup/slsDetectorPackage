// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Readers.h"

namespace sls::test::master_file {

/* --scalar reads-- */
template <>
struct Reader<JsonContext, int> {
    static int read(const JsonContext &ctx, const std::string &name) {
        return ctx.doc[name.c_str()].GetInt();
    }
};

template<>
struct Reader<JsonContext, uint32_t> {

    static uint32_t read(const JsonContext& ctx,
                         const std::string& name) {

        return ctx.doc[name.c_str()].GetUint();

    }
};

template<>
struct Reader<JsonContext, uint64_t> {

    static uint64_t read(const JsonContext& ctx,
                         const std::string& name) {

        return ctx.doc[name.c_str()].GetUint64();
    }
};

template<>
struct Reader<JsonContext, double> {

    static double read(const JsonContext& ctx,
                       const std::string& name) {

        return ctx.doc[name.c_str()].GetDouble();
    }
};

template<>
struct Reader<JsonContext, std::string> {

    static std::string read(const JsonContext& ctx,
                            const std::string& name) {

        return ctx.doc[name.c_str()].GetString();
    }
};

} // namespace sls::test::master_file