// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "receiver_defs.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <rapidjson/writer.h>

#ifdef HDF5C
#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif
#endif

#include <chrono>
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
    uint32_t dbitoffset{0};
    uint64_t dbitlist{0};
    slsDetectorDefs::ROI roi{};
    uint32_t counterMask{0};
    ns exptime1{0};
    ns exptime2{0};
    ns exptime3{0};
    ns gateDelay1{0};
    ns gateDelay2{0};
    ns gateDelay3{0};
    uint32_t gates;
    std::map<std::string, std::string> additionalJsonHeader;

    // Final Attributes (after acquisition)
    uint64_t framesInFile{0};

    MasterAttributes() = default;
    virtual ~MasterAttributes() = default;
    virtual void WriteMasterBinaryAttributes(FILE *fd);
    void
    GetBinaryMasterAttributes(rapidjson::Writer<rapidjson::StringBuffer> *w);
    void WriteBinaryAttributes(FILE *fd, std::string message);
    void
    WriteFinalBinaryAttributes(rapidjson::Writer<rapidjson::StringBuffer> *w);
#ifdef HDF5C
    virtual void WriteMasterHDF5Attributes(H5File *fd, Group *group);
    void WriteHDF5Attributes(H5File *fd, Group *group);
    void WriteFinalHDF5Attributes(H5File *fd, Group *group);
    void WriteHDF5Exptime(H5File *fd, Group *group);
    void WriteHDF5Period(H5File *fd, Group *group);
    void WriteHDF5DynamicRange(H5File *fd, Group *group);
    void WriteHDF5TenGiga(H5File *fd, Group *group);
#endif
};

class GotthardMasterAttributes : public MasterAttributes {
  public:
    GotthardMasterAttributes() = default;
    void WriteMasterBinaryAttributes(FILE *fd) override;
#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override;
#endif
};

class JungfrauMasterAttributes : public MasterAttributes {
  public:
    JungfrauMasterAttributes() = default;
    void WriteMasterBinaryAttributes(FILE *fd) override;
#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override;
#endif
};

class EigerMasterAttributes : public MasterAttributes {
  public:
    EigerMasterAttributes() = default;
    void WriteMasterBinaryAttributes(FILE *fd) override;
#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override;
#endif
};

class Mythen3MasterAttributes : public MasterAttributes {
  public:
    Mythen3MasterAttributes() = default;
    void WriteMasterBinaryAttributes(FILE *fd) override;
#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override;
#endif
};

class Gotthard2MasterAttributes : public MasterAttributes {
  public:
    Gotthard2MasterAttributes() = default;
    void WriteMasterBinaryAttributes(FILE *fd) override;
#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override;
#endif
};

class MoenchMasterAttributes : public MasterAttributes {
  public:
    MoenchMasterAttributes() = default;
    void WriteMasterBinaryAttributes(FILE *fd) override;
#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override;
#endif
};

class CtbMasterAttributes : public MasterAttributes {
  public:
    CtbMasterAttributes() = default;
    void WriteMasterBinaryAttributes(FILE *fd) override;
#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override;
#endif
};
