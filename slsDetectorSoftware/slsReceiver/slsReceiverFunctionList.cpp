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

#include <iomanip>			//set precision


#include <string.h>
#include <iostream>
using namespace std;

FILE* slsReceiverFunctionList::sfilefd(NULL);
int slsReceiverFunctionList::listening_thread_running(0);

slsReceiverFunctionList::slsReceiverFunctionList(bool shortfname):
				enableFileWrite(1),
				shortFileName(shortfname),
				shortFileNameIndex(0),
				fileIndex(0),
				frameIndexNeeded(true),
				framesCaught(0),
				startFrameIndex(-1),
				frameIndex(0),
				totalFramesCaught(0),
				startAcquisitionIndex(-1),
				acquisitionIndex(0),
				framesInFile(0),
				prevframenum(0),
				status(IDLE),
				latestData(NULL),
				udpSocket(NULL),
				server_port(DEFAULT_UDP_PORTNO),
				fifo(NULL)
{
	strcpy(savefilename,"");
	strcpy(actualfilename,"");
	strcpy(filePath,"");
	strcpy(fileName,"run");
	eth = new char[MAX_STR_LENGTH];
	strcpy(eth,"");

}



int slsReceiverFunctionList::setEnableFileWrite(int i){
	if(i!=-1)
		enableFileWrite=i;
	return enableFileWrite;
}


void slsReceiverFunctionList::setEthernetInterface(char* c){
	strcpy(eth,c);
}



int slsReceiverFunctionList::getFrameIndex(){
	if(startFrameIndex==-1)
		frameIndex=0;
	else
		frameIndex=((int)(*((int*)latestData)) - startFrameIndex)/2;
	return frameIndex;
}



int slsReceiverFunctionList::getAcquisitionIndex(){
	if(startAcquisitionIndex==-1)
		acquisitionIndex=0;
	else
		acquisitionIndex=((int)(*((int*)latestData)) - startAcquisitionIndex)/2;
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



int slsReceiverFunctionList::startReceiver(){
#ifdef VERBOSE
	cout << "Starting Receiver" << endl;
#endif
	int err = 0;
	if(!listening_thread_running){
		cout << "Starting new acquisition threadddd ...." << endl;
		listening_thread_running=1;

		fifo = new CircularFifo<dataStruct,FIFO_SIZE>();

		//error creating writing thread
		err = pthread_create(&writing_thread, NULL,startWritingThread, (void*) this);
		if(err){
			listening_thread_running=0;
			status = IDLE;
			if(fifo) delete fifo;
			cout << "Cant create writing thread. Status:" << status << endl << endl;
			return FAIL;
		}
		cout << "Writing thread created successfully." << endl;


		//error creating listenign thread
		err = 0;
		err = pthread_create(&listening_thread, NULL,startListeningThread, (void*) this);
		if(err){
			listening_thread_running=0;
			status = IDLE;
			//stop writing thread
			pthread_join(writing_thread,NULL);
			if(fifo) delete fifo;
			cout << endl << "Cant create listening thread. Status:" << status << endl << endl;
			return FAIL;
		}

		while(status!=RUNNING);
		cout << "Listening thread created successfully." << endl;
	}

	return OK;
}





int slsReceiverFunctionList::stopReceiver(){
#ifdef VERBOSE
	cout << "Stopping Receiver" << endl;
#endif

	if(listening_thread_running){
		cout << "Stopping new acquisition threadddd ...." << endl;
		//stop listening thread
		listening_thread_running=0;
		udpSocket->ShutDownSocket();
		pthread_join(listening_thread,NULL);
		status = IDLE;

		//stop writing thread
		pthread_join(writing_thread,NULL);
		if(fifo)	delete fifo;
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
	int rc;
	dataStruct *dataReadFrame;

	//reset variables for each acquisition
	startFrameIndex=-1;
	shortFileNameIndex=1;


	// A do/while(FALSE) loop is used to make error cleanup easier.  The
	//  close() of each of the socket descriptors is only done once at the
	//  very end of the program.
	do {

		if(!strlen(eth)){
			cout<<"warning:eth is empty.listening to all"<<endl;
			udpSocket = new genericSocket(server_port,genericSocket::UDP);
		}else
			udpSocket = new genericSocket(server_port,genericSocket::UDP,eth);

		if (udpSocket->getErrorStatus()){
			break;
		}


		while (listening_thread_running) {
			status = RUNNING;

			buffer = new char[BUFFER_SIZE];
			//receiver 2 half frames
			rc = udpSocket->ReceiveDataOnly(buffer,sizeof(buffer));
			if( rc < 0)
				cerr << "recvfrom() failed" << endl;

			//start for each scan
			if(startFrameIndex==-1){
				startFrameIndex=(int)(*((int*)buffer))-2;
				//cout<<"startFrameIndex:"<<startFrameIndex<<endl;
				prevframenum=startFrameIndex;
			}

			//start of acquisition
			if(startAcquisitionIndex==-1){
				startAcquisitionIndex=startFrameIndex;
				//cout<<"startAcquisitionIndex:"<<startAcquisitionIndex<<endl;
			}



			//so that it doesnt write the last frame twice
			if(listening_thread_running){
				//s.assign(buffer);
				if(fifo->isFull())
					;//cout<<"**********************FIFO FULLLLLLLL************************"<<endl;
				else{
					//cout<<"read index:"<<(int)(*(int*)buffer)<<endl;
					dataReadFrame = new dataStruct;
					dataReadFrame->buffer=buffer;
					dataReadFrame->rc=rc;
					//cout<<"read buffer:"<<dataReadFrame<<endl;
					fifo->push(dataReadFrame);
				}
			}

		}
	} while (listening_thread_running);
	listening_thread_running=0;
	status = IDLE;

	//Close down any open socket descriptors
	udpSocket->Disconnect();


#ifdef VERBOSE
	cout << "listening_thread_running:" << listening_thread_running << endl;
#endif

	return 0;
}






void* slsReceiverFunctionList::startWritingThread(void* this_pointer){
	((slsReceiverFunctionList*)this_pointer)->startWriting();

	return this_pointer;
}




int slsReceiverFunctionList::startWriting(){
#ifdef VERYVERBOSE
	cout << "In startWriting()" <<endl;
#endif
	// Variable and structure definitions
	int currframenum,ret;
	dataStruct *dataWriteFrame;

	//reset variables for each acquisition
	framesInFile=0;
	framesCaught=0;
	frameIndex=0;

	latestData = new char[BUFFER_SIZE];
	if(sfilefd) sfilefd=0;

	strcpy(savefilename,"");
	strcpy(actualfilename,"");


	cout << "Ready!" << endl;

	if(enableFileWrite){
		//create file name
		if(!frameIndexNeeded)	sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
		else					sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,framesCaught,fileIndex);

		//for sebastian
		if(!shortFileName)	strcpy(actualfilename,savefilename);
		else				sprintf(actualfilename, "%s/%s_%d_%09d.raw", filePath,fileName,fileIndex, 0);

		//start writing
		sfilefd = fopen((const char *) (actualfilename), "w");
		cout << "Saving to " << actualfilename << endl;
	}


	while(listening_thread_running){

		//when it reaches MAX_FRAMES_PER_FILE,start writing new file
		if (framesInFile == MAX_FRAMES_PER_FILE) {

			if(enableFileWrite){
				fclose(sfilefd);
				//create file name
				if(!frameIndexNeeded)	sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
				else					sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,framesCaught,fileIndex);
				//for sebastian
				if(!shortFileName)		strcpy(actualfilename,savefilename);
				else					sprintf(actualfilename, "%s/%s_%d_%09d.raw", filePath,fileName,fileIndex, shortFileNameIndex);
				shortFileNameIndex++;
				//start writing in new file
				sfilefd = fopen((const char *) (actualfilename), "w");
				cout << "saving to " << actualfilename << "\t";
			}

			currframenum=(int)(*((int*)latestData));
			cout << "packet loss " << fixed << setprecision(4) << ((currframenum-prevframenum-(2*framesInFile))/(double)(2*framesInFile))*100.000 << "%\t\t"
					"framenum " << currframenum << "\t\t"
					"p " << prevframenum << endl;

			prevframenum=currframenum;
			framesInFile = 0;
		}

		//actual writing from fifo
		if(!fifo->isEmpty()){

			dataWriteFrame = new dataStruct;
			if(fifo->pop(dataWriteFrame)){
				//cout<<"write buffer:"<<dataWriteFrame<<endl<<endl;
				framesCaught++;
				totalFramesCaught++;
				memcpy(latestData,dataWriteFrame->buffer,BUFFER_SIZE);
				//cout<<"list write \t index:"<<(int)(*(int*)latestData)<<endl;
				if(enableFileWrite)
					fwrite(dataWriteFrame->buffer, 1, dataWriteFrame->rc, sfilefd);
				framesInFile++;
				delete dataWriteFrame->buffer;
				delete dataWriteFrame;

			}
		}
	}

	cout << "Total Frames Caught:"<< totalFramesCaught << endl;
	//close file
	if(sfilefd)		fclose(sfilefd);

#ifdef VERBOSE
	cout << "sfield:" << (int)sfilefd << endl;
#endif

	return 0;
}








char* slsReceiverFunctionList::readFrame(char* c){
	//cout<<"latestdata:"<<(int)(*(int*)latestData)<<endl;
	strcpy(c,savefilename);
	return latestData;

}


#endif

