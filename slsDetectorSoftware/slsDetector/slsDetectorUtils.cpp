#include "slsDetectorUtils.h"
#include "slsDetectorCommand.h"
#include "postProcessing.h"


#include  <cstdlib>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <time.h> //clock()
#include <string>
using namespace std;

slsDetectorUtils::slsDetectorUtils():
		stoppedFlag(0),
		timerValue(0),
		currentSettings(0),
		currentThresholdEV(0),
		totalProgress(0),
		progressIndex(0),
		acquisition_finished(NULL),
		measurement_finished(NULL),
		acqFinished_p(NULL),
		measFinished_p(NULL),
		progress_call(0),
		pProgressCallArg(0)
		{
#ifdef VERBOSE
  registerAcquisitionFinishedCallback(&dummyAcquisitionFinished,this);
  registerMeasurementFinishedCallback(&dummyMeasurementFinished,this);
  cout << "done " << endl;
#endif
}
 




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

	progressIndex=0;
	*stoppedFlag=0;
	void *status;
	setJoinThread(0);

	int nm=timerValue[MEASUREMENTS_NUMBER];
	if (nm<1)
		nm=1;

	// verify receiver is idle
	if(receiver){
		pthread_mutex_lock(&mg);
		if(getReceiverStatus()!=IDLE)
			if(stopReceiver() == FAIL)
				*stoppedFlag=1;
		pthread_mutex_unlock(&mg);
	}

	// start processing thread
	if (*threadedProcessing)
		startThread(delflag);


	//resets frames caught in receiver
	if(receiver){
		pthread_mutex_lock(&mg);
		if (resetFramesCaught() == FAIL)
			*stoppedFlag=1;
		pthread_mutex_unlock(&mg);
	}

	// loop through measurements
	for(int im=0;im<nm;++im) {

		if (*stoppedFlag)
			break;

		// start receiver
		if(receiver){

			pthread_mutex_lock(&mg);
			if(startReceiver() == FAIL) {
				cout << "Start receiver failed " << endl;
				stopReceiver();
				*stoppedFlag=1;
				pthread_mutex_unlock(&mg);
				break;
			}
			pthread_mutex_unlock(&mg);

			//let processing thread listen to these packets
			sem_post(&sem_newRTAcquisition);
		}

		// detector start
		startAndReadAll();

		if (*threadedProcessing==0){
			processData(delflag);
		}


		// stop receiver
		if(receiver){
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

		int findex = 0;
		pthread_mutex_lock(&mg);
		findex = incrementFileIndex();
		pthread_mutex_unlock(&mg);

		if (measurement_finished){
			pthread_mutex_lock(&mg);
			measurement_finished(im,findex,measFinished_p);
			pthread_mutex_unlock(&mg);
		}

		if (*stoppedFlag) {
			break;
		}

	}//end measurements loop im


	// waiting for the data processing thread to finish!
	if (*threadedProcessing) {
		setJoinThread(1);

		//let processing thread continue and checkjointhread
		sem_post(&sem_newRTAcquisition);

		pthread_join(dataProcessingThread, &status);
	}


	if(progress_call)
		progress_call(getCurrentProgress(),pProgressCallArg);

	if (acquisition_finished)
		acquisition_finished(getCurrentProgress(),getDetectorStatus(),acqFinished_p);

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

	int nf=1, nc=1, ns=1, nm=1;

	if (timerValue[FRAME_NUMBER])
		nf=timerValue[FRAME_NUMBER];

	if (timerValue[CYCLES_NUMBER]>0)
		nc=timerValue[CYCLES_NUMBER];

	if (timerValue[STORAGE_CELL_NUMBER]>0)
		ns=timerValue[STORAGE_CELL_NUMBER]+1;

	if (timerValue[MEASUREMENTS_NUMBER]>0)
		nm=timerValue[MEASUREMENTS_NUMBER];

	totalProgress=nm*nf*nc*ns;

#ifdef VERBOSE
	cout << "nm " << nm << endl;
	cout << "nf " << nf << endl;
	cout << "nc " << nc << endl;
	cout << "ns " << ns << endl;
	cout << "Set total progress " << totalProgress << endl;
#endif
	return totalProgress;
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
  cout << fixed << setprecision(2) << setw (6)
		  << 100.*((double)progressIndex)/((double)totalProgress) << " \%";
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
  cout << fixed << setprecision(2) << setw (6)
		  << 100.*((double)progressIndex)/((double)totalProgress) << " \%";
  pthread_mutex_unlock(&mp);
#ifdef VERBOSE
  cout << endl;
#else
  cout << "\r" << flush;
#endif
}


int slsDetectorUtils::retrieveDetectorSetup(string const fname1, int level){


  slsDetectorCommand *cmd;

  int skip=0;
  string fname;
  string str;
  ifstream infile;
  int iargval;
  int interrupt=0;
  char *args[10];

  char myargs[10][1000];
;

  string sargname, sargval;
  int iline=0;
  
  if (level==2) {
#ifdef VERBOSE
    cout << "config file read" << endl;
#endif
    fname=fname1+string(".det");
  }  else
    fname=fname1;

  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    cmd=new slsDetectorCommand(this);
    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      std::cout<<  str << std::endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	std::cout<< "Line is a comment " << std::endl;
	std::cout<< str << std::endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	iargval=0;
	while (ssstr.good()) {
	  ssstr >> sargname;
	  //  if (ssstr.good()) {
	  strcpy(myargs[iargval],sargname.c_str());
	  args[iargval]=myargs[iargval];
#ifdef VERBOSE
	  std::cout<< args[iargval]  << std::endl;
#endif
	  iargval++;
	  // }
	  skip=0;
	}

	if (level!=2) {
	  if (string(args[0])==string("trimbits"))
	    skip=1;
	}
	if (skip==0)
	  cmd->executeLine(iargval,args,PUT_ACTION);
      }
      iline++;
    }
    delete cmd;
    infile.close();

  } else {
    std::cout<< "Error opening  " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read  " << iline << " lines" << std::endl;
#endif

  if (getErrorMask())
	  return FAIL;

  return OK;


}


int slsDetectorUtils::dumpDetectorSetup(string const fname, int level){

	slsDetectorCommand *cmd;
	detectorType type = getDetectorsType();
	string names[100];
	int nvar=0;

	// common config
	names[nvar++]="fname";
	names[nvar++]="index";
	names[nvar++]="enablefwrite";
	names[nvar++]="overwrite";
	names[nvar++]="dr";
	names[nvar++]="settings";
	names[nvar++]="exptime";
	names[nvar++]="period";
	names[nvar++]="frames";
	names[nvar++]="cycles";
	names[nvar++]="measurements";
	names[nvar++]="timing";

	switch (type) {
	case EIGER:
		names[nvar++]="flags";
		names[nvar++]="clkdivider";
		names[nvar++]="threshold";
		names[nvar++]="ratecorr";
		names[nvar++]="trimbits";
		break;
	case GOTTHARD:
		names[nvar++]="delay";
		break;
	case JUNGFRAU:
		names[nvar++]="delay";
		names[nvar++]="clkdivider";
		break;
	case JUNGFRAUCTB:
		names[nvar++]="dac:0";
		names[nvar++]="dac:1";
		names[nvar++]="dac:2";
		names[nvar++]="dac:3";
		names[nvar++]="dac:4";
		names[nvar++]="dac:5";
		names[nvar++]="dac:6";
		names[nvar++]="dac:7";
		names[nvar++]="dac:8";
		names[nvar++]="dac:9";
		names[nvar++]="dac:10";
		names[nvar++]="dac:11";
		names[nvar++]="dac:12";
		names[nvar++]="dac:13";
		names[nvar++]="dac:14";
		names[nvar++]="dac:15";
		names[nvar++]="adcvpp";



		names[nvar++]="adcclk";
		names[nvar++]="clkdivider";
		names[nvar++]="adcphase";
		names[nvar++]="adcpipeline";
		names[nvar++]="adcinvert"; //
		names[nvar++]="adcdisable";
		names[nvar++]="patioctrl";
		names[nvar++]="patclkctrl";
		names[nvar++]="patlimits";
		names[nvar++]="patloop0";
		names[nvar++]="patnloop0";
		names[nvar++]="patwait0";
		names[nvar++]="patwaittime0";
		names[nvar++]="patloop1";
		names[nvar++]="patnloop1";
		names[nvar++]="patwait1";
		names[nvar++]="patwaittime1";
		names[nvar++]="patloop2";
		names[nvar++]="patnloop2";
		names[nvar++]="patwait2";
		names[nvar++]="patwaittime2";
		break;
	default:
		break;
	}


	int iv=0;
	string fname1;



	ofstream outfile;
	char *args[4];
	for (int ia=0; ia<4; ia++) {
		args[ia]=new char[1000];
	}


	if (level==2) {
		fname1=fname+string(".config");
		writeConfigurationFile(fname1);
		fname1=fname+string(".det");
	} else
		fname1=fname;



	outfile.open(fname1.c_str(),ios_base::out);
	if (outfile.is_open()) {
		cmd=new slsDetectorCommand(this);
		for (iv=0; iv<nvar; iv++) {
			strcpy(args[0],names[iv].c_str());
			outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;
		}

		delete cmd;

		outfile.close();
	}
	else {
		std::cout<< "Error opening parameters file " << fname1 << " for writing" << std::endl;
		return FAIL;
	}

#ifdef VERBOSE
	std::cout<< "wrote " <<iv << " lines to  "<< fname1 << std::endl;
#endif

	return OK;

} 




int slsDetectorUtils::readDataFile(std::string fname, short int *data, int nch) {
	std::ifstream infile;
	int iline=0;
	std::string str;
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {
		iline=readDataFile(infile, data, nch, 0);
		infile.close();
	} else {
		std::cout<< "Could not read file " << fname << std::endl;
		return -1;
	}
	return iline;
}


int slsDetectorUtils::readDataFile(std::ifstream &infile, short int *data, int nch, int offset) {
	int ichan, iline=0;
	short int idata;
	int interrupt=0;
	std::string str;
	while (infile.good() and interrupt==0) {
		getline(infile,str);
		std::istringstream ssstr(str);
		ssstr >> ichan >> idata;
		if (ssstr.fail() || ssstr.bad()) {
			interrupt=1;
			break;
		}
		if (iline<nch) {
			if (ichan>=offset) {
				data[iline]=idata;
				iline++;
			}
		} else {
			interrupt=1;
			break;
		}
		return iline;
	};
	return iline;
}


int slsDetectorUtils::writeDataFile(std::string fname,int nch, short int *data) {
	std::ofstream outfile;
	if (data==NULL)
		return slsDetectorDefs::FAIL;
	outfile.open (fname.c_str(),std::ios_base::out);
	if (outfile.is_open())       {
		writeDataFile(outfile, nch, data, 0);
		outfile.close();
		return slsDetectorDefs::OK;
	} else {
		std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
		return slsDetectorDefs::FAIL;
	}
}


int slsDetectorUtils::writeDataFile(std::ofstream &outfile,int nch, short int *data, int offset) {
	if (data==NULL)
		return slsDetectorDefs::FAIL;
	for (int ichan=0; ichan<nch; ichan++)
		outfile << ichan+offset << " " << *(data+ichan) << std::endl;
	return slsDetectorDefs::OK;
}
