#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "Result.h"
#include "ToString.h"
#include "test-CmdProxy-global.h"
#include "tests/globals.h"
#include "versionAPI.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

/* Pattern */

TEST_CASE("pattern", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        // no proper test for put
        REQUIRE_THROWS(proxy.Call("pattern", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("pattern", {}, -1, GET));
    }
}

TEST_CASE("savepattern", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(
            proxy.Call("savepattern", {"/tmp/pattern.txt"}, -1, GET));
        REQUIRE_NOTHROW(
            proxy.Call("savepattern", {"/tmp/pattern.txt"}, -1, PUT));
    } else {
        REQUIRE_THROWS(
            proxy.Call("savepattern", {"/tmp/pattern.txt"}, -1, PUT));
    }
}

TEST_CASE("patioctrl", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getPatternIOControl();
        {
            std::ostringstream oss;
            proxy.Call("patioctrl", {"0xc15004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patioctrl 0xc15004808d0a21a4\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patioctrl", {"0x0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patioctrl 0x0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patioctrl", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patioctrl 0x0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternIOControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patioctrl", {}, -1, GET));
    }
}

TEST_CASE("patclkctrl", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getPatternClockControl();
        {
            std::ostringstream oss;
            proxy.Call("patclkctrl", {"0xc15004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patclkctrl 0xc15004808d0a21a4\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patclkctrl", {"0x0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patclkctrl 0x0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patclkctrl", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patclkctrl 0x0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternClockControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patclkctrl", {}, -1, GET));
    }
}

TEST_CASE("patword", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        int addr = 0x23;
        std::string saddr = sls::ToStringHex(addr);
        auto prev_val = det.getPatternWord(addr);
        {
            std::ostringstream oss;
            proxy.Call("patword", {saddr, "0xc15004808d0a21a4"}, -1, PUT, oss);
            REQUIRE(oss.str() ==
                    "patword [" + saddr + ", 0xc15004808d0a21a4]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patword", {saddr, "0x0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patword [" + saddr + ", 0x0]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patword", {saddr}, -1, GET, oss);
            REQUIRE(oss.str() == "patword 0x0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWord(addr, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patword", {"0x23"}, -1, GET));
    }
}

TEST_CASE("patlimits", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopAddresses(-1);
        {
            std::ostringstream oss;
            proxy.Call("patlimits", {"0x20", "0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patlimits [0x20, 0x5c]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patlimits", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patlimits [0x20, 0x5c]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopAddresses(-1, prev_val[i][0], prev_val[i][1],
                                        {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patlimits", {}, -1, GET));
    }
}

TEST_CASE("patloop0", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopAddresses(0);
        {
            std::ostringstream oss;
            proxy.Call("patloop0", {"0x20", "0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patloop0 [0x20, 0x5c]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patloop0", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patloop0 [0x20, 0x5c]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopAddresses(0, prev_val[i][0], prev_val[i][1], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patloop0", {}, -1, GET));
    }
}

TEST_CASE("patloop1", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopAddresses(1);
        {
            std::ostringstream oss;
            proxy.Call("patloop1", {"0x20", "0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patloop1 [0x20, 0x5c]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patloop1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patloop1 [0x20, 0x5c]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopAddresses(1, prev_val[i][0], prev_val[i][1], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patloop1", {}, -1, GET));
    }
}

TEST_CASE("patloop2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopAddresses(2);
        {
            std::ostringstream oss;
            proxy.Call("patloop2", {"0x20", "0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patloop2 [0x20, 0x5c]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patloop2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patloop2 [0x20, 0x5c]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopAddresses(2, prev_val[i][0], prev_val[i][1], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patloop2", {}, -1, GET));
    }
}

TEST_CASE("patnloop0", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopCycles(0);
        {
            std::ostringstream oss;
            proxy.Call("patnloop0", {"5"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patnloop0 5\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patnloop0", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patnloop0 5\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopCycles(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patnloop0", {}, -1, GET));
    }
}

TEST_CASE("patnloop1", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopCycles(1);
        {
            std::ostringstream oss;
            proxy.Call("patnloop1", {"5"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patnloop1 5\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patnloop1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patnloop1 5\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopCycles(1, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patnloop1", {}, -1, GET));
    }
}

TEST_CASE("patnloop2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternLoopCycles(2);
        {
            std::ostringstream oss;
            proxy.Call("patnloop2", {"5"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patnloop2 5\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patnloop2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patnloop2 5\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternLoopCycles(2, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patnloop2", {}, -1, GET));
    }
}

TEST_CASE("patwait0", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternWaitAddr(0);
        {
            std::ostringstream oss;
            proxy.Call("patwait0", {"0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patwait0 0x5c\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patwait0", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patwait0 0x5c\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWaitAddr(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwait0", {}, -1, GET));
    }
}

TEST_CASE("patwait1", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternWaitAddr(1);
        {
            std::ostringstream oss;
            proxy.Call("patwait1", {"0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patwait1 0x5c\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patwait1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patwait1 0x5c\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWaitAddr(1, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwait1", {}, -1, GET));
    }
}

TEST_CASE("patwait2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternWaitAddr(2);
        {
            std::ostringstream oss;
            proxy.Call("patwait2", {"0x5c"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patwait2 0x5c\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patwait2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patwait2 0x5c\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWaitAddr(2, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwait2", {}, -1, GET));
    }
}

TEST_CASE("patwaittime0", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternWaitTime(0);
        {
            std::ostringstream oss;
            proxy.Call("patwaittime0", {"8589936640"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patwaittime0 8589936640\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patwaittime0", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patwaittime0 8589936640\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWaitTime(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwaittime0", {}, -1, GET));
    }
}

TEST_CASE("patwaittime1", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternWaitTime(1);
        {
            std::ostringstream oss;
            proxy.Call("patwaittime1", {"8589936640"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patwaittime1 8589936640\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patwaittime1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patwaittime1 8589936640\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWaitTime(1, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwaittime1", {}, -1, GET));
    }
}

TEST_CASE("patwaittime2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getPatternWaitTime(2);
        {
            std::ostringstream oss;
            proxy.Call("patwaittime2", {"8589936640"}, -1, PUT, oss);
            REQUIRE(oss.str() == "patwaittime2 8589936640\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("patwaittime2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "patwaittime2 8589936640\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPatternWaitTime(2, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("patwaittime2", {}, -1, GET));
    }
}

TEST_CASE("patmask", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
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

TEST_CASE("patsetbit", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3) {
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

TEST_CASE("patternstart", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(proxy.Call("patternstart", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("patternstart", {}, -1, PUT));
    }
}