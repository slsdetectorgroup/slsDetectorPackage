#ifdef SLS_RECEIVER_FUNCTION_LIST
/********************************************//**
 * @file slsReceiverFunctionList.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "slsReceiverFunctionList.h"



#include <signal.h>  		// SIGINT
#include <sys/stat.h> 		// stat
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), shut down
#include <arpa/inet.h>		// sock_addr_in, htonl, INADDR_ANY
#include <stdlib.h>			// exit()


#include <string.h>
#include <iostream>
using namespace std;

FILE* slsReceiverFunctionList::sfilefd(NULL);
int slsReceiverFunctionList::listening_thread_running(0);

slsReceiverFunctionList::slsReceiverFunctionList():
				fileIndex(0),
				frameIndexNeeded(true),
				framesCaught(0),
				startFrameIndex(-1),
				frameIndex(0),
				totalFramesCaught(0),
				startAcquisitionIndex(-1),
				acquisitionIndex(0),
				framesInFile(0),
				status(IDLE),
				udpSocket(NULL),
				server_port(DEFAULT_UDP_PORT)
{
	strcpy(savefilename,"");
	strcpy(filePath,"");
	strcpy(fileName,"run");
	strcpy(buffer,"");
}



int slsReceiverFunctionList::getFrameIndex(){
	if(startFrameIndex==-1)
		frameIndex=0;
	else
		frameIndex=((int)(*((int*)buffer)) - startFrameIndex)/2;
	return frameIndex;
}



int slsReceiverFunctionList::getAcquisitionIndex(){
	if(startAcquisitionIndex==-1)
		acquisitionIndex=0;
	else
		acquisitionIndex=((int)(*((int*)buffer)) - startAcquisitionIndex)/2;
	return acquisitionIndex;
}



char* slsReceiverFunctionList::setFileName(char c[]){
	if(strlen(c))
		strcpy(fileName,c);

	return getFileName();
}



char* slsReceiverFunctionList::setFilePath(char c[]){
	if(strlen(c)){
		//check if filepath exists
		struct stat st;
		if(stat(c,&st) == 0)
			strcpy(filePath,c);
	}
	return getFilePath();
}



int slsReceiverFunctionList::setFileIndex(int i){
	if(i>=0)
		fileIndex = i;
	return getFileIndex();
}


bool slsReceiverFunctionList::resetTotalFramesCaught(bool i){
	frameIndexNeeded = i;
	startAcquisitionIndex = -1;
	totalFramesCaught = 0;

	return frameIndexNeeded;
}



void slsReceiverFunctionList::closeFile(int p){
	if(listening_thread_running){
		cout << "Closing file and Exiting." << endl;
		fclose(sfilefd);
	}
	exit(0);
}





int slsReceiverFunctionList::startReceiver(){
#ifdef VERBOSE
	cout << "Starting Receiver" << endl;
#endif
	int err = 0;
	if(!listening_thread_running){
		printf("Starting new acquisition threadddd ....\n");
		listening_thread_running=1;
		err = pthread_create(&listening_thread, NULL,startListeningThread, (void*) this);
		if(!err){
			while(status!=RUNNING);
			cout << endl << "Thread created successfully." << endl;
		}else{
			listening_thread_running=0;
			status = IDLE;
			cout << endl << "Cant create thread. Status:" << status << endl;
			return FAIL;
		}
	}

	return OK;
}





int slsReceiverFunctionList::stopReceiver(){
#ifdef VERBOSE
	cout << "Stopping Receiver" << endl;
#endif

	if(listening_thread_running){
		cout << "Stopping new acquisition threadddd ...." << endl;
		//stop thread
		listening_thread_running=0;
		udpSocket->ShutDownSocket();
		pthread_join(listening_thread,NULL);
		status = IDLE;
	}
	cout << "Status:" << status << endl;

	return OK;
}



void* slsReceiverFunctionList::startListeningThread(void* this_pointer){
	((slsReceiverFunctionList*)this_pointer)->startListening();

	return this_pointer;
}



int slsReceiverFunctionList::startListening(){
#ifdef VERYVERBOSE
	cout << "In startListening()\n");
#endif

	// Variable and structure definitions
	int rc, currframenum, prevframenum;

	//reset variables for each acquisition
	framesInFile=0;
	framesCaught=0;
	startFrameIndex=-1;
	frameIndex=0;


	//Catch signal SIGINT to close files properly
	signal(SIGINT,closeFile);


	//create file name
	if(!frameIndexNeeded)
		sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
	else
		sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,framesCaught,fileIndex);


	// A do/while(FALSE) loop is used to make error cleanup easier.  The
	//  close() of each of the socket descriptors is only done once at the
	//  very end of the program.
	do {

		udpSocket = new genericSocket(server_port,genericSocket::UDP);
		if (udpSocket->getErrorStatus()){
			break;
		}

		sfilefd = fopen((const char *) (savefilename), "w");
		cout << "Saving to " << savefilename << ". Ready! " << endl;

		while (listening_thread_running) {

			if (framesInFile == 20000) {
				fclose(sfilefd);

				currframenum=(int)(*((int*)buffer));
				//create file name
				if(!frameIndexNeeded)
					sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
				else
					sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,framesCaught,fileIndex);

				cout << "saving to " << savefilename << "\t\t"
						"packet loss " << ((currframenum-prevframenum-(2*framesInFile))/(double)(2*framesInFile))*100.000 << "%\t\t"
						"framenum " << currframenum << endl;
				sfilefd = fopen((const char *) (savefilename), "w");
				prevframenum=currframenum;
				framesInFile = 0;
			}
			status = RUNNING;


			//receiver 2 half frames
			rc = udpSocket->ReceiveDataOnly(buffer,sizeof(buffer));
			if( rc < 0)
				cerr << "recvfrom() failed" << endl;

			//for each scan
			if(startFrameIndex==-1){
				startFrameIndex=(int)(*((int*)buffer))-2;
				prevframenum=startFrameIndex;
			}
			//start of acquisition
			if(startAcquisitionIndex==-1)
				startAcquisitionIndex=startFrameIndex;



			//so that it doesnt write the last frame twice
			if(listening_thread_running){
				fwrite(buffer, 1, rc, sfilefd);
				framesInFile++;
				framesCaught++;
				totalFramesCaught++;
			}

		}
	} while (listening_thread_running);
	listening_thread_running=0;
	status = IDLE;

	//Close down any open socket descriptors
	udpSocket->Disconnect();

	//close file
	fclose(sfilefd);
#ifdef VERBOSE
	cout << "sfield:" << (int)sfilefd << endl;
#endif

	return 0;
}






char* slsReceiverFunctionList::readFrame(char* c){
	strcpy(c,savefilename);
	return buffer;
}


#endif

