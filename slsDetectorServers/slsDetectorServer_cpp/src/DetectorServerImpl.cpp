#include "DetectorServerImpl.hpp"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include <fmt/format.h>

namespace sls {

template class DetectorServerImpl<true>;  // forward declare
template class DetectorServerImpl<false>; // forward declare

template <bool isStopServer>
DetectorServerImpl<isStopServer>::DetectorServerImpl() {
    udpDetails[0].srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails[0].dstport = DEFAULT_UDP_DST_PORTNO;

    createSharedMemory();
}

template <bool isStopServer>
DetectorServerImpl<isStopServer>::~DetectorServerImpl() {
    shm.removeSharedMemory();
}

template <bool isStopServer>
void DetectorServerImpl<isStopServer>::createSharedMemory() {
    shm = SharedMemory<acquisitionStatus>(0, -1, "server");

    if (shm.exists()) {
        shm.openSharedMemory(true); // stop server
    } else {
        LOG(logINFOBLUE) << "Creating shared memory for acquisition status";
        try {
            shm.createSharedMemory();
        } catch (const SharedMemoryAlreadyExistsError &e) {
            shm.openSharedMemory(true); // potential race conditions between
                                        // stop and control server
        }
    }
}

template <bool isStopServer>
void DetectorServerImpl<isStopServer>::updateSrcMacAddress(
    const uint64_t srcmac) {
    LOG(logINFO) << "Updating source MAC address to: "
                 << fmt::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                                (srcmac >> 40) & 0xff, (srcmac >> 32) & 0xff,
                                (srcmac >> 24) & 0xff, (srcmac >> 16) & 0xff,
                                (srcmac >> 8) & 0xff, srcmac & 0xff);

    udpDetails[0].srcmac = srcmac;

    // TODO: update UDP header with new source MAC address

    // TODO: do i need to keep track of the configured member ?
}

template <bool isStopServer>
bool DetectorServerImpl<isStopServer>::get_update_mode() const {
    return updateMode;
}

template <bool isStopServer>
uint64_t DetectorServerImpl<isStopServer>::get_source_udp_mac() const {
    return udpDetails[0].srcmac;
}

template <bool isStopServer>
void DetectorServerImpl<isStopServer>::set_source_udp_ip(const uint32_t srcip) {
    udpDetails[0].srcip = srcip;
}

template <bool isStopServer>
uint32_t DetectorServerImpl<isStopServer>::get_source_udp_ip() const {
    return udpDetails[0].srcip;
}

template <bool isStopServer>
void DetectorServerImpl<isStopServer>::set_destination_udp_ip(
    const uint32_t dstip) {
    udpDetails[0].dstip = dstip;
}

template <bool isStopServer>
uint32_t DetectorServerImpl<isStopServer>::get_destination_udp_ip() const {
    return udpDetails[0].dstip;
}

template <bool isStopServer>
void DetectorServerImpl<isStopServer>::set_destination_udp_mac(
    const uint64_t dstmac) {
    // TODO: configuremac, check unicast address
    udpDetails[0].dstmac = dstmac;
}

template <bool isStopServer>
uint64_t DetectorServerImpl<isStopServer>::get_destination_udp_mac() const {
    return udpDetails[0].dstmac;
}

template <bool isStopServer>
void DetectorServerImpl<isStopServer>::set_destination_udp_port(
    const uint16_t dstport) {
    udpDetails[0].dstport = dstport;
}

template <bool isStopServer>
uint16_t DetectorServerImpl<isStopServer>::get_destination_udp_port() const {
    return udpDetails[0].dstport;
}

template <bool isStopServer>
detector_setup_status
DetectorServerImpl<isStopServer>::get_detector_setup_status() const {
    return detectorSetupStatus;
}

} // namespace sls