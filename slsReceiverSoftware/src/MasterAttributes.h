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
    virtual void WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        LOG(logERROR) << "WriteMasterHdf5Attributes should have been called "
                         "by a child class";
    };

    void WriteHDF5Attributes(H5File *fd, Group *group){
        // clang-format off
        // version
        {
            double version = BINARY_WRITER_VERSION;
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            Attribute attribute = fd->createAttribute(
                "version", PredType::NATIVE_DOUBLE, dataspace);
            attribute.write(PredType::NATIVE_DOUBLE, &version);
        }
        // timestamp
        {
            time_t t = time(nullptr);
            StrType strdatatype(PredType::C_S1, 256);
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group->createDataSet("timestamp", strdatatype, dataspace);
            dataset.write(std::string(ctime(&t)), strdatatype);
        }
        // detector type
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("detector type", strdatatype, dataspace);
            dataset.write(sls::ToString(detType), strdatatype);
        }
        // Image Size
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "image size", PredType::NATIVE_INT, dataspace);
            dataset.write(&imageSize, PredType::NATIVE_INT);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("bytes"));
        }
        //TODO: make this into an array?
        // x
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "number of pixels in x axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&nPixels.x, PredType::NATIVE_INT);
        }
        // y
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "number of pixels in y axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&nPixels.y, PredType::NATIVE_INT);
        }
        // Maximum frames per file
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "maximum frames per file", PredType::NATIVE_INT, dataspace);
            dataset.write(&maxFramesPerFile, PredType::NATIVE_INT);
        }
        // Total Frames
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "total frames", PredType::STD_U64LE, dataspace);
            dataset.write(&totalFrames, PredType::STD_U64LE);
        }
    };

    void WriteHDF5Exptime(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
            group->createDataSet("exposure time", strdatatype, dataspace);
        dataset.write(sls::ToString(exptime), strdatatype);
    };

    void WriteHDF5Period(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
            group->createDataSet("acquisition period", strdatatype, dataspace);
        dataset.write(sls::ToString(period), strdatatype);
    };

    void WriteHDF5DynamicRange(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet(
            "dynamic range", PredType::NATIVE_INT, dataspace);
        dataset.write(&dynamicRange, PredType::NATIVE_INT);
        DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        Attribute attribute =
            dataset.createAttribute("unit", strdatatype, dataspaceAttr);
        attribute.write(strdatatype, std::string("bits"));
    };

    void WriteHDF5TenGiga(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet(
            "ten giga enable", PredType::NATIVE_INT, dataspace);
        dataset.write(&tenGiga, PredType::NATIVE_INT);
    };
#endif
};
// clang-format on

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

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        // Roi xmin
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "roi xmin", PredType::NATIVE_INT, dataspace);
            dataset.write(&roi.xmin, PredType::NATIVE_INT);
        }
        // Roi xmax
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "roi xmax", PredType::NATIVE_INT, dataspace);
            dataset.write(&roi.xmax, PredType::NATIVE_INT);
        }
    };
#endif
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

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
    };
#endif
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

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5DynamicRange(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        // SubExptime
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset = group->createDataSet("sub exposure time",
                                                   strdatatype, dataspace);
            dataset.write(sls::ToString(subExptime), strdatatype);
        }
        // SubPeriod
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("sub period", strdatatype, dataspace);
            dataset.write(sls::ToString(subPeriod), strdatatype);
        }
        // Quad
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group->createDataSet("quad", PredType::NATIVE_INT, dataspace);
            dataset.write(&quad, PredType::NATIVE_INT);
        }
    };
#endif
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

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5DynamicRange(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        // Exptime1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("exposure time1", strdatatype, dataspace);
            dataset.write(sls::ToString(exptime1), strdatatype);
        }
        // Exptime2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("exposure time2", strdatatype, dataspace);
            dataset.write(sls::ToString(exptime2), strdatatype);
        }
        // Exptime3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("exposure time3", strdatatype, dataspace);
            dataset.write(sls::ToString(exptime3), strdatatype);
        }
        // GateDelay1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("gate delay1", strdatatype, dataspace);
            dataset.write(sls::ToString(gateDelay1), strdatatype);
        }
        // GateDelay2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("gate delay2", strdatatype, dataspace);
            dataset.write(sls::ToString(gateDelay2), strdatatype);
        }
        // GateDelay3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("gate delay3", strdatatype, dataspace);
            dataset.write(sls::ToString(gateDelay3), strdatatype);
        }
        // Gates
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group->createDataSet("gates", PredType::STD_U32LE, dataspace);
            dataset.write(&gates, PredType::STD_U64LE);
        }
    };
#endif
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

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
    };
#endif
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

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        // ADC Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "adc mask", PredType::NATIVE_INT, dataspace);
            dataset.write(&adcmask, PredType::NATIVE_INT);
        }
    };
#endif
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

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        // ADC Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "adc mask", PredType::NATIVE_INT, dataspace);
            dataset.write(&adcmask, PredType::NATIVE_INT);
        }
        // Analog Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "analog flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&analog, PredType::NATIVE_INT);
        }
        // Digital Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "digital flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&digital, PredType::NATIVE_INT);
        }
        // Dbit Offset
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "dbit offset", PredType::NATIVE_INT, dataspace);
            dataset.write(&dbitoffset, PredType::NATIVE_INT);
        }
        // Dbit List
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "dbit bitset list", PredType::STD_U64LE, dataspace);
            dataset.write(&dbitlist, PredType::STD_U64LE);
        }
    };
#endif
};