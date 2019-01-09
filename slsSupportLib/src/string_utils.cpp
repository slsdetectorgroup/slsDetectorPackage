
#include "string_utils.h"
#include "container_utils.h"
#include <sstream>
namespace sls{


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


std::string concatenateNonEmptyStrings(const std::vector<std::string>& vec){
    std::string ret;
    for (const auto& s : vec)
        if (!s.empty())
            ret += s + '+';
    return ret;
}

std::string concatenateIfDifferent(std::vector<std::string> container)
{
    if (allEqual(container)) {
        return container.front();
    } else {
        std::string result;
        for (const auto& s : container)
            result += s + '+';
        return result;
    }
}


}; // namespace sls