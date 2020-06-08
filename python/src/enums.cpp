/* WARINING This file is auto generated any edits might be overwritten without
 * warning */

#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls_detector_defs.h"
namespace py = pybind11;
void init_enums(py::module &m) {
    py::class_<slsDetectorDefs> Defs(m, "slsDetectorDefs");
    py::class_<slsDetectorDefs::xy> xy(m, "xy");
    xy.def(py::init());
    xy.def(py::init<int, int>());
    xy.def_readwrite("x", &slsDetectorDefs::xy::x);
    xy.def_readwrite("y", &slsDetectorDefs::xy::y);

    py::enum_<slsDetectorDefs::detectorType>(Defs, "detectorType")
        .value("GET_DETECTOR_TYPE",
               slsDetectorDefs::detectorType::GET_DETECTOR_TYPE)
        .value("GENERIC", slsDetectorDefs::detectorType::GENERIC)
        .value("EIGER", slsDetectorDefs::detectorType::EIGER)
        .value("GOTTHARD", slsDetectorDefs::detectorType::GOTTHARD)
        .value("JUNGFRAU", slsDetectorDefs::detectorType::JUNGFRAU)
        .value("CHIPTESTBOARD", slsDetectorDefs::detectorType::CHIPTESTBOARD)
        .value("MOENCH", slsDetectorDefs::detectorType::MOENCH)
        .value("MYTHEN3", slsDetectorDefs::detectorType::MYTHEN3)
        .value("GOTTHARD2", slsDetectorDefs::detectorType::GOTTHARD2)
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

    py::enum_<slsDetectorDefs::frameDiscardPolicy>(Defs, "frameDiscardPolicy")
        .value("GET_FRAME_DISCARD_POLICY",
               slsDetectorDefs::frameDiscardPolicy::GET_FRAME_DISCARD_POLICY)
        .value("NO_DISCARD", slsDetectorDefs::frameDiscardPolicy::NO_DISCARD)
        .value("DISCARD_EMPTY_FRAMES",
               slsDetectorDefs::frameDiscardPolicy::DISCARD_EMPTY_FRAMES)
        .value("DISCARD_PARTIAL_FRAMES",
               slsDetectorDefs::frameDiscardPolicy::DISCARD_PARTIAL_FRAMES)
        .value("NUM_DISCARD_POLICIES",
               slsDetectorDefs::frameDiscardPolicy::NUM_DISCARD_POLICIES)
        .export_values();

    py::enum_<slsDetectorDefs::fileFormat>(Defs, "fileFormat")
        .value("GET_FILE_FORMAT", slsDetectorDefs::fileFormat::GET_FILE_FORMAT)
        .value("BINARY", slsDetectorDefs::fileFormat::BINARY)
        .value("HDF5", slsDetectorDefs::fileFormat::HDF5)
        .value("NUM_FILE_FORMATS",
               slsDetectorDefs::fileFormat::NUM_FILE_FORMATS)
        .export_values();

    py::enum_<slsDetectorDefs::dimension>(Defs, "dimension")
        .value("X", slsDetectorDefs::dimension::X)
        .value("Y", slsDetectorDefs::dimension::Y)
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
        .value("GET_TIMING_MODE", slsDetectorDefs::timingMode::GET_TIMING_MODE)
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
        .value("VSVP", slsDetectorDefs::dacIndex::VSVP)
        .value("VTR", slsDetectorDefs::dacIndex::VTR)
        .value("VRF", slsDetectorDefs::dacIndex::VRF)
        .value("VRS", slsDetectorDefs::dacIndex::VRS)
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
        .value("VIS", slsDetectorDefs::dacIndex::VIS)
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
        .value("VSHAPER", slsDetectorDefs::dacIndex::VSHAPER)
        .value("VSHAPERNEG", slsDetectorDefs::dacIndex::VSHAPERNEG)
        .value("VIPRE_OUT", slsDetectorDefs::dacIndex::VIPRE_OUT)
        .value("VTH3", slsDetectorDefs::dacIndex::VTH3)
        .value("VTH1", slsDetectorDefs::dacIndex::VTH1)
        .value("VICIN", slsDetectorDefs::dacIndex::VICIN)
        .value("VCAS", slsDetectorDefs::dacIndex::VCAS)
        .value("VPREAMP", slsDetectorDefs::dacIndex::VPREAMP)
        .value("VPL", slsDetectorDefs::dacIndex::VPL)
        .value("VIPRE", slsDetectorDefs::dacIndex::VIPRE)
        .value("VIINSH", slsDetectorDefs::dacIndex::VIINSH)
        .value("VPH", slsDetectorDefs::dacIndex::VPH)
        .value("VTRIM", slsDetectorDefs::dacIndex::VTRIM)
        .value("VDCSH", slsDetectorDefs::dacIndex::VDCSH)
        .value("VBP_COLBUF", slsDetectorDefs::dacIndex::VBP_COLBUF)
        .value("VB_SDA", slsDetectorDefs::dacIndex::VB_SDA)
        .value("VCASC_SFP", slsDetectorDefs::dacIndex::VCASC_SFP)
        .value("VIPRE_CDS", slsDetectorDefs::dacIndex::VIPRE_CDS)
        .value("IBIAS_SFP", slsDetectorDefs::dacIndex::IBIAS_SFP)
        .value("ADC_VPP", slsDetectorDefs::dacIndex::ADC_VPP)
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
        .value("GET_SETTINGS", slsDetectorDefs::detectorSettings::GET_SETTINGS)
        .value("STANDARD", slsDetectorDefs::detectorSettings::STANDARD)
        .value("FAST", slsDetectorDefs::detectorSettings::FAST)
        .value("HIGHGAIN", slsDetectorDefs::detectorSettings::HIGHGAIN)
        .value("DYNAMICGAIN", slsDetectorDefs::detectorSettings::DYNAMICGAIN)
        .value("LOWGAIN", slsDetectorDefs::detectorSettings::LOWGAIN)
        .value("MEDIUMGAIN", slsDetectorDefs::detectorSettings::MEDIUMGAIN)
        .value("VERYHIGHGAIN", slsDetectorDefs::detectorSettings::VERYHIGHGAIN)
        .value("DYNAMICHG0", slsDetectorDefs::detectorSettings::DYNAMICHG0)
        .value("FIXGAIN1", slsDetectorDefs::detectorSettings::FIXGAIN1)
        .value("FIXGAIN2", slsDetectorDefs::detectorSettings::FIXGAIN2)
        .value("FORCESWITCHG1",
               slsDetectorDefs::detectorSettings::FORCESWITCHG1)
        .value("FORCESWITCHG2",
               slsDetectorDefs::detectorSettings::FORCESWITCHG2)
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
        .value("UNDEFINED", slsDetectorDefs::detectorSettings::UNDEFINED)
        .value("UNINITIALIZED",
               slsDetectorDefs::detectorSettings::UNINITIALIZED)
        .export_values();

    py::enum_<slsDetectorDefs::clockIndex>(Defs, "clockIndex")
        .value("ADC_CLOCK", slsDetectorDefs::clockIndex::ADC_CLOCK)
        .value(" DBIT_CLOCK", slsDetectorDefs::clockIndex::DBIT_CLOCK)
        .value(" RUN_CLOCK", slsDetectorDefs::clockIndex::RUN_CLOCK)
        .value(" SYNC_CLOCK", slsDetectorDefs::clockIndex::SYNC_CLOCK)
        .export_values();

    py::enum_<slsDetectorDefs::readoutMode>(Defs, "readoutMode")
        .value("ANALOG_ONLY", slsDetectorDefs::readoutMode::ANALOG_ONLY)
        .value(" DIGITAL_ONLY", slsDetectorDefs::readoutMode::DIGITAL_ONLY)
        .value(" ANALOG_AND_DIGITAL",
               slsDetectorDefs::readoutMode::ANALOG_AND_DIGITAL)
        .export_values();

    py::enum_<slsDetectorDefs::speedLevel>(Defs, "speedLevel")
        .value("FULL_SPEED", slsDetectorDefs::speedLevel::FULL_SPEED)
        .value(" HALF_SPEED", slsDetectorDefs::speedLevel::HALF_SPEED)
        .value(" QUARTER_SPEED", slsDetectorDefs::speedLevel::QUARTER_SPEED)
        .export_values();

    py::enum_<slsDetectorDefs::portType>(Defs, "portType")
        .value("CONTROL_PORT", slsDetectorDefs::portType::CONTROL_PORT)
        .value("STOP_PORT", slsDetectorDefs::portType::STOP_PORT)
        .value("DATA_PORT", slsDetectorDefs::portType::DATA_PORT)
        .export_values();

    py::enum_<slsDetectorDefs::masterFlags>(Defs, "masterFlags")
        .value("GET_MASTER", slsDetectorDefs::masterFlags::GET_MASTER)
        .value("NO_MASTER", slsDetectorDefs::masterFlags::NO_MASTER)
        .value("IS_MASTER", slsDetectorDefs::masterFlags::IS_MASTER)
        .value("IS_SLAVE", slsDetectorDefs::masterFlags::IS_SLAVE)
        .export_values();

    py::enum_<slsDetectorDefs::frameModeType>(Defs, "frameModeType")
        .value("GET_FRAME_MODE", slsDetectorDefs::frameModeType::GET_FRAME_MODE)
        .value("PEDESTAL", slsDetectorDefs::frameModeType::PEDESTAL)
        .value("NEW_PEDESTAL", slsDetectorDefs::frameModeType::NEW_PEDESTAL)
        .value("FLATFIELD", slsDetectorDefs::frameModeType::FLATFIELD)
        .value("NEW_FLATFIELD", slsDetectorDefs::frameModeType::NEW_FLATFIELD)
        .export_values();

    py::enum_<slsDetectorDefs::detectorModeType>(Defs, "detectorModeType")
        .value("GET_DETECTOR_MODE",
               slsDetectorDefs::detectorModeType::GET_DETECTOR_MODE)
        .value("COUNTING", slsDetectorDefs::detectorModeType::COUNTING)
        .value("INTERPOLATING",
               slsDetectorDefs::detectorModeType::INTERPOLATING)
        .value("ANALOG", slsDetectorDefs::detectorModeType::ANALOG)
        .export_values();

    py::enum_<slsDetectorDefs::burstMode>(Defs, "burstMode")
        .value("BURST_OFF", slsDetectorDefs::burstMode::BURST_OFF)
        .value("BURST_INTERNAL", slsDetectorDefs::burstMode::BURST_INTERNAL)
        .value("BURST_EXTERNAL", slsDetectorDefs::burstMode::BURST_EXTERNAL)
        .value("NUM_BURST_MODES", slsDetectorDefs::burstMode::NUM_BURST_MODES)
        .export_values();

    py::enum_<slsDetectorDefs::timingSourceType>(Defs, "timingSourceType")
        .value("TIMING_INTERNAL",
               slsDetectorDefs::timingSourceType::TIMING_INTERNAL)
        .value(" TIMING_EXTERNAL",
               slsDetectorDefs::timingSourceType::TIMING_EXTERNAL)
        .export_values();
}
