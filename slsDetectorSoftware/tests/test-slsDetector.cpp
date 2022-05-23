// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

// #include "catch.hpp"
// #include "sls/container_utils.h"
// #include "slsDetector.h"
// #include "sls/sls_detector_defs.h"
// #include "sls/string_utils.h"
// #include <algorithm>
// #include <iostream>

// TEST_CASE("Set and get trimen", "[detector]") {
//     // Free shared memory to be sure that we start in a clean state
//     slsDetector::freeSharedMemory(20, 20);

//     // Create a detector and check that the type is set correctly
//     slsDetector d(slsDetectorDefs::detectorType::EIGER, 20, 20);
//     CHECK(d.getDetectorTypeAsEnum() == slsDetectorDefs::detectorType::EIGER);

//     // At the beginning there should be no trimen set
//     auto res = d.getTrimEn();
//     CHECK(res.empty());

//     std::vector<int> energies{5200, 6400, 8500, 9900, 12000};
//     d.setTrimEn(energies);
//     auto res2 = d.getTrimEn();

//     // Check that the size and every element matches what we set
//     CHECK(res2.size() == energies.size());
//     for (size_t i = 0; i != res2.size(); ++i)
//         CHECK(res2[i] == energies[i]);

//     // Setting trimen with too many vales throws an exception and keeps the
//     // old values
//     std::vector<int> too_many(150, 1000);
//     CHECK_THROWS(d.setTrimEn(too_many));
//     auto res3 = d.getTrimEn();
//     CHECK(res3.size() == energies.size());
//     for (size_t i = 0; i != res3.size(); ++i)
//         CHECK(res3[i] == energies[i]);

//     // Setting trimen without arguments resets to zero
//     d.setTrimEn();
//     CHECK(d.getTrimEn().empty());

//     // Clean up before next test
//     d.freeSharedMemory();
// }

// TEST_CASE("Set additional JSON header", "[detector]") {
//     slsDetector::freeSharedMemory(20, 20);
//     slsDetector d(slsDetectorDefs::detectorType::EIGER, 20, 20);
//     auto header = d.getAdditionalJsonHeader();
//     CHECK(header.empty());

//     // The header set is not validated
//     d.setAdditionalJsonHeader("any header");
//     header = d.getAdditionalJsonHeader();
//     CHECK(header == "any header");

//     // make sure reset works
//     d.setAdditionalJsonHeader("");
//     CHECK(d.getAdditionalJsonHeader().empty());

//     // Setting and getting one parameter
//     d.setAdditionalJsonParameter("exptime", "5");
//     CHECK(d.getAdditionalJsonParameter("exptime") == "5");
//     CHECK(d.getAdditionalJsonHeader() == "\"exptime\":5");

//     // Making sure setting another paramer does not mess up
//     // the first
//     d.setAdditionalJsonParameter("gain", "low");
//     CHECK(d.getAdditionalJsonParameter("exptime") == "5");
//     CHECK(d.getAdditionalJsonParameter("gain") == "low");
//     CHECK(d.getAdditionalJsonHeader() == "\"exptime\":5,\"gain\":\"low\"");

//     // Change a value
//     d.setAdditionalJsonParameter("exptime", "90");
//     CHECK(d.getAdditionalJsonParameter("exptime") == "90");
//     CHECK(d.getAdditionalJsonHeader() == "\"exptime\":90,\"gain\":\"low\"");

//     // Ask for a key that does not exists
//     // TODO!(Erik) Is an empty string the right return or should we throw
//     CHECK(d.getAdditionalJsonParameter("somerandomkey").empty());

//     // Throws if value or key is empty
//     CHECK_THROWS(d.setAdditionalJsonParameter("somekey", ""));
//     CHECK_THROWS(d.setAdditionalJsonParameter("", "parameter"));
//     CHECK_THROWS(d.setAdditionalJsonParameter("", ""));

//     // Throws if key or value has illegal char
//     CHECK_THROWS(d.setAdditionalJsonParameter("mykey,", "5"));
//     CHECK_THROWS(d.setAdditionalJsonParameter("some:key", "9"));
//     CHECK_THROWS(d.setAdditionalJsonParameter("some\"key", "1"));
//     CHECK_THROWS(d.setAdditionalJsonParameter("key", "value:"));
//     CHECK_THROWS(d.setAdditionalJsonParameter("key", "va,lue"));
//     CHECK_THROWS(d.setAdditionalJsonParameter("key", "va\"l\"ue"));

//     d.freeSharedMemory();
// }

// TEST_CASE("Set ROI", "[detector]") {
//     using ROI = slsDetectorDefs::ROI;

//     slsDetector::freeSharedMemory(20,20);
//     slsDetector d(slsDetectorDefs::detectorType::EIGER, 20, 20);

//     int n{0};
//     d.getROI(n);
//     CHECK(n == 0);
//     CHECK(d.getNRoi() == 0);

//     // set one ROI
//     ROI r;
//     r.xmin = 5;
//     r.xmax = 100;
//     r.ymin = 10;
//     r.ymax = 300;
//     d.setROI(1, &r);

//     auto res2 = d.getROI(n);
//     CHECK(n == 1);
//     CHECK(d.getNRoi() == 1);

//     CHECK(res2->xmin == 5);
//     CHECK(res2->xmax == 100);
//     CHECK(res2->ymin == 10);
//     CHECK(res2->ymax == 300);

//     d.freeSharedMemory();
// }

// TEST_CASE("Set multiple ROIs", "[detector]") {
//     using ROI = slsDetectorDefs::ROI;

//     slsDetector::freeSharedMemory(20, 20);
//     slsDetector d(slsDetectorDefs::detectorType::EIGER, 20, 20);

//     // set one ROI
//     constexpr int n = 3;
//     ROI r[n];
//     r[0].xmin = 500;
//     r[0].xmax = 60000;
//     r[0].ymin = 100;
//     r[0].ymax = 800;

//     r[1].xmin = 2;
//     r[1].xmax = 100;
//     r[1].ymin = 1;
//     r[1].ymax = 300;

//     r[2].xmin = 200;
//     r[2].xmax = 300;
//     r[2].ymin = 15;
//     r[2].ymax = 307;
//     d.setROI(n, r);

//     int n_roi{0};
//     auto res = d.getROI(n_roi);
//     CHECK(n_roi == n);
//     CHECK(d.getNRoi() == n);

//     CHECK(res[0].xmin == 2);
//     CHECK(res[0].xmax == 100);
//     CHECK(res[0].ymin == 1);
//     CHECK(res[0].ymax == 300);

//     CHECK(res[1].xmin == 200);
//     CHECK(res[1].xmax == 300);
//     CHECK(res[1].ymin == 15);
//     CHECK(res[1].ymax == 307);

//     CHECK(res[2].xmin == 500);
//     CHECK(res[2].xmax == 60000);
//     CHECK(res[2].ymin == 100);
//     CHECK(res[2].ymax == 800);

//     d.freeSharedMemory();
// }

// TEST_CASE("Padding and discard policy", "[detector][new]"){
//     slsDetector::freeSharedMemory(20, 20);
//     slsDetector d(slsDetectorDefs::detectorType::EIGER, 20, 20);

//     //
//     d.setPartialFramesPadding(false);
//     CHECK(d.getPartialFramesPadding() == false);
//     d.setPartialFramesPadding(true);
//     CHECK(d.getPartialFramesPadding() == true);

//     d.freeSharedMemory();

// }

// TEST_CASE("create detParamets struct", "[detector][new]"){
//     detParameters par;
//     CHECK(sizeof(par) == 32);
//     CHECK(par.nChanX == 0);
//     CHECK(par.nChanY == 0);
//     CHECK(par.nChipX == 0);
//     CHECK(par.nChipY == 0);
//     CHECK(par.nDacs == 0);
//     CHECK(par.dynamicRange == 0);
//     CHECK(par.nGappixelsX == 0);
//     CHECK(par.nGappixelsY == 0);

//     detParameters par2{slsDetectorDefs::detectorType::EIGER};
//     CHECK(sizeof(par2) == 32);
//     CHECK(par2.nChanX == 256);
//     CHECK(par2.nChanY == 256);
//     CHECK(par2.nChipX == 4);
//     CHECK(par2.nChipY == 1);
//     CHECK(par2.nDacs == 16);
//     CHECK(par2.dynamicRange == 16);
//     CHECK(par2.nGappixelsX == 6);
//     CHECK(par2.nGappixelsY == 1);
// }

// TEST_CASE("ctb digital offset and list", "[detector][ctb]"){
//     slsDetector::freeSharedMemory(20, 20);
//     slsDetector d(slsDetectorDefs::detectorType::CHIPTESTBOARD, 20, 20);

//     // dbit offset
//     CHECK(d.getReceiverDbitOffset() == 0);
//     CHECK(d.setReceiverDbitOffset(-1) == 0);
//     CHECK(d.setReceiverDbitOffset(0) == 0);
//     CHECK(d.setReceiverDbitOffset(5) == 5);
//     CHECK(d.getReceiverDbitOffset() == 5);

//     // dbit list
//     std::vector <int> list = d.getReceiverDbitList();
//     CHECK(list.empty());

//     for (int i = 0; i < 10; ++i)
//         list.push_back(i);
//     d.setReceiverDbitList(list);

//     CHECK(d.getReceiverDbitList().size() == 10);

//     list.push_back(64);
//     CHECK_THROWS_AS(d.setReceiverDbitList(list), RuntimeError);
//     CHECK_THROWS_WITH(d.setReceiverDbitList(list),
//         Catch::Matchers::Contains("be between 0 and 63"));

//     list.clear();
//     for (int i = 0; i < 65; ++i)
//         list.push_back(i);
//     CHECK(list.size() == 65);
//     CHECK_THROWS_WITH(d.setReceiverDbitList(list),
//         Catch::Matchers::Contains("be greater than 64"));

//     list.clear();
//     d.setReceiverDbitList(list);
//     CHECK(d.getReceiverDbitList().empty());

// }