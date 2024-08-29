// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <sys/types.h>

int resetFPGA(char *mess);
int loadDeviceTree(char *mess);

int checksBeforeCreatingDeviceTree(char *mess);
int createDeviceTree(char *mess);
int verifyDeviceTree(char *mess);
#ifndef VIRTUAL
int createSymbolicLinksForDevices(int adcDeviceIndex, int dacDeviceIndex,
                                  char *mess);
#endif