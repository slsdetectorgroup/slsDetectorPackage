// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/Version.h"

namespace sls {

TEST_CASE("check if version is semantic", "[.version]") {

    auto [version_string, has_semantic_version] =
        GENERATE(std::make_tuple("developer 0x250512", false),
                 std::make_tuple("0.0.0 0x250512", false));

    Version version(version_string);

    CHECK(version.hasSemanticVersioning() == has_semantic_version);
}

} // namespace sls
