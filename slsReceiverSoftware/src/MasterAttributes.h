// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "receiver_defs.h"
#include "sls/ToString.h"
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

class MasterAttributes {
  public:
    // (before acquisition)
    slsDetectorDefs::detectorType detType{slsDetectorDefs::GENERIC};
    slsDetectorDefs::timingMode timingMode{slsDetectorDefs::AUTO_TIMING};
    slsDetectorDefs::xy geometry{};
    uint32_t imageSize{0};
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
    uint32_t dynamicRange{0};
    uint32_t tenGiga{0};
    int thresholdEnergyeV{0};
    std::array<int, 3> thresholdAllEnergyeV = {{0, 0, 0}};
    ns subExptime{0};
    ns subPeriod{0};
    uint32_t quad{0};
    uint32_t readNRows;
    std::vector<int64_t> ratecorr;
    uint32_t adcmask{0};
    uint32_t analog{0};
    uint32_t analogSamples{0};
    uint32_t digital{0};
    uint32_t digitalSamples{0};
    uint32_t dbitreorder{1};
    uint32_t dbitoffset{0};
    uint64_t dbitlist{0};
    uint32_t transceiverMask{0};
    uint32_t transceiver{0};
    uint32_t transceiverSamples{0};
    std::vector<slsDetectorDefs::ROI> rois{};
    uint32_t counterMask{0};
    std::array<ns, 3> exptimeArray{};
    std::array<ns, 3> gateDelayArray{};
    uint32_t gates;
    std::map<std::string, std::string> additionalJsonHeader;
    uint64_t framesInFile{0};
    slsDetectorDefs::speedLevel readoutSpeed{slsDetectorDefs::FULL_SPEED};

    MasterAttributes() = default;
    ~MasterAttributes() = default;

    void
    GetBinaryAttributes(rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteHDF5Attributes(H5::H5File *fd, H5::Group *group);
#endif

    void GetCommonBinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
    void GetFinalBinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
    void GetBinaryRois(rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteHDF5String(H5::Group* group, const std::string& name, const std::string& value);
    template <typename T>
    void WriteHDF5Int(H5::Group* group, const std::string& name, const T& value) {
        H5::DataSpace dataspace(H5S_SCALAR);
        H5::PredType const* h5type;
        if constexpr (std::is_same_v<T, int>) {
            h5type = &H5::PredType::NATIVE_INT;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            h5type = &H5::PredType::STD_U64LE;
        } else {
            throw RuntimeError("Unsupported type for WriteHDF5Int");
        }
        H5::DataSet dataset = group->createDataSet(name, *h5type, dataspace);
        dataset.write(&value, *h5type);
    }

    void WriteCommonHDF5Attributes(H5::H5File *fd, H5::Group *group);
    void WriteFinalHDF5Attributes(H5::Group *group);
    void WriteHDF5ROIs(H5::Group *group);
    void WriteHDF5DynamicRange(H5::Group *group);
    void WriteHDF5TenGiga(H5::Group *group);
    void WriteHDF5NumUDPInterfaces(H5::Group *group);
    void WriteHDF5ReadNRows(H5::Group *group);
    void WriteHDF5ThresholdEnergy(H5::Group *group);
    void WriteHDF5ThresholdEnergies(H5::Group *group);
    void WriteHDF5SubExpTime(H5::Group *group);
    void WriteHDF5SubPeriod(H5::Group *group);
    void WriteHDF5SubQuad(H5::Group *group);
    void WriteHDF5RateCorrections(H5::Group *group);
    void WriteHDF5CounterMask(H5::Group *group);
    void WriteHDF5ExptimeArray(H5::Group *group);
    void WriteHDF5GateDelayArray(H5::Group *group);
    void WriteHDF5Gates(H5::Group *group);
    void WriteHDF5BurstMode(H5::Group *group);
    void WriteHDF5AdcMask(H5::Group *group);
    void WriteHDF5AnalogFlag(H5::Group *group);
    void WriteHDF5AnalogSamples(H5::Group *group);
    void WriteHDF5DigitalFlag(H5::Group *group);
    void WriteHDF5DigitalSamples(H5::Group *group);
    void WriteHDF5DbitOffset(H5::Group *group);
    void WriteHDF5DbitList(H5::Group *group);
    void WriteHDF5DbitReorder(H5::Group *group);
    void WriteHDF5TransceiverMask(H5::Group *group);
    void WriteHDF5TransceiverFlag(H5::Group *group);
    void WriteHDF5TransceiverSamples(H5::Group *group);
    void WriteHDF5ReadoutSpeed(H5::Group *group);
#endif

    void GetJungfrauBinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteJungfrauHDF5Attributes(H5::Group *group);
#endif

    void GetEigerBinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteEigerHDF5Attributes(H5::Group *group);
#endif

    void GetMythen3BinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteMythen3HDF5Attributes(H5::Group *group);
#endif

    void GetGotthard2BinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteGotthard2HDF5Attributes(H5::Group *group);
#endif

    void GetMoenchBinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteMoenchHDF5Attributes(H5::Group *group);
#endif

    void
    GetCtbBinaryAttributes(rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteCtbHDF5Attributes(H5::Group *group);
#endif

    void GetXilinxCtbBinaryAttributes(
        rapidjson::PrettyWriter<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    void WriteXilinxCtbHDF5Attributes(H5::Group *group);
#endif
};

} // namespace sls
