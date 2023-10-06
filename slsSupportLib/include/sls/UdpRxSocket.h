// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once
/*
UDP socket class to receive data. The intended use is in the
receiver listener loop. Should be used RAII style...
*/

#include <stdint.h>
#include <sys/types.h> //ssize_t
namespace sls {

class UdpRxSocket {
    const ssize_t packet_size_;
    int sockfd_{-1};

  public:
    UdpRxSocket(uint16_t port, ssize_t packet_size,
                const char *hostname = nullptr, int kernel_buffer_size = 0);
    ~UdpRxSocket();
    bool ReceivePacket(char *dst) noexcept;
    int getBufferSize() const;
    void setBufferSize(int size);
    ssize_t getPacketSize() const noexcept;
    void Shutdown();
};

} // namespace sls
