// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "clara.hpp"
#include "sls/ServerSocket.h"

#include "tests/testenum.h"

#include "sls/ServerInterface.h"
#include "sls/container_utils.h"
#include <iostream>
#include <unordered_map>

namespace sls {

// For hashing of enum with C++11, not needed in 14
struct EnumClassHash {
    template <typename T> std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

using Interface = sls::ServerInterface;
using func_ptr = void (*)(Interface &);

/********************************************
 *           Mapped functions               *
 ********************************************/

void read_data(Interface &socket) {
    auto data = sls::make_unique<char[]>(sls::DATA_SIZE);
    std::cout << "Read: " << socket.Receive(data.get(), sls::DATA_SIZE)
              << " bytes into buffer\n";
}

void read_half_data(Interface &socket) {
    auto data = sls::make_unique<char[]>(sls::DATA_SIZE);
    std::cout << "Read: " << socket.Receive(data.get(), sls::DATA_SIZE / 2)
              << " bytes into buffer\n";
}

void read_int(Interface &socket) {
    auto i = socket.Receive<int>();
    std::cout << "Read <int>: " << i << "\n";
}

void read_combined(Interface &socket) {
    auto i = socket.Receive<int>();
    auto d = socket.Receive<double>();
    auto f = socket.Receive<float>();
    std::cout << "read i: " << i << " d: " << d << " f: " << f << "\n";
}

// Map from int to function pointer, in this case probably a map would be faster
std::unordered_map<sls::func_id, func_ptr, EnumClassHash> fmap{
    {sls::func_id::read_data, &read_data},
    {sls::func_id::read_int, &read_int},
    {sls::func_id::read_half_data, &read_half_data},
    {sls::func_id::combined, &read_combined}};

} // namespace sls

int main(int argc, char **argv) {
    std::cout << "Starting test server...\n";
    int port = 2345;

    // Parse command line arguments using clara
    auto cli = clara::Opt(port, "port")["-p"]["--port"]("Port to send to");
    auto result = cli.parse(clara::Args(argc, argv));
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage()
                  << std::endl;
        exit(1);
    }
    std::cout << "Listening to port: " << port << "\n";

    auto server = sls::ServerSocket(port);
    while (true) {
        try {
            auto socket = server.accept();
            auto fnum = socket.Receive<sls::func_id>();
            std::cout << "Calling func: " << (int)fnum << "\n";
            (*sls::fmap[fnum])(socket); // call mapped function

        } catch (const sls::RuntimeError &e) {
            // Do nothing, error is printed when the exeption is created
        }
    }
}
