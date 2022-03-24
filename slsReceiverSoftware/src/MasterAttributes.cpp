// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterAttributes.h"

#include <rapidjson/stringbuffer.h>


void MasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
    LOG(logERROR) << "WriteMasterBinaryAttributes should have been called "
                     "by a child class";
}

void MasterAttributes::GetBinaryMasterAttributes(rapidjson::Writer<rapidjson::StringBuffer>* w) {
    time_t t = time(nullptr);
    
    w->Key("Version");
    w->SetMaxDecimalPlaces(2);
    w->Double(BINARY_WRITER_VERSION);

    w->Key("Timestamp");
    w->String(ctime(&t));

    w->Key("Detector Type");
    w->String(sls::ToString(detType).c_str());

    w->Key("Timing Mode");
    w->String(sls::ToString(timingMode).c_str());

    w->Key("Geometry");
    w->StartObject();
    w->Key("x");
    w->Uint(geometry.x);
    w->Key("y");
    w->Uint(geometry.y);
    w->EndObject();

    w->Key("Image Size in bytes");
    w->Uint(imageSize);

    w->Key("Pixels");
    w->StartArray();
    w->Uint(nPixels.x);
    w->Uint(nPixels.y);
    w->EndArray();

    w->Key("Max Frames Per File");
    w->Uint(maxFramesPerFile);

    w->Key("Frame Discard Policy");
    w->String(sls::ToString(frameDiscardMode).c_str());

    w->Key("Frame Padding");
    w->Uint(framePadding);

    w->Key("Scan Parameters");
    w->String(sls::ToString(scanParams).c_str());

    w->Key("Total Frames");
    w->Uint64(totalFrames);
};

void MasterAttributes::WriteBinaryAttributes(FILE *fd, std::string message) {
    if (fwrite((void *)message.c_str(), 1, message.length(), fd) !=
        message.length()) {
        throw sls::RuntimeError(
            "Master binary file incorrect number of bytes written to file");
    }
};

void MasterAttributes::WriteFinalBinaryAttributes(FILE *fd) {
 /*
    FILE* fp = fopen(json_file_name.c_str(), "r");
char readBuffer[65536];
FileReadStream is(fp, readBuffer, sizeof(readBuffer));

Document d, d2;
d.ParseStream(is);
assert(d.IsArray());
fclose(fp);
d2.SetObject();
Value json_objects(kObjectType);
json_objects.AddMember("three", 3, d2.GetAllocator());
d.PushBack(json_objects, d2.GetAllocator());

FILE* outfile = fopen(json_file_name.c_str(), "w");
char writeBuffer[65536];
FileWriteStream os(outfile, writeBuffer, sizeof(writeBuffer));

Writer writer(os);
d.Accept (writer);
fclose(outfile);
*/
    // adding few common parameters to the end
    /*std::ostringstream oss;

    if (!additionalJsonHeader.empty()) {
        oss << "Additional Json Header     : "
            << sls::ToString(additionalJsonHeader) << '\n';
    }
    oss << "Frames in File             : " << framesInFile << '\n';

    // adding sls_receiver header format
    oss << '\n'
        << "#Frame Header" << '\n'
        << "Frame Number               : 8 bytes" << '\n'
        << "SubFrame Number/ExpLength  : 4 bytes" << '\n'
        << "Packet Number              : 4 bytes" << '\n'
        << "Bunch ID                   : 8 bytes" << '\n'
        << "Timestamp                  : 8 bytes" << '\n'
        << "Module Id                  : 2 bytes" << '\n'
        << "Row                        : 2 bytes" << '\n'
        << "Column                     : 2 bytes" << '\n'
        << "Reserved                   : 2 bytes" << '\n'
        << "Debug                      : 4 bytes" << '\n'
        << "Round Robin Number         : 2 bytes" << '\n'
        << "Detector Type              : 1 byte" << '\n'
        << "Header Version             : 1 byte" << '\n'
        << "Packets Caught Mask        : 64 bytes" << '\n';

    std::string message = oss.str();

    // writing to file
    if (fwrite((void *)message.c_str(), 1, message.length(), fd) !=
        message.length()) {
        throw sls::RuntimeError(
            "Master binary file incorrect number of bytes written to file");
    }*/
};

#ifdef HDF5C
void MasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
    LOG(logERROR) << "WriteMasterHdf5Attributes should have been called "
                     "by a child class";
};

void MasterAttributes::WriteHDF5Attributes(H5File *fd, Group *group) {
    char c[1024];
    memset(c, 0, sizeof(c));
    // clang-format off
        // version
        {
            double version = BINARY_WRITER_VERSION;
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            Attribute attribute = fd->createAttribute(
                "Version", PredType::NATIVE_DOUBLE, dataspace);
            attribute.write(PredType::NATIVE_DOUBLE, &version);
        }
        // timestamp
        {
            time_t t = time(nullptr);
            StrType strdatatype(PredType::C_S1, 256);
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
            group->createDataSet("Timestamp", strdatatype, dataspace);
            sls::strcpy_safe(c, std::string(ctime(&t)));
            dataset.write(c, strdatatype);
        }
        // detector type
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Detector Type", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(detType));
            dataset.write(c, strdatatype);
        }
        // timing mode
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Timing Mode", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(timingMode));
            dataset.write(c, strdatatype);
        }
        //TODO: make this into an array?
        // geometry x
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Geometry in x axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&geometry.x, PredType::NATIVE_INT);
        }
        // geometry y
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Geometry in y axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&geometry.y, PredType::NATIVE_INT);
        }
        // Image Size
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Image Size", PredType::NATIVE_INT, dataspace);
            dataset.write(&imageSize, PredType::NATIVE_INT);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
            dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
            sls::strcpy_safe(c, "bytes");
            attribute.write(strdatatype, c);
        }
        //TODO: make this into an array?
        // npixels x
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of pixels in x axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&nPixels.x, PredType::NATIVE_INT);
        }
        // npixels y
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of pixels in y axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&nPixels.y, PredType::NATIVE_INT);
        }
        // Maximum frames per file
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Maximum frames per file", PredType::NATIVE_INT, dataspace);
            dataset.write(&maxFramesPerFile, PredType::NATIVE_INT);
        }
        // Frame Discard Policy
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Frame Discard Policy", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(frameDiscardMode));
            dataset.write(c, strdatatype);
        }        
        // Frame Padding
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Frame Padding", PredType::NATIVE_INT, dataspace);
            dataset.write(&framePadding, PredType::NATIVE_INT);
        }
        // Scan Parameters
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Scan Parameters", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(scanParams));
            dataset.write(c, strdatatype);
        }   
        // Total Frames
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Total Frames", PredType::STD_U64LE, dataspace);
            dataset.write(&totalFrames, PredType::STD_U64LE);
        }
    };

    void MasterAttributes::WriteFinalHDF5Attributes(H5File *fd, Group *group) {
        char c[1024];
        memset(c, 0, sizeof(c));
        // Total Frames in file
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Frames in File", PredType::STD_U64LE, dataspace);
            dataset.write(&framesInFile, PredType::STD_U64LE);
        }
        // additional json header
        if (!additionalJsonHeader.empty()) {
            std::string json = sls::ToString(additionalJsonHeader);
            StrType strdatatype(PredType::C_S1, json.length());
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
            group->createDataSet("Additional JSON Header", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(additionalJsonHeader));
            dataset.write(c, strdatatype);
        }
    };

    void MasterAttributes::WriteHDF5Exptime(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
        group->createDataSet("Exposure Time", strdatatype, dataspace);
        char c[1024];
        memset(c, 0, sizeof(c));
        sls::strcpy_safe(c, sls::ToString(exptime));
        dataset.write(c, strdatatype);
    };

    void MasterAttributes::WriteHDF5Period(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
        group->createDataSet("Acquisition Period", strdatatype, dataspace);
        char c[1024];
        memset(c, 0, sizeof(c));
        sls::strcpy_safe(c, sls::ToString(period));
        dataset.write(c, strdatatype);
    };

    void MasterAttributes::WriteHDF5DynamicRange(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet(
            "Dynamic Range", PredType::NATIVE_INT, dataspace);
        dataset.write(&dynamicRange, PredType::NATIVE_INT);
        DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        Attribute attribute =
        dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
        char c[1024] = "bits";
        attribute.write( strdatatype, c);
    };

    void MasterAttributes::WriteHDF5TenGiga(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet(
            "Ten Giga Enable", PredType::NATIVE_INT, dataspace);
        dataset.write(&tenGiga, PredType::NATIVE_INT);
    };
#endif

    void GotthardMasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
        std::ostringstream oss;
        oss //<< MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Roi (xmin, xmax)           : " << sls::ToString(roi) << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void GotthardMasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
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

    void JungfrauMasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        
        MasterAttributes::GetBinaryMasterAttributes(&writer);

        writer.Key("Exptime");
        writer.String(sls::ToString(exptime).c_str());
        
        writer.Key("Period");
        writer.String(sls::ToString(period).c_str());

        writer.Key("Number of UDP Interfaces");
        writer.Uint(numUDPInterfaces);        

        writer.Key("Number of rows");
        writer.Uint(readNRows); 

        writer.EndObject();
        
        std::string message = s.GetString();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void JungfrauMasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of UDP Interfaces", PredType::NATIVE_INT, dataspace);
            dataset.write(&numUDPInterfaces, PredType::NATIVE_INT);
        }
        // readNRows
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of rows", PredType::NATIVE_INT, dataspace);
            dataset.write(&readNRows, PredType::NATIVE_INT);
        }
    };
#endif

    void EigerMasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
        std::ostringstream oss;
        oss //<< MasterAttributes::GetBinaryMasterAttributes()
            << "Dynamic Range              : " << dynamicRange << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Threshold Energy           : " << thresholdEnergyeV << '\n'
            << "SubExptime                 : " << sls::ToString(subExptime)
            << '\n'
            << "SubPeriod                  : " << sls::ToString(subPeriod)
            << '\n'
            << "Quad                       : " << quad << '\n'
            << "Number of rows             : " << readNRows << '\n'
            << "Rate Corrections           : " << sls::ToString(ratecorr)
            << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void EigerMasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5DynamicRange(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        char c[1024];
        memset(c, 0, sizeof(c));
        // threshold
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Threshold Energy", PredType::NATIVE_INT, dataspace);
            dataset.write(&thresholdEnergyeV, PredType::NATIVE_INT);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
            sls::strcpy_safe(c, "eV");
            attribute.write(strdatatype, c);
        }
        // SubExptime
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset = group->createDataSet("Sub Exposure Time",
                                                   strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(subExptime));
            dataset.write(c, strdatatype);
        }
        // SubPeriod
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Sub Period", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(subPeriod));
            dataset.write(c, strdatatype);
        }
        // Quad
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group->createDataSet("Quad", PredType::NATIVE_INT, dataspace);
            dataset.write(&quad, PredType::NATIVE_INT);
        }
        // readNRows
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of rows", PredType::NATIVE_INT, dataspace);
            dataset.write(&readNRows, PredType::NATIVE_INT);
        }
        // Rate corrections
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 1024);
            DataSet dataset = group->createDataSet("Rate Corrections",
                                                   strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(ratecorr));
            dataset.write(c, strdatatype);
        }
    };
#endif

    void Mythen3MasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
        std::ostringstream oss;
        oss //<< MasterAttributes::GetBinaryMasterAttributes()
            << "Dynamic Range              : " << dynamicRange << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Counter Mask               : " << sls::ToStringHex(counterMask)
            << '\n'
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
            << "Gates                      : " << gates << '\n'
            << "Threshold Energies         : "
            << sls::ToString(thresholdAllEnergyeV) << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void Mythen3MasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5DynamicRange(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        char c[1024];
        memset(c, 0, sizeof(c));
        // Counter Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Counter Mask", PredType::STD_U32LE, dataspace);
            dataset.write(&counterMask, PredType::STD_U32LE);
        }
        // Exptime1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Exposure Time1", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(exptime1));
            dataset.write(c, strdatatype);
        }
        // Exptime2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Exposure Time2", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(exptime2));
            dataset.write(c, strdatatype);
        }
        // Exptime3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Exposure Time3", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(exptime3));
            dataset.write(c, strdatatype);
        }
        // GateDelay1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Gate Delay1", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(gateDelay1));
            dataset.write(c, strdatatype);
        }
        // GateDelay2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Gate Delay2", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(gateDelay2));
            dataset.write(c, strdatatype);
        }
        // GateDelay3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Gate Delay3", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(gateDelay3));
            dataset.write(c, strdatatype);
        }
        // Gates
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group->createDataSet("Gates", PredType::STD_U32LE, dataspace);
            dataset.write(&gates, PredType::STD_U32LE);
        }
        // Threshold Energies
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 1024);
            DataSet dataset = group->createDataSet("Threshold Energies",
                                                   strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(thresholdAllEnergyeV));
            dataset.write(c, strdatatype);
        }
    };
#endif

    void Gotthard2MasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
        std::ostringstream oss;
        oss //<< MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Burst Mode                 : " << sls::ToString(burstMode)
            << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void Gotthard2MasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        // burst mode
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Burst Mode", strdatatype, dataspace);
            char c[1024];
            memset(c, 0, sizeof(c));
            sls::strcpy_safe(c, sls::ToString(burstMode));
            dataset.write(c, strdatatype);
        }
    };
#endif

    void MoenchMasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
        std::ostringstream oss;
        oss //<< MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "ADC Mask                   : " << sls::ToStringHex(adcmask)
            << '\n'
            << "Analog Samples             : " << analogSamples << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void MoenchMasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        // ADC Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "ADC Mask", PredType::NATIVE_INT, dataspace);
            dataset.write(&adcmask, PredType::NATIVE_INT);
        }
        // Analog Samples
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Analog Samples", PredType::NATIVE_INT, dataspace);
            dataset.write(&analogSamples, PredType::NATIVE_INT);
        }
    };
#endif

    void CtbMasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
        std::ostringstream oss;
        oss //<< MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "ADC Mask                   : " << sls::ToStringHex(adcmask)
            << '\n'
            << "Analog Flag                : " << analog << '\n'
            << "Analog Samples             : " << analogSamples << '\n'
            << "Digital Flag               : " << digital << '\n'
            << "Digital Samples            : " << digitalSamples << '\n'
            << "Dbit Offset                : " << dbitoffset << '\n'
            << "Dbit Bitset                : " << dbitlist << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void CtbMasterAttributes::WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        // ADC Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "ADC mMsk", PredType::NATIVE_INT, dataspace);
            dataset.write(&adcmask, PredType::NATIVE_INT);
        }
        // Analog Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Analog Flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&analog, PredType::NATIVE_INT);
        }
        // Analog Samples
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Analog Samples", PredType::NATIVE_INT, dataspace);
            dataset.write(&analogSamples, PredType::NATIVE_INT);
        }
        // Digital Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Digital Flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&digital, PredType::NATIVE_INT);
        }
        // Digital Samples
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Digital Samples", PredType::NATIVE_INT, dataspace);
            dataset.write(&digitalSamples, PredType::NATIVE_INT);
        }
        // Dbit Offset
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Dbit Offset", PredType::NATIVE_INT, dataspace);
            dataset.write(&dbitoffset, PredType::NATIVE_INT);
        }
        // Dbit List
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Dbit Bitset List", PredType::STD_U64LE, dataspace);
            dataset.write(&dbitlist, PredType::STD_U64LE);
        }
    };
#endif
