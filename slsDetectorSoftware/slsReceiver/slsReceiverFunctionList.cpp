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
				udpSocket(-1),
				status(IDLE),
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
		if (udpSocket != -1)
			shutdown(udpSocket, SHUT_RDWR);
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
	/*udpSocket = -1;*/
	int rc1, rc2, rc;
	int currframenum, prevframenum;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t clientaddrlen = sizeof(clientaddr);

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
		// The socket() function returns a socket descriptor, which represents
		// an endpoint.  The statement also identifies that the INET
		// (Internet Protocol) address family with the UDP transport
		// (SOCK_DGRAM) will be used for this socket.
		udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if (udpSocket < 0) {
			cerr << "socket() failed" << endl;
			break;
		}


		// After the socket descriptor is created, a bind() function gets a
		// unique name for the socket.  In this example, the user sets the
		// s_addr to zero, which means that the UDP port of 3555 will be
		// bound to all IP addresses on the system.

		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(server_port);
		//serveraddr.sin_addr.s_addr = inet_addr(server_ip);
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

		rc = bind(udpSocket, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
		if (rc < 0) {
			cerr << "bind() failed" << endl;
			break;
		}


		sfilefd = fopen((const char *) (savefilename), "w");
		cout << "Saving to " << savefilename << ". Ready! " << endl;

		while (listening_thread_running) {

			// The server uses the recvfrom() function to receive that data.
			// The recvfrom() function waits indefinitely for data to arrive.
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

			rc1 = recvfrom(udpSocket, buffer, HALF_BUFFER_SIZE, 0,
					(struct sockaddr *) &clientaddr, &clientaddrlen);
			//cout << "rc1 done" << endl;
			rc2 = recvfrom(udpSocket, buffer+HALF_BUFFER_SIZE, HALF_BUFFER_SIZE, 0,
					(struct sockaddr *) &clientaddr, &clientaddrlen);


			//for each scan
			if(startFrameIndex==-1){
				startFrameIndex=(int)(*((int*)buffer))-2;
				prevframenum=startFrameIndex;
			}
			//start of acquisition
			if(startAcquisitionIndex==-1)
				startAcquisitionIndex=startFrameIndex;

			//cout << "rc2 done" << endl;
			if ((rc1 < 0) || (rc2 < 0)) {
				perror("recvfrom() failed");
				break;
			}

			//so that it doesnt write the last frame twice
			if(listening_thread_running){
				fwrite(buffer, 1, rc1, sfilefd);
				fwrite(buffer+HALF_BUFFER_SIZE, 1, rc2, sfilefd);
				framesInFile++;
				framesCaught++;
				totalFramesCaught++;
				//cout << "saving" << endl;
			}
		}
	} while (listening_thread_running);
	listening_thread_running=0;
	status = IDLE;

	//Close down any open socket descriptors
	if (udpSocket != -1){
		cout << "Closing udpSocket" << endl;
		close(udpSocket);
	}

	//close file
	fclose(sfilefd);
	cout << "sfield:" << (int)sfilefd << endl;

	return 0;
}








char* slsReceiverFunctionList::readFrame(char* c){
	strcpy(c,savefilename);
	return buffer;
}


#endif

