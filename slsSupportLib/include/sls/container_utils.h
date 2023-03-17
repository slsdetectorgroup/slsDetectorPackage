// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef CONTAINER_UTILS_H
#define CONTAINER_UTILS_H

#include <algorithm>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
// #include <utility> //support pair in vectors

#include "sls/TypeTraits.h"

namespace sls {

// C++11 make_unique implementation for exception safety
// already available as std::make_unique in C++14
template <typename T, typename... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(Args &&...args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
typename std::enable_if<std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(std::size_t n) {
    typedef typename std::remove_extent<T>::type RT;
    return std::unique_ptr<T>(new RT[n]);
}

/** Compare elements in a Container to see if they are all equal */
template <typename Container> bool allEqual(const Container &c) {
    if (!c.empty() &&
        std::all_of(begin(c), end(c),
                    [c](const typename Container::value_type &element) {
                        return element == c.front();
                    }))
        return true;
    return false;
}

/**
 * Compare elements but with specified tolerance, useful
 * for floating point values.
 */
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
allEqualWithTol(const std::vector<T> &container, const T tol) {
    if (container.empty())
        return false;

    const auto &first = container[0];
    return std::all_of(container.cbegin(), container.cend(),
                       [first, tol](const T &element) {
                           return (std::abs(element - first) < tol);
                       });
}

template <typename T>
bool allEqualTo(const std::vector<T> &container, const T value) {
    if (container.empty())
        return false;

    return std::all_of(container.cbegin(), container.cend(),
                       [value](const T &element) { return element == value; });
}

template <typename T>
bool allEqualToWithTol(const std::vector<T> &container, const T value,
                       const T tol) {
    if (container.empty())
        return false;

    return std::all_of(container.cbegin(), container.cend(),
                       [value, tol](const T &element) {
                           return (std::abs(element - value) < tol);
                       });
}

template <typename T>
bool anyEqualTo(const std::vector<T> &container, const T value) {
    return std::any_of(container.cbegin(), container.cend(),
                       [value](const T &element) { return element == value; });
}

template <typename T>
bool anyEqualToWithTol(const std::vector<T> &container, const T value,
                       const T tol) {
    return std::any_of(container.cbegin(), container.cend(),
                       [value, tol](const T &element) {
                           return (std::abs(element - value) < tol);
                       });
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
sum(const std::vector<T> &container) {
    return std::accumulate(container.cbegin(), container.cend(), T{0});
}

template <typename T> T minusOneIfDifferent(const std::vector<T> &container) {
    if (allEqual(container))
        return container.front();
    return static_cast<T>(-1);
}

inline int minusOneIfDifferent(const std::vector<bool> &container) {
    if (allEqual(container))
        return static_cast<int>(container.front());
    return -1;
}

template <typename T>
std::vector<T>
minusOneIfDifferent(const std::vector<std::vector<T>> &container) {
    if (allEqual(container))
        return container.front();
    return std::vector<T>{-1};
}

template <typename T, size_t size>
std::array<T, size>
minusOneIfDifferent(const std::vector<std::array<T, size>> &container) {
    if (allEqual(container))
        return container.front();

    std::array<T, size> arr;
    arr.fill(static_cast<T>(-1));
    return arr;
}

/**
 * Return the first value if all values are equal
 * otherwise return default_value. If no default
 * value is supplied it will be default constructed
 */
template <typename Container>
typename Container::value_type
Squash(const Container &c, typename Container::value_type default_value = {}) {
    if (!c.empty() &&
        std::all_of(begin(c), end(c),
                    [c](const typename Container::value_type &element) {
                        return element == c.front();
                    }))
        return c.front();
    return default_value;
}

template <typename Container> bool hasDuplicates(Container c) {
    std::sort(c.begin(), c.end());
    auto pos = std::adjacent_find(c.begin(), c.end());
    return pos != c.end(); // if we found something there are duplicates
}

template <typename T>
typename std::enable_if<is_container<T>::value, bool>::type
removeDuplicates(T &c) {
    auto containerSize = c.size();
    std::sort(c.begin(), c.end());
    c.erase(std::unique(c.begin(), c.end()), c.end());
    if (c.size() != containerSize) {
        return true;
    }
    return false;
}

} // namespace sls

#endif // CONTAINER_UTILS_H
