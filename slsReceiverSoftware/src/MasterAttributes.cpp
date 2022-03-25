// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterAttributes.h"

#include <rapidjson/stringbuffer.h>

void MasterAttributes::WriteMasterBinaryAttributes(FILE *fd) {
    LOG(logERROR) << "WriteMasterBinaryAttributes should have been called "
                     "by a child class";
}

void MasterAttributes::GetBinaryMasterAttributes(
    rapidjson::Writer<rapidjson::StringBuffer> *w) {
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
    w->StartObject();
    w->Key("x");
    w->Uint(nPixels.x);
    w->Key("y");
    w->Uint(nPixels.y);
    w->EndObject();

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

void MasterAttributes::WriteFinalBinaryAttributes(
    rapidjson::Writer<rapidjson::StringBuffer> *w) {
    // adding few common parameters to the end

    if (!additionalJsonHeader.empty()) {
        w->Key("Additional Json Header");
        w->String(sls::ToString(additionalJsonHeader).c_str());
    }

    w->Key("Frames in File");
    w->Uint64(framesInFile);

    w->Key("Frame Header Format");
    w->StartObject();
    w->Key("Frame Number");
    w->String("8 bytes");
    w->Key("SubFrame Number/ExpLength");
    w->String("4 bytes");
    w->Key("Packet Number");
    w->String("4 bytes");
    w->Key("Bunch ID");
    w->String("8 bytes");
    w->Key("Timestamp");
    w->String("8 bytes");
    w->Key("Module Id");
    w->String("2 bytes");
    w->Key("Row");
    w->String("2 bytes");
    w->Key("Column");
    w->String("2 bytes");
    w->Key("Reserved");
    w->String("2 bytes");
    w->Key("Debug");
    w->String("4 bytes");
    w->Key("Round Robin Number");
    w->String("2 bytes");
    w->Key("Detector Type");
    w->String("1 byte");
    w->Key("Header Version");
    w->String("1 byte");
    w->Key("Packets Caught Mask");
    w->String("64 bytes");
    w->EndObject();
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
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        MasterAttributes::GetBinaryMasterAttributes(&writer);

        writer.Key("Exptime");
        writer.String(sls::ToString(exptime).c_str());
        
        writer.Key("Period");
        writer.String(sls::ToString(period).c_str());

        writer.Key("Roi (xmin, xmax)");
        writer.String(sls::ToString(roi).c_str());

        MasterAttributes::WriteFinalBinaryAttributes(&writer);   
        writer.EndObject();
        std::string message = s.GetString();
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

        MasterAttributes::WriteFinalBinaryAttributes(&writer);   
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
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        MasterAttributes::GetBinaryMasterAttributes(&writer);

        writer.Key("Dynamic Range");
        writer.Uint(dynamicRange);

        writer.Key("Ten Giga");
        writer.Uint(tenGiga);

        writer.Key("Exptime");
        writer.String(sls::ToString(exptime).c_str());
        
        writer.Key("Period");
        writer.String(sls::ToString(period).c_str());

        writer.Key("Threshold Energy");
        writer.Int(thresholdEnergyeV);

        writer.Key("Sub Exptime");
        writer.String(sls::ToString(subExptime).c_str());
        
        writer.Key("Sub Period");
        writer.String(sls::ToString(subPeriod).c_str());

        writer.Key("Quad");
        writer.Int(quad);

        writer.Key("Number of rows");
        writer.Int(readNRows);

        writer.Key("Rate Corrections");
        writer.String(sls::ToString(ratecorr).c_str());

        MasterAttributes::WriteFinalBinaryAttributes(&writer);   
        writer.EndObject();
        std::string message = s.GetString();
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
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        MasterAttributes::GetBinaryMasterAttributes(&writer);

        writer.Key("Dynamic Range");
        writer.Uint(dynamicRange);

        writer.Key("Ten Giga");
        writer.Uint(tenGiga);

        writer.Key("Period");
        writer.String(sls::ToString(period).c_str());

        writer.Key("Counter Mask");
        writer.String(sls::ToStringHex(counterMask).c_str());

        writer.Key("Exptime1");
        writer.String(sls::ToString(exptime1).c_str());
    
        writer.Key("Exptime2");
        writer.String(sls::ToString(exptime2).c_str());

        writer.Key("Exptime3");
        writer.String(sls::ToString(exptime3).c_str());

        writer.Key("GateDelay1");
        writer.String(sls::ToString(gateDelay1).c_str());
    
        writer.Key("GateDelay2");
        writer.String(sls::ToString(gateDelay2).c_str());

        writer.Key("GateDelay3");
        writer.String(sls::ToString(gateDelay3).c_str());

        writer.Key("Gates");
        writer.Uint(gates);

        writer.Key("Threshold Energies");
        writer.String(sls::ToString(thresholdAllEnergyeV).c_str());

        MasterAttributes::WriteFinalBinaryAttributes(&writer);   
        writer.EndObject();
        std::string message = s.GetString();
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
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        MasterAttributes::GetBinaryMasterAttributes(&writer);

        writer.Key("Exptime");
        writer.String(sls::ToString(exptime).c_str());
        
        writer.Key("Period");
        writer.String(sls::ToString(period).c_str());

        writer.Key("Burst Mode");
        writer.String(sls::ToString(burstMode).c_str());

        MasterAttributes::WriteFinalBinaryAttributes(&writer);   
        writer.EndObject();
        std::string message = s.GetString();
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
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        MasterAttributes::GetBinaryMasterAttributes(&writer);

        writer.Key("Exptime");
        writer.String(sls::ToString(exptime).c_str());
        
        writer.Key("Period");
        writer.String(sls::ToString(period).c_str());

        writer.Key("Ten Giga");
        writer.Uint(tenGiga);

        writer.Key("ADC Mask");
        writer.String(sls::ToStringHex(adcmask).c_str());

        writer.Key("Analog Samples");
        writer.Uint(analogSamples);

        MasterAttributes::WriteFinalBinaryAttributes(&writer);   
        writer.EndObject();
        std::string message = s.GetString();
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
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        MasterAttributes::GetBinaryMasterAttributes(&writer);

        writer.Key("Exptime");
        writer.String(sls::ToString(exptime).c_str());
        
        writer.Key("Period");
        writer.String(sls::ToString(period).c_str());

        writer.Key("Ten Giga");
        writer.Uint(tenGiga);

        writer.Key("ADC Mask");
        writer.String(sls::ToStringHex(adcmask).c_str());

        writer.Key("Analog Flag");
        writer.Uint(analog);

        writer.Key("Analog Samples");
        writer.Uint(analogSamples);

        writer.Key("Digital Flag");
        writer.Uint(digital);

        writer.Key("Digital Samples");
        writer.Uint(digitalSamples);

        writer.Key("Dbit Offset");
        writer.Uint(dbitoffset);

        writer.Key("Dbit Bitset");
        writer.Uint64(dbitlist);

        MasterAttributes::WriteFinalBinaryAttributes(&writer);   
        writer.EndObject();
        std::string message = s.GetString();
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
