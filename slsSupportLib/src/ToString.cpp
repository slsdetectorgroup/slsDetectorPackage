// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/ToString.h"
#include "sls/network_utils.h"

namespace sls {

std::string ToString(const slsDetectorDefs::xy &coord) {
    std::ostringstream oss;
    oss << '[' << coord.x << ", " << coord.y << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &os, const slsDetectorDefs::xy &coord) {
    return os << ToString(coord);
}

std::string ToString(const slsDetectorDefs::ROI &roi) {
    std::ostringstream oss;
    oss << '[' << roi.xmin << ", " << roi.xmax;
    if (roi.ymin != -1 || roi.ymax != -1) {
        oss << ", " << roi.ymin << ", " << roi.ymax;
    }
    oss << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &os, const slsDetectorDefs::ROI &roi) {
    return os << ToString(roi);
}

std::string ToString(const slsDetectorDefs::rxParameters &r) {
    std::ostringstream oss;
    oss << '[' << "detType:" << r.detType << std::endl
        << "numberOfModule.x:" << r.numberOfModule.x << std::endl
        << "numberOfModule.y:" << r.numberOfModule.y << std::endl
        << "moduleIndex:" << r.moduleIndex << std::endl
        << "hostname:" << r.hostname << std::endl
        << "udpInterfaces:" << r.udpInterfaces << std::endl
        << "udp_dstport:" << r.udp_dstport << std::endl
        << "udp_dstip:" << IpAddr(r.udp_dstip) << std::endl
        << "udp_dstmac:" << MacAddr(r.udp_dstmac) << std::endl
        << "udp_dstport2:" << r.udp_dstport2 << std::endl
        << "udp_dstip2:" << IpAddr(r.udp_dstip2) << std::endl
        << "udp_dstmac2:" << MacAddr(r.udp_dstmac2) << std::endl
        << "frames:" << r.frames << std::endl
        << "triggers:" << r.triggers << std::endl
        << "bursts:" << r.bursts << std::endl
        << "additionalStorageCells:" << r.additionalStorageCells << std::endl
        << "analogSamples:" << r.analogSamples << std::endl
        << "digitalSamples:" << r.digitalSamples << std::endl
        << "expTime:" << ToString(std::chrono::nanoseconds(r.expTimeNs))
        << std::endl
        << "period:" << ToString(std::chrono::nanoseconds(r.periodNs))
        << std::endl
        << "subExpTime:" << ToString(std::chrono::nanoseconds(r.subExpTimeNs))
        << std::endl
        << "subDeadTime:" << ToString(std::chrono::nanoseconds(r.subDeadTimeNs))
        << std::endl
        << "activate:" << r.activate << std::endl
        << "leftDataStream:" << r.dataStreamLeft << std::endl
        << "rightDataStream:" << r.dataStreamRight << std::endl
        << "quad:" << r.quad << std::endl
        << "readNRows:" << r.readNRows << std::endl
        << "thresholdEnergyeV:" << ToString(r.thresholdEnergyeV) << std::endl
        << "dynamicRange:" << r.dynamicRange << std::endl
        << "timMode:" << r.timMode << std::endl
        << "tenGiga:" << r.tenGiga << std::endl
        << "roMode:" << r.roMode << std::endl
        << "adcMask:" << r.adcMask << std::endl
        << "adc10gMask:" << r.adc10gMask << std::endl
        << "roi.xmin:" << r.roi.xmin << std::endl
        << "roi.xmax:" << r.roi.xmax << std::endl
        << "countermask:" << r.countermask << std::endl
        << "burstType:" << r.burstType << std::endl
        << "exptime1:" << ToString(std::chrono::nanoseconds(r.expTime1Ns))
        << std::endl
        << "exptime2:" << ToString(std::chrono::nanoseconds(r.expTime2Ns))
        << std::endl
        << "exptime3:" << ToString(std::chrono::nanoseconds(r.expTime3Ns))
        << std::endl
        << "gateDelay1:" << ToString(std::chrono::nanoseconds(r.gateDelay1Ns))
        << std::endl
        << "gateDelay2:" << ToString(std::chrono::nanoseconds(r.gateDelay2Ns))
        << std::endl
        << "gateDelay3:" << ToString(std::chrono::nanoseconds(r.gateDelay3Ns))
        << std::endl
        << "gates:" << r.gates << std::endl
        << "scanParams:" << ToString(r.scanParams) << std::endl
        << "transceiverSamples:" << r.transceiverSamples << std::endl
        << "transceiverMask:" << r.transceiverMask << std::endl
        << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::rxParameters &r) {
    return os << ToString(r);
}

std::string ToString(const slsDetectorDefs::scanParameters &r) {
    std::ostringstream oss;
    oss << '[';
    if (r.enable) {
        oss << "enabled" << std::endl
            << "dac " << ToString(r.dacInd) << std::endl
            << "start " << r.startOffset << std::endl
            << "stop " << r.stopOffset << std::endl
            << "step " << r.stepSize << std::endl
            << "settleTime "
            << ToString(std::chrono::nanoseconds{r.dacSettleTime_ns})
            << std::endl;
    } else {
        oss << "disabled";
    }
    oss << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::scanParameters &r) {
    return os << ToString(r);
}

std::string ToString(const slsDetectorDefs::currentSrcParameters &r) {
    std::ostringstream oss;
    if (r.fix < -1 || r.fix > 1 || r.normal < -1 || r.normal > 1) {
        throw RuntimeError("Invalid current source parameters. Cannot print.");
    }
    oss << '[';
    if (r.enable) {
        oss << "enabled";
        // [jungfrau]
        if (r.fix != -1) {
            oss << (r.fix == 1 ? ", fix" : ", nofix");
        }
        // [jungfrau chip v1.1]
        if (r.normal != -1) {
            oss << ", " << ToStringHex(r.select, 16);
            oss << (r.normal == 1 ? ", normal" : ", low");
        }
        // [jungfrau chip v1.0]
        else {
            oss << ", " << r.select;
        }
    } else {
        oss << "disabled";
    }
    oss << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::currentSrcParameters &r) {
    return os << ToString(r);
}

std::string ToString(const slsDetectorDefs::pedestalParameters &r) {
    std::ostringstream oss;
    oss << '[';
    if (r.enable)
        oss << "enabled, " << std::to_string(r.frames) << ", " << r.loops;
    else
        oss << "disabled";

    oss << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::pedestalParameters &r) {
    return os << ToString(r);
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
    case defs::XILINX_CHIPTESTBOARD:
        return std::string("Xilinx_ChipTestBoard");
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
    case defs::HIGHGAIN0:
        return std::string("highgain0");
    case defs::FIXGAIN1:
        return std::string("fixgain1");
    case defs::FIXGAIN2:
        return std::string("fixgain2");
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
    case defs::GAIN0:
        return std::string("gain0");
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
    case defs::G2_108MHZ:
        return std::string("108");
    case defs::G2_144MHZ:
        return std::string("144");
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
    case defs::TRANSCEIVER_ONLY:
        return std::string("transceiver");
    case defs::DIGITAL_AND_TRANSCEIVER:
        return std::string("digital_transceiver");
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
    case defs::TRIMBIT_SCAN:
        return std::string("trimbits");
    case defs::HIGH_VOLTAGE:
        return std::string("highvoltage");
    case defs::IO_DELAY:
        return std::string("iodelay");
    case defs::TEMPERATURE_ADC:
        return std::string("temp_adc");
    case defs::TEMPERATURE_FPGA:
        return std::string("temp_fpga");
    case defs::TEMPERATURE_FPGAEXT:
        return std::string("temp_fpgaext");
    case defs::TEMPERATURE_10GE:
        return std::string("temp_10ge");
    case defs::TEMPERATURE_DCDC:
        return std::string("temp_dcdc");
    case defs::TEMPERATURE_SODL:
        return std::string("temp_sodl");
    case defs::TEMPERATURE_SODR:
        return std::string("temp_sodr");
    case defs::TEMPERATURE_FPGA2:
        return std::string("temp_fpgafl");
    case defs::TEMPERATURE_FPGA3:
        return std::string("temp_fpgafr");
    case defs::SLOW_ADC_TEMP:
        return std::string("temp_slowadc");
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
    case defs::BURST_INTERNAL:
        return std::string("burst_internal");
    case defs::BURST_EXTERNAL:
        return std::string("burst_external");
    case defs::CONTINUOUS_INTERNAL:
        return std::string("cw_internal");
    case defs::CONTINUOUS_EXTERNAL:
        return std::string("cw_external");
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

std::string ToString(defs::M3_GainCaps s) {
    std::ostringstream os;
    if (s & defs::M3_C10pre)
        os << "C10pre, ";
    if (s & defs::M3_C15sh)
        os << "C15sh, ";
    if (s & defs::M3_C30sh)
        os << "C30sh, ";
    if (s & defs::M3_C50sh)
        os << "C50sh, ";
    if (s & defs::M3_C225ACsh)
        os << "C225ACsh, ";
    if (s & defs::M3_C15pre)
        os << "C15pre, ";
    auto rs = os.str();
    rs.erase(rs.end() - 2);
    return rs;
}

std::string ToString(const defs::portPosition s) {
    switch (s) {
    case defs::LEFT:
        return std::string("left");
    case defs::RIGHT:
        return std::string("right");
    case defs::TOP:
        return std::string("top");
    case defs::BOTTOM:
        return std::string("bottom");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::streamingInterface s) {
    std::ostringstream os;
    std::string rs;
    switch (s) {
    case defs::streamingInterface::NONE:
        return std::string("none");
    default:
        if ((s & defs::streamingInterface::LOW_LATENCY_LINK) !=
            defs::streamingInterface::NONE)
            os << "lll, ";
        if ((s & defs::streamingInterface::ETHERNET_10GB) !=
            defs::streamingInterface::NONE)
            os << "10gbe, ";
        auto rs = os.str();
        rs.erase(rs.end() - 2, rs.end());
        return rs;
    }
}

std::string ToString(const defs::vetoAlgorithm s) {
    switch (s) {
    case defs::ALG_HITS:
        return std::string("hits");
    case defs::ALG_RAW:
        return std::string("raw");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::gainMode s) {
    switch (s) {
    case defs::DYNAMIC:
        return std::string("dynamic");
    case defs::FORCE_SWITCH_G1:
        return std::string("forceswitchg1");
    case defs::FORCE_SWITCH_G2:
        return std::string("forceswitchg2");
    case defs::FIX_G1:
        return std::string("fixg1");
    case defs::FIX_G2:
        return std::string("fixg2");
    case defs::FIX_G0:
        return std::string("fixg0");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::polarity s) {
    switch (s) {
    case defs::POSITIVE:
        return std::string("pos");
    case defs::NEGATIVE:
        return std::string("neg");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::timingInfoDecoder s) {
    switch (s) {
    case defs::SWISSFEL:
        return std::string("swissfel");
    case defs::SHINE:
        return std::string("shine");
    default:
        return std::string("Unknown");
    }
}

std::string ToString(const defs::collectionMode s) {
    switch (s) {
    case defs::HOLE:
        return std::string("hole");
    case defs::ELECTRON:
        return std::string("electron");
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
    if (s == "Xilinx_ChipTestBoard")
        return defs::XILINX_CHIPTESTBOARD;
    throw RuntimeError("Unknown detector type " + s);
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
    if (s == "highgain0")
        return defs::HIGHGAIN0;
    if (s == "fixgain1")
        return defs::FIXGAIN1;
    if (s == "fixgain2")
        return defs::FIXGAIN2;
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
    if (s == "gain0")
        return defs::GAIN0;
    if (s == "g4_lg")
        return defs::G4_LOWGAIN;
    throw RuntimeError("Unknown setting " + s);
}

template <> defs::speedLevel StringTo(const std::string &s) {
    if (s == "full_speed")
        return defs::FULL_SPEED;
    if (s == "0")
        return defs::FULL_SPEED;
    if (s == "half_speed")
        return defs::HALF_SPEED;
    if (s == "1")
        return defs::HALF_SPEED;
    if (s == "quarter_speed")
        return defs::QUARTER_SPEED;
    if (s == "2")
        return defs::QUARTER_SPEED;
    if (s == "108")
        return defs::G2_108MHZ;
    if (s == "144")
        return defs::G2_144MHZ;
    throw RuntimeError("Unknown speed " + s);
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
    throw RuntimeError("Unknown timing mode " + s);
}

template <> defs::frameDiscardPolicy StringTo(const std::string &s) {
    if (s == "nodiscard")
        return defs::NO_DISCARD;
    if (s == "discardempty")
        return defs::DISCARD_EMPTY_FRAMES;
    if (s == "discardpartial")
        return defs::DISCARD_PARTIAL_FRAMES;
    throw RuntimeError("Unknown frame discard policy " + s);
}

template <> defs::fileFormat StringTo(const std::string &s) {
    if (s == "hdf5")
        return defs::HDF5;
    if (s == "binary")
        return defs::BINARY;
    throw RuntimeError("Unknown file format " + s);
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
    throw RuntimeError("Unknown external signal flag " + s);
}

template <> defs::readoutMode StringTo(const std::string &s) {
    if (s == "analog")
        return defs::ANALOG_ONLY;
    if (s == "digital")
        return defs::DIGITAL_ONLY;
    if (s == "analog_digital")
        return defs::ANALOG_AND_DIGITAL;
    if (s == "transceiver")
        return defs::TRANSCEIVER_ONLY;
    if (s == "digital_transceiver")
        return defs::DIGITAL_AND_TRANSCEIVER;
    throw RuntimeError("Unknown readout mode " + s);
}

template <> defs::dacIndex StringTo(const std::string &s) {
    if (s == "dac 0" || s == "0")
        return defs::DAC_0;
    if (s == "dac 1" || s == "1")
        return defs::DAC_1;
    if (s == "dac 2" || s == "2")
        return defs::DAC_2;
    if (s == "dac 3" || s == "3")
        return defs::DAC_3;
    if (s == "dac 4" || s == "4")
        return defs::DAC_4;
    if (s == "dac 5" || s == "5")
        return defs::DAC_5;
    if (s == "dac 6" || s == "6")
        return defs::DAC_6;
    if (s == "dac 7" || s == "7")
        return defs::DAC_7;
    if (s == "dac 8" || s == "8")
        return defs::DAC_8;
    if (s == "dac 9" || s == "9")
        return defs::DAC_9;
    if (s == "dac 10" || s == "10")
        return defs::DAC_10;
    if (s == "dac 11" || s == "11")
        return defs::DAC_11;
    if (s == "dac 12" || s == "12")
        return defs::DAC_12;
    if (s == "dac 13" || s == "13")
        return defs::DAC_13;
    if (s == "dac 14" || s == "14")
        return defs::DAC_14;
    if (s == "dac 15" || s == "15")
        return defs::DAC_15;
    if (s == "dac 16" || s == "16")
        return defs::DAC_16;
    if (s == "dac 17" || s == "17")
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
    if (s == "trimbits")
        return defs::TRIMBIT_SCAN;
    if (s == "highvoltage")
        return defs::HIGH_VOLTAGE;
    if (s == "iodelay")
        return defs::IO_DELAY;
    if (s == "temp_adc")
        return defs::TEMPERATURE_ADC;
    if (s == "temp_fpga")
        return defs::TEMPERATURE_FPGA;
    if (s == "temp_fpgaext")
        return defs::TEMPERATURE_FPGAEXT;
    if (s == "temp_10ge")
        return defs::TEMPERATURE_10GE;
    if (s == "temp_dcdc")
        return defs::TEMPERATURE_DCDC;
    if (s == "temp_sodl")
        return defs::TEMPERATURE_SODL;
    if (s == "temp_sodr")
        return defs::TEMPERATURE_SODR;
    if (s == "temp_fpgafl")
        return defs::TEMPERATURE_FPGA2;
    if (s == "temp_fpgafr")
        return defs::TEMPERATURE_FPGA3;
    if (s == "temp_slowadc")
        return defs::SLOW_ADC_TEMP;
    throw RuntimeError("Unknown dac Index " + s);
}

template <> defs::burstMode StringTo(const std::string &s) {
    if (s == "burst_internal")
        return defs::BURST_INTERNAL;
    if (s == "burst_external")
        return defs::BURST_EXTERNAL;
    if (s == "cw_internal")
        return defs::CONTINUOUS_INTERNAL;
    if (s == "cw_external")
        return defs::CONTINUOUS_EXTERNAL;
    throw RuntimeError("Unknown burst mode " + s);
}

template <> defs::timingSourceType StringTo(const std::string &s) {
    if (s == "internal")
        return defs::TIMING_INTERNAL;
    if (s == "external")
        return defs::TIMING_EXTERNAL;
    throw RuntimeError("Unknown timing source type " + s);
}

template <> defs::M3_GainCaps StringTo(const std::string &s) {
    if (s == "C10pre")
        return defs::M3_C10pre;
    if (s == "C15sh")
        return defs::M3_C15sh;
    if (s == "C30sh")
        return defs::M3_C30sh;
    if (s == "C50sh")
        return defs::M3_C50sh;
    if (s == "C225ACsh")
        return defs::M3_C225ACsh;
    if (s == "C15pre")
        return defs::M3_C15pre;
    throw RuntimeError("Unknown gain cap " + s);
}

template <> defs::portPosition StringTo(const std::string &s) {
    if (s == "left")
        return defs::LEFT;
    if (s == "right")
        return defs::RIGHT;
    if (s == "top")
        return defs::TOP;
    if (s == "bottom")
        return defs::BOTTOM;
    throw RuntimeError("Unknown port position " + s);
}

template <> defs::streamingInterface StringTo(const std::string &s) {
    std::string rs = s;
    if (s.find(',') != std::string::npos)
        rs.erase(rs.find(','));
    if (rs == "none")
        return defs::streamingInterface::NONE;
    if (rs == "lll")
        return defs::streamingInterface::LOW_LATENCY_LINK;
    if (rs == "10gbe")
        return defs::streamingInterface::ETHERNET_10GB;
    throw RuntimeError("Unknown streamingInterface type " + s);
}

template <> defs::vetoAlgorithm StringTo(const std::string &s) {
    if (s == "hits")
        return defs::ALG_HITS;
    if (s == "raw")
        return defs::ALG_RAW;
    throw RuntimeError("Unknown veto algorithm " + s);
}

template <> defs::gainMode StringTo(const std::string &s) {
    if (s == "dynamic")
        return defs::DYNAMIC;
    if (s == "forceswitchg1")
        return defs::FORCE_SWITCH_G1;
    if (s == "forceswitchg2")
        return defs::FORCE_SWITCH_G2;
    if (s == "fixg1")
        return defs::FIX_G1;
    if (s == "fixg2")
        return defs::FIX_G2;
    if (s == "fixg0")
        return defs::FIX_G0;
    throw RuntimeError("Unknown gain mode " + s);
}

template <> defs::polarity StringTo(const std::string &s) {
    if (s == "pos")
        return defs::POSITIVE;
    if (s == "neg")
        return defs::NEGATIVE;
    throw RuntimeError("Unknown polarity mode " + s);
}

template <> defs::timingInfoDecoder StringTo(const std::string &s) {
    if (s == "swissfel")
        return defs::SWISSFEL;
    if (s == "shine")
        return defs::SHINE;
    throw RuntimeError("Unknown Timing Info Decoder " + s);
}

template <> defs::collectionMode StringTo(const std::string &s) {
    if (s == "hole")
        return defs::HOLE;
    if (s == "electron")
        return defs::ELECTRON;
    throw RuntimeError("Unknown collection mode " + s);
}

template <> uint8_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    int value = std::stoi(s, nullptr, base);
    if (value < std::numeric_limits<uint8_t>::min() ||
        value > std::numeric_limits<uint8_t>::max()) {
        throw RuntimeError("Cannot scan uint8_t from string '" + s +
                           "'. Value must be in range 0 - 255.");
    }
    return static_cast<uint8_t>(value);
}

template <> uint16_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    int value = std::stoi(s, nullptr, base);
    if (value < std::numeric_limits<uint16_t>::min() ||
        value > std::numeric_limits<uint16_t>::max()) {
        throw RuntimeError("Cannot scan uint16_t from string '" + s +
                           "'. Value must be in range 0 - 65535.");
    }
    return static_cast<uint16_t>(value);
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

template <> bool StringTo(const std::string &s) {
    int i = std::stoi(s, nullptr, 10);
    switch (i) {
    case 0:
        return false;
    case 1:
        return true;
    default:
        throw RuntimeError("Unknown boolean. Expecting be 0 or 1.");
    }
}

template <> int64_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stol(s, nullptr, base);
}

} // namespace sls