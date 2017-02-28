#pragma once

#include "sls_receiver_defs.h"
#include <stdint.h> 

#define MAX_DIMENSIONS					2
//socket
#define GOODBYE 						-200
#define RECEIVE_SOCKET_BUFFER_SIZE 		(100*1024*1024)
#define MAX_SOCKET_INPUT_PACKET_QUEUE 	250000

//files
#define DO_NOTHING						0
#define CREATE_FILES					1
#define DO_EVERYTHING					2

//binary file/ fifo
#define FILE_FRAME_HDR_FNUM_SIZE		8
#define FILE_FRAME_HDR_SNUM_SIZE		4
#define FILE_FRAME_HDR_BID_SIZE			8
#define FILE_FRAME_HEADER_SIZE			(FILE_FRAME_HDR_FNUM_SIZE + FILE_FRAME_HDR_SNUM_SIZE + FILE_FRAME_HDR_BID_SIZE)
#define FIFO_HEADER_NUMBYTES			4
#define FILE_BUFFER_SIZE        		(16*1024*1024) //16mb

//hdf5
#define MAX_CHUNKED_IMAGES 				1

//versions
#define STREAMER_VERSION				1.0
#define HDF5_WRITER_VERSION 			1.0
#define BINARY_WRITER_VERSION 			1.0 //1 decimal places


//parameters to calculate fifo depth
#define SAMPLE_TIME_IN_NS				100000000//100ms
#define MAX_JOBS_PER_THREAD				1000


#define DUMMY_PACKET_VALUE				0xFFFFFFFF

#define LISTENER_PRIORITY				99
#define PROCESSOR_PRIORITY				90
#define STREAMER_PRIORITY				80
#define TCP_PRIORITY					50
