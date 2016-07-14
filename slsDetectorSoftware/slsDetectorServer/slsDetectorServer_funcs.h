#ifndef SERVER_FUNCS_H
#define SERVER_FUNCS_H

#include "sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <stdlib.h>





//basic server functions
void checkFirmwareCompatibility();
int init_detector(int);
int decode_function(int);
int function_table();
//int swap_int32(int val);
//int64_t swap_int64(int64_t val);
int M_nofunc(int);
int exit_server(int);
int exec_command(int);

//advnaced server functions
int lock_server(int);
int get_last_client_ip(int);
int set_port(int);
int send_update(int);
int update_client(int);
int set_master(int);
int set_synchronization(int);

//detector specific functions
//F_GET_ERROR
int get_detector_type(int);
int set_number_of_modules(int);
int get_max_number_of_modules(int);
int set_external_signal_flag(int);
int set_external_communication_mode(int);
int get_id(int);
int digital_test(int);
//F_ANALOG_TEST
//F_ENABLE_ANALOG_OUT
//F_CALIBRATION_PULSE
int set_dac(int);
int get_adc(int);
int write_register(int);
int read_register(int);
//F_WRITE_MEMORY
//F_READ_MEMORY
int set_channel(int);
int get_channel(int);
//F_SET_ALL_CHANNELS
int set_chip(int);
int get_chip(int);
//F_SET_ALL_CHIPS
int set_module(int);
int get_module(int);
//F_SET_ALL_MODULES
int set_settings(int);
int get_threshold_energy(int);
int set_threshold_energy(int);
int start_acquisition(int);
int stop_acquisition(int);
int start_readout(int);
int get_run_status(int);
int start_and_read_all(int);
int read_frame(int);
int read_all(int);
int set_timer(int);
int get_time_left(int);
int set_dynamic_range(int);
int set_readout_flags(int);
int set_roi(int);
int set_speed(int);
int execute_trimming(int);
int configure_mac(int);
int load_image(int);
int read_counter_block(int);
int reset_counter_block(int);
int start_receiver(int);
int stop_receiver(int);
int calibrate_pedestal(int);
int enable_ten_giga(int);
int set_all_trimbits(int);
int set_counter_bit(int);
int pulse_pixel(int);
int pulse_pixel_and_move(int);
int pulse_chip(int);
int set_rate_correct(int);
int get_rate_correct(int);

#endif
