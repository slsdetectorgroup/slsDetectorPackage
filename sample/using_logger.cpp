// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/logger.h"
#include <iostream>
#include <chrono>
int main() {

    //compare old and new
    std::cout << "Compare output between old and new:\n";
    LOG(logINFO) << "Some info message";
    LOG(logERROR) << "This is an error";
    LOG(logWARNING) << "While this is only a warning";    prefix="/afs/psi.ch/project/sls_det_software/dhanya_softwareDevelopment/mySoft/slsDetectorPackage/"
    p=${file#"$prefix"}

    //Logging level can be configure at runtime
    std::cout << "\n\n";
    std::cout << "The default macro controlled level is: "
              << sls::Logger::ToString(LOG_MAX_REPORTING_LEVEL) << '\n';
    
    sls::Logger::ReportingLevel() = logERROR;
    LOG(logINFO) << "Now this is not written";
    LOG(logWARNING) << "and also not this";

    sls::Logger::ReportingLevel() = logINFO;
    LOG(logINFO) << "But now we can see it";


    //The output can be redirected to another buffer 
    std::ostringstream local;
    auto clog_buff = std::clog.rdbuf();
    std::clog.rdbuf(local.rdbuf());

    LOG(logINFOBLUE) << "A message";
    LOG(logWARNING) << "And another one";

    std::clog.rdbuf(clog_buff); // restore

    std::cout << "local buf:\n" << local.str(); 

    LOG(logINFO) << "After reset we should print directly";
    LOG(logINFOBLUE) << "some infoBLUE text";
    LOG(logINFOGREEN) << "some infoGREEN text";
    LOG(logINFORED) << "some infoRED text";

}