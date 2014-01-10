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
					status(IDLE),
					udpSocket(NULL),
					server_port(DEFAULT_UDP_PORTNO),
					eth(NULL),
					maxPacketsPerFile(0),
					enableFileWrite(1),
					fileIndex(0),
					frameIndexNeeded(0),
					acqStarted(false),
					measurementStarted(false),
					startFrameIndex(0),
					frameIndex(0),
					packetsCaught(0),
					totalPacketsCaught(0),
					packetsInFile(0),
					startAcquisitionIndex(0),
					acquisitionIndex(0),
					packetsPerFrame(GOTTHARD_PACKETS_PER_FRAME),
					frameIndexMask(GOTTHARD_FRAME_INDEX_MASK),
					packetIndexMask(GOTTHARD_PACKET_INDEX_MASK),
					frameIndexOffset(GOTTHARD_FRAME_INDEX_OFFSET),
					acquisitionPeriod(SAMPLE_TIME_IN_NS),
					shortFrame(-1),
					currframenum(0),
					prevframenum(0),
					bufferSize(GOTTHARD_BUFFER_SIZE),
					latestData(NULL),
					guiDataReady(0),
					guiData(NULL),
					guiFileName(NULL),
					nFrameToGui(0),
					fifosize(GOTTHARD_FIFO_SIZE),
					numJobsPerThread(-1),
					mem0(NULL),
					dataCompression(false),
					fifo(NULL),
					fifoFree(NULL),
					buffer(NULL),
					numWriterThreads(1),
					thread_started(0),
					writerthreads_mask(0x0),
					currentWriterThreadIndex(-1),
					totalListeningFrameCount(0),
					running(0),
					filter(NULL),
					startAcquisitionCallBack(NULL),
					pStartAcquisition(NULL),
					acquisitionFinishedCallBack(NULL),
					pAcquisitionFinished(NULL),
					rawDataReadyCallBack(NULL),
					pRawDataReady(NULL){

	maxPacketsPerFile = MAX_FRAMES_PER_FILE * packetsPerFrame;
	//moench variables
	if(myDetectorType == MOENCH){
		fifosize = MOENCH_FIFO_SIZE;
		bufferSize = MOENCH_BUFFER_SIZE;
		packetsPerFrame = MOENCH_PACKETS_PER_FRAME;
		maxPacketsPerFile = MOENCH_MAX_FRAMES_PER_FILE * MOENCH_PACKETS_PER_FRAME;
		frameIndexMask = MOENCH_FRAME_INDEX_MASK;
		frameIndexOffset = MOENCH_FRAME_INDEX_OFFSET;
		packetIndexMask = MOENCH_PACKET_INDEX_MASK;
	}

	//variable initialization
	onePacketSize = bufferSize/packetsPerFrame;
	eth = new char[MAX_STR_LENGTH];
	guiFileName = new char[MAX_STR_LENGTH];
	latestData = new char[bufferSize];
	strcpy(eth,"");
	strcpy(guiFileName,"");
	strcpy(latestData,"");
	strcpy(savefilename,"");
	strcpy(filePath,"");
	strcpy(fileName,"run");


	setupFifoStructure();
	setupFilter();

	pthread_mutex_init(&dataReadyMutex,NULL);
	pthread_mutex_init(&status_mutex,NULL);
	pthread_mutex_init(&progress_mutex,NULL);
	pthread_mutex_init(&write_mutex,NULL);

	if(createThreads() == FAIL){
		cout << "ERROR: Could not create writer threads" << endl;
		exit (-1);
	}

}



slsReceiverFunctionList::~slsReceiverFunctionList(){
	if(udpSocket) 	delete udpSocket;
	if(eth) 		delete [] eth;
	if(latestData) 	delete [] latestData;
	if(guiFileName) delete [] guiFileName;
	if(mem0)		free(mem0);
	if(filter)		delete filter;
	if(fifo)		delete fifo;
	if(fifoFree)	delete fifoFree;
}



void slsReceiverFunctionList::setEthernetInterface(char* c){
	strcpy(eth,c);
}



uint32_t slsReceiverFunctionList::getFrameIndex(){
	if(!packetsCaught)
		frameIndex=0;
	else
		frameIndex = currframenum - startFrameIndex;
	return frameIndex;
}



uint32_t slsReceiverFunctionList::getAcquisitionIndex(){
	if(!totalPacketsCaught)
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



int slsReceiverFunctionList::setEnableFileWrite(int i){
	if(i!=-1)
		enableFileWrite=i;
	return enableFileWrite;
}




void slsReceiverFunctionList::resetTotalFramesCaught(){
	acqStarted = false;
	startAcquisitionIndex = 0;
	totalPacketsCaught = 0;
}



int slsReceiverFunctionList::setShortFrame(int i){
	shortFrame=i;

	if(shortFrame!=-1){
		bufferSize = GOTTHARD_SHORT_BUFFER_SIZE;
		maxPacketsPerFile = SHORT_MAX_FRAMES_PER_FILE * GOTTHARD_SHORT_PACKETS_PER_FRAME;
		packetsPerFrame = GOTTHARD_SHORT_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_SHORT_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_SHORT_FRAME_INDEX_OFFSET;

	}else{
		bufferSize = GOTTHARD_BUFFER_SIZE;
		maxPacketsPerFile = MAX_FRAMES_PER_FILE * GOTTHARD_PACKETS_PER_FRAME;
		packetsPerFrame = GOTTHARD_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_FRAME_INDEX_OFFSET;
	}


	onePacketSize = bufferSize/packetsPerFrame;

	//if the filter is inititalized with the wrong readout
	if(filter->getPacketsPerFrame() != packetsPerFrame)
		setupFilter();

	return shortFrame;
}






int slsReceiverFunctionList::setNFrameToGui(int i){
	if(i>=0){
		nFrameToGui = i;
		setupFifoStructure();
	}
	return nFrameToGui;
}




int64_t slsReceiverFunctionList::setAcquisitionPeriod(int64_t index){
	if(index >= 0){
		if(index != acquisitionPeriod){
			acquisitionPeriod = index;
			setupFifoStructure();
		}
	}
	return acquisitionPeriod;
}

/*
void slsReceiverFunctionList::setupFilter(){
	char *fformat;
	char *tit;
	int runmin;
	int runmax;
	int nbins=1500;
	int hmin=-500;
	int hmax=1000;
	int sign=1;
	double hc=0;
	int xmin=0;
	int xmax = ?;
	int ymin=0;
	int ymax = ?;
	int cmsub=0;

	mdecoder = NULL;
	gdecoder = NULL;
	if(myDetectorType == MOENCH){
		mdecoder=new moench02ModuleData(hc);
		xmax = 160;
		ymax = 160;
	}else{
		gdecoder = new gotthardModuleData(hc,shortFrame!=-1);
		if(shortFrame){
			gdecoder = new gotthardModuleData(hc,true);
			xmax = 1;
			ymax = 256;
		}else{
			gdecoder = new gotthardModuleData(hc,false);
			xmax = 1;
			ymax = 1280;
		}
	}

}
*/
void slsReceiverFunctionList::setupFilter(){
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
	switch(packetsPerFrame){
	case MOENCH_PACKETS_PER_FRAME:
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


		filter = new singlePhotonFilter(x,y, MOENCH_FRAME_INDEX_MASK, MOENCH_PACKET_INDEX_MASK, MOENCH_FRAME_INDEX_OFFSET, 0,
				MOENCH_PACKETS_PER_FRAME, 0,map, mask,fifoFree,MOENCH_BUFFER_SIZE,
				&totalPacketsCaught,&packetsCaught,&currframenum);
		break;

	case GOTTHARD_SHORT_PACKETS_PER_FRAME:
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
		filter = new singlePhotonFilter(x,y,frameIndexMask, GOTTHARD_PACKET_INDEX_MASK, frameIndexOffset, 0,
				GOTTHARD_SHORT_PACKETS_PER_FRAME, 0,map, mask,fifoFree,GOTTHARD_SHORT_BUFFER_SIZE,
				&totalPacketsCaught,&packetsCaught,&currframenum);
		break;
	default:
		x = 1;
		y = (GOTTHARD_DATA_BYTES/GOTTHARD_PACKETS_PER_FRAME);
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


		filter = new singlePhotonFilter(x,y,GOTTHARD_FRAME_INDEX_MASK, GOTTHARD_PACKET_INDEX_MASK, GOTTHARD_FRAME_INDEX_OFFSET,	0,
				GOTTHARD_PACKETS_PER_FRAME, 1,map, mask,fifoFree,GOTTHARD_BUFFER_SIZE,
				&totalPacketsCaught,&packetsCaught,&currframenum);
		break;


	}


	filter->registerCallBackFreeFifo(&(freeFifoBufferCallBack),this);

}




/******************* need to look at exit strategy **************************/
void slsReceiverFunctionList::enableDataCompression(bool enable){
	dataCompression = enable;
	if(filter){
		if(filter->enableCompression(enable) == FAIL)
			exit(-1);
		else{
			createThreads(true);
			if(enable)
				numWriterThreads = MAX_NUM_WRITER_THREADS;
			else
				numWriterThreads = 1;
			createThreads();
		}
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





void slsReceiverFunctionList::copyFrameToGui(char* startbuf){

	//random read when gui not ready
	if((!nFrameToGui) && (!guiData)){
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
		pthread_mutex_unlock(&dataReadyMutex);
	}

	//random read or nth frame read, gui needs data now
	else{
		//nth frame read, block current process if the guireader hasnt read it yet
		if(nFrameToGui)
			sem_wait(&smp);

		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
		//send the first one
		memcpy(latestData,startbuf,bufferSize);
		strcpy(guiFileName,savefilename);
		guiDataReady=1;
		pthread_mutex_unlock(&dataReadyMutex);
	}
}




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

	//if same, return
	if(oldn == numJobsPerThread)
		return;


	//otherwise memory too much if numjobsperthread is at max = 1000
	fifosize = GOTTHARD_FIFO_SIZE;
	if(myDetectorType == MOENCH)
		fifosize = MOENCH_FIFO_SIZE;

	if(fifosize % numJobsPerThread)
		fifosize = (fifosize/numJobsPerThread)+1;
	else
		fifosize = fifosize/numJobsPerThread;


	cout << "Number of Frames per buffer:" << numJobsPerThread << endl;
	cout << "Fifo Size:" << fifosize << endl;

	/*
	//for testing
	 numJobsPerThread = 3; fifosize = 11;
	 */

	//deleting old structure and creating fifo structure
	if(fifoFree){
		while(!fifoFree->isEmpty())
			fifoFree->pop(buffer);
		delete fifoFree;
	}
	if(fifo)	delete fifo;
	if(mem0) 	free(mem0);
	fifoFree 	= new CircularFifo<char>(fifosize);
	fifo 		= new CircularFifo<char>(fifosize);


	//allocate memory
	mem0=(char*)malloc((bufferSize * numJobsPerThread + HEADER_SIZE_NUM_TOT_PACKETS)*fifosize);
	/** shud let the client know about this */
	if (mem0==NULL)
		cout<<"++++++++++++++++++++++ COULD NOT ALLOCATE MEMORY FOR LISTENING !!!!!!!+++++++++++++++++++++" << endl;
	buffer=mem0;
	//push the addresses into freed fifoFree and writingFifoFree
	while (buffer<(mem0+(bufferSize * numJobsPerThread + HEADER_SIZE_NUM_TOT_PACKETS)*(fifosize-1))) {
		fifoFree->push(buffer);
		buffer+=(bufferSize * numJobsPerThread + HEADER_SIZE_NUM_TOT_PACKETS);
	}

	cout << "Fifo structure reconstructed" << endl;
}






int slsReceiverFunctionList::createUDPSocket(){
	if(udpSocket)
		udpSocket->ShutDownSocket();

	//if eth is mistaken with ip address
	if (strchr(eth,'.')!=NULL)
		strcpy(eth,"");

	//if no eth, listen to all
	if(!strlen(eth)){
		cout<<"warning:eth is empty.listening to all"<<endl;

		udpSocket = new genericSocket(server_port,genericSocket::UDP,bufferSize,packetsPerFrame);
	}
	//normal socket
	else{
		cout<<"eth:"<<eth<<endl;
		udpSocket = new genericSocket(server_port,genericSocket::UDP,bufferSize,packetsPerFrame,eth);
	}

	//error
	int iret = udpSocket->getErrorStatus();
	if (iret){
//#ifdef VERBOSE
		cout << "Could not create UDP socket on port " << server_port  << " error:" << iret << endl;
//#endif

		return FAIL;
	}
	return OK;
}





void slsReceiverFunctionList::freeFifoBufferCallBack (char* fbuffer, void *this_pointer){
	((slsReceiverFunctionList*)this_pointer)->freeFifoBuffer(fbuffer);
}



void slsReceiverFunctionList::freeFifoBuffer(char* fbuffer){
	fifoFree->push(fbuffer);
}




int slsReceiverFunctionList::createThreads(bool destroy){
	int i;

	if(!destroy){

		//listening thread
		pthread_mutex_lock(&status_mutex);
		status = IDLE;
		running = 0;
		pthread_mutex_unlock(&(status_mutex));

		sem_init(&listensmp,0,0);
		if(pthread_create(&listening_thread, NULL,startListeningThread, (void*) this)){
			cout << "Could not create listening thread" << endl;
			return FAIL;
		}

		//#ifdef VERBOSE
		cout << "Listening thread created successfully." << endl;
		//#endif


		//start writer threads
		cout << "Creating Writer Threads";
		writerthreads_mask = 0x0;
		currentWriterThreadIndex = -1;

		for(i = 0; i < numWriterThreads; ++i){
			sem_init(&writersmp[i],0,0);
			thread_started = 0;
			currentWriterThreadIndex = i;
			if(pthread_create(&writing_thread[i], NULL,startWritingThread, (void*) this)){
				cout << "Could not create writer thread with index " << i << endl;
				return FAIL;
			}
			while(!thread_started);
			cout << ".";
			cout << flush;
		}
		//#ifdef VERBOSE
		cout << endl << "Writer threads created successfully." << endl;
		//#endif

		//assign priorities
		struct sched_param tcp_param, listen_param, write_param;
		int policy= SCHED_RR;
		bool rights = true;

		tcp_param.sched_priority = 50;
		listen_param.sched_priority = 99;
		write_param.sched_priority = 90;

		if (pthread_setschedparam(listening_thread, policy, &listen_param) == EPERM)
			rights = false;
		for(i = 0; i < numWriterThreads; ++i)
			if(rights)
				if (pthread_setschedparam(writing_thread[i], policy, &write_param) == EPERM){
					rights = false;
					break;
				}
		if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
			rights = false;

		if(!rights)
			cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;


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


	}

	else{
		//cancel threads
		for(i = 0; i < numWriterThreads; ++i){
			if(pthread_cancel(writing_thread[i])!=0)
				cout << "Unable to cancel Thread of index" << i << endl;
			sem_post(&writersmp[i]);
			sem_destroy(&writersmp[i]);
		}
		//semaphore destroy
		sem_post(&listensmp);
		sem_destroy(&listensmp);
		cout << "Threads destroyed" << endl;
	}

	return OK;
}










int slsReceiverFunctionList::setupWriter(){

	//reset writing thread variables
	packetsInFile=0;
	packetsCaught=0;
	frameIndex=0;
	if(sfilefd) sfilefd=NULL;
	guiData = NULL;
	guiDataReady=0;
	strcpy(guiFileName,"");
	cbAction = DO_EVERYTHING;


	//printouts
	cout << "Max Packets Per File:" << maxPacketsPerFile << endl;
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
		return createNewFile();
	else{
		//create file name for gui purposes, and set up acquistion parameters
		sprintf(savefilename, "%s/%s_fxxx_%d.raw", filePath,fileName,fileIndex);
		filter->setupAcquisitionParameters(filePath,fileName,fileIndex);
		// if(enableFileWrite && cbAction > DO_NOTHING)
		// This commented option doesnt exist as we save and do ebverything for data compression
		//create file
		return filter->initTree();
	}
}





int slsReceiverFunctionList::createNewFile(){

	//create file name
	if(frameIndexNeeded==-1)
		sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
	else
		sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,(packetsCaught/packetsPerFrame),fileIndex);


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
			return FAIL;
		}
		//setting buffer
		setvbuf(sfilefd,NULL,_IOFBF,BUF_SIZE);
		//printing packet losses and file names
		if(!packetsCaught)
			cout << savefilename << endl;
		else{
			cout << savefilename
					<< "\tpacket loss "
					<< setw(4)<<fixed << setprecision(4)<< dec <<
					(int)((((currframenum-prevframenum)-(packetsInFile/packetsPerFrame))/(double)(currframenum-prevframenum))*100.000)
					<< "%\tframenum "
					<< dec << currframenum //<< "\t\t p " << prevframenum
					<< "\tindex " << dec << getFrameIndex()
					<< "\tlost " << dec << (((int)(currframenum-prevframenum))-(packetsInFile/packetsPerFrame)) << endl;

		}
	}

	//reset counters for each new file
	if(packetsCaught){
		prevframenum = currframenum;
		packetsInFile = 0;
	}

	return OK;
}





int slsReceiverFunctionList::startReceiver(char message[]){
//#ifdef VERBOSE
	cout << "Starting Receiver" << endl;
//#endif


	//reset listening thread variables
	measurementStarted = false;
	startFrameIndex = 0;
	totalListeningFrameCount = 0;

	//udp socket
	if(createUDPSocket() == FAIL){
		strcpy(message,"Could not create UDP Socket.\n");
		cout << endl << message << endl;
		return FAIL;
	}
	cout << "UDP socket created successfully on port " << server_port << endl;


	if(setupWriter() == FAIL){
		//stop udp socket
		if(udpSocket)
			udpSocket->ShutDownSocket();

		sprintf(message,"Could not create file %s.\n",savefilename);
		return FAIL;
	}

	//initialize semaphore
	sem_init(&smp,0,1);

	//status
	pthread_mutex_lock(&status_mutex);
	status = RUNNING;
	receiver_threads_running = 1;
	running = 1;
	pthread_mutex_unlock(&(status_mutex));


	//start listening /writing
	sem_post(&listensmp);
	for(int i=0; i < numWriterThreads; ++i)
		sem_post(&writersmp[i]);

	cout << "Receiver Started.\nStatus:" << status << endl;

	return OK;
}




int slsReceiverFunctionList::stopReceiver(){
//#ifdef VERBOSE
	cout << "Stopping Receiver" << endl;
//#endif

	if(status == RUNNING)
		startReadout();

	while(status == TRANSMITTING)
		usleep(5000);

	//semaphore destroy
	sem_post(&smp);
	sem_destroy(&smp);

	//change status
	pthread_mutex_lock(&status_mutex);
	receiver_threads_running = 0;
	status = IDLE;
	pthread_mutex_unlock(&(status_mutex));

	cout << "Receiver Stopped.\nStatus:" << status << endl;
	return OK;
}





void slsReceiverFunctionList::startReadout(){
	//wait so that all packets which take time has arrived
	usleep(50000);

	pthread_mutex_lock(&status_mutex);
	status = TRANSMITTING;
	pthread_mutex_unlock(&status_mutex);
	cout << "Status: Transmitting" << endl;

	//kill udp socket to tell the listening thread to push last packet
	if(udpSocket)
		udpSocket->ShutDownSocket();

}



void* slsReceiverFunctionList::startListeningThread(void* this_pointer){
	((slsReceiverFunctionList*)this_pointer)->startListening();

	return this_pointer;
}



void* slsReceiverFunctionList::startWritingThread(void* this_pointer){
	((slsReceiverFunctionList*)this_pointer)->startWriting();
	return this_pointer;
}






int slsReceiverFunctionList::startListening(){
#ifdef VERYVERBOSE
	cout << "In startListening()" << endl;
#endif

	int lastpacketoffset, expected, rc, packetcount, maxBufferSize, carryonBufferSize;
	int lastframeheader;// for moench to check for all the packets in last frame
	char* tempchar = NULL;

	while(1){
		//variables that need to be checked/set before each acquisition
		carryonBufferSize = 0;
		maxBufferSize = packetsPerFrame * numJobsPerThread * onePacketSize;
		if(tempchar) {delete [] tempchar;tempchar = NULL;}
		tempchar = new char[onePacketSize * (packetsPerFrame - 1)]; //gotthard: 1packet size, moench:39 packet size


		while(running){

			//pop
			fifoFree->pop(buffer);
#ifdef VERYDEBUG
			cout << "*** popped from fifo free" << (void*)buffer << endl;
#endif

			//receive
			if(!carryonBufferSize){
				rc = udpSocket->ReceiveDataOnly(buffer + HEADER_SIZE_NUM_TOT_PACKETS, maxBufferSize);
				expected = maxBufferSize;
			}else{
#ifdef VERYDEBUG
				cout << "***carry on buffer" << carryonBufferSize << endl;
#endif
				//if there is a packet from previous buffer, copy it and listen to n less frame
				memcpy(buffer + HEADER_SIZE_NUM_TOT_PACKETS, tempchar, carryonBufferSize);
				rc = udpSocket->ReceiveDataOnly((buffer + HEADER_SIZE_NUM_TOT_PACKETS + carryonBufferSize),maxBufferSize - carryonBufferSize);
				expected = maxBufferSize - carryonBufferSize;
			}

#ifdef VERYDEBUG
			cout << "*** rc:" << rc << endl;
			cout << "*** expected:" << expected << endl;
#endif
			//start indices
			//start of scan
			if((!measurementStarted) && (rc > 0)){
				//gotthard has +1 for frame number
				if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
					startFrameIndex = (((((uint32_t)(*((uint32_t*)(buffer + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
							& (frameIndexMask)) >> frameIndexOffset);
				else
					startFrameIndex = ((((uint32_t)(*((uint32_t*)(buffer+HEADER_SIZE_NUM_TOT_PACKETS))))
							& (frameIndexMask)) >> frameIndexOffset);
				cout<<"startFrameIndex:"<<startFrameIndex<<endl;
				prevframenum=startFrameIndex;
				measurementStarted = true;
				//start of acquisition
				if(!acqStarted){
					startAcquisitionIndex=startFrameIndex;
					currframenum = startAcquisitionIndex;
					acqStarted = true;
					cout<<"startAcquisitionIndex:"<<startAcquisitionIndex<<endl;
				}
			}


			//problem in receiving or end of acquisition
			if((rc < expected)||(rc <= 0)){
#ifdef VERYVERBOSE
				cerr << "recvfrom() failed:"<<endl;
#endif
				if(status != TRANSMITTING){
					cout<<"*** shoule never be here********************************"<<endl;/**/
					fifoFree->push(buffer);
					exit(-1);
					continue;
				}
				//push the last buffer into fifo
				if(rc > 0){
					packetcount = (rc/onePacketSize);
#ifdef VERYDEBUG
					cout << "*** last packetcount:" << packetcount << endl;
#endif
					(*((uint16_t*)(buffer))) = packetcount;
					totalListeningFrameCount += packetcount;
					while(!fifo->push(buffer));
#ifdef VERYDEBUG
					cout << "*** last lbuf1:" << (void*)buffer << endl;
#endif
				}
				//push dummy buffer
				fifoFree->pop(buffer);
				(*((uint16_t*)(buffer))) = 0xFFFF;
				while(!fifo->push(buffer));
#ifdef VERYDEBUG
				cout << "pushed in dummy buffer:" << (void*)buffer << endl;
#endif
				cout << "Total count listened to " << totalListeningFrameCount/packetsPerFrame << endl;
				pthread_mutex_lock(&status_mutex);
				running = 0;
				pthread_mutex_unlock(&(status_mutex));
				break;
			}


			//reset
			packetcount = packetsPerFrame * numJobsPerThread;
			carryonBufferSize = 0;

			//check if last packet valid and calculate packet count
			switch(myDetectorType){



			case MOENCH:
				lastpacketoffset = (((numJobsPerThread * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef VERYDEBUG
				cout <<"first packet:"<< ((((uint32_t)(*((uint32_t*)(buffer+HEADER_SIZE_NUM_TOT_PACKETS))))) & (packetIndexMask)) << endl;
				cout <<"first header:"<< (((((uint32_t)(*((uint32_t*)(buffer+HEADER_SIZE_NUM_TOT_PACKETS))))) & (frameIndexMask)) >> frameIndexOffset) << endl;
				cout << "last packet offset:" << lastpacketoffset << endl;
				cout <<"last packet:"<< ((((uint32_t)(*((uint32_t*)(buffer+lastpacketoffset))))) & (packetIndexMask)) << endl;
				cout <<"last header:"<< (((((uint32_t)(*((uint32_t*)(buffer+lastpacketoffset))))) & (frameIndexMask)) >> frameIndexOffset) << endl;
#endif
				//moench last packet value is 0
				if( ((((uint32_t)(*((uint32_t*)(buffer+lastpacketoffset))))) & (packetIndexMask))){
					lastframeheader = ((((uint32_t)(*((uint32_t*)(buffer+lastpacketoffset))))) & (frameIndexMask)) >> frameIndexOffset;
					carryonBufferSize += onePacketSize;
					lastpacketoffset -= onePacketSize;
					--packetcount;
					while (lastframeheader == (((((uint32_t)(*((uint32_t*)(buffer+lastpacketoffset))))) & (frameIndexMask)) >> frameIndexOffset)){
						carryonBufferSize += onePacketSize;
						lastpacketoffset -= onePacketSize;
						--packetcount;
					}
					memcpy(tempchar, buffer+(lastpacketoffset+onePacketSize), carryonBufferSize);
#ifdef VERYDEBUG
					cout << "tempchar header:" << (((((uint32_t)(*((uint32_t*)(tempchar)))))
							& (frameIndexMask)) >> frameIndexOffset) << endl;
			cout << "header:" << (((((uint32_t)(*((uint32_t*)(buffer + HEADER_SIZE_NUM_TOT_PACKETS)))))
										& (frameIndexMask)) >> frameIndexOffset) << endl;
#endif
			}
				break;



			default:
				if(shortFrame == -1){
					lastpacketoffset = (((numJobsPerThread * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef VERYDEBUG
					cout << "last packet offset:" << lastpacketoffset << endl;
#endif

					if((packetsPerFrame -1) != ((((uint32_t)(*((uint32_t*)(buffer+lastpacketoffset))))+1) & (packetIndexMask))){
						memcpy(tempchar,buffer+lastpacketoffset, onePacketSize);
#ifdef VERYDEBUG
						cout << "tempchar header:" << (((((uint32_t)(*((uint32_t*)(tempchar))))+1)
								& (frameIndexMask)) >> frameIndexOffset) << endl;
#endif
						carryonBufferSize = onePacketSize;
						--packetcount;
					}
				}
#ifdef VERYDEBUG
			cout << "header:" << (((((uint32_t)(*((uint32_t*)(buffer + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
										& (frameIndexMask)) >> frameIndexOffset) << endl;
#endif
				break;




			}
#ifdef VERYDEBUG
			cout << "*** packetcount:" << packetcount << endl;
#endif
			//write packet count and push
			(*((uint16_t*)(buffer))) = packetcount;
			totalListeningFrameCount += packetcount;
			while(!fifo->push(buffer));
#ifdef VERYDEBUG
			cout << "*** pushed into listening fifo" << endl;
#endif
		}
		sem_wait(&listensmp);
	}

	return OK;
}















int slsReceiverFunctionList::startWriting(){
	int ithread = currentWriterThreadIndex;
#ifdef VERYVERBOSE
	cout << ithread << "In startWriting()" <<endl;
#endif

	thread_started = 1;

	int numpackets,tempframenum;
	char* wbuf;

	while(1){


		while(receiver_threads_running){


			//pop
			fifo->pop(wbuf);
			numpackets =	(uint16_t)(*((uint16_t*)wbuf));
#ifdef VERYDEBUG
			cout << "numpackets:" << hex << numpackets << endl;
			cout << ithread << "*** popped from fifo " << numpackets << endl;
#endif




			//last dummy packet
			if(numpackets == 0xFFFF){
#ifdef VERYDEBUG
				cout << "popped last dummy frame:" << (void*)wbuf << endl;
#endif
				//data compression, check if jobs done
				if(dataCompression){
					while(!filter->checkIfJobsDone())
						usleep(50000);
				}
				//free fifo
				while(!fifoFree->push(wbuf));
#ifdef VERYDEBUG
				cout << "fifo freed:" << (void*)wbuf << endl;
#endif
				//update status
				pthread_mutex_lock(&status_mutex);
				status = RUN_FINISHED;
				pthread_mutex_unlock(&(status_mutex));
				cout << "Status: Run Finished" << endl;
				//close file
				if(sfilefd){
#ifdef VERBOSE
					cout << "sfield:" << (int)sfilefd << endl;
#endif
					fclose(sfilefd);
					sfilefd = NULL;
				}
				//report
				cout << "Total Packets Caught:" << dec << totalPacketsCaught << endl;
				cout << "Total Frames Caught:"<< dec << (totalPacketsCaught/packetsPerFrame) << endl;
				//acquisition end
				if (acquisitionFinishedCallBack)
					acquisitionFinishedCallBack((totalPacketsCaught/packetsPerFrame), pAcquisitionFinished);
				continue;
			}




			//for progress
			if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
				tempframenum = (((((uint32_t)(*((uint32_t*)(wbuf + HEADER_SIZE_NUM_TOT_PACKETS))))+1)& (frameIndexMask)) >> frameIndexOffset);
			else
				tempframenum = ((((uint32_t)(*((uint32_t*)(wbuf + HEADER_SIZE_NUM_TOT_PACKETS))))& (frameIndexMask)) >> frameIndexOffset);

			if(numWriterThreads == 1)
				currframenum = tempframenum;
			else{
				pthread_mutex_lock(&progress_mutex);
				if(tempframenum > currframenum)
					currframenum = tempframenum;
				pthread_mutex_unlock(&progress_mutex);
			}
#ifdef VERYDEBUG
	cout << ithread << " tempframenum:" << dec << tempframenum << " curframenum:" << currframenum << endl;
#endif


			//without datacompression: write datacall back, or write data, free fifo
			if(!dataCompression){
				if (cbAction < DO_EVERYTHING)
					rawDataReadyCallBack(currframenum, wbuf, numpackets * onePacketSize, sfilefd, guiData,pRawDataReady);
				else if (numpackets > 0){
					if(numWriterThreads >1)
						pthread_mutex_lock(&progress_mutex);
					writeToFile_withoutCompression(wbuf, numpackets);
					if(numWriterThreads >1)
						pthread_mutex_unlock(&progress_mutex);
				}
				while(!fifoFree->push(wbuf));
#ifdef VERYVERBOSE
				cout<<"buf freed:"<<(void*)wbuf<<endl;
#endif
			}


			//data compression
			else{
				;
			}


			//copy to gui
			copyFrameToGui(wbuf + HEADER_SIZE_NUM_TOT_PACKETS);
		}
		sem_wait(&writersmp[ithread]);
	}


	return OK;
}




void slsReceiverFunctionList::writeToFile_withoutCompression(char* buf,int numpackets){
	int packetsToSave, offset,tempframenum,lastpacket;

	//file write
	if((enableFileWrite) && (sfilefd)){

		offset = HEADER_SIZE_NUM_TOT_PACKETS;
		while(numpackets > 0){
			//for progress and packet loss calculation(new files)
			if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
				tempframenum = (((((uint32_t)(*((uint32_t*)(buf + HEADER_SIZE_NUM_TOT_PACKETS))))+1)& (frameIndexMask)) >> frameIndexOffset);
			else
				tempframenum = ((((uint32_t)(*((uint32_t*)(buf + HEADER_SIZE_NUM_TOT_PACKETS))))& (frameIndexMask)) >> frameIndexOffset);

			if(numWriterThreads == 1)
				currframenum = tempframenum;
			else{
				if(tempframenum > currframenum)
					currframenum = tempframenum;
			}
#ifdef VERYDEBUG
			cout << "tempframenum:" << dec << tempframenum << " curframenum:" << currframenum << endl;
#endif

			//to create new file when max reached
			packetsToSave = maxPacketsPerFile - packetsInFile;
			if(packetsToSave > numpackets)
				packetsToSave = numpackets;

			fwrite(buf+offset, 1, packetsToSave * onePacketSize, sfilefd);
			packetsInFile += packetsToSave;
			packetsCaught += packetsToSave;
			totalPacketsCaught += packetsToSave;

			//new file
			if(packetsInFile >= maxPacketsPerFile){
				lastpacket = (((packetsToSave - 1) * onePacketSize) + offset);

				//for packet loss
				if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
					tempframenum = (((((uint32_t)(*((uint32_t*)(buf + lastpacket))))+1)& (frameIndexMask)) >> frameIndexOffset);
				else
					tempframenum = ((((uint32_t)(*((uint32_t*)(buf + lastpacket))))& (frameIndexMask)) >> frameIndexOffset);

				if(numWriterThreads == 1)
					currframenum = tempframenum;
				else{
					if(tempframenum > currframenum)
						currframenum = tempframenum;
				}
#ifdef VERYDEBUG
				cout << "tempframenum:" << dec << tempframenum << " curframenum:" << currframenum << endl;
#endif

				createNewFile();
			}

			offset += (packetsToSave * onePacketSize);
			numpackets -= packetsToSave;
		}
	}
	//no file write
	else{
		packetsInFile += numpackets;
		packetsCaught += numpackets;
		totalPacketsCaught += numpackets;
	}

}

#endif
