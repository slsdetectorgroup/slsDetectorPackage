// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/Version.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/string_utils.h"
#include <cstdio>
#include <sstream>

namespace sls {

bool Version::parseSemanticVersion(const std::string &version, int &major,
                                   int &minor, int &patch, std::string &extra) {
    char extraBuffer[256] = {'\0'};
    // Parse major.minor.patch and capture any optional trailing suffix.
    const int scanned = sscanf(version.c_str(), "%d.%d.%d%255s", &major, &minor,
                               &patch, extraBuffer);

    if (scanned == 3) {
        extra.clear();
        return true;
    }
    if (scanned == 4) {
        extra = extraBuffer;
        return true;
    }
    return false;
}

Version::Version(const std::string &s) {
    auto list = split(s, ' ');
    // only date from previous releases
    if (list.size() == 1) {
        date_ = list[0];
    }
    // semantic versioning + date
    else {
        version_ = list[0];
        date_ = list[1];
    }
}

bool Version::hasSemanticVersioning() const {

    return (version_ != defaultVersion_[0]) && (version_ != defaultVersion_[1]);
}

std::string Version::getVersion() const { return version_; }
std::string Version::getDate() const { return date_; }

std::string Version::concise() const {
    if (hasSemanticVersioning())
        return version_;
    return date_;
}

int Version::getMajorVersion() const {
    int major = 0, minor = 0, patch = 0;
    if (sscanf(version_.c_str(), "%d.%d.%d", &major, &minor, &patch) == 3) {
        return major;
    }
    throw sls::RuntimeError("Could not get major version from " + version_);
}

bool Version::isBackwardCompatible(const Version &other) const {
    return getMajorVersion() == other.getMajorVersion();
}

bool Version::operator!=(const Version &other) const {
    return !(*this == other);
}

bool Version::operator==(const Version &other) const {
    // both have semantic versioning
    if (hasSemanticVersioning() && other.hasSemanticVersioning()) {
        return version_ == other.getVersion();
    }
    // compare dates
    return date_ == other.getDate();
}

bool Version::operator<=(const Version &other) const {
    // both have semantic versioning
    if (hasSemanticVersioning() && other.hasSemanticVersioning()) {
        int major = 0, minor = 0, patch = 0;
        int otherMajor = 0, otherMinor = 0, otherPatch = 0;
        std::string extra;
        std::string otherExtra;

        if (Version::parseSemanticVersion(version_, major, minor, patch,
                                          extra) &&
            Version::parseSemanticVersion(other.getVersion(), otherMajor,
                                          otherMinor, otherPatch, otherExtra)) {
            if (major != otherMajor)
                return major < otherMajor;
            if (minor != otherMinor)
                return minor < otherMinor;
            if (patch != otherPatch)
                return patch < otherPatch;
            return extra <= otherExtra;
        }

        LOG(logWARNING) << "Could not compare semantic versions: " << version_
                        << " and " << other.getVersion()
                        << ". Comparing dates instead.";
    }
    // compare dates
    return date_ <= other.getDate();
}

std::ostream &operator<<(std::ostream &out, const Version &v) {
    std::ostringstream oss;
    oss << v.getVersion() << " " << v.getDate();
    return out << oss.str();
}

} // namespace sls
