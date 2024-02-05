// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterAttributes.h"
#include <time.h>

namespace sls {

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
    case slsDetectorDefs::MOENCH:
        GetMoenchBinaryAttributes(w);
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
    case slsDetectorDefs::CHIPTESTBOARD:
        GetCtbBinaryAttributes(w);
        break;
    case slsDetectorDefs::XILINX_CHIPTESTBOARD:
        GetXilinxCtbBinaryAttributes(w);
        break;
    default:
        throw RuntimeError("Unknown Detector type to get master attributes");
    }
    GetFinalBinaryAttributes(w);
    w->EndObject();
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5Attributes(H5::H5File *fd, H5::Group *group) {
    WriteCommonHDF5Attributes(fd, group);
    switch (detType) {
    case slsDetectorDefs::GOTTHARD:
        WriteGotthardHDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::JUNGFRAU:
        WriteJungfrauHDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::MOENCH:
        WriteMoenchHDF5Attributes(fd, group);
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
    case slsDetectorDefs::CHIPTESTBOARD:
        WriteCtbHDF5Attributes(fd, group);
        break;
    case slsDetectorDefs::XILINX_CHIPTESTBOARD:
        WriteXilinxCtbHDF5Attributes(fd, group);
        break;
    default:
        throw RuntimeError("Unknown Detector type to get master attributes");
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
    time_t t = std::time(nullptr);
    std::string sTime(ctime(&t));
    std::replace(sTime.begin(), sTime.end(), '\n', '\0');
    w->String(sTime.c_str());
    w->Key("Detector Type");
    w->String(ToString(detType).c_str());
    w->Key("Timing Mode");
    w->String(ToString(timingMode).c_str());
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
    w->String(ToString(frameDiscardMode).c_str());
    w->Key("Frame Padding");
    w->Uint(framePadding);
    w->Key("Scan Parameters");
    w->String(ToString(scanParams).c_str());
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
        w->String(ToString(additionalJsonHeader).c_str());
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
void MasterAttributes::WriteCommonHDF5Attributes(H5::H5File *fd,
                                                 H5::Group *group) {
    char c[1024]{};
    // version
    {
        double version = BINARY_WRITER_VERSION;
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::Attribute attribute = fd->createAttribute(
            "Version", H5::PredType::NATIVE_DOUBLE, dataspace);
        attribute.write(H5::PredType::NATIVE_DOUBLE, &version);
    }
    // timestamp
    {
        time_t t = std::time(nullptr);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset =
            group->createDataSet("Timestamp", strdatatype, dataspace);
        strcpy_safe(c, std::string(ctime(&t)));
        dataset.write(c, strdatatype);
    }
    // detector type
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::DataSet dataset =
            group->createDataSet("Detector Type", strdatatype, dataspace);
        strcpy_safe(c, ToString(detType));
        dataset.write(c, strdatatype);
    }
    // timing mode
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::DataSet dataset =
            group->createDataSet("Timing Mode", strdatatype, dataspace);
        strcpy_safe(c, ToString(timingMode));
        dataset.write(c, strdatatype);
    }
    // TODO: make this into an array?
    // geometry x
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Geometry in x axis", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&geometry.x, H5::PredType::NATIVE_INT);
    }
    // geometry y
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Geometry in y axis", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&geometry.y, H5::PredType::NATIVE_INT);
    }
    // Image Size
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Image Size", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&imageSize, H5::PredType::NATIVE_INT);
        H5::DataSpace dataspaceAttr = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::Attribute attribute =
            dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
        strcpy_safe(c, "bytes");
        attribute.write(strdatatype, c);
    }
    // TODO: make this into an array?
    // npixels x
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Number of pixels in x axis", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&nPixels.x, H5::PredType::NATIVE_INT);
    }
    // npixels y
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Number of pixels in y axis", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&nPixels.y, H5::PredType::NATIVE_INT);
    }
    // Maximum frames per file
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Maximum frames per file", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&maxFramesPerFile, H5::PredType::NATIVE_INT);
    }
    // Frame Discard Policy
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::DataSet dataset = group->createDataSet("Frame Discard Policy",
                                                   strdatatype, dataspace);
        strcpy_safe(c, ToString(frameDiscardMode));
        dataset.write(c, strdatatype);
    }
    // Frame Padding
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Frame Padding", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&framePadding, H5::PredType::NATIVE_INT);
    }
    // Scan Parameters
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::DataSet dataset =
            group->createDataSet("Scan Parameters", strdatatype, dataspace);
        strcpy_safe(c, ToString(scanParams));
        dataset.write(c, strdatatype);
    }
    // Total Frames
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Total Frames", H5::PredType::STD_U64LE, dataspace);
        dataset.write(&totalFrames, H5::PredType::STD_U64LE);
    }
    // Receiver Roi xmin
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "receiver roi xmin", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.xmin, H5::PredType::NATIVE_INT);
    }
    // Receiver Roi xmax
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "receiver roi xmax", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.xmax, H5::PredType::NATIVE_INT);
    }
    // Receiver Roi ymin
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "receiver roi ymin", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.ymin, H5::PredType::NATIVE_INT);
    }
    // Receiver Roi ymax
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "receiver roi ymax", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&receiverRoi.ymax, H5::PredType::NATIVE_INT);
    }
}

void MasterAttributes::WriteFinalHDF5Attributes(H5::H5File *fd,
                                                H5::Group *group) {
    char c[1024]{};
    // Total Frames in file
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "Frames in File", H5::PredType::STD_U64LE, dataspace);
        dataset.write(&framesInFile, H5::PredType::STD_U64LE);
    }
    // additional json header
    if (!additionalJsonHeader.empty()) {
        std::string json = ToString(additionalJsonHeader);
        H5::StrType strdatatype(H5::PredType::C_S1, json.length());
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet("Additional JSON Header",
                                                   strdatatype, dataspace);
        strcpy_safe(c, ToString(additionalJsonHeader));
        dataset.write(c, strdatatype);
    }
}

void MasterAttributes::WriteHDF5Exptime(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);
    H5::DataSet dataset =
        group->createDataSet("Exposure Time", strdatatype, dataspace);
    char c[1024]{};
    strcpy_safe(c, ToString(exptime));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5Period(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);
    H5::DataSet dataset =
        group->createDataSet("Acquisition Period", strdatatype, dataspace);
    char c[1024]{};
    strcpy_safe(c, ToString(period));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5DynamicRange(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Dynamic Range", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&dynamicRange, H5::PredType::NATIVE_INT);
    H5::DataSpace dataspaceAttr = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);
    H5::Attribute attribute =
        dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
    char c[1024] = "bits";
    attribute.write(strdatatype, c);
}

void MasterAttributes::WriteHDF5TenGiga(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Ten Giga Enable", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&tenGiga, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5ROI(H5::H5File *fd, H5::Group *group) {
    // Roi xmin
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "roi xmin", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&detectorRoi.xmin, H5::PredType::NATIVE_INT);
    }
    // Roi xmax
    {
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::DataSet dataset = group->createDataSet(
            "roi xmax", H5::PredType::NATIVE_INT, dataspace);
        dataset.write(&detectorRoi.xmax, H5::PredType::NATIVE_INT);
    }
}

void MasterAttributes::WriteHDF5NumUDPInterfaces(H5::H5File *fd,
                                                 H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Number of UDP Interfaces", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&numUDPInterfaces, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5ReadNRows(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Number of rows", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&readNRows, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5ThresholdEnergy(H5::H5File *fd,
                                                H5::Group *group) {
    char c[1024]{};
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Threshold Energy", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&thresholdEnergyeV, H5::PredType::NATIVE_INT);
    H5::DataSpace dataspaceAttr = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);
    H5::Attribute attribute =
        dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
    strcpy_safe(c, "eV");
    attribute.write(strdatatype, c);
}

void MasterAttributes::WriteHDF5ThresholdEnergies(H5::H5File *fd,
                                                  H5::Group *group) {
    char c[1024]{};
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 1024);
    H5::DataSet dataset =
        group->createDataSet("Threshold Energies", strdatatype, dataspace);
    strcpy_safe(c, ToString(thresholdAllEnergyeV));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5SubExpTime(H5::H5File *fd, H5::Group *group) {
    char c[1024]{};
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);
    H5::DataSet dataset =
        group->createDataSet("Sub Exposure Time", strdatatype, dataspace);
    strcpy_safe(c, ToString(subExptime));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5SubPeriod(H5::H5File *fd, H5::Group *group) {
    char c[1024]{};
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);
    H5::DataSet dataset =
        group->createDataSet("Sub Period", strdatatype, dataspace);
    strcpy_safe(c, ToString(subPeriod));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5SubQuad(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset =
        group->createDataSet("Quad", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&quad, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5RateCorrections(H5::H5File *fd,
                                                H5::Group *group) {
    char c[1024]{};
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 1024);
    H5::DataSet dataset =
        group->createDataSet("Rate Corrections", strdatatype, dataspace);
    strcpy_safe(c, ToString(ratecorr));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5CounterMask(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Counter Mask", H5::PredType::STD_U32LE, dataspace);
    dataset.write(&counterMask, H5::PredType::STD_U32LE);
}

void MasterAttributes::WriteHDF5ExptimeArray(H5::H5File *fd, H5::Group *group) {
    for (int i = 0; i != 3; ++i) {
        char c[1024]{};
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::DataSet dataset =
            group->createDataSet("Exposure Time1", strdatatype, dataspace);
        strcpy_safe(c, ToString(exptimeArray[i]));
        dataset.write(c, strdatatype);
    }
}

void MasterAttributes::WriteHDF5GateDelayArray(H5::H5File *fd,
                                               H5::Group *group) {
    for (int i = 0; i != 3; ++i) {
        char c[1024]{};
        H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256);
        H5::DataSet dataset =
            group->createDataSet("Gate Delay1", strdatatype, dataspace);
        strcpy_safe(c, ToString(gateDelayArray[i]));
        dataset.write(c, strdatatype);
    }
}

void MasterAttributes::WriteHDF5Gates(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset =
        group->createDataSet("Gates", H5::PredType::STD_U32LE, dataspace);
    dataset.write(&gates, H5::PredType::STD_U32LE);
}

void MasterAttributes::WriteHDF5BurstMode(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);
    H5::DataSet dataset =
        group->createDataSet("Burst Mode", strdatatype, dataspace);
    char c[1024]{};
    strcpy_safe(c, ToString(burstMode));
    dataset.write(c, strdatatype);
}

void MasterAttributes::WriteHDF5AdcMask(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset =
        group->createDataSet("ADC Mask", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&adcmask, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5AnalogFlag(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Analog Flag", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&analog, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5AnalogSamples(H5::H5File *fd,
                                              H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Analog Samples", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&analogSamples, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DigitalFlag(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Digital Flag", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&digital, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DigitalSamples(H5::H5File *fd,
                                               H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Digital Samples", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&digitalSamples, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DbitOffset(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Dbit Offset", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&dbitoffset, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5DbitList(H5::H5File *fd, H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Dbit Bitset List", H5::PredType::STD_U64LE, dataspace);
    dataset.write(&dbitlist, H5::PredType::STD_U64LE);
}

void MasterAttributes::WriteHDF5TransceiverMask(H5::H5File *fd,
                                                H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Transceiver Mask", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&transceiverMask, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5TransceiverFlag(H5::H5File *fd,
                                                H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Transceiver Flag", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&transceiver, H5::PredType::NATIVE_INT);
}

void MasterAttributes::WriteHDF5TransceiverSamples(H5::H5File *fd,
                                                   H5::Group *group) {
    H5::DataSpace dataspace = H5::DataSpace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(
        "Transceiver Samples", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(&transceiverSamples, H5::PredType::NATIVE_INT);
}
#endif

void MasterAttributes::GetGotthardBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(ToString(exptime).c_str());
    w->Key("Period");
    w->String(ToString(period).c_str());
    w->Key("Detector Roi");
    w->StartObject();
    w->Key("xmin");
    w->Uint(detectorRoi.xmin);
    w->Key("xmax");
    w->Uint(detectorRoi.xmax);
    w->EndObject();
};

#ifdef HDF5C
void MasterAttributes::WriteGotthardHDF5Attributes(H5::H5File *fd,
                                                   H5::Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5ROI(fd, group);
}
#endif

void MasterAttributes::GetJungfrauBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(ToString(exptime).c_str());
    w->Key("Period");
    w->String(ToString(period).c_str());
    w->Key("Number of UDP Interfaces");
    w->Uint(numUDPInterfaces);
    w->Key("Number of rows");
    w->Uint(readNRows);
}

#ifdef HDF5C
void MasterAttributes::WriteJungfrauHDF5Attributes(H5::H5File *fd,
                                                   H5::Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5NumUDPInterfaces(fd, group);
    MasterAttributes::WriteHDF5ReadNRows(fd, group);
}
#endif

void MasterAttributes::GetMoenchBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(ToString(exptime).c_str());
    w->Key("Period");
    w->String(ToString(period).c_str());
    w->Key("Number of UDP Interfaces");
    w->Uint(numUDPInterfaces);
    w->Key("Number of rows");
    w->Uint(readNRows);
}

#ifdef HDF5C
void MasterAttributes::WriteMoenchHDF5Attributes(H5::H5File *fd,
                                                 H5::Group *group) {
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
    w->String(ToString(exptime).c_str());
    w->Key("Period");
    w->String(ToString(period).c_str());
    w->Key("Threshold Energy");
    w->Int(thresholdEnergyeV);
    w->Key("Sub Exptime");
    w->String(ToString(subExptime).c_str());
    w->Key("Sub Period");
    w->String(ToString(subPeriod).c_str());
    w->Key("Quad");
    w->Int(quad);
    w->Key("Number of rows");
    w->Int(readNRows);
    w->Key("Rate Corrections");
    w->String(ToString(ratecorr).c_str());
}

#ifdef HDF5C
void MasterAttributes::WriteEigerHDF5Attributes(H5::H5File *fd,
                                                H5::Group *group) {
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
    w->String(ToString(period).c_str());
    w->Key("Counter Mask");
    w->String(ToStringHex(counterMask).c_str());
    for (int i = 0; i != 3; ++i) {
        w->Key((std::string("Exptime") + std::to_string(i + 1)).c_str());
        w->String(ToString(exptimeArray[i]).c_str());
    }
    for (int i = 0; i != 3; ++i) {
        w->Key((std::string("GateDelay") + std::to_string(i + 1)).c_str());
        w->String(ToString(gateDelayArray[i]).c_str());
    }
    w->Key("Gates");
    w->Uint(gates);
    w->Key("Threshold Energies");
    w->String(ToString(thresholdAllEnergyeV).c_str());
}

#ifdef HDF5C
void MasterAttributes::WriteMythen3HDF5Attributes(H5::H5File *fd,
                                                  H5::Group *group) {
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
    w->String(ToString(exptime).c_str());
    w->Key("Period");
    w->String(ToString(period).c_str());
    w->Key("Burst Mode");
    w->String(ToString(burstMode).c_str());
}

#ifdef HDF5C
void MasterAttributes::WriteGotthard2HDF5Attributes(H5::H5File *fd,
                                                    H5::Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5BurstMode(fd, group);
}
#endif

void MasterAttributes::GetCtbBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(ToString(exptime).c_str());
    w->Key("Period");
    w->String(ToString(period).c_str());
    w->Key("Ten Giga");
    w->Uint(tenGiga);
    w->Key("ADC Mask");
    w->String(ToStringHex(adcmask).c_str());
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
    w->Key("Transceiver Mask");
    w->String(ToStringHex(transceiverMask).c_str());
    w->Key("Transceiver Flag");
    w->Uint(transceiver);
    w->Key("Transceiver Samples");
    w->Uint(transceiverSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteCtbHDF5Attributes(H5::H5File *fd,
                                              H5::Group *group) {
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
    MasterAttributes::WriteHDF5TransceiverMask(fd, group);
    MasterAttributes::WriteHDF5TransceiverFlag(fd, group);
    MasterAttributes::WriteHDF5TransceiverSamples(fd, group);
}
#endif

void MasterAttributes::GetXilinxCtbBinaryAttributes(
    rapidjson::PrettyWriter<rapidjson::StringBuffer> *w) {
    w->Key("Exptime");
    w->String(ToString(exptime).c_str());
    w->Key("Period");
    w->String(ToString(period).c_str());
    w->Key("ADC Mask");
    w->String(ToStringHex(adcmask).c_str());
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
    w->Key("Transceiver Mask");
    w->String(ToStringHex(transceiverMask).c_str());
    w->Key("Transceiver Flag");
    w->Uint(transceiver);
    w->Key("Transceiver Samples");
    w->Uint(transceiverSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteXilinxCtbHDF5Attributes(H5::H5File *fd,
                                                    H5::Group *group) {
    MasterAttributes::WriteHDF5Exptime(fd, group);
    MasterAttributes::WriteHDF5Period(fd, group);
    MasterAttributes::WriteHDF5AdcMask(fd, group);
    MasterAttributes::WriteHDF5AnalogFlag(fd, group);
    MasterAttributes::WriteHDF5AnalogSamples(fd, group);
    MasterAttributes::WriteHDF5DigitalFlag(fd, group);
    MasterAttributes::WriteHDF5DigitalSamples(fd, group);
    MasterAttributes::WriteHDF5DbitOffset(fd, group);
    MasterAttributes::WriteHDF5DbitList(fd, group);
    MasterAttributes::WriteHDF5TransceiverMask(fd, group);
    MasterAttributes::WriteHDF5TransceiverFlag(fd, group);
    MasterAttributes::WriteHDF5TransceiverSamples(fd, group);
}
#endif
} // namespace sls
