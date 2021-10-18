// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "GeneralData.h"
#include "catch.hpp"

#include <iostream>

// using namespace sls;

// TEST_CASE("Parse jungfrauctb header", "[receiver]") {

//     struct packet {
//         unsigned char emptyHeader[6];
//         unsigned char reserved[4];
//         unsigned char packetNumber[1];
//         unsigned char frameNumber[3];
//         unsigned char bunchid[8];
//         unsigned char data[UDP_PACKET_DATA_BYTES];
//     } __attribute__((packed));

//     MoenchData data;

//     packet test_packet;
//     test_packet.packetNumber[0] = 53u;
//     test_packet.frameNumber[0] = 32u;
//     test_packet.frameNumber[1] = 15u;
//     test_packet.frameNumber[2] = 91u;

//     test_packet.bunchid[0] = 91u;
//     test_packet.bunchid[1] = 25u;
//     test_packet.bunchid[2] = 15u;
//     test_packet.bunchid[3] = 1u;
//     test_packet.bunchid[4] = 32u;
//     test_packet.bunchid[5] = 251u;
//     test_packet.bunchid[6] = 18u;
//     test_packet.bunchid[7] = 240u;

//     int index = 0;
//     char *packetData = reinterpret_cast<char *>(&test_packet);
//     uint32_t dynamicRange{0};
//     bool oddStartingPacket{0};
//     uint64_t frameNumber{0};
//     uint32_t packetNumber{0};
//     uint32_t subFrameNumber{0};
//     uint64_t bunchId{0};

//     data.GetHeaderInfo(index, packetData, dynamicRange, oddStartingPacket,
//                        frameNumber, packetNumber, subFrameNumber, bunchId);

//     CHECK(packetNumber == 53);
//     CHECK(frameNumber == 0x5b0f20);
//     CHECK(bunchId == 0xf012fb20010f195b);
//     CHECK(subFrameNumber == -1);
// }

// TEST_CASE("Parse header gotthard data", "[receiver]") {
//     GotthardData data;
//     struct packet {
//         uint32_t frameNumber;
//         unsigned char data[GOTTHARD_PACKET_SIZE];
//     } __attribute__((packed));
//     packet test_packet;
//     test_packet.frameNumber = 25698u;

//     int index = 0;
//     char *packetData = reinterpret_cast<char *>(&test_packet);
//     uint32_t dynamicRange{0};
//     bool oddStartingPacket{0};
//     uint64_t frameNumber{0};
//     uint32_t packetNumber{0};
//     uint32_t subFrameNumber{0};
//     uint64_t bunchId{0};

//     data.GetHeaderInfo(index, packetData, dynamicRange, oddStartingPacket,
//                        frameNumber, packetNumber, subFrameNumber, bunchId);

//     CHECK(frameNumber == test_packet.frameNumber/2);
//     CHECK(subFrameNumber == -1);
//     CHECK(bunchId == -1);

// }