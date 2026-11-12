// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/Version.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/string_utils.h"
#include <cstdio>
#include <sstream>

namespace sls {

std::string SemanticVersion::toString() const {
    std::ostringstream oss;
    oss << major << "." << minor << "." << patch;
    if (!prerelease.empty()) {
        oss << prerelease;
    }
    return oss.str();
}

bool SemanticVersion::operator==(const SemanticVersion &other) const {
    return major == other.major && minor == other.minor &&
           patch == other.patch && prerelease == other.prerelease;
}

bool SemanticVersion::operator<=(const SemanticVersion &other) const {
    if (major != other.major)
        return major < other.major;
    if (minor != other.minor)
        return minor < other.minor;
    if (patch != other.patch)
        return patch < other.patch;

    // A release version is greater than its prerelease
    if (prerelease.empty() && !other.prerelease.empty())
        return false;
    if (!prerelease.empty() && other.prerelease.empty())
        return true;
    return prerelease <= other.prerelease;
}

SemanticVersion Version::parseSemanticVersion(const std::string &version) {
    SemanticVersion ver;

    // assuming only released versions have semantic versioning
    if (version == "developer" || version == "0.0.0") {
        return ver;
    }

    char prereleaseBuffer[256] = {'\0'};
    // Parse major.minor.patch and capture any optional trailing suffix.
    const int scanned = sscanf(version.c_str(), "%d.%d.%d%255s", &ver.major,
                               &ver.minor, &ver.patch, prereleaseBuffer);

    if (scanned == 3 || scanned == 4) {
        hasSemanticVersioning_ = true;
        if (scanned == 4)
            ver.prerelease = prereleaseBuffer;
        else
            ver.prerelease.clear();
    }
    return ver;
}

Version::Version(const std::string &s) {
    auto list = split(s, ' ');

    // only date from previous releases
    if (list.size() == 1) {
        date_ = list[0];
    }

    // semantic versioning + date
    else {
        auto version = list[0];
        date_ = list[1];
        version_ = parseSemanticVersion(version);
    }
}

std::string Version::toString() const {
    std::ostringstream oss;
    oss << version_ << " " << getDate();
    return oss.str();
}

bool Version::hasSemanticVersioning() const { return hasSemanticVersioning_; }

std::string Version::getVersion() const { return version_.toString(); }

std::string Version::getDate() const { return date_; }

std::string Version::concise() const {
    if (hasSemanticVersioning_)
        return version_.toString();
    return date_;
}

SemanticVersion Version::getSemanticVersion() const {
    if (!hasSemanticVersioning_) {
        throw sls::RuntimeError("No semantic versioning available for " +
                                toString());
    }
    return version_;
}

int Version::getMajorVersion() const {
    if (!hasSemanticVersioning_) {
        throw sls::RuntimeError("Could not get major version from " +
                                toString());
    }
    return version_.major;
}

int Version::getMinorVersion() const {
    if (!hasSemanticVersioning_) {
        throw sls::RuntimeError("Could not get minor version from " +
                                toString());
    }
    return version_.minor;
}

int Version::getPatchVersion() const {
    if (!hasSemanticVersioning_) {
        throw sls::RuntimeError("Could not get patch version from " +
                                toString());
    }
    return version_.patch;
}

std::string Version::getPrereleaseVersion() const {
    if (!hasSemanticVersioning_) {
        throw sls::RuntimeError("Could not get prerelease version from " +
                                toString());
    }
    return version_.prerelease;
}

bool Version::isBackwardCompatible(const Version &other) const {
    if (!hasSemanticVersioning_ || !other.hasSemanticVersioning()) {
        throw sls::RuntimeError("Cannot check backward compatibility without "
                                "semantic versioning for " +
                                toString() + " and " + other.toString());
    }
    return getMajorVersion() == other.getMajorVersion();
}

bool Version::operator==(const Version &other) const {
    if (hasSemanticVersioning() && other.hasSemanticVersioning()) {
        return version_ == other.getSemanticVersion();
    }
    return date_ == other.getDate();
}

bool Version::operator!=(const Version &other) const {
    return !(*this == other);
}

bool Version::operator<=(const Version &other) const {
    if (hasSemanticVersioning() && other.hasSemanticVersioning()) {
        return version_ <= other.getSemanticVersion();
    }
    return date_ <= other.getDate();
}

std::ostream &operator<<(std::ostream &out, const SemanticVersion &v) {
    return out << v.toString();
}

std::ostream &operator<<(std::ostream &out, const Version &v) {
    return out << v.toString();
}

} // namespace sls
