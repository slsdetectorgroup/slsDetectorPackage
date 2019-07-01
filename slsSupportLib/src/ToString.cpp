#include "ToString.h"

namespace sls {

std::string ToString(const std::vector<std::string> &vec,
                     const char delimiter) {
    std::ostringstream os;
    if (vec.empty())
        return os.str();
    auto it = vec.cbegin();
    os << *it++;
    if (vec.size() > 1) {
        while (it != vec.cend())
            os << delimiter << *it++;
    }
    return os.str();
}
} // namespace sls