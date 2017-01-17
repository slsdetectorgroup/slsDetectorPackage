  /********************************************//**
 * @file singlePhotonFilter.cpp
 * @short single photon filter using trees
 ***********************************************/
#include "singlePhotonFilter.h"
#include <errno.h>


#define FILE_BUF_SIZE        (16*1024*1024) //16mb
#define HEADER_SIZE_NUM_FRAMES	2
#define HEADER_SIZE_NUM_PACKETS	1


singlePhotonFilter::singlePhotonFilter(int nx, int ny,
		int fmask, int pmask, int foffset, int poffset, int pperf, int iValue,
		int16_t *m, int16_t *s, CircularFifo<char>* f, int d, int* tfcaught, int* fcaught,uint32_t* cframenum):
#ifdef MYROOT1
										myTree(NULL),
#else

										nHitsPerFile(0),
										nTotalHits(0),
#endif
										myFile(NULL),
										nHitsPerFrame(0),
										nChannelsX(nx),
										nChannelsY(ny),
										nClusterX(CLUSTER_SIZE),
										nClusterY(CLUSTER_SIZE),
										map(m),
										dataSize(d),
										mask(s),
										nped(DEFAULT_NUM_PEDESTAL),
										nsigma(DEFAULT_SIGMA),
										nbackground(DEFAULT_BACKGROUND),
										outcorr(DEFAULT_CORRECTION),
										fnum(0),
										pnum(0),
										ptot(0),
										f0(0),
										frame_index_mask(fmask),
										packet_index_mask(pmask),
										frame_index_offset(foffset),
										packet_index_offset(poffset),
										packets_per_frame(pperf),
										incrementValue(iValue),
										firstTime(true),
										ret(0),
										pIndex(0),
										fIndex(0),
										thread_started(0),
										threads_mask(0x0),
										currentThread(-1),
										thisThreadIndex(-1),
										fileIndex(0),
										fifo(f),
										totalFramesCaught(tfcaught),
										framesCaught(fcaught),
										currentframenum(cframenum),
										freeFifoCallBack(NULL),	
										pFreeFifo(NULL){
	//cluster
  if (nChannelsX)
    nClusterX = 1;	
  
#ifndef MYROOT1
  //photonHitList=(single_photon_hit**) (new int*[nChannelsX*nChannelsY/(nClusterX*nClusterY)*1000]);
  photonHitList=new single_photon_hit*[nChannelsX*nChannelsY/(nClusterX*nClusterY)*1000];
  for (int ii=0; ii<nChannelsX*nChannelsY/(nClusterX*nClusterY)*1000; ii++)
    photonHitList[ii]=new single_photon_hit(nClusterX, nClusterY);
  cout << nClusterX << " " << nClusterY << " " << nClusterX*nClusterY << endl;
#endif

	sqrtCluster = sqrt(nClusterX*nClusterY);
	deltaX = nClusterX/2;// 0 or 1
	clusterCenterPixel = (deltaX * nClusterY) + 1;


	stat = new movingStat[nChannelsX*nChannelsY];
	nHitStat = new movingStat();

	myPhotonHit 		= new single_photon_hit(nClusterX,nClusterY);
	//	myPhotonHit->data 	= new double[nClusterX*nClusterY];
	myPhotonHit->x 		= 0;
	myPhotonHit->y 		= 0;
	myPhotonHit->rms 	= 0;
	myPhotonHit->ped 	= 0;
	myPhotonHit->iframe = -1;

	for(int i=0;i < NUM_THREADS; i++){
		/*smp[i] = NULL;*/
		mem0[i]=NULL;
	}
	numFramesAlloted = new int[NUM_THREADS];

	strcpy(savefilename,"");
	strcpy(filePath,"");
	strcpy(fileName,"run");

	pthread_mutex_init(&write_mutex,NULL);
	pthread_mutex_init(&running_mutex,NULL);
	pthread_mutex_init(&frnum_mutex,NULL);



}


singlePhotonFilter::~singlePhotonFilter(){
	enableCompression(false);
	if(numFramesAlloted) delete [] numFramesAlloted;
	writeToFile();
	closeFile();
	if(myFile) delete myFile;
	if(mask) delete mask;
	if(stat) delete stat;
	if(nHitStat) delete nHitStat;
	/*if(smp) delete []smp;*/
	if(mem0) delete [] mem0;
	if(fifo) delete fifo;
}






int singlePhotonFilter::enableCompression(bool enable){
//#ifdef VERBOSE
	cout << "Compression set to " <<  enable;
#ifdef MYROOT1
	cout << " with root" << endl;
#else
	cout << " without root" << endl;
#endif
//#endif
	if(enable){
		threads_mask = 0x0;
		currentThread = -1;

		for(int i=0; i<NUM_THREADS; ++i){
			//initialize semaphore
			sem_init(&smp[i],0,0);
			//create threads
			thread_started = 0;
			thisThreadIndex = i;
			if(pthread_create(&find_hits_thread[i], NULL,createThreads, (void*) this)){
				cout << "Could not create thread with index " << i << endl;
				return FAIL;
			}
			while(!thread_started);
		}
	}else{
		if(thread_started){
		for(int i=0; i<NUM_THREADS; ++i){
			//cancel threads
				while(pthread_cancel(find_hits_thread[i])!=0)
					cout << "Unable to cancel Thread " << i << endl;/*pthread_join(find_hits_thread[i],NULL);*/
				pthread_mutex_lock(&write_mutex);
				closeFile();
				pthread_mutex_unlock(&write_mutex);
				//semaphore destroy
				sem_post(&smp[i]);
				sem_destroy(&smp[i]);
			}
		}
	}
	return OK;
}


void* singlePhotonFilter::createThreads(void *this_pointer){
	((singlePhotonFilter*)this_pointer)->findHits();
	return this_pointer;
}



int singlePhotonFilter::initTree(){
#ifdef MYROOT1
	writeToFile();
	closeFile();
	sprintf(savefilename, "%s/%s_%d_.root", filePath,fileName,fileIndex);

	//file
	myFile = new TFile(savefilename, "RECREATE"); /** later  return error if it exists */
	cout<<"File created: "<<savefilename<<endl;
	//tree
	char c1[10],c2[10],cdata[100];
	sprintf(c1,"%d",nClusterX);
	sprintf(c2,"%d",nClusterY);
	sprintf(cdata,"data[%s][%s]/D",c1,c2);


	sprintf(savefilename, "%s_%d_", fileName,fileIndex);
	myTree = new TTree(savefilename, savefilename);
	myTree->Branch("iframe",&myPhotonHit->iframe,"iframe/I");
	myTree->Branch("x",&myPhotonHit->x,"x/I");
	myTree->Branch("y",&myPhotonHit->y,"y/I");
	myTree->Branch("data",myPhotonHit->data,cdata);
	myTree->Branch("pedestal",&myPhotonHit->ped,"pedestal/D");
	myTree->Branch("rms",&myPhotonHit->rms,"rms/D");
#else

	writeToFile();
	closeFile();
	sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,nTotalHits,fileIndex);
	myFile = fopen(savefilename, "w");
	setvbuf(myFile,NULL,_IOFBF,FILE_BUF_SIZE);
	cout<<"File created: "<<savefilename<<endl;
	nHitsPerFile = 0;
#endif
	return OK;
}



int singlePhotonFilter::writeToFile(){
	if(nHitsPerFrame){
#ifdef MYROOT1
	if((myTree) && (myFile)){
		myTree->Write();
		nHitsPerFrame = 0;
		return OK;
	}else
		cout << "ERROR: Could not write to " << nHitsPerFrame << " hits to file as file or tree doesnt exist" << endl;
#else
		if(myFile){
		  int ii;
			/*cout<<"writing "<< nHitsPerFrame << " hits to file" << endl;*/
		  for (ii=0; ii<nHitsPerFrame; ii++) {
		    photonHitList[ii]->write(myFile);
		    // delete  photonHitList[ii];
		  }
		  // delete  photonHitList[ii];
		  // photonHitList[0]=new single_photon_hit(nClusterX,nClusterY);

		  //			fwrite((void*)(photonHitList), 1, sizeof(single_photon_hit)*nHitsPerFrame, myFile);
			/*framesInFile += nHitsPerFrame;*/
		  nHitsPerFrame = 0;
			//cout<<"Exiting writeToFile"<<endl;
		  return OK;
		}else
			cout << "ERROR: Could not write to " << nHitsPerFrame <<" hits to file as file doesnt exist" << endl;

#endif
	}
	return FAIL;
}



int singlePhotonFilter::closeFile(){
#ifdef MYROOT1
	if(myTree){
		if (myFile){
			myFile = myTree->GetCurrentFile();
			myFile->Close();
			myFile = NULL;//delete myFile;
		}
		myTree = NULL;//delete myTree;
	}
#else
	if(myFile)
		fclose(myFile);
	myFile = NULL;
#endif
	return OK;
}





void singlePhotonFilter::setupAcquisitionParameters(char *outfpath, char* outfname, int outfIndex){
	fileIndex = outfIndex;
	strcpy(filePath,outfpath);
	strcpy(fileName,outfname);

	fnum = 0; pnum = 0; ptot = 0; f0 = 0; firstTime = true; currentThread = -1;
	*framesCaught = 0;
	*currentframenum = 0;

	//initialize
	for (int ir=0; ir<nChannelsX; ir++){
		for (int ic=0; ic<nChannelsY; ic++){
			stat[ir*nChannelsY+ic].Clear();
			stat[ir*nChannelsY+ic].SetN(nbackground);
		}
	}

	nHitStat->Clear();
	nHitStat->SetN(nbackground);
#ifndef MYROOT1
	nTotalHits = 0;
#endif
}


/*
	rets
case 0:  waiting for next packet of new frame
case 1:  finished with full frame,
	     start new frame
case -1: last packet of current frame,
		 invalidate remaining packets,
		 start new frame
case -2: first packet of new frame,
		 invalidate remaining packets,
		 check buffer needs to be pushed,
		 start new frame with the current packet,
		 then ret = 0
case -3: last packet of new frame,
		 invalidate remaining packets,
		 check buffer needs to be pushed,
		 start new frame with current packet,
		 then ret = -1 (invalidate remaining packets and start a new frame)
 */
int singlePhotonFilter::verifyFrame(char *inData){
	ret = 0;
	pIndex = 0;
	fIndex = 0;
	fIndex = (((uint32_t)(*((uint32_t*)inData)))& frame_index_mask) >> frame_index_offset;
	pIndex = (((uint32_t)(*((uint32_t*)inData)))& packet_index_mask) >> packet_index_offset;

	//check validity of packet index
	if ((pIndex < 0) && (pIndex >= packets_per_frame)){
		cout << "cannot decode packet index:" << pIndex << endl;
		//its already dealt with cuz this frame will be discarded in the end
	}
	pIndex += incrementValue;

	//for moench, put first packet last
	if (pIndex == 0)
		pIndex = packets_per_frame;
//#ifdef VERYVERBOSE
	cout<<"fi:"<<fIndex<< " pi:"<< pIndex << endl;
//#endif
	//firsttime
	if (firstTime){
		firstTime = false;
		f0 = fIndex;
		fnum = fIndex;
		pnum = pIndex - 1; //should b 0 at first
		ptot = 0;
	}

	//if it is not matching withthe frame number
	if (fIndex != fnum){
		cout << "**Frame number doesnt match:Missing Packet! " << fnum << " "
				"Expected f " << fnum << " p " << pnum + 1 << " received f " << fIndex << " p " << pIndex << endl;

		if (ptot == 0) {
			if (pIndex == 1)//so that its not moved to next line.
				ret = 0;
			else
				ret = -1; //moved to next line
		}
		else
			ret = -2;//so remember and moved to next line and copy

		if ((pnum+1 == packets_per_frame) && (pIndex == packets_per_frame)) //so remember and moved to next line and copy and again move to next line
			ret = -3;

		fnum = fIndex;
		pnum = pIndex;
		ptot = 1;
  //ret = -2; dont return here.. if is the end of packets, uve toreturn -1 so that remaining ones are FFFFd and moved to new line
	}

	//if missing a packet, discard
	else if (pIndex != pnum + 1){/**else */
		cout << "**packet number doesnt match:Missing Packet! " << fnum << " "
				"Expected f" << fnum << " p " << pnum + 1 << " received f " << fnum << " p " << pIndex << endl;
		pnum = pIndex;
		ptot++;
	}
	//no missing packet
	else{
		pnum++;
		ptot++;
	}


	//if its the last index
	if (pIndex == packets_per_frame){
		//got all packets
		if (ptot == packets_per_frame){
			/*myPhotonHit.iframe = fnum - f0;*/
			fnum = fIndex + 1;
			ptot = 0;
			pnum = 0;
			ret = 1;
		}else{
		/*	cout << "* Some packets have been missed!*************************************" << fnum << " " << pnum<<" " << ptot<<endl;*/
			ptot = 0;
			pnum = 0;
			fnum = fIndex + 1;
			if(!ret)
				ret = -1;
		}
	}

	//index not 40, but total  is 40.. strange
	else if (ptot == packets_per_frame){
		cout << "***** Some packets have been missed! Shouldnt be here " << fnum << " " << pnum<< endl;
		ptot = 0;
		pnum = pIndex - 1;
		fnum = fIndex;
		ret = -1;
	}

	return ret;
}


void singlePhotonFilter::findHits(){
	int ir,ic,r,c,i;
	int currentIndex;
	int pixelIndex;
	int clusterIndex;
	//	single_photon_hit *hit;

	double *clusterData;//[nClusterX*nClusterY];//	= hit.data;
	double sigmarms;
	double clusterrms;
	double clusterped;
	uint32_t clusteriframe = 0;
	int dum;
	double tot; // total value of pixel
	char* isData;
	char* freeData;
	int16_t* myData;
	int index = thisThreadIndex;



	//thread created
	pthread_mutex_lock(&running_mutex);
	thread_started = 1;
	pthread_mutex_unlock(&running_mutex);


	while(1){


		//wait for job
		while((dum = sem_wait(&smp[index]))!=0)
			cout<<"semwait:["<<index<<"]:"<<dum<<endl;
		//proceed to get details for job
		if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL)!=0)
			cout << "Could not set Thread " << index <<" cancel state to be disabled" << endl;

		isData = mem0[index];
		freeData = isData;

		pthread_mutex_lock(&running_mutex);
		threads_mask|=(1<<index);
		pthread_mutex_unlock(&running_mutex);

		//wait for acknowledgement to start
		while((dum = sem_wait(&smp[index]))!=0)
			cout<<"got data semwait:["<<index<<"]:"<<dum<<endl;

		isData += HEADER_SIZE_NUM_FRAMES;
/*cout<<"currentframenum:"<<(*currentframenum)<<endl;*/
		//for all the frames in one buffer
		for (i=0; i < numFramesAlloted[index]; ++i){

			//ignore frames which have less than 2 packets
			if((uint8_t)(*((uint8_t*)isData)) == packets_per_frame){

				(*framesCaught)++;
				(*totalFramesCaught)++;


				isData += HEADER_SIZE_NUM_PACKETS;
				clusteriframe = (((uint32_t)(*((uint32_t*)(isData)))& frame_index_mask) >>frame_index_offset);
				//progress
				if((clusteriframe + PROGRESS_INCREMENT) > *currentframenum){
					pthread_mutex_lock(&frnum_mutex);
					*currentframenum = clusteriframe;
					pthread_mutex_unlock(&frnum_mutex);
					/*cout<<"currentframenum:"<<dec<<(*currentframenum)<<endl;*/
				}

#ifdef VERYVERBOSE
				cout << "scurrframnum:" << clusteriframe << endl;
#endif
				clusteriframe -= f0;
				myData = (int16_t*)isData;
			       
			
				clusterData=photonHitList[nHitsPerFrame]->data;

				//for each pixel
				for (ir=0; ir<nChannelsX; ++ir){
					for (ic=0; ic<nChannelsY;++ic){

						currentIndex = (ir * nChannelsY) + ic;

						//validate mapping
						if ((map[currentIndex] < 0) || (map[currentIndex] >= (dataSize))){
							cout << "Bad Channel Mapping index: " << map[currentIndex] << endl;
							continue;
						}

						//if frame within pedestal number
						if (clusteriframe < nped){
							stat[currentIndex].Calc((double)(mask[currentIndex] ^ myData[map[currentIndex]]));
							// frame outside pedestal number
						}else{

							dum = 1;
							tot = 0;
							clusterrms = stat[currentIndex].StandardDeviation();//-1
							clusterped = stat[currentIndex].Mean();//0
							sigmarms = clusterrms * nsigma;


							clusterData[clusterCenterPixel] = ((double)(mask[currentIndex] ^ myData[map[currentIndex]])) - clusterped;
							for (r=-deltaX; r <= deltaX; ++r ){
								if (((ir+r) < 0) || ((ir+r) >= nChannelsX))
									continue;
								for(c=-1; c <= 1; ++c){
									if (((ic+c) < 0) || ((ic+c) >= nChannelsY))
										continue;


									pixelIndex = currentIndex + (r*nChannelsY+c);

									if ((map[pixelIndex] < 0) || (map[pixelIndex] >= dataSize)){
										cout << "Bad Channel Mapping index: " << map[pixelIndex] << endl;
										continue;
									}

									clusterIndex = pixelIndex-(currentIndex - deltaX * nChannelsY - 1);
									clusterData[clusterIndex] = ((double)(mask[pixelIndex] ^ myData[map[pixelIndex]])) - stat[pixelIndex].Mean();
									tot += clusterData[clusterIndex];
									//discard negative events
									if (clusterData[clusterIndex] > clusterData[clusterCenterPixel])
										dum = 2;

								}

							}


							if (tot < sqrtCluster * sigmarms)
								dum = 0;
							//discard events (for pedestal) where sum of the neighbours is too large.
							if (clusterData[clusterCenterPixel] < sigmarms && dum != 0)
								dum = 3;
							//Appriximated running average
							if (clusterData[clusterCenterPixel] > -sigmarms &&
									clusterData[clusterCenterPixel] < sigmarms &&
									dum == 0){
								stat[currentIndex].Calc((double)(mask[currentIndex]^myData[map[currentIndex]]));
							}
							// this is an event and we are in the center
							else if (dum == 1){
								pthread_mutex_lock(&write_mutex);
#ifdef MYROOT1
								myTree->Fill();
#else
								for (int ix=0; ix<nClusterX*nClusterY; ix++)
								photonHitList[nHitsPerFrame]->data[ix] = clusterData[ix];

								photonHitList[nHitsPerFrame]->x = ic;
								photonHitList[nHitsPerFrame]->y = ir;
								photonHitList[nHitsPerFrame]->rms = clusterrms;
								photonHitList[nHitsPerFrame]->ped = clusterped;
								photonHitList[nHitsPerFrame]->iframe = clusteriframe;
								//hit.write(myFile);
								
								nHitsPerFrame++;

								//	cout << nHitsPerFrame << " " << nChannelsX*nChannelsY/(nClusterX*nClusterY)*1000 << endl;

								//	photonHitList[nHitsPerFrame]=new single_photon_hit(nClusterX,nClusterY);
								//	cout << "done" << endl;
								nHitsPerFile++;
								nTotalHits++;
								if(nHitsPerFile >= MAX_HITS_PER_FILE-1)
									initTree();
#endif
								pthread_mutex_unlock(&write_mutex);
							}

						}
					}
				}
			}else{
				//cout<< "did no receiver fulll frame"<<endl;
				isData += HEADER_SIZE_NUM_PACKETS;
			}
			//calulate the average hits per frame
			nHitStat->Calc((double)nHitsPerFrame);
			//write for each frame, not packet

			pthread_mutex_lock(&write_mutex);
			cout << "write to file "  << nHitsPerFrame << endl;
			writeToFile();
			pthread_mutex_unlock(&write_mutex);

			//increment offset
			isData += dataSize;

			/*
			if ((clusteriframe%1000 == 0) && (clusteriframe != 0) ){
				cout << dec << "Frame: " << clusteriframe << " Hit Avg over last frames: " <<
						nHitStat->Mean() <<  "  .. "<<nHitStat->StandardDeviation() << endl;
				cout<<"writing "<< nHitsPerFrame << " hits to file" << endl;
			}
			 */
		}

		pthread_mutex_lock(&write_mutex);
		if(freeFifoCallBack)
			freeFifoCallBack(freeData,pFreeFifo);
		//fifo->push(freeData);//fifo->push(isData);
		pthread_mutex_unlock(&write_mutex);

		//thread not running
		pthread_mutex_lock(&running_mutex);
		threads_mask^=(1<<index);
		pthread_mutex_unlock(&running_mutex);

		if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)!=0)
			cout << "Could not set Thread " << index <<" cancel state to be enabled" << endl;
	}



	//	delete [] clusterData;
}




void singlePhotonFilter::assignJobsForThread(char *theData, int numThisData){
	//cout << "1 Entering assignJobsForThread" << endl;
	while(1){
		++currentThread;
		if(currentThread == NUM_THREADS)
			currentThread=0;
		if(!((1<<currentThread)&threads_mask)){
			mem0[currentThread] = theData;
			/*cout<<"thedata:"<<((theData-listmem0)/4096)<<" numFramesAlloted:"<<numThisData<<endl;
			if(((theData-listmem0)/4096)+numThisData>=25000) {
				cout<<"*****************problem: "<<((theData-listmem0)/4096)<<" :"<<numThisData<<endl;
			}*/
			numFramesAlloted[currentThread] = numThisData;
			sem_post(&smp[currentThread]);
			while(!((1<<currentThread)&threads_mask));
				//usleep(10000);
			sem_post(&smp[currentThread]);
			break;
		}
	}
	//cout << "4 Exiting assignJobsForThread" << endl;
}


int singlePhotonFilter::checkIfJobsDone(){
	//cout<<"Checking if jobs are done"<<endl;
	if(threads_mask){
		//cout<<"Not done!"<<threads_mask<<endl;
		return 0;
	}
	pthread_mutex_lock(&write_mutex);
	writeToFile();
	closeFile();
	pthread_mutex_unlock(&write_mutex);
	cout<<"All done!"<<endl;
	return 1;
}

