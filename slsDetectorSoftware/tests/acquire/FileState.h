// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/Detector.h"
#include "sls/logger.h"

#include <string>

namespace sls::test::acquire {

struct FileState {
    std::string file_path;
    std::string file_prefix;
    int64_t file_acq_index;
    bool file_write;
    bool file_overwrite;
    slsDetectorDefs::fileFormat file_format;
};

inline FileState default_file_state() {
    return {"/tmp", "sls_test", 0, true, true, slsDetectorDefs::BINARY};
}

inline FileState get_file_state(const Detector &det) {
    return FileState{
        det.getFilePath().tsquash("Inconsistent file path"),
        det.getFileNamePrefix().tsquash("Inconsistent file prefix"),
        det.getAcquisitionIndex().tsquash(
            "Inconsistent file acquisition index"),
        det.getFileWrite().tsquash("Inconsistent file write"),
        det.getFileOverWrite().tsquash("Inconsistent file overwrite"),
        det.getFileFormat().tsquash("Inconsistent file format")};
}

inline void set_file_state(Detector &det, const FileState &s) {
    if (!s.file_path.empty())
        det.setFilePath(s.file_path);
    det.setFileNamePrefix(s.file_prefix);
    det.setAcquisitionIndex(s.file_acq_index);
    det.setFileWrite(s.file_write);
    det.setFileOverWrite(s.file_overwrite);
    det.setFileFormat(s.file_format);
}

inline std::string
get_master_file_name(const FileState &s = default_file_state()) {
    auto master_file_prefix = s.file_path + "/" + s.file_prefix + "_master_" +
                              std::to_string(s.file_acq_index);
    if (s.file_format == defs::BINARY)
        return master_file_prefix + ".json";
    return master_file_prefix + ".h5";
}

inline std::string
get_virtual_file_name(const FileState &s = default_file_state()) {
    return s.file_path + "/" + s.file_prefix + "_virtual_" +
           std::to_string(s.file_acq_index) + ".h5";
}

inline std::string
get_first_port_first_file_name(const FileState &s = default_file_state()) {
    auto file_prefix = s.file_path + "/" + s.file_prefix + "_d0_f0_" +
                       std::to_string(s.file_acq_index);
    if (s.file_format == defs::BINARY)
        return file_prefix + ".raw";
    return file_prefix + ".h5";
}

inline std::ostream &operator<<(std::ostream &os, const FileState &s) {
    os << "File State:"
       << "\n  Path: " << s.file_path << "\n  Prefix: " << s.file_prefix
       << "\n  Acq Index: " << s.file_acq_index << "\n  Write: " << s.file_write
       << "\n  Overwrite: " << s.file_overwrite
       << "\n  Format: " << ToString(s.file_format)
       << "\n  Master File: " << get_master_file_name(s)
       << "\n  Virtual File: " << get_virtual_file_name(s);
    return os;
}

/**
 * @brief RAII guard for restoring the file state of a detector.
 *
 * The constructor saves the current file state and sets a new state, while the
 * destructor restores the original state.
 */
class FileStateGuard {
  public:
    explicit FileStateGuard(Detector &det, const FileState &new_state)
        : det(det), saved_(get_file_state(det)) {
        set_file_state(det, new_state);
    }
    ~FileStateGuard() { set_file_state(det, saved_); }

  private:
    Detector &det;
    FileState saved_;
};

} // namespace sls::test::acquire