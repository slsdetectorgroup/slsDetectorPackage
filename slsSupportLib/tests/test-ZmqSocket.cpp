#include "ZmqSocket.h"
#include "catch.hpp"

TEST_CASE("Send header on localhost") {
    constexpr int port = 50001;
    ZmqSocket sub("localhost", port);
    sub.Connect();

    ZmqSocket pub(port, "*");
    

    zmqHeader header;
    
    header.fname = "hej";
    header.data = 0;

    pub.SendHeader(0, header);



    zmqHeader received_header;
    sub.ReceiveHeader(0, received_header, 0);

    REQUIRE(received_header.fname == "hej");
}