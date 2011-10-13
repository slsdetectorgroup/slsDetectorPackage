#ifndef COMMUNICATION_FUNCS_H
#define COMMUNICATION_FUNCS_H

#define SEND_REC_MAX_SIZE 4096
#define DEFAULT_PORTNO    1955
#include <sys/types.h>
#include <sys/socket.h>


#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include "sls_detector_defs.h"

int bindSocket(unsigned short int port_number);
int acceptConnection();
void closeConnection();
void exitServer();
int sendDataOnly(void* buf,int length);
int receiveDataOnly(void* buf,int length);

int getServerError();
int sendChannel(sls_detector_channel *myChan); 
int sendChip(sls_detector_chip *myChip);
int sendModule(sls_detector_module *myMod);
int receiveChannel(sls_detector_channel *myChan); 
int receiveChip(sls_detector_chip* myChip); 
int  receiveModule(sls_detector_module* myMod);

#endif
