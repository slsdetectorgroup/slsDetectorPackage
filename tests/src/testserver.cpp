#include "ServerSocket.h"
#include "clara.hpp"

#include <iostream>
#include "container_utils.h"

constexpr size_t MB = 1048576;

int main(int argc, char **argv) {
    std::cout << "Test server\n";
    int port = 2345;
    auto cli = clara::Opt(port, "port")["-p"]["--port"]("Port to send to");

    auto result = cli.parse(clara::Args(argc, argv));
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage()
                  << std::endl;
        exit(1);
    }
    std::cout << "Listening to port: " << port << "\n";
    auto server = sls::ServerSocket(port);

    auto data = sls::make_unique<char[]>(1*MB);

    while (true) {
        try {
            auto socket = server.accept();
            auto val = socket.receive<long>();
            std::cout << "Value: " << val << "\n";
            std::cout << "Read: " << socket.receiveData(data.get(), 1*MB) << " bytes";

        } catch (const sls::RuntimeError &e) {
        }
    }
}
