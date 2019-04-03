
#include "GeneralData.h"
#include "catch.hpp"


#include <iostream>



// using namespace sls;

TEST_CASE("Parse jungfrauctb header", "[receiver]") {

    // typedef struct {
    //     unsigned char emptyHeader[6];
    //     unsigned char reserved[4];
    //     unsigned char packetNumber[1];
    //     unsigned char frameNumber[3];
    //     unsigned char bunchid[8];
    // } jfrauctb_packet_header_t;

    struct packet {
        unsigned char emptyHeader[6];
        unsigned char reserved[4];
        unsigned char packetNumber[1];
        unsigned char frameNumber[3];
        unsigned char bunchid[8];
        unsigned char data[UDP_PACKET_DATA_BYTES];
    } __attribute__((packed));

    MoenchData data;
    // GetHeaderInfo(int index, char *packetData, uint32_t dynamicRange,
    //               bool oddStartingPacket, uint64_t &frameNumber,
    //               uint32_t &packetNumber, uint32_t &subFrameNumber,
    //               uint64_t &bunchId)


    packet test_packet;
    test_packet.packetNumber[0] = (unsigned char)53;
    test_packet.frameNumber[0] = (unsigned char)32;
    test_packet.frameNumber[1] = (unsigned char)15;
    test_packet.frameNumber[2] = (unsigned char)91;

    test_packet.bunchid[0] = (unsigned char)91;
    test_packet.bunchid[1] = (unsigned char)25;
    test_packet.bunchid[2] = (unsigned char)15;
    test_packet.bunchid[3] = (unsigned char)1;
    test_packet.bunchid[4] = (unsigned char)32;
    test_packet.bunchid[5] = (unsigned char)251;
    test_packet.bunchid[6] = (unsigned char)18;
    test_packet.bunchid[7] = (unsigned char)240;

    int index = 0;
    char *packetData = reinterpret_cast<char *>(&test_packet);
    uint32_t dynamicRange{0};
    bool oddStartingPacket{0};
    uint64_t frameNumber{0};
    uint32_t packetNumber{0};
    uint32_t subFrameNumber{0};
    uint64_t bunchId{0};

    data.GetHeaderInfo(index, packetData, dynamicRange, oddStartingPacket,
                       frameNumber, packetNumber, subFrameNumber, bunchId);

    CHECK(packetNumber == 53);
    CHECK(frameNumber == 0x5b0f20);
    CHECK(bunchId == 0xf012fb20010f195b);
    CHECK(subFrameNumber == -1);
}

TEST_CASE("Parse header gotthard data", "[receiver]") { 
    GotthardData data;
    struct packet {
        uint32_t frameNumber;
        unsigned char data[GOTTHARD_PACKET_SIZE];
    } __attribute__((packed));
    packet test_packet;
    test_packet.frameNumber = 25698u;

    int index = 0;
    char *packetData = reinterpret_cast<char *>(&test_packet);
    uint32_t dynamicRange{0};
    bool oddStartingPacket{0};
    uint64_t frameNumber{0};
    uint32_t packetNumber{0};
    uint32_t subFrameNumber{0};
    uint64_t bunchId{0};

    data.GetHeaderInfo(index, packetData, dynamicRange, oddStartingPacket,
                       frameNumber, packetNumber, subFrameNumber, bunchId);

    CHECK(frameNumber == test_packet.frameNumber/2);
    CHECK(subFrameNumber == -1);
    CHECK(bunchId == -1);

}