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

    void WakeUpReceiver() noexcept;

  public:
    // Linux doubles the requested SO_RCVBUF (kernel bookkeeping overhead) and
    // getsockopt returns the doubled value. macOS/BSD return the size as set.
#ifdef __linux__
    static constexpr int kernelBufferSizeFactor = 2;
#else
    static constexpr int kernelBufferSizeFactor = 1;
#endif

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
