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
    uint32_t adcmask{0};
    int analog{0};
    int analogSamples{0};
    int digital{0};
    int digitalSamples{0};
    int dbitreorder{1};
    int dbitoffset{0};
    uint64_t dbitlist{0};
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

    MasterAttributes() = default;
    ~MasterAttributes() = default;

    void GetBinaryAttributes(writer *w);
#ifdef HDF5C
    void WriteHDF5Attributes(H5::H5File *fd, H5::Group *group);
#endif

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

    template <typename T>
    std::enable_if_t<(!std::is_class_v<T> || std::is_same_v<T, std::string>),
                     void>
    WriteBinary(writer *w, const std::string &name, const T &value) {
        w->Key(name.c_str());
        WriteBinaryValue(w, value);
    }

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

    void WriteBinaryXY(writer *w, const std::string &name, const defs::xy &xy);
    void WriteBinaryScanParameters(writer *w);
    void WriteBinaryJsonHeader(writer *w);
    void WriteBinaryRois(writer *w);
    void WriteBinaryFrameHeaderFormat(writer *w);
#ifdef HDF5C
    void WriteHDF5String(H5::Group *group, const std::string &name,
                         const std::string &value);
    void WriteHDF5StringArray(H5::Group *group, const std::string &name,
                              const std::vector<std::string> &value);
    void WriteHDF5XY(H5::Group *group, const std::string &name,
                     const defs::xy &xy);

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

    template <typename T>
    typename std::enable_if<!std::is_class<T>::value, void>::type
    WriteHDF5Int(H5::Group *group, const std::string &name, const T &value) {
        H5::DataSpace dataspace(H5S_SCALAR);
        auto h5type = GetHDF5Type<T>();
        H5::DataSet dataset = group->createDataSet(name, *h5type, dataspace);
        dataset.write(&value, *h5type);
    }

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

    void WriteHDF5ScanParameters(H5::Group *group);
    void WriteHDF5JsonHeader(H5::Group *group);
    void WriteHDF5ROIs(H5::Group *group);
    void WriteHDF5ExptimeArray(H5::Group *group);
    void WriteHDF5GateDelayArray(H5::Group *group);
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
};

} // namespace sls
