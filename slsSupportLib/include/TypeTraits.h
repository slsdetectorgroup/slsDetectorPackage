#pragma once
#include <type_traits>

namespace sls {

/**
 * Type trait to check if atemplate parameter is a std::chrono::duration
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
           is_container_helper<typename T::value_type, typename T::size_type,
                               typename T::iterator, typename T::const_iterator,
                               decltype(std::declval<T>().size()),
                               decltype(std::declval<T>().begin()),
                               decltype(std::declval<T>().end()),
                               decltype(std::declval<T>().cbegin()),
                               decltype(std::declval<T>().cend()),
                               decltype(std::declval<T>().empty())>,
           void>::type> : public std::true_type {};

} // namespace sls