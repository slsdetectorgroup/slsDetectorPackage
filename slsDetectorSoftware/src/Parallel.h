#pragma once
#include "Container3.h"
#include "Result.h"
#include <future>
#include <vector>

/*
Can't use std::forward<CT>(Args)...) in parallel call since it would
end up moving temporary objects into the first called function
leaving the other ones with moved from args.
*/

namespace experimental {
using sls::Container3;
template <class CT> struct NonDeduced { using type = CT; };

template <typename RT, typename Class, typename... CT>
sls::Result<RT> Parallel(RT (Class::*func)(CT...) const,
                         const Container3<std::unique_ptr<Class>> &objects,
                         const Container3<bool> &mask,
                         typename NonDeduced<CT>::type... Args) {
    std::vector<std::future<RT>> futures;
    for (size_t i = 0; i < objects.size(); ++i) {
        if (mask[i])
            futures.push_back(
                std::async(std::launch::async, func, objects[i].get(), Args...));
    }
    sls::Result<RT> result;
    for (auto &f : futures) {
        result.push_back(f.get());
    }
    return result;
}

template <typename RT, typename Class, typename... CT>
sls::Result<RT> Parallel(RT (Class::*func)(CT...),
                         const Container3<std::unique_ptr<Class>> &objects,
                         const Container3<bool> &mask,
                         typename NonDeduced<CT>::type... Args) {
    std::vector<std::future<RT>> futures;
    for (size_t i = 0; i < objects.size(); ++i) {
        if (mask[i])
            futures.push_back(
                std::async(std::launch::async, func, &objects[i], Args...));
    }
    sls::Result<RT> result;
    for (auto &f : futures) {
        result.push_back(f.get());
    }
    return result;
}

template <typename Class, typename... CT>
void Parallel(void (Class::*func)(CT...) const,
              const Container3<std::unique_ptr<Class>> &objects,
              const Container3<bool> &mask,
              typename NonDeduced<CT>::type... Args) {
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < objects.size(); ++i) {
        if (mask[i])
            futures.push_back(
                std::async(std::launch::async, func, &objects[i], Args...));
    }
    for (auto &f : futures) {
        f.get();
    }
}

template <typename Class, typename... CT>
void Parallel(void (Class::*func)(CT...),
              const Container3<std::unique_ptr<Class>> &objects,
              const Container3<bool> &mask,
              typename NonDeduced<CT>::type... Args) {

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < objects.size(); ++i) {
        if (mask[i])
            futures.push_back(
                std::async(std::launch::async, func, &objects[i], Args...));
    }
    for (auto &f : futures)
        f.get();
}

} // namespace experimental