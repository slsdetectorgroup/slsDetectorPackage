#ifndef RECEIVER_H
#define RECEIVER_H
#include <stdio.h>
#include "communication_funcs.h"




#define GOODBYE -200

int sockfd;

int function_table();
int decode_function(int);
int M_nofunc(int);
int exit_server(int);
int exec_command(int);


int lock_receiver(int);
int set_port(int);
int get_last_client_ip(int);
int update_client(int);
int send_update(int);
//int set_master(int);
//int set_synchronization(int);

// General purpose functions

//int init_receiver();
int set_file_name(int);
int set_file_dir(int);
int set_file_index(int);
int start_receiver(int);
int stop_receiver(int);
int get_receiver_status(int);
int get_frames_caught(int);
int get_frame_index(int);



#endif
