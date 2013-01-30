#ifndef RECEIVER_DEFS_H
#define RECEIVER_DEFS_H

#include "sls_detector_defs.h"

#include <stdint.h> 

#define GOODBYE 				-200

#define BUFFER_SIZE 			1286*2
#define DATA_BYTES	 			2560
#define MAX_FRAMES				20000
#define PACKETS_PER_FRAME		2

#define SHORT_BUFFER_SIZE		518
#define SHORT_MAX_FRAMES		100000
#define SHORT_PACKETS_PER_FRAME	1


#define FIFO_SIZE				25000

//#define THIS_SOFTWARE_VERSION 0x20120919
#endif
