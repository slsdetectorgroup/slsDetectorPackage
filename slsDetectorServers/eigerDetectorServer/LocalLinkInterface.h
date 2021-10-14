// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "HardwareIO.h"

struct LocalLinkInterface {
    xfs_u32 ll_fifo_base;
    unsigned int ll_fifo_ctrl_reg;
};

int Local_Init(struct LocalLinkInterface *ll, unsigned int ll_fifo_badr);
int Local_Reset1(struct LocalLinkInterface *ll, unsigned int rst_mask);
int Local_ctrl_reg_write_mask(struct LocalLinkInterface *ll, unsigned int mask,
                              unsigned int val);
void Local_LocalLinkInterface(struct LocalLinkInterface *ll,
                              unsigned int ll_fifo_badr);
unsigned int Local_StatusVector(struct LocalLinkInterface *ll);
int Local_Reset(struct LocalLinkInterface *ll);
int Local_Write(struct LocalLinkInterface *ll, unsigned int buffer_len,
                void *buffer);
int Local_Read(struct LocalLinkInterface *ll, unsigned int buffer_len,
               void *buffer);
int Local_Test(struct LocalLinkInterface *ll, unsigned int buffer_len,
               void *buffer);
