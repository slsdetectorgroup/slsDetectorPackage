// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "RegisterDefs.h"
#include "sls/sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN (0x230000)
#define KERNEL_DATE_VRSN  "Wed Nov 29 17:32:14 CET 2023"

#define LINKED_SERVER_NAME "xilinx_ctbDetectorServer"

#define CTRL_SRVR_INIT_TIME_US (2 * 1000 * 1000)

/* Hardware Definitions */
#define NCHAN (1)

enum ADCINDEX { V_PWR_IO };
enum DACINDEX { D0 };


/** Default Parameters */
#define DEFAULT_NUM_FRAMES            (1)
#define DEFAULT_NUM_CYCLES            (1)
