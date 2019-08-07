
#include "string_utils.h"
#include "container_utils.h"
#include "network_utils.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
namespace sls {

std::vector<std::string> split(const std::string &strToSplit, char delimeter) {
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter)) {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}

std::string concatenateNonEmptyStrings(const std::vector<std::string> &vec) {
    std::string ret;
    for (const auto &s : vec)
        if (!s.empty())
            ret += s + '+';
    return ret;
}

std::string concatenateIfDifferent(const std::vector<std::string> &container) {
    if (allEqual(container)) {
        return container.front();
    } else {
        std::string result;
        for (const auto &s : container)
            result += s + '+';
        return result;
    }
}
template <typename T>
std::string concatenateIfDifferent(const std::vector<T> &container) {
    if (allEqual(container)) {
        return container.front().str();
    } else {
        std::string result;
        for (const auto &s : container)
            result += s.str() + '+';
        return result;
    }
}

std::string RemoveUnit(std::string &str) {
    auto it = str.begin();
    while (it != str.end()) {
        if (std::isalpha(*it))
            break;
        ++it;
    }
    auto pos = it - str.begin();
    auto unit = str.substr(pos);
    str.erase(it, end(str));
    return unit;
}

template std::string concatenateIfDifferent(const std::vector<IpAddr> &);
template std::string concatenateIfDifferent(const std::vector<MacAddr> &);

}; // namespace sls