// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

// Class initially from Gerd and was called mmap_test.c
#pragma once

#include "xfs_types.h"

xfs_u8 HWIO_xfs_in8(xfs_u32 InAddress);
xfs_u16 HWIO_xfs_in16(xfs_u32 InAddress);
xfs_u32 HWIO_xfs_in32(xfs_u32 InAddress);

void HWIO_xfs_out8(xfs_u32 OutAddress, xfs_u8 Value);
void HWIO_xfs_out16(xfs_u32 OutAddress, xfs_u16 Value);
void HWIO_xfs_out32(xfs_u32 OutAddress, xfs_u32 Value);
