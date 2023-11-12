// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

/**
 * \file Result.h
 * Result is a thin wrapper around std::vector and used for returning values
 * from the detector. Since every module could have a different value, we need
 * to return a vector instead of just a single value.
 *
 * Easy conversions to single values are provided using the squash and tsquash
 * method.
 */

#include <algorithm>
#include <iostream>
#include <vector>

#include "sls/ToString.h"
#include "sls/TypeTraits.h"
#include "sls/container_utils.h"

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

    /** Custom constructor from integer type to Result<ns> or Result<bool> */
    template <typename V, typename = typename std::enable_if<
                              std::is_integral<V>::value &&
                              (std::is_same<T, time::ns>::value ||
                               std::is_same<T, bool>::value)>::type>
    Result(const Result<V> &from) {
        vec.reserve(from.size());
        for (const auto &item : from)
            vec.push_back(T(item));
    }

    /** Custom constructor from integer type to Result<ns> or Result<bool> */
    template <typename V, typename = typename std::enable_if<
                              std::is_integral<V>::value &&
                              (std::is_same<T, time::ns>::value ||
                               std::is_same<T, bool>::value)>::type>
    Result(Result<V> &from) {
        vec.reserve(from.size());
        for (const auto &item : from)
            vec.push_back(T(item));
    }

    /** Custom constructor from integer type to Result<ns> or Result<bool> */
    template <typename V, typename = typename std::enable_if<
                              std::is_integral<V>::value &&
                              (std::is_same<T, time::ns>::value ||
                               std::is_same<T, bool>::value)>::type>
    Result(Result<V> &&from) {
        vec.reserve(from.size());
        for (const auto &item : from)
            vec.push_back(T(item));
    }

    /**
     * Forward arguments to the constructor of std::vector
     * @tparam Args template paramter pack to forward
     */
    template <typename... Args>
    Result(Args &&...args) : vec(std::forward<Args>(args)...) {}

    using value_type = typename std::vector<T>::value_type;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using size_type = typename std::vector<T>::size_type;
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    auto begin() noexcept -> decltype(vec.begin()) { return vec.begin(); }
    auto begin() const noexcept -> decltype(vec.begin()) { return vec.begin(); }
    auto cbegin() const noexcept -> decltype(vec.cbegin()) {
        return vec.cbegin();
    }
    auto end() noexcept -> decltype(vec.end()) { return vec.end(); }
    auto end() const noexcept -> decltype(vec.end()) { return vec.end(); }
    auto cend() const noexcept -> decltype(vec.cend()) { return vec.cend(); }
    auto size() const noexcept -> decltype(vec.size()) { return vec.size(); }
    auto empty() const noexcept -> decltype(vec.empty()) { return vec.empty(); }
    auto front() -> decltype(vec.front()) { return vec.front(); }
    auto front() const -> decltype(vec.front()) { return vec.front(); }
    void reserve(size_type new_cap) { vec.reserve(new_cap); }

    template <typename V>
    auto push_back(V value) -> decltype(vec.push_back(value)) {
        vec.push_back(std::forward<V>(value));
    }

    auto operator[](size_type pos) -> decltype(vec[pos]) { return vec[pos]; }
    const_reference operator[](size_type pos) const { return vec[pos]; }

    /**
     * If all elements are equal it returns the front value
     * otherwise a default constructed T
     */
    T squash() const { return Squash(vec); }

    /**
     * If all elements are equal it returns the front value
     * otherwise throws an exception with custom message provided
     */
    T tsquash(const std::string &error_msg) {
        if (equal())
            return vec.front();
        else
            throw RuntimeError(error_msg);
    }
    /**
     * If all elements are equal return the front value, otherwise
     * return the supplied default value
     */
    T squash(const T &default_value) const {
        return Squash(vec, default_value);
    }

    /** Test whether all elements of the result are equal */
    bool equal() const noexcept { return allEqual(vec); }

    /** Test whether any element of the result are equal to a value */
    bool any(const T &value) const noexcept { return anyEqualTo(vec, value); }

    template <typename V, typename... Args, typename = AllSame<V, Args...>>
    typename std::enable_if<std::is_same<V, T>::value, bool>::type
    contains_only(const V &a, const Args &...args) const noexcept {
        auto values = {a, args...};
        for (const auto &element : vec) {
            int found = 0;
            for (const auto &value : values) {
                if (value == element)
                    found++;
            }
            if (!found)
                return false;
        }
        return true;
    }

    /** Convert Result<T> to std::vector<T> */
    operator std::vector<T>() { return vec; }
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