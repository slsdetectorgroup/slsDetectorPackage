// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/sls_detector_defs.h"
#include <cstdint>

namespace sls {

#define MAX_DIMENSIONS                  (2)
#define MAX_NUMBER_OF_LISTENING_THREADS (2)

// socket
#define GOODBYE                    (-200)
#define RECEIVE_SOCKET_BUFFER_SIZE (100 * 1024 * 1024)

#define MAX_SOCKET_INPUT_PACKET_QUEUE (250000)

// files

// versions
#define HDF5_WRITER_VERSION   (6.5) // 1 decimal places
#define BINARY_WRITER_VERSION (7.1) // 1 decimal places

#define MAX_FRAMES_PER_FILE           20000
#define SHORT_MAX_FRAMES_PER_FILE     100000
#define MOENCH_MAX_FRAMES_PER_FILE    100000
#define EIGER_MAX_FRAMES_PER_FILE     10000
#define JFRAU_MAX_FRAMES_PER_FILE     10000
#define CTB_MAX_FRAMES_PER_FILE       20000
#define MYTHEN3_MAX_FRAMES_PER_FILE   10000
#define GOTTHARD2_MAX_FRAMES_PER_FILE 20000

#define STATISTIC_FRAMENUMBER_INFINITE (20000)

// binary
#define FILE_BUFFER_SIZE (16 * 1024 * 1024) // 16mb

// fifo
#define FIFO_HEADER_NUMBYTES   (16)
#define FIFO_DATASIZE_NUMBYTES (4)
#define FIFO_PADDING_NUMBYTES                                                  \
    (4) // for 8 byte alignment due to sls_receiver_header structure

// hdf5
#define MAX_CHUNKED_IMAGES (1)

// parameters to calculate fifo depth
#define SAMPLE_TIME_IN_NS (100000000) // 100ms

// to differentiate between gotthard and short gotthard
#define GOTTHARD_PACKET_SIZE (1286)

#define DUMMY_PACKET_VALUE (0xFFFFFFFF)

#define LISTENER_PRIORITY  (90)
#define PROCESSOR_PRIORITY (70)
#define STREAMER_PRIORITY  (10)
#define TCP_PRIORITY       (10)


#ifdef HDF5C
#define DATASET_NAME "/data"
#endif
} // namespace sls
