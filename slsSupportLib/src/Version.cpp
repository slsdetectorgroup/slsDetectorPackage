// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/Version.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/string_utils.h"
#include <sstream>

namespace sls {

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
        // release version
        std::string otherVersion = other.getVersion();
        if (version_ == otherVersion)
            return true;
        // less than
        return (std::lexicographical_compare(version_.begin(), version_.end(),
                                             otherVersion.begin(),
                                             otherVersion.end()));
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
