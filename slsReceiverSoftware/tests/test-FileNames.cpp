#include "BinaryFile.h"
#include "BinaryFileStatic.h"
#ifndef HDF5C
#define HDF5C
#endif
#include "HDF5FileStatic.h"
#include "catch.hpp"

SCENARIO("File name creation raw files", "[receiver]") {
    GIVEN("These parameters and a binary file") {

        std::string fpath = "/home/test";
        std::string fnameprefix = "hej";
        uint64_t findex{0};
        bool frindexenable{true};
        uint64_t fnum{0};
        int dindex{0};
        int numunits{1};
        int unitindex{0};
        bool fixedw_findex{false};

        WHEN("called with default arguments and true") {
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex);
            THEN("filename contains frame index") {
                REQUIRE(fname == "/home/test/hej_d0_f0_0.raw");
            }
        }
        WHEN("the file index is set") {
            fnum = 123456;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex);
            THEN("The frame number is in the file name") {
                REQUIRE(fname == "/home/test/hej_d0_f123456_0.raw");
            }
        }
        WHEN("setting numunits ") {
            dindex = 2;
            numunits = 2;
            unitindex = 0;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex, numunits,
                unitindex);

            unitindex = 1;
            auto fname2 = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex, numunits,
                unitindex);
            THEN("this gets reflected in d number") {
                REQUIRE(fname == "/home/test/hej_d4_f0_0.raw");
                REQUIRE(fname2 == "/home/test/hej_d5_f0_0.raw");
            }
        }
        WHEN("measurements index is set") {
            findex = 96;
            dindex = 0;
            auto fname = BinaryFileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex);
            THEN("this is printed in the file name") {
                REQUIRE(fname == "/home/test/hej_d0_f0_96.raw");
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

SCENARIO("File name creation hdf5 files", "[receiver]") {
    GIVEN("Some paramters") {

        std::string fpath = "/home/test";
        std::string fnameprefix = "hej";
        uint64_t findex{0};
        bool frindexenable{true};
        uint64_t fnum{0};
        int dindex{0};
        int numunits{1};
        int unitindex{0};
        bool fixedw_findex{false};

        WHEN("called with default arguments and true for frindexenable") {
            auto fname = HDF5FileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex);
            THEN("filename contains frame index") {
                REQUIRE(fname == "/home/test/hej_d0_f0_0.h5");
            }
        }
        WHEN("the frame number is set") {
            fnum = 123456;
            auto fname = HDF5FileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex);
            THEN("The frame number is in the file name") {
                REQUIRE(fname == "/home/test/hej_d0_f123456_0.h5");
            }
        }
        WHEN("setting numunits ") {
            dindex = 2;
            numunits = 2;
            unitindex = 0;
            auto fname = HDF5FileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex, numunits,
                unitindex);

            unitindex = 1;
            auto fname2 = HDF5FileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex, numunits,
                unitindex);
            THEN("this gets reflected in d number") {
                REQUIRE(fname == "/home/test/hej_d4_f0_0.h5");
                REQUIRE(fname2 == "/home/test/hej_d5_f0_0.h5");
            }
        }
        WHEN("set findex") {
            findex = 96;
            dindex = 0;
            auto fname = HDF5FileStatic::CreateFileName(
                &fpath[0], &fnameprefix[0], findex, fnum, dindex);
            THEN("this is printed in the file name") {
                REQUIRE(fname == "/home/test/hej_d0_f0_96.h5");
            }
        }
    }
}