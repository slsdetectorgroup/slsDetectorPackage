#pragma once
#include "sls_detector_defs.h"
#include "logger.h"

enum numberMode {DEC, HEX};
#define GOODBYE 					(-200)
#define REBOOT						(-400)

// initialization functions
int printSocketReadError();
void init_detector();
int decode_function(int);
const char* getRetName();
const char* getTimerName(enum timerIndex ind);
const char* getSpeedName(enum speedVariable ind);
const char* getFunctionName(enum detFuncs func);
void function_table();
void functionNotImplemented();
void modeNotImplemented(char* modename, int mode);
void validate(int arg, int retval, char* modename, enum numberMode nummode);
void validate64(int64_t arg, int64_t retval, char* modename, enum numberMode nummode);
int executeCommand(char* command, char* result, enum TLogLevel level);
int M_nofunc(int);
int M_nofuncMode(int);

// functions called by client
int exec_command(int);
int get_detector_type(int);
int set_external_signal_flag(int);
int set_external_communication_mode(int);
int get_id(int);
int digital_test(int);
int set_dac(int);
int get_adc(int);
int write_register(int);
int read_register(int);
int set_module(int);
int get_module(int);
int set_settings(int);
int get_threshold_energy(int);
int start_acquisition(int);
int stop_acquisition(int);
int start_readout(int);
int get_run_status(int);
int start_and_read_all(int);
int read_all(int);
int set_timer(int);
int get_time_left(int);
int set_dynamic_range(int);
int set_readout_flags(int);
int set_roi(int);
int set_speed(int);
int exit_server(int);
int lock_server(int);
int get_last_client_ip(int);
int set_port(int);
int update_client(int);
int send_update(int);
int configure_mac(int);
int load_image(int);
int read_counter_block(int);
int reset_counter_block(int);
int calibrate_pedestal(int);
int enable_ten_giga(int);
int set_all_trimbits(int);
int set_pattern_io_control(int);
int set_pattern_clock_control(int);
int set_pattern_word(int);
int set_pattern_loop(int);
int set_pattern_wait_addr(int);
int set_pattern_wait_time(int);
int set_pattern_mask(int);
int get_pattern_mask(int);
int set_pattern_bit_mask(int);
int get_pattern_bit_mask(int);
int write_adc_register(int);
int set_counter_bit(int);
int pulse_pixel(int);
int pulse_pixel_and_move(int);
int pulse_chip(int);
int set_rate_correct(int);
int get_rate_correct(int);
int set_network_parameter(int);
int program_fpga(int);
int reset_fpga(int);
int power_chip(int);
int set_activate(int);
int prepare_acquisition(int);
int threshold_temp(int);
int temp_control(int);
int temp_event(int);
int auto_comp_disable(int);
int storage_cell_start(int);
int check_version(int);
int software_trigger(int);
int led(int);
int digital_io_delay(int);
int copy_detector_server(int);
int reboot_controller(int);
int set_adc_enable_mask(int);
int get_adc_enable_mask(int);
int set_adc_invert(int);
int get_adc_invert(int);
int set_external_sampling_source(int);
int set_external_sampling(int);
int set_starting_frame_number(int);
int get_starting_frame_number(int);
int set_quad(int);
int get_quad(int);
int set_interrupt_subframe(int);
int get_interrupt_subframe(int);
int set_read_n_lines(int);
int get_read_n_lines(int);
