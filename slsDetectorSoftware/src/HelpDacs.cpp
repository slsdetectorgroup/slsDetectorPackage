// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_defs.h"
#include "sls/string_utils.h"
#include <sstream>

namespace sls {

std::string GetHelpDac(std::string dac) {
    if (sls::is_int(dac)) {
        return std::string("[dac name] [dac or mV value] [(optional unit) mV] "
                           "\n\t[Ctb] Use dac index for dac name.");
    }
    if (dac == "vthreshold") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger][Mythen3] "
            "Detector threshold voltage for single photon counters.\n\t[Eiger] "
            "Sets vcmp_ll, vcmp_lr, vcmp_rl, vcmp_rr and vcp to the same "
            "value. \n\t[Mythen3] Sets vth1, vth2 and vth3 to the same value "
            "for enabled counters.");
    }
    if (dac == "vsvp") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ?? ");
    }
    if (dac == "vsvn") {
        return std::string("[dac or mV value][(optional unit) mV] \n\t[Eiger] "
                           "Dac for ?? \n\t[Mythen3] voltage to define "
                           "feedback resistance of the first shaper");
    }
    if (dac == "vtrim") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ?? "
            "\n\t[Mythen3] Dac for the voltage defining the trim bit size.");
    }
    if (dac == "vrpreamp") {
        return std::string("[dac or mV value][(optional unit) mV] \n\t[Eiger] "
                           "Dac for ?? \n\t[Mythen3] voltage to define the "
                           "preamplifier feedback resistance.");
    }
    if (dac == "vrshaper") {
        return std::string("[dac or mV value][(optional unit) mV] \n\t[Eiger] "
                           "Dac for ?? \n\t[Mythen3] voltage to define "
                           "feedback resistance of the first shaper");
    }
    if (dac == "vtgstv") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vcmp_ll") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vcmp_lr") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vcal") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vcmp_rl") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vcmp_rr") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "rxb_rb") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "rxb_lb") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vcp") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vcn") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vishaper") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "iodelay") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for ??");
    }
    if (dac == "vref_ds") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Gotthard][Jungfrau] Dac for ??");
    }
    if (dac == "vcascn_pb") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for ??");
    }
    if (dac == "vcascp_pb") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for ??");
    }
    if (dac == "vout_cm") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Gotthard] Dac for ??\n\t[Moench] Dac for 5");
    }
    if (dac == "vcasc_out") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for ??");
    }
    if (dac == "vin_cm") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Gotthard] Dac for ??\n\t[Moench] Dac for 2");
    }
    if (dac == "vref_comp") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Gotthard][Jungfrau] Dac for ??");
    }
    if (dac == "ib_test_c") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for ??");
    }
    if (dac == "vrshaper_n") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] voltage to "
            "define feedback resistance of the second shaper.");
    }
    if (dac == "vipre") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
            "preamplifier's input transistor current.\n\t[Moench] Dac for 1");
    }
    if (dac == "vdcsh") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
            "reference (DC) voltage for the shaper.");
    }
    if (dac == "vth1") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for first "
            "detector threshold voltage. Overwrites even if counter disabled.");
    }
    if (dac == "vth2") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
            "second detector threshold voltage. Overwrites even if counter "
            "disabled.");
    }
    if (dac == "vth3") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for third "
            "detector threshold voltage. Overwrites even if counter disabled.");
    }
    if (dac == "vcal_n") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
            "low voltage for analog pulsing.");
    }
    if (dac == "vcal_p") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
            "high voltage for analog pulsing.");
    }
    if (dac == "vcassh") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
            "shaper's cascode voltage.");
    }
    if (dac == "vcas") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
            "preamplifier's cascode voltage.");
    }
    if (dac == "vicin") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
            "bias current for the comparator.");
    }
    if (dac == "vipre_out") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
            "preamplifier's output transistor current.");
    }
    if (dac == "vref_h_adc") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "reference voltage high of ADC.");
    }
    if (dac == "vb_comp_fe") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "comparator current of analogue front end.");
    }
    if (dac == "vb_comp_adc") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "comparator current of ADC.");
    }
    if (dac == "vcom_cds") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "common mode voltage of CDS stage.");
    }
    if (dac == "vref_rstore") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Gotthard2] Dac for reference charging voltage "
                           "of temparory storage cell in high gain.");
    }
    if (dac == "vb_opa_1st") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] dac dac for "
            "opa current for driving the other DACs in chip.");
    }
    if (dac == "vref_comp_fe") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "reference voltage of the comparator of analogue front end.");
    }
    if (dac == "vcom_adc1") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "common mode voltage of ADC DAC bank 1.");
    }
    if (dac == "vref_prech") {
        return std::string(
            "[dac or mV value][(optional unit) mV] "
            "\n\t[Gotthard2][Jungfrau] "
            "Dac for reference votlage for precharing the preamplifier.");
    }
    if (dac == "vref_l_adc") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "reference voltage low for ADC.");
    }
    if (dac == "vref_cds") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "reference voltage of CDS applied to the temporary storage cell in "
            "medium and low gain.");
    }
    if (dac == "vb_cs") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "current injection into preamplifier.");
    }
    if (dac == "vb_opa_fd") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "current for CDS opa stage.");
    }
    if (dac == "vcom_adc2") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
            "common mode voltage of ADC DAC bank 2.");
    }
    if (dac == "vb_ds") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Jungfrau] Dac for ??");
    }
    if (dac == "vb_comp") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Jungfrau] Dac for ??");
    }
    if (dac == "vb_pixbuf") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Jungfrau] Dac for ??");
    }
    if (dac == "vin_com") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Jungfrau] Dac for ??");
    }
    if (dac == "vdd_prot") {
        return std::string("[dac or mV value][(optional unit) mV] "
                           "\n\t[Jungfrau] Dac for ??");
    }
    if (dac == "vbp_colbuf") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 0");
    }
    if (dac == "vb_sda") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 3");
    }
    if (dac == "vcasc_sfp") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 4");
    }
    if (dac == "vipre_cds") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 6");
    }
    if (dac == "ibias_sfp") {
        return std::string(
            "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 7");
    }

    // clang-format off
    if (dac == "vtgstv") { return std::string(""); }
    // clang-format on

    throw sls::RuntimeError("Unknown dac command");
}

std::string GetHelpDacWrapper(const std::string &cmd,
                              const std::vector<std::string> &args) {
    std::ostringstream os;
    os << cmd << ' ';
    if (args.size() == 0) {
        os << GetHelpDac(std::to_string(0)) << '\n';
    } else {
        os << args[0] << ' ' << GetHelpDac(args[0]) << '\n';
    }
    return os.str();
}

} // namespace sls
