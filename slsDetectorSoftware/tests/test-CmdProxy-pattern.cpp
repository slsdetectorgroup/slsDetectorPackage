// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/Result.h"
#include "sls/ToString.h"
#include "sls/versionAPI.h"
#include "test-CmdProxy-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/* Pattern */

TEST_CASE("pattern", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        // no proper test for put
        REQUIRE_THROWS(proxy.Call("pattern", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("pattern", {}, -1, GET));
    }
}

TEST_CASE("savepattern", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(
            proxy.Call("savepattern", {"/tmp/pattern.txt"}, -1, GET));
        if (det.size() == 1) {
            REQUIRE_NOTHROW(
                proxy.Call("savepattern", {"/tmp/pattern.txt"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(
            proxy.Call("savepattern", {"/tmp/pattern.txt"}, -1, PUT));
    }
}

TEST_CASE("defaultpattern", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(proxy.Call("defaultpattern", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("defaultpattern", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("defaultpattern", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("defaultpattern", {}, -1, PUT));
    }
}

TEST_CASE("patioctrl", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getPatternIOControl();
        {
            std::ostringstream oss;
            proxy.Call("patioctrl", {"0xc15004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patioctrl 0xc15004808d0a21a4\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patioctrl", {"0xaadf0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patioctrl 0x00000000000aadf0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patioctrl", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patioctrl 0x00000000000aadf0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternIOControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patioctrl", {}, -1, GET));
    }
}

TEST_CASE("patword", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        int addr = 0x23;
        std::string saddr = ToStringHex(addr, 4);
        auto prev_val = det.getPatternWord(addr);
        {
            std::ostringstream oss;
            proxy.Call("patword", {saddr, "0xc15004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0xc15004808d0a21a4]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patword", {saddr, "0xaadf0"}, -1, PUT, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0x00000000000aadf0]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patword", {saddr}, -1, GET, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0x00000000000aadf0]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWord(addr, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patword", {"0x23"}, -1, GET));
    }
}

TEST_CASE("patlimits", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopAddresses(-1);
        {
            std::ostringstream oss;
            proxy.Call("patlimits", {"0x20", "0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patlimits [0x0020, 0x005c]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patlimits", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patlimits [0x0020, 0x005c]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopAddresses(-1, prev_val[i][0], prev_val[i][1],
                                        {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patlimits", {}, -1, GET));
    }
}

TEST_CASE("patloop", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternLoopAddresses(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patloop" + sLoop;
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {"0x20", "0x5c"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " [0x0020, 0x005c]\n");
                }
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " [0x0020, 0x005c]\n");
                }
            }
            {
                std::ostringstream oss;
                proxy.Call("patloop", {sLoop, "0x20", "0x5c"}, -1, PUT, oss);
                REQUIRE(oss.str() ==
                        "patloop " + sLoop + " [0x0020, 0x005c]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("patloop", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() ==
                        "patloop " + sLoop + " [0x0020, 0x005c]\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternLoopAddresses(iLoop, prev_val[iDet][0],
                                            prev_val[iDet][1], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patloop", {"0"}, -1, GET));
    }
}

TEST_CASE("patnloop", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternLoopCycles(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patnloop" + sLoop;
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {"5"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 5\n");
                }
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 5\n");
                }
            }
            {
                std::ostringstream oss;
                proxy.Call("patnloop", {sLoop, "5"}, -1, PUT, oss);
                REQUIRE(oss.str() == "patnloop " + sLoop + " 5\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("patnloop", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() == "patnloop " + sLoop + " 5\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternLoopCycles(iLoop, prev_val[iDet], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patnloop", {"0"}, -1, GET));
    }
}

TEST_CASE("patwait", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternWaitAddr(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patwait" + sLoop;
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {"0x5c"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 0x005c\n");
                }
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 0x005c\n");
                }
            }
            {
                std::ostringstream oss;
                proxy.Call("patwait", {sLoop, "0x5c"}, -1, PUT, oss);
                REQUIRE(oss.str() == "patwait " + sLoop + " 0x005c\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("patwait", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() == "patwait " + sLoop + " 0x005c\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternWaitAddr(iLoop, prev_val[iDet], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwait", {"0"}, -1, GET));
    }
}

TEST_CASE("patwaittime", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternWaitTime(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patwaittime" + sLoop;
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {"8589936640"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 8589936640\n");
                }
                { // depreciated
                    std::ostringstream oss;
                    proxy.Call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 8589936640\n");
                }
            }
            {
                std::ostringstream oss;
                proxy.Call("patwaittime", {sLoop, "8589936640"}, -1, PUT, oss);
                REQUIRE(oss.str() == "patwaittime " + sLoop + " 8589936640\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("patwaittime", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() == "patwaittime " + sLoop + " 8589936640\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternWaitTime(iLoop, prev_val[iDet], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwaittime", {"0"}, -1, GET));
    }
}

TEST_CASE("patmask", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternMask();
        {
            std::ostringstream oss;
            proxy.Call("patmask", {"0x842f020204200dc0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patmask 0x842f020204200dc0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patmask", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patmask 0x842f020204200dc0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patmask", {}, -1, GET));
    }
}

TEST_CASE("patsetbit", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternBitMask();
        {
            std::ostringstream oss;
            proxy.Call("patsetbit", {"0x842f020204200dc0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patsetbit 0x842f020204200dc0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patsetbit", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patsetbit 0x842f020204200dc0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternBitMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patsetbit", {}, -1, GET));
    }
}

TEST_CASE("patternstart", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("patternstart", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_NOTHROW(proxy.Call("patternstart", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("patternstart", {}, -1, PUT));
    }
}

} // namespace sls
