/* WARINING This file is auto generated any edits might be overwritten without
 * warning */

// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "py_headers.h"

#include "sls/Pattern.h"
#include "sls/sls_detector_defs.h"
namespace py = pybind11;
void init_enums(py::module &m) {
    py::class_<slsDetectorDefs> Defs(m, "slsDetectorDefs");
    py::class_<slsDetectorDefs::xy> xy(m, "xy");
    xy.def(py::init());
    xy.def(py::init<int, int>());
    xy.def_readwrite("x", &slsDetectorDefs::xy::x);
    xy.def_readwrite("y", &slsDetectorDefs::xy::y);

    py::enum_<slsDetectorDefs::detectorType>(Defs, "detectorType")
        .value("GENERIC", slsDetectorDefs::detectorType::GENERIC)
        .value("EIGER", slsDetectorDefs::detectorType::EIGER)
        .value("GOTTHARD", slsDetectorDefs::detectorType::GOTTHARD)
        .value("JUNGFRAU", slsDetectorDefs::detectorType::JUNGFRAU)
        .value("CHIPTESTBOARD", slsDetectorDefs::detectorType::CHIPTESTBOARD)
        .value("MOENCH", slsDetectorDefs::detectorType::MOENCH)
        .value("MYTHEN3", slsDetectorDefs::detectorType::MYTHEN3)
        .value("GOTTHARD2", slsDetectorDefs::detectorType::GOTTHARD2)
        .value("XILINX_CHIPTESTBOARD",
               slsDetectorDefs::detectorType::XILINX_CHIPTESTBOARD)
        .export_values();

    py::enum_<slsDetectorDefs::runStatus>(Defs, "runStatus")
        .value("IDLE", slsDetectorDefs::runStatus::IDLE)
        .value("ERROR", slsDetectorDefs::runStatus::ERROR)
        .value("WAITING", slsDetectorDefs::runStatus::WAITING)
        .value("RUN_FINISHED", slsDetectorDefs::runStatus::RUN_FINISHED)
        .value("TRANSMITTING", slsDetectorDefs::runStatus::TRANSMITTING)
        .value("RUNNING", slsDetectorDefs::runStatus::RUNNING)
        .value("STOPPED", slsDetectorDefs::runStatus::STOPPED)
        .export_values();

    py::enum_<slsDetectorDefs::dimension>(Defs, "dimension")
        .value("X", slsDetectorDefs::dimension::X)
        .value("Y", slsDetectorDefs::dimension::Y)
        .export_values();

    py::enum_<slsDetectorDefs::frameDiscardPolicy>(Defs, "frameDiscardPolicy")
        .value("NO_DISCARD", slsDetectorDefs::frameDiscardPolicy::NO_DISCARD)
        .value("DISCARD_EMPTY_FRAMES",
               slsDetectorDefs::frameDiscardPolicy::DISCARD_EMPTY_FRAMES)
        .value("DISCARD_PARTIAL_FRAMES",
               slsDetectorDefs::frameDiscardPolicy::DISCARD_PARTIAL_FRAMES)
        .value("NUM_DISCARD_POLICIES",
               slsDetectorDefs::frameDiscardPolicy::NUM_DISCARD_POLICIES)
        .export_values();

    py::enum_<slsDetectorDefs::fileFormat>(Defs, "fileFormat")
        .value("BINARY", slsDetectorDefs::fileFormat::BINARY)
        .value("HDF5", slsDetectorDefs::fileFormat::HDF5)
        .value("NUM_FILE_FORMATS",
               slsDetectorDefs::fileFormat::NUM_FILE_FORMATS)
        .export_values();

    py::enum_<slsDetectorDefs::externalSignalFlag>(Defs, "externalSignalFlag")
        .value("TRIGGER_IN_RISING_EDGE",
               slsDetectorDefs::externalSignalFlag::TRIGGER_IN_RISING_EDGE)
        .value("TRIGGER_IN_FALLING_EDGE",
               slsDetectorDefs::externalSignalFlag::TRIGGER_IN_FALLING_EDGE)
        .value("INVERSION_ON",
               slsDetectorDefs::externalSignalFlag::INVERSION_ON)
        .value("INVERSION_OFF",
               slsDetectorDefs::externalSignalFlag::INVERSION_OFF)
        .export_values();

    py::enum_<slsDetectorDefs::timingMode>(Defs, "timingMode")
        .value("AUTO_TIMING", slsDetectorDefs::timingMode::AUTO_TIMING)
        .value("TRIGGER_EXPOSURE",
               slsDetectorDefs::timingMode::TRIGGER_EXPOSURE)
        .value("GATED", slsDetectorDefs::timingMode::GATED)
        .value("BURST_TRIGGER", slsDetectorDefs::timingMode::BURST_TRIGGER)
        .value("TRIGGER_GATED", slsDetectorDefs::timingMode::TRIGGER_GATED)
        .value("NUM_TIMING_MODES",
               slsDetectorDefs::timingMode::NUM_TIMING_MODES)
        .export_values();

    py::enum_<slsDetectorDefs::dacIndex>(Defs, "dacIndex")
        .value("DAC_0", slsDetectorDefs::dacIndex::DAC_0)
        .value("DAC_1", slsDetectorDefs::dacIndex::DAC_1)
        .value("DAC_2", slsDetectorDefs::dacIndex::DAC_2)
        .value("DAC_3", slsDetectorDefs::dacIndex::DAC_3)
        .value("DAC_4", slsDetectorDefs::dacIndex::DAC_4)
        .value("DAC_5", slsDetectorDefs::dacIndex::DAC_5)
        .value("DAC_6", slsDetectorDefs::dacIndex::DAC_6)
        .value("DAC_7", slsDetectorDefs::dacIndex::DAC_7)
        .value("DAC_8", slsDetectorDefs::dacIndex::DAC_8)
        .value("DAC_9", slsDetectorDefs::dacIndex::DAC_9)
        .value("DAC_10", slsDetectorDefs::dacIndex::DAC_10)
        .value("DAC_11", slsDetectorDefs::dacIndex::DAC_11)
        .value("DAC_12", slsDetectorDefs::dacIndex::DAC_12)
        .value("DAC_13", slsDetectorDefs::dacIndex::DAC_13)
        .value("DAC_14", slsDetectorDefs::dacIndex::DAC_14)
        .value("DAC_15", slsDetectorDefs::dacIndex::DAC_15)
        .value("DAC_16", slsDetectorDefs::dacIndex::DAC_16)
        .value("DAC_17", slsDetectorDefs::dacIndex::DAC_17)
        .value("VSVP", slsDetectorDefs::dacIndex::VSVP)
        .value("VTRIM", slsDetectorDefs::dacIndex::VTRIM)
        .value("VRPREAMP", slsDetectorDefs::dacIndex::VRPREAMP)
        .value("VRSHAPER", slsDetectorDefs::dacIndex::VRSHAPER)
        .value("VSVN", slsDetectorDefs::dacIndex::VSVN)
        .value("VTGSTV", slsDetectorDefs::dacIndex::VTGSTV)
        .value("VCMP_LL", slsDetectorDefs::dacIndex::VCMP_LL)
        .value("VCMP_LR", slsDetectorDefs::dacIndex::VCMP_LR)
        .value("VCAL", slsDetectorDefs::dacIndex::VCAL)
        .value("VCMP_RL", slsDetectorDefs::dacIndex::VCMP_RL)
        .value("RXB_RB", slsDetectorDefs::dacIndex::RXB_RB)
        .value("RXB_LB", slsDetectorDefs::dacIndex::RXB_LB)
        .value("VCMP_RR", slsDetectorDefs::dacIndex::VCMP_RR)
        .value("VCP", slsDetectorDefs::dacIndex::VCP)
        .value("VCN", slsDetectorDefs::dacIndex::VCN)
        .value("VISHAPER", slsDetectorDefs::dacIndex::VISHAPER)
        .value("VTHRESHOLD", slsDetectorDefs::dacIndex::VTHRESHOLD)
        .value("IO_DELAY", slsDetectorDefs::dacIndex::IO_DELAY)
        .value("VREF_DS", slsDetectorDefs::dacIndex::VREF_DS)
        .value("VCASCN_PB", slsDetectorDefs::dacIndex::VCASCN_PB)
        .value("VCASCP_PB", slsDetectorDefs::dacIndex::VCASCP_PB)
        .value("VOUT_CM", slsDetectorDefs::dacIndex::VOUT_CM)
        .value("VCASC_OUT", slsDetectorDefs::dacIndex::VCASC_OUT)
        .value("VIN_CM", slsDetectorDefs::dacIndex::VIN_CM)
        .value("VREF_COMP", slsDetectorDefs::dacIndex::VREF_COMP)
        .value("IB_TESTC", slsDetectorDefs::dacIndex::IB_TESTC)
        .value("VB_COMP", slsDetectorDefs::dacIndex::VB_COMP)
        .value("VDD_PROT", slsDetectorDefs::dacIndex::VDD_PROT)
        .value("VIN_COM", slsDetectorDefs::dacIndex::VIN_COM)
        .value("VREF_PRECH", slsDetectorDefs::dacIndex::VREF_PRECH)
        .value("VB_PIXBUF", slsDetectorDefs::dacIndex::VB_PIXBUF)
        .value("VB_DS", slsDetectorDefs::dacIndex::VB_DS)
        .value("VREF_H_ADC", slsDetectorDefs::dacIndex::VREF_H_ADC)
        .value("VB_COMP_FE", slsDetectorDefs::dacIndex::VB_COMP_FE)
        .value("VB_COMP_ADC", slsDetectorDefs::dacIndex::VB_COMP_ADC)
        .value("VCOM_CDS", slsDetectorDefs::dacIndex::VCOM_CDS)
        .value("VREF_RSTORE", slsDetectorDefs::dacIndex::VREF_RSTORE)
        .value("VB_OPA_1ST", slsDetectorDefs::dacIndex::VB_OPA_1ST)
        .value("VREF_COMP_FE", slsDetectorDefs::dacIndex::VREF_COMP_FE)
        .value("VCOM_ADC1", slsDetectorDefs::dacIndex::VCOM_ADC1)
        .value("VREF_L_ADC", slsDetectorDefs::dacIndex::VREF_L_ADC)
        .value("VREF_CDS", slsDetectorDefs::dacIndex::VREF_CDS)
        .value("VB_CS", slsDetectorDefs::dacIndex::VB_CS)
        .value("VB_OPA_FD", slsDetectorDefs::dacIndex::VB_OPA_FD)
        .value("VCOM_ADC2", slsDetectorDefs::dacIndex::VCOM_ADC2)
        .value("VCASSH", slsDetectorDefs::dacIndex::VCASSH)
        .value("VTH2", slsDetectorDefs::dacIndex::VTH2)
        .value("VRSHAPER_N", slsDetectorDefs::dacIndex::VRSHAPER_N)
        .value("VIPRE_OUT", slsDetectorDefs::dacIndex::VIPRE_OUT)
        .value("VTH3", slsDetectorDefs::dacIndex::VTH3)
        .value("VTH1", slsDetectorDefs::dacIndex::VTH1)
        .value("VICIN", slsDetectorDefs::dacIndex::VICIN)
        .value("VCAS", slsDetectorDefs::dacIndex::VCAS)
        .value("VCAL_N", slsDetectorDefs::dacIndex::VCAL_N)
        .value("VIPRE", slsDetectorDefs::dacIndex::VIPRE)
        .value("VCAL_P", slsDetectorDefs::dacIndex::VCAL_P)
        .value("VDCSH", slsDetectorDefs::dacIndex::VDCSH)
        .value("VBP_COLBUF", slsDetectorDefs::dacIndex::VBP_COLBUF)
        .value("VB_SDA", slsDetectorDefs::dacIndex::VB_SDA)
        .value("VCASC_SFP", slsDetectorDefs::dacIndex::VCASC_SFP)
        .value("VIPRE_CDS", slsDetectorDefs::dacIndex::VIPRE_CDS)
        .value("IBIAS_SFP", slsDetectorDefs::dacIndex::IBIAS_SFP)
        .value("ADC_VPP", slsDetectorDefs::dacIndex::ADC_VPP)
        .value("HIGH_VOLTAGE", slsDetectorDefs::dacIndex::HIGH_VOLTAGE)
        .value("TEMPERATURE_ADC", slsDetectorDefs::dacIndex::TEMPERATURE_ADC)
        .value("TEMPERATURE_FPGA", slsDetectorDefs::dacIndex::TEMPERATURE_FPGA)
        .value("TEMPERATURE_FPGAEXT",
               slsDetectorDefs::dacIndex::TEMPERATURE_FPGAEXT)
        .value("TEMPERATURE_10GE", slsDetectorDefs::dacIndex::TEMPERATURE_10GE)
        .value("TEMPERATURE_DCDC", slsDetectorDefs::dacIndex::TEMPERATURE_DCDC)
        .value("TEMPERATURE_SODL", slsDetectorDefs::dacIndex::TEMPERATURE_SODL)
        .value("TEMPERATURE_SODR", slsDetectorDefs::dacIndex::TEMPERATURE_SODR)
        .value("TEMPERATURE_FPGA2",
               slsDetectorDefs::dacIndex::TEMPERATURE_FPGA2)
        .value("TEMPERATURE_FPGA3",
               slsDetectorDefs::dacIndex::TEMPERATURE_FPGA3)
        .value("TRIMBIT_SCAN", slsDetectorDefs::dacIndex::TRIMBIT_SCAN)
        .value("V_POWER_A", slsDetectorDefs::dacIndex::V_POWER_A)
        .value("V_POWER_B", slsDetectorDefs::dacIndex::V_POWER_B)
        .value("V_POWER_C", slsDetectorDefs::dacIndex::V_POWER_C)
        .value("V_POWER_D", slsDetectorDefs::dacIndex::V_POWER_D)
        .value("V_POWER_IO", slsDetectorDefs::dacIndex::V_POWER_IO)
        .value("V_POWER_CHIP", slsDetectorDefs::dacIndex::V_POWER_CHIP)
        .value("I_POWER_A", slsDetectorDefs::dacIndex::I_POWER_A)
        .value("I_POWER_B", slsDetectorDefs::dacIndex::I_POWER_B)
        .value("I_POWER_C", slsDetectorDefs::dacIndex::I_POWER_C)
        .value("I_POWER_D", slsDetectorDefs::dacIndex::I_POWER_D)
        .value("I_POWER_IO", slsDetectorDefs::dacIndex::I_POWER_IO)
        .value("V_LIMIT", slsDetectorDefs::dacIndex::V_LIMIT)
        .value("SLOW_ADC0", slsDetectorDefs::dacIndex::SLOW_ADC0)
        .value("SLOW_ADC1", slsDetectorDefs::dacIndex::SLOW_ADC1)
        .value("SLOW_ADC2", slsDetectorDefs::dacIndex::SLOW_ADC2)
        .value("SLOW_ADC3", slsDetectorDefs::dacIndex::SLOW_ADC3)
        .value("SLOW_ADC4", slsDetectorDefs::dacIndex::SLOW_ADC4)
        .value("SLOW_ADC5", slsDetectorDefs::dacIndex::SLOW_ADC5)
        .value("SLOW_ADC6", slsDetectorDefs::dacIndex::SLOW_ADC6)
        .value("SLOW_ADC7", slsDetectorDefs::dacIndex::SLOW_ADC7)
        .value("SLOW_ADC_TEMP", slsDetectorDefs::dacIndex::SLOW_ADC_TEMP)
        .export_values();

    py::enum_<slsDetectorDefs::detectorSettings>(Defs, "detectorSettings")
        .value("STANDARD", slsDetectorDefs::detectorSettings::STANDARD)
        .value("FAST", slsDetectorDefs::detectorSettings::FAST)
        .value("HIGHGAIN", slsDetectorDefs::detectorSettings::HIGHGAIN)
        .value("DYNAMICGAIN", slsDetectorDefs::detectorSettings::DYNAMICGAIN)
        .value("LOWGAIN", slsDetectorDefs::detectorSettings::LOWGAIN)
        .value("MEDIUMGAIN", slsDetectorDefs::detectorSettings::MEDIUMGAIN)
        .value("VERYHIGHGAIN", slsDetectorDefs::detectorSettings::VERYHIGHGAIN)
        .value("HIGHGAIN0", slsDetectorDefs::detectorSettings::HIGHGAIN0)
        .value("FIXGAIN1", slsDetectorDefs::detectorSettings::FIXGAIN1)
        .value("FIXGAIN2", slsDetectorDefs::detectorSettings::FIXGAIN2)
        .value("VERYLOWGAIN", slsDetectorDefs::detectorSettings::VERYLOWGAIN)
        .value("G1_HIGHGAIN", slsDetectorDefs::detectorSettings::G1_HIGHGAIN)
        .value("G1_LOWGAIN", slsDetectorDefs::detectorSettings::G1_LOWGAIN)
        .value("G2_HIGHCAP_HIGHGAIN",
               slsDetectorDefs::detectorSettings::G2_HIGHCAP_HIGHGAIN)
        .value("G2_HIGHCAP_LOWGAIN",
               slsDetectorDefs::detectorSettings::G2_HIGHCAP_LOWGAIN)
        .value("G2_LOWCAP_HIGHGAIN",
               slsDetectorDefs::detectorSettings::G2_LOWCAP_HIGHGAIN)
        .value("G2_LOWCAP_LOWGAIN",
               slsDetectorDefs::detectorSettings::G2_LOWCAP_LOWGAIN)
        .value("G4_HIGHGAIN", slsDetectorDefs::detectorSettings::G4_HIGHGAIN)
        .value("G4_LOWGAIN", slsDetectorDefs::detectorSettings::G4_LOWGAIN)
        .value("GAIN0", slsDetectorDefs::detectorSettings::GAIN0)
        .value("UNDEFINED", slsDetectorDefs::detectorSettings::UNDEFINED)
        .value("UNINITIALIZED",
               slsDetectorDefs::detectorSettings::UNINITIALIZED)
        .export_values();

    py::enum_<slsDetectorDefs::clockIndex>(Defs, "clockIndex")
        .value("ADC_CLOCK", slsDetectorDefs::clockIndex::ADC_CLOCK)
        .value("DBIT_CLOCK", slsDetectorDefs::clockIndex::DBIT_CLOCK)
        .value("RUN_CLOCK", slsDetectorDefs::clockIndex::RUN_CLOCK)
        .value("SYNC_CLOCK", slsDetectorDefs::clockIndex::SYNC_CLOCK)
        .export_values();

    py::enum_<slsDetectorDefs::readoutMode>(Defs, "readoutMode")
        .value("ANALOG_ONLY", slsDetectorDefs::readoutMode::ANALOG_ONLY)
        .value("DIGITAL_ONLY", slsDetectorDefs::readoutMode::DIGITAL_ONLY)
        .value("ANALOG_AND_DIGITAL",
               slsDetectorDefs::readoutMode::ANALOG_AND_DIGITAL)
        .value("TRANSCEIVER_ONLY",
               slsDetectorDefs::readoutMode::TRANSCEIVER_ONLY)
        .value("DIGITAL_AND_TRANSCEIVER",
               slsDetectorDefs::readoutMode::DIGITAL_AND_TRANSCEIVER)
        .export_values();

    py::enum_<slsDetectorDefs::speedLevel>(Defs, "speedLevel")
        .value("FULL_SPEED", slsDetectorDefs::speedLevel::FULL_SPEED)
        .value("HALF_SPEED", slsDetectorDefs::speedLevel::HALF_SPEED)
        .value("QUARTER_SPEED", slsDetectorDefs::speedLevel::QUARTER_SPEED)
        .value("G2_108MHZ", slsDetectorDefs::speedLevel::G2_108MHZ)
        .value("G2_144MHZ", slsDetectorDefs::speedLevel::G2_144MHZ)
        .export_values();

    py::enum_<slsDetectorDefs::burstMode>(Defs, "burstMode")
        .value("BURST_INTERNAL", slsDetectorDefs::burstMode::BURST_INTERNAL)
        .value("BURST_EXTERNAL", slsDetectorDefs::burstMode::BURST_EXTERNAL)
        .value("CONTINUOUS_INTERNAL",
               slsDetectorDefs::burstMode::CONTINUOUS_INTERNAL)
        .value("CONTINUOUS_EXTERNAL",
               slsDetectorDefs::burstMode::CONTINUOUS_EXTERNAL)
        .value("NUM_BURST_MODES", slsDetectorDefs::burstMode::NUM_BURST_MODES)
        .export_values();

    py::enum_<slsDetectorDefs::timingSourceType>(Defs, "timingSourceType")
        .value("TIMING_INTERNAL",
               slsDetectorDefs::timingSourceType::TIMING_INTERNAL)
        .value("TIMING_EXTERNAL",
               slsDetectorDefs::timingSourceType::TIMING_EXTERNAL)
        .export_values();

    py::enum_<slsDetectorDefs::M3_GainCaps>(Defs, "M3_GainCaps",
                                            py::arithmetic())
        .value("M3_C10pre", slsDetectorDefs::M3_GainCaps::M3_C10pre)
        .value("M3_C15sh", slsDetectorDefs::M3_GainCaps::M3_C15sh)
        .value("M3_C30sh", slsDetectorDefs::M3_GainCaps::M3_C30sh)
        .value("M3_C50sh", slsDetectorDefs::M3_GainCaps::M3_C50sh)
        .value("M3_C225ACsh", slsDetectorDefs::M3_GainCaps::M3_C225ACsh)
        .value("M3_C15pre", slsDetectorDefs::M3_GainCaps::M3_C15pre)
        .export_values();

    py::enum_<slsDetectorDefs::portPosition>(Defs, "portPosition")
        .value("LEFT", slsDetectorDefs::portPosition::LEFT)
        .value("RIGHT", slsDetectorDefs::portPosition::RIGHT)
        .value("TOP", slsDetectorDefs::portPosition::TOP)
        .value("BOTTOM", slsDetectorDefs::portPosition::BOTTOM)
        .export_values();

    py::enum_<slsDetectorDefs::fpgaPosition>(Defs, "fpgaPosition")
        .value("FRONT_LEFT", slsDetectorDefs::fpgaPosition::FRONT_LEFT)
        .value("FRONT_RIGHT", slsDetectorDefs::fpgaPosition::FRONT_RIGHT)
        .export_values();

    py::enum_<slsDetectorDefs::streamingInterface>(Defs, "streamingInterface",
                                                   py::arithmetic())
        .value("NONE", slsDetectorDefs::streamingInterface::NONE)
        .value("LOW_LATENCY_LINK",
               slsDetectorDefs::streamingInterface::LOW_LATENCY_LINK)
        .value("ETHERNET_10GB",
               slsDetectorDefs::streamingInterface::ETHERNET_10GB)
        .value("ALL", slsDetectorDefs::streamingInterface::ALL)
        .export_values()
        .def(py::self | slsDetectorDefs::streamingInterface())
        .def(py::self & slsDetectorDefs::streamingInterface());

    py::enum_<slsDetectorDefs::vetoAlgorithm>(Defs, "vetoAlgorithm")
        .value("ALG_HITS", slsDetectorDefs::vetoAlgorithm::ALG_HITS)
        .value("ALG_RAW", slsDetectorDefs::vetoAlgorithm::ALG_RAW)
        .export_values();

    py::enum_<slsDetectorDefs::gainMode>(Defs, "gainMode")
        .value("DYNAMIC", slsDetectorDefs::gainMode::DYNAMIC)
        .value("FORCE_SWITCH_G1", slsDetectorDefs::gainMode::FORCE_SWITCH_G1)
        .value("FORCE_SWITCH_G2", slsDetectorDefs::gainMode::FORCE_SWITCH_G2)
        .value("FIX_G1", slsDetectorDefs::gainMode::FIX_G1)
        .value("FIX_G2", slsDetectorDefs::gainMode::FIX_G2)
        .value("FIX_G0", slsDetectorDefs::gainMode::FIX_G0)
        .export_values();

    py::enum_<slsDetectorDefs::polarity>(Defs, "polarity")
        .value("POSITIVE", slsDetectorDefs::polarity::POSITIVE)
        .value("NEGATIVE", slsDetectorDefs::polarity::NEGATIVE)
        .export_values();

    py::enum_<slsDetectorDefs::timingInfoDecoder>(Defs, "timingInfoDecoder")
        .value("SWISSFEL", slsDetectorDefs::timingInfoDecoder::SWISSFEL)
        .value("SHINE", slsDetectorDefs::timingInfoDecoder::SHINE)
        .export_values();

    py::enum_<slsDetectorDefs::collectionMode>(Defs, "collectionMode")
        .value("HOLE", slsDetectorDefs::collectionMode::HOLE)
        .value("ELECTRON", slsDetectorDefs::collectionMode::ELECTRON)
        .export_values();
}
