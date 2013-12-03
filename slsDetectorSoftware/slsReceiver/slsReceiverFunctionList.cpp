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
#include <sys/mman.h>		//munmap



#include <string.h>
#include <iostream>
using namespace std;

FILE* slsReceiverFunctionList::sfilefd(NULL);
int slsReceiverFunctionList::receiver_threads_running(0);

slsReceiverFunctionList::slsReceiverFunctionList(detectorType det):
														myDetectorType(det),
														maxFramesPerFile(MAX_FRAMES_PER_FILE),
														enableFileWrite(1),
														fileIndex(0),
														frameIndexNeeded(0),
														framesCaught(0),
														acqStarted(false),
														measurementStarted(false),
														startFrameIndex(0),
														frameIndex(0),
														totalFramesCaught(0),
														totalPacketsCaught(0),
														framesInFile(0),
														startAcquisitionIndex(0),
														acquisitionIndex(0),
														prevframenum(0),
														listening_thread_running(0),
														writing_thread_running(0),
														status(IDLE),
														latestData(NULL),
														udpSocket(NULL),
														server_port(DEFAULT_UDP_PORTNO),
														fifo(NULL),
														fifofree(NULL),
														fifosize(GOTTHARD_FIFO_SIZE),
														shortFrame(-1),
														bufferSize(GOTTHARD_BUFFER_SIZE),
														packetsPerFrame(GOTTHARD_PACKETS_PER_FRAME),
														guiDataReady(0),
														guiData(NULL),
														guiFileName(NULL),
														currframenum(0),
														nFrameToGui(0),
														frameIndexMask(GOTTHARD_FRAME_INDEX_MASK),
														frameIndexOffset(GOTTHARD_FRAME_INDEX_OFFSET),
														dataCompression(false),
														numJobsPerThread(-1),
														currentPacketOffset(0),
														currentFrameOffset(0),
														currentPacketCount(0),
														currentFrameCount(-1),
														totalListeningFrameCount(0),
														acquisitionPeriod(SAMPLE_TIME_IN_NS),
														startAcquisitionCallBack(NULL),
														pStartAcquisition(NULL),
														acquisitionFinishedCallBack(NULL),
														pAcquisitionFinished(NULL),
														rawDataReadyCallBack(NULL),
														pRawDataReady(NULL)



{

	if(myDetectorType == MOENCH){
		fifosize = MOENCH_FIFO_SIZE;
		maxFramesPerFile = MOENCH_MAX_FRAMES_PER_FILE;
		bufferSize = MOENCH_BUFFER_SIZE;
		packetsPerFrame = MOENCH_PACKETS_PER_FRAME;
		frameIndexMask = MOENCH_FRAME_INDEX_MASK;
		frameIndexOffset = MOENCH_FRAME_INDEX_OFFSET;
	}

	oneBufferSize = bufferSize/packetsPerFrame;

	strcpy(savefilename,"");
	strcpy(filePath,"");
	strcpy(fileName,"run");
	guiFileName = new char[MAX_STR_LENGTH];
	strcpy(guiFileName,"");
	eth = new char[MAX_STR_LENGTH];
	strcpy(eth,"");

	latestData = new char[bufferSize];


	setupFifoStructure();


	//for the filter
	int16_t* map;
	int16_t* mask;
	int initial_offset = 2;
	int later_offset = 1;

	int mask_y_offset = 120;
	int mask_adc = 0x7fff;
	int num_packets_in_col = 4;
	int num_packets_in_row = 10;
	int num_pixels_per_packet_in_row = 40;
	int num_pixels_per_packet_in_col = 16;
	int offset,ipacket;


	int x,y,i,j;
	int ipx,ipy,ix,iy;

	/** not for roi */
	//filter
	switch(myDetectorType){
	case MOENCH:
		x = MOENCH_PIXELS_IN_ONE_ROW;
		y = MOENCH_PIXELS_IN_ONE_ROW;

		mask = new int16_t[x*y];
		map = new int16_t[x*y];

		//set up mask for moench
		for(i=0;i<x; i++)
			for(j=0;j<y; j++){
				if (j<mask_y_offset)
					mask[i*y+j] = mask_adc;
				else
					mask[i*y+j] = 0;
			}

		//set up mapping for moench
		for (ipy = 0; ipx < num_packets_in_col; ipx++ )
			for (ipx = 0; ipy < num_packets_in_row; ipy++ ){
				offset = initial_offset;
				for (ix = 0; ix < num_pixels_per_packet_in_col; ix++ ){
					for (iy = 0; iy < num_pixels_per_packet_in_row; iy++ ){
						ipacket = (ipx + 1) + (ipy * num_packets_in_row);
						if (ipacket == MOENCH_PACKETS_PER_FRAME)
							ipacket = 0;
						map[ (ipx * num_pixels_per_packet_in_col + ix) * y + (ipy * num_pixels_per_packet_in_row + iy) ] =
								ipacket * MOENCH_ONE_PACKET_SIZE + offset;
						offset += later_offset;
					}

				}
			}


		filter = new singlePhotonFilter(x,y, MOENCH_FRAME_INDEX_MASK, MOENCH_PACKET_INDEX_MASK, MOENCH_FRAME_INDEX_OFFSET,
				0, MOENCH_PACKETS_PER_FRAME, 0,map, mask,fifofree,MOENCH_BUFFER_SIZE,&totalFramesCaught,&framesCaught,&currframenum);
		break;
	default:
		x = 1;
		y = (GOTTHARD_DATA_BYTES/GOTTHARD_PACKETS_PER_FRAME);/*bufferSize/sizeof(int16_t)*/
		offset = initial_offset;

		mask = new int16_t[x*y];
		map = new int16_t[x*y];

		//set up mask for gotthard
		for (i=0; i < x; i++)
			for (j=0; j < y; j++){
				mask[i*y+j] = 0;
			}

		//set up mapping for gotthard
		for (i=0; i < x; i++)
			for (j=0; j < y; j++){
				//since there are 2 packets
				if (j == y/2){
					offset += initial_offset;
					offset += 1;
				}
				map[i*y+j] = offset;
				offset += 1;
			}


		filter = new singlePhotonFilter(x,y,GOTTHARD_FRAME_INDEX_MASK, GOTTHARD_PACKET_INDEX_MASK, GOTTHARD_FRAME_INDEX_OFFSET,
				0, GOTTHARD_PACKETS_PER_FRAME, 1,map, mask,fifofree,GOTTHARD_BUFFER_SIZE,&totalFramesCaught,&framesCaught,&currframenum);
		break;
	}


	pthread_mutex_init(&status_mutex,NULL);
	pthread_mutex_init(&dataReadyMutex,NULL);

	filter->registerCallBackFreeFifo(&(freeFifoBufferCallBack),this);


	dataCompression = false;
}



slsReceiverFunctionList::~slsReceiverFunctionList(){
	if(latestData) delete [] latestData;
	if(fifofree) delete [] fifofree;
	if(fifo) delete [] fifo;
	if(guiFileName) delete [] guiFileName;
	if(eth) delete [] eth;
	if(mem0) free(mem0);
}




int slsReceiverFunctionList::setEnableFileWrite(int i){
	if(i!=-1)
		enableFileWrite=i;
	return enableFileWrite;
}


void slsReceiverFunctionList::setEthernetInterface(char* c){
	strcpy(eth,c);
}



uint32_t slsReceiverFunctionList::getFrameIndex(){
	if(!framesCaught)
		frameIndex=0;
	else
		frameIndex = currframenum - startFrameIndex;
	return frameIndex;
}



uint32_t slsReceiverFunctionList::getAcquisitionIndex(){
	if(!totalFramesCaught)
		acquisitionIndex=0;
	else
		acquisitionIndex = currframenum - startAcquisitionIndex;
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
		else{
			strcpy(filePath,"");
			cout<<"FilePath does not exist:"<<filePath<<endl;
		}
	}
	return getFilePath();
}



int slsReceiverFunctionList::setFileIndex(int i){
	if(i>=0)
		fileIndex = i;
	return getFileIndex();
}


void slsReceiverFunctionList::resetTotalFramesCaught(){
	acqStarted = false;
	startAcquisitionIndex = 0;
	totalFramesCaught = 0;
	totalPacketsCaught = 0;
}



int slsReceiverFunctionList::startReceiver(char message[]){
//#ifdef VERBOSE
	cout << "Starting Receiver" << endl;
//#endif
	cout << endl;

	int err = 0;
	if(!receiver_threads_running){
#ifdef VERBOSE
		cout << "Starting new acquisition threadddd ...." << endl;
#endif
		//change status
		pthread_mutex_lock(&status_mutex);
		status = IDLE;
		listening_thread_running = 0;
		writing_thread_running = 0;
		receiver_threads_running = 1;
		pthread_mutex_unlock(&(status_mutex));


		// creating listening thread----------
		err = pthread_create(&listening_thread, NULL,startListeningThread, (void*) this);
		if(err){
			//change status
			pthread_mutex_lock(&status_mutex);
			status = IDLE;
			listening_thread_running = 0;
			receiver_threads_running = 0;
			pthread_mutex_unlock(&(status_mutex));

			sprintf(message,"Cant create listening thread. Status:%d\n",status);
			cout << endl << message << endl;
			return FAIL;
		}
		//wait till udp socket created
		while(!listening_thread_running);
		if(listening_thread_running==-1){
			//change status
			pthread_mutex_lock(&status_mutex);
			status = IDLE;
			listening_thread_running = 0;
			receiver_threads_running = 0;
			pthread_mutex_unlock(&(status_mutex));
			strcpy(message,"Could not create UDP Socket.\n");
			return FAIL;
		}
#ifdef VERBOSE
		cout << "Listening thread created successfully." << endl;
#endif

		// creating writing thread----------
		err = 0;
		err = pthread_create(&writing_thread, NULL,startWritingThread, (void*) this);
		if(err){
			//change status
			pthread_mutex_lock(&status_mutex);
			status = IDLE;
			writing_thread_running = 0;
			receiver_threads_running = 0;
			pthread_mutex_unlock(&(status_mutex));

			//stop listening thread
			pthread_join(listening_thread,NULL);
			sprintf(message,"Cant create writing thread. Status:%d\n",status);
			cout << endl << message << endl;
			return FAIL;
		}
		//wait till file is created
		while(!writing_thread_running);
		if(writing_thread_running==-1){
			//change status
			pthread_mutex_lock(&status_mutex);
			status = IDLE;
			listening_thread_running = 0;
			receiver_threads_running = 0;
			pthread_mutex_unlock(&(status_mutex));
			if(udpSocket) udpSocket->ShutDownSocket();
			pthread_join(listening_thread,NULL);
			sprintf(message,"Could not create file %s.\n",savefilename);
			return FAIL;
		}
#ifdef VERBOSE
		cout << "Writing thread created successfully." << endl;
#endif


		//change status----------
		pthread_mutex_lock(&status_mutex);
		status = RUNNING;
		pthread_mutex_unlock(&(status_mutex));
		cout << "Threads created successfully." << endl;




		struct sched_param tcp_param, listen_param, write_param;
		int policy= SCHED_RR;

		tcp_param.sched_priority = 50;
		listen_param.sched_priority = 99;
		write_param.sched_priority = 90;

		/*** ???????????????????????????*/
		if(!dataCompression){
			if (pthread_setschedparam(listening_thread, policy, &listen_param) == EPERM)
				cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;
			if (pthread_setschedparam(writing_thread, policy, &write_param) == EPERM)
				cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;
			if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
				cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;
		}

		//pthread_getschedparam(pthread_self(),&policy,&tcp_param);
		//cout << "current priority of main tcp thread is " << tcp_param.sched_priority << endl;

	}

	//initialize semaphore
	sem_init(&smp,0,1);

	return OK;
}





int slsReceiverFunctionList::stopReceiver(){
//#ifdef VERBOSE
	cout << "Stopping Receiver" << endl;
//#endif

	if(receiver_threads_running){
#ifdef VERBOSE
		cout << "Stopping new acquisition thread" << endl;
#endif
		//stop listening thread
		pthread_mutex_lock(&status_mutex);
		receiver_threads_running=0;
		pthread_mutex_unlock(&(status_mutex));

		if(listening_thread_running == 1){
			if(udpSocket) udpSocket->ShutDownSocket();
			pthread_join(listening_thread,NULL);
		}
		if(writing_thread_running == 1)
			pthread_join(writing_thread,NULL);

	}
	//change status
	pthread_mutex_lock(&status_mutex);
	status = IDLE;
	pthread_mutex_unlock(&(status_mutex));;

	//semaphore destroy
	sem_post(&smp);
	sem_destroy(&smp);


	cout << "Receiver Stopped.\nStatus:" << status << endl;
	return OK;
}



void* slsReceiverFunctionList::startListeningThread(void* this_pointer){
	((slsReceiverFunctionList*)this_pointer)->startListening();

	return this_pointer;
}





void slsReceiverFunctionList::processFrameForFifo(){

	//write packet count and set/increment counters/offsets
	if(currentFrameCount > -1){
		(*((uint8_t*)(buffer+currentFrameOffset))) = currentPacketCount;
		currentPacketCount = 0;
		currentFrameCount++;
//#ifdef VERYVERBOSE
		totalListeningFrameCount++;
//#endif
#ifdef VERYVERBOSE
		cout<<"lcurrframnum:"<< dec<<
			(((uint32_t)(*((uint32_t*)(buffer+currentPacketOffset))) & frameIndexMask) >> frameIndexOffset)<<"*"<<endl;
#endif
		currentFrameOffset += (HEADER_SIZE_NUM_PACKETS + bufferSize);
		currentPacketOffset = currentFrameOffset + HEADER_SIZE_NUM_PACKETS;
	}


	//write frame count for each buffer and write buffer
	if(currentFrameCount >= numJobsPerThread){
		(*((uint16_t*)buffer)) = currentFrameCount;
		while(!fifo->push(buffer));
#ifdef VERYVERBOSE
		cout << "lbuf1:" << (void*)buffer << endl;
#endif
	}

	//pop freefifo and reset counters, set offsets
	if((currentFrameCount >= numJobsPerThread) || (currentFrameCount == -1)){
		fifofree->pop(buffer);
#ifdef VERYVERBOSE
		cout << "lbuf1 popped:" << (void*)buffer << endl;
#endif
		currentFrameCount = 0;
		currentPacketCount = 0;
		currentPacketOffset = HEADER_SIZE_NUM_FRAMES + HEADER_SIZE_NUM_PACKETS;
		currentFrameOffset = HEADER_SIZE_NUM_FRAMES;
	}
}




int slsReceiverFunctionList::startListening(){
#ifdef VERYVERBOSE
	cout << "In startListening()\n");
#endif
	int rc=0;
	measurementStarted = false;
	startFrameIndex = 0;

	int ret=1;
	bool newFrame = true;
	char *tempchar = new char[oneBufferSize];
	int tempoffset= 0;


	currentPacketOffset = 0;
	currentFrameOffset = 0;
	currentFrameCount = -1;
	currentPacketCount = 0;
//#ifdef VERYVERBOSE
	totalListeningFrameCount = 0;
//#endif


	//to increase socket receiver buffer size and max length of input queue by changing kernel settings
	if(system("echo $((100*1024*1024)) > /proc/sys/net/core/rmem_max"))
		cout << "\nWARNING: Could not change socket receiver buffer size in file /proc/sys/net/core/rmem_max" << endl;
	else if(system("echo 250000 > /proc/sys/net/core/netdev_max_backlog"))
		cout << "\nWARNING: Could not change max length of input queue in file /proc/sys/net/core/netdev_max_backlog" << endl;

	/** permanent setting heiner
		net.core.rmem_max = 104857600 # 100MiB
		net.core.netdev_max_backlog = 250000
		sysctl -p
		// from the manual
		sysctl -w net.core.rmem_max=16777216
		sysctl -w net.core.netdev_max_backlog=250000
	 */

	//creating udp socket
	if (strchr(eth,'.')!=NULL) strcpy(eth,"");
	if(!strlen(eth)){
		cout<<"warning:eth is empty.listening to all"<<endl;
		if(udpSocket){
			delete udpSocket;
			udpSocket = NULL;
		}
		udpSocket = new genericSocket(server_port,genericSocket::UDP,oneBufferSize,1);//packetsPerFrame);
	}else{
		cout<<"eth:"<<eth<<endl;
		udpSocket = new genericSocket(server_port,genericSocket::UDP,oneBufferSize,1,eth);//packetsPerFrame,eth);
	}
	int iret = udpSocket->getErrorStatus();
	if (iret){
		//#ifdef VERBOSE
		std::cout<< "Could not create UDP socket on port "<< server_port  << " error:"<<iret<<std::endl;
		//#endif
		pthread_mutex_lock(&status_mutex);
		listening_thread_running = -1;
		pthread_mutex_unlock(&status_mutex);
		return 0;
	}else{

		while (receiver_threads_running) {

			if(newFrame)
				processFrameForFifo();


			//let tcp thread know this thread is in working condition
			if(!startFrameIndex){
				if(!listening_thread_running){
					pthread_mutex_lock(&status_mutex);
					listening_thread_running = 1;
					pthread_mutex_unlock(&(status_mutex));
				}
			}


			//receive 1 packet
			rc = udpSocket->ReceiveDataOnly(buffer+currentPacketOffset,oneBufferSize);
			if( rc <= 0){
#ifdef VERYVERBOSE
				cerr << "recvfrom() failed:"<<endl;
#endif
				if(status != TRANSMITTING){
					continue;
				}
				//the end of listening thread, after pushing off to fifo
				else{
					//push the last buffer into fifo
					if(currentFrameCount > 0){
						(*((uint8_t*)(buffer+currentFrameOffset))) = currentPacketCount;
						if(currentPacketCount != 0){
							currentFrameCount++;
							totalListeningFrameCount++;
						}
						(*((uint16_t*)buffer)) = currentFrameCount;
						fifo->push(buffer);
#ifdef VERYVERBOSE
						cout <<" last lbuf1:" << (void*)buffer << endl;
#endif
					}
					//push in dummy packet
					while(fifofree->isEmpty());
					fifofree->pop(buffer);
					(*((uint16_t*)buffer)) = 0xFFFF;
					fifo->push(buffer);
#ifdef VERYVERBOSE
					cout << "pushed in dummy buffer:" << (void*)buffer << endl;
#endif

					break;
				}
			}
			//manipulate buffer number to inlude frame number and packet number for gotthard
			if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
				(*((uint32_t*)(buffer+currentPacketOffset)))++;


			//start for each scan
			if(!measurementStarted){
				startFrameIndex = ((((uint32_t)(*((uint32_t*)(buffer+currentPacketOffset)))) & (frameIndexMask)) >> frameIndexOffset);
				cout<<"startFrameIndex:"<<startFrameIndex<<endl;
				prevframenum=startFrameIndex;
				measurementStarted = true;
			}

			//start of acquisition
			if(!acqStarted){
				startAcquisitionIndex=startFrameIndex;
				currframenum = startAcquisitionIndex;
				acqStarted = true;
				cout<<"startAcquisitionIndex:"<<startAcquisitionIndex<<endl;
			}


			ret = filter->verifyFrame(buffer+currentPacketOffset);


			//ret = -1, -3 :packets of next frame, so copy it to a later offset or to new buffer
			if(ret < -1){
				if((currentFrameCount + 1) >= numJobsPerThread){
					memcpy(tempchar, buffer+currentPacketOffset, oneBufferSize);
					processFrameForFifo();
					memcpy(buffer+currentPacketOffset,tempchar,oneBufferSize);
				}else{
					tempoffset = currentPacketCount;
					processFrameForFifo();
					memcpy(buffer+currentPacketOffset,buffer+tempoffset,oneBufferSize);
				}

				//ret = -2, not last frame of next packet. so wait for next packet
				if(ret == -2)
					ret = 0;
				//rer = -3, last packet, so new frame
			}

			currentPacketCount++;

			//ret = 0, wait for next packet
			if(ret == 0){
				currentPacketOffset += oneBufferSize;
				newFrame = false;
			}

			// ret = -1, 1, last packet rxd for current frame, so new frame please
			else
				newFrame = true;
		}
	}

	delete  tempchar;

	pthread_mutex_lock(&status_mutex);
	listening_thread_running = FINISHED;
	pthread_mutex_unlock(&(status_mutex));

//#ifdef VERYVERBOSE
	cout << "Total count listened to " << totalListeningFrameCount << endl;
//#endif
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

	char *wbuf;
	int numFrames = 0;
	int numFramesToBeSaved = 0;
	int i,offset,npackets;


	//reset counters
	framesInFile=0;
	framesCaught=0;
	frameIndex=0;
	if(sfilefd) sfilefd=NULL;
	strcpy(savefilename,"");
	//reset this before each acq or you send old data
	guiData = NULL;
	guiDataReady=0;
	strcpy(guiFileName,"");
	//by default, we read/write everything, this changes with the first callback
	cbAction = DO_EVERYTHING;

	//printouts
	cout << "Max Frames Per File:" << maxFramesPerFile << endl;
	if (rawDataReadyCallBack)
		cout << "Note: Data Write has been defined exernally" << endl;
	if (dataCompression)
		cout << "Data Compression is enabled with " << numJobsPerThread << " number of jobs per thread" << endl;
	if(nFrameToGui)
		cout << "Sending every " << nFrameToGui << "th frame to gui" <<  endl;


	//acquisition start call back returns enable write
	if (startAcquisitionCallBack)
		cbAction=startAcquisitionCallBack(filePath,fileName,fileIndex,bufferSize,pStartAcquisition);

	if(cbAction < DO_EVERYTHING)
		cout << endl << "Note: Call back activated. Data saving must be taken care of by user in call back." << endl;
	else if(enableFileWrite==0)
		cout << endl << "Note: Data will not be saved" << endl;


	//creating first file
	if(!dataCompression)
		i = createNewFile();
	else{
		//create file name for gui purposes, and set up acquistion parameters
		sprintf(savefilename, "%s/%s_fxxx_%d.raw", filePath,fileName,fileIndex);
		filter->setupAcquisitionParameters(filePath,fileName,fileIndex);
		// if(enableFileWrite && cbAction > DO_NOTHING)
		// This commented option doesnt exist as we save and do ebverything for data compression
		//create file
		i = filter->initTree();
	}

	//file/tree not created
	if(i == FAIL){
		cout << " Error: Could not create file " << savefilename << endl;
		pthread_mutex_lock(&status_mutex);
		writing_thread_running = -1;
		pthread_mutex_unlock(&(status_mutex));
	}

	else{
		//let tcp thread know it started successfully
		pthread_mutex_lock(&status_mutex);
		writing_thread_running = 1;
		pthread_mutex_unlock(&(status_mutex));

		cout << "Ready!" << endl;

		//will always run till acquisition over and then runs till fifo is empty
		while(receiver_threads_running){// || (!fifo->isEmpty())){
			//pop fifo
			if(fifo->pop(wbuf)){

				//number of frames per buffer
				numFrames =	(uint16_t)(*((uint16_t*)wbuf));



				//last dummy packet
				if(numFrames == 0xFFFF){
#ifdef VERYVERBOSE
					cout << "popped last dummy frame:" << (void*)wbuf << endl;
#endif
					//fifofree->push(wbuf);
/*
					cout <<"fifofree is full?:" << fifofree->isFull()<<endl;
					cout <<"fifo is empty?:" << fifo->isEmpty()<<endl;
*/
					//if(status == TRANSMITTING)cout<<"transmitting as well"<<endl; else cout <<"not transmitting"<<endl;
					if(fifo->isEmpty() && status == TRANSMITTING){
						//data compression
						if(dataCompression){
							//check if jobs done
							while(!filter->checkIfJobsDone())
								usleep(50000);
						}
#ifdef VERYVERBOSE
						cout << "fifo freed:" << (void*)wbuf << endl;
#endif
						fifofree->push(wbuf);
/*
						cout <<"fifofree is full?:" << fifofree->isFull()<<endl;
						cout <<"fifo is empty?:" << fifo->isEmpty()<<endl;
*/
						pthread_mutex_lock(&status_mutex);
						status = RUN_FINISHED;
						pthread_mutex_unlock(&(status_mutex));
						cout << "Status: Run Finished" << endl;
						break;
					}else{
						cout<<"*************************should not be here!"<<endl;
						numJobsPerThread = -1;
						break;
					}

				}




				//real frames
#ifdef VERYVERBOSE
				cout << "buf1:" << (void*)wbuf << endl;
#endif
				//must be updated only in singlephoton if compression, else lock issues. (exception in the beginning for startframeindex)
				if(!dataCompression){
					currframenum = (((uint32_t)(*((uint32_t*)(wbuf + HEADER_SIZE_NUM_FRAMES + HEADER_SIZE_NUM_PACKETS)))& frameIndexMask) >>frameIndexOffset);
#ifdef VERYVERBOSE
					cout << "currframnum:" << dec << currframenum << endl;
#endif
				}
				//send it for writing and data compression
				if(dataCompression)
					filter->assignJobsForThread(wbuf, numFrames);

				//standard way
				else{
					//write data call back
					if (cbAction < DO_EVERYTHING) {
						rawDataReadyCallBack(currframenum, wbuf, numFrames * bufferSize, sfilefd, guiData,pRawDataReady);
					}
					else {
						offset = HEADER_SIZE_NUM_FRAMES;

						while(numFrames > 0){
							//to make sure, we write according to max frames per file
							numFramesToBeSaved = maxFramesPerFile - framesInFile;
							if(numFramesToBeSaved > numFrames)
								numFramesToBeSaved = numFrames;

							//write packets to file
							for(i = 0; i < numFramesToBeSaved; i++){
								//determine number of packets
								npackets = (uint8_t)(*((uint8_t*)(wbuf + offset)));
								totalPacketsCaught += npackets;
								offset += HEADER_SIZE_NUM_PACKETS;
								//write to file
								if(enableFileWrite){
									if(sfilefd)
										fwrite(wbuf+offset, 1, npackets * oneBufferSize, sfilefd);
									else{
										if(!totalPacketsCaught)
											cout << "ERROR: You do not have permissions to overwrite: " << savefilename << endl;
									}
								}
								if(npackets == packetsPerFrame){
									framesCaught++;
									totalFramesCaught++;
								}

								//increment offset
								offset += bufferSize;
							}



							//increment/decrement counters
							framesInFile += numFramesToBeSaved;
							numFrames -= numFramesToBeSaved;
							//create new file
							if(framesInFile >= maxFramesPerFile)
								createNewFile();

						}
					}
				}

				if(((uint8_t)(*((uint8_t*)(wbuf + HEADER_SIZE_NUM_FRAMES)))) == packetsPerFrame){
					copyFrameToGui(wbuf + HEADER_SIZE_NUM_FRAMES + HEADER_SIZE_NUM_PACKETS);
				}


				if(!dataCompression){
					fifofree->push(wbuf);
#ifdef VERYVERBOSE
					cout<<"buf freed:"<<(void*)wbuf<<endl;
#endif
				}
			}

		}
	}



	if(!dataCompression){
		if(sfilefd){
#ifdef VERBOSE
			cout << "sfield:" << (int)sfilefd << endl;
#endif
			fclose(sfilefd);
			sfilefd = NULL;
		}
		cout << "Total Packets Caught and written to files:" << dec << totalPacketsCaught << endl;
	}
	cout << "Total Full Frames Caught:"<< dec << totalFramesCaught << endl;



	//acquistion over call back
	if (acquisitionFinishedCallBack)
		acquisitionFinishedCallBack(totalFramesCaught, pAcquisitionFinished);


	pthread_mutex_lock(&status_mutex);
	writing_thread_running = FINISHED;
	pthread_mutex_unlock(&(status_mutex));


	return 0;
}



int slsReceiverFunctionList::createNewFile(){

	//create file name
	if(frameIndexNeeded==-1)
		sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
	else
		sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,framesCaught,fileIndex);


	//if filewrite and we are allowed to write
	if(enableFileWrite && cbAction > DO_NOTHING){
		//close
		if(sfilefd){
			fclose(sfilefd);
			sfilefd = NULL;
		}
		//open file
		if (NULL == (sfilefd = fopen((const char *) (savefilename), "w"))){
			cout << "Error: Could not create file " << savefilename << endl;
			pthread_mutex_lock(&status_mutex);
			writing_thread_running = -1;
			pthread_mutex_unlock(&(status_mutex));
			return FAIL;
		}
		//setting buffer
		setvbuf(sfilefd,NULL,_IOFBF,BUF_SIZE);
		//printing packet losses and file names
		if(!framesCaught)
			cout << savefilename << endl;
		else{
			cout << savefilename
					<< "\tpacket loss "
					<< setw(4)<<fixed << setprecision(4)<< dec <<
					(int)((((currframenum-prevframenum)-framesInFile)/(double)(currframenum-prevframenum))*100.000)
					<< "%\tframenum "
					<< dec << currframenum //<< "\t\t p " << prevframenum
					<< "\tindex " << dec << getFrameIndex()
					<< "\tpackets lost " << dec << (currframenum-prevframenum)-framesInFile << endl;

		}
	}

	//reset counters for each new file
	if(framesCaught){
		prevframenum = currframenum;
		framesInFile = 0;
	}

	return OK;
}


void slsReceiverFunctionList::copyFrameToGui(char* startbuf){

	//random read when gui not ready
	if((!nFrameToGui) && (!guiData)){
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
		pthread_mutex_unlock(&dataReadyMutex);
	}

	//random read or nth frame read, gui needs data now
	else{
		//nth frame read
		//block current process if the guireader hasnt read it yet
		if(nFrameToGui)
			sem_wait(&smp);

		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
		/*pthread_mutex_unlock(&dataReadyMutex);*/
		//send the first one
		memcpy(latestData,startbuf,bufferSize);
		strcpy(guiFileName,savefilename);
		/*pthread_mutex_lock(&dataReadyMutex);*/
		guiDataReady=1;
		pthread_mutex_unlock(&dataReadyMutex);
	}
}





void slsReceiverFunctionList::readFrame(char* c,char** raw){
	//point to gui data
	if (guiData == NULL)
		guiData = latestData;

	//copy data and filename
	strcpy(c,guiFileName);
	//could not get gui data
	if(!guiDataReady){
		*raw = NULL;
	}
	//data ready, set guidata to receive new data
	else{
		*raw = guiData;
		guiData = NULL;

		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady = 0;
		pthread_mutex_unlock(&dataReadyMutex);
		if((nFrameToGui) && (receiver_threads_running)){
			//release after getting data
			sem_post(&smp);
		}
	}
}




int slsReceiverFunctionList::setShortFrame(int i){
	shortFrame=i;

	if(shortFrame!=-1){
		bufferSize = GOTTHARD_SHORT_BUFFER_SIZE;
		maxFramesPerFile = SHORT_MAX_FRAMES_PER_FILE;
		packetsPerFrame = GOTTHARD_SHORT_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_SHORT_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_SHORT_FRAME_INDEX_OFFSET;

	}else{
		bufferSize = GOTTHARD_BUFFER_SIZE;
		maxFramesPerFile = MAX_FRAMES_PER_FILE;
		packetsPerFrame = GOTTHARD_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_FRAME_INDEX_OFFSET;
	}


	oneBufferSize = bufferSize/packetsPerFrame;

	//if the filter is inititalized with the wrong readout
	if(filter->getPacketsPerFrame() != packetsPerFrame){

		int16_t* map;
		int16_t* mask;

		int initial_offset = 2;
		int x,y,i,j,offset = 0;

		switch(packetsPerFrame){
		case GOTTHARD_SHORT_PACKETS_PER_FRAME://roi readout for gotthard
			x = 1;
			y = (GOTTHARD_SHORT_DATABYTES/sizeof(int16_t));
			offset = initial_offset;


			mask = new int16_t[x*y];
			map = new int16_t[x*y];

			//set up mask for gotthard short
			for (i=0; i < x; i++)
				for (j=0; j < y; j++){
					mask[i*y+j] = 0;
				}
			//set up mapping for gotthard short
			for (i=0; i < x; i++)
				for (j=0; j < y; j++){
					map[i*y+j] = offset;
					offset += 2;
				}

			delete filter;
			filter = new singlePhotonFilter(x,y,frameIndexMask, GOTTHARD_PACKET_INDEX_MASK, frameIndexOffset,
					0, GOTTHARD_SHORT_PACKETS_PER_FRAME, 0,map, mask,fifofree,GOTTHARD_SHORT_BUFFER_SIZE,&totalFramesCaught,&framesCaught,&currframenum);
			break;

		default: //normal readout for gotthard
			x = 1;
			y = (GOTTHARD_DATA_BYTES/sizeof(int16_t));
			offset = initial_offset;

			mask = new int16_t[x*y];
			map = new int16_t[x*y];

			//set up mask for gotthard
			for (i=0; i < x; i++)
				for (j=0; j < y; j++){
					mask[i*y+j] = 0;
				}

			//set up mapping for gotthard
			for (i=0; i < x; i++)
				for (j=0; j < y; j++){
					//since there are 2 packets
					if (j == y/2){
						offset += initial_offset;
						offset += 1;
					}
					map[i*y+j] = offset;
					offset += 1;
				}

			delete filter;
			filter = new singlePhotonFilter(x,y,frameIndexMask, GOTTHARD_PACKET_INDEX_MASK, frameIndexOffset,
					0, GOTTHARD_PACKETS_PER_FRAME, 1,map, mask,fifofree,GOTTHARD_BUFFER_SIZE,&totalFramesCaught,&framesCaught,&currframenum);
			break;
		}



	}
	return shortFrame;
}



void slsReceiverFunctionList::startReadout(){
	//wait so that all packets which take time has arrived
	usleep(50000);

	pthread_mutex_lock(&status_mutex);
	status = TRANSMITTING;
	pthread_mutex_unlock(&status_mutex);
	cout << "Status: Transmitting" << endl;

	//push last packet
	if((udpSocket) && (listening_thread_running == 1))
		udpSocket->ShutDownSocket();
}
#endif




int slsReceiverFunctionList::setNFrameToGui(int i){
	if(i>=0){
		nFrameToGui = i;
		setupFifoStructure();
	}
	return nFrameToGui;
};


int64_t slsReceiverFunctionList::setAcquisitionPeriod(int64_t index){
	if(index >= 0){
		if(index != acquisitionPeriod){
			acquisitionPeriod = index;
			setupFifoStructure();
		}
	}
	return acquisitionPeriod;
};



void slsReceiverFunctionList::setupFifoStructure(){
	int64_t i;
	int oldn = numJobsPerThread;

	//if every nth frame mode
	if(nFrameToGui)
		numJobsPerThread = nFrameToGui;

	//random nth frame mode
	else{
		if(!acquisitionPeriod)
			i = SAMPLE_TIME_IN_NS;
		else
			i = SAMPLE_TIME_IN_NS/acquisitionPeriod;
		if (i > MAX_JOBS_PER_THREAD)
			numJobsPerThread = MAX_JOBS_PER_THREAD;
		else if (i < 1)
			numJobsPerThread = 1;
		else
			numJobsPerThread = i;
	}

	/*for testing
	 numJobsPerThread = 3;*/

	//if same, return
	if(oldn == numJobsPerThread)
		return;


	//deleting old structure and creating fifo structure
	if(fifofree){
		while(!fifofree->isEmpty())
			fifofree->pop(buffer);
		delete fifofree;
	}
	if(fifo)		delete fifo;
	if(mem0) 		free(mem0);
	fifofree 	= new CircularFifo<char>(fifosize);
	fifo 		= new CircularFifo<char>(fifosize);


	//otherwise memory too much if numjobsperthread is at max = 1000
	fifosize = GOTTHARD_FIFO_SIZE;
	if(myDetectorType == MOENCH)
		fifosize = MOENCH_FIFO_SIZE;

	if(fifosize % numJobsPerThread)
		fifosize = (fifosize/numJobsPerThread)+1;
	else
		fifosize = fifosize/numJobsPerThread;



	/*for testing
	 fifosize = 11;*/


	//allocate memory
	mem0=(char*)malloc(((bufferSize+HEADER_SIZE_NUM_PACKETS)*numJobsPerThread+HEADER_SIZE_NUM_FRAMES)*fifosize);

	if (mem0==NULL) /** shud let the client know about this */
		cout<<"++++++++++++++++++++++ COULD NOT ALLOCATE MEMORY!!!!!!!+++++++++++++++++++++" << endl;
	buffer=mem0;

	//push the addresses into freed fifo
	while (buffer<(mem0+((bufferSize+HEADER_SIZE_NUM_PACKETS)*numJobsPerThread+HEADER_SIZE_NUM_FRAMES)*(fifosize-1))) {
		fifofree->push(buffer);
		buffer+=((bufferSize+HEADER_SIZE_NUM_PACKETS)*numJobsPerThread+HEADER_SIZE_NUM_FRAMES);
	}

	cout<<"Number of Frames per buffer:"<<numJobsPerThread<<endl;
	cout << "Fifo structure reconstructed" << endl;
}
