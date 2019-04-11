#pragma once
#include <string>
struct SingleDetectorConfig {
    slsDetectorDefs::detectorType type_enum =
        slsDetectorDefs::detectorType::EIGER;
    const std::string hostname = "beb083";
    const std::string type_string = "Eiger";
    const std::string my_ip = "129.129.205.242";
};
