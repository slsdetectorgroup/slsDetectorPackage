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

inline bool operator==(sls::ns lhs, sls::ns rhs) {
    return lhs.count() == rhs.count();
}

#ifdef HDF5C
std::optional<H5::H5File> h5File{};
#endif

/** std::string */
void read_from_json(const Document &doc, const std::string &name,
                    std::string &retval) {
    retval = doc[name.c_str()].GetString();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          std::string &retval) {
    dataset.read(retval, dataset.getStrType());
}
#endif

/** int */
void read_from_json(const Document &doc, const std::string &name, int &retval) {
    retval = doc[name.c_str()].GetInt();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          int &retval) {
    dataset.read(&retval, H5::PredType::NATIVE_INT);
}
#endif

/** uint64_t */
void read_from_json(const Document &doc, const std::string &name,
                    uint64_t &retval) {
    retval = doc[name.c_str()].GetUint64();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          uint64_t &retval) {
    dataset.read(&retval, H5::PredType::STD_U64LE);
}
#endif

/** uint32_t */
void read_from_json(const Document &doc, const std::string &name,
                    uint32_t &retval) {
    retval = doc[name.c_str()].GetUint();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          uint32_t &retval) {
    dataset.read(&retval, H5::PredType::STD_U32LE);
}
#endif

/** double */
void read_from_json(const Document &doc, const std::string &name,
                    double &retval) {
    retval = doc[name.c_str()].GetDouble();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          double &retval) {
    dataset.read(&retval, H5::PredType::NATIVE_DOUBLE);
}
#endif

/** std::vector<int64_t> */
void read_from_json(const Document &doc, const std::string &name,
                    std::vector<int64_t> &retval) {
    for (const auto &item : doc[name.c_str()].GetArray()) {
        retval.push_back(item.GetInt64());
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          std::vector<int64_t> &retval) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    retval.resize(dims[0]);
    dataset.read(retval.data(), H5::PredType::STD_I64LE);
}
#endif

/** std::vector<defs::ROI> */
void read_from_json(const Document &doc, const std::string &name,
                    std::vector<defs::ROI> &retval) {
    for (const auto &item : doc[name.c_str()].GetArray()) {
        defs::ROI r{};
        r.xmin = item["xmin"].GetInt();
        r.xmax = item["xmax"].GetInt();
        r.ymin = item["ymin"].GetInt();
        r.ymax = item["ymax"].GetInt();
        retval.push_back(r);
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          std::vector<defs::ROI> &retval) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    H5::CompType cType(sizeof(defs::ROI));
    cType.insertMember("xmin", HOFFSET(defs::ROI, xmin),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("xmax", HOFFSET(defs::ROI, xmax),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("ymin", HOFFSET(defs::ROI, ymin),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("ymax", HOFFSET(defs::ROI, ymax),
                       H5::PredType::NATIVE_INT);
    retval.resize(dims[0]);
    dataset.read(retval.data(), cType);
}
#endif

/** std::array<int, 3UL> */
void read_from_json(const Document &doc, const std::string &name,
                    std::array<int, 3UL> &retval) {
    const auto &json_values = doc[name.c_str()].GetArray();
    if (json_values.Size() != retval.size()) {
        throw sls::RuntimeError("JSON array " + name +
                                " does not have num elements as expected");
    }
    int index = 0;
    for (const auto &item : json_values) {
        retval[index++] = item.GetInt();
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          std::array<int, 3UL> &retval) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    if (dims[0] != retval.size()) {
        throw sls::RuntimeError("HDF5 dataset " + name +
                                " does not have num elements as expected");
    }
    dataset.read(retval.data(), H5::PredType::NATIVE_INT);
}
#endif

/* std::array<sls::ns, 3UL> */
void read_from_json(const Document &doc, const std::string &name,
                    std::array<sls::ns, 3UL> &retval) {
    const auto &json_values = doc[name.c_str()].GetArray();
    if (json_values.Size() != retval.size()) {
        throw sls::RuntimeError("JSON array " + name +
                                " does not have num elements as expected");
    }
    int index = 0;
    for (const auto &item : json_values) {
        std::string sval = item.GetString();
        retval[index++] = StringTo<sls::ns>(sval);
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          std::array<sls::ns, 3UL> &retval) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    if (dims[0] != retval.size()) {
        throw sls::RuntimeError("HDF5 dataset " + name +
                                " does not have num elements as expected");
    }
    std::vector<const char *> strValues(dims[0]);
    dataset.read(strValues.data(), dataset.getStrType());
    for (size_t i = 0; i < dims[0]; ++i) {
        retval[i] = StringTo<sls::ns>(strValues[i]);
    }
}
#endif

/** defs::xy */
void read_from_json(const Document &doc, const std::string &name,
                    defs::xy &retval) {
    retval.x = doc[name.c_str()]["x"].GetInt();
    retval.y = doc[name.c_str()]["y"].GetInt();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          defs::xy &retval) {
    H5::CompType cType(sizeof(defs::xy));
    cType.insertMember("x", HOFFSET(defs::xy, x), H5::PredType::NATIVE_INT);
    cType.insertMember("y", HOFFSET(defs::xy, y), H5::PredType::NATIVE_INT);
    dataset.read(&retval, cType);
}
#endif

/** defs::scanParameters */
void read_from_json(const Document &doc, const std::string &name,
                    defs::scanParameters &retval) {
    const auto &s = doc[name.c_str()].GetObject();
    retval.enable = s["enable"].GetInt();
    retval.dacInd = static_cast<defs::dacIndex>(s["dacInd"].GetInt());
    retval.startOffset = s["start offset"].GetInt();
    retval.stopOffset = s["stop offset"].GetInt();
    retval.stepSize = s["step size"].GetInt();
    retval.dacSettleTime_ns = s["dac settle time ns"].GetInt64();
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          defs::scanParameters &retval) {
    H5::CompType cType(sizeof(defs::scanParameters));
    cType.insertMember("enable", HOFFSET(defs::scanParameters, enable),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("dacInd", HOFFSET(defs::scanParameters, dacInd),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("startOffset",
                       HOFFSET(defs::scanParameters, startOffset),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("stopOffset", HOFFSET(defs::scanParameters, stopOffset),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("stepSize", HOFFSET(defs::scanParameters, stepSize),
                       H5::PredType::NATIVE_INT);
    cType.insertMember("dacSettleTime_ns",
                       HOFFSET(defs::scanParameters, dacSettleTime_ns),
                       H5::PredType::STD_I64LE);
    dataset.read(&retval, cType);
}
#endif

/** std::map<std::string, std::string> */
void read_from_json(const Document &doc, const std::string &name,
                    std::map<std::string, std::string> &retval) {
    for (const auto &m : doc[name.c_str()].GetObject()) {
        retval[m.name.GetString()] = m.value.GetString();
    }
}
#ifdef HDF5C
void read_from_h5_dataset(const H5::DataSet &dataset, const std::string &name,
                          std::map<std::string, std::string> &retval) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    if (dims[0] == 0) {
        return; // empty dataset
    }
    auto strType = dataset.getStrType();
    H5::CompType mapType(sizeof(char *) * 2);
    mapType.insertMember("key", 0, strType);
    mapType.insertMember("value", sizeof(char *), strType);
    struct KeyValue {
        const char *key;
        const char *value;
    };
    std::vector<KeyValue> kv_vector(dims[0]);
    dataset.read(kv_vector.data(), mapType);
    for (const auto &kv : kv_vector) {
        retval[kv.key] = kv.value;
    }
}
#endif

/** test parameter in file */
template <typename T>
void test_json_parameter(const Document &doc, const std::string &name,
                         const T &expected) {
    REQUIRE(doc.HasMember(name.c_str()));
    T retval{};
    read_from_json(doc, name, retval);
    REQUIRE(retval == expected);
}
#ifdef HDF5C
template <typename T>
void test_h5_dataset(const std::string &name, const T &expected) {
    auto dataset = h5File->openDataSet(HDF5_GROUP + name);
    T retval{};
    read_from_h5_dataset(dataset, name, retval);
    REQUIRE(retval == expected);
}
#endif

template <typename T>
void check_master_file(const std::optional<Document> &doc,
                       const std::string &name, const T &expected) {
    if (doc.has_value()) {
        const auto &d = *doc;
        test_json_parameter(d, name, expected);
    } else {
#ifdef HDF5C
        if (!h5File.has_value()) {
            throw sls::RuntimeError("HDF5 file is not opened for testing " +
                                    name);
        }
        test_h5_dataset(name, expected);
#else
        throw sls::RuntimeError("Document is not available for testing " +
                                name);
#endif
    }
}

void test_master_file_version(const Detector &det,
                              const std::optional<Document> &doc) {
    // different values for json and hdf5
    // hdf5 version in atttribute and not dataset
    double retval{};
    std::string name = "Version";
    if (doc.has_value()) {
        const auto &d = *doc;
        REQUIRE(d.HasMember("Version"));
        read_from_json(d, name, retval);
        REQUIRE(retval == BINARY_WRITER_VERSION);
    } else {
#ifdef HDF5C
        if (!h5File.has_value()) {
            throw sls::RuntimeError(
                "HDF5 file is not opened for testing Version");
        }
        auto attr = h5File->openAttribute("Version");
        attr.read(attr.getDataType(), &retval);
        REQUIRE(retval == HDF5_WRITER_VERSION);
#else
        throw sls::RuntimeError(
            "Document is not available for testing Version");
#endif
    }
}

void test_master_file_type(const Detector &det,
                           const std::optional<Document> &doc) {
    auto det_type = det.getDetectorType().tsquash("Inconsistent detector type");
    check_master_file<std::string>(doc, "Detector Type", ToString(det_type));
}

void test_master_file_timing_mode(const Detector &det,
                                  const std::optional<Document> &doc) {
    auto timing_mode = det.getTimingMode().tsquash("Inconsistent timing mode");
    check_master_file<std::string>(doc, "Timing Mode", ToString(timing_mode));
}

void test_master_file_geometry(const Detector &det,
                               const std::optional<Document> &doc) {
    auto modGeometry = det.getModuleGeometry();
    auto portperModGeometry = det.getPortPerModuleGeometry();
    auto geometry = defs::xy{modGeometry.x * portperModGeometry.x,
                             modGeometry.y * portperModGeometry.y};
    check_master_file<defs::xy>(doc, "Geometry", geometry);
}

void test_master_file_image_size(const Detector &det,
                                 const std::optional<Document> &doc) {

    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    int bytes_per_pixel = det.getDynamicRange().squash() / 8;
    detParameters par(det_type);

    int image_size = 0;
    switch (det_type) {

    case defs::EIGER: {
        int num_chips = (par.nChipX / 2);
        image_size = par.nChanX * par.nChanY * num_chips * bytes_per_pixel;
    } break;

    case defs::JUNGFRAU:
    case defs::MOENCH: {
        auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");
        image_size = (par.nChanX * par.nChanY * par.nChipX * par.nChipY *
                      bytes_per_pixel) /
                     num_udp_interfaces;
    } break;

    case defs::MYTHEN3: {
        int counter_mask = det.getCounterMask().squash();
        int num_counters = __builtin_popcount(counter_mask);
        int num_channels_per_counter = par.nChanX / MAX_NUM_COUNTERS;
        image_size = num_channels_per_counter * num_counters * par.nChipX *
                     bytes_per_pixel;
    } break;

    case defs::GOTTHARD2: {
        image_size = par.nChanX * par.nChipX * bytes_per_pixel;
    } break;

    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD: {
        testCtbAcquireInfo test_info;
        image_size = calculate_ctb_image_size(
                         test_info, (det_type == defs::XILINX_CHIPTESTBOARD))
                         .first;
    } break;

    default:
        throw sls::RuntimeError("Unsupported detector type for this test");
    }

    check_master_file<int>(doc, "Image Size", image_size);
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

    check_master_file<defs::xy>(doc, "Pixels", portSize);
}

void test_master_file_max_frames_per_file(const Detector &det,
                                          const std::optional<Document> &doc) {
    auto max_frames_per_file =
        det.getFramesPerFile().tsquash("Inconsistent max frames per file");

    check_master_file<int>(doc, "Max Frames Per File", max_frames_per_file);
}

void test_master_file_frame_discard_policy(const Detector &det,
                                           const std::optional<Document> &doc) {
    auto policy = det.getRxFrameDiscardPolicy().tsquash(
        "Inconsistent frame discard policy");

    check_master_file<std::string>(doc, "Frame Discard Policy",
                                   ToString(policy));
}

void test_master_file_frame_padding(const Detector &det,
                                    const std::optional<Document> &doc) {
    auto padding = static_cast<int>(
        det.getPartialFramesPadding().tsquash("Inconsistent frame padding"));

    check_master_file<int>(doc, "Frame Padding", padding);
}

void test_master_file_scan_parameters(const Detector &det,
                                      const std::optional<Document> &doc) {
    auto scan_params = det.getScan().tsquash("Inconsistent scan parameters");

    check_master_file<defs::scanParameters>(doc, "Scan Parameters",
                                            scan_params);
}

void test_master_file_total_frames(const Detector &det,
                                   const std::optional<Document> &doc) {
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
        auto burst_mode = det.getBurstMode().tsquash("Inconsistent burst mode");
        auto numBursts =
            det.getNumberOfBursts().tsquash("Inconsistent number of bursts");
        // auto
        if (timing_mode == defs::AUTO_TIMING) {
            // burst mode, repeats = #bursts
            if (burst_mode == defs::BURST_INTERNAL ||
                burst_mode == defs::BURST_EXTERNAL) {
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
            if (burst_mode == defs::CONTINUOUS_INTERNAL ||
                burst_mode == defs::CONTINUOUS_EXTERNAL) {
                numFrames = 1;
            }
        }
    } else if (det_type == defs::JUNGFRAU) {
        numAdditionalStorageCells =
            det.getNumberOfAdditionalStorageCells().tsquash(
                "Inconsistent number of additional storage cells");
    }
    uint64_t total_frames =
        numFrames * repeats * (int64_t)(numAdditionalStorageCells + 1);

    check_master_file<uint64_t>(doc, "Total Frames", total_frames);
}

void test_master_file_rois(const Detector &det,
                           const std::optional<Document> &doc) {
    auto rois = det.getRxROI();
    auto detsize = det.getDetectorSize();
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    // compensate for m3 channel size and counter mask mess
    if (det_type == defs::MYTHEN3) {
        int nchan = detsize.x / MAX_NUM_COUNTERS;
        auto counter_mask = det.getCounterMask().tsquash(
            "Inconsistent counter mask for Mythen3 detector");
        int num_counters = __builtin_popcount(counter_mask);
        detsize.x = nchan * num_counters;
    }
    // replace -1 for complete ROI
    bool is2D = (detsize.y > 1);
    for (auto &roi : rois) {
        if (roi.completeRoi()) {
            roi.xmin = 0;
            roi.xmax = detsize.x - 1;
            if (is2D) {
                roi.ymin = 0;
                roi.ymax = detsize.y - 1;
            }
        }
    }

    check_master_file<std::vector<defs::ROI>>(doc, "Receiver Rois", rois);
}

void test_master_file_exptime(const Detector &det,
                              const std::optional<Document> &doc) {
    auto exptime = det.getExptime().tsquash("Inconsistent exposure time");

    check_master_file<std::string>(doc, "Exposure Time", ToString(exptime));
}

void test_master_file_period(const Detector &det,
                             const std::optional<Document> &doc) {
    auto period = det.getPeriod().tsquash("Inconsistent period");

    check_master_file<std::string>(doc, "Acquisition Period", ToString(period));
}

void test_master_file_num_udp_interfaces(const Detector &det,
                                         const std::optional<Document> &doc) {
    auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
        "Inconsistent number of UDP interfaces");

    check_master_file<int>(doc, "Number of UDP Interfaces", num_udp_interfaces);
}

void test_master_file_read_n_rows(const Detector &det,
                                  const std::optional<Document> &doc) {
    auto readnrows = det.getReadNRows().tsquash("Inconsistent number of rows");

    check_master_file<int>(doc, "Number of rows", readnrows);
}

void test_master_file_readout_speed(const Detector &det,
                                    const std::optional<Document> &doc) {
    auto readout_speed =
        det.getReadoutSpeed().tsquash("Inconsistent readout speed");

    check_master_file<std::string>(doc, "Readout Speed",
                                   ToString(readout_speed));
}

void test_master_file_frames_in_file(const std::optional<Document> &doc,
                                     const int frames_in_file) {
    check_master_file<int>(doc, "Frames in File", frames_in_file);
}

void test_master_file_json_header(const Detector &det,
                                  const std::optional<Document> &doc) {
    auto json_header =
        det.getAdditionalJsonHeader().tsquash("Inconsistent JSON header");

    check_master_file<std::map<std::string, std::string>>(
        doc, "Additional Json Header", json_header);
}

void test_master_file_dynamic_range(const Detector &det,
                                    const std::optional<Document> &doc) {
    auto dr = det.getDynamicRange().tsquash("Inconsistent dynamic range");

    check_master_file<int>(doc, "Dynamic Range", dr);
}

void test_master_file_ten_giga(const Detector &det,
                               const std::optional<Document> &doc) {
    auto ten_giga =
        static_cast<int>(det.getTenGiga().tsquash("Inconsistent ten giga"));

    check_master_file<int>(doc, "Ten Giga", ten_giga);
}

void test_master_file_threshold_energy(const Detector &det,
                                       const std::optional<Document> &doc) {
    auto threshold =
        det.getThresholdEnergy().tsquash("Inconsistent threshold energy");

    check_master_file<int>(doc, "Threshold Energy", threshold);
}

void test_master_file_sub_exptime(const Detector &det,
                                  const std::optional<Document> &doc) {
    auto sub_exptime =
        det.getSubExptime().tsquash("Inconsistent sub exposure time");

    check_master_file<std::string>(doc, "Sub Exposure Time",
                                   ToString(sub_exptime));
}

void test_master_file_sub_period(const Detector &det,
                                 const std::optional<Document> &doc) {
    auto exptime = det.getSubExptime().tsquash("Inconsistent sub exptime");
    auto deadtime = det.getSubDeadTime().tsquash("Inconsistent sub deadtime");
    auto sub_period = exptime + deadtime;

    check_master_file<std::string>(doc, "Sub Period", ToString(sub_period));
}

void test_master_file_quad(const Detector &det,
                           const std::optional<Document> &doc) {
    auto quad = static_cast<int>(det.getQuad().tsquash("Inconsistent quad"));

    check_master_file<int>(doc, "Quad", quad);
}

void test_master_file_rate_corrections(const Detector &det,
                                       const std::optional<Document> &doc) {
    std::vector<int64_t> dead_times;
    for (auto item : det.getRateCorrection())
        dead_times.push_back(item.count());

    check_master_file<std::vector<int64_t>>(doc, "Rate Corrections",
                                            dead_times);
}

void test_master_file_counter_mask(const Detector &det,
                                   const std::optional<Document> &doc) {
    auto counter_mask = static_cast<int>(
        det.getCounterMask().tsquash("Inconsistent counter mask"));

    check_master_file<int>(doc, "Counter Mask", counter_mask);
}

void test_master_file_exptimes(const Detector &det,
                               const std::optional<Document> &doc) {
    auto exptimes =
        det.getExptimeForAllGates().tsquash("Inconsistent exposure times");

    check_master_file<std::array<sls::ns, 3UL>>(doc, "Exposure Times",
                                                exptimes);
}

void test_master_file_gate_delays(const Detector &det,
                                  const std::optional<Document> &doc) {
    auto gate_delays =
        det.getGateDelayForAllGates().tsquash("Inconsistent GateDelay");

    check_master_file<std::array<sls::ns, 3UL>>(doc, "Gate Delays",
                                                gate_delays);
}

void test_master_file_gates(const Detector &det,
                            const std::optional<Document> &doc) {
    auto gates = det.getNumberOfGates().tsquash("Inconsistent number of gates");

    check_master_file<int>(doc, "Gates", gates);
}

void test_master_file_threadhold_energies(const Detector &det,
                                          const std::optional<Document> &doc) {
    auto threshold_energies =
        det.getAllThresholdEnergy().tsquash("Inconsistent threshold energies");

    check_master_file<std::array<int, 3UL>>(doc, "Threshold Energies",
                                            threshold_energies);
}

void test_master_file_burst_mode(const Detector &det,
                                 const std::optional<Document> &doc) {
    auto burst_mode = det.getBurstMode().tsquash("Inconsistent burst mode");

    check_master_file<std::string>(doc, "Burst Mode", ToString(burst_mode));
}

void test_master_file_adc_mask(const Detector &det,
                               const std::optional<Document> &doc) {
    testCtbAcquireInfo test_ctb_config;
    auto adc_mask = test_ctb_config.adc_enable_10g;
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto tengiga = test_ctb_config.ten_giga;
        if (!tengiga)
            adc_mask = test_ctb_config.adc_enable_1g;
    }

    check_master_file<uint32_t>(doc, "ADC Mask", adc_mask);
}

void test_master_file_analog_flag(const Detector &det,
                                  const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto romode = test_info.readout_mode;
    auto analog = static_cast<int>(
        (romode == defs::ANALOG_ONLY || romode == defs::ANALOG_AND_DIGITAL));

    check_master_file<int>(doc, "Analog Flag", analog);
}

void test_master_file_analog_samples(const Detector &det,
                                     const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto analog_samples = test_info.num_adc_samples;

    check_master_file<int>(doc, "Analog Samples", analog_samples);
}

void test_master_file_digital_flag(const Detector &det,
                                   const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto romode = test_info.readout_mode;
    auto digital = static_cast<int>(romode == defs::DIGITAL_ONLY ||
                                    romode == defs::ANALOG_AND_DIGITAL ||
                                    romode == defs::DIGITAL_AND_TRANSCEIVER);

    check_master_file<int>(doc, "Digital Flag", digital);
}

void test_master_file_digital_samples(const Detector &det,
                                      const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto digital_samples = test_info.num_dbit_samples;

    check_master_file<int>(doc, "Digital Samples", digital_samples);
}

void test_master_file_dbit_offset(const Detector &det,
                                  const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto dbit_offset = test_info.dbit_offset;

    check_master_file<int>(doc, "Dbit Offset", dbit_offset);
}

void test_master_file_dbit_reorder(const Detector &det,
                                   const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto dbit_reorder = test_info.dbit_reorder;

    check_master_file<int>(doc, "Dbit Reorder", dbit_reorder);
}

void test_master_file_dbit_bitset(const Detector &det,
                                  const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    uint64_t dbit_bitset = 0;
    for (auto &i : test_info.dbit_list) {
        dbit_bitset |= (static_cast<uint64_t>(1) << i);
    }

    check_master_file<uint64_t>(doc, "Dbit Bitset", dbit_bitset);
}

void test_master_file_transceiver_mask(const Detector &det,
                                       const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto trans_mask = test_info.transceiver_mask;

    check_master_file<int>(doc, "Transceiver Mask", trans_mask);
}

void test_master_file_transceiver_flag(const Detector &det,
                                       const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto romode = test_info.readout_mode;
    auto trans = static_cast<int>(romode == defs::DIGITAL_AND_TRANSCEIVER ||
                                  romode == defs::TRANSCEIVER_ONLY);

    check_master_file<int>(doc, "Transceiver Flag", trans);
}

void test_master_file_transceiver_samples(const Detector &det,
                                          const std::optional<Document> &doc) {
    testCtbAcquireInfo test_info;
    auto trans_samples = test_info.num_trans_samples;
    check_master_file<int>(doc, "Transceiver Samples", trans_samples);
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
    if (result == 0) {
        std::cout << "JSON parse error: " << GetParseError_En(result.Code())
                  << " (at offset " << result.Offset() << ")" << std::endl;

        // Optional: Show problematic snippet
        size_t offset = result.Offset();
        std::string context =
            json_str.substr(std::max(0, (int)offset - 20), 40);
        std::cout << "Context around error: \"" << context << "\"" << std::endl;
    }
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
    test_master_file_frames_in_file(doc, num_frames);

    // hdf5
#ifdef HDF5C
    fname = master_file_prefix + ".h5"; // /tmp/sls_test_master_0.h5
    try {
        open_hdf5_file(fname);
        test_master_file_metadata(det, std::nullopt);
        test_master_file_frames_in_file(std::nullopt, num_frames);
    } catch (H5::Exception &e) {
        LOG(logERROR) << "HDF5 error: " << e.getDetailMsg();
        throw;
    }
#endif
}

} // namespace sls
