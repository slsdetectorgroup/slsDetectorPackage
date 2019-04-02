
#include "catch.hpp"

#include "ClientSocket.h"
#include "Timer.h"
#include "logger.h"
#include "network_utils.h"
#include "slsDetector.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include "sls_detector_funcs.h"
#include <iostream>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include "network_utils.h"

using namespace sls;

int main() {

    IpAddr a("129.129.205.242");
    IpAddr b(4073554305);

    std::cout << "a is: " << a << " and b is: " << b << "\n";
    if (a == b)
        std::cout << "a is equal to b\n";

    std::cout << "as hex they look like: " << a.hex() << "\n";
    std::cout << "and the best thing is that the size is only: " << sizeof(a) << " bytes\n";


    return 0;
}
