// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "LocalLinkInterface.h"
#include "slsDetectorServer_defs.h"
#include <stdlib.h>

int Beb_Beb();
void Beb_ClearHeaderData(int ten_gig);
int Beb_SetUpUDPHeader(unsigned int header_number, int ten_gig,
                       uint64_t src_mac, uint32_t src_ip, uint16_t src_port,
                       uint64_t dst_mac, uint32_t dst_ip, uint16_t dst_port);
int Beb_SetHeaderData(uint64_t src_mac, uint32_t src_ip, uint16_t src_port,
                      uint64_t dst_mac, uint32_t dst_ip, uint16_t dst_port);
void Beb_AdjustIPChecksum(struct udp_header_type *ip);

int Beb_GetModuleConfiguration(int *master, int *top, int *normal);
int Beb_IsTransmitting(int *retval, int tengiga, int waitForDelay);

void Beb_SetTopVariable(int val);
int Beb_SetTop(enum TOPINDEX ind);
int Beb_SetMaster(enum MASTERINDEX ind);
int Beb_SetActivate(int enable);
int Beb_GetActivate(int *retval);
int Beb_SetDataStream(enum portPosition port, int enable);
int Beb_GetDataStream(enum portPosition port, int *retval);
int Beb_Set32bitOverflow(int val);
int Beb_GetTenGigaFlowControl();
int Beb_SetTenGigaFlowControl(int value);
int Beb_GetTransmissionDelayFrame();
int Beb_SetTransmissionDelayFrame(int value);
int Beb_GetTransmissionDelayLeft();
int Beb_SetTransmissionDelayLeft(int value);
int Beb_GetTransmissionDelayRight();
int Beb_SetTransmissionDelayRight(int value);

u_int32_t Beb_GetFirmwareRevision();
u_int32_t Beb_GetFirmwareSoftwareAPIVersion();

void Beb_ResetFrameNumber();
int Beb_SetUpTransferParameters(short the_bit_mode);
int Beb_StopAcquisition();
int Beb_RequestNImages(int ten_gig, unsigned int nimages,
                       int test_just_send_out_packets_no_wait);
int Beb_GetBebFPGATemp();
int Beb_SetModuleId(uint32_t detid);
int Beb_SetQuad(int value);
int Beb_GetQuad();
int *Beb_GetDetectorPosition();
int Beb_SetDetectorPosition(int pos[]);
int Beb_SetNextFrameNumber(uint64_t value);
int Beb_GetNextFrameNumber(uint64_t *retval, int tengigaEnable);
void Beb_SetReadNRows(int value);
int Beb_GetNumberofDestinations(int *retval);
int Beb_SetNumberofDestinations(int value);

uint16_t Beb_swap_uint16(uint16_t val);
int Beb_open(u_int32_t **csp0base, u_int32_t offset);
u_int32_t Beb_Read32(u_int32_t *baseaddr, u_int32_t offset);
u_int32_t Beb_Write32(u_int32_t *baseaddr, u_int32_t offset, u_int32_t data);
void Beb_close(int fd, u_int32_t *csp0base);
