#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "sls_receiver_exceptions.h"
#include "SharedMemory.h"
#include "receiverInterface.h"
#include "gitInfoLib.h"
#include "versionAPI.h"
#include "usersFunctions.h"
#include "slsDetectorCommand.h"
#include "postProcessingFuncs.h"


#include  <sys/types.h>
#include  <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bitset>
#include <cstdlib>
#include <math.h>


using namespace std;


slsDetector::slsDetector(detectorType type, int multiId, int id, bool verify, multiSlsDetector* m)
: slsDetectorUtils(),
  detId(id),
  sharedMemory(0),
  thisDetector(0),
  multiDet(m),
  thisReceiver(0),
  controlSocket(0),
  stopSocket(0),
  dataSocket(0),
  ffcoefficients(0),
  fferrors(0),
  detectorModules(0),
  dacs(0),
  adcs(0),
  chipregs(0),
  chanregs(0),
  gain(0),
  offset(0) {
	/* called from put hostname command,
	 * so sls shared memory will be created */

	// ensure shared memory was not created before
	SharedMemory* shm = new SharedMemory(multiId, id);
	if (shm->IsExisting()) {
		cprintf(YELLOW BOLD,"Warning: Weird, this shared memory should have been "
				"deleted before! %s. Freeing it again.\n", shm->GetName().c_str());
		freeSharedMemory(multiId, id);
	}
	delete shm;

	initSharedMemory(true, type, multiId, verify);
	initializeDetectorStructure(type);
	initializeMembers();
	initializeDetectorStructurePointers();
}

slsDetector::slsDetector(int multiId, int id, bool verify, multiSlsDetector* m)
: slsDetectorUtils(),
  detId(id),
  sharedMemory(0),
  thisDetector(0),
  multiDet(m),
  thisReceiver(0),
  controlSocket(0),
  stopSocket(0),
  dataSocket(0),
  ffcoefficients(0),
  fferrors(0),
  detectorModules(0),
  dacs(0),
  adcs(0),
  chipregs(0),
  chanregs(0),
  gain(0),
  offset(0) {
	/* called from multi constructor to populate structure,
	 * so sls shared memory will be opened, not created */

	// getDetectorType Froom shm will check if it was already existing
	detectorType type = getDetectorTypeFromShm(multiId, verify);

	initSharedMemory(false, type, multiId, verify);
	initializeMembers();
}

slsDetector::~slsDetector() {
	if (sharedMemory) {
		sharedMemory->UnmapSharedMemory(thisDetector);
		delete sharedMemory;
	}
	if(thisReceiver)
		delete thisReceiver;
	if(controlSocket)
		delete controlSocket;
	if(stopSocket)
		delete stopSocket;
	if(dataSocket)
		delete dataSocket;

	/* detectorModules, dacs..ffoerrors are offsets from the
	 * shared memory and created within shared memory structure.
	 * Deleting shared memory will also delete memory pointed to
	 * by these pointers
	 */
}


bool slsDetector::isMultiSlsDetectorClass() {
	return false;
}


double* slsDetector::decodeData(int *datain, int &nn, double *fdata) {

	double *dataout = 0;
	if (fdata) {
		dataout=fdata;
		nn=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
		if (thisDetector->myDetectorType == JUNGFRAUCTB)
			nn=thisDetector->dataBytes/2;
	} else {
		if (thisDetector->myDetectorType == JUNGFRAUCTB) {
			nn=thisDetector->dataBytes/2;
			dataout=new double[nn];
		} else {
			dataout=new double[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
			nn=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
		}
	}

	int ival = 0, ipos = 0, ichan=0, ibyte = 0;
	char *ptr = (char*)datain;
	char iptr = 0;
	int nbits=thisDetector->dynamicRange;
	int nch=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;

	if (thisDetector->timerValue[PROBES_NUMBER]==0) {
		if (thisDetector->myDetectorType==JUNGFRAUCTB) {
			for (ichan=0; ichan<nn; ++ichan) {
				dataout[ichan]=*((u_int16_t*)ptr);
				ptr+=2;
			}
			std::cout<< "decoded "<< ichan << " channels" << std::endl;
		} else {
			switch (nbits) {
			case 1:
				for (ibyte=0; ibyte<thisDetector->dataBytes; ++ibyte) {
					iptr=ptr[ibyte];
					for (ipos=0; ipos<8; ++ipos) {
						ival=(iptr>>(ipos))&0x1;
						dataout[ichan]=ival;
						++ichan;
					}
				}
				break;
			case 4:
				for (ibyte=0; ibyte<thisDetector->dataBytes; ++ibyte) {
					iptr=ptr[ibyte];
					for (ipos=0; ipos<2; ++ipos) {
						ival=(iptr>>(ipos*4))&0xf;
						dataout[ichan]=ival;
						++ichan;
					}
				}
				break;
			case 8:
				for (ichan=0; ichan<thisDetector->dataBytes; ++ichan) {
					ival=ptr[ichan]&0xff;
					dataout[ichan]=ival;
				}
				break;
			case 16:
				for (ichan=0; ichan<nch; ++ichan) {
					dataout[ichan]=*((u_int16_t*)ptr);
					ptr+=2;
				}
				break;
			default:
				int mask=0xffffffff;
				if(thisDetector->myDetectorType == MYTHEN) mask=0xffffff;
				for (ichan=0; ichan<nch; ++ichan) {
					dataout[ichan]=datain[ichan]&mask;
				}
			}
		}
	} else {
		for (ichan=0; ichan<nch; ++ichan) {
			dataout[ichan]=datain[ichan];
		}
	}
	return dataout;
}


int64_t slsDetector::clearAllErrorMask() {
	clearErrorMask();
	pthread_mutex_lock(&ms);
	for(int i=0;i<multiDet->getNumberOfDetectors();++i){
		multiDet->setErrorMask(multiDet->getErrorMask()|(0<<i));
	}
	pthread_mutex_unlock(&ms);
	return getErrorMask();
}

void slsDetector::setAcquiringFlag(bool b) {
	multiDet->setAcquiringFlag(b);
}

bool slsDetector::getAcquiringFlag() {
	return multiDet->getAcquiringFlag();
}


bool slsDetector::isAcquireReady() {
	return multiDet->isAcquireReady();
}


int slsDetector::checkVersionCompatibility(portType t) {
	int fnum = F_CHECK_VERSION;
	if (t == DATA_PORT)
		fnum = F_RECEIVER_CHECK_VERSION;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);
	int64_t arg = 0;

	// detector
	if (t == CONTROL_PORT) {

		switch (thisDetector->myDetectorType) {
		case EIGER:		arg = APIEIGER; break;
		case JUNGFRAU:	arg = APIJUNGFRAU; break;
		case GOTTHARD:	arg = APIGOTTHARD; break;
		default:
			std::cout<< "Check version compatibility is not implemented for this "
					"detector" << std::endl;
			setErrorMask((getErrorMask())|(VERSION_COMPATIBILITY));
			return FAIL;
		}

#ifdef VERBOSE
		std::cout<< std::endl<< "Checking version compatibility with detector with "
				"value " << hex << arg << std::endl;
#endif
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			// control port
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&arg,sizeof(arg));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret == FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					cprintf(RED, "Detector returned error: (Control Server) %s", mess);
					if(strstr(mess,"Unrecognized Function")!=NULL)
						std::cout << "The detector server is too old to get API version. "
								"Please update detector server!" << std::endl;
					setErrorMask((getErrorMask())|(VERSION_COMPATIBILITY));
					thisDetector->detectorControlAPIVersion = 0;
				} else {
					thisDetector->detectorControlAPIVersion = arg;
				}
				disconnectControl();
			}
			if (ret!= FAIL) {
				ret = FAIL;

				// stop port
				if (connectStop() == OK){
					stopSocket->SendDataOnly(&fnum,sizeof(fnum));
					stopSocket->SendDataOnly(&arg,sizeof(arg));
					stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
					if (ret == FAIL) {
						stopSocket->ReceiveDataOnly(mess,sizeof(mess));
						cprintf(RED, "Detector returned error: (Stop Server) %s", mess);
						if(strstr(mess,"Unrecognized Function")!=NULL)
							std::cout << "The detector server is too old to get API "
									"version. Please update detector server!" << std::endl;
						setErrorMask((getErrorMask())|(VERSION_COMPATIBILITY));
						thisDetector->detectorStopAPIVersion = 0;
					}  else {
						thisDetector->detectorStopAPIVersion = arg;
					}
					disconnectStop();
				}
			}
		}
	}

	// receiver
	else {
		arg = APIRECEIVER;
#ifdef VERBOSE
		std::cout<< std::endl<< "Checking version compatibility with receiver with "
				"value " << hex << arg << std::endl;
#endif
		if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
			// data port
			if (connectData() == OK){
				// ignoring retval
				int64_t retval = -1;
				ret=thisReceiver->sendInt(fnum,retval,arg);
				if (ret==FAIL){
					setErrorMask((getErrorMask())|(VERSION_COMPATIBILITY));
					if(strstr(mess,"Unrecognized Function")!=NULL)
						std::cout << "The receiver software is too old to get API "
								"version. Please update receiver software!" << std::endl;
					thisDetector->receiverAPIVersion = 0;
				} else  {
					thisDetector->receiverAPIVersion = arg;
				}
				disconnectData();
			}
		}
	}

	return ret;
}




int64_t slsDetector::getId( idMode mode, int imod) {

	int64_t retval=-1;
	int fnum=F_GET_ID,fnum2 = F_GET_RECEIVER_ID;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< std::endl;
	if  (mode==MODULE_SERIAL_NUMBER)
		std::cout<< "Getting id  of "<< imod << std::endl;
	else
		std::cout<< "Getting id type "<< mode << std::endl;
#endif
	if (mode==THIS_SOFTWARE_VERSION) {
		ret=OK;
		retval=GITDATE;
	} else if (mode==RECEIVER_VERSION) {
		if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
			if (connectData() == OK){
				ret=thisReceiver->getInt(fnum2,retval);
				disconnectData();
			}
			if(ret==FORCE_UPDATE)
				ret=updateReceiver();
		}
	} else {
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() != OK)
				ret = FAIL;
			else{
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&mode,sizeof(mode));
				if (mode==MODULE_SERIAL_NUMBER)
					controlSocket->SendDataOnly(&imod,sizeof(imod));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL)
					controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				else {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				disconnectControl();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}
	}
	if (ret==FAIL) {
		std::cout<< "Get id failed " << std::endl;
		return ret;
	} else {
#ifdef VERBOSE
		if  (mode==MODULE_SERIAL_NUMBER)
			std::cout<< "Id of "<< imod <<" is " << hex <<retval << setbase(10)
			<< std::endl;
		else
			std::cout<< "Id "<< mode <<" is " << hex <<retval << setbase(10)
			<< std::endl;
#endif
		return retval;
	}
}


void slsDetector::freeSharedMemory(int multiId, int slsId) {
	SharedMemory* shm = new SharedMemory(multiId, slsId);
	shm->RemoveSharedMemory();
	delete shm;
}

void slsDetector::freeSharedMemory() {
	if (sharedMemory) {
		sharedMemory->UnmapSharedMemory(thisDetector);
		sharedMemory->RemoveSharedMemory();
		delete sharedMemory;
		sharedMemory = 0;
	}
	thisDetector = 0;
}

string slsDetector::getUserDetails() {
	cprintf(RED, "Error: Get User details should not be called at this level\n");
	return string("");
}

void slsDetector::setHostname(const char *name) {
	setTCPSocket(string(name));
	if (thisDetector->onlineFlag == ONLINE_FLAG)
		updateDetector();
}

string slsDetector::getHostname(int pos) {
	return string(thisDetector->hostname);
}

void slsDetector::addMultipleDetectors(const char* name) {
	cprintf(RED, "Error: Add Multiple Detectors should not be called at this level\n");
}

/*
 * pre: sharedMemory=0, thisDetector = 0
 * exceptions are caught in calling function, shm unmapped and deleted
 */
void slsDetector::initSharedMemory(bool created, detectorType type, int multiId,
		bool verify) {
	try {
		// calculate shared memory size
		int sz = calculateSharedMemorySize(type);

		// shared memory object with name
		sharedMemory = new SharedMemory(multiId, detId);

		// create
		if (created) {
			thisDetector = (sharedSlsDetector*)sharedMemory->CreateSharedMemory(sz);
		}
		// open and verify version
		else {
			thisDetector = (sharedSlsDetector*)sharedMemory->OpenSharedMemory(sz);
			if (verify && thisDetector->shmversion != SLS_SHMVERSION) {
				cprintf(RED, "Single shared memory (%d-%d:)version mismatch "
						"(expected 0x%x but got 0x%x)\n",
						multiId, detId, SLS_SHMVERSION,
						thisDetector->shmversion);
				throw SharedMemoryException();
			}
		}
	}  catch(...) {
		if (sharedMemory) {
			// unmap
			if (thisDetector) {
				sharedMemory->UnmapSharedMemory(thisDetector);
				thisDetector = 0;
			}
			// delete
			delete sharedMemory;
			sharedMemory = 0;
		}
		throw;
	}
}


void slsDetector::setDetectorSpecificParameters(detectorType type, detParameterList& list) {
	switch (type) {
	case MYTHEN:
		list.nModMaxX = 24;
		list.nModMaxY = 1;
		list.nChanX = 128;
		list.nChanY = 1;
		list.nChipX = 10;
		list.nChipY = 1;
		list.nDacs = 6;
		list.nAdcs = 0;
		list.nGain = 0;
		list.nOffset = 0;
		list.dynamicRange = 24;
		list.moveFlag = 1;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case PICASSO: /** is this needed?*/
		list.nModMaxX = 24;
		list.nModMaxY = 1;
		list.nChanX = 128;
		list.nChanY = 1;
		list.nChipX = 12;
		list.nChipY = 1;
		list.nDacs = 6;
		list.nAdcs = 0;
		list.nGain = 0;
		list.nOffset = 0;
		list.dynamicRange = 24;
		list.moveFlag = 0;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case GOTTHARD:
		list.nModMaxX = 1;
		list.nModMaxY = 1;
		list.nChanX = 128;
		list.nChanY = 1;
		list.nChipX = 10;
		list.nChipY = 1;
		list.nDacs = 8;
		list.nAdcs = 5;
		list.nGain = 0;
		list.nOffset = 0;
		list.dynamicRange = 16;
		list.moveFlag = 0;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case PROPIX:
		list.nModMaxX = 1;
		list.nModMaxY = 1;
		list.nChanX = 22;
		list.nChanY = 22;
		list.nChipX = 1;
		list.nChipY = 1;
		list.nDacs = 8;
		list.nAdcs = 5;
		list.nGain = 0;
		list.nOffset = 0;
		list.dynamicRange = 16;
		list.moveFlag = 0;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case MOENCH:
		list.nModMaxX = 1;
		list.nModMaxY = 1;
		list.nChanX = 160;
		list.nChanY = 160;
		list.nChipX = 1;
		list.nChipY = 1;
		list.nDacs = 8;
		list.nAdcs = 1;
		list.nGain = 0;
		list.nOffset = 0;
		list.dynamicRange = 16;
		list.moveFlag = 0;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case JUNGFRAU:
		list.nModMaxX = 1;
		list.nModMaxY = 1;
		list.nChanX = 256;
		list.nChanY = 256;
		list.nChipX = 4;
		list.nChipY = 2;
		list.nDacs = 16;
		list.nAdcs = 0;
		list.nGain = 0;
		list.nOffset = 0;
		list.dynamicRange = 16;
		list.moveFlag = 0;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case JUNGFRAUCTB:
		list.nModMaxX = 1;
		list.nModMaxY = 1;
		list.nChanX = 36;
		list.nChanY = 1;
		list.nChipX = 1;
		list.nChipY = 1;
		list.nDacs = 16;
		list.nAdcs = 9;//???? When calculating size, only d+a=16 how come?FIXME
		list.nGain = 0;
		list.nOffset = 0;
		list.dynamicRange =16;
		list.moveFlag = 0;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case EIGER:
		list.nModMaxX = 1;
		list.nModMaxY = 1;
		list.nChanX = 256;
		list.nChanY = 256;
		list.nChipX = 4;
		list.nChipY = 1;
		list.nDacs = 16;
		list.nAdcs = 0;
		list.nGain = 0; // can be set back to 4 in case we require it again
		list.nOffset = 0; // can be set back to 4 in case we require it again
		list.dynamicRange = 16;
		list.moveFlag = 0;
		list.nGappixelsX = 6;
		list.nGappixelsY = 1;
		break;
	default:
		cprintf(RED,"Unknown detector type!\n");
		throw exception();
	}
}


int slsDetector::calculateSharedMemorySize(detectorType type) {
	// get the detector parameters based on type
	detParameterList detlist;
	setDetectorSpecificParameters(type, detlist);
	int nm = detlist.nModMaxX * detlist.nModMaxY;
	int nch = detlist.nChanX * detlist.nChanY;
	int nc = detlist.nChipX * detlist.nChipY;
	int nd = detlist.nDacs + detlist.nAdcs;
	int ng = detlist.nGain;
	int no = detlist.nOffset;

	/** The size of the shared memory is
	 * size of shared structure +
	 * ffcoefficents+fferrors+modules+dacs+adcs+chips+chans+gain+offset */
	int sz = sizeof(sharedSlsDetector) +
			nm * (
					2 * nch * nc * sizeof(double) +
					sizeof(sls_detector_module) +
					sizeof(int) * nc +
					sizeof(dacs_t) * nd +
					sizeof(int) * nch * nc +
					sizeof(int) * ng +
					sizeof(int) * no);
#ifdef VERBOSE
	std::cout<<"Size of shared memory is " << sz << std::endl;
#endif
	return sz;
}


void slsDetector::initializeDetectorStructure(detectorType type) {
	thisDetector->shmversion = SLS_SHMVERSION;
	thisDetector->onlineFlag = OFFLINE_FLAG;
	thisDetector->stoppedFlag = 0;
	strncpy(thisDetector->hostname, DEFAULT_HOSTNAME, MAX_STR_LENGTH-1);
	thisDetector->hostname[MAX_STR_LENGTH-1] = 0;
	thisDetector->offset[X] = 0;
	thisDetector->offset[Y] = 0;
	thisDetector->controlPort = DEFAULT_PORTNO;
	thisDetector->stopPort = DEFAULT_PORTNO + 1;
	thisDetector->myDetectorType = type;
	strncpy(thisDetector->settingsDir, getenv("HOME"),  MAX_STR_LENGTH-1);
	thisDetector->settingsDir[MAX_STR_LENGTH-1] = 0;
	strncpy(thisDetector->calDir, getenv("HOME"),  MAX_STR_LENGTH-1);
	thisDetector->calDir[MAX_STR_LENGTH-1] = 0;
	thisDetector->nTrimEn = 0;
	for(int i = 0; i < MAX_TRIMEN; ++i)
		thisDetector->trimEnergies[i] = 0;
	thisDetector->progressIndex = 0;
	thisDetector->totalProgress = 1;
	strcpy(thisDetector->filePath, "/");
	thisDetector->correctionMask = 0;
	thisDetector->threadedProcessing = 1;
	thisDetector->tDead = 0;
	strncpy(thisDetector->flatFieldDir, getenv("HOME"),  MAX_STR_LENGTH-1);
	thisDetector->flatFieldDir[MAX_STR_LENGTH-1] = 0;
	strcpy(thisDetector->flatFieldFile, "none");
	thisDetector->nBadChans = 0;
	strcpy(thisDetector->badChanFile, "none");
	thisDetector->nBadFF = 0;
	for (int i = 0; i < MAX_BADCHANS; ++i) {
		thisDetector->badChansList[i] = 0;
		thisDetector->badFFList[i] = 0;
	}
	strcpy(thisDetector->angConvFile, "none");
	memset(thisDetector->angOff, 0, MAXMODS * sizeof(angleConversionConstant));
	thisDetector->angDirection = 1;
	thisDetector->fineOffset = 0;
	thisDetector->globalOffset = 0;
	thisDetector->numberOfPositions = 0;
	for (int i = 0; i < MAXPOS; ++i) {
		thisDetector->detPositions[i] = 0;
	}
	thisDetector->binSize = 0.001;
	thisDetector->nROI = 0;
	memset(thisDetector->roiLimits, 0, MAX_ROIS * sizeof(ROI));
	thisDetector->roFlags = NORMAL_READOUT;
	strcpy(thisDetector->settingsFile, "none");
	thisDetector->currentSettings = UNINITIALIZED;
	thisDetector->currentThresholdEV = -1;
	thisDetector->timerValue[FRAME_NUMBER] = 1;
	thisDetector->timerValue[ACQUISITION_TIME] = 0;
	thisDetector->timerValue[FRAME_PERIOD] = 0;
	thisDetector->timerValue[DELAY_AFTER_TRIGGER] = 0;
	thisDetector->timerValue[GATES_NUMBER] = 0;
	thisDetector->timerValue[PROBES_NUMBER] = 0;
	thisDetector->timerValue[CYCLES_NUMBER] = 1;
	thisDetector->timerValue[ACTUAL_TIME] = 0;
	thisDetector->timerValue[MEASUREMENT_TIME] = 0;
	thisDetector->timerValue[PROGRESS] = 0;
	thisDetector->timerValue[MEASUREMENTS_NUMBER] = 1;
	thisDetector->timerValue[FRAMES_FROM_START] = 0;
	thisDetector->timerValue[FRAMES_FROM_START_PG] = 0;
	thisDetector->timerValue[SAMPLES_JCTB] = 1;
	thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME] = 0;
	thisDetector->timerValue[STORAGE_CELL_NUMBER] = 0;
	thisDetector->timerValue[SUBFRAME_DEADTIME] = 0;
	thisDetector->actionMask = 0;
	for (int i = 0; i < MAX_ACTIONS; ++i) {
		strcpy(thisDetector->actionScript[i], "none");
		strcpy(thisDetector->actionParameter[i], "none");
	}
	for (int i = 0; i < MAX_SCAN_LEVELS; ++i) {
		thisDetector->scanMode[i] = 0;
		strcpy(thisDetector->scanScript[i], "none");
		strcpy(thisDetector->scanParameter[i], "none");
		thisDetector->nScanSteps[i] = 0;
		{
			double initValue = 0;
			std::fill_n(thisDetector->scanSteps[i], MAX_SCAN_STEPS, initValue);
		}
		thisDetector->scanPrecision[i] = 0;
	}
	strcpy(thisDetector->receiver_hostname, "none");
	thisDetector->receiverTCPPort = DEFAULT_PORTNO+2;
	thisDetector->receiverUDPPort = DEFAULT_UDP_PORTNO;
	thisDetector->receiverUDPPort2 = DEFAULT_UDP_PORTNO + 1;
	strcpy(thisDetector->receiverUDPIP, "none");
	strcpy(thisDetector->receiverUDPMAC, "none");
	strncpy(thisDetector->detectorMAC, DEFAULT_DET_MAC, MAX_STR_LENGTH-1);
	thisDetector->detectorMAC[MAX_STR_LENGTH-1] = 0;
	strncpy(thisDetector->detectorIP, DEFAULT_DET_IP, MAX_STR_LENGTH-1);
	thisDetector->detectorIP[MAX_STR_LENGTH-1] = 0;
	thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
	thisDetector->tenGigaEnable = 0;
	thisDetector->flippedData[X] = 0;
	thisDetector->flippedData[Y] = 0;
	thisDetector->zmqport = DEFAULT_ZMQ_CL_PORTNO +
			(detId * ((thisDetector->myDetectorType == EIGER) ? 2 : 1));
	thisDetector->receiver_zmqport = DEFAULT_ZMQ_RX_PORTNO +
			(detId * ((thisDetector->myDetectorType == EIGER) ? 2 : 1));
	thisDetector->receiver_upstream = false;
	thisDetector->receiver_read_freq = 0;
	memset(thisDetector->zmqip, 0, MAX_STR_LENGTH);
	memset(thisDetector->receiver_zmqip, 0, MAX_STR_LENGTH);
	thisDetector->gappixels = 0;
	memset(thisDetector->receiver_additionalJsonHeader, 0, MAX_STR_LENGTH);
	thisDetector->receiver_framesPerFile = -1;
	thisDetector->detectorControlAPIVersion = 0;
	thisDetector->detectorStopAPIVersion = 0;
	thisDetector->receiverAPIVersion = 0;
	thisDetector->receiver_frameDiscardMode = NO_DISCARD;
	thisDetector->receiver_framePadding = 1;
	thisDetector->activated = true;
	thisDetector->receiver_deactivatedPaddingEnable = true;
	thisDetector->receiver_silentMode = false;

	// get the detector parameters based on type
	detParameterList detlist;
	setDetectorSpecificParameters(type, detlist);
	thisDetector->nModMax[X] = detlist.nModMaxX;
	thisDetector->nModMax[Y] = detlist.nModMaxY;
	thisDetector->nChan[X] = detlist.nChanX;
	thisDetector->nChan[Y] = detlist.nChanY;
	thisDetector->nChip[X] = detlist.nChipX;
	thisDetector->nChip[Y] = detlist.nChipY;
	thisDetector->nDacs = detlist.nDacs;
	thisDetector->nAdcs = detlist.nAdcs;
	thisDetector->nGain = detlist.nGain;
	thisDetector->nOffset = detlist.nOffset;
	thisDetector->dynamicRange = detlist.dynamicRange;
	thisDetector->moveFlag = detlist.moveFlag;
	thisDetector->nGappixels[X] = detlist.nGappixelsX;
	thisDetector->nGappixels[Y] = detlist.nGappixelsY;


	// derived parameters
	thisDetector->nModsMax = thisDetector->nModMax[X] * thisDetector->nModMax[Y];
	thisDetector->nChans = thisDetector->nChan[X] * thisDetector->nChan[Y];
	thisDetector->nChips = thisDetector->nChip[X] * thisDetector->nChip[Y];
	// number of modules is initally the maximum number of modules
	thisDetector->nMod[X] = thisDetector->nModMax[X];
	thisDetector->nMod[Y] = thisDetector->nModMax[Y];
	thisDetector->nMods = thisDetector->nModsMax;

	// calculating databytes
	thisDetector->dataBytes = thisDetector->nMods * thisDetector->nChips *
			thisDetector->nChans * thisDetector->dynamicRange/8;
	thisDetector->dataBytesInclGapPixels =
			(thisDetector->nMod[X] * thisDetector->nChip[X] * thisDetector->nChan[X] +
					thisDetector->gappixels * thisDetector->nGappixels[X]) *
					(thisDetector->nMod[Y] * thisDetector->nChip[Y] * thisDetector->nChan[Y] +
							thisDetector->gappixels * thisDetector->nGappixels[Y]) *
							thisDetector->dynamicRange/8;

	// special for jctb and mythen in 24 bit/ with probes
	if(thisDetector->myDetectorType==JUNGFRAUCTB){
		getTotalNumberOfChannels();
	}
	else if(thisDetector->myDetectorType == MYTHEN){
		if (thisDetector->dynamicRange == 24 || thisDetector->timerValue[PROBES_NUMBER] > 0) {
			thisDetector->dataBytes = thisDetector->nMods * thisDetector->nChips *
					thisDetector->nChans * 4;
			thisDetector->dataBytesInclGapPixels =
					(thisDetector->nMod[X] * thisDetector->nChip[X] * thisDetector->nChan[X] +
							thisDetector->gappixels * thisDetector->nGappixels[X]) *
							(thisDetector->nMod[Y] * thisDetector->nChip[Y] *
									thisDetector->nChan[Y] +
									thisDetector->gappixels * thisDetector->nGappixels[Y]) *
									4;
		}
	}

	/** calculates the memory offsets for
	 * flat field coefficients and errors,
	 * module structures, dacs, adcs, chips and channels */
	thisDetector->ffoff = sizeof(sharedSlsDetector);
	thisDetector->fferroff = thisDetector->ffoff + sizeof(double) *
			thisDetector->nChans * thisDetector->nChips * thisDetector->nModsMax;
	thisDetector->modoff = thisDetector->fferroff + sizeof(double) *
			thisDetector->nChans * thisDetector->nChips * thisDetector->nModsMax;
	thisDetector->dacoff = thisDetector->modoff +
			sizeof(sls_detector_module) * thisDetector->nModsMax;
	thisDetector->adcoff = thisDetector->dacoff +
			sizeof(dacs_t) * thisDetector->nDacs * thisDetector->nModsMax;
	thisDetector->chipoff = thisDetector->adcoff +
			sizeof(dacs_t) * thisDetector->nAdcs * thisDetector->nModsMax;
	thisDetector->chanoff = thisDetector->chipoff +
			sizeof(int) * thisDetector->nChips * thisDetector->nModsMax;
	thisDetector->gainoff = thisDetector->chanoff +
			sizeof(int) * thisDetector->nGain * thisDetector->nModsMax;
	thisDetector->offsetoff = thisDetector->gainoff +
			sizeof(int) * thisDetector->nOffset * thisDetector->nModsMax;

}


void slsDetector::initializeMembers() {
	// slsdetector
	// assign addresses
	char *goff = (char*)thisDetector;
	ffcoefficients = (double*)(goff + thisDetector->ffoff);
	fferrors = (double*)(goff + thisDetector->fferroff);
	detectorModules = (sls_detector_module*)(goff + thisDetector->modoff);
	dacs = (dacs_t*)(goff + thisDetector->dacoff);
	adcs = (dacs_t*)(goff + thisDetector->adcoff);
	chipregs = (int*)(goff + thisDetector->chipoff);
	chanregs = (int*)(goff + thisDetector->chanoff);
	gain = (int*)(goff + thisDetector->gainoff);
	offset = (int*)(goff + thisDetector->offsetoff);
	if (thisReceiver) {
		delete thisReceiver;
		thisReceiver = 0;
	}
	thisReceiver = new receiverInterface(dataSocket);


	// slsDetectorUtils
	stoppedFlag = &thisDetector->stoppedFlag;
	timerValue = thisDetector->timerValue;
	currentSettings = &thisDetector->currentSettings;
	currentThresholdEV = &thisDetector->currentThresholdEV;

	// fileIO
	filePath = thisDetector->filePath;
	fileName=multiDet->fileName;
	fileIndex=multiDet->fileIndex;
	framesPerFile=multiDet->framesPerFile;
	fileFormatType=multiDet->fileFormatType;

	if (thisDetector->myDetectorType != MYTHEN)
		fileIO::setFileFormat(BINARY);
	switch(thisDetector->myDetectorType) {
	case GOTTHARD:
	case PROPIX:
		fileIO::setFramesPerFile(MAX_FRAMES_PER_FILE);
		break;
	case EIGER:
		fileIO::setFramesPerFile(EIGER_MAX_FRAMES_PER_FILE);
		break;
	case MOENCH:
		fileIO::setFramesPerFile(MOENCH_MAX_FRAMES_PER_FILE);
		break;
	case JUNGFRAU:
		fileIO::setFramesPerFile(JFRAU_MAX_FRAMES_PER_FILE);
		break;
	case JUNGFRAUCTB:
		fileIO::setFramesPerFile(JFRAU_MAX_FRAMES_PER_FILE);
		break;
	default:
		break;
	}


	//postProcessing
	threadedProcessing = &thisDetector->threadedProcessing;
	correctionMask = &thisDetector->correctionMask;
	flatFieldDir = thisDetector->flatFieldDir;
	flatFieldFile = thisDetector->flatFieldFile;
	expTime = &timerValue[ACQUISITION_TIME];
	badChannelMask = NULL;
	fdata = NULL;
	thisData = NULL;


	// slsDetectorActions
	actionMask = &thisDetector->actionMask;
	actionScript = thisDetector->actionScript;
	actionParameter = thisDetector->actionParameter;
	nScanSteps = thisDetector->nScanSteps;
	scanSteps = thisDetector->scanSteps;
	scanMode = thisDetector->scanMode;
	scanPrecision = thisDetector->scanPrecision;
	scanScript = thisDetector->scanScript;
	scanParameter = thisDetector->scanParameter;

	// angularConversion
	numberOfPositions = &thisDetector->numberOfPositions;
	detPositions = thisDetector->detPositions;
	angConvFile = thisDetector->angConvFile;
	binSize = &thisDetector->binSize;
	fineOffset = &thisDetector->fineOffset;
	globalOffset = &thisDetector->globalOffset;
	angDirection = &thisDetector->angDirection;
	moveFlag = &thisDetector->moveFlag;
	sampleDisplacement = NULL;

	// badChannelCorrections
	badChanFile = thisDetector->badChanFile;
	nBadChans = &thisDetector->nBadChans;
	badChansList = thisDetector->badChansList;
	nBadFF = &thisDetector->nBadFF;
	badFFList = thisDetector->badFFList;

	//energyConversion
	settingsFile = thisDetector->settingsFile;
}




void slsDetector::initializeDetectorStructurePointers() {
	sls_detector_module* thisMod;
	for (int imod = 0; imod < thisDetector->nModsMax; ++imod) {

		// initializes the ffcoefficients values to 0
		for (int i = 0; i < thisDetector->nChans * thisDetector->nChips; ++i) {
			*(ffcoefficients + i + thisDetector->nChans * thisDetector->nChips * imod) = 0;
		}
		// initializes the fferrors values to 0
		for (int i = 0; i < thisDetector->nChans * thisDetector->nChips; ++i) {
			*(fferrors + i + thisDetector->nChans * thisDetector->nChips * imod) = 0;
		}
		// set thisMod to point to one of the detector structure modules
		thisMod = detectorModules + imod;

		thisMod->module = imod;
		thisMod->serialnumber = 0;
		thisMod->nchan = thisDetector->nChans*thisDetector->nChips;
		thisMod->nchip = thisDetector->nChips;
		thisMod->ndac = thisDetector->nDacs;
		thisMod->nadc = thisDetector->nAdcs;
		thisMod->reg = 0;
		// dacs, adcs, chipregs and chanregs for thisMod is not allocated in
		// detectorModules in shared memory as they are already allocated separately
		// in shared memory (below)
		thisMod->gain = -1.;
		thisMod->offset = -1.;

		// initializes the dacs values to 0
		for (int i = 0; i < thisDetector->nDacs; ++i) {
			*(dacs + i + thisDetector->nDacs * imod) = 0;
		}
		// initializes the adc values to 0
		for (int i = 0; i < thisDetector->nAdcs; ++i) {
			*(adcs + i + thisDetector->nAdcs * imod) = 0;
		}
		// initializes the chip registers to 0
		for (int i = 0; i < thisDetector->nChips; ++i) {
			*(chipregs + i + thisDetector->nChips * imod) = -1;
		}
		// initializes the channel registers to 0
		for (int i = 0; i < thisDetector->nChans * thisDetector->nChips; ++i) {
			*(chanregs + i + thisDetector->nChans * thisDetector->nChips * imod) = -1;
		}
		// initializes the gain values to 0
		for (int i = 0; i < thisDetector->nGain; ++i) {
			*(gain + i + thisDetector->nGain * imod) = 0;
		}
		// initializes the offset values to 0
		for (int i = 0; i < thisDetector->nOffset; ++i) {
			*(offset + i + thisDetector->nOffset * imod) = 0;
		}
	}
}


slsDetectorDefs::sls_detector_module*  slsDetector::createModule() {
	return createModule(thisDetector->myDetectorType);
}


slsDetectorDefs::sls_detector_module*  slsDetector::createModule(detectorType type) {
	// get the detector parameters based on type
	detParameterList detlist;
	int nch = 0, nc = 0, nd = 0, na = 0;
	try {
		setDetectorSpecificParameters(type, detlist);
		nch = detlist.nChanX * detlist.nChanY;
		nc = detlist.nChipX * detlist.nChipY;
		nd = detlist.nDacs;
		na = detlist.nAdcs;
	} catch(...) {
		return NULL;
	}
	dacs_t *dacs=new dacs_t[nd];
	dacs_t *adcs=new dacs_t[na];
	int *chipregs=new int[nc];
	int *chanregs=new int[nch*nc];

	sls_detector_module *myMod = (sls_detector_module*)malloc(sizeof(sls_detector_module));
	myMod->ndac=nd;
	myMod->nadc=na;
	myMod->nchip=nc;
	myMod->nchan=nch*nc;
	myMod->dacs=dacs;
	myMod->adcs=adcs;
	myMod->chipregs=chipregs;
	myMod->chanregs=chanregs;
	return myMod;
}


void  slsDetector::deleteModule(sls_detector_module *myMod) {
	delete [] myMod->dacs;
	delete [] myMod->adcs;
	delete [] myMod->chipregs;
	delete [] myMod->chanregs;
	delete myMod;
}




int slsDetector::connectControl() {
	if (controlSocket){
		if (controlSocket->Connect() >= 0)
			return OK;
		else{
			std::cout << "cannot connect to detector" << endl;
			setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
			return FAIL;
		}
	}
	return UNDEFINED;
}

void slsDetector::disconnectControl() {
	if (controlSocket)
		controlSocket->Disconnect();
}



int slsDetector::connectData() {
	if (dataSocket){
		if (dataSocket->Connect() >= 0)
			return OK;
		else{
			std::cout << "cannot connect to receiver" << endl;
			setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
			return FAIL;}
	}
	return UNDEFINED;
}

void slsDetector::disconnectData() {
	if (dataSocket)
		dataSocket->Disconnect();
}



int slsDetector::connectStop() {
	if (stopSocket){
		if (stopSocket->Connect() >= 0)
			return OK;
		else{
			std::cout << "cannot connect to stop server" << endl;
			setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
			return FAIL;
		}
	}
	return UNDEFINED;
}

void slsDetector::disconnectStop() {
	if (stopSocket)
		stopSocket->Disconnect();
}



int slsDetector::sendChannel(sls_detector_channel *myChan) {
	int ts=0;
	ts+=controlSocket->SendDataOnly(&(myChan->chan),sizeof(myChan->chan));
	ts+=controlSocket->SendDataOnly(&(myChan->chip),sizeof(myChan->chip));
	ts+=controlSocket->SendDataOnly(&(myChan->module),sizeof(myChan->module));
	ts=controlSocket->SendDataOnly(&(myChan->reg),sizeof(myChan->reg));
	return ts;
}

int slsDetector::sendChip(sls_detector_chip *myChip) {
	int ts=0;
	//send chip structure
	ts+=controlSocket->SendDataOnly(&(myChip->chip),sizeof(myChip->chip));
	ts+=controlSocket->SendDataOnly(&(myChip->module),sizeof(myChip->module));
	ts+=controlSocket->SendDataOnly(&(myChip->nchan),sizeof(myChip->nchan));
	ts+=controlSocket->SendDataOnly(&(myChip->reg),sizeof(myChip->reg));
	ts+=controlSocket->SendDataOnly(myChip->chanregs,sizeof(myChip->chanregs));
#ifdef VERY_VERBOSE
	std::cout<< "chip structure sent" << std::endl;
	std::cout<< "now sending " << myChip->nchan << " channles" << std::endl;
#endif
	ts=controlSocket->SendDataOnly(myChip->chanregs,sizeof(int)*myChip->nchan );
#ifdef VERBOSE
	std::cout<< "chip's channels sent " <<ts<< std::endl;
#endif
	return ts;
}

int slsDetector::sendModule(sls_detector_module *myMod) {
	int ts=0;
	//send module structure
	ts+=controlSocket->SendDataOnly(&(myMod->module),sizeof(myMod->module));
	ts+=controlSocket->SendDataOnly(&(myMod->serialnumber),sizeof(myMod->serialnumber));
	ts+=controlSocket->SendDataOnly(&(myMod->nchan),sizeof(myMod->nchan));
	ts+=controlSocket->SendDataOnly(&(myMod->nchip),sizeof(myMod->nchip));
	ts+=controlSocket->SendDataOnly(&(myMod->ndac),sizeof(myMod->ndac));
	ts+=controlSocket->SendDataOnly(&(myMod->nadc),sizeof(myMod->nadc));
	ts+=controlSocket->SendDataOnly(&(myMod->reg),sizeof(myMod->reg));
	// only for sending structures like in old mythen server
	if (thisDetector->myDetectorType == MYTHEN) {
		ts+=controlSocket->SendDataOnly(myMod->dacs,sizeof(myMod->ndac));
		ts+=controlSocket->SendDataOnly(myMod->adcs,sizeof(myMod->nadc));
		ts+=controlSocket->SendDataOnly(myMod->chipregs,sizeof(myMod->nchip));
		ts+=controlSocket->SendDataOnly(myMod->chanregs,sizeof(myMod->nchan));
	}
	ts+=controlSocket->SendDataOnly(&(myMod->gain),sizeof(myMod->gain));
	ts+=controlSocket->SendDataOnly(&(myMod->offset), sizeof(myMod->offset));

	// actual data to the pointers
	ts+=controlSocket->SendDataOnly(myMod->dacs,sizeof(dacs_t)*(myMod->ndac));
	ts+=controlSocket->SendDataOnly(myMod->adcs,sizeof(dacs_t)*(myMod->nadc));
	if(thisDetector->myDetectorType != JUNGFRAU){
		ts+=controlSocket->SendDataOnly(myMod->chipregs,sizeof(int)*(myMod->nchip));
		ts+=controlSocket->SendDataOnly(myMod->chanregs,sizeof(int)*(myMod->nchan));
	}
	return ts;
}

int slsDetector::receiveChannel(sls_detector_channel *myChan) {
	int ts=0;
	ts+=controlSocket->ReceiveDataOnly(&(myChan->chan),sizeof(myChan->chan));
	ts+=controlSocket->ReceiveDataOnly(&(myChan->chip),sizeof(myChan->chip));
	ts+=controlSocket->ReceiveDataOnly(&(myChan->module),sizeof(myChan->module));
	ts=controlSocket->ReceiveDataOnly(&(myChan->reg),sizeof(myChan->reg));
	return ts;
}

int slsDetector::receiveChip(sls_detector_chip* myChip) {
	int *ptr=myChip->chanregs;
	int nchanold=myChip->nchan;
	int ts=0;
	int nch;

	//receive chip structure
	ts+=controlSocket->ReceiveDataOnly(&(myChip->chip),sizeof(myChip->chip));
	ts+=controlSocket->ReceiveDataOnly(&(myChip->module),sizeof(myChip->module));
	ts+=controlSocket->ReceiveDataOnly(&(myChip->nchan),sizeof(myChip->nchan));
	ts+=controlSocket->ReceiveDataOnly(&(myChip->reg),sizeof(myChip->reg));
	ts+=controlSocket->ReceiveDataOnly(myChip->chanregs,sizeof(myChip->chanregs));

	myChip->chanregs=ptr;
	if (nchanold<(myChip->nchan)) {
		nch=nchanold;
		printf("number of channels received is too large!\n");
	} else
		nch=myChip->nchan;

	ts+=controlSocket->ReceiveDataOnly(myChip->chanregs,sizeof(int)*nch);

	return ts;
}

int  slsDetector::receiveModule(sls_detector_module* myMod) {

	dacs_t *dacptr=myMod->dacs;
	dacs_t *adcptr=myMod->adcs;
	int *chipptr=myMod->chipregs;
	int *chanptr=myMod->chanregs;
	int ts=0;
	//send module structure
	ts+=controlSocket->ReceiveDataOnly(&(myMod->module),sizeof(myMod->module));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->serialnumber),sizeof(myMod->serialnumber));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->nchan),sizeof(myMod->nchan));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->nchip),sizeof(myMod->nchip));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->ndac),sizeof(myMod->ndac));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->nadc),sizeof(myMod->nadc));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->reg),sizeof(myMod->reg));
	// only for sending structures like in old mythen server
	if (thisDetector->myDetectorType == MYTHEN) {
		ts+=controlSocket->ReceiveDataOnly(myMod->dacs,sizeof(myMod->ndac));
		ts+=controlSocket->ReceiveDataOnly(myMod->adcs,sizeof(myMod->nadc));
		ts+=controlSocket->ReceiveDataOnly(myMod->chipregs,sizeof(myMod->nchip));
		ts+=controlSocket->ReceiveDataOnly(myMod->chanregs,sizeof(myMod->nchan));
	}

	ts+=controlSocket->ReceiveDataOnly(&(myMod->gain), sizeof(myMod->gain));
	ts+=controlSocket->ReceiveDataOnly(&(myMod->offset), sizeof(myMod->offset));


	myMod->dacs=dacptr;
	myMod->adcs=adcptr;
	myMod->chipregs=chipptr;
	myMod->chanregs=chanptr;

#ifdef VERBOSE
	std::cout<< "received module " << myMod->module << " of size "<< ts
			<< " register " << myMod->reg << std::endl;
#endif
	ts+=controlSocket->ReceiveDataOnly(myMod->dacs,sizeof(dacs_t)*(myMod->ndac));
#ifdef VERBOSE
	std::cout<< "received dacs " << myMod->module << " of size "<< ts << std::endl;
#endif
	ts+=controlSocket->ReceiveDataOnly(myMod->adcs,sizeof(dacs_t)*(myMod->nadc));
#ifdef VERBOSE
	std::cout<< "received adcs " << myMod->module << " of size "<< ts << std::endl;
#endif

	if(thisDetector->myDetectorType != JUNGFRAU){
		ts+=controlSocket->ReceiveDataOnly(myMod->chipregs,sizeof(int)*(myMod->nchip));
#ifdef VERBOSE
		std::cout<< "received chips " << myMod->module << " of size "<< ts << std::endl;
#endif
		ts+=controlSocket->ReceiveDataOnly(myMod->chanregs,sizeof(int)*(myMod->nchan));
#ifdef VERBOSE
		std::cout<< "nchans= " << thisDetector->nChans << " nchips= " << thisDetector->nChips;
		std::cout<< "mod - nchans= " << myMod->nchan << " nchips= " <<myMod->nchip;
		std::cout<< "received chans " << myMod->module << " of size "<< ts << std::endl;
#endif
	}

#ifdef VERBOSE
	std::cout<< "received module " << myMod->module << " of size "<< ts << " register "
			<< myMod->reg << std::endl;
#endif

	return ts;
}


slsReceiverDefs::detectorType slsDetector::getDetectorTypeFromShm(int multiId, bool verify) {

	detectorType type = GENERIC;
	SharedMemory* shm = 0;

	try {
		// create
		shm = new SharedMemory(multiId, detId);

		// shm not created before
		if (!shm->IsExisting()) {
			cprintf(RED,"Shared memory %s does not exist.\n"
					"Corrupted Multi Shared memory. Please free shared memory.\n",
					shm->GetName().c_str());
			throw SharedMemoryException();
		}

		// only basic size of structure (just version is required)
		sharedSlsDetector* sdet = 0;
		size_t sz = sizeof(sharedSlsDetector);

		// open, map, verify version
		sdet = (sharedSlsDetector*)shm->OpenSharedMemory(sz);
		if (verify && sdet->shmversion != SLS_SHMVERSION) {
			cprintf(RED, "Single shared memory (%d-%d:)version mismatch "
					"(expected 0x%x but got 0x%x)\n",
					multiId, detId, SLS_SHMVERSION, sdet->shmversion);
			// unmap and throw
			sharedMemory->UnmapSharedMemory(thisDetector);
			throw SharedMemoryException();
		}

		// get type, unmap
		type = sdet->myDetectorType;
		shm->UnmapSharedMemory(sdet);
		delete shm;

	} catch (...) {
		if (shm)
			delete shm;
		throw;
	}

	return type;
}


slsDetectorDefs::detectorType slsDetector::getDetectorType(const char *name, int cport) {
	int fnum=F_GET_DETECTOR_TYPE;
	int retval = FAIL;
	detectorType t = GENERIC;
	MySocketTCP* mySocket = 0;

	try {
		mySocket = new MySocketTCP(name, cport);
	} catch(...) {
		cout << "Cannot create socket to server " << name << " over port " << cport << endl;
		return t;
	}


	char m[MAX_STR_LENGTH];
#ifdef VERBOSE
	cout << "Getting detector type " << endl;
#endif
	if (mySocket->Connect() >= 0) {
		mySocket->SendDataOnly(&fnum,sizeof(fnum));
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));
		if (retval!=FAIL) {
			mySocket->ReceiveDataOnly(&t,sizeof(t));
#ifdef VERBOSE
			cout << "Detector type is "<< t << endl;
#endif
		} else {
			mySocket->ReceiveDataOnly(m,sizeof(m));
			std::cout<< "Detector returned error: " << m << std::endl;
		}
		mySocket->Disconnect();
	} else {
		cout << "Cannot connect to server " << name << " over port " << cport << endl;
	}
	delete mySocket;
	return t;
}


int slsDetector::setDetectorType(detectorType const type) {

	int arg, retval=FAIL;
	int fnum=F_GET_DETECTOR_TYPE,fnum2=F_GET_RECEIVER_TYPE;
	arg=int(type);
	detectorType retType=type;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting detector type to " << arg << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			if (retval!=FAIL)
				controlSocket->ReceiveDataOnly(&retType,sizeof(retType));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (retval==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		if (type==GET_DETECTOR_TYPE)
			retType=thisDetector->myDetectorType;//FIXME: throw exception?
		else {
			retType=type;
			thisDetector->myDetectorType=type;
		}
		retval=OK;
	}
#ifdef VERBOSE
	std::cout<< "Detector type set to " << retType << std::endl;
#endif
	if (retval==FAIL) {
		std::cout<< "Set detector type failed " << std::endl;
		retType=GENERIC;
	}
	else
		thisDetector->myDetectorType=retType;


	//receiver
	if((retType != GENERIC) && (thisDetector->receiverOnlineFlag==ONLINE_FLAG)
			&& (arg != GENERIC)) {
		retval = FAIL;
		if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "Sending detector type to Receiver " <<
					(int)thisDetector->myDetectorType << std::endl;
#endif
			if (connectData() == OK){
				retval=thisReceiver->sendInt(fnum2,arg,(int)thisDetector->myDetectorType);
				disconnectData();
			}
			if(retval==FAIL){
				cout << "ERROR: Could not send detector type to receiver" << endl;
				setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTTYPE_NOT_SET));
			}
		}
	}

	return retType;
}



int slsDetector::setDetectorType(string const stype) {
	return setDetectorType(getDetectorType(stype));
}


slsDetectorDefs::detectorType slsDetector::getDetectorsType(int pos) {
	return thisDetector->myDetectorType;
}

string slsDetector::sgetDetectorsType(int pos) {
	return getDetectorType(getDetectorsType(pos));
}


string slsDetector::getDetectorType() {
	return sgetDetectorsType();
}


int slsDetector::getNMods() {
	return thisDetector->nMods;
}

int slsDetector::getNMod(dimension d) {
	return thisDetector->nMod[d];
}


int slsDetector::getMaxMods() {
	return thisDetector->nModsMax;
}


int slsDetector::getNMaxMod(dimension d) {
	return thisDetector->nModMax[d];
}



int slsDetector::getMaxNumberOfModules(dimension d) {

	int retval;
	int fnum=F_GET_MAX_NUMBER_OF_MODULES;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	if (d<X || d>Y) {
		std::cout<< "Get max number of modules in wrong dimension " << d << std::endl;
		return ret;
	}
#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Getting max number of modules in dimension "<< d  <<std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&d,sizeof(d));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		ret=OK;
		retval=thisDetector->nModMax[d];
	}
#ifdef VERBOSE
	std::cout<< "Max number of modules in dimension "<< d <<" is " << retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Get max number of modules failed " << retval << std::endl;
		return retval;
	}  else {
		thisDetector->nModMax[d]=retval;
		thisDetector->nModsMax=thisDetector->nModMax[X]*thisDetector->nModMax[Y];



	}
	return thisDetector->nModMax[d];
}



int slsDetector::setNumberOfModules(int n, dimension d) {

	int arg[2], retval=1;
	int fnum=F_SET_NUMBER_OF_MODULES;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="dummy";
	int connect;

	arg[0]=d;
	arg[1]=n;


	if (d<X || d>Y) {
		std::cout<< "Set number of modules in wrong dimension " << d << std::endl;
		return ret;
	}


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting number of modules of dimension "<< d <<  " to " << n << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		connect = connectControl();
		if (connect == UNDEFINED)
			cout << "no control socket?" << endl;
		else if (connect == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}	else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		cout << "offline" << endl;
		ret=OK;
		if (n==GET_FLAG)
			;
		else {
			if (n<=0 || n>thisDetector->nModMax[d]) {
				ret=FAIL;
			} else {
				thisDetector->nMod[d]=n;
			}
		}
		retval=thisDetector->nMod[d];
	}
#ifdef VERBOSE
	std::cout<< "Number of modules in dimension "<< d <<" is " << retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Set number of modules failed " << std::endl;
	}  else {
		thisDetector->nMod[d]=retval;
		thisDetector->nMods=thisDetector->nMod[X]*thisDetector->nMod[Y];


		if (thisDetector->nModsMax<thisDetector->nMods)
			thisDetector->nModsMax=thisDetector->nMods;

		if (thisDetector->nModMax[X]<thisDetector->nMod[X])
			thisDetector->nModMax[X]=thisDetector->nMod[X];

		if (thisDetector->nModMax[Y]<thisDetector->nMod[Y])
			thisDetector->nModMax[Y]=thisDetector->nMod[Y];

		int dr=thisDetector->dynamicRange;
		if ((thisDetector->myDetectorType==MYTHEN) && (dr==24))
			dr=32;

		thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*
				thisDetector->nChips*thisDetector->nChans*dr/8;
		thisDetector->dataBytesInclGapPixels =
				(thisDetector->nMod[X] * thisDetector->nChip[X] *
						thisDetector->nChan[X] + thisDetector->gappixels *
						thisDetector->nGappixels[X]) *
				(thisDetector->nMod[Y] * thisDetector->nChip[Y] *
						thisDetector->nChan[Y] + thisDetector->gappixels *
						thisDetector->nGappixels[Y]) *
				thisDetector->dynamicRange/8;

		if(thisDetector->myDetectorType==MYTHEN){
			if (thisDetector->timerValue[PROBES_NUMBER]!=0) {
				thisDetector->dataBytes=thisDetector->nMod[X]*
						thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
				thisDetector->dataBytesInclGapPixels =
						(thisDetector->nMod[X] * thisDetector->nChip[X] * thisDetector->nChan[X] + thisDetector->gappixels * thisDetector->nGappixels[X]) *
						(thisDetector->nMod[Y] * thisDetector->nChip[Y] * thisDetector->nChan[Y] + thisDetector->gappixels * thisDetector->nGappixels[Y]) *
						4;
			}
		}

		if(thisDetector->myDetectorType==JUNGFRAUCTB){
			getTotalNumberOfChannels();
		}


#ifdef VERBOSE
std::cout<< "Data size is " << thisDetector->dataBytes << std::endl;
std::cout<< "nModX " << thisDetector->nMod[X] << " nModY " << thisDetector->nMod[Y]
		<< " nChips " << thisDetector->nChips << " nChans " << thisDetector->nChans
		<< " dr " << dr << std::endl;
#endif
	}

	if(n != GET_FLAG){
		pthread_mutex_lock(&ms);
		multiDet->updateOffsets();
		pthread_mutex_unlock(&ms);
	}

	return thisDetector->nMod[d];
}



int slsDetector::getChansPerMod(int imod) {
	return thisDetector->nChans*thisDetector->nChips;
}

int slsDetector::getChansPerMod( dimension d,int imod) {
	return thisDetector->nChan[d]*thisDetector->nChip[d];
}


int slsDetector::getTotalNumberOfChannels() {
#ifdef VERBOSE
	cout << "total number of channels" << endl;
#endif
	if(thisDetector->myDetectorType==JUNGFRAUCTB){
		if (thisDetector->roFlags&DIGITAL_ONLY)
			thisDetector->nChan[X]=4;
		else if (thisDetector->roFlags&ANALOG_AND_DIGITAL)
			thisDetector->nChan[X]=36;
		else
			thisDetector->nChan[X]=32;

		if (thisDetector->nChan[X]>=32) {
			if (thisDetector->nROI>0) {
				thisDetector->nChan[X]-=32;
				for (int iroi=0; iroi<thisDetector->nROI; ++iroi)
					thisDetector->nChan[X]+=
							thisDetector->roiLimits[iroi].xmax-
							thisDetector->roiLimits[iroi].xmin+1;
			}
		}
		thisDetector->nChans=thisDetector->nChan[X];
		thisDetector->dataBytes=thisDetector->nChans*thisDetector->nChips*
				thisDetector->nMods*2*thisDetector->timerValue[SAMPLES_JCTB];
		thisDetector->dataBytesInclGapPixels = thisDetector->dataBytes;
	} else {
#ifdef VERBOSE
cout << "det type is "<< thisDetector->myDetectorType << endl;
cout << "Total number of channels is "<< thisDetector->nChans*thisDetector->nChips*
		thisDetector->nMods << " data bytes is " << thisDetector->dataBytes << endl;
// excluding gap pixels
#endif
;
	}
	return thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
}

int slsDetector::getTotalNumberOfChannels(dimension d) {
	getTotalNumberOfChannels();
	return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nMod[d];
}

int slsDetector::getTotalNumberOfChannelsInclGapPixels(dimension d) {
	getTotalNumberOfChannels();
	return (thisDetector->nChan[d] * thisDetector->nChip[d] + thisDetector->gappixels
			* thisDetector->nGappixels[d]) * thisDetector->nMod[d];
}


int slsDetector::getMaxNumberOfChannels() {
	if(thisDetector->myDetectorType==JUNGFRAUCTB) return 36*thisDetector->nChips*
			thisDetector->nModsMax;
	return thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;
}

int slsDetector::getMaxNumberOfChannels(dimension d) {
	if(thisDetector->myDetectorType==JUNGFRAUCTB) {
		if (d==X) return 36*thisDetector->nChip[d]*thisDetector->nModMax[d];
		else return 1*thisDetector->nChip[d]*thisDetector->nModMax[d];
	}
	return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nModMax[d];
};

int slsDetector::getMaxNumberOfChannelsInclGapPixels(dimension d) {
	if(thisDetector->myDetectorType==JUNGFRAUCTB) {
		if (d==X) return 36*thisDetector->nChip[d]*thisDetector->nModMax[d];
		else return 1*thisDetector->nChip[d]*thisDetector->nModMax[d];
	}
	return (thisDetector->nChan[d] * thisDetector->nChip[d] + thisDetector->gappixels *
			thisDetector->nGappixels[d]) * thisDetector->nModMax[d];
}


int slsDetector::getNChans() {
	return thisDetector->nChans;
}

int slsDetector::getNChans(dimension d) {
	return thisDetector->nChan[d];
}

int slsDetector::getNChips() {
	return thisDetector->nChips;
}

int slsDetector::getNChips(dimension d) {
	return thisDetector->nChip[d];
}

int slsDetector::getDetectorOffset(dimension d) {
	return thisDetector->offset[d];
}

void slsDetector::setDetectorOffset(dimension d, int off) {
	if (off >= 0)
		thisDetector->offset[d] = off;
}

int slsDetector::setOnline(int off) {
	int old=thisDetector->onlineFlag;
	if (off!=GET_ONLINE_FLAG) {
		thisDetector->onlineFlag=off;
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			setTCPSocket();
			if (thisDetector->onlineFlag==ONLINE_FLAG && old==OFFLINE_FLAG) {
				cout << "Detector connecting for the first time - updating!" << endl;
				updateDetector();
			}
			else if(thisDetector->onlineFlag==OFFLINE_FLAG){
				std::cout << "cannot connect to detector" << endl;
				setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
			}
		}
	}
	return thisDetector->onlineFlag;
}



string slsDetector::checkOnline() {
	string retval="";
	if(!controlSocket){
		//this already sets the online/offline flag
		setTCPSocket();
		if(thisDetector->onlineFlag==OFFLINE_FLAG)
			return string(thisDetector->hostname);
		else
			return string("");
	}
	//still cannot connect to socket, controlSocket=0
	if(controlSocket){
		if (connectControl() == FAIL) {
			controlSocket->SetTimeOut(5);
			thisDetector->onlineFlag=OFFLINE_FLAG;
			delete controlSocket;
			controlSocket=0;
			retval = string(thisDetector->hostname);
#ifdef VERBOSE
			std::cout<< "offline!" << std::endl;
#endif
		}  else {
			thisDetector->onlineFlag=ONLINE_FLAG;
			controlSocket->SetTimeOut(100);
			disconnectControl();
#ifdef VERBOSE
			std::cout<< "online!" << std::endl;
#endif
		}
	}
	//still cannot connect to socket, stopSocket=0
	if(stopSocket){
		if (connectStop() == FAIL) {
			stopSocket->SetTimeOut(5);
			thisDetector->onlineFlag=OFFLINE_FLAG;
			delete stopSocket;
			stopSocket=0;
			retval = string(thisDetector->hostname);
#ifdef VERBOSE
			std::cout<< "stop offline!" << std::endl;
#endif
		}  else {
			thisDetector->onlineFlag=ONLINE_FLAG;
			stopSocket->SetTimeOut(100);
			disconnectStop();
#ifdef VERBOSE
			std::cout<< "stop online!" << std::endl;
#endif
		}
	}
	return retval;
}




int slsDetector::setTCPSocket(string const name, int const control_port, int const stop_port) {
	char thisName[MAX_STR_LENGTH];
	int thisCP, thisSP;
	int retval=OK;

	if (strcmp(name.c_str(),"")!=0) {
#ifdef VERBOSE
		std::cout<< "setting hostname" << std::endl;
#endif
		strcpy(thisName,name.c_str());
		strcpy(thisDetector->hostname,thisName);
		if (controlSocket) {
			delete controlSocket;
			controlSocket=0;
		}
		if (stopSocket) {
			delete stopSocket;
			stopSocket=0;
		}
	} else
		strcpy(thisName,thisDetector->hostname);

	if (control_port>0) {
#ifdef VERBOSE
		std::cout<< "setting control port" << std::endl;
#endif
		thisCP=control_port;
		thisDetector->controlPort=thisCP;
		if (controlSocket) {
			delete controlSocket;
			controlSocket=0;
		}
	} else
		thisCP=thisDetector->controlPort;

	if (stop_port>0) {
#ifdef VERBOSE
		std::cout<< "setting stop port" << std::endl;
#endif
		thisSP=stop_port;
		thisDetector->stopPort=thisSP;
		if (stopSocket) {
			delete stopSocket;
			stopSocket=0;
		}
	} else
		thisSP=thisDetector->stopPort;


	// create control socket
	if (!controlSocket) {
		try {
			controlSocket = new MySocketTCP(thisName, thisCP);
#ifdef VERYVERBOSE
			std::cout<< "Control socket connected " <<
					thisName  << " " << thisCP << std::endl;
#endif
		} catch(...) {
#ifdef VERBOSE
			std::cout<< "Could not connect Control socket " <<
					thisName  << " " << thisCP << std::endl;
#endif
			controlSocket = 0;
			retval = FAIL;
		}
	}


	// create stop socket
	if (!stopSocket) {
		try {
			stopSocket = new MySocketTCP(thisName, thisSP);
#ifdef VERYVERBOSE
			std::cout<< "Stop socket connected " <<
					thisName  << " " << thisSP << std::endl;
#endif
		} catch(...) {
#ifdef VERBOSE
			std::cout<< "Could not connect Stop socket " <<
					thisName  << " " << thisSP << std::endl;
#endif
			stopSocket = 0;
			retval = FAIL;
		}
	}


	if (retval!=FAIL) {
		checkOnline();

		// check for version compatibility
		switch (thisDetector->myDetectorType) {
		case EIGER:
		case JUNGFRAU:
		case GOTTHARD:
			if ((thisDetector->detectorControlAPIVersion == 0) ||
					(thisDetector->detectorStopAPIVersion == 0))    	{
				if (checkVersionCompatibility(CONTROL_PORT) == FAIL)
					thisDetector->onlineFlag=OFFLINE_FLAG;
			}
			break;
		default:
			break;
		}

	} else {
		thisDetector->onlineFlag=OFFLINE_FLAG;
#ifdef VERBOSE
		std::cout<< "offline!" << std::endl;
#endif
	}
	return retval;
}



int slsDetector::setPort(portType index, int num) {

	int fnum=F_SET_PORT, fnum2 = F_SET_RECEIVER_PORT;
	int retval;
	//  uint64_t ut;
	char mess[MAX_STR_LENGTH]="";
	int ret=FAIL;
	bool online=false;
	MySocketTCP *s = 0;

	if (num>1024) {
		switch(index) {
		case CONTROL_PORT:
			s=controlSocket;
			retval=thisDetector->controlPort;
#ifdef VERBOSE
			cout << "s="<< s<< endl;
			cout << thisDetector->controlPort<< " " << " " << thisDetector->stopPort
					<< endl;
#endif
			if (s==0) {

#ifdef VERBOSE
				cout << "s=NULL"<< endl;
				cout << thisDetector->controlPort<< " " << " " << thisDetector->stopPort
						<< endl;
#endif
				setTCPSocket("",DEFAULT_PORTNO);
			}
			if (controlSocket) {
				s=controlSocket;
			} else {
#ifdef VERBOSE
				cout << "still cannot connect!"<< endl;
				cout << thisDetector->controlPort<< " " << " " << thisDetector->stopPort
						<< endl;
#endif

				setTCPSocket("",retval);
			}
			online =  (thisDetector->onlineFlag==ONLINE_FLAG);

			//not an error.could be from config file
			if(num==thisDetector->controlPort)
				return thisDetector->controlPort;
			//reusable port, so print error
			else if((num==thisDetector->stopPort)||(num==thisDetector->receiverTCPPort)){
				std::cout<< "Can not connect to port in use " << std::endl;
				setErrorMask((getErrorMask())|(COULDNOT_SET_CONTROL_PORT));
				return thisDetector->controlPort;
			}
			break;


		case DATA_PORT:
			s=dataSocket;
			retval=thisDetector->receiverTCPPort;
			if(strcmp(thisDetector->receiver_hostname,"none")){
				if (s==0) setReceiverTCPSocket("",retval);
				if (dataSocket)s=dataSocket;
			}
			online =  (thisDetector->receiverOnlineFlag==ONLINE_FLAG);

			//not an error. could be from config file
			if(num==thisDetector->receiverTCPPort)
				return thisDetector->receiverTCPPort;
			//reusable port, so print error
			else if((num==thisDetector->stopPort)||(num==thisDetector->controlPort)){
				std::cout<< "Can not connect to port in use " << std::endl;
				setErrorMask((getErrorMask())|(COULDNOT_SET_DATA_PORT));
				return thisDetector->receiverTCPPort;
			}
			break;


		case STOP_PORT:
			s=stopSocket;
			retval=thisDetector->stopPort;
			if (s==0) setTCPSocket("",-1,DEFAULT_PORTNO+1);
			if (stopSocket) s=stopSocket;
			else setTCPSocket("",-1,retval);
			online =  (thisDetector->onlineFlag==ONLINE_FLAG);

			//not an error. could be from config file
			if(num==thisDetector->stopPort)
				return thisDetector->stopPort;
			//reusable port, so print error
			else if((num==thisDetector->receiverTCPPort)||(num==thisDetector->controlPort)){
				std::cout<< "Can not connect to port in use " << std::endl;
				setErrorMask((getErrorMask())|(COULDNOT_SET_STOP_PORT));
				return thisDetector->stopPort;
			}
			break;

		default:
			s=0;
			break;
		}




		//send to current port to change port
		if (online) {
			if (s) {
				if  (s->Connect()>=0) {
					if(s==dataSocket)
						fnum = fnum2;
					s->SendDataOnly(&fnum,sizeof(fnum));
					s->SendDataOnly(&index,sizeof(index));
					s->SendDataOnly(&num,sizeof(num));
					s->ReceiveDataOnly(&ret,sizeof(ret));
					if (ret==FAIL) {
						s->ReceiveDataOnly(mess,sizeof(mess));
						std::cout<< "Detector returned error: " << mess << std::endl;
					} else {
						s->ReceiveDataOnly(&retval,sizeof(retval));
					}
					s->Disconnect();
				}else{
					if (index == CONTROL_PORT){
						std::cout << "cannot connect to detector" << endl;
						setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
					}else if (index == DATA_PORT){
						std::cout << "cannot connect to receiver" << endl;
						setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
					}
				}
			}
		}



		if (ret!=FAIL) {
			switch(index) {
			case CONTROL_PORT:
				thisDetector->controlPort=retval;
				break;
			case DATA_PORT:
				if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
					thisDetector->receiverTCPPort=retval;
					setReceiverOnline(ONLINE_FLAG);
					setReceiver(thisDetector->receiver_hostname);
				}
				break;
			case STOP_PORT:
				thisDetector->stopPort=retval;
				break;
			default:
				break;
			}
#ifdef VERBOSE
			cout << "ret is ok" << endl;
#endif

		} else {
			switch(index) {
			case CONTROL_PORT:
				thisDetector->controlPort=num;
				setErrorMask((getErrorMask())|(COULDNOT_SET_CONTROL_PORT));
				break;
			case DATA_PORT:
				if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
					thisDetector->receiverTCPPort=retval;
					setErrorMask((getErrorMask())|(COULDNOT_SET_DATA_PORT));
				}else{
					thisDetector->receiverTCPPort=num;
					if(strcmp(thisDetector->receiver_hostname,"none"))
						setReceiver(thisDetector->receiver_hostname);
				}
				break;
			case STOP_PORT:
				thisDetector->stopPort=num;
				setErrorMask((getErrorMask())|(COULDNOT_SET_STOP_PORT));
				break;
			default:
				break;
			}
		}
	}




	switch(index) {
	case CONTROL_PORT:
		retval=thisDetector->controlPort;
		break;
	case DATA_PORT:
		retval=thisDetector->receiverTCPPort;
		break;
	case STOP_PORT:
		retval=thisDetector->stopPort;
		break;
	default:
		retval=-1;
		break;
	}



#ifdef VERBOSE
	cout << thisDetector->controlPort<< " " << thisDetector->receiverTCPPort
			<< " " << thisDetector->stopPort << endl;
#endif



	return retval;

}

int slsDetector::getControlPort() {
	return  thisDetector->controlPort;
}
int slsDetector::getStopPort() {
	return thisDetector->stopPort;
}
int slsDetector::getReceiverPort() {
	return thisDetector->receiverTCPPort;
}


int slsDetector::lockServer(int lock) {
	int fnum=F_LOCK_SERVER;
	int retval=-1;
	int ret=OK;
	char mess[MAX_STR_LENGTH]="";

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&lock,sizeof(lock));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return retval;


}


string slsDetector::getLastClientIP() {

	int fnum=F_GET_LAST_CLIENT_IP;
	char clientName[INET_ADDRSTRLEN];
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			} else {
				controlSocket->ReceiveDataOnly(clientName,sizeof(clientName));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return string(clientName);

}


int slsDetector::exitServer() {

	int retval = FAIL;
	int fnum=F_EXIT_SERVER;

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (controlSocket) {
			controlSocket->Connect();
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
		}
	}
	if (retval!=OK) {
		std::cout<< std::endl;
		std::cout<< "Shutting down the server" << std::endl;
		std::cout<< std::endl;
	}
	return retval;

}


int slsDetector::execCommand(string cmd, string answer) {

	char arg[MAX_STR_LENGTH], retval[MAX_STR_LENGTH];
	int fnum=F_EXEC_COMMAND;

	int ret=FAIL;

	strcpy(arg,cmd.c_str());

#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Sending command " << arg << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			if (controlSocket->SendDataOnly(&fnum,sizeof(fnum))>=0) {
				if (controlSocket->SendDataOnly(arg,MAX_STR_LENGTH)>=0) {
					if (controlSocket->ReceiveDataOnly(retval,MAX_STR_LENGTH)>=0) {
						ret=OK;
						answer=retval;
					}
				}
			}
			disconnectControl();
		}
#ifdef VERBOSE
		std::cout<< "Detector answer is " << answer << std::endl;
#endif
	}
	return ret;
}


int slsDetector::updateDetectorNoWait() {

	enum readOutFlags ro;
	// int ret=OK;
	enum detectorSettings t;
	int thr, n = 0, nm;
	// int it;
	int64_t retval;// tns=-1;
	char lastClientIP[INET_ADDRSTRLEN];

	n += 	controlSocket->ReceiveDataOnly(lastClientIP,sizeof(lastClientIP));
#ifdef VERBOSE
	cout << "Updating detector last modified by " << lastClientIP << std::endl;
#endif
	n += 	controlSocket->ReceiveDataOnly(&nm,sizeof(nm));
	thisDetector->nMod[X]=nm;
	n += 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
	/// Should be overcome at a certain point!

	if (thisDetector->myDetectorType==MYTHEN) {
		thisDetector->nModMax[X]=nm;
		thisDetector->nModMax[Y]=1;
		thisDetector->nModsMax=thisDetector->nModMax[Y]*thisDetector->nModMax[X];
		thisDetector->nMod[Y]=1;
	} else {
		thisDetector->nMod[Y]=nm;
	}

	thisDetector->nMods=thisDetector->nMod[Y]*thisDetector->nMod[X];
	if (thisDetector->nModsMax<thisDetector->nMods)
		thisDetector->nModsMax=thisDetector->nMods;

	if (thisDetector->nModMax[X]<thisDetector->nMod[X])
		thisDetector->nModMax[X]=thisDetector->nMod[X];

	if (thisDetector->nModMax[Y]<thisDetector->nMod[Y])
		thisDetector->nModMax[Y]=thisDetector->nMod[Y];

	n += 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
	thisDetector->dynamicRange=nm;

	n += 	controlSocket->ReceiveDataOnly( &nm,sizeof(nm));
	thisDetector->dataBytes=nm;

	n += 	controlSocket->ReceiveDataOnly( &t,sizeof(t));
	thisDetector->currentSettings=t;

	if((thisDetector->myDetectorType == EIGER) ||
			(thisDetector->myDetectorType == MYTHEN)){
		n += 	controlSocket->ReceiveDataOnly( &thr,sizeof(thr));
		thisDetector->currentThresholdEV=thr;
	}

	n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[FRAME_NUMBER]=retval;

	n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[ACQUISITION_TIME]=retval;

	if(thisDetector->myDetectorType == EIGER){
		n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
		thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME]=retval;

		n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
		thisDetector->timerValue[SUBFRAME_DEADTIME]=retval;
	}

	n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[FRAME_PERIOD]=retval;

	if(thisDetector->myDetectorType != EIGER) {
		n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
		thisDetector->timerValue[DELAY_AFTER_TRIGGER]=retval;
	}

	if ((thisDetector->myDetectorType != JUNGFRAU) &&
			(thisDetector->myDetectorType != EIGER)){
		n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
		thisDetector->timerValue[GATES_NUMBER]=retval;
	}

	if (thisDetector->myDetectorType == MYTHEN){
		n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
		thisDetector->timerValue[PROBES_NUMBER]=retval;
	}

	n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
	thisDetector->timerValue[CYCLES_NUMBER]=retval;

	if (thisDetector->myDetectorType == JUNGFRAUCTB){
		n += 	controlSocket->ReceiveDataOnly( &retval,sizeof(int64_t));
		if (retval>=0)
			thisDetector->timerValue[SAMPLES_JCTB]=retval;
		n += controlSocket->ReceiveDataOnly( &ro,sizeof(ro));

		thisDetector->roFlags=ro;

		getTotalNumberOfChannels();
	}


	if (!n)
		printf("n: %d\n", n);

	return OK;

}




int slsDetector::updateDetector() {
	int fnum=F_UPDATE_CLIENT;
	int ret=OK;
	char mess[MAX_STR_LENGTH]="";

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			} else
				updateDetectorNoWait();
			disconnectControl();
		}
	}
	return ret;
}


int slsDetector::readConfigurationFile(string const fname) {



	string ans;
	string str;
	ifstream infile;
	//char *args[1000];

	string sargname, sargval;
#ifdef VERBOSE
	int iline=0;
	std::cout<< "config file name "<< fname << std::endl;
#endif
	infile.open(fname.c_str(), ios_base::in);
	if (infile.is_open()) {
#ifdef VERBOSE
		iline=readConfigurationFile(infile);
#else
		readConfigurationFile(infile);
#endif
		infile.close();
	} else {
		std::cout<< "Error opening configuration file " << fname <<
				" for reading" << std::endl;
		setErrorMask((getErrorMask())|(CONFIG_FILE));
		return FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Read configuration file of " << iline << " lines" << std::endl;
#endif
	return OK;

}


int slsDetector::readConfigurationFile(ifstream &infile) {




	slsDetectorCommand *cmd=new slsDetectorCommand(this);

	string ans;
	string str;
	int iargval;
	int interrupt=0;
	char *args[100];
	char myargs[1000][1000];

	string sargname, sargval;
	int iline=0;
	while (infile.good() and interrupt==0) {
		sargname="none";
		sargval="0";
		getline(infile,str);
		++iline;
#ifdef VERBOSE
		std::cout<<  str << std::endl;
#endif
		if (str.find('#')!=string::npos) {
#ifdef VERBOSE
			std::cout<< "Line is a comment " << std::endl;
			std::cout<< str << std::endl;
#endif
			continue;
		} else if (str.length()<2) {
#ifdef VERBOSE
			std::cout<< "Empty line " << std::endl;
#endif
			continue;
		} else {
			istringstream ssstr(str);
			iargval=0;
			while (ssstr.good()) {
				ssstr >> sargname;
				//if (ssstr.good()) {
#ifdef VERBOSE
				std::cout<< iargval << " " << sargname  << std::endl;
#endif
				strcpy(myargs[iargval],sargname.c_str());
				args[iargval]=myargs[iargval];
				++iargval;
				//}
			}
			ans=cmd->executeLine(iargval,args,PUT_ACTION);
#ifdef VERBOSE
			std::cout<< ans << std::endl;
#endif
		}
		++iline;
	}
	delete cmd;
	return OK;

}










int slsDetector::writeConfigurationFile(string const fname) {

	ofstream outfile;
#ifdef VERBOSE
	int ret;
#endif
	outfile.open(fname.c_str(),ios_base::out);
	if (outfile.is_open()) {
#ifdef VERBOSE
		ret=writeConfigurationFile(outfile);
#else
		writeConfigurationFile(outfile);
#endif
		outfile.close();
	}
	else {
		std::cout<< "Error opening configuration file " << fname <<
				" for writing" << std::endl;
		setErrorMask((getErrorMask())|(CONFIG_FILE));
		return FAIL;
	}
#ifdef VERBOSE
	std::cout<< "wrote " <<ret << " lines to configuration file " << std::endl;
#endif
	return OK;


}


int slsDetector::writeConfigurationFile(ofstream &outfile, int id) {
	;
	slsDetectorCommand *cmd=new slsDetectorCommand(this);
	detectorType type = thisDetector->myDetectorType;
	string names[100];
	int nvar=0;

	// common config
	names[nvar++] = "hostname";
	names[nvar++] = "port";
	names[nvar++] = "stopport";
	names[nvar++] = "settingsdir";
	names[nvar++] = "caldir";
	names[nvar++] = "ffdir";
	names[nvar++] = "outdir";
	names[nvar++] = "angdir";
	names[nvar++] = "moveflag";
	names[nvar++] = "lock";

	// receiver config
	if (type != MYTHEN) {
		names[nvar++] = "detectormac";
		names[nvar++] = "detectorip";
		names[nvar++] = "zmqport";
		names[nvar++] = "rx_zmqport";
		names[nvar++] = "zmqip";
		names[nvar++] = "rx_zmqip";
		names[nvar++] = "rx_tcpport";
		names[nvar++] = "rx_udpport";
		names[nvar++] = "rx_udpport2";
		names[nvar++] = "rx_udpip";
		names[nvar++] = "rx_hostname";
		names[nvar++] = "r_readfreq";
	}

	// detector specific config
	switch (type) {
	case MYTHEN:
		names[nvar++] = "nmod";
		names[nvar++] = "waitstates";
		names[nvar++] = "setlength";
		names[nvar++] = "clkdivider";
		names[nvar++] = "extsig";
		break;
	case GOTTHARD:
	case PROPIX:
		names[nvar++] = "extsig";
		names[nvar++] = "vhighvoltage";
		break;
		break;
	case MOENCH:
		names[nvar++] = "extsig";
		names[nvar++] = "vhighvoltage";
		break;
	case EIGER:
		names[nvar++] = "vhighvoltage";
		names[nvar++] = "trimen";
		names[nvar++] = "iodelay";
		names[nvar++] = "tengiga";
		break;
	case JUNGFRAU:
		names[nvar++] = "powerchip";
		names[nvar++] = "vhighvoltage";
		break;
	case JUNGFRAUCTB:
		names[nvar++] = "powerchip";
		names[nvar++] = "vhighvoltage";
		break;
	default:
		std::cout << "detector type " <<
		getDetectorType(thisDetector->myDetectorType) << " not implemented in "
				"writing config file" << std::endl;
		nvar = 0;
		break;
	}



	int nsig=4;
	int iv=0;
	char *args[100];
	char myargs[100][1000];

	for (int ia=0; ia<100; ++ia) {
		args[ia]=myargs[ia];
	}


	for (iv=0; iv<nvar; ++iv) {
		cout << iv << " " << names[iv] << endl;
		if (names[iv]=="extsig") {
			for (int is=0; is<nsig; ++is) {
				sprintf(args[0],"%s:%d",names[iv].c_str(),is);

				if (id>=0)
					outfile << id << ":";

				outfile << args[0] << " " << cmd->executeLine(1,args,GET_ACTION)
						<< std::endl;
			}
		} else {
			strcpy(args[0],names[iv].c_str());
			if (id>=0)
				outfile << id << ":";
			outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION)
					<< std::endl;
		}
	}

	delete cmd;
	return OK;
}



string slsDetector::getSettingsFile() {
	string s(thisDetector->settingsFile);
	if (s.length()>6) {
		if (s.substr(s.length()-6,3)==string(".sn") && s.substr(s.length()-3)!=string("xxx") )
		return s.substr(0,s.length()-6);
	}
	return string(thisDetector->settingsFile);
}


int slsDetector::writeSettingsFile(string fname, int imod, int iodelay, int tau) {

	return writeSettingsFile(fname,thisDetector->myDetectorType, detectorModules[imod],
			iodelay, tau);

}

slsDetectorDefs::detectorSettings  slsDetector::getSettings(int imod) {

	return sendSettingsOnly(GET_SETTINGS, imod);
}


slsDetectorDefs::detectorSettings slsDetector::setSettings( detectorSettings isettings,
		int imod) {
#ifdef VERBOSE
	std::cout<< "slsDetector setSettings " << isettings << std::endl;
#endif

	if (isettings == -1)
		return getSettings(imod);

	detectorType detType = thisDetector->myDetectorType;
	switch (detType) {

	// eiger: only set client shared memory variable for Eiger,
	// settings threshold loads the module data (trimbits, dacs etc.)
	case EIGER:
		switch(isettings) {
		case STANDARD:
		case HIGHGAIN:
		case LOWGAIN:
		case VERYHIGHGAIN:
		case VERYLOWGAIN:
			thisDetector->currentSettings = isettings;
			break;
		default:
			printf("Unknown settings %s for this detector!\n",
					getDetectorSettings(isettings).c_str());
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			break;
		}
		return thisDetector->currentSettings;

		// send only the settings, detector server will update dac values already in server
		case GOTTHARD:
		case PROPIX:
		case JUNGFRAU:
		case MOENCH:
			return sendSettingsOnly(isettings);
			break;

			// others send whole module to detector
		default:
			break;
	}



	// MYTHEN ONLY (sends whole detector to module)

	sls_detector_module *myMod=createModule();
	int modmi=imod, modma=imod+1, im=imod;
	string settingsfname, calfname;
	string ssettings;

	//not included in module structure
	int iodelay = -1;
	int tau = -1;
	int* gainval=0, *offsetval=0;
	if(thisDetector->nGain)
		gainval=new int[thisDetector->nGain];
	if(thisDetector->nOffset)
		offsetval=new int[thisDetector->nOffset];


	switch (isettings) {
	case STANDARD:
		ssettings="/standard";
		thisDetector->currentSettings=STANDARD;
		break;
	case FAST:
		ssettings="/fast";
		thisDetector->currentSettings=FAST;
		break;
	case HIGHGAIN:
		ssettings="/highgain";
		thisDetector->currentSettings=HIGHGAIN;
		break;
	default:
		break;
	}


	if (isettings !=  thisDetector->currentSettings) {
		printf("Unknown settings %s for this detector!\n",
				getDetectorSettings(isettings).c_str());
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
	}else{
		if (imod<0) {
			modmi=0;
			//  modma=thisDetector->nModMax[X]*thisDetector->nModMax[Y];
			modma=thisDetector->nMod[X]*thisDetector->nMod[Y];
		}

		for (im=modmi; im<modma; ++im) {
			ostringstream ostfn, oscfn;
			myMod->module=im;

			std::cout << std::endl << "Loading settings for module:" << im << std::endl;

			//create file names
			ostfn << thisDetector->settingsDir << ssettings <<"/noise.sn"
					<< setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im)
					<< setbase(10);
			oscfn << thisDetector->calDir << ssettings << "/calibration.sn"
					<< setfill('0') << setw(3) << hex << getId(MODULE_SERIAL_NUMBER, im)
					<< setbase(10);


			//settings file****
			settingsfname=ostfn.str();
#ifdef VERBOSE
			cout << "the settings file name is "<<settingsfname << endl;
#endif
			if (NULL == readSettingsFile(settingsfname,detType, iodelay, tau, myMod)) {
				//if it didnt open, try default settings file
				ostringstream ostfn_default;
				ostfn_default << thisDetector->settingsDir << ssettings
						<< ssettings << ".trim";
				settingsfname=ostfn_default.str();
#ifdef VERBOSE
				cout << settingsfname << endl;
#endif
				if (NULL == readSettingsFile(settingsfname,detType, iodelay, tau,
						myMod)) {
					//if default doesnt work, return error
					std::cout << "Could not open settings file" << endl;
					setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
					return  thisDetector->currentSettings;
				}
			}



			//calibration file****
			int ret=0;
			calfname=oscfn.str();
#ifdef VERBOSE
			cout << "Specific file:"<< calfname << endl;
#endif
			//extra gain and offset
			if(thisDetector->nGain)
				ret = readCalibrationFile(calfname,gainval, offsetval);
			//normal gain and offset inside sls_detector_module
			else
				ret = readCalibrationFile(calfname,myMod->gain, myMod->offset);

			//if it didnt open, try default
			if(ret != OK){
				ostringstream oscfn_default;
				oscfn_default << thisDetector->calDir << ssettings << ssettings << ".cal";
				calfname=oscfn_default.str();
#ifdef VERBOSE
				cout << "Default file:" << calfname << endl;
#endif
				//extra gain and offset
				if(thisDetector->nGain)
					ret = readCalibrationFile(calfname,gainval, offsetval);
				//normal gain and offset inside sls_detector_module
				else
					ret = readCalibrationFile(calfname,myMod->gain, myMod->offset);
			}
			//if default doesnt work, return error
			if(ret != OK){
				std::cout << "Could not open calibration file" << calfname << endl;
				setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
				return  thisDetector->currentSettings;
			}


			//if everything worked, set module****
			setModule(*myMod,iodelay,tau,-1,gainval,offsetval);
		}
	}



	deleteModule(myMod);
	if(gainval)	delete [] gainval;
	if(offsetval) delete [] offsetval;

	if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
		int isett=getSettings(imod);
		double t[]=defaultTDead;
		if (isett>-1 && isett<3) {
			thisDetector->tDead=t[isett];
		}
	}


	if (getSettings(imod) != isettings){
		std::cout << "Could not set settings" << endl;
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
	}

	return  thisDetector->currentSettings;
}



slsDetectorDefs::detectorSettings slsDetector::sendSettingsOnly(detectorSettings isettings,
		int imod) {
	int fnum = F_SET_SETTINGS;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);
	int retval = -1;
	int arg[2];
	arg[0] = isettings;
	arg[1] = imod;
#ifdef VERBOSE
	std::cout<< "Setting settings of module " << arg[1] << " to " << arg[0] << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			} else{
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				thisDetector->currentSettings = (detectorSettings)retval;
#ifdef VERBOSE
				std::cout<< "Settings are " << retval << std::endl;
#endif
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return thisDetector->currentSettings;
}



int slsDetector::getThresholdEnergy(int imod) {

	int fnum=  F_GET_THRESHOLD_ENERGY;
	int retval;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
#ifdef VERBOSE
	std::cout<< "Getting threshold energy "<< std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&imod,sizeof(imod));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				std::cout<< "Detector returned error: "<< std::endl;
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<<  mess << std::endl;
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				thisDetector->currentThresholdEV=retval;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return  thisDetector->currentThresholdEV;
}

int slsDetector::setThresholdEnergy(int e_eV,  int imod, detectorSettings isettings, int tb) {

	//currently only for eiger
	if (thisDetector->myDetectorType == EIGER) {
		setThresholdEnergyAndSettings(e_eV,isettings,tb);
		return  thisDetector->currentThresholdEV;
	}

	int fnum=  F_SET_THRESHOLD_ENERGY;
	int retval;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
#ifdef VERBOSE
	std::cout<< "Setting threshold energy "<< std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&e_eV,sizeof(e_eV));
			controlSocket->SendDataOnly(&imod,sizeof(imod));
			controlSocket->SendDataOnly(&isettings,sizeof(isettings));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				std::cout<< "Detector returned error: "<< std::endl;
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<<  mess << std::endl;
			} else {
#ifdef VERBOSE
				std::cout<< "Detector returned OK "<< std::endl;
#endif
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				thisDetector->currentThresholdEV=retval;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		thisDetector->currentThresholdEV=e_eV;
	}
	return   thisDetector->currentThresholdEV;
}



int slsDetector::setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings, int tb) {

	//if settings provided, use that, else use the shared memory variable
	detectorSettings is = ((isettings != GET_SETTINGS) ? isettings:
			thisDetector->currentSettings);
	string ssettings;
	switch (is) {
	case STANDARD:
		ssettings="/standard";
		thisDetector->currentSettings=STANDARD;
		break;
	case HIGHGAIN:
		ssettings="/highgain";
		thisDetector->currentSettings=HIGHGAIN;
		break;
	case LOWGAIN:
		ssettings="/lowgain";
		thisDetector->currentSettings=LOWGAIN;
		break;
	case VERYHIGHGAIN:
		ssettings="/veryhighgain";
		thisDetector->currentSettings=VERYHIGHGAIN;
		break;
	case VERYLOWGAIN:
		ssettings="/verylowgain";
		thisDetector->currentSettings=VERYLOWGAIN;
		break;
	default:
		printf("Error: Unknown settings %s for this detector!\n",
				getDetectorSettings(is).c_str());
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}

	//verify e_eV exists in trimEneregies[]
	if (!thisDetector->nTrimEn ||
			(e_eV < thisDetector->trimEnergies[0]) ||
			(e_eV > thisDetector->trimEnergies[thisDetector->nTrimEn-1]) ) {
		printf("Error: This energy %d not defined for this module!\n", e_eV);
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}

	//find if interpolation required
	bool interpolate = true;
	for (int i = 0; i < thisDetector->nTrimEn; ++i) {
		if (thisDetector->trimEnergies[i] == e_eV) {
			interpolate = false;
			break;
		}
	}

	//fill detector module structure
	sls_detector_module *myMod = NULL;
	int iodelay = -1;			//not included in the module
	int tau = -1;			//not included in the module

	//normal
	if(!interpolate) {
		//find their directory names
		ostringstream ostfn;
		ostfn << thisDetector->settingsDir << ssettings << "/" << e_eV << "eV"
				<< "/noise.sn" << setfill('0') <<  setw(3) << dec << getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
		string settingsfname = ostfn.str();
#ifdef VERBOSE
		printf("Settings File is %s\n", settingsfname.c_str());
#endif
		//read the files
		myMod=createModule();
		if (NULL == readSettingsFile(settingsfname,thisDetector->myDetectorType,
				iodelay, tau, myMod, tb)) {
			if(myMod)deleteModule(myMod);
			return FAIL;
		}
	}


	//interpolate
	else {
		//find the trim values
		int trim1 = -1, trim2 = -1;
		for (int i = 0; i < thisDetector->nTrimEn; ++i) {
			if (e_eV < thisDetector->trimEnergies[i]) {
				trim2 = thisDetector->trimEnergies[i];
				trim1 = thisDetector->trimEnergies[i-1];
				break;
			}
		}
		//find their directory names
		ostringstream ostfn;
		ostfn << thisDetector->settingsDir << ssettings << "/" << trim1 << "eV"
				<< "/noise.sn" << setfill('0') <<  setw(3) << dec <<
				getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
		string settingsfname1 = ostfn.str();
		ostfn.str(""); ostfn.clear();
		ostfn << thisDetector->settingsDir << ssettings << "/" << trim2 << "eV"
				<< "/noise.sn" << setfill('0') <<  setw(3) << dec <<
				getId(DETECTOR_SERIAL_NUMBER) << setbase(10);
		string settingsfname2 = ostfn.str();
		//read the files
#ifdef VERBOSE
		printf("Settings Files are %s and %s\n",settingsfname1.c_str(),
				settingsfname2.c_str());
#endif
		sls_detector_module *myMod1=createModule();
		sls_detector_module *myMod2=createModule();
		int iodelay1 = -1;			//not included in the module
		int tau1 = -1;			//not included in the module
		int iodelay2 = -1;			//not included in the module
		int tau2 = -1;			//not included in the module
		if (NULL == readSettingsFile(settingsfname1,thisDetector->myDetectorType,
				iodelay1, tau1, myMod1, tb)) {
			setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		if (NULL == readSettingsFile(settingsfname2,thisDetector->myDetectorType,
				iodelay2, tau2, myMod2, tb)) {
			setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		if (iodelay1 != iodelay2) {
			printf("iodelays do not match between files\n");
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		iodelay = iodelay1;

		//interpolate  module
		myMod = interpolateTrim(thisDetector->myDetectorType, myMod1, myMod2,
				e_eV, trim1, trim2, tb);
		if (myMod == NULL) {
			printf("Could not interpolate, different dac values in files\n");
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		//interpolate tau
		tau = linearInterpolation(e_eV, trim1, trim2, tau1, tau2);
		//printf("new tau:%d\n",tau);

		deleteModule(myMod1);
		deleteModule(myMod2);
	}


	myMod->module=0;
	myMod->reg=thisDetector->currentSettings;
	setModule(*myMod, iodelay, tau, e_eV, 0, 0, tb);
	deleteModule(myMod);
	if (getSettings(-1) != is){
		std::cout << "Could not set settings in detector" << endl;
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}

	return OK;
}




string slsDetector::getSettingsDir() {
	return std::string(thisDetector->settingsDir);
}
string slsDetector::setSettingsDir(string s) {
	sprintf(thisDetector->settingsDir, "%s", s.c_str()); 
	return thisDetector->settingsDir;
}
string slsDetector::getCalDir() {
	return thisDetector->calDir;
}
string slsDetector::setCalDir(string s) {
	sprintf(thisDetector->calDir, "%s", s.c_str()); 
	return thisDetector->calDir;
}





int slsDetector::loadSettingsFile(string fname, int imod) {

	sls_detector_module  *myMod=NULL;

	int iodelay = -1;
	int tau = -1;

	string fn=fname;
	fn=fname;
	int mmin=0, mmax=setNumberOfModules();
	if (imod>=0) {
		mmin=imod;
		mmax=imod+1;
	}
	for (int im=mmin; im<mmax; ++im) {
		ostringstream ostfn;
		ostfn << fname;
		switch (thisDetector->myDetectorType) {
		case MYTHEN:
			if (fname.find(".sn")==string::npos && fname.find(".trim")==
					string::npos && fname.find(".settings")==string::npos) {
				ostfn << ".sn"  << setfill('0') << setw(3) << hex <<
						getId(MODULE_SERIAL_NUMBER, im);
			}
			break;
		case EIGER:
			if (fname.find(".sn")==string::npos && fname.find(".trim")==
					string::npos && fname.find(".settings")==string::npos) {
				ostfn << ".sn"  << setfill('0') <<  setw(3) << dec <<
						getId(DETECTOR_SERIAL_NUMBER, im);
			}
			break;
		default:
			break;
		}
		fn=ostfn.str();
		myMod=readSettingsFile(fn, thisDetector->myDetectorType,iodelay, tau, myMod);

		if (myMod) {
			myMod->module=im;
			//settings is saved in myMod.reg for all except mythen
			if(thisDetector->myDetectorType!=MYTHEN)
				myMod->reg=-1;
			setModule(*myMod,iodelay,tau,-1,0,0);
			deleteModule(myMod);
		} else
			return FAIL;
	}
	return OK;
}


int slsDetector::saveSettingsFile(string fname, int imod) {

	sls_detector_module  *myMod=NULL;
	int ret=FAIL;
	int iodelay = -1;
	int tau = -1;

	int mmin=0,  mmax=setNumberOfModules();
	if (imod>=0) {
		mmin=imod;
		mmax=imod+1;
	}
	for (int im=mmin; im<mmax; ++im) {
		string fn=fname;
		ostringstream ostfn;
		ostfn << fname;
		switch (thisDetector->myDetectorType) {
		case MYTHEN:
			ostfn << ".sn"  << setfill('0') << setw(3) << hex <<
			getId(MODULE_SERIAL_NUMBER,im);
			break;
		case EIGER:
			ostfn << ".sn"  << setfill('0') <<  setw(3) << dec <<
			getId(DETECTOR_SERIAL_NUMBER);
			break;
		default:
			break;
		}
		fn=ostfn.str();
		if ((myMod=getModule(im))) {

			if(thisDetector->myDetectorType == EIGER){
				iodelay = (int)setDAC((dacs_t)-1,IO_DELAY,0,-1);
				tau = (int64_t)getRateCorrectionTau();
			}
			ret=writeSettingsFile(fn, thisDetector->myDetectorType, *myMod,
					iodelay, tau);
			deleteModule(myMod);
		}
	}
	return ret;
}




int slsDetector::loadCalibrationFile(string fname, int imod) {

	if(thisDetector->myDetectorType == EIGER) {
		std::cout << "Not required for this detector!" << std::endl;
		return FAIL;
	}

	sls_detector_module  *myMod=NULL;
	string fn=fname;

	int* gainval=0; int* offsetval=0;
	if(thisDetector->nGain){
		gainval=new int[thisDetector->nGain];
		for(int i=0;i<thisDetector->nGain;++i)
			gainval[i] = -1;
	}
	if(thisDetector->nOffset){
		offsetval=new int[thisDetector->nOffset];
		for(int i=0;i<thisDetector->nOffset;++i)
			offsetval[i] = -1;
	}

	fn=fname;


	int mmin=0, mmax=setNumberOfModules();
	if (imod>=0) {
		mmin=imod;
		mmax=imod+1;
	}
	for (int im=mmin; im<mmax; ++im) {
		string fn=fname;
		ostringstream ostfn;
		ostfn << fname ;
		switch (thisDetector->myDetectorType) {
		case MYTHEN:
			if (fname.find(".sn")==string::npos && fname.find(".cal")==string::npos) {
				ostfn << ".sn"  << setfill('0') << setw(3) << hex <<
						getId(MODULE_SERIAL_NUMBER, im);
			}
			break;
		case EIGER:
			if (fname.find(".sn")==string::npos && fname.find(".cal")==string::npos) {
				ostfn << "."  << setfill('0') <<  setw(3) << dec <<
						getId(DETECTOR_SERIAL_NUMBER);
			}
			break;
		default:
			break;
		}
		fn=ostfn.str();
		if((myMod=getModule(im))){
			//extra gain and offset
			if(thisDetector->nGain){
				if(readCalibrationFile(fn, gainval, offsetval)==FAIL)
					return FAIL;
			} //normal gain and offset inside sls_detector_module
			else{
				if(readCalibrationFile(fn,myMod->gain, myMod->offset)==FAIL)
					return FAIL;
			}
			setModule(*myMod,-1,-1,-1,gainval,offsetval);

			deleteModule(myMod);
			if(gainval) delete[]gainval;
			if(offsetval) delete[] offsetval;
		} else
			return FAIL;
	}
	return OK;
}


int slsDetector::saveCalibrationFile(string fname, int imod) {


	sls_detector_module  *myMod=NULL;
	int ret=FAIL;

	int mmin=0,  mmax=setNumberOfModules();
	if (imod>=0) {
		mmin=imod;
		mmax=imod+1;
	}
	for (int im=mmin; im<mmax; ++im) {
		string fn=fname;
		ostringstream ostfn;
		ostfn << fname;
		switch (thisDetector->myDetectorType) {
		case MYTHEN:
			ostfn << ".sn"  << setfill('0') << setw(3) << hex <<
			getId(MODULE_SERIAL_NUMBER,im);
			break;
		case EIGER:
			ostfn << ".sn" << setfill('0') <<  setw(3) << dec <<
			getId(DETECTOR_SERIAL_NUMBER);
			break;
		default:
			break;
		}
		fn=ostfn.str();
		if ((myMod=getModule(im))) {
			//extra gain and offset
			if(thisDetector->nGain)
				ret=writeCalibrationFile(fn,gain, offset);
			//normal gain and offset inside sls_detector_module
			else
				ret=writeCalibrationFile(fn,myMod->gain, myMod->offset);

			deleteModule(myMod);
		}else
			return FAIL;
	}
	return ret;
}




slsDetectorDefs::masterFlags  slsDetector::setMaster(masterFlags flag) {


	int fnum=F_SET_MASTER;
	masterFlags retval=GET_MASTER;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

#ifdef VERBOSE
	std::cout<< "Setting master flags to "<< flag << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&flag,sizeof(flag));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

#ifdef VERBOSE
	std::cout<< "Master flag set to  "<< retval   << std::endl;
#endif
	return retval;
}




slsDetectorDefs::synchronizationMode slsDetector::setSynchronization(synchronizationMode flag) {



	int fnum=F_SET_SYNCHRONIZATION_MODE;
	synchronizationMode retval=GET_SYNCHRONIZATION_MODE;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

#ifdef VERBOSE
	std::cout<< "Setting synchronization mode to "<< flag << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&flag,sizeof(flag));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

#ifdef VERBOSE
	std::cout<< "Readout flag set to  "<< retval   << std::endl;
#endif
	return retval;

}


int slsDetector::setTotalProgress() {

	int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1, nm=1, ns=1;

	if (thisDetector->timerValue[FRAME_NUMBER])
		nf=thisDetector->timerValue[FRAME_NUMBER];

	if (thisDetector->timerValue[CYCLES_NUMBER]>0)
		nc=thisDetector->timerValue[CYCLES_NUMBER];

	if (thisDetector->timerValue[STORAGE_CELL_NUMBER]>0)
		ns=thisDetector->timerValue[STORAGE_CELL_NUMBER]+1;

	if (thisDetector->numberOfPositions>0)
		npos=thisDetector->numberOfPositions;

	if (timerValue[MEASUREMENTS_NUMBER]>0)
		nm=timerValue[MEASUREMENTS_NUMBER];


	if ((thisDetector->nScanSteps[0]>0) && (thisDetector->actionMask &
			(1 << MAX_ACTIONS)))
		nscan[0]=thisDetector->nScanSteps[0];

	if ((thisDetector->nScanSteps[1]>0) && (thisDetector->actionMask &
			(1 << (MAX_ACTIONS+1))))
		nscan[1]=thisDetector->nScanSteps[1];

	thisDetector->totalProgress=nf*nc*ns*npos*nm*nscan[0]*nscan[1];

#ifdef VERBOSE
cout << "nc " << nc << endl;
cout << "nm " << nm << endl;
cout << "nf " << nf << endl;
cout << "ns " << ns << endl;
cout << "npos " << npos << endl;
cout << "nscan[0] " << nscan[0] << endl;
cout << "nscan[1] " << nscan[1] << endl;

cout << "Set total progress " << thisDetector->totalProgress << endl;
#endif
return thisDetector->totalProgress;
}

double slsDetector::getCurrentProgress() {

	return 100.*((double)thisDetector->progressIndex)/((double)thisDetector->totalProgress);
}



slsDetectorDefs::runStatus slsDetector::getRunStatus() {
	int fnum=F_GET_RUN_STATUS;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
	strcpy(mess,"could not get run status");
	runStatus retval=ERROR;
#ifdef VERBOSE
	std::cout<< "Getting status "<< std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (stopSocket) {
			if  (connectStop() == OK) {
				stopSocket->SendDataOnly(&fnum,sizeof(fnum));
				stopSocket->ReceiveDataOnly(&ret,sizeof(ret));

				//cout << "________:::____________" << ret << endl;

				if (ret==FAIL) {
					stopSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				} else {
					stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
					//cout << "____________________" << retval << endl;
				}
				disconnectStop();
			}
		}
	}
	return retval;


}


int slsDetector::prepareAcquisition() {
	int fnum = F_PREPARE_ACQUISITION;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Preparing Detector for Acquisition" << std::endl;
#endif
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(PREPARE_ACQUISITION));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}else
		std::cout << "cannot connect to detector" << endl;

	return ret;
}

int slsDetector::cleanupAcquisition() {
	int fnum = F_CLEANUP_ACQUISITION;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Cleaning up Detector after Acquisition " << std::endl;
#endif
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(CLEANUP_ACQUISITION));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}else
		std::cout << "cannot connect to detector" << endl;

	return ret;

}



int slsDetector::startAcquisition() {

	int fnum=F_START_ACQUISITION;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< "Starting acquisition "<< std::endl;
#endif
	thisDetector->stoppedFlag=0;
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return ret;
}


int slsDetector::stopAcquisition() {

	// get status before stopping acquisition
	runStatus s = ERROR, r = ERROR;
	if (thisDetector->receiver_upstream) {
		s = getRunStatus();
		r = getReceiverStatus();
	}

	int fnum=F_STOP_ACQUISITION;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< "Stopping acquisition "<< std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (stopSocket) {
			if  (connectStop() == OK) {
				stopSocket->SendDataOnly(&fnum,sizeof(fnum));
				stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					stopSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				disconnectStop();
			}
		}
	}
	thisDetector->stoppedFlag=1;

	// if rxr streaming and acquisition finished, restream dummy stop packet
	if ((thisDetector->receiver_upstream) && (s == IDLE) && (r == IDLE)) {
		restreamStopFromReceiver();
	}

	return ret;


}


int slsDetector::sendSoftwareTrigger() {

	int fnum=F_SOFTWARE_TRIGGER;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< "Sending software trigger "<< std::endl;
#endif
	thisDetector->stoppedFlag=0;
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return ret;
}



int slsDetector::startReadOut() {

	int fnum=F_START_READOUT;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< "Starting readout "<< std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return ret;
}



int* slsDetector::startAndReadAll() {
	//cout << "Start and read all "<< endl;

	int* retval;
	//#ifdef VERBOSE
#ifdef VERBOSE
	int i=0;
#endif
	//#endif
	if(thisDetector->myDetectorType == EIGER) {
		if (prepareAcquisition() == FAIL)
			return NULL;
	}
	startAndReadAllNoWait();
	//#ifdef VERBOSE
	// std::cout<< "started" << std::endl;
	//#endif
	while ((retval=getDataFromDetector())){
#ifdef VERBOSE
		++i;
		std::cout<< i << std::endl;
		//#else
		//std::cout<< "-" << flush;
#endif
		dataQueue.push(retval);

		//std::cout<< "pushed" << std::endl;
	}
	disconnectControl();

#ifdef VERBOSE
	std::cout<< "received "<< i<< " frames" << std::endl;
	//#else
	// std::cout << std::endl;
#endif
	return dataQueue.front(); // check what we return!
	/* while ((retval=getDataFromDetectorNoWait()))
     ++i;
     #ifdef VERBOSE
     std::cout<< "Received " << i << " frames"<< std::endl;
     #endif
     return dataQueue.front(); // check what we return!
	 */

}



int slsDetector::startAndReadAllNoWait() {

	int fnum= F_START_AND_READ_ALL;

#ifdef VERBOSE
	std::cout<< "Starting and reading all frames "<< std::endl;
#endif
	thisDetector->stoppedFlag=0;
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			//std::cout<< "connected" << std::endl;
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			return OK;
		}
	}
	return FAIL;
}




int* slsDetector::getDataFromDetector(int *retval) {
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="Nothing";
	int nel=thisDetector->dataBytes/sizeof(int);
	int n;
	int *r=retval;

	int nodatadetectortype = false;
	detectorType types = getDetectorsType();
	if(types == EIGER || types == JUNGFRAU || GOTTHARD || PROPIX){
		nodatadetectortype = true;
	}


	if (!nodatadetectortype && retval==NULL)
		retval=new int[nel];


#ifdef VERBOSE
	std::cout<< "getting data "<< thisDetector->dataBytes << " " << nel<< std::endl;
#endif
	controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
#ifdef VERBOSE
	cout << "ret=" << ret << endl;
#endif

	if (ret!=OK) {
		n= controlSocket->ReceiveDataOnly(mess,sizeof(mess));
		// if(thisDetector->receiverOnlineFlag == OFFLINE_FLAG)
		if (ret==FAIL) {
			thisDetector->stoppedFlag=1;
			std::cout<< "Detector returned: " << mess << " " << n << std::endl;
		} else {
			;
#ifdef VERBOSE
			std::cout<< "Detector successfully returned: " << mess << " " << n
					<< std::endl;
#endif
		}
		if ((!nodatadetectortype) && (r==NULL)){
			delete [] retval;
		}
		return NULL;
	} else 	if (!nodatadetectortype){

		n=controlSocket->ReceiveDataOnly(retval,thisDetector->dataBytes);

#ifdef VERBOSE
		std::cout<< "Received "<< n << " data bytes" << std::endl;
#endif
		if (n!=thisDetector->dataBytes) {
			std::cout<< "wrong data size received from detector: received " <<
					n << " but expected " << thisDetector->dataBytes << std::endl;
			thisDetector->stoppedFlag=1;
			ret=FAIL;
			if (r==NULL) {
				delete [] retval;
			}
			return NULL;
		}
		// for (int ib=0; ib<thisDetector->dataBytes/8; ++ib)
		//   cout << ((*(((u_int64_t*)retval)+ib))>>17&1) ;


	}
	//	cout << "get data returning " << endl;
	//	cout << endl;
	return retval;

}



int* slsDetector::readFrame() {

	int fnum=F_READ_FRAME;
	int* retval=NULL;

#ifdef VERBOSE
	std::cout<< "slsDetector: Reading frame "<< std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			retval=getDataFromDetector();
			if (retval) {
				dataQueue.push(retval);
				disconnectControl();
			}
		}
	}
	return retval;
}




int* slsDetector::readAll() {

	int fnum=F_READ_ALL;
	int* retval; // check what we return!

#ifdef VERBOSE
	int i=0;
	std::cout<< "Reading all frames "<< std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));

			while ((retval=getDataFromDetector())){
#ifdef VERBOSE
				++i;
				std::cout<< i << std::endl;
#endif
				dataQueue.push(retval);
			}
			disconnectControl();
		}
	}
#ifdef VERBOSE
	std::cout<< "received "<< i<< " frames" << std::endl;
#endif
	return dataQueue.front(); // check what we return!

}


int slsDetector::readAllNoWait() {

	int fnum= F_READ_ALL;

#ifdef VERBOSE
	std::cout<< "Reading all frames "<< std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			return OK;
		}
	}
	return FAIL;
}



int slsDetector::configureMAC() {
	int i;
	int ret=FAIL;
	int fnum=F_CONFIGURE_MAC;
	char mess[MAX_STR_LENGTH]="";
	char arg[6][50];memset(arg,0,sizeof(char)*6*50);
	int retval=-1;

	// to send 3d positions to detector
	bool sendpos = 0;
	int pos[3]={0,0,0};

	// only jungfrau and eiger, send x, y and z in detector udp header
	if (thisDetector->myDetectorType == JUNGFRAU ||
			thisDetector->myDetectorType == EIGER) {
		sendpos = true;
		int max = multiDet->getNumberOfDetectors(Y);

		pos[0] = (detId % max); // row
		pos[1] = (detId / max) * ((thisDetector->myDetectorType == EIGER) ? 2 : 1);// col for horiz. udp ports
	}
#ifdef VERBOSE
	cout << "SLS [" << detId << "] - (" << pos[0] << "," << pos[1] << "," <<
			pos[2] << ")" << endl;
#endif


	//if udpip wasnt initialized in config file
	if(!(strcmp(thisDetector->receiverUDPIP,"none"))){
		//hostname is an ip address
		if(strchr(thisDetector->receiver_hostname,'.')!=NULL)
			strcpy(thisDetector->receiverUDPIP,thisDetector->receiver_hostname);
		//if hostname not ip, convert it to ip
		else{
			struct addrinfo *result;
			if (!dataSocket->ConvertHostnameToInternetAddress(
					thisDetector->receiver_hostname, &result)) {
				// on success
				memset(thisDetector->receiverUDPIP, 0, MAX_STR_LENGTH);
				// on failure, back to none
				if (dataSocket->ConvertInternetAddresstoIpString(result,
						thisDetector->receiverUDPIP, MAX_STR_LENGTH)) {
					strcpy(thisDetector->receiverUDPIP, "none");
				}
			}
		}
	}
	strcpy(arg[0],thisDetector->receiverUDPIP);
	strcpy(arg[1],thisDetector->receiverUDPMAC);
	sprintf(arg[2],"%x",thisDetector->receiverUDPPort);
	strcpy(arg[3],thisDetector->detectorMAC);
	strcpy(arg[4],thisDetector->detectorIP);
	sprintf(arg[5],"%x",thisDetector->receiverUDPPort2);
#ifdef VERBOSE
	std::cout<< "Configuring MAC"<< std::endl;
#endif



	for(i=0;i<2;++i){
		if(!strcmp(arg[i],"none")){
			std::cout<< "Configure MAC Error. IP/MAC Addresses not set"<< std::endl;
			setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
			return FAIL;
		}
	}

#ifdef VERBOSE
	std::cout<< "IP/MAC Addresses valid "<< std::endl;
#endif


	{
		//converting IPaddress to hex
		stringstream ss(arg[0]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, '.')) {
			sprintf(cword,"%s%02x",cword,atoi(s.c_str()));
		}
		bzero(arg[0], 50);
		strcpy(arg[0],cword);
#ifdef VERBOSE
		std::cout<<"receiver udp ip:"<<arg[0]<<"."<<std::endl;
#endif
	}

	{
		//converting MACaddress to hex
		stringstream ss(arg[1]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, ':')) {
			sprintf(cword,"%s%s",cword,s.c_str());
		}
		bzero(arg[1], 50);
		strcpy(arg[1],cword);
#ifdef VERBOSE
		std::cout<<"receiver mac:"<<arg[1]<<"."<<std::endl;
#endif
	}

#ifdef VERBOSE
	std::cout<<"receiver udp port:"<<arg[2]<<"."<<std::endl;
#endif

	{
		stringstream ss(arg[3]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, ':')) {
			sprintf(cword,"%s%s",cword,s.c_str());
		}
		bzero(arg[3], 50);
		strcpy(arg[3],cword);
#ifdef VERBOSE
		std::cout<<"detecotor mac:"<<arg[3]<<"."<<std::endl;
#endif
	}

	{
		//converting IPaddress to hex
		stringstream ss(arg[4]);
		char cword[50]="";
		bzero(cword, 50);
		string s;
		while (getline(ss, s, '.')) {
			sprintf(cword,"%s%02x",cword,atoi(s.c_str()));
		}
		bzero(arg[4], 50);
		strcpy(arg[4],cword);
#ifdef VERBOSE
		std::cout<<"detector ip:"<<arg[4]<<"."<<std::endl;
#endif
	}

#ifdef VERBOSE
	std::cout<<"receiver udp port2:"<<arg[5]<<"."<<std::endl;
#endif

	//send to server
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			if(sendpos)
				controlSocket->SendDataOnly(pos,sizeof(pos));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
			}
			else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				if (thisDetector->myDetectorType == EIGER) {
					//rewrite detectormac, detector ip
					char arg[2][50];
					memset(arg,0,sizeof(arg));
					uint64_t idetectormac = 0;
					uint32_t idetectorip = 0;
					controlSocket->ReceiveDataOnly(arg,sizeof(arg));
					sscanf(arg[0], "%lx",    &idetectormac);
					sscanf(arg[1], "%x",    &idetectorip);
					sprintf(arg[0],"%02x:%02x:%02x:%02x:%02x:%02x",
							(unsigned int)((idetectormac>>40)&0xFF),
							(unsigned int)((idetectormac>>32)&0xFF),
							(unsigned int)((idetectormac>>24)&0xFF),
							(unsigned int)((idetectormac>>16)&0xFF),
							(unsigned int)((idetectormac>>8)&0xFF),
							(unsigned int)((idetectormac>>0)&0xFF));
					sprintf(arg[1],"%d.%d.%d.%d",
							(idetectorip>>24)&0xff,
							(idetectorip>>16)&0xff,
							(idetectorip>>8)&0xff,
							(idetectorip)&0xff);
					if (strcasecmp(arg[0],thisDetector->detectorMAC)) {
						memset(thisDetector->detectorMAC, 0, MAX_STR_LENGTH);
						strcpy(thisDetector->detectorMAC, arg[0]);
						cprintf(RESET,"%d: Detector MAC updated to %s\n", detId,
								thisDetector->detectorMAC);
					}
					if (strcasecmp(arg[1],thisDetector->detectorIP)) {
						memset(thisDetector->detectorIP, 0, MAX_STR_LENGTH);
						strcpy(thisDetector->detectorIP, arg[1]);
						cprintf(RESET,"%d: Detector IP updated to %s\n", detId,
								thisDetector->detectorIP);
					}
				}
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	if (ret==FAIL) {
		ret=FAIL;
		std::cout<< "Configuring MAC failed " << std::endl;
		setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
	}

	return ret;
}




int64_t slsDetector::setTimer(timerIndex index, int64_t t, int imod) {


	int fnum=F_SET_TIMER,fnum2=F_SET_RECEIVER_TIMER;
	int64_t retval = -1;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

	if (index!=MEASUREMENTS_NUMBER) {


#ifdef VERBOSE
		std::cout<< "Setting timer "<< index << " to " <<  t << "ns/value" << std::endl;
#endif
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&index,sizeof(index));
				controlSocket->SendDataOnly(&t,sizeof(t));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					setErrorMask((getErrorMask())|(DETECTOR_TIMER_VALUE_NOT_SET));
				} else {
					controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
					thisDetector->timerValue[index]=retval;
				}
				disconnectControl();
				if (ret==FORCE_UPDATE) {
					updateDetector();
#ifdef VERBOSE
					std::cout<< "Updated!" << std::endl;
#endif

				}
			}
		} else {
			//std::cout<< "offline " << std::endl;
			if (t>=0)
				thisDetector->timerValue[index]=t;
			if((thisDetector->myDetectorType==GOTTHARD)||
					(thisDetector->myDetectorType==PROPIX)||
					(thisDetector->myDetectorType==JUNGFRAU)||
					(thisDetector->myDetectorType==MOENCH))
				thisDetector->timerValue[PROBES_NUMBER]=0;
			if(thisDetector->myDetectorType==JUNGFRAUCTB && index==SAMPLES_JCTB) {
				getTotalNumberOfChannels();
				//			  thisDetector->dataBytes=getTotalNumberOfChannels()*
				//thisDetector->dynamicRange/8*thisDetector->timerValue[SAMPLES_JCTB];
			}

		}
	} else {
		if (t>=0)
			thisDetector->timerValue[index]=t;
	}
#ifdef VERBOSE
	std::cout<< "Timer " << index << " set to  "<< thisDetector->timerValue[index]
																			<< "ns"  << std::endl;
#endif

	if ((thisDetector->myDetectorType==MYTHEN)&&(index==PROBES_NUMBER)) {
		setDynamicRange();
		//cout << "Changing probes: data size = " << thisDetector->dataBytes <<endl;
	}
	if ((thisDetector->myDetectorType==JUNGFRAUCTB) && (index==SAMPLES_JCTB)) {
		setDynamicRange();
		cout << "Changing samples: data size = " << thisDetector->dataBytes <<endl;
	}


	if(t!=-1){
		if ((thisDetector->myDetectorType==MYTHEN)&&(index==PROBES_NUMBER)) {
			setDynamicRange();
			//cout << "Changing probes: data size = " << thisDetector->dataBytes <<endl;
		}

		/* set progress */
		if ((index==FRAME_NUMBER) || (index==CYCLES_NUMBER) ||
				(index==STORAGE_CELL_NUMBER)) {
			setTotalProgress();
		}

		//if eiger, rate corr on, a put statement, dr=32 &setting subexp or dr =16
		//& setting exptime, set ratecorr to update table
		double r;
		if( (thisDetector->myDetectorType == EIGER) &&
				getRateCorrection(r) &&
				(t>=0) &&

				(((index == SUBFRAME_ACQUISITION_TIME) &&
						(thisDetector->dynamicRange == 32))||
						((index == ACQUISITION_TIME) &&
								(thisDetector->dynamicRange == 16)))

						&& (t>=0) && getRateCorrection(r)){
			setRateCorrection(r);
		}
	}




	//send acquisiton time/period/subexptime/frame/cycles/samples to receiver
	if((index==FRAME_NUMBER)||(index==FRAME_PERIOD)||(index==CYCLES_NUMBER)||
			(index==ACQUISITION_TIME) || (index==SUBFRAME_ACQUISITION_TIME) ||
			(index==SUBFRAME_DEADTIME) ||
			(index==SAMPLES_JCTB) || (index==STORAGE_CELL_NUMBER)){
		string timername = getTimerType(index);
		if(ret != FAIL){
			int64_t args[2];
			retval = -1;
			args[0] = index;
			args[1] = thisDetector->timerValue[index];

			if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){

				//set #frames * #cycles
				if((index==FRAME_NUMBER)||(index==CYCLES_NUMBER)||
						(index==STORAGE_CELL_NUMBER)){
					timername.assign("(Number of Frames) * (Number of cycles) "
							"* (Number of storage cells)");
					args[1] = thisDetector->timerValue[FRAME_NUMBER] *
							((thisDetector->timerValue[CYCLES_NUMBER] > 0) ?
									(thisDetector->timerValue[CYCLES_NUMBER]) : 1) *
							((thisDetector->timerValue[STORAGE_CELL_NUMBER] > 0)
									? (thisDetector->timerValue[STORAGE_CELL_NUMBER])+1 : 1);
#ifdef VERBOSE
					std::cout << "Setting/Getting " << timername  << " " << index
							<<" to/from receiver " << args[1] << std::endl;
#endif
				}
#ifdef VERBOSE
				// set period/exptime/subexptime/subperiod
				else std::cout << "Setting/Getting " << timername  << " " << index
						<< " to/from receiver " << args[1] << std::endl;
#endif

				char mess[MAX_STR_LENGTH]="";
				if (connectData() == OK){
					ret=thisReceiver->sendIntArray(fnum2,retval,args,mess);
					disconnectData();
				}
				if((args[1] != retval)|| (ret==FAIL)){
					ret = FAIL;
					cout << "ERROR: " << timername << " in receiver set incorrectly to "
							<< retval << " instead of " << args[1] << endl;

					if(strstr(mess,"receiver is not idle")==NULL) {
						switch(index) {
						case ACQUISITION_TIME:
							setErrorMask((getErrorMask())|(RECEIVER_ACQ_TIME_NOT_SET));
							break;
						case FRAME_PERIOD:
							setErrorMask((getErrorMask())|(RECEIVER_ACQ_PERIOD_NOT_SET));
							break;
						case SUBFRAME_ACQUISITION_TIME:
						case SUBFRAME_DEADTIME:
						case SAMPLES_JCTB:
							setErrorMask((getErrorMask())|(RECEIVER_TIMER_NOT_SET));
							break;
						default:
							setErrorMask((getErrorMask())|(RECEIVER_FRAME_NUM_NOT_SET));
							break;
						}
					}
				}
				if(ret==FORCE_UPDATE)
					updateReceiver();
			}
		}
	}
	return thisDetector->timerValue[index];
}



int64_t slsDetector::getTimeLeft(timerIndex index, int imod) {


	int fnum=F_GET_TIME_LEFT;
	int64_t retval = FAIL;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

#ifdef VERBOSE
	std::cout<< "Getting  timer  "<< index <<  std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (stopSocket) {
			if  (connectStop() == OK) {
				stopSocket->SendDataOnly(&fnum,sizeof(fnum));
				stopSocket->SendDataOnly(&index,sizeof(index));
				stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					stopSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				} else {
					stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
				}
				disconnectStop();
			}
		}
	}
#ifdef VERBOSE
	std::cout<< "Time left is  "<< retval << std::endl;
#endif
	return retval;

}



int slsDetector::setSpeed(speedVariable sp, int value) {


	int fnum=F_SET_SPEED;
	int retval=-1;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

#ifdef VERBOSE
	std::cout<< "Setting speed  variable"<< sp << " to " <<  value << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&sp,sizeof(sp));
			controlSocket->SendDataOnly(&value,sizeof(value));
#ifdef VERBOSE
			std::cout<< "Sent  "<< n << " bytes "  << std::endl;
#endif
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_SET_SPEED_PARAMETERS));
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
#ifdef VERBOSE
	std::cout<< "Speed set to  "<< retval  << std::endl;
#endif
	return retval;

}


int slsDetector::setDynamicRange(int n) {

	// cout << "single "  << endl;
	int fnum=F_SET_DYNAMIC_RANGE,fnum2=F_SET_RECEIVER_DYNAMIC_RANGE;
	int retval=-1,retval1;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK, rateret=OK;

#ifdef VERBOSE
	std::cout<< "Setting dynamic range to "<< n << std::endl;
#endif
	if ((thisDetector->myDetectorType == MYTHEN) &&(n==24))
		n=32;



	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&n,sizeof(n));
			//rate correction is switched off if not 32 bit mode
			if(thisDetector->myDetectorType == EIGER){
				controlSocket->ReceiveDataOnly(&rateret,sizeof(rateret));
				if (rateret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					if(strstr(mess,"Rate Correction")!=NULL){
						if(strstr(mess,"32")!=NULL)
							setErrorMask((getErrorMask())|(RATE_CORRECTION_NOT_32or16BIT));
						else
							setErrorMask((getErrorMask())|(COULD_NOT_SET_RATE_CORRECTION));
					}
				}
			}
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else if(thisDetector->myDetectorType==MYTHEN){
		if (n>0)
			thisDetector->dynamicRange=n;
		retval=thisDetector->dynamicRange;
	}
	//cout << "detector returned dynamic range " << retval << endl;
	if (ret!=FAIL && retval>0) {
		/* checking the number of probes to chose the data size */



		thisDetector->dataBytes=thisDetector->nMod[X]*thisDetector->nMod[Y]*
				thisDetector->nChips*thisDetector->nChans*retval/8;
		thisDetector->dataBytesInclGapPixels =
				(thisDetector->nMod[X] * thisDetector->nChip[X] *
						thisDetector->nChan[X] + thisDetector->gappixels *
						thisDetector->nGappixels[X]) *
				(thisDetector->nMod[Y] * thisDetector->nChip[Y] *
						thisDetector->nChan[Y] + thisDetector->gappixels *
						thisDetector->nGappixels[Y]) *
				retval/8;
		if (thisDetector->myDetectorType==JUNGFRAUCTB) {
			// thisDetector->nChip[X]=retval/16;
			// thisDetector->nChips=thisDetector->nChip[X]*thisDetector->nChip[Y];
			// cout << thisDetector->nMod[X]*thisDetector->nMod[Y] << " "
			//<< thisDetector->nChans*thisDetector->nChips << " " << retval<< " ";
			getTotalNumberOfChannels();
			//thisDetector->dataBytes=getTotalNumberOfChannels()*
			//retval/8*thisDetector->timerValue[SAMPLES_JCTB];
			//cout << "data bytes: "<< thisDetector->dataBytes << endl;
		}
		if(thisDetector->myDetectorType==MYTHEN){
			if (thisDetector->timerValue[PROBES_NUMBER]!=0) {
				thisDetector->dataBytes=thisDetector->nMod[X]*
						thisDetector->nMod[Y]*thisDetector->nChips*thisDetector->nChans*4;
				thisDetector->dataBytesInclGapPixels =
						(thisDetector->nMod[X] * thisDetector->nChip[X] *
								thisDetector->nChan[X] + thisDetector->gappixels *
								thisDetector->nGappixels[X]) *
						(thisDetector->nMod[Y] * thisDetector->nChip[Y] *
								thisDetector->nChan[Y] + thisDetector->gappixels *
								thisDetector->nGappixels[Y]) *
						4;
			}

			if (retval==32)
				thisDetector->dynamicRange=24;
		}

		thisDetector->dynamicRange=retval;


#ifdef VERBOSE
		std::cout<< "Dynamic range set to  "<< thisDetector->dynamicRange   << std::endl;
		std::cout<< "Data bytes "<< thisDetector->dataBytes   << std::endl;
#endif
	}


	//receiver
	if(ret != FAIL){
		retval = thisDetector->dynamicRange;
		if((n==-1) && (ret!= FORCE_UPDATE)) n =-1;
		if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "Sending/Getting dynamic range to/from receiver " <<
					n << std::endl;
#endif
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum2,retval1,n);
				disconnectData();
			}
			if ((ret==FAIL) || (retval1 != retval)){
				ret = FAIL;
				cout << "ERROR:Dynamic range in receiver set incorrectly to " <<
						retval1 << " instead of " << retval << endl;
				setErrorMask((getErrorMask())|(RECEIVER_DYNAMIC_RANGE));
			}
			if(ret==FORCE_UPDATE)
				updateReceiver();
		}
	}

	return thisDetector->dynamicRange;
}


int slsDetector::getDataBytes() {
	return thisDetector->dataBytes;
}


int slsDetector::getDataBytesInclGapPixels() {
	return thisDetector->dataBytesInclGapPixels;
}


dacs_t slsDetector::setDAC(dacs_t val, dacIndex index, int mV, int imod) {


	dacs_t retval[2];
	retval[0] = -1;
	retval[1] = -1;
	int fnum=F_SET_DAC;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
	int arg[3];


	if ( (index==HV_NEW) &&((thisDetector->myDetectorType == GOTTHARD) ||
			(thisDetector->myDetectorType == PROPIX)))
		index=HV_POT;

	arg[0]=index;
	arg[1]=imod;
	arg[2]=mV;


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting DAC "<< index << " of module " << imod  <<  " to " <<
			val << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->SendDataOnly(&val,sizeof(val));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				controlSocket->ReceiveDataOnly(retval,sizeof(retval));
				if (index <  thisDetector->nDacs){

					if (dacs) {
						if (imod>=0) {
							*(dacs+index+imod*thisDetector->nDacs)=retval[0];
						}
						else {
							for (imod=0; imod<thisDetector->nModsMax; ++imod)
								*(dacs+index+imod*thisDetector->nDacs)=retval[0];
						}
					}
				}
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();

		}
	}
#ifdef VERBOSE
	std::cout<< "Dac set to "<< retval[0] << " dac units (" << retval[1] << "mV)"
			<< std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Set dac " << index << " of module " << imod  <<  " to " <<
				val << " failed." << std::endl;
	}
	if(mV)
		return retval[1];

	return retval[0];
}




dacs_t slsDetector::getADC(dacIndex index, int imod) {

	dacs_t retval = 0;
	int fnum=F_GET_ADC;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	int arg[2];
	arg[0]=index;
	arg[1]=imod;



#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Getting ADC "<< index << " of module " << imod  <<   std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectStop() == OK){
			stopSocket->SendDataOnly(&fnum,sizeof(fnum));
			stopSocket->SendDataOnly(arg,sizeof(arg));
			stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
				if (adcs) {
					*(adcs+index+imod*thisDetector->nAdcs)=retval;
				}
			} else {
				stopSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectStop();
			/*commented out to allow adc read during acquire, also not required
      if (ret==FORCE_UPDATE)
	updateDetector();*/
		}
	}
#ifdef VERBOSE
	std::cout<< "ADC returned "<< retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Get ADC failed " << std::endl;
	}

	return retval;



}



slsDetectorDefs::externalCommunicationMode slsDetector::setExternalCommunicationMode(
		externalCommunicationMode pol) {




	int arg[1];
	externalCommunicationMode  retval;
	int fnum=F_SET_EXTERNAL_COMMUNICATION_MODE;
	char mess[MAX_STR_LENGTH]="";

	arg[0]=pol;

	int ret=FAIL;
	retval=GET_EXTERNAL_COMMUNICATION_MODE;

#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting communication to mode " << pol << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		retval=GET_EXTERNAL_COMMUNICATION_MODE;
		ret=FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Communication mode "<<   " set to" << retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Setting communication mode failed" << std::endl;
	}
	return retval;

}



slsDetectorDefs::externalSignalFlag slsDetector::setExternalSignalFlags(
		externalSignalFlag pol, int signalindex) {




	int arg[2];
	externalSignalFlag  retval;
	int ret=FAIL;
	int fnum=F_SET_EXTERNAL_SIGNAL_FLAG;
	char mess[MAX_STR_LENGTH]="";

	arg[0]=signalindex;
	arg[1]=pol;

	retval=GET_EXTERNAL_SIGNAL_FLAG;

#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting signal "<< signalindex <<  " to flag" << pol << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		retval=GET_EXTERNAL_SIGNAL_FLAG;
		ret=FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Signal "<< signalindex <<  " flag set to" << retval << std::endl;
	if (ret==FAIL) {
		std::cout<< "Set signal flag failed " << std::endl;
	}
#endif
	return retval;






}




int slsDetector::setReadOutFlags(readOutFlags flag) {


	int fnum=F_SET_READOUT_FLAGS;
	readOutFlags retval;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

#ifdef VERBOSE
	std::cout<< "Setting readout flags to "<< flag << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&flag,sizeof(flag));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_SET_READOUT_FLAGS));
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				thisDetector->roFlags=retval;
				if (thisDetector->myDetectorType==JUNGFRAUCTB) {

					getTotalNumberOfChannels();
					//thisDetector->dataBytes=getTotalNumberOfChannels()*
					//thisDetector->dynamicRange/8*thisDetector->timerValue[SAMPLES_JCTB];
				}
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		if (flag!=GET_READOUT_FLAGS)
			thisDetector->roFlags=flag;
	}

#ifdef VERBOSE
	std::cout<< "Readout flag set to  "<< retval   << std::endl;
#endif
	return thisDetector->roFlags;
}


uint32_t slsDetector::writeRegister(uint32_t addr, uint32_t val) {

	uint32_t retval = 0;
	int fnum=F_WRITE_REGISTER;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
	uint32_t arg[2];
	arg[0]=addr;
	arg[1]=val;

#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Writing to register "<< hex<<addr <<  " data " << hex<<val << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
#ifdef VERBOSE
	std::cout<< "Register returned "<< retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Write to register failed " << std::endl;
		setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
	}

	return retval;
}



uint32_t slsDetector::readRegister(uint32_t addr) {

	uint32_t retval = 0;
	int fnum=F_READ_REGISTER;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
	uint32_t arg;
	arg=addr;


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Reading register "<< hex<<addr  << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		// if (connectControl() == OK){
		if (stopSocket) {
			if  (connectStop() == OK) {
				stopSocket->SendDataOnly(&fnum,sizeof(fnum));
				stopSocket->SendDataOnly(&arg,sizeof(arg));
				stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL)
					stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
				else {
					stopSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				disconnectStop();
			}
		}
	}
#ifdef VERBOSE
	std::cout<< "Register returned "<< retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Read register failed " << std::endl;
		setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
	}
	return retval;

}




uint32_t slsDetector::setBit(uint32_t addr, int n) {
	if (n<0 || n>31) {
		std::cout << "Bit number out of Range" << std:: endl;
		setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
	}

	// normal bit range
	//TODO! (Erik) Check for errors! cannot use value since reg is 32bits
	else {
		uint32_t val = readRegister(addr);
		writeRegister(addr,val | 1<<n);
	}

	return readRegister(addr);
}


uint32_t slsDetector::clearBit(uint32_t addr, int n) {
	if (n<0 || n>31) {
		std::cout << "Bit number out of Range" << std:: endl;
		setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
	}

	// normal bit range
	else {
		uint32_t val = readRegister(addr);
		writeRegister(addr,val & ~(1<<n));
	}

	return readRegister(addr);
}



string slsDetector::setNetworkParameter(networkParameter index, string value) {
	int i;
	switch (index) {
	case DETECTOR_MAC:
		return setDetectorMAC(value);
	case DETECTOR_IP:
		return setDetectorIP(value);
	case RECEIVER_HOSTNAME:
		return setReceiver(value);
	case RECEIVER_UDP_IP:
		return setReceiverUDPIP(value);
	case RECEIVER_UDP_MAC:
		return setReceiverUDPMAC(value);
	case RECEIVER_UDP_PORT:
		sscanf(value.c_str(),"%d",&i);
		setReceiverUDPPort(i);
		return getReceiverUDPPort();
	case RECEIVER_UDP_PORT2:
		sscanf(value.c_str(),"%d",&i);
		if(thisDetector->myDetectorType == EIGER) {
			setReceiverUDPPort2(i);
			return getReceiverUDPPort2();
		} else {
			setReceiverUDPPort(i);
			return getReceiverUDPPort();
		}
	case DETECTOR_TXN_DELAY_LEFT:
	case DETECTOR_TXN_DELAY_RIGHT:
	case DETECTOR_TXN_DELAY_FRAME:
	case FLOW_CONTROL_10G:
		sscanf(value.c_str(),"%d",&i);
		return setDetectorNetworkParameter(index, i);
	case CLIENT_STREAMING_PORT:
		return setClientStreamingPort(value);
	case RECEIVER_STREAMING_PORT:
		return setReceiverStreamingPort(value);
	case CLIENT_STREAMING_SRC_IP:
		return setClientStreamingIP(value);
	case RECEIVER_STREAMING_SRC_IP:
		return setReceiverStreamingIP(value);
	case ADDITIONAL_JSON_HEADER:
		return setAdditionalJsonHeader(value);
	case RECEIVER_UDP_SCKT_BUF_SIZE:
		sscanf(value.c_str(),"%d",&i);
		setReceiverUDPSocketBufferSize(i);
		return getReceiverUDPSocketBufferSize();

	default:
		return (char*)("unknown network parameter");
	}

}



string slsDetector::getNetworkParameter(networkParameter index) {
	ostringstream ss;string s;
	switch (index) {
	case DETECTOR_MAC:
		return getDetectorMAC();
	case DETECTOR_IP:
		return getDetectorIP();
	case RECEIVER_HOSTNAME:
		return getReceiver();
	case RECEIVER_UDP_IP:
		return getReceiverUDPIP();
	case RECEIVER_UDP_MAC:
		return getReceiverUDPMAC();
	case RECEIVER_UDP_PORT:
		return getReceiverUDPPort();
	case RECEIVER_UDP_PORT2:
		return getReceiverUDPPort2();
	case DETECTOR_TXN_DELAY_LEFT:
	case DETECTOR_TXN_DELAY_RIGHT:
	case DETECTOR_TXN_DELAY_FRAME:
	case FLOW_CONTROL_10G:
		return setDetectorNetworkParameter(index, -1);
	case CLIENT_STREAMING_PORT:
		return getClientStreamingPort();
	case RECEIVER_STREAMING_PORT:
		return getReceiverStreamingPort();
	case CLIENT_STREAMING_SRC_IP:
		return getClientStreamingIP();
	case RECEIVER_STREAMING_SRC_IP:
		return getReceiverStreamingIP();
	case ADDITIONAL_JSON_HEADER:
		return getAdditionalJsonHeader();
	case RECEIVER_UDP_SCKT_BUF_SIZE:
		return getReceiverUDPSocketBufferSize();
	case RECEIVER_REAL_UDP_SCKT_BUF_SIZE:
		return getReceiverRealUDPSocketBufferSize();

	default:
		return (char*)("unknown network parameter");
	}

}



string slsDetector::getDetectorMAC() {
	return string(thisDetector->detectorMAC);
}

string slsDetector::getDetectorIP() {
	return string(thisDetector->detectorIP);
}

string slsDetector::getReceiver() {
	return string(thisDetector->receiver_hostname);
}

string slsDetector::getReceiverUDPIP() {
	return string(thisDetector->receiverUDPIP);
}

string slsDetector::getReceiverUDPMAC() {
	return string(thisDetector->receiverUDPMAC);
}
string slsDetector::getReceiverUDPPort() {
	ostringstream ss; ss << thisDetector->receiverUDPPort; string s = ss.str();
	return s;
}
string slsDetector::getReceiverUDPPort2() {
	ostringstream ss; ss << thisDetector->receiverUDPPort2; string s = ss.str();
	return s;
}
string slsDetector::getClientStreamingPort() {
	ostringstream ss; ss << thisDetector->zmqport; string s = ss.str();
	return s;
}
string slsDetector::getReceiverStreamingPort() {
	ostringstream ss; ss << thisDetector->receiver_zmqport; string s = ss.str();
	return s;
}
string slsDetector::getClientStreamingIP() {
	return string(thisDetector->zmqip);
}
string slsDetector::getReceiverStreamingIP() {
	return string(thisDetector->receiver_zmqip);
}
string slsDetector::getAdditionalJsonHeader() {
	return string(thisDetector->receiver_additionalJsonHeader);
}
string slsDetector::getReceiverUDPSocketBufferSize() {
	return setReceiverUDPSocketBufferSize();
}

string slsDetector::getReceiverRealUDPSocketBufferSize() {

	int fnum=F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE;
	int ret = FAIL;
	int retval = -1;

	if(thisDetector->receiverOnlineFlag == ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Getting real UDP Socket Buffer size to receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(ret==FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			printf("Warning: Could not get real socket buffer size\n");
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	ostringstream ss;
	ss << retval;
	string s = ss.str();
	return s;
}


string slsDetector::setDetectorMAC(string detectorMAC) {
	if(detectorMAC.length()==17){
		if((detectorMAC[2]==':')&&(detectorMAC[5]==':')&&(detectorMAC[8]==':')&&
				(detectorMAC[11]==':')&&(detectorMAC[14]==':')){
			strcpy(thisDetector->detectorMAC,detectorMAC.c_str());
			if(!strcmp(thisDetector->receiver_hostname,"none"))
#ifdef VERBOSE
				std::cout << "Warning: Receiver hostname not set yet." << endl;
#else
			;
#endif
			else if(setUDPConnection()==FAIL)
				std::cout<< "Warning: UDP connection set up failed" << std::endl;
		}else{
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			std::cout << "Warning: server MAC Address should be in xx:xx:xx:xx:xx:xx "
					"format" << endl;
		}
	}
	else{
		setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		std::cout << "Warning: server MAC Address should be in xx:xx:xx:xx:xx:xx "
				"format" << std::endl;
	}

	return string(thisDetector->detectorMAC);
}



string slsDetector::setDetectorIP(string detectorIP) {
	struct sockaddr_in sa;
	//taking function arguments into consideration
	if(detectorIP.length()){
		if(detectorIP.length()<16){
			int result = inet_pton(AF_INET, detectorIP.c_str(), &(sa.sin_addr));
			if(result!=0){
				strcpy(thisDetector->detectorIP,detectorIP.c_str());
				if(!strcmp(thisDetector->receiver_hostname,"none"))
#ifdef VERBOSE
					std::cout << "Warning: Receiver hostname not set yet." << endl;
#else
				;
#endif
				else if(setUDPConnection()==FAIL)
					std::cout<< "Warning: UDP connection set up failed" << std::endl;
			}else{
				setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
				std::cout << "Warning: Detector IP Address should be VALID and in "
						"xxx.xxx.xxx.xxx format" << std::endl;
			}
		}
	}
	return string(thisDetector->detectorIP);
}



string slsDetector::setReceiver(string receiverIP) {

	if(receiverIP == "none") {
		memset(thisDetector->receiver_hostname, 0, MAX_STR_LENGTH);
		strcpy(thisDetector->receiver_hostname,"none");
		thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
		return string(thisDetector->receiver_hostname);
	}

	if(getRunStatus()==RUNNING){
		cprintf(RED,"Acquisition already running, Stopping it.\n");
		stopAcquisition();
	}
	updateDetector();

	strcpy(thisDetector->receiver_hostname,receiverIP.c_str());

	if(setReceiverOnline(ONLINE_FLAG)==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Setting up receiver with" << endl;
		std::cout << "detector type:" << slsDetectorBase::getDetectorType(
				thisDetector->myDetectorType) << endl;
		std::cout << "detector id:" << detId << endl;
		std::cout << "detector hostname:" << thisDetector->hostname << endl;
		std::cout << "file path:" << fileIO::getFilePath() << endl;
		std::cout << "file name:" << fileIO::getFileName() << endl;
		std::cout << "file index:" << fileIO::getFileIndex() << endl;
		std::cout << "file format:" << fileIO::getFileFormat() << endl;
		std::cout << "r_framesperfile:" << thisDetector->receiver_framesPerFile << endl;
		std::cout << "r_discardpolicy:" << thisDetector->receiver_frameDiscardMode << endl;
		std::cout << "r_padding:" << thisDetector->receiver_framePadding << endl;
		pthread_mutex_lock(&ms);
		std::cout << "write enable:" << multiDet->enableWriteToFileMask() << endl;
		std::cout << "overwrite enable:" << multiDet->enableOverwriteMask() << endl;
		pthread_mutex_unlock(&ms);
		std::cout << "frame index needed:" <<  ((thisDetector->timerValue[FRAME_NUMBER]
			*thisDetector->timerValue[CYCLES_NUMBER])>1) << endl;
		std::cout << "frame period:" << thisDetector->timerValue[FRAME_PERIOD] << endl;
		std::cout << "frame number:" << thisDetector->timerValue[FRAME_NUMBER] << endl;
		std::cout << "sub exp time:" << thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME]
																 << endl;
		std::cout << "sub dead time:" << thisDetector->timerValue[SUBFRAME_DEADTIME] << endl;
		std::cout << "dynamic range:" << thisDetector->dynamicRange << endl;
		std::cout << "flippeddatax:" << thisDetector->flippedData[X] << endl;
		if (thisDetector->myDetectorType == EIGER) {
			std::cout << "activated: " << thisDetector->activated << endl;
			std::cout << "receiver deactivated padding: " << thisDetector->receiver_deactivatedPaddingEnable << endl;
		}
		std::cout << "silent Mode:" << thisDetector->receiver_silentMode << endl;
		std::cout << "10GbE:" << thisDetector->tenGigaEnable << endl;
		std::cout << "Gap pixels: " << thisDetector->gappixels << endl;
		std::cout << "rx streaming source ip:" << thisDetector->receiver_zmqip << endl;
		std::cout << "rx additional json header:" << thisDetector->receiver_additionalJsonHeader << endl;
		std::cout << "enable gap pixels:" << thisDetector->gappixels << endl;
		std::cout << "rx streaming port:" << thisDetector->receiver_zmqport << endl;
		std::cout << "r_readfreq:" << thisDetector->receiver_read_freq << endl;
		std::cout << "rx_datastream:" << enableDataStreamingFromReceiver(-1) << endl << endl;

#endif
		if(setDetectorType()!= GENERIC){
			sendMultiDetectorSize();
			setDetectorId();
			setDetectorHostname();
			setUDPConnection();
			//setReceiverUDPSocketBufferSize(atoi(getReceiverUDPSocketBufferSize().c_str()));

			setFilePath(fileIO::getFilePath());
			setFileName(fileIO::getFileName());
			setFileIndex(fileIO::getFileIndex());
			setFileFormat(fileIO::getFileFormat());
			setReceiverFramesPerFile(thisDetector->receiver_framesPerFile);
			setReceiverFramesDiscardPolicy(thisDetector->receiver_frameDiscardMode);
			setReceiverPartialFramesPadding(thisDetector->receiver_framePadding);
			pthread_mutex_lock(&ms);
			int imask = multiDet->enableWriteToFileMask();
			pthread_mutex_unlock(&ms);
			enableWriteToFile(imask);
			pthread_mutex_lock(&ms);
			imask = multiDet->enableOverwriteMask();
			pthread_mutex_unlock(&ms);
			overwriteFile(imask);
			setTimer(FRAME_PERIOD,thisDetector->timerValue[FRAME_PERIOD]);
			setTimer(FRAME_NUMBER,thisDetector->timerValue[FRAME_NUMBER]);
			setTimer(ACQUISITION_TIME,thisDetector->timerValue[ACQUISITION_TIME]);
			if(thisDetector->myDetectorType == EIGER) {
				setTimer(SUBFRAME_ACQUISITION_TIME,
						thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME]);
				setTimer(SUBFRAME_DEADTIME,thisDetector->timerValue[SUBFRAME_DEADTIME]);
			}
			if(thisDetector->myDetectorType == JUNGFRAUCTB)
				setTimer(SAMPLES_JCTB,thisDetector->timerValue[SAMPLES_JCTB]);
			setDynamicRange(thisDetector->dynamicRange);
			if(thisDetector->myDetectorType == EIGER){
				setFlippedData(X,-1);
				activate(-1);
				setDeactivatedRxrPaddingMode(thisDetector->receiver_deactivatedPaddingEnable);
			}
			setReceiverSilentMode(thisDetector->receiver_silentMode);

			if(thisDetector->myDetectorType == EIGER)
				enableTenGigabitEthernet(thisDetector->tenGigaEnable);

			enableGapPixels(thisDetector->gappixels);

			// data streaming
			setReadReceiverFrequency(thisDetector->receiver_read_freq);
			setReceiverStreamingPort(getReceiverStreamingPort());
			setReceiverStreamingIP(getReceiverStreamingIP());
			setAdditionalJsonHeader(getAdditionalJsonHeader());
			enableDataStreamingFromReceiver(enableDataStreamingFromReceiver(-1));

			if(thisDetector->myDetectorType == GOTTHARD)
				sendROI(-1, NULL);
		}
	}

	return string(thisDetector->receiver_hostname);
}




string slsDetector::setReceiverUDPIP(string udpip) {
	struct sockaddr_in sa;
	//taking function arguments into consideration
	if(udpip.length()){
		if(udpip.length()<16){
			int result = inet_pton(AF_INET, udpip.c_str(), &(sa.sin_addr));
			if(result==0){
				setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
				std::cout << "Warning: Receiver UDP IP Address should be VALID "
						"and in xxx.xxx.xxx.xxx format" << std::endl;
			}else{
				strcpy(thisDetector->receiverUDPIP,udpip.c_str());
				if(!strcmp(thisDetector->receiver_hostname,"none")) {
#ifdef VERBOSE
					std::cout << "Warning: Receiver hostname not set yet." << endl;
#else
					;
#endif
				}
				else if(setUDPConnection()==FAIL){
					std::cout<< "Warning: UDP connection set up failed" << std::endl;
				}
			}
		}
	}
	return string(thisDetector->receiverUDPIP);
}




string slsDetector::setReceiverUDPMAC(string udpmac) {
	if(udpmac.length()!=17){
		setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		std::cout << "Warning: receiver udp mac address should be in xx:xx:xx:xx:xx:xx "
				"format" << std::endl;
	}
	else{
		if((udpmac[2]==':')&&(udpmac[5]==':')&&(udpmac[8]==':')&&
				(udpmac[11]==':')&&(udpmac[14]==':')){
			strcpy(thisDetector->receiverUDPMAC,udpmac.c_str());
			if(!strcmp(thisDetector->receiver_hostname,"none")) {
#ifdef VERBOSE
				std::cout << "Warning: Receiver hostname not set yet." << endl;
#else
			;
#endif
		}
			/* else  if(setUDPConnection()==FAIL){ commented out to be replaced by user
			 * defined udpmac
    	  std::cout<< "Warning: UDP connection set up failed" << std::endl;
      }*/
		}else{
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			std::cout << "Warning: receiver udp mac address should be in xx:xx:xx:xx:xx:xx "
					"format" << std::endl;
		}
	}


	return string(thisDetector->receiverUDPMAC);
}



int slsDetector::setReceiverUDPPort(int udpport) {
	thisDetector->receiverUDPPort = udpport;
	if(!strcmp(thisDetector->receiver_hostname,"none"))
#ifdef VERBOSE
		std::cout << "Warning: Receiver hostname not set yet." << endl;
#else
	;
#endif
	else if(setUDPConnection()==FAIL){
		std::cout<< "Warning: UDP connection set up failed" << std::endl;
	}
	return thisDetector->receiverUDPPort;
}

int slsDetector::setReceiverUDPPort2(int udpport) {
	thisDetector->receiverUDPPort2 = udpport;
	if(!strcmp(thisDetector->receiver_hostname,"none"))
#ifdef VERBOSE
		std::cout << "Warning: Receiver hostname not set yet." << endl;
#else
	;
#endif
	else if(setUDPConnection()==FAIL){
		std::cout<< "Warning: UDP connection set up failed" << std::endl;
	}
	return thisDetector->receiverUDPPort2;
}


string slsDetector::setClientStreamingPort(string port) {
	int defaultport = 0;
	int numsockets = (thisDetector->myDetectorType == EIGER) ? 2:1;
	int arg = 0;

	//multi command, calculate individual ports
	size_t found = port.find("multi");
	if(found != string::npos) {
		port.erase(found,5);
		sscanf(port.c_str(),"%d",&defaultport);
		arg = defaultport + (detId * numsockets);
	}
	else
		sscanf(port.c_str(),"%d",&arg);
	thisDetector->zmqport = arg;

	return getClientStreamingPort();
}




string slsDetector::setReceiverStreamingPort(string port) {
	int defaultport = 0;
	int numsockets = (thisDetector->myDetectorType == EIGER) ? 2:1;
	int arg = 0;

	//multi command, calculate individual ports
	size_t found = port.find("multi");
	if(found != string::npos) {
		port.erase(found,5);
		sscanf(port.c_str(),"%d",&defaultport);
		arg = defaultport + (detId * numsockets);
	}
	else
		sscanf(port.c_str(),"%d",&arg);

	thisDetector->receiver_zmqport = arg;

	// send to receiver
	int fnum=F_SET_RECEIVER_STREAMING_PORT;
	int ret = FAIL;
	int retval=-1;
	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending receiver streaming port to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if (ret==FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			std::cout << "Warning: Could not set receiver zmq port" << std::endl;
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return getReceiverStreamingPort();
}

string slsDetector::setClientStreamingIP(string sourceIP) {

	struct addrinfo *result;
	// on failure to convert to a valid ip
	if (dataSocket->ConvertHostnameToInternetAddress(sourceIP.c_str(), &result)) {
		std::cout << "Warning: Could not convert zmqip into a valid IP" << sourceIP
				<< std::endl;
		setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		return getClientStreamingIP();
	}
	// on success put IP as string into arg
	else {
		memset(thisDetector->zmqip, 0, MAX_STR_LENGTH);
		dataSocket->ConvertInternetAddresstoIpString(result, thisDetector->zmqip,
				MAX_STR_LENGTH);
	}

	return getClientStreamingIP();
}


string slsDetector::setReceiverStreamingIP(string sourceIP) {

	int fnum=F_RECEIVER_STREAMING_SRC_IP;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH];
	memset(arg,0,sizeof(arg));
	char retval[MAX_STR_LENGTH];
	memset(retval,0, sizeof(retval));

	// if empty, give rx_hostname
	if (sourceIP.empty())  {
		if(!strcmp(thisDetector->receiver_hostname,"none")) {
			std::cout << "Receiver hostname not set yet. Cannot create rx_zmqip "
					"from none\n" << endl;
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			return getReceiverStreamingIP();
		}
		sourceIP.assign(thisDetector->receiver_hostname);
	}

	// verify the ip
	{
		struct addrinfo *result;
		// on failure to convert to a valid ip
		if (dataSocket->ConvertHostnameToInternetAddress(sourceIP.c_str(), &result)) {
			std::cout << "Warning: Could not convert rx_zmqip into a valid IP" <<
					sourceIP << std::endl;
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			return getReceiverStreamingIP();
		}
		// on success put IP as string into arg
		else {
			dataSocket->ConvertInternetAddresstoIpString(result, arg, MAX_STR_LENGTH);
		}
	}

	// set it anyway, else it is lost
	memset(thisDetector->receiver_zmqip, 0, MAX_STR_LENGTH);
	strcpy(thisDetector->receiver_zmqip, arg);


	// if zmqip is empty, update it
	if (! strlen(thisDetector->zmqip))
		strcpy(thisDetector->zmqip, arg);


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending receiver streaming source ip to receiver " << arg <<
				std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			std::cout << "Warning: Could not set rx_zmqip" << std::endl;
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return getReceiverStreamingIP();
}



string slsDetector::setAdditionalJsonHeader(string jsonheader) {

	int fnum=F_ADDITIONAL_JSON_HEADER;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH];
	memset(arg,0,sizeof(arg));
	char retval[MAX_STR_LENGTH];
	memset(retval,0, sizeof(retval));

	strcpy(arg, jsonheader.c_str());

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending additional json header " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			std::cout << "Warning: Could not set additional json header" << std::endl;
		} else
			strcpy(thisDetector->receiver_additionalJsonHeader, retval);
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return getAdditionalJsonHeader();
}


string slsDetector::setReceiverUDPSocketBufferSize(int udpsockbufsize) {

	int fnum=F_RECEIVER_UDP_SOCK_BUF_SIZE;
	int ret = FAIL;
	int retval = -1;
	int arg = udpsockbufsize;

	if(thisDetector->receiverOnlineFlag == ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending UDP Socket Buffer size to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			printf("Warning: Could not set udp socket buffer size\n");
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	ostringstream ss;
	ss << retval;
	string s = ss.str();
	return s;
}





string slsDetector::setDetectorNetworkParameter(networkParameter index, int delay) {
	int fnum = F_SET_NETWORK_PARAMETER;
	int ret = FAIL;
	int retval = -1;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< "Setting Transmission delay of mode "<< index << " to " <<  delay << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&index,sizeof(index));
			controlSocket->SendDataOnly(&delay,sizeof(delay));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(DETECTOR_NETWORK_PARAMETER));
			} else
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
#ifdef VERBOSE
	std::cout<< "Speed set to  "<< retval  << std::endl;
#endif

	ostringstream ss;
	ss << retval;
	string s =  ss.str();
	return s;
}


int slsDetector::setUDPConnection() {

	int ret = FAIL;
	int fnum = F_SETUP_RECEIVER_UDP;
	char args[3][MAX_STR_LENGTH];
	memset(args,0,sizeof(args));
	char retval[MAX_STR_LENGTH];
	memset(retval,0,sizeof(retval));

	//called before set up
	if(!strcmp(thisDetector->receiver_hostname,"none")){
#ifdef VERBOSE
		std::cout << "Warning: Receiver hostname not set yet." << endl;
#endif
		return FAIL;
	}

	//if no udp ip given, use hostname
	if(!strcmp(thisDetector->receiverUDPIP,"none")){
		//hostname is an ip address
		if(strchr(thisDetector->receiver_hostname,'.')!=NULL)
			strcpy(thisDetector->receiverUDPIP,thisDetector->receiver_hostname);
		//if hostname not ip, convert it to ip
		else{
			struct addrinfo *result;
			if (!dataSocket->ConvertHostnameToInternetAddress(
					thisDetector->receiver_hostname, &result)) {
				// on success
				memset(thisDetector->receiverUDPIP, 0, MAX_STR_LENGTH);
				// on failure, back to none
				if (dataSocket->ConvertInternetAddresstoIpString(result,
						thisDetector->receiverUDPIP, MAX_STR_LENGTH)) {
					strcpy(thisDetector->receiverUDPIP, "none");
				}
			}
		}
	}


	//copy arguments to args[][]
	strcpy(args[0],thisDetector->receiverUDPIP);
	sprintf(args[1],"%d",thisDetector->receiverUDPPort);
	sprintf(args[2],"%d",thisDetector->receiverUDPPort2);
#ifdef VERBOSE
	std::cout << "Receiver udp ip address: " << thisDetector->receiverUDPIP << std::endl;
	std::cout << "Receiver udp port: " << thisDetector->receiverUDPPort << std::endl;
	std::cout << "Receiver udp port2: " << thisDetector->receiverUDPPort2 << std::endl;
#endif

	//set up receiver for UDP Connection and get receivermac address
	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Setting up UDP Connection for Receiver " << args[0] << "\t" << args[1] << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendUDPDetails(fnum,retval,args);
			disconnectData();
		}
		if(ret!=FAIL){
			strcpy(thisDetector->receiverUDPMAC,retval);

#ifdef VERBOSE
			std::cout << "Receiver mac address: " << thisDetector->receiverUDPMAC << std::endl;
#endif
			if(ret==FORCE_UPDATE)
				updateReceiver();

			//configure detector with udp details, -100 is so it doesnt overwrite
			//the previous value
			if(configureMAC()==FAIL){
				setReceiverOnline(OFFLINE_FLAG);
				std::cout << "could not configure mac" << endl;
			}
		}
	}else {
		ret=FAIL;
		setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
	}
#ifdef VERBOSE
	printReceiverConfiguration();
#endif
	return ret;
}



int slsDetector::digitalTest( digitalTestMode mode, int imod) {


	int retval;
	int fnum=F_DIGITAL_TEST;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Getting id of "<< mode << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&mode,sizeof(mode));
			if ((mode==CHIP_TEST)|| (mode==DIGITAL_BIT_TEST))
				controlSocket->SendDataOnly(&imod,sizeof(imod));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	} else {
		ret=FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Id "<< mode <<" is " << retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Get id failed " << std::endl;
		return ret;
	} else
		return retval;
}



int slsDetector::executeTrimming(trimMode mode, int par1, int par2, int imod) {

	int fnum= F_EXECUTE_TRIMMING;
	int retval=FAIL;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;
	int arg[3];
	arg[0]=imod;
	arg[1]=par1;
	arg[2]=par2;


#ifdef VERBOSE
	std::cout<< "Trimming module " << imod << " with mode "<< mode << " parameters "
			<< par1 << " " << par2 << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
#ifdef VERBOSE
			std::cout<< "sending mode bytes= "<<  controlSocket->SendDataOnly(
					&mode,sizeof(mode)) << std::endl;
#endif
			controlSocket->SendDataOnly(&mode,sizeof(mode));
			controlSocket->SendDataOnly(arg,sizeof(arg));

			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			} else {
#ifdef VERBOSE
				std::cout<< "Detector trimmed "<< ret   << std::endl;
#endif
				/*
	  controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
	  thisDetector->roFlags=retval;
				 */
				retval=ret;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return retval;

}



int slsDetector::loadImageToDetector(imageType index,string const fname) {

	int ret=FAIL;
	short int arg[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];

#ifdef VERBOSE
	std::cout<< std::endl<< "Loading ";
	if(!index)
		std::cout<<"Dark";
	else
		std::cout<<"Gain";
	std::cout<<" image from file " << fname << std::endl;
#endif

	if(readDataFile(fname,arg)){
		ret = sendImageToDetector(index,arg);
		return ret;
	}
	std::cout<< "Could not open file "<< fname << std::endl;
	return ret;
}


int slsDetector::sendImageToDetector(imageType index,short int imageVals[]) {

	int ret=FAIL;
	int retval;
	int fnum=F_LOAD_IMAGE;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<<"Sending image to detector " <<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&index,sizeof(index));
			controlSocket->SendDataOnly(imageVals,thisDetector->dataBytes);
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}



int slsDetector::writeCounterBlockFile(string const fname,int startACQ) {

	int ret=FAIL;
	short int counterVals[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];

#ifdef VERBOSE
	std::cout<< std::endl<< "Reading Counter to \""<<fname;
	if(startACQ==1)
		std::cout<<"\" and Restarting Acquisition";
	std::cout<<std::endl;
#endif

	ret=getCounterBlock(counterVals,startACQ);
	if(ret==OK)
		ret=writeDataFile(fname,counterVals);
	return ret;
}



int slsDetector::getCounterBlock(short int arg[],int startACQ) {

	int ret=FAIL;
	int fnum=F_READ_COUNTER_BLOCK;
	char mess[MAX_STR_LENGTH]="";

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&startACQ,sizeof(startACQ));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(arg,thisDetector->dataBytes);
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}




int slsDetector::resetCounterBlock(int startACQ) {

	int ret=FAIL;
	int fnum=F_RESET_COUNTER_BLOCK;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< std::endl<< "Resetting Counter";
	if(startACQ==1)
		std::cout<<" and Restarting Acquisition";
	std::cout<<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&startACQ,sizeof(startACQ));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}




int slsDetector::setCounterBit(int i) {
	int fnum=F_SET_COUNTER_BIT;
	int ret = FAIL;
	int retval=-1;
	char mess[MAX_STR_LENGTH]="";

	if(thisDetector->onlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		if(i ==-1)
			std::cout<< "Getting counter bit from detector" << endl;
		else if(i==0)
			std::cout<< "Resetting counter bit in detector " << endl;
		else
			std::cout<< "Setting counter bit in detector " <<  endl;
#endif
		if (connectControl() == OK){

			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&i,sizeof(i));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_SET_COUNTER_BIT));
			}
			controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();

		}
	}
	return retval;
}




int slsDetector::setROI(int n,ROI roiLimits[], int imod) {
	int ret = FAIL;
	//sort ascending order
	int temp;

	for(int i=0;i<n;++i){

		//	  cout << "*** ROI "<< i << " xmin " << roiLimits[i].xmin << " xmax "
		//<< roiLimits[i].xmax << endl;
		for(int j=i+1;j<n;++j){
			if(roiLimits[j].xmin<roiLimits[i].xmin){

				temp=roiLimits[i].xmin;roiLimits[i].xmin=roiLimits[j].xmin;
				roiLimits[j].xmin=temp;

				temp=roiLimits[i].xmax;roiLimits[i].xmax=roiLimits[j].xmax;
				roiLimits[j].xmax=temp;

				temp=roiLimits[i].ymin;roiLimits[i].ymin=roiLimits[j].ymin;
				roiLimits[j].ymin=temp;

				temp=roiLimits[i].ymax;roiLimits[i].ymax=roiLimits[j].ymax;
				roiLimits[j].ymax=temp;
			}
		}
		//	cout << "UUU ROI "<< i << " xmin " << roiLimits[i].xmin << " xmax "
		//<< roiLimits[i].xmax  << endl;
	}

	ret = sendROI(n,roiLimits);

	if(thisDetector->myDetectorType==JUNGFRAUCTB) getTotalNumberOfChannels();
	return ret;
}


slsDetectorDefs::ROI* slsDetector::getROI(int &n, int imod) {
	sendROI(-1,NULL);
	n=thisDetector->nROI;
	if(thisDetector->myDetectorType==JUNGFRAUCTB) getTotalNumberOfChannels();
	return thisDetector->roiLimits;
}

int slsDetector::getNRoi(){
	return thisDetector->nROI;
}


int slsDetector::sendROI(int n,ROI roiLimits[]) {
	int ret=FAIL;
	int fnum=F_SET_ROI;
	char mess[MAX_STR_LENGTH]="";
	int arg = n;
	int retvalsize=0;
	ROI retval[MAX_ROIS];
	int nrec=-1;
	if (roiLimits==NULL)
		roiLimits=thisDetector->roiLimits;

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&arg,sizeof(arg));
			if(arg==-1){;
#ifdef VERBOSE
			cout << "Getting ROI from detector" << endl;
#endif
			}else{
#ifdef VERBOSE
				cout << "Sending ROI of size " << arg << " to detector" << endl;
#endif
				controlSocket->SendDataOnly(roiLimits,arg*sizeof(ROI));
			}
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));

			if (ret!=FAIL){
				controlSocket->ReceiveDataOnly(&retvalsize,sizeof(retvalsize));
				nrec = controlSocket->ReceiveDataOnly(retval,retvalsize*sizeof(ROI));
				if(nrec!=(retvalsize*(int)sizeof(ROI))){
					ret=FAIL;
					std::cout << " wrong size received: received " << nrec <<
							"but expected " << retvalsize*sizeof(ROI) << endl;
				}
			}else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	//update client
	if(ret==FAIL){
		setErrorMask((getErrorMask())|(COULDNOT_SET_ROI));
	} else {
		for(int i=0;i<retvalsize;++i)
			thisDetector->roiLimits[i]=retval[i];
		thisDetector->nROI = retvalsize;
	}

#ifdef VERBOSE
	for(int j=0;j<thisDetector->nROI;++j)
		cout<<"ROI [" <<j<<"] ("<< roiLimits[j].xmin<<"\t"<<roiLimits[j].xmax<<"\t"
		<<roiLimits[j].ymin<<"\t"<<roiLimits[j].ymax<<")"<<endl;
#endif

	// old firmware requires configuremac after setting roi
	if (thisDetector->myDetectorType == GOTTHARD && n != -1) {
		configureMAC();
	}

	// update roi in receiver
	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
		int fnum=F_RECEIVER_SET_ROI;
#ifdef VERBOSE
		std::cout << "Sending ROI to receiver " << thisDetector->nROI << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendROI(fnum, thisDetector->nROI, thisDetector->roiLimits);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(COULDNOT_SET_ROI));
	}


	return ret;
}




int slsDetector::writeAdcRegister(int addr, int val) {

	int retval=-1;
	int fnum=F_WRITE_ADC_REG;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	uint32_t arg[2];
	arg[0]=addr;
	arg[1]=val;


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Writing to adc register "<< hex<<addr <<  " data " << hex<<val << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
#ifdef VERBOSE
	std::cout<< "ADC Register returned "<< retval << std::endl;
#endif
	if (ret==FAIL) {
		std::cout<< "Write ADC to register failed " << std::endl;
		setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
	}
	return retval;

}


int slsDetector::activate(int const enable) {
	int fnum = F_ACTIVATE;
	int fnum2 = F_RECEIVER_ACTIVATE;
	int retval = -1;
	int arg = enable;
	char mess[MAX_STR_LENGTH]="";
	int ret = FAIL;

	if(thisDetector->myDetectorType != EIGER){
		std::cout<< "Not implemented for this detector" << std::endl;
		setErrorMask((getErrorMask())|(DETECTOR_ACTIVATE));
		return -1;
	}

#ifdef VERBOSE
	if(!enable)
		std::cout<< "Deactivating Detector" << std::endl;
	else if(enable == -1)
		std::cout<< "Getting Detector activate mode" << std::endl;
	else
		std::cout<< "Activating Detector" << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(DETECTOR_ACTIVATE));
			} else {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				thisDetector->activated = retval;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
#ifdef VERBOSE
	if(retval==1)
		std::cout << "Detector Activated"  << std::endl;
	else if(retval==0)
		std::cout << "Detector Deactivated" << std::endl;
	else
		std::cout << "Detector Activation unknown:" << retval << std::endl;
#endif

	if(ret!=FAIL){
		int arg = thisDetector->activated;
		if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "Activating/Deactivating Receiver: " << arg << std::endl;
#endif
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum2,retval,arg);
				disconnectData();
			}
			if(ret==FAIL)
				setErrorMask((getErrorMask())|(RECEIVER_ACTIVATE));
		}
	}
#ifdef VERBOSE
	if(retval==1)
		std::cout << "Receiver Activated"  << std::endl;
	else if(retval==0)
		std::cout << "Receiver Deactivated" << std::endl;
	else
		std::cout << "Receiver Activation unknown:" << retval << std::endl;
#endif


	return thisDetector->activated;

}



int slsDetector::setDeactivatedRxrPaddingMode(int padding) {
	int fnum = F_RECEIVER_DEACTIVATED_PADDING_ENABLE;
	int retval = -1;
	int arg = padding;
	int ret = OK;

	if(thisDetector->myDetectorType != EIGER){
		std::cout<< "Not implemented for this detector" << std::endl;
		setErrorMask((getErrorMask())|(RECEIVER_ACTIVATE));
		return -1;
	}

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "Deactivated Receiver Padding Enable: " << arg << std::endl;
#endif
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum,retval,arg);
				disconnectData();
			}
			if(ret==FAIL)
				setErrorMask((getErrorMask())|(RECEIVER_ACTIVATE));
			else
				thisDetector->receiver_deactivatedPaddingEnable = retval;
		}

	return thisDetector->receiver_deactivatedPaddingEnable;
}




int slsDetector::getFlippedData(dimension d) {
	return thisDetector->flippedData[d];
}



int slsDetector::setFlippedData(dimension d, int value) {
	int retval=-1;
	int fnum=F_SET_FLIPPED_DATA_RECEIVER;
	int ret=FAIL;
	int args[2]={X,-1};


	if(thisDetector->myDetectorType!= EIGER){
		std::cout << "Flipped Data is not implemented in this detector" << std::endl;
		setErrorMask((getErrorMask())|(RECEIVER_FLIPPED_DATA_NOT_SET));
		return -1;
	}

#ifdef VERBOSE
	std::cout << std::endl;
	std::cout << "Setting/Getting flipped data across axis " << d <<" with value "
			<< value << std::endl;
#endif
	if(value > -1){
		thisDetector->flippedData[d] = value;
		args[1] = value;
	}else
		args[1] = thisDetector->flippedData[d];

	args[0] = d;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
		if (connectData() == OK){
			ret=thisReceiver->sendIntArray(fnum,retval,args);

			disconnectData();
		}

		if((args[1] != retval && args[1]>=0) || (ret==FAIL)){
			ret = FAIL;
			setErrorMask((getErrorMask())|(RECEIVER_FLIPPED_DATA_NOT_SET));
		}

		if(ret==FORCE_UPDATE)
			updateReceiver();
	}


	return thisDetector->flippedData[d];
}



int slsDetector::setAllTrimbits(int val, int imod) {
	int fnum=F_SET_ALL_TRIMBITS;
	int retval = FAIL;
	char mess[MAX_STR_LENGTH]="";
	int ret=OK;

#ifdef VERBOSE
	std::cout<< "Setting all trimbits to "<< val << std::endl;
#endif
	if (getDetectorsType() == MYTHEN) {
		if (val>=0) {
			setChannel((val<<((int)TRIMBIT_OFF))|((int)COMPARATOR_ENABLE)); // trimbit scan
		}
		return val;
	} else {
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&val,sizeof(val));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					setErrorMask((getErrorMask())|(ALLTIMBITS_NOT_SET));
				} else {
					controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				}
				disconnectControl();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}
	}

#ifdef VERBOSE
	std::cout<< "All trimbits were set to "<< retval   << std::endl;
#endif
	return retval;
}



int slsDetector::enableGapPixels(int val) {

	if(val > 0 && thisDetector->myDetectorType!= EIGER)
		val = -1;

	if (val >= 0) {
		val=(val>0)?1:0;

		// send to receiver
		int ret=OK;
		int retval=-1;
		int fnum=F_ENABLE_GAPPIXELS_IN_RECEIVER;
		int arg=val;
		if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum,retval,arg);
				disconnectData();
			}
			if((arg != retval) || (ret==FAIL)){
				ret = FAIL;
				setErrorMask((getErrorMask())|(RECEIVER_ENABLE_GAPPIXELS_NOT_SET));
			}

			if(ret==FORCE_UPDATE)
				updateReceiver();
		}


		// update client
		if (ret == OK) {
			thisDetector->gappixels = val;
			thisDetector->dataBytesInclGapPixels = 0;

			if (thisDetector->dynamicRange != 4) {
				thisDetector->dataBytesInclGapPixels =
						(thisDetector->nMod[X] * thisDetector->nChip[X] *
								thisDetector->nChan[X] + thisDetector->gappixels *
								thisDetector->nGappixels[X]) *
						(thisDetector->nMod[Y] * thisDetector->nChip[Y] *
								thisDetector->nChan[Y] + thisDetector->gappixels *
								thisDetector->nGappixels[Y]) *
						thisDetector->dynamicRange/8;
				// set data bytes for other detector ( for future use)
				if(thisDetector->myDetectorType==JUNGFRAUCTB)
					getTotalNumberOfChannels();
				else if(thisDetector->myDetectorType==MYTHEN){
					if (thisDetector->dynamicRange==24 ||
							thisDetector->timerValue[PROBES_NUMBER]>0) {
						thisDetector->dataBytesInclGapPixels =
								(thisDetector->nMod[X] * thisDetector->nChip[X] *
										thisDetector->nChan[X] + thisDetector->gappixels *
										thisDetector->nGappixels[X]) *
								(thisDetector->nMod[Y] * thisDetector->nChip[Y] *
										thisDetector->nChan[Y] + thisDetector->gappixels *
										thisDetector->nGappixels[Y]) *
								4;
					}
				}
			}
		}
	}

	return thisDetector->gappixels;
}



int slsDetector::setTrimEn(int nen, int *en) {
	if (en) {
		for (int ien=0; ien<nen; ien++)
			thisDetector->trimEnergies[ien]=en[ien];
		thisDetector->nTrimEn=nen;
	}
	return (thisDetector->nTrimEn);
}


int slsDetector::getTrimEn(int *en) {
	if (en) {
		for (int ien=0; ien<thisDetector->nTrimEn; ien++)
			en[ien]=thisDetector->trimEnergies[ien];
	}
	return (thisDetector->nTrimEn);
}




int slsDetector::pulsePixel(int n,int x,int y) {
	int ret=FAIL;
	int fnum=F_PULSE_PIXEL;
	char mess[MAX_STR_LENGTH]="";
	int arg[3];
	arg[0] = n; arg[1] = x; arg[2] = y;

#ifdef VERBOSE
	std::cout<< std::endl<< "Pulsing Pixel " << n << " number of times at (" <<
			x << "," << "y)" << endl << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_PULSE_PIXEL));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}


int slsDetector::pulsePixelNMove(int n,int x,int y) {
	int ret=FAIL;
	int fnum=F_PULSE_PIXEL_AND_MOVE;
	char mess[MAX_STR_LENGTH]="";
	int arg[3];
	arg[0] = n; arg[1] = x; arg[2] = y;

#ifdef VERBOSE
	std::cout<< std::endl<< "Pulsing Pixel " << n << " number of times and move "
			"by deltax:" << x << " deltay:" << y << endl << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_PULSE_PIXEL_NMOVE));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}



int slsDetector::pulseChip(int n) {
	int ret=FAIL;
	int fnum=F_PULSE_CHIP;
	char mess[MAX_STR_LENGTH]="";


#ifdef VERBOSE
	std::cout<< std::endl<< "Pulsing Pixel " << n << " number of times" << endl << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&n,sizeof(n));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(COULD_NOT_PULSE_CHIP));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;
}




int slsDetector::setThresholdTemperature(int val, int imod) {

	int retval = -1;
	int fnum = F_THRESHOLD_TEMP;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	int arg[2];
	arg[0]=val;
	arg[1]=imod;


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting/Getting Threshold Temperature to "<< val << " of module "
			<< imod  << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectStop() == OK){
			stopSocket->SendDataOnly(&fnum,sizeof(fnum));
			stopSocket->SendDataOnly(arg,sizeof(arg));
			stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
#ifdef VERBOSE
std::cout<< "Threshold Temperature returned "<< retval << std::endl;
#endif
			} else {
				stopSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(TEMPERATURE_CONTROL));
			}
			disconnectStop();
		}
	}

	return retval;
}



int slsDetector::setTemperatureControl(int val, int imod) {

	int retval = -1;
	int fnum = F_TEMP_CONTROL;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	int arg[2];
	arg[0]=val;
	arg[1]=imod;


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting/Getting Threshold Temperature to "<< val << " of module "
			<< imod  << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectStop() == OK){
			stopSocket->SendDataOnly(&fnum,sizeof(fnum));
			stopSocket->SendDataOnly(arg,sizeof(arg));
			stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
#ifdef VERBOSE
std::cout<< "Threshold Temperature returned "<< retval << std::endl;
#endif
			} else {
				stopSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(TEMPERATURE_CONTROL));
			}
			disconnectStop();
		}
	}

	return retval;
}




int slsDetector::setTemperatureEvent(int val, int imod) {

	int retval = -1;
	int fnum = F_TEMP_EVENT;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	int arg[2];
	arg[0]=val;
	arg[1]=imod;


#ifdef VERBOSE
	std::cout<< std::endl;
	std::cout<< "Setting/Getting Threshold Temperature to "<< val << " of module "
			<< imod  << std::endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectStop() == OK){
			stopSocket->SendDataOnly(&fnum,sizeof(fnum));
			stopSocket->SendDataOnly(arg,sizeof(arg));
			stopSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				stopSocket->ReceiveDataOnly(&retval,sizeof(retval));
#ifdef VERBOSE
std::cout<< "Threshold Temperature returned "<< retval << std::endl;
#endif
			} else {
				stopSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(TEMPERATURE_CONTROL));
			}
			disconnectStop();
		}
	}

	return retval;
}



int slsDetector::setStoragecellStart(int pos) {
	int ret=FAIL;
	int fnum=F_STORAGE_CELL_START;
	char mess[MAX_STR_LENGTH];
	memset(mess, 0, MAX_STR_LENGTH);
	int retval=-1;

#ifdef VERBOSE
	std::cout<< "Sending storage cell start index " << pos << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&pos,sizeof(pos));
			//check opening error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(STORAGE_CELL_START));
			}else
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return retval;
}



int slsDetector::programFPGA(string fname) {
	int ret=FAIL;
	int fnum=F_PROGRAM_FPGA;
	char mess[MAX_STR_LENGTH]="";
	size_t filesize=0;
	char* fpgasrc = NULL;

	if(thisDetector->myDetectorType != JUNGFRAU &&
			thisDetector->myDetectorType != JUNGFRAUCTB){
		std::cout << "Not implemented for this detector" << std::endl;
		return FAIL;
	}


	//check if it exists
	struct stat st;
	if(stat(fname.c_str(),&st)){
		std::cout << "Programming file does not exist" << endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}

	// open src
	FILE* src = fopen(fname.c_str(),"rb");
	if (src == NULL) {
		std::cout << "Could not open source file for programming: " << fname << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}

	// create temp destination file
	char destfname[] = "/tmp/Jungfrau_MCB.XXXXXX";
	int dst = mkstemp(destfname); // create temporary file and open it in r/w
	if (dst == -1) {
		std::cout << "Could not create destination file in /tmp for programming: "
				<< destfname << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}

	// convert src to dst rawbin
#ifdef VERBOSE
	std::cout << "Converting " << fname << " to " <<  destfname << std::endl;
#endif
	int filepos,x,y,i;
	// Remove header (0...11C)
	for (filepos=0; filepos < 0x11C; ++filepos)
		fgetc(src);
	// Write 0x80 times 0xFF (0...7F)
	{
		char c = 0xFF;
		for (filepos=0; filepos < 0x80; ++filepos)
			write(dst, &c, 1);
	}
	// Swap bits and write to file
	for (filepos=0x80; filepos < 0x1000000; ++filepos)	{
		x = fgetc(src);
		if (x < 0) break;
		y=0;
		for (i=0; i < 8; ++i)
			y=y| (   (( x & (1<<i) ) >> i)    << (7-i)     );	// This swaps the bits
		write(dst, &y, 1);
	}
	if (filepos < 0x1000000){
		std::cout << "Could not convert programming file. EOF before end of flash"
				<< std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	if(fclose(src)){
		std::cout << "Could not close source file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	if(close(dst)){
		std::cout << "Could not close destination file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
#ifdef VERBOSE
	std::cout << "File has been converted to "  <<  destfname << std::endl;
#endif

	//loading dst file to memory
	FILE* fp = fopen(destfname,"r");
	if(fp == NULL){
		std::cout << "Could not open rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	if(fseek(fp,0,SEEK_END)){
		std::cout << "Seek error in rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	filesize = ftell(fp);
	if(filesize <= 0){
		std::cout << "Could not get length of rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	rewind(fp);
	fpgasrc = (char*)malloc(filesize+1);
	if(fpgasrc == NULL){
		std::cout << "Could not allocate size of program" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	if(fread(fpgasrc, sizeof(char), filesize, fp) != filesize){
		std::cout << "Could not read rawbin file" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}

	if(fclose(fp)){
		std::cout << "Could not close destination file after converting" << std::endl;
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	unlink(destfname); // delete temporary file
#ifdef VERBOSE
	std::cout << "Successfully loaded the rawbin file to program memory" << std::endl;
#endif


	// send program from memory to detector
#ifdef VERBOSE
	std::cout<< "Sending programming binary to detector " << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&filesize,sizeof(filesize));
			//check opening error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				filesize = 0;
			}


			//erasing flash
			if(ret!=FAIL){
				std::cout<< "This can take awhile. Please be patient..." << endl;
				printf("Erasing Flash:%d%%\r",0);
				std::cout << flush;
				//erasing takes 65 seconds, printing here (otherwise need threads
				//in server-unnecessary)
				int count = 66;
				while(count>0){
					usleep(1 * 1000 * 1000);
					--count;
					printf("Erasing Flash:%d%%\r",(int) (((double)(65-count)/65)*100));
					std::cout << flush;
				}
				std::cout<<std::endl;
				printf("Writing to Flash:%d%%\r",0);
				std::cout << flush;
			}


			//sending program in parts of 2mb each
			size_t unitprogramsize = 0;
			int currentPointer = 0;
			size_t totalsize= filesize;
			while(ret != FAIL && (filesize > 0)){

				unitprogramsize = MAX_FPGAPROGRAMSIZE;  //2mb
				if(unitprogramsize > filesize) //less than 2mb
					unitprogramsize = filesize;
#ifdef VERBOSE
				std::cout << "unitprogramsize:" << unitprogramsize << "\t filesize:"
						<< filesize << std::endl;
#endif
				controlSocket->SendDataOnly(fpgasrc+currentPointer,unitprogramsize);
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL) {
					filesize-=unitprogramsize;
					currentPointer+=unitprogramsize;

					//print progress
					printf("Writing to Flash:%d%%\r",
							(int) (((double)(totalsize-filesize)/totalsize)*100));
					std::cout << flush;
				}else{
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				}
			}
			std::cout<<std::endl;

			//check ending error
			if ((ret == FAIL) &&
					(strstr(mess,"not implemented") == NULL) &&
					(strstr(mess,"locked") == NULL) &&
					(strstr(mess,"-update") == NULL)) {
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				}
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}


		//remapping stop server
		if ((ret == FAIL) &&
				(strstr(mess,"not implemented") == NULL) &&
				(strstr(mess,"locked") == NULL) &&
				(strstr(mess,"-update") == NULL)) {
			fnum=F_RESET_FPGA;
			int stopret;
			if (connectStop() == OK){
				stopSocket->SendDataOnly(&fnum,sizeof(fnum));
				stopSocket->ReceiveDataOnly(&stopret,sizeof(stopret));
				if (stopret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				}
				disconnectControl();
			}
		}
	}
	if (ret != FAIL) {
		printf("You can now restart the detector servers in normal mode.\n");
	}

	//free resources
	if(fpgasrc != NULL)
		free(fpgasrc);

	return ret;
}


int slsDetector::resetFPGA() {
	int ret=FAIL;
	int fnum=F_RESET_FPGA;
	char mess[MAX_STR_LENGTH]="";

	if(thisDetector->myDetectorType != JUNGFRAU){
		std::cout << "Not implemented for this detector" << std::endl;
		return FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Sending reset to FPGA " << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		// control server
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(RESET_ERROR));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return ret;

}



int slsDetector::powerChip(int ival) {
	int ret=FAIL;
	int fnum=F_POWER_CHIP;
	char mess[MAX_STR_LENGTH]="";
	int retval=-1;

	if(thisDetector->myDetectorType != JUNGFRAU &&
			thisDetector->myDetectorType != JUNGFRAUCTB ){
		std::cout << "Not implemented for this detector" << std::endl;
		return FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Sending power on/off/get to the chip " << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&ival,sizeof(ival));
			//check opening error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(POWER_CHIP));
			}else
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return retval;

}


int slsDetector::setAutoComparatorDisableMode(int ival) {
	int ret=FAIL;
	int fnum=F_AUTO_COMP_DISABLE;
	char mess[MAX_STR_LENGTH]="";
	int retval=-1;

	if(thisDetector->myDetectorType != JUNGFRAU){
		std::cout << "Not implemented for this detector" << std::endl;
		return FAIL;
	}
#ifdef VERBOSE
	std::cout<< "Enabling/disabling Auto comp disable mode " << endl;
#endif
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&ival,sizeof(ival));
			//check opening error
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL) {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(AUTO_COMP_DISABLE));
			}else
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}
	return retval;

}



int slsDetector::getChanRegs(double* retval,bool fromDetector) {
	int n=getTotalNumberOfChannels();
	if(fromDetector){
		for(int im=0;im<setNumberOfModules();++im)
			getModule(im);
	}
	//the original array has 0 initialized
	if(chanregs){
		for (int i=0; i<n; ++i)
			retval[i] = (double) (chanregs[i] & TRIMBITMASK);
	}
	return n;
}


int slsDetector::setModule(int reg, int imod) {
	sls_detector_module myModule;

#ifdef VERBOSE
	std::cout << "slsDetector set module " << std::endl;
#endif
	int charegs[thisDetector->nChans*thisDetector->nChips];
	int chiregs[thisDetector->nChips];
	dacs_t das[thisDetector->nDacs], ads[thisDetector->nAdcs];
	int mmin=imod, mmax=imod+1;
	int ret=FAIL;

	if (imod==-1) {
		mmin=0;
		mmax=thisDetector->nModsMax;
	}



	for (int im=mmin; im<mmax; ++im) {

		myModule.module=im;
		myModule.nchan=thisDetector->nChans;
		myModule.nchip=thisDetector->nChips;
		myModule.ndac=thisDetector->nDacs;
		myModule.nadc=thisDetector->nAdcs;

		myModule.reg=reg;
		if (detectorModules) {
			myModule.gain=(detectorModules+im)->gain;
			myModule.offset=(detectorModules+im)->offset;
			myModule.serialnumber=(detectorModules+im)->serialnumber;
		} else {
			myModule.gain=-1;
			myModule.offset=-1;
			myModule.serialnumber=-1;
		}


		for (int i=0; i<thisDetector->nAdcs; ++i)
			ads[i]=-1;

		if (chanregs)
			myModule.chanregs=chanregs+im*thisDetector->nChips*thisDetector->nChans;
		else {
			for (int i=0; i<thisDetector->nChans*thisDetector->nChips; ++i)
				charegs[i]=-1;
			myModule.chanregs=charegs;
		}
		if (chipregs)
			myModule.chipregs=chanregs+im*thisDetector->nChips;
		else {
			for (int ichip=0; ichip<thisDetector->nChips; ++ichip)
				chiregs[ichip]=-1;
			myModule.chipregs=chiregs;
		}
		if (dacs)
			myModule.dacs=dacs+im*thisDetector->nDacs;
		else {
			for (int i=0; i<thisDetector->nDacs; ++i)
				das[i]=-1;
			myModule.dacs=das;
		}
		if (adcs)
			myModule.adcs=adcs+im*thisDetector->nAdcs;
		else {
			for (int i=0; i<thisDetector->nAdcs; ++i)
				ads[i]=-1;
			myModule.adcs=ads;
		}
		ret=setModule(myModule,-1,-1,-1,0,0);
	}
	return ret;


}

int slsDetector::setModule(sls_detector_module module, int iodelay, int tau,
		int e_eV, int* gainval, int* offsetval, int tb) {

	int fnum=F_SET_MODULE;
	int retval=-1;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	int imod=module.module;


#ifdef VERBOSE
	std::cout << "slsDetector set module " << std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			//to exclude trimbits
			if(!tb) {
				module.nchan=0;
				module.nchip=0;
			}
			sendModule(&module);

			//not included in module
			if(gainval && (thisDetector->nGain))
				controlSocket->SendDataOnly(gainval,sizeof(int)*thisDetector->nGain);
			if(offsetval && (thisDetector->nOffset))
				controlSocket->SendDataOnly(offsetval,sizeof(int)*thisDetector->nOffset);
			if(thisDetector->myDetectorType == EIGER) {
				controlSocket->SendDataOnly(&iodelay,sizeof(iodelay));
				controlSocket->SendDataOnly(&tau,sizeof(tau));
				controlSocket->SendDataOnly(&e_eV,sizeof(e_eV));
			}


			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				if(strstr(mess,"default tau")!=NULL)
					setErrorMask((getErrorMask())|(RATE_CORRECTION_NO_TAU_PROVIDED));
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if (ret!=FAIL) {
		if (detectorModules) {
			if (imod>=0 && imod<thisDetector->nMod[X]*thisDetector->nMod[Y]) {
				if(tb) {
					(detectorModules+imod)->nchan=module.nchan;
					(detectorModules+imod)->nchip=module.nchip;
				}
				(detectorModules+imod)->ndac=module.ndac;
				(detectorModules+imod)->nadc=module.nadc;
				if(tb) {
					thisDetector->nChips=module.nchip;
					thisDetector->nChans=module.nchan/module.nchip;
				}
				thisDetector->nDacs=module.ndac;
				thisDetector->nAdcs=module.nadc;

				if(thisDetector->myDetectorType != JUNGFRAU){
					if(tb) {
						for (int ichip=0; ichip<thisDetector->nChips; ++ichip) {
							if (chipregs)
								chipregs[ichip+thisDetector->nChips*imod]=
										module.chipregs[ichip];

							if (chanregs) {
								for (int i=0; i<thisDetector->nChans; ++i) {
									chanregs[i+ichip*
											 thisDetector->nChans+
											 thisDetector->nChips*thisDetector->nChans*
											 imod]=module.chanregs[ichip*thisDetector->nChans+i];
								}
							}
						}
					}
					if (adcs) {
						for (int i=0; i<thisDetector->nAdcs; ++i)
							adcs[i+imod*thisDetector->nAdcs]=module.adcs[i];
					}
				}

				if (dacs) {
					for (int i=0; i<thisDetector->nDacs; ++i)
						dacs[i+imod*thisDetector->nDacs]=module.dacs[i];
				}

				(detectorModules+imod)->gain=module.gain;
				(detectorModules+imod)->offset=module.offset;
				(detectorModules+imod)->serialnumber=module.serialnumber;
				(detectorModules+imod)->reg=module.reg;
			}
		}

		if ((thisDetector->nGain) && (gainval) && (gain)) {
			for (int i=0; i<thisDetector->nGain; ++i)
				gain[i+imod*thisDetector->nGain]=gainval[i];
		}

		if ((thisDetector->nOffset) && (offsetval) && (offset))  {
			for (int i=0; i<thisDetector->nOffset; ++i)
				offset[i+imod*thisDetector->nOffset]=offsetval[i];
		}

		if (e_eV != -1)
			thisDetector->currentThresholdEV = e_eV;

	}

#ifdef VERBOSE
	std::cout<< "Module register returned "<<  retval << std::endl;
#endif

	return retval;
}





slsDetectorDefs::sls_detector_module  *slsDetector::getModule(int imod) {

#ifdef VERBOSE
	std::cout << "slsDetector get module " << std::endl;
#endif

	int fnum=F_GET_MODULE;
	sls_detector_module *myMod=createModule();

	int* gainval=0, *offsetval=0;
	if(thisDetector->nGain)
		gainval=new int[thisDetector->nGain];
	if(thisDetector->nOffset)
		offsetval=new int[thisDetector->nOffset];

	//char *ptr,  *goff=(char*)thisDetector;

	// int chanreg[thisDetector->nChans*thisDetector->nChips];
	//int chipreg[thisDetector->nChips];
	//double dac[thisDetector->nDacs], adc[thisDetector->nAdcs];

	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
	// int n;

#ifdef VERBOSE
	std::cout<< "getting module " << imod << std::endl;
#endif

	myMod->module=imod;
	// myMod.nchan=thisDetector->nChans*thisDetector->nChips;
	//myMod.chanregs=chanreg;
	//myMod.nchip=thisDetector->nChips;
	//myMod.chipregs=chipreg;
	//myMod.ndac=thisDetector->nDacs;
	//myMod.dacs=dac;
	//myMod.ndac=thisDetector->nAdcs;
	//myMod.dacs=adc;


	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&imod,sizeof(imod));

			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				receiveModule(myMod);

				//extra gain and offset - eiger
				if(thisDetector->nGain)
					controlSocket->ReceiveDataOnly(gainval,sizeof(int)*thisDetector->nGain);
				if(thisDetector->nOffset)
					controlSocket->ReceiveDataOnly(offsetval,sizeof(int)*thisDetector->nOffset);
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if (ret!=FAIL) {
		if (detectorModules) {
			if (imod>=0 && imod<thisDetector->nMod[X]*thisDetector->nMod[Y]) {
				(detectorModules+imod)->nchan=myMod->nchan;
				(detectorModules+imod)->nchip=myMod->nchip;
				(detectorModules+imod)->ndac=myMod->ndac;
				(detectorModules+imod)->nadc=myMod->nadc;
				thisDetector->nChips=myMod->nchip;
				thisDetector->nChans=myMod->nchan/myMod->nchip;
				thisDetector->nDacs=myMod->ndac;
				thisDetector->nAdcs=myMod->nadc;

				if(thisDetector->myDetectorType != JUNGFRAU){
					for (int ichip=0; ichip<thisDetector->nChips; ++ichip) {
						if (chipregs)
							chipregs[ichip+thisDetector->nChips*imod]=
									myMod->chipregs[ichip];

						if (chanregs) {
							for (int i=0; i<thisDetector->nChans; ++i) {
								chanregs[i+ichip*thisDetector->nChans+
										 thisDetector->nChips*thisDetector->nChans*imod]=
												 myMod->chanregs[ichip*thisDetector->nChans+i];
							}
						}
					}

					if (adcs) {
						for (int i=0; i<thisDetector->nAdcs; ++i)
							adcs[i+imod*thisDetector->nAdcs]=myMod->adcs[i];
					}
				}

				if (dacs) {
					for (int i=0; i<thisDetector->nDacs; ++i) {
						dacs[i+imod*thisDetector->nDacs]=myMod->dacs[i];
						//cprintf(BLUE,"dac%d:%d\n",i, myMod->dacs[i]);
					}
				}
				(detectorModules+imod)->gain=myMod->gain;
				(detectorModules+imod)->offset=myMod->offset;
				(detectorModules+imod)->serialnumber=myMod->serialnumber;
				(detectorModules+imod)->reg=myMod->reg;
			}
		}

		if ((thisDetector->nGain) && (gainval) && (gain)) {
			for (int i=0; i<thisDetector->nGain; ++i)
				gain[i+imod*thisDetector->nGain]=gainval[i];
		}

		if ((thisDetector->nOffset) && (offsetval) && (offset))  {
			for (int i=0; i<thisDetector->nOffset; ++i)
				offset[i+imod*thisDetector->nOffset]=offsetval[i];
		}

	} else {
		deleteModule(myMod);
		myMod=NULL;
	}

	if(gainval) delete[]gainval;
	if(offsetval) delete[]offsetval;

	return myMod;
}




int slsDetector::setChannel(int64_t reg, int ichan, int ichip, int imod) {
	sls_detector_channel myChan;
#ifdef VERBOSE
	std::cout<< "Setting channel "<< ichan << " " << ichip << " " << imod <<
			" to " << reg << std::endl;
#endif
	//int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1, chamin=ichan,
	//chamax=ichan+1;

	int ret;

	/*  if (imod==-1) {
      mmin=0;
      mmax=thisDetector->nModsMax;
      }

      if (ichip==-1) {
      chimin=0;
      chimax=thisDetector->nChips;
      }

      if (ichan==-1) {
      chamin=0;
      chamax=thisDetector->nChans;
      }*/

	// for (int im=mmin; im<mmax; ++im) {
	//  for (int ichi=chimin; ichi<chimax; ++ichi) {
	//    for (int icha=chamin; icha<chamax; ++icha) {
	myChan.chan=ichan;//icha;
	myChan.chip=ichip;//ichi;
	myChan.module=imod;//im;
	myChan.reg=reg;
	ret=setChannel(myChan);
	//     }
	// }
	// }
	return ret;
}



int slsDetector::setChannel(sls_detector_channel chan) {
	int fnum=F_SET_CHANNEL;
	int retval;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	int ichan=chan.chan;
	int ichip=chan.chip;
	int imod=chan.module;

	cout << "Set chan " << ichan << " chip " <<ichip << " mod " << imod << " reg "
			<< hex << chan.reg << endl;

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			sendChannel(&chan);

			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if (ret!=FAIL) {
		if (chanregs) {

			int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1, chamin=ichan,
					chamax=ichan+1;

			if (imod==-1) {
				mmin=0;
				mmax=thisDetector->nModsMax;
			}

			if (ichip==-1) {
				chimin=0;
				chimax=thisDetector->nChips;
			}

			if (ichan==-1) {
				chamin=0;
				chamax=thisDetector->nChans;
			}






			for (int im=mmin; im<mmax; ++im) {
				for (int ichi=chimin; ichi<chimax; ++ichi) {
					for (int icha=chamin; icha<chamax; ++icha) {

						*(chanregs+im*thisDetector->nChans*
								thisDetector->nChips+ichi*thisDetector->nChips+icha)=retval;

					}
				}
			}

		}
	}
#ifdef VERBOSE
	std::cout<< "Channel register returned "<<  retval << std::endl;
#endif
	return retval;

}





slsDetectorDefs::sls_detector_channel  slsDetector::getChannel(int ichan, int ichip,
		int imod) {


	int fnum=F_GET_CHANNEL;
	sls_detector_channel myChan;
	int arg[3];
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";
	arg[0]=ichan;
	arg[1]=ichip;
	arg[2]=imod;
	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));

			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				receiveChannel(&myChan);
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if (ret!=FAIL) {
		if (chanregs) {
			*(chanregs+imod*thisDetector->nChans*thisDetector->nChips+ichip*
					thisDetector->nChips+ichan)=myChan.reg;
		}
	}

#ifdef VERBOSE
	std::cout<< "Returned channel "<< ichan << " " << ichip << " " << imod << " "
			<<  myChan.reg << std::endl;
#endif
	return myChan;
}



int slsDetector::setChip(int reg, int ichip, int imod) {
	sls_detector_chip myChip;

#ifdef VERBOSE
	std::cout<< "Setting chip "<<  ichip << " " << imod << " to " << reg <<  std::endl;
#endif


	int chregs[thisDetector->nChans];
	int mmin=imod, mmax=imod+1, chimin=ichip, chimax=ichip+1;
	int ret=FAIL;
	if (imod==-1) {
		mmin=0;
		mmax=thisDetector->nModsMax;
	}

	if (ichip==-1) {
		chimin=0;
		chimax=thisDetector->nChips;
	}

	myChip.nchan=thisDetector->nChans;
	myChip.reg=reg;
	for (int im=mmin; im<mmax; ++im) {
		for (int ichi=chimin; ichi<chimax; ++ichi) {
			myChip.chip=ichi;
			myChip.module=im;
			if (chanregs)
				myChip.chanregs=(chanregs+ichi*thisDetector->nChans+im*
						thisDetector->nChans*thisDetector->nChips);
			else {
				for (int i=0; i<thisDetector->nChans; ++i)
					chregs[i]=-1;
				myChip.chanregs=chregs;
			}
			ret=setChip(myChip);
		}
	}
	return ret;
}

int slsDetector::setChip(sls_detector_chip chip) {

	int fnum=F_SET_CHIP;
	int retval = FAIL;
	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";

	int ichi=chip.chip;
	int im=chip.module;

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			sendChip(&chip);
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if (ret!=FAIL) {
		if (chipregs)
			*(chipregs+ichi+im*thisDetector->nChips)=retval;
	}

#ifdef VERBOSE
	std::cout<< "Chip register returned "<<  retval << std::endl;
#endif
	return retval;
}


slsDetectorDefs::sls_detector_chip slsDetector::getChip(int ichip, int imod) {

	int fnum=F_GET_CHIP;
	sls_detector_chip myChip;
	int chanreg[thisDetector->nChans];

	int ret=FAIL;
	char mess[MAX_STR_LENGTH]="";


	myChip.chip=ichip;
	myChip.module=imod;
	myChip.nchan=thisDetector->nChans;
	myChip.chanregs=chanreg;

	int arg[2];
	arg[0]=ichip;
	arg[1]=imod;




	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(arg,sizeof(arg));

			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				receiveChip(&myChip);
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if (ret!=FAIL) {
		if (chipregs)
			*(chipregs+ichip+imod*thisDetector->nChips)=myChip.reg;
		if (chanregs) {
			for (int ichan=0; ichan<thisDetector->nChans; ++ichan)
				*(chanregs+imod*thisDetector->nChans*thisDetector->nChips+ichip*
						thisDetector->nChans+ichan)=*((myChip.chanregs)+ichan);
		}
	}
#ifdef VERBOSE
	std::cout<< "Returned chip "<<  ichip << " " << imod << " " <<  myChip.reg << std::endl;
#endif

	return myChip;
}



int slsDetector::getMoveFlag(int imod) {
	if (moveFlag)
		return *moveFlag;
	else return 1;
}


int slsDetector::fillModuleMask(int *mM) {
	if (mM)
		for (int i=0; i<getNMods(); ++i)
			mM[i]=i;

	return getNMods();
}

int slsDetector::calibratePedestal(int frames) {
	int ret=FAIL;
	int retval=-1;
	int fnum=F_CALIBRATE_PEDESTAL;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<<"Calibrating Pedestal " <<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&frames,sizeof(frames));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return retval;
}



int slsDetector::setRateCorrection(double t) {

	if (getDetectorsType() == EIGER){
		int fnum=F_SET_RATE_CORRECT;
		int ret=FAIL;
		char mess[MAX_STR_LENGTH]="";
		int64_t arg = t;

#ifdef VERBOSE
		std::cout<< "Setting Rate Correction to " << arg << endl;
#endif
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->SendDataOnly(&arg,sizeof(arg));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret==FAIL) {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
					if(strstr(mess,"default tau")!=NULL)
						setErrorMask((getErrorMask())|(RATE_CORRECTION_NO_TAU_PROVIDED));
					if(strstr(mess,"32")!=NULL)
						setErrorMask((getErrorMask())|(RATE_CORRECTION_NOT_32or16BIT));
					else
						setErrorMask((getErrorMask())|(COULD_NOT_SET_RATE_CORRECTION));
				}
				disconnectControl();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}
		return ret;  //only success/fail
	}


	//mythen
	double tdead[]=defaultTDead;
	if (t==0) {
#ifdef VERBOSE
		std::cout<< "unsetting rate correction" << std::endl;
#endif
		thisDetector->correctionMask&=~(1<<RATE_CORRECTION);
	} else {
		thisDetector->correctionMask|=(1<<RATE_CORRECTION);
		if (t>0)
			thisDetector->tDead=t;
		else {
			if (thisDetector->currentSettings<3 && thisDetector->currentSettings>-1)
				thisDetector->tDead=tdead[thisDetector->currentSettings];
			else
				thisDetector->tDead=0;
		}
#ifdef VERBOSE
		std::cout<< "Setting rate correction with dead time "<< thisDetector->tDead
				<< std::endl;
#endif
	}
	return thisDetector->correctionMask&(1<<RATE_CORRECTION);


}


int slsDetector::getRateCorrection(double &t) {

	if (thisDetector->myDetectorType == EIGER){
		t = getRateCorrectionTau();
		return t;
	}

	if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
		std::cout<< "Rate correction is enabled with dead time "<<
				thisDetector->tDead << std::endl;
#endif
		t=thisDetector->tDead;
		return 1;
	} else
		t=0;
#ifdef VERBOSE
	std::cout<< "Rate correction is disabled " << std::endl;
#endif
	return 0;
};

double slsDetector::getRateCorrectionTau() {

	if(thisDetector->myDetectorType == EIGER){
		int fnum=F_GET_RATE_CORRECT;
		int ret=FAIL;
		char mess[MAX_STR_LENGTH]="";
		int64_t retval = -1;
#ifdef VERBOSE
		std::cout<< "Setting Rate Correction to " << arg << endl;
#endif
		if (thisDetector->onlineFlag==ONLINE_FLAG) {
			if (connectControl() == OK){
				controlSocket->SendDataOnly(&fnum,sizeof(fnum));
				controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
				if (ret!=FAIL)
					controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				else {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					std::cout<< "Detector returned error: " << mess << std::endl;
				}
				disconnectControl();
				if (ret==FORCE_UPDATE)
					updateDetector();
			}
		}

		return double(retval);
	}


	//mythen only
	if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
		std::cout<< "Rate correction is enabled with dead time "<<
				thisDetector->tDead << std::endl;
#endif
		return thisDetector->tDead;
		//return 1;
	} else
#ifdef VERBOSE
		std::cout<< "Rate correction is disabled " << std::endl;
#endif
	return 0;
}


int slsDetector::getRateCorrection() {

	if (thisDetector->myDetectorType == EIGER){
		double t = getRateCorrectionTau();
		return (int)t;
	}

	if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
		return 1;
	} else
		return 0;
}

int slsDetector::rateCorrect(double* datain, double *errin, double* dataout,
		double *errout) {
	double tau=thisDetector->tDead;
	double t=thisDetector->timerValue[ACQUISITION_TIME];
	// double data;
	double e;
	if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
		std::cout<< "Rate correcting data with dead time "<< tau << " and "
				"acquisition time "<< t << std::endl;
#endif
		for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*
		thisDetector->nChans*thisDetector->nChips; ++ichan) {

			if (errin==NULL) {
				e=sqrt(datain[ichan]);
			} else
				e=errin[ichan];

			postProcessingFuncs::rateCorrect(datain[ichan], e, dataout[ichan],
					errout[ichan], tau, t);
		}
	}

	return 0;
}



int slsDetector::setFlatFieldCorrection(string fname) {
	double data[thisDetector->nModMax[X]*thisDetector->nModMax[Y]*
				thisDetector->nChans*thisDetector->nChips];
	int im=0;
	int nch;
	thisDetector->nBadFF=0;

	char ffffname[MAX_STR_LENGTH*2];
	if (fname=="default") {
		fname=string(thisDetector->flatFieldFile);
	}

	if (fname=="") {
#ifdef VERBOSE
		std::cout<< "disabling flat field correction" << std::endl;
#endif
		thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);



	} else {
#ifdef VERBOSE
		std::cout<< "Setting flat field correction from file " << fname << std::endl;
#endif


		sprintf(ffffname,"%s/%s",thisDetector->flatFieldDir,fname.c_str());
		nch=readDataFile(string(ffffname),data);
		if (nch>0) {

			//???? bad ff chans?
			int nm=getNMods();
			int chpm[nm];
			int mMask[nm];
			for (int i=0; i<nm; ++i) {
				chpm[im]=getChansPerMod(im);
				mMask[im]=im;
			}
			fillModuleMask(mMask);

			if ((postProcessingFuncs::calculateFlatField(&nm, chpm, mMask,
					badChannelMask, data, ffcoefficients, fferrors))>=0) {
				strcpy(thisDetector->flatFieldFile,fname.c_str());


				thisDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);

				setFlatFieldCorrection(ffcoefficients, fferrors);

			}
		}  else {
			std::cout<< "Flat field from file " << fname << " is not valid " <<
					nch << std::endl;

		}
	}

	return thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);

}




int slsDetector::setFlatFieldCorrection(double *corr, double *ecorr) {
	if (corr!=NULL) {
		for (int ichan=0; ichan<thisDetector->nMod[Y]*thisDetector->nMod[X]*
		thisDetector->nChans*thisDetector->nChips; ++ichan) {
			// #ifdef VERBOSE
			//       std::cout<< ichan << " "<< corr[ichan] << std::endl;
			// #endif
			ffcoefficients[ichan]=corr[ichan];
			if (ecorr!=NULL)
				fferrors[ichan]=ecorr[ichan];
			else
				fferrors[ichan]=1;

			cout << ichan << " " <<  ffcoefficients[ichan] << endl;
		}
		thisDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);
	} else
		thisDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
#ifdef VERBOSE
	cout << "set ff corrections " << ((thisDetector->correctionMask)&
			(1<<FLAT_FIELD_CORRECTION)) << endl;
#endif


	return thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}





int slsDetector::getFlatFieldCorrection(double *corr, double *ecorr) {
	if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
#ifdef VERBOSE
		std::cout<< "Flat field correction is enabled" << std::endl;
#endif
		if (corr) {
			for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*
			thisDetector->nChans*thisDetector->nChips; ++ichan) {
				corr[ichan]=ffcoefficients[ichan];
				if (ecorr) {
					//ecorr[ichan]=ffcoefficients[ichan]/fferrors[ichan];
					ecorr[ichan]=fferrors[ichan];
				}
			}
		}
		return 1;
	} else {
#ifdef VERBOSE
		std::cout<< "Flat field correction is disabled" << std::endl;
#endif
		if (corr)
			for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nMod[Y]*
			thisDetector->nChans*thisDetector->nChips; ++ichan) {
				corr[ichan]=1;
				if (ecorr)
					ecorr[ichan]=0;
			}
		return 0;
	}

}



int slsDetector::flatFieldCorrect(double* datain, double *errin, double* dataout,
		double *errout) {
#ifdef VERBOSE
	std::cout<< "Flat field correcting data" << std::endl;
#endif
	double e, eo;
	if (thisDetector->correctionMask & (1<<FLAT_FIELD_CORRECTION)) {
		for (int ichan=0; ichan<thisDetector->nMod[X]*thisDetector->nChans*
		thisDetector->nChips; ++ichan) {
			if (errin==NULL) {
				e=0;
			} else {
				e=errin[ichan];
			}
			postProcessingFuncs::flatFieldCorrect(datain[ichan],e,dataout[ichan],
					eo,ffcoefficients[ichan],fferrors[ichan]);
			if (errout)
				errout[ichan]=eo;
		}
	}
	return 0;

}




int slsDetector::setBadChannelCorrection(string fname) {

	// int nbadmod;
	int ret=0;
	//int badchanlist[MAX_BADCHANS];
	//int off;

	string fn=fname;

	if (fname=="default")
		fname=string(badChanFile);

	if (nBadChans && badChansList)
		ret=setBadChannelCorrection(fname, *nBadChans, badChansList);

	if (ret) {
		*correctionMask|=(1<<DISCARD_BAD_CHANNELS);
		strcpy(badChanFile,fname.c_str());
	} else
		*correctionMask&=~(1<<DISCARD_BAD_CHANNELS);

	fillBadChannelMask();
	return  (*correctionMask)&(1<<DISCARD_BAD_CHANNELS);
}







int slsDetector::setBadChannelCorrection(int nch, int *chs, int ff) {
#ifdef VERBOSE
	cout << "setting " << nch << " bad chans " << endl;
#endif
	if (ff==0) {
		if (nch<MAX_BADCHANS && nch>0) {
			thisDetector->correctionMask|=(1<<DISCARD_BAD_CHANNELS);
			thisDetector->nBadChans=0;
			for (int ich=0 ;ich<nch; ++ich) {
				if (chs[ich]>=0 && chs[ich]<getMaxNumberOfChannels()) {
					thisDetector->badChansList[ich]=chs[ich];
					++thisDetector->nBadChans;
					}
			}
		} else
			thisDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
	} else {
		if (nch<MAX_BADCHANS && nch>0) {
			thisDetector->nBadFF=nch;
			for (int ich=0 ;ich<nch; ++ich) {
				thisDetector->badFFList[ich]=chs[ich];
			}
		}
	}
#ifdef VERBOSE
	cout << "badchans flag is "<< (thisDetector->correctionMask&
			(1<< DISCARD_BAD_CHANNELS)) << endl;
#endif
	// fillBadChannelMask();
	if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
		return thisDetector->nBadChans+thisDetector->nBadFF;
	} else
		return 0;

}


int slsDetector::getBadChannelCorrection(int *bad) {
	int ichan;
	if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
		if (bad) {
			for (ichan=0; ichan<thisDetector->nBadChans; ++ichan)
				bad[ichan]=thisDetector->badChansList[ichan];
			for (int ich=0; ich<thisDetector->nBadFF; ++ich)
				bad[ichan+ich]=thisDetector->badFFList[ich];
		}
		return thisDetector->nBadChans+thisDetector->nBadFF;
	} else
		return 0;
}


int slsDetector::readAngularConversionFile(string fname) {

	return readAngularConversion(fname,thisDetector->nModsMax,  thisDetector->angOff);

}

int slsDetector::readAngularConversion(ifstream& ifs) {

	return readAngularConversion(ifs,thisDetector->nModsMax,  thisDetector->angOff);

}


int slsDetector:: writeAngularConversion(string fname) {

	return writeAngularConversion(fname, thisDetector->nMods, thisDetector->angOff);

}


int slsDetector:: writeAngularConversion(ofstream &ofs) {

	return writeAngularConversion(ofs, thisDetector->nMods, thisDetector->angOff);

}

int slsDetector::getAngularConversion(int &direction,  angleConversionConstant *angconv) {
	direction=thisDetector->angDirection;
	if (angconv) {
		for (int imod=0; imod<thisDetector->nMods; ++imod) {
			(angconv+imod)->center=thisDetector->angOff[imod].center;
			(angconv+imod)->r_conversion=thisDetector->angOff[imod].r_conversion;
			(angconv+imod)->offset=thisDetector->angOff[imod].offset;
			(angconv+imod)->ecenter=thisDetector->angOff[imod].ecenter;
			(angconv+imod)->er_conversion=thisDetector->angOff[imod].er_conversion;
			(angconv+imod)->eoffset=thisDetector->angOff[imod].eoffset;
		}
	}
	if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) {
		return 1;
	} else {
		return 0;
	}
}



angleConversionConstant* slsDetector::getAngularConversionPointer(int imod) {
	return &thisDetector->angOff[imod];
}


int slsDetector::printReceiverConfiguration() {

	std::cout << "Detector IP:\t\t" << getNetworkParameter(DETECTOR_IP) << std::endl;
	std::cout << "Detector MAC:\t\t" << getNetworkParameter(DETECTOR_MAC) << std::endl;

	std::cout << "Receiver Hostname:\t" << getNetworkParameter(RECEIVER_HOSTNAME) << std::endl;
	std::cout << "Receiver UDP IP:\t" << getNetworkParameter(RECEIVER_UDP_IP) << std::endl;
	std::cout << "Receiver UDP MAC:\t" << getNetworkParameter(RECEIVER_UDP_MAC) << std::endl;
	std::cout << "Receiver UDP Port:\t" << getNetworkParameter(RECEIVER_UDP_PORT) << std::endl;

	if(thisDetector->myDetectorType == EIGER)
		std::cout << "Receiver UDP Port2:\t" <<  getNetworkParameter(RECEIVER_UDP_PORT2)
		<< std::endl;
	std::cout << std::endl;

	return OK;
}





int slsDetector::setReceiverOnline(int off) {
	if (off!=GET_ONLINE_FLAG) {
		// no receiver
		if(!strcmp(thisDetector->receiver_hostname,"none"))
			thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
		else
			thisDetector->receiverOnlineFlag = off;
		// check receiver online
		if (thisDetector->receiverOnlineFlag==ONLINE_FLAG){
			setReceiverTCPSocket();
			// error in connecting
			if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
				std::cout << "cannot connect to receiver" << endl;
				setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
			}
		}

	}
	return thisDetector->receiverOnlineFlag;
}



string slsDetector::checkReceiverOnline() {
	string retval = "";
	//if it doesnt exits, create data socket
	if(!dataSocket){
		//this already sets the online/offline flag
		setReceiverTCPSocket();
		if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG)
			return string(thisDetector->receiver_hostname);
		else
			return string("");
	}
	//still cannot connect to socket, dataSocket=0
	if(dataSocket){
		if (connectData() == FAIL) {
			dataSocket->SetTimeOut(5);
			thisDetector->receiverOnlineFlag=OFFLINE_FLAG;
			delete dataSocket;
			dataSocket=0;
#ifdef VERBOSE
			std::cout<< "receiver offline!" << std::endl;
#endif
			return string(thisDetector->receiver_hostname);
		}  else {
			thisDetector->receiverOnlineFlag=ONLINE_FLAG;
			dataSocket->SetTimeOut(100);
			disconnectData();
#ifdef VERBOSE
			std::cout<< "receiver online!" << std::endl;
#endif
			return string("");
		}
	}
	return retval;
}





int slsDetector::setReceiverTCPSocket(string const name, int const receiver_port) {

	char thisName[MAX_STR_LENGTH];
	int thisRP;
	int retval=OK;

	//if receiver ip given
	if (strcmp(name.c_str(),"")!=0) {
#ifdef VERBOSE
		std::cout<< "setting receiver" << std::endl;
#endif
		strcpy(thisName,name.c_str());
		strcpy(thisDetector->receiver_hostname,thisName);
		if (dataSocket){
			delete dataSocket;
			dataSocket=0;
		}
	} else
		strcpy(thisName,thisDetector->receiver_hostname);

	//if receiverTCPPort given
	if (receiver_port>0) {
#ifdef VERBOSE
		std::cout<< "setting data port" << std::endl;
#endif
		thisRP=receiver_port;
		thisDetector->receiverTCPPort=thisRP;
		if (dataSocket){
			delete dataSocket;
			dataSocket=0;
		}
	} else
		thisRP=thisDetector->receiverTCPPort;

	//create data socket
	if (!dataSocket) {
		try {
			dataSocket = new MySocketTCP(thisName, thisRP);
#ifdef VERYVERBOSE
			std::cout<< "Data socket connected " <<
					thisName  << " " << thisRP << std::endl;
#endif
		} catch(...) {
#ifdef VERBOSE
			std::cout<< "Could not connect Data socket " <<
					thisName  << " " << thisRP << std::endl;
#endif
			dataSocket = 0;
			retval = FAIL;
		}
	}


	//check if it connects
	if (retval!=FAIL) {
		checkReceiverOnline();
		thisReceiver->setSocket(dataSocket);
		// check for version compatibility
		switch (thisDetector->myDetectorType) {
		case EIGER:
		case JUNGFRAU:
		case GOTTHARD:
			if (thisDetector->receiverAPIVersion == 0){
				if (checkVersionCompatibility(DATA_PORT) == FAIL)
					thisDetector->receiverOnlineFlag=OFFLINE_FLAG;
			}
			break;
		default:
			break;
		}

	} else {
		thisDetector->receiverOnlineFlag=OFFLINE_FLAG;
#ifdef VERBOSE
		std::cout<< "offline!" << std::endl;
#endif
	}
	return retval;
}



int slsDetector::lockReceiver(int lock) {
	int fnum=F_LOCK_RECEIVER;
	int ret = FAIL;
	int retval=-1;
	int arg=lock;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Locking or Unlocking Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return retval;
}






string slsDetector::getReceiverLastClientIP() {
	int fnum=F_GET_LAST_RECEIVER_CLIENT_IP;
	int ret = FAIL;
	char retval[INET_ADDRSTRLEN]="";

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Geting Last Client IP connected to Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getLastClientIP(fnum,retval);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return string(retval);
}


int slsDetector::exitReceiver() {

	int retval = FAIL;
	int fnum=F_EXIT_RECEIVER;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
		if (dataSocket) {
			dataSocket->Connect();
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectData();
		}
	}
	if (retval!=OK) {
		std::cout<< std::endl;
		std::cout<< "Shutting down the receiver" << std::endl;
		std::cout<< std::endl;
	}
	return retval;

}


int slsDetector::updateReceiverNoWait() {

	int n = 0,ind;
	char path[MAX_STR_LENGTH];
	char lastClientIP[INET_ADDRSTRLEN];

	n += 	dataSocket->ReceiveDataOnly(lastClientIP,sizeof(lastClientIP));
#ifdef VERBOSE
	cout << "Updating receiver last modified by " << lastClientIP << std::endl;
#endif

	// filepath
	n += 	dataSocket->ReceiveDataOnly(path,MAX_STR_LENGTH);
	pthread_mutex_lock(&ms);
	fileIO::setFilePath(path);
	pthread_mutex_unlock(&ms);

	// filename
	n += 	dataSocket->ReceiveDataOnly(path,MAX_STR_LENGTH);
	pthread_mutex_lock(&ms);
	fileIO::setFileName(path);
	pthread_mutex_unlock(&ms);

	// index
	n += 	dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	pthread_mutex_lock(&ms);
	fileIO::setFileIndex(ind);
	pthread_mutex_unlock(&ms);

	//file format
	n += 	dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	pthread_mutex_lock(&ms);
	fileIO::setFileFormat(ind);
	pthread_mutex_unlock(&ms);

	// frames per file
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_framesPerFile = ind;

	// frame discard policy
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_frameDiscardMode = (frameDiscardPolicy)ind;

	// frame padding
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_framePadding = ind;

	// file write enable
	n += 	dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	pthread_mutex_lock(&ms);
	multiDet->enableWriteToFileMask(ind);
	pthread_mutex_unlock(&ms);

	// file overwrite enable
	n += 	dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	pthread_mutex_lock(&ms);
	multiDet->enableOverwriteMask(ind);
	pthread_mutex_unlock(&ms);

	// gap pixels
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->gappixels = ind;

	// receiver read frequency
	n += 	dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_read_freq = ind;

	// receiver streaming port
	n += 	dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_zmqport = ind;

	// streaming source ip
	n += 	dataSocket->ReceiveDataOnly(path,MAX_STR_LENGTH);
	strcpy(thisDetector->receiver_zmqip, path);

	// additional json header
	n +=  dataSocket->ReceiveDataOnly(path,MAX_STR_LENGTH);
	strcpy(thisDetector->receiver_additionalJsonHeader, path);

	// receiver streaming enable
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_upstream = ind;

	// activate
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->activated = ind;

	// deactivated padding enable
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_deactivatedPaddingEnable = ind;

	// silent mode
	n += dataSocket->ReceiveDataOnly(&ind,sizeof(ind));
	thisDetector->receiver_silentMode = ind;

	if (!n) printf("n: %d\n", n);

	return OK;

}





int slsDetector::updateReceiver() {
	int fnum=F_UPDATE_RECEIVER_CLIENT;
	int ret=OK;
	char mess[MAX_STR_LENGTH]="";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
		if (connectData() == OK){
			dataSocket->SendDataOnly(&fnum,sizeof(fnum));
			dataSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret == FAIL) {
				dataSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Receiver returned error: " << mess << std::endl;
			}
			else
				updateReceiverNoWait();

			//if ret is force update, do not update now as client is updating
			//receiver currently
			disconnectData();
		}
	}

	return ret;
}



void slsDetector::sendMultiDetectorSize() {
	int fnum=F_SEND_RECEIVER_MULTIDETSIZE;
	int ret = FAIL;
	int retval = -1;
	int arg[2];

	pthread_mutex_lock(&ms);
	multiDet->getNumberOfDetectors(arg[0],arg[1]);
	pthread_mutex_unlock(&ms);

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending multi detector size to Receiver (" << arg[0] << ","
				<< arg[1] << ")" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendIntArray(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL){
			std::cout << "Could not set position Id" << std::endl;
			setErrorMask((getErrorMask())|(RECEIVER_MULTI_DET_SIZE_NOT_SET));
		}
	}
}


void slsDetector::setDetectorId() {
	int fnum=F_SEND_RECEIVER_DETPOSID;
	int ret = FAIL;
	int retval = -1;
	int arg = detId;

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending detector pos id to Receiver " << detId << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if((ret==FAIL) || (retval != arg)){
			std::cout << "Could not set position Id" << std::endl;
			setErrorMask((getErrorMask())|(RECEIVER_DET_POSID_NOT_SET));
		}
	}
}


void slsDetector::setDetectorHostname() {
	int fnum=F_SEND_RECEIVER_DETHOSTNAME;
	int ret = FAIL;
	char retval[MAX_STR_LENGTH]="";


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending detector hostname to Receiver " <<
				thisDetector->hostname << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,thisDetector->hostname);
			disconnectData();
		}
		if((ret==FAIL) || (strcmp(retval,thisDetector->hostname)))
			setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTNAME_NOT_SET));
	}
}


string slsDetector::getFilePath() {
	return setFilePath();
}





string slsDetector::setFilePath(string s) {
	int fnum = F_SET_RECEIVER_FILE_PATH;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH]="";
	char retval[MAX_STR_LENGTH] = "";
	struct stat st;

	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(!s.empty()){
			if(stat(s.c_str(),&st)){
				std::cout << "path does not exist" << endl;
				setErrorMask((getErrorMask())|(FILE_PATH_DOES_NOT_EXIST));
			}else{
				pthread_mutex_lock(&ms);
				fileIO::setFilePath(s);
				pthread_mutex_unlock(&ms);
			}
		}
	}

	else{
		strcpy(arg,s.c_str());
#ifdef VERBOSE
		std::cout << "Sending file path to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			fileIO::setFilePath(string(retval));
			pthread_mutex_unlock(&ms);
		}
		else if(!s.empty()){
			std::cout << "file path does not exist" << endl;
			setErrorMask((getErrorMask())|(FILE_PATH_DOES_NOT_EXIST));
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	pthread_mutex_lock(&ms);
	s = fileIO::getFilePath();
	pthread_mutex_unlock(&ms);

	return s;
}


string slsDetector::getFileName() {
	return setFileName();
}

string slsDetector::setFileName(string s) {
	int fnum=F_SET_RECEIVER_FILE_NAME;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH]="";
	char retval[MAX_STR_LENGTH]="";
	string sretval="";

	/*if(!s.empty()){
		pthread_mutex_lock(&ms);
		fileIO::setFileName(s);
		s=multiDet->createReceiverFilePrefix();
		pthread_mutex_unlock(&ms);
	}*/

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
		strcpy(arg,s.c_str());
#ifdef VERBOSE
		std::cout << "Sending file name to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendString(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
#ifdef VERBOSE
			std::cout << "Complete file prefix from receiver: " << retval << std::endl;
#endif
			/*
			pthread_mutex_lock(&ms);
			fileIO::setFileName(multiDet->getNameFromReceiverFilePrefix(string(retval)));
			pthread_mutex_unlock(&ms);
			 */
			sretval = fileIO::getNameFromReceiverFilePrefix(string(retval));
		}

		if(ret==FORCE_UPDATE)
			updateReceiver();
		return sretval;

	} else {
		if(!s.empty()){
			pthread_mutex_lock(&ms);
			fileIO::setFileName(s);
			pthread_mutex_unlock(&ms);
		}
		pthread_mutex_lock(&ms);
		s = fileIO::getFileName();
		pthread_mutex_unlock(&ms);

		return s;

	}

}


int slsDetector::setReceiverFramesPerFile(int f) {
	int fnum = F_SET_RECEIVER_FRAMES_PER_FILE;
	int ret = FAIL;
	int retval = -1;
	int arg = f;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending frames per file to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
		else if(ret!=FAIL && retval > -1){
			thisDetector->receiver_framesPerFile = retval;
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return thisDetector->receiver_framesPerFile;
}


slsReceiverDefs::frameDiscardPolicy slsDetector::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
	int fnum = F_RECEIVER_DISCARD_POLICY;
	int ret = FAIL;
	int retval = -1;
	int arg = f;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending frames discard policy to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
		else if(ret!=FAIL && retval > -1){
			thisDetector->receiver_frameDiscardMode = (frameDiscardPolicy)retval;
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return thisDetector->receiver_frameDiscardMode;
}

int slsDetector::setReceiverPartialFramesPadding(int f) {
	int fnum = F_RECEIVER_PADDING_ENABLE;
	int ret = FAIL;
	int retval = -1;
	int arg = f;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending partial frames enable to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
		else if(ret!=FAIL && retval > -1){
			thisDetector->receiver_framePadding = (bool)retval;
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return thisDetector->receiver_framePadding;
}

slsReceiverDefs::fileFormat slsDetector::setFileFormat(fileFormat f) {
	int fnum=F_SET_RECEIVER_FILE_FORMAT;
	int ret = FAIL;
	int arg = -1;
	int retval = -1;



	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(f>=0){
			pthread_mutex_lock(&ms);
			fileIO::setFileFormat(f);
			pthread_mutex_unlock(&ms);
		}
	}

	else{
		arg = (int)f;
#ifdef VERBOSE
		std::cout << "Sending file format to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret == FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_FILE_FORMAT));
		else{
			pthread_mutex_lock(&ms);
			fileIO::setFileFormat(retval);
			pthread_mutex_unlock(&ms);
			if(ret==FORCE_UPDATE)
				updateReceiver();
		}
	}

	return fileIO::getFileFormat();
}



slsReceiverDefs::fileFormat slsDetector::getFileFormat() {
	return setFileFormat();
}

int slsDetector::getFileIndex() {
	return setFileIndex();
}

int slsDetector::setFileIndex(int i) {
	int fnum=F_SET_RECEIVER_FILE_INDEX;
	int ret = FAIL;
	int retval=-1;
	int arg = i;


	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(i>=0){
			pthread_mutex_lock(&ms);
			fileIO::setFileIndex(i);
			pthread_mutex_unlock(&ms);
		}
	}

	else{
#ifdef VERBOSE
		std::cout << "Sending file index to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			fileIO::setFileIndex(retval);
			pthread_mutex_unlock(&ms);
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	return fileIO::getFileIndex();
}





int slsDetector::startReceiver() {
	int fnum=F_START_RECEIVER;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Starting Receiver " << std::endl;
#endif

		if (connectData() == OK){
			ret=thisReceiver->executeFunction(fnum,mess);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
		else if (ret == FAIL){
			if(strstr(mess,"UDP")!=NULL)
				setErrorMask((getErrorMask())|(COULDNOT_CREATE_UDP_SOCKET));
			else if(strstr(mess,"file")!=NULL)
				setErrorMask((getErrorMask())|(COULDNOT_CREATE_FILE));
			else
				setErrorMask((getErrorMask())|(COULDNOT_START_RECEIVER));
		}
	}

	return ret;
}




int slsDetector::stopReceiver() {
	int fnum=F_STOP_RECEIVER;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Stopping Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->executeFunction(fnum,mess);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
		else if (ret == FAIL)
			setErrorMask((getErrorMask())|(COULDNOT_STOP_RECEIVER));
	}

	return ret;
}




slsDetectorDefs::runStatus slsDetector::startReceiverReadout() {
	int fnum=F_START_RECEIVER_READOUT;
	int ret = FAIL;
	int retval=-1;
	runStatus s=ERROR;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Starting Receiver Readout" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(retval!=-1)
			s=(runStatus)retval;
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return s;
}





slsDetectorDefs::runStatus slsDetector::getReceiverStatus() {
	int fnum=F_GET_RECEIVER_STATUS;
	int ret = FAIL;
	int retval=-1;
	runStatus s=ERROR;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Getting Receiver Status" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(retval!=-1)
			s=(runStatus)retval;
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return s;
}




int slsDetector::getFramesCaughtByReceiver() {
	int fnum=F_GET_RECEIVER_FRAMES_CAUGHT;
	int ret = FAIL;
	int retval=-1;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Getting Frames Caught by Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return retval;
}



int slsDetector::getFramesCaughtByAnyReceiver() {
	return getFramesCaughtByReceiver();
}



int slsDetector::getReceiverCurrentFrameIndex() {
	int fnum=F_GET_RECEIVER_FRAME_INDEX;
	int ret = FAIL;
	int retval=-1;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Getting Current Frame Index of Receiver " << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->getInt(fnum,retval);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return retval;
}




int slsDetector::resetFramesCaught() {
	int fnum=F_RESET_RECEIVER_FRAMES_CAUGHT;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "Reset Frames Caught by Receiver" << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->executeFunction(fnum,mess);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
	}

	return ret;
}










int slsDetector::enableWriteToFile(int enable) {
	int fnum=F_ENABLE_RECEIVER_FILE_WRITE;
	int ret = FAIL;
	int retval=-1;
	int arg = enable;


	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(enable>=0){
			pthread_mutex_lock(&ms);
			multiDet->enableWriteToFileMask(enable);
			pthread_mutex_unlock(&ms);
		}
	}

	else if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending enable file write to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			multiDet->enableWriteToFileMask(retval);
			pthread_mutex_unlock(&ms);
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	pthread_mutex_lock(&ms);
	retval = multiDet->enableWriteToFileMask();
	pthread_mutex_unlock(&ms);

	return retval;
}




int slsDetector::overwriteFile(int enable) {
	int fnum=F_ENABLE_RECEIVER_OVERWRITE;
	int ret = FAIL;
	int retval=-1;
	int arg = enable;


	if(thisDetector->receiverOnlineFlag==OFFLINE_FLAG){
		if(enable>=0){
			pthread_mutex_lock(&ms);
			multiDet->enableOverwriteMask(enable);
			pthread_mutex_unlock(&ms);
		}
	}

	else if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending enable file write to receiver " << arg << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret!=FAIL){
			pthread_mutex_lock(&ms);
			multiDet->enableOverwriteMask(retval);
			pthread_mutex_unlock(&ms);
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	pthread_mutex_lock(&ms);
	retval = multiDet->enableOverwriteMask();
	pthread_mutex_unlock(&ms);

	return retval;
}













int slsDetector::setReadReceiverFrequency(int freq) {

	if (freq >= 0) {
		thisDetector->receiver_read_freq = freq;

		int fnum=F_READ_RECEIVER_FREQUENCY;
		int ret = FAIL;
		int retval=-1;
		int arg = freq;

		if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "Sending read frequency to receiver " << arg  << std::endl;
#endif
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum,retval,arg);
				disconnectData();
			}
			if((ret == FAIL) || (retval != freq)) {
				cout << "could not set receiver read frequency to " << freq
						<<" Returned:" << retval << endl;
				setErrorMask((getErrorMask())|(RECEIVER_READ_FREQUENCY));
			}

			if(ret==FORCE_UPDATE)
				updateReceiver();
		}
	}

	return thisDetector->receiver_read_freq;
}



int slsDetector::setReceiverReadTimer(int time_in_ms) {
	int fnum=F_READ_RECEIVER_TIMER;
	int ret = FAIL;
	int arg = time_in_ms;
	int retval = -1;

	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Sending read timer to receiver " << arg  << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,arg);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			updateReceiver();
	}

	if ((time_in_ms > 0) && (retval != time_in_ms)){
		cout << "could not set receiver read timer to " << time_in_ms
				<<" Returned:" << retval << endl;
		setErrorMask((getErrorMask())|(RECEIVER_READ_TIMER));
	}
	return retval;
}


int slsDetector::enableDataStreamingToClient(int enable) {
	cprintf(RED,"ERROR: Must be called from the multi Detector level\n");
	return 0;
}





int slsDetector::enableDataStreamingFromReceiver(int enable) {

	if (enable >= 0) {
		int fnum=F_STREAM_DATA_FROM_RECEIVER;
		int ret = FAIL;
		int retval=-1;
		int arg = enable;


		if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
			std::cout << "***************Sending Data Streaming in Receiver "
					<< arg  << std::endl;
#endif
			if (connectData() == OK){
				ret=thisReceiver->sendInt(fnum,retval,arg);
				disconnectData();
			}
			if(ret==FAIL) {
				retval = -1;
				cout << "could not set data streaming in receiver to " <<
						enable <<" Returned:" << retval << endl;
				setErrorMask((getErrorMask())|(DATA_STREAMING));
			} else {
				thisDetector->receiver_upstream = retval;
				if(ret==FORCE_UPDATE)
					updateReceiver();
			}
		}
	}

	return thisDetector->receiver_upstream;
}



int slsDetector::enableReceiverCompression(int i) {
	int fnum=F_ENABLE_RECEIVER_COMPRESSION;
	int ret = FAIL;
	int retval=-1;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		std::cout << "Getting/Enabling/Disabling Receiver Compression with argument "
				<< i << std::endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,i);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(COULDNOT_ENABLE_COMPRESSION));
	}
	return retval;
}






int slsDetector::enableTenGigabitEthernet(int i) {
	int ret=FAIL;
	int retval = -1;
	int fnum=F_ENABLE_TEN_GIGA,fnum2 = F_ENABLE_RECEIVER_TEN_GIGA;
	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<< std::endl<< "Enabling / Disabling 10Gbe" << endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&i,sizeof(i));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret==FAIL){
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
				setErrorMask((getErrorMask())|(DETECTOR_TEN_GIGA));
			}
			controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}


	if(ret!=FAIL){
		//must also configuremac
		if((i != -1)&&(retval == i))
			if(configureMAC() != FAIL){
				if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
					ret = FAIL;
					retval=-1;
#ifdef VERBOSE
					std::cout << "Enabling / Disabling 10Gbe in receiver: "
							<< i << std::endl;
#endif
					if (connectData() == OK){
						ret=thisReceiver->sendInt(fnum2,retval,i);
						disconnectData();
					}
					if(ret==FAIL)
						setErrorMask((getErrorMask())|(RECEIVER_TEN_GIGA));
				}
			}
	}

	if(ret != FAIL)
		thisDetector->tenGigaEnable=retval;
	return retval;
}




int slsDetector::setReceiverFifoDepth(int i) {
	int fnum=F_SET_RECEIVER_FIFO_DEPTH;
	int ret = FAIL;
	int retval=-1;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		if(i ==-1)
			std::cout<< "Getting Receiver Fifo Depth" << endl;
		else
			std::cout<< "Setting Receiver Fifo Depth to " << i << endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,i);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(COULD_NOT_SET_FIFO_DEPTH));
	}
	return retval;
}



int slsDetector::setReceiverSilentMode(int i) {
	int fnum=F_SET_RECEIVER_SILENT_MODE;
	int ret = FAIL;
	int retval=-1;


	if(thisDetector->receiverOnlineFlag==ONLINE_FLAG){
#ifdef VERBOSE
		if(i ==-1)
			std::cout<< "Getting Receiver Silent Mode" << endl;
		else
			std::cout<< "Setting Receiver Silent Mode to " << i << endl;
#endif
		if (connectData() == OK){
			ret=thisReceiver->sendInt(fnum,retval,i);
			disconnectData();
		}
		if(ret==FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
		else
			thisDetector->receiver_silentMode = retval;
	}
	return thisDetector->receiver_silentMode;
}




int slsDetector::restreamStopFromReceiver() {
	int fnum=F_RESTREAM_STOP_FROM_RECEIVER;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = "";

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG) {
#ifdef VERBOSE
		std::cout << "To Restream stop dummy from Receiver via zmq" << std::endl;
#endif

		if (connectData() == OK){
			ret=thisReceiver->executeFunction(fnum,mess);
			disconnectData();
		}
		if(ret==FORCE_UPDATE)
			ret=updateReceiver();
		else if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RESTREAM_STOP_FROM_RECEIVER));
			std::cout << " Could not restream stop dummy packet from receiver" << endl;
		}
	}

	return ret;
}




int slsDetector::setCTBPattern(string fname) {

	uint64_t word;

	int addr=0;

	FILE *fd=fopen(fname.c_str(),"r");
	if (fd) {
		while (fread(&word, sizeof(word), 1,fd)) {
			setCTBWord(addr,word);
			// cout << hex << addr << " " << word << dec << endl;
			++addr;
		}

		fclose(fd);
	} else
		return -1;





	return addr;


}


uint64_t slsDetector::setCTBWord(int addr,uint64_t word) {

	//uint64_t ret;

	int ret=FAIL;
	uint64_t retval=-1;
	int fnum=F_SET_CTB_PATTERN;
	int mode=0; //sets word

	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<<"Setting CTB word" <<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&mode,sizeof(mode));
			controlSocket->SendDataOnly(&addr,sizeof(addr));
			controlSocket->SendDataOnly(&word,sizeof(word));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL)
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return retval;


}


int slsDetector::setCTBPatLoops(int level,int &start, int &stop, int &n) {


	int retval[3], args[4];

	args[0]=level;
	args[1]=start;
	args[2]=stop;
	args[3]=n;


	int ret=FAIL;
	int fnum=F_SET_CTB_PATTERN;
	int mode=1; //sets loop

	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<<"Setting CTB word" <<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&mode,sizeof(mode));
			controlSocket->SendDataOnly(&args,sizeof(args));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
				start=retval[0];
				stop=retval[1];
				n=retval[2];
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return ret;


}


int slsDetector::setCTBPatWaitAddr(int level, int addr) {




	int retval=-1;


	int ret=FAIL;
	int fnum=F_SET_CTB_PATTERN;
	int mode=2; //sets loop

	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<<"Setting CTB word" <<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&mode,sizeof(mode));
			controlSocket->SendDataOnly(&level,sizeof(level));
			controlSocket->SendDataOnly(&addr,sizeof(addr));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return retval;



}


int slsDetector::setCTBPatWaitTime(int level, uint64_t t) {


	uint64_t retval=-1;

	int ret=FAIL;
	//   uint64_t retval=-1;
	int fnum=F_SET_CTB_PATTERN;
	int mode=3; //sets loop

	char mess[MAX_STR_LENGTH]="";

#ifdef VERBOSE
	std::cout<<"Setting CTB word" <<std::endl;
#endif

	if (thisDetector->onlineFlag==ONLINE_FLAG) {
		if (connectControl() == OK){
			controlSocket->SendDataOnly(&fnum,sizeof(fnum));
			controlSocket->SendDataOnly(&mode,sizeof(mode));
			controlSocket->SendDataOnly(&level,sizeof(level));
			controlSocket->SendDataOnly(&t,sizeof(t));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			if (ret!=FAIL) {
				controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
			} else {
				controlSocket->ReceiveDataOnly(mess,sizeof(mess));
				std::cout<< "Detector returned error: " << mess << std::endl;
			}
			disconnectControl();
			if (ret==FORCE_UPDATE)
				updateDetector();
		}
	}

	return retval;


}



