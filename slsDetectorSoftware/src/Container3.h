#pragma once
#include <array>
#include <cstddef>
#include <iostream>
#include <numeric>

namespace sls{

template <typename T> class Container3 {
    std::array<size_t, 3> shape_{0, 0, 0};
    T *data_{nullptr};

  public:
    Container3(){};

    Container3(std::array<size_t, 3> shape)
        : Container3(shape[0], shape[1], shape[2]) {}

    Container3(std::array<size_t, 3> shape, T value)
        : Container3(shape[0], shape[1], shape[2], value) {}

    Container3(size_t x, size_t y, size_t z)
        : shape_{x, y, z}, data_(new T[size()]{}) {}

    Container3(size_t x, size_t y, size_t z, const T &value)
        : shape_{x, y, z}, data_(new T[size()]) {
        std::fill_n(data_, size(), value);
    }

    // Copy constructor
    Container3(const Container3 &other) {
        shape_ = other.shape_;
        data_ = new T[size()];
        std::copy(other.data_, other.data_ + size(), data_);
    }

    // Move constructor
    Container3(Container3 &&other) {
        shape_ = other.shape_;
        other.shape_ = {0, 0, 0};
        data_ = other.data_;
        other.data_ = nullptr;
    }

    // Copy assignment
    Container3 &operator=(const Container3 &other) {
        if (this != &other) {
            Container3<T> tmp(other);
            std::swap(shape_, tmp.shape_);
            std::swap(data_, tmp.data_);
        }
        return *this;
    }

    // Move assignment
    Container3 &operator=(Container3 &&other) {
        if (this != &other) {
            shape_ = other.shape_;
            other.shape_ = {0, 0, 0};
            delete[] data_;
            data_ = other.data_;
            other.data_ = nullptr;
        }
        return *this;
    }

    ~Container3() { delete[] data_; }

    size_t size() const noexcept {
        return std::accumulate(std::begin(shape_), std::end(shape_), 1,
                               std::multiplies<size_t>());
    }
    size_t size(size_t i) const noexcept { return shape_[i]; }

    std::array<size_t, 3> shape() const noexcept { return shape_; }

    T *data() noexcept { return data_; }
    const T *data() const noexcept { return data_; }

    bool is_valid_index(size_t x, size_t y, size_t z) const noexcept {
        return x < shape_[0] && y < shape_[1] && z < shape_[2];
    }

    // Will truncate if other is larger in any dimension
    // In the future move object out of other, rename function
    void copy_data(const Container3 &other) {
        for (size_t i = 0; i < std::min(size(0), other.size(0)); ++i) {
            for (size_t j = 0; j < std::min(size(1), other.size(1)); ++j) {
                for (size_t k = 0; k < std::min(size(2), other.size(2)); ++k) {
                    (*this)(i, j, k) = other(i, j, k);
                }
            }
        }
    }

    void move_data(Container3 &other) {
        for (size_t i = 0; i < std::min(size(0), other.size(0)); ++i) {
            for (size_t j = 0; j < std::min(size(1), other.size(1)); ++j) {
                for (size_t k = 0; k < std::min(size(2), other.size(2)); ++k) {
                    (*this)(i, j, k) = std::move(other(i, j, k));
                }
            }
        }
    }

    void resize(size_t x, size_t y, size_t z) {
        Container3<T> tmp(x, y, z);
        tmp.move_data(*this);
        *this = std::move(tmp);
    }

    T &operator()(size_t x, size_t y, size_t z) noexcept {
        return data_[element_offset(x, y, z)];
    }
    const T &operator()(size_t x, size_t y, size_t z) const noexcept {
        return data_[element_offset(x, y, z)];
    }

    // throws on out of bounds access
    const T &at(size_t x, size_t y, size_t z) const {
        if (!is_valid_index(x, y, z))
            throw std::runtime_error("Index error");
        return data_[element_offset(x, y, z)];
    }

    T &at(size_t x, size_t y, size_t z) {
        return const_cast<T &>(
            static_cast<const Container3 &>(*this).at(x, y, z));
    }

    T &operator[](size_t i) { return data_[i]; }
    const T &operator[](size_t i) const { return data_[i]; }

    T *begin() { return data_; }

    T *end() { return data_ + size(); }

    void clear(){
        *this = Container3<T>();
    }

    // reference to position, grow if needed, prefer resize and .at()
    T &at_can_grow(size_t x, size_t y, size_t z) {
        if (!is_valid_index(x, y, z)) {
            resize(std::max(x + 1, size(0)), std::max(y + 1, size(1)),
                   std::max(z + 1, size(2)));
        }
        return data_[element_offset(x, y, z)];
    }

  private:
    size_t element_offset(size_t x, size_t y, size_t z) const noexcept {
        return x * shape_[1] * shape_[2] + y * shape_[2] + z;
    }
};

} // namespace sls