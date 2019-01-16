#include "CmdLineParser.h"
#include "catch.hpp"
#include <exception>
#include <string>
//tests to add
//help for all docs
//command for all depreciated commands

TEST_CASE("Parse with no arguments results in no command and default id")
{
    //build up argc and argv
    //first argument is the command used to call the binary
    int argc = 1;
    char* argv[argc];
    char a0[] = "call";
    argv[0]   = a0;

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string(""));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Parse empty string")
{
    std::string s = "";
    CmdLineParser p;
    p.Parse(s);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string(""));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Parse a command without client id and detector id results in default")
{
    int argc = 2;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "vrf";
    argv[0]   = a0;
    argv[1]   = a1;

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Parse a string without client id and detector id results in default")
{
    std::string s = "vrf";
    CmdLineParser p;
    p.Parse(s);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Parse a command with value but without client or detector id")
{
    int argc = 3;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "vrf";
    char a2[] = "3000";
    argv[0]   = a0;
    argv[1]   = a1;
    argv[2]   = a2;

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 1);
    REQUIRE(p.arguments()[0] == std::string("3000"));
}
TEST_CASE("Parse a string with value but without client or detector id")
{
    std::string s = "vrf 3000\n";

    CmdLineParser p;
    p.Parse(s);

    REQUIRE(p.detector_id() == -1);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 1);
    REQUIRE(p.arguments()[0] == std::string("3000"));
}

TEST_CASE("Decodes position")
{
    int argc = 2;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "7:vrf";
    argv[0]   = a0;
    argv[1]   = a1;

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == 7);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}
TEST_CASE("Decodes position from string")
{
    std::string s =  "7:vrf\n";

    CmdLineParser p;
    p.Parse(s);

    REQUIRE(p.detector_id() == 7);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Decodes double digit position")
{
    int argc = 2;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "73:vcmp";
    argv[0]   = a0;
    argv[1]   = a1;

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == 73);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vcmp"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Decodes double digit position from string")
{

    std::string s = "73:vcmp";
    CmdLineParser p;
    p.Parse(s);

    REQUIRE(p.detector_id() == 73);
    REQUIRE(p.multi_id() == 0);
    REQUIRE(p.command() == std::string("vcmp"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Decodes position and id")
{
    int argc = 2;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "5-8:vrf";
    argv[0]   = a0;
    argv[1]   = a1;

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == 8);
    REQUIRE(p.multi_id() == 5);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}
TEST_CASE("Decodes position and id from string")
{

    std::string s = "5-8:vrf";


    CmdLineParser p;
    p.Parse(s);

    REQUIRE(p.detector_id() == 8);
    REQUIRE(p.multi_id() == 5);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Double digit id")
{
    int argc = 2;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "56-8:vrf";
    argv[0]   = a0;
    argv[1]   = a1;

    CmdLineParser p;
    p.Parse(argc, argv);

    REQUIRE(p.detector_id() == 8);
    REQUIRE(p.multi_id() == 56);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Double digit id from string")
{
    std::string s = "56-8:vrf";


    CmdLineParser p;
    p.Parse(s);

    REQUIRE(p.detector_id() == 8);
    REQUIRE(p.multi_id() == 56);
    REQUIRE(p.command() == std::string("vrf"));
    REQUIRE(p.arguments().size() == 0);
}

TEST_CASE("Calling with wrong id throws invalid_argument")
{

    int argc = 2;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "asvldkn:vrf";
    argv[0]   = a0;
    argv[1]   = a1;

    CmdLineParser p;
    CHECK_THROWS(p.Parse(argc, argv));
}
TEST_CASE("Calling with string with wrong id throws invalid_argument")
{
    std::string s = "asvldkn:vrf";
    CmdLineParser p;
    CHECK_THROWS(p.Parse(s));
}


TEST_CASE("Calling with wrong client throws invalid_argument")
{

    int argc = 2;
    char* argv[argc];
    char a0[] = "call";
    char a1[] = "lki-3:vrf";
    argv[0]   = a0;
    argv[1]   = a1;

    CmdLineParser p;
    CHECK_THROWS(p.Parse(argc, argv));
}
TEST_CASE("Calling with string with wrong client throws invalid_argument")
{
    std::string s = "lki-3:vrf";
    CmdLineParser p;
    CHECK_THROWS(p.Parse(s));
}

TEST_CASE("Parses string with two arguments"){
  std::string s = "trimen 3000 4000\n";
  CmdLineParser p;
  p.Parse(s);

  REQUIRE("trimen" == p.command());
  REQUIRE("3000" == p.arguments()[0]);
  REQUIRE("4000" == p.arguments()[1]);
  REQUIRE(2 == p.arguments().size());
}