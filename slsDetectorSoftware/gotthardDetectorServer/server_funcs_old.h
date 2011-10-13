#ifndef SERVER_FUNCS_H
#define SERVER_FUNCS_H
#include <stdio.h>
#include "communication_funcs.h"




#define GOODBYE -200
int function_table();

int decode_function();

int init_detector(int);

int M_nofunc(int);
int exit_server(int);

int getGotthard(int);
int setGotthard(int);



#endif
