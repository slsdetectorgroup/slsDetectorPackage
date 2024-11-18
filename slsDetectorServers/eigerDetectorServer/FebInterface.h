// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

int Feb_Interface_WriteTo(unsigned int ch);
int Feb_Interface_ReadFrom(unsigned int ch, unsigned int ntrys);
int Feb_Interface_FebInterface();
int Feb_Interface_SetAddress(unsigned int leftAddr, unsigned int rightAddr);
int Feb_Interface_SetByteOrder();
int Feb_Interface_ReadRegister(unsigned int sub_num, unsigned int reg_num,
                               unsigned int *value_read);
int Feb_Interface_ReadRegisters(unsigned int sub_num, unsigned int nreads,
                                unsigned int *reg_nums,
                                unsigned int *values_read);
int Feb_Interface_WriteRegister(unsigned int sub_num, unsigned int reg_num,
                                unsigned int value, int wait_on,
                                unsigned int wait_on_address);
int Feb_Interface_WriteRegisters(unsigned int sub_num, unsigned int nwrites,
                                 unsigned int *reg_nums, unsigned int *values,
                                 int *wait_ons,
                                 unsigned int *wait_on_addresses);
int Feb_Interface_WriteMemoryInLoops(unsigned int sub_num, unsigned int mem_num,
                                     unsigned int start_address,
                                     unsigned int nwrites,
                                     unsigned int *values);
int Feb_Interface_WriteMemory(unsigned int sub_num, unsigned int mem_num,
                              unsigned int start_address, unsigned int nwrites,
                              unsigned int *values);
