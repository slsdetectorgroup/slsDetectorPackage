// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "test-Caller-global.h"
#include "tests/globals.h"
#include "receiver_defs.h"
#include "sls/ToString.h"

#include <filesystem>
#include <sstream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>


namespace sls {

using test::GET;
using test::PUT;
using namespace rapidjson;

Document parse_binary_master_attributes(std::string file_path) {
    std::ifstream file(file_path);
    REQUIRE(file.is_open());
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_str = buffer.str();  

    Document doc;
    ParseResult result = doc.Parse(json_str.c_str());
    REQUIRE(result != 0);
    return doc;
}

void test_common_traits(const Document &doc, Detector &det) {
    // confirm version
    REQUIRE(doc.HasMember("Version"));
    REQUIRE(doc["Version"].IsNumber());
    REQUIRE(doc["Version"].GetDouble() == BINARY_WRITER_VERSION);

    // type
    REQUIRE(doc.HasMember("Detector Type"));
    REQUIRE(doc["Detector Type"].GetString() == ToString(det.getDetectorType()));

    //Timing mode
    REQUIRE(doc.HasMember("Timing Mode"));
    REQUIRE(doc["Timing Mode"].GetString() == ToString(det.getTimingMode()));

    // geometry
    REQUIRE(doc.HasMember("Geometry"));
    REQUIRE(doc["Geometry"].HasMember("x"));
    REQUIRE(doc["Geometry"].HasMember("y"));
    auto geometry = det.getModuleGeometry();
    REQUIRE(doc["Geometry"]["x"].GetInt() == geometry.x);
    REQUIRE(doc["Geometry"]["y"].GetInt() == geometry.y);

    // image size
    REQUIRE(doc.HasMember("Image Size"));
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    int bytes_per_pixel = det.getDynamicRange().squash() / 8;
    detParameters par(det_type);

    switch (det_type) {
        case defs::EIGER: {
            int num_chips = (par.nChipX / 2);
            size_t image_size = par.nChanX * par.nChanY * num_chips * bytes_per_pixel;
            REQUIRE(doc["Image Size"].GetUint64() == image_size);
        }
            break;
        case defs::JUNGFRAU:
        case defs::MOENCH: {
            auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");
            size_t image_size = (par.nChanX * par.nChanY * par.nChipX * par.nChipY * bytes_per_pixel) / num_udp_interfaces;
            REQUIRE(doc["Image Size"].GetUint64() == image_size);
        }
            break;

        case defs::MYTHEN3: {
            int counter_mask = det.getCounterMask().squash();
            int num_counters = __builtin_popcount(counter_mask);
            int num_channels_per_counter = par.nChanX / 3;
            size_t image_size = num_channels_per_counter * num_counters * par.nChipX * bytes_per_pixel;
            REQUIRE(doc["Image Size"].GetUint64() == image_size);
        }
            break;

        case defs::GOTTHARD2: {
            size_t image_size = par.nChanX * par.nChipX * bytes_per_pixel;
            REQUIRE(doc["Image Size"].GetUint64() == image_size);
        }
            break;

        case defs::CHIPTESTBOARD:
        case defs::XILINX_CHIPTESTBOARD: {
            testCtbAcquireInfo test_info;
            test_info.readout_mode = det.getReadoutMode()[0];
            test_info.ten_giga = det.getTenGiga()[0];
            test_info.num_adc_samples = det.getNumberOfAnalogSamples()[0];
            test_info.num_dbit_samples = det.getNumberOfDigitalSamples()[0];
            test_info.num_trans_samples = det.getNumberOfTransceiverSamples()[0];
            test_info.adc_enable_1g = det.getADCEnableMask()[0];
            test_info.adc_enable_10g = det.getTenGigaADCEnableMask()[0];
            test_info.dbit_offset = det.getRxDbitOffset()[0];
            test_info.dbit_list = det.getRxDbitList()[0];
            test_info.dbit_reorder = det.getRxDbitReorder()[0];
            test_info.transceiver_mask = det.getTransceiverEnableMask()[0];
            size_t image_size = calculate_ctb_image_size(test_info);
            REQUIRE(doc["Image Size"].GetUint64() == image_size);
        }
            break;
        default:
            throw sls::RuntimeError("Unsupported detector type for this test");
    }

    
    // det size
    REQUIRE(doc.HasMember("Pixels"));
    REQUIRE(doc["Pixels"].HasMember("x"));
    REQUIRE(doc["Pixels"].HasMember("y"));
    auto detsize = det.getDetectorSize();
    REQUIRE(doc["Pixels"]["x"].GetInt() == detsize.x);
    REQUIRE(doc["Pixels"]["y"].GetInt() == detsize.y);

    
    /*
    "Max Frames Per File": 10000,
    "Frame Discard Policy": "nodiscard",
    "Frame Padding": 1,
    "Scan Parameters": "[disabled]",
    "Total Frames": 1,
    "Receiver Rois": [
        "[0, 1023, 0, 511]"
    ],
    "Exptime": "200us",
    "Period": "2ms",
    "Number of UDP Interfaces": 1,
    "Number of rows": 512,
    "Readout Speed": "half_speed",
    "Frames in File": 1,
    */
}

TEST_CASE("check_master_file_attributes", "[.cmdcall][.cmdacquire][.cmdattr]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().tsquash("Inconsistent detector types to test");
    switch(det_type) {
        case defs::EIGER:
        case defs::JUNGFRAU:
        case defs::MOENCH:
        case defs::MYTHEN3:
        case defs::GOTTHARD2: 
            create_files_for_acquire(det, caller);
            break;
        case defs::CHIPTESTBOARD:
        case defs::XILINX_CHIPTESTBOARD: {
            testCtbAcquireInfo test_ctb_config;
            create_ctb_files_for_acquire(det, caller, 1, test_ctb_config);
        }
            break;
        default:
            throw sls::RuntimeError("Unsupported detector type for this test");
    }

    // binary
    std::string file_path = "/tmp/sls_test_master_0.json";
    REQUIRE(std::filesystem::exists(file_path) == true);

    auto doc = parse_binary_master_attributes(file_path);
    test_common_traits(doc, det);


    // hdf5 (TODO)
    file_path = "/tmp/sls_test_master_0.h5";
    REQUIRE(std::filesystem::exists(file_path) == true);
}

} // namespace sls
