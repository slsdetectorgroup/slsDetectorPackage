// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "XILINX_FMC.h"
#include "arm64.h"
#include "clogger.h"
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

// clang-format off
#define FMC_BASE_PATH "/root/fmc/"
#define FMC_VADJ_EN "FMC_VADJ_EN"
#define FMCP_VADJ_EN "FMCP_VADJ_EN"
#define FMCP_3V3_EN "FMCP_3V3_EN"
#define FMC_3V3_EN "FMC_3V3_EN"
#define FMC_12V_EN "FMC_12V_EN"
#define FMCP_12V_EN "FMCP_12V_EN"

static const char *fmc_files[] = {
    FMC_VADJ_EN,
    FMCP_VADJ_EN,
    FMCP_3V3_EN,
    FMC_3V3_EN,
    FMC_12V_EN,
    FMCP_12V_EN
};
#define FMC_NUM_FILES (sizeof(fmc_files) / sizeof(fmc_files[0]))
// clang-format on

int XILINX_FMC_enable_all(char* error_message, int message_size) {
    LOG(logINFOBLUE, ("enable FMC power\n"));
    #ifdef VIRTUAL
        return;
    #endif
    char full_path[64];
    for (size_t i = 0; i < FMC_NUM_FILES; ++i) {
        const char* file = fmc_files[i];
        snprintf(full_path, sizeof(full_path), "%s%s", FMC_BASE_PATH, file);
        FILE *fp = fopen(full_path, "w");
        if (fp == NULL) {
            snprintf(error_message, message_size, "XILINX_FMC: Couuld not enable.\n");
            LOG(logERROR, (error_message));
            return 1;
        }

        if (fprintf(fp, "1\n") != 1) {
            snprintf(error_message, message_size, "XILINX_FMC: Could not write enable.\n");
            LOG(logERROR, (error_message));
            return 1;
        }
        fclose(fp);
    }
    return 0;
}

int XILINX_FMC_disable_all(char* error_message, int message_size) {
    LOG(logINFOBLUE, ("disable FMC power\n"));
    #ifdef VIRTUAL
        return;
    #endif
    char full_path[64];
    for (size_t i = 0; i < FMC_NUM_FILES; ++i) {
        const char* file = fmc_files[i];
        snprintf(full_path, sizeof(full_path), "%s%s", FMC_BASE_PATH, file);
        FILE *fp = fopen(full_path, "w");
        if (fp == NULL) {
            snprintf(error_message, message_size, "XILINX_FMC: Could not disable\n");
            LOG(logERROR, (error_message));
            return 1;
        }
        if (fprintf(fp, "0\n") != 1) {
            snprintf(error_message, message_size, "XILINX_FMC: Could not write disable.\n");
            LOG(logERROR, (error_message));
            return 1;
        }
        fclose(fp);
    }
    return 0;
}