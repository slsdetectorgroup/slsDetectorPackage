#pragma once
#include <string>
struct SingleDetectorConfig {
    slsDetectorDefs::detectorType type_enum =
        slsDetectorDefs::detectorType::CHIPTESTBOARD;
    const std::string hostname = "bchip173";
    const std::string type_string = "Chiptestboard";
    const std::string my_ip = "129.129.205.171";
};
