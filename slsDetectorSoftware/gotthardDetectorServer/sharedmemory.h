#ifndef SM
#define SM

#include "sls_detector_defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
//#include <asm/page.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>


#include <sys/shm.h>

#include <sys/ipc.h>
#include <sys/stat.h>

/* key for shared memory */
#define SMKEY 10001

#define SMSV 1
#define SMCL 2


struct statusdata {
  int runnumber;
  int stop;
  char status[20];
} ;


/* for shared memory */

int inism(int clsv);
void write_status_sm(char *status);
void write_stop_sm(int v);
void write_runnumber_sm(int v);

#endif
