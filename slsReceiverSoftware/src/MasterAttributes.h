// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "receiver_defs.h"
#include "sls/ToString.h"
#include "sls/TypeTraits.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <chrono>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <string_view>

#ifdef HDF5C
#include "H5Cpp.h"
#endif

namespace sls {

using ns = std::chrono::nanoseconds;
using writer = rapidjson::PrettyWriter<rapidjson::StringBuffer>;

class MasterAttributes {
  public:
    // (before acquisition)
    slsDetectorDefs::detectorType detType{slsDetectorDefs::GENERIC};
    slsDetectorDefs::timingMode timingMode{slsDetectorDefs::AUTO_TIMING};
    slsDetectorDefs::xy geometry{};
    int imageSize{0};
    slsDetectorDefs::xy nPixels{};
    uint32_t maxFramesPerFile{0};
    slsDetectorDefs::frameDiscardPolicy frameDiscardMode{
        slsDetectorDefs::NO_DISCARD};
    int framePadding{1};
    slsDetectorDefs::scanParameters scanParams{};
    uint64_t totalFrames{0};
    ns exptime{0};
    ns period{0};
    slsDetectorDefs::burstMode burstMode{slsDetectorDefs::BURST_INTERNAL};
    int numUDPInterfaces{0};
    int dynamicRange{0};
    int tenGiga{0};
    int thresholdEnergyeV{0};
    std::array<int, 3> thresholdAllEnergyeV = {{0, 0, 0}};
    ns subExptime{0};
    ns subPeriod{0};
    int quad{0};
    int readNRows;
    std::vector<int64_t> ratecorr;
    uint32_t adcMask{0};
    int analog{0};
    int analogSamples{0};
    int digital{0};
    int digitalSamples{0};
    int dbitReorder{1};
    int dbitOffset{0};
    uint64_t dbitList{0};
    int transceiverMask{0};
    int transceiver{0};
    int transceiverSamples{0};
    std::vector<slsDetectorDefs::ROI> rois{};
    int counterMask{0};
    std::array<ns, 3> exptimeArray{};
    std::array<ns, 3> gateDelayArray{};
    int gates;
    std::map<std::string, std::string> additionalJsonHeader;
    uint64_t framesInFile{0};
    slsDetectorDefs::speedLevel readoutSpeed{slsDetectorDefs::FULL_SPEED};

    inline static const std::string_view N_DETECTOR_TYPE = "Detector Type";
    inline static const std::string_view N_TIMING_MODE = "Timing Mode";
    inline static const std::string_view N_GEOMETRY = "Geometry";
    inline static const std::string_view N_IMAGE_SIZE = "Image Size";
    inline static const std::string_view N_PIXELS = "Pixels";
    inline static const std::string_view N_MAX_FRAMES_PER_FILE =
        "Max Frames Per File";
    inline static const std::string_view N_FRAME_DISCARD_POLICY =
        "Frame Discard Policy";
    inline static const std::string_view N_FRAME_PADDING = "Frame Padding";
    inline static const std::string_view N_TOTAL_FRAMES = "Total Frames";
    inline static const std::string_view N_FRAMES_IN_FILE = "Frames in File";
    inline static const std::string_view N_EXPOSURE_TIME = "Exposure Time";
    inline static const std::string_view N_ACQUISITION_PERIOD =
        "Acquisition Period";
    inline static const std::string_view N_NUM_UDP_INTERFACES =
        "Number of UDP Interfaces";
    inline static const std::string_view N_NUMBER_OF_ROWS = "Number of Rows";
    inline static const std::string_view N_READOUT_SPEED = "Readout Speed";
    inline static const std::string_view N_DYNAMIC_RANGE = "Dynamic Range";
    inline static const std::string_view N_TEN_GIGA = "Ten Giga";
    inline static const std::string_view N_THRESHOLD_ENERGY =
        "Threshold Energy";
    inline static const std::string_view N_SUB_EXPOSURE_TIME =
        "Sub Exposure Time";
    inline static const std::string_view N_SUB_ACQUISITION_PERIOD =
        "Sub Acquisition Period";
    inline static const std::string_view N_QUAD = "Quad";
    inline static const std::string_view N_RATE_CORRECTIONS =
        "Rate Corrections";
    inline static const std::string_view N_COUNTER_MASK = "Counter Mask";
    inline static const std::string_view N_EXPOSURE_TIMES = "Exposure Times";
    inline static const std::string_view N_GATE_DELAYS = "Gate Delays";
    inline static const std::string_view N_GATES = "Gates";
    inline static const std::string_view N_THRESHOLD_ENERGIES =
        "Threshold Energies";
    inline static const std::string_view N_BURST_MODE = "Burst Mode";
    inline static const std::string_view N_ADC_MASK = "ADC Mask";
    inline static const std::string_view N_ANALOG = "Analog Flag";
    inline static const std::string_view N_ANALOG_SAMPLES = "Analog Samples";
    inline static const std::string_view N_DIGITAL = "Digital Flag";
    inline static const std::string_view N_DIGITAL_SAMPLES = "Digital Samples";
    inline static const std::string_view N_DBIT_REORDER = "Dbit Reorder";
    inline static const std::string_view N_DBIT_OFFSET = "Dbit Offset";
    inline static const std::string_view N_DBIT_BITSET = "Dbit Bitset";
    inline static const std::string_view N_TRANSCEIVER_MASK =
        "Transceiver Mask";
    inline static const std::string_view N_TRANSCEIVER = "Transceiver Flag";
    inline static const std::string_view N_TRANSCEIVER_SAMPLES =
        "Transceiver Samples";
    inline static const std::string_view N_VERSION = "Version";
    inline static const std::string_view N_TIMESTAMP = "Timestamp";
    inline static const std::string_view N_RECEIVER_ROIS = "Receiver Rois";
    inline static const std::string_view N_SCAN_PARAMETERS = "Scan Parameters";
    inline static const std::string_view N_ADDITIONAL_JSON_HEADER =
        "Additional JSON Header";

    MasterAttributes() = default;
    ~MasterAttributes() = default;

    void GetBinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteHDF5Attributes(H5::H5File *fd, H5::Group *group);
#endif

    void GetCommonBinaryAttributes(writer *w);
    void GetFinalBinaryAttributes(writer *w);

#ifdef HDF5C
    void WriteCommonHDF5Attributes(H5::H5File *fd, H5::Group *group);
    void WriteFinalHDF5Attributes(H5::Group *group);
#endif

    void GetJungfrauBinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteJungfrauHDF5Attributes(H5::Group *group);
#endif

    void GetEigerBinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteEigerHDF5Attributes(H5::Group *group);
#endif

    void GetMythen3BinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteMythen3HDF5Attributes(H5::Group *group);
#endif

    void GetGotthard2BinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteGotthard2HDF5Attributes(H5::Group *group);
#endif

    void GetMoenchBinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteMoenchHDF5Attributes(H5::Group *group);
#endif

    void GetCtbBinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteCtbHDF5Attributes(H5::Group *group);
#endif

    void GetXilinxCtbBinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteXilinxCtbHDF5Attributes(H5::Group *group);
#endif

    void WriteBinaryDetectorType(writer *w);
#ifdef HDF5C
    void WriteHDF5DetectorType(H5::Group *group);
#endif
    void WriteBinaryTimingMode(writer *w);
#ifdef HDF5C
    void WriteHDF5TimingMode(H5::Group *group);
#endif
    void WriteBinaryGeometry(writer *w);
#ifdef HDF5C
    void WriteHDF5Geometry(H5::Group *group);
#endif
    void WriteBinaryImageSize(writer *w);
#ifdef HDF5C
    void WriteHDF5ImageSize(H5::Group *group);
#endif
    void WriteBinaryPixels(writer *w);
#ifdef HDF5C
    void WriteHDF5Pixels(H5::Group *group);
#endif
    void WriteBinaryMaxFramesPerFile(writer *w);
#ifdef HDF5C
    void WriteHDF5MaxFramesPerFile(H5::Group *group);
#endif
    void WriteBinaryFrameDiscardPolicy(writer *w);
#ifdef HDF5C
    void WriteHDF5FrameDiscardPolicy(H5::Group *group);
#endif
    void WriteBinaryFramePadding(writer *w);
#ifdef HDF5C
    void WriteHDF5FramePadding(H5::Group *group);
#endif
    void WriteBinaryTotalFrames(writer *w);
#ifdef HDF5C
    void WriteHDF5TotalFrames(H5::Group *group);
#endif
    void WriteBinaryFramesInFile(writer *w);
#ifdef HDF5C
    void WriteHDF5FramesInFile(H5::Group *group);
#endif

    void WriteBinaryExposureTme(writer *w);
#ifdef HDF5C
    void WriteHDF5ExposureTime(H5::Group *group);
#endif
    void WriteBinaryAcquisitionPeriod(writer *w);
#ifdef HDF5C
    void WriteHDF5AcquisitionPeriod(H5::Group *group);
#endif
    void WriteBinaryNumberOfUDPInterfaces(writer *w);
#ifdef HDF5C
    void WriteHDF5NumberOfUDPInterfaces(H5::Group *group);
#endif
    void WriteBinaryNumberOfRows(writer *w);
#ifdef HDF5C
    void WriteHDF5NumberOfRows(H5::Group *group);
#endif
    void WriteBinaryReadoutSpeed(writer *w);
#ifdef HDF5C
    void WriteHDF5ReadoutSpeed(H5::Group *group);
#endif
    void WriteBinaryDynamicRange(writer *w);
#ifdef HDF5C
    void WriteHDF5DynamicRange(H5::Group *group);
#endif
    void WriteBinaryTenGiga(writer *w);
#ifdef HDF5C
    void WriteHDF5TenGiga(H5::Group *group);
#endif
    void WriteBinaryThresholdEnergy(writer *w);
#ifdef HDF5C
    void WriteHDF5ThresholdEnergy(H5::Group *group);
#endif
    void WriteBinarySubExposureTime(writer *w);
#ifdef HDF5C
    void WriteHDF5SubExposureTime(H5::Group *group);
#endif
    void WriteBinarySubAcquisitionPeriod(writer *w);
#ifdef HDF5C
    void WriteHDF5SubAcquisitionPeriod(H5::Group *group);
#endif
    void WriteBinaryQuad(writer *w);
#ifdef HDF5C
    void WriteHDF5Quad(H5::Group *group);
#endif
    void WriteBinaryRateCorrections(writer *w);
#ifdef HDF5C
    void WriteHDF5RateCorrections(H5::Group *group);
#endif
    void WriteBinaryCounterMask(writer *w);
#ifdef HDF5C
    void WriteHDF5CounterMask(H5::Group *group);
#endif
    void WriteBinaryExptimeArray(writer *w);
#ifdef HDF5C
    void WriteHDF5ExptimeArray(H5::Group *group);
#endif
    void WriteBinaryGateDelayArray(writer *w);
#ifdef HDF5C
    void WriteHDF5GateDelayArray(H5::Group *group);
#endif
    void WriteBinaryGates(writer *w);
#ifdef HDF5C
    void WriteHDF5Gates(H5::Group *group);
#endif
    void WriteBinaryThresholdAllEnergy(writer *w);
#ifdef HDF5C
    void WriteHDF5ThresholdAllEnergy(H5::Group *group);
#endif
    void WriteBinaryBurstMode(writer *w);
#ifdef HDF5C
    void WriteHDF5BurstMode(H5::Group *group);
#endif
    void WriteBinaryAdcMask(writer *w);
#ifdef HDF5C
    void WriteHDF5AdcMask(H5::Group *group);
#endif
    void WriteBinaryAnalogFlag(writer *w);
#ifdef HDF5C
    void WriteHDF5AnalogFlag(H5::Group *group);
#endif
    void WriteBinaryAnalogSamples(writer *w);
#ifdef HDF5C
    void WriteHDF5AnalogSamples(H5::Group *group);
#endif
    void WriteBinaryDigitalFlag(writer *w);
#ifdef HDF5C
    void WriteHDF5DigitalFlag(H5::Group *group);
#endif
    void WriteBinaryDigitalSamples(writer *w);
#ifdef HDF5C
    void WriteHDF5DigitalSamples(H5::Group *group);
#endif
    void WriteBinaryDBitReorder(writer *w);
#ifdef HDF5C
    void WriteHDF5DBitReorder(H5::Group *group);
#endif
    void WriteBinaryDBitOffset(writer *w);
#ifdef HDF5C
    void WriteHDF5DBitOffset(H5::Group *group);
#endif
    void WriteBinaryDBitBitset(writer *w);
#ifdef HDF5C
    void WriteHDF5DBitBitset(H5::Group *group);
#endif
    void WriteBinaryTransceiverMask(writer *w);
#ifdef HDF5C
    void WriteHDF5TransceiverMask(H5::Group *group);
#endif
    void WriteBinaryTransceiverFlag(writer *w);
#ifdef HDF5C
    void WriteHDF5TransceiverFlag(H5::Group *group);
#endif
    void WriteBinaryTransceiverSamples(writer *w);
#ifdef HDF5C
    void WriteHDF5TransceiverSamples(H5::Group *group);
#endif

    /** writes according to type */
    template <typename T> void WriteBinaryValue(writer *w, const T &value) {
        if constexpr (std::is_same_v<T, int>) {
            w->Int(value);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            w->Uint64(value);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            w->Int64(value);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            w->Uint(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            w->String(value.c_str());
        } else if constexpr (is_duration<T>::value) {
            w->String(ToString(value).c_str());
        } else {
            throw RuntimeError("Unsupported type for Binary write: " +
                               std::string(typeid(T).name()));
        }
    }

    /** For non-arrays */
    template <typename T>
    std::enable_if_t<(!std::is_class_v<T> || std::is_same_v<T, std::string>),
                     void>
    WriteBinary(writer *w, const std::string &name, const T &value) {
        w->Key(name.c_str());
        WriteBinaryValue(w, value);
    }

    /** For arrays */
    template <typename T>
    std::enable_if_t<(std::is_class_v<T> && !std::is_same_v<T, std::string>),
                     void>
    WriteBinary(writer *w, const std::string &name, const T &value) {
        w->Key(name.c_str());
        w->StartArray();
        for (const auto &v : value) {
            WriteBinaryValue(w, v);
        }
        w->EndArray();
    }

#ifdef HDF5C
    void WriteHDF5String(H5::Group *group, const std::string &name,
                         const std::string &value);
    void WriteHDF5StringArray(H5::Group *group, const std::string &name,
                              const std::vector<std::string> &value);

    /** get type */
    template <typename T> H5::PredType const *GetHDF5Type() {
        if constexpr (std::is_same_v<T, int>) {
            return &H5::PredType::NATIVE_INT;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return &H5::PredType::STD_U64LE;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return &H5::PredType::STD_I64LE;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return &H5::PredType::STD_U32LE;
        } else {
            throw RuntimeError("Unsupported type for HDF5");
        }
    }

    /** For non-arrays */
    template <typename T>
    typename std::enable_if<!std::is_class<T>::value, void>::type
    WriteHDF5Int(H5::Group *group, const std::string &name, const T &value) {
        H5::DataSpace dataspace(H5S_SCALAR);
        auto h5type = GetHDF5Type<T>();
        H5::DataSet dataset = group->createDataSet(name, *h5type, dataspace);
        dataset.write(&value, *h5type);
    }

    /** For arrays */
    template <typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    WriteHDF5Int(H5::Group *group, const std::string &name, const T &value) {
        using ElemT = typename T::value_type;
        auto h5type = GetHDF5Type<ElemT>();
        hsize_t dims[1] = {value.size()};
        H5::DataSpace dataspace(1, dims);
        H5::DataSet dataset = group->createDataSet(name, *h5type, dataspace);
        dataset.write(value.data(), *h5type);
    }

#endif
    void WriteBinaryXY(writer *w, const std::string &name, const defs::xy &xy);
#ifdef HDF5C
    void WriteHDF5XY(H5::Group *group, const std::string &name,
                     const defs::xy &xy);
#endif
    void WriteBinaryVersion(writer *w);
#ifdef HDF5C
    void WriteHDF5Version(H5::H5File *fd);
#endif
    void WriteBinaryTimestamp(writer *w);
#ifdef HDF5C
    void WriteHDF5Timestamp(H5::Group *group);
#endif
    void WriteBinaryRois(writer *w);
#ifdef HDF5C
    void WriteHDF5ROIs(H5::Group *group);
#endif
    void WriteBinaryScanParameters(writer *w);
#ifdef HDF5C
    void WriteHDF5ScanParameters(H5::Group *group);
#endif
    void WriteBinaryJsonHeader(writer *w);
#ifdef HDF5C
    void WriteHDF5JsonHeader(H5::Group *group);
#endif
    void WriteBinaryFrameHeaderFormat(writer *w);
};

} // namespace sls
