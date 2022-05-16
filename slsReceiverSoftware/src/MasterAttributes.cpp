// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterAttributes.h"

void MasterAttributes::GetBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->StartObject();
    GetCommonBinaryAttributes(w);
    switch (detType) {
    case slsDetectorDefs::GOTTHARD:
        GetGotthardBinaryAttributes(w);
        break;
    case slsDetectorDefs::JUNGFRAU:
        GetJungfrauBinaryAttributes(w);
        break;
    case slsDetectorDefs::EIGER:
        GetEigerBinaryAttributes(w);
        break;
    case slsDetectorDefs::MYTHEN3:
        GetMythen3BinaryAttributes(w);
        break;
    case slsDetectorDefs::GOTTHARD2:
        GetGotthard2BinaryAttributes(w);
        break;
    case slsDetectorDefs::MOENCH:
        GetMoenchBinaryAttributes(w);
        break;
    case slsDetectorDefs::CHIPTESTBOARD:
        GetCtbBinaryAttributes(w);
        break;
    default:
        throw sls::RuntimeError(
            "Unknown Detector type to get master attributes");
    }
    GetFinalBinaryAttributes(w);
    w->EndObject();
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5Attributes(H5File *fd, Group *group) {
    WriteCommonHDF5Attributes(fd, group);
    switch (detType) {
    case slsDetectorDefs::GOTTHARD:
        WriteGotthardHDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::JUNGFRAU:
        WriteJungfrauHDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::EIGER:
        WriteEigerHDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::MYTHEN3:
        WriteMythen3HDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::GOTTHARD2:
        WriteGotthard2HDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::MOENCH:
        WriteMoenchHDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::CHIPTESTBOARD:
        WriteCtbHDF5Attributes(fd, group);
        break;
    default:
        throw sls::RuntimeError(
            "Unknown Detector type to get master attributes");
    }
    WriteFinalHDF5Attributes(fd, group);
}
#endif

void MasterAttributes::GetCommonBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Version");
    w->SetMaxDecimalPlaces(2);
    w->Double(BINARY_WRITER_VERSION);
    w->Key("Timestamp");
    time_t t = time(nullptr);
    std::string sTime(ctime(&t));
    std::replace(sTime.begin(), sTime.end(), '\n', '\0');
    w->String(sTime.c_str());
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
    w->Key("Receiver Roi");
    w->StartObject();
    w->Key("xmin");
    w->Uint(receiverRoi.xmin);
    w->Key("xmax");
    w->Uint(receiverRoi.xmax);
    w->Key("ymin");
    w->Uint(receiverRoi.ymin);
    w->Key("ymax");
    w->Uint(receiverRoi.ymax);
    w->EndObject();
}

void MasterAttributes::GetFinalBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
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
}

#ifdef HDF5C
void MasterAttributes::WriteCommonHDF5Attributes(H5File *fd, Group *group) {
    char c[1024]{};
    // version
    {
        double version = BINARY_WRITER_VERSION;
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        Attribute attribute =
            fd->createAttribute("Version", PredType::NATIVE_DOUBLE, dataspace);
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
    // TODO: make this into an array?
    // geometry x
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Geometry in x axis",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&geometry.x, PredType::NATIVE_INT);
    }
    // geometry y
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Geometry in y axis",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&geometry.y, PredType::NATIVE_INT);
    }
    // Image Size
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset =
            group->createDataSet("Image Size", PredType::NATIVE_INT, dataspace);
        dataset.write(&imageSize, PredType::NATIVE_INT);
        DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        Attribute attribute =
            dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
        sls::strcpy_safe(c, "bytes");
        attribute.write(strdatatype, c);
    }
    // TODO: make this into an array?
    // npixels x
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Number of pixels in x axis",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&nPixels.x, PredType::NATIVE_INT);
    }
    // npixels y
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Number of pixels in y axis",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&nPixels.y, PredType::NATIVE_INT);
    }
    // Maximum frames per file
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Maximum frames per file",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&maxFramesPerFile, PredType::NATIVE_INT);
    }
    // Frame Discard Policy
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset = group->createDataSet("Frame Discard Policy",
                                               strdatatype, dataspace);
        sls::strcpy_safe(c, sls::ToString(frameDiscardMode));
        dataset.write(c, strdatatype);
    }
    // Frame Padding
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Frame Padding",
                                               PredType::NATIVE_INT, dataspace);
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
        DataSet dataset = group->createDataSet("Total Frames",
                                               PredType::STD_U64LE, dataspace);
        dataset.write(&totalFrames, PredType::STD_U64LE);
    }
    // Receiver Roi xmin
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("receiver roi xmin",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.xmin, PredType::NATIVE_INT);
    }
    // Receiver Roi xmax
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("receiver roi xmax",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.xmax, PredType::NATIVE_INT);
    }
    // Receiver Roi ymin
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("receiver roi ymin",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.ymin, PredType::NATIVE_INT);
    }
    // Receiver Roi ymax
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("receiver roi ymax",
                                               PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.ymax, PredType::NATIVE_INT);
    }
}

void MasterAttributes::WriteFinalHDF5Attributes(H5File *fd, Group *group) {
    char c[1024]{};
    // Total Frames in file
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Frames in File",
                                               PredType::STD_U64LE, dataspace);
        dataset.write(&framesInFile, PredType::STD_U64LE);
    }
    // additional json header
    if (!additionalJsonHeader.empty()) {
        std::string json = sls::ToString(additionalJsonHeader);
        StrType strdatatype(PredType::C_S1, json.length());
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet("Additional JSON Header",
                                               strdatatype, dataspace);
        sls::strcpy_safe(c, sls::ToString(additionalJsonHeader));
        dataset.write(c, strdatatype);
    }
}

void MasterAttributes::WriteHDF5Exptime(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 256);
    DataSet dataset =
        group->createDataSet("Exposure Time", strdatatype, dataspace);
    char c[1024]{};
    sls::strcpy_safe(c, sls::ToString(exptime));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5Period(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 256);
    DataSet dataset =
        group->createDataSet("Acquisition Period", strdatatype, dataspace);
    char c[1024]{};
    sls::strcpy_safe(c, sls::ToString(period));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5DynamicRange(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Dynamic Range", PredType::NATIVE_INT, dataspace);
    dataset.write(&dynamicRange, PredType::NATIVE_INT);
    DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 256);
    Attribute attribute =
        dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
    char c[1024] = "bits";
    attribute.write(strdatatype, c);
}

void MasterAttributes::WriteHDF5TenGiga(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset = group->createDataSet("Ten Giga Enable",
                                           PredType::NATIVE_INT, dataspace);
    dataset.write(&tenGiga, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5ROI(H5File *fd, Group *group) {
    // Roi xmin
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset =
            group->createDataSet("roi xmin", PredType::NATIVE_INT, dataspace);
        dataset.write(&detectorRoi.xmin, PredType::NATIVE_INT);
    }
    // Roi xmax
    {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset =
            group->createDataSet("roi xmax", PredType::NATIVE_INT, dataspace);
        dataset.write(&detectorRoi.xmax, PredType::NATIVE_INT);
    }
}

void MasterAttributes::WriteHDF5NumUDPInterfaces(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset = group->createDataSet("Number of UDP Interfaces",
                                           PredType::NATIVE_INT, dataspace);
    dataset.write(&numUDPInterfaces, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5ReadNRows(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Number of rows", PredType::NATIVE_INT, dataspace);
    dataset.write(&readNRows, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5ThresholdEnergy(H5File *fd, Group *group) {
    char c[1024]{};
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset = group->createDataSet("Threshold Energy",
                                           PredType::NATIVE_INT, dataspace);
    dataset.write(&thresholdEnergyeV, PredType::NATIVE_INT);
    DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 256);
    Attribute attribute =
        dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
    sls::strcpy_safe(c, "eV");
    attribute.write(strdatatype, c);
}

void MasterAttributes::WriteHDF5ThresholdEnergies(H5File *fd, Group *group) {
    char c[1024]{};
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 1024);
    DataSet dataset =
        group->createDataSet("Threshold Energies", strdatatype, dataspace);
    sls::strcpy_safe(c, sls::ToString(thresholdAllEnergyeV));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5SubExpTime(H5File *fd, Group *group) {
    char c[1024]{};
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 256);
    DataSet dataset =
        group->createDataSet("Sub Exposure Time", strdatatype, dataspace);
    sls::strcpy_safe(c, sls::ToString(subExptime));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5SubPeriod(H5File *fd, Group *group) {
    char c[1024]{};
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 256);
    DataSet dataset =
        group->createDataSet("Sub Period", strdatatype, dataspace);
    sls::strcpy_safe(c, sls::ToString(subPeriod));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5SubQuad(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Quad", PredType::NATIVE_INT, dataspace);
    dataset.write(&quad, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5RateCorrections(H5File *fd, Group *group) {
    char c[1024]{};
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 1024);
    DataSet dataset =
        group->createDataSet("Rate Corrections", strdatatype, dataspace);
    sls::strcpy_safe(c, sls::ToString(ratecorr));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5CounterMask(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Counter Mask", PredType::STD_U32LE, dataspace);
    dataset.write(&counterMask, PredType::STD_U32LE);
}

void MasterAttributes::WriteHDF5ExptimeArray(H5File *fd, Group *group) {
    for (int i = 0; i != 3; ++i) {
        char c[1024]{};
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
            group->createDataSet("Exposure Time1", strdatatype, dataspace);
        sls::strcpy_safe(c, sls::ToString(exptimeArray[i]));
        dataset.write(c, strdatatype);
    }
}

void MasterAttributes::WriteHDF5GateDelayArray(H5File *fd, Group *group) {
    for (int i = 0; i != 3; ++i) {
        char c[1024]{};
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
            group->createDataSet("Gate Delay1", strdatatype, dataspace);
        sls::strcpy_safe(c, sls::ToString(gateDelayArray[i]));
        dataset.write(c, strdatatype);
    }
}

void MasterAttributes::WriteHDF5Gates(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Gates", PredType::STD_U32LE, dataspace);
    dataset.write(&gates, PredType::STD_U32LE);
}

void MasterAttributes::WriteHDF5BurstMode(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    StrType strdatatype(PredType::C_S1, 256);
    DataSet dataset =
        group->createDataSet("Burst Mode", strdatatype, dataspace);
    char c[1024]{};
    sls::strcpy_safe(c, sls::ToString(burstMode));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5AdcMask(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("ADC Mask", PredType::NATIVE_INT, dataspace);
    dataset.write(&adcmask, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5AnalogFlag(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Analog Flag", PredType::NATIVE_INT, dataspace);
    dataset.write(&analog, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5AnalogSamples(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Analog Samples", PredType::NATIVE_INT, dataspace);
    dataset.write(&analogSamples, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DigitalFlag(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Digital Flag", PredType::NATIVE_INT, dataspace);
    dataset.write(&digital, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DigitalSamples(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset = group->createDataSet("Digital Samples",
                                           PredType::NATIVE_INT, dataspace);
    dataset.write(&digitalSamples, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DbitOffset(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset =
        group->createDataSet("Dbit Offset", PredType::NATIVE_INT, dataspace);
    dataset.write(&dbitoffset, PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DbitList(H5File *fd, Group *group) {
    DataSpace dataspace = DataSpace(H5S_SCALAR);
    DataSet dataset = group->createDataSet("Dbit Bitset List",
                                           PredType::STD_U64LE, dataspace);
    dataset.write(&dbitlist, PredType::STD_U64LE);
}
#endif

void MasterAttributes::GetGotthardBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(sls::ToString(exptime).c_str());
    w->Key("Period");
    w->String(sls::ToString(period).c_str());
    w->Key("Detector Roi");
    w->StartObject();
    w->Key("xmin");
    w->Uint(detectorRoi.xmin);
    w->Key("xmax");
    w->Uint(detectorRoi.xmax);
    w->EndObject();
};

#ifdef HDF5C
void MasterAttributes::WriteGotthardHDF5Attributes(H5File *fd, Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5ROI(fd, group);
}
#endif

void MasterAttributes::GetJungfrauBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(sls::ToString(exptime).c_str());
    w->Key("Period");
    w->String(sls::ToString(period).c_str());
    w->Key("Number of UDP Interfaces");
    w->Uint(numUDPInterfaces);
    w->Key("Number of rows");
    w->Uint(readNRows);
}

#ifdef HDF5C
void MasterAttributes::WriteJungfrauHDF5Attributes(H5File *fd, Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5NumUDPInterfaces(fd, group);
    MasterAttributes::WriteHDF5ReadNRows(fd, group);
}
#endif

void MasterAttributes::GetEigerBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Dynamic Range");
    w->Uint(dynamicRange);
    w->Key("Ten Giga");
    w->Uint(tenGiga);
    w->Key("Exptime");
    w->String(sls::ToString(exptime).c_str());
    w->Key("Period");
    w->String(sls::ToString(period).c_str());
    w->Key("Threshold Energy");
    w->Int(thresholdEnergyeV);
    w->Key("Sub Exptime");
    w->String(sls::ToString(subExptime).c_str());
    w->Key("Sub Period");
    w->String(sls::ToString(subPeriod).c_str());
    w->Key("Quad");
    w->Int(quad);
    w->Key("Number of rows");
    w->Int(readNRows);
    w->Key("Rate Corrections");
    w->String(sls::ToString(ratecorr).c_str());
}

#ifdef HDF5C
void MasterAttributes::WriteEigerHDF5Attributes(H5File *fd, Group *group) {
    MasterAttributes::WriteHDF5DynamicRange(fd, group);
    MasterAttributes::WriteHDF5TenGiga(fd, group);
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5ThresholdEnergy(fd, group);
    MasterAttributes::WriteHDF5SubExpTime(fd, group);
    MasterAttributes::WriteHDF5SubPeriod(fd, group);
    MasterAttributes::WriteHDF5SubQuad(fd, group);
    MasterAttributes::WriteHDF5ReadNRows(fd, group);
    MasterAttributes::WriteHDF5RateCorrections(fd, group);
}
#endif

void MasterAttributes::GetMythen3BinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Dynamic Range");
    w->Uint(dynamicRange);
    w->Key("Ten Giga");
    w->Uint(tenGiga);
    w->Key("Period");
    w->String(sls::ToString(period).c_str());
    w->Key("Counter Mask");
    w->String(sls::ToStringHex(counterMask).c_str());
    for (int i = 0; i != 3; ++i) {
        w->Key((std::string("Exptime") + std::to_string(i + 1)).c_str());
        w->String(sls::ToString(exptimeArray[i]).c_str());
    }
    for (int i = 0; i != 3; ++i) {
        w->Key((std::string("GateDelay") + std::to_string(i + 1)).c_str());
        w->String(sls::ToString(gateDelayArray[i]).c_str());
    }
    w->Key("Gates");
    w->Uint(gates);
    w->Key("Threshold Energies");
    w->String(sls::ToString(thresholdAllEnergyeV).c_str());
}

#ifdef HDF5C
void MasterAttributes::WriteMythen3HDF5Attributes(H5File *fd, Group *group) {
    MasterAttributes::WriteHDF5DynamicRange(fd, group);
    MasterAttributes::WriteHDF5TenGiga(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5CounterMask(fd, group);
    MasterAttributes::WriteHDF5ExptimeArray(fd, group);
    MasterAttributes::WriteHDF5GateDelayArray(fd, group);
    MasterAttributes::WriteHDF5Gates(fd, group);
    MasterAttributes::WriteHDF5ThresholdEnergies(fd, group);
}
#endif

void MasterAttributes::GetGotthard2BinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(sls::ToString(exptime).c_str());
    w->Key("Period");
    w->String(sls::ToString(period).c_str());
    w->Key("Burst Mode");
    w->String(sls::ToString(burstMode).c_str());
}

#ifdef HDF5C
void MasterAttributes::WriteGotthard2HDF5Attributes(H5File *fd, Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5BurstMode(fd, group);
}
#endif

void MasterAttributes::GetMoenchBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(sls::ToString(exptime).c_str());
    w->Key("Period");
    w->String(sls::ToString(period).c_str());
    w->Key("Ten Giga");
    w->Uint(tenGiga);
    w->Key("ADC Mask");
    w->String(sls::ToStringHex(adcmask).c_str());
    w->Key("Analog Samples");
    w->Uint(analogSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteMoenchHDF5Attributes(H5File *fd, Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5TenGiga(fd, group);
    MasterAttributes::WriteHDF5AdcMask(fd, group);
    MasterAttributes::WriteHDF5AnalogSamples(fd, group);
}
#endif

void MasterAttributes::GetCtbBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(sls::ToString(exptime).c_str());
    w->Key("Period");
    w->String(sls::ToString(period).c_str());
    w->Key("Ten Giga");
    w->Uint(tenGiga);
    w->Key("ADC Mask");
    w->String(sls::ToStringHex(adcmask).c_str());
    w->Key("Analog Flag");
    w->Uint(analog);
    w->Key("Analog Samples");
    w->Uint(analogSamples);
    w->Key("Digital Flag");
    w->Uint(digital);
    w->Key("Digital Samples");
    w->Uint(digitalSamples);
    w->Key("Dbit Offset");
    w->Uint(dbitoffset);
    w->Key("Dbit Bitset");
    w->Uint64(dbitlist);
}

#ifdef HDF5C
void MasterAttributes::WriteCtbHDF5Attributes(H5File *fd, Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5TenGiga(fd, group);
    MasterAttributes::WriteHDF5AdcMask(fd, group);
    MasterAttributes::WriteHDF5AnalogFlag(fd, group);
    MasterAttributes::WriteHDF5AnalogSamples(fd, group);
    MasterAttributes::WriteHDF5DigitalFlag(fd, group);
    MasterAttributes::WriteHDF5DigitalSamples(fd, group);
    MasterAttributes::WriteHDF5DbitOffset(fd, group);
    MasterAttributes::WriteHDF5DbitList(fd, group);
}
#endif
