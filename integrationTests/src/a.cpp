
#include "catch.hpp"

#include "ClientSocket.h"
#include "Timer.h"
#include "logger.h"
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include "sls_detector_funcs.h"
#include <iostream>
#include <vector>

#include <netdb.h>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define VERBOSE

using sls::RuntimeError;
using sls::SharedMemoryError;
using sls::SocketError;
using sls::DetectorError;

int main() {


    std::string hostname;
    std::cout << "Enter hostname: ";
    std::cin >> hostname;

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    struct sockaddr_in serverAddr {};
    // std::cout << "sizeof(result):" << sizeof(hints) << '\n';
    // std::cout << "sizeof(serverAddr):" << sizeof(serverAddr) << '\n';

    int port = 1952;

    if (getaddrinfo(hostname.c_str(), NULL, &hints, &result) != 0) {
        std::string msg = "ClientSocket cannot decode host:" + hostname + " on port " +
                          std::to_string(port) + "\n";
        throw 5;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr.s_addr, &((struct sockaddr_in *)result->ai_addr)->sin_addr,
           sizeof(in_addr_t));
    freeaddrinfo(result);

    char address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &serverAddr.sin_addr, address, INET_ADDRSTRLEN);
    std::cout << "ip of host is: " << address << '\n';

    sls::ClientSocket(false, serverAddr);

    // const std::string hostname = "beb083";
    // auto type = slsDetector::getTypeFromDetector(hostname);
    // slsDetector d(type);
    // d.setHostname(hostname);
    // d.setOnline(true);
    // std::cout << "hostname: " << d.getHostname() << '\n';
    // d.setThresholdTemperature(50);
    // try{
    //     d.setThresholdTemperature(50);
    // }catch(const DetectorError &e){
    //     std::cout << "Caught: " << e.what() << '\n';
    // }
    // std::cout << "hostname: " << d.getHostname() << '\n';
    // std::cout << "exptime: " << d.setDAC(-1, slsDetectorDefs::E_Vrf, 0) << '\n';




    // slsDetector d2(type);
    // std::cout << "Online: " << d2.getOnlineFlag() << '\n';
    // d2.setHostname("beb55555");
    // d2.setOnline(true);
    // std::cout << "Online: " << d2.getOnlineFlag() << '\n';
    // std::cout << "hostname: " << d2.getHostname() << '\n';

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
