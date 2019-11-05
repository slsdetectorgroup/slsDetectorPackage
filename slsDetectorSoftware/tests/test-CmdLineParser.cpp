#include "CmdLineParser.h"
#include "catch.hpp"
#include <exception>
#include <string>


// tests to add
// help for all docs
// command for all depreciated commands

using vs = std::vector<std::string>;
using sls::CmdLineParser;

SCENARIO("Construction", "[support]") {
    GIVEN("A default constructed CmdLineParser") {
        CmdLineParser p;
        THEN("The state of the object is valid") {
            REQUIRE(p.detector_id() == -1);
            REQUIRE(p.multi_id() == 0);
            REQUIRE(p.command().empty());
            REQUIRE(p.arguments().empty());
            REQUIRE(p.argv().empty());
            REQUIRE(p.argv().data() == nullptr);
        }
    }
}

SCENARIO("Parsing a string with the command line parser", "[support]") {
    GIVEN("A CmdLineParser") {
        CmdLineParser p;
        WHEN("Parsing an empty string") {
            std::string s;
            p.Parse(s);
            THEN("command and arguments are empty") {
                REQUIRE(p.detector_id() == -1);
                REQUIRE(p.multi_id() == 0);
                REQUIRE(p.command().empty());
                REQUIRE(p.arguments().empty());
                REQUIRE(p.argv().empty());
            }
        }
        WHEN("Parsing a string with a single command") {
            std::string s = "vrf";
            p.Parse(s);
            THEN("command is assigned and id's remain default") {
                REQUIRE(p.command() == "vrf");
                REQUIRE(p.detector_id() == -1);
                REQUIRE(p.multi_id() == 0);
                REQUIRE(p.arguments().empty());
                REQUIRE(p.argv().size() == 1);
            }
        }
        WHEN("Parsing a string with command and value") {
            std::string s = "vthreshold 1500";
            p.Parse(s);
            THEN("cmd and value are assigned and id's remain default") {
                REQUIRE(p.command() == "vthreshold");
                REQUIRE(p.arguments()[0] == "1500");
                REQUIRE(p.arguments().size() == 1);
                REQUIRE(p.detector_id() == -1);
                REQUIRE(p.multi_id() == 0);
            }
        }
        WHEN("Parsing a string with detector id and command") {
            vs arg{"9:vcp", "53:vthreshold", "128:vtrim", "5:threshold"};
            std::vector<int> det_id{9, 53, 128, 5};
            vs res{"vcp", "vthreshold", "vtrim", "threshold"};

            THEN("Values are correctly decoded") {
                for (size_t i = 0; i != arg.size(); ++i) {
                    p.Parse(arg[i]);
                    REQUIRE(p.detector_id() == det_id[i]);
                    REQUIRE(p.multi_id() == 0);
                    REQUIRE(p.command() == res[i]);
                    REQUIRE(p.arguments().empty());
                    REQUIRE(p.argv().size() == 1);
                }
            }
        }
        WHEN("Parsing a string with multi_id detector id and command") {
            vs arg{"8-12:vrf", "0-52:vcmp", "19-10:vtrim", "31-127:threshold"};
            std::vector<int> det_id{12, 52, 10, 127};
            std::vector<int> multi_id{8, 0, 19, 31};
            vs res{"vrf", "vcmp", "vtrim", "threshold"};

            THEN("Values are correctly decoded") {
                for (size_t i = 0; i != arg.size(); ++i) {
                    p.Parse(arg[i]);
                    REQUIRE(p.detector_id() == det_id[i]);
                    REQUIRE(p.multi_id() == multi_id[i]);
                    REQUIRE(p.command() == res[i]);
                    REQUIRE(p.arguments().empty());
                    REQUIRE(p.argv().size() == 1);
                }
            }
        }

        WHEN("Parsing string with cmd and multiple arguments") {
            std::string s = "trimen 5000 6000 7000";
            p.Parse(s);
            THEN("cmd and args are correct") {
                REQUIRE(p.command() == "trimen");
                REQUIRE(p.arguments().size() == 3);
                REQUIRE(p.arguments()[0] == "5000");
                REQUIRE(p.arguments()[1] == "6000");
                REQUIRE(p.arguments()[2] == "7000");
            }
        }

        WHEN("Cliend id and or detector id cannot be decoded") {
            vs arg{"o:cmd",     "-5:cmd",    "aedpva:cmd",
                   "5-svc:vrf", "asv-5:cmd", "savc-asa:cmd"};
            THEN("Parsing Throws") {
                for (size_t i = 0; i != arg.size(); ++i) {
                    REQUIRE_THROWS(p.Parse(arg[i]));
                }
            }
        }
    }
}

SCENARIO("Parsing strings with -h or --help", "[support]") {
    GIVEN("A parser") {
        CmdLineParser p;
        WHEN("Parsing a string with a command and help ") {
            std::string s = "-h list";

            THEN("the command is correct and isHelp is set") {
                p.Parse(s);
                REQUIRE(p.detector_id() == -1);
                REQUIRE(p.multi_id() == 0);
                REQUIRE(p.command() == "list");
                REQUIRE(p.isHelp());
                REQUIRE(p.arguments().empty());
                REQUIRE(p.argv().size() == 1);
            }
        }
        WHEN("Parsing a string with -h at a different position"){
            std::string s = "list -h something";
            THEN("its also done right"){
                p.Parse(s);
                REQUIRE(p.isHelp());
                REQUIRE(p.command() == "list");
                REQUIRE(p.arguments().size() == 1);
                REQUIRE(p.arguments().front() == "something");
            }
        }
        WHEN("Parsing a string with -help at a different position"){
            std::string s = "list --help something";
            THEN("its also done right"){
                p.Parse(s);
                REQUIRE(p.isHelp());
                REQUIRE(p.command() == "list");
                REQUIRE(p.arguments().size() == 1);
                REQUIRE(p.arguments().front() == "something");
            }
        }
    }
}

TEST_CASE("Parse with no arguments results in no command and default id",
          "[support]") {
    // build up argc and argv
    // first argument is the command used to call the binary
    int argc = 1;
    const char *const argv[]{"call"};
    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command().empty());
    REQUIRE(p.arguments().empty());
}

TEST_CASE(
    "Parse a command without client id and detector id results in default",
    "[support]") {
    int argc = 2;
    const char *const argv[]{"caller", "vrf"};
    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == "vrf");
    REQUIRE(p.arguments().empty());
}

TEST_CASE("Parse a command with value but without client or detector id",
          "[support]") {
    int argc = 3;
    const char *const argv[]{"caller", "vrf", "3000"};
    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == "vrf");
    REQUIRE(p.arguments().size() == 1);
    REQUIRE(p.arguments()[0] == "3000");
}

TEST_CASE("Decodes position") {
    int argc = 2;
    const char *const argv[]{"caller", "7:vrf"};

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == 7);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == "vrf");
    REQUIRE(p.arguments().empty());
}

TEST_CASE("Decodes double digit position", "[support]") {
    int argc = 2;
    const char *const argv[]{"caller", "73:vcmp"};
    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == 73);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == "vcmp");
    REQUIRE(p.arguments().empty());
}

TEST_CASE("Decodes position and id", "[support]") {
    int argc = 2;
    const char *const argv[]{"caller", "5-8:vrf"};
    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == 8);
    REQUIRE(p.multi_id() == 5);
    REQUIRE(p.command() == "vrf");
    REQUIRE(p.arguments().empty());
}

TEST_CASE("Double digit id", "[support]") {
    int argc = 2;
    const char *const argv[]{"caller", "56-8:vrf"};
    CmdLineParser p;
    p.Parse(argc, argv);
    REQUIRE(p.detector_id() == 8);
    REQUIRE(p.multi_id() == 56);
    REQUIRE(p.command() == "vrf");
    REQUIRE(p.arguments().empty());
}

TEST_CASE("Calling with wrong id throws invalid_argument", "[support]") {
    int argc = 2;
    const char *const argv[]{"caller", "asvldkn:vrf"};
    CmdLineParser p;
    CHECK_THROWS(p.Parse(argc, argv));
}

TEST_CASE("Calling with wrong client throws invalid_argument", "[support]") {
    int argc = 2;
    const char *const argv[]{"caller", "lki-3:vrf"};
    CmdLineParser p;
    CHECK_THROWS(p.Parse(argc, argv));
}

TEST_CASE("Build up argv", "[support]") {
    CmdLineParser p;
    REQUIRE(p.argv().empty());
    REQUIRE(p.argv().data() == nullptr);

    std::string s = "trimen 3000 4000\n";
    p.Parse(s);
    REQUIRE(p.argv().data() != nullptr);
    REQUIRE(p.argv().size() == 3);
}