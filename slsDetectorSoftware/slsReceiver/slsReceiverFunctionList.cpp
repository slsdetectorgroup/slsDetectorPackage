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
														startAcquisitionIndex(0),
														acquisitionIndex(0),
														packetsInFile(0),
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
														startAcquisitionCallBack(NULL),
														pStartAcquisition(NULL),
														acquisitionFinishedCallBack(NULL),
														pAcquisitionFinished(NULL),
														rawDataReadyCallBack(NULL),
														pRawDataReady(NULL)



{
	int aligned_frame_size = GOTTHARD_ALIGNED_FRAME_SIZE;

	if(myDetectorType == MOENCH){
		aligned_frame_size = MOENCH_ALIGNED_FRAME_SIZE;
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
	fifofree = new CircularFifo<char>(fifosize);
	fifo = new CircularFifo<char>(fifosize);



	mem0=(char*)malloc(aligned_frame_size*fifosize);
	if (mem0==NULL) {
		cout<<"++++++++++++++++++++++ COULD NOT ALLOCATE MEMORY!!!!!!!+++++++++++++++++++++" << endl;
	}
	buffer=mem0;
	while (buffer<(mem0+aligned_frame_size*(fifosize-1))) {
		fifofree->push(buffer);
		buffer+=aligned_frame_size;
	}


	vector <vector<int16_t> > map;
	vector <vector<int16_t> > mask;
	int initial_offset = 4;
	int later_offset = 2;

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
		mask.resize(x);
		for(i=0;i<x; i++)
			mask[i].resize(y);
		map.resize(x);
		for(i=0;i<x; i++)
			map[i].resize(y);


		//set up mask for moench
		for(i=0;i<x; i++)
			for(j=0;j<y; j++){
				if (j<mask_y_offset)
					mask[i][j] = mask_adc;
				else
					mask[i][j] = 0;
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
						map[ ipx * num_pixels_per_packet_in_col + ix][ ipy * num_pixels_per_packet_in_row + iy] =
								ipacket * MOENCH_ONE_PACKET_SIZE + offset;
						offset += later_offset;
					}

				}
			}


		filter = new singlePhotonFilter(x,y, MOENCH_FRAME_INDEX_MASK, MOENCH_PACKET_INDEX_MASK, MOENCH_FRAME_INDEX_OFFSET, 0, MOENCH_PACKETS_PER_FRAME, 0,map, mask,MOENCH_BUFFER_SIZE);
		break;
	default:
		x = 1;
		y = (GOTTHARD_DATA_BYTES/GOTTHARD_PACKETS_PER_FRAME);/*bufferSize/sizeof(int16_t)*/
		offset = initial_offset;

		mask.resize(x);
		for(int i=0;i<x; i++)
			mask[i].resize(y);
		map.resize(x);
		for(int i=0;i<x; i++)
			map[i].resize(y);

		//set up mask for moench
		for (i=0; i < x; i++)
			for (j=0; j < y; j++){
				mask[i][j] = 0;
			}

		//set up mapping for gotthard
		for (i=0; i < x; i++)
			for (j=0; j < y; j++){
				//since there are 2 packets
				if (y == y/2)
					offset += initial_offset;
				map[i][j] = offset;
				offset += 2;
			}


		filter = new singlePhotonFilter(x,y,GOTTHARD_FRAME_INDEX_MASK, GOTTHARD_PACKET_INDEX_MASK, GOTTHARD_FRAME_INDEX_OFFSET, 0, GOTTHARD_PACKETS_PER_FRAME, 1,map, mask,GOTTHARD_BUFFER_SIZE);
		break;
	}

	pthread_mutex_init(&status_mutex,NULL);
	pthread_mutex_init(&dataReadyMutex,NULL);


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
#ifdef VERBOSE
	cout << "Starting Receiver" << endl;
#endif
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
		if(listening_thread_running!=1){
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
		if(writing_thread_running!=1){
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


		if (pthread_setschedparam(listening_thread, policy, &listen_param) == EPERM)
			cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;
		if (pthread_setschedparam(writing_thread, policy, &write_param) == EPERM)
			cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;
		if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
			cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;


		//pthread_getschedparam(pthread_self(),&policy,&tcp_param);
		//cout << "current priority of main tcp thread is " << tcp_param.sched_priority << endl;

	}

	//initialize semaphore
	sem_init(&smp,0,1);


	return OK;
}





int slsReceiverFunctionList::stopReceiver(){
#ifdef VERBOSE
	cout << "Stopping Receiver" << endl;
#endif

	if(receiver_threads_running){
#ifdef VERBOSE
		cout << "Stopping new acquisition threadddd ...." << endl;
#endif
		//stop listening thread
		pthread_mutex_lock(&status_mutex);
		receiver_threads_running=0;
		pthread_mutex_unlock(&(status_mutex));

		if(udpSocket) udpSocket->ShutDownSocket();
		pthread_join(listening_thread,NULL);
		pthread_join(writing_thread,NULL);
	}
	//change status
	pthread_mutex_lock(&status_mutex);
	status = IDLE;
	pthread_mutex_unlock(&(status_mutex));

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










int slsReceiverFunctionList::startListening(){
#ifdef VERYVERBOSE
	cout << "In startListening()\n");
#endif
	int rc;

	measurementStarted = false;
	startFrameIndex = 0;

	int offset=0;
	int ret=1;
	int i=0;
	uint32_t *framenum;
	char *tempchar = new char[oneBufferSize];


	// A do/while(FALSE) loop is used to make error cleanup easier.  The
	//  close() of each of the socket descriptors is only done once at the
	//  very end of the program.
	do {

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
		if (udpSocket->getErrorStatus()){
#ifdef VERBOSE
			std::cout<< "Could not create UDP socket "<< server_port  << std::endl;
#endif
			pthread_mutex_lock(&status_mutex);
			listening_thread_running = -1;
			pthread_mutex_unlock(&status_mutex);
			break;
		}

		filter->setupAcquisitionParameters();

		while (receiver_threads_running) {
			if(!listening_thread_running){
				pthread_mutex_lock(&status_mutex);
				listening_thread_running = 1;
				pthread_mutex_unlock(&(status_mutex));
			}

			if (!fifofree->isEmpty()) {
				if (ret!=0)
					fifofree->pop(buffer);

				if(ret == -2){
					memcpy(buffer,tempchar,oneBufferSize);
					offset = oneBufferSize;
				}

				//receiver 2 half frames / 1 short frame / 40 moench frames
				rc = udpSocket->ReceiveDataOnly(buffer+offset,oneBufferSize);
				if( rc <= 0){
#ifdef VERYVERBOSE
					cerr << "recvfrom() failed" << endl;
#endif
					continue;
				}
				//cout<<"got index:"<<hex<<(((uint32_t)(*((uint32_t*)(buffer+offset))) & (frameIndexMask)) >> frameIndexOffset);


				if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
					(*((uint32_t*)(buffer+offset)))++;


				ret = filter->verifyFrame(buffer+offset);


				//start for each scan
				if(!measurementStarted){
					startFrameIndex = ((((uint32_t)(*((uint32_t*)buffer))) & (frameIndexMask)) >> frameIndexOffset);
					cout<<"startFrameIndex:"<<hex<<startFrameIndex<<endl;
					prevframenum=startFrameIndex;
					measurementStarted = true;
				}

				//start of acquisition
				if(!acqStarted){
					startAcquisitionIndex=startFrameIndex;
					currframenum = startAcquisitionIndex;
					acqStarted = true;
					cout<<"startAcquisitionIndex:"<<hex<<startAcquisitionIndex<<endl;
				}


				/*
				if(ret < 0){
					offset = 0;
					continue;
				}
				 */
				//last packet, but not the full frame, must push previous frame
				if(ret == -1){
					//set reminaing headers invalid
					for( i = offset+ oneBufferSize; i < bufferSize; i += oneBufferSize)
						(*((uint32_t*)(buffer+i))) = 0xFFFFFFFF;
					offset = 0;
				}

				//first packet of new frame, must push previous frame
				else if(ret == -2){
					//copy the new frame to a temp
					memcpy(tempchar, buffer+offset, oneBufferSize);
					//set all the new frame header and remaining headers invalid
					for(i = offset; i < bufferSize; i += oneBufferSize);
					(*((uint32_t*)(buffer+i))) = 0xFFFFFFFF;
				}
				//wait for new packets of same frame
				else if (ret == 0){
					offset += oneBufferSize;
					continue;
				}
				//wait for next frame
				else{
					offset = 0;
				}

				//so that it doesnt write the last frame twice
				if((receiver_threads_running) && (!fifo->isFull())){
					fifo->push(buffer);
				}

			}
		}
	} while (receiver_threads_running);
	pthread_mutex_lock(&status_mutex);
	receiver_threads_running=0;
	status = IDLE;
	pthread_mutex_unlock(&status_mutex);


	//Close down any open socket descriptors
	udpSocket->Disconnect();
	delete tempchar;

#ifdef VERBOSE
	cout << "receiver_threads_running:" << receiver_threads_running << endl;
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

	char *wbuf;
	int sleepnumber=0;
	int frameFactor=0;
	int i;

	packetsInFile=0;
	framesCaught=0;
	frameIndex=0;
	if(sfilefd) sfilefd=NULL;
	strcpy(savefilename,"");



	//reset this before each acq or you send old data
	guiData = NULL;
	guiDataReady=0;
	strcpy(guiFileName,"");

	cout << "Max Frames Per File:" << maxFramesPerFile << endl;
	if (rawDataReadyCallBack)
		cout << "Note: Data Write has been defined exernally" << endl;
	if(nFrameToGui)
		cout << "Sending every " << nFrameToGui << "th frame to gui" <<  endl;


	//by default, we read/write everything
	cbAction = DO_EVERYTHING;
	//acquisition start call back returns enable write
	if (startAcquisitionCallBack)
		cbAction=startAcquisitionCallBack(filePath,fileName,fileIndex,bufferSize,pStartAcquisition);
	if(cbAction < DO_EVERYTHING)
		cout << endl << "Note: Call back activated. Data saving must be taken care of by user in call back." << endl;
	else if(enableFileWrite==0)
		cout << endl << "Note: Data will not be saved" << endl;


	cout << "Ready!" << endl;


	if (dataCompression)
		filter->enableFilter(true);

	//will always run till acquisition over and then runs till fifo is empty
	while(receiver_threads_running || (!fifo->isEmpty())){

		//start a new file
		if (((int)(packetsInFile/packetsPerFrame) >= maxFramesPerFile)  || (strlen(savefilename) == 0)){

			//create file name
			if(frameIndexNeeded==-1)	sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
			else						sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,framesCaught,fileIndex);

			if(enableFileWrite && cbAction > DO_NOTHING){

				//create tree and file
				if(dataCompression){
					if(enableFileWrite){
						filter->writeToFile();
						filter->initTree(savefilename);
					}
				}
				/*else{*///the standard way
				if(sfilefd){
					fclose(sfilefd);
					sfilefd = NULL;
				}

				if (NULL == (sfilefd = fopen((const char *) (savefilename), "w"))){
					cout << "Error: Could not create file " << savefilename << endl;
					pthread_mutex_lock(&status_mutex);
					writing_thread_running = -1;
					pthread_mutex_unlock(&(status_mutex));
					break;
				}
				//setting buffer
				setvbuf(sfilefd,NULL,_IOFBF,BUF_SIZE);
				/*}*/

				//printing packet losses and file names
				//if(prevframenum != 0)
				if(!framesCaught)
					cout << savefilename << endl;
				else{
					cout << savefilename
							<< "\tpacket loss "
							<< setw(4)<<fixed << setprecision(4)<< dec <<
							(int)(((((currframenum-prevframenum)*packetsPerFrame)-(packetsInFile))/(double)((currframenum-prevframenum)*packetsPerFrame))*100.000)/*packetloss*/
							<< "%\tframenum "
							<< dec << currframenum //<< "\t\t p " << prevframenum
							<< "\tindex " << dec << getFrameIndex()
							<< "\tpackets lost " << dec << ((currframenum-prevframenum)*packetsPerFrame)-(packetsInFile) << endl;
				}
			}

			pthread_mutex_lock(&status_mutex);
			writing_thread_running = 1;
			pthread_mutex_unlock(&(status_mutex));


			//if(prevframenum != 0){
			if(framesCaught){
				prevframenum = currframenum;
				packetsInFile=0;
			}
		}


		//pop fifo
		if(!fifo->isEmpty()){

			if(fifo->pop(wbuf)){

				currframenum = ((uint32_t)(*((uint32_t*)wbuf))& frameIndexMask) >>frameIndexOffset;
				//cout<<"currframenum: "<<hex<<currframenum<<endl;
				//cout<<"currframenum2:"<<hex<<((((uint32_t)(*((uint32_t*)((char*)(wbuf+oneBufferSize)))))& frameIndexMask) >> frameIndexOffset)<<endl;;

				for(i=0; i < packetsPerFrame; i++){
					if(((uint32_t)(*((uint32_t*)((char*)(wbuf+oneBufferSize))))) == 0xFFFFFFFF){
						//cout<<"found one: currframenum:"<<currframenum<<" currframe2:"<<((((uint32_t)(*((uint32_t*)((char*)(wbuf+oneBufferSize)))))& frameIndexMask) >> frameIndexOffset)<<endl;
						break;
					}
				}

				packetsInFile += i;
				totalPacketsCaught += i;
				//count only if you get full frames
				if(i == packetsPerFrame){
					framesCaught++;
					totalFramesCaught++;
				}

				//write data call back
				if (cbAction < DO_EVERYTHING) {
					rawDataReadyCallBack(currframenum, wbuf, i*oneBufferSize, sfilefd, guiData,pRawDataReady);
				}
				//default writing to file
				else if(enableFileWrite){
					/*if(!dataCompression){*/
					if(sfilefd)
						fwrite(wbuf, 1, i*oneBufferSize, sfilefd);
					else{
						cout << "You do not have permissions to overwrite: " << savefilename << endl;
						usleep(50000);
					}
					/*}*/
				}





				//does not read every frame
				if(!nFrameToGui){
					if((guiData) && (i == packetsPerFrame)){
						pthread_mutex_lock(&dataReadyMutex);
						guiDataReady=0;
						pthread_mutex_unlock(&dataReadyMutex);
						memcpy(latestData,wbuf,bufferSize);
						strcpy(guiFileName,savefilename);
						pthread_mutex_lock(&dataReadyMutex);
						guiDataReady=1;
						pthread_mutex_unlock(&dataReadyMutex);
					}else{
						pthread_mutex_lock(&dataReadyMutex);
						guiDataReady=0;
						pthread_mutex_unlock(&dataReadyMutex);
					}
				}
				//reads every nth frame
				else{
					if (i != packetsPerFrame)//so no 1 packet frame writing over previous 2 packet frame
						;
					else if(frameFactor){
						frameFactor--;
					}else{
						frameFactor = nFrameToGui-1;
						//block current process if the guireader hasnt read it yet
						sem_wait(&smp);
						//copy data and set guidataready
						pthread_mutex_lock(&dataReadyMutex);
						guiDataReady=0;
						pthread_mutex_unlock(&dataReadyMutex);
						memcpy(latestData,wbuf,bufferSize);
						strcpy(guiFileName,savefilename);
						pthread_mutex_lock(&dataReadyMutex);
						guiDataReady = 1;
						pthread_mutex_unlock(&dataReadyMutex);

					}
				}

				fifofree->push(wbuf);
			}
		}
		else{//cout<<"************************fifo empty**********************************"<<endl;
			//change status to idle if the fifo is empty and status is transmitting
			if(status == TRANSMITTING){
				pthread_mutex_lock(&status_mutex);
				status = RUN_FINISHED;
				pthread_mutex_unlock(&(status_mutex));
				cout << "Status: Run Finished" << endl;
			}else{
				sleepnumber++;
				usleep(50000);
			}
		}
	}

	pthread_mutex_lock(&status_mutex);
	receiver_threads_running=0;
	pthread_mutex_unlock(&status_mutex);

	cout << "Total Packets Caught:" << dec << totalPacketsCaught << endl;
	//cout << "RealTime Full Frames Caught:" << dec << framesCaught << endl;
	cout << "Total Full Frames Caught:"<< dec << totalFramesCaught << endl;


	if(sfilefd){
#ifdef VERBOSE
		cout << "sfield:" << (int)sfilefd << endl;
#endif
		fclose(sfilefd);
		sfilefd = NULL;
	}

	//acquistion over call back
	if (acquisitionFinishedCallBack)
		acquisitionFinishedCallBack(totalFramesCaught, pAcquisitionFinished);

	return 0;
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

		vector <vector<int16_t> > map;
		vector <vector<int16_t> > mask;
		int initial_offset = 4;
		int later_offset = 2;
		int x,y,i,j,offset = 0;

		switch(packetsPerFrame){
		case GOTTHARD_SHORT_PACKETS_PER_FRAME://roi readout for gotthard
			x = 1;
			y = (GOTTHARD_DATA_BYTES/GOTTHARD_PACKETS_PER_FRAME)/2;
			offset = initial_offset;

			mask.resize(x);
			for(int i=0;i<x; i++)
				mask[i].resize(y);
			map.resize(x);
			for(int i=0;i<x; i++)
				map[i].resize(y);
			//set up mask for moench
			for (int i=0; i < x; i++)
				for (int j=0; j < y; j++){
					mask[i][j] = 0;
				}
			//set up mapping for gotthard
			for (int i=0; i < x; i++)
				for (int j=0; j < y; j++){
					map[i][j] = offset;
					offset += 2;
				}

			delete filter;
			filter = new singlePhotonFilter(x,y,frameIndexMask, GOTTHARD_PACKET_INDEX_MASK, frameIndexOffset, 0, GOTTHARD_SHORT_PACKETS_PER_FRAME, 0,map, mask,GOTTHARD_SHORT_BUFFER_SIZE);
			break;

		default: //normal readout for gotthard
			x = 1;
			y = (GOTTHARD_DATA_BYTES/GOTTHARD_PACKETS_PER_FRAME);
			offset = initial_offset;

			mask.resize(x);
			for(int i=0;i<x; i++)
				mask[i].resize(y);
			map.resize(x);
			for(int i=0;i<x; i++)
				map[i].resize(y);

			//set up mask for moench
			for (int i=0; i < x; i++)
				for (int j=0; j < y; j++){
					mask[i][j] = 0;
				}

			//set up mapping for gotthard
			for (int i=0; i < x; i++)
				for (int j=0; j < y; j++){
					//since there are 2 packets
					if (y == y/2)
						offset += initial_offset;
					map[i][j] = offset;
					offset += 2;
				}

			delete filter;
			filter = new singlePhotonFilter(x,y,frameIndexMask, GOTTHARD_PACKET_INDEX_MASK, frameIndexOffset, 0, GOTTHARD_PACKETS_PER_FRAME, 1,map, mask,GOTTHARD_BUFFER_SIZE);
			break;
		}



	}
	return shortFrame;
}




void slsReceiverFunctionList::startReadout(){
	pthread_mutex_lock(&status_mutex);
	status = TRANSMITTING;
	pthread_mutex_unlock(&status_mutex);
	cout << "Status: Transmitting" << endl;
}
#endif

