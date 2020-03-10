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
#define DO_EVERYTHING					1

#define STATISTIC_FRAMENUMBER_INFINITE	20000

//fifo
#define FIFO_HEADER_NUMBYTES			8


//hdf5
#define MAX_CHUNKED_IMAGES 				1

//versions
#define HDF5_WRITER_VERSION 				4.0 //1 decimal places
#define BINARY_WRITER_VERSION 				4.0 //1 decimal places


//parameters to calculate fifo depth
#define SAMPLE_TIME_IN_NS				100000000//100ms
#define MAX_JOBS_PER_THREAD				1000


//to differentiate between gotthard and short gotthard
#define GOTTHARD_PACKET_SIZE			1286


#define DUMMY_PACKET_VALUE				0xFFFFFFFF

#define LISTENER_PRIORITY				90
#define PROCESSOR_PRIORITY				70
#define STREAMER_PRIORITY				10
#define TCP_PRIORITY					10


struct masterAttributes {
    double version;
    uint32_t detectorType;
    uint32_t dynamicRange;
    uint32_t tenGiga;
    uint32_t imageSize;
    uint32_t nPixelsX;
    uint32_t nPixelsY;
    uint32_t maxFramesPerFile;
    uint64_t totalFrames;
    uint64_t exptimeNs;
    uint64_t subExptimeNs;
    uint64_t subPeriodNs;
    uint64_t periodNs;
    uint32_t gapPixelsEnable;
    uint32_t quadEnable;
 };
