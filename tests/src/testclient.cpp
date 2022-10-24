// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "clara.hpp"
#include "sls/ClientSocket.h"
#include "sls/sls_detector_exceptions.h"
#include "tests/testenum.h"

#include "sls/container_utils.h"
#include <iostream>

bool help = false;

int main(int argc, char **argv) {
    std::cout << "Test client\n";
    std::string hostname{"localhost"};
    int port = 2345;
    auto cli =
        clara::Help(help) |
        clara::Opt(hostname, "hostname")["-n"]["--hostname"]("Hostname") |
        clara::Opt(port, "port")["-p"]["--port"]("Port to send to");

    auto result = cli.parse(clara::Args(argc, argv));
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage()
                  << std::endl;
        exit(1);
    }
    if (help) {
        std::cout << cli << std::endl;
        return 0;
    }

    std::cout << "Sending to: " << hostname << ":" << port << "\n";

    auto data = sls::make_unique<char[]>(sls::DATA_SIZE);

    // Many connections sending small amounts
    for (int i = 0; i != 100; ++i) {
        std::cout << "Sending: " << i << "\n";
        auto socket = sls::ClientSocket("test", hostname, port);
        std::cout << "Sent: " << socket.Send(sls::func_id::read_int)
                  << " bytes\n";
        std::cout << "Sent: " << socket.Send(i) << " bytes\n";
    }

    // Sending larger blocks
    for (int i = 0; i != 5; ++i) {
        std::cout << "Sending data\n";
        auto socket = sls::ClientSocket("test", hostname, port);
        std::cout << "Sent: " << socket.Send(sls::func_id::read_data)
                  << " bytes\n";
        std::cout << "Sent: " << socket.Send(data.get(), sls::DATA_SIZE)
                  << " bytes\n";
    }

    // Send too little data
    {
        auto socket = sls::ClientSocket("test", hostname, port);
        std::cout << "Sent: " << socket.Send(sls::func_id::read_data)
                  << " bytes\n";
        std::cout << "Sent: " << socket.Send(data.get(), sls::DATA_SIZE / 2)
                  << " bytes\n";
    }
    // Send too much data
    try {
        auto socket = sls::ClientSocket("test", hostname, port);
        std::cout << "Sent: " << socket.Send(sls::func_id::read_half_data)
                  << " bytes\n";
        std::cout << "Sent: " << socket.Send(data.get(), sls::DATA_SIZE)
                  << " bytes\n";
    } catch (const sls::SocketError &e) {
    }
    // Some ints again
    for (int i = 0; i != 10; ++i) {
        std::cout << "Sending: " << i << "\n";
        auto socket = sls::ClientSocket("test", hostname, port);
        std::cout << "Sent: " << socket.Send(sls::func_id::read_int)
                  << " bytes\n";
        std::cout << "Sent: " << socket.Send(i) << " bytes\n";
    }

    // some combined sends
    {
        int a = 9;
        double b = 18.3;
        float c = -1.2;
        auto socket = sls::ClientSocket("test", hostname, port);
        int s = socket.SendAll(sls::func_id::combined, a, b, c);
        std::cout << "send all: " << s << "\n";
    }
}
