// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Readers.h"
#include "ReadersJson.h"
#ifdef HDF5C
#include "ReadersH5.h"
#endif

#include "catch.hpp"

namespace sls::test::master_file {

template <typename Context> class Checker;

/** JSON Specialization */
template <> class Checker<JsonContext> {
  public:
    explicit Checker(const std::string &path) : ctx_(path) {}
    explicit Checker(JsonContext ctx) : ctx_(std::move(ctx)) {}
    const JsonContext &context() const { return ctx_; }

    template <typename T>
    void check(const std::string &name, const T &expected,
               AccessType = AccessType::Dataset) const {
        CAPTURE(name);
        REQUIRE(ctx_.doc.HasMember(name.c_str()));
        auto retval = read<JsonContext, T>(ctx_, name);
        REQUIRE(retval == expected);
    }

  private:
    JsonContext ctx_;
};

/** HDF5 Specialization */
#ifdef HDF5C

template <> class Checker<H5Context> {
  public:
    explicit Checker(const std::string &path) : ctx_(path) {}
    explicit Checker(H5Context ctx) : ctx_(std::move(ctx)) {}
    const H5Context &context() const { return ctx_; }

    template <typename T>
    void check(const std::string &name, const T &expected,
               AccessType access = AccessType::Dataset) const {
        auto retval = read<H5Context, T>(ctx_, name, access);
        REQUIRE(retval == expected);
    }

  private:
    H5Context ctx_;
};

#endif

} // namespace sls::test::master_file