#include "BinaryFileStatic.h"
#include "HDF5FileStatic.h"
#include "catch.hpp"

SCENARIO("File name creation", "[receiver]") {
    GIVEN("These parameters") {

        std::string fpath = "/home/test";
        std::string fnameprefix = "hej";
        uint64_t findex{0};
        bool frindexenable{true};
        uint64_t fnum{0};
        int dindex{1};
        int numunits{1};
        int unitindex{0};

        WHEN("called with default arguments and true") {
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, frindexenable);
            THEN("filename contains frame index") {
                REQUIRE(fname == "/home/test/hej_f000000000000_0.raw");
            }
        }
        WHEN("frindexenable instead is false") {
            frindexenable = false;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, frindexenable);
            THEN("there is no frame index in the name") {
                REQUIRE(fname == "/home/test/hej_0.raw");
            }
        }
        WHEN("the frame number is set") {
            fnum = 123456;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, frindexenable, fnum);
            THEN("The frame number is in the file name") {
                REQUIRE(fname == "/home/test/hej_f000000123456_0.raw");
            }
        }
        WHEN("frame number and dindex is set") {
            fnum = 569;
            dindex = 7;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, frindexenable, fnum,
                dindex);
            THEN("Both appear in the file name") {
                REQUIRE(fname == "/home/test/hej_d7_f000000000569_0.raw");
            }
        }
        WHEN("setting numunits ") {
            dindex = 2;
            numunits = 2;
            unitindex = 0;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, frindexenable, fnum, dindex,
                numunits, unitindex);

            unitindex = 1;
            auto fname2 = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0],

                findex, frindexenable, fnum, dindex, numunits, unitindex);
            THEN("this gets reflected in d number") {
                REQUIRE(fname == "/home/test/hej_d4_f000000000000_0.raw");
                REQUIRE(fname2 == "/home/test/hej_d5_f000000000000_0.raw");
            }
        }
        WHEN("sett findex") {
            findex = 96;
            dindex = 0;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, frindexenable, fnum,
                dindex);
            THEN("this is printed in the file name") {
                REQUIRE(fname == "/home/test/hej_d0_f000000000000_96.raw");
            }
        }
    }
}

SCENARIO("Creating master file name", "[receiver]") {
    GIVEN("these parameters") {
        std::string fpath = "/home/test";
        std::string fnameprefix = "hej";
        uint64_t findex{0};

        WHEN("the master file name is created") {
            THEN("all parameters are found") {
                BinaryFileStatic b;
                auto fname =
                    b.CreateMasterFileName(&fpath[0], &fnameprefix[0], findex);
                REQUIRE(fname == "/home/test/hej_master_0.raw");
            }
        }
        WHEN("flie index is changed") {
            THEN("its visible in the file name") {
                findex = 398;
                BinaryFileStatic b;
                auto fname =
                    b.CreateMasterFileName(&fpath[0], &fnameprefix[0], findex);
                REQUIRE(fname == "/home/test/hej_master_398.raw");
            }
        }
    }
}