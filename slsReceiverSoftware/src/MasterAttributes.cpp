// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterAttributes.h"
#include <time.h>

namespace sls {

void MasterAttributes::GetBinaryAttributes(writer *w) {
    w->StartObject();
    GetCommonBinaryAttributes(w);
    switch (detType) {
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
    case slsDetectorDefs::JUNGFRAU:
        WriteJungfrauHDF5Attributes(group);
        break;
    case slsDetectorDefs::MOENCH:
        WriteMoenchHDF5Attributes(group);
        break;
    case slsDetectorDefs::EIGER:
        WriteEigerHDF5Attributes(group);
        break;
    case slsDetectorDefs::MYTHEN3:
        WriteMythen3HDF5Attributes(group);
        break;
    case slsDetectorDefs::GOTTHARD2:
        WriteGotthard2HDF5Attributes(group);
        break;
    case slsDetectorDefs::CHIPTESTBOARD:
        WriteCtbHDF5Attributes(group);
        break;
    case slsDetectorDefs::XILINX_CHIPTESTBOARD:
        WriteXilinxCtbHDF5Attributes(group);
        break;
    default:
        throw RuntimeError("Unknown Detector type to get master attributes");
    }
    WriteFinalHDF5Attributes(group);
}
#endif

void MasterAttributes::WriteBinaryRois(writer *w) {
    w->Key("Receiver Rois");
    w->StartArray();
    for (const slsDetectorDefs::ROI &roi : rois) {
        auto roiArray = roi.getIntArray();
        w->StartObject();
        w->Key("xmin");
        w->Int(roiArray[0]);
        w->Key("xmax");
        w->Int(roiArray[1]);
        w->Key("ymin");
        w->Int(roiArray[2]);
        w->Key("ymax");
        w->Int(roiArray[3]);
        w->EndObject();
    }
    w->EndArray();
}

void MasterAttributes::WriteBinaryXY(writer *w, const std::string &name,
                                     const defs::xy &xy) {
    w->Key(name.c_str());
    w->StartObject();
    w->Key("x");
    w->Uint(xy.x);
    w->Key("y");
    w->Uint(xy.y);
    w->EndObject();
}

void MasterAttributes::WriteBinaryScanParameters(writer *w) {
    w->Key("Scan Parameters");
    w->StartObject();
    w->Key("enable");
    w->Int(scanParams.enable);
    w->Key("dacInd");
    w->Int(scanParams.dacInd);
    w->Key("start offset");
    w->Int(scanParams.startOffset);
    w->Key("stop offset");
    w->Int(scanParams.stopOffset);
    w->Key("step size");
    w->Int(scanParams.stepSize);
    w->Key("dac settle time ns");
    w->Int64(scanParams.dacSettleTime_ns);
    w->EndObject();
}

void MasterAttributes::WriteBinaryJsonHeader(writer *w) {
    w->Key("Additional Json Header");
    w->StartObject();
    for (const auto &pair : additionalJsonHeader) {
        w->Key(pair.first.c_str());
        w->String(pair.second.c_str());
    }
    w->EndObject();
}

void MasterAttributes::WriteBinaryFrameHeaderFormat(writer *w) {
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
void MasterAttributes::WriteHDF5String(H5::Group *group,
                                       const std::string &name,
                                       const std::string &value) {
    H5::DataSpace dataspace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::DataSet dataset = group->createDataSet(name, strdatatype, dataspace);
    const char *cstr = value.c_str();
    dataset.write(&cstr, strdatatype);
}

void MasterAttributes::WriteHDF5StringArray(
    H5::Group *group, const std::string &name,
    const std::vector<std::string> &value) {
    std::vector<const char *> c;
    for (auto &s : value) {
        c.push_back(s.c_str());
    }
    hsize_t dims[1] = {c.size()};
    H5::DataSpace dataspace(1, dims);
    H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::DataSet dataset = group->createDataSet(name, strdatatype, dataspace);
    dataset.write(c.data(), strdatatype);
}

void MasterAttributes::WriteHDF5XY(H5::Group *group, const std::string &name,
                                   const defs::xy &xy) {
    H5::CompType c(sizeof(defs::xy));
    c.insertMember("x", HOFFSET(defs::xy, x), H5::PredType::NATIVE_INT);
    c.insertMember("y", HOFFSET(defs::xy, y), H5::PredType::NATIVE_INT);
    H5::DataSpace dataspace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(name, c, dataspace);
    dataset.write(&xy, c);
}

void MasterAttributes::WriteHDF5ScanParameters(H5::Group *group) {
    H5::CompType c(sizeof(defs::scanParameters));
    c.insertMember("enable", HOFFSET(defs::scanParameters, enable),
                   H5::PredType::NATIVE_INT);
    c.insertMember("dacInd", HOFFSET(defs::scanParameters, dacInd),
                   H5::PredType::NATIVE_INT);
    c.insertMember("startOffset", HOFFSET(defs::scanParameters, startOffset),
                   H5::PredType::NATIVE_INT);
    c.insertMember("stopOffset", HOFFSET(defs::scanParameters, stopOffset),
                   H5::PredType::NATIVE_INT);
    c.insertMember("stepSize", HOFFSET(defs::scanParameters, stepSize),
                   H5::PredType::NATIVE_INT);
    c.insertMember("dacSettleTime_ns",
                   HOFFSET(defs::scanParameters, dacSettleTime_ns),
                   H5::PredType::STD_I64LE);
    H5::DataSpace dataspace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet("Scan Parameters", c, dataspace);
    dataset.write(&scanParams, c);
}

void MasterAttributes::WriteHDF5JsonHeader(H5::Group *group) {
    H5::StrType strType(H5::PredType::C_S1, H5T_VARIABLE);
    H5::CompType mapType(sizeof(char *) * 2);
    mapType.insertMember("Key", 0, strType);
    mapType.insertMember("Value", sizeof(char *), strType);
    // create string struct just so its not dangling pointer
    // with push_back
    struct KeyValue {
        std::string key;
        std::string value;
    };
    struct KVRaw {
        const char *key;
        const char *value;
    };
    std::vector<KVRaw> raw;
    std::vector<KeyValue> value;
    value.reserve(additionalJsonHeader.size());
    raw.reserve(additionalJsonHeader.size());
    for (const auto &pair : additionalJsonHeader) {
        value.push_back({pair.first, pair.second});
    }
    for (const auto &item : value) {
        raw.push_back({item.key.c_str(), item.value.c_str()});
    }
    hsize_t dims[1] = {value.size()};
    H5::DataSpace dataspace(1, dims);
    H5::DataSet dataset =
        group->createDataSet("Additional Json Header", mapType, dataspace);
    dataset.write(raw.data(), mapType);
}

void MasterAttributes::WriteHDF5ROIs(H5::Group *group) {
    H5::CompType c(sizeof(defs::ROI));
    c.insertMember("xmin", HOFFSET(defs::ROI, xmin), H5::PredType::NATIVE_INT);
    c.insertMember("xmax", HOFFSET(defs::ROI, xmax), H5::PredType::NATIVE_INT);
    c.insertMember("ymin", HOFFSET(defs::ROI, ymin), H5::PredType::NATIVE_INT);
    c.insertMember("ymax", HOFFSET(defs::ROI, ymax), H5::PredType::NATIVE_INT);
    hsize_t dims[1] = {rois.size()}; // 1d dataspace with size of roi elements
    H5::DataSpace dataspace(1, dims);
    H5::DataSet dataset = group->createDataSet("Receiver Rois", c, dataspace);
    dataset.write(rois.data(), c);
}

void MasterAttributes::WriteHDF5ExptimeArray(H5::Group *group) {
    std::vector<std::string> timeStrings;
    for (auto &e : exptimeArray) {
        timeStrings.push_back(ToString(e));
    }
    WriteHDF5StringArray(group, "Exposure Times", timeStrings);
}

void MasterAttributes::WriteHDF5GateDelayArray(H5::Group *group) {
    std::vector<std::string> timeStrings;
    for (auto &g : gateDelayArray) {
        timeStrings.push_back(ToString(g));
    }
    WriteHDF5StringArray(group, "Gate Delays", timeStrings);
}
#endif

void MasterAttributes::GetCommonBinaryAttributes(writer *w) {
    w->Key("Version");
    w->SetMaxDecimalPlaces(2);
    w->Double(BINARY_WRITER_VERSION);
    {
        time_t t = std::time(nullptr);
        std::string sTime(ctime(&t));
        std::replace(sTime.begin(), sTime.end(), '\n', '\0');
        WriteBinary(w, "Timestamp", sTime);
    }
    WriteBinary(w, "Detector Type", ToString(detType));
    WriteBinary(w, "Timing Mode", ToString(timingMode));
    WriteBinaryXY(w, "Geometry", geometry);
    WriteBinary(w, "Image Size", imageSize);
    WriteBinaryXY(w, "Pixels", nPixels);
    WriteBinary(w, "Max Frames Per File", maxFramesPerFile);
    WriteBinary(w, "Frame Discard Policy", ToString(frameDiscardMode));
    WriteBinary(w, "Frame Padding", framePadding);
    WriteBinaryScanParameters(w);
    WriteBinary(w, "Total Frames", totalFrames);
}

void MasterAttributes::GetFinalBinaryAttributes(writer *w) {
    WriteBinary(w, "Frames in File", framesInFile);
    WriteBinaryJsonHeader(w);
}

#ifdef HDF5C
void MasterAttributes::WriteCommonHDF5Attributes(H5::H5File *fd,
                                                 H5::Group *group) {
    time_t t = std::time(nullptr);
    std::string sTime(ctime(&t));
    WriteHDF5String(group, "Timestamp", sTime);
    WriteHDF5String(group, "Detector Type", ToString(detType));
    WriteHDF5String(group, "Timing Mode", ToString(timingMode));
    WriteHDF5XY(group, "Geometry", geometry);
    WriteHDF5Int(group, "Image Size", imageSize);
    WriteHDF5XY(group, "Pixels", nPixels);
    WriteHDF5Int(group, "Max Frames Per File", maxFramesPerFile);
    WriteHDF5String(group, "Frame Discard Policy", ToString(frameDiscardMode));
    WriteHDF5Int(group, "Frame Padding", framePadding);
    WriteHDF5ScanParameters(group);
    WriteHDF5Int(group, "Total Frames", totalFrames);
}

void MasterAttributes::WriteFinalHDF5Attributes(H5::Group *group) {
    WriteHDF5Int(group, "Frames in File", framesInFile);
    WriteHDF5JsonHeader(group);
}
#endif

void MasterAttributes::GetJungfrauBinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinary(w, "Exposure Time", ToString(exptime));
    WriteBinary(w, "Acquisition Period", ToString(period));
    WriteBinary(w, "Number of UDP Interfaces", numUDPInterfaces);
    WriteBinary(w, "Number of rows", readNRows);
    WriteBinary(w, "Readout Speed", ToString(readoutSpeed));
}

#ifdef HDF5C
void MasterAttributes::WriteJungfrauHDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5String(group, "Exposure Time", ToString(exptime));
    WriteHDF5String(group, "Acquisition Period", ToString(period));
    WriteHDF5Int(group, "Number of UDP Interfaces", numUDPInterfaces);
    WriteHDF5Int(group, "Number of rows", readNRows);
    WriteHDF5String(group, "Readout Speed", ToString(readoutSpeed));
}
#endif

void MasterAttributes::GetMoenchBinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinary(w, "Exposure Time", ToString(exptime));
    WriteBinary(w, "Acquisition Period", ToString(period));
    WriteBinary(w, "Number of UDP Interfaces", numUDPInterfaces);
    WriteBinary(w, "Number of rows", readNRows);
    WriteBinary(w, "Readout Speed", ToString(readoutSpeed));
}

#ifdef HDF5C
void MasterAttributes::WriteMoenchHDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5String(group, "Exposure Time", ToString(exptime));
    WriteHDF5String(group, "Acquisition Period", ToString(period));
    WriteHDF5Int(group, "Number of UDP Interfaces", numUDPInterfaces);
    WriteHDF5Int(group, "Number of rows", readNRows);
    WriteHDF5String(group, "Readout Speed", ToString(readoutSpeed));
}
#endif

void MasterAttributes::GetEigerBinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinary(w, "Dynamic Range", dynamicRange);
    WriteBinary(w, "Ten Giga", tenGiga);
    WriteBinary(w, "Exposure Time", ToString(exptime));
    WriteBinary(w, "Acquisition Period", ToString(period));
    WriteBinary(w, "Threshold Energy", thresholdEnergyeV);
    WriteBinary(w, "Sub Exposure Time", ToString(subExptime));
    WriteBinary(w, "Sub Period", ToString(subPeriod));
    WriteBinary(w, "Quad", quad);
    WriteBinary(w, "Number of rows", readNRows);
    WriteBinary(w, "Rate Corrections", ratecorr);
    WriteBinary(w, "Readout Speed", ToString(readoutSpeed));
}

#ifdef HDF5C
void MasterAttributes::WriteEigerHDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5Int(group, "Dynamic Range", dynamicRange);
    WriteHDF5Int(group, "Ten Giga", tenGiga);
    WriteHDF5String(group, "Exposure Time", ToString(exptime));
    WriteHDF5String(group, "Acquisition Period", ToString(period));
    WriteHDF5Int(group, "Threshold Energy", thresholdEnergyeV);
    WriteHDF5String(group, "Sub Exposure Time", ToString(subExptime));
    WriteHDF5String(group, "Sub Period", ToString(subPeriod));
    WriteHDF5Int(group, "Quad", quad);
    WriteHDF5Int(group, "Number of rows", readNRows);
    WriteHDF5Int(group, "Rate Corrections", ratecorr);
    WriteHDF5String(group, "Readout Speed", ToString(readoutSpeed));
}
#endif

void MasterAttributes::GetMythen3BinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinary(w, "Dynamic Range", dynamicRange);
    WriteBinary(w, "Ten Giga", tenGiga);
    WriteBinary(w, "Acquisition Period", ToString(period));
    WriteBinary(w, "Counter Mask", counterMask);
    WriteBinary(w, "Exposure Times", exptimeArray);
    WriteBinary(w, "Gate Delays", gateDelayArray);
    WriteBinary(w, "Gates", gates);
    WriteBinary(w, "Threshold Energies", thresholdAllEnergyeV);
    WriteBinary(w, "Readout Speed", ToString(readoutSpeed));
}

#ifdef HDF5C
void MasterAttributes::WriteMythen3HDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5Int(group, "Dynamic Range", dynamicRange);
    WriteHDF5Int(group, "Ten Giga", tenGiga);
    WriteHDF5String(group, "Acquisition Period", ToString(period));
    WriteHDF5Int(group, "Counter Mask", counterMask);
    WriteHDF5ExptimeArray(group);
    WriteHDF5GateDelayArray(group);
    WriteHDF5Int(group, "Gates", gates);
    WriteHDF5Int(group, "Threshold Energies", thresholdAllEnergyeV);
    WriteHDF5String(group, "Readout Speed", ToString(readoutSpeed));
}
#endif

void MasterAttributes::GetGotthard2BinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinary(w, "Exposure Time", ToString(exptime));
    WriteBinary(w, "Acquisition Period", ToString(period));
    WriteBinary(w, "Burst Mode", ToString(burstMode));
    WriteBinary(w, "Readout Speed", ToString(readoutSpeed));
}

#ifdef HDF5C
void MasterAttributes::WriteGotthard2HDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5String(group, "Exposure Time", ToString(exptime));
    WriteHDF5String(group, "Acquisition Period", ToString(period));
    WriteHDF5String(group, "Burst Mode", ToString(burstMode));
    WriteHDF5String(group, "Readout Speed", ToString(readoutSpeed));
}
#endif

void MasterAttributes::GetCtbBinaryAttributes(writer *w) {
    WriteBinary(w, "Exposure Time", ToString(exptime));
    WriteBinary(w, "Acquisition Period", ToString(period));
    WriteBinary(w, "Ten Giga", tenGiga);
    WriteBinary(w, "ADC Mask", adcmask);
    WriteBinary(w, "Analog Flag", analog);
    WriteBinary(w, "Analog Samples", analogSamples);
    WriteBinary(w, "Digital Flag", digital);
    WriteBinary(w, "Digital Samples", digitalSamples);
    WriteBinary(w, "Dbit Offset", dbitoffset);
    WriteBinary(w, "Dbit Reorder", dbitreorder);
    WriteBinary(w, "Dbit Bitset", dbitlist);
    WriteBinary(w, "Transceiver Mask", transceiverMask);
    WriteBinary(w, "Transceiver Flag", transceiver);
    WriteBinary(w, "Transceiver Samples", transceiverSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteCtbHDF5Attributes(H5::Group *group) {
    WriteHDF5String(group, "Exposure Time", ToString(exptime));
    WriteHDF5String(group, "Acquisition Period", ToString(period));
    WriteHDF5Int(group, "Ten Giga", tenGiga);
    WriteHDF5Int(group, "ADC Mask", adcmask);
    WriteHDF5Int(group, "Analog Flag", analog);
    WriteHDF5Int(group, "Analog Samples", analogSamples);
    WriteHDF5Int(group, "Digital Flag", digital);
    WriteHDF5Int(group, "Digital Samples", digitalSamples);
    WriteHDF5Int(group, "Dbit Offset", dbitoffset);
    WriteHDF5Int(group, "Dbit Reorder", dbitreorder);
    WriteHDF5Int(group, "Dbit Bitset", dbitlist);
    WriteHDF5Int(group, "Transceiver Mask", transceiverMask);
    WriteHDF5Int(group, "Transceiver Flag", transceiver);
    WriteHDF5Int(group, "Transceiver Samples", transceiverSamples);
}
#endif

void MasterAttributes::GetXilinxCtbBinaryAttributes(writer *w) {
    WriteBinary(w, "Exposure Time", ToString(exptime));
    WriteBinary(w, "Acquisition Period", ToString(period));
    WriteBinary(w, "ADC Mask", adcmask);
    WriteBinary(w, "Analog Flag", analog);
    WriteBinary(w, "Analog Samples", analogSamples);
    WriteBinary(w, "Digital Flag", digital);
    WriteBinary(w, "Digital Samples", digitalSamples);
    WriteBinary(w, "Dbit Offset", dbitoffset);
    WriteBinary(w, "Dbit Reorder", dbitreorder);
    WriteBinary(w, "Dbit Bitset", dbitlist);
    WriteBinary(w, "Transceiver Mask", transceiverMask);
    WriteBinary(w, "Transceiver Flag", transceiver);
    WriteBinary(w, "Transceiver Samples", transceiverSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteXilinxCtbHDF5Attributes(H5::Group *group) {
    WriteHDF5String(group, "Exposure Time", ToString(exptime));
    WriteHDF5String(group, "Acquisition Period", ToString(period));
    WriteHDF5Int(group, "ADC Mask", adcmask);
    WriteHDF5Int(group, "Analog Flag", analog);
    WriteHDF5Int(group, "Analog Samples", analogSamples);
    WriteHDF5Int(group, "Digital Flag", digital);
    WriteHDF5Int(group, "Digital Samples", digitalSamples);
    WriteHDF5Int(group, "Dbit Offset", dbitoffset);
    WriteHDF5Int(group, "Dbit Reorder", dbitreorder);
    WriteHDF5Int(group, "Dbit Bitset", dbitlist);
    WriteHDF5Int(group, "Transceiver Mask", transceiverMask);
    WriteHDF5Int(group, "Transceiver Flag", transceiver);
    WriteHDF5Int(group, "Transceiver Samples", transceiverSamples);
}
#endif
} // namespace sls
