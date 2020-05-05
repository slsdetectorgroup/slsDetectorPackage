#pragma once
#include "TypeTraits.h"
#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace sls {
template <typename T, size_t Capacity> class FixedCapacityContainer {

  public:
    using size_type = typename std::array<T, Capacity>::size_type;
    using value_type = typename std::array<T, Capacity>::value_type;
    using iterator = typename std::array<T, Capacity>::iterator;
    using const_iterator = typename std::array<T, Capacity>::const_iterator;

  private:
    size_type current_size{};
    std::array<T, Capacity> data_;

  public:
    FixedCapacityContainer() = default;

    explicit FixedCapacityContainer(std::initializer_list<T> l)
        : current_size(l.size()) {
        size_check(l.size());
        std::copy(l.begin(), l.end(), data_.begin());
    }

    /** Copy construct from another container */
    template <typename V,
              typename = typename std::enable_if<
                  is_container<V>::value &&
                  std::is_same<T, typename V::value_type>::value>::type>
    FixedCapacityContainer(const V &v) : current_size(v.size()) {
        size_check(v.size());
        std::copy(v.begin(), v.end(), data_.begin());
    }

    /** copy assignment from another container */
    template <typename V>
    typename std::enable_if<is_container<V>::value,
                            FixedCapacityContainer &>::type
    operator=(const V &other) {
        size_check(other.size());
        std::copy(other.begin(), other.end(), data_.begin());
        current_size = other.size();
        return *this;
    }

    /** Compare FixedCapacityContainer with any other container*/
    // template <typename V>
    // typename std::enable_if<is_container<V>::value, bool>::type
    // operator==(const V &other) const noexcept {
    //     if (current_size != other.size()) {
    //         return false;
    //     } else {
    //         for (size_t i = 0; i != current_size; ++i) {
    //             if (data_[i] != other[i]) {
    //                 return false;
    //             }
    //         }
    //     }
    //     return true;
    // }

    // template <typename V>
    // typename std::enable_if<is_container<V>::value, bool>::type
    // operator!=(const V &other) const noexcept {
    //     return !(*this == other);
    // }

    operator std::vector<T>() { return std::vector<T>(begin(), end()); }

    T &operator[](size_t i) { return data_[i]; }
    const T &operator[](size_t i) const { return data_[i]; }
    constexpr size_type size() const noexcept { return current_size; }
    bool empty() const noexcept { return current_size == 0; }
    constexpr size_t capacity() const noexcept { return Capacity; }

    void push_back(const T &value) {
        if (current_size == Capacity) {
            throw std::runtime_error("Container is full");
        } else {
            data_[current_size] = value;
            ++current_size;
        }
    }

    void resize(size_t new_size) {
        if (new_size > Capacity) {
            throw std::runtime_error("Cannot resize beyond capacity");
        } else {
            current_size = new_size;
        }
    }

    void erase(T *ptr) {
        if (ptr >= begin() && ptr < end()) {
            current_size = static_cast<size_t>(ptr - begin());
        } else {
            throw std::runtime_error("tried to erase with a ptr outside obj");
        }
    }

    template <typename Container>
    bool is_equal(const Container &c) const noexcept {
        if (current_size != c.size()) {
            return false;
        } else {
            for (size_t i = 0; i != current_size; ++i) {
                if (data_[i] != c[i]) {
                    return false;
                }
            }
        }
        return true;
    }

    T &front() noexcept { return data_.front(); }
    T &back() noexcept { return data_[current_size - 1]; }
    constexpr const T &front() const noexcept { return data_.front(); }
    constexpr const T &back() const noexcept { return data_[current_size - 1]; }

    // iterators
    iterator begin() noexcept { return data_.begin(); }
    // auto begin() noexcept -> decltype(data_.begin()) { return data_.begin();
    // }
    const_iterator begin() const noexcept { return data_.begin(); }
    iterator end() noexcept { return &data_[current_size]; }
    const_iterator end() const noexcept { return &data_[current_size]; }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    const_iterator cend() const noexcept { return &data_[current_size]; }

    void size_check(size_type s) const {
        if (s > Capacity) {
            throw std::runtime_error(
                "Capacity needs to be same size or larger than vector");
        }
    }

} __attribute__((packed));

template <typename T, size_t CapacityLhs, typename V, size_t CapacityRhs>
bool operator==(const FixedCapacityContainer<T, CapacityLhs> &lhs,
                const FixedCapacityContainer<V, CapacityRhs> &rhs) {
    return lhs.is_equal(rhs);
}

template <typename T, size_t CapacityLhs, typename V, size_t CapacityRhs>
bool operator!=(const FixedCapacityContainer<T, CapacityLhs> &lhs,
                const FixedCapacityContainer<V, CapacityRhs> &rhs) {
    return !(lhs.is_equal(rhs));
}

// Compare with array

template <typename T, size_t CapacityLhs, typename V, size_t Size>
bool operator==(const FixedCapacityContainer<T, CapacityLhs> &lhs,
                const std::array<V, Size> &rhs) {
    return lhs.is_equal(rhs);
}

template <typename T, size_t Size, typename V, size_t CapacityRhs>
bool operator==(const std::array<T, Size> &lhs,
                const FixedCapacityContainer<V, CapacityRhs> &rhs) {
    return rhs.is_equal(lhs);
}

template <typename T, size_t CapacityLhs, typename V, size_t Size>
bool operator!=(const FixedCapacityContainer<T, CapacityLhs> &lhs,
                const std::array<V, Size> &rhs) {
    return !lhs.is_equal(rhs);
}

template <typename T, size_t Size, typename V, size_t CapacityRhs>
bool operator!=(const std::array<T, Size> &lhs,
                const FixedCapacityContainer<V, CapacityRhs> &rhs) {
    return !rhs.is_equal(lhs);
}

// Compare with vector

template <typename T, size_t CapacityLhs, typename V>
bool operator==(const FixedCapacityContainer<T, CapacityLhs> &lhs,
                const std::vector<V> &rhs) {
    return lhs.is_equal(rhs);
}

template <typename T, typename V, size_t CapacityRhs>
bool operator==(const std::vector<T> &lhs,
                const FixedCapacityContainer<V, CapacityRhs> &rhs) {
    return rhs.is_equal(lhs);
}

template <typename T, size_t CapacityLhs, typename V>
bool operator!=(const FixedCapacityContainer<T, CapacityLhs> &lhs,
                const std::vector<V> &rhs) {
    return !lhs.is_equal(rhs);
}

template <typename T, typename V, size_t CapacityRhs>
bool operator!=(const std::vector<T> &lhs,
                const FixedCapacityContainer<V, CapacityRhs> &rhs) {
    return !rhs.is_equal(lhs);
}

/** support flipped order compare */
// template <typename T, size_t Capacity, typename C>
// typename std::enable_if<is_container<C>::value, bool>::type operator==(
//     const C &container,
//     const FixedCapacityContainer<T, Capacity> &fixed_container) noexcept {
//     return fixed_container.operator==(container);
// }

// /** support flipped order compare */
// template <typename T, size_t Capacity, typename C>
// typename std::enable_if<is_container<C>::value, bool>::type operator!=(
//     const C &container,
//     const FixedCapacityContainer<T, Capacity> &fixed_container) noexcept {
//     return fixed_container.operator!=(container);
// }

} // namespace sls
