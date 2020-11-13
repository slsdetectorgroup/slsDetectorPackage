#pragma once

#include "LocalLinkInterface.h"
#include "slsDetectorServer_defs.h"
#include <stdlib.h>

struct BebInfo {
    unsigned int beb_number;
    unsigned int serial_address;
    char src_mac_1GbE[50];
    char src_mac_10GbE[50];
    char src_ip_1GbE[50];
    char src_ip_10GbE[50];
    unsigned int src_port_1GbE;
    unsigned int src_port_10GbE;
};

void BebInfo_BebInfo(struct BebInfo *bebInfo, unsigned int beb_num);
void BebInfo_BebDstInfo(struct BebInfo *bebInfo, unsigned int beb_num);
int BebInfo_SetSerialAddress(struct BebInfo *bebInfo, unsigned int add);
int BebInfo_SetHeaderInfo(
    struct BebInfo *bebInfo, int ten_gig, char *src_mac, char *src_ip,
    unsigned int
        src_port); // src_port fixed 42000+beb_number or 52000 + beb_number);
unsigned int BebInfo_GetBebNumber(struct BebInfo *bebInfo);
unsigned int BebInfo_GetSerialAddress(struct BebInfo *bebInfo);
char *BebInfo_GetSrcMAC(struct BebInfo *bebInfo, int ten_gig);
char *BebInfo_GetSrcIP(struct BebInfo *bebInfo, int ten_gig);
unsigned int BebInfo_GetSrcPort(struct BebInfo *bebInfo, int ten_gig);
void BebInfo_Print(struct BebInfo *bebInfo);
void Beb_ClearBebInfos();
int Beb_InitBebInfos();
int Beb_CheckSourceStuffBebInfo();
unsigned int Beb_GetBebInfoIndex(unsigned int beb_numb);

void Beb_GetModuleConfiguration(int *master, int *top, int *normal);
int Beb_IsTransmitting(int *retval, int tengiga, int waitForDelay);

void Beb_SetTopVariable(int val);
int Beb_SetTop(enum TOPINDEX ind);
int Beb_SetMaster(enum MASTERINDEX ind);
int Beb_SetActivate(int enable);
int Beb_GetActivate(int *retval);
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
int Beb_WriteTo(unsigned int index);
int Beb_SetMAC(char *mac, uint8_t *dst_ptr);
int Beb_SetIP(char *ip, uint8_t *dst_ptr);
int Beb_SetPortNumber(unsigned int port_number, uint8_t *dst_ptr);
void Beb_AdjustIPChecksum(struct udp_header_type *ip);

int Beb_SetHeaderData(unsigned int beb_number, int ten_gig, char *dst_mac,
                      char *dst_ip, unsigned int dst_port);
int Beb_SetHeaderData1(char *src_mac, char *src_ip, unsigned int src_port,
                       char *dst_mac, char *dst_ip, unsigned int dst_port);

void Beb_SwapDataFun(int little_endian, unsigned int n, unsigned int *d);
int Beb_SetByteOrder();
void Beb_Beb();
int Beb_SetBebSrcHeaderInfos(unsigned int beb_number, int ten_gig,
                             char *src_mac, char *src_ip,
                             unsigned int src_port);
int Beb_SetUpUDPHeader(unsigned int beb_number, int ten_gig,
                       unsigned int header_number, char *dst_mac, char *dst_ip,
                       unsigned int dst_port);

/*int Beb_SendMultiReadRequest(unsigned int beb_number, unsigned int
 * left_right, int ten_gig, unsigned int dst_number, unsigned int npackets,
 * unsigned int packet_size, int stop_read_when_fifo_empty=1);*/
int Beb_SendMultiReadRequest(unsigned int beb_number, unsigned int left_right,
                             int ten_gig, unsigned int dst_number,
                             unsigned int npackets, unsigned int packet_size,
                             int stop_read_when_fifo_empty);

int Beb_StopAcquisition();
int Beb_SetUpTransferParameters(short the_bit_mode);
/*int Beb_RequestNImages(unsigned int beb_number, unsigned int left_right, int
 * ten_gig, unsigned int dst_number, unsigned int nimages, int
 * test_just_send_out_packets_no_wait=0); //all images go to the same
 * destination!*/
int Beb_RequestNImages(unsigned int beb_number, int ten_gig,
                       unsigned int dst_number, unsigned int nimages,
                       int test_just_send_out_packets_no_wait);

int Beb_Test(unsigned int beb_number);

int Beb_GetBebFPGATemp();

void Beb_SetDetectorNumber(uint32_t detid);
int Beb_SetQuad(int value);
int Beb_GetQuad();
int *Beb_GetDetectorPosition();
int Beb_SetDetectorPosition(int pos[]);
int Beb_SetNextFrameNumber(uint64_t value);
int Beb_GetNextFrameNumber(uint64_t *retval, int tengigaEnable);

void Beb_SetReadNLines(int value);

uint16_t Beb_swap_uint16(uint16_t val);
int Beb_open(u_int32_t **csp0base, u_int32_t offset);
u_int32_t Beb_Read32(u_int32_t *baseaddr, u_int32_t offset);
u_int32_t Beb_Write32(u_int32_t *baseaddr, u_int32_t offset, u_int32_t data);
void Beb_close(int fd, u_int32_t *csp0base);
