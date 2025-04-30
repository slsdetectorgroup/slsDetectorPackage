// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/versionAPI.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

TEST_CASE("moench_acquire_check_file_size", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::MOENCH) {

        // save previous state
        testFileInfo prev_file_info = get_file_state(det);
        testCommonDetAcquireInfo prev_det_config_info =
            get_common_acquire_config_state(det);

        // save previous specific det type config
        auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");
        auto n_rows =
            det.getReadNRows().tsquash("inconsistent number of rows to test");

        // defaults
        int num_frames_to_acquire = 2;
        testFileInfo test_file_info;
        set_file_state(det, test_file_info);
        testCommonDetAcquireInfo det_config;
        det_config.num_frames_to_acquire = num_frames_to_acquire;
        set_common_acquire_config_state(det, det_config);

        // set default specific det type config
        det.setReadNRows(400);

        // acquire
        test_acquire_with_receiver(caller, std::chrono::seconds{2});

        // check frames caught
        test_frames_caught(det, num_frames_to_acquire);

        // check file size (assuming local pc)
        size_t expected_image_size = 0;
        // pixels_row * pixels_col * bytes_per_pixel
        if (num_udp_interfaces == 1) {
            expected_image_size = 400 * 400 * 2;
        } else {
            expected_image_size = 400 * 200 * 2;
        }
        test_acquire_binary_file_size(test_file_info, num_frames_to_acquire,
                                      expected_image_size);

        // restore previous state
        set_file_state(det, prev_file_info);
        set_common_acquire_config_state(det, prev_det_config_info);

        // restore previous specific det type config
        det.setReadNRows(n_rows);
    }
}

/* dacs */

TEST_CASE("Setting and reading back moench dacs", "[.cmdcall][.dacs]") {
    // vbp_colbuf, vipre, vin_cm, vb_sda, vcasc_sfp, vout_cm, vipre_cds,
    // ibias_sfp
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MOENCH) {
        SECTION("vbp_colbuf") {
            test_dac_caller(defs::VBP_COLBUF, "vbp_colbuf", 1300);
        }
        SECTION("vipre") { test_dac_caller(defs::VIPRE, "vipre", 1000); }
        SECTION("vin_cm") { test_dac_caller(defs::VIN_CM, "vin_cm", 1400); }
        SECTION("vb_sda") { test_dac_caller(defs::VB_SDA, "vb_sda", 680); }
        SECTION("vcasc_sfp") {
            test_dac_caller(defs::VCASC_SFP, "vcasc_sfp", 1428);
        }
        SECTION("vout_cm") { test_dac_caller(defs::VOUT_CM, "vout_cm", 1200); }
        SECTION("vipre_cds") {
            test_dac_caller(defs::VIPRE_CDS, "vipre_cds", 800);
        }
        SECTION("ibias_sfp") {
            test_dac_caller(defs::IBIAS_SFP, "ibias_sfp", 900);
        }
        // eiger
        REQUIRE_THROWS(caller.call("vthreshold", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vsvp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vsvn", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrpreamp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vtgstv", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_ll", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_lr", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_rl", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_rr", {}, -1, GET));
        REQUIRE_THROWS(caller.call("rxb_rb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("rxb_lb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcn", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vishaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("iodelay", {}, -1, GET));
        // mythen3
        REQUIRE_THROWS(caller.call("vrpreamp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrshaper_n", {}, -1, GET));
        // REQUIRE_THROWS(caller.call("vipre", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vishaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vdcsh", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth1", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth2", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth3", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal_n", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal_p", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcassh", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcas", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vicin", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vipre_out", {}, -1, GET));
        // gotthard2
        REQUIRE_THROWS(caller.call("vref_h_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_comp_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_cds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_rstore", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_opa_1st", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_adc1", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_l_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_cds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_cs", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_opa_fd", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_adc2", {}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(caller.call("vb_comp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vdd_prot", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vin_com", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_prech", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_pixbuf", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_ds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_comp", {}, -1, GET));
    }
}

} // namespace sls
