#include "catch.hpp"
#include "logger.h"
#include "logger2.h"

#include <iostream>
#include <fstream>
#include <chrono>

TEST_CASE("Get time"){
    auto now = std::chrono::system_clock::now();
    sls::Logger log;
    auto time = log.Time(now); 

}

TEST_CASE("fail"){

    FILE_LOG(logINFO) << "A message";
    FILE_LOG(logWARNING) << "An error";
    FILE_LOG(logERROR) << "A warning";

// sls::Logger::ReportingLevel() = logERROR;
    // std::cout << sls::Logger::ReportingLevel() << '\n';
    LOG(logINFO) << "A new message";
    LOG(logERROR) << "A new error";
    LOG(logWARNING) << "A new warning";

    LOG(logDEBUG3) << "This should not be printed";

    std::ostringstream local;
    auto clog_buff = std::clog.rdbuf();
    std::clog.rdbuf(local.rdbuf());
    LOG(logERROR) << "This should also not be printed";

    std::clog.rdbuf(clog_buff); // restore
    LOG(logERROR) << "But this should";

    std::cout << "we got: " << local.str() << '\n';


    // sls::Logger::ReportingLevel() = logDEBUG1;
    // std::cout << sls::Logger::ReportingLevel() << '\n';
    // std::ostream& os = std::cout;
    // Output2FILE::Stream();

    // os << "hej pa dig\n";

    // std::ostringstream oss;
    // std::ostream& os = oss;
    // auto& out = Output2FILE::Stream();
    // out = os;
    // Output2FILE::Stream() = std::cout;
    // FILE_LOG(logERROR) << "An error message";


    // CHECK(false);


}