
#include "catch.hpp"

#include "ClientSocket.h"
#include "Timer.h"
#include "logger.h"
#include "network_utils.h"
#include "slsDetector.h"
#include "multiSlsDetector.h"
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

    std::vector<IpAddr> vec;
    vec.push_back(a);
    return 0;
}
