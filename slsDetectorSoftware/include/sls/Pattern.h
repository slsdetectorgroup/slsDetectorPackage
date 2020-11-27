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
    // loop0 start, loop0 stop .. loop2 start, loop2 stop
    uint32_t loop[6];
    uint32_t nloop[3];
    uint32_t wait[3];
    uint64_t waittime[3];
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
    constexpr size_t size() const noexcept { return sizeof(patternParameters); }
    void validate() const;
    void load(const std::string &fname);
    void save(const std::string &fname);
    std::string str() const;
};

} // namespace sls
#endif