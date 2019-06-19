


#include "Detector.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const std::chrono::nanoseconds& t)
{
    os << t.count();
    return os;
}


template<typename T>
void print(const std::vector<T>& vec){
    std::cout << "Size: " << vec.size() << '\n';
    for (const auto& it:vec)
        std::cout << it << '\n';
}

using sls::Detector;
int main() {

    Detector d;
    auto t0 = d.getExptime();
    auto t1 = d.getExptime({1});

    print(t0);
    print(t1);


    // d.acquire();
}
