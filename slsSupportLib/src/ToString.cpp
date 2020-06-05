#include "ToString.h"

namespace sls {

std::string ToString(const slsDetectorDefs::ROI &roi) {
    std::ostringstream oss;
    oss << '[' << roi.xmin << ", " << roi.xmax << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &os, const slsDetectorDefs::ROI &roi) {
    return os << ToString(roi);
}

std::string ToString(const defs::runStatus s) {
    switch (s) {
    case defs::ERROR:
        return std::string("error");
    case defs::WAITING:
        return std::string("waiting");
    case defs::RUNNING:
        return std::string("running");
    case defs::TRANSMITTING:
        return std::string("transmitting");
    case defs::RUN_FINISHED:
        return std::string("finished");
    case defs::STOPPED:
        return std::string("stopped");
    default:
        return std::string("idle");
    }
}

std::string ToString(const defs::detectorType s) {
    switch (s) {
    case defs::EIGER:
        return std::string("Eiger");
    case defs::GOTTHARD:
        return std::string("Gotthard");
    case defs::JUNGFRAU:
        return std::string("Jungfrau");
    case defs::CHIPTESTBOARD:
        return std::string("ChipTestBoard");
    case defs::MOENCH:
        return std::string("Moench");
    case defs::MYTHEN3:
        return std::string("Mythen3");
    case defs::GOTTHARD2:
        return std::string("Gotthard2");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::detectorSettings s) {
    switch (s) {
    case defs::STANDARD:
        return std::string("standard");
    case defs::FAST:
        return std::string("fast");
    case defs::HIGHGAIN:
        return std::string("highgain");
    case defs::DYNAMICGAIN:
        return std::string("dynamicgain");
    case defs::LOWGAIN:
        return std::string("lowgain");
    case defs::MEDIUMGAIN:
        return std::string("mediumgain");
    case defs::VERYHIGHGAIN:
        return std::string("veryhighgain");
    case defs::DYNAMICHG0:
        return std::string("dynamichg0");
    case defs::FIXGAIN1:
        return std::string("fixgain1");
    case defs::FIXGAIN2:
        return std::string("fixgain2");
    case defs::FORCESWITCHG1:
        return std::string("forceswitchg1");
    case defs::FORCESWITCHG2:
        return std::string("forceswitchg2");
    case defs::VERYLOWGAIN:
        return std::string("verylowgain");
    case defs::G1_HIGHGAIN:
        return std::string("g1_hg");
    case defs::G1_LOWGAIN:
        return std::string("g1_lg");
    case defs::G2_HIGHCAP_HIGHGAIN:
        return std::string("g2_hc_hg");
    case defs::G2_HIGHCAP_LOWGAIN:
        return std::string("g2_hc_lg");
    case defs::G2_LOWCAP_HIGHGAIN:
        return std::string("g2_lc_hg");
    case defs::G2_LOWCAP_LOWGAIN:
        return std::string("g2_lc_lg");
    case defs::G4_HIGHGAIN:
        return std::string("g4_hg");
    case defs::G4_LOWGAIN:
        return std::string("g4_lg");
    case defs::UNDEFINED:
        return std::string("undefined");
    case defs::UNINITIALIZED:
        return std::string("uninitialized");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::speedLevel s) {
    switch (s) {
    case defs::FULL_SPEED:
        return std::string("full_speed");
    case defs::HALF_SPEED:
        return std::string("half_speed");
    case defs::QUARTER_SPEED:
        return std::string("quarter_speed");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::timingMode s) {
    switch (s) {
    case defs::AUTO_TIMING:
        return std::string("auto");
    case defs::TRIGGER_EXPOSURE:
        return std::string("trigger");
    case defs::GATED:
        return std::string("gating");
    case defs::BURST_TRIGGER:
        return std::string("burst_trigger");
    case defs::TRIGGER_GATED:
        return std::string("trigger_gating");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::frameDiscardPolicy s) {
    switch (s) {
    case defs::NO_DISCARD:
        return std::string("nodiscard");
    case defs::DISCARD_EMPTY_FRAMES:
        return std::string("discardempty");
    case defs::DISCARD_PARTIAL_FRAMES:
        return std::string("discardpartial");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::fileFormat s) {
    switch (s) {
    case defs::HDF5:
        return std::string("hdf5");
    case defs::BINARY:
        return std::string("binary");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::externalSignalFlag s) {
    switch (s) {
    case defs::TRIGGER_IN_RISING_EDGE:
        return std::string("trigger_in_rising_edge");
    case defs::TRIGGER_IN_FALLING_EDGE:
        return std::string("trigger_in_falling_edge");
    case defs::INVERSION_ON:
        return std::string("inversion_on");
    case defs::INVERSION_OFF:
        return std::string("inversion_off");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::readoutMode s) {
    switch (s) {
    case defs::ANALOG_ONLY:
        return std::string("analog");
    case defs::DIGITAL_ONLY:
        return std::string("digital");
    case defs::ANALOG_AND_DIGITAL:
        return std::string("analog_digital");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::frameModeType s) {
    switch (s) {
    case defs::PEDESTAL:
        return std::string("pedestal");
    case defs::NEW_PEDESTAL:
        return std::string("newpedestal");
    case defs::FLATFIELD:
        return std::string("flatfield");
    case defs::NEW_FLATFIELD:
        return std::string("newflatfield");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::detectorModeType s) {
    switch (s) {
    case defs::COUNTING:
        return std::string("counting");
    case defs::INTERPOLATING:
        return std::string("interpolating");
    case defs::ANALOG:
        return std::string("analog");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::dacIndex s, const defs::detectorType t) {
    switch (t) {
    case defs::EIGER:
        switch (s) {
        case defs::SVP:
            return std::string("vsvp");
        case defs::VTR:
            return std::string("vtr");
        case defs::VRF:
            return std::string("vrf");
        case defs::VRS:
            return std::string("vrs");
        case defs::SVN:
            return std::string("vsvn");
        case defs::VTGSTV:
            return std::string("vtgstv");
        case defs::VCMP_LL:
            return std::string("vcmp_ll");
        case defs::VCMP_LR:
            return std::string("vcmp_lr");
        case defs::CAL:
            return std::string("vcal");
        case defs::VCMP_RL:
            return std::string("vcmp_rl");
        case defs::RXB_RB:
            return std::string("rxb_rb");
        case defs::RXB_LB:
            return std::string("rxb_lb");
        case defs::VCMP_RR:
            return std::string("vcmp_rr");
        case defs::VCP:
            return std::string("vcp");
        case defs::VCN:
            return std::string("vcn");
        case defs::VIS:
            return std::string("vis");
        case defs::THRESHOLD:
            return std::string("vthreshold");
        default:
            return std::string("Unknown");
        }
    case defs::GOTTHARD:
        switch (s) {
        case defs::VREF_DS:
            return std::string("vref_ds");
        case defs::VCASCN_PB:
            return std::string("vcascn_pb");
        case defs::VCASCP_PB:
            return std::string("vcascp_pb");
        case defs::VOUT_CM:
            return std::string("vout_cm");
        case defs::VCASC_OUT:
            return std::string("vcasc_out");
        case defs::VIN_CM:
            return std::string("vin_cm");
        case defs::VREF_COMP:
            return std::string("vref_comp");
        case defs::IB_TESTC:
            return std::string("ib_test_c");
        default:
            return std::string("Unknown");
        }
    case defs::JUNGFRAU:
        switch (s) {
        case defs::VB_COMP:
            return std::string("vb_comp");
        case defs::VDD_PROT:
            return std::string("vdd_prot");
        case defs::VIN_COM:
            return std::string("vin_com");
        case defs::VREF_PRECH:
            return std::string("vref_prech");
        case defs::VB_PIXBUF:
            return std::string("vb_pixbuf");
        case defs::VB_DS:
            return std::string("vb_ds");
        case defs::VREF_DS:
            return std::string("vref_ds");
        case defs::VREF_COMP:
            return std::string("vref_comp");
        default:
            return std::string("Unknown");
        }
    case defs::GOTTHARD2:
        switch (s) {
        case defs::VREF_H_ADC:
            return std::string("vref_h_adc");
        case defs::VB_COMP_FE:
            return std::string("vb_comp_fe");
        case defs::VB_COMP_ADC:
            return std::string("vb_comp_adc");
        case defs::VCOM_CDS:
            return std::string("vcom_cds");
        case defs::VREF_RSTORE:
            return std::string("vref_rstore");
        case defs::VB_OPA_1ST:
            return std::string("vb_opa_1st");
        case defs::VREF_COMP_FE:
            return std::string("vref_comp_fe");
        case defs::VCOM_ADC1:
            return std::string("vcom_adc1");
        case defs::VREF_PRECH:
            return std::string("vref_prech");
        case defs::VREF_L_ADC:
            return std::string("vref_l_adc");
        case defs::VREF_CDS:
            return std::string("vref_cds");
        case defs::VB_CS:
            return std::string("vb_cs");
        case defs::VB_OPA_FD:
            return std::string("vb_opa_fd");
        case defs::VCOM_ADC2:
            return std::string("vcom_adc2");
        default:
            return std::string("Unknown");
        }
    case defs::MYTHEN3:
        switch (s) {
        case defs::CASSH:
            return std::string("vcassh");
        case defs::VTH2:
            return std::string("vth2");
        case defs::SHAPER1:
            return std::string("vshaper");
        case defs::SHAPER2:
            return std::string("vshaperneg");
        case defs::VIPRE_OUT:
            return std::string("vipre_out");
        case defs::VTH3:
            return std::string("vth3");
        case defs::THRESHOLD:
            return std::string("vth1");
        case defs::VICIN:
            return std::string("vicin");
        case defs::CAS:
            return std::string("vcas");
        case defs::PREAMP:
            return std::string("vpreamp");
        case defs::VPL:
            return std::string("vpl");
        case defs::VIPRE:
            return std::string("vipre");
        case defs::VIINSH:
            return std::string("viinsh");
        case defs::CALIBRATION_PULSE:
            return std::string("vph");
        case defs::TRIMBIT_SIZE:
            return std::string("vtrim");
        case defs::VDCSH:
            return std::string("vdcsh");
        default:
            return std::string("Unknown");
        }
    case defs::MOENCH:
        switch (s) {
        case defs::VBP_COLBUF:
            return std::string("vbp_colbuf");
        case defs::VIPRE:
            return std::string("vipre");
        case defs::VIN_CM:
            return std::string("vin_cm");
        case defs::VB_SDA:
            return std::string("vb_sda");
        case defs::VCASC_SFP:
            return std::string("vcasc_sfp");
        case defs::VOUT_CM:
            return std::string("vout_cm");
        case defs::VIPRE_CDS:
            return std::string("vipre_cds");
        case defs::IBIAS_SFP:
            return std::string("ibias_sfp");
        default:
            return std::string("Unknown");
        }
    case defs::CHIPTESTBOARD:
        if (s < 0 || s > 17) {
            return std::string("Unknown");
        } else {
            return std::string("dac ") + std::to_string(static_cast<int>(s));
        }
    default:
        return std::string("Unknown detector");
    }
}

std::string ToString(const defs::burstMode s) {
    switch (s) {
    case defs::BURST_OFF:
        return std::string("off");
    case defs::BURST_INTERNAL:
        return std::string("internal");
    case defs::BURST_EXTERNAL:
        return std::string("external");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::timingSourceType s) {
    switch (s) {
    case defs::TIMING_INTERNAL:
        return std::string("internal");
    case defs::TIMING_EXTERNAL:
        return std::string("external");
    default:
        return std::string("Unknown");
    }
}

const std::string &ToString(const std::string &s) { return s; }

template <> defs::detectorType StringTo(const std::string &s) {
    if (s == "Eiger")
        return defs::EIGER;
    if (s == "Gotthard")
        return defs::GOTTHARD;
    if (s == "Jungfrau")
        return defs::JUNGFRAU;
    if (s == "ChipTestBoard")
        return defs::CHIPTESTBOARD;
    if (s == "Moench")
        return defs::MOENCH;
    if (s == "Mythen3")
        return defs::MYTHEN3;
    if (s == "Gotthard2")
        return defs::GOTTHARD2;
    throw sls::RuntimeError("Unknown detector type " + s);
}

template <> defs::detectorSettings StringTo(const std::string &s) {
    if (s == "standard")
        return defs::STANDARD;
    if (s == "fast")
        return defs::FAST;
    if (s == "highgain")
        return defs::HIGHGAIN;
    if (s == "dynamicgain")
        return defs::DYNAMICGAIN;
    if (s == "lowgain")
        return defs::LOWGAIN;
    if (s == "mediumgain")
        return defs::MEDIUMGAIN;
    if (s == "veryhighgain")
        return defs::VERYHIGHGAIN;
    if (s == "dynamichg0")
        return defs::DYNAMICHG0;
    if (s == "fixgain1")
        return defs::FIXGAIN1;
    if (s == "fixgain2")
        return defs::FIXGAIN2;
    if (s == "forceswitchg1")
        return defs::FORCESWITCHG1;
    if (s == "forceswitchg2")
        return defs::FORCESWITCHG2;
    if (s == "verylowgain")
        return defs::VERYLOWGAIN;
    if (s == "g1_hg")
        return defs::G1_HIGHGAIN;
    if (s == "g1_lg")
        return defs::G1_LOWGAIN;
    if (s == "g2_hc_hg")
        return defs::G2_HIGHCAP_HIGHGAIN;
    if (s == "g2_hc_lg")
        return defs::G2_HIGHCAP_LOWGAIN;
    if (s == "g2_lc_hg")
        return defs::G2_LOWCAP_HIGHGAIN;
    if (s == "g2_lc_lg")
        return defs::G2_LOWCAP_LOWGAIN;
    if (s == "g4_hg")
        return defs::G4_HIGHGAIN;
    if (s == "g4_lg")
        return defs::G4_LOWGAIN;
    throw sls::RuntimeError("Unknown setting " + s);
}

template <> defs::speedLevel StringTo(const std::string &s) {
    if (s == "full_speed")
        return defs::FULL_SPEED;
    if (s == "half_speed")
        return defs::HALF_SPEED;
    if (s == "quarter_speed")
        return defs::QUARTER_SPEED;
    throw sls::RuntimeError("Unknown speed " + s);
}

template <> defs::timingMode StringTo(const std::string &s) {
    if (s == "auto")
        return defs::AUTO_TIMING;
    if (s == "trigger")
        return defs::TRIGGER_EXPOSURE;
    if (s == "gating")
        return defs::GATED;
    if (s == "burst_trigger")
        return defs::BURST_TRIGGER;
    if (s == "trigger_gating")
        return defs::TRIGGER_GATED;
    throw sls::RuntimeError("Unknown timing mode " + s);
}

template <> defs::frameDiscardPolicy StringTo(const std::string &s) {
    if (s == "nodiscard")
        return defs::NO_DISCARD;
    if (s == "discardempty")
        return defs::DISCARD_EMPTY_FRAMES;
    if (s == "discardpartial")
        return defs::DISCARD_PARTIAL_FRAMES;
    throw sls::RuntimeError("Unknown frame discard policy " + s);
}

template <> defs::fileFormat StringTo(const std::string &s) {
    if (s == "hdf5")
        return defs::HDF5;
    if (s == "binary")
        return defs::BINARY;
    throw sls::RuntimeError("Unknown file format " + s);
}

template <> defs::externalSignalFlag StringTo(const std::string &s) {
    if (s == "trigger_in_rising_edge")
        return defs::TRIGGER_IN_RISING_EDGE;
    if (s == "trigger_in_falling_edge")
        return defs::TRIGGER_IN_FALLING_EDGE;
    if (s == "inversion_on")
        return defs::INVERSION_ON;
    if (s == "inversion_off")
        return defs::INVERSION_OFF;
    throw sls::RuntimeError("Unknown external signal flag " + s);
}

template <> defs::readoutMode StringTo(const std::string &s) {
    if (s == "analog")
        return defs::ANALOG_ONLY;
    if (s == "digital")
        return defs::DIGITAL_ONLY;
    if (s == "analog_digital")
        return defs::ANALOG_AND_DIGITAL;
    throw sls::RuntimeError("Unknown readout mode " + s);
}

template <> defs::frameModeType StringTo(const std::string &s) {
    if (s == "pedestal")
        return defs::PEDESTAL;
    if (s == "newpedestal")
        return defs::NEW_PEDESTAL;
    if (s == "flatfield")
        return defs::FLATFIELD;
    if (s == "newflatfield")
        return defs::NEW_FLATFIELD;
    throw sls::RuntimeError("Unknown frame mode " + s);
}

template <> defs::detectorModeType StringTo(const std::string &s) {
    if (s == "counting")
        return defs::COUNTING;
    if (s == "interpolating")
        return defs::INTERPOLATING;
    if (s == "analog")
        return defs::ANALOG;
    throw sls::RuntimeError("Unknown detector mode " + s);
}

template <>
defs::dacIndex StringTo(const std::string &s, const defs::detectorType t) {
    int index = 0;
    switch (t) {
    case defs::EIGER:
        if (s == "vsvp")
            return defs::SVP;
        if (s == "vtr")
            return defs::VTR;
        if (s == "vrf")
            return defs::VRF;
        if (s == "vrs")
            return defs::VRS;
        if (s == "vsvn")
            return defs::SVN;
        if (s == "vtgstv")
            return defs::VTGSTV;
        if (s == "vcmp_ll")
            return defs::VCMP_LL;
        if (s == "vcmp_lr")
            return defs::VCMP_LR;
        if (s == "vcal")
            return defs::CAL;
        if (s == "vcmp_rl")
            return defs::VCMP_RL;
        if (s == "rxb_rb")
            return defs::RXB_RB;
        if (s == "rxb_lb")
            return defs::RXB_LB;
        if (s == "vcmp_rr")
            return defs::VCMP_RR;
        if (s == "vcp")
            return defs::VCP;
        if (s == "vcn")
            return defs::VCN;
        if (s == "vis")
            return defs::VIS;
        if (s == "vthreshold")
            return defs::THRESHOLD;
        throw sls::RuntimeError("Unknown dac Index " + s);
    case defs::GOTTHARD:
        if (s == "vref_ds")
            return defs::VREF_DS;
        if (s == "vcascn_pb")
            return defs::VCASCN_PB;
        if (s == "vcascp_pb")
            return defs::VCASCP_PB;
        if (s == "vout_cm")
            return defs::VOUT_CM;
        if (s == "vcasc_out")
            return defs::VCASC_OUT;
        if (s == "vin_cm")
            return defs::VIN_CM;
        if (s == "vref_comp")
            return defs::VREF_COMP;
        if (s == "ib_test_c")
            return defs::IB_TESTC;
        throw sls::RuntimeError("Unknown dac Index " + s);
    case defs::JUNGFRAU:
        if (s == "vb_comp")
            return defs::VB_COMP;
        if (s == "vdd_prot")
            return defs::VDD_PROT;
        if (s == "vin_com")
            return defs::VIN_COM;
        if (s == "vref_prech")
            return defs::VREF_PRECH;
        if (s == "vb_pixbuf")
            return defs::VB_PIXBUF;
        if (s == "vb_ds")
            return defs::VB_DS;
        if (s == "vref_ds")
            return defs::VREF_DS;
        if (s == "vref_comp")
            return defs::VREF_COMP;
        throw sls::RuntimeError("Unknown dac Index " + s);
    case defs::GOTTHARD2:
        if (s == "vref_h_adc")
            return defs::VREF_H_ADC;
        if (s == "vb_comp_fe")
            return defs::VB_COMP_FE;
        if (s == "vb_comp_adc")
            return defs::VB_COMP_ADC;
        if (s == "vcom_cds")
            return defs::VCOM_CDS;
        if (s == "vref_rstore")
            return defs::VREF_RSTORE;
        if (s == "vb_opa_1st")
            return defs::VB_OPA_1ST;
        if (s == "vref_comp_fe")
            return defs::VREF_COMP_FE;
        if (s == "vcom_adc1")
            return defs::VCOM_ADC1;
        if (s == "vref_prech")
            return defs::VREF_PRECH;
        if (s == "vref_l_adc")
            return defs::VREF_L_ADC;
        if (s == "vref_cds")
            return defs::VREF_CDS;
        if (s == "vb_cs")
            return defs::VB_CS;
        if (s == "vb_opa_fd")
            return defs::VB_OPA_FD;
        if (s == "vcom_adc2")
            return defs::VCOM_ADC2;
        throw sls::RuntimeError("Unknown dac Index " + s);
    case defs::MYTHEN3:
        if (s == "vcassh")
            return defs::CASSH;
        if (s == "vth2")
            return defs::VTH2;
        if (s == "vshaper")
            return defs::SHAPER1;
        if (s == "vshaperneg")
            return defs::SHAPER2;
        if (s == "vipre_out")
            return defs::VIPRE_OUT;
        if (s == "vth3")
            return defs::VTH3;
        if (s == "vth1")
            return defs::THRESHOLD;
        if (s == "vicin")
            return defs::VICIN;
        if (s == "vcas")
            return defs::CAS;
        if (s == "vpreamp")
            return defs::PREAMP;
        if (s == "vpl")
            return defs::VPL;
        if (s == "vipre")
            return defs::VIPRE;
        if (s == "viinsh")
            return defs::VIINSH;
        if (s == "vph")
            return defs::CALIBRATION_PULSE;
        if (s == "vtrim")
            return defs::TRIMBIT_SIZE;
        if (s == "vdcsh")
            return defs::VDCSH;
        throw sls::RuntimeError("Unknown dac Index " + s);
    case defs::MOENCH:
        if (s == "vbp_colbuf")
            return defs::VBP_COLBUF;
        if (s == "vipre")
            return defs::VIPRE;
        if (s == "vin_cm")
            return defs::VIN_CM;
        if (s == "vb_sda")
            return defs::VB_SDA;
        if (s == "vcasc_sfp")
            return defs::VCASC_SFP;
        if (s == "vout_cm")
            return defs::VOUT_CM;
        if (s == "vipre_cds")
            return defs::VIPRE_CDS;
        if (s == "ibias_sfp")
            return defs::IBIAS_SFP;
        throw sls::RuntimeError("Unknown dac Index " + s);
    case defs::CHIPTESTBOARD:
        if (sscanf(s.c_str(), "dac %d", &index) != 1) {
            throw sls::RuntimeError("Unknown dac Index " + s);
        }
        if (index < 0 || index > 17) {
            throw sls::RuntimeError("Unknown dac Index " + s);
        } else {
            return static_cast<defs::dacIndex>(index);
        }
    default:
        throw sls::RuntimeError("Unknown detector");
    }
}

template <> defs::burstMode StringTo(const std::string &s) {
    if (s == "off")
        return defs::BURST_OFF;
    if (s == "internal")
        return defs::BURST_INTERNAL;
    if (s == "external")
        return defs::BURST_EXTERNAL;
    throw sls::RuntimeError("Unknown burst mode " + s);
}

template <> defs::timingSourceType StringTo(const std::string &s) {
    if (s == "internal")
        return defs::TIMING_INTERNAL;
    if (s == "external")
        return defs::TIMING_EXTERNAL;
    throw sls::RuntimeError("Unknown timing source type " + s);
}

template <> uint32_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stoul(s, nullptr, base);
}

template <> uint64_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stoull(s, nullptr, base);
}

template <> int StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stoi(s, nullptr, base);
}

template <> int64_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stol(s, nullptr, base);
}

} // namespace sls