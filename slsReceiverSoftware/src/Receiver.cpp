// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/Receiver.h"
#include "ClientInterface.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"

#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>

namespace sls {

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

Receiver::~Receiver() = default;

Receiver::Receiver(uint16_t port) {
    validatePortNumber(port);
    tcpipInterface = make_unique<ClientInterface>(port);
}

std::string Receiver::getReceiverVersion() {
    return tcpipInterface->getReceiverVersion();
}

void Receiver::registerCallBackStartAcquisition(
    void (*func)(const startCallbackHeader, void *), void *arg) {
    tcpipInterface->registerCallBackStartAcquisition(func, arg);
}

void Receiver::registerCallBackAcquisitionFinished(
    void (*func)(const endCallbackHeader, void *), void *arg) {
    tcpipInterface->registerCallBackAcquisitionFinished(func, arg);
}

void Receiver::registerCallBackRawDataReady(
    void (*func)(sls_receiver_header &, const dataCallbackHeader, char *,
                 size_t &, void *),
    void *arg) {
    tcpipInterface->registerCallBackRawDataReady(func, arg);
}

} // namespace sls