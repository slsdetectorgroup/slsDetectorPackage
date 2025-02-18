// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "Implementation.h"
#include "receiver_defs.h"
#include "sls/ServerSocket.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_funcs.h"

#include <atomic>
#include <future>

namespace sls {

class ServerInterface;

class ClientInterface : private virtual slsDetectorDefs {
    enum numberMode { DEC, HEX };
    detectorType detType;
    uint16_t portNumber{0};
    ServerSocket server;
    std::unique_ptr<Implementation> receiver;
    std::unique_ptr<std::thread> tcpThread;
    int ret{OK};
    int fnum{-1};
    int lockedByClient{0};

    std::atomic<bool> killTcpThread{false};

  public:
    virtual ~ClientInterface();
    ClientInterface(uint16_t portNumber = DEFAULT_TCP_RX_PORTNO);
    std::string getReceiverVersion();

    //***callback functions***
    /** params: file path, file name, file index, image size */
    void registerCallBackStartAcquisition(int (*func)(const startCallbackHeader,
                                                      void *),
                                          void *arg);

    /** params: total frames caught */
    void registerCallBackAcquisitionFinished(
        void (*func)(const endCallbackHeader, void *), void *arg);

    /** params: sls_receiver_header, pointer to data, image size */
    void registerCallBackRawDataReady(void (*func)(sls_receiver_header &,
                                                   const dataCallbackHeader,
                                                   char *, size_t &, void *),
                                      void *arg);

  private:
    void startTCPServer();
    int functionTable();
    int decodeFunction(ServerInterface &socket);
    void functionNotImplemented();
    void modeNotImplemented(const std::string &modename, int mode);
    template <typename T>
    void validate(T arg, T retval, const std::string &modename, numberMode hex);
    void verifyLock();
    void verifyIdle(ServerInterface &socket);

    int lock_receiver(ServerInterface &socket);
    int get_last_client_ip(ServerInterface &socket);
    int get_version(ServerInterface &socket);
    int setup_receiver(ServerInterface &socket);
    void setDetectorType(detectorType arg);
    int set_detector_roi(ServerInterface &socket);
    int set_num_frames(ServerInterface &socket);
    int set_num_triggers(ServerInterface &socket);
    int set_num_bursts(ServerInterface &socket);
    int set_num_add_storage_cells(ServerInterface &socket);
    int set_timing_mode(ServerInterface &socket);
    int set_burst_mode(ServerInterface &socket);
    int set_num_analog_samples(ServerInterface &socket);
    int set_num_digital_samples(ServerInterface &socket);
    int set_exptime(ServerInterface &socket);
    int set_period(ServerInterface &socket);
    int set_subexptime(ServerInterface &socket);
    int set_subdeadtime(ServerInterface &socket);
    int set_dynamic_range(ServerInterface &socket);
    int set_streaming_frequency(ServerInterface &socket);
    int get_streaming_frequency(ServerInterface &socket);
    int get_status(ServerInterface &socket);
    int start_receiver(ServerInterface &socket);
    int stop_receiver(ServerInterface &socket);
    int set_file_dir(ServerInterface &socket);
    int get_file_dir(ServerInterface &socket);
    int set_file_name(ServerInterface &socket);
    int get_file_name(ServerInterface &socket);
    int set_file_index(ServerInterface &socket);
    int get_file_index(ServerInterface &socket);
    int get_frame_index(ServerInterface &socket);
    int get_missing_packets(ServerInterface &socket);
    int get_frames_caught(ServerInterface &socket);
    int set_file_write(ServerInterface &socket);
    int get_file_write(ServerInterface &socket);
    int set_master_file_write(ServerInterface &socket);
    int get_master_file_write(ServerInterface &socket);
    int enable_compression(ServerInterface &socket);
    int set_overwrite(ServerInterface &socket);
    int get_overwrite(ServerInterface &socket);
    int enable_tengiga(ServerInterface &socket);
    int set_fifo_depth(ServerInterface &socket);
    int set_activate(ServerInterface &socket);
    int set_streaming(ServerInterface &socket);
    int get_streaming(ServerInterface &socket);
    int set_streaming_timer(ServerInterface &socket);
    int get_flip_rows(ServerInterface &socket);
    int set_flip_rows(ServerInterface &socket);
    int set_file_format(ServerInterface &socket);
    int get_file_format(ServerInterface &socket);
    int set_streaming_port(ServerInterface &socket);
    int get_streaming_port(ServerInterface &socket);
    int set_silent_mode(ServerInterface &socket);
    int get_silent_mode(ServerInterface &socket);
    int restream_stop(ServerInterface &socket);
    int set_additional_json_header(ServerInterface &socket);
    int get_additional_json_header(ServerInterface &socket);
    int set_udp_socket_buffer_size(ServerInterface &socket);
    int get_real_udp_socket_buffer_size(ServerInterface &socket);
    int set_frames_per_file(ServerInterface &socket);
    int get_frames_per_file(ServerInterface &socket);
    int set_discard_policy(ServerInterface &socket);
    int get_discard_policy(ServerInterface &socket);
    int set_padding_enable(ServerInterface &socket);
    int get_padding_enable(ServerInterface &socket);
    int set_readout_mode(ServerInterface &socket);
    int set_adc_mask(ServerInterface &socket);
    int set_dbit_list(ServerInterface &socket);
    int get_dbit_list(ServerInterface &socket);
    int set_dbit_offset(ServerInterface &socket);
    int get_dbit_offset(ServerInterface &socket);
    int set_quad_type(ServerInterface &socket);
    int set_read_n_rows(ServerInterface &socket);
    MacAddr setUdpIp(IpAddr arg);
    int set_udp_ip(ServerInterface &socket);
    MacAddr setUdpIp2(IpAddr arg);
    int set_udp_ip2(ServerInterface &socket);
    int set_udp_port(ServerInterface &socket);
    int set_udp_port2(ServerInterface &socket);
    int set_num_interfaces(ServerInterface &socket);
    int set_adc_mask_10g(ServerInterface &socket);
    int set_counter_mask(ServerInterface &socket);
    int increment_file_index(ServerInterface &socket);
    int set_additional_json_parameter(ServerInterface &socket);
    int get_additional_json_parameter(ServerInterface &socket);
    int get_progress(ServerInterface &socket);
    int set_num_gates(ServerInterface &socket);
    int set_gate_delay(ServerInterface &socket);
    int get_thread_ids(ServerInterface &socket);
    int get_streaming_start_fnum(ServerInterface &socket);
    int set_streaming_start_fnum(ServerInterface &socket);
    int set_rate_correct(ServerInterface &socket);
    int set_scan(ServerInterface &socket);
    int set_threshold(ServerInterface &socket);
    int get_streaming_hwm(ServerInterface &socket);
    int set_streaming_hwm(ServerInterface &socket);
    int set_all_threshold(ServerInterface &socket);
    int set_detector_datastream(ServerInterface &socket);
    int get_arping(ServerInterface &socket);
    int set_arping(ServerInterface &socket);
    int get_receiver_roi(ServerInterface &socket);
    int set_receiver_roi(ServerInterface &socket);
    int set_receiver_roi_metadata(ServerInterface &socket);
    int set_num_transceiver_samples(ServerInterface &socket);
    int set_transceiver_mask(ServerInterface &socket);
    int set_row(ServerInterface &socket);
    int set_column(ServerInterface &socket);

    Implementation *impl() {
        if (receiver != nullptr) {
            return receiver.get();
        } else {
            throw SocketError(
                "Receiver not set up. Please use rx_hostname first.\n");
        }
    }

    int (ClientInterface::*flist[NUM_REC_FUNCTIONS])(ServerInterface &socket);

    //***callback parameters***

    int (*startAcquisitionCallBack)(const startCallbackHeader,
                                    void *) = nullptr;
    void *pStartAcquisition{nullptr};
    void (*acquisitionFinishedCallBack)(const endCallbackHeader,
                                        void *) = nullptr;
    void *pAcquisitionFinished{nullptr};
    void (*rawDataReadyCallBack)(sls_receiver_header &, dataCallbackHeader,
                                 char *, size_t &, void *) = nullptr;
    void *pRawDataReady{nullptr};

    pid_t parentThreadId{0};
    pid_t tcpThreadId{0};
    std::vector<std::string> udpips =
        std::vector<std::string>(MAX_NUMBER_OF_LISTENING_THREADS);
    // necessary if Receiver objects using threads with callbacks
    static std::mutex callbackMutex;
};

} // namespace sls
