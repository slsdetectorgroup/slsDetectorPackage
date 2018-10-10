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



int ClientInterface::SendString(int fnum, char retval[], char arg[]){
	int ret = slsDetectorDefs::FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(arg,MAX_STR_LENGTH);
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,sizeof(mess));
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	if(strstr(mess,"Unrecognized Function")==NULL)
		mySocket->ReceiveDataOnly(retval,MAX_STR_LENGTH);

	return ret;
}



int ClientInterface::SendUDPDetails(int fnum, char retval[], char arg[3][MAX_STR_LENGTH]){
	char args[3][MAX_STR_LENGTH];
	int ret = slsDetectorDefs::FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(arg,sizeof(args));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,sizeof(mess));
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	else
		mySocket->ReceiveDataOnly(retval,MAX_STR_LENGTH);

	return ret;
}


int ClientInterface::SendInt(int fnum, int &retval, int arg){
	int ret = slsDetectorDefs::FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(&arg,sizeof(arg));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,sizeof(mess));
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	if(strstr(mess,"Unrecognized Function")==NULL)
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));

	return ret;
}



int ClientInterface::GetInt(int fnum, int &retval){
	int ret = slsDetectorDefs::FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,sizeof(mess));
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	if(strstr(mess,"Unrecognized Function")==NULL)
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));

	return ret;
}



int ClientInterface::SendInt(int fnum, int64_t &retval, int64_t arg){
	int ret = slsDetectorDefs::FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(&arg,sizeof(arg));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,sizeof(mess));
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	if(strstr(mess,"Unrecognized Function")==NULL)
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));

	return ret;
}



int ClientInterface::SendIntArray(int fnum, int64_t &retval, int64_t arg[2], char mess[]){
	int64_t args[2];
	int ret = slsDetectorDefs::FAIL;
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(arg,sizeof(args));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,MAX_STR_LENGTH);
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	if(strstr(mess,"Unrecognized Function")==NULL)
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));

	return ret;
}



int ClientInterface::SendIntArray(int fnum, int &retval, int arg[2]){
	int args[2];
	int ret = slsDetectorDefs::FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(arg,sizeof(args));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,sizeof(mess));
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	if(strstr(mess,"Unrecognized Function")==NULL)
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));

	return ret;
}



int ClientInterface::GetInt(int fnum, int64_t &retval){
	int ret = slsDetectorDefs::FAIL;
	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	mySocket->ReceiveDataOnly(&retval,sizeof(retval));
	return ret;
}


int ClientInterface::GetLastClientIP(int fnum, char retval[]){
	int ret = slsDetectorDefs::FAIL;
	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	mySocket->ReceiveDataOnly(retval,INET_ADDRSTRLEN);
	return ret;
}



int ClientInterface::ExecuteFunction(int fnum,char mess[]){
	int ret = slsDetectorDefs::FAIL;
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,MAX_STR_LENGTH);
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}

	return ret;
}



int ClientInterface::SendROI(int fnum, int n, slsReceiverDefs::ROI roiLimits[]) {
	int ret = slsDetectorDefs::FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(&n,sizeof(n));
	mySocket->SendDataOnly(roiLimits,n * sizeof(slsReceiverDefs::ROI));
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret==slsDetectorDefs::FAIL){
		mySocket->ReceiveDataOnly(mess,sizeof(mess));
		cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	}
	return ret;
}


