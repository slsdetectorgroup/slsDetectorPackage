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

char lastClientIP[INET_ADDRSTRLEN];
char thisClientIP[INET_ADDRSTRLEN];
int lockStatus;

int bindSocket(unsigned short int port_number);
int acceptConnection(int socketDescriptor);
void closeConnection(int file_Des);
void exitServer(int socketDescriptor);
int sendDataOnly(int file_des, void* buf,int length);
int receiveDataOnly(int file_des, void* buf,int length);

int getServerError(int socketDescriptor);
int sendChannel(int file_des, sls_detector_channel *myChan); 
int sendChip(int file_des, sls_detector_chip *myChip);
int sendModule(int file_des, sls_detector_module *myMod);
int receiveChannel(int file_des, sls_detector_channel *myChan); 
int receiveChip(int file_des, sls_detector_chip* myChip); 
int  receiveModule(int file_des, sls_detector_module* myMod);

#endif
