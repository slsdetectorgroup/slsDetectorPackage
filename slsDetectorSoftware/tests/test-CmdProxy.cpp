#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "tests/globals.h"

// auto GET = slsDetectorDefs::GET_ACTION;
// auto PUT = slsDetectorDefs::PUT_ACTION;

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

TEST_CASE("Unknown command", "[.cmd]") {

    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("vsaevrreavv", {}, -1, PUT));
}

TEST_CASE("type", "[.cmd]"){
    Detector det;
    CmdProxy proxy(&det);
    auto dt = det.getDetectorType().squash();

    std::ostringstream oss;
    proxy.Call("type", {}, -1, GET, oss);
    auto ans = oss.str().erase(0, strlen("type "));
    REQUIRE(ans == sls::ToString(dt) + '\n');
    REQUIRE(dt == test::type);

}


TEST_CASE("initialchecks", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto check = det.getInitialChecks();
    auto dtstr = sls::ToString(check);
    auto hostname = det.getHostname();
    std::string hostnamestr;
    for (auto &it : hostname) {
        hostnamestr += (it + "+");
    }
    {
        std::ostringstream oss;
        proxy.Call("initialchecks", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("initialchecks", {}, -1, GET, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");        
    }
    {
        det.setHostname(hostname);
        std::ostringstream oss;
        proxy.Call("initialchecks", {}, -1, GET, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");        
    }   
    det.setInitialChecks(check);
}

// TEST_CASE("dacs", "[.cmd]") {
//     REQUIRE_NOTHROW(multiSlsDetectorClient("daclist", GET));
//     REQUIRE_NOTHROW(multiSlsDetectorClient("dacvalues", GET));
//     int prev_val = 0;
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_ds", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vb_ds "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_ds 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_ds", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vb_ds 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vb_ds " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_comp", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vb_comp "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_comp 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_comp", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vb_comp 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vb_comp " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_pixbuf", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vb_pixbuf
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_pixbuf 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vb_pixbuf", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vb_pixbuf 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vb_pixbuf " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vref_ds "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vref_ds 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vref_comp
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vref_comp 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_prech", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("vref_prech ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_prech 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_prech", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "vref_prech 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vref_prech " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vin_com", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vin_com "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vin_com 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vin_com", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vin_com 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vin_com " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vdd_prot", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vdd_prot
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vdd_prot 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vdd_prot", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vdd_prot 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vdd_prot " +
//         std::to_string(prev_val), PUT));

//         REQUIRE_THROWS(multiSlsDetectorClient("vthreshold", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vsvp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vsvn", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vtr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vrf", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vrs", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vtgstv", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_ll", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_lr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcal", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_rl", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_rr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("rxb_rb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("rxb_lb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcn", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vis", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("iodelay", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcascn_pb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcascp_pb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vout_cm", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcasc_out", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vin_cm", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("ib_test_c", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vpreamp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vshaper", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vshaperneg", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vipre", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("viinsh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vdcsh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth1", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth3", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vpl", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vph", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vtrim", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcassh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcas", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vicin", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vipre_out", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_h_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp_fe", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_cds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_rstore", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_opa_1st", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_comp_fe", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_adc1", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_l_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_cds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_cs", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_opa_fd", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_adc2", GET));
//     }

//     else if (test::type == slsDetectorDefs::GOTTHARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vref_ds "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vref_ds 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vref_ds " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcascn_pb", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vcascn_pb
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcascn_pb 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcascn_pb", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vcascn_pb 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vcascn_pb " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcascp_pb", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vcascp_pb
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcascp_pb 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcascp_pb", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vcascp_pb 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vcascp_pb " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vout_cm", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vout_cm "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vout_cm 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vout_cm", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vout_cm 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vout_cm " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcasc_out", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vcasc_out
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcasc_out 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vcasc_out", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vcasc_out 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vcasc_out " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vin_cm", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vin_cm "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vin_cm 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vin_cm", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vin_cm 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vin_cm " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("vref_comp
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "vref_comp 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vref_comp " +
//         std::to_string(prev_val), PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("ib_test_c", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("ib_test_c
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("ib_test_c 1000", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("ib_test_c", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "ib_test_c 1000\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("ib_test_c " +
//         std::to_string(prev_val), PUT));

//         REQUIRE_THROWS(multiSlsDetectorClient("vthreshold", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vsvp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vsvn", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vtr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vrf", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vrs", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vtgstv", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_ll", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_lr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcal", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_rl", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_rr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("rxb_rb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("rxb_lb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcn", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vis", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("iodelay", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vpreamp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vshaper", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vshaperneg", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vipre", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("viinsh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vdcsh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth1", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth3", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vpl", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vph", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vtrim", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcassh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcas", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vicin", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vipre_out", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_h_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp_fe", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_cds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_rstore", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_opa_1st", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_comp_fe", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_adc1", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_prech", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_l_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_cds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_cs", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_opa_fd", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_adc2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_ds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_pixbuf", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vin_com", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vdd_prot", GET));
//     }

//     else if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         for (int i = 0; i < 18; ++i) {
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("dac " +
//                 std::to_string(i), GET, nullptr, oss)); std::string s =
//                 (oss.str()).erase (0, ("dac " + std::to_string(i)).length() +
//                 1); prev_val = std::stoi(s);
//             }
//             {
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("dac " +
//                 std::to_string(i) + " 1000", PUT)); std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("dac " +
//                 std::to_string(i), GET, nullptr, oss)); REQUIRE(oss.str() ==
//                 "dac "  + std::to_string(i) + " 1000\n");
//             }
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dac " + std::to_string(i)
//             + " " + std::to_string(prev_val), PUT));
//         }

//         //REQUIRE_THROWS(multiSlsDetectorClient("vthreshold", GET)); dac 0
//         //REQUIRE_THROWS(multiSlsDetectorClient("vsvp", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vsvn", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vtr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vrf", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vrs", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vtgstv", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_ll", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_lr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcal", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_rl", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcmp_rr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("rxb_rb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("rxb_lb", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcn", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vis", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("iodelay", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vref_ds", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vcascn_pb", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vcascp_pb", GET));
//        // REQUIRE_THROWS(multiSlsDetectorClient("vout_cm", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vcasc_out", GET));
//        // REQUIRE_THROWS(multiSlsDetectorClient("vin_cm", GET));
//        // REQUIRE_THROWS(multiSlsDetectorClient("vref_comp", GET));
//        // REQUIRE_THROWS(multiSlsDetectorClient("ib_test_c", GET));
//        //REQUIRE_THROWS(multiSlsDetectorClient("vpreamp", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vshaper", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vshaperneg", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vipre", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("viinsh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vdcsh", GET));
//        // REQUIRE_THROWS(multiSlsDetectorClient("vth1", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vth3", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vpl", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vph", GET));
//         //REQUIRE_THROWS(multiSlsDetectorClient("vtrim", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcassh", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcas", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vicin", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vipre_out", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_h_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp_fe", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_cds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_rstore", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_opa_1st", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_comp_fe", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_adc1", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_prech", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_l_adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vref_cds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_cs", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_opa_fd", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vcom_adc2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_ds", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_comp", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vb_pixbuf", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vin_com", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vdd_prot", GET));
//     }

// }

TEST_CASE("user", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("user", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(proxy.Call("user", {}, -1, PUT));
}

// TEST_CASE("execcommand", "[.cmd]") {
//     REQUIRE_NOTHROW(multiSlsDetectorClient("execcommand ls", PUT));
// }

TEST_CASE("port", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("port", {"1942"}, -1, PUT, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("port", {}, -1, GET, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    proxy.Call("port", {"1952"}, -1, PUT);
}

TEST_CASE("stopport", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("stopport", {"1942"}, -1, PUT, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("stopport", {}, -1, GET, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    proxy.Call("stopport", {"1953"}, -1, PUT);
    auto port = det.getStopPort().squash();
    REQUIRE(port == 1953);
}

// TEST_CASE("reg", "[.cmd]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("reg 0x01", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "reg 0xacdc2014\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("reg 0x64 5", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "reg [0x64, 5]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("frames", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "frames 5\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("getbit 0x64 0", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "getbit 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("setbit 0x64 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "setbit [0x64, 1]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("clearbit 0x64 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "clearbit [0x64, 0]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("frames", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "frames 6\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//     } else if (test::type == slsDetectorDefs::GOTTHARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("reg 0x023", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "reg 0xacdc1980\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("reg 0x70 5", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "reg [0x70, 5]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("frames", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "frames 5\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("getbit 0x70 0", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "getbit 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("setbit 0x70 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "setbit [0x70, 1]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("clearbit 0x70 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "clearbit [0x70, 0]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("frames", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "frames 6\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//     } else if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("reg 0x01", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "reg 0xacdc2016\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("reg 0x64 5", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "reg [0x64, 5]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("frames", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "frames 5\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("getbit 0x64 0", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "getbit 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("setbit 0x64 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "setbit [0x64, 1]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("clearbit 0x64 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "clearbit [0x64, 0]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("frames", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "frames 6\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//     }
// }

// TEST_CASE("update", "[.cmd][.ctb][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU || test::type ==
//     slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_THROWS(multiSlsDetectorClient("update", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("update
//         jungfrauDetectorServer_developer", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("update
//         jungfrauDetectorServer_developer pc13784", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("update
//         jungfrauDetectorServer_developer pc13784 dfd.pff", PUT));
//         //REQUIRE_NOTHROW(multiSlsDetectorClient("update
//         jungfrauDetectorServer_developer pc13784
//         /afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof",
//         PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("update", GET));
//     }
// }

// TEST_CASE("copydetectorserver", "[.cmd][.ctb][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU || test::type ==
//     slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_THROWS(multiSlsDetectorClient("copydetectorserver", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("copydetectorserver
//         jungfrauDetectorServer_developer", PUT));
//         //REQUIRE_NOTHROW(multiSlsDetectorClient("copydetectorserver
//         jungfrauDetectorServer_developer pc13784", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("copydetectorserver", GET));
//     }
// }

// TEST_CASE("rebootcontroller", "[.cmd][.ctb][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU || test::type ==
//     slsDetectorDefs::CHIPTESTBOARD) {
//         ;//REQUIRE_NOTHROW(multiSlsDetectorClient("rebootcontroller", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("rebootcontroller", GET));
//     }
// }

// TEST_CASE("programfpga", "[.cmd][.ctb][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU || test::type ==
//     slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_THROWS(multiSlsDetectorClient("programfpga fdgd.oki", PUT));
//         //REQUIRE_NOTHROW(multiSlsDetectorClient("programfpga
//         /afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof",
//         PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("programfpga", GET));
//     }
// }

// TEST_CASE("detectormode", "[.cmd][.moench]") {
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode counting", PUT,
//         nullptr, oss)); REQUIRE(oss.str() == "detectormode counting\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode interpolating",
//         PUT, nullptr, oss)); REQUIRE(oss.str() == "detectormode
//         interpolating\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode analog", PUT,
//         nullptr, oss)); REQUIRE(oss.str() == "detectormode analog\n");
//     }
//     REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode counting", PUT));
//     REQUIRE_THROWS(multiSlsDetectorClient("detectormode pedestal", PUT));
// }

// TEST_CASE("framemode", "[.cmd][.moench]") {
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("framemode pedestal", PUT,
//         nullptr, oss)); REQUIRE(oss.str() == "framemode pedestal\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("framemode newpedestal", PUT,
//         nullptr, oss)); REQUIRE(oss.str() == "framemode newpedestal\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("framemode flatfield", PUT,
//         nullptr, oss)); REQUIRE(oss.str() == "framemode flatfield\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("framemode newflatfield", PUT,
//         nullptr, oss)); REQUIRE(oss.str() == "framemode newflatfield\n");
//     }
//     REQUIRE_NOTHROW(multiSlsDetectorClient("framemode pedestal", PUT));
//     REQUIRE_THROWS(multiSlsDetectorClient("framemode counting", PUT));
// }

// TEST_CASE("emin", "[.cmd][.moench]") {
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("emin 100", PUT, nullptr,
//         oss)); REQUIRE(oss.str() == "emin 100\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("emax 200", PUT, nullptr,
//         oss)); REQUIRE(oss.str() == "emax 200\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara emax", GET,
//         nullptr, oss)); REQUIRE(oss.str() == "rx_jsonpara 200\n");
//     }
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT));
// }

// TEST_CASE("rx_jsonpara", "[.cmd][.moench]") {
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader
//     \"key1\":\"value1\"", PUT));
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara key1", GET,
//         nullptr, oss)); REQUIRE(oss.str() == "rx_jsonpara value1\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara key1 value2",
//         PUT, nullptr, oss)); REQUIRE(oss.str() == "rx_jsonpara [key1,
//         value2]\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara key2 98", PUT,
//         nullptr, oss)); REQUIRE(oss.str() == "rx_jsonpara [key2, 98]\n");
//     }
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader", GET,
//         nullptr, oss)); REQUIRE(oss.str() == "rx_jsonaddheader
//         \"key1\":\"value2\",\"key2\":98\n");
//     }
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT));
// }

// TEST_CASE("patsetbit", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         uint64_t val = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patsetbit", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("patsetbit
//             ")); val = stoul(s, 0, 16);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patsetbit
//             0x842f020204200dc0", PUT, nullptr, oss)); REQUIRE(oss.str() ==
//             "patsetbit 0x842f020204200dc0\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("patsetbit " +
//         sls::ToStringHex(val), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("patsetbit", GET));
//     }
// }

// TEST_CASE("patmask", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         uint64_t val = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patmask", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("patmask "));
//             val = stoul(s, 0, 16);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patmask
//             0x842f020204200dc0", PUT, nullptr, oss)); REQUIRE(oss.str() ==
//             "patmask 0x842f020204200dc0\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("patmask " +
//         sls::ToStringHex(val), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("patmask", GET));
//     }
// }

// TEST_CASE("patwaittime", "[.cmd][.ctb]") {
//     for (int loop = 0; loop < 3; ++loop) {
//         if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//             uint64_t val = 0;
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patwaittime" +
//                 std::to_string(loop), GET, nullptr, oss)); std::string s =
//                 (oss.str()).erase (0, strlen("patwaittime") + 2); val =
//                 std::stoul(s);
//             }
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patwaittime" +
//                 std::to_string(loop) + " 8589936640", PUT, nullptr, oss));
//                 REQUIRE(oss.str() == "patwaittime" + std::to_string(loop) + "
//                 8589936640\n");
//             }
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patwaittime" +
//             std::to_string(loop) + ' ' + std::to_string(val), PUT));
//         } else {
//             REQUIRE_THROWS(multiSlsDetectorClient("patwaittime" +
//             std::to_string(loop), GET));
//         }
//     }
// }

// TEST_CASE("patwait", "[.cmd][.ctb]") {
//     for (int loop = 0; loop < 3; ++loop) {
//         if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//             int val = 0;
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patwait" +
//                 std::to_string(loop), GET, nullptr, oss)); std::string s =
//                 (oss.str()).erase (0, strlen("patwait") + 2); val = stoul(s,
//                 0, 16);
//             }
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patwait" +
//                 std::to_string(loop) + " 0x5c", PUT, nullptr, oss));
//                 REQUIRE(oss.str() == "patwait" + std::to_string(loop) + "
//                 0x5c\n");
//             }
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patwait" +
//             std::to_string(loop) + ' ' + sls::ToStringHex(val), PUT));
//         } else {
//             REQUIRE_THROWS(multiSlsDetectorClient("patwait" +
//             std::to_string(loop), GET));
//         }
//     }
// }

// TEST_CASE("patnloop", "[.cmd][.ctb]") {
//     for (int loop = 0; loop < 3; ++loop) {
//         if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//             int val = 0;
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patnloop" +
//                 std::to_string(loop), GET, nullptr, oss)); std::string s =
//                 (oss.str()).erase (0, strlen("patnloop") + 2); val =
//                 std::stoi(s);
//             }
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patnloop" +
//                 std::to_string(loop) + " 5", PUT, nullptr, oss));
//                 REQUIRE(oss.str() == "patnloop" + std::to_string(loop) + "
//                 5\n");
//             }
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patnloop" +
//             std::to_string(loop) + ' ' + std::to_string(val), PUT));
//         } else {
//             REQUIRE_THROWS(multiSlsDetectorClient("patnloop" +
//             std::to_string(loop), GET));
//         }
//     }
// }

// TEST_CASE("patloop", "[.cmd][.ctb]") {
//     for (int loop = 0; loop < 3; ++loop) {
//         if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//             uint32_t limit1 = 0, limit2 = 0;
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patloop" +
//                 std::to_string(loop), GET, nullptr, oss)); std::string s =
//                 oss.str(); auto t = sls::split(s, ' '); s = t[1].erase (0,
//                 1); limit1 = stoul(s, 0, 16); limit2 = stoul(t[2], 0, 16);
//             }
//             {
//                 std::ostringstream oss;
//                 REQUIRE_NOTHROW(multiSlsDetectorClient("patloop" +
//                 std::to_string(loop) + " 0x20 0x5c", PUT, nullptr, oss));
//                 REQUIRE(oss.str() == "patloop" + std::to_string(loop) + "
//                 [0x20, 0x5c]\n");
//             }
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patloop" +
//             std::to_string(loop) + ' ' + sls::ToStringHex(limit1) + ' ' +
//             sls::ToStringHex(limit2), PUT));
//             REQUIRE_THROWS(multiSlsDetectorClient("patloop" +
//             std::to_string(loop) + " 0x3", PUT));
//         } else {
//             REQUIRE_THROWS(multiSlsDetectorClient("patloop" +
//             std::to_string(loop), GET));
//         }
//     }
// }

// TEST_CASE("patlimits", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         uint32_t patlimit1 = 0, patlimit2 = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patlimits", GET, nullptr,
//             oss)); std::string s = oss.str(); auto t = sls::split(s, ' '); s
//             = t[1].erase (0, 1); patlimit1 = stoul(s, 0, 16); patlimit2 =
//             stoul(t[2], 0, 16);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patlimits 0x20 0x5c",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "patlimits [0x20,
//             0x5c]\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("patlimits " +
//         sls::ToStringHex(patlimit1) + ' ' + sls::ToStringHex(patlimit2),
//         PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("patlimits", GET));
//     }
// }

// TEST_CASE("patword", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         uint64_t prev_value = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("patword ")); prev_value = stoul(s, 0, 16);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23
//             0xc15004808d0a21a4", PUT, nullptr, oss)); REQUIRE(oss.str() ==
//             "patword [0x23, 0xc15004808d0a21a4]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23 0x0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "patword [0x23, 0x0]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "patword 0x0\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23 " +
//         std::to_string(prev_value), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("patword 0x23", GET));
//     }
// }

// TEST_CASE("patclkctrl", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         uint64_t prev_value = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("patclkctrl ")); prev_value = stoul(s, 0, 16);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl
//             0xc15004808d0a21a4", PUT, nullptr, oss)); REQUIRE(oss.str() ==
//             "patclkctrl 0xc15004808d0a21a4\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl 0x0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "patclkctrl 0x0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "patclkctrl 0x0\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl " +
//         std::to_string(prev_value), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("patclkctrl", GET));
//     }
// }

// TEST_CASE("patioctrl", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         uint64_t prev_value = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("patioctrl
//             ")); prev_value = stoul(s, 0, 16);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl
//             0xc15004808d0a21a4", PUT, nullptr, oss)); REQUIRE(oss.str() ==
//             "patioctrl 0xc15004808d0a21a4\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl 0x0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "patioctrl 0x0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "patioctrl 0x0\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl " +
//         std::to_string(prev_value), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("patioctrl", GET));
//     }
// }

// TEST_CASE("savepattern", "[.cmd][.ctb]") {
//     REQUIRE_THROWS(multiSlsDetectorClient("savepattern", GET));
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("savepattern
//             /tmp/pat.txt", PUT, nullptr, oss)); REQUIRE(oss.str() ==
//             "savepattern /tmp/pat.txt\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("savepattern /tmp/pat.txt",
//         PUT));
//     }
// }

// TEST_CASE("pattern", "[.cmd][.ctb]") {
//     REQUIRE_THROWS(multiSlsDetectorClient("pattern", GET));
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         ;// todo test with real file?
//     }
// }

// TEST_CASE("led", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("led 1", PUT, nullptr,
//             oss)); REQUIRE(oss.str() == "led 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("led 0", PUT, nullptr,
//             oss)); REQUIRE(oss.str() == "led 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("led", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "led 0\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("led", GET));
//     }
// }

// TEST_CASE("diodelay", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("diodelay 0x01010 125", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("diodelay 0x01010 775",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "diodelay [0x01010,
//             775]\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("diodelay 0x01010 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("diodelay [0x01010, 776]",
//         PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("diodelay", GET));
//     }
// }

// TEST_CASE("rx_dbitoffset", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "rx_dbitoffset 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "rx_dbitoffset 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 15", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "rx_dbitoffset 15\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "rx_dbitoffset 15\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 0", PUT));
//     } else {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "rx_dbitoffset 0\n");
//     }
// }

// TEST_CASE("rx_dbitlist", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitlist 0 4 5 8 9 10 52
//         63", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_dbitlist", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "rx_dbitlist [0, 4, 5, 8, 9,
//             10, 52, 63]\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitlist all", PUT));
//     } else {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitlist", GET, nullptr,
//         oss)); REQUIRE(oss.str() == "rx_dbitlist []\n");
//     }
// }

// TEST_CASE("extsampling", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsampling 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "extsampling 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsampling", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "extsampling 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsampling 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "extsampling 0\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("extsampling", GET));
//     }
// }

// TEST_CASE("extsamplingsrc", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "extsamplingsrc 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "extsamplingsrc 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc 15", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "extsamplingsrc 15\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "extsamplingsrc 15\n");
//         }
//     REQUIRE_THROWS(multiSlsDetectorClient("extsamplingsrc 64", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("extsamplingsrc", GET));
//     }
// }

// TEST_CASE("adcinvert", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD || test::type ==
//     slsDetectorDefs::JUNGFRAU) {
//         std::string s;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcinvert", GET, nullptr,
//             oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcinvert 0x8d0a21d4",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "adcinvert
//             0x8d0a21d4\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
//             REQUIRE(oss.str() == s);
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("adcinvert", GET));
//     }
// }

// TEST_CASE("adcenable", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         std::string s;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcenable", GET, nullptr,
//             oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcenable 0x8d0a21d4",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "adcenable
//             0x8d0a21d4\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
//             REQUIRE(oss.str() == s);
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("adcenable", GET));
//     }
// }

// TEST_CASE("vm_a", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vm_a", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vm_b", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vm_c", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vm_d", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vm_io", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("im_a", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("im_b", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("im_c", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("im_d", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("im_io", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("vm_a", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vm_b", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vm_c", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vm_d", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("vm_io", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("im_a", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("im_b", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("im_c", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("im_d", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("im_io", GET));
//     }
// }

// TEST_CASE("v_a", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         std::string s;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_limit", GET, nullptr,
//             oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("v_limit 1500", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "v_limit 1500\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("v_limit", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "v_limit 1500\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_a", GET, nullptr,
//             oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
//             REQUIRE(oss.str() == s);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_b", GET, nullptr,
//             oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
//             REQUIRE(oss.str() == s);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_c", GET, nullptr,
//             oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
//             REQUIRE(oss.str() == s);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_d", GET, nullptr,
//             oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
//             REQUIRE(oss.str() == s);
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_io", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_chip", GET)); // do not
//         set vchip
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("v_limit", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("v_a", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("v_b", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("v_c", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("v_d", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("v_io", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("v_chip", GET));
//     }
// }

// TEST_CASE("adcvpp", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         int prev_val = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("adcvpp "));
//             prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp 1", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "adcvpp 1\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp 1140 mv", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp mv", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "adcvpp 1140 mv\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp " +
//         std::to_string(prev_val), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("adcvpp", GET));
//     }
// }

// TEST_CASE("dbitpipeline", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "dbitpipeline 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "dbitpipeline 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 15", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "dbitpipeline 15\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "dbitpipeline 15\n");
//         }
//     REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 0", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("dbitpipeline", GET));
//     }
// }

// TEST_CASE("adcpipeline", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "adcpipeline 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "adcpipeline 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 15", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "adcpipeline 15\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "adcpipeline 15\n");
//         }
//     REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 0", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("adcpipeline", GET));
//     }
// }

// TEST_CASE("maxdbitphaseshift", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD ) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("maxdbitphaseshift", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("maxdbitphaseshift 120", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("maxdbitphaseshift", GET));
//     }
// }

// TEST_CASE("dbitphase", "[.cmd][.ctb]") {
//     if (test::type != slsDetectorDefs::CHIPTESTBOARD) {
//        REQUIRE_THROWS(multiSlsDetectorClient("dbitphase", GET));
//     } else {
//         int prev_val = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("dbitphase
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase 20", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "dbitphase 20\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "dbitphase 0\n");
//         }
//         if (test::type == slsDetectorDefs::GOTTHARD) {
//             REQUIRE_THROWS(multiSlsDetectorClient("dbitphase deg", GET));
//         } else {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase 20 deg", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase deg", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "dbitphase 20 deg\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase " +
//         std::to_string(prev_val), PUT));
//     }
// }

// TEST_CASE("romode", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("romode digital", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "romode digital\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("romode analog_digital",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "romode
//             analog_digital\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("romode analog", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "romode analog\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("romode", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "romode analog\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("romode", GET));
//     }
// }

// TEST_CASE("samples", "[.cmd][.ctb]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         uint64_t prev_value1 = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("asamples", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("asamples
//             ")); prev_value1 = std::stoi(s);
//         }
//         std::cout<<"asamples:"<<prev_value1<<std::endl;
//         uint64_t prev_value2 = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("dsamples
//             ")); prev_value2 = std::stoi(s);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("samples 1200", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "samples 1200\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("samples 1000", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "samples 1000\n");
//         }
//          {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("asamples 2200", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "asamples 2200\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("asamples 4000", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "asamples 4000\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples 1200", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "dsamples 1200\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples 1000", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "dsamples 1000\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("samples", GET));   //
//         different values REQUIRE_NOTHROW(multiSlsDetectorClient("asamples " +
//         std::to_string(prev_value1), PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples " +
//         std::to_string(prev_value2), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("samples", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("asamples", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("dsamples", GET));
//     }
// }

// TEST_CASE("imagetest", "[.cmd][.gotthard]") {
//     if (test::type == slsDetectorDefs::GOTTHARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("imagetest 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "imagetest 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("imagetest 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "imagetest 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("imagetest", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "imagetest 0\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("imagetest", GET));
//     }
// }

// TEST_CASE("extsig", "[.cmd][.gotthard]") {
//     if (test::type == slsDetectorDefs::GOTTHARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsig
//             trigger_in_falling_edge", PUT, nullptr, oss)); REQUIRE(oss.str()
//             == "extsig trigger_in_falling_edge\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsig
//             trigger_in_rising_edge", PUT, nullptr, oss)); REQUIRE(oss.str()
//             == "extsig trigger_in_rising_edge\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("extsig", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "extsig trigger_in_rising_edge\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("extsig gating", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("extsig", GET));
//     }
// }

// TEST_CASE("exptimel", "[.cmd][.gotthard]") {
//     if (test::type == slsDetectorDefs::GOTTHARD) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("exptime 5 s", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:exptimel s", GET,
//             nullptr, oss)); std::string st = oss.str(); std::string s =
//             st.erase (0, strlen("exptimel ")); double val = std::stod(s);
//             REQUIRE(val >= 0);
//             REQUIRE(val < 1000);
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("exptime 1 ms", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("exptimel", GET));
//     }
// }

// TEST_CASE("periodl", "[.cmd][.gotthard]") {
//     if (test::type == slsDetectorDefs::GOTTHARD || test::type ==
//     slsDetectorDefs::JUNGFRAU || test::type ==
//     slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 2", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("period 5", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:periodl s", GET,
//             nullptr, oss)); std::string st = oss.str(); std::string s =
//             st.erase (0, strlen("periodl ")); double val = std::stod(s);
//             REQUIRE(val >= 0);
//             REQUIRE(val < 1000);
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("period 1 s", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("periodl", GET));
//     }
// }

// TEST_CASE("roi", "[.cmd][.gotthard]") {
//     if (test::type == slsDetectorDefs::GOTTHARD) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("roi 0 255", PUT, nullptr,
//             oss)); REQUIRE(oss.str() == "roi [0, 255] \n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("roi 256 511", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "roi [256, 511] \n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("clearroi", PUT, nullptr,
//             oss)); REQUIRE(oss.str() == "clearroi [-1, -1] \n");
//         }
//     REQUIRE_THROWS(multiSlsDetectorClient("roi 0 256", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("roi", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("clearroi", PUT));
//     }
// }

// TEST_CASE("storagecell_delay", "[.cmd][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_delay 1.62ms",
//             PUT, nullptr, oss)); REQUIRE(oss.str() ==
//             "storagecell_delay 1.62ms\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_delay", GET,
//             nullptr, oss)); REQUIRE(oss.str() ==
//             "storagecell_delay 1.62ms\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_delay 0",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "storagecell_delay
//             0\n");
//         }
//     REQUIRE_THROWS(multiSlsDetectorClient("storagecell_delay 1638376ns",
//     PUT)); } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("storagecell_delay", GET));
//     }
// }

// TEST_CASE("storagecell_start", "[.cmd][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start 1",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "storagecell_start
//             1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start 0",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "storagecell_start
//             0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start 15",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "storagecell_start
//             15\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "storagecell_start 15\n");
//         }
//     REQUIRE_THROWS(multiSlsDetectorClient("storagecell_start 16", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("storagecell_start", GET));
//     }
// }

// TEST_CASE("storagecells", "[.cmd][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "storagecells 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells 15", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "storagecells 15\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "storagecells 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "storagecells 0\n");
//         }
//     REQUIRE_THROWS(multiSlsDetectorClient("storagecells 16", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("storagecells", GET));
//     }
// }

// TEST_CASE("auto_comp_disable", "[.cmd][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("auto_comp_disable 1",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "auto_comp_disable
//             1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("auto_comp_disable 0",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "auto_comp_disable
//             0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("auto_comp_disable", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "auto_comp_disable 0\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("auto_comp_disable", GET));
//     }
// }

// TEST_CASE("temp_", "[.cmd][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         std::string s;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_threshold", GET,
//             nullptr, oss)); s = oss.str();
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
//             REQUIRE(oss.str() == s);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("temp_control 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "temp_control 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("temp_control 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "temp_control 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("temp_control", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "temp_control 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("temp_event", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "temp_event 0\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("temp_event 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "temp_event cleared\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_event 1", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_threshold", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_control", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_event", GET));
//     }
// }

// TEST_CASE("pulse", "[.cmd][.eiger]") {
//     REQUIRE_THROWS(multiSlsDetectorClient("pulse", GET));
//     REQUIRE_THROWS(multiSlsDetectorClient("pulsenmove", GET));
//     REQUIRE_THROWS(multiSlsDetectorClient("pulsechip", GET));
//     if (test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("pulse 1 1 5", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("pulsenmove 1 1 5", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("pulsechip 1", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("pulse 1 1 5", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("pulsenmove 1 1 5", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("pulsechip 1", PUT));
//     }
// }

// TEST_CASE("partialreset", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "partialreset 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "partialreset 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "partialreset 0\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 0", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("partialreset", GET));
//     }
// }

// TEST_CASE("measuredsubperiod", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("dr 32", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
//         sleep(3);
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:measuredsubperiod ms",
//             GET, nullptr, oss)); std::string st = oss.str(); std::string s =
//             st.erase (0, strlen("measuredsubperiod ")); double val =
//             std::stod(s); REQUIRE(val >= 0); REQUIRE(val < 1000);
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("dr 16", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("measuredsubperiod", GET));
//     }
// }

// TEST_CASE("measuredperiod", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 2", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("period 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
//         sleep(3);
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:measuredperiod", GET,
//             nullptr, oss)); std::string st = oss.str(); std::string s =
//             st.erase (0, strlen("measuredperiod ")); double val =
//             std::stod(s); REQUIRE(val >= 1.0); REQUIRE(val < 2.0);
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("measuredperiod", GET));
//     }
// }

// TEST_CASE("readnlines", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines 256", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "readnlines 256\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "readnlines 256\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines 16", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "readnlines 16\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("readnlines 0", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines 256", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("readnlines", GET));
//     }
// }

// TEST_CASE("ratecorr", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr 120", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "ratecorr 120ns\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "ratecorr 120ns\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "ratecorr 0ns\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr -1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr 0", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("ratecorr", GET));
//     }
// }



// TEST_CASE("trimval", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("trimval 63", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "trimval 63\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("trimval", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "trimval 63\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("trimval 31", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "trimval 31\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("trimval 0", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("trimval", GET));
//     }
// }

// TEST_CASE("flippeddatax", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:flippeddatax", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "flippeddatax 0\n");
//         }
//         DetectorImpl d;
//         if (d.size() > 1) {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("1:flippeddatax", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "flippeddatax 1\n");
//         }
//     } else {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("flippeddatax", GET, nullptr,
//         oss)); REQUIRE(oss.str() == "flippeddatax 0\n");
//     }
// }



// TEST_CASE("parallel", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("parallel 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "parallel 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("parallel", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "parallel 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("parallel 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "parallel 0\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("parallel", GET));
//     }
// }

// TEST_CASE("gappixels", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "gappixels 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "gappixels 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels 0", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "gappixels 0\n");
//         }
//     } else {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels", GET, nullptr,
//         oss)); REQUIRE(oss.str() == "gappixels 0\n");
//         REQUIRE_THROWS(multiSlsDetectorClient("gappixels 1", PUT));
//     }
// }

// TEST_CASE("settingspath", "[.cmd][.eiger]") {
//     std::string s;
//     std::ostringstream oss;
//     REQUIRE_NOTHROW(multiSlsDetectorClient("settingspath", GET, nullptr,
//     oss)); s = oss.str(); REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
// }

TEST_CASE("zmqip", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss1, oss2;
    auto zmqip = det.getClientZmqIp();
    proxy.Call("zmqip", {}, 0, GET, oss1);
    REQUIRE(oss1.str() == "zmqip " + zmqip[0].str() + '\n');

    proxy.Call("zmqip", {zmqip[0].str()}, 0, PUT, oss2);
    REQUIRE(oss2.str() == "zmqip " + zmqip[0].str() + '\n');

    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqIP(zmqip[i], {i});
    }
}

TEST_CASE("zmqport", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    int socketsperdetector = 1;
    auto det_type = det.getDetectorType().squash();
    int prev = 1;
    if (det_type == slsDetectorDefs::EIGER) {
        socketsperdetector *= 2;
    } else if (det_type == slsDetectorDefs::JUNGFRAU) {
        prev = det.getNumberofUDPInterfaces().squash();
        det.setNumberofUDPInterfaces(2);
        socketsperdetector *= 2;
    }
    int port = 3500;
    auto port_str = std::to_string(port);
    {
        std::ostringstream oss;
        proxy.Call("zmqport", {port_str}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqport " + port_str + '\n');
    }
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "zmqport " + std::to_string(port + i * socketsperdetector) + '\n');
    }

    port = 1954;
    port_str = std::to_string(port);
    {
        std::ostringstream oss;
        proxy.Call("zmqport", {port_str}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqport " + port_str + '\n');
    }
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "zmqport " + std::to_string(port + i * socketsperdetector) + '\n');
    }
    if (det_type == slsDetectorDefs::JUNGFRAU) {
        det.setNumberofUDPInterfaces(prev);
    }
}


TEST_CASE("fpath", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto fpath = det.getFilePath().squash();

    std::ostringstream oss1, oss2, oss3;
    proxy.Call("fpath", {}, -1, GET, oss1);
    REQUIRE(oss1.str() == "fpath " + fpath + "\n");
    proxy.Call("fpath", {fpath}, -1, PUT, oss2);
    REQUIRE(oss2.str() == "fpath " + fpath + "\n");
    proxy.Call("fpath", {}, -1, GET, oss3);
    REQUIRE(oss3.str() == "fpath " + fpath + "\n");
}

TEST_CASE("fformat", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto fformat = det.getFileFormat();
    {
        std::ostringstream oss;
        proxy.Call("fformat", {"binary"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fformat binary\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fformat", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fformat binary\n");
    }

    // Reset file format after test
    for (int i = 0; i != det.size(); ++i) {
        det.setFileFormat(fformat[i], {i});
    }
}

// TEST_CASE("txndelay", "[.cmd][.eiger][.jungfrau]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame 50000",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_frame 50000\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_frame 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_frame 0\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_left 50000",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_left", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_left 50000\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_left 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_left", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_left 0\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_right 50000",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_right", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_right 50000\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_right 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_right", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_right 0\n");
//         }
//     } else if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_frame 5", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_frame 5\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_frame 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "txndelay_frame 0\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("txndelay_frame 32", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("txndelay_left 32", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("txndelay_right 32", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("txndelay_frame", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("txndelay_left 32", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("txndelay_right 32", PUT));
//     }
// }

// TEST_CASE("flowcontrol_10g", "[.cmd][.eiger][.jungfrau]") {
//     if (test::type == slsDetectorDefs::EIGER || test::type ==
//     slsDetectorDefs::JUNGFRAU) {
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("flowcontrol_10g 1",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:flowcontrol_10g", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "flowcontrol_10g 1\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("flowcontrol_10g 0",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:flowcontrol_10g", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "flowcontrol_10g 0\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("flowcontrol_10g", GET));
//     }
// }

// TEST_CASE("network", "[.cmd]") {
//     /* {TODO custom srcip in globals
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip 129.129.205.203",
//         PUT)); std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip", GET, nullptr,
//         oss)); REQUIRE(oss.str() == "udp_srcip 129.129.205.203\n");
//     }*/
//     std::string udp_dstip;
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip", GET, nullptr,
//         oss)); udp_dstip = oss.str();
//     }
//     {
//         REQUIRE_NOTHROW(multiSlsDetectorClient(udp_dstip, PUT));
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip", GET, nullptr,
//         oss)); REQUIRE(oss.str() == udp_dstip);
//     }
//     /* {TODO custom dstip in globals
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac
//         10:e7:c6:48:bd:3f", PUT)); std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac", GET, nullptr,
//         oss)); REQUIRE(oss.str() == "udp_dstmac 10:e7:c6:48:bd:3f\n");
//     }  */
//     {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport 6200", PUT));
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport", GET, nullptr,
//         oss)); REQUIRE(oss.str() == "udp_dstport 6200\n");
//     }
//     {
//         DetectorImpl d;
//         int socketsperdetector = 1;
//         if (test::type == slsDetectorDefs::EIGER) {
//             socketsperdetector *= 2;
//         } else if (test::type == slsDetectorDefs::JUNGFRAU) {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 2", PUT));
//             socketsperdetector *= 2;
//         }
//         int port = 5500;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport " +
//         std::to_string(port), PUT)); for (int i = 0; i != d.size(); ++i) {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) +
//             ":udp_dstport", GET, nullptr, oss)); REQUIRE(oss.str() ==
//             "udp_dstport " + std::to_string(port + i * socketsperdetector) +
//             '\n');
//         }
//         port = 50001;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport " +
//         std::to_string(port), PUT)); for (int i = 0; i != d.size(); ++i) {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) +
//             ":udp_dstport", GET, nullptr, oss)); REQUIRE(oss.str() ==
//             "udp_dstport " + std::to_string(port + i * socketsperdetector) +
//             '\n');
//         }
//         if (test::type == slsDetectorDefs::JUNGFRAU) {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport 50001", PUT));
//     }
//     REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 0.0.0.0", PUT));
//     REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 124586954", PUT));
//     REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 999.999.0.0.0.5", PUT));

//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//        /*  {TODO custom srcip2 in globals
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip2
//             129.129.205.203", PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip2", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "udp_srcip2
//             129.129.205.203\n");
//        }*/
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip2", GET,
//             nullptr, oss)); udp_dstip = oss.str();
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient(udp_dstip, PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip2", GET,
//             nullptr, oss)); REQUIRE(oss.str() == udp_dstip);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac2
//             10:e7:c6:48:bd:3f", PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac2", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "udp_dstmac2
//             10:e7:c6:48:bd:3f\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2 6400",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "udp_dstport2 6400\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport2 50002", PUT));
//     } else if (test::type == slsDetectorDefs::EIGER) {
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2 6400",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "udp_dstport2 6400\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport2 50002", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("udp_dstip2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("udp_srcmac2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("udp_dstmac2", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("udp_dstport2", GET));
//     }
// }

// TEST_CASE("selinterface", "[.cmd][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("selinterface 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:selinterface", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "selinterface 0\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("selinterface 1", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:selinterface", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "selinterface 1\n");
//         }
//        REQUIRE_THROWS(multiSlsDetectorClient("selinterface 2", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("selinterface", GET));
//     }
// }

// TEST_CASE("numinterfaces", "[.cmd][.jungfrau]") {
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 2", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:numinterfaces", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "numinterfaces 2\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:numinterfaces", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "numinterfaces 1\n");
//         }
//     } else {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:numinterfaces", GET,
//         nullptr, oss)); REQUIRE(oss.str() == "numinterfaces 1\n");
//     }
//     REQUIRE_THROWS(multiSlsDetectorClient("numinterfaces 3", PUT));
//     REQUIRE_THROWS(multiSlsDetectorClient("numinterfaces 0", PUT));
// }

TEST_CASE("timing", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    det.setTimingMode(defs::AUTO_TIMING); // start in auto
    {
        std::ostringstream oss1, oss2;
        proxy.Call("timing", {"auto"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "timing auto\n");
        proxy.Call("timing", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "timing auto\n");
    }
    {
        std::ostringstream oss1, oss2;
        proxy.Call("timing", {"trigger"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "timing trigger\n");
        proxy.Call("timing", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "timing trigger\n");
    }
    auto det_type = det.getDetectorType().squash();
    if (det_type == slsDetectorDefs::EIGER) {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing gating\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing gating\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"burst_trigger"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing burst_trigger\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing burst_trigger\n");
        }
    } else {
        REQUIRE_THROWS(proxy.Call("timing", {"gating"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("timing", {"burst_trigger"}, -1, PUT));
    }
    det.setTimingMode(defs::AUTO_TIMING); // reset to auto
}

// TEST_CASE("adc", "[.cmd][.ctb]") {
//     if (test::type != slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_THROWS(multiSlsDetectorClient("adc 8", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("adc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("adc 5", PUT));
//         for(int i = 0; i <= 8; ++i) {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adc " +
//             std::to_string(i), GET));
//         }
//     }
// }

// TEST_CASE("temp_fpga", "[.cmd][.eiger][.jungfrau][.gotthard]") {
//     if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpga", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpga 0", PUT));
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpga", GET, nullptr,
//         oss)); std::string s = (oss.str()).erase (0, strlen("temp_fpga "));
//         REQUIRE(std::stoi(s) != -1);
//     }
// }

// TEST_CASE("temp_adc", "[.cmd][.jungfrau][.gotthard]") {
//     if (test::type != slsDetectorDefs::GOTTHARD && test::type !=
//     slsDetectorDefs::JUNGFRAU ) {
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_adc", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_adc 0", PUT));
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_adc", GET, nullptr,
//         oss)); std::string s = (oss.str()).erase (0, strlen("temp_adc "));
//         REQUIRE(std::stoi(s) != -1);
//     }
// }

// TEST_CASE("temp", "[.cmd][.eiger]") {
//     if (test::type != slsDetectorDefs::EIGER) {
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgaext", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_10ge", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_dcdc", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_sodl", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_sodr", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafl", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafr", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgaext 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_10ge 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_dcdc 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_sodl 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_sodr 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafl 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafr 0", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpgaext", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("temp_fpgaext ")); REQUIRE(std::stoi(s) != -1);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_10ge", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("temp_10ge ")); REQUIRE(std::stoi(s) != -1);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_dcdc", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("temp_dcdc ")); REQUIRE(std::stoi(s) != -1);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_sodl", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("temp_sodl ")); REQUIRE(std::stoi(s) != -1);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_sodr", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("temp_sodr ")); REQUIRE(std::stoi(s) != -1);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpgafl", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("temp_fpgafl ")); REQUIRE(std::stoi(s) != -1);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpgafr", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("temp_fpgafr ")); REQUIRE(std::stoi(s) != -1);
//         }
//     }
// }

// TEST_CASE("vhighvoltage", "[.cmd]") {
//     if (test::type == slsDetectorDefs::GOTTHARD) {
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 90", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "vhighvoltage 90\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "vhighvoltage 0\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("vhighvoltage 50", PUT));
//     } else if (test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 50", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 120", PUT));
//         sleep(2);
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:vhighvoltage", GET,
//         nullptr, oss)); REQUIRE(oss.str() == "vhighvoltage 120\n");
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
//             sleep(2);
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:vhighvoltage", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "vhighvoltage 0\n");
//         }
//     } else {
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 50", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "vhighvoltage 50\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 120", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "vhighvoltage 120\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "vhighvoltage 0\n");
//         }
//     }
// }

// TEST_CASE("maxadcphaseshift", "[.cmd][.ctb][.jungfrau]") {
//     if (test::type != slsDetectorDefs::CHIPTESTBOARD && test::type !=
//     slsDetectorDefs::JUNGFRAU) {
//        REQUIRE_THROWS(multiSlsDetectorClient("maxadcphaseshift", GET));
//     } else {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("maxadcphaseshift", GET));
//     }
// }

// TEST_CASE("adcphase", "[.cmd][.ctb][.jungfrau][.gotthard]") {

//     if (test::type == slsDetectorDefs::GOTTHARD) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 120", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 0", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("adcphase 120 deg", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("adcphase", GET));
//         // get is -1
//     } else if (test::type == slsDetectorDefs::CHIPTESTBOARD || test::type ==
//     slsDetectorDefs::JUNGFRAU) {
//         int prev_val = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("adcphase
//             ")); prev_val = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 20", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "adcphase 20\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "adcphase 0\n");
//         }
//         if (test::type == slsDetectorDefs::GOTTHARD) {
//             REQUIRE_THROWS(multiSlsDetectorClient("adcphase deg", GET));
//         } else {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 20 deg", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase deg", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "adcphase 20 deg\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase " +
//         std::to_string(prev_val), PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("adcphase", GET));
//     }
// }

// TEST_CASE("syncclk", "[.cmd][.ctb]") {
//     if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
//         REQUIRE_THROWS(multiSlsDetectorClient("syncclk", GET));
//     } else {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("syncclk", GET));
//         REQUIRE_THROWS(multiSlsDetectorClient("syncclk 40", PUT));
//     }
// }

// TEST_CASE("adcclk", "[.cmd][.ctb]") {
//     if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
//        REQUIRE_THROWS(multiSlsDetectorClient("adcclk", GET));
//     } else {
//         int prev_clk = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("adcclk "));
//             prev_clk = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk 20", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "adcclk 20\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk 10", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "adcclk 10\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk " +
//         std::to_string(prev_clk), PUT));
//     }
// }

// TEST_CASE("dbitclk", "[.cmd][.ctb]") {
//     if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
//        REQUIRE_THROWS(multiSlsDetectorClient("dbitclk", GET));
//     } else {
//         int prev_clk = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("dbitclk "));
//             prev_clk = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk 20", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "dbitclk 20\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk 10", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "dbitclk 10\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk " +
//         std::to_string(prev_clk), PUT));
//     }
// }

// TEST_CASE("runclk", "[.cmd][.ctb]") {
//     if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
//        REQUIRE_THROWS(multiSlsDetectorClient("runclk", GET));
//     } else {
//         int prev_runclk = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("runclk", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("runclk "));
//             prev_runclk = std::stoi(s);
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("runclk 20", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("runclk", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "runclk 20\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("runclk 10", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("runclk", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "runclk 10\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("runclk " +
//         std::to_string(prev_runclk), PUT));
//     }
// }

// TEST_CASE("speed", "[.cmd][.eiger][.jungfrau]") {
//     if(test::type != slsDetectorDefs::EIGER && test::type !=
//     slsDetectorDefs::JUNGFRAU) {
//         REQUIRE_THROWS(multiSlsDetectorClient("speed", GET));
//     } else {
//         /*{TODO : only for new jungfrau boards
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed 0", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "speed full_speed\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed full_speed", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "speed full_speed\n");
//         }*/
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed 1", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "speed half_speed\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed half_speed", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "speed half_speed\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed 2", PUT));
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "speed quarter_speed\n");
//         }
//         {
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed quarter_speed",
//             PUT)); std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "speed quarter_speed\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("speed 3", PUT));
//     }
// }

// TEST_CASE("triggers", "[.cmd]") {
//     {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 10", PUT));
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("triggers", GET, nullptr,
//         oss)); REQUIRE(oss.str() == "triggers 10\n");
//     }
//     REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 1", PUT));
// }

// TEST_CASE("settings", "[.cmd]") {
//     switch(test::type) {
//         case slsDetectorDefs::EIGER:
//             REQUIRE_THROWS(multiSlsDetectorClient("settings mediumgain",
//             PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("settings
//             standard", PUT)); break;

//         case slsDetectorDefs::JUNGFRAU:
//             REQUIRE_THROWS(multiSlsDetectorClient("settings standard", PUT));
//             REQUIRE_NOTHROW(multiSlsDetectorClient("settings dynamicgain",
//             PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("settings
//             dynamichg0", PUT));
//             REQUIRE_NOTHROW(multiSlsDetectorClient("settings fixgain1",
//             PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("settings
//             fixgain2", PUT));
//             REQUIRE_NOTHROW(multiSlsDetectorClient("settings forceswitchg1",
//             PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("settings
//             forceswitchg2", PUT)); break;

//         case slsDetectorDefs::GOTTHARD:
//             REQUIRE_NOTHROW(multiSlsDetectorClient("settings dynamicgain",
//             PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("settings
//             highgain", PUT));
//             REQUIRE_NOTHROW(multiSlsDetectorClient("settings lowgain", PUT));
//             REQUIRE_NOTHROW(multiSlsDetectorClient("settings mediumgain",
//             PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("settings
//             veryhighgain", PUT)); break;

//         default:
//             REQUIRE_THROWS(multiSlsDetectorClient("settings", GET));
//             break;
//     }
// }

// TEST_CASE("threshold", "[.cmd]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("threshold 6400 standard",
//         PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("thresholdnotb 6400
//         standard", PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("threshold
//         6400", PUT)); REQUIRE_NOTHROW(multiSlsDetectorClient("thresholdnotb
//         6400", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("threshold", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("threshold
//             ")); REQUIRE(std::stoi(s) == 6400);
//             REQUIRE_THROWS(multiSlsDetectorClient("thresholdnotb", GET));
//         }
//     } else {
//          REQUIRE_THROWS(multiSlsDetectorClient("threshold", GET));
//          REQUIRE_THROWS(multiSlsDetectorClient("thresholdnotb", GET));
//     }
// }

// TEST_CASE("detsize", "[.cmd]") {
//     CHECK_NOTHROW(multiSlsDetectorClient("detsize", GET));
// }

// TEST_CASE("type", "[.cmd]") {
//     CHECK_NOTHROW(multiSlsDetectorClient("type", GET));
// }

TEST_CASE("firmwareversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    proxy.Call("firmwareversion", {}, -1, GET, oss);
    REQUIRE_THROWS(proxy.Call("firmwareversion", {"4"}, -1, PUT, oss));
}
// TEST_CASE("status", "[.cmd]") {
//     Detector det;
//     CmdProxy proxy(&det);

//     proxy.Call("timing", {"auto"}, -1, PUT);
//     proxy.Call("frames", {"10"}, -1, PUT);
//     proxy.Call("period", {"1"}, -1, PUT);

//     {
//         std::ostringstream oss;
//         proxy.Call("start", {}, -1, PUT, oss);
//         REQUIRE(oss.str() == "start successful\n");
//     }
//     {
//         std::ostringstream oss;
//         proxy.Call("status", {}, -1, GET, oss);
//         REQUIRE(oss.str() == "status running\n");
//     }
//     {
//         std::ostringstream oss;
//         proxy.Call("stop", {}, -1, PUT, oss);
//         REQUIRE(oss.str() == "stop successful\n");
//     }
// {
//     std::ostringstream oss;
//     REQUIRE_NOTHROW(multiSlsDetectorClient("status", GET, nullptr, oss));
//     REQUIRE(oss.str() != "status running\n");
//     REQUIRE(oss.str() != "status waiting\n");
//     REQUIRE(oss.str() != "status transmitting\n");
// }
// REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
// proxy.Call("frames", {"1"}, -1, PUT);
// }

// TEST_CASE("framesl", "[.cmd][.jungfrau][gotthard][ctb]") {
//     if(test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_THROWS(multiSlsDetectorClient("framesl", GET));
//     } else {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 10", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("period 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("framesl", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("framesl "));
//             int framesl = std::stoi(s);
//             REQUIRE(framesl > 0);
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//     }
// }

// TEST_CASE("triggersl", "[.cmd][.jungfrau][gotthard][ctb]") {
//     if(test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_THROWS(multiSlsDetectorClient("triggersl", GET));
//     } else {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 10", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("triggersl", GET, nullptr,
//             oss)); std::string s = (oss.str()).erase (0, strlen("triggersl
//             ")); int triggersl = std::stoi(s); REQUIRE(triggersl == 8);
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 1", PUT));
//     }
// }

// TEST_CASE("delayl", "[.cmd][.jungfrau][gotthard][ctb]") {
//     if(test::type == slsDetectorDefs::EIGER) {
//         REQUIRE_THROWS(multiSlsDetectorClient("delayl", GET));
//     } else  if(test::type == slsDetectorDefs::GOTTHARD) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("delay 0", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("delayl", GET));
//         // delayl always gives 0 for gotthard
//     } else {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 2", PUT));
//          REQUIRE_NOTHROW(multiSlsDetectorClient("delay 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("delayl s", GET, nullptr,
//             oss)); REQUIRE(oss.str()  == "delayl 1s\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("delay 0", PUT));
//     }
// }


TEST_CASE("frames", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("frames", {"1000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }

    {
        std::ostringstream oss;
        proxy.Call("frames", {}, -1, GET, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("frames", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "frames 1\n");
    }
}

TEST_CASE("fwrite", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("fwrite", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fwrite", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fwrite", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fwrite 0\n");
    }
}

TEST_CASE("foverwrite", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("foverwrite", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("foverwrite", {}, -1, GET, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("foverwrite", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "foverwrite 0\n");
    }
}

TEST_CASE("fmaster", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("fmaster", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fmaster", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fmaster", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fmaster 1\n");
    }
}

TEST_CASE("findex", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("findex", {"57"}, -1, PUT, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("findex", {}, -1, GET, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {

        std::ostringstream oss;
        proxy.Call("findex", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "findex 0\n");
    }
}

TEST_CASE("fname", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("fname", {"somename"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fname", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fname", {"run"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fname run\n");
    }
}

TEST_CASE("lock", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("lock", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("lock", {}, -1, GET, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("lock", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "lock 0\n");
    }
}

// TEST_CASE("lastclient", "[.cmd]") {
//     REQUIRE_NOTHROW(multiSlsDetectorClient("lastclient", GET));
// }

TEST_CASE("exptime", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    {
        std::ostringstream oss;
        proxy.Call("exptime", {"0.05"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 0.05\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("exptime", {}, -1, GET, oss);
        REQUIRE(oss.str() == "exptime 50ms\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("exptime", {"1s"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 1s\n");
    }
}

TEST_CASE("period", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("period", {"1.25s"}, -1, PUT, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("period", {}, -1, GET, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("period", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "period 0\n");
    }
}

TEST_CASE("delay", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("delay", {"1"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("delay", {}, -1, GET));
    } else {
        {
            std::ostringstream oss;
            proxy.Call("delay", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("delay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("delay", {"0s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 0s\n");
        }
    }
}