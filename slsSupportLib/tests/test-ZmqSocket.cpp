#include "ZmqSocket.h"
#include "catch.hpp"

TEST_CASE("Throws when cannot create socket"){
    REQUIRE_THROWS(ZmqSocket("sdiasodjajpvv", 5076001));
}

TEST_CASE("Get port number for sub"){
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    REQUIRE(sub.GetPortNumber() == port);
}

TEST_CASE("Get port number for pub"){
    constexpr int port = 50001;
    ZmqSocket pub(port, "*");
    REQUIRE(pub.GetPortNumber() == port);
}

TEST_CASE("Server address"){
    constexpr int port = 50001;
    ZmqSocket pub(port, "*");
    REQUIRE(pub.GetZmqServerAddress() == std::string("tcp://*:50001"));
}

TEST_CASE("Send header on localhost") {
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    sub.Connect();

    ZmqSocket pub(port, "*");
    
    // Header to send
    zmqHeader header;
    header.data = false; // if true we wait for the data
    header.jsonversion = 0;
    header.dynamicRange = 32;
    header.fileIndex = 7;
    header.ndetx = 3;
    header.ndety = 1;
    header.npixelsx = 724;
    header.npixelsy = 324;
    header.imageSize = 200;
    header.fname = "hej";

    pub.SendHeader(0, header);

    zmqHeader received_header;
    sub.ReceiveHeader(0, received_header, 0);

    REQUIRE(received_header.fname == "hej");
    REQUIRE(received_header.dynamicRange == 32);
    REQUIRE(received_header.fileIndex == 7);
    REQUIRE(received_header.ndetx == 3);
    REQUIRE(received_header.ndety == 1);
    REQUIRE(received_header.npixelsx == 724);
    REQUIRE(received_header.npixelsy == 324);
    REQUIRE(received_header.imageSize == 200);
    
}

TEST_CASE("Send serveral headers of different length"){
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    sub.Connect();

    ZmqSocket pub(port, "*");

    zmqHeader header;
    header.data = false; // if true we wait for the data
    header.fname = "short_name";

    zmqHeader received_header;

    pub.SendHeader(0, header);
    sub.ReceiveHeader(0, received_header, 0);
    REQUIRE(received_header.fname == "short_name");

    header.fname = "this_time_a_much_longer_name";
    pub.SendHeader(0, header);
    sub.ReceiveHeader(0, received_header, 0);
    REQUIRE(received_header.fname == "this_time_a_much_longer_name");

    header.fname = "short";
    pub.SendHeader(0, header);
    sub.ReceiveHeader(0, received_header, 0);
    REQUIRE(received_header.fname == "short");
}