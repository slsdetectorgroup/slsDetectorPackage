// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include <sls/Detector.h>

#include <chrono>
#include <iostream>
#include "sls/TimeHelper.h"
#include "sls/ToString.h"
// std::ostream &operator<<(std::ostream &os, const std::chrono::nanoseconds &t) {
//     os << t.count() << "ns";
//     return os;
// }

// template <typename T, typename _ = void>
// struct is_container : std::false_type {};

// template <typename... Ts> struct is_container_helper {};

// template <typename T>
// struct is_container<
//     T, typename std::conditional<
//            false,
//            is_container_helper<typename T::value_type, typename T::size_type,
//                                typename T::iterator, typename T::const_iterator,
//                                decltype(std::declval<T>().size()),
//                                decltype(std::declval<T>().begin()),
//                                decltype(std::declval<T>().end()),
//                                decltype(std::declval<T>().cbegin()),
//                                decltype(std::declval<T>().cend()),
//                                decltype(std::declval<T>().empty())>,
//            void>::type> : public std::true_type {};

// template <typename Container>
// auto operator<<(std::ostream &os, const Container &con) ->
//     typename std::enable_if<is_container<Container>::value,
//                             std::ostream &>::type {
//     if (con.empty())
//         return os << "[]";
//     auto it = con.cbegin();
//     os << '[' << *it++;
//     while (it != con.cend())
//         os << ", " << *it++;
//     return os << ']';
// }

using sls::Detector;
using std::chrono::nanoseconds;
using std::chrono::seconds;

int main() {
    Detector d;
    // d.setConfig("/home/l_frojdh/virtual.config");

    // d.setExptime(nanoseconds(500)); // set exptime of all modules
    // auto t0 = d.getExptime();
    // std::cout << "exptime: " << t0 << '\n';

    // d.setExptime(seconds(1), {1});  // set exptime of module one
    // auto t1 = d.getExptime({1,3,5}); // get exptime of module 1, 3 and 5
    // std::cout << "exptime: " <<t1 << '\n';

    std::cout << "Period: " <<  d.getPeriod() << '\n';
    std::cout << "Period: " <<  sls::ToString(d.getPeriod().squash()) << '\n';
    // std::cout << "fname: " << d.getFname() << "\n";

    // std::cout << "fwrite: " << std::boolalpha << d.getFwrite() << '\n';

    // d.freeSharedMemory();
   
}
