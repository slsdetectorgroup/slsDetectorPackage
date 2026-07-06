#include "DetectorServerImpl.hpp"
#include "sls/logger.h"
#include <fmt/format.h>

namespace sls {

DetectorServerImpl::DetectorServerImpl() {
    udpDetails[0].srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails[0].dstport = DEFAULT_UDP_DST_PORTNO;

    createSharedMemory();
}

DetectorServerImpl::~DetectorServerImpl() { shm.removeSharedMemory(); }

void DetectorServerImpl::createSharedMemory() {
    shm = SharedMemory<acquisitionStatus>(0, -1, "server");

    if (shm.exists()) {
        shm.openSharedMemory(true); // stop server
    } else {
        LOG(logINFOBLUE) << "Creating shared memory for acquisition status";
        shm.createSharedMemory();
    }
}

void DetectorServerImpl::updateSrcMacAddress(const uint64_t srcmac) {
    LOG(logINFO) << "Updating source MAC address to: "
                 << fmt::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                                (srcmac >> 40) & 0xff, (srcmac >> 32) & 0xff,
                                (srcmac >> 24) & 0xff, (srcmac >> 16) & 0xff,
                                (srcmac >> 8) & 0xff, srcmac & 0xff);

    udpDetails[0].srcmac = srcmac;

    // TODO: update UDP header with new source MAC address

    // TODO: do i need to keep track of the configured member ?
}

bool DetectorServerImpl::get_update_mode() const { return updateMode; }

uint64_t DetectorServerImpl::get_source_udp_mac() const {
    return udpDetails[0].srcmac;
}

void DetectorServerImpl::set_source_udp_ip(const uint32_t srcip) {
    udpDetails[0].srcip = srcip;
}

uint32_t DetectorServerImpl::get_source_udp_ip() const {
    return udpDetails[0].srcip;
}

void DetectorServerImpl::set_destination_udp_ip(const uint32_t dstip) {
    udpDetails[0].dstip = dstip;
}

uint32_t DetectorServerImpl::get_destination_udp_ip() const {
    return udpDetails[0].dstip;
}

void DetectorServerImpl::set_destination_udp_mac(const uint64_t dstmac) {
    // TODO: configuremac, check unicast address
    udpDetails[0].dstmac = dstmac;
}

uint64_t DetectorServerImpl::get_destination_udp_mac() const {
    return udpDetails[0].dstmac;
}

void DetectorServerImpl::set_destination_udp_port(const uint16_t dstport) {
    udpDetails[0].dstport = dstport;
}

uint16_t DetectorServerImpl::get_destination_udp_port() const {
    return udpDetails[0].dstport;
}

} // namespace sls