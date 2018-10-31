#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "sls_detector_exceptions.h"
#include "SharedMemory.h"
#include "ClientInterface.h"
#include "gitInfoLib.h"
#include "versionAPI.h"
#include "slsDetectorCommand.h"
#include "utilities.h"
#include "MySocketTCP.h"


#include  <sys/types.h>
#include  <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bitset>
#include <cstdlib>
#include <math.h>
#include <iomanip>
#include <sys/stat.h>

#define DEFAULT_HOSTNAME  "localhost"


slsDetector::slsDetector(detectorType type, int multiId, int id, bool verify)
: detId(id),
  sharedMemory(0),
  thisDetector(0),
  thisDetectorControl(0),
  thisDetectorStop(0),
  thisReceiver(0),
  controlSocket(0),
  stopSocket(0),
  dataSocket(0),
  detectorModules(0),
  dacs(0),
  adcs(0),
  chanregs(0) {
	/* called from put hostname command,
	 * so sls shared memory will be created */

	// ensure shared memory was not created before
	auto shm = SharedMemory(multiId, id);
	if (shm.IsExisting()) {
		FILE_LOG(logWARNING) << "Weird, this shared memory should have been "
				"deleted before! " << shm.GetName() << ". Freeing it again";
		freeSharedMemory(multiId, id);
	}

	initSharedMemory(true, type, multiId, verify);
	initializeDetectorStructure(type);
	initializeMembers();
	initializeDetectorStructurePointers();
}


slsDetector::slsDetector(int multiId, int id, bool verify)
: detId(id),
  sharedMemory(0),
  thisDetector(0),
  thisDetectorControl(0),
  thisDetectorStop(0),
  thisReceiver(0),
  controlSocket(0),
  stopSocket(0),
  dataSocket(0),
  detectorModules(0),
  dacs(0),
  adcs(0),
  chanregs(0) {
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
	if (thisDetectorControl)
		delete thisDetectorControl;
	if (thisDetectorStop)
		delete thisDetectorStop;
	if (thisReceiver)
		delete thisReceiver;
	if (controlSocket)
		delete controlSocket;
	if (stopSocket)
		delete stopSocket;
	if (dataSocket)
		delete dataSocket;

	/* detectorModules, dacs..ffoerrors are offsets from the
	 * shared memory and created within shared memory structure.
	 * Deleting shared memory will also delete memory pointed to
	 * by these pointers
	 */
}


int slsDetector::checkVersionCompatibility(portType t) {
	int fnum = F_CHECK_VERSION;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = {0};
	int64_t arg = 0;

	// detector
	if (t == CONTROL_PORT) {

		// get api version number for detector server
		switch (thisDetector->myDetectorType) {
		case EIGER:		arg = APIEIGER; break;
		case JUNGFRAU:	arg = APIJUNGFRAU; break;
		case GOTTHARD:	arg = APIGOTTHARD; break;
		default:
			FILE_LOG(logERROR) << "Check version compatibility is not implemented for this detector";
			setErrorMask((getErrorMask())|(VERSION_COMPATIBILITY));
			return FAIL;
		}
		FILE_LOG(logDEBUG1) << "Checking version compatibility with detector with "
				"value " << std::hex << arg << std::dec;


		// control server
		if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
			ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), NULL, 0, mess);
			disconnectControl();
			if (ret == FAIL)
				thisDetector->detectorControlAPIVersion = 0;

			// stop server
			else {
				thisDetector->detectorControlAPIVersion = arg;
				ret = FAIL;
				if (connectStop() == OK) {
					ret = thisDetectorStop->Client_Send(fnum, &arg, sizeof(arg), NULL, 0, mess);
					disconnectStop();
					if (ret == FAIL)
						thisDetector->detectorStopAPIVersion = 0;
					else
						thisDetector->detectorStopAPIVersion = arg;
				}
			}
		}
	}

	// receiver
	else {
		fnum = F_RECEIVER_CHECK_VERSION;
		arg = APIRECEIVER;
		FILE_LOG(logDEBUG1) << "Checking version compatibility with receiver with "
				"value " << std::hex << arg << std::dec;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret=thisReceiver->Client_Send(fnum, &arg, sizeof(arg), NULL, 0);
			disconnectData();
			if (ret == FAIL)
				thisDetector->receiverAPIVersion = 0;
			else
				thisDetector->receiverAPIVersion = arg;
		}
	}

	if (ret == FAIL) {
		setErrorMask((getErrorMask())|(VERSION_COMPATIBILITY));
		if (strstr(mess,"Unrecognized Function") != NULL) {
			FILE_LOG(logERROR) << "The " << ((t == CONTROL_PORT) ? "detector" : "receiver") <<
					" server is too old to get API version. Please update detector server!";
		}
	}

	return ret;
}




int64_t slsDetector::getId( idMode mode) {
	int fnum = F_GET_ID;
	int ret = FAIL;
	int arg = (int)mode;
	int64_t retval = -1;

	FILE_LOG(logDEBUG1) << "Getting id type " << mode;

	// client version
	if (mode == THIS_SOFTWARE_VERSION) {
		ret = OK;
		retval = GITDATE;
	}

	// receiver version
	else if (mode==RECEIVER_VERSION) {
		fnum = F_GET_RECEIVER_ID;
		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL)
				setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
			else if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}

	// detector versions
	else  {
		if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
			ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectControl();

			// handle ret
			if (ret == FAIL)
				setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
			else if (ret == FORCE_UPDATE)
				updateDetector();
		}
	}

	if (ret != FAIL) {
		FILE_LOG(logDEBUG1) << "Id ("<< mode << "): 0x" << std::hex << retval << std::dec;
	}

	return retval;
}



void slsDetector::freeSharedMemory(int multiId, int slsId) {
	auto shm = SharedMemory(multiId, slsId);
	shm.RemoveSharedMemory();
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



void slsDetector::setHostname(const char *name) {
	setTCPSocket(std::string(name));
	if (thisDetector->onlineFlag == ONLINE_FLAG)
		updateDetector();
}

std::string slsDetector::getHostname() {
	return std::string(thisDetector->hostname);
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
				FILE_LOG(logERROR) << "Single shared memory "
						"(" << multiId << "-" << detId << ":) "
						"version mismatch "
						"(expected 0x" << std::hex << SLS_SHMVERSION <<
						" but got 0x" << thisDetector->shmversion << ")" << std::dec;
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
	case GOTTHARD:
		list.nChanX = 128;
		list.nChanY = 1;
		list.nChipX = 10;
		list.nChipY = 1;
		list.nDacs = 8;
		list.nAdcs = 5;
		list.dynamicRange = 16;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case JUNGFRAU:
		list.nChanX = 256;
		list.nChanY = 256;
		list.nChipX = 4;
		list.nChipY = 2;
		list.nDacs = 16;
		list.nAdcs = 0;
		list.dynamicRange = 16;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case JUNGFRAUCTB:
		list.nChanX = 36;
		list.nChanY = 1;
		list.nChipX = 1;
		list.nChipY = 1;
		list.nDacs = 16;
		list.nAdcs = 9;//???? When calculating size, only d+a=16 how come?FIXME
		list.dynamicRange =16;
		list.nGappixelsX = 0;
		list.nGappixelsY = 0;
		break;
	case EIGER:
		list.nChanX = 256;
		list.nChanY = 256;
		list.nChipX = 4;
		list.nChipY = 1;
		list.nDacs = 16;
		list.nAdcs = 0;
		list.dynamicRange = 16;
		list.nGappixelsX = 6;
		list.nGappixelsY = 1;
		break;
	default:
		FILE_LOG(logERROR) << "Unknown detector type!";
		throw std::exception();
	}
}


int slsDetector::calculateSharedMemorySize(detectorType type) {
	// get the detector parameters based on type
	detParameterList detlist;
	setDetectorSpecificParameters(type, detlist);
	int nch = detlist.nChanX * detlist.nChanY;
	int nc = detlist.nChipX * detlist.nChipY;
	int nd = detlist.nDacs + detlist.nAdcs;

	/** The size of the shared memory is
	 * size of shared structure +
	 * ffcoefficents+fferrors+modules+dacs+adcs+chans */
	int sz = sizeof(sharedSlsDetector) +
			2 * nch * nc * sizeof(double) +
			sizeof(sls_detector_module) +
			sizeof(int) * nd +
			sizeof(int) * nch * nc;
	FILE_LOG(logDEBUG1) << "Size of shared memory is " << sz;
	return sz;
}


void slsDetector::initializeDetectorStructure(detectorType type) {
	thisDetector->shmversion = SLS_SHMVERSION;
	thisDetector->onlineFlag = OFFLINE_FLAG;
	thisDetector->stoppedFlag = 0;
	strncpy(thisDetector->hostname, DEFAULT_HOSTNAME, MAX_STR_LENGTH-1);
	thisDetector->hostname[MAX_STR_LENGTH-1] = 0;
	thisDetector->myDetectorType = type;
	thisDetector->offset[X] = 0;
	thisDetector->offset[Y] = 0;
	thisDetector->multiSize[X] = 0;
	thisDetector->multiSize[Y] = 0;
	thisDetector->controlPort = DEFAULT_PORTNO;
	thisDetector->stopPort = DEFAULT_PORTNO + 1;
	strncpy(thisDetector->settingsDir, getenv("HOME"),  MAX_STR_LENGTH-1);
	thisDetector->settingsDir[MAX_STR_LENGTH-1] = 0;
	thisDetector->nTrimEn = 0;
	for(int i = 0; i < MAX_TRIMEN; ++i)
		thisDetector->trimEnergies[i] = 0;
	// thisDetector->threadedProcessing = 1;
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
	thisDetector->detectorControlAPIVersion = 0;
	thisDetector->detectorStopAPIVersion = 0;
	thisDetector->receiverAPIVersion = 0;
	thisDetector->receiver_frameDiscardMode = NO_DISCARD;
	thisDetector->receiver_framePadding = 1;
	thisDetector->activated = true;
	thisDetector->receiver_deactivatedPaddingEnable = true;
	thisDetector->receiver_silentMode = false;
	strcpy(thisDetector->receiver_filePath, "/");
	strcpy(thisDetector->receiver_fileName, "run");
	thisDetector->receiver_fileIndex = 0;
	thisDetector->receiver_fileFormatType = BINARY;
	switch(thisDetector->myDetectorType) {
	case GOTTHARD:
		thisDetector->receiver_framesPerFile = MAX_FRAMES_PER_FILE;
		break;
	case EIGER:
		thisDetector->receiver_framesPerFile = EIGER_MAX_FRAMES_PER_FILE;
		break;
	case JUNGFRAU:
		thisDetector->receiver_framesPerFile = JFRAU_MAX_FRAMES_PER_FILE;
		break;
	case JUNGFRAUCTB:
		thisDetector->receiver_framesPerFile = JFRAU_MAX_FRAMES_PER_FILE;
		break;
	default:
		break;
	}
	thisDetector->receiver_fileWriteEnable = 1;
	thisDetector->receiver_overWriteEnable = 1;

	// get the detector parameters based on type
	detParameterList detlist;
	setDetectorSpecificParameters(type, detlist);
	thisDetector->nChan[X] = detlist.nChanX;
	thisDetector->nChan[Y] = detlist.nChanY;
	thisDetector->nChip[X] = detlist.nChipX;
	thisDetector->nChip[Y] = detlist.nChipY;
	thisDetector->nDacs = detlist.nDacs;
	thisDetector->nAdcs = detlist.nAdcs;
	thisDetector->dynamicRange = detlist.dynamicRange;
	thisDetector->nGappixels[X] = detlist.nGappixelsX;
	thisDetector->nGappixels[Y] = detlist.nGappixelsY;


	// derived parameters
	thisDetector->nChans = thisDetector->nChan[X] * thisDetector->nChan[Y];
	thisDetector->nChips = thisDetector->nChip[X] * thisDetector->nChip[Y];

	// calculating databytes
	thisDetector->dataBytes = thisDetector->nChips * thisDetector->nChans *
			thisDetector->dynamicRange/8;
	thisDetector->dataBytesInclGapPixels =
			(thisDetector->nChip[X] * thisDetector->nChan[X] +
					thisDetector->gappixels * thisDetector->nGappixels[X]) *
					(thisDetector->nChip[Y] * thisDetector->nChan[Y] +
							thisDetector->gappixels * thisDetector->nGappixels[Y]) *
							thisDetector->dynamicRange/8;

	// special for jctb
	if (thisDetector->myDetectorType==JUNGFRAUCTB) {
		getTotalNumberOfChannels();
	}

	/** calculates the memory offsets for
	 * flat field coefficients and errors,
	 * module structures, dacs, adcs, chips and channels */
	thisDetector->modoff = sizeof(sharedSlsDetector);
	thisDetector->dacoff = thisDetector->modoff +
			sizeof(sls_detector_module);
	thisDetector->adcoff = thisDetector->dacoff +
			sizeof(int) * thisDetector->nDacs;
	thisDetector->chanoff = thisDetector->adcoff +
			sizeof(int) * thisDetector->nChips;
}


void slsDetector::initializeMembers() {
	// slsdetector
	// assign addresses
	char *goff = (char*)thisDetector;
	detectorModules = (sls_detector_module*)(goff + thisDetector->modoff);
	dacs = (int*)(goff + thisDetector->dacoff);
	adcs = (int*)(goff + thisDetector->adcoff);
	chanregs = (int*)(goff + thisDetector->chanoff);
	if (thisDetectorControl) {
		delete thisDetectorControl;
		thisDetectorControl = 0;
	}
	if (thisDetectorStop) {
		delete thisDetectorStop;
		thisDetectorStop = 0;
	}
	if (thisReceiver) {
		delete thisReceiver;
		thisReceiver = 0;
	}
	thisDetectorControl = new ClientInterface(controlSocket, detId, "Detector (Control server)");
	thisDetectorStop = new ClientInterface(stopSocket, detId, "Detector (Stop server)");
	thisReceiver = new ClientInterface(dataSocket, detId, "Receiver");
}




void slsDetector::initializeDetectorStructurePointers() {
	sls_detector_module* thisMod;

	// set thisMod to point to one of the detector structure modules
	thisMod = detectorModules;

	thisMod->serialnumber = 0;
	thisMod->nchan = thisDetector->nChans*thisDetector->nChips;
	thisMod->nchip = thisDetector->nChips;
	thisMod->ndac = thisDetector->nDacs;
	thisMod->nadc = thisDetector->nAdcs;
	thisMod->reg = 0;
	thisMod->iodelay = 0;
	thisMod->tau = 0;
	thisMod->eV = 0;
	// dacs, adcs and chanregs for thisMod is not allocated in
	// detectorModules in shared memory as they are already allocated separately
	// in shared memory (below)

	// initializes the dacs values to 0
	for (int i = 0; i < thisDetector->nDacs; ++i) {
		*(dacs + i) = 0;
	}
	// initializes the adc values to 0
	for (int i = 0; i < thisDetector->nAdcs; ++i) {
		*(adcs + i) = 0;
	}
	// initializes the channel registers to 0
	for (int i = 0; i < thisDetector->nChans * thisDetector->nChips; ++i) {
		*(chanregs + i) = -1;
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
	int *dacs = new int[nd];
	int *adcs = new int[na];
	int *chanregs = new int[nch*nc];

	sls_detector_module *myMod = (sls_detector_module*)malloc(sizeof(sls_detector_module));
	myMod->ndac = nd;
	myMod->nadc = na;
	myMod->nchip = nc;
	myMod->nchan = nch*nc;
	myMod->dacs = dacs;
	myMod->adcs = adcs;
	myMod->chanregs = chanregs;
	return myMod;
}


void  slsDetector::deleteModule(sls_detector_module *myMod) {
	delete [] myMod->dacs;
	delete [] myMod->adcs;
	delete [] myMod->chanregs;
	delete myMod;
}




int slsDetector::connectControl() {
	if (controlSocket) {
		if (controlSocket->Connect() >= 0)
			return OK;
		else {
			FILE_LOG(logERROR) << "Cannot connect to detector";
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

void slsDetector::connectDataError() {
    FILE_LOG(logERROR) << "Cannot connect to receiver";
    setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
}

int slsDetector::connectData() {
	if (dataSocket) {
		if (dataSocket->Connect() >= 0)
			return OK;
		else {
		    connectDataError();
			return FAIL;
		}
	}
	return UNDEFINED;
}

void slsDetector::disconnectData() {
	if (dataSocket)
		dataSocket->Disconnect();
}



int slsDetector::connectStop() {
	if (stopSocket) {
		if (stopSocket->Connect() >= 0)
			return OK;
		else {
			FILE_LOG(logERROR) << "Cannot connect to stop server";
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



int slsDetector::sendModule(sls_detector_module *myMod) {
	int ts = 0;
	ts += controlSocket->SendDataOnly(&(myMod->serialnumber), sizeof(myMod->serialnumber));
	ts += controlSocket->SendDataOnly(&(myMod->nchan), sizeof(myMod->nchan));
	ts += controlSocket->SendDataOnly(&(myMod->nchip), sizeof(myMod->nchip));
	ts += controlSocket->SendDataOnly(&(myMod->ndac), sizeof(myMod->ndac));
	ts += controlSocket->SendDataOnly(&(myMod->nadc), sizeof(myMod->nadc));
	ts += controlSocket->SendDataOnly(&(myMod->reg), sizeof(myMod->reg));
	ts += controlSocket->SendDataOnly(&(myMod->iodelay), sizeof(myMod->iodelay));
	ts += controlSocket->SendDataOnly(&(myMod->tau), sizeof(myMod->tau));
	ts += controlSocket->SendDataOnly(&(myMod->eV), sizeof(myMod->eV));

	ts += controlSocket->SendDataOnly(myMod->dacs, sizeof(int) * (myMod->ndac));
	ts += controlSocket->SendDataOnly(myMod->adcs, sizeof(int) * (myMod->nadc));
	if (thisDetector->myDetectorType == EIGER) {
		ts += controlSocket->SendDataOnly(myMod->chanregs, sizeof(int) * (myMod->nchan));
	}
	return ts;
}


int  slsDetector::receiveModule(sls_detector_module* myMod) {
	int ts = 0;
	ts += controlSocket->ReceiveDataOnly(&(myMod->serialnumber), sizeof(myMod->serialnumber));
	ts += controlSocket->ReceiveDataOnly(&(myMod->nchan), sizeof(myMod->nchan));
	ts += controlSocket->ReceiveDataOnly(&(myMod->nchip), sizeof(myMod->nchip));
	ts += controlSocket->ReceiveDataOnly(&(myMod->ndac), sizeof(myMod->ndac));
	ts += controlSocket->ReceiveDataOnly(&(myMod->nadc), sizeof(myMod->nadc));
	ts += controlSocket->ReceiveDataOnly(&(myMod->reg), sizeof(myMod->reg));
	ts += controlSocket->ReceiveDataOnly(&(myMod->iodelay), sizeof(myMod->iodelay));
	ts += controlSocket->ReceiveDataOnly(&(myMod->tau), sizeof(myMod->tau));
	ts += controlSocket->ReceiveDataOnly(&(myMod->eV), sizeof(myMod->eV));

	ts += controlSocket->ReceiveDataOnly(myMod->dacs,sizeof(int)*(myMod->ndac));
	FILE_LOG(logDEBUG1) << "received dacs of size "<< ts;
	ts += controlSocket->ReceiveDataOnly(myMod->adcs,sizeof(int)*(myMod->nadc));
	FILE_LOG(logDEBUG1) << "received adc  of size "<< ts;
	if (thisDetector->myDetectorType == EIGER) {
		ts += controlSocket->ReceiveDataOnly(myMod->chanregs, sizeof(int)*(myMod->nchan));
		FILE_LOG(logDEBUG1) << "nchans= " << thisDetector->nChans << " nchips= " << thisDetector->nChips
				<< "mod - nchans= " << myMod->nchan << " nchips= " <<myMod->nchip
				<< "received chans of size "<< ts;
	}
	FILE_LOG(logDEBUG1) << "received module of size "<< ts << " register " << myMod->reg;
	return ts;
}


slsDetectorDefs::detectorType slsDetector::getDetectorTypeFromShm(int multiId, bool verify) {
	auto shm = SharedMemory(multiId, detId);
	if (!shm.IsExisting()) {
		FILE_LOG(logERROR) << "Shared memory " << shm.GetName() << " does not exist.\n"
				"Corrupted Multi Shared memory. Please free shared memory.";
		throw SharedMemoryException();
	}

	size_t sz = sizeof(sharedSlsDetector);

	// open, map, verify version
	auto sdet = (sharedSlsDetector*)shm.OpenSharedMemory(sz);
	if (verify && sdet->shmversion != SLS_SHMVERSION) {
		FILE_LOG(logERROR) << "Single shared memory "
				"(" << multiId << "-" << detId << ":)version mismatch "
				"(expected 0x" << std::hex << SLS_SHMVERSION <<
				" but got 0x" << sdet->shmversion << ")" << std::dec;
		// unmap and throw
		sharedMemory->UnmapSharedMemory(thisDetector);
		throw SharedMemoryException();
	}

	// get type, unmap
	auto type = sdet->myDetectorType;
	shm.UnmapSharedMemory(sdet);
	return type;
}


// static function
slsDetectorDefs::detectorType slsDetector::getDetectorType(const char *name, int cport) {
	int fnum = F_GET_DETECTOR_TYPE;
	int ret = FAIL;
	detectorType retval = GENERIC;
	MySocketTCP* mySocket = 0;

	try {
		mySocket = new MySocketTCP(name, cport);
	} catch(...) {
		FILE_LOG(logERROR) << "Cannot create socket to control server " << name
				<< " over port " << cport;
		return retval;
	}

	FILE_LOG(logDEBUG1) << "Getting detector type ";
	if (mySocket->Connect() >= 0) {
		mySocket->SendDataOnly(&fnum,sizeof(fnum));
		mySocket->ReceiveDataOnly(&ret,sizeof(ret));
		mySocket->ReceiveDataOnly(&retval,sizeof(retval));
		mySocket->Disconnect();
	} else {
		FILE_LOG(logERROR) << "Cannot connect to server " << name << " over port " << cport;
	}
	if (ret != FAIL) {
		FILE_LOG(logDEBUG1) << "Detector type is " << retval;
	}

	delete mySocket;
	return retval;
}


int slsDetector::setDetectorType(detectorType const type) {
	int fnum = F_GET_DETECTOR_TYPE;
	int ret = FAIL;
	detectorType retval = GENERIC;
	FILE_LOG(logDEBUG1) << "Setting detector type to " << type;

	// if unspecified, then get from detector
	if (type == GET_DETECTOR_TYPE) {
		if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
			ret = thisDetectorControl->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
			disconnectControl();

			// handle ret
			if (ret == FAIL) {
				// ret is never fail with this function
				setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
				thisDetector->myDetectorType = GENERIC;
			} else {
				thisDetector->myDetectorType = (detectorType)retval;
				FILE_LOG(logDEBUG1) << "Detector Type: " << retval;
				if (ret == FORCE_UPDATE)
					ret = updateDetector();
			}
		}
	} else ret = OK;

	// receiver
	if ((thisDetector->receiverOnlineFlag == ONLINE_FLAG) && ret == OK) {
		fnum = F_GET_RECEIVER_TYPE;
		ret = FAIL;
		int arg = (int)thisDetector->myDetectorType;
		retval = GENERIC;
		FILE_LOG(logINFO) << "Sending detector type to Receiver " << (int)thisDetector->myDetectorType;

		if (connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				FILE_LOG(logERROR) << "Could not send detector type to receiver";
				setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTTYPE_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver Type: " << retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return retval;
}



int slsDetector::setDetectorType(std::string const stype) {
	return setDetectorType(getDetectorType(stype));
}


slsDetectorDefs::detectorType slsDetector::getDetectorsType() {
	return thisDetector->myDetectorType;
}

std::string slsDetector::sgetDetectorsType() {
	return getDetectorType(getDetectorsType());
}


std::string slsDetector::getDetectorType() {
	return sgetDetectorsType();
}


int slsDetector::getTotalNumberOfChannels() {
	FILE_LOG(logDEBUG1) << "Get total number of channels";

	if (thisDetector->myDetectorType == JUNGFRAUCTB) {
		if (thisDetector->roFlags & DIGITAL_ONLY)
			thisDetector->nChan[X] = 4;
		else if (thisDetector->roFlags & ANALOG_AND_DIGITAL)
			thisDetector->nChan[X] = 36;
		else
			thisDetector->nChan[X] = 32;
		if (thisDetector->nChan[X] >= 32) {
			if (thisDetector->nROI > 0) {
				thisDetector->nChan[X] -= 32;
				for (int iroi = 0; iroi < thisDetector->nROI; ++iroi)
					thisDetector->nChan[X] +=
							thisDetector->roiLimits[iroi].xmax -
							thisDetector->roiLimits[iroi].xmin + 1;
			}
		}
		thisDetector->nChans = thisDetector->nChan[X];
		thisDetector->dataBytes = thisDetector->nChans * thisDetector->nChips * 2
				* thisDetector->timerValue[SAMPLES_JCTB];
	}

	FILE_LOG(logDEBUG1) << "Total number of channels: " <<
			thisDetector->nChans * thisDetector->nChips <<
			 ". Data bytes: " << thisDetector->dataBytes;

	return thisDetector->nChans * thisDetector->nChips;
}

int slsDetector::getTotalNumberOfChannels(dimension d) {
	getTotalNumberOfChannels();
	return thisDetector->nChan[d] * thisDetector->nChip[d];
}

int slsDetector::getTotalNumberOfChannelsInclGapPixels(dimension d) {
	getTotalNumberOfChannels();
	return (thisDetector->nChan[d] * thisDetector->nChip[d] + thisDetector->gappixels
			* thisDetector->nGappixels[d]);
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

void slsDetector::updateMultiSize(int detx, int dety) {
	thisDetector->multiSize[0] = detx;
	thisDetector->multiSize[1] = dety;
}


int slsDetector::setOnline(int off) {
	if (off != GET_ONLINE_FLAG) {
		thisDetector->onlineFlag = off;
		// set online
		if (thisDetector->onlineFlag == ONLINE_FLAG) {
			int old = thisDetector->onlineFlag;
			setTCPSocket();
			// connecting first time
			if (thisDetector->onlineFlag == ONLINE_FLAG && old == OFFLINE_FLAG) {
				FILE_LOG(logINFO) << "Detector connecting for the first time - updating!";
				updateDetector();
			}
			// error
			else if (thisDetector->onlineFlag == OFFLINE_FLAG) {
				FILE_LOG(logERROR) << "Cannot connect to detector";
				setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_DETECTOR));
			}
		}
	}
	return thisDetector->onlineFlag;
}


std::string slsDetector::checkOnline() {
	std::string retval;
	//if it doesnt exit, create socket and call this function again
	if (!controlSocket) {
		setTCPSocket();
		if (thisDetector->onlineFlag == OFFLINE_FLAG)
			return std::string(thisDetector->hostname);
		else
			return retval;
	}
	//still cannot connect to socket, controlSocket=0
	if (controlSocket) {
		if (connectControl() == FAIL) {
			controlSocket->SetTimeOut(5);
			thisDetector->onlineFlag = OFFLINE_FLAG;
			delete controlSocket;
			controlSocket = 0;
			retval = std::string(thisDetector->hostname);
			FILE_LOG(logDEBUG1) << "offline!";
		}  else {
			thisDetector->onlineFlag = ONLINE_FLAG;
			controlSocket->SetTimeOut(100);
			disconnectControl();
			FILE_LOG(logDEBUG1) << "online!";
		}
	}
	//still cannot connect to socket, stopSocket=0
	if (stopSocket) {
		if (connectStop() == FAIL) {
			stopSocket->SetTimeOut(5);
			thisDetector->onlineFlag = OFFLINE_FLAG;
			delete stopSocket;
			stopSocket = 0;
			retval = std::string(thisDetector->hostname);
			FILE_LOG(logDEBUG1) << "stop offline!";
		}  else {
			thisDetector->onlineFlag = ONLINE_FLAG;
			stopSocket->SetTimeOut(100);
			disconnectStop();
			FILE_LOG(logDEBUG1) << "stop online!";
		}
	}
	return retval;
}


int slsDetector::setTCPSocket(std::string const name, int const control_port, int const stop_port) {
	char thisName[MAX_STR_LENGTH] = {0};
	int thisCP = 0, thisSP = 0;
	int ret = OK;

	// hostname
	if (name.empty()) {
		strcpy(thisName,thisDetector->hostname);
	} else {
		FILE_LOG(logDEBUG1) << "Setting hostname";
		strcpy(thisName, name.c_str());
		strcpy(thisDetector->hostname, thisName);
		if (controlSocket) {
			delete controlSocket;
			controlSocket = 0;
		}
		if (stopSocket) {
			delete stopSocket;
			stopSocket = 0;
		}
	}

	// control port
	if (control_port <= 0) {
		thisCP = thisDetector->controlPort;
	} else {
		FILE_LOG(logDEBUG1) << "Setting control port";
		thisCP = control_port;
		thisDetector->controlPort = thisCP;
		if (controlSocket) {
			delete controlSocket;
			controlSocket = 0;
		}
	}

	// stop port
	if (stop_port <= 0) {
		thisSP = thisDetector->stopPort;
	} else {
		FILE_LOG(logDEBUG1) << "Setting stop port";
		thisSP = stop_port;
		thisDetector->stopPort = thisSP;
		if (stopSocket) {
			delete stopSocket;
			stopSocket = 0;
		}
	}

	// create control socket
	if (!controlSocket) {
		try {
			controlSocket = new MySocketTCP(thisName, thisCP);
			FILE_LOG(logDEBUG1) << "Control socket connected " << thisName  << " " << thisCP;
		} catch(...) {
			FILE_LOG(logERROR) << "Could not connect control socket " << thisName  << " " << thisCP;
			controlSocket = 0;
			ret = FAIL;
		}
	}


	// create stop socket
	if (!stopSocket) {
		try {
			stopSocket = new MySocketTCP(thisName, thisSP);
			FILE_LOG(logDEBUG1) << "Stop socket connected " << thisName  << " " << thisSP;
		} catch(...) {
			FILE_LOG(logERROR) << "Could not connect Stop socket " << thisName  << " " << thisSP;
			stopSocket = 0;
			ret = FAIL;
		}
	}


	if (ret == FAIL) {
		thisDetector->onlineFlag = OFFLINE_FLAG;
		FILE_LOG(logDEBUG1) << "Detector offline";
	}

	// check online and version compatibility
	else {
		checkOnline();
		thisDetectorControl->SetSocket(controlSocket);
		thisDetectorStop->SetSocket(stopSocket);
		// check for version compatibility
		switch (thisDetector->myDetectorType) {
		case EIGER:
		case JUNGFRAU:
		case GOTTHARD:
			// check version compatibility only if it hasnt been checked yet (shm value)
			if ((thisDetector->detectorControlAPIVersion == 0) ||
					(thisDetector->detectorStopAPIVersion == 0))    	{
				if (checkVersionCompatibility(CONTROL_PORT) == FAIL)
					thisDetector->onlineFlag=OFFLINE_FLAG;
			}
			break;
		default:
			break;
		}
	}

	return ret;
}




int slsDetector::setPort(portType index, int num) {
	int fnum = F_SET_PORT;
	int ret = FAIL;
	int retval = -1;

	// set
	if (num >= 0) {

		switch(index) {

		case CONTROL_PORT:
			FILE_LOG(logDEBUG1) << "Setting control port " << " to " << num;

			// same port
			if (num == thisDetector->controlPort)
				return thisDetector->controlPort;

			// control socket not created
			if (!controlSocket) {
				FILE_LOG(logDEBUG1) << "Control socket not created. "
						"Connecting to port: " << thisDetector->controlPort;
				setTCPSocket();
			}

			// set port
			if (controlSocket && thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
				ret = thisDetectorControl->Client_Send(fnum, &num, sizeof(num), &retval, sizeof(retval));
				disconnectControl();

				// handle ret
				if (ret == FAIL) {
					setErrorMask((getErrorMask())|(COULDNOT_SET_CONTROL_PORT));
				} else {
					thisDetector->controlPort = retval;
					FILE_LOG(logDEBUG1) << "Control port: " << retval;
					if (ret == FORCE_UPDATE)
						ret = updateDetector();
				}
			}
			break;

		case STOP_PORT:
			FILE_LOG(logDEBUG1) << "Setting stop port " << " to " << num;

			// same port
			if (num == thisDetector->stopPort)
				return thisDetector->stopPort;

			// stop socket not created
			if (!stopSocket) {
				FILE_LOG(logDEBUG1) << "Stop socket not created. "
						"Connecting to port: " << thisDetector->stopPort;
				setTCPSocket();
			}

			// set port
			if (stopSocket && thisDetector->onlineFlag == ONLINE_FLAG && connectStop() == OK) {
				ret = thisDetectorStop->Client_Send(fnum, &num, sizeof(num), &retval, sizeof(retval));
				disconnectStop();

				// handle ret
				if (ret == FAIL) {
					setErrorMask((getErrorMask())|(COULDNOT_SET_STOP_PORT));
				} else {
					thisDetector->stopPort = retval;
					FILE_LOG(logDEBUG1) << "Stop port: " << retval;
				}
			}
			break;

		case DATA_PORT:
			FILE_LOG(logDEBUG1) << "Setting receiver port " << " to " << num;

			// same port
			if (num == thisDetector->receiverTCPPort)
				return thisDetector->receiverTCPPort;

			// if receiver hostname not given yet
			if (!strcmp(thisDetector->receiver_hostname, "none")) {
				thisDetector->receiverTCPPort = num;
				break;
			}

			// control socket not created
			if (!dataSocket) {
				FILE_LOG(logDEBUG1) << "Data socket not created. "
						"Connecting to port: " << thisDetector->receiverTCPPort;;
				setReceiverTCPSocket();
			}

			// set port
			if (dataSocket && thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
				ret = thisReceiver->Client_Send(fnum, &num, sizeof(num), &retval, sizeof(retval));
				disconnectData();

				// handle ret
				if (ret == FAIL) {
					setErrorMask((getErrorMask())|(COULDNOT_SET_DATA_PORT));
				} else {
					thisDetector->receiverTCPPort = retval;
					FILE_LOG(logDEBUG1) << "Receiver port: " << retval;
					if (ret == FORCE_UPDATE)
						ret = updateReceiver();
				}
			}
			break;

		default:
			FILE_LOG(logERROR) << "Unknown port index " << index;
			return -1;
		}
	}

	// get
	switch(index) {
	case CONTROL_PORT:
		return thisDetector->controlPort;
	case STOP_PORT:
		return thisDetector->stopPort;
	case DATA_PORT:
		return thisDetector->receiverTCPPort;
	default:
		return -1;
	}
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
	int fnum = F_LOCK_SERVER;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting detector server lock to " << lock;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &lock, sizeof(lock), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Lock: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


std::string slsDetector::getLastClientIP() {
	int fnum = F_GET_LAST_CLIENT_IP;
	int ret = FAIL;
	char retval[INET_ADDRSTRLEN] = {0};
	FILE_LOG(logDEBUG1) << "Getting last client ip to detector server";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Last client IP to detector: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return std::string(retval);
}



int slsDetector::exitServer() {
	int fnum = F_EXIT_SERVER;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Sending exit command to detector server";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();
		// no ret handling as ret never fail
		FILE_LOG(logINFO) << "Shutting down the Detector server";
	}
	return ret;
}


int slsDetector::execCommand(std::string cmd) {
	int fnum = F_EXEC_COMMAND;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};
	strcpy(arg, cmd.c_str());
	FILE_LOG(logDEBUG1) << "Sending command to detector " << arg;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum,
				arg, sizeof(arg), retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logINFO) << "Detector " << detId << " returned:\n" << retval;

		}
	}
	return ret;
}


int slsDetector::updateDetectorNoWait() {
	int n = 0, i32 = 0;
	int64_t i64 = 0;
	char lastClientIP[INET_ADDRSTRLEN] = {0};

	n += controlSocket->ReceiveDataOnly(lastClientIP, sizeof(lastClientIP));
	FILE_LOG(logDEBUG1) << "Updating detector last modified by " << lastClientIP;

	n += controlSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->dynamicRange = i32;

	n += controlSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->dataBytes = i32;

	n += controlSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->currentSettings = (detectorSettings)i32;

	if (thisDetector->myDetectorType == EIGER) {
		n += controlSocket->ReceiveDataOnly(&i32, sizeof(i32));
		thisDetector->currentThresholdEV = i32;
	}

	n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
	thisDetector->timerValue[FRAME_NUMBER] = i64;

	n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
	thisDetector->timerValue[ACQUISITION_TIME] = i64;

	if (thisDetector->myDetectorType == EIGER) {
		n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
		thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME] = i64;

		n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
		thisDetector->timerValue[SUBFRAME_DEADTIME] = i64;
	}

	n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
	thisDetector->timerValue[FRAME_PERIOD] = i64;

	if (thisDetector->myDetectorType != EIGER) {
		n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
		thisDetector->timerValue[DELAY_AFTER_TRIGGER] = i64;
	}

	if ((thisDetector->myDetectorType != JUNGFRAU) &&
			(thisDetector->myDetectorType != EIGER)) {
		n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
		thisDetector->timerValue[GATES_NUMBER] = i64;
	}

	n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
	thisDetector->timerValue[CYCLES_NUMBER] = i64;

	if (thisDetector->myDetectorType == JUNGFRAUCTB) {
		n += controlSocket->ReceiveDataOnly(&i64, sizeof(i64));
		if (i64 >= 0)
			thisDetector->timerValue[SAMPLES_JCTB] = i64;

		n += controlSocket->ReceiveDataOnly(&i32, sizeof(i32));
		thisDetector->roFlags = (readOutFlags)i32;

		getTotalNumberOfChannels();
	}

	if (!n) {
		FILE_LOG(logERROR) << "Could not update detector, received 0 bytes";
	}

	return OK;
}


int slsDetector::updateDetector() {
	int fnum = F_UPDATE_CLIENT;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Sending update client to detector server";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		// no ret handling as ret never fail
		updateDetectorNoWait();
		disconnectControl();
	}
	return ret;
}


int slsDetector::writeConfigurationFile(std::string const fname, multiSlsDetector* m) {
	int iline = 0;
	std::ofstream outfile;
	outfile.open(fname.c_str(),std::ios_base::out);
	if (outfile.is_open()) {
		iline = writeConfigurationFile(outfile, m);
		outfile.close();
	}
	else {
		FILE_LOG(logERROR) << "Error opening configuration file " << fname <<
				" for writing";
		setErrorMask((getErrorMask())|(CONFIG_FILE));
		return FAIL;
	}
	FILE_LOG(logINFO) << iline << " lines written to configuration file";
	return OK;
}


int slsDetector::writeConfigurationFile(std::ofstream &outfile, multiSlsDetector* m) {

	FILE_LOG(logDEBUG1) << "Write configuration file";

	std::vector<std::string> names;
	// common config
	names.push_back("hostname");
	names.push_back("port");
	names.push_back("stopport");
	names.push_back("settingsdir");
	names.push_back("ffdir");
	names.push_back("outdir");
	names.push_back("lock");
	// receiver config
	names.push_back("detectormac");
	names.push_back("detectorip");
	names.push_back("zmqport");
	names.push_back("rx_zmqport");
	names.push_back("zmqip");
	names.push_back("rx_zmqip");
	names.push_back("rx_tcpport");
	names.push_back("rx_udpport");
	names.push_back("rx_udpport2");
	names.push_back("rx_udpip");
	names.push_back("rx_hostname");
	names.push_back("r_readfreq");
	// detector specific config
	switch (thisDetector->myDetectorType) {
	case GOTTHARD:
		names.push_back("extsig:0");
		names.push_back("vhighvoltage");
		break;
	case EIGER:
		names.push_back("vhighvoltage");
		names.push_back("trimen");
		names.push_back("iodelay");
		names.push_back("tengiga");
		break;
	case JUNGFRAU:
		names.push_back("powerchip");
		names.push_back("vhighvoltage");
		break;
	case JUNGFRAUCTB:
		names.push_back("powerchip");
		names.push_back("vhighvoltage");
		break;
	default:
		FILE_LOG(logERROR) << "Unknown detector type " << thisDetector->myDetectorType;
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		return FAIL;
	}

	auto cmd = slsDetectorCommand(m);
	for (unsigned int iline = 0; iline < names.size(); ++iline) {
		char* args[] = {(char*)names[iline].c_str()};
		outfile << detId << ":";
		outfile << names[iline] << " " << cmd.executeLine(1, args, GET_ACTION) << std::endl;
	}
	return OK;
}


std::string slsDetector::getSettingsFile() {
	std::string s(thisDetector->settingsFile);
	// return without suffix .sn[detid]
	if (s.length() > 6) {
		if (s.substr(s.length()-6,3) == std::string(".sn") && s.substr(s.length()-3)!=std::string("xxx"))
			return s.substr(0,s.length()-6);
	}
	return std::string(thisDetector->settingsFile);
}


int slsDetector::writeSettingsFile(std::string fname) {
	return writeSettingsFile(fname, detectorModules[0]);
}


slsDetectorDefs::detectorSettings  slsDetector::getSettings() {
	return sendSettingsOnly(GET_SETTINGS);
}


slsDetectorDefs::detectorSettings slsDetector::setSettings( detectorSettings isettings) {
	FILE_LOG(logDEBUG1) << "slsDetector setSettings " << isettings;

	if (isettings == -1)
		return getSettings();

	// eiger: only set shm, setting threshold loads the module data
	if (thisDetector->myDetectorType == EIGER) {
		switch (isettings) {
		case STANDARD:
		case HIGHGAIN:
		case LOWGAIN:
		case VERYHIGHGAIN:
		case VERYLOWGAIN:
			thisDetector->currentSettings = isettings;
			return thisDetector->currentSettings;
		default:
			FILE_LOG(logERROR) << "Unknown settings " << getDetectorSettings(isettings) <<
			" for this detector!";
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			return GET_SETTINGS;
		}
	}

	// others: send only the settings, detector server will update dac values already in server
	return sendSettingsOnly(isettings);
}


slsDetectorDefs::detectorSettings slsDetector::sendSettingsOnly(detectorSettings isettings) {
	int fnum = F_SET_SETTINGS;
	int ret = FAIL;
	int arg = (int)isettings;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting settings to " << arg;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "Settings: " << retval;
			thisDetector->currentSettings = (detectorSettings)retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return thisDetector->currentSettings;
}



int slsDetector::getThresholdEnergy() {
	int fnum = F_GET_THRESHOLD_ENERGY;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Getting threshold energy";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Threshold: " << retval;
			thisDetector->currentThresholdEV = retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return thisDetector->currentThresholdEV;
}


int slsDetector::setThresholdEnergy(int e_eV, detectorSettings isettings, int tb) {

	// check as there is client processing
	if (thisDetector->myDetectorType == EIGER) {
		setThresholdEnergyAndSettings(e_eV, isettings, tb);
		return thisDetector->currentThresholdEV;
	}

	FILE_LOG(logERROR) << "Set threshold energy not implemented for this detector";
	setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
	return -1;
}


int slsDetector::setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings, int tb) {

	// if settings provided, use that, else use the shared memory variable
	detectorSettings is = ((isettings != GET_SETTINGS) ? isettings:
			thisDetector->currentSettings);
	std::string ssettings;
	switch (is) {
	case STANDARD:
		ssettings = "/standard";
		thisDetector->currentSettings=STANDARD;
		break;
	case HIGHGAIN:
		ssettings = "/highgain";
		thisDetector->currentSettings=HIGHGAIN;
		break;
	case LOWGAIN:
		ssettings = "/lowgain";
		thisDetector->currentSettings=LOWGAIN;
		break;
	case VERYHIGHGAIN:
		ssettings = "/veryhighgain";
		thisDetector->currentSettings=VERYHIGHGAIN;
		break;
	case VERYLOWGAIN:
		ssettings = "/verylowgain";
		thisDetector->currentSettings=VERYLOWGAIN;
		break;
	default:
		FILE_LOG(logERROR) << "Unknown settings " << getDetectorSettings(is) << " for this detector!";
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}

	//verify e_eV exists in trimEneregies[]
	if (!thisDetector->nTrimEn ||
			(e_eV < thisDetector->trimEnergies[0]) ||
			(e_eV > thisDetector->trimEnergies[thisDetector->nTrimEn-1]) ) {
		FILE_LOG(logERROR) << "This energy " << e_eV << " not defined for this module!";
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

	//normal
	if (!interpolate) {
		//find their directory names
		std::ostringstream ostfn;
		ostfn << thisDetector->settingsDir << ssettings << "/" << e_eV << "eV"
				<< "/noise.sn" << std::setfill('0') <<  std::setw(3) << std::dec << getId(DETECTOR_SERIAL_NUMBER) << std::setbase(10);
		std::string settingsfname = ostfn.str();
		FILE_LOG(logDEBUG1) << "Settings File is " << settingsfname;

		//read the files
		myMod = createModule(); // readSettings also checks if create module is null
		if (NULL == readSettingsFile(settingsfname, myMod, tb)) {
			if (myMod)
				deleteModule(myMod);
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
		std::ostringstream ostfn;
		ostfn << thisDetector->settingsDir << ssettings << "/" << trim1 << "eV"
				<< "/noise.sn" << std::setfill('0') <<  std::setw(3) << std::dec <<
				getId(DETECTOR_SERIAL_NUMBER) << std::setbase(10);
		std::string settingsfname1 = ostfn.str();
		ostfn.str(""); ostfn.clear();
		ostfn << thisDetector->settingsDir << ssettings << "/" << trim2 << "eV"
				<< "/noise.sn" << std::setfill('0') <<  std::setw(3) << std::dec <<
				getId(DETECTOR_SERIAL_NUMBER) << std::setbase(10);
		std::string settingsfname2 = ostfn.str();
		//read the files
		FILE_LOG(logDEBUG1) << "Settings Files are " << settingsfname1 << " and " << settingsfname2;
		sls_detector_module *myMod1=createModule();
		sls_detector_module *myMod2=createModule();
		if (NULL == readSettingsFile(settingsfname1, myMod1, tb)) {
			setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		if (NULL == readSettingsFile(settingsfname2, myMod2, tb)) {
			setErrorMask((getErrorMask())|(SETTINGS_FILE_NOT_OPEN));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		if (myMod1->iodelay != myMod2->iodelay) {
			FILE_LOG(logERROR) << "Iodelays do not match between files";
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		myMod->iodelay = myMod1->iodelay;

		//interpolate  module
		myMod = interpolateTrim( myMod1, myMod2, e_eV, trim1, trim2, tb);
		if (myMod == NULL) {
			FILE_LOG(logERROR) << "Could not interpolate, different dac values in files";
			setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
			deleteModule(myMod1);
			deleteModule(myMod2);
			return FAIL;
		}
		//interpolate tau
		myMod->tau = linearInterpolation(e_eV, trim1, trim2, myMod1->tau, myMod2->tau);
		//printf("new tau:%d\n",tau);

		deleteModule(myMod1);
		deleteModule(myMod2);
	}


	myMod->reg = thisDetector->currentSettings;
	myMod->eV = e_eV;
	setModule(*myMod, tb);
	deleteModule(myMod);
	if (getSettings() != is) {
		FILE_LOG(logERROR) << "Could not set settings in detector";
		setErrorMask((getErrorMask())|(SETTINGS_NOT_SET));
		return FAIL;
	}
	return OK;
}


std::string slsDetector::getSettingsDir() {
	return std::string(thisDetector->settingsDir);
}


std::string slsDetector::setSettingsDir(std::string s) {
	sprintf(thisDetector->settingsDir, s.c_str());
	return thisDetector->settingsDir;
}


int slsDetector::loadSettingsFile(std::string fname) {
	std::string fn = fname;
	std::ostringstream ostfn;
	ostfn << fname;

	// find specific file if it has detid in file name (.snxxx)
	if (thisDetector->myDetectorType == EIGER) {
		if (fname.find(".sn")==std::string::npos && fname.find(".trim")==
				std::string::npos && fname.find(".settings")==std::string::npos) {
			ostfn << ".sn"  << std::setfill('0') <<  std::setw(3) << std::dec <<
					getId(DETECTOR_SERIAL_NUMBER);
		}
	}
	fn = ostfn.str();

	// read settings file
	sls_detector_module  *myMod=NULL;
	myMod = readSettingsFile(fn, myMod);

	// set module
	int ret = FAIL;
	if (myMod != NULL) {
		myMod->reg = -1;
		myMod->eV = -1;
		ret = setModule(*myMod);
		deleteModule(myMod);
	}
	return ret;
}


int slsDetector::saveSettingsFile(std::string fname) {
	std::string fn = fname;
	std::ostringstream ostfn;
	ostfn << fname;

	// find specific file if it has detid in file name (.snxxx)
	if (thisDetector->myDetectorType == EIGER) {
		ostfn << ".sn"  << std::setfill('0') <<  std::setw(3) << std::dec <<
				getId(DETECTOR_SERIAL_NUMBER);
	}
	fn=ostfn.str();

	// get module
	int ret = FAIL;
	sls_detector_module *myMod = NULL;
	if ((myMod = getModule())) {
		ret = writeSettingsFile(fn, *myMod);
		deleteModule(myMod);
	}
	return ret;
}


slsDetectorDefs::runStatus slsDetector::getRunStatus() {
	int fnum = F_GET_RUN_STATUS;
	int ret = FAIL;
	runStatus retval = ERROR;
	FILE_LOG(logDEBUG1) << "Getting status";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectStop() == OK) {
		ret = thisDetectorStop->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectStop();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Detector status: " << runStatusType(retval);
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::prepareAcquisition() {
	int fnum = F_PREPARE_ACQUISITION;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Preparing Detector for Acquisition";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(PREPARE_ACQUISITION));
		} else {
			FILE_LOG(logDEBUG1) << "Prepare Acquisition successful";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return ret;
}





int slsDetector::startAcquisition() {
	int fnum = F_START_ACQUISITION;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Starting Acquisition";

	thisDetector->stoppedFlag = 0;
	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Starting Acquisition successful";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
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

	int fnum = F_STOP_ACQUISITION;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Stopping Acquisition";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectStop() == OK) {
		ret = thisDetectorStop->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectStop();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Stopping Acquisition successful";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	thisDetector->stoppedFlag = 1;

	// if rxr streaming and acquisition finished, restream dummy stop packet
	if ((thisDetector->receiver_upstream) && (s == IDLE) && (r == IDLE)) {
		restreamStopFromReceiver();
	}
	return ret;
}


int slsDetector::sendSoftwareTrigger() {
	int fnum = F_SOFTWARE_TRIGGER;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Sending software trigger";

	thisDetector->stoppedFlag = 0;
	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Sending software trigger successful";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return ret;
}


int slsDetector::startAndReadAll() {
	int fnum = F_START_AND_READ_ALL;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Starting and reading all frames";

	thisDetector->stoppedFlag = 0;
	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			thisDetector->stoppedFlag = 1;
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Detector successfully finished acquisition";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return ret;
}


int slsDetector::startReadOut() {
	int fnum = F_START_READOUT;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Starting readout";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Starting detector readout successful";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return ret;
}


int slsDetector::readAll() {
	int fnum = F_READ_ALL;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Reading all frames";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			thisDetector->stoppedFlag = 1;
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Detector successfully finished reading all frames";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return ret;
}


int slsDetector::configureMAC() {
	int fnum = F_CONFIGURE_MAC;
	int ret = FAIL;
	char args[9][50];
	memset(args, 0, sizeof(args));
	char retvals[3][50];
	memset(retvals, 0, sizeof(retvals));
	FILE_LOG(logDEBUG1) << "Configuring MAC";


	// if rx_udpip is none
	if (!(strcmp(thisDetector->receiverUDPIP,"none"))) {
		//hostname is an ip address
		if (strchr(thisDetector->receiver_hostname,'.') != NULL)
			strcpy(thisDetector->receiverUDPIP, thisDetector->receiver_hostname);
		//if hostname not ip, convert it to ip
		else {
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
	// rx_udpip and rx_udpmac is none
	if ((!strcmp(thisDetector->receiverUDPIP, "none")) ||
			(!strcmp(thisDetector->receiverUDPMAC, "none")))	{
		FILE_LOG(logERROR) << "Configure MAC Error. IP/MAC Addresses not set";
		setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
		return FAIL;
	}
	FILE_LOG(logDEBUG1) << "rx_hostname and rx_udpip is valid ";

	// copy to args
	strcpy(args[0],thisDetector->receiverUDPIP);
	strcpy(args[1],thisDetector->receiverUDPMAC);
	sprintf(args[2],"%x",thisDetector->receiverUDPPort);
	strcpy(args[3],thisDetector->detectorMAC);
	strcpy(args[4],thisDetector->detectorIP);
	sprintf(args[5],"%x",thisDetector->receiverUDPPort2);
	// 2d positions to detector to put into udp header
	{
		int pos[3] = {0, 0, 0};
		int max = thisDetector->multiSize[1];
		// row
		pos[0] = (detId % max);
		// col for horiz. udp ports
		pos[1] = (detId / max) * ((thisDetector->myDetectorType == EIGER) ? 2 : 1);
		// pos[2] (z is reserved)
		FILE_LOG(logDEBUG1) << "Detector [" << detId << "] - ("
				<< pos[0] << "," << pos[1] << ")";
		sprintf(args[6], "%x", pos[0]);
		sprintf(args[7], "%x", pos[1]);
		sprintf(args[8], "%x", pos[2]);
	}
	{
		//converting IPaddress to std::hex
		std::stringstream ss(args[0]);
		char cword[50]="";
		bzero(cword, 50);
		std::string s;
		while (getline(ss, s, '.')) {
			sprintf(cword,"%s%02x",cword,atoi(s.c_str()));
		}
		bzero(args[0], 50);
		strcpy(args[0],cword);
		FILE_LOG(logDEBUG1) << "receiver udp ip:" << args[0] << "-";
	}
	{
		//converting MACaddress to std::hex
		std::stringstream ss(args[1]);
		char cword[50]="";
		bzero(cword, 50);
		std::string s;
		while (getline(ss, s, ':')) {
			sprintf(cword,"%s%s",cword,s.c_str());
		}
		bzero(args[1], 50);
		strcpy(args[1],cword);
		FILE_LOG(logDEBUG1) << "receiver udp mac:" << args[1] << "-";
	}
	FILE_LOG(logDEBUG1) << "receiver udp port:" << args[2] << "-";
	{
		std::stringstream ss(args[3]);
		char cword[50]="";
		bzero(cword, 50);
		std::string s;
		while (getline(ss, s, ':')) {
			sprintf(cword,"%s%s",cword,s.c_str());
		}
		bzero(args[3], 50);
		strcpy(args[3],cword);
		FILE_LOG(logDEBUG1) << "detecotor udp mac:" << args[3] << "-";
	}
	{
		//converting IPaddress to std::hex
		std::stringstream ss(args[4]);
		char cword[50]="";
		bzero(cword, 50);
		std::string s;
		while (getline(ss, s, '.')) {
			sprintf(cword,"%s%02x",cword,atoi(s.c_str()));
		}
		bzero(args[4], 50);
		strcpy(args[4],cword);
		FILE_LOG(logDEBUG1) << "detecotor udp ip:" << args[4] << "-";
	}
	FILE_LOG(logDEBUG1) << "receiver udp port2:" << args[5] << "-";
	FILE_LOG(logDEBUG1) << "row:" << args[6] << "-";
	FILE_LOG(logDEBUG1) << "col:" << args[7] << "-";
	FILE_LOG(logDEBUG1) << "reserved:" << args[8] << "-";


	// send to server
	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_CONFIGURE_MAC));
		} else {
			// get detectormac, detector ip
			uint64_t idetectormac = 0;
			uint32_t idetectorip = 0;
			sscanf(retvals[1], "%lx", &idetectormac);
			sscanf(retvals[2], "%x", &idetectorip);
			sprintf(retvals[1],"%02x:%02x:%02x:%02x:%02x:%02x",
					(unsigned int)((idetectormac>>40)&0xFF),
					(unsigned int)((idetectormac>>32)&0xFF),
					(unsigned int)((idetectormac>>24)&0xFF),
					(unsigned int)((idetectormac>>16)&0xFF),
					(unsigned int)((idetectormac>>8)&0xFF),
					(unsigned int)((idetectormac>>0)&0xFF));
			sprintf(retvals[2],"%d.%d.%d.%d",
					(idetectorip>>24)&0xff,
					(idetectorip>>16)&0xff,
					(idetectorip>>8)&0xff,
					(idetectorip)&0xff);
			// update if different
			if (strcasecmp(retvals[1],thisDetector->detectorMAC)) {
				memset(thisDetector->detectorMAC, 0, MAX_STR_LENGTH);
				strcpy(thisDetector->detectorMAC, retvals[1]);
				FILE_LOG(logINFO) << detId << ": Detector MAC updated to " <<
						thisDetector->detectorMAC;
			}
			if (strcasecmp(retvals[2],thisDetector->detectorIP)) {
				memset(thisDetector->detectorIP, 0, MAX_STR_LENGTH);
				strcpy(thisDetector->detectorIP, retvals[2]);
				FILE_LOG(logINFO) << detId << ": Detector IP updated to " <<
						thisDetector->detectorIP;
			}
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return ret;
}


int64_t slsDetector::setTimer(timerIndex index, int64_t t) {
	int fnum = F_SET_TIMER;
	int ret = FAIL;
	int64_t args[2] = {(int64_t)index, t};
	int64_t retval = -1;
	FILE_LOG(logDEBUG1) << "Setting " << getTimerType(index) << " to " << t << " ns/value";

	// meausurement is only shm level
	if (index == MEASUREMENTS_NUMBER) {
		if (t >= 0) {
			thisDetector->timerValue[index] = t;
			FILE_LOG(logDEBUG1) << getTimerType(index) << ": " << t;
		}
		return thisDetector->timerValue[index];
	}

	// send to detector
	int64_t oldtimer = thisDetector->timerValue[index];
	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(DETECTOR_TIMER_VALUE_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << getTimerType(index) << ": " << retval;
			thisDetector->timerValue[index] = retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}

	// setting timers consequences (eiger (ratecorr) and jctb (databytes))
	// (a get can also change timer value, hence check difference)
	if (oldtimer != thisDetector->timerValue[index]) {
		// jctb: change samples, change databytes
		if (thisDetector->myDetectorType == JUNGFRAUCTB) {
			if (index == SAMPLES_JCTB) {
				setDynamicRange();
				FILE_LOG(logINFO) << "Changing samples: data size = " << thisDetector->dataBytes;
			}
		}
		// eiger: change exptime/subexptime, set rate correction to update table
		else if (thisDetector->myDetectorType == EIGER) {
			int dr = thisDetector->dynamicRange;
			if ((dr == 32 && index == SUBFRAME_ACQUISITION_TIME) ||
					(dr == 16 && index == ACQUISITION_TIME)) {
				int r = getRateCorrection();
				if (r)
					setRateCorrection(r);
			}
		}
	}

	// send to reciever
	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && ret == OK) {
		switch (index) {
		case FRAME_NUMBER:
		case FRAME_PERIOD:
		case CYCLES_NUMBER:
		case ACQUISITION_TIME:
		case SUBFRAME_ACQUISITION_TIME:
		case SUBFRAME_DEADTIME:
		case SAMPLES_JCTB:
		case STORAGE_CELL_NUMBER:
			// send
			fnum = F_SET_RECEIVER_TIMER;
			ret = FAIL;
			args[1] = thisDetector->timerValue[index];	// to the value given by detector
			retval = -1;

			// rewrite args
			if ((index == FRAME_NUMBER) || (index == CYCLES_NUMBER) || (index == STORAGE_CELL_NUMBER)) {
				args[1] = thisDetector->timerValue[FRAME_NUMBER] *
						((thisDetector->timerValue[CYCLES_NUMBER] > 0) ?
								(thisDetector->timerValue[CYCLES_NUMBER]) : 1) *
								((thisDetector->timerValue[STORAGE_CELL_NUMBER] > 0)
										? (thisDetector->timerValue[STORAGE_CELL_NUMBER])+1 : 1);
			}
			FILE_LOG(logDEBUG1) << "Sending " <<
					(((index == FRAME_NUMBER) ||
							(index == CYCLES_NUMBER) ||
							(index == STORAGE_CELL_NUMBER)) ?
									"(#Frames) * (#cycles) * (#storage cells)" : getTimerType(index)) <<
									 " to receiver: " << args[1];

			if (connectData() == OK) {
				char mess[MAX_STR_LENGTH] = {0};
				ret = thisReceiver->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval), mess);
				disconnectData();

				// handle ret
				if (ret == FAIL) {
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
					case FRAME_NUMBER:
					case CYCLES_NUMBER:
					case STORAGE_CELL_NUMBER:
						setErrorMask((getErrorMask())|(RECEIVER_FRAME_NUM_NOT_SET));
						break;
					default:
						setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
						break;
					}
				} else if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
			break;
		default:
			break;
		}
	}

	return thisDetector->timerValue[index];
}



int64_t slsDetector::getTimeLeft(timerIndex index) {
	int fnum = F_GET_TIME_LEFT;
	int ret = FAIL;
	int64_t retval = -1;
	FILE_LOG(logDEBUG1) << "Getting " << getTimerType(index) << " left";


	if (thisDetector->onlineFlag == ONLINE_FLAG && connectStop() == OK) {
		ret = thisDetectorStop->Client_Send(fnum, &index, sizeof(index), &retval, sizeof(retval));
		disconnectStop();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << getTimerType(index) << " left: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::setSpeed(speedVariable sp, int value) {
	int fnum = F_SET_SPEED;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting speed index " << sp << " to " << value;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &value, sizeof(value), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_SET_SPEED_PARAMETERS));
		} else {
			FILE_LOG(logDEBUG1) << "Speed index " << sp << ": " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::setDynamicRange(int n) {
	int fnum = F_SET_DYNAMIC_RANGE;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting dynamic range to " << n;

	// send to detector
	int olddr = thisDetector->dynamicRange;
	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		char mess[MAX_STR_LENGTH] = {0};
		ret = thisDetectorControl->Client_Send(fnum, &n, sizeof(n), &retval, sizeof(retval), mess);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			// eiger: dr set correctly, consequent rate correction was a problem
			if ((thisDetector->myDetectorType == EIGER &&
					strstr(mess,"Rate Correction") != NULL)) {
				// rate correction is switched off if not 32 bit mode
				if (strstr(mess,"32") != NULL)
					setErrorMask((getErrorMask())|(RATE_CORRECTION_NOT_32or16BIT));
				else
					setErrorMask((getErrorMask())|(COULD_NOT_SET_RATE_CORRECTION));
				ret = OK; // dr was set correctly to reach rate correction
			}
			else
				setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		}
		// ret might be set to OK above
		if (ret != FAIL) {
			FILE_LOG(logDEBUG1) << "Dynamic Range: " << retval;
			thisDetector->dynamicRange = retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}

	// setting dr consequences on databytes shm
	// (a get can also change timer value, hence check difference)
	if (olddr != thisDetector->dynamicRange) {
		thisDetector->dataBytes = thisDetector->nChips * thisDetector->nChans * retval / 8;
		thisDetector->dataBytesInclGapPixels =
				(thisDetector->nChip[X] * thisDetector->nChan[X] +
						thisDetector->gappixels * thisDetector->nGappixels[X]) *
						(thisDetector->nChip[Y] * thisDetector->nChan[Y] +
								thisDetector->gappixels * thisDetector->nGappixels[Y]) *
								retval / 8;
		if (thisDetector->myDetectorType == JUNGFRAUCTB)
			getTotalNumberOfChannels();
		FILE_LOG(logDEBUG1) << "Data bytes " << thisDetector->dataBytes;
		FILE_LOG(logDEBUG1) << "Data bytes including gap pixels" << thisDetector->dataBytesInclGapPixels;
	}

	// send to receiver
	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && ret == OK) {
		fnum = F_SET_RECEIVER_DYNAMIC_RANGE;
		ret = FAIL;
		n = thisDetector->dynamicRange;
		retval = -1;
		FILE_LOG(logINFO) << "Sending dynamic range to receiver " << n;
		if (connectData() == OK) {
			ret=thisReceiver->Client_Send(fnum, &n, sizeof(n), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_DYNAMIC_RANGE));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver Dynamic range: " << retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
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


int slsDetector::setDAC(int val, dacIndex index, int mV) {
	if ((index == HV_NEW) && (thisDetector->myDetectorType == GOTTHARD))
		index = HV_POT;

	int fnum = F_SET_DAC;
	int ret = FAIL;
	int args[3] = {(int)index, mV, val};
	int retvals[2] = {-1, -1};
	FILE_LOG(logDEBUG1) << "Setting DAC " << index << " to " <<	val << (mV ? "mV" : "dac units");

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
		disconnectControl();

		// handle ret
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		else {
			FILE_LOG(logDEBUG1) << "Dac index " << index << ": "
					<< retvals[0] << " dac units (" << retvals[1] << "mV)";
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	if (mV)
		return retvals[1];
	return retvals[0];
}


int slsDetector::getADC(dacIndex index) {
	int fnum = F_GET_ADC;
	int ret = FAIL;
	int arg = (int)index;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Getting ADC " << index;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		else
			FILE_LOG(logDEBUG1) << "ADC (" << index << "): " << retval;
		/*commented out to allow adc read during acquire
			 else if (ret == FORCE_UPDATE)
				updateDetector();*/
	}
	return retval;
}


slsDetectorDefs::externalCommunicationMode slsDetector::setExternalCommunicationMode(
		externalCommunicationMode pol) {
	int fnum = F_SET_EXTERNAL_COMMUNICATION_MODE;
	int ret = FAIL;
	int arg = (int)pol;
	externalCommunicationMode retval = GET_EXTERNAL_COMMUNICATION_MODE;
	FILE_LOG(logDEBUG1) << "Setting communication to mode " << pol;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Timing Mode: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


slsDetectorDefs::externalSignalFlag slsDetector::setExternalSignalFlags(
		externalSignalFlag pol, int signalindex) {
	int fnum = F_SET_EXTERNAL_SIGNAL_FLAG;
	int ret = FAIL;
	int args[2] = {signalindex, pol};
	externalSignalFlag retval = GET_EXTERNAL_SIGNAL_FLAG;
	FILE_LOG(logDEBUG1) << "Setting signal " << signalindex <<  " to flag " << pol;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Ext Signal (" << signalindex << "): " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::setReadOutFlags(readOutFlags flag) {
	int fnum = F_SET_READOUT_FLAGS;
	int ret = FAIL;
	int arg = (int)flag;
	readOutFlags retval = GET_READOUT_FLAGS;
	FILE_LOG(logDEBUG1) << "Setting readout flags to " << flag;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_SET_READOUT_FLAGS));
		} else {
			FILE_LOG(logDEBUG1) << "Readout flag: " << retval;
			thisDetector->roFlags = (readOutFlags)retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return thisDetector->roFlags;
}


uint32_t slsDetector::writeRegister(uint32_t addr, uint32_t val) {
	int fnum = F_WRITE_REGISTER;
	int ret = FAIL;
	uint32_t args[2] = {addr, val};
	uint32_t retval = -1;
	FILE_LOG(logDEBUG1) << "Writing to register 0x" << std::hex <<
			addr <<  "data: 0x" << std::hex << val << std::dec;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
		else {
			FILE_LOG(logDEBUG1) << "Register 0x" <<
					std::hex << addr << ": 0x" << std::hex << retval << std::dec;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


uint32_t slsDetector::readRegister(uint32_t addr) {
	int fnum = F_READ_REGISTER;
	int ret = FAIL;
	uint32_t arg = -1;
	uint32_t retval = -1;
	FILE_LOG(logDEBUG1) << "Reading register 0x" << std::hex <<	addr << std::dec;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
		else {
			FILE_LOG(logDEBUG1) << "Register 0x" <<
					std::hex << addr << ": 0x" << std::hex << retval << std::dec;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}




uint32_t slsDetector::setBit(uint32_t addr, int n) {
	if (n < 0 || n > 31) {
		FILE_LOG(logERROR) << "Bit number " <<  n << " out of Range";
		setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
		return -1;
	}
	else {
		uint32_t val = readRegister(addr);
		if (getErrorMask() | REGISER_WRITE_READ)
			return -1;
		return writeRegister(addr, val | 1 << n);
	}
}


uint32_t slsDetector::clearBit(uint32_t addr, int n) {
	if (n < 0 || n > 31) {
		FILE_LOG(logERROR) << "Bit number " <<  n << " out of Range";
		setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
		return -1;
	}
	else {
		uint32_t val = readRegister(addr);
		if (getErrorMask() | REGISER_WRITE_READ)
			return -1;
		return writeRegister(addr, val & ~(1 << n));
	}
}


std::string slsDetector::setNetworkParameter(networkParameter index, std::string value) {
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
		setReceiverUDPPort(stoi(value));
		return getReceiverUDPPort();
	case RECEIVER_UDP_PORT2:
		if (thisDetector->myDetectorType == EIGER) {
			setReceiverUDPPort2(stoi(value));
			return getReceiverUDPPort2();
		} else {
			setReceiverUDPPort(stoi(value));
			return getReceiverUDPPort();
		}
	case DETECTOR_TXN_DELAY_LEFT:
	case DETECTOR_TXN_DELAY_RIGHT:
	case DETECTOR_TXN_DELAY_FRAME:
	case FLOW_CONTROL_10G:
		return setDetectorNetworkParameter(index, stoi(value));
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
		setReceiverUDPSocketBufferSize(stoi(value));
		return getReceiverUDPSocketBufferSize();
	default:
		return (char*)("unknown network parameter");
	}
}


std::string slsDetector::getNetworkParameter(networkParameter index) {
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
		return std::string("unknown network parameter");
	}

}


std::string slsDetector::getDetectorMAC() {
	return std::string(thisDetector->detectorMAC);
}


std::string slsDetector::getDetectorIP() {
	return std::string(thisDetector->detectorIP);
}


std::string slsDetector::getReceiver() {
	return std::string(thisDetector->receiver_hostname);
}


std::string slsDetector::getReceiverUDPIP() {
	return std::string(thisDetector->receiverUDPIP);
}


std::string slsDetector::getReceiverUDPMAC() {
	return std::string(thisDetector->receiverUDPMAC);
}


std::string slsDetector::getReceiverUDPPort() {
	return std::to_string(thisDetector->receiverUDPPort);
}


std::string slsDetector::getReceiverUDPPort2() {
	return std::to_string(thisDetector->receiverUDPPort2);
}


std::string slsDetector::getClientStreamingPort() {
	return std::to_string(thisDetector->zmqport);
}


std::string slsDetector::getReceiverStreamingPort() {
	return std::to_string(thisDetector->receiver_zmqport);
}


std::string slsDetector::getClientStreamingIP() {
	return std::string(thisDetector->zmqip);
}


std::string slsDetector::getReceiverStreamingIP() {
	return std::string(thisDetector->receiver_zmqip);
}


std::string slsDetector::getAdditionalJsonHeader() {
	return std::string(thisDetector->receiver_additionalJsonHeader);
}


std::string slsDetector::getReceiverUDPSocketBufferSize() {
	return setReceiverUDPSocketBufferSize();
}


std::string slsDetector::getReceiverRealUDPSocketBufferSize() {
	int fnum = F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Getting real UDP Socket Buffer size to receiver";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		} else {
			FILE_LOG(logDEBUG1) << "Real Receiver UDP Socket Buffer size: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return std::to_string(retval);
}


std::string slsDetector::setDetectorMAC(std::string detectorMAC) {

	// invalid format
	if ((detectorMAC.length() != 17) ||
			(detectorMAC[2] == ':') || (detectorMAC[5] == ':') || (detectorMAC[8] == ':') ||
			(detectorMAC[11] == ':') ||(detectorMAC[14] == ':')) {
		setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		FILE_LOG(logERROR) << "server MAC Address should be in xx:xx:xx:xx:xx:xx format";
	}
	// valid format
	else {
		strcpy(thisDetector->detectorMAC, detectorMAC.c_str());
		if (!strcmp(thisDetector->receiver_hostname, "none")) {
			FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
		} else if (setUDPConnection() == FAIL) {
			FILE_LOG(logWARNING) << "UDP connection set up failed";
		}
	}
	return std::string(thisDetector->detectorMAC);
}


std::string slsDetector::setDetectorIP(std::string detectorIP) {
	struct sockaddr_in sa;
	if (detectorIP.length() && detectorIP.length() < 16) {
		int result = inet_pton(AF_INET, detectorIP.c_str(), &(sa.sin_addr));
		// invalid format
		if (result == 0) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			FILE_LOG(logERROR) << "Detector IP Address should be VALID and in "
					"xxx.xxx.xxx.xxx format";
		}
		// valid format
		else {
			strcpy(thisDetector->detectorIP, detectorIP.c_str());
			if (!strcmp(thisDetector->receiver_hostname, "none")) {
				FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
			} else if (setUDPConnection() == FAIL) {
				FILE_LOG(logWARNING) << "UDP connection set up failed";
			}
		}
	}
	return std::string(thisDetector->detectorIP);
}


std::string slsDetector::setReceiver(std::string receiverIP) {
	FILE_LOG(logDEBUG1) << "Setting up Receiver with " << receiverIP;
	// recieverIP is none
	if (receiverIP == "none") {
		memset(thisDetector->receiver_hostname, 0, MAX_STR_LENGTH);
		strcpy(thisDetector->receiver_hostname, "none");
		thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
		return std::string(thisDetector->receiver_hostname);
	}
	// stop acquisition if running
	if (getRunStatus()==RUNNING) {
		FILE_LOG(logWARNING) << "Acquisition already running, Stopping it.";
		stopAcquisition();
	}
	// update detector before receiver
	updateDetector();

	// start updating
	strcpy(thisDetector->receiver_hostname, receiverIP.c_str());

	if (setReceiverOnline(ONLINE_FLAG)==ONLINE_FLAG) {
		FILE_LOG(logDEBUG1) <<
				"detector type:" << (slsDetectorDefs::getDetectorType(thisDetector->myDetectorType)) <<
				"\ndetector id:" << detId <<
				"\ndetector hostname:" << thisDetector->hostname <<
				"\nfile path:" << thisDetector->receiver_filePath <<
				"\nfile name:" << thisDetector->receiver_fileName <<
				"\nfile index:" << thisDetector->receiver_fileIndex <<
				"\nfile format:" << thisDetector->receiver_fileFormatType <<
				"\nr_framesperfile:" << thisDetector->receiver_framesPerFile <<
				"\nr_discardpolicy:" << thisDetector->receiver_frameDiscardMode <<
				"\nr_padding:" << thisDetector->receiver_framePadding <<
				"\nwrite enable:" << thisDetector->receiver_fileWriteEnable <<
				"\noverwrite enable:" << thisDetector->receiver_overWriteEnable <<
				"\nframe index needed:" <<
				((thisDetector->timerValue[FRAME_NUMBER] * thisDetector->timerValue[CYCLES_NUMBER]) > 1) <<
				"\nframe period:" << (thisDetector->timerValue[FRAME_PERIOD]) <<
				"\nframe number:" << (thisDetector->timerValue[FRAME_NUMBER]) <<
				"\nsub exp time:" << (thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME]) <<
				"\nsub dead time:" << (thisDetector->timerValue[SUBFRAME_DEADTIME]) <<
				"\nsamples:" << (thisDetector->timerValue[SAMPLES_JCTB]) <<
				"\ndynamic range:" << thisDetector->dynamicRange <<
				"\nflippeddatax:" << (thisDetector->flippedData[X]) <<
				"\nactivated: " << thisDetector->activated <<
				"\nreceiver deactivated padding: " << thisDetector->receiver_deactivatedPaddingEnable <<
				"\nsilent Mode:" << thisDetector->receiver_silentMode <<
				"\n10GbE:" << thisDetector->tenGigaEnable	<<
				"\nGap pixels: " << thisDetector->gappixels	<<
				"\nr_readfreq:" << thisDetector->receiver_read_freq	<<
				"\nrx streaming port:" << thisDetector->receiver_zmqport <<
				"\nrx streaming source ip:" << thisDetector->receiver_zmqip	<<
				"\nrx additional json header:" << thisDetector->receiver_additionalJsonHeader <<
				"\nrx_datastream:" << enableDataStreamingFromReceiver(-1) << std::endl;

		if (setDetectorType(thisDetector->myDetectorType) != GENERIC) {
			sendMultiDetectorSize();
			setDetectorId();
			setDetectorHostname();
			setUDPConnection();
			setFilePath(thisDetector->receiver_filePath);
			setFileName(thisDetector->receiver_fileName);
			setFileIndex(thisDetector->receiver_fileIndex);
			setFileFormat(thisDetector->receiver_fileFormatType);
			setReceiverFramesPerFile(thisDetector->receiver_framesPerFile);
			setReceiverFramesDiscardPolicy(thisDetector->receiver_frameDiscardMode);
			setReceiverPartialFramesPadding(thisDetector->receiver_framePadding);
			enableWriteToFile(thisDetector->receiver_fileWriteEnable);
			overwriteFile(thisDetector->receiver_overWriteEnable);
			setTimer(FRAME_PERIOD,thisDetector->timerValue[FRAME_PERIOD]);
			setTimer(FRAME_NUMBER,thisDetector->timerValue[FRAME_NUMBER]);
			setTimer(ACQUISITION_TIME,thisDetector->timerValue[ACQUISITION_TIME]);
			if (thisDetector->myDetectorType == EIGER) {
				setTimer(SUBFRAME_ACQUISITION_TIME,
						thisDetector->timerValue[SUBFRAME_ACQUISITION_TIME]);
				setTimer(SUBFRAME_DEADTIME,thisDetector->timerValue[SUBFRAME_DEADTIME]);
			}
			if (thisDetector->myDetectorType == JUNGFRAUCTB)
				setTimer(SAMPLES_JCTB,thisDetector->timerValue[SAMPLES_JCTB]);
			setDynamicRange(thisDetector->dynamicRange);
			if (thisDetector->myDetectorType == EIGER) {
				setFlippedData(X,-1);
				activate(-1);
				setDeactivatedRxrPaddingMode(thisDetector->receiver_deactivatedPaddingEnable);
			}
			setReceiverSilentMode(thisDetector->receiver_silentMode);
			if (thisDetector->myDetectorType == EIGER)
				enableTenGigabitEthernet(thisDetector->tenGigaEnable);
			enableGapPixels(thisDetector->gappixels);
			// data streaming
			setReceiverStreamingFrequency(thisDetector->receiver_read_freq);
			setReceiverStreamingPort(getReceiverStreamingPort());
			setReceiverStreamingIP(getReceiverStreamingIP());
			setAdditionalJsonHeader(getAdditionalJsonHeader());
			enableDataStreamingFromReceiver(enableDataStreamingFromReceiver(-1));
			if (thisDetector->myDetectorType == GOTTHARD)
				sendROI(-1, NULL);
		}
	}
	return std::string(thisDetector->receiver_hostname);
}


std::string slsDetector::setReceiverUDPIP(std::string udpip) {
	struct sockaddr_in sa;
	if (udpip.length() && udpip.length() < 16) {
		int result = inet_pton(AF_INET, udpip.c_str(), &(sa.sin_addr));
		// invalid format
		if (result == 0) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			FILE_LOG(logERROR) << "Receiver UDP IP Address should be VALID "
					"and in xxx.xxx.xxx.xxx format" << std::endl;
		}
		// valid format
		else {
			strcpy(thisDetector->receiverUDPIP,udpip.c_str());
			if (!strcmp(thisDetector->receiver_hostname, "none")) {
				FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
			} else if (setUDPConnection() == FAIL) {
				FILE_LOG(logWARNING) << "UDP connection set up failed";
			}
		}
	}
	return std::string(thisDetector->receiverUDPIP);
}


std::string slsDetector::setReceiverUDPMAC(std::string udpmac) {
	// invalid format
	if ((udpmac.length() != 17) ||
			(udpmac[2] == ':') || (udpmac[5] == ':') || (udpmac[8] == ':') ||
			(udpmac[11] == ':') ||(udpmac[14] == ':')) {
		setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		FILE_LOG(logERROR) << "receiver udp MAC Address should be in xx:xx:xx:xx:xx:xx format";
	}
	// valid format
	else {
		strcpy(thisDetector->receiverUDPMAC, udpmac.c_str());
		if (!strcmp(thisDetector->receiver_hostname, "none")) {
			FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
		}
		// not doing setUDPConnection as rx_udpmac will get replaced,(must use configuremac)
		strcpy(thisDetector->receiverUDPMAC,udpmac.c_str());
	}
	return std::string(thisDetector->receiverUDPMAC);
}


int slsDetector::setReceiverUDPPort(int udpport) {
	thisDetector->receiverUDPPort = udpport;
	if (!strcmp(thisDetector->receiver_hostname, "none")) {
		FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
	}
	else if (setUDPConnection() == FAIL) {
		FILE_LOG(logWARNING) << "UDP connection set up failed";
	}
	return thisDetector->receiverUDPPort;
}


int slsDetector::setReceiverUDPPort2(int udpport) {
	thisDetector->receiverUDPPort2 = udpport;
	if (!strcmp(thisDetector->receiver_hostname, "none")) {
		FILE_LOG(logDEBUG1) << "Receiver hostname not set yet";
	}
	else if (setUDPConnection() == FAIL) {
		FILE_LOG(logWARNING) << "UDP connection set up failed";
	}
	return thisDetector->receiverUDPPort2;
}


std::string slsDetector::setClientStreamingPort(std::string port) {
	thisDetector->zmqport = stoi(port);
	return getClientStreamingPort();
}


std::string slsDetector::setReceiverStreamingPort(std::string port) {
	// copy now else it is lost if rx_hostname not set yet
	thisDetector->receiver_zmqport = stoi(port);

	int fnum = F_SET_RECEIVER_STREAMING_PORT;
	int ret = FAIL;
	int arg = thisDetector->receiver_zmqport;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending receiver streaming port to receiver " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver streaming port: " << retval;
			thisDetector->receiver_zmqport = retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return getReceiverStreamingPort();
}


std::string slsDetector::setClientStreamingIP(std::string sourceIP) {
	struct addrinfo *result;
	// on failure to convert to a valid ip
	if (dataSocket->ConvertHostnameToInternetAddress(sourceIP.c_str(), &result)) {
		FILE_LOG(logWARNING) << "Could not convert zmqip into a valid IP" << sourceIP;
		setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		return getClientStreamingIP();
	}
	// on success put IP as std::string into arg
	memset(thisDetector->zmqip, 0, MAX_STR_LENGTH);
	dataSocket->ConvertInternetAddresstoIpString(result, thisDetector->zmqip, MAX_STR_LENGTH);
	return getClientStreamingIP();
}


std::string slsDetector::setReceiverStreamingIP(std::string sourceIP) {
	int fnum = F_RECEIVER_STREAMING_SRC_IP;
	int ret = FAIL;
	char args[MAX_STR_LENGTH] = {0};
	char retvals[MAX_STR_LENGTH] = {0};
	FILE_LOG(logDEBUG1) << "Sending receiver streaming IP to receiver " << sourceIP;

	// if empty, give rx_hostname
	if (sourceIP.empty())  {
		if (!strcmp(thisDetector->receiver_hostname, "none")) {
			FILE_LOG(logWARNING) << "Receiver hostname not set yet. Cannot create rx_zmqip from none";
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
			FILE_LOG(logWARNING) << "Could not convert rx_zmqip into a valid IP" << sourceIP;
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
			return getReceiverStreamingIP();
		}
		// on success put IP as std::string into arg
		dataSocket->ConvertInternetAddresstoIpString(result, args, sizeof(args));
	}

	// set it anyway, else it is lost if rx_hostname is not set yet
	memset(thisDetector->receiver_zmqip, 0, MAX_STR_LENGTH);
	strcpy(thisDetector->receiver_zmqip, args);
	// if zmqip is empty, update it
	if (! strlen(thisDetector->zmqip))
		strcpy(thisDetector->zmqip, args);
	FILE_LOG(logDEBUG1) << "Sending receiver streaming IP to receiver " << args;

	// send to receiver
	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver streaming port: " << retvals;
			memset(thisDetector->receiver_zmqip, 0, MAX_STR_LENGTH);
			strcpy(thisDetector->receiver_zmqip, retvals);
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return getReceiverStreamingIP();
}


std::string slsDetector::setAdditionalJsonHeader(std::string jsonheader) {
	int fnum = F_ADDITIONAL_JSON_HEADER;
	int ret = FAIL;
	char args[MAX_STR_LENGTH] = {0};
	char retvals[MAX_STR_LENGTH] = {0};
	strcpy(args, jsonheader.c_str());
	FILE_LOG(logDEBUG1) << "Sending additional json header " << args;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		} else {
			FILE_LOG(logDEBUG1) << "Additional json header: " << retvals;
			memset(thisDetector->receiver_additionalJsonHeader, 0, MAX_STR_LENGTH);
			strcpy(thisDetector->receiver_additionalJsonHeader, retvals);
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return getAdditionalJsonHeader();
}


std::string slsDetector::setReceiverUDPSocketBufferSize(int udpsockbufsize) {
	int fnum = F_RECEIVER_UDP_SOCK_BUF_SIZE;
	int ret = FAIL;
	int arg = udpsockbufsize;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending UDP Socket Buffer size to receiver " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_SET_NETWORK_PARAMETER));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver UDP Socket Buffer size: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return std::to_string(retval);
}


std::string slsDetector::setDetectorNetworkParameter(networkParameter index, int delay) {
	int fnum = F_SET_NETWORK_PARAMETER;
	int ret = FAIL;
	int args[2] = {(int)index, delay};
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting network parameter index " << index << " to " <<  delay;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(DETECTOR_NETWORK_PARAMETER));
		} else {
			FILE_LOG(logDEBUG1) << "Network Parameter (" << index << "): " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return std::to_string(retval);
}


int slsDetector::setUDPConnection() {
	int fnum = F_SETUP_RECEIVER_UDP;
	int ret = FAIL;
	char args[3][MAX_STR_LENGTH] = {0};
	char retvals[MAX_STR_LENGTH] = {0};
	FILE_LOG(logDEBUG1) << "Setting UDP Connection";

	// called before set up
	if (!strcmp(thisDetector->receiver_hostname, "none")) {
		FILE_LOG(logDEBUG1) << "Receiver hostname not set yet.";
		return FAIL;
	}
	// if no udp ip given, use hostname
	if (!strcmp(thisDetector->receiverUDPIP, "none")) {
		// hostname is an ip address
		if (strchr(thisDetector->receiver_hostname,'.') != NULL)
			strcpy(thisDetector->receiverUDPIP, thisDetector->receiver_hostname);
		// if hostname not ip, convert it to ip
		else {
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
	FILE_LOG(logDEBUG1) << "Receiver udp ip address: " << thisDetector->receiverUDPIP;
	FILE_LOG(logDEBUG1) << "Receiver udp port: " << thisDetector->receiverUDPPort;
	FILE_LOG(logDEBUG1) << "Receiver udp port2: " << thisDetector->receiverUDPPort2;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver UDP MAC returned : " << retvals;
			memset(thisDetector->receiverUDPMAC, 0, MAX_STR_LENGTH);
			strcpy(thisDetector->receiverUDPMAC, retvals);
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();

			//configure detector with udp details
			if (configureMAC() == FAIL) {
				setReceiverOnline(OFFLINE_FLAG); //FIXME: Needed??
			}
		}
	}
	printReceiverConfiguration(logDEBUG1);
	return ret;
}


int slsDetector::digitalTest( digitalTestMode mode, int ival) {
	int fnum = F_DIGITAL_TEST;
	int ret = FAIL;
	int args[2] = {(int)mode, ival};
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending digital test of mode " << mode << ", ival " << ival;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Digital Test returned: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::loadImageToDetector(imageType index,std::string const fname) {
	int ret = FAIL;
	short int args[thisDetector->nChans * thisDetector->nChips];
	FILE_LOG(logDEBUG1) << "Loading " << (!index ? "Dark" : "Gain") << "image from file " << fname;

	if (readDataFile(fname, args, getTotalNumberOfChannels())) {
		ret = sendImageToDetector(index,args);
		return ret;
	}

	setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
	FILE_LOG(logERROR) << "Could not open file " << fname;
	return ret;
}


int slsDetector::sendImageToDetector(imageType index,short int imageVals[]) {
	int fnum = F_LOAD_IMAGE;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Sending image to detector";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum,
				imageVals, thisDetector->dataBytes, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::writeCounterBlockFile(std::string const fname,int startACQ) {
	int ret = FAIL;
	short int retvals[thisDetector->nChans * thisDetector->nChips];
	FILE_LOG(logDEBUG1) << "Reading Counter to " << fname <<
			(startACQ ? " and Restarting Acquisition" : "\n");

	ret = getCounterBlock(retvals, startACQ);
	if (ret == FAIL)
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
	else
		ret=writeDataFile(fname, getTotalNumberOfChannels(), retvals);
	return ret;
}


int slsDetector::getCounterBlock(short int image[],int startACQ) {
	int fnum = F_READ_COUNTER_BLOCK;
	int ret = FAIL;
	int arg = startACQ;
	FILE_LOG(logDEBUG1) << "Reading Counter block with startacq: " << startACQ;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum,
				&arg, sizeof(arg), image, thisDetector->dataBytes);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::resetCounterBlock(int startACQ) {
	int fnum = F_RESET_COUNTER_BLOCK;
	int ret = FAIL;
	int arg = startACQ;
	FILE_LOG(logDEBUG1) << "Resetting Counter with startacq: " << startACQ;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::setCounterBit(int i) {
	int fnum = F_SET_COUNTER_BIT;
	int ret = FAIL;
	int arg = i;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending counter bit " << arg;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_SET_COUNTER_BIT));
		} else {
			FILE_LOG(logDEBUG1) << "Counter bit: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::setROI(int n,ROI roiLimits[]) {
	// sort ascending order
	for (int i = 0; i < n; ++i) {
		for (int j = i + 1; j < n; ++j) {
			if (roiLimits[j].xmin < roiLimits[i].xmin) {
				int temp = roiLimits[i].xmin;
				roiLimits[i].xmin = roiLimits[j].xmin;
				roiLimits[j].xmin = temp;
				temp = roiLimits[i].xmax;
				roiLimits[i].xmax = roiLimits[j].xmax;
				roiLimits[j].xmax = temp;
				temp = roiLimits[i].ymin;
				roiLimits[i].ymin = roiLimits[j].ymin;
				roiLimits[j].ymin = temp;
				temp = roiLimits[i].ymax;
				roiLimits[i].ymax = roiLimits[j].ymax;
				roiLimits[j].ymax = temp;
			}
		}
	}

	int ret = sendROI(n,roiLimits);
	if (thisDetector->myDetectorType == JUNGFRAUCTB)
		getTotalNumberOfChannels();
	return ret;
}


slsDetectorDefs::ROI* slsDetector::getROI(int &n) {
	sendROI(-1,NULL);
	n = thisDetector->nROI;
	if (thisDetector->myDetectorType == JUNGFRAUCTB)
		getTotalNumberOfChannels();
	return thisDetector->roiLimits;
}


int slsDetector::getNRoi() {
	return thisDetector->nROI;
}


int slsDetector::sendROI(int n, ROI roiLimits[]) {
	int fnum = F_SET_ROI;
	int ret = FAIL;
	int narg = n;
	// send roiLimits if given, else from shm
	ROI* arg = (roiLimits != NULL) ? roiLimits : thisDetector->roiLimits;
	int nretval = 0;
	ROI retval[MAX_ROIS];
	FILE_LOG(logDEBUG1) << "Sending ROI to detector" << narg;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		controlSocket->SendDataOnly(&fnum, sizeof(fnum));
		controlSocket->SendDataOnly(&narg, sizeof(narg));
		if (narg != -1) {
			for(int i = 0; i < narg; ++i) {
				controlSocket->SendDataOnly(&arg[i].xmin, sizeof(int));
				controlSocket->SendDataOnly(&arg[i].xmax, sizeof(int));
				controlSocket->SendDataOnly(&arg[i].ymin, sizeof(int));
				controlSocket->SendDataOnly(&arg[i].ymax, sizeof(int));
			}
		}
		controlSocket->ReceiveDataOnly(&ret, sizeof(ret));

		// handle ret
		if (ret == FAIL) {
			char mess[MAX_STR_LENGTH] = {0};
			setErrorMask((getErrorMask())|(COULDNOT_SET_ROI));
			controlSocket->ReceiveDataOnly(mess, MAX_STR_LENGTH);
			FILE_LOG(logERROR) << "Detector " << detId << " returned error: " << mess;
		} else {
			controlSocket->ReceiveDataOnly(&nretval, sizeof(nretval));
			int nrec = 0;
			for(int i = 0; i < nretval; ++i) {
				nrec += controlSocket->ReceiveDataOnly(&retval[i].xmin, sizeof(int));
				nrec += controlSocket->ReceiveDataOnly(&retval[i].xmax, sizeof(int));
				nrec += controlSocket->ReceiveDataOnly(&retval[i].ymin, sizeof(int));
				nrec += controlSocket->ReceiveDataOnly(&retval[i].ymax, sizeof(int));
			}
			thisDetector->nROI = nretval;
			FILE_LOG(logDEBUG1) << "nRoi: " << nretval;
			for (int i = 0; i < nretval; ++i) {
				thisDetector->roiLimits[i] = retval[i];
				FILE_LOG(logDEBUG1) << "ROI [" << i << "] (" <<
						thisDetector->roiLimits[i].xmin << "," <<
						thisDetector->roiLimits[i].xmax << "," <<
						thisDetector->roiLimits[i].ymin << "," <<
						thisDetector->roiLimits[i].ymax << ")";
			}

		}
		disconnectControl();
		if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}

	// old firmware requires configuremac after setting roi
	if (thisDetector->myDetectorType == GOTTHARD) {
		ret = configureMAC();
	}

	// update roi in receiver
	if (ret == OK && thisDetector->receiverOnlineFlag == ONLINE_FLAG) {
		fnum = F_RECEIVER_SET_ROI;
		ret = FAIL;
		narg = thisDetector->nROI;
		arg = thisDetector->roiLimits;
		FILE_LOG(logDEBUG1) << "Sending ROI to receiver " << thisDetector->nROI;

		if (connectData() == OK) {
			dataSocket->SendDataOnly(&fnum, sizeof(fnum));
			dataSocket->SendDataOnly(&narg, sizeof(narg));
			if (narg != -1) {
				for(int i = 0; i < narg; ++i) {
					dataSocket->SendDataOnly(&arg[i].xmin, sizeof(int));
					dataSocket->SendDataOnly(&arg[i].xmax, sizeof(int));
					dataSocket->SendDataOnly(&arg[i].ymin, sizeof(int));
					dataSocket->SendDataOnly(&arg[i].ymax, sizeof(int));
				}
			}
			dataSocket->ReceiveDataOnly(&ret, sizeof(ret));

			// handle ret
			char mess[MAX_STR_LENGTH] = {0};
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(COULDNOT_SET_ROI));
				dataSocket->ReceiveDataOnly(mess, MAX_STR_LENGTH);
				FILE_LOG(logERROR) << "Receiver " << detId << " returned error: " << mess;
			}
			disconnectData();
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return ret;
}


int slsDetector::writeAdcRegister(int addr, int val) {
	int fnum = F_WRITE_ADC_REG;
	int ret = FAIL;
	uint32_t args[2] = {(uint32_t)addr, (uint32_t)val};
	FILE_LOG(logDEBUG1) << "Writing to ADC register 0x" << std::hex <<
			addr <<  "data: 0x" << std::hex << val << std::dec;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(REGISER_WRITE_READ));
		else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::activate(int const enable) {
	int fnum = F_ACTIVATE;
	int ret = FAIL;
	int arg = enable;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting activate flag to " << arg;

	// detector
	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(DETECTOR_ACTIVATE));
		} else {
			FILE_LOG(logDEBUG1) << "Activate: " << retval;
			thisDetector->activated = retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}

	// receiver
	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && ret == OK) {
		fnum = F_RECEIVER_ACTIVATE;
		ret = FAIL;
		arg = thisDetector->activated;
		retval = -1;
		FILE_LOG(logDEBUG1) << "Setting activate flag " << arg << " to receiver";

		if (connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_ACTIVATE));
			} else if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return thisDetector->activated;
}


int slsDetector::setDeactivatedRxrPaddingMode(int padding) {
	int fnum = F_RECEIVER_DEACTIVATED_PADDING_ENABLE;
	int ret = OK;
	int arg = padding;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable: " << arg;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG && connectData() == OK) {
		ret=thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_ACTIVATE));
		else {
			FILE_LOG(logDEBUG1) << "Deactivated Receiver Padding Enable:" << retval;
			thisDetector->receiver_deactivatedPaddingEnable = retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return thisDetector->receiver_deactivatedPaddingEnable;
}


int slsDetector::getFlippedData(dimension d) {
	return thisDetector->flippedData[d];
}


int slsDetector::setFlippedData(dimension d, int value) {
	int fnum = F_SET_FLIPPED_DATA_RECEIVER;
	int ret = OK;
	int args[2] = {(int)d, value};
	int retval = -1;

	// flipped across y
	if (d == Y) {
		FILE_LOG(logERROR) << "Flipped across Y axis is not implemented";
		setErrorMask((getErrorMask())|(RECEIVER_FLIPPED_DATA_NOT_SET));
		return -1;
	}

	// replace get with shm value (write to shm right away as it is a det value, not rx value)
	if (value > -1)
		thisDetector->flippedData[d] = (value > 0) ? 1 : 0;
	args[1] = thisDetector->flippedData[d];
	FILE_LOG(logDEBUG1) << "Setting flipped data across axis " << d << " with value: " << value;

	if (thisDetector->receiverOnlineFlag==ONLINE_FLAG && connectData() == OK) {
		ret=thisReceiver->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(RECEIVER_FLIPPED_DATA_NOT_SET));
		else {
			FILE_LOG(logDEBUG1) << "Flipped data:" << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return thisDetector->flippedData[d];
}


int slsDetector::setAllTrimbits(int val) {
	int fnum = F_SET_ALL_TRIMBITS;
	int ret = FAIL;
	int arg = val;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting all trimbits to " << val;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(ALLTIMBITS_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "All trimbit value: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::enableGapPixels(int val) {
	if (val >= 0) {
		int fnum = F_ENABLE_GAPPIXELS_IN_RECEIVER;
		int ret = OK;
		int arg = val;
		int retval = -1;
		FILE_LOG(logDEBUG1) << "Sending gap pixels enable to receiver: " << arg;

		if (thisDetector->receiverOnlineFlag==ONLINE_FLAG && connectData() == OK) {
			ret=thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL)
				setErrorMask((getErrorMask())|(RECEIVER_ENABLE_GAPPIXELS_NOT_SET));
			else {
				FILE_LOG(logDEBUG1) << "Gap pixels enable to receiver:" << retval;
				thisDetector->gappixels = retval;

				// update databytes
				thisDetector->dataBytesInclGapPixels = 0;
				if (thisDetector->dynamicRange != 4) {
					thisDetector->dataBytesInclGapPixels =
							(thisDetector->nChip[X] * thisDetector->nChan[X] +
									thisDetector->gappixels * thisDetector->nGappixels[X]) *
									(thisDetector->nChip[Y] * thisDetector->nChan[Y] +
											thisDetector->gappixels * thisDetector->nGappixels[Y]) *
											thisDetector->dynamicRange / 8;
				}
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->gappixels;
}


int slsDetector::setTrimEn(int nen, int *en) {
	if (en) {
		for (int ien = 0; ien < nen; ++ien)
			thisDetector->trimEnergies[ien]=en[ien];
		thisDetector->nTrimEn=nen;
	}
	return (thisDetector->nTrimEn);
}


int slsDetector::getTrimEn(int *en) {
	if (en) {
		for (int ien=0; ien<thisDetector->nTrimEn; ++ien)
			en[ien] = thisDetector->trimEnergies[ien];
	}
	return (thisDetector->nTrimEn);
}


int slsDetector::pulsePixel(int n, int x, int y) {
	int fnum = F_PULSE_PIXEL;
	int ret = FAIL;
	int args[3] = {n , x, y};
	FILE_LOG(logDEBUG1) << "Pulsing pixel " << n << " number of times at (" <<
			x << "," << y << ")";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_PULSE));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::pulsePixelNMove(int n, int x, int y) {
	int fnum = F_PULSE_PIXEL_AND_MOVE;
	int ret = FAIL;
	int args[3] = {n, x, y};
	FILE_LOG(logDEBUG1) << "Pulsing pixel " << n << " number of times and move by delta (" <<
			x << "," << y << ")";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_PULSE));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::pulseChip(int n) {
	int fnum = F_PULSE_CHIP;
	int ret = FAIL;
	int arg = n;
	FILE_LOG(logDEBUG1) << "Pulsing chip " << n << " number of times";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_PULSE));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::setThresholdTemperature(int val) {
	int fnum = F_THRESHOLD_TEMP;
	int ret = FAIL;
	int arg = val;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting threshold temperature to " << val;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectStop() == OK) {
		ret = thisDetectorStop->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectStop();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(TEMPERATURE_CONTROL));
		} else {
			FILE_LOG(logDEBUG1) << "Threshold temperature: " << retval;
		} // no updateDetector as it is stop server
	}
	return retval;
}


int slsDetector::setTemperatureControl(int val) {
	int fnum = F_TEMP_CONTROL;
	int ret = FAIL;
	int arg = val;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting temperature control to " << val;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectStop() == OK) {
		ret = thisDetectorStop->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectStop();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(TEMPERATURE_CONTROL));
		} else {
			FILE_LOG(logDEBUG1) << "Temperature control: " << retval;
		} // no updateDetector as it is stop server
	}
	return retval;
}


int slsDetector::setTemperatureEvent(int val) {
	int fnum = F_TEMP_EVENT;
	int ret = FAIL;
	int arg = val;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting temperature event to " << val;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectStop() == OK) {
		ret = thisDetectorStop->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectStop();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(TEMPERATURE_CONTROL));
		} else {
			FILE_LOG(logDEBUG1) << "Temperature event: " << retval;
		} // no updateDetector as it is stop server
	}
	return retval;
}


int slsDetector::setStoragecellStart(int pos) {
	int fnum = F_STORAGE_CELL_START;
	int ret = FAIL;
	int arg = pos;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting storage cell start to " << pos;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(STORAGE_CELL_START));
		} else {
			FILE_LOG(logDEBUG1) << "Storage cell start: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::programFPGA(std::string fname) {
	// only jungfrau implemented (client processing, so check now)
	if (thisDetector->myDetectorType != JUNGFRAU &&	thisDetector->myDetectorType != JUNGFRAUCTB) {
		FILE_LOG(logERROR) << "Not implemented for this detector";
		setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
		return FAIL;
	}
	FILE_LOG(logDEBUG1) << "Programming FPGA with file name:" << fname;
	size_t filesize = 0;
	char* fpgasrc = NULL;

	//check if it exists
	{
		struct stat st;
		if (stat(fname.c_str(),&st)) {
			FILE_LOG(logERROR) << "Programming file does not exist";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}
	}

	{
		// open src
		FILE* src = fopen(fname.c_str(),"rb");
		if (src == NULL) {
			FILE_LOG(logERROR) << "Could not open source file for programming: " << fname;
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}

		// create temp destination file
		char destfname[] = "/tmp/Jungfrau_MCB.XXXXXX";
		int dst = mkstemp(destfname); // create temporary file and open it in r/w
		if (dst == -1) {
			FILE_LOG(logERROR) << "Could not create destination file in /tmp for programming: "	<< destfname;
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}

		// convert src to dst rawbin
		FILE_LOG(logDEBUG1) << "Converting " << fname << " to " <<  destfname;
		{
			int filepos, x, y, i;
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
			for (filepos = 0x80; filepos < 0x1000000; ++filepos)	{
				x = fgetc(src);
				if (x < 0) break;
				y=0;
				for (i = 0; i < 8; ++i)
					y=y| (   (( x & (1<<i) ) >> i)    << (7-i)     );	// This swaps the bits
				write(dst, &y, 1);
			}
			if (filepos < 0x1000000) {
				FILE_LOG(logERROR) << "Could not convert programming file. EOF before end of flash";
				setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				return FAIL;
			}
		}
		if (fclose(src)) {
			FILE_LOG(logERROR) << "Could not close source file";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}
		if (close(dst)) {
			FILE_LOG(logERROR) << "Could not close destination file";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}
		FILE_LOG(logDEBUG1) << "File has been converted to "  <<  destfname;

		//loading dst file to memory
		FILE* fp = fopen(destfname,"r");
		if (fp == NULL) {
			FILE_LOG(logERROR) << "Could not open rawbin file";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}
		if (fseek(fp, 0, SEEK_END)) {
			FILE_LOG(logERROR) << "Seek error in rawbin file";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}
		filesize = ftell(fp);
		if (filesize <= 0) {
			FILE_LOG(logERROR) << "Could not get length of rawbin file";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}
		rewind(fp);
		fpgasrc = (char*)malloc(filesize + 1);
		if (fpgasrc == NULL) {
			FILE_LOG(logERROR) << "Could not allocate size of program";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			return FAIL;
		}
		if (fread(fpgasrc, sizeof(char), filesize, fp) != filesize) {
			FILE_LOG(logERROR) << "Could not read rawbin file";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			if (fpgasrc != NULL)
				free(fpgasrc);
			return FAIL;
		}

		if (fclose(fp)) {
			FILE_LOG(logERROR) << "Could not close destination file after converting";
			setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
			if (fpgasrc != NULL)
				free(fpgasrc);
			return FAIL;
		}
		unlink(destfname); // delete temporary file
		FILE_LOG(logDEBUG1) << "Successfully loaded the rawbin file to program memory";
	}

	// send program from memory to detector
	int fnum = F_PROGRAM_FPGA;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = {0};
	FILE_LOG(logDEBUG1) << "Sending programming binary to detector";

	if (thisDetector->onlineFlag == ONLINE_FLAG) {
		if (connectControl() == OK) {
			controlSocket->SendDataOnly(&fnum, sizeof(fnum));
			controlSocket->SendDataOnly(&filesize, sizeof(filesize));
			controlSocket->ReceiveDataOnly(&ret,sizeof(ret));
			// opening error
			if (ret == FAIL) {
				controlSocket->ReceiveDataOnly(mess, sizeof(mess));
				FILE_LOG(logERROR) << "Detector " << detId << " returned error: " << mess;
				setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				filesize = 0;
			}


			//erasing flash
			if (ret != FAIL) {
				FILE_LOG(logINFO) << "This can take awhile. Please be patient...";
				printf("Erasing Flash: %d%%\r", 0);
				std::cout << std::flush;
				//erasing takes 65 seconds, printing here (otherwise need threads
				//in server-unnecessary)
				const int ERASE_TIME = 65;
				int count = ERASE_TIME + 1;
				while(count > 0) {
					usleep(1 * 1000 * 1000);
					--count;
					printf("Erasing Flash:%d%%\r",
							(int) (((double)(ERASE_TIME - count) / ERASE_TIME) * 100));
					std::cout << std::flush;
				}
				std::cout << std::endl;
				printf("Writing to Flash:%d%%\r", 0);
				std::cout << std::flush;
			}


			//sending program in parts of 2mb each
			size_t unitprogramsize = 0;
			int currentPointer = 0;
			size_t totalsize = filesize;
			while(ret != FAIL && (filesize > 0)) {

				unitprogramsize = MAX_FPGAPROGRAMSIZE;  //2mb
				if (unitprogramsize > filesize) //less than 2mb
					unitprogramsize = filesize;
				FILE_LOG(logDEBUG1) << "unitprogramsize:" << unitprogramsize <<
						"\t filesize:" << filesize;

				controlSocket->SendDataOnly(fpgasrc + currentPointer, unitprogramsize);
				controlSocket->ReceiveDataOnly(&ret, sizeof(ret));
				if (ret != FAIL) {
					filesize -= unitprogramsize;
					currentPointer += unitprogramsize;

					//print progress
					printf("Writing to Flash:%d%%\r",
							(int) (((double)(totalsize - filesize) / totalsize) * 100));
					std::cout << std::flush;
				} else {
					controlSocket->ReceiveDataOnly(mess,sizeof(mess));
					FILE_LOG(logERROR) << "Detector returned error: " << mess;
					setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				}
			}
			std::cout << std::endl;

			//check ending error
			if ((ret == FAIL) &&
					(strstr(mess,"not implemented") == NULL) &&
					(strstr(mess,"locked") == NULL) &&
					(strstr(mess,"-update") == NULL)) {
				controlSocket->ReceiveDataOnly(&ret, sizeof(ret));
				if (ret == FAIL) {
					controlSocket->ReceiveDataOnly(mess, sizeof(mess));
					FILE_LOG(logERROR) << "Detector returned error: " << mess;
					setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				}
			}
			disconnectControl();
			if (ret == FORCE_UPDATE)
				updateDetector();
		}


		//remapping stop server
		if ((ret == FAIL) &&
				(strstr(mess,"not implemented") == NULL) &&
				(strstr(mess,"locked") == NULL) &&
				(strstr(mess,"-update") == NULL)) {
			fnum = F_RESET_FPGA;
			int stopret = FAIL;
			if (connectStop() == OK) {
				stopSocket->SendDataOnly(&fnum, sizeof(fnum));
				stopSocket->ReceiveDataOnly(&stopret, sizeof(stopret));
				if (stopret==FAIL) {
					controlSocket->ReceiveDataOnly(mess, sizeof(mess));
					FILE_LOG(logERROR) << "Detector returned error: " << mess;
					setErrorMask((getErrorMask())|(PROGRAMMING_ERROR));
				}
				disconnectStop();
			}
		}
	}
	if (ret != FAIL) {
		FILE_LOG(logINFO) << "You can now restart the detector servers in normal mode.";
	}

	//free resources
	if (fpgasrc != NULL)
		free(fpgasrc);

	return ret;
}


int slsDetector::resetFPGA() {
	int fnum = F_RESET_FPGA;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Sending reset FPGA";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RESET_ERROR));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int slsDetector::powerChip(int ival) {
	int fnum = F_POWER_CHIP;
	int ret = FAIL;
	int arg = ival;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting power chip to " << ival;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(POWER_CHIP));
		} else {
			FILE_LOG(logDEBUG1) << "Power chip: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::setAutoComparatorDisableMode(int ival) {
	int fnum = F_AUTO_COMP_DISABLE;
	int ret = FAIL;
	int arg = ival;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting auto comp disable mode to " << ival;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(AUTO_COMP_DISABLE));
		} else {
			FILE_LOG(logDEBUG1) << "Auto comp disable: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}



int slsDetector::getChanRegs(double* retval) {
	int n = getTotalNumberOfChannels();
	// update chanregs
	sls_detector_module *myMod = getModule();
	deleteModule(myMod);
	//the original array has 0 initialized
	if (chanregs) {
		for (int i = 0; i < n; ++i)
			retval[i] = (double) (chanregs[i] & TRIMBITMASK);
	}
	return n;
}


int slsDetector::setModule(sls_detector_module module, int tb) {
	int fnum = F_SET_MODULE;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting module with tb:" << tb;

	//to exclude trimbits
	if (!tb) {
		module.nchan = 0;
		module.nchip = 0;
	}

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		controlSocket->SendDataOnly(&fnum, sizeof(fnum));
		sendModule(&module);
		controlSocket->ReceiveDataOnly(&ret,sizeof(ret));

		// handle ret
		if (ret == FAIL) {
			char mess[MAX_STR_LENGTH] = {0};
			controlSocket->ReceiveDataOnly(mess,sizeof(mess));
			FILE_LOG(logERROR) << "Detector " << detId << " returned error: " << mess;
			if (strstr(mess,"default tau") != NULL)
				setErrorMask((getErrorMask())|(RATE_CORRECTION_NO_TAU_PROVIDED));
			else
				setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		}
		controlSocket->ReceiveDataOnly(&retval,sizeof(retval));
		FILE_LOG(logDEBUG1) << "Set Module returned: " << retval;

		disconnectControl();
		if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}

	// update client structure
	if (ret == OK) {
		if (detectorModules) {
			if (thisDetector->myDetectorType == EIGER && tb && chanregs) {
				for (int ichip = 0; ichip < thisDetector->nChips; ++ichip) {
					for (int i = 0; i < thisDetector->nChans; ++i) {
						chanregs[i + ichip * thisDetector->nChans] =
								module.chanregs[ichip * thisDetector->nChans + i];
					}
				}
			}
			if (adcs) {
				for (int i = 0; i < thisDetector->nAdcs; ++i) {
					adcs[i] = module.adcs[i];
				}
				if (dacs) {
					for (int i = 0; i < thisDetector->nDacs; ++i)
						dacs[i] = module.dacs[i];
				}
				(detectorModules)->serialnumber = module.serialnumber;
				(detectorModules)->reg = module.reg;
				(detectorModules)->iodelay = module.iodelay;
				(detectorModules)->tau = module.tau;
				(detectorModules)->eV = module.eV;
			}
		}
		if (module.eV != -1) {
			thisDetector->currentThresholdEV = module.eV;
		}
	}
	return ret;
}


slsDetectorDefs::sls_detector_module  *slsDetector::getModule() {
	int fnum = F_GET_MODULE;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Getting module";

	sls_detector_module *myMod = createModule();
	if (myMod == NULL) {
		FILE_LOG(logERROR) << "Could not create module";
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		return NULL;

	}

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum,NULL, 0, NULL, 0);

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			receiveModule(myMod);
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
		disconnectControl();
	}

	// update client structure
	if (ret == OK) {
		if (detectorModules) {
			if (thisDetector->myDetectorType == EIGER && chanregs) {
				for (int ichip = 0; ichip < thisDetector->nChips; ++ichip) {
					for (int i = 0; i < thisDetector->nChans; ++i) {
						chanregs[i+ichip*thisDetector->nChans] =
								myMod->chanregs[ichip*thisDetector->nChans+i];
					}
				}
			}
			if (adcs) {
				for (int i = 0; i < thisDetector->nAdcs; ++i)
					adcs[i] = myMod->adcs[i];
			}
			if (dacs) {
				for (int i = 0; i < thisDetector->nDacs; ++i) {
					dacs[i] = myMod->dacs[i];
				}
			}
			(detectorModules)->serialnumber = myMod->serialnumber;
			(detectorModules)->reg = myMod->reg;
			(detectorModules)->iodelay = myMod->iodelay;
			(detectorModules)->tau = myMod->tau;
			(detectorModules)->eV = myMod->eV;
		}
	} else {
		deleteModule(myMod);
		myMod = NULL;
	}
	return myMod;
}


int slsDetector::setRateCorrection(int64_t t) {
	int fnum = F_SET_RATE_CORRECT;
	int ret = FAIL;
	int64_t arg = t;
	FILE_LOG(logDEBUG1) << "Setting Rate Correction to " << arg;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		char mess[MAX_STR_LENGTH] = {0};
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), NULL, 0, mess);
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			if (strstr(mess,"default tau")!=NULL)
				setErrorMask((getErrorMask())|(RATE_CORRECTION_NO_TAU_PROVIDED));
			if (strstr(mess,"32")!=NULL)
				setErrorMask((getErrorMask())|(RATE_CORRECTION_NOT_32or16BIT));
			else
				setErrorMask((getErrorMask())|(COULD_NOT_SET_RATE_CORRECTION));
		} else if (ret == FORCE_UPDATE)
			ret = updateDetector();
	}
	return ret;
}


int64_t slsDetector::getRateCorrection() {
	int fnum = F_GET_RATE_CORRECT;
	int ret = FAIL;
	int64_t retval = -1;
	FILE_LOG(logDEBUG1) << "Getting rate correction";

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Rate correction: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


void slsDetector::printReceiverConfiguration(TLogLevel level) {
	FILE_LOG(level) << "#Detector " << detId << ":\n" <<
			"Receiver Hostname:\t" << getNetworkParameter(RECEIVER_HOSTNAME) <<
			"\nDetector UDP IP (Source):\t\t" << getNetworkParameter(DETECTOR_IP) <<
			"\nDetector UDP MAC:\t\t" << getNetworkParameter(DETECTOR_MAC) <<
			"\nReceiver UDP IP:\t" << getNetworkParameter(RECEIVER_UDP_IP) <<
			"\nReceiver UDP MAC:\t" << getNetworkParameter(RECEIVER_UDP_MAC) <<
			"\nReceiver UDP Port:\t" << getNetworkParameter(RECEIVER_UDP_PORT) <<
			"\nReceiver UDP Port2:\t" <<  getNetworkParameter(RECEIVER_UDP_PORT2);
}


int slsDetector::setReceiverOnline(int off) {
	if (off != GET_ONLINE_FLAG) {
		// no receiver
		if (!strcmp(thisDetector->receiver_hostname, "none"))
			thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
		else
			thisDetector->receiverOnlineFlag = off;
		// set online
		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG) {
			setReceiverTCPSocket();
			// error in connecting
			if (thisDetector->receiverOnlineFlag == OFFLINE_FLAG) {
				FILE_LOG(logERROR) << "Cannot connect to receiver";
				setErrorMask((getErrorMask())|(CANNOT_CONNECT_TO_RECEIVER));
			}
		}
	}
	return thisDetector->receiverOnlineFlag;
}


std::string slsDetector::checkReceiverOnline() {
	std::string retval;
	//if it doesnt exit, create socket and call this function again
	if (!dataSocket) {
		setReceiverTCPSocket();
		if (thisDetector->receiverOnlineFlag == OFFLINE_FLAG)
			return std::string(thisDetector->receiver_hostname);
		else
			return retval;
	}
	//still cannot connect to socket, dataSocket=0
	if (dataSocket) {
		if (connectData() == FAIL) {
			dataSocket->SetTimeOut(5);
			thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
			delete dataSocket;
			dataSocket = 0;
			FILE_LOG(logDEBUG1) << "receiver " << detId << " offline!";
			return std::string(thisDetector->receiver_hostname);
		}  else {
			thisDetector->receiverOnlineFlag = ONLINE_FLAG;
			dataSocket->SetTimeOut(100);
			disconnectData();
			FILE_LOG(logDEBUG1) << "receiver " << detId << " online!";
		}
	}
	return retval;
}


int slsDetector::setReceiverTCPSocket(std::string const name, int const receiver_port) {
	char thisName[MAX_STR_LENGTH] = {0};
	int thisRP = 0;
	int ret = OK;

	// rx_hostname
	if (name.empty()) {
		if (!strcmp(thisDetector->receiver_hostname, "none")) {
			FILE_LOG(logDEBUG1) << "No rx_hostname given yet";
			thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
			return FAIL;
		}
		strcpy(thisName,thisDetector->receiver_hostname);
	} else {
		FILE_LOG(logDEBUG1) << "Setting rx_hostname";
		strcpy(thisName, name.c_str());
		strcpy(thisDetector->receiver_hostname, thisName);
		if (dataSocket) {
			delete dataSocket;
			dataSocket = 0;
		}
	}

	// data port
	if (receiver_port <= 0) {
		thisRP = thisDetector->receiverTCPPort;
	} else {
		FILE_LOG(logDEBUG1) << "Setting data port";
		thisRP = receiver_port;
		thisDetector->receiverTCPPort = thisRP;
		if (dataSocket) {
			delete dataSocket;
			dataSocket = 0;
		}
	}

	// create data socket
	if (!dataSocket) {
		try {
			dataSocket = new MySocketTCP(thisName, thisRP);
			FILE_LOG(logDEBUG1) << "Data socket connected " << thisName  << " " << thisRP;
		} catch(...) {
			FILE_LOG(logERROR) << "Could not connect Data socket " << thisName  << " " << thisRP;
			dataSocket = 0;
			ret = FAIL;
		}
	}

	if (ret == FAIL) {
		thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
		FILE_LOG(logDEBUG1) << "Receiver offline";
	}

	// check online and version compatibility
	else {
		checkReceiverOnline();
		thisReceiver->SetSocket(dataSocket);
		// check for version compatibility
		if (thisDetector->receiverAPIVersion == 0) {
			if (checkVersionCompatibility(DATA_PORT) == FAIL)
				thisDetector->receiverOnlineFlag = OFFLINE_FLAG;
		}
	}
	return ret;
}


int slsDetector::lockReceiver(int lock) {
	int fnum = F_LOCK_RECEIVER;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting receiver server lock to " << lock;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &lock, sizeof(lock), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver Lock: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return retval;
}


std::string slsDetector::getReceiverLastClientIP() {
	int fnum = F_GET_LAST_RECEIVER_CLIENT_IP;
	int ret = FAIL;
	char retval[INET_ADDRSTRLEN] = {0};
	FILE_LOG(logDEBUG1) << "Getting last client ip to receiver server";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Last client IP to receiver: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return std::string(retval);
}


int slsDetector::exitReceiver() {
	int fnum = F_EXIT_RECEIVER;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Sending exit command to receiver server";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectData();
		// no ret handling as ret never fail
		FILE_LOG(logINFO) << "Shutting down the receiver server";
	}
	return ret;
}


int slsDetector::execReceiverCommand(std::string cmd) {
	int fnum = F_EXEC_RECEIVER_COMMAND;
	int ret = FAIL;
	char arg[MAX_STR_LENGTH] = {0};
	char retval[MAX_STR_LENGTH] = {0};
	strcpy(arg, cmd.c_str());
	FILE_LOG(logDEBUG1) << "Sending command to receiver " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, arg, sizeof(arg), retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logINFO) << "Receiver " << detId << " returned:\n" << retval;
		}
	}
	return ret;
}


int slsDetector::updateReceiverNoWait() {

	int n = 0, i32 = 0;
	char cstring[MAX_STR_LENGTH] = {0};
	char lastClientIP[INET_ADDRSTRLEN] = {0};

	n += dataSocket->ReceiveDataOnly(lastClientIP, sizeof(lastClientIP));
	FILE_LOG(logDEBUG1) << "Updating receiver last modified by " << lastClientIP;

	// filepath
	n += dataSocket->ReceiveDataOnly(cstring, sizeof(cstring));
	strcpy(thisDetector->receiver_filePath, cstring);

	// filename
	n += dataSocket->ReceiveDataOnly(cstring, sizeof(cstring));
	strcpy(thisDetector->receiver_fileName, cstring);

	// index
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_fileIndex = i32;

	//file format
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_fileFormatType = (fileFormat)i32;

	// frames per file
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_framesPerFile = i32;

	// frame discard policy
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_frameDiscardMode = (frameDiscardPolicy)i32;

	// frame padding
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_framePadding = i32;

	// file write enable
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_fileWriteEnable = i32;

	// file overwrite enable
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_overWriteEnable = i32;

	// gap pixels
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->gappixels = i32;

	// receiver read frequency
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_read_freq = i32;

	// receiver streaming port
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_zmqport = i32;

	// streaming source ip
	n += dataSocket->ReceiveDataOnly(cstring, sizeof(cstring));
	strcpy(thisDetector->receiver_zmqip, cstring);

	// additional json header
	n += dataSocket->ReceiveDataOnly(cstring, sizeof(cstring));
	strcpy(thisDetector->receiver_additionalJsonHeader, cstring);

	// receiver streaming enable
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_upstream = i32;

	// activate
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->activated = i32;

	// deactivated padding enable
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_deactivatedPaddingEnable = i32;

	// silent mode
	n += dataSocket->ReceiveDataOnly(&i32, sizeof(i32));
	thisDetector->receiver_silentMode = i32;

	if (!n) {
		FILE_LOG(logERROR) << "Could not update receiver, received 0 bytes\n";
	}
	return OK;
}





int slsDetector::updateReceiver() {
	int fnum = F_UPDATE_RECEIVER_CLIENT;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Sending update client to receiver server";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, NULL, 0);
		if (ret == FAIL)
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		else
			updateReceiverNoWait();
		disconnectData();
	}
	return ret;
}


void slsDetector::sendMultiDetectorSize() {
	int fnum = F_SEND_RECEIVER_MULTIDETSIZE;
	int ret = FAIL;
	int args[2] = {thisDetector->multiSize[0], thisDetector->multiSize[1]};
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending multi detector size to Receiver (" <<
			thisDetector->multiSize[0] << "," << thisDetector->multiSize[1] << ")";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RECEIVER_MULTI_DET_SIZE_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver multi size returned: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
}


void slsDetector::setDetectorId() {
	int fnum = F_SEND_RECEIVER_DETPOSID;
	int ret = FAIL;
	int arg = detId;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending detector pos id to Receiver " << detId;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RECEIVER_DET_POSID_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver Position Id returned: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
}


void slsDetector::setDetectorHostname() {
	int fnum = F_SEND_RECEIVER_DETHOSTNAME;
	int ret = FAIL;
	char args[MAX_STR_LENGTH] = {0};
	char retvals[MAX_STR_LENGTH] = {0};
	strcpy(args, thisDetector->hostname);
	FILE_LOG(logDEBUG1) << "Sending detector hostname to Receiver " << args;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RECEIVER_DET_HOSTNAME_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver set detector hostname: " << retvals;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
}


std::string slsDetector::getFilePath() {
	return thisDetector->receiver_filePath;
}


std::string slsDetector::setFilePath(std::string s) {
	if (!s.empty()) {
		int fnum = F_SET_RECEIVER_FILE_PATH;
		int ret = FAIL;
		char args[MAX_STR_LENGTH] = {0};
		char retvals[MAX_STR_LENGTH] = {0};
		strcpy(args, s.c_str());
		FILE_LOG(logDEBUG1) << "Sending file path to Receiver " << args;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				if (!s.empty()) {
					FILE_LOG(logERROR) << "file path does not exist";
					setErrorMask((getErrorMask())|(FILE_PATH_DOES_NOT_EXIST));
				} else
					setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver file path: " << retvals;
				strcpy(thisDetector->receiver_filePath, retvals);
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_filePath;
}


std::string slsDetector::getFileName() {
	return thisDetector->receiver_fileName;
}


std::string slsDetector::setFileName(std::string s) {
	if (!s.empty()) {
		int fnum = F_SET_RECEIVER_FILE_NAME;
		int ret = FAIL;
		char args[MAX_STR_LENGTH] = {0};
		char retvals[MAX_STR_LENGTH] = {0};
		strcpy(args, s.c_str());
		FILE_LOG(logDEBUG1) << "Sending file name to Receiver " << args;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver file name: " << retvals;
				strcpy(thisDetector->receiver_fileName, retvals);
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_fileName;
}


int slsDetector::setReceiverFramesPerFile(int f) {
	if (f >= 0) {
		int fnum = F_SET_RECEIVER_FRAMES_PER_FILE;
		int ret = FAIL;
		int arg = f;
		int retval = -1;
		FILE_LOG(logDEBUG1) << "Setting receiver frames per file to " << arg;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver frames per file: " << retval;
				thisDetector->receiver_framesPerFile = retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_framesPerFile;
}


slsDetectorDefs::frameDiscardPolicy slsDetector::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
	int fnum = F_RECEIVER_DISCARD_POLICY;
	int ret = FAIL;
	int arg = (int)f;
	frameDiscardPolicy retval = (frameDiscardPolicy)-1;
	FILE_LOG(logDEBUG1) << "Setting receiver frames discard policy to " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver frames discard policy: " << retval;
			thisDetector->receiver_frameDiscardMode = retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}

	return thisDetector->receiver_frameDiscardMode;
}


int slsDetector::setReceiverPartialFramesPadding(int f) {
	int fnum = F_RECEIVER_PADDING_ENABLE;
	int ret = FAIL;
	int arg = f;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Setting receiver partial frames enable to " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver partial frames enable: " << retval;
			thisDetector->receiver_framePadding = retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}

	return thisDetector->receiver_framePadding;
}


slsDetectorDefs::fileFormat slsDetector::setFileFormat(fileFormat f) {
	if (f != GET_FILE_FORMAT) {
		int fnum = F_SET_RECEIVER_FILE_FORMAT;
		int ret = FAIL;
		int arg = f;
		fileFormat retval = (fileFormat)-1;
		FILE_LOG(logDEBUG1) << "Setting receiver file format to " << arg;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver file format: " << retval;
				thisDetector->receiver_fileFormatType = retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return getFileFormat();
}


slsDetectorDefs::fileFormat slsDetector::getFileFormat() {
	return thisDetector->receiver_fileFormatType;
}


int slsDetector::getFileIndex() {
	return thisDetector->receiver_fileIndex;
}


int slsDetector::setFileIndex(int i) {
	if (i >= 0) {
		int fnum = F_SET_RECEIVER_FILE_INDEX;
		int ret = FAIL;
		int arg = i;
		int retval = -1;
		FILE_LOG(logDEBUG1) << "Setting file index to " << arg;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver file index: " << retval;
				thisDetector->receiver_fileIndex = retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_fileIndex;
}


int slsDetector::incrementFileIndex() {
	if (thisDetector->receiver_fileWriteEnable)
		return setFileIndex(thisDetector->receiver_fileIndex + 1);
	return thisDetector->receiver_fileIndex;
}


int slsDetector::startReceiver() {
	int fnum = F_START_RECEIVER;
	int ret = FAIL;
	char mess[MAX_STR_LENGTH] = {0};
	FILE_LOG(logDEBUG1) << "Starting Receiver";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, NULL, 0, mess);
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			if (strstr(mess,"UDP")!=NULL)
				setErrorMask((getErrorMask())|(COULDNOT_CREATE_UDP_SOCKET));
			else if (strstr(mess,"file")!=NULL)
				setErrorMask((getErrorMask())|(COULDNOT_CREATE_FILE));
			else
				setErrorMask((getErrorMask())|(COULDNOT_START_RECEIVER));
		} else if (ret == FORCE_UPDATE)
			ret = updateReceiver();
	}
	return ret;
}


int slsDetector::stopReceiver() {
	int fnum = F_STOP_RECEIVER;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Stopping Receiver";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULDNOT_STOP_RECEIVER));
		} else if (ret == FORCE_UPDATE)
			ret = updateReceiver();
	}
	return ret;
}


slsDetectorDefs::runStatus slsDetector::getReceiverStatus() {
	int fnum = F_GET_RECEIVER_STATUS;
	int ret = FAIL;
	runStatus retval = ERROR;
	FILE_LOG(logDEBUG1) << "Getting Receiver Status";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver Status: " << runStatusType(retval);
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return retval;
}


int slsDetector::getFramesCaughtByReceiver() {
	int fnum = F_GET_RECEIVER_FRAMES_CAUGHT;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Getting Frames Caught by Receiver";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Frames Caught by Receiver: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return retval;
}


int slsDetector::getReceiverCurrentFrameIndex() {
	int fnum = F_GET_RECEIVER_FRAME_INDEX;
	int ret = FAIL;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Getting Current Frame Index of Receiver";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Current Frame Index of Receiver: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return retval;
}


int slsDetector::resetFramesCaught() {
	int fnum = F_RESET_RECEIVER_FRAMES_CAUGHT;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Reset Frames Caught by Receiver";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else if (ret == FORCE_UPDATE)
			ret = updateReceiver();
	}
	return ret;
}


int slsDetector::enableWriteToFile(int enable) {
	if (enable >= 0) {
		int fnum = F_ENABLE_RECEIVER_FILE_WRITE;
		int ret = FAIL;
		int arg = enable;
		int retval = -1;
		FILE_LOG(logDEBUG1) << "Sending enable file write to receiver: " << arg;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver file write enable: " << retval;
				thisDetector->receiver_fileWriteEnable = retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_fileWriteEnable;
}


int slsDetector::overwriteFile(int enable) {
	if (enable >= 0) {
		int fnum = F_ENABLE_RECEIVER_OVERWRITE;
		int ret = FAIL;
		int arg = enable;
		int retval = -1;
		FILE_LOG(logDEBUG1) << "Sending enable file overwrite to receiver: " << arg;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver file overwrite enable: " << retval;
				thisDetector->receiver_overWriteEnable = retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_overWriteEnable;
}


int slsDetector::setReceiverStreamingFrequency(int freq) {
	if (freq >= 0) {
		int fnum = F_RECEIVER_STREAMING_FREQUENCY;
		int ret = FAIL;
		int arg = freq;
		int retval = -1;
		FILE_LOG(logDEBUG1) << "Sending read frequency to receiver: " << arg;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_STREAMING_FREQUENCY));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver read frequency: " << retval;
				thisDetector->receiver_read_freq = retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_read_freq;
}



int slsDetector::setReceiverStreamingTimer(int time_in_ms) {
	int fnum = F_RECEIVER_STREAMING_TIMER;
	int ret = FAIL;
	int arg = time_in_ms;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending read timer to receiver: " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RECEIVER_STREAMING_TIMER));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver read timer: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return retval;
}


int slsDetector::enableDataStreamingFromReceiver(int enable) {
	if (enable >= 0) {
		int fnum = F_STREAM_DATA_FROM_RECEIVER;
		int ret = FAIL;
		int arg = enable;
		int retval = -1;
		FILE_LOG(logDEBUG1) << "Sending Data Streaming to receiver: " << arg;

		if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(DATA_STREAMING));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver Data Streaming: " << retval;
				thisDetector->receiver_upstream = retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->receiver_upstream;
}


int slsDetector::enableTenGigabitEthernet(int i) {
	int fnum = F_ENABLE_TEN_GIGA;
	int ret = FAIL;
	int arg = i;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Enabling / Disabling 10Gbe: " << arg;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(DETECTOR_TEN_GIGA));
		} else {
			FILE_LOG(logDEBUG1) << "10Gbe: " << retval;
			thisDetector->tenGigaEnable = retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
			// have to configure mac after setting 10GbE
			ret = configureMAC();
		}
	}


	// receiver
	if ((thisDetector->receiverOnlineFlag == ONLINE_FLAG) && ret == OK) {
		fnum = F_ENABLE_RECEIVER_TEN_GIGA;
		ret = FAIL;
		arg = thisDetector->tenGigaEnable;
		retval = -1;
		FILE_LOG(logINFO) << "Sending 10Gbe enable to Receiver " << arg;

		if (connectData() == OK) {
			ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
			disconnectData();

			// handle ret
			if (ret == FAIL) {
				setErrorMask((getErrorMask())|(RECEIVER_TEN_GIGA));
			} else {
				FILE_LOG(logDEBUG1) << "Receiver 10Gbe enable: " << retval;
				if (ret == FORCE_UPDATE)
					ret = updateReceiver();
			}
		}
	}
	return thisDetector->tenGigaEnable;
}


int slsDetector::setReceiverFifoDepth(int i) {
	int fnum = F_SET_RECEIVER_FIFO_DEPTH;
	int ret = FAIL;
	int arg = i;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending Receiver Fifo Depth: " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(COULD_NOT_SET_FIFO_DEPTH));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver Fifo Depth: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return retval;
}



int slsDetector::setReceiverSilentMode(int i) {
	int fnum = F_SET_RECEIVER_SILENT_MODE;
	int ret = FAIL;
	int arg = i;
	int retval = -1;
	FILE_LOG(logDEBUG1) << "Sending Receiver Silent Mode: " << arg;

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, &arg, sizeof(arg), &retval, sizeof(retval));
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RECEIVER_PARAMETER_NOT_SET));
		} else {
			FILE_LOG(logDEBUG1) << "Receiver Data Streaming: " << retval;
			thisDetector->receiver_silentMode = retval;
			if (ret == FORCE_UPDATE)
				ret = updateReceiver();
		}
	}
	return thisDetector->receiver_silentMode;
}




int slsDetector::restreamStopFromReceiver() {
	int fnum = F_RESTREAM_STOP_FROM_RECEIVER;
	int ret = FAIL;
	FILE_LOG(logDEBUG1) << "Restream stop dummy from Receiver via zmq";

	if (thisDetector->receiverOnlineFlag == ONLINE_FLAG && connectData() == OK) {
		ret = thisReceiver->Client_Send(fnum, NULL, 0, NULL, 0);
		disconnectData();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(RESTREAM_STOP_FROM_RECEIVER));
		} else if (ret == FORCE_UPDATE)
			ret = updateReceiver();
	}
	return ret;
}



int slsDetector::setCTBPattern(std::string fname) {
	uint64_t word;
	int addr = 0;
	FILE *fd = fopen(fname.c_str(),"r");
	if (fd > 0) {
		while (fread(&word, sizeof(word), 1, fd)) {
			setCTBWord(addr, word);
			++addr;
		}
		fclose(fd);
	} else
		return -1;
	return addr;
}


uint64_t slsDetector::setCTBWord(int addr, uint64_t word) {
	int fnum = F_SET_CTB_PATTERN;
	int ret = FAIL;
	int mode = 0; // sets word
	uint64_t args[3] =  {(uint64_t) mode, (uint64_t)addr, word};
	uint64_t retval = -1;
	FILE_LOG(logDEBUG1) << "Setting CTB word, addr: 0x" << std::hex << addr << ", word: 0x" << word << std::dec;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Set CTB word: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::setCTBPatLoops(int level,int &start, int &stop, int &n) {
	int fnum = F_SET_CTB_PATTERN;
	int ret = FAIL;
	int mode = 1; // sets loop
	uint64_t args[5] =  {(uint64_t) mode, (uint64_t)level, (uint64_t)start, (uint64_t)stop, (uint64_t)n};
	uint64_t retvals[3] = {0, 0, 0};
	FILE_LOG(logDEBUG1) << "Setting CTB Pat Loops, "
			"level: " << level << ", start: " << start << ", stop: " << stop << ", n: " << n;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), retvals, sizeof(retvals));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Set CTB Pat Loops: " <<
					retvals[0] << ", " << retvals[1] << ", " << retvals[2];
			start = retvals[0];
			stop = retvals[1];
			n = retvals[2];
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return ret;
}


int slsDetector::setCTBPatWaitAddr(int level, int addr) {
	int fnum = F_SET_CTB_PATTERN;
	int ret = FAIL;
	int mode = 2; // sets loop
	uint64_t args[3] =  {(uint64_t) mode, (uint64_t)level, (uint64_t)addr};
	uint64_t retval = -1;
	FILE_LOG(logDEBUG1) << "Setting CTB Wait Addr, "
			"level: " << level << ", addr: 0x" << std::hex << addr << std::dec;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Set CTB Wait Addr: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}


int slsDetector::setCTBPatWaitTime(int level, uint64_t t) {
	int fnum = F_SET_CTB_PATTERN;
	int ret = FAIL;
	int mode = 3; // sets loop
	uint64_t args[3] =  {(uint64_t) mode, (uint64_t)level, t};
	uint64_t retval = -1;
	FILE_LOG(logDEBUG1) << "Setting CTB Wait Time, level: " << level << ", t: " << t;

	if (thisDetector->onlineFlag == ONLINE_FLAG && connectControl() == OK) {
		ret = thisDetectorControl->Client_Send(fnum, args, sizeof(args), &retval, sizeof(retval));
		disconnectControl();

		// handle ret
		if (ret == FAIL) {
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		} else {
			FILE_LOG(logDEBUG1) << "Set CTB Wait Time: " << retval;
			if (ret == FORCE_UPDATE)
				ret = updateDetector();
		}
	}
	return retval;
}



slsDetectorDefs::sls_detector_module* slsDetector::interpolateTrim(
		sls_detector_module* a, sls_detector_module* b,
		const int energy, const int e1, const int e2, int tb) {

	// only implemented for eiger currently (in terms of which dacs)
	if (thisDetector->myDetectorType != EIGER) {
		FILE_LOG(logERROR) << "Interpolation of Trim values not implemented for this detector!";
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		return NULL;
	}

	sls_detector_module*  myMod = createModule(thisDetector->myDetectorType);
	if (myMod == NULL) {
		FILE_LOG(logERROR) << "Could not create module";
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		return NULL;

	}
	enum eiger_DacIndex{SVP, VTR, VRF, VRS, SVN, VTGSTV, VCMP_LL, VCMP_LR, CAL,
		VCMP_RL, RXB_RB, RXB_LB, VCMP_RR, VCP, VCN, VIS};

	//Copy other dacs
	int dacs_to_copy[] = {SVP, VTR, SVN, VTGSTV, RXB_RB, RXB_LB, VCN, VIS};
	int num_dacs_to_copy = sizeof(dacs_to_copy) / sizeof(dacs_to_copy[0]);
	for (int i = 0; i <  num_dacs_to_copy; ++i) {
		if (a->dacs[dacs_to_copy[i]] != b->dacs[dacs_to_copy[i]]) {
			deleteModule(myMod);
			return NULL;
		}
		myMod->dacs[dacs_to_copy[i]] = a->dacs[dacs_to_copy[i]];
	}


	//Copy irrelevant dacs (without failing): CAL
	if (a->dacs[CAL] != b->dacs[CAL]) {
		FILE_LOG(logWARNING) << "DAC CAL differs in both energies "
				"(" << a->dacs[CAL] << "," << b->dacs[CAL] << ")!\n"
				"Taking first: " << a->dacs[CAL];
	}
	myMod->dacs[CAL] = a->dacs[CAL];


	//Interpolate vrf, vcmp, vcp
	int dacs_to_interpolate[] = {VRF, VCMP_LL, VCMP_LR, VCMP_RL, VCMP_RR, VCP,  VRS};
	int num_dacs_to_interpolate = sizeof(dacs_to_interpolate) / sizeof(dacs_to_interpolate[0]);
	for (int i = 0; i <  num_dacs_to_interpolate; ++i) {
		myMod->dacs[dacs_to_interpolate[i]] = linearInterpolation(energy, e1, e2,
				a->dacs[dacs_to_interpolate[i]], b->dacs[dacs_to_interpolate[i]]);
	}

	//Interpolate all trimbits
	if (tb) {
		for (int i = 0; i<myMod->nchan; ++i)
			myMod->chanregs[i] = linearInterpolation(energy, e1, e2, a->chanregs[i], b->chanregs[i]);
	}
	return myMod;
}


slsDetectorDefs::sls_detector_module* slsDetector::readSettingsFile(std::string fname,
		sls_detector_module* myMod, int tb) {

	FILE_LOG(logDEBUG1) << "Read settings file " << fname;

	// flag creating module to delete later
	bool modCreated = false;
	if (myMod == NULL) {
		myMod = createModule(thisDetector->myDetectorType);
		if (myMod == NULL) {
			FILE_LOG(logERROR) << "Could not create module";
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
			return NULL;

		}
		modCreated = true;
	}

	std::vector<std::string> names;
	switch (thisDetector->myDetectorType) {
	case GOTTHARD:
		names.push_back("Vref");
		names.push_back("VcascN");
		names.push_back("VcascP");
		names.push_back("Vout");
		names.push_back("Vcasc");
		names.push_back("Vin");
		names.push_back("Vref_comp");
		names.push_back("Vib_test");
		break;
	case EIGER:
		break;
	case JUNGFRAU:
		names.push_back("VDAC0");
		names.push_back("VDAC1");
		names.push_back("VDAC2");
		names.push_back("VDAC3");
		names.push_back("VDAC4");
		names.push_back("VDAC5");
		names.push_back("VDAC6");
		names.push_back("VDAC7");
		names.push_back("VDAC8");
		names.push_back("VDAC9");
		names.push_back("VDAC10");
		names.push_back("VDAC11");
		names.push_back("VDAC12");
		names.push_back("VDAC13");
		names.push_back("VDAC14");
		names.push_back("VDAC15");
		break;
	default:
		FILE_LOG(logERROR) << "Unknown detector type - unknown format for settings file";
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		return NULL;
	}


	// open file
	std::ifstream infile;
	if (thisDetector->myDetectorType == EIGER)
		infile.open(fname.c_str(), std::ifstream::binary);
	else
		infile.open(fname.c_str(), std::ios_base::in);
	if (!infile.is_open()) {
		FILE_LOG(logERROR) << "Could not open settings file for reading: " << fname;
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		if (modCreated)
			deleteModule(myMod);
		return NULL;
	}

	// eiger
	if (thisDetector->myDetectorType == EIGER) {
		bool allread = false;
		infile.read((char*) myMod->dacs, sizeof(int) * (myMod->ndac));
		if (infile.good()) {
			infile.read((char*)&myMod->iodelay, sizeof(myMod->iodelay));
			if(infile.good()) {
				infile.read((char*)&myMod->tau, sizeof(myMod->tau));
				if (tb) {
					if(infile.good()) {
						infile.read((char*) myMod->chanregs, sizeof(int) * (myMod->nchan));
						if (infile)
							allread = true;
					}
				} else if (infile)
					allread = true;

			}
		}
		if (!allread) {
			FILE_LOG(logERROR) << "Could not load all values for settings for " << fname;
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
			if (modCreated)
				deleteModule(myMod);
			return NULL;
		}
		for(int i = 0; i < myMod->ndac; ++i)
			FILE_LOG(logDEBUG1) << "dac " << i << ":" << myMod->dacs[i];
		FILE_LOG(logDEBUG1) << "iodelay:" << myMod->iodelay;
		FILE_LOG(logDEBUG1) << "tau:" << myMod->tau;
	}

	// gotthard, jungfrau
	else {
		size_t idac = 0;
		std::string str;
		while(infile.good()) {
			getline(infile,str);
			FILE_LOG(logDEBUG1) << str;
			std::string sargname;
			int ival = 0;
			std::istringstream ssstr(str);
			ssstr >> sargname >> ival;
			bool found = false;
			for (size_t i = 0; i < names.size(); ++i) {
				if (sargname == names[i]) {
					myMod->dacs[i] = ival;
					found = true;
					FILE_LOG(logDEBUG1) << names[i] << "(" << i << "): " << ival;
					++idac;
				}
			}
			if (!found) {
				FILE_LOG(logERROR) << "Unknown dac " << sargname;
				setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
				if (modCreated)
					deleteModule(myMod);
				return NULL;
			}
		}
		// not all read
		if (idac != names.size()) {
			FILE_LOG(logERROR) << "Could read only " << idac << " dacs. Expected " << names.size() << "dacs";
			setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
			if (modCreated)
				deleteModule(myMod);
			return NULL;
		}
	}

	infile.close();
	strcpy(thisDetector->settingsFile, fname.c_str());
	FILE_LOG(logINFO) << "Settings file loaded: " << thisDetector->settingsFile;
	return myMod;
}


int slsDetector::writeSettingsFile(std::string fname,  sls_detector_module mod) {

	FILE_LOG(logDEBUG1) << "Write settings file " << fname;

	std::vector<std::string> names;
	switch (thisDetector->myDetectorType) {
	case GOTTHARD:
		names.push_back("Vref");
		names.push_back("VcascN");
		names.push_back("VcascP");
		names.push_back("Vout");
		names.push_back("Vcasc");
		names.push_back("Vin");
		names.push_back("Vref_comp");
		names.push_back("Vib_test");
		break;
	case EIGER:
		break;
	case JUNGFRAU:
		names.push_back("VDAC0");
		names.push_back("VDAC1");
		names.push_back("VDAC2");
		names.push_back("VDAC3");
		names.push_back("VDAC4");
		names.push_back("VDAC5");
		names.push_back("VDAC6");
		names.push_back("VDAC7");
		names.push_back("VDAC8");
		names.push_back("VDAC9");
		names.push_back("VDAC10");
		names.push_back("VDAC11");
		names.push_back("VDAC12");
		names.push_back("VDAC13");
		names.push_back("VDAC14");
		names.push_back("VDAC15");
		break;
	default:
		FILE_LOG(logERROR) << "Unknown detector type - unknown format for settings file";
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		return FAIL;
	}

	// open file
	std::ofstream outfile;
	if (thisDetector->myDetectorType == EIGER)
		outfile.open(fname.c_str(), std::ofstream::binary);
	else
		outfile.open(fname.c_str(), std::ios_base::out);
	if (!outfile.is_open()) {
		FILE_LOG(logERROR) << "Could not open settings file for writing: " << fname;
		setErrorMask((getErrorMask())|(OTHER_ERROR_CODE));
		return FAIL;
	}

	// eiger
	if (thisDetector->myDetectorType == EIGER) {
		for(int i = 0; i < mod.ndac; ++i)
			FILE_LOG(logINFO) << "dac " << i << ":" << mod.dacs[i];
		FILE_LOG(logINFO) << "iodelay: " << mod.iodelay;
		FILE_LOG(logINFO) << "tau: " << mod.tau;

		outfile.write((char*)mod.dacs, sizeof(int) * (mod.ndac));
		outfile.write(reinterpret_cast<char*>(&mod.iodelay), sizeof(mod.iodelay));
		outfile.write(reinterpret_cast<char*>(&mod.tau), sizeof(mod.tau));
		outfile.write((char*)mod.chanregs, sizeof(int) * (mod.nchan));
	}

	// gotthard, jungfrau
	else {
		for (int i = 0; i < mod.ndac; ++i) {
			FILE_LOG(logDEBUG1) << "dac " << i << ": " << mod.dacs[i];
			outfile << names[i] << " " << (int)mod.dacs[i] << std::endl;
		}
	}

	outfile.close();
	return OK;
}
