// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef COMMUNICATION_FUNCS_H
#define COMMUNICATION_FUNCS_H

#include "sls/sls_detector_defs.h"

typedef enum { INT16, INT32, INT64, OTHER } intType;

// communciate with stop server
#ifdef VIRTUAL
#define FILE_STATUS "/tmp/Sls_virtual_server_status_"
#define FILE_STOP   "/tmp/Sls_virtual_server_stop_"
#define FD_STATUS   0
#define FD_STOP     1
#endif

int bindSocket(unsigned short int port_number);
int acceptConnection(int socketDescriptor);
void closeConnection(int file_Des);
void exitServer(int socketDescriptor);

void swapData(void *val, int length, intType itype);
int sendData(int file_des, void *buf, int length, intType itype);
int receiveData(int file_des, void *buf, int length, intType itype);
int sendDataOnly(int file_des, void *buf, int length);
int receiveDataOnly(int file_des, void *buf, int length);

int sendModule(int file_des, sls_detector_module *myMod);
int receiveModule(int file_des, sls_detector_module *myMod);

/**
 * Servers sets and prints error message for locked server
 * @returns success of operaton
 */
void Server_LockedError();

/**
 * Server verifies if it is unlocked,
 * sets and prints appropriate message if it is locked and different clients
 * @returns success of operaton
 */
int Server_VerifyLock();

/**
 * Server sends result to client (also set ret to force_update if different
 * clients)
 * @param fileDes file descriptor for the socket
 * @param itype 32 or 64 or others to determine to swap data from big endian to
 * little endian
 * @param retval pointer to result
 * @param retvalSize size of result
 * @returns result of operation
 */
int Server_SendResult(int fileDes, intType itype, void *retval, int retvalSize);

/**
 * Convert mac address from integer to char array
 * @param cmac char arrary result
 * @param size size of char array result
 * @param mac mac address as an integer
 */
void getMacAddressinString(char *cmac, int size, uint64_t mac);

/**
 * Convert ip address from integer to char array
 * @param cip char arrary result
 * @param ip ip address as an integer
 */
void getIpAddressinString(char *cip, uint32_t ip);

/**
 * Convert string to ip address
 * @param cip string source
 * @param ip result
 */
void getIpAddressFromString(char *cip, uint32_t *ip);

#endif
