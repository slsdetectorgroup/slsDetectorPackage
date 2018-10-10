#include "ClientInterface.h"


#include  <sys/types.h>
#include  <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bitset>
#include <cstdlib>
#include <iostream>



ClientInterface::ClientInterface(MySocketTCP *socket, int n, std::string t):
mySocket(socket),
index(n),
type(t){}

ClientInterface::~ClientInterface(){}


void ClientInterface::SetSocket(MySocketTCP *socket) {
	mySocket=socket;
}


int ClientInterface::PrintSocketReadError() {
	FILE_LOG(logERROR) << "Reading from socket failed. Possible socket crash";
	return FAIL;
}


void ClientInterface::Server_SendResult(int ret, void* retval, int retvalSize) {
	mySocket->SendDataOnly(&ret,sizeof(ret));
	mySocket->SendDataOnly(retval, retvalSize);
}


int ClientInterface::Client_GetMesage(char* mess) {
	int ret = OK;
	if (!mess){
		char messref[MAX_STR_LENGTH];
		memset(messref, 0, MAX_STR_LENGTH);
		mess = (char*)messref;
	}
	mySocket->ReceiveDataOnly(mess,MAX_STR_LENGTH);
	cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	if(strstr(mess,"Unrecognized Function")!=NULL)
		ret = FAIL;

	return ret;
}


int ClientInterface::Client_Send(int fnum,
		void* args, int sizeOfArgs,
		void* retval, int sizeOfRetval,
		char* mess) {

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(args, sizeOfArgs);

	int ret = FAIL;
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret == FAIL) {
		if (Client_GetMesage(mess) == FAIL)
		return FAIL;
	}
	mySocket->ReceiveDataOnly(retval, sizeOfRetval);

	return ret;
}


int ClientInterface::Client_Send(int fnum,
		void* args, int sizeOfArgs,
		void* args2, int sizeOfArgs2,
		void* retval, int sizeOfRetval,
		char* mess) {

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(args, sizeOfArgs);
	mySocket->SendDataOnly(args2, sizeOfArgs2);

	int ret = FAIL;
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret == FAIL) {
		if (Client_GetMesage(mess) == FAIL)
		return FAIL;
	}
	mySocket->ReceiveDataOnly(retval, sizeOfRetval);

	return ret;
}





