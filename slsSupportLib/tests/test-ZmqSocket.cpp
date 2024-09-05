// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/ZmqSocket.h"

namespace sls {

TEST_CASE("Throws when cannot create socket") {
    REQUIRE_THROWS(ZmqSocket("sdiasodjajpvv", 50001));
}

TEST_CASE("Get port number for sub") {
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    REQUIRE(sub.GetPortNumber() == port);
}

TEST_CASE("Get port number for pub") {
    constexpr int port = 50001;
    ZmqSocket pub(port);
    REQUIRE(pub.GetPortNumber() == port);
}

TEST_CASE("Server address") {
    constexpr int port = 50001;
    ZmqSocket pub(port);
    REQUIRE(pub.GetZmqServerAddress() == std::string("tcp://0.0.0.0:50001"));
}

TEST_CASE("Send header on localhost") {
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    sub.Connect();

    ZmqSocket pub(port);

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

TEST_CASE("Send serveral headers of different length") {
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    sub.Connect();

    ZmqSocket pub(port);

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

TEST_CASE("Send header and data") {
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    sub.Connect();

    ZmqSocket pub(port);

    std::vector<int> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const int nbytes = data.size() * sizeof(decltype(data)::value_type);
    zmqHeader header;
    header.data = true;
    header.imageSize = nbytes;

    pub.SendHeader(0, header);
    pub.SendData((char *)data.data(), nbytes);

    zmqHeader received_header;
    sub.ReceiveHeader(0, received_header, 0);
    std::vector<int> received_data(received_header.imageSize / sizeof(int));
    sub.ReceiveData(0, (char *)received_data.data(), received_header.imageSize);

    REQUIRE(data.size() == received_data.size());
    for (size_t i = 0; i != data.size(); ++i) {
        REQUIRE(data[i] == received_data[i]);
    }
}

} // namespace sls