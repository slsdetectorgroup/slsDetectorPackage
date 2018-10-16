#ifndef CONTAINER_UTILS_H
#define CONTAINER_UTILS_H

#include <algorithm>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>
#include <sstream>
#include <memory>
namespace sls {



// C++11 make_unique implementation for exeption safety
// already available as std::make_unique in C++14
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
bool allEqual(const std::vector<T>& container)
{
    if (container.empty())
        return false;
    const auto& first = container[0];
    return std::all_of(container.cbegin(), container.cend(),
        [first](const T& element) { return element == first; });
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
allEqualWithTol(const std::vector<T>& container, const T tol)
{
    if (container.empty())
        return false;

    const auto& first = container[0];
    return std::all_of(container.cbegin(), container.cend(),
        [first, tol](const T& element) {
            return (std::abs(element - first) < tol);
        });
}

template <typename T>
bool allEqualTo(const std::vector<T>& container, const T value)
{
    if (container.empty())
        return false;

    return std::all_of(container.cbegin(), container.cend(),
        [value](const T& element) { return element == value; });
}

template <typename T>
bool allEqualToWithTol(const std::vector<T>& container, const T value,
    const T tol)
{
    if (container.empty())
        return false;

    return std::all_of(container.cbegin(), container.cend(),
        [value, tol](const T& element) {
            return (std::abs(element - value) < tol);
        });
}

template <typename T>
bool anyEqualTo(const std::vector<T>& container, const T value)
{
    return std::any_of(container.cbegin(), container.cend(),
        [value](const T& element) { return element == value; });
}

template <typename T>
bool anyEqualToWithTol(const std::vector<T>& container, const T value,
    const T tol)
{
    return std::any_of(container.cbegin(), container.cend(),
        [value, tol](const T& element) { return (std::abs(element - value) < tol); });
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
sum(const std::vector<T>& container)
{
    return std::accumulate(container.cbegin(), container.cend(), T{ 0 });
}

template <typename T>
T minusOneIfDifferent(const std::vector<T>& container)
{
    if (allEqual(container)) {
        return container.front();
    } else {
        return static_cast<T>(-1);
    }
}

//TODO!(Erik)Should try to move away from using this in the slsDetectorPackage
inline
std::string concatenateIfDifferent(std::vector<std::string> container)
{
    if (allEqual(container)) {
        return container.front();
    } else {
        std::string result;
        for (const auto& s : container)
            result += s + "+";
        return result;
    }
}
inline
std::vector<std::string> split(const std::string& strToSplit, char delimeter)
{
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter)) {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}

inline
std::string concatenateNonEmptyStrings(const std::vector<std::string>& vec){
    std::string ret;
    for (const auto& s : vec)
        if (!s.empty())
            ret += s + "+";
    return ret;
}

} // namespace sls

#endif // CONTAINER_UTILS_H
