// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Readers.h"

#ifdef HDF5C

namespace sls::test::master_file {

template <>
struct Reader<H5Context, int> {

    static int read(const H5Context& ctx,
                    const std::string& name) {

        auto ds = ctx.file.openDataSet(HDF5_GROUP + name);
        int out{};
        ds.read(&out, H5::PredType::NATIVE_INT);
        return out;
    }
};

} // namespace sls::test::master_file

#endif