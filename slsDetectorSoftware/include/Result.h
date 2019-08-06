#pragma once

/**
 * \file Result.h
 * Result is a thin wrapper around std::vector and used for returning values
 * from the detector. Since every module could have a different value, we need
 * to return a vector instead of just a single value.
 *
 * Easy conversions to single values are provided using the squash method.
 */

#include <algorithm>
#include <iostream>
#include <vector>

#include "ToString.h"
#include "container_utils.h"

namespace sls {

/**
 * @tparam T type to store in the result
 * @tparam Allocator for the underlying vector, default
 */
template <class T, class Allocator = std::allocator<T>> class Result {
    /** wrapped vector */
    std::vector<T, Allocator> vec;

  public:
    Result() = default;
    Result(std::initializer_list<T> list) : vec(list){};

    /**
     * Forward arguments to the constructor of std::vector
     * @tparam Args template paramter pack to forward
     */
    template <typename... Args>
    Result(Args &&... args) : vec(std::forward<Args>(args)...) {}

    using value_type = typename std::vector<T>::value_type;
    using iterator = decltype(vec.begin());
    using const_iterator = decltype(vec.cbegin());
    using size_type = decltype(vec.size());
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    auto begin() -> decltype(vec.begin()) { return vec.begin(); }
    auto end() -> decltype(vec.end()) { return vec.end(); }
    auto begin() const -> decltype(vec.begin()) { return vec.begin(); }
    auto end() const -> decltype(vec.end()) { return vec.end(); }
    auto cbegin() const -> decltype(vec.cbegin()) { return vec.cbegin(); }
    auto cend() const -> decltype(vec.cend()) { return vec.cend(); }
    auto size() const -> decltype(vec.size()) { return vec.size(); }
    auto empty() const -> decltype(vec.empty()) { return vec.empty(); }
    auto front() const -> decltype(vec.front()) { return vec.front(); }
    auto front() -> decltype(vec.front()) { return vec.front(); }

    template<typename V>
    auto push_back(V value) -> decltype(vec.push_back(value)){
      vec.push_back(std::forward<V>(value));
    }

    reference operator[](size_type pos) { return vec[pos]; }
    const_reference operator[](size_type pos) const { return vec[pos]; }

    /**
     * If all elements are equal it returns the front value
     * otherwise a default constructed T
     */
    T squash() const { return Squash(vec); }

    /**
     * If all elements are equal return the front value, otherwise
     * return the supplied default value
     */
    T squash(T default_value) const { return Squash(vec, default_value); }

    // bool equal() { return Equal(vec); }

    /** Test whether all elements of the result are equal */
    bool equal() const noexcept { return allEqual(vec); }

    /** Convert Result<T> to std::vector<T> */
    operator std::vector<T>() { return vec; }

    /** Convert Result<T> to T using squash() */
    operator T() { return squash(); }
};

/**
 * operator << overload to print Result, uses ToString for the conversion
 * @tparam T type stored in the Result
 */
template <typename T>
std::ostream &operator<<(std::ostream &os, const Result<T> &res) {
    return os << ToString(res);
}

} // namespace sls