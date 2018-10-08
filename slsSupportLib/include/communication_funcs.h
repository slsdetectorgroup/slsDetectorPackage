#ifndef COMMUNICATION_FUNCS_H
#define COMMUNICATION_FUNCS_H

#define SEND_REC_MAX_SIZE 4096
#define DEFAULT_PORTNO    1952
#include <sys/types.h>
#include <sys/socket.h>


#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

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


int getServerError(int socketDescriptor);
int sendChannel(int file_des, sls_detector_channel *myChan); 
int sendChip(int file_des, sls_detector_chip *myChip);
int sendModule(int file_des, sls_detector_module *myMod);
int sendModuleGeneral(int file_des, sls_detector_module *myMod, int sendAll);
int receiveChannel(int file_des, sls_detector_channel *myChan); 
int receiveChip(int file_des, sls_detector_chip* myChip); 
int receiveModule(int file_des, sls_detector_module* myMod);
int receiveModuleGeneral(int file_des, sls_detector_module* myMod, int receiveAll);

#endif
