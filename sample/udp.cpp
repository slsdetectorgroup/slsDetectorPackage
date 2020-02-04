#include "UdpSocket.h"
#include <iostream>
#include <chrono>
#include <thread>
int main(){
    std::cout << "HEJ\n";
    sls::UdpSocket s(50010, 1024);

    while(true){
        std::cout << "Got: " << s.ReceivePacket() << " bytes\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}