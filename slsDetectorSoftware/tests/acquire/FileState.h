// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/Detector.h"

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
    return {
        "/tmp",
        "sls_test",
        0,
        true,
        true,
        slsDetectorDefs::BINARY
    };
}

inline FileState get_file_state(const Detector& det) {
    return FileState{
        det.getFilePath().tsquash("Inconsistent file path"),
        det.getFileNamePrefix().tsquash("Inconsistent file prefix"),
        det.getAcquisitionIndex().tsquash("Inconsistent file acquisition index"),
        det.getFileWrite().tsquash("Inconsistent file write"),
        det.getFileOverWrite().tsquash("Inconsistent file overwrite"),
        det.getFileFormat().tsquash("Inconsistent file format")
    };
}

inline void set_file_state(Detector& det, const FileState& s) {
    det.setFilePath(s.file_path);
    det.setFileNamePrefix(s.file_prefix);
    det.setAcquisitionIndex(s.file_acq_index);
    det.setFileWrite(s.file_write);
    det.setFileOverWrite(s.file_overwrite);
    det.setFileFormat(s.file_format);
}

class FileStateGuard {
public:
    explicit FileStateGuard(Detector& det) : det(det), saved_(get_file_state(det)) {}
    ~FileStateGuard() {
        set_file_state(det, saved_);
    }

private:
    Detector& det;
    FileState saved_;
};


} // namespace sls::test::acquire