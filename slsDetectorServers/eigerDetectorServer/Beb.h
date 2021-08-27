#pragma once

#include "LocalLinkInterface.h"
#include "slsDetectorServer_defs.h"
#include <stdlib.h>

void Beb_Beb();
void Beb_ClearHeaderData(int ten_gig);
int Beb_SetUpUDPHeader(unsigned int header_number, int ten_gig, char *src_mac, char *src_ip, unsigned int src_port, char *dst_mac, char *dst_ip, unsigned int dst_port);
int Beb_SetHeaderData(char *src_mac, char *src_ip, unsigned int src_port, char *dst_mac, char *dst_ip, unsigned int dst_port);
int Beb_SetMAC(char *mac, uint8_t *dst_ptr);
int Beb_SetIP(char *ip, uint8_t *dst_ptr);
int Beb_SetPortNumber(unsigned int port_number, uint8_t *dst_ptr);
void Beb_AdjustIPChecksum(struct udp_header_type *ip);

void Beb_GetModuleConfiguration(int *master, int *top, int *normal);
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
void Beb_SetDetectorNumber(uint32_t detid);
int Beb_SetQuad(int value);
int Beb_GetQuad();
int *Beb_GetDetectorPosition();
int Beb_SetDetectorPosition(int pos[]);
int Beb_SetNextFrameNumber(uint64_t value);
int Beb_GetNextFrameNumber(uint64_t *retval, int tengigaEnable);
void Beb_SetPartialReadout(int value);
int Beb_GetNumberofDestinations(int *retval);
int Beb_SetNumberofDestinations(int value);

uint16_t Beb_swap_uint16(uint16_t val);
int Beb_open(u_int32_t **csp0base, u_int32_t offset);
u_int32_t Beb_Read32(u_int32_t *baseaddr, u_int32_t offset);
u_int32_t Beb_Write32(u_int32_t *baseaddr, u_int32_t offset, u_int32_t data);
void Beb_close(int fd, u_int32_t *csp0base);
