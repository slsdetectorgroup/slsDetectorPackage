#include "string.h"

std::string GetDacHelp(std::string dac) {
    if (dac.find("dac") != std::string::npos) {
        return std::string("[dac name] [dac or mV value] [(optional unit) mV] "
                           "\n\t[Ctb] Use dac index for dac name.");
    }
    if (dac == "vthreshold") {
        return std::string(
            "vthreshold [dac or mV value][(optional unit) mV] "
            "\n\t[Eiger][Mythen3] Detector threshold voltage for single "
            "photon counters.\n\t[Eiger] Sets vcmp_ll, vcmp_lr, vcmp_rl, "
            "vcmp_rr and vcp to the same value. \n\t[Mythen3] Sets vth1, "
            "vth2 and vth3 to the same value.");
    }
    if (dac == "vthreshold") {
    }
    throw sls::RuntimeError("Unknown dac command");
}