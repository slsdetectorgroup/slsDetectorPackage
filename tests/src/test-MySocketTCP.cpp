#include "catch.hpp"
#include "MySocketTCP.h"
// #include "multiSlsDetector.h"
#include <iostream>
TEST_CASE("Construct") {


//   MySocketTCP(const char* const host_ip_or_name, unsigned short int const port_number):  genericSocket(host_ip_or_name, port_number,TCP){setPacketSize(TCP_PACKET_SIZE);}; // sender (client): where to? ip
    unsigned short int const port_number = 1966;
    auto receiver = MySocketTCP(port_number); 
  REQUIRE(false);
  
}