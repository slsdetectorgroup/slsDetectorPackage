#include "slsDetectorUtils.h"
#include "usersFunctions.h"
#include "slsDetectorCommand.h"
#include "postProcessing.h"
#include "enCalLogClass.h"
#include "angCalLogClass.h"

#include  <cstdlib>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <time.h> //clock()
#include <string>
#include <iterator>
using namespace std;

slsDetectorUtils::slsDetectorUtils()  {


#ifdef VERBOSE
  cout << "setting callbacks" << endl;
#endif
  acquisition_finished=NULL;
  acqFinished_p=NULL;
  measurement_finished=NULL;
  measFinished_p=NULL;
  progress_call=0;
  pProgressCallArg=0;
  registerGetPositionCallback(&defaultGetPosition, NULL);
  registerConnectChannelsCallback(&defaultConnectChannels,NULL);
  registerDisconnectChannelsCallback(&defaultDisconnectChannels,NULL);
  registerGoToPositionCallback(&defaultGoToPosition,NULL);
  registerGoToPositionNoWaitCallback(&defaultGoToPositionNoWait,NULL);
  registerGetI0Callback(&defaultGetI0,NULL);
#ifdef VERBOSE

  registerAcquisitionFinishedCallback(&dummyAcquisitionFinished,this);
  registerMeasurementFinishedCallback(&dummyMeasurementFinished,this);
  cout << "done " << endl;
#endif

};
 




int  slsDetectorUtils::acquire(int delflag){

	//ensure acquire isnt started multiple times by same client
	if (isAcquireReady() ==  FAIL)
		return FAIL;

#ifdef VERBOSE
	struct timespec begin,end;
	clock_gettime(CLOCK_REALTIME, &begin);
#endif

	//in the real time acquisition loop, processing thread will wait for a post each time
	sem_init(&sem_newRTAcquisition,1,0);
	//in the real time acquistion loop, main thread will wait for processing thread to be done each time (which in turn waits for receiver/ext process)
	sem_init(&sem_endRTAcquisition,1,0);


	bool receiver = (setReceiverOnline()==ONLINE_FLAG);
	if(!receiver){
		setDetectorIndex(-1);
	}
	pthread_mutex_lock(&mg);
	int nc=setTimer(CYCLES_NUMBER,-1);
	int nf=setTimer(FRAME_NUMBER,-1);
	pthread_mutex_unlock(&mg);


	if (nc==0) nc=1;
	if (nf==0) nf=1;
	int multiframe = nc*nf;

	progressIndex=0;
	*stoppedFlag=0;

	angCalLogClass *aclog=NULL;
	enCalLogClass *eclog=NULL;
	int connectChannels=0;

#ifdef VERBOSE
	cout << "Acquire function "<< delflag << endl;
	cout << "Stopped flag is "<< stoppedFlag << delflag << endl;
#endif

	void *status;
	if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION)) || getActionMode(angCalLog) || (getScanMode(0)==positionScan)|| (getScanMode(1)==positionScan)) {
		if (connectChannels==0)
			if (connect_channels) {
				connect_channels(CCarg);
				connectChannels=1;
			}
	}
	if (getActionMode(angCalLog)) {
		aclog=new angCalLogClass(this);
	}
	if (getActionMode(enCalLog)) {
		eclog=new enCalLogClass(this);

	}


	setJoinThread(0);
	positionFinished(0);


	int nm=timerValue[MEASUREMENTS_NUMBER];
	if (nm<1)
		nm=1;


	int  np=getNumberOfPositions();
	if (np<1)
		np=1;


	int ns0=1;
	if (*actionMask & (1 << MAX_ACTIONS)) {
		ns0=getScanSteps(0);
		if (ns0<1)
			ns0=1;
	}

	int ns1=1;
	if (*actionMask & (1 << (MAX_ACTIONS+1))) {
		ns1=getScanSteps(1);
		if (ns1<1)
			ns1=1;
	}

	// verify receiver is idle
	if(receiver){
		pthread_mutex_lock(&mg);
		if(getReceiverStatus()!=IDLE)
			if(stopReceiver() == FAIL)
				*stoppedFlag=1;
		pthread_mutex_unlock(&mg);
	}


	if (*threadedProcessing)
		startThread(delflag);
#ifdef VERBOSE
	cout << " starting thread " << endl;
#endif

	//resets frames caught in receiver
	if(receiver){
		pthread_mutex_lock(&mg);
		if (resetFramesCaught() == FAIL)
			*stoppedFlag=1;
		pthread_mutex_unlock(&mg);
	}


	for(int im=0;im<nm;++im) {

#ifdef VERBOSE
		cout << " starting measurement "<< im << " of " << nm << endl;
#endif

		// start script
		if (*stoppedFlag==0) {
			executeAction(startScript);
		}

		for (int is0=0; is0<ns0; ++is0) {

			if (*stoppedFlag==0) {
				executeScan(0,is0);
			} else
				break;


			for (int is1=0; is1<ns1; ++is1) {

				if (*stoppedFlag==0) {
					executeScan(1,is1);
				} else
					break;

				if (*stoppedFlag==0) {
					executeAction(scriptBefore);
				} else
					break;

				ResetPositionIndex();

				for (int ip=0; ip<np; ++ip) {

					if (*stoppedFlag==0) {
						if  (getNumberOfPositions()>0) {
							moveDetector(detPositions[ip]);
							IncrementPositionIndex();
#ifdef VERBOSE
							std::cout<< "moving to position" << std::endl;
#endif
						}
					} else
						break;


					pthread_mutex_lock(&mp);
					createFileName();
					pthread_mutex_unlock(&mp);

					// script before
					if (*stoppedFlag==0) {
						executeAction(scriptBefore);
					} else
						break;


					// header before
					if (*stoppedFlag==0) {
						executeAction(headerBefore);

						if (*correctionMask&(1<< ANGULAR_CONVERSION) || aclog){
							positionFinished(0);
							setCurrentPosition(getDetectorPosition());
						}

						if (aclog)
							aclog->addStep(getCurrentPosition(), getCurrentFileName());

						if (eclog)
							eclog->addStep(setDAC(-1,THRESHOLD,0), getCurrentFileName());


						if (*correctionMask&(1<< I0_NORMALIZATION)) {
							if (get_i0)
								get_i0(0, IOarg);
						}

						setCurrentFrameIndex(0);

						if (multiframe>1)
							setFrameIndex(0);
						else
							setFrameIndex(-1);

						// file name and start receiver
						if(receiver){
							pthread_mutex_lock(&mp);
							createFileName();
							pthread_mutex_unlock(&mp);
							//send receiver file name
							pthread_mutex_lock(&mg);
							setFileName(fileIO::getFileName());

							if(startReceiver() == FAIL) {
								cout << "Start receiver failed " << endl;
								stopReceiver();
								*stoppedFlag=1;
								pthread_mutex_unlock(&mg);
								break;
							}
#ifdef VERBOSE
							cout << "Receiver started " << endl;
#endif
							pthread_mutex_unlock(&mg);

							//let processing thread listen to these packets
							sem_post(&sem_newRTAcquisition);
						}
#ifdef VERBOSE
						cout << "Acquiring " << endl;
#endif
						startAndReadAll();
#ifdef VERBOSE
						cout << "detector finished" << endl;
						cout << "returned! " << endl;
#endif


						if (*correctionMask&(1<< I0_NORMALIZATION)) {
							if (get_i0)
								currentI0=get_i0(1,IOarg);
						}
#ifdef VERBOSE
						cout << "pos finished? " << endl;
#endif     

						positionFinished(1);

#ifdef VERBOSE
						cout << "done! " << endl;
#endif


						if (*threadedProcessing==0){
#ifdef VERBOSE
							cout << "start unthreaded process data " << endl;
#endif
							processData(delflag);
						}

					} else
						break;

					while (dataQueueSize()) usleep(100000);

					// close file
					if(!receiver){
						detectorType type = getDetectorsType();
						if ((type==GOTTHARD) || (type==MOENCH) || (type==JUNGFRAUCTB) ){
							if((*correctionMask)&(1<<WRITE_FILE))
								closeDataFile();
						}
					}

					// stop receiver
					else{
						pthread_mutex_lock(&mg);
						if (stopReceiver() == FAIL) {
							*stoppedFlag = 1;
							pthread_mutex_unlock(&mg);
						} else {
							pthread_mutex_unlock(&mg);
							  if (*threadedProcessing && dataReady)
								  sem_wait(&sem_endRTAcquisition); // waits for receiver's external process to be done sending data to gui
						  }
					}

					// header after
					if (*stoppedFlag==0) {
						pthread_mutex_lock(&mp);
						executeAction(headerAfter);
						pthread_mutex_unlock(&mp);
					}

					if (*stoppedFlag) {
#ifdef VERBOSE
						std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
						break;
					}

				}//end position loop ip


				//script after
				if (*stoppedFlag==0) {
					executeAction(scriptAfter);
				}

				if (*stoppedFlag) {
#ifdef VERBOSE
					std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
					break;
				}

			}//end scan1 loop is1


			if (*stoppedFlag) {
#ifdef VERBOSE
				std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
				break;
			}

		}//end scan0 loop is0



		if (*stoppedFlag==0) {
			executeAction(stopScript);
		}

		if (*stoppedFlag) {
#ifdef VERBOSE
			cout << "findex incremented " << endl;
#endif
			if(*correctionMask&(1<<WRITE_FILE))
				IncrementFileIndex();
			pthread_mutex_lock(&mg);
			setFileIndex(fileIO::getFileIndex());
			pthread_mutex_unlock(&mg);
#ifdef VERBOSE
			std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
			break;
		}


#ifdef VERBOSE
		cout << "findex incremented " << endl;
#endif
		if(*correctionMask&(1<<WRITE_FILE))
			IncrementFileIndex();
		pthread_mutex_lock(&mg); //cout << "lock"<< endl;
		setFileIndex(fileIO::getFileIndex());
		pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;

		if (measurement_finished){
			pthread_mutex_lock(&mg); //cout << "lock"<< endl;
			measurement_finished(im,*fileIndex,measFinished_p);
			pthread_mutex_unlock(&mg);//cout << "unlock"<< endl;
		}

		if (*stoppedFlag) {
#ifdef VERBOSE
			std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
			break;
		}

	}//end measurements loop im



	// waiting for the data processing thread to finish!
	if (*threadedProcessing) {
#ifdef VERBOSE
		cout << "wait for data processing thread" << endl;
#endif
		setJoinThread(1);

		//let processing thread continue and checkjointhread
		sem_post(&sem_newRTAcquisition);

		pthread_join(dataProcessingThread, &status);
#ifdef VERBOSE
		cout << "data processing thread joined" << endl;
#endif
	}


	if(progress_call)
		progress_call(getCurrentProgress(),pProgressCallArg);


	if (connectChannels) {
		if (disconnect_channels)
			disconnect_channels(DCarg);
	}

	if (aclog)
		delete aclog;

	if (eclog)
		delete eclog;
#ifdef VERBOSE
	cout << "acquisition finished callback " << endl;
#endif
	if (acquisition_finished)
		acquisition_finished(getCurrentProgress(),getDetectorStatus(),acqFinished_p);
#ifdef VERBOSE
	cout << "acquisition finished callback done " << endl;
#endif

  sem_destroy(&sem_newRTAcquisition);
  sem_destroy(&sem_endRTAcquisition);

#ifdef VERBOSE
	clock_gettime(CLOCK_REALTIME, &end);
	cout << "Elapsed time for acquisition:" << (( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0) << " seconds" << endl;
#endif

  setAcquiringFlag(false);

  return OK;


}





//Naveen change

int slsDetectorUtils::setTotalProgress() {
  
  int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1, nm=1;

  if (timerValue[FRAME_NUMBER])
    nf=timerValue[FRAME_NUMBER];

  if (timerValue[CYCLES_NUMBER]>0)
    nc=timerValue[CYCLES_NUMBER];

  if (timerValue[MEASUREMENTS_NUMBER]>0)
    nm=timerValue[MEASUREMENTS_NUMBER];

  if (*numberOfPositions>0)
    npos=*numberOfPositions;

  if ((nScanSteps[0]>0) && (*actionMask & (1 << MAX_ACTIONS)))
    nscan[0]=nScanSteps[0];

  if ((nScanSteps[1]>0) && (*actionMask & (1 << (MAX_ACTIONS+1))))
    nscan[1]=nScanSteps[1];
      
  totalProgress=nm*nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
  cout << "nc " << nc << endl;
  cout << "nm " << nm << endl;
  cout << "nf " << nf << endl;
  cout << "npos " << npos << endl;
  cout << "nscan[0] " << nscan[0] << endl;
  cout << "nscan[1] " << nscan[1] << endl;

  cout << "Set total progress " << totalProgress << endl;
#endif
  return totalProgress;
}









int slsDetectorUtils::setBadChannelCorrection(string fname, int &nbadtot, int *badchanlist, int off){

  int nbad;
  int badlist[MAX_BADCHANS];

  ifstream infile;
  string str;
  //int interrupt=0;
  //int ich;
  //int chmin,chmax;
#ifdef VERBOSE
  std::cout << "utils: Setting bad channel correction to " << fname << std::endl;
#endif
 // int modmi=0;
  int modma=1;
  int singlefile=0;

  string fn;
  int offset=off;


  nbadtot=0;

  if (fname=="" || fname=="none") {
    ;
  } else { 

    if (fname.find(".sn")==string::npos && fname.find(".chans")==string::npos) {
      modma=setNumberOfModules();
      singlefile=1;
    }

    for (int im=0; im<modma; im++) {
      

      if (singlefile) {

	ostringstream ostfn;
	ostfn << fname << ".sn"  << setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im); 
	fn=ostfn.str();
  
      } else
	fn=fname;
      


      infile.open(fn.c_str(), ios_base::in);
      if (infile.is_open()==0) {
	std::cout << "could not open file " << fname <<std::endl;
	return -1;
      }


      nbad=setBadChannelCorrection(infile, nbad, badlist, offset);
      infile.close();
    
      for (int ich=0; ich<nbad; ich++) {
	if (nbadtot<MAX_BADCHANS) {
	  badchanlist[nbadtot]=badlist[ich];
	  nbadtot++;
	}
      }
    
      offset+=getChansPerMod(im); 
    
    }
  
  }
  if (nbadtot>0 && nbadtot<MAX_BADCHANS) {
    return nbadtot;
  } else
    return 0;

}




double slsDetectorUtils::getCurrentProgress() {
  pthread_mutex_lock(&mp);
#ifdef VERBOSE
  cout << progressIndex << " / " << totalProgress << endl;
#endif
  
  double p=100.*((double)progressIndex)/((double)totalProgress);
  pthread_mutex_unlock(&mp);
  return p;
}



void slsDetectorUtils::incrementProgress()  {
  pthread_mutex_lock(&mp);
  progressIndex++;
  cout << fixed << setprecision(2) << setw (6) << 100.*((double)progressIndex)/((double)totalProgress) << " \%";
  pthread_mutex_unlock(&mp);
#ifdef VERBOSE
  cout << endl;
#else
  cout << "\r" << flush;
#endif

};


void slsDetectorUtils::setCurrentProgress(int i){
  pthread_mutex_lock(&mp);
  progressIndex=i;
  cout << fixed << setprecision(2) << setw (6) << 100.*((double)progressIndex)/((double)totalProgress) << " \%";
  pthread_mutex_unlock(&mp);
#ifdef VERBOSE
  cout << endl;
#else
  cout << "\r" << flush;
#endif
}




