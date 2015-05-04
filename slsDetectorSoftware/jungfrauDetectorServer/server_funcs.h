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
int init_detector(int,int);

int M_nofunc(int);
int exit_server(int);




// General purpose functions
int get_detector_type(int);
int set_number_of_modules(int);
int get_max_number_of_modules(int);


int exec_command(int);
int set_external_signal_flag(int);
int set_external_communication_mode(int);
int get_id(int);
int digital_test(int);
int write_register(int);
int read_register(int);
int set_dac(int);
int get_adc(int);
int set_channel(int);
int set_chip(int);
int set_module(int);
int get_channel(int);
int get_chip(int);
int get_module(int);

int get_threshold_energy(int); 
int set_threshold_energy(int);  
int set_settings(int);
int start_acquisition(int);
int stop_acquisition(int);
int start_readout(int);
int get_run_status(int);
int read_frame(int);
int read_all(int);
int start_and_read_all(int);
int set_timer(int);
int get_time_left(int);
int set_dynamic_range(int);
int set_roi(int);
int get_roi(int);
int set_speed(int);
void prepareADC(void);
int set_readout_flags(int);
int execute_trimming(int);
int lock_server(int);
int set_port(int);
int get_last_client_ip(int);
int set_master(int);
int set_synchronization(int);

int update_client(int);
int send_update(int);
int configure_mac(int);

int load_image(int);
int read_counter_block(int);
int reset_counter_block(int);

int start_receiver(int);
int stop_receiver(int);


int calibrate_pedestal(int);

int set_roi(int);
int set_ctb_pattern(int);

int write_adc_register(int);;
#endif
