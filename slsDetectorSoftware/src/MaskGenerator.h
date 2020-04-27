#pragma once
#include "Container3.h"
#include <initializer_list>
#include <vector>

namespace sls{

class MaskGenerator {
    enum class OperationMode { ALL, ROW, SINGLE };

    OperationMode mode_{OperationMode::ALL};
    std::vector<int> idx0; // hack
    size_t x_{0};
    size_t y_{0};
    size_t z_{0};

  public:
    MaskGenerator(){};
    explicit MaskGenerator(size_t i) : mode_(OperationMode::SINGLE), x_(i) {}
    MaskGenerator(size_t i, size_t j)
        : mode_(OperationMode::SINGLE), x_(i), y_(j) {}
    MaskGenerator(size_t i, size_t j, size_t k)
        : mode_(OperationMode::SINGLE), x_(i), y_(j), z_(k) {}

    explicit MaskGenerator(const std::vector<int> vec)
        : mode_(OperationMode::ROW), idx0(vec) {}

    explicit MaskGenerator(const std::vector<int> vec, size_t y)
        : mode_(OperationMode::ROW), idx0(vec), y_(y) {}

    template <typename T> Container3<bool> mask(const Container3<T>& cont) {
        return mask(cont.shape());
    }
    Container3<bool> mask(std::array<size_t, 3> shape) {
        Container3<bool> m(shape);
        if (idx0.size()== 1 && idx0[0] == -1){
            idx0.resize(m.size(0));
            std::iota(begin(idx0), end(idx0), 0);
        }
        switch (mode_) {
        case OperationMode::ALL:
            for (auto &item : m)
                item = true;
            break;
        case OperationMode::ROW:
            for (auto i : idx0) {
                m(i, 0, 0) = true;
            }
            break;
        case OperationMode::SINGLE:
            m(x_, y_, z_) = true;
            break;
        }
        return m;
    }
    Container3<bool> mask() { return {}; }
};

} // namespace sls