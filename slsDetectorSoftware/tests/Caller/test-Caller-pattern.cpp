// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/Result.h"
#include "sls/ToString.h"
#include "sls/versionAPI.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/* Pattern */

TEST_CASE("patfname", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(caller.call("patfname", {}, -1, PUT));
        REQUIRE_NOTHROW(caller.call("patfname", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("patfname", {}, -1, GET));
    }
}

TEST_CASE("pattern", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        // no proper test for put
        REQUIRE_THROWS(caller.call("pattern", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("pattern", {}, -1, GET));
    }
}

TEST_CASE("savepattern", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(
            caller.call("savepattern", {"/tmp/pattern.txt"}, -1, GET));
        if (det.size() == 1) {
            REQUIRE_NOTHROW(
                caller.call("savepattern", {"/tmp/pattern.txt"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(
            caller.call("savepattern", {"/tmp/pattern.txt"}, -1, PUT));
    }
}

TEST_CASE("defaultpattern", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(caller.call("defaultpattern", {}, -1, GET));
        REQUIRE_NOTHROW(caller.call("defaultpattern", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("defaultpattern", {}, -1, GET));
        REQUIRE_THROWS(caller.call("defaultpattern", {}, -1, PUT));
    }
}

TEST_CASE("patioctrl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getPatternIOControl();
        if (det_type == defs::CHIPTESTBOARD) {
            std::ostringstream oss;
            caller.call("patioctrl", {"0xc15004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patioctrl 0xc15004808d0a21a4\n");
        }
        {
            std::ostringstream oss;
            caller.call("patioctrl", {"0xaadf0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patioctrl 0x00000000000aadf0\n");
        }
        {
            std::ostringstream oss;
            caller.call("patioctrl", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patioctrl 0x00000000000aadf0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternIOControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("patioctrl", {}, -1, GET));
    }
}

TEST_CASE("patword", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        int addr = 0x23;
        std::string saddr = ToStringHex(addr, 4);
        auto prev_val = det.getPatternWord(addr);
        {
            std::ostringstream oss;
            caller.call("patword", {saddr, "0xc15004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0xc15004808d0a21a4]\n");
        }
        {
            std::ostringstream oss;
            caller.call("patword", {saddr, "0x815004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0x815004808d0a21a4]\n");
        }
        {
            std::ostringstream oss;
            caller.call("patword", {saddr, "0xaadf0"}, -1, PUT, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0x00000000000aadf0]\n");
        }
        {
            std::ostringstream oss;
            caller.call("patword", {saddr}, -1, GET, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0x00000000000aadf0]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWord(addr, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("patword", {"0x23"}, -1, GET));
    }
}

TEST_CASE("patlimits", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopAddresses(-1);
        {
            std::ostringstream oss;
            caller.call("patlimits", {"0x20", "0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patlimits [0x0020, 0x005c]\n");
        }
        {
            std::ostringstream oss;
            caller.call("patlimits", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patlimits [0x0020, 0x005c]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopAddresses(-1, prev_val[i][0], prev_val[i][1],
                                        {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("patlimits", {}, -1, GET));
    }
}

TEST_CASE("patloop", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternLoopAddresses(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patloop" + sLoop;
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {"0x20", "0x5c"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " [0x0020, 0x005c]\n");
                }
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " [0x0020, 0x005c]\n");
                }
            }
            {
                std::ostringstream oss;
                caller.call("patloop", {sLoop, "0x20", "0x5c"}, -1, PUT, oss);
                REQUIRE(oss.str() ==
                        "patloop " + sLoop + " [0x0020, 0x005c]\n");
            }
            {
                std::ostringstream oss;
                caller.call("patloop", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() ==
                        "patloop " + sLoop + " [0x0020, 0x005c]\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternLoopAddresses(iLoop, prev_val[iDet][0],
                                            prev_val[iDet][1], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("patloop", {"0"}, -1, GET));
    }
}

TEST_CASE("patnloop", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternLoopCycles(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patnloop" + sLoop;
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {"5"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 5\n");
                }
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 5\n");
                }
            }
            {
                std::ostringstream oss;
                caller.call("patnloop", {sLoop, "5"}, -1, PUT, oss);
                REQUIRE(oss.str() == "patnloop " + sLoop + " 5\n");
            }
            {
                std::ostringstream oss;
                caller.call("patnloop", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() == "patnloop " + sLoop + " 5\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternLoopCycles(iLoop, prev_val[iDet], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("patnloop", {"0"}, -1, GET));
    }
}

TEST_CASE("patwait", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternWaitAddr(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patwait" + sLoop;
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {"0x5c"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 0x005c\n");
                }
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 0x005c\n");
                }
            }
            {
                std::ostringstream oss;
                caller.call("patwait", {sLoop, "0x5c"}, -1, PUT, oss);
                REQUIRE(oss.str() == "patwait " + sLoop + " 0x005c\n");
            }
            {
                std::ostringstream oss;
                caller.call("patwait", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() == "patwait " + sLoop + " 0x005c\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternWaitAddr(iLoop, prev_val[iDet], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("patwait", {"0"}, -1, GET));
    }
}

TEST_CASE("patwaittime", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        for (int iLoop = 0; iLoop != MAX_PATTERN_LEVELS; ++iLoop) {
            // m3 only has 3 levels
            if (det_type == defs::MYTHEN3 && iLoop >= 3) {
                continue;
            }
            auto prev_val = det.getPatternWaitTime(iLoop);
            std::string sLoop = ToString(iLoop);
            if (iLoop < 3) {
                std::string deprecatedCmd = "patwaittime" + sLoop;
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {"8589936640"}, -1, PUT, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 8589936640\n");
                }
                { // deprecated
                    std::ostringstream oss;
                    caller.call(deprecatedCmd, {}, -1, GET, oss);
                    REQUIRE(oss.str() == deprecatedCmd + " 8589936640\n");
                }
            }
            {
                std::ostringstream oss;
                caller.call("patwaittime", {sLoop, "8589936640"}, -1, PUT, oss);
                REQUIRE(oss.str() == "patwaittime " + sLoop + " 8589936640\n");
            }
            {
                std::ostringstream oss;
                caller.call("patwaittime", {sLoop}, -1, GET, oss);
                REQUIRE(oss.str() == "patwaittime " + sLoop + " 8589936640\n");
            }
            for (int iDet = 0; iDet != det.size(); ++iDet) {
                det.setPatternWaitTime(iLoop, prev_val[iDet], {iDet});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("patwaittime", {"0"}, -1, GET));
    }
}

TEST_CASE("patmask", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternMask();
        {
            std::ostringstream oss;
            caller.call("patmask", {"0x842f020204200dc0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patmask 0x842f020204200dc0\n");
        }
        {
            std::ostringstream oss;
            caller.call("patmask", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patmask 0x842f020204200dc0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("patmask", {}, -1, GET));
    }
}

TEST_CASE("patsetbit", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternBitMask();
        {
            std::ostringstream oss;
            caller.call("patsetbit", {"0x842f020204200dc0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patsetbit 0x842f020204200dc0\n");
        }
        {
            std::ostringstream oss;
            caller.call("patsetbit", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patsetbit 0x842f020204200dc0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternBitMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("patsetbit", {}, -1, GET));
    }
}

TEST_CASE("patternstart", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("patternstart", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_NOTHROW(caller.call("patternstart", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("patternstart", {}, -1, PUT));
    }
}

} // namespace sls
