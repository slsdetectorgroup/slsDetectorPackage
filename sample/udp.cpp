#include "UdpRxSocket.h"
#include "sls_detector_defs.h"
#include <chrono>
#include <fmt/format.h>
#include <iostream>
#include <thread>
#include "network_utils.h"

// Assume packages arrive in order

// Assume frame nr starts from 0

using header_t = slsDetectorDefs::sls_detector_header;

int main() {
    fmt::print("Hej!\n");

    // constexpr ssize_t expected_packages = 128;
    // constexpr ssize_t n_pixels = 512 * 1024;
    constexpr ssize_t packet_size = 8240;
    constexpr ssize_t payload_size = 8240 - sizeof(header_t);
    int port = 50020;
    // fmt::print("header size: {}\n", sizeof(header_t));

    sls::UdpRxSocket s(port, packet_size, nullptr, 212992*2);
    fmt::print("buffer: {}\n", s.getBufferSize());
    s.setBufferSize(212992*4);
    fmt::print("buffer: {}\n", s.getBufferSize());
    // auto header = reinterpret_cast<header_t *>(s.buffer());
    // char *data = s.buffer() + sizeof(header_t);
    // fmt::print("buffer start: {}\nheader: {}\ndata: {}\n", fmt::ptr(s.buffer()),
    //            fmt::ptr(header), fmt::ptr(data));

    // int n = 0;

    // fmt::print("Buffer size: {}\n", s.buffer_size());
    // std::vector<uint16_t> image(n_pixels);
    // char *image_data = (char *)image.data();
    // uint64_t frame_nr = 0;
    // while (true) {

    //     if (s.ReceivePacket()) {

    //         // fmt::print("frame: {} pkt: {} dst: {}\n", header->frameNumber,
    //         // header->packetNumber, header->packetNumber*payload_size);
    //         if (header->frameNumber != frame_nr) {
    //             // dispatch frame
    //             fmt::print("frame {} done! got: {} pkgs\n", frame_nr, n);
    //             frame_nr = header->frameNumber;
    //             n = 0;
    //             std::this_thread::sleep_for(std::chrono::milliseconds(1));
    //         }
    //         ++n;
    //         memcpy(image_data + header->packetNumber * payload_size, data,
    //                payload_size);

    //     } else {
    //         std::cout << "timeout\n";
    //     }
    // }
}