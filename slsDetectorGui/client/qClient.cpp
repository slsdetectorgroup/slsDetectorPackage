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
#include <sstream>
using namespace std;



//-------------------------------------------------------------------------------------------------------------------------------------------------


int main(int argc, char *argv[]){

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

qClient::~qClient() {
	if(mySocket) delete mySocket;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qClient::executeLine(int narg, char *args[]){

	char arg[MAX_STR_LENGTH] = "";
	int iarg = -1;
	char answer[100];
	string retval = "";
	string cmd = args[0];
	string argument;


	//validate command structure
	if(narg<1){
		cout << "Error: no command parsed" << endl;
		return FAIL;
	}


	//help
	if (cmd == "help"){
		retval = printCommands();
	}

	//file name
	else if (cmd == "status"){

		if(narg>1){
			argument = args[1];
			//start acquisition
			if(argument == "start")
				sendToGuiServer(F_START_ACQUISITION);
			else if (argument == "stop")
				sendToGuiServer(F_STOP_ACQUISITION);
			else{
				cout << "Error: could not parse arguments: " << argument << endl;
				printCommands();
				return FAIL;
			}
		}
		retval = getStatus();
	}


	else if (cmd == "acquire"){
		sendToGuiServer(F_START_AND_READ_ALL);
		retval = getStatus();
	}


	else if (cmd == "exit"){
		if (sendToGuiServer(F_EXIT_SERVER) == OK)
			retval = "Gui Server Exited successfully.";
		else
			retval = "Gui Server could not exit successfully";
	}


	//unrecognized command
	else{
		cout << "Error: unrecognized command" << endl;
		return FAIL;
	}


	//print result
	cout << cmd << ": " << retval << endl;

	return OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


string qClient::printCommands(){
	ostringstream os;
	os << "\nexit \t exits server in gui" << std::endl;
	os << "status \t gets status of acquisition in gui. - can be running or idle" << std::endl;
	os << "status i  starts/stops acquistion in gui-non blocking. i is start or stop" << std::endl;
	os << "acquire  starts acquistion in gui-blocking" << std::endl;
	return os.str();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


string qClient::getStatus(){
	int fnum = F_GET_RUN_STATUS;
	int ret = FAIL;
	runStatus retval=ERROR;
	int progress = 0;
	char answer[100];

	if  (mySocket->Connect() >= 0) {
		mySocket->SendDataOnly(&fnum,sizeof(fnum));
		mySocket->ReceiveDataOnly(&ret,sizeof(ret));
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));
		mySocket->ReceiveDataOnly(&progress,sizeof(progress));
		mySocket->Disconnect();
	}else
		exit(-1);


	sprintf(answer,"%d%% ",progress);
	strcat(answer,slsDetectorBase::runStatusType((runStatus)retval).c_str());

	return string(answer);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qClient::sendToGuiServer(int fnum){
	int ret = FAIL;
	char mess[100] = "";

	if  (mySocket->Connect() >= 0) {
		mySocket->SendDataOnly(&fnum,sizeof(fnum));
		mySocket->ReceiveDataOnly(&ret,sizeof(ret));
		if (ret == FAIL){
			mySocket->ReceiveDataOnly(mess,sizeof(mess));
			std::cout<< "Gui returned error: " << mess << std::endl;
		}
		mySocket->Disconnect();
	}else
		exit(-1);

	return ret;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

