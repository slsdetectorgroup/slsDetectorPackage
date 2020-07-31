#pragma once

#include "ToString.h"
#include "logger.h"
#include "sls_detector_defs.h"

#ifdef HDF5C
#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif
#endif

#include <chrono>
using ns = std::chrono::nanoseconds;

// versions
#define HDF5_WRITER_VERSION   (6.1) // 1 decimal places
#define BINARY_WRITER_VERSION (6.1) // 1 decimal places

class MasterAttributes {
  public:
    slsDetectorDefs::detectorType detType{slsDetectorDefs::GENERIC};
    uint32_t imageSize{0};
    slsDetectorDefs::xy nPixels{};
    uint32_t maxFramesPerFile{0};
    uint64_t totalFrames{0};
    ns exptime{0};
    ns period{0};
    uint32_t dynamicRange{0};
    uint32_t tenGiga{0};
    ns subExptime{0};
    ns subPeriod{0};
    uint32_t quad{0};
    uint32_t adcmask{0};
    uint32_t analog{0};
    uint32_t digital{0};
    uint32_t dbitoffset{0};
    uint64_t dbitlist{0};
    slsDetectorDefs::ROI roi{};
    ns exptime1{0};
    ns exptime2{0};
    ns exptime3{0};
    ns gateDelay1{0};
    ns gateDelay2{0};
    ns gateDelay3{0};
    uint32_t gates;

    MasterAttributes(){};
    virtual ~MasterAttributes(){};

    virtual void WriteMasterBinaryAttributes(FILE *fd) {
        LOG(logERROR) << "WriteMasterBinaryAttributes should have been called "
                         "by a child class";
    }

    std::string GetBinaryMasterAttributes() {
        time_t t = time(nullptr);
        std::ostringstream oss;
        oss << "Version                    : " << std::setprecision(2)
            << BINARY_WRITER_VERSION << '\n'
            << "TimeStamp                  : " << ctime(&t) << '\n'
            << "Detector Type              : " << sls::ToString(detType) << '\n'
            << "Image Size                 : " << imageSize << " bytes" << '\n'
            << "Pixels                     : " << sls::ToString(nPixels) << '\n'
            << "Max Frames Per File        : " << maxFramesPerFile << '\n'
            << "Total Frames               : " << totalFrames << '\n';
        return oss.str();
    };

    void WriteBinaryAttributes(FILE *fd, std::string message) {
        message += std::string("\n#Frame Header\n"
                               "Frame Number               : 8 bytes\n"
                               "SubFrame Number/ExpLength  : 4 bytes\n"
                               "Packet Number              : 4 bytes\n"
                               "Bunch ID                   : 8 bytes\n"
                               "Timestamp                  : 8 bytes\n"
                               "Module Id                  : 2 bytes\n"
                               "Row                        : 2 bytes\n"
                               "Column                     : 2 bytes\n"
                               "Reserved                   : 2 bytes\n"
                               "Debug                      : 4 bytes\n"
                               "Round Robin Number         : 2 bytes\n"
                               "Detector Type              : 1 byte\n"
                               "Header Version             : 1 byte\n"
                               "Packets Caught Mask        : 64 bytes\n");

        if (fwrite((void *)message.c_str(), 1, message.length(), fd) !=
            message.length()) {
            throw sls::RuntimeError(
                "Master binary file incorrect number of bytes written to file");
        }
    };

#ifdef HDF5C
    virtual void Write
#endif
};

class GotthardMasterAttributes : public MasterAttributes {
  public:
    GotthardMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Roi (xmin, xmax)           : " << sls::ToString(roi) << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };
};

class JungfrauMasterAttributes : public MasterAttributes {
  public:
    JungfrauMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };
};

class EigerMasterAttributes : public MasterAttributes {
  public:
    EigerMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Dynamic Range              : " << dynamicRange << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "SubExptime                 : " << sls::ToString(subExptime)
            << '\n'
            << "SubPeriod                  : " << sls::ToString(subPeriod)
            << '\n'
            << "Quad                       : " << quad << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };
};

class Mythen3MasterAttributes : public MasterAttributes {
  public:
    Mythen3MasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Dynamic Range              : " << dynamicRange << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Exptime1                   : " << sls::ToString(exptime1)
            << '\n'
            << "Exptime2                   : " << sls::ToString(exptime2)
            << '\n'
            << "Exptime3                   : " << sls::ToString(exptime3)
            << '\n'
            << "GateDelay1                 : " << sls::ToString(gateDelay1)
            << '\n'
            << "GateDelay2                 : " << sls::ToString(gateDelay2)
            << '\n'
            << "GateDelay3                 : " << sls::ToString(gateDelay3)
            << '\n'
            << "Gates                      : " << gates << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };
};

class Gotthard2MasterAttributes : public MasterAttributes {
  public:
    Gotthard2MasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };
};

class MoenchMasterAttributes : public MasterAttributes {
  public:
    MoenchMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "ADC Mask                   : " << sls::ToStringHex(adcmask)
            << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };
};

class CtbMasterAttributes : public MasterAttributes {
  public:
    CtbMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "ADC Mask                   : " << sls::ToStringHex(adcmask)
            << '\n'
            << "Analog Flag                : " << analog << '\n'
            << "Digital Flag               : " << digital << '\n'
            << "Dbit Offset                : " << dbitoffset << '\n'
            << "Dbit Bitset                : " << dbitlist << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };
};