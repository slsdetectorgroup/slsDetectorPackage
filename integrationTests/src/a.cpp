
#include "catch.hpp"

#include "ClientSocket.h"
#include "Timer.h"
#include "logger.h"
#include "slsDetector.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include "sls_detector_funcs.h"
#include <iostream>
#include <vector>

#define VERBOSE

using sls::RuntimeError;
using sls::SharedMemoryError;
using sls::SocketError;

int main() {


    // const std::string hostname = "beb083";
    // auto type = slsDetector::getTypeFromDetector(hostname);
    // slsDetector d(type);
    // d.setHostname(hostname);
    // d.setReceiverHostname("mpc2408");
    // std::cout << "hostname: " << d.getHostname() << '\n';
    // std::cout << "port: " << d.getControlPort() << '\n';
    // d.setOnline(true);
    // d.setReceiverOnline(true);
    // std::cout << "reciver version: " << std::hex << d.getReceiverVersion() << '\n';
    // // std::cout << "version: " << d.getId(slsDetectorDefs::CLIENT_RECEIVER_API_VERSION) << '\n';
    // d.freeSharedMemory();
    // //Catch exception
    // try {
    //     throw RuntimeError("something went wrong");
    // } catch (RuntimeError &e) {
    //     std::cout << "Caught RuntimeError with message : " << e.what() << '\n';
    // }

    // //Catch base class
    // try {
    //     throw SharedMemoryError("Could not create shared memory");
    // } catch (RuntimeError &e) {
    //     std::cout << "Caught: " << e.what() << '\n';
    // }

    // //Catch base class after looking for something else
    // try {
    //     throw SharedMemoryError("Could not create shared memory");
    // } catch (SocketError &e) {

    //     std::cout << "Caught Socket error: " << e.what() << '\n';

    // } catch (RuntimeError &e) {
    //     std::cout << "Caught base class: " << e.what() << '\n';
    // }

    // //Catch any after looking for something else
    // try {
    //     throw SharedMemoryError("Could not create shared memory");
    // } catch (SocketError &e) {

    //     std::cout << "Caught Socket error: " << e.what() << '\n';

    // } catch (...) {
    //     std::cout << "Caught Something else probably should have let me crash\n";
    // }


    // throw RuntimeError("This one we missed");
    return 0;
}
