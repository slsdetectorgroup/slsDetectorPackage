// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"
#include <memory>

namespace sls {

class ClientInterface;

class Receiver : private virtual slsDetectorDefs {

  public:
    /**
     * Constructor
     * Starts up a Receiver server.
     * Assembles a Receiver using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    Receiver(uint16_t port = 1954);

    ~Receiver();

    /**
     * get get Receiver Version
     \returns id
     */
    std::string getReceiverVersion();

    /**
     * Start Acquisition Call back (slsMultiReceiver writes data if file write
     * enabled) if registerCallBackRawDataReady or
     * registerCallBackRawDataModifyReady registered
     * Call back arguments are:
     * - startCallbackHeader metadata
     */
    void registerCallBackStartAcquisition(
        void (*func)(const startCallbackHeader, void *), void *arg);

    /**
     * Call back for acquisition finished
     * callback argument is:
     * - startCallbackHeader metadata
     */
    void registerCallBackAcquisitionFinished(
        void (*func)(const endCallbackHeader, void *), void *arg);

    /**
     * Call back for raw data
     * args to raw data ready callback are:
     * - sls_receiver_header frame metadata,
     * - dataCallbackHeader metadata
     * - pointer to data
     * - image size in bytes. Can be modified to the new size to be
     * written/streamed. (only smaller value allowed).
     */
    void registerCallBackRawDataReady(void (*func)(sls_receiver_header &,
                                                   const dataCallbackHeader,
                                                   char *, size_t &, void *),
                                      void *arg);

  private:
    std::unique_ptr<ClientInterface> tcpipInterface;
};

} // namespace sls