// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterAttributes.h"
#include <time.h>

namespace sls {

void MasterAttributes::GetCommonBinaryAttributes(writer *w) {
    WriteBinaryVersion(w);
    WriteBinaryTimestamp(w);
    WriteBinaryDetectorType(w);
    WriteBinaryTimingMode(w);
    WriteBinaryGeometry(w);
    WriteBinaryImageSize(w);
    WriteBinaryPixels(w);
    WriteBinaryMaxFramesPerFile(w);
    WriteBinaryFrameDiscardPolicy(w);
    WriteBinaryFramePadding(w);
    WriteBinaryScanParameters(w);
    WriteBinaryTotalFrames(w);
}

void MasterAttributes::GetFinalBinaryAttributes(writer *w) {
    WriteBinaryFramesInFile(w);
    WriteBinaryJsonHeader(w);
}

#ifdef HDF5C
void MasterAttributes::WriteCommonHDF5Attributes(H5::H5File *fd,
                                                 H5::Group *group) {
    WriteHDF5Version(fd);
    WriteHDF5Timestamp(group);
    WriteHDF5DetectorType(group);
    WriteHDF5TimingMode(group);
    WriteHDF5Geometry(group);
    WriteHDF5ImageSize(group);
    WriteHDF5Pixels(group);
    WriteHDF5MaxFramesPerFile(group);
    WriteHDF5FrameDiscardPolicy(group);
    WriteHDF5FramePadding(group);
    WriteHDF5ScanParameters(group);
    WriteHDF5TotalFrames(group);
}

void MasterAttributes::WriteFinalHDF5Attributes(H5::Group *group) {
    WriteHDF5FramesInFile(group);
    WriteHDF5JsonHeader(group);
}
#endif

void MasterAttributes::GetJungfrauBinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinaryExposureTme(w);
    WriteBinaryAcquisitionPeriod(w);
    WriteBinaryNumberOfUDPInterfaces(w);
    WriteBinaryNumberOfRows(w);
    WriteBinaryReadoutSpeed(w);
}

#ifdef HDF5C
void MasterAttributes::WriteJungfrauHDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5ExposureTime(group);
    WriteHDF5AcquisitionPeriod(group);
    WriteHDF5NumberOfUDPInterfaces(group);
    WriteHDF5NumberOfRows(group);
    WriteHDF5ReadoutSpeed(group);
}
#endif

void MasterAttributes::GetMoenchBinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinaryExposureTme(w);
    WriteBinaryAcquisitionPeriod(w);
    WriteBinaryNumberOfUDPInterfaces(w);
    WriteBinaryNumberOfRows(w);
    WriteBinaryReadoutSpeed(w);
}

#ifdef HDF5C
void MasterAttributes::WriteMoenchHDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5ExposureTime(group);
    WriteHDF5AcquisitionPeriod(group);
    WriteHDF5NumberOfUDPInterfaces(group);
    WriteHDF5NumberOfRows(group);
    WriteHDF5ReadoutSpeed(group);
}
#endif

void MasterAttributes::GetEigerBinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinaryDynamicRange(w);
    WriteBinaryTenGiga(w);
    WriteBinaryExposureTme(w);
    WriteBinaryAcquisitionPeriod(w);
    WriteBinaryThresholdEnergy(w);
    WriteBinarySubExposureTime(w);
    WriteBinarySubAcquisitionPeriod(w);
    WriteBinaryQuad(w);
    WriteBinaryNumberOfRows(w);
    WriteBinaryRateCorrections(w);
    WriteBinaryReadoutSpeed(w);
}

#ifdef HDF5C
void MasterAttributes::WriteEigerHDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5DynamicRange(group);
    WriteHDF5TenGiga(group);
    WriteHDF5ExposureTime(group);
    WriteHDF5AcquisitionPeriod(group);
    WriteHDF5ThresholdEnergy(group);
    WriteHDF5SubExposureTime(group);
    WriteHDF5SubAcquisitionPeriod(group);
    WriteHDF5Quad(group);
    WriteHDF5NumberOfRows(group);
    WriteHDF5RateCorrections(group);
    WriteHDF5ReadoutSpeed(group);
}
#endif

void MasterAttributes::GetMythen3BinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinaryDynamicRange(w);
    WriteBinaryTenGiga(w);
    WriteBinaryAcquisitionPeriod(w);
    WriteBinaryCounterMask(w);
    WriteBinaryExptimeArray(w);
    WriteBinaryGateDelayArray(w);
    WriteBinaryGates(w);
    WriteBinaryThresholdAllEnergy(w);
    WriteBinaryReadoutSpeed(w);
}

#ifdef HDF5C
void MasterAttributes::WriteMythen3HDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5DynamicRange(group);
    WriteHDF5TenGiga(group);
    WriteHDF5AcquisitionPeriod(group);
    WriteHDF5CounterMask(group);
    WriteHDF5ExptimeArray(group);
    WriteHDF5GateDelayArray(group);
    WriteHDF5Gates(group);
    WriteHDF5ThresholdAllEnergy(group);
    WriteHDF5ReadoutSpeed(group);
}
#endif

void MasterAttributes::GetGotthard2BinaryAttributes(writer *w) {
    WriteBinaryRois(w);
    WriteBinaryExposureTme(w);
    WriteBinaryAcquisitionPeriod(w);
    WriteBinaryBurstMode(w);
    WriteBinaryReadoutSpeed(w);
}

#ifdef HDF5C
void MasterAttributes::WriteGotthard2HDF5Attributes(H5::Group *group) {
    WriteHDF5ROIs(group);
    WriteHDF5ExposureTime(group);
    WriteHDF5AcquisitionPeriod(group);
    WriteHDF5BurstMode(group);
    WriteHDF5ReadoutSpeed(group);
}
#endif

void MasterAttributes::GetCtbBinaryAttributes(writer *w) {
    WriteBinaryExposureTme(w);
    WriteBinaryAcquisitionPeriod(w);
    WriteBinaryTenGiga(w);
    WriteBinaryAdcMask(w);
    WriteBinaryAnalogFlag(w);
    WriteBinaryAnalogSamples(w);
    WriteBinaryDigitalFlag(w);
    WriteBinaryDigitalSamples(w);
    WriteBinaryDBitOffset(w);
    WriteBinaryDBitReorder(w);
    WriteBinaryDBitBitset(w);
    WriteBinaryTransceiverMask(w);
    WriteBinaryTransceiverFlag(w);
    WriteBinaryTransceiverSamples(w);
}

#ifdef HDF5C
void MasterAttributes::WriteCtbHDF5Attributes(H5::Group *group) {
    WriteHDF5ExposureTime(group);
    WriteHDF5AcquisitionPeriod(group);
    WriteHDF5TenGiga(group);
    WriteHDF5AdcMask(group);
    WriteHDF5AnalogFlag(group);
    WriteHDF5AnalogSamples(group);
    WriteHDF5DigitalFlag(group);
    WriteHDF5DigitalSamples(group);
    WriteHDF5DBitOffset(group);
    WriteHDF5DBitReorder(group);
    WriteHDF5DBitBitset(group);
    WriteHDF5TransceiverMask(group);
    WriteHDF5TransceiverFlag(group);
    WriteHDF5TransceiverSamples(group);
}
#endif

void MasterAttributes::GetXilinxCtbBinaryAttributes(writer *w) {
    WriteBinaryExposureTme(w);
    WriteBinaryAcquisitionPeriod(w);
    WriteBinaryAdcMask(w);
    WriteBinaryAnalogFlag(w);
    WriteBinaryAnalogSamples(w);
    WriteBinaryDigitalFlag(w);
    WriteBinaryDigitalSamples(w);
    WriteBinaryDBitOffset(w);
    WriteBinaryDBitReorder(w);
    WriteBinaryDBitBitset(w);
    WriteBinaryTransceiverMask(w);
    WriteBinaryTransceiverFlag(w);
    WriteBinaryTransceiverSamples(w);
}

#ifdef HDF5C
void MasterAttributes::WriteXilinxCtbHDF5Attributes(H5::Group *group) {
    WriteHDF5ExposureTime(group);
    WriteHDF5AcquisitionPeriod(group);
    WriteHDF5AdcMask(group);
    WriteHDF5AnalogFlag(group);
    WriteHDF5AnalogSamples(group);
    WriteHDF5DigitalFlag(group);
    WriteHDF5DigitalSamples(group);
    WriteHDF5DBitOffset(group);
    WriteHDF5DBitReorder(group);
    WriteHDF5DBitBitset(group);
    WriteHDF5TransceiverMask(group);
    WriteHDF5TransceiverFlag(group);
    WriteHDF5TransceiverSamples(group);
}
#endif

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

void MasterAttributes::WriteBinaryDetectorType(writer *w) {
    WriteBinary(w, N_DETECTOR_TYPE.data(), ToString(detType));
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5DetectorType(H5::Group *group) {
    WriteHDF5String(group, N_DETECTOR_TYPE.data(), ToString(detType));
}
#endif

void MasterAttributes::WriteBinaryTimingMode(writer *w) {
    WriteBinary(w, N_TIMING_MODE.data(), ToString(timingMode));
}
#ifdef HDF5C
void MasterAttributes::WriteHDF5TimingMode(H5::Group *group) {
    WriteHDF5String(group, N_TIMING_MODE.data(), ToString(timingMode));
}
#endif

void MasterAttributes::WriteBinaryGeometry(writer *w) {
    WriteBinaryXY(w, N_GEOMETRY.data(), geometry);
}
#ifdef HDF5C
void MasterAttributes::WriteHDF5Geometry(H5::Group *group) {
    WriteHDF5XY(group, N_GEOMETRY.data(), geometry);
}
#endif

void MasterAttributes::WriteBinaryImageSize(writer *w) {
    WriteBinary(w, N_IMAGE_SIZE.data(), imageSize);
}
#ifdef HDF5C
void MasterAttributes::WriteHDF5ImageSize(H5::Group *group) {
    WriteHDF5Int(group, N_IMAGE_SIZE.data(), imageSize);
}
#endif
void MasterAttributes::WriteBinaryPixels(writer *w) {
    WriteBinaryXY(w, N_PIXELS.data(), nPixels);
}
#ifdef HDF5C
void MasterAttributes::WriteHDF5Pixels(H5::Group *group) {
    WriteHDF5XY(group, N_PIXELS.data(), nPixels);
}
#endif

void MasterAttributes::WriteBinaryMaxFramesPerFile(writer *w) {
    WriteBinary(w, N_MAX_FRAMES_PER_FILE.data(), maxFramesPerFile);
}
#ifdef HDF5C
void MasterAttributes::WriteHDF5MaxFramesPerFile(H5::Group *group) {
    WriteHDF5Int(group, N_MAX_FRAMES_PER_FILE.data(), maxFramesPerFile);
}
#endif
void MasterAttributes::WriteBinaryFrameDiscardPolicy(writer *w) {
    WriteBinary(w, N_FRAME_DISCARD_POLICY.data(), ToString(frameDiscardMode));
}
#ifdef HDF5C
void MasterAttributes::WriteHDF5FrameDiscardPolicy(H5::Group *group) {
    WriteHDF5String(group, N_FRAME_DISCARD_POLICY.data(),
                    ToString(frameDiscardMode));
}
#endif
void MasterAttributes::WriteBinaryFramePadding(writer *w) {
    WriteBinary(w, N_FRAME_PADDING.data(), framePadding);
}
#ifdef HDF5C
void MasterAttributes::WriteHDF5FramePadding(H5::Group *group) {
    WriteHDF5Int(group, N_FRAME_PADDING.data(), framePadding);
}
#endif

void MasterAttributes::WriteBinaryTotalFrames(writer *w) {
    WriteBinary(w, N_TOTAL_FRAMES.data(), totalFrames);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5TotalFrames(H5::Group *group) {
    WriteHDF5Int(group, N_TOTAL_FRAMES.data(), totalFrames);
}
#endif

void MasterAttributes::WriteBinaryFramesInFile(writer *w) {
    WriteBinary(w, N_FRAMES_IN_FILE.data(), framesInFile);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5FramesInFile(H5::Group *group) {
    WriteHDF5Int(group, N_FRAMES_IN_FILE.data(), framesInFile);
}
#endif

void MasterAttributes::WriteBinaryExposureTme(writer *w) {
    WriteBinary(w, N_EXPOSURE_TIME.data(), ToString(exptime));
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5ExposureTime(H5::Group *group) {
    WriteHDF5String(group, N_EXPOSURE_TIME.data(), ToString(exptime));
}
#endif

void MasterAttributes::WriteBinaryAcquisitionPeriod(writer *w) {
    WriteBinary(w, N_ACQUISITION_PERIOD.data(), ToString(period));
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5AcquisitionPeriod(H5::Group *group) {
    WriteHDF5String(group, N_ACQUISITION_PERIOD.data(), ToString(period));
}
#endif

void MasterAttributes::WriteBinaryNumberOfUDPInterfaces(writer *w) {
    WriteBinary(w, N_NUM_UDP_INTERFACES.data(), numUDPInterfaces);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5NumberOfUDPInterfaces(H5::Group *group) {
    WriteHDF5Int(group, N_NUM_UDP_INTERFACES.data(), numUDPInterfaces);
}
#endif

void MasterAttributes::WriteBinaryNumberOfRows(writer *w) {
    WriteBinary(w, N_NUMBER_OF_ROWS.data(), readNRows);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5NumberOfRows(H5::Group *group) {
    WriteHDF5Int(group, N_NUMBER_OF_ROWS.data(), readNRows);
}
#endif

void MasterAttributes::WriteBinaryReadoutSpeed(writer *w) {
    WriteBinary(w, N_READOUT_SPEED.data(), ToString(readoutSpeed));
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5ReadoutSpeed(H5::Group *group) {
    WriteHDF5String(group, N_READOUT_SPEED.data(), ToString(readoutSpeed));
}
#endif

void MasterAttributes::WriteBinaryDynamicRange(writer *w) {
    WriteBinary(w, N_DYNAMIC_RANGE.data(), dynamicRange);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5DynamicRange(H5::Group *group) {
    WriteHDF5Int(group, N_DYNAMIC_RANGE.data(), dynamicRange);
}
#endif

void MasterAttributes::WriteBinaryTenGiga(writer *w) {
    WriteBinary(w, N_TEN_GIGA.data(), tenGiga);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5TenGiga(H5::Group *group) {
    WriteHDF5Int(group, N_TEN_GIGA.data(), tenGiga);
}
#endif

void MasterAttributes::WriteBinaryThresholdEnergy(writer *w) {
    WriteBinary(w, N_THRESHOLD_ENERGY.data(), thresholdEnergyeV);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5ThresholdEnergy(H5::Group *group) {
    WriteHDF5Int(group, N_THRESHOLD_ENERGY.data(), thresholdEnergyeV);
}
#endif

void MasterAttributes::WriteBinarySubExposureTime(writer *w) {
    WriteBinary(w, N_SUB_EXPOSURE_TIME.data(), ToString(subExptime));
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5SubExposureTime(H5::Group *group) {
    WriteHDF5String(group, N_SUB_EXPOSURE_TIME.data(), ToString(subExptime));
}
#endif

void MasterAttributes::WriteBinarySubAcquisitionPeriod(writer *w) {
    WriteBinary(w, N_SUB_ACQUISITION_PERIOD.data(), ToString(subPeriod));
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5SubAcquisitionPeriod(H5::Group *group) {
    WriteHDF5String(group, N_SUB_ACQUISITION_PERIOD.data(),
                    ToString(subPeriod));
}
#endif

void MasterAttributes::WriteBinaryQuad(writer *w) {
    WriteBinary(w, N_QUAD.data(), quad);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5Quad(H5::Group *group) {
    WriteHDF5Int(group, N_QUAD.data(), quad);
}
#endif

void MasterAttributes::WriteBinaryRateCorrections(writer *w) {
    WriteBinary(w, N_RATE_CORRECTIONS.data(), ratecorr);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5RateCorrections(H5::Group *group) {
    WriteHDF5Int(group, N_RATE_CORRECTIONS.data(), ratecorr);
}
#endif

void MasterAttributes::WriteBinaryCounterMask(writer *w) {
    WriteBinary(w, N_COUNTER_MASK.data(), counterMask);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5CounterMask(H5::Group *group) {
    WriteHDF5Int(group, N_COUNTER_MASK.data(), counterMask);
}
#endif

void MasterAttributes::WriteBinaryExptimeArray(writer *w) {
    WriteBinary(w, N_EXPOSURE_TIMES.data(), exptimeArray);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5ExptimeArray(H5::Group *group) {
    std::vector<std::string> timeStrings;
    for (auto &e : exptimeArray) {
        timeStrings.push_back(ToString(e));
    }
    WriteHDF5StringArray(group, N_EXPOSURE_TIMES.data(), timeStrings);
}
#endif

void MasterAttributes::WriteBinaryGateDelayArray(writer *w) {
    WriteBinary(w, N_GATE_DELAYS.data(), gateDelayArray);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5GateDelayArray(H5::Group *group) {
    std::vector<std::string> timeStrings;
    for (auto &g : gateDelayArray) {
        timeStrings.push_back(ToString(g));
    }
    WriteHDF5StringArray(group, N_GATE_DELAYS.data(), timeStrings);
}
#endif

void MasterAttributes::WriteBinaryGates(writer *w) {
    WriteBinary(w, N_GATES.data(), gates);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5Gates(H5::Group *group) {
    WriteHDF5Int(group, N_GATES.data(), gates);
}
#endif

void MasterAttributes::WriteBinaryThresholdAllEnergy(writer *w) {
    WriteBinary(w, N_THRESHOLD_ENERGIES.data(), thresholdAllEnergyeV);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5ThresholdAllEnergy(H5::Group *group) {
    WriteHDF5Int(group, N_THRESHOLD_ENERGIES.data(), thresholdAllEnergyeV);
}
#endif

void MasterAttributes::WriteBinaryBurstMode(writer *w) {
    WriteBinary(w, N_BURST_MODE.data(), ToString(burstMode));
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5BurstMode(H5::Group *group) {
    WriteHDF5String(group, N_BURST_MODE.data(), ToString(burstMode));
}
#endif

void MasterAttributes::WriteBinaryAdcMask(writer *w) {
    WriteBinary(w, N_ADC_MASK.data(), adcMask);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5AdcMask(H5::Group *group) {
    WriteHDF5Int(group, N_ADC_MASK.data(), adcMask);
}
#endif

void MasterAttributes::WriteBinaryAnalogFlag(writer *w) {
    WriteBinary(w, N_ANALOG.data(), analog);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5AnalogFlag(H5::Group *group) {
    WriteHDF5Int(group, N_ANALOG.data(), analog);
}
#endif

void MasterAttributes::WriteBinaryAnalogSamples(writer *w) {
    WriteBinary(w, N_ANALOG_SAMPLES.data(), analogSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5AnalogSamples(H5::Group *group) {
    WriteHDF5Int(group, N_ANALOG_SAMPLES.data(), analogSamples);
}
#endif

void MasterAttributes::WriteBinaryDigitalFlag(writer *w) {
    WriteBinary(w, N_DIGITAL.data(), digital);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5DigitalFlag(H5::Group *group) {
    WriteHDF5Int(group, N_DIGITAL.data(), digital);
}
#endif

void MasterAttributes::WriteBinaryDigitalSamples(writer *w) {
    WriteBinary(w, N_DIGITAL_SAMPLES.data(), digitalSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5DigitalSamples(H5::Group *group) {
    WriteHDF5Int(group, N_DIGITAL_SAMPLES.data(), digitalSamples);
}
#endif

void MasterAttributes::WriteBinaryDBitOffset(writer *w) {
    WriteBinary(w, N_DBIT_OFFSET.data(), dbitOffset);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5DBitOffset(H5::Group *group) {
    WriteHDF5Int(group, N_DBIT_OFFSET.data(), dbitOffset);
}
#endif

void MasterAttributes::WriteBinaryDBitReorder(writer *w) {
    WriteBinary(w, N_DBIT_REORDER.data(), dbitReorder);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5DBitReorder(H5::Group *group) {
    WriteHDF5Int(group, N_DBIT_REORDER.data(), dbitReorder);
}
#endif

void MasterAttributes::WriteBinaryDBitBitset(writer *w) {
    WriteBinary(w, N_DBIT_BITSET.data(), dbitList);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5DBitBitset(H5::Group *group) {
    WriteHDF5Int(group, N_DBIT_BITSET.data(), dbitList);
}
#endif

void MasterAttributes::WriteBinaryTransceiverMask(writer *w) {
    WriteBinary(w, N_TRANSCEIVER_MASK.data(), transceiverMask);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5TransceiverMask(H5::Group *group) {
    WriteHDF5Int(group, N_TRANSCEIVER_MASK.data(), transceiverMask);
}
#endif

void MasterAttributes::WriteBinaryTransceiverFlag(writer *w) {
    WriteBinary(w, N_TRANSCEIVER.data(), transceiver);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5TransceiverFlag(H5::Group *group) {
    WriteHDF5Int(group, N_TRANSCEIVER.data(), transceiver);
}
#endif

void MasterAttributes::WriteBinaryTransceiverSamples(writer *w) {
    WriteBinary(w, N_TRANSCEIVER_SAMPLES.data(), transceiverSamples);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5TransceiverSamples(H5::Group *group) {
    WriteHDF5Int(group, N_TRANSCEIVER_SAMPLES.data(), transceiverSamples);
}
#endif

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
#endif

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

#ifdef HDF5C
void MasterAttributes::WriteHDF5XY(H5::Group *group, const std::string &name,
                                   const defs::xy &xy) {
    H5::CompType c(sizeof(defs::xy));
    c.insertMember("x", HOFFSET(defs::xy, x), H5::PredType::NATIVE_INT);
    c.insertMember("y", HOFFSET(defs::xy, y), H5::PredType::NATIVE_INT);
    H5::DataSpace dataspace(H5S_SCALAR);
    H5::DataSet dataset = group->createDataSet(name, c, dataspace);
    dataset.write(&xy, c);
}
#endif

void MasterAttributes::WriteBinaryVersion(writer *w) {
    w->Key(N_VERSION.data());
    w->SetMaxDecimalPlaces(2);
    w->Double(BINARY_WRITER_VERSION);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5Version(H5::H5File *fd) {
    H5::DataSpace dataspace(H5S_SCALAR);
    H5::Attribute attribute = fd->createAttribute(
        N_VERSION.data(), H5::PredType::NATIVE_DOUBLE, dataspace);
    double version = HDF5_WRITER_VERSION;
    attribute.write(H5::PredType::NATIVE_DOUBLE, &version);
}
#endif

void MasterAttributes::WriteBinaryTimestamp(writer *w) {
    time_t t = std::time(nullptr);
    std::string sTime(ctime(&t));
    std::replace(sTime.begin(), sTime.end(), '\n', '\0');
    WriteBinary(w, N_TIMESTAMP.data(), sTime);
}

#ifdef HDF5C
void MasterAttributes::WriteHDF5Timestamp(H5::Group *group) {
    time_t t = std::time(nullptr);
    std::string sTime(ctime(&t));
    WriteHDF5String(group, N_TIMESTAMP.data(), sTime);
}
#endif

void MasterAttributes::WriteBinaryRois(writer *w) {
    w->Key(N_RECEIVER_ROIS.data());
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

#ifdef HDF5C
void MasterAttributes::WriteHDF5ROIs(H5::Group *group) {
    H5::CompType c(sizeof(defs::ROI));
    c.insertMember("xmin", HOFFSET(defs::ROI, xmin), H5::PredType::NATIVE_INT);
    c.insertMember("xmax", HOFFSET(defs::ROI, xmax), H5::PredType::NATIVE_INT);
    c.insertMember("ymin", HOFFSET(defs::ROI, ymin), H5::PredType::NATIVE_INT);
    c.insertMember("ymax", HOFFSET(defs::ROI, ymax), H5::PredType::NATIVE_INT);
    hsize_t dims[1] = {rois.size()}; // 1d dataspace with size of roi elements
    H5::DataSpace dataspace(1, dims);
    H5::DataSet dataset =
        group->createDataSet(N_RECEIVER_ROIS.data(), c, dataspace);
    dataset.write(rois.data(), c);
}

#endif

void MasterAttributes::WriteBinaryScanParameters(writer *w) {
    w->Key(N_SCAN_PARAMETERS.data());
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

#ifdef HDF5C
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
    H5::DataSet dataset =
        group->createDataSet(N_SCAN_PARAMETERS.data(), c, dataspace);
    dataset.write(&scanParams, c);
}
#endif

void MasterAttributes::WriteBinaryJsonHeader(writer *w) {
    w->Key(N_ADDITIONAL_JSON_HEADER.data());
    w->StartObject();
    for (const auto &pair : additionalJsonHeader) {
        w->Key(pair.first.c_str());
        w->String(pair.second.c_str());
    }
    w->EndObject();
}

#ifdef HDF5C
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
    H5::DataSet dataset = group->createDataSet(N_ADDITIONAL_JSON_HEADER.data(),
                                               mapType, dataspace);
    dataset.write(raw.data(), mapType);
}
#endif

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

} // namespace sls
