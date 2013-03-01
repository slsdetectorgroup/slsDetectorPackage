/*
 * qClient.cpp
 *
 *  Created on: Feb 27, 2013
 *      Author: Dhanya Maliakal
 */
// Qt Project Class Headers
#include "qClient.h"
// Project Class Headers
#include "MySocketTCP.h"
#include "slsDetectorBase.h"
// C++ Include Headers
#include <iostream>
using namespace std;




int main(int argc, char *argv[])

{
	qClient *cl =new qClient(argv[1]);
	cl->executeLine(argc-2, argv+2);

  delete cl;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qClient::qClient(char* hostname){
	//create socket
	mySocket = new MySocketTCP(hostname, DEFAULT_GUI_PORTNO);
	if (mySocket->getErrorStatus()){
		cout << "Error: could not connect to host:" << hostname << " with port " << DEFAULT_GUI_PORTNO << endl;
		delete mySocket;
		exit(-1);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

int qClient::executeLine(int narg, char *args[]){

	char arg[MAX_STR_LENGTH] = "";
	int iarg = -1;
	char answer[100];
	string retval = "";
	string cmd = args[0];


	//validate command structure
	if(narg<1){
		cout << "Error: no command parsed" << endl;
		return slsDetectorDefs::FAIL;
	}


	//file name
	if (cmd == "status"){
		retval = getStatus();
	}


	//unrecognized command
	else{
		cout << "Error: unrecognized command" << endl;
		return slsDetectorDefs::FAIL;
	}


	//print result
	cout << cmd << ": " << retval << endl;

	return slsDetectorDefs::OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


string qClient::getStatus(){
	int fnum = slsDetectorDefs::F_GET_RUN_STATUS;
	int ret = slsDetectorDefs::FAIL;
	int retval = -1;
	slsDetectorDefs::runStatus s=slsDetectorDefs::ERROR;

	if  (mySocket->Connect() >= 0) {
		mySocket->SendDataOnly(&fnum,sizeof(fnum));
		mySocket->ReceiveDataOnly(&ret,sizeof(ret));
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));
	}
	mySocket->Disconnect();

	if(retval==-1)
		retval=slsDetectorDefs::ERROR;

	return slsDetectorBase::runStatusType((slsDetectorDefs::runStatus)retval);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------
