#include "ServerSocket.h"
#include "clara.hpp"

#include "tests/testenum.h"

#include "container_utils.h"
#include <iostream>
#include <unordered_map>
#include "ServerInterface2.h"

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

using Interface = sls::ServerInterface2;
using func_ptr = int (*)(Interface &);

int read_data(Interface &socket) {
    auto data = sls::make_unique<char[]>(DATA_SIZE);
    std::cout << "Read: " << socket.receiveData(data.get(), DATA_SIZE)
              << " bytes into buffer\n";
    return 0;
}

int read_int(Interface &socket) {
    auto i = socket.receive<int>();
    std::cout << "Read <int>: " << i << "\n";
    return 0;
}

static std::unordered_map<func_id, func_ptr, EnumClassHash> fmap{
    {func_id::read_data, &read_data}, {func_id::read_int, &read_int}};

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

    while (true) {
        try {
            auto socket = server.accept();
            auto fnum = socket.receive<func_id>();
            std::cout << "Calling func: " << (int)fnum << "\n";
            auto ret = (*fmap[fnum])(socket);
            // std::cout << "function returned: " << ret << "\n";

        } catch (const sls::RuntimeError &e) {
        }
    }
}
