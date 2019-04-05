
#include "catch.hpp"

#include "ClientSocket.h"
#include "Timer.h"
#include "logger.h"
#include "network_utils.h"
#include "slsDetector.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include "sls_detector_funcs.h"
#include <iomanip>
#include <iostream>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>

#include "network_utils.h"

using namespace sls;
using ROI = slsDetectorDefs::ROI;

// Easy printing of an ROI
std::ostream &operator<<(std::ostream &out, const ROI &r) {
    return out << "xmin: " << std::setw(5) << r.xmin
               << " xmax: " << std::setw(5) << r.xmax
               << " ymin: " << std::setw(5) << r.ymin
               << " ymax: " << std::setw(5) << r.ymax;
}

int main() {

    slsDetectorDefs::ROI roilimits[5];
    roilimits[0].xmin = 5;
    roilimits[0].xmax = 12;
    roilimits[0].ymin = 5;
    roilimits[0].ymax = 15;

    roilimits[1].xmin = 0;
    roilimits[1].xmax = 3;
    roilimits[1].ymin = 20;
    roilimits[1].ymax = 25;

    roilimits[2].xmin = 500;
    roilimits[2].xmax = 600;
    roilimits[2].ymin = 100;
    roilimits[2].ymax = 200;

    roilimits[3].xmin = 300;
    roilimits[3].xmax = 500;
    roilimits[3].ymin = 800;
    roilimits[3].ymax = 900;

    roilimits[4].xmin = 1000;
    roilimits[4].xmax = 2000;
    roilimits[4].ymin = 300;
    roilimits[4].ymax = 500;

    std::cout << "Before sorting:\n";
    for (auto r : roilimits) {
        std::cout << r << '\n';
    }

    std::sort(std::begin(roilimits), std::end(roilimits),
              [](ROI a, ROI b) { return a.xmin < b.xmin; });

    std::cout << "After sorting: \n";
    for (auto r : roilimits) {
        std::cout << r << '\n';
    }
}
