#ifndef SERVER_FUNCS_H
#define SERVER_FUNCS_H


#include "sls_detector_defs.h"


#include <stdio.h>
/*
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
*/
#include "communication_funcs.h"




#define GOODBYE -200

int sockfd;

int function_table();
int decode_function(int);
int init_detector(int);
int M_nofunc(int);





// General purpose functions
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
int send_update(int);
int update_client(int);
int configure_mac(int);
int load_image(int);
int read_counter_block(int);
int reset_counter_block(int);
int write_adc_register(int);
int check_version(int);
#endif
