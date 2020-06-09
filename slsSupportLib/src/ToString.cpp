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

std::string ToString(const defs::dacIndex s) {
    switch (s) {
    case defs::DAC_0:
        return std::string("dac 0");
    case defs::DAC_1:
        return std::string("dac 1");
    case defs::DAC_2:
        return std::string("dac 2");
    case defs::DAC_3:
        return std::string("dac 3");
    case defs::DAC_4:
        return std::string("dac 4");
    case defs::DAC_5:
        return std::string("dac 5");
    case defs::DAC_6:
        return std::string("dac 6");
    case defs::DAC_7:
        return std::string("dac 7");
    case defs::DAC_8:
        return std::string("dac 8");
    case defs::DAC_9:
        return std::string("dac 9");
    case defs::DAC_10:
        return std::string("dac 10");
    case defs::DAC_11:
        return std::string("dac 11");
    case defs::DAC_12:
        return std::string("dac 12");
    case defs::DAC_13:
        return std::string("dac 13");
    case defs::DAC_14:
        return std::string("dac 14");
    case defs::DAC_15:
        return std::string("dac 15");
    case defs::DAC_16:
        return std::string("dac 16");
    case defs::DAC_17:
        return std::string("dac 17");
    case defs::VSVP:
        return std::string("vsvp");
    case defs::VTRIM:
        return std::string("vtrim");
    case defs::VRPREAMP:
        return std::string("vrpreamp");
    case defs::VRSHAPER:
        return std::string("vrshaper");
    case defs::VSVN:
        return std::string("vsvn");
    case defs::VTGSTV:
        return std::string("vtgstv");
    case defs::VCMP_LL:
        return std::string("vcmp_ll");
    case defs::VCMP_LR:
        return std::string("vcmp_lr");
    case defs::VCAL:
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
    case defs::VISHAPER:
        return std::string("vishaper");
    case defs::VTHRESHOLD:
        return std::string("vthreshold");
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
    case defs::VCASSH:
        return std::string("vcassh");
    case defs::VTH2:
        return std::string("vth2");
    case defs::VRSHAPER_N:
        return std::string("vrshaper_n");
    case defs::VIPRE_OUT:
        return std::string("vipre_out");
    case defs::VTH3:
        return std::string("vth3");
    case defs::VTH1:
        return std::string("vth1");
    case defs::VICIN:
        return std::string("vicin");
    case defs::VCAS:
        return std::string("vcas");
    case defs::VCAL_N:
        return std::string("vcal_n");
    case defs::VIPRE:
        return std::string("vipre");
    case defs::VCAL_P:
        return std::string("vcal_p");
    case defs::VDCSH:
        return std::string("vdcsh");
    case defs::VBP_COLBUF:
        return std::string("vbp_colbuf");
    case defs::VB_SDA:
        return std::string("vb_sda");
    case defs::VCASC_SFP:
        return std::string("vcasc_sfp");
    case defs::VIPRE_CDS:
        return std::string("vipre_cds");
    case defs::IBIAS_SFP:
        return std::string("ibias_sfp");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const std::vector<defs::dacIndex> &vec) {
    std::ostringstream os;
    os << '[';
    if (!vec.empty()) {
        auto it = vec.begin();
        os << ToString(*it++);
        while (it != vec.end())
            os << ", " << ToString(*it++);
    }
    os << ']';
    return os.str();
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

template <> defs::dacIndex StringTo(const std::string &s) {
    if (s == "dac 0")
        return defs::DAC_0;
    if (s == "dac 1")
        return defs::DAC_1;
    if (s == "dac 2")
        return defs::DAC_2;
    if (s == "dac 3")
        return defs::DAC_3;
    if (s == "dac 4")
        return defs::DAC_4;
    if (s == "dac 5")
        return defs::DAC_5;
    if (s == "dac 6")
        return defs::DAC_6;
    if (s == "dac 7")
        return defs::DAC_7;
    if (s == "dac 8")
        return defs::DAC_8;
    if (s == "dac 9")
        return defs::DAC_9;
    if (s == "dac 10")
        return defs::DAC_10;
    if (s == "dac 11")
        return defs::DAC_11;
    if (s == "dac 12")
        return defs::DAC_12;
    if (s == "dac 13")
        return defs::DAC_13;
    if (s == "dac 14")
        return defs::DAC_14;
    if (s == "dac 15")
        return defs::DAC_15;
    if (s == "dac 16")
        return defs::DAC_16;
    if (s == "dac 17")
        return defs::DAC_17;
    if (s == "vsvp")
        return defs::VSVP;
    if (s == "vtrim")
        return defs::VTRIM;
    if (s == "vrpreamp")
        return defs::VRPREAMP;
    if (s == "vrshaper")
        return defs::VRSHAPER;
    if (s == "vsvn")
        return defs::VSVN;
    if (s == "vtgstv")
        return defs::VTGSTV;
    if (s == "vcmp_ll")
        return defs::VCMP_LL;
    if (s == "vcmp_lr")
        return defs::VCMP_LR;
    if (s == "vcal")
        return defs::VCAL;
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
    if (s == "vishaper")
        return defs::VISHAPER;
    if (s == "vthreshold")
        return defs::VTHRESHOLD;
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
    if (s == "vcassh")
        return defs::VCASSH;
    if (s == "vth2")
        return defs::VTH2;
    if (s == "vrshaper_n")
        return defs::VRSHAPER_N;
    if (s == "vipre_out")
        return defs::VIPRE_OUT;
    if (s == "vth3")
        return defs::VTH3;
    if (s == "vth1")
        return defs::VTH1;
    if (s == "vicin")
        return defs::VICIN;
    if (s == "vcas")
        return defs::VCAS;
    if (s == "vcal_n")
        return defs::VCAL_N;
    if (s == "vipre")
        return defs::VIPRE;
    if (s == "vcal_p")
        return defs::VCAL_P;
    if (s == "vdcsh")
        return defs::VDCSH;
    if (s == "vbp_colbuf")
        return defs::VBP_COLBUF;
    if (s == "vb_sda")
        return defs::VB_SDA;
    if (s == "vcasc_sfp")
        return defs::VCASC_SFP;
    if (s == "vipre_cds")
        return defs::VIPRE_CDS;
    if (s == "ibias_sfp")
        return defs::IBIAS_SFP;
    throw sls::RuntimeError("Unknown dac Index " + s);
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