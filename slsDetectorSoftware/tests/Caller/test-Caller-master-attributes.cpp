// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "receiver_defs.h"
#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <sstream>
#include <string>

#ifdef HDF5C
#include "H5Cpp.h"
const std::string HDF5_GROUP = "/entry/instrument/detector/";
#endif

namespace sls {

using test::GET;
using test::PUT;
using namespace rapidjson;

#ifdef HDF5C
std::optional<H5::H5File> h5File{};
#endif

void test_master_file_version(const Detector &det,
                              const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Version"));
        REQUIRE(d["Version"].IsNumber());
        REQUIRE(d["Version"].GetDouble() == BINARY_WRITER_VERSION);
    } else {
#ifdef HDF5C
        if (!h5File.has_value()) {
            throw sls::RuntimeError(
                "HDF5 file is not opened for testing file version");
        }
        auto attr = h5File->openAttribute("version");
        REQUIRE(attr.getDataType().getClass() == H5T_FLOAT);
        double version;
        attr.read(attr.getDataType(), &version);
        REQUIRE(version == HDF5_WRITER_VERSION);
#else
        throw sls::RuntimeError(
            "Document is not available for testing file version");
#endif
    }
}

void test_master_file_type(const Detector &det,
                           const std::optional<Document> &doc) {
        auto det_type =
            det.getDetectorType().tsquash("Inconsistent detector type");
                            if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Detector Type"));
        REQUIRE(d["Detector Type"].GetString() == ToString(det_type));
    } else {
#ifdef HDF5C
        if (!h5File.has_value()) {
            throw sls::RuntimeError(
                "HDF5 file is not opened for testing detector type");
        }
        std::string dset_name = HDF5_GROUP + "Detector Type";
        auto dataset = h5File->openDataSet(dset_name);
        std::string value;
        dataset.read(value, dataset.getStrType());
        REQUIRE(value == ToString(det_type));
#else
        throw sls::RuntimeError(
            "Document is not available for testing detector type");
#endif
    }
}

void test_master_file_timing_mode(const Detector &det,
                                  const std::optional<Document> &doc) {
    auto timing_mode = det.getTimingMode().tsquash("Inconsistent timing mode");
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Timing Mode"));
        REQUIRE(d["Timing Mode"].GetString() == ToString(timing_mode));
    } else {
#ifdef HDF5C
        if (!h5File.has_value()) {
            throw sls::RuntimeError(
                "HDF5 file is not opened for testing timing mode");
        }
        std::string dset_name = HDF5_GROUP + "Timing Mode";
        auto dataset = h5File->openDataSet(dset_name);
        std::string value;
        dataset.read(value, dataset.getStrType());
        REQUIRE(value == ToString(timing_mode));
#else
        throw sls::RuntimeError(
            "Document is not available for testing timing mode");
#endif
    }
}

void test_master_file_geometry(const Detector &det,
                               const std::optional<Document> &doc) {
    auto modGeometry = det.getModuleGeometry();
    auto portperModGeometry = det.getPortPerModuleGeometry();
    auto geometry = defs::xy{modGeometry.x * portperModGeometry.x,
                            modGeometry.y * portperModGeometry.y};
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Geometry"));
        REQUIRE(d["Geometry"].HasMember("x"));
        REQUIRE(d["Geometry"].HasMember("y"));
        REQUIRE(d["Geometry"]["x"].GetInt() == geometry.x);
        REQUIRE(d["Geometry"]["y"].GetInt() == geometry.y);
    } else {
#ifdef HDF5C
        if (!h5File.has_value()) {
            throw sls::RuntimeError(
                "HDF5 file is not opened for testing geometry");
        }
        std::string dset_name = HDF5_GROUP + "Geometry";
        auto dataset = h5File->openDataSet(dset_name);
        H5::CompType cType(sizeof(defs::xy));
        cType.insertMember("x", HOFFSET(defs::xy, x), H5::PredType::NATIVE_INT);
        cType.insertMember("y", HOFFSET(defs::xy, y), H5::PredType::NATIVE_INT);
        defs::xy value{};
        dataset.read(&value, cType);
        REQUIRE(value == geometry);
#else
        throw sls::RuntimeError(
            "Document is not available for testing geometry");
#endif
    }
}

void test_master_file_image_size(const Detector &det,
                                 const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Image Size in bytes"));

        auto det_type = det.getDetectorType().tsquash(
            "Inconsistent detector types to test");

        int bytes_per_pixel = det.getDynamicRange().squash() / 8;
        detParameters par(det_type);

        switch (det_type) {
        case defs::EIGER: {
            int num_chips = (par.nChipX / 2);
            size_t image_size =
                par.nChanX * par.nChanY * num_chips * bytes_per_pixel;
            REQUIRE(d["Image Size in bytes"].GetUint64() == image_size);
        } break;
        case defs::JUNGFRAU:
        case defs::MOENCH: {
            auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
                "inconsistent number of udp interfaces");
            size_t image_size = (par.nChanX * par.nChanY * par.nChipX *
                                 par.nChipY * bytes_per_pixel) /
                                num_udp_interfaces;
            REQUIRE(d["Image Size in bytes"].GetUint64() == image_size);
        } break;

        case defs::MYTHEN3: {
            int counter_mask = det.getCounterMask().squash();
            int num_counters = __builtin_popcount(counter_mask);
            int num_channels_per_counter = par.nChanX / MAX_NUM_COUNTERS;
            size_t image_size = num_channels_per_counter * num_counters *
                                par.nChipX * bytes_per_pixel;
            REQUIRE(d["Image Size in bytes"].GetUint64() == image_size);
        } break;

        case defs::GOTTHARD2: {
            size_t image_size = par.nChanX * par.nChipX * bytes_per_pixel;
            REQUIRE(d["Image Size in bytes"].GetUint64() == image_size);
        } break;

        case defs::CHIPTESTBOARD:
        case defs::XILINX_CHIPTESTBOARD: {
            testCtbAcquireInfo test_info;
            size_t image_size =
                calculate_ctb_image_size(
                    test_info, (det_type == defs::XILINX_CHIPTESTBOARD))
                    .first;
            REQUIRE(d["Image Size in bytes"].GetUint64() == image_size);
        } break;
        default:
            throw sls::RuntimeError("Unsupported detector type for this test");
        }
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_det_size(const Detector &det,
                               const std::optional<Document> &doc) {

    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    auto portSize = det.getPortSize()[0];

    // m3 assumes all counters enabled when getting num channels from client
    // TODO: in future, remove assumption
    if (det_type == defs::MYTHEN3) {
        int nchan = portSize.x / MAX_NUM_COUNTERS;
        auto counter_mask = det.getCounterMask().tsquash(
            "Inconsistent counter mask for Mythen3 detector");
        int num_counters = __builtin_popcount(counter_mask);
        portSize.x = nchan * num_counters;
    } else if (det_type == defs::CHIPTESTBOARD ||
               det_type == defs::XILINX_CHIPTESTBOARD) {
        testCtbAcquireInfo test_info;
        portSize.x = calculate_ctb_image_size(
                         test_info, det_type == defs::XILINX_CHIPTESTBOARD)
                         .second;
        portSize.y = 1;
    }

    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Pixels"));
        REQUIRE(d["Pixels"].HasMember("x"));
        REQUIRE(d["Pixels"].HasMember("y"));
        REQUIRE(d["Pixels"]["x"].GetInt() == portSize.x);
        REQUIRE(d["Pixels"]["y"].GetInt() == portSize.y);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_max_frames_per_file(const Detector &det,
                                          const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Max Frames Per File"));
        auto value =
            det.getFramesPerFile().tsquash("Inconsistent frames per file");
        REQUIRE(d["Max Frames Per File"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_frame_discard_policy(const Detector &det,
                                           const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Frame Discard Policy"));
        auto value = det.getRxFrameDiscardPolicy().tsquash(
            "Inconsistent frame discard policy");
        REQUIRE(d["Frame Discard Policy"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_frame_padding(const Detector &det,
                                    const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Frame Padding"));
        auto value =
            det.getPartialFramesPadding().tsquash("Inconsistent frame padding");
        REQUIRE(d["Frame Padding"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_scan_parameters(const Detector &det,
                                      const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Scan Parameters"));
        auto value = det.getScan().tsquash("Inconsistent scan parameters");
        REQUIRE(d["Scan Parameters"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_total_frames(const Detector &det,
                                   const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Total Frames"));
        uint64_t repeats = det.getNumberOfTriggers().tsquash(
            "Inconsistent number of triggers");
        uint64_t numFrames =
            det.getNumberOfFrames().tsquash("Inconsistent number of frames");
        int numAdditionalStorageCells = 0;
        auto det_type =
            det.getDetectorType().tsquash("Inconsistent detector types");
        if (det_type == defs::GOTTHARD2) {
            auto timing_mode =
                det.getTimingMode().tsquash("Inconsistent timing mode");
            auto burst_mdoe =
                det.getBurstMode().tsquash("Inconsistent burst mode");
            auto numBursts = det.getNumberOfBursts().tsquash(
                "Inconsistent number of bursts");
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

        REQUIRE(d["Total Frames"].GetUint64() == numberOfTotalFrames);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_rois(const Detector &det,
                           const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Receiver Rois"));
        auto rois = det.getRxROI();
        auto file_rois = d["Receiver Rois"].GetArray();
        REQUIRE(file_rois.Size() == rois.size());
        auto detsize = det.getDetectorSize();
        auto det_type = det.getDetectorType().tsquash(
            "Inconsistent detector types to test");
        // compensate for m3 channel size and counter mask mess
        if (det_type == defs::MYTHEN3) {
            int nchan = detsize.x / MAX_NUM_COUNTERS;
            auto counter_mask = det.getCounterMask().tsquash(
                "Inconsistent counter mask for Mythen3 detector");
            int num_counters = __builtin_popcount(counter_mask);
            detsize.x = nchan * num_counters;
        }
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
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_exptime(const Detector &det,
                              const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Exptime"));
        auto value = det.getExptime().tsquash("Inconsistent exposure time");
        REQUIRE(d["Exptime"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_period(const Detector &det,
                             const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Period"));
        auto value = det.getPeriod().tsquash("Inconsistent period");
        REQUIRE(d["Period"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_num_udp_interfaces(const Detector &det,
                                         const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Number of UDP Interfaces"));
        auto value = det.getNumberofUDPInterfaces().tsquash(
            "Inconsistent number of UDP interfaces");
        REQUIRE(d["Number of UDP Interfaces"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_read_n_rows(const Detector &det,
                                  const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Number of rows"));
        auto value = det.getReadNRows().tsquash("Inconsistent number of rows");
        REQUIRE(d["Number of rows"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_readout_speed(const Detector &det,
                                    const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Readout Speed"));
        auto value =
            det.getReadoutSpeed().tsquash("Inconsistent readout speed");
        REQUIRE(d["Readout Speed"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_frames_in_file(const Detector &det,
                                     const std::optional<Document> &doc,
                                     const int frames_in_file) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Frames in File"));
        REQUIRE(d["Frames in File"].GetInt() == frames_in_file);
    } else {
#ifdef HDF5C
        if (!h5File.has_value()) {
            throw sls::RuntimeError(
                "HDF5 file is not opened for testing frames in file");
        }
        std::string dset_name = HDF5_GROUP + "Frames in File";
        auto dataset = h5File->openDataSet(dset_name);
        int value;
        dataset.read(&value, H5::PredType::NATIVE_INT);
        REQUIRE(value == frames_in_file);
#else
        throw sls::RuntimeError(
            "Document is not available for testing frames in file");
#endif
    }
}

void test_master_file_json_header(const Detector &det,
                                      const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Additional Json Header"));
        auto value = det.getAdditionalJsonHeader().tsquash("Inconsistent JSON header");
        REQUIRE(d["Additional Json Header"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}   

void test_master_file_dynamic_range(const Detector &det,
                                    const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Dynamic Range"));
        auto value =
            det.getDynamicRange().tsquash("Inconsistent dynamic range");
        REQUIRE(d["Dynamic Range"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_ten_giga(const Detector &det,
                               const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Ten Giga"));
        auto value = det.getTenGiga().tsquash("Inconsistent ten giga");
        REQUIRE(d["Ten Giga"].GetInt() == static_cast<int>(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_threshold_energy(const Detector &det,
                                       const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Threshold Energy"));
        auto value =
            det.getThresholdEnergy().tsquash("Inconsistent threshold energy");
        REQUIRE(d["Threshold Energy"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_sub_exptime(const Detector &det,
                                  const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Sub Exptime"));
        auto value =
            det.getSubExptime().tsquash("Inconsistent sub exposure time");
        REQUIRE(d["Sub Exptime"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}
void test_master_file_sub_period(const Detector &det,
                                 const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Sub Period"));
        auto exptime = det.getSubExptime().tsquash("Inconsistent sub exptime");
        auto deadtime =
            det.getSubDeadTime().tsquash("Inconsistent sub deadtime");
        auto value = exptime + deadtime;
        REQUIRE(d["Sub Period"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_quad(const Detector &det,
                           const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Quad"));
        auto value = det.getQuad().tsquash("Inconsistent quad");
        REQUIRE(d["Quad"].GetInt() == static_cast<int>(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_rate_corrections(const Detector &det,
                                       const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Rate Corrections"));
        std::vector<int64_t> dead_times;
        for (auto item : det.getRateCorrection())
            dead_times.push_back(item.count());
        REQUIRE(d["Rate Corrections"].GetString() == ToString(dead_times));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_counter_mask(const Detector &det,
                                   const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Counter Mask"));
        auto value = det.getCounterMask().tsquash("Inconsistent counter mask");
        REQUIRE(d["Counter Mask"].GetUint() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_exptimes(const Detector &det,
                               const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        for (int i = 0; i != 3; ++i) {
            std::string key = "Exptime" + std::to_string(i + 1);
            REQUIRE(d.HasMember(key.c_str()));
            auto value = det.getExptime(i).tsquash(
                "Inconsistent exposure time for " + std::to_string(i + 1));
            REQUIRE(d[key.c_str()].GetString() == ToString(value));
        }
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_gate_delays(const Detector &det,
                                  const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        for (int i = 0; i != 3; ++i) {
            std::string key = "GateDelay" + std::to_string(i + 1);
            REQUIRE(d.HasMember(key.c_str()));
            auto value = det.getGateDelay(i).tsquash(
                "Inconsistent GateDelay for " + std::to_string(i + 1));
            REQUIRE(d[key.c_str()].GetString() == ToString(value));
        }
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_gates(const Detector &det,
                            const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Gates"));
        auto value =
            det.getNumberOfGates().tsquash("Inconsistent number of gates");
        REQUIRE(d["Gates"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_threadhold_energies(const Detector &det,
                                          const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Threshold Energies"));
        auto value = det.getAllThresholdEnergy().tsquash(
            "Inconsistent threshold energies");
        REQUIRE(d["Threshold Energies"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_burst_mode(const Detector &det,
                                 const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Burst Mode"));
        auto value = det.getBurstMode().tsquash("Inconsistent burst mode");
        REQUIRE(d["Burst Mode"].GetString() == ToString(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_adc_mask(const Detector &det,
                               const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("ADC Mask"));
        testCtbAcquireInfo test_ctb_config;
        auto value = test_ctb_config.adc_enable_10g;
        auto det_type = det.getDetectorType().squash();
        if (det_type == defs::CHIPTESTBOARD) {
            auto tengiga = test_ctb_config.ten_giga;
            if (!tengiga)
                value = test_ctb_config.adc_enable_1g;
        }
        REQUIRE(d["ADC Mask"].GetString() == ToStringHex(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_analog_flag(const Detector &det,
                                  const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Analog Flag"));
        testCtbAcquireInfo test_info;
        auto romode = test_info.readout_mode;
        auto value =
            (romode == defs::ANALOG_ONLY || romode == defs::ANALOG_AND_DIGITAL);
        REQUIRE(d["Analog Flag"].GetInt() == static_cast<int>(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_analog_samples(const Detector &det,
                                     const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Analog Samples"));
        testCtbAcquireInfo test_info;
        auto value = test_info.num_adc_samples;
        REQUIRE(d["Analog Samples"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_digital_flag(const Detector &det,
                                   const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Digital Flag"));
        testCtbAcquireInfo test_info;
        auto romode = test_info.readout_mode;
        auto value = (romode == defs::DIGITAL_ONLY ||
                      romode == defs::ANALOG_AND_DIGITAL ||
                      romode == defs::DIGITAL_AND_TRANSCEIVER);
        REQUIRE(d["Digital Flag"].GetInt() == static_cast<int>(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_digital_samples(const Detector &det,
                                      const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Digital Samples"));
        testCtbAcquireInfo test_info;
        auto value = test_info.num_dbit_samples;
        REQUIRE(d["Digital Samples"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_dbit_offset(const Detector &det,
                                  const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Dbit Offset"));
        testCtbAcquireInfo test_info;
        auto value = test_info.dbit_offset;
        REQUIRE(d["Dbit Offset"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_dbit_reorder(const Detector &det,
                                   const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Dbit Reorder"));
        testCtbAcquireInfo test_info;
        auto value = test_info.dbit_reorder;
        REQUIRE(d["Dbit Reorder"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_dbit_bitset(const Detector &det,
                                  const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Dbit Bitset"));
        testCtbAcquireInfo test_info;
        uint64_t value = 0;
        for (auto &i : test_info.dbit_list) {
            value |= (static_cast<uint64_t>(1) << i);
        }
        REQUIRE(d["Dbit Bitset"].GetUint64() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_transceiver_mask(const Detector &det,
                                       const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Transceiver Mask"));
        testCtbAcquireInfo test_info;
        auto value = test_info.transceiver_mask;
        REQUIRE(d["Transceiver Mask"].GetUint() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_transceiver_flag(const Detector &det,
                                       const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Transceiver Flag"));
        testCtbAcquireInfo test_info;
        auto romode = test_info.readout_mode;
        auto value = (romode == defs::DIGITAL_AND_TRANSCEIVER ||
                      romode == defs::TRANSCEIVER_ONLY);
        REQUIRE(d["Transceiver Flag"].GetInt() == static_cast<int>(value));
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_transceiver_samples(const Detector &det,
                                          const std::optional<Document> &doc) {
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Transceiver Samples"));
        testCtbAcquireInfo test_info;
        auto value = test_info.num_trans_samples;
        REQUIRE(d["Transceiver Samples"].GetInt() == value);
    } else {
        throw sls::RuntimeError(
            "Not implemented yet");
    }
}

void test_master_file_common_metadata(const Detector &det,
                                      const std::optional<Document> &doc) {
    test_master_file_version(det, doc);
    test_master_file_type(det, doc);
    test_master_file_timing_mode(det, doc);
    test_master_file_geometry(det, doc);
    test_master_file_image_size(det, doc);
    test_master_file_det_size(det, doc);
    test_master_file_max_frames_per_file(det, doc);
    test_master_file_frame_discard_policy(det, doc);
    test_master_file_frame_padding(det, doc);
    test_master_file_scan_parameters(det, doc);
    test_master_file_total_frames(det, doc);
    test_master_file_json_header(det, doc);
    // TODO: test frame header format?
}

void test_master_file_jungfrau_metadata(const Detector &det,
                                        const std::optional<Document> &doc) {
    test_master_file_common_metadata(det, doc);
    // Jungfrau specific metadata
    test_master_file_rois(det, doc);
    test_master_file_exptime(det, doc);
    test_master_file_period(det, doc);
    test_master_file_num_udp_interfaces(det, doc);
    test_master_file_read_n_rows(det, doc);
    test_master_file_readout_speed(det, doc);
}

void test_master_file_eiger_metadata(const Detector &det,
                                     const std::optional<Document> &doc) {
    test_master_file_common_metadata(det, doc);
    // Eiger specific metadata
    test_master_file_rois(det, doc);
    test_master_file_dynamic_range(det, doc);
    test_master_file_ten_giga(det, doc);
    test_master_file_exptime(det, doc);
    test_master_file_period(det, doc);
    test_master_file_threshold_energy(det, doc);
    test_master_file_sub_exptime(det, doc);
    test_master_file_sub_period(det, doc);
    test_master_file_quad(det, doc);
    test_master_file_read_n_rows(det, doc);
    test_master_file_rate_corrections(det, doc);
    test_master_file_readout_speed(det, doc);
}

void test_master_file_moench_metadata(const Detector &det,
                                      const std::optional<Document> &doc) {
    test_master_file_common_metadata(det, doc);
    // Moench specific metadata
    test_master_file_rois(det, doc);
    test_master_file_exptime(det, doc);
    test_master_file_period(det, doc);
    test_master_file_num_udp_interfaces(det, doc);
    test_master_file_read_n_rows(det, doc);
    test_master_file_readout_speed(det, doc);
}

void test_master_file_mythen3_metadata(const Detector &det,
                                       const std::optional<Document> &doc) {
    test_master_file_common_metadata(det, doc);
    // Mythen3 specific metadata
    test_master_file_rois(det, doc);
    test_master_file_dynamic_range(det, doc);
    test_master_file_ten_giga(det, doc);
    test_master_file_period(det, doc);
    test_master_file_counter_mask(det, doc);
    test_master_file_exptimes(det, doc);
    test_master_file_gate_delays(det, doc);
    test_master_file_gates(det, doc);
    test_master_file_threadhold_energies(det, doc);
    test_master_file_readout_speed(det, doc);
}

void test_master_file_gotthard2_metadata(const Detector &det,
                                         const std::optional<Document> &doc) {
    test_master_file_common_metadata(det, doc);
    // Gotthard2 specific metadata
    test_master_file_exptime(det, doc);
    test_master_file_period(det, doc);
    test_master_file_burst_mode(det, doc);
    test_master_file_readout_speed(det, doc);
}

void test_master_file_ctb_metadata(const Detector &det,
                                   const std::optional<Document> &doc) {
    auto det_type = det.getDetectorType().squash();
    test_master_file_common_metadata(det, doc);
    // Ctb specific metadata
    test_master_file_exptime(det, doc);
    test_master_file_period(det, doc);
    if (det_type == defs::CHIPTESTBOARD)
        test_master_file_ten_giga(det, doc);
    test_master_file_adc_mask(det, doc);
    test_master_file_analog_flag(det, doc);
    test_master_file_analog_samples(det, doc);
    test_master_file_digital_flag(det, doc);
    test_master_file_digital_samples(det, doc);
    test_master_file_dbit_offset(det, doc);
    test_master_file_dbit_reorder(det, doc);
    test_master_file_dbit_bitset(det, doc);
    test_master_file_transceiver_mask(det, doc);
    test_master_file_transceiver_flag(det, doc);
    test_master_file_transceiver_samples(det, doc);
}

void test_master_file_metadata(const Detector &det,
                               const std::optional<Document> &doc) {
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types");
    switch (det_type) {
    case defs::JUNGFRAU:
        test_master_file_jungfrau_metadata(det, doc);
        break;
    case defs::EIGER:
        test_master_file_eiger_metadata(det, doc);
        break;
    case defs::MOENCH:
        test_master_file_moench_metadata(det, doc);
        break;
    case defs::MYTHEN3:
        test_master_file_mythen3_metadata(det, doc);
        break;
    case defs::GOTTHARD2:
        test_master_file_gotthard2_metadata(det, doc);
        break;
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD:
        test_master_file_ctb_metadata(det, doc);
        break;
    default:
        break;
    }
}

Document parse_binary_master_attributes(std::string file_path) {
    REQUIRE(std::filesystem::exists(file_path) == true);
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

#ifdef HDF5C
void open_hdf5_file(const std::string &file_path) {
    REQUIRE(std::filesystem::exists(file_path) == true);
    h5File = std::make_optional<H5::H5File>(file_path, H5F_ACC_RDONLY);
    REQUIRE(H5Lexists(h5File->getId(), HDF5_GROUP.c_str(), H5P_DEFAULT) ==
            true);
}
#endif

TEST_CASE("check_master_file_attributes", "[.cmdcall][.cmdacquire][.cmdattr]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    int64_t num_frames = 1;
    switch (det_type) {
    case defs::EIGER:
    case defs::JUNGFRAU:
    case defs::MOENCH:
    case defs::MYTHEN3:
    case defs::GOTTHARD2:
        create_files_for_acquire(det, caller, num_frames);
        break;
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD: {
        testCtbAcquireInfo test_ctb_config;
        create_files_for_acquire(det, caller, num_frames, test_ctb_config);
    } break;
    default:
        throw sls::RuntimeError("Unsupported detector type for this test");
    }

    testFileInfo file_info;
    std::string master_file_prefix = file_info.getMasterFileNamePrefix();

    // binary
    std::string fname =
        master_file_prefix + ".json"; // /tmp/sls_test_master_0.json
    auto doc = std::make_optional(parse_binary_master_attributes(fname));
    test_master_file_metadata(det, doc);
    test_master_file_frames_in_file(det, doc, num_frames);

    // hdf5
#ifdef HDF5C
    fname = master_file_prefix + ".h5"; // /tmp/sls_test_master_0.h5
    try {
        open_hdf5_file(fname);
        test_master_file_metadata(det, std::nullopt);
        test_master_file_frames_in_file(det, std::nullopt, num_frames);
    } catch (H5::Exception &e) {
        LOG(logERROR) << "HDF5 error: " << e.getDetailMsg();
        throw;
    }
#endif
}

} // namespace sls
