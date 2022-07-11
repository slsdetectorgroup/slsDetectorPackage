// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"

#ifdef __cplusplus
#include <memory>
namespace sls {
#endif

// Common C/C++ structure to handle pattern data
typedef struct __attribute__((packed)) {
    uint64_t word[MAX_PATTERN_LENGTH];
    uint64_t ioctrl;
    uint32_t limits[2];
    uint32_t startloop[MAX_PATTERN_LEVELS];
    uint32_t stoploop[MAX_PATTERN_LEVELS];
    uint32_t nloop[MAX_PATTERN_LEVELS];
    uint32_t wait[MAX_PATTERN_LEVELS];
    uint64_t waittime[MAX_PATTERN_LEVELS];
} patternParameters;

#ifdef __cplusplus
class Pattern {
    patternParameters *pat = new patternParameters{};

  public:
    Pattern();
    ~Pattern();
    Pattern(const Pattern &other);
    bool operator==(const Pattern &other) const;
    bool operator!=(const Pattern &other) const;
    patternParameters *data();
    patternParameters *data() const;
    size_t size() const noexcept { return sizeof(patternParameters); }
    void validate() const;
    void load(const std::string &fname);
    void save(const std::string &fname);
    std::string str() const;
};

} // namespace sls
#endif
