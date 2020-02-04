#pragma once
#include "receiver_defs.h"
#include "sls_detector_defs.h"
#include "Implementation.h"
#include "ServerSocket.h"
class MySocketTCP;
class ServerInterface;

#include <atomic>
#include <future>

class ClientInterface : private virtual slsDetectorDefs {
  private:
    enum numberMode { DEC, HEX };

  public:
    virtual ~ClientInterface();
    ClientInterface(int portNumber = -1);
    int64_t getReceiverVersion();

    //***callback functions***
    /** params: filepath, filename, fileindex, datasize */
    void registerCallBackStartAcquisition(int (*func)(std::string, std::string, uint64_t,
                                                      uint32_t, void *),
                                          void *arg);

    /** params: total frames caught */
    void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void *),
                                             void *arg);

    /** params: sls_receiver_header frame metadata, dataPointer, dataSize */
    void registerCallBackRawDataReady(void (*func)(char *, char *, uint32_t,
                                                   void *),
                                      void *arg);

    /** params: sls_receiver_header frame metadata, dataPointer, modified size */
    void registerCallBackRawDataModifyReady(void (*func)(char *, char *,
                                                         uint32_t &, void *),
                                            void *arg);

  private:
    void startTCPServer();
    int functionTable();
    int decodeFunction(sls::ServerInterface &socket);
    void functionNotImplemented();
    void modeNotImplemented(const std::string& modename, int mode);
    template <typename T>
    void validate(T arg, T retval, const std::string& modename, numberMode hex);
    void verifyLock();
    void verifyIdle(sls::ServerInterface &socket);


    int exec_command(sls::ServerInterface &socket);
    int exit_server(sls::ServerInterface &socket);
    int lock_receiver(sls::ServerInterface &socket);
    int get_last_client_ip(sls::ServerInterface &socket);
    int set_port(sls::ServerInterface &socket);
    int update_client(sls::ServerInterface &socket);
    int send_update(sls::ServerInterface &socket);
    int get_version(sls::ServerInterface &socket);
    int set_detector_type(sls::ServerInterface &socket);
    int set_detector_hostname(sls::ServerInterface &socket);
    int set_roi(sls::ServerInterface &socket);
    int set_num_frames(sls::ServerInterface &socket);
    int set_num_analog_samples(sls::ServerInterface &socket);
    int set_num_digital_samples(sls::ServerInterface &socket);
    int set_exptime(sls::ServerInterface &socket);
    int set_period(sls::ServerInterface &socket);
    int set_subexptime(sls::ServerInterface &socket);
    int set_subdeadtime(sls::ServerInterface &socket);
    int set_dynamic_range(sls::ServerInterface &socket);
    int set_streaming_frequency(sls::ServerInterface &socket);
    int get_status(sls::ServerInterface &socket);
    int start_receiver(sls::ServerInterface &socket);
    int stop_receiver(sls::ServerInterface &socket);
    int set_file_dir(sls::ServerInterface &socket);
    int set_file_name(sls::ServerInterface &socket);
    int set_file_index(sls::ServerInterface &socket);
    int get_frame_index(sls::ServerInterface &socket);
    int get_missing_packets(sls::ServerInterface &socket);
    int get_frames_caught(sls::ServerInterface &socket);
    int enable_file_write(sls::ServerInterface &socket);
    int enable_master_file_write(sls::ServerInterface &socket);
    int enable_compression(sls::ServerInterface &socket);
    int enable_overwrite(sls::ServerInterface &socket);
    int enable_tengiga(sls::ServerInterface &socket);
    int set_fifo_depth(sls::ServerInterface &socket);
    int set_activate(sls::ServerInterface &socket);
    int set_data_stream_enable(sls::ServerInterface &socket);
    int set_streaming_timer(sls::ServerInterface &socket);
    int set_flipped_data(sls::ServerInterface &socket);
    int set_file_format(sls::ServerInterface &socket);
    int set_detector_posid(sls::ServerInterface &socket);
    int set_multi_detector_size(sls::ServerInterface &socket);
    int set_streaming_port(sls::ServerInterface &socket);
    int set_streaming_source_ip(sls::ServerInterface &socket);
    int set_silent_mode(sls::ServerInterface &socket);
    int enable_gap_pixels(sls::ServerInterface &socket);
    int restream_stop(sls::ServerInterface &socket);
    int set_additional_json_header(sls::ServerInterface &socket);
    int get_additional_json_header(sls::ServerInterface &socket);
    int set_udp_socket_buffer_size(sls::ServerInterface &socket);
    int get_real_udp_socket_buffer_size(sls::ServerInterface &socket);
    int set_frames_per_file(sls::ServerInterface &socket);
    int check_version_compatibility(sls::ServerInterface &socket);
    int set_discard_policy(sls::ServerInterface &socket);
    int set_padding_enable(sls::ServerInterface &socket);
    int set_deactivated_padding_enable(sls::ServerInterface &socket);
    int set_readout_mode(sls::ServerInterface &socket);
    int set_adc_mask(sls::ServerInterface &socket);
    int set_dbit_list(sls::ServerInterface &socket);
    int get_dbit_list(sls::ServerInterface &socket);
    int set_dbit_offset(sls::ServerInterface &socket);
    int set_quad_type(sls::ServerInterface &socket);
    int set_read_n_lines(sls::ServerInterface &socket);
    int set_udp_ip(sls::ServerInterface &socket);
    int set_udp_ip2(sls::ServerInterface &socket);
    int set_udp_port(sls::ServerInterface &socket);
    int set_udp_port2(sls::ServerInterface &socket);
    int set_num_interfaces(sls::ServerInterface &socket);
    int set_adc_mask_10g(sls::ServerInterface &socket);  
    int set_num_counters(sls::ServerInterface &socket);  

    Implementation *impl() {
        if (receiver != nullptr) {
            return receiver.get();
        } else {
            throw sls::SocketError(
                "Receiver not set up. Please use rx_hostname first.\n");
        }
    }

    detectorType myDetectorType;
    std::unique_ptr<Implementation> receiver{nullptr};
    int (ClientInterface::*flist[NUM_REC_FUNCTIONS])(
        sls::ServerInterface &socket);
    int ret{OK};
    int fnum{-1};
    int lockedByClient{0};
    int portNumber{0};
    std::atomic<bool> killTcpThread{false};
    std::unique_ptr<std::thread> tcpThread;



    //***callback parameters***
   
    int (*startAcquisitionCallBack)(std::string, std::string, uint64_t, uint32_t,
                                    void *) = nullptr;
    void *pStartAcquisition{nullptr};
    void (*acquisitionFinishedCallBack)(uint64_t, void *) = nullptr;
    void *pAcquisitionFinished{nullptr};
    void (*rawDataReadyCallBack)(char *, char *, uint32_t, void *) = nullptr;
    void (*rawDataModifyReadyCallBack)(char *, char *, uint32_t &,
                                       void *) = nullptr;
    void *pRawDataReady{nullptr};

  protected:
    std::unique_ptr<sls::ServerSocket> server{nullptr};
};
