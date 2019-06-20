#include "Detector.h"

#include <chrono>
#include <iostream>

std::ostream &operator<<(std::ostream &os, const std::chrono::nanoseconds &t) {
    os << t.count() << "ns";
    return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    os << "[";
    auto it = vec.cbegin();
    std::cout << *it++;
    while(it!=vec.cend())
        os << ", " << *it++;
    os << "]";
    return os;
}

using sls::Detector;
using std::chrono::nanoseconds;
using std::chrono::seconds;

int main() {
    Detector d;
    d.setConfig("/home/l_frojdh/virtual.config");

    d.setExptime(nanoseconds(500)); // set exptime of all modules
    auto t0 = d.getExptime();
    std::cout << "exptime: " << t0 << '\n';

    d.setExptime(seconds(1), {0});  // set exptime of module one
    auto t1 = d.getExptime({0}); // get exptime of module 1
    std::cout << "exptime: " <<t1 << '\n';


    std::cout << "Period: " <<  d.getPeriod() << '\n';
    std::cout << "fname: " << d.getFname() << "\n";

    std::cout << "fwrite: " << std::boolalpha << d.getFwrite() << '\n';

    d.freeSharedMemory();
   
}
