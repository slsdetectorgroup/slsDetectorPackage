#include "qClient.h"

#include "MySocketTCP.h"

#include <iostream>
#include <sstream>


int main(int argc, char *argv[]){
	qClient* cl = 0;
	try {
		cl = new qClient(argv[1]);
	} catch(...) {
		return 0;
	}
	cl->executeLine(argc-2, argv+2);
	delete cl;

	return 0;
}


qClient::qClient(char* hostname):
	mySocket(0),
	myStopSocket(0){

	try {
		// control socket
		mySocket = new MySocketTCP(hostname, DEFAULT_GUI_PORTNO);
		// stop socket
		myStopSocket = new MySocketTCP(hostname, DEFAULT_GUI_PORTNO+1);
	} catch(...) {
		if (mySocket == 0)
			std::cout << "Error: could not connect to control server:" <<
			hostname << " with port " << DEFAULT_GUI_PORTNO << '\n';
		else
			std::cout << "Error: could not connect to stop server:" <<
			hostname << " with port " << DEFAULT_GUI_PORTNO + 1 << '\n';
		throw;
	}
}


qClient::~qClient() {
	if(mySocket) delete mySocket;
	if(myStopSocket) delete myStopSocket;
}


int qClient::executeLine(int narg, char *args[]){

	std::string retval = "";
	std::string cmd = args[0];
	std::string argument;


	//validate command structure
	if(narg<1){
		std::cout << "Error: no command parsed\n";
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
				startAcquisition();
			else if (argument == "stop")
				stopAcquisition();
			else{
				cprintf(RED,"Error: could not parse arguments: %s\n", argument.c_str());
				printCommands();
				return FAIL;
			}
		}
		retval = getStatus();
	}


	else if (cmd == "acquire"){
		startAcquisition(true);
		retval = getStatus();
	}


	else if (cmd == "exit"){
		return exitServer();
	}


	//unrecognized command
	else{
		std::cout << "Error: unrecognized command\n";
		return FAIL;
	}


	//print result
	std::cout << cmd << ": " << retval << '\n';

	return OK;
}


std::string qClient::printCommands(){
	std::ostringstream os;
	os << "\nexit \t exits servers in gui" << std::endl;
	os << "status \t gets status of acquisition in gui. - can be running or idle" << std::endl;
	os << "status i  starts/stops acquistion in gui-non blocking. i is start or stop" << std::endl;
	os << "acquire  starts acquistion in gui-blocking" << std::endl;
	return os.str();
}


std::string qClient::getStatus(){
	int fnum = F_GET_RUN_STATUS;
	int ret = FAIL;
	runStatus retval=ERROR;
	int progress = 0;
	char answer[100];

	if  (myStopSocket->Connect() >= 0) {
		myStopSocket->SendDataOnly(&fnum,sizeof(fnum));
		myStopSocket->ReceiveDataOnly(&ret,sizeof(ret));
		myStopSocket->ReceiveDataOnly(&retval,sizeof(retval));
		myStopSocket->ReceiveDataOnly(&progress,sizeof(progress));
		myStopSocket->Disconnect();
	}else
		exit(-1);


	sprintf(answer,"%d%% ",progress);
	strcat(answer,slsDetectorDefs::runStatusType((runStatus)retval).c_str());

	return std::string(answer);
}


int qClient::startAcquisition(bool blocking){
	int fnum = F_START_ACQUISITION;
	if(blocking) fnum = F_START_AND_READ_ALL;
	int ret = FAIL;

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


int qClient::stopAcquisition(){
	int fnum = F_STOP_ACQUISITION;
	int ret = FAIL;

	if  (myStopSocket->Connect() >= 0) {
		myStopSocket->SendDataOnly(&fnum,sizeof(fnum));
		myStopSocket->ReceiveDataOnly(&ret,sizeof(ret));
		if (ret == FAIL){
			myStopSocket->ReceiveDataOnly(mess,sizeof(mess));
			std::cout<< "Gui returned error: " << mess << '\n';
		}
		myStopSocket->Disconnect();
	}else
		exit(-1);

	return ret;
}


int qClient::exitServer(){
	int fnum = F_EXIT_SERVER;
	int ret = FAIL;

	if  (myStopSocket->Connect() >= 0) {
		myStopSocket->SendDataOnly(&fnum,sizeof(fnum));
		myStopSocket->ReceiveDataOnly(&ret,sizeof(ret));
		myStopSocket->ReceiveDataOnly(mess,sizeof(mess));
		std::cout << mess << '\n';
		myStopSocket->Disconnect();
	}else
		exit(-1);

	return ret;
}
