#ifndef COMMUNICATION_FUNCS_H
#define COMMUNICATION_FUNCS_H


#include "sls_detector_defs.h"

typedef enum{
	INT16,
	INT32,
	INT64,
	OTHER
}intType;




int bindSocket(unsigned short int port_number);
int acceptConnection(int socketDescriptor);
void closeConnection(int file_Des);
void exitServer(int socketDescriptor);

void swapData(void* val,int length,intType itype);
int sendData(int file_des, void* buf,int length, intType itype);
int receiveData(int file_des, void* buf,int length, intType itype);
int sendDataOnly(int file_des, void* buf,int length);
int receiveDataOnly(int file_des, void* buf,int length);

int sendModule(int file_des, sls_detector_module *myMod);
int receiveModule(int file_des, sls_detector_module* myMod);

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
 * Server sends result to client (also set ret to force_update if different clients)
 * @param fileDes file descriptor for the socket
 * @param itype 32 or 64 or others to determine to swap data from big endian to little endian
 * @param update 1 if one must update if different clients, else 0
 * @param retval pointer to result
 * @param retvalSize size of result
 * @returns result of operation
 */
int Server_SendResult(int fileDes, intType itype, int update, void* retval, int retvalSize);

#endif
