#include "logger.h"
#include "logger2.h"

#include <iostream>
#include <chrono>
int main(){

    std::cout << "Compare output between old and new:\n";
    FILE_LOG(logINFO) << "Old message";
    LOG(logINFO) << "New message";
    FILE_LOG(logERROR) << "Old error";
    LOG(logERROR) << "New error";
    FILE_LOG(logWARNING) << "Old warning";
    LOG(logWARNING) << "New warning";

    // // sls::Logger::ReportingLevel() = logERROR;

    // const auto N = 100000;
    // auto t0 = std::chrono::steady_clock::now();
    // for (int i = 0; i!=N; ++i){
    //     // LOG(logWARNING) << "HEY";
    //     FILE_LOG(logWARNING) << "HEY";
    // }
    // auto t1 = std::chrono::steady_clock::now();
    // for (int i = 0; i!=N; ++i){
    //     LOG(logWARNING) << "HEY";
    // }
    // auto t2 = std::chrono::steady_clock::now();

    // std::cout << "Old: " << (t1-t0).count() << "\n";
    // std::cout << "New: " << (t2-t1).count() << "\n";
}