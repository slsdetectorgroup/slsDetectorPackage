// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <type_traits>
#include <vector>

namespace sls {

/**
 * Type trait to check if a template parameter is a std::chrono::duration
 */

template <typename T, typename _ = void>
struct is_duration : std::false_type {};

template <typename... Ts> struct is_duration_helper {};

template <typename T>
struct is_duration<T,
                   typename std::conditional<
                       false,
                       is_duration_helper<typename T::rep, typename T::period,
                                          decltype(std::declval<T>().min()),
                                          decltype(std::declval<T>().max()),
                                          decltype(std::declval<T>().zero())>,
                       void>::type> : public std::true_type {};

/**
 * Has str method
 */
template <typename T, typename _ = void> struct has_str : std::false_type {};

template <typename... Ts> struct has_str_helper {};

template <typename T>
struct has_str<T, typename std::conditional<
                      false, has_str_helper<decltype(std::declval<T>().str())>,
                      void>::type> : public std::true_type {};

/**
 * Has emplace_back method
 */
template <typename T, typename _ = void>
struct has_emplace_back : std::false_type {};

template <typename... Ts> struct has_emplace_back_helper {};

template <typename T>
struct has_emplace_back<
    T, typename std::conditional<
           false,
           has_emplace_back_helper<decltype(std::declval<T>().emplace_back())>,
           void>::type> : public std::true_type {};

/**
 * Type trait to evaluate if template parameter is
 * complying with a standard container
 */
template <typename T, typename _ = void>
struct is_container : std::false_type {};

template <typename... Ts> struct is_container_helper {};

template <typename T>
struct is_container<
    T, typename std::conditional<
           false,
           is_container_helper<
               typename std::remove_reference<T>::type::value_type,
               typename std::remove_reference<T>::type::size_type,
               typename std::remove_reference<T>::type::iterator,
               typename std::remove_reference<T>::type::const_iterator,
               decltype(std::declval<T>().size()),
               decltype(std::declval<T>().begin()),
               decltype(std::declval<T>().end()),
               decltype(std::declval<T>().cbegin()),
               decltype(std::declval<T>().cend()),
               decltype(std::declval<T>().empty())>,
           void>::type> : public std::true_type {};

/**
 * Type trait to evaluate if template parameter is
 * complying with a standard container
 */
template <typename T, typename _ = void>
struct is_light_container : std::false_type {};

template <typename... Ts> struct is_light_container_helper {};

template <typename T>
struct is_light_container<
    T, typename std::conditional<
           false,
           is_container_helper<typename T::value_type, typename T::size_type,
                               typename T::iterator, typename T::const_iterator,
                               decltype(std::declval<T>().size()),
                               decltype(std::declval<T>().begin()),
                               decltype(std::declval<T>().end())>,
           void>::type> : public std::true_type {};

template <typename T> struct is_vector : public std::false_type {};

template <typename T>
struct is_vector<std::vector<T>> : public std::true_type {};

template <class...> struct Conjunction : std::true_type {};
template <class B1> struct Conjunction<B1> : B1 {};
template <class B1, class... Bn>
struct Conjunction<B1, Bn...>
    : std::conditional<bool(B1::value), Conjunction<Bn...>, B1>::type {};

template <typename T, typename... Ts>
using AllSame =
    typename std::enable_if<Conjunction<std::is_same<T, Ts>...>::value>::type;
} // namespace sls