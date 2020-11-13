#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"

#include <chrono>
#include <sstream>
#include <thread>

#include "tests/globals.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

TEST_CASE("Unknown command", "[.cmd]") {

    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("vsaevrreavv", {}, -1, PUT));
}

/* configuration */

TEST_CASE("config", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    // put only
    REQUIRE_THROWS(proxy.Call("config", {}, -1, GET));
}

// free: not testing

TEST_CASE("parameters", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    // put only
    REQUIRE_THROWS(proxy.Call("parameters", {}, -1, GET));
    /*
        auto prev_val = det.getNumberOfFrames().tsquash("Number of frames has to
       be same to test");
        {
            system("echo 'frames 2' > /tmp/tempsetup.det ");
            std::ostringstream oss;
            proxy.Call("parameters", {"/tmp/tempsetup.det"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parameters /tmp/tempsetup.det\n");
            REQUIRE(det.getNumberOfFrames().tsquash("failed") == 2);
        }
        {
            system("echo '0:frames 1' > /tmp/tempsetup.det ");
            std::ostringstream oss;
            proxy.Call("parameters", {"/tmp/tempsetup.det"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parameters /tmp/tempsetup.det\n");
            REQUIRE(det.getNumberOfFrames({0}).tsquash("failed") == 1);
        }
        det.setNumberOfFrames(prev_val);
        */
}

TEST_CASE("hostname", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("hostname", {}, -1, GET));
}

// virtual: not testing

TEST_CASE("versions", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("versions", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("versions", {"0"}, -1, PUT));
}

TEST_CASE("packageversion", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("packageversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("packageversion", {"0"}, -1, PUT));
}

TEST_CASE("clientversion", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("clientversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("clientversion", {"0"}, -1, PUT));
}

TEST_CASE("firmwareversion", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("firmwareversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("firmwareversion", {"0"}, -1, PUT));
}

TEST_CASE("detectorserverversion", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("detectorserverversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("detectorserverversion", {"0"}, -1, PUT));
}

TEST_CASE("serialnumber", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("serialnumber", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("serialnumber", {"0"}, -1, PUT));
}

TEST_CASE("type", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto dt = det.getDetectorType().squash();

    std::ostringstream oss;
    proxy.Call("type", {}, -1, GET, oss);
    auto ans = oss.str().erase(0, strlen("type "));
    REQUIRE(ans == sls::ToString(dt) + '\n');
    // REQUIRE(dt == test::type);
}

TEST_CASE("detsize", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("detsize", {}, -1, GET));
}

TEST_CASE("settingslist", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(proxy.Call("settingslist", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(proxy.Call("settingslist", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("settingslist", {}, -1, PUT));
    }
}

TEST_CASE("settings", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    std::vector<std::string> sett;
    switch (det_type) {
    case defs::JUNGFRAU:
        sett.push_back("dynamicgain");
        sett.push_back("dynamichg0");
        sett.push_back("fixgain1");
        sett.push_back("fixgain2");
        sett.push_back("forceswitchg1");
        sett.push_back("forceswitchg2");
        break;
    case defs::GOTTHARD:
        sett.push_back("highgain");
        sett.push_back("dynamicgain");
        sett.push_back("lowgain");
        sett.push_back("mediumgain");
        sett.push_back("veryhighgain");
        break;
    case defs::GOTTHARD2:
        sett.push_back("dynamicgain");
        sett.push_back("fixgain1");
        sett.push_back("fixgain2");
        break;
    case defs::MOENCH:
        sett.push_back("g1_hg");
        sett.push_back("g1_lg");
        sett.push_back("g2_hc_hg");
        sett.push_back("g2_hc_lg");
        sett.push_back("g2_lc_hg");
        sett.push_back("g2_lc_lg");
        sett.push_back("g4_hg");
        sett.push_back("g4_lg");
        break;
    default:
        if (det_type == defs::EIGER) {
            // FIXME: need to remove when settings removed
            REQUIRE_NOTHROW(proxy.Call("settings", {}, -1, GET));
            REQUIRE_THROWS(proxy.Call("settings", {"standard"}, -1, PUT));
        } else {
            REQUIRE_THROWS(proxy.Call("settings", {}, -1, GET));
        }
        return;
    }

    auto prev_val = det.getSettings();
    for (auto &it : sett) {
        {
            std::ostringstream oss;
            proxy.Call("settings", {it}, -1, PUT, oss);
            REQUIRE(oss.str() == "settings " + it + "\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("settings", {}, -1, GET, oss);
            REQUIRE(oss.str() == "settings " + it + "\n");
        }
    }
    for (int i = 0; i != det.size(); ++i) {
        if (prev_val[i] != defs::UNDEFINED &&
            prev_val[i] != defs::UNINITIALIZED) {
            det.setSettings(prev_val[i], {i});
        }
    }
}

TEST_CASE("trimbits", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("trimbits", {}, -1, GET));
}

TEST_CASE("trimval", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::MYTHEN3 || det_type == defs::EIGER) {
        auto prev_val = det.getAllTrimbits();
        {
            std::ostringstream oss;
            proxy.Call("trimval", {"63"}, -1, PUT, oss);
            REQUIRE(oss.str() == "trimval 63\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("trimval", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "trimval 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("trimval", {}, -1, GET, oss);
            REQUIRE(oss.str() == "trimval 0\n");
        }
        REQUIRE_THROWS(proxy.Call("trimval", {"64"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("trimval", {"-2"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] != -1) {
                det.setAllTrimbits(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("trimval", {}, -1, GET));
    }
}

/* acquisition parameters */

// acquire: not testing

TEST_CASE("frames", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val =
        det.getNumberOfFrames().tsquash("#frames must be same to test");
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
    REQUIRE_THROWS(proxy.Call("frames", {"0"}, -1, PUT));
    det.setNumberOfFrames(prev_val);
}

TEST_CASE("triggers", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val =
        det.getNumberOfTriggers().tsquash("#triggers must be same to test");
    {
        std::ostringstream oss;
        proxy.Call("triggers", {"1000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "triggers 1000\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("triggers", {}, -1, GET, oss);
        REQUIRE(oss.str() == "triggers 1000\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("triggers", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "triggers 1\n");
    }
    REQUIRE_THROWS(proxy.Call("triggers", {"0"}, -1, PUT));
    det.setNumberOfTriggers(prev_val);
}

TEST_CASE("exptime", "[.cmd][.time]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw sls::RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    {
        std::ostringstream oss;
        proxy.Call("exptime", {"0.05"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 0.05\n");
    }
    if (det_type != defs::MYTHEN3) {
        std::ostringstream oss;
        proxy.Call("exptime", {}, -1, GET, oss);
        REQUIRE(oss.str() == "exptime 50ms\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("exptime", {"1s"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 1s\n");
    }
    if (det_type != defs::JUNGFRAU) {
        {
            std::ostringstream oss;
            proxy.Call("exptime", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime 0\n");
        }
        {
            // Get exptime of single module
            std::ostringstream oss;
            proxy.Call("exptime", {}, 0, GET, oss);
            REQUIRE(oss.str() == "exptime 0ns\n");
        }
    }
    det.setExptime(-1, prev_val);
}

TEST_CASE("period", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getPeriod();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setPeriod(prev_val[i], {i});
    }
}

TEST_CASE("delay", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("delay", {"1"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("delay", {}, -1, GET));
    } else if (det_type == defs::GOTTHARD) {
        // extra delays for master (can throw when setting)
        REQUIRE_NOTHROW(proxy.Call("delay", {}, -1, GET));
    } else {
        auto prev_val = det.getDelayAfterTrigger();
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
        for (int i = 0; i != det.size(); ++i) {
            det.setDelayAfterTrigger(prev_val[i], {i});
        }
    }
}

TEST_CASE("framesl", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("framesl", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(proxy.Call("framesl", {}, -1, GET));
    }
}

TEST_CASE("triggersl", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("triggersl", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(proxy.Call("triggersl", {}, -1, GET));
    }
}

TEST_CASE("delayl", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::EIGER:
    case defs::CHIPTESTBOARD:
    case defs::MOENCH:
    case defs::GOTTHARD2:
    case defs::MYTHEN3:
        REQUIRE_THROWS(proxy.Call("delayl", {}, -1, GET));
        break;
    default:
        REQUIRE_NOTHROW(proxy.Call("delayl", {}, -1, GET));
        break;
    }
}

TEST_CASE("periodl", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::EIGER:
    case defs::CHIPTESTBOARD:
    case defs::MOENCH:
    case defs::GOTTHARD2:
    case defs::MYTHEN3:
        REQUIRE_THROWS(proxy.Call("periodl", {}, -1, GET));
        break;
    default:
        REQUIRE_NOTHROW(proxy.Call("periodl", {}, -1, GET));
        break;
    }
}

TEST_CASE("dr", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto dr = det.getDynamicRange().squash();
        std::array<int, 4> vals{4, 8, 16, 32};
        for (const auto val : vals) {
            std::ostringstream oss1, oss2;
            proxy.Call("dr", {std::to_string(val)}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dr " + std::to_string(val) + '\n');
            proxy.Call("dr", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dr " + std::to_string(val) + '\n');
        }
        det.setDynamicRange(dr);
    } else if (det_type == defs::MYTHEN3) {
        auto dr = det.getDynamicRange().squash();
        // not updated in firmware to support dr 1
        std::array<int, 3> vals{8, 16, 32};
        for (const auto val : vals) {
            std::ostringstream oss1, oss2;
            proxy.Call("dr", {std::to_string(val)}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dr " + std::to_string(val) + '\n');
            proxy.Call("dr", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dr " + std::to_string(val) + '\n');
        }
        det.setDynamicRange(dr);
    } else {
        // For the other detectors we should get an error message
        // except for dr 16
        REQUIRE_THROWS(proxy.Call("dr", {"4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dr", {"8"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dr", {"32"}, -1, PUT));

        std::ostringstream oss1, oss2;
        proxy.Call("dr", {"16"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "dr 16\n");
        proxy.Call("dr", {"16"}, -1, PUT, oss2);
        REQUIRE(oss2.str() == "dr 16\n");
    }
}

TEST_CASE("drlist", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("drlist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("drlist", {}, -1, PUT));
}

TEST_CASE("timing", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getTimingMode();
    det.setTimingMode(defs::AUTO_TIMING);
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
    if (det_type == defs::EIGER) {
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
        REQUIRE_THROWS(proxy.Call("timing", {"trigger_gating"}, -1, PUT));
    } else if (det_type == defs::MYTHEN3) {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing gating\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing gating\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"trigger_gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing trigger_gating\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing trigger_gating\n");
        }
        REQUIRE_THROWS(proxy.Call("timing", {"burst_trigger"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("timing", {"gating"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("timing", {"burst_trigger"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("timing", {"trigger_gating"}, -1, PUT));
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setTimingMode(prev_val[i], {i});
    }
}

TEST_CASE("timinglist", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("timinglist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("timinglist", {}, -1, PUT));
}

TEST_CASE("speed", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        auto prev_val = det.getSpeed();
        // full speed for jungfrau only works for new boards
        /*
        {
            std::ostringstream oss1, oss2, oss3, oss4;
            proxy.Call("speed", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "speed full_speed\n");
            proxy.Call("speed", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "speed full_speed\n");
            proxy.Call("speed", {"full_speed"}, -1, PUT, oss3);
            REQUIRE(oss3.str() == "speed full_speed\n");
            proxy.Call("speed", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "speed full_speed\n");
        }
        */
        {
            std::ostringstream oss1, oss2, oss3, oss4;
            proxy.Call("speed", {"1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "speed half_speed\n");
            proxy.Call("speed", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "speed half_speed\n");
            proxy.Call("speed", {"half_speed"}, -1, PUT, oss3);
            REQUIRE(oss3.str() == "speed half_speed\n");
            proxy.Call("speed", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "speed half_speed\n");
        }
        {
            std::ostringstream oss1, oss2, oss3, oss4;
            proxy.Call("speed", {"2"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "speed quarter_speed\n");
            proxy.Call("speed", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "speed quarter_speed\n");
            proxy.Call("speed", {"quarter_speed"}, -1, PUT, oss3);
            REQUIRE(oss3.str() == "speed quarter_speed\n");
            proxy.Call("speed", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "speed quarter_speed\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setSpeed(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("speed", {"0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("speed", {}, -1, GET));
    }
}

TEST_CASE("adcphase", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD || det_type == defs::JUNGFRAU ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        if (det_type == defs::GOTTHARD) {
            std::ostringstream oss1;
            proxy.Call("adcphase", {"20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "adcphase 20\n");
            // cant get, cant use deg
            REQUIRE_THROWS(proxy.Call("adcphase", {}, -1, GET));
            REQUIRE_THROWS(proxy.Call("adcphase", {"20", "deg"}, -1, PUT));
        } else {
            auto prev_val = det.getADCPhase();
            {
                std::ostringstream oss1, oss2;
                proxy.Call("adcphase", {"20"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "adcphase 20\n");
                proxy.Call("adcphase", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "adcphase 20\n");
            }
            {
                std::ostringstream oss1, oss2;
                proxy.Call("adcphase", {"20", "deg"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "adcphase 20 deg\n");
                proxy.Call("adcphase", {"deg"}, -1, GET, oss2);
                REQUIRE(oss2.str() == "adcphase 20 deg\n");
            }
            for (int i = 0; i != det.size(); ++i) {
                det.setADCPhase(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adcphase", {"0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("adcphase", {}, -1, GET));
    }
}

TEST_CASE("maxadcphaseshift", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3 ||   // only because clk index of 0 exists
        det_type == defs::GOTTHARD2) { // only because clk index of 0 exists
        REQUIRE_NOTHROW(proxy.Call("maxadcphaseshift", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("maxadcphaseshift", {}, -1, GET));
    }
}

TEST_CASE("dbitphase", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getDBITPhase();
        {
            std::ostringstream oss1, oss2;
            proxy.Call("dbitphase", {"20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dbitphase 20\n");
            proxy.Call("dbitphase", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dbitphase 20\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("dbitphase", {"20", "deg"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dbitphase 20 deg\n");
            proxy.Call("dbitphase", {"deg"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dbitphase 20 deg\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDBITPhase(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("dbitphase", {"0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dbitphase", {}, -1, GET));
    }
}

TEST_CASE("maxdbitphaseshift", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MYTHEN3 ||   // only because clk index of 0 exists
        det_type == defs::GOTTHARD2) { // only because clk index of 0 exists
        REQUIRE_NOTHROW(proxy.Call("maxdbitphaseshift", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("maxdbitphaseshift", {}, -1, GET));
    }
}

TEST_CASE("clkfreq", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("clkfreq", {"0", "2"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkfreq", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkfreq", {"7"}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("clkfreq", {"0"}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("clkfreq", {"0"}, -1, GET));
    }
}

TEST_CASE("clkphase", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("clkphase", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkphase", {"7"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkphase", {"4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkphase", {"7", "4"}, -1, PUT));
        auto prev_val = det.getClockPhase(0);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("clkphase", {"0", "20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkphase 20\n");
            proxy.Call("clkphase", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkphase 20\n");
        }
        std::string s_deg_val = "15";
        if (det_type == defs::MYTHEN3) {
            s_deg_val = "14";
        } else if (det_type == defs::GOTTHARD2) {
            s_deg_val = "23";
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("clkphase", {"0", s_deg_val, "deg"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkphase " + s_deg_val + " deg\n");
            proxy.Call("clkphase", {"0", "deg"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkphase " + s_deg_val + " deg\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setClockPhase(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("clkphase", {"0"}, -1, GET));
    }
}

TEST_CASE("clkdiv", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("clkdiv", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"7"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"7", "4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"7", "4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"0", "1"}, -1, PUT));
        auto prev_val = det.getClockDivider(0);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("clkdiv", {"0", "3"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkdiv 3\n");
            proxy.Call("clkdiv", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkdiv 3\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setClockDivider(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("clkdiv", {"0"}, -1, GET));
    }
}

TEST_CASE("maxclkphaseshift", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {"0", "2"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {"7"}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("maxclkphaseshift", {"0"}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {"0"}, -1, GET));
    }
}

TEST_CASE("highvoltage", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    auto prev_val = det.getHighVoltage();
    // selected values
    if (det_type == defs::GOTTHARD) {
        REQUIRE_THROWS(proxy.Call("highvoltage", {"50"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"90"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 90\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 90\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    // range 0, 60 - 200
    else if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
             det_type == defs::MOENCH) {
        REQUIRE_THROWS(proxy.Call("highvoltage", {"50"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"90"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 90\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 90\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    // full range 0 - 200 (get needs to wait)
    else if (det_type == defs::EIGER) {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"50"}, 0, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 50\n");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            proxy.Call("highvoltage", {}, 0, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 50\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"120"}, 0, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 120\n");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            proxy.Call("highvoltage", {}, 0, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 120\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, 0, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            proxy.Call("highvoltage", {}, 0, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    // full range 0 - 200
    else {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"50"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 50\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 50\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"120"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 120\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 120\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setHighVoltage(prev_val[i], {i});
    }
}

TEST_CASE("powerchip", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::MOENCH) {
        auto prev_val = det.getPowerChip();
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {}, -1, GET, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPowerChip(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("powerchip", {}, -1, GET));
    }
}

TEST_CASE("imagetest", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    // cannot test only for virtual eiger/jungfrau
    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getImageTestMode();
        {
            std::ostringstream oss1, oss2;
            proxy.Call("imagetest", {"1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "imagetest 1\n");
            proxy.Call("imagetest", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "imagetest 1\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("imagetest", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "imagetest 0\n");
            proxy.Call("imagetest", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "imagetest 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setImageTestMode(prev_val[i], {i});
        }
    } else if (det_type != defs::JUNGFRAU && det_type != defs::EIGER) {
        // wont fail for eiger and jungfrau virtual servers
        REQUIRE_THROWS(proxy.Call("imagetest", {}, -1, GET));
    }
}

TEST_CASE("extsig", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getExternalSignalFlags(0);
        REQUIRE_THROWS(proxy.Call("extsig", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"1"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_on"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_off"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_rising_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_rising_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_rising_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_falling_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_falling_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_falling_edge\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSignalFlags(0, prev_val[i], {i});
        }
    } else if (det_type == defs::MYTHEN3) {
        auto prev_val_0 = det.getExternalSignalFlags(0);
        auto prev_val_1 = det.getExternalSignalFlags(1);
        REQUIRE_THROWS(proxy.Call("extsig", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"8"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_on"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_off"}, -1, PUT));
        REQUIRE_THROWS(
            proxy.Call("extsig", {"1", "trigger_in_rising_edge"}, -1, PUT));
        REQUIRE_THROWS(
            proxy.Call("extsig", {"1", "trigger_in_falling_edge"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_rising_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_rising_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_rising_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_falling_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_falling_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_falling_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"1", "inversion_off"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "extsig 1 inversion_off\n");
            proxy.Call("extsig", {"1"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 1 inversion_off\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"1", "inversion_on"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "extsig 1 inversion_on\n");
            proxy.Call("extsig", {"1"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 1 inversion_on\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSignalFlags(0, prev_val_0[i], {i});
            det.setExternalSignalFlags(1, prev_val_1[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("extsig", {}, -1, GET));
    }
}

TEST_CASE("parallel", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::MYTHEN3) {
        auto prev_val = det.getParallelMode();
        {
            std::ostringstream oss;
            proxy.Call("parallel", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parallel 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("parallel", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parallel 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("parallel", {}, -1, GET, oss);
            REQUIRE(oss.str() == "parallel 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setParallelMode(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("parallel", {}, -1, GET));
    }
}

/** temperature */

TEST_CASE("templist", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("templist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("templist", {}, -1, PUT));
}

TEST_CASE("tempvalues", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("tempvalues", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("tempvalues", {}, -1, PUT));
}

TEST_CASE("temp_adc", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD) {
        REQUIRE_NOTHROW(proxy.Call("temp_adc", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(proxy.Call("temp_adc", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_adc "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(proxy.Call("temp_adc", {}, -1, GET));
    }
}

TEST_CASE("temp_fpga", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD ||
        det_type == defs::EIGER) {
        REQUIRE_NOTHROW(proxy.Call("temp_fpga", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(proxy.Call("temp_fpga", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_fpga "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(proxy.Call("temp_fpga", {}, -1, GET));
    }
}

/* dacs */

TEST_CASE("daclist", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("daclist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("daclist", {}, -1, PUT));
}

TEST_CASE("dacvalues", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("dacvalues", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("dacvalues", {}, -1, PUT));
}

TEST_CASE("defaultdacs", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD) {
        REQUIRE_THROWS(proxy.Call("defaultdacs", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("defaultdacs", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("defaultdacs", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("defaultdacs", {}, -1, PUT));
    }
}

/* acquisition */

TEST_CASE("trigger", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("trigger", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER && det_type != defs::MYTHEN3) {
        REQUIRE_THROWS(proxy.Call("trigger", {}, -1, PUT));
    } else if (det_type == defs::MYTHEN3) {
        REQUIRE_NOTHROW(proxy.Call("trigger", {}, -1, PUT));
    } else if (det_type == defs::EIGER) {
        auto prev_timing =
            det.getTimingMode().tsquash("inconsistent timing mode in test");
        auto prev_frames =
            det.getNumberOfFrames().tsquash("inconsistent #frames in test");
        auto prev_exptime =
            det.getExptime().tsquash("inconsistent exptime in test");
        auto prev_period =
            det.getPeriod().tsquash("inconsistent period in test");
        det.setTimingMode(defs::TRIGGER_EXPOSURE);
        det.setNumberOfFrames(1);
        det.setExptime(std::chrono::milliseconds(1));
        det.setPeriod(std::chrono::milliseconds(1));
        auto startframenumber = det.getStartingFrameNumber().tsquash(
            "inconsistent frame nr in test");
        det.startDetector();
        {
            std::ostringstream oss;
            proxy.Call("trigger", {}, -1, PUT, oss);
            REQUIRE(oss.str() == "trigger successful\n");
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
        auto currentfnum = det.getStartingFrameNumber().tsquash(
            "inconsistent frame nr in test");
        REQUIRE(startframenumber + 1 == currentfnum);
        det.stopDetector();
        det.setTimingMode(prev_timing);
        det.setNumberOfFrames(prev_frames);
    }
}

TEST_CASE("clearbusy", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("clearbusy", {}, -1, PUT));
    REQUIRE_THROWS(proxy.Call("clearbusy", {"0"}, -1, PUT));
    REQUIRE_THROWS(proxy.Call("clearbusy", {}, -1, GET));
}

TEST_CASE("start", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    // PUT only command
    REQUIRE_THROWS(proxy.Call("start", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw sls::RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    det.setExptime(-1, std::chrono::seconds(2));
    {
        std::ostringstream oss;
        proxy.Call("start", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "start successful\n");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    det.stopDetector();
    det.setExptime(-1, prev_val);
}

TEST_CASE("stop", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    // PUT only command
    REQUIRE_THROWS(proxy.Call("stop", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw sls::RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    det.setExptime(-1, std::chrono::seconds(2));
    det.startDetector();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("stop", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "stop successful\n");
    }
    if (det_type == defs::JUNGFRAU) {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status stopped\n");
    } else {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status idle\n");
    }
    det.setExptime(-1, prev_val);
}

TEST_CASE("status", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw sls::RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    det.setExptime(-1, std::chrono::seconds(2));
    det.startDetector();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    det.stopDetector();
    {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status idle\n");
    }
    det.setExptime(-1, prev_val);
}

TEST_CASE("startframenumber", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        auto prev_sfnum = det.getStartingFrameNumber();
        REQUIRE_THROWS(proxy.Call("startframenumber", {"0"}, -1, PUT));
        {
            std::ostringstream oss;
            proxy.Call("startframenumber", {"3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "startframenumber 3\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("startframenumber", {}, -1, GET, oss);
            REQUIRE(oss.str() == "startframenumber 3\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("startframenumber", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "startframenumber 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setStartingFrameNumber(prev_sfnum[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("startframenumber", {}, -1, GET));
    }
}

TEST_CASE("scan", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    defs::dacIndex ind = defs::DAC_0;
    defs::dacIndex notImplementedInd = defs::DAC_0;
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::CHIPTESTBOARD:
        ind = defs::DAC_0;
        notImplementedInd = defs::VSVP;
        break;
    case defs::EIGER:
        ind = defs::VCMP_LL;
        notImplementedInd = defs::VCASCP_PB;
        break;
    case defs::JUNGFRAU:
        ind = defs::VB_COMP;
        notImplementedInd = defs::VSVP;
        break;
    case defs::GOTTHARD:
        ind = defs::VREF_DS;
        notImplementedInd = defs::VSVP;
        break;
    case defs::MOENCH:
        ind = defs::VBP_COLBUF;
        notImplementedInd = defs::VSVP;
        break;
    case defs::GOTTHARD2:
        ind = defs::VB_COMP_FE;
        notImplementedInd = defs::VSVP;
        break;
    case defs::MYTHEN3:
        ind = defs::VTH2;
        notImplementedInd = defs::VSVP;
        break;
    default:
        break;
    }

    // when taking acquisition
    // auto previous = det.getDAC(ind, false);
    // auto notImplementedPrevious = det.getDAC(notImplementedInd, false);

    {
        std::ostringstream oss;
        proxy.Call("scan", {sls::ToString(ind), "500", "1500", "500"}, -1, PUT,
                   oss);
        CHECK(oss.str() ==
              "scan [" + sls::ToString(ind) + ", 500, 1500, 500]\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("scan", {}, -1, GET, oss);
        CHECK(oss.str() == "scan [enabled\ndac " + sls::ToString(ind) +
                               "\nstart 500\nstop 1500\nstep "
                               "500\nsettleTime 1ms\n]\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("scan", {sls::ToString(ind), "500", "1500", "500", "2s"}, -1,
                   PUT, oss);
        CHECK(oss.str() ==
              "scan [" + sls::ToString(ind) + ", 500, 1500, 500, 2s]\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("scan", {}, -1, GET, oss);
        CHECK(oss.str() == "scan [enabled\ndac " + sls::ToString(ind) +
                               "\nstart 500\nstop 1500\nstep "
                               "500\nsettleTime 2s\n]\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("scan", {"0"}, -1, PUT, oss);
        CHECK(oss.str() == "scan [0]\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("scan", {}, -1, GET, oss);
        CHECK(oss.str() == "scan [disabled]\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("scan", {sls::ToString(ind), "1500", "500", "-500"}, -1, PUT,
                   oss);
        CHECK(oss.str() ==
              "scan [" + sls::ToString(ind) + ", 1500, 500, -500]\n");
    }
    CHECK_THROWS(proxy.Call(
        "scan", {sls::ToString(notImplementedInd), "500", "1500", "500"}, -1,
        PUT));
    CHECK_THROWS(proxy.Call("scan", {sls::ToString(ind), "500", "1500", "-500"},
                            -1, PUT));
    CHECK_THROWS(proxy.Call("scan", {sls::ToString(ind), "1500", "500", "500"},
                            -1, PUT));

    if (det_type == defs::MYTHEN3 || defs::EIGER) {
        {
            std::ostringstream oss;
            proxy.Call("scan", {"trimbits", "0", "63", "16", "2s"}, -1, PUT,
                       oss);
            CHECK(oss.str() == "scan [trimbits, 0, 63, 16, 2s]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("scan", {}, -1, GET, oss);
            CHECK(oss.str() ==
                  "scan [enabled\ndac trimbits\nstart 0\nstop 48\nstep "
                  "16\nsettleTime 2s\n]\n");
        }
    }

    // Switch off scan for future tests
    det.setScan(defs::scanParameters());
    // acquire for each?

    // when taking acquisition
    // Reset all dacs to previous value
    // for (int i = 0; i != det.size(); ++i) {
    //     det.setDAC(ind, previous[i], false, {i});
    //     det.setDAC(notImplementedInd, notImplementedPrevious[i], false, {i});
    // }
}

TEST_CASE("scanerrmsg", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("scanerrmsg", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("scanerrmsg", {""}, -1, PUT));
}

TEST_CASE("gappixels", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::JUNGFRAU || det_type == defs::EIGER) {
        auto prev_val = det.getGapPixelsinCallback();
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        det.setGapPixelsinCallback(prev_val);
    } else {
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        REQUIRE_THROWS(proxy.Call("gappixels", {"1"}, -1, PUT));
    }
}

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("numinterfaces", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent numinterfaces to test");
        {
            std::ostringstream oss;
            proxy.Call("numinterfaces", {"2"}, -1, PUT, oss);
            REQUIRE(oss.str() == "numinterfaces 2\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("numinterfaces", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "numinterfaces 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("numinterfaces", {}, -1, GET, oss);
            REQUIRE(oss.str() == "numinterfaces 1\n");
        }
        det.setNumberofUDPInterfaces(prev_val);
    } else {
        std::ostringstream oss;
        proxy.Call("numinterfaces", {}, -1, GET, oss);
        REQUIRE(oss.str() == "numinterfaces 1\n");
        REQUIRE_THROWS(proxy.Call("numinterfaces", {"1"}, -1, PUT));
    }
    REQUIRE_THROWS(proxy.Call("numinterfaces", {"3"}, -1, PUT));
    REQUIRE_THROWS(proxy.Call("numinterfaces", {"0"}, -1, PUT));
}

TEST_CASE("udp_srcip", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getSourceUDPIP();
    REQUIRE_THROWS(proxy.Call("udp_srcip", {"0.0.0.0"}, -1, PUT));
    {
        std::ostringstream oss;
        proxy.Call("udp_srcip", {"129.129.205.12"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_srcip 129.129.205.12\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setSourceUDPIP(prev_val[i], {i});
    }
}

TEST_CASE("udp_dstip", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_dstip", {"0.0.0.0"}, -1, PUT));
}

TEST_CASE("udp_srcmac", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getSourceUDPMAC();
    REQUIRE_THROWS(proxy.Call("udp_srcmac", {"00:00:00:00:00:00"}, -1, PUT));
    {
        std::ostringstream oss;
        proxy.Call("udp_srcmac", {"00:50:c2:42:34:12"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_srcmac 00:50:c2:42:34:12\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setSourceUDPMAC(prev_val[i], {i});
    }
}

TEST_CASE("udp_dstmac", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_dstmac", {"00:00:00:00:00:00"}, -1, PUT));
}

TEST_CASE("udp_dstport", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getDestinationUDPPort();
    {
        std::ostringstream oss;
        proxy.Call("udp_dstport", {"50084"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_dstport 50084\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setDestinationUDPPort(prev_val[i], {i});
    }
}

TEST_CASE("udp_srcip2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getSourceUDPIP2();
        REQUIRE_THROWS(proxy.Call("udp_srcip2", {"0.0.0.0"}, -1, PUT));
        {
            std::ostringstream oss;
            proxy.Call("udp_srcip2", {"129.129.205.12"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_srcip2 129.129.205.12\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setSourceUDPIP2(prev_val[i], {i});
        }
    }
}

TEST_CASE("udp_dstip2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("udp_dstip2", {"0.0.0.0"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("udp_dstip2", {}, -1, GET));
    }
}

TEST_CASE("udp_srcmac2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getSourceUDPMAC2();
        REQUIRE_THROWS(
            proxy.Call("udp_srcmac2", {"00:00:00:00:00:00"}, -1, PUT));
        {
            std::ostringstream oss;
            proxy.Call("udp_srcmac2", {"00:50:c2:42:34:12"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_srcmac2 00:50:c2:42:34:12\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setSourceUDPMAC2(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("udp_srcmac2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstmac2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(
            proxy.Call("udp_dstmac2", {"00:00:00:00:00:00"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("udp_dstmac2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstport2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2 ||
        det_type == defs::EIGER) {
        auto prev_val = det.getDestinationUDPPort2();
        {
            std::ostringstream oss;
            proxy.Call("udp_dstport2", {"50084"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_dstport2 50084\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDestinationUDPPort2(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("udp_dstport2", {}, -1, GET));
    }
}

TEST_CASE("udp_reconfigure", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_reconfigure", {}, -1, GET));
    REQUIRE_NOTHROW(proxy.Call("udp_reconfigure", {}, -1, PUT));
}

TEST_CASE("udp_validate", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_validate", {}, -1, GET));
    REQUIRE_NOTHROW(proxy.Call("udp_validate", {}, -1, PUT));
}

TEST_CASE("tengiga", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3) {
        auto tengiga = det.getTenGiga();
        det.setTenGiga(false);

        std::ostringstream oss1, oss2;
        proxy.Call("tengiga", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "tengiga 1\n");
        proxy.Call("tengiga", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "tengiga 1\n");

        for (int i = 0; i != det.size(); ++i) {
            det.setTenGiga(tengiga[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("tengiga", {}, -1, GET));
    }
}

TEST_CASE("flowcontrol10g", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        auto prev_val = det.getTenGigaFlowControl();
        {
            std::ostringstream oss;
            proxy.Call("flowcontrol10g", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "flowcontrol10g 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("flowcontrol10g", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "flowcontrol10g 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("flowcontrol10g", {}, -1, GET, oss);
            REQUIRE(oss.str() == "flowcontrol10g 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTenGigaFlowControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("flowcontrol10g", {}, -1, GET));
    }
}

TEST_CASE("txndelay_frame", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getTransmissionDelayFrame();
        auto val = 5000;
        if (det_type == defs::JUNGFRAU) {
            val = 5;
        }
        std::string sval = std::to_string(val);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("txndelay_frame", {sval}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txndelay_frame " + sval + "\n");
            proxy.Call("txndelay_frame", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txndelay_frame " + sval + "\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTransmissionDelayFrame(prev_val[i]);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("txndelay_frame", {}, -1, GET));
    }
}

/* ZMQ Streaming Parameters (Receiver<->Client) */

TEST_CASE("zmqport", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);

    int socketsperdetector = 1;
    auto det_type = det.getDetectorType().squash();
    int prev = 1;
    if (det_type == defs::EIGER) {
        socketsperdetector *= 2;
    } else if (det_type == defs::JUNGFRAU) {
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
        REQUIRE(oss.str() == "zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
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
        REQUIRE(oss.str() == "zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }
    if (det_type == defs::JUNGFRAU) {
        det.setNumberofUDPInterfaces(prev);
    }
}

TEST_CASE("zmqip", "[.cmd][.new]") {
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

TEST_CASE("zmqhwm", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getClientZmqHwm();
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {"50"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {}, -1, GET, oss);
        REQUIRE(oss.str() == "zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {"-1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm -1\n");
    }
    det.setClientZmqHwm(prev_val);
}

/* Advanced */

TEST_CASE("programfpga", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        // TODO program a real board?
        /// afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof
        REQUIRE_THROWS(proxy.Call("programfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("programfpga", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("programfpga", {"/tmp/test.pof"}, -1, PUT));
    }
}

TEST_CASE("resetfpga", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH) {
        std::ostringstream oss;
        proxy.Call("resetfpga", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "resetfpga successful\n");
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, PUT));
    }
}

TEST_CASE("copydetectorserver", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        // TODO: send real server?
        // std::ostringstream oss;
        // proxy.Call("copydetectorserver",{"jungfrauDetectorServerv4.0.1.0",
        // "pc13784"}, -1, PUT, oss);
        // REQUIRE(oss.str() == "copydetectorserver successful\n");
        REQUIRE_THROWS(proxy.Call("copydetectorserver", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("copydetectorserver", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("copydetectorserver", {}, -1, PUT));
    }
}

TEST_CASE("rebootcontroller", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::GOTTHARD) {
        // TODO: reboot real server?
        // REQUIRE_NOTHROW(proxy.Call("rebootcontroller", {}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("rebootcontroller", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("rebootcontroller", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("rebootcontroller", {}, -1, PUT));
    }
}

TEST_CASE("update", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH) {
        // TODO: update real server and firmware?
        // REQUIRE_NOTHROW(proxy.Call("update",
        // {"jungfrauDetectorServerv4.0.1.0", "pc13784",
        // "/afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof"},
        // -1, PUT));
        REQUIRE_THROWS(proxy.Call("update", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("update", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("update", {}, -1, PUT));
    }
}

TEST_CASE("reg", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = sls::ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x5"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "reg [" + saddr + ", 0x5]\n");
            proxy.Call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x5\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
    // cannot check for eiger virtual server
    else {
        REQUIRE_NOTHROW(proxy.Call("reg", {"0x64"}, -1, GET));
    }
}

TEST_CASE("adcreg", "[.cmd]") {
    // TODO! what is a safe value to use?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("adcreg", {"0x8", "0x3"}, -1, PUT, oss);
        REQUIRE(oss.str() == "adcreg [0x8, 0x3]\n");
        // This is a put only command
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("adcreg", {"0x0", "0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    }
}

TEST_CASE("setbit", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = sls::ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x0"}, -1, PUT);
            proxy.Call("setbit", {saddr, "1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "setbit [" + saddr + ", 1]\n");
            proxy.Call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x2\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
}

TEST_CASE("clearbit", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = sls::ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x3"}, -1, PUT);
            proxy.Call("clearbit", {saddr, "1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clearbit [" + saddr + ", 1]\n");
            proxy.Call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
}

TEST_CASE("getbit", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = sls::ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x3"}, -1, PUT);
            proxy.Call("getbit", {saddr, "1"}, -1, GET, oss1);
            REQUIRE(oss1.str() == "getbit 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
    // cannot check for eiger virtual server
    else {
        REQUIRE_NOTHROW(proxy.Call("getbit", {"0x64", "1"}, -1, GET));
    }
}

TEST_CASE("firmwaretest", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::GOTTHARD ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("firmwaretest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "firmwaretest successful\n");
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, PUT));
    }
}

TEST_CASE("bustest", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::GOTTHARD ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("bustest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "bustest successful\n");
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, PUT));
    }
}

TEST_CASE("initialchecks", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto check = det.getInitialChecks();
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
        std::ostringstream oss;
        proxy.Call("initialchecks", {}, -1, GET, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    det.setInitialChecks(check);
}

TEST_CASE("adcinvert", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::JUNGFRAU) {
        auto prev_val = det.getADCInvert();
        {
            std::ostringstream oss;
            proxy.Call("adcinvert", {"0x8d0a21d4"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcinvert 0x8d0a21d4\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcinvert", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcinvert 0x8d0a21d4\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCInvert(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adcinvert", {}, -1, GET));
    }
}

/* Insignificant */

TEST_CASE("port", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getControlPort({0}).squash();
    {
        std::ostringstream oss;
        proxy.Call("port", {"1942"}, 0, PUT, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("port", {}, 0, GET, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    det.setControlPort(prev_val, {0});
}

TEST_CASE("stopport", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getStopPort({0}).squash();
    {
        std::ostringstream oss;
        proxy.Call("stopport", {"1942"}, 0, PUT, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("stopport", {}, 0, GET, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    det.setStopPort(prev_val, {0});
}

TEST_CASE("lock", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getDetectorLock();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setDetectorLock(prev_val[i], {i});
    }
}

TEST_CASE("execcommand", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("execcommand", {"ls"}, -1, PUT));
}

TEST_CASE("framecounter", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        auto framecounter = det.getNumberOfFramesFromStart().squash();
        std::ostringstream oss;
        proxy.Call("framecounter", {}, -1, GET, oss);
        REQUIRE(oss.str() ==
                "framecounter " + std::to_string(framecounter) + "\n");
        REQUIRE_NOTHROW(proxy.Call("framecounter", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("framecounter", {}, -1, GET));
    }
}

TEST_CASE("runtime", "[.cmd][.new]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("runtime", {}, -1, GET, oss);
        // Get only
        REQUIRE_THROWS(proxy.Call("runtime", {"2019"}, -1, PUT));
        REQUIRE_NOTHROW(proxy.Call("runtime", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("runtime", {}, -1, GET));
    }
}

TEST_CASE("frametime", "[.cmd][.new]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("frametime", {}, -1, GET, oss);
        // Get only
        REQUIRE_THROWS(proxy.Call("frametime", {"2019"}, -1, PUT));
        REQUIRE_NOTHROW(proxy.Call("frametime", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("frametime", {}, -1, GET));
    }
}

TEST_CASE("user", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("user", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(proxy.Call("user", {}, -1, PUT));
    REQUIRE_NOTHROW(proxy.Call("user", {}, -1, GET));
}
