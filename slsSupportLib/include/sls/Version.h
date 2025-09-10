// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <stdint.h>
#include <string>

namespace sls {

class Version {
  private:
    std::string version_;
    std::string date_;
    inline static const std::string defaultVersion_[] = {"developer", "0.0.0"};

  public:
    explicit Version(const std::string &s);

    bool hasSemanticVersioning() const;
    std::string getVersion() const;
    std::string getDate() const;
    std::string concise() const;
    int getMajorVersion() const;

    // expects semantic versioning
    bool isBackwardCompatible(const Version &other) const;
    bool operator!=(const Version &other) const;
    bool operator==(const Version &other) const;
    bool operator<=(const Version &other) const;
};

std::ostream &operator<<(std::ostream &out, const Version &v);
} // namespace sls
