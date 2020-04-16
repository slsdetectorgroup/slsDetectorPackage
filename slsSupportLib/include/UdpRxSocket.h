
#pragma once
/*
UDP socket class to receive data. The intended use is in the 
receiver listener loop. Should be used RAII style...
*/

#include <sys/types.h> //ssize_t
namespace sls {

class UdpRxSocket {
    const ssize_t packet_size_;
    int sockfd_{-1};

  public:
    UdpRxSocket(int port, ssize_t packet_size, const char *hostname = nullptr,
                size_t kernel_buffer_size = 0);
    ~UdpRxSocket();
    bool ReceivePacket(char *dst) noexcept;
    size_t getBufferSize() const;
    void setBufferSize(ssize_t size);
    ssize_t getPacketSize() const noexcept;
    void Shutdown();

    // Only for backwards compatibility, this drops the EIGER small pkt, may be
    // removed
    ssize_t ReceiveDataOnly(char *dst) noexcept;
};

} // namespace sls
