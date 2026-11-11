// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <iosfwd>
#include <stdint.h>
#include <string>

namespace sls {

struct SemanticVersion {
    int major{0};
    int minor{0};
    int patch{0};
    std::string prerelease{};

    std::string toString() const;
    bool operator==(const SemanticVersion &other) const;
    bool operator<=(const SemanticVersion &other) const;
};

class Version {
  private:
    /** Note: "0.0.0" is considered not semantic versioning here */
    bool hasSemanticVersioning_{false};
    SemanticVersion version_{};
    std::string date_;
    SemanticVersion parseSemanticVersion(const std::string &version);

  public:
    explicit Version(const std::string &s);
    std::string toString() const;
    bool hasSemanticVersioning() const;
    std::string getVersion() const;
    std::string getDate() const;
    std::string concise() const;

    // throws if not semantic versioning
    SemanticVersion getSemanticVersion() const;
    int getMajorVersion() const;
    int getMinorVersion() const;
    int getPatchVersion() const;
    std::string getPrereleaseVersion() const;
    bool isBackwardCompatible(const Version &other) const;

    bool operator!=(const Version &other) const;
    bool operator==(const Version &other) const;
    bool operator<=(const Version &other) const;
};

std::ostream &operator<<(std::ostream &out, const SemanticVersion &v);
std::ostream &operator<<(std::ostream &out, const Version &v);
} // namespace sls
