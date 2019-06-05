#include "ClientSocket.h"
#include "clara.hpp"

#include <iostream>
#include "container_utils.h"

int main(int argc, char** argv) {
    std::cout << "Test client\n";
    std::string hostname{"localhost"};
    int port = 2345;
    auto cli =
        clara::Opt(hostname, "hostname")["-hn"]["--hostname"]("Hostname") |
        clara::Opt(port, "port")["-p"]["--port"]("Port to send to");

    auto result = cli.parse(clara::Args(argc, argv));
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage()
                  << std::endl;
        exit(1);
    }
    std::cout << "Sending to: " << hostname << ":" << port << "\n";
    
    auto data = sls::make_unique<char[]>(5000);

    for (int64_t i = 0; i!=10; ++i){
        std::cout << "Sending: " << i << "\n";
        auto socket = sls::ClientSocket("test", hostname, port);
        std::cout << "Sent: " << socket.sendData(i) << " bytes\n";
        std::cout << "Sent: " << socket.sendData(data.get(), 5000) << " bytes\n";

    }
        


}
