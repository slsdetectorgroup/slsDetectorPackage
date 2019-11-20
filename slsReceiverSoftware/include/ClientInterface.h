#pragma once
#include "receiver_defs.h"
#include "sls_detector_defs.h"
#include "Implementation.h"
#include "ServerSocket.h"
class MySocketTCP;
class ServerInterface;



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
    void startTCPSocket();
    void stopTCPSocket();
    static void *startTCPServerThread(void *this_pointer);
    void startTCPServer();

    int function_table();
    int decode_function(sls::ServerInterface2 &socket);
    void functionNotImplemented();
    void modeNotImplemented(const std::string& modename, int mode);
    template <typename T>
    void validate(T arg, T retval, std::string modename, numberMode hex);

    int exec_command(sls::ServerInterface2 &socket);
    int exit_server(sls::ServerInterface2 &socket);
    int lock_receiver(sls::ServerInterface2 &socket);
    int get_last_client_ip(sls::ServerInterface2 &socket);
    int set_port(sls::ServerInterface2 &socket);
    int update_client(sls::ServerInterface2 &socket);
    int send_update(sls::ServerInterface2 &socket);
    int get_version(sls::ServerInterface2 &socket);
    int set_detector_type(sls::ServerInterface2 &socket);
    int set_detector_hostname(sls::ServerInterface2 &socket);
    int set_roi(sls::ServerInterface2 &socket);
    int set_num_frames(sls::ServerInterface2 &socket);
    int set_num_analog_samples(sls::ServerInterface2 &socket);
    int set_num_digital_samples(sls::ServerInterface2 &socket);
    int set_exptime(sls::ServerInterface2 &socket);
    int set_period(sls::ServerInterface2 &socket);
    int set_subexptime(sls::ServerInterface2 &socket);
    int set_subdeadtime(sls::ServerInterface2 &socket);
    int set_dynamic_range(sls::ServerInterface2 &socket);
    int set_streaming_frequency(sls::ServerInterface2 &socket);
    int get_status(sls::ServerInterface2 &socket);
    int start_receiver(sls::ServerInterface2 &socket);
    int stop_receiver(sls::ServerInterface2 &socket);
    int set_file_dir(sls::ServerInterface2 &socket);
    int set_file_name(sls::ServerInterface2 &socket);
    int set_file_index(sls::ServerInterface2 &socket);
    int get_frame_index(sls::ServerInterface2 &socket);
    int get_missing_packets(sls::ServerInterface2 &socket);
    int get_frames_caught(sls::ServerInterface2 &socket);
    int enable_file_write(sls::ServerInterface2 &socket);
    int enable_master_file_write(sls::ServerInterface2 &socket);
    int enable_compression(sls::ServerInterface2 &socket);
    int enable_overwrite(sls::ServerInterface2 &socket);
    int enable_tengiga(sls::ServerInterface2 &socket);
    int set_fifo_depth(sls::ServerInterface2 &socket);
    int set_activate(sls::ServerInterface2 &socket);
    int set_data_stream_enable(sls::ServerInterface2 &socket);
    int set_streaming_timer(sls::ServerInterface2 &socket);
    int set_flipped_data(sls::ServerInterface2 &socket);
    int set_file_format(sls::ServerInterface2 &socket);
    int set_detector_posid(sls::ServerInterface2 &socket);
    int set_multi_detector_size(sls::ServerInterface2 &socket);
    int set_streaming_port(sls::ServerInterface2 &socket);
    int set_streaming_source_ip(sls::ServerInterface2 &socket);
    int set_silent_mode(sls::ServerInterface2 &socket);
    int enable_gap_pixels(sls::ServerInterface2 &socket);
    int restream_stop(sls::ServerInterface2 &socket);
    int set_additional_json_header(sls::ServerInterface2 &socket);
    int get_additional_json_header(sls::ServerInterface2 &socket);
    int set_udp_socket_buffer_size(sls::ServerInterface2 &socket);
    int get_real_udp_socket_buffer_size(sls::ServerInterface2 &socket);
    int set_frames_per_file(sls::ServerInterface2 &socket);
    int check_version_compatibility(sls::ServerInterface2 &socket);
    int set_discard_policy(sls::ServerInterface2 &socket);
    int set_padding_enable(sls::ServerInterface2 &socket);
    int set_deactivated_padding_enable(sls::ServerInterface2 &socket);
    int set_readout_mode(sls::ServerInterface2 &socket);
    int set_adc_mask(sls::ServerInterface2 &socket);
    int set_dbit_list(sls::ServerInterface2 &socket);
    int get_dbit_list(sls::ServerInterface2 &socket);
    int set_dbit_offset(sls::ServerInterface2 &socket);
    int set_quad_type(sls::ServerInterface2 &socket);
    int set_read_n_lines(sls::ServerInterface2 &socket);
    int set_udp_ip(sls::ServerInterface2 &socket);
    int set_udp_ip2(sls::ServerInterface2 &socket);
    int set_udp_port(sls::ServerInterface2 &socket);
    int set_udp_port2(sls::ServerInterface2 &socket);
    int set_num_interfaces(sls::ServerInterface2 &socket);

    detectorType myDetectorType;
    std::unique_ptr<Implementation> receiver{nullptr};
    int (ClientInterface::*flist[NUM_REC_FUNCTIONS])(
        sls::ServerInterface2 &socket);
    int ret{OK};
    int fnum{-1};
    /** Lock Status if server locked to a client */
    int lockStatus{0};

    int killTCPServerThread{0};
    pthread_t TCPServer_thread;
    bool tcpThreadCreated{false};

    int portNumber;

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

  private:
    void VerifyLock();
    void VerifyIdle(sls::ServerInterface2 &socket);

    Implementation *impl() {
        if (receiver != nullptr) {
            return receiver.get();
        } else {
            throw sls::SocketError(
                "Receiver not set up. Please use rx_hostname first.\n");
        }
    }
};
