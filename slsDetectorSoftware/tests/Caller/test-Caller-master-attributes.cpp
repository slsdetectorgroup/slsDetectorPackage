// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "receiver_defs.h"
#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <sstream>

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

void test_master_file_version(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Version"));
    REQUIRE(doc["Version"].IsNumber());
    REQUIRE(doc["Version"].GetDouble() == BINARY_WRITER_VERSION);
}

void test_master_file_type(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Detector Type"));
    auto value = det.getDetectorType().tsquash("Inconsistent detector type");
    REQUIRE(doc["Detector Type"].GetString() == ToString(value));
}

void test_master_file_timing_mode(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Timing Mode"));
    auto value = det.getTimingMode().tsquash("Inconsistent timing mode");
    REQUIRE(doc["Timing Mode"].GetString() == ToString(value));
}

void test_master_file_geometry(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Geometry"));
    REQUIRE(doc["Geometry"].HasMember("x"));
    REQUIRE(doc["Geometry"].HasMember("y"));
    auto modGeometry = det.getModuleGeometry();
    auto portperModGeometry = det.getPortPerModuleGeometry();
    auto value = defs::xy{modGeometry.x * portperModGeometry.x,
                          modGeometry.y * portperModGeometry.y};
    REQUIRE(doc["Geometry"]["x"].GetInt() == value.x);
    REQUIRE(doc["Geometry"]["y"].GetInt() == value.y);
}

void test_master_file_image_size(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Image Size in bytes"));

    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    int bytes_per_pixel = det.getDynamicRange().squash() / 8;
    detParameters par(det_type);

    switch (det_type) {
    case defs::EIGER: {
        int num_chips = (par.nChipX / 2);
        size_t image_size =
            par.nChanX * par.nChanY * num_chips * bytes_per_pixel;
        REQUIRE(doc["Image Size in bytes"].GetUint64() == image_size);
    } break;
    case defs::JUNGFRAU:
    case defs::MOENCH: {
        auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");
        size_t image_size = (par.nChanX * par.nChanY * par.nChipX * par.nChipY *
                             bytes_per_pixel) /
                            num_udp_interfaces;
        REQUIRE(doc["Image Size in bytes"].GetUint64() == image_size);
    } break;

    case defs::MYTHEN3: {
        int counter_mask = det.getCounterMask().squash();
        int num_counters = __builtin_popcount(counter_mask);
        int num_channels_per_counter = par.nChanX / 3;
        size_t image_size = num_channels_per_counter * num_counters *
                            par.nChipX * bytes_per_pixel;
        REQUIRE(doc["Image Size in bytes"].GetUint64() == image_size);
    } break;

    case defs::GOTTHARD2: {
        size_t image_size = par.nChanX * par.nChipX * bytes_per_pixel;
        REQUIRE(doc["Image Size in bytes"].GetUint64() == image_size);
    } break;

    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD: {
        testCtbAcquireInfo test_info;
        size_t image_size = calculate_ctb_image_size(
            test_info, (det_type == defs::XILINX_CHIPTESTBOARD));
        REQUIRE(doc["Image Size in bytes"].GetUint64() == image_size);
    } break;
    default:
        throw sls::RuntimeError("Unsupported detector type for this test");
    }
}

void test_master_file_det_size(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Pixels"));
    REQUIRE(doc["Pixels"].HasMember("x"));
    REQUIRE(doc["Pixels"].HasMember("y"));
    auto portSize = det.getPortSize()[0];
    REQUIRE(doc["Pixels"]["x"].GetInt() == portSize.x);
    REQUIRE(doc["Pixels"]["y"].GetInt() == portSize.y);
}

void test_master_file_max_frames_per_file(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Max Frames Per File"));
    auto value = det.getFramesPerFile().tsquash("Inconsistent frames per file");
    REQUIRE(doc["Max Frames Per File"].GetInt() == value);
}

void test_master_file_frame_discard_policy(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Frame Discard Policy"));
    auto value = det.getRxFrameDiscardPolicy().tsquash(
        "Inconsistent frame discard policy");
    REQUIRE(doc["Frame Discard Policy"].GetString() == ToString(value));
}

void test_master_file_frame_padding(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Frame Padding"));
    auto value =
        det.getPartialFramesPadding().tsquash("Inconsistent frame padding");
    REQUIRE(doc["Frame Padding"].GetInt() == value);
}

void test_master_file_scan_parameters(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Scan Parameters"));
    auto value = det.getScan().tsquash("Inconsistent scan parameters");
    REQUIRE(doc["Scan Parameters"].GetString() == ToString(value));
}

void test_master_file_total_frames(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Total Frames"));
    uint64_t repeats =
        det.getNumberOfTriggers().tsquash("Inconsistent number of triggers");
    uint64_t numFrames =
        det.getNumberOfFrames().tsquash("Inconsistent number of frames");
    int numAdditionalStorageCells = 0;
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types");
    if (det_type == defs::GOTTHARD2) {
        auto timing_mode =
            det.getTimingMode().tsquash("Inconsistent timing mode");
        auto burst_mdoe = det.getBurstMode().tsquash("Inconsistent burst mode");
        auto numBursts =
            det.getNumberOfBursts().tsquash("Inconsistent number of bursts");
        // auto
        if (timing_mode == defs::AUTO_TIMING) {
            // burst mode, repeats = #bursts
            if (burst_mdoe == defs::BURST_INTERNAL ||
                burst_mdoe == defs::BURST_EXTERNAL) {
                repeats = numBursts;
            }
            // continuous, repeats = 1 (no trigger as well)
            else {
                repeats = 1;
            }
        }
        // trigger
        else {
            // continuous, numFrames is limited
            if (burst_mdoe == defs::CONTINUOUS_INTERNAL ||
                burst_mdoe == defs::CONTINUOUS_EXTERNAL) {
                numFrames = 1;
            }
        }
    } else if (det_type == defs::JUNGFRAU) {
        numAdditionalStorageCells =
            det.getNumberOfAdditionalStorageCells().tsquash(
                "Inconsistent number of additional storage cells");
    }
    uint64_t numberOfTotalFrames =
        numFrames * repeats * (int64_t)(numAdditionalStorageCells + 1);

    REQUIRE(doc["Total Frames"].GetUint64() == numberOfTotalFrames);
}

void test_master_file_rois(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Receiver Rois"));
    auto rois = det.getRxROI();
    auto file_rois = doc["Receiver Rois"].GetArray();
    REQUIRE(file_rois.Size() == rois.size());
    auto detsize = det.getDetectorSize();
    bool is2D = (detsize.y > 1);
    for (size_t i = 0; i < rois.size(); ++i) {
        if (rois[i].completeRoi()) {
            if (is2D) {
                std::string roi_string =
                    "[0, " + std::to_string(detsize.x - 1) + ", 0, " +
                    std::to_string(detsize.y - 1) + "]";
                REQUIRE(file_rois[i].GetString() == roi_string);
            } else {
                std::string roi_string =
                    "[0, " + std::to_string(detsize.x - 1) + "]";
                REQUIRE(file_rois[i].GetString() == roi_string);
            }
        } else {
            REQUIRE(file_rois[i].GetString() == ToString(rois[i]));
        }
    }
}

void test_master_file_exptime(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Exptime"));
    auto value = det.getExptime().tsquash("Inconsistent exposure time");
    REQUIRE(doc["Exptime"].GetString() == ToString(value));
}

void test_master_file_period(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Period"));
    auto value = det.getPeriod().tsquash("Inconsistent period");
    REQUIRE(doc["Period"].GetString() == ToString(value));
}

void test_master_file_num_udp_interfaces(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Number of UDP Interfaces"));
    auto value = det.getNumberofUDPInterfaces().tsquash(
        "Inconsistent number of UDP interfaces");
    REQUIRE(doc["Number of UDP Interfaces"].GetInt() == value);
}
void test_master_file_read_n_rows(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Number of rows"));
    auto value = det.getReadNRows().tsquash("Inconsistent number of rows");
    REQUIRE(doc["Number of rows"].GetInt() == value);
}

void test_master_file_readout_speed(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Readout Speed"));
    auto value = det.getReadoutSpeed().tsquash("Inconsistent readout speed");
    REQUIRE(doc["Readout Speed"].GetString() == ToString(value));
}

void test_master_file_frames_in_file(const Document &doc, Detector &det,
                                     const int frames_in_file) {
    REQUIRE(doc.HasMember("Frames in File"));
    REQUIRE(doc["Frames in File"].GetInt() == frames_in_file);
}

void test_master_file_dynamic_range(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Dynamic Range"));
    auto value = det.getDynamicRange().tsquash("Inconsistent dynamic range");
    REQUIRE(doc["Dynamic Range"].GetInt() == value);
}

void test_master_file_ten_giga(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Ten Giga"));
    auto value = det.getTenGiga().tsquash("Inconsistent ten giga");
    REQUIRE(doc["Ten Giga"].GetInt() == static_cast<int>(value));
}

void test_master_file_threshold_energy(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Threshold Energy"));
    auto value =
        det.getThresholdEnergy().tsquash("Inconsistent threshold energy");
    REQUIRE(doc["Threshold Energy"].GetInt() == value);
}

void test_master_file_sub_exptime(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Sub Exptime"));
    auto value = det.getSubExptime().tsquash("Inconsistent sub exposure time");
    REQUIRE(doc["Sub Exptime"].GetString() == ToString(value));
}

void test_master_file_sub_period(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Sub Period"));
    auto exptime = det.getSubExptime().tsquash("Inconsistent sub exptime");
    auto deadtime = det.getSubDeadTime().tsquash("Inconsistent sub deadtime");
    auto value = exptime + deadtime;
    REQUIRE(doc["Sub Period"].GetString() == ToString(value));
}

void test_master_file_quad(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Quad"));
    auto value = det.getQuad().tsquash("Inconsistent quad");
    REQUIRE(doc["Quad"].GetInt() == static_cast<int>(value));
}

void test_master_file_rate_corrections(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Rate Corrections"));
    std::vector<int64_t> dead_times;
    for (auto item : det.getRateCorrection())
        dead_times.push_back(item.count());
    REQUIRE(doc["Rate Corrections"].GetString() == ToString(dead_times));
}

void test_master_file_counter_mask(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Counter Mask"));
    auto value = det.getCounterMask().tsquash("Inconsistent counter mask");
    std::stringstream ss;
    ss << "0x" << std::hex << value;
    std::string hexStr = ss.str();
    REQUIRE(doc["Counter Mask"].GetString() == hexStr);
}

void test_master_file_exptimes(const Document &doc, Detector &det) {
    for (int i = 0; i != 3; ++i) {
        std::string key = "Exptime" + std::to_string(i + 1);
        REQUIRE(doc.HasMember(key.c_str()));
        auto value = det.getExptime(i).tsquash(
            "Inconsistent exposure time for " + std::to_string(i + 1));
        REQUIRE(doc[key.c_str()].GetString() == ToString(value));
    }
}

void test_master_file_gate_delays(const Document &doc, Detector &det) {
    for (int i = 0; i != 3; ++i) {
        std::string key = "GateDelay" + std::to_string(i + 1);
        REQUIRE(doc.HasMember(key.c_str()));
        auto value = det.getGateDelay(i).tsquash("Inconsistent GateDelay for " +
                                                 std::to_string(i + 1));
        REQUIRE(doc[key.c_str()].GetString() == ToString(value));
    }
}

void test_master_file_gates(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Gates"));
    auto value = det.getNumberOfGates().tsquash("Inconsistent number of gates");
    REQUIRE(doc["Gates"].GetInt() == value);
}

void test_master_file_threadhold_energies(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Threshold Energies"));
    auto value =
        det.getAllThresholdEnergy().tsquash("Inconsistent threshold energies");
    REQUIRE(doc["Threshold Energies"].GetString() == ToString(value));
}

void test_master_file_burst_mode(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Burst Mode"));
    auto value = det.getBurstMode().tsquash("Inconsistent burst mode");
    REQUIRE(doc["Burst Mode"].GetString() == ToString(value));
}

void test_master_file_adc_mask(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("ADC Mask"));
    auto tengiga = det.getTenGiga().tsquash("Inconsistent ten giga");
    auto value = det.getADCEnableMask().tsquash("Inconsistent ADC mask");
    if (tengiga) {
        value = det.getTenGigaADCEnableMask().tsquash(
            "Inconsistent ten giga ADC mask");
    }
    REQUIRE(doc["ADC Mask"].GetUint() == value);
}

void test_master_file_analog_flag(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Analog Flag"));
    auto romode = det.getReadoutMode().tsquash("Inconsistent analog flag");
    auto value =
        (romode == defs::ANALOG_ONLY || romode == defs::ANALOG_AND_DIGITAL);
    REQUIRE(doc["Analog Flag"].GetInt() == static_cast<int>(value));
}

void test_master_file_analog_samples(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Analog Samples"));
    auto value = det.getNumberOfAnalogSamples().tsquash(
        "Inconsistent number of analog samples");
    REQUIRE(doc["Analog Samples"].GetInt() == value);
}

void test_master_file_digital_flag(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Digital Flag"));
    auto romode = det.getReadoutMode().tsquash("Inconsistent digital flag");
    auto value =
        (romode == defs::DIGITAL_ONLY || romode == defs::ANALOG_AND_DIGITAL ||
         romode == defs::DIGITAL_AND_TRANSCEIVER);
    REQUIRE(doc["Digital Flag"].GetInt() == static_cast<int>(value));
}
void test_master_file_digital_samples(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Digital Samples"));
    auto value = det.getNumberOfDigitalSamples().tsquash(
        "Inconsistent number of digital samples");
    REQUIRE(doc["Digital Samples"].GetInt() == value);
}

void test_master_file_dbit_offset(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Dbit Offset"));
    auto value = det.getRxDbitOffset().tsquash("Inconsistent Dbit offset");
    REQUIRE(doc["Dbit Offset"].GetInt() == value);
}

void test_master_file_dbit_reorder(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Dbit Reorder"));
    auto value = det.getRxDbitReorder().tsquash("Inconsistent Dbit reorder");
    REQUIRE(doc["Dbit Reorder"].GetString() == ToString(value));
}

void test_master_file_dbit_bitset(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Dbit Bitset"));
    auto value = det.getRxDbitList().tsquash("Inconsistent Dbit bitset");
    REQUIRE(doc["Dbit Bitset"].GetString() == ToString(value));
}

void test_master_file_transceiver_mask(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Transceiver Mask"));
    auto value =
        det.getTransceiverEnableMask().tsquash("Inconsistent transceiver mask");
    REQUIRE(doc["Transceiver Mask"].GetUint() == value);
}

void test_master_file_transceiver_flag(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Transceiver Flag"));
    auto romode = det.getReadoutMode().tsquash("Inconsistent transceiver flag");
    auto value = (romode == defs::DIGITAL_AND_TRANSCEIVER ||
                  romode == defs::TRANSCEIVER_ONLY);
    REQUIRE(doc["Transceiver Flag"].GetInt() == static_cast<int>(value));
}

void test_master_file_transceiver_samples(const Document &doc, Detector &det) {
    REQUIRE(doc.HasMember("Transceiver Samples"));
    auto value = det.getNumberOfTransceiverSamples().tsquash(
        "Inconsistent number of transceiver samples");
    REQUIRE(doc["Transceiver Samples"].GetInt() == value);
}

void test_master_file_common_metadata(const Document &doc, Detector &det) {
    test_master_file_version(doc, det);
    test_master_file_type(doc, det);
    test_master_file_timing_mode(doc, det);
    test_master_file_geometry(doc, det);
    test_master_file_image_size(doc, det);
    test_master_file_det_size(doc, det);
    test_master_file_max_frames_per_file(doc, det);
    test_master_file_frame_discard_policy(doc, det);
    test_master_file_frame_padding(doc, det);
    test_master_file_scan_parameters(doc, det);
    test_master_file_total_frames(doc, det);
    // TODO: test frame header format?
}

void test_master_file_jungfrau_metadata(const Document &doc, Detector &det) {
    test_master_file_common_metadata(doc, det);
    // Jungfrau specific metadata
    test_master_file_rois(doc, det);
    test_master_file_exptime(doc, det);
    test_master_file_period(doc, det);
    test_master_file_num_udp_interfaces(doc, det);
    test_master_file_read_n_rows(doc, det);
    test_master_file_readout_speed(doc, det);
}

void test_master_file_eiger_metadata(const Document &doc, Detector &det) {
    test_master_file_common_metadata(doc, det);
    // Eiger specific metadata
    test_master_file_rois(doc, det);
    test_master_file_dynamic_range(doc, det);
    test_master_file_ten_giga(doc, det);
    test_master_file_exptime(doc, det);
    test_master_file_period(doc, det);
    test_master_file_threshold_energy(doc, det);
    test_master_file_sub_exptime(doc, det);
    test_master_file_sub_period(doc, det);
    test_master_file_quad(doc, det);
    test_master_file_read_n_rows(doc, det);
    test_master_file_rate_corrections(doc, det);
    test_master_file_readout_speed(doc, det);
}

void test_master_file_moench_metadata(const Document &doc, Detector &det) {
    test_master_file_common_metadata(doc, det);
    // Moench specific metadata
    test_master_file_rois(doc, det);
    test_master_file_exptime(doc, det);
    test_master_file_period(doc, det);
    test_master_file_num_udp_interfaces(doc, det);
    test_master_file_read_n_rows(doc, det);
    test_master_file_readout_speed(doc, det);
}

void test_master_file_mythen3_metadata(const Document &doc, Detector &det) {
    test_master_file_common_metadata(doc, det);
    // Mythen3 specific metadata
    test_master_file_rois(doc, det);
    test_master_file_dynamic_range(doc, det);
    test_master_file_ten_giga(doc, det);
    test_master_file_period(doc, det);
    test_master_file_counter_mask(doc, det);
    test_master_file_exptimes(doc, det);
    test_master_file_gate_delays(doc, det);
    test_master_file_gates(doc, det);
    test_master_file_threadhold_energies(doc, det);
    test_master_file_readout_speed(doc, det);
}

void test_master_file_gotthard2_metadata(const Document &doc, Detector &det) {
    test_master_file_common_metadata(doc, det);
    // Gotthard2 specific metadata
    test_master_file_exptime(doc, det);
    test_master_file_period(doc, det);
    test_master_file_burst_mode(doc, det);
    test_master_file_readout_speed(doc, det);
}

void test_master_file_ctb_metadata(const Document &doc, Detector &det) {
    test_master_file_common_metadata(doc, det);
    // Ctb specific metadata
    test_master_file_exptime(doc, det);
    test_master_file_period(doc, det);
    test_master_file_ten_giga(doc, det);
    test_master_file_adc_mask(doc, det);
    test_master_file_analog_flag(doc, det);
    test_master_file_analog_samples(doc, det);
    test_master_file_digital_flag(doc, det);
    test_master_file_digital_samples(doc, det);
    test_master_file_dbit_offset(doc, det);
    test_master_file_dbit_reorder(doc, det);
    test_master_file_dbit_bitset(doc, det);
    test_master_file_transceiver_mask(doc, det);
    test_master_file_transceiver_flag(doc, det);
    test_master_file_transceiver_samples(doc, det);
}

void test_master_file_metadata(const Document &doc, Detector &det) {
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types");
    switch (det_type) {
    case defs::JUNGFRAU:
        test_master_file_jungfrau_metadata(doc, det);
        break;
    case defs::EIGER:
        test_master_file_eiger_metadata(doc, det);
        break;
    case defs::MOENCH:
        test_master_file_moench_metadata(doc, det);
        break;
    case defs::MYTHEN3:
        test_master_file_mythen3_metadata(doc, det);
        break;
    case defs::GOTTHARD2:
        test_master_file_gotthard2_metadata(doc, det);
        break;
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD:
        test_master_file_ctb_metadata(doc, det);
        break;
    default:
        break;
    }
}

TEST_CASE("check_master_file_attributes", "[.cmdcall][.cmdacquire][.cmdattr]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    switch (det_type) {
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
        create_files_for_acquire(det, caller, 1, test_ctb_config);
    } break;
    default:
        throw sls::RuntimeError("Unsupported detector type for this test");
    }

    // binary
    std::string file_path = "/tmp/sls_test_master_0.json";
    REQUIRE(std::filesystem::exists(file_path) == true);

    auto doc = parse_binary_master_attributes(file_path);
    test_master_file_metadata(doc, det);
    test_master_file_frames_in_file(doc, det, 1);

    // hdf5 (TODO)
    file_path = "/tmp/sls_test_master_0.h5";
    REQUIRE(std::filesystem::exists(file_path) == true);
}

} // namespace sls
