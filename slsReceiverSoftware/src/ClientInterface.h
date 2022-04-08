// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "Implementation.h"
#include "receiver_defs.h"
#include "sls/ServerSocket.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_funcs.h"
class ServerInterface;

#include <atomic>
#include <future>

class ClientInterface : private virtual slsDetectorDefs {
    enum numberMode { DEC, HEX };
    detectorType detType;
    int portNumber{0};
    sls::ServerSocket server;
    std::unique_ptr<Implementation> receiver;
    std::unique_ptr<std::thread> tcpThread;
    int ret{OK};
    int fnum{-1};
    int lockedByClient{0};

    std::atomic<bool> killTcpThread{false};

  public:
    virtual ~ClientInterface();
    ClientInterface(int portNumber = -1);
    int64_t getReceiverVersion();

    //***callback functions***
    /** params: file path, file name, file index, image size */
    void registerCallBackStartAcquisition(int (*func)(const std::string &, const std::string &,
                                                      uint64_t, size_t, void *),
                                          void *arg);

    /** params: total frames caught */
    void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void *),
                                             void *arg);

    /** params: sls_receiver_header pointer, pointer to data, image size */
    void registerCallBackRawDataReady(void (*func)(sls_receiver_header *,
                                                   char *, size_t, void *),
                                      void *arg);

    /** params: sls_receiver_header pointer, pointer to data, reference to image size */
    void registerCallBackRawDataModifyReady(void (*func)(sls_receiver_header *,
                                                         char *, size_t &,
                                                         void *),
                                            void *arg);

  private:
    void startTCPServer();
    int functionTable();
    int decodeFunction(sls::ServerInterface &socket);
    void functionNotImplemented();
    void modeNotImplemented(const std::string &modename, int mode);
    template <typename T>
    void validate(T arg, T retval, const std::string &modename, numberMode hex);
    void verifyLock();
    void verifyIdle(sls::ServerInterface &socket);

    int lock_receiver(sls::ServerInterface &socket);
    int get_last_client_ip(sls::ServerInterface &socket);
    int get_version(sls::ServerInterface &socket);
    int setup_receiver(sls::ServerInterface &socket);
    void setDetectorType(detectorType arg);
    int set_detector_roi(sls::ServerInterface &socket);
    int set_num_frames(sls::ServerInterface &socket);
    int set_num_triggers(sls::ServerInterface &socket);
    int set_num_bursts(sls::ServerInterface &socket);
    int set_num_add_storage_cells(sls::ServerInterface &socket);
    int set_timing_mode(sls::ServerInterface &socket);
    int set_burst_mode(sls::ServerInterface &socket);
    int set_num_analog_samples(sls::ServerInterface &socket);
    int set_num_digital_samples(sls::ServerInterface &socket);
    int set_exptime(sls::ServerInterface &socket);
    int set_period(sls::ServerInterface &socket);
    int set_subexptime(sls::ServerInterface &socket);
    int set_subdeadtime(sls::ServerInterface &socket);
    int set_dynamic_range(sls::ServerInterface &socket);
    int set_streaming_frequency(sls::ServerInterface &socket);
    int get_streaming_frequency(sls::ServerInterface &socket);
    int get_status(sls::ServerInterface &socket);
    int start_receiver(sls::ServerInterface &socket);
    int stop_receiver(sls::ServerInterface &socket);
    int set_file_dir(sls::ServerInterface &socket);
    int get_file_dir(sls::ServerInterface &socket);
    int set_file_name(sls::ServerInterface &socket);
    int get_file_name(sls::ServerInterface &socket);
    int set_file_index(sls::ServerInterface &socket);
    int get_file_index(sls::ServerInterface &socket);
    int get_frame_index(sls::ServerInterface &socket);
    int get_missing_packets(sls::ServerInterface &socket);
    int get_frames_caught(sls::ServerInterface &socket);
    int set_file_write(sls::ServerInterface &socket);
    int get_file_write(sls::ServerInterface &socket);
    int set_master_file_write(sls::ServerInterface &socket);
    int get_master_file_write(sls::ServerInterface &socket);
    int enable_compression(sls::ServerInterface &socket);
    int set_overwrite(sls::ServerInterface &socket);
    int get_overwrite(sls::ServerInterface &socket);
    int enable_tengiga(sls::ServerInterface &socket);
    int set_fifo_depth(sls::ServerInterface &socket);
    int set_activate(sls::ServerInterface &socket);
    int set_streaming(sls::ServerInterface &socket);
    int get_streaming(sls::ServerInterface &socket);
    int set_streaming_timer(sls::ServerInterface &socket);
    int get_flip_rows(sls::ServerInterface &socket);
    int set_flip_rows(sls::ServerInterface &socket);
    int set_file_format(sls::ServerInterface &socket);
    int get_file_format(sls::ServerInterface &socket);
    int set_streaming_port(sls::ServerInterface &socket);
    int get_streaming_port(sls::ServerInterface &socket);
    int set_streaming_source_ip(sls::ServerInterface &socket);
    int get_streaming_source_ip(sls::ServerInterface &socket);
    int set_silent_mode(sls::ServerInterface &socket);
    int get_silent_mode(sls::ServerInterface &socket);
    int restream_stop(sls::ServerInterface &socket);
    int set_additional_json_header(sls::ServerInterface &socket);
    int get_additional_json_header(sls::ServerInterface &socket);
    int set_udp_socket_buffer_size(sls::ServerInterface &socket);
    int get_real_udp_socket_buffer_size(sls::ServerInterface &socket);
    int set_frames_per_file(sls::ServerInterface &socket);
    int get_frames_per_file(sls::ServerInterface &socket);
    int check_version_compatibility(sls::ServerInterface &socket);
    int set_discard_policy(sls::ServerInterface &socket);
    int get_discard_policy(sls::ServerInterface &socket);
    int set_padding_enable(sls::ServerInterface &socket);
    int get_padding_enable(sls::ServerInterface &socket);
    int set_readout_mode(sls::ServerInterface &socket);
    int set_adc_mask(sls::ServerInterface &socket);
    int set_dbit_list(sls::ServerInterface &socket);
    int get_dbit_list(sls::ServerInterface &socket);
    int set_dbit_offset(sls::ServerInterface &socket);
    int get_dbit_offset(sls::ServerInterface &socket);
    int set_quad_type(sls::ServerInterface &socket);
    int set_read_n_rows(sls::ServerInterface &socket);
    sls::MacAddr setUdpIp(sls::IpAddr arg);
    int set_udp_ip(sls::ServerInterface &socket);
    sls::MacAddr setUdpIp2(sls::IpAddr arg);
    int set_udp_ip2(sls::ServerInterface &socket);
    int set_udp_port(sls::ServerInterface &socket);
    int set_udp_port2(sls::ServerInterface &socket);
    int set_num_interfaces(sls::ServerInterface &socket);
    int set_adc_mask_10g(sls::ServerInterface &socket);
    int set_counter_mask(sls::ServerInterface &socket);
    int increment_file_index(sls::ServerInterface &socket);
    int set_additional_json_parameter(sls::ServerInterface &socket);
    int get_additional_json_parameter(sls::ServerInterface &socket);
    int get_progress(sls::ServerInterface &socket);
    int set_num_gates(sls::ServerInterface &socket);
    int set_gate_delay(sls::ServerInterface &socket);
    int get_thread_ids(sls::ServerInterface &socket);
    int get_streaming_start_fnum(sls::ServerInterface &socket);
    int set_streaming_start_fnum(sls::ServerInterface &socket);
    int set_rate_correct(sls::ServerInterface &socket);
    int set_scan(sls::ServerInterface &socket);
    int set_threshold(sls::ServerInterface &socket);
    int get_streaming_hwm(sls::ServerInterface &socket);
    int set_streaming_hwm(sls::ServerInterface &socket);
    int set_all_threshold(sls::ServerInterface &socket);
    int set_detector_datastream(sls::ServerInterface &socket);
    int get_arping(sls::ServerInterface &socket);
    int set_arping(sls::ServerInterface &socket);
    int get_receiver_roi(sls::ServerInterface &socket);
    int set_receiver_roi(sls::ServerInterface &socket);

    Implementation *impl() {
        if (receiver != nullptr) {
            return receiver.get();
        } else {
            throw sls::SocketError(
                "Receiver not set up. Please use rx_hostname first.\n");
        }
    }

    int (ClientInterface::*flist[NUM_REC_FUNCTIONS])(
        sls::ServerInterface &socket);

    //***callback parameters***

    int (*startAcquisitionCallBack)(const std::string &, const std::string &, uint64_t, size_t,
                                    void *) = nullptr;
    void *pStartAcquisition{nullptr};
    void (*acquisitionFinishedCallBack)(uint64_t, void *) = nullptr;
    void *pAcquisitionFinished{nullptr};
    void (*rawDataReadyCallBack)(sls_receiver_header *, char *, size_t,
                                 void *) = nullptr;
    void (*rawDataModifyReadyCallBack)(sls_receiver_header *, char *, size_t &,
                                       void *) = nullptr;
    void *pRawDataReady{nullptr};

    pid_t parentThreadId{0};
    pid_t tcpThreadId{0};
    std::vector<std::string> udpips =
        std::vector<std::string>(MAX_NUMBER_OF_LISTENING_THREADS);
};
