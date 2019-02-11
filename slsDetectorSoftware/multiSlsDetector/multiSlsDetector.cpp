/*******************************************************************

Date:       $Date$
Revision:   $Rev$
Author:     $Author$
URL:        $URL$
ID:         $Id$

 ********************************************************************/

#include "multiSlsDetector.h"
#include "SharedMemory.h"
#include "slsDetector.h"
#include "sls_receiver_exceptions.h"
#include "ThreadPool.h"
#include "ZmqSocket.h"
#include "multiSlsDetectorClient.h"
#include "multiSlsDetectorCommand.h"
#include "postProcessingFuncs.h"
#include "usersFunctions.h"

#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <rapidjson/document.h> //json header in zmq stream
#include <sys/ipc.h>
#include <sys/shm.h>



multiSlsDetector::multiSlsDetector(int id, bool verify, bool update)
: slsDetectorUtils(),
  detId(id),
  sharedMemory(0),
  thisMultiDetector(0),
  client_downstream(false),
  threadpool(0) {
	setupMultiDetector(verify, update);
}



multiSlsDetector::~multiSlsDetector() {
	// delete zmq sockets first
	for (std::vector<ZmqSocket*>::const_iterator it = zmqSocket.begin(); it != zmqSocket.end(); ++it) {
		delete(*it);
	}
	zmqSocket.clear();

	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		delete(*it);
	}
	detectors.clear();

	if (sharedMemory) {
		sharedMemory->UnmapSharedMemory(thisMultiDetector);
		delete sharedMemory;
	}

	destroyThreadPool();
}


bool multiSlsDetector::isMultiSlsDetectorClass() {
	return true;
}

void multiSlsDetector::setupMultiDetector(bool verify, bool update) {
	if (initSharedMemory(verify))
		// shared memory just created, so initialize the structure
		initializeDetectorStructure();
	initializeMembers(verify);
	if (update)
		updateUserdetails();
}

std::string multiSlsDetector::concatResultOrPos(std::string (slsDetector::*somefunc)(int), int pos) {
	if (pos >= 0 && pos < (int)detectors.size()) {
		return (detectors[pos]->*somefunc)(pos);
	} else {
		std::string s;
		for (unsigned int i = 0; i < detectors.size(); ++i) {
			s += (detectors[i]->*somefunc)(pos) + "+";
		}
		return s;
	}
}

template <typename T>
T multiSlsDetector::callDetectorMember(T (slsDetector::*somefunc)())
{
	//(Erik) to handle enums, probably a bad idea but follow previous code
	T defaultValue = static_cast<T>(-1);
	std::vector<T> values(detectors.size(), defaultValue);
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		values[idet] = (detectors[idet]->*somefunc)();
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return minusOneIfDifferent(values);
}

std::string multiSlsDetector::callDetectorMember(std::string (slsDetector::*somefunc)()) {
	std::string concatenatedValue, firstValue;
	bool valueNotSame = false;

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		std::string thisValue = (detectors[idet]->*somefunc)();
		;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));

		if (firstValue.empty()) {
			concatenatedValue = thisValue;
			firstValue        = thisValue;
		} else {
			concatenatedValue += "+" + thisValue;
		}
		if (firstValue != thisValue)
			valueNotSame = true;
	}
	if (valueNotSame)
		return concatenatedValue;
	else
		return firstValue;
}

template <typename T, typename V>
T multiSlsDetector::callDetectorMember(T (slsDetector::*somefunc)(V), V value) {
	//(Erik) to handle enums, probably a bad idea but follow previous code
	T defaultValue = static_cast<T>(-1);
	std::vector<T> values(detectors.size(), defaultValue);
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		values[idet] = (detectors[idet]->*somefunc)(value);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return minusOneIfDifferent(values);
}

template <typename T, typename P1, typename P2>
T multiSlsDetector::callDetectorMember(T (slsDetector::*somefunc)(P1, P2),
		P1 par1, P2 par2) {
	//(Erik) to handle enums, probably a bad idea but follow previous code
	T defaultValue = static_cast<T>(-1);
	std::vector<T> values(detectors.size(), defaultValue);
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		values[idet] = (detectors[idet]->*somefunc)(par1, par2);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return minusOneIfDifferent(values);
}

template <typename T>
T multiSlsDetector::parallelCallDetectorMember(T (slsDetector::*somefunc)()) {
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		std::vector<T> return_values(detectors.size(), -1);
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			Task* task = new Task(new func0_t<T>(somefunc,
					detectors[idet], &return_values[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		return minusOneIfDifferent(return_values);
	}
}

template <typename T, typename P1>
T multiSlsDetector::parallelCallDetectorMember(T (slsDetector::*somefunc)(P1),
		P1 value) {
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		std::vector<T> return_values(detectors.size(), -1);
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			Task* task = new Task(new func1_t<T, P1>(somefunc,
					detectors[idet], value, &return_values[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		return minusOneIfDifferent(return_values);
	}
}

template <typename T, typename P1, typename P2>
T multiSlsDetector::parallelCallDetectorMember(T (slsDetector::*somefunc)(P1, P2),
		P1 par1, P2 par2) {
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		std::vector<T> return_values(detectors.size(), -1);
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			Task* task = new Task(new func2_t<T, P1, P2>(somefunc,
					detectors[idet], par1, par2, &return_values[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		return minusOneIfDifferent(return_values);
	}
}

int multiSlsDetector::parallelCallDetectorMember(int (slsDetector::*somefunc)(int, int, int),
		int v0, int v1, int v2) {
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		std::vector<int> return_values(detectors.size(), -1);
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			Task* task = new Task(new func3_t<int, int, int, int>(somefunc,
					detectors[idet], v0, v1, v2, &return_values[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		return minusOneIfDifferent(return_values);
	}
}

template <typename T>
T multiSlsDetector::minusOneIfDifferent(const std::vector<T>& return_values) {
	T ret = static_cast<T>(-100);
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (ret == static_cast<T>(-100))
			ret = return_values[idet];
		else if (ret != return_values[idet])
			ret = static_cast<T>(-1);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return ret;
}


int multiSlsDetector::decodeNMod(int i, int& id, int& im) {
#ifdef VERBOSE
	cout << " Module " << i << " belongs to detector " << id << endl;
	;
	cout << getMaxMods();
#endif

	if (i < 0 || i >= getMaxMods()) {
		id = -1;
		im = -1;
#ifdef VERBOSE
		cout << " A---------" << id << " position " << im << endl;
#endif

		return -1;
	}
	int nm;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		nm = detectors[idet]->getNMods();
		if (nm > i) {
			id = idet;
			im = i;
#ifdef VERBOSE
			cout << " B---------" << id << " position " << im << endl;
#endif
			return im;
		} else {
			i -= nm;
		}
	}
	id = -1;
	im = -1;
#ifdef VERBOSE
	cout << " C---------" << id << " position " << im << endl;
#endif
	return -1;
}


int multiSlsDetector::decodeNChannel(int offsetX, int offsetY, int& channelX, int& channelY) {
	channelX = -1;
	channelY = -1;
	//loop over
	for (unsigned int i = 0; i < detectors.size(); ++i) {
		int x = detectors[i]->getDetectorOffset(X);
		int y = detectors[i]->getDetectorOffset(Y);
		//check x offset range
		if ((offsetX >= x) &&
				(offsetX < (x +	detectors[i]->getMaxNumberOfChannelsInclGapPixels(X)))) {
			if (offsetY == -1) {
				channelX = offsetX - x;
				return i;
			} else {
				//check y offset range
				if ((offsetY >= y) &&
						(offsetY < (y +	detectors[i]->getMaxNumberOfChannelsInclGapPixels(Y)))) {
					channelX = offsetX - x;
					channelY = offsetY - y;
					return i;
				}
			}
		}
	}
	return -1;
}


double* multiSlsDetector::decodeData(int* datain, int& nn, double* fdata) {
	double* dataout;

	if (fdata)
		dataout = fdata;
	else {
		if (detectors[0]->getDetectorsType() == JUNGFRAUCTB) {
			nn      = thisMultiDetector->dataBytes / 2;
			dataout = new double[nn];
		} else {
			nn      = thisMultiDetector->numberOfChannels;
			dataout = new double[nn];
		}
	}

	int n;
	double* detp = dataout;
	int* datap   = datain;

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		detectors[i]->decodeData(datap, n, detp);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
#ifdef VERBOSE
		cout << "increment pointers " << endl;
#endif
		datap += detectors[i]->getDataBytes() / sizeof(int);
		detp += n;
#ifdef VERBOSE
		cout << "done " << endl;
#endif
	}

	return dataout;
}



int multiSlsDetector::writeDataFile(std::string fname, double* data, double* err,
		double* ang, char dataformat, int nch) {
#ifdef VERBOSE
	cout << "using overloaded multiSlsDetector function to write formatted data file "
			<< getTotalNumberOfChannels() << endl;
#endif

	std::ofstream outfile;
	int choff = 0, off = 0; //idata,
	double *pe = err, *pa = ang;
	int nch_left = nch, n; //, nd;

	if (nch_left <= 0)
		nch_left = getTotalNumberOfChannels();

	if (data == NULL)
		return FAIL;

	outfile.open(fname.c_str(), std::ios_base::out);
	if (outfile.is_open()) {

		for (unsigned int i = 0; i < detectors.size(); ++i) {
			n = detectors[i]->getTotalNumberOfChannels();
			if (nch_left < n)
				n = nch_left;
#ifdef VERBOSE
			cout << " write " << i << " position " << off << " offset " << choff << endl;
#endif
			fileIOStatic::writeDataFile(outfile, n, data + off, pe, pa, dataformat, choff);
			if (detectors[i]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << i));

			nch_left -= n;

			choff += detectors[i]->getMaxNumberOfChannels();

			off += n;

			if (pe)
				pe = err + off;

			if (pa)
				pa = ang + off;
		}
		outfile.close();
		return OK;
	} else {
		std::cout << "Could not open file " << fname << "for writing" << std::endl;
		return FAIL;
	}
}

int multiSlsDetector::writeDataFile(std::string fname, int* data) {
	std::ofstream outfile;
	int choff = 0, off = 0;
#ifdef VERBOSE
	cout << "using overloaded multiSlsDetector function to write raw data file " << endl;
#endif

	if (data == NULL)
		return FAIL;

	outfile.open(fname.c_str(), std::ios_base::out);
	if (outfile.is_open()) {
		for (unsigned int i = 0; i < detectors.size(); ++i) {
#ifdef VERBOSE
			cout << " write " << i << " position " << off << " offset " << choff << endl;
#endif
			detectors[i]->writeDataFile(outfile,
					detectors[i]->getTotalNumberOfChannels(), data + off, choff);
			if (detectors[i]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << i));
			choff += detectors[i]->getMaxNumberOfChannels();
			off += detectors[i]->getTotalNumberOfChannels();
		}
		outfile.close();
		return OK;
	} else {
		std::cout << "Could not open file " << fname << "for writing" << std::endl;
		return FAIL;
	}
}

int multiSlsDetector::readDataFile(std::string fname, double* data, double* err,
		double* ang, char dataformat) {
#ifdef VERBOSE
	cout << "using overloaded multiSlsDetector function to read formatted data file " << endl;
#endif

	std::ifstream infile;
	int iline = 0;
	std::string str;
	int choff = 0, off = 0;
	double *pe = err, *pa = ang;

#ifdef VERBOSE
	std::cout << "Opening file " << fname << std::endl;
#endif
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {

		for (unsigned int i = 0; i < detectors.size(); ++i) {
			iline += detectors[i]->readDataFile(detectors[i]->getTotalNumberOfChannels(),
					infile, data + off, pe, pa, dataformat, choff);
			if (detectors[i]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << i));
			choff += detectors[i]->getMaxNumberOfChannels();
			off += detectors[i]->getTotalNumberOfChannels();
			if (pe)
				pe = pe + off;
			if (pa)
				pa = pa + off;
		}

		infile.close();
	} else {
		std::cout << "Could not read file " << fname << std::endl;
		return -1;
	}
	return iline;
}

int multiSlsDetector::readDataFile(std::string fname, int* data) {
#ifdef VERBOSE
	cout << "using overloaded multiSlsDetector function to read raw data file " << endl;
#endif
	std::ifstream infile;
	int iline = 0;
	std::string str;
	int choff = 0, off = 0;

#ifdef VERBOSE
	std::cout << "Opening file " << fname << std::endl;
#endif
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {

		for (unsigned int i = 0; i < detectors.size(); ++i) {
			iline += detectors[i]->readDataFile(infile, data + off,
					detectors[i]->getTotalNumberOfChannels(), choff);
			if (detectors[i]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << i));
			choff += detectors[i]->getMaxNumberOfChannels();
			off += detectors[i]->getTotalNumberOfChannels();
		}
		infile.close();
	} else {
		std::cout << "Could not read file " << fname << std::endl;
		return -1;
	}
	return iline;
}


std::string multiSlsDetector::getErrorMessage(int& critical) {
	int64_t multiMask, slsMask = 0;
	std::string retval = "";
	char sNumber[100];
	critical = 0;

	multiMask = getErrorMask();
	if (multiMask) {
		if (multiMask & MULTI_DETECTORS_NOT_ADDED) {
			retval.append("Detectors not added:\n" + std::string(getNotAddedList()) +
					std::string("\n"));
			critical = 1;
		}
		if (multiMask & MULTI_HAVE_DIFFERENT_VALUES) {
			retval.append("A previous multi detector command gave different values\n"
					"Please check the console\n");
			critical = 0;
		}
		if (multiMask & MULTI_CONFIG_FILE_ERROR) {
			retval.append("Could not load Config File\n");
			critical = 0;
		}

		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			//if the detector has error
			if (multiMask & (1 << idet)) {
				//append detector id
				sprintf(sNumber, "%d", idet);
				retval.append("Detector " + std::string(sNumber) + std::string(":\n"));
				//get sls det error mask
				slsMask = detectors[idet]->getErrorMask();
#ifdef VERYVERBOSE
				//append sls det error mask
				sprintf(sNumber, "0x%lx", slsMask);
				retval.append("Error Mask " + std::string(sNumber) + std::string("\n"));
#endif
				//get the error critical level
				if ((slsMask > 0xFFFFFFFF) | critical)
					critical = 1;
				//append error message
				retval.append(errorDefs::getErrorMessage(slsMask));
			}
		}
	}
	return retval;
}

int64_t multiSlsDetector::clearAllErrorMask() {
	clearErrorMask();
	clearNotAddedList();
	for (unsigned int idet = 0; idet < detectors.size(); ++idet)
		detectors[idet]->clearErrorMask();

	return getErrorMask();
}

void multiSlsDetector::setErrorMaskFromAllDetectors() {
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
}

void multiSlsDetector::setAcquiringFlag(bool b) {
	thisMultiDetector->acquiringFlag = b;
}

bool multiSlsDetector::getAcquiringFlag() {
	return thisMultiDetector->acquiringFlag;
}

bool multiSlsDetector::isAcquireReady() {
	if (thisMultiDetector->acquiringFlag) {
		std::cout << "Acquire has already started. "
				"If previous acquisition terminated unexpectedly, "
				"reset busy flag to restart.(sls_detector_put busy 0)" << std::endl;
		return FAIL;
	}
	thisMultiDetector->acquiringFlag = true;
	return OK;
}

int multiSlsDetector::checkVersionCompatibility(portType t) {
	return parallelCallDetectorMember(&slsDetector::checkVersionCompatibility, t);
}

int64_t multiSlsDetector::getId(idMode mode, int imod) {
	int id, im;
	int64_t ret;
	if (decodeNMod(imod, id, im) >= 0) {
		if (id < 0 || id >= (int)detectors.size())
			return -1;
		ret = detectors[id]->getId(mode, im);
		if (detectors[id]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << id));
		return ret;
	}

	return callDetectorMember(&slsDetector::getId, mode, imod);
}


slsDetector* multiSlsDetector::getSlsDetector(unsigned int pos) {
	if (pos < detectors.size()) {
		return detectors[pos];
	}
	return 0;
}

slsDetector *multiSlsDetector::operator()(int pos) const {
	if (pos >= 0 && pos < (int)detectors.size())
		return detectors[pos];
	return NULL;
}


void multiSlsDetector::freeSharedMemory(int multiId) {
	// get number of detectors
	int numDetectors = 0;
	SharedMemory* shm = new SharedMemory(multiId, -1);

	// get number of detectors from multi shm
	if (shm->IsExisting()) {
		sharedMultiSlsDetector* mdet = (sharedMultiSlsDetector*)shm->OpenSharedMemory(
				sizeof(sharedMultiSlsDetector));
		numDetectors = mdet->numberOfDetectors;
		shm->UnmapSharedMemory(mdet);
		shm->RemoveSharedMemory();
	}
	delete shm;

	for (int i = 0; i < numDetectors; ++i) {
		SharedMemory* shm = new SharedMemory(multiId, i);
		shm->RemoveSharedMemory();
		delete shm;
	}
}



void multiSlsDetector::freeSharedMemory() {
	// clear zmq vector
	for (std::vector<ZmqSocket*>::const_iterator it = zmqSocket.begin(); it != zmqSocket.end(); ++it) {
		delete(*it);
	}
	zmqSocket.clear();

	// should be done before the detector list is deleted
	clearAllErrorMask();

	// clear sls detector vector shm
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		(*it)->freeSharedMemory();
		delete (*it);
	}
	detectors.clear();

	// clear multi detector shm
	if (sharedMemory) {
		if (thisMultiDetector) {
			sharedMemory->UnmapSharedMemory(thisMultiDetector);
			thisMultiDetector = 0;
		}
		sharedMemory->RemoveSharedMemory();
		delete sharedMemory;
		sharedMemory = 0;
	}

	// zmq
	destroyThreadPool();
	client_downstream = false;
}


std::string multiSlsDetector::getUserDetails() {
	std::ostringstream sstream;

	if (!detectors.size()) {
		return std::string("none");
	}

	//hostname
	sstream << "\nHostname: " << getHostname();
	//type
	sstream<< "\nType: ";

	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it)
		sstream<< (*it)->sgetDetectorsType() << "+";
	//PID
	sstream << "\nPID: " << thisMultiDetector->lastPID
			//user
			<< "\nUser: " << thisMultiDetector->lastUser
			<< "\nDate: " << thisMultiDetector->lastDate << endl;

	std::string s = sstream.str();
	return s;
}

/*
 * pre: sharedMemory=0, thisMultiDetector = 0, detectors.size() = 0
 * exceptions are caught in calling function, shm unmapped and deleted
 */
bool multiSlsDetector::initSharedMemory(bool verify) {

	size_t sz = sizeof(sharedMultiSlsDetector);
	bool created = false;

	try {
		// shared memory object with name
		sharedMemory = new SharedMemory(detId, -1);

		//create
		if (!sharedMemory->IsExisting()) {
			thisMultiDetector = (sharedMultiSlsDetector*)sharedMemory->CreateSharedMemory(sz);
			created = true;
		}
		// open and verify version
		else {
			thisMultiDetector = (sharedMultiSlsDetector*)sharedMemory->OpenSharedMemory(sz);
			if (verify && thisMultiDetector->shmversion != MULTI_SHMVERSION) {
				cprintf(RED, "Multi shared memory (%d) version mismatch "
						"(expected 0x%x but got 0x%x)\n", detId,
						MULTI_SHMVERSION, thisMultiDetector->shmversion);
				throw SharedMemoryException();
			}
		}
	} catch (...) {
		if (sharedMemory) {
			// unmap
			if (thisMultiDetector) {
				sharedMemory->UnmapSharedMemory(thisMultiDetector);
				thisMultiDetector = 0;
			}
			// delete
			delete sharedMemory;
			sharedMemory = 0;
		}
		throw;
	}

	return created;
}


void multiSlsDetector::initializeDetectorStructure() {
	thisMultiDetector->shmversion = MULTI_SHMVERSION;
	thisMultiDetector->numberOfDetectors = 0;
	thisMultiDetector->numberOfDetector[X] = 0;
	thisMultiDetector->numberOfDetector[Y] = 0;
	thisMultiDetector->onlineFlag = 1;
	thisMultiDetector->stoppedFlag = 0;
	thisMultiDetector->masterPosition = -1;
	thisMultiDetector->syncMode = GET_SYNCHRONIZATION_MODE;
	thisMultiDetector->dataBytes = 0;
	thisMultiDetector->dataBytesInclGapPixels = 0;
	thisMultiDetector->numberOfChannels = 0;
	thisMultiDetector->numberOfChannel[X] = 0;
	thisMultiDetector->numberOfChannel[Y] = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[X] = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[Y] = 0;
	thisMultiDetector->maxNumberOfChannels = 0;
	thisMultiDetector->maxNumberOfChannel[X] = 0;
	thisMultiDetector->maxNumberOfChannel[Y] = 0;
	thisMultiDetector->maxNumberOfChannelInclGapPixels[X] = 0;
	thisMultiDetector->maxNumberOfChannelInclGapPixels[Y] = 0;
	thisMultiDetector->maxNumberOfChannelsPerDetector[X] = 0;
	thisMultiDetector->maxNumberOfChannelsPerDetector[Y] = 0;
	for (int i = 0; i < MAX_TIMERS; ++i) {
		thisMultiDetector->timerValue[i] = 0;
	}
	thisMultiDetector->currentSettings = GET_SETTINGS;
	thisMultiDetector->currentThresholdEV = -1;
	thisMultiDetector->progressIndex = 0;
	thisMultiDetector->totalProgress = 1;
	thisMultiDetector->fileIndex = 0;
	strcpy(thisMultiDetector->fileName, "run");
	strcpy(thisMultiDetector->filePath, "/");
	thisMultiDetector->framesPerFile = 1;
	thisMultiDetector->fileFormatType = ASCII;
	thisMultiDetector->correctionMask = (1 << WRITE_FILE) | (1 << OVERWRITE_FILE);
	thisMultiDetector->threadedProcessing = 1;
	thisMultiDetector->tDead = 0;
	strncpy(thisMultiDetector->flatFieldDir, getenv("HOME"), MAX_STR_LENGTH-1);
	thisMultiDetector->flatFieldDir[MAX_STR_LENGTH-1] = 0;
	strcpy(thisMultiDetector->flatFieldFile, "none");
	strcpy(thisMultiDetector->badChanFile, "none");
	strcpy(thisMultiDetector->angConvFile, "none");
	thisMultiDetector->angDirection = 1;
	thisMultiDetector->fineOffset = 0;
	thisMultiDetector->globalOffset = 0;
	thisMultiDetector->binSize = 0.001;
	for (int i = 0; i < 2; ++i) {
		thisMultiDetector->sampleDisplacement[i] = 0.0;
	}
	thisMultiDetector->numberOfPositions = 0;
	for (int i = 0; i < MAXPOS; ++i) {
		thisMultiDetector->detPositions[i] = 0.0;
	}
	thisMultiDetector->actionMask = 0;
	for (int i = 0; i < MAX_ACTIONS; ++i) {
		strcpy(thisMultiDetector->actionScript[i], "none");
		strcpy(thisMultiDetector->actionParameter[i], "none");
	}
	for (int i = 0; i < MAX_SCAN_LEVELS; ++i) {
		thisMultiDetector->scanMode[i] = 0;
		strcpy(thisMultiDetector->scanScript[i], "none");
		strcpy(thisMultiDetector->	scanParameter[i], "none");
		thisMultiDetector->nScanSteps[i] = 0;
		{
			double initValue = 0;
			std::fill_n(thisMultiDetector->scanSteps[i], MAX_SCAN_STEPS, initValue);
		}
		thisMultiDetector->scanPrecision[i] = 0;

	}
	thisMultiDetector->acquiringFlag = false;
	thisMultiDetector->externalgui = false;
	thisMultiDetector->receiverOnlineFlag = OFFLINE_FLAG;
	thisMultiDetector->receiver_upstream = false;
}

void multiSlsDetector::initializeMembers(bool verify) {
	//slsDetectorUtils
	stoppedFlag = &thisMultiDetector->stoppedFlag;
	timerValue = thisMultiDetector->timerValue;
	currentSettings = &thisMultiDetector->currentSettings;
	currentThresholdEV = &thisMultiDetector->currentThresholdEV;

	//fileIO.h
	filePath = thisMultiDetector->filePath;
	fileName = thisMultiDetector->fileName;
	fileIndex = &thisMultiDetector->fileIndex;
	framesPerFile = &thisMultiDetector->framesPerFile;
	fileFormatType = &thisMultiDetector->fileFormatType;

	//postprocessing
	threadedProcessing = &thisMultiDetector->threadedProcessing;
	correctionMask = &thisMultiDetector->correctionMask;
	flatFieldDir = thisMultiDetector->flatFieldDir;
	flatFieldFile = thisMultiDetector->flatFieldFile;
	expTime = &timerValue[ACQUISITION_TIME];
	badChannelMask = NULL;
	fdata = NULL;
	thisData = NULL;

	//slsDetectorActions
	actionMask = &thisMultiDetector->actionMask;
	actionScript = thisMultiDetector->actionScript;
	actionParameter = thisMultiDetector->actionParameter;
	nScanSteps = thisMultiDetector->nScanSteps;
	scanSteps = thisMultiDetector->scanSteps;
	scanMode = thisMultiDetector->scanMode;
	scanPrecision = thisMultiDetector->scanPrecision;
	scanScript = thisMultiDetector->scanScript;
	scanParameter = thisMultiDetector->scanParameter;

	//angularConversion.h
	numberOfPositions = &thisMultiDetector->numberOfPositions;
	detPositions = thisMultiDetector->detPositions;
	angConvFile = thisMultiDetector->angConvFile;
	binSize = &thisMultiDetector->binSize;
	fineOffset = &thisMultiDetector->fineOffset;
	globalOffset = &thisMultiDetector->globalOffset;
	angDirection = &thisMultiDetector->angDirection;
	moveFlag = NULL;
	sampleDisplacement = thisMultiDetector->sampleDisplacement;

	//badChannelCorrections.h or postProcessing_Standalone.h
	badChanFile = thisMultiDetector->badChanFile;
	nBadChans = NULL;
	badChansList = NULL;
	nBadFF = NULL;
	badFFList = NULL;

	//multiSlsDetector
	for (std::vector<ZmqSocket*>::const_iterator it = zmqSocket.begin(); it != zmqSocket.end(); ++it) {
		delete(*it);
	}
	zmqSocket.clear();

	// get objects from single det shared memory (open)
	for (int i = 0; i < thisMultiDetector->numberOfDetectors; i++) {
		try {
			slsDetector* sdet = new slsDetector(detId, i, verify, this);
			detectors.push_back(sdet);
		} catch (...) {
			// clear detectors list
			for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
				delete(*it);
			}
			detectors.clear();
			throw;
		}
	}

	// depend on number of detectors
	updateOffsets();
	createThreadPool();
}


void multiSlsDetector::updateUserdetails() {
	thisMultiDetector->lastPID = getpid();
	memset(thisMultiDetector->lastUser, 0, SHORT_STRING_LENGTH);
	memset(thisMultiDetector->lastDate, 0, SHORT_STRING_LENGTH);
	try {
		strncpy(thisMultiDetector->lastUser, exec("whoami").c_str(), SHORT_STRING_LENGTH-1);
		thisMultiDetector->lastUser[SHORT_STRING_LENGTH-1] = 0;
		strncpy(thisMultiDetector->lastDate, exec("date").c_str(), DATE_LENGTH-1);
		thisMultiDetector->lastDate[DATE_LENGTH-1] = 0;
	} catch(...) {
		strcpy(thisMultiDetector->lastUser, "errorreading");
		strcpy(thisMultiDetector->lastDate, "errorreading");
	}
}


std::string multiSlsDetector::exec(const char* cmd) {
	int bufsize = 128;
	char buffer[bufsize];
	std::string result = "";
	FILE* pipe = popen(cmd, "r");
	if (!pipe) throw std::exception();
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, bufsize, pipe) != NULL)
				result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);
	result.erase(result.find_last_not_of(" \t\n\r")+1);
	return result;
}


void multiSlsDetector::setHostname(const char* name) {
	// this check is there only to allow the previous detsizechan command
	if (thisMultiDetector->numberOfDetectors) {
		cprintf(RED, "Warning: There are already detector(s) in shared memory."
				"Freeing Shared memory now.\n");
		freeSharedMemory();
		setupMultiDetector();
	}
	addMultipleDetectors(name);
}


std::string multiSlsDetector::getHostname(int pos) {
	return concatResultOrPos(&slsDetector::getHostname, pos);
}

void multiSlsDetector::addMultipleDetectors(const char* name) {
	size_t p1 = 0;
	std::string temp = std::string(name);
	size_t p2 = temp.find('+', p1);
	//single
	if (p2 == std::string::npos) {
		addSlsDetector(temp);
	}
	// multi
	else {
		while(p2 != std::string::npos) {
			addSlsDetector(temp.substr(p1, p2-p1));
			temp = temp.substr(p2 + 1);
			p2 = temp.find('+');
		}
	}

	// a get to update shared memory online flag
	setOnline();
	updateOffsets();
	createThreadPool();
}

void multiSlsDetector::addSlsDetector (std::string s) {
#ifdef VERBOSE
	cout << "Adding detector " << s << endl;
#endif
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		if ((*it)->getHostname((it-detectors.begin())) == s) {
			cout << "Detector " << s << "already part of the multiDetector!" << endl
					<< "Remove it before adding it back in a new position!" << endl;
			return;
		}
	}

	//check entire shared memory if it doesnt exist?? needed?
	//could be that detectors not loaded completely cuz of crash in new slsdetector in initsharedmemory

	// get type by connecting
	detectorType type = slsDetector::getDetectorType(s.c_str(), DEFAULT_PORTNO);
	if (type == GENERIC) {
		cout << "Could not connect to Detector " << s << " to determine the type!" << endl;
		setErrorMask(getErrorMask() | MULTI_DETECTORS_NOT_ADDED);
		appendNotAddedList(s.c_str());
		return;
	}



	int pos = (int)detectors.size();
	slsDetector* sdet = new slsDetector(type, detId, pos, false, this);
	detectors.push_back(sdet);
	thisMultiDetector->numberOfDetectors = detectors.size();

	detectors[pos]->setHostname(s.c_str()); // also updates client

	thisMultiDetector->dataBytes += detectors[pos]->getDataBytes();
	thisMultiDetector->dataBytesInclGapPixels += detectors[pos]->getDataBytesInclGapPixels();
	thisMultiDetector->numberOfChannels += detectors[pos]->getTotalNumberOfChannels();
	thisMultiDetector->maxNumberOfChannels += detectors[pos]->getMaxNumberOfChannels();
}


slsDetectorDefs::detectorType multiSlsDetector::getDetectorsType(int pos) {
	detectorType dt = GENERIC;
	if (pos >= 0 && pos < (int)detectors.size()) {
		return detectors[pos]->getDetectorsType();
	} else
		return detectors[0]->getDetectorsType();// needed??
	return dt;
}


std::string multiSlsDetector::sgetDetectorsType(int pos) {
	return concatResultOrPos(&slsDetector::sgetDetectorsType, pos);
}


std::string multiSlsDetector::getDetectorType() {
	return sgetDetectorsType();
}


void multiSlsDetector::createThreadPool() {
	if (threadpool)
		destroyThreadPool();
	int numthreads = (int)detectors.size();
	if (numthreads < 1) {
		numthreads = 1; //create threadpool anyway, threads initialized only when >1 detector added
	}
	threadpool = new ThreadPool(numthreads);
	switch (threadpool->initialize_threadpool()) {
	case 0:
		std::cerr << "Failed to initialize thread pool!" << endl;
		throw ThreadpoolException();
	case 1:
#ifdef VERBOSE
		cout << "Not initializing threads, not multi detector" << endl;
#endif
		break;
	default:
#ifdef VERBOSE
		cout << "Initialized Threadpool " << threadpool << endl;
#endif
		break;
	}
}


void multiSlsDetector::destroyThreadPool() {
	if (threadpool) {
		delete threadpool;
		threadpool = 0;
#ifdef VERBOSE
		cout << "Destroyed Threadpool " << threadpool << endl;
#endif
	}
}


int multiSlsDetector::getNumberOfDetectors() {
	return (int)detectors.size();
}

int multiSlsDetector::getNumberOfDetectors(dimension d) {
	return thisMultiDetector->numberOfDetector[d];
}

void multiSlsDetector::getNumberOfDetectors(int& nx, int& ny) {
	nx=thisMultiDetector->numberOfDetector[X];ny=thisMultiDetector->numberOfDetector[Y];
}

int multiSlsDetector::getNMods() {
	int nm = 0;
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		nm += (*it)->getNMods();
	}
	return nm;
}

int multiSlsDetector::getNMod(dimension d) {
	int nm = 0;
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		nm += (*it)->getNMod(d);
	}
	return nm;
}

int multiSlsDetector::getMaxMods() {
	int ret = 0;
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		ret += (*it)->getMaxMods();
	}
	return ret;
}

int multiSlsDetector::getMaxMod(dimension d) {
	int ret = 0, ret1;
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		ret1 = (*it)->getNMaxMod(d);
#ifdef VERBOSE
		cout << "detector " << (it-detectors.begin()) << " maxmods " <<
				ret1 << " in direction " << d << endl;
#endif
		ret += ret1;
	}
#ifdef VERBOSE
	cout << "max mods in direction " << d << " is " << ret << endl;
#endif

	return ret;
}

int multiSlsDetector::getMaxNumberOfModules(dimension d) {
	int ret = 0;
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		ret += (*it)->getMaxNumberOfModules(d);
	}
	return ret;
}

int multiSlsDetector::setNumberOfModules(int p, dimension d) {
	int ret = 0;
	int nm = 0, mm = 0, nt = p;

	thisMultiDetector->dataBytes              = 0;
	thisMultiDetector->dataBytesInclGapPixels = 0;
	thisMultiDetector->numberOfChannels       = 0;

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (p < 0)
			nm = p;
		else {
			mm = detectors[idet]->getMaxNumberOfModules();
			if (nt > mm) {
				nm = mm;
				nt -= nm;
			} else {
				nm = nt;
				nt -= nm;
			}
		}
		ret += detectors[idet]->setNumberOfModules(nm);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		thisMultiDetector->dataBytes += detectors[idet]->getDataBytes();
		thisMultiDetector->dataBytesInclGapPixels += detectors[idet]->getDataBytesInclGapPixels();
		thisMultiDetector->numberOfChannels += detectors[idet]->getTotalNumberOfChannels();
	}

	if (p != -1)
		updateOffsets();
	return ret;
}

int multiSlsDetector::getChansPerMod(int imod) {
	int id = -1, im = -1;
	decodeNMod(imod, id, im);
	if (id >= 0 && id < (int)detectors.size()) {
		return detectors[id]->getChansPerMod(im);
	}
	return -1;
}

int multiSlsDetector::getTotalNumberOfChannels() {
	thisMultiDetector->numberOfChannels = 0;
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		thisMultiDetector->numberOfChannels += (*it)->getTotalNumberOfChannels();
	}
	return thisMultiDetector->numberOfChannels;
}

int multiSlsDetector::getTotalNumberOfChannels(dimension d) {
	return thisMultiDetector->numberOfChannel[d];
}

int multiSlsDetector::getTotalNumberOfChannelsInclGapPixels(dimension d) {
	return thisMultiDetector->numberOfChannelInclGapPixels[d];
}

int multiSlsDetector::getMaxNumberOfChannels() {
	thisMultiDetector->maxNumberOfChannels = 0;
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		thisMultiDetector->maxNumberOfChannels += (*it)->getMaxNumberOfChannels();
	}
	return thisMultiDetector->maxNumberOfChannels;
}

int multiSlsDetector::getMaxNumberOfChannels(dimension d) {
	return thisMultiDetector->maxNumberOfChannel[d];
}

int multiSlsDetector::getMaxNumberOfChannelsInclGapPixels(dimension d) {
	return thisMultiDetector->maxNumberOfChannelInclGapPixels[d];
}

int multiSlsDetector::getMaxNumberOfChannelsPerDetector(dimension d) {
	return thisMultiDetector->maxNumberOfChannelsPerDetector[d];
}

int multiSlsDetector::setMaxNumberOfChannelsPerDetector(dimension d,int i) {
	thisMultiDetector->maxNumberOfChannelsPerDetector[d] = i;
	return thisMultiDetector->maxNumberOfChannelsPerDetector[d];
}

int multiSlsDetector::getDetectorOffset(dimension d, int pos) {
	if (pos < 0 || pos >= (int)detectors.size())
		return -1;
	return detectors[pos]->getDetectorOffset(d);
}


void multiSlsDetector::setDetectorOffset(dimension d, int off, int pos) {
	if (pos < 0 || pos >= (int)detectors.size())
		detectors[pos]->setDetectorOffset(d, off);
}


void multiSlsDetector::updateOffsets() {
	//cannot paralllize due to slsdetector calling this via parentdet->
#ifdef VERBOSE
	cout << endl
			<< "Updating Multi-Detector Offsets" << endl;
#endif
	int offsetX = 0, offsetY = 0, numX = 0, numY = 0, maxX = 0, maxY = 0;
	int maxChanX   = thisMultiDetector->maxNumberOfChannelsPerDetector[X];
	int maxChanY   = thisMultiDetector->maxNumberOfChannelsPerDetector[Y];
	int prevChanX  = 0;
	int prevChanY  = 0;
	bool firstTime = true;

	thisMultiDetector->numberOfChannel[X]    = 0;
	thisMultiDetector->numberOfChannel[Y]    = 0;
	thisMultiDetector->maxNumberOfChannel[X] = 0;
	thisMultiDetector->maxNumberOfChannel[Y] = 0;
	thisMultiDetector->numberOfDetector[X]   = 0;
	thisMultiDetector->numberOfDetector[Y]   = 0;

	// gap pixels
	int offsetX_gp = 0, offsetY_gp = 0, numX_gp = 0, numY_gp = 0, maxX_gp = 0, maxY_gp = 0;
	int prevChanX_gp = 0, prevChanY_gp = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[X]    = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[Y]    = 0;
	thisMultiDetector->maxNumberOfChannelInclGapPixels[X] = 0;
	thisMultiDetector->maxNumberOfChannelInclGapPixels[Y] = 0;

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
#ifdef VERBOSE
		cout << "offsetX:" << offsetX << " prevChanX:" << prevChanX << " "
				"offsetY:" << offsetY << " prevChanY:" << prevChanY << endl;
		cout << "offsetX_gp:" << offsetX_gp << " "
				"prevChanX_gp:" << prevChanX_gp << " "
				"offsetY_gp:" << offsetY_gp << " "
				"prevChanY_gp:" << prevChanY_gp << endl;
#endif
		//cout<<" totalchan:"<< detectors[idet]->getTotalNumberOfChannels(Y)
		//<<" maxChanY:"<<maxChanY<<endl;
		//incrementing in both direction
		if (firstTime) {
			//incrementing in both directions
			firstTime = false;
			if ((maxChanX > 0) && ((offsetX + detectors[idet]->getTotalNumberOfChannels(X))
					> maxChanX))
				cout << "\nDetector[" << idet << "] exceeds maximum channels "
				"allowed for complete detector set in X dimension!" << endl;
			if ((maxChanY > 0) && ((offsetY + detectors[idet]->getTotalNumberOfChannels(Y))
					> maxChanY))
				cout << "\nDetector[" << idet << "] exceeds maximum channels "
				"allowed for complete detector set in Y dimension!" << endl;
			prevChanX    = detectors[idet]->getTotalNumberOfChannels(X);
			prevChanY    = detectors[idet]->getTotalNumberOfChannels(Y);
			prevChanX_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			prevChanY_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			numX += detectors[idet]->getTotalNumberOfChannels(X);
			numY += detectors[idet]->getTotalNumberOfChannels(Y);
			numX_gp += detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			numY_gp += detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			maxX += detectors[idet]->getMaxNumberOfChannels(X);
			maxY += detectors[idet]->getMaxNumberOfChannels(Y);
			maxX_gp += detectors[idet]->getMaxNumberOfChannelsInclGapPixels(X);
			maxY_gp += detectors[idet]->getMaxNumberOfChannelsInclGapPixels(Y);
			++thisMultiDetector->numberOfDetector[X];
			++thisMultiDetector->numberOfDetector[Y];
#ifdef VERBOSE
			cout << "incrementing in both direction" << endl;
#endif
		}

		//incrementing in y direction
		else if ((maxChanY == -1) || ((maxChanY > 0) &&
				((offsetY + prevChanY + detectors[idet]->getTotalNumberOfChannels(Y))
						<= maxChanY))) {
			offsetY += prevChanY;
			offsetY_gp += prevChanY_gp;
			prevChanY    = detectors[idet]->getTotalNumberOfChannels(Y);
			prevChanY_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			numY += detectors[idet]->getTotalNumberOfChannels(Y);
			numY_gp += detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			maxY += detectors[idet]->getMaxNumberOfChannels(Y);
			maxY_gp += detectors[idet]->getMaxNumberOfChannelsInclGapPixels(Y);
			// increment in y again only in the first column (else you double increment)
			if (thisMultiDetector->numberOfDetector[X] == 1)
			    ++thisMultiDetector->numberOfDetector[Y];
#ifdef VERBOSE
			cout << "incrementing in y direction" << endl;
#endif
		}

		//incrementing in x direction
		else {
			if ((maxChanX > 0) &&
					((offsetX + prevChanX + detectors[idet]->getTotalNumberOfChannels(X))
							> maxChanX))
				cout << "\nDetector[" << idet << "] exceeds maximum channels "
				"allowed for complete detector set in X dimension!" << endl;
			offsetY      = 0;
			offsetY_gp   = 0;
			prevChanY    = detectors[idet]->getTotalNumberOfChannels(Y);
			prevChanY_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			numY         = 0; //assuming symmetry with this statement.
			//whats on 1st column should be on 2nd column
			numY_gp      = 0;
			maxY         = 0;
			maxY_gp      = 0;
			offsetX += prevChanX;
			offsetX_gp += prevChanX_gp;
			prevChanX    = detectors[idet]->getTotalNumberOfChannels(X);
			prevChanX_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			numX += detectors[idet]->getTotalNumberOfChannels(X);
			numX_gp += detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			maxX += detectors[idet]->getMaxNumberOfChannels(X);
			maxX_gp += detectors[idet]->getMaxNumberOfChannelsInclGapPixels(X);
			++thisMultiDetector->numberOfDetector[X];
#ifdef VERBOSE
			cout << "incrementing in x direction" << endl;
#endif
		}

		double bytesperchannel = (double)detectors[idet]->getDataBytes() /
				(double)(detectors[idet]->getMaxNumberOfChannels(X)
						* detectors[idet]->getMaxNumberOfChannels(Y));
		detectors[idet]->setDetectorOffset(X, (bytesperchannel >= 1.0) ? offsetX_gp : offsetX);
		detectors[idet]->setDetectorOffset(Y, (bytesperchannel >= 1.0) ? offsetY_gp : offsetY);

#ifdef VERBOSE
		cout << "Detector[" << idet << "] has offsets (" <<
				detectors[idet]->getDetectorOffset(X) << ", " <<
				detectors[idet]->getDetectorOffset(Y) << ")" << endl;
#endif
		//offsetY has been reset sometimes and offsetX the first time,
		//but remember the highest values
		if (numX > thisMultiDetector->numberOfChannel[X])
			thisMultiDetector->numberOfChannel[X] = numX;
		if (numY > thisMultiDetector->numberOfChannel[Y])
			thisMultiDetector->numberOfChannel[Y] = numY;
		if (numX_gp > thisMultiDetector->numberOfChannelInclGapPixels[X])
			thisMultiDetector->numberOfChannelInclGapPixels[X] = numX_gp;
		if (numY_gp > thisMultiDetector->numberOfChannelInclGapPixels[Y])
			thisMultiDetector->numberOfChannelInclGapPixels[Y] = numY_gp;
		if (maxX > thisMultiDetector->maxNumberOfChannel[X])
			thisMultiDetector->maxNumberOfChannel[X] = maxX;
		if (maxY > thisMultiDetector->maxNumberOfChannel[Y])
			thisMultiDetector->maxNumberOfChannel[Y] = maxY;
		if (maxX_gp > thisMultiDetector->maxNumberOfChannelInclGapPixels[X])
			thisMultiDetector->maxNumberOfChannelInclGapPixels[X] = maxX_gp;
		if (maxY_gp > thisMultiDetector->maxNumberOfChannelInclGapPixels[Y])
			thisMultiDetector->maxNumberOfChannelInclGapPixels[Y] = maxY_gp;
	}
#ifdef VERBOSE
	cout << "Number of Channels in X direction:" << thisMultiDetector->numberOfChannel[X] << endl;
	cout << "Number of Channels in Y direction:" << thisMultiDetector->numberOfChannel[Y] << endl
			<< endl;
	cout << "Number of Channels in X direction with Gap Pixels:" <<
			thisMultiDetector->numberOfChannelInclGapPixels[X] << endl;
	cout << "Number of Channels in Y direction with Gap Pixels:" <<
			thisMultiDetector->numberOfChannelInclGapPixels[Y] << endl
			<< endl;
#endif
}


int multiSlsDetector::setOnline(int off) {
	if (off != GET_ONLINE_FLAG)
		thisMultiDetector->onlineFlag = parallelCallDetectorMember(&slsDetector::setOnline, off);
	return thisMultiDetector->onlineFlag;
}


std::string multiSlsDetector::checkOnline() {
	std::string offlineDetectors = "";
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		std::string tmp = (*it)->checkOnline();
		if (!tmp.empty())
			offlineDetectors += tmp + "+";
	}
	return offlineDetectors;
}


int multiSlsDetector::setPort(portType t, int p) {
	return callDetectorMember(&slsDetector::setPort, t, p);
}

int multiSlsDetector::lockServer(int p) {
	return callDetectorMember(&slsDetector::lockServer, p);
}

std::string multiSlsDetector::getLastClientIP() {
	return callDetectorMember(&slsDetector::getLastClientIP);
}

int multiSlsDetector::exitServer() {
	int ival = FAIL, iv;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		iv = detectors[idet]->exitServer();
		if (iv == OK)
			ival = iv;
	}
	return ival;
}

int multiSlsDetector::readConfigurationFile(std::string const fname) {

	freeSharedMemory();
	setupMultiDetector();


	multiSlsDetectorClient* cmd;
	std::string ans;
	std::string str;
	std::ifstream infile;
	int iargval;
	int interrupt = 0;
	char* args[1000];

	char myargs[1000][1000];

	std::string sargname, sargval;
	int iline = 0;
	std::cout << "config file name " << fname << std::endl;
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {

		while (infile.good() and interrupt == 0) {
			sargname = "none";
			sargval  = "0";
			getline(infile, str);
			++iline;

			// remove comments that come after
			if (str.find('#') != std::string::npos)
				str.erase(str.find('#'));
#ifdef VERBOSE
			std::cout << "string:" << str << std::endl;
#endif
			if (str.length() < 2) {
#ifdef VERBOSE
				std::cout << "Empty line or Comment " << std::endl;
#endif
				continue;
			} else {
				std::istringstream ssstr(str);
				iargval = 0;
				while (ssstr.good()) {
					ssstr >> sargname;
#ifdef VERBOSE
					std::cout << iargval << " " << sargname << std::endl;
#endif
					strcpy(myargs[iargval], sargname.c_str());
					args[iargval] = myargs[iargval];
#ifdef VERBOSE
					std::cout << "--" << iargval << " " << args[iargval] << std::endl;
#endif
					++iargval;
				}
#ifdef VERBOSE
				cout << endl;
				for (int ia = 0; ia < iargval; ia++)
					cout << args[ia] << " ??????? ";
				cout << endl;
#endif
				cmd = new multiSlsDetectorClient(iargval, args, PUT_ACTION, this);
				delete cmd;
			}
			++iline;
		}

		infile.close();
	} else {
		std::cout << "Error opening configuration file " << fname << " for reading" << std::endl;
		setErrorMask(getErrorMask() | MULTI_CONFIG_FILE_ERROR);
		return FAIL;
	}
#ifdef VERBOSE
	std::cout << "Read configuration file of " << iline << " lines" << std::endl;
#endif

	setNumberOfModules(-1);
	getMaxNumberOfModules();

	if (getErrorMask()) {
		int c;
		cprintf(RED, "\n----------------\n Error Messages\n----------------\n%s\n",
				getErrorMessage(c).c_str());
		return FAIL;
	}

	return OK;
}


int multiSlsDetector::writeConfigurationFile(std::string const fname) {

	std::string names[] = {
			"detsizechan",
			"hostname",
			"master",
			"sync",
			"outdir",
			"ffdir",
			"headerbefore",
			"headerafter",
			"headerbeforepar",
			"headerafterpar",
			"badchannels",
			"angconv",
			"globaloff",
			"binsize",
			"threaded"
	};

	int nvar = 15;
	char* args[100];
	for (int ia = 0; ia < 100; ++ia) {
		args[ia] = new char[1000];
	}
	int ret = OK, ret1 = OK;

	std::ofstream outfile;
	int iline = 0;

	outfile.open(fname.c_str(), std::ios_base::out);
	if (outfile.is_open()) {

		slsDetectorCommand* cmd = new slsDetectorCommand(this);

		// complete size of detector
		cout << iline << " " << names[iline] << endl;
		strcpy(args[0], names[iline].c_str());
		outfile << names[iline] << " " << cmd->executeLine(1, args, GET_ACTION) << std::endl;
		++iline;

		// hostname of the detectors
		cout << iline << " " << names[iline] << endl;
		strcpy(args[0], names[iline].c_str());
		outfile << names[iline] << " " << cmd->executeLine(1, args, GET_ACTION) << std::endl;
		++iline;

		// single detector configuration
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			outfile << endl;
			ret1 = detectors[idet]->writeConfigurationFile(outfile, idet);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
			if (ret1 == FAIL)
				ret = FAIL;
		}

		outfile << endl;
		//other configurations
		while (iline < nvar) {
			cout << iline << " " << names[iline] << endl;
			strcpy(args[0], names[iline].c_str());
			outfile << names[iline] << " " << cmd->executeLine(1, args, GET_ACTION) << std::endl;
			++iline;
		}

		delete cmd;
		outfile.close();
#ifdef VERBOSE
		std::cout << "wrote " << iline << " lines to configuration file " << std::endl;
#endif
	} else {
		std::cout << "Error opening configuration file " << fname << " for writing" << std::endl;
		setErrorMask(getErrorMask() | MULTI_CONFIG_FILE_ERROR);
		ret = FAIL;
	}

	for (int ia = 0; ia < 100; ++ia) {
		delete[] args[ia];
	}

	return ret;
}



std::string multiSlsDetector::getSettingsFile() {
	return callDetectorMember(&slsDetector::getSettingsFile);
}


slsDetectorDefs::detectorSettings multiSlsDetector::getSettings(int pos) {
	int ret = -100;
	int posmin = 0, posmax = (int)detectors.size();
	if (pos >= 0) {
		posmin = pos;
		posmax = pos + 1;
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return GET_SETTINGS;
	} else {
		//return storage values
		detectorSettings* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new detectorSettings(GET_SETTINGS);
			Task* task = new Task(new func1_t<detectorSettings, int>(&slsDetector::getSettings,
					detectors[idet], -1, iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (iret[idet] != NULL) {
				if (ret == -100)
					ret = *iret[idet];
				else if (ret != *iret[idet])
					ret = GET_SETTINGS;
				delete iret[idet];
			} else
				ret = GET_SETTINGS;
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	thisMultiDetector->currentSettings = (detectorSettings)ret;
	return (detectorSettings)ret;
}



int multiSlsDetector::getThresholdEnergy(int pos) {
	int i, posmin, posmax;
	int ret1 = -100, ret;

	if (pos < 0) {
		posmin = 0;
		posmax = (int)detectors.size();
	} else {
		posmin = pos;
		posmax = pos + 1;
	}

	for (i = posmin; i < posmax; ++i) {
		ret = detectors[i]->getThresholdEnergy();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 == -100)
			ret1 = ret;
		else if (ret < (ret1 - 200) || ret > (ret1 + 200))
			ret1 = -1;
	}
	thisMultiDetector->currentThresholdEV = ret1;
	return ret1;
}


int multiSlsDetector::setThresholdEnergy(int e_eV, int pos, detectorSettings isettings, int tb) {
	int posmin, posmax;
	int ret = -100;
	if (pos < 0) {
		posmin = 0;
		posmax = (int)detectors.size();
	} else {
		posmin = pos;
		posmax = pos + 1;
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		//return storage values
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new int(-1);
			Task* task = new Task(new func4_t<int, int, int, detectorSettings,
					int>(&slsDetector::setThresholdEnergy,
					detectors[idet], e_eV, -1, isettings, tb, iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (iret[idet] != NULL) {
				if (ret == -100)
					ret = *iret[idet];
				else if (*iret[idet] < (ret - 200) || *iret[idet] > (ret + 200))
					ret = -1;
				delete iret[idet];
			} else
				ret = -1;
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	thisMultiDetector->currentThresholdEV = ret;
	return ret;
}


slsDetectorDefs::detectorSettings multiSlsDetector::setSettings(detectorSettings isettings,
		int pos) {
	int ret = -100;
	int posmin = 0, posmax = (int)detectors.size();
	if (pos >= 0) {
		posmin = pos;
		posmax = pos + 1;
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return GET_SETTINGS;
	} else {
		//return storage values
		detectorSettings* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (detectors[idet]) {
				iret[idet] = new detectorSettings(GET_SETTINGS);
				Task* task = new Task(new func2_t<detectorSettings, detectorSettings, int>
				(&slsDetector::setSettings, detectors[idet], isettings, -1, iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (detectors[idet]) {
				if (iret[idet] != NULL) {
					if (ret == -100)
						ret = *iret[idet];
					else if (ret != *iret[idet])
						ret = GET_SETTINGS;
					delete iret[idet];
				} else
					ret = GET_SETTINGS;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	thisMultiDetector->currentSettings = (detectorSettings)ret;
	return (detectorSettings)ret;
}


std::string multiSlsDetector::getSettingsDir() {
	return callDetectorMember(&slsDetector::getSettingsDir);
}


std::string multiSlsDetector::setSettingsDir(std::string s) {

	if (s.find('+') == std::string::npos) {
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			detectors[idet]->setSettingsDir(s);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	} else {
		size_t p1 = 0;
		size_t p2 = s.find('+', p1);
		int id    = 0;
		while (p2 != std::string::npos) {
			detectors[id]->setSettingsDir(s.substr(p1, p2 - p1));
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			++id;
			s  = s.substr(p2 + 1);
			p2 = s.find('+');
			if (id >= (int)detectors.size())
				break;
		}
	}
	return getSettingsDir();
}

std::string multiSlsDetector::getCalDir() {
	return callDetectorMember(&slsDetector::getCalDir);
}

std::string multiSlsDetector::setCalDir(std::string s) {

	if (s.find('+') == std::string::npos) {
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			detectors[idet]->setCalDir(s);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	} else {
		size_t p1 = 0;
		size_t p2 = s.find('+', p1);
		int id    = 0;
		while (p2 != std::string::npos) {

			if (detectors[id]) {
				detectors[id]->setCalDir(s.substr(p1, p2 - p1));
				if (detectors[id]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << id));
			}
			++id;
			s  = s.substr(p2 + 1);
			p2 = s.find('+');
			if (id >= (int)detectors.size())
				break;
		}
	}
	return getCalDir();
}


int multiSlsDetector::loadSettingsFile(std::string fname, int imod) {
	int ret = OK;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->loadSettingsFile(fname, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int* iret[detectors.size()];
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		iret[idet] = new int(OK);
		Task* task = new Task(new func2_t<int, std::string, int>(&slsDetector::loadSettingsFile,
				detectors[idet], fname, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (iret[idet] != NULL) {
			if (*iret[idet] != OK)
				ret = FAIL;
			delete iret[idet];
		} else
			ret = FAIL;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}

	return ret;
}

int multiSlsDetector::saveSettingsFile(std::string fname, int imod) {
	int id = -1, im = -1, ret;

	if (decodeNMod(imod, id, im) >= 0) {
		if (id < 0 || id >= (int)detectors.size())
			return -1;
		ret = detectors[id]->saveSettingsFile(fname, im);
		if (detectors[id]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << id));
		return ret;

	}
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret = detectors[idet]->saveSettingsFile(fname, imod);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return ret;
}


int multiSlsDetector::loadCalibrationFile(std::string fname, int imod) {
	int ret = OK;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->loadCalibrationFile(fname, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int* iret[detectors.size()];
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (detectors[idet]) {
			iret[idet] = new int(OK);
			Task* task = new Task(new func2_t<int, std::string, int>(&slsDetector::loadCalibrationFile,
					detectors[idet], fname, imod, iret[idet]));
			threadpool->add_task(task);
		}
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (detectors[idet]) {
			if (iret[idet] != NULL) {
				if (*iret[idet] != OK)
					ret = FAIL;
				delete iret[idet];
			} else
				ret = FAIL;
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	return ret;
}

int multiSlsDetector::saveCalibrationFile(std::string fname, int imod) {
	int id = -1, im = -1, ret;
	if (decodeNMod(imod, id, im) >= 0) {
		if (id < 0 || id >= (int)detectors.size())
			return -1;
		ret = detectors[id]->saveCalibrationFile(fname, im);
		if (detectors[id]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << id));
		return ret;
	}
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret = detectors[idet]->saveCalibrationFile(fname, imod);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return ret;
}



int multiSlsDetector::setMaster(int i) {
	int ret = -1, slave = 0;
	masterFlags f;
#ifdef VERBOSE
	cout << "settin master in position " << i << endl;
#endif
	if (i >= 0 && i < (int)detectors.size()) {
#ifdef VERBOSE
		cout << "detector position " << i << " ";
#endif
		thisMultiDetector->masterPosition = i;
		detectors[i]->setMaster(IS_MASTER);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));

		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			if (i != (int)idet) {
#ifdef VERBOSE
				cout << "detector position " << idet << " ";
#endif
				detectors[idet]->setMaster(IS_SLAVE);
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}

	} else if (i == -2) {
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
#ifdef VERBOSE
			cout << "detector position " << idet << " ";
#endif
			detectors[idet]->setMaster(NO_MASTER);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	// check return value

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
#ifdef VERBOSE
		cout << "detector position " << idet << " ";
#endif
		f = detectors[idet]->setMaster(GET_MASTER);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));

		switch (f) {
		case NO_MASTER:
			if (ret != -1)
				ret = -2;
			break;
		case IS_MASTER:
			if (ret == -1)
				ret = idet;
			else
				ret = -2;
			break;
		case IS_SLAVE:
			slave = 1;
			break;
		default:
			ret = -2;
		}

	}
	if (slave > 0 && ret < 0)
		ret = -2;

	if (ret < 0)
		ret = -1;

	thisMultiDetector->masterPosition = ret;

	return thisMultiDetector->masterPosition;
}


slsDetectorDefs::synchronizationMode multiSlsDetector::setSynchronization(synchronizationMode sync) {
	synchronizationMode ret = GET_SYNCHRONIZATION_MODE, ret1 = GET_SYNCHRONIZATION_MODE;

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->setSynchronization(sync);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (idet == 0)
			ret = ret1;
		else if (ret != ret1)
			ret = GET_SYNCHRONIZATION_MODE;
	}

	thisMultiDetector->syncMode = ret;

	return thisMultiDetector->syncMode;
}

slsDetectorDefs::runStatus multiSlsDetector::getRunStatus() {
	runStatus s = IDLE, s1 = IDLE;
	if (thisMultiDetector->masterPosition >= 0) {
		s = detectors[thisMultiDetector->masterPosition]->getRunStatus();
		if (detectors[thisMultiDetector->masterPosition]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << thisMultiDetector->masterPosition));
		return s;
	}

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		s1 = detectors[i]->getRunStatus();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (s1 == ERROR) {
			return ERROR;
		}
		if (s1 != IDLE)
			s = s1;
	}
	return s;
}

int multiSlsDetector::prepareAcquisition() {

	int i      = 0;
	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::prepareAcquisition,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	//master
	int ret1 = OK;
	i = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->prepareAcquisition();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}

	return ret;
}

int multiSlsDetector::cleanupAcquisition() {
	int i   = 0;
	int ret = OK, ret1 = OK;
	int posmin = 0, posmax = detectors.size();

	i = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->cleanupAcquisition();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::cleanupAcquisition,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}
	return ret;
}


int multiSlsDetector::startAcquisition() {
	if (getDetectorsType() == EIGER) {
		if (prepareAcquisition() == FAIL)
			return FAIL;
	}

	int i      = 0;
	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::startAcquisition,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	//master
	int ret1 = OK;
	i        = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->startAcquisition();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}
	return ret;
}


int multiSlsDetector::stopAcquisition() {
	pthread_mutex_lock(&mg); // locks due to processing thread using threadpool when in use
	int i   = 0;
	int ret = OK, ret1 = OK;
	int posmin = 0, posmax = detectors.size();

	i = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->stopAcquisition();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::stopAcquisition,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	*stoppedFlag = 1;
	pthread_mutex_unlock(&mg);
	return ret;
}



int multiSlsDetector::sendSoftwareTrigger() {
	int i      = 0;
	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::sendSoftwareTrigger,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	//master
	int ret1 = OK;
	i        = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->sendSoftwareTrigger();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}
	return ret;
}



int multiSlsDetector::startReadOut() {
	int i   = 0;
	int ret = OK, ret1 = OK;
	i = thisMultiDetector->masterPosition;
	if (i >= 0) {
		ret = detectors[i]->startReadOut();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret != OK)
			ret1 = FAIL;
	}
	for (i = 0; i < (int)detectors.size(); ++i) {
		ret = detectors[i]->startReadOut();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret != OK)
			ret1 = FAIL;
	}

	return ret1;
}


int* multiSlsDetector::startAndReadAll() {
#ifdef VERBOSE
	cout << "Start and read all " << endl;
#endif
	int* retval = NULL;
	int i = 0;
	if (thisMultiDetector->onlineFlag == ONLINE_FLAG) {

		if (getDetectorsType() == EIGER) {
			if (prepareAcquisition() == FAIL)
				return NULL;
		}
		startAndReadAllNoWait();

		while ((retval = getDataFromDetector())) {
			++i;
#ifdef VERBOSE
			std::cout << i << std::endl;
#endif
			dataQueue.push(retval);
		}

		for (unsigned int id = 0; id < detectors.size(); ++id) {
			if (detectors[id]) {
				detectors[id]->disconnectControl();
			}
		}
	}
#ifdef VERBOSE
	std::cout << "Recieved " << i << " frames" << std::endl;
#endif
	return dataQueueFront();
}


int multiSlsDetector::startAndReadAllNoWait() {
	pthread_mutex_lock(&mg); // locks due to processing thread using threadpool when in use
	int i      = 0;
	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition){
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::startAndReadAllNoWait,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	//master
	int ret1 = OK;
	i        = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->startAndReadAllNoWait();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}
	pthread_mutex_unlock(&mg);
	return ret;
}



int* multiSlsDetector::getDataFromDetector() {

	int nel     = thisMultiDetector->dataBytes / sizeof(int);
	int n       = 0;
	int* retval = NULL;
	int *retdet, *p = retval;
	int nodatadet          = -1;
	int nodatadetectortype = false;
	detectorType types     = getDetectorsType();
	if (types == EIGER || types == JUNGFRAU || GOTTHARD || PROPIX) {
		nodatadetectortype = true;
	}

	if (!nodatadetectortype)
		retval = new int[nel];
	p = retval;

	for (unsigned int id = 0; id < detectors.size(); ++id) {
		retdet = detectors[id]->getDataFromDetector(p);
		if (detectors[id]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << id));
		if (!nodatadetectortype) {
			n = detectors[id]->getDataBytes();
			if (retdet) {
				;
#ifdef VERBOSE
				cout << "Detector " << id << " returned " << n << " bytes " << endl;
#endif
			} else {
				nodatadet = id;
#ifdef VERBOSE
				cout << "Detector " << id << " does not have data left " << endl;
#endif
			}
			p += n / sizeof(int);
		}
	}

	//eiger returns only null
	if (nodatadetectortype) {
		return NULL;
	}

	if (nodatadet >= 0) {
		for (unsigned int id = 0; id < detectors.size(); ++id) {
			if ((int)id != nodatadet) {
				if (detectors[id]) {
					//#ifdef VERBOSE
					cout << "Stopping detector " << id << endl;
					//#endif
					detectors[id]->stopAcquisition();
					if (detectors[id]->getErrorMask())
						setErrorMask(getErrorMask() | (1 << id));

					while ((retdet = detectors[id]->getDataFromDetector())) {
						if (detectors[id]->getErrorMask())
							setErrorMask(getErrorMask() | (1 << id));
#ifdef VERBOSE
						cout << "Detector " << id << " still sent data " << endl;
#endif
						delete[] retdet;
					}
					if (detectors[id]->getErrorMask())
						setErrorMask(getErrorMask() | (1 << id));
				}
			}
		}
		delete[] retval;
		return NULL;
	}

	return retval;
}


int* multiSlsDetector::readFrame() {
	int nel = thisMultiDetector->dataBytes / sizeof(int);
	int n;
	int* retval = new int[nel];
	int *retdet, *p = retval;

	for (unsigned int id = 0; id < detectors.size(); ++id) {
		if (detectors[id]) {
			retdet = detectors[id]->readFrame();
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			if (retdet) {
				n = detectors[id]->getDataBytes();
				if (detectors[id]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << id));
				memcpy(p, retdet, n);
				delete[] retdet;
				p += n / sizeof(int);
			} else {
#ifdef VERBOSE
				cout << "Detector " << id << " does not have data left " << endl;
#endif
				delete[] retval;
				return NULL;
			}
		}
	}
	dataQueue.push(retval);
	return retval;
}


int* multiSlsDetector::readAll() {
	int* retval = NULL;
	int i = 0;
#ifdef VERBOSE
	std::cout << "Reading all frames " << std::endl;
#endif
	if (thisMultiDetector->onlineFlag == ONLINE_FLAG) {

		for (unsigned int id = 0; id < detectors.size(); ++id) {
			detectors[id]->readAllNoWait();
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
		}
		while ((retval = getDataFromDetector())) {
			++i;
#ifdef VERBOSE
			std::cout << i << std::endl;
#endif
			dataQueue.push(retval);
		}
		for (unsigned int id = 0; id < detectors.size(); ++id) {
			detectors[id]->disconnectControl();
		}
	}

#ifdef VERBOSE
	std::cout << "received " << i << " frames" << std::endl;
#endif
	return dataQueueFront();
}


int* multiSlsDetector::popDataQueue() {
	int* retval = NULL;
	if (!dataQueue.empty()) {
		retval = dataQueue.front();
		dataQueue.pop();
	}
	return retval;
}

detectorData* multiSlsDetector::popFinalDataQueue() {
	detectorData* retval = NULL;
	if (!finalDataQueue.empty()) {
		retval = finalDataQueue.front();
		finalDataQueue.pop();
	}
	return retval;
}

void multiSlsDetector::resetDataQueue() {
	int* retval = NULL;
	while (!dataQueue.empty()) {
		retval = dataQueue.front();
		dataQueue.pop();
		delete[] retval;
	}
}

void multiSlsDetector::resetFinalDataQueue() {
	detectorData* retval = NULL;
	while (!finalDataQueue.empty()) {
		retval = finalDataQueue.front();
		finalDataQueue.pop();
		delete retval;
	}
}

int multiSlsDetector::configureMAC() {
	return callDetectorMember(&slsDetector::configureMAC);
}

int64_t multiSlsDetector::setTimer(timerIndex index, int64_t t, int imod) {
	int64_t ret=-100;

	// single (for gotthard 25 um)
	if (imod != -1) {
		if (imod >= 0 && imod < (int)detectors.size()) {
			ret = detectors[imod]->setTimer(index,t,imod);
			if(detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask()|(1<<imod));
			return ret;
		}
		return -1;
	}

	// multi
	if(!threadpool){
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}else{
		//return storage values
		int64_t* iret[thisMultiDetector->numberOfDetectors];
		for(int idet=0; idet<thisMultiDetector->numberOfDetectors; ++idet){
			if(detectors[idet]){
				iret[idet]= new int64_t(-1);
				Task* task = new Task(new func3_t<int64_t,timerIndex,int64_t,int>(&slsDetector::setTimer,
						detectors[idet],index,t,imod,iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for(int idet=0; idet<thisMultiDetector->numberOfDetectors; ++idet){
			if(detectors[idet]){
				if(iret[idet] != NULL){
					if (ret==-100)
						ret=*iret[idet];
					else if (ret!=*iret[idet])
						ret=-1;
					delete iret[idet];
				}else ret=-1;
				if(detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask()|(1<<idet));
			}
		}
	}
	if (index == SAMPLES_JCTB)
		setDynamicRange();

	thisMultiDetector->timerValue[index] = ret;
	return ret;
}

int64_t multiSlsDetector::getTimeLeft(timerIndex index, int imod) {
	int64_t ret = -100;

	{	// single
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->getTimeLeft(index, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}
	if (thisMultiDetector->masterPosition >= 0)
		if (detectors[thisMultiDetector->masterPosition]) {
			ret = detectors[thisMultiDetector->masterPosition]->getTimeLeft(index, imod);
			if (detectors[thisMultiDetector->masterPosition]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << thisMultiDetector->masterPosition));
			return ret;
		}
	return callDetectorMember(&slsDetector::getTimeLeft, index, imod);
}

int multiSlsDetector::setSpeed(speedVariable index, int value) {
	int ret1 = -100, ret;

	for (unsigned i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->setSpeed(index, value);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 == -100)
			ret1 = ret;
		else if (ret != ret1)
			ret1 = FAIL;
	}

	return ret1;
}

int multiSlsDetector::setDynamicRange(int p) {
	int ret                                   = -100;
	thisMultiDetector->dataBytes              = 0;
	thisMultiDetector->dataBytesInclGapPixels = 0;
	thisMultiDetector->numberOfChannels       = 0;

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		//return storage values
		int* iret[detectors.size()];
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			iret[idet] = new int(-1);
			Task* task = new Task(new func1_t<int, int>(&slsDetector::setDynamicRange,
					detectors[idet], p, iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			if (iret[idet] != NULL) {
				thisMultiDetector->dataBytes += detectors[idet]->getDataBytes();
				thisMultiDetector->dataBytesInclGapPixels +=
						detectors[idet]->getDataBytesInclGapPixels();
				thisMultiDetector->numberOfChannels +=
						detectors[idet]->getTotalNumberOfChannels();
				if (ret == -100)
					ret = *iret[idet];
				else if (ret != *iret[idet])
					ret = -1;
				delete iret[idet];
			} else
				ret = -1;
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	//for usability for the user
	if (getDetectorsType() == EIGER) {
		if (p == 32) {
			std::cout << "Setting Clock to Quarter Speed to cope with Dynamic Range of 32" << std::endl;
			setSpeed(CLOCK_DIVIDER, 2);
		} else if (p == 16) {
			std::cout << "Setting Clock to Half Speed for Dynamic Range of 16" << std::endl;
			setSpeed(CLOCK_DIVIDER, 1);
		}
		if (p != -1)
			updateOffsets();
	}
	return ret;
}

int multiSlsDetector::getDataBytes() {
	int n_bytes = 0;
	for (unsigned int ip = 0; ip < detectors.size(); ++ip) {
		n_bytes += detectors[ip]->getDataBytes();
	}
	return n_bytes;
}


dacs_t multiSlsDetector::setDAC(dacs_t val, dacIndex idac, int mV, int imod) {
	dacs_t ret = -100;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 && id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->setDAC(val, idac, mV, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int posmin = 0, posmax = detectors.size();
	dacs_t* iret[posmax - posmin];

	for (int idet = posmin; idet < posmax; ++idet) {
		iret[idet] = new dacs_t(-1);
		Task* task = new Task(new func4_t<dacs_t, dacs_t, dacIndex, int, int>
		(&slsDetector::setDAC, detectors[idet], val, idac, mV, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (int idet = posmin; idet < posmax; ++idet) {
		if (iret[idet] != NULL) {

			// highvoltage of slave, ignore value
			if ((idac == HV_NEW) && (*iret[idet] == -999))
				;
			else {
				if (ret == -100)
					ret = *iret[idet];
				else if (ret != *iret[idet])
					ret = -1;
			}
			delete iret[idet];
		} else
			ret = -1;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	if (ret == -100)
		ret = -1;

	return ret;
}

dacs_t multiSlsDetector::getADC(dacIndex idac, int imod) {
	dacs_t ret = -100;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 && id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->getADC(idac, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int posmin = 0, posmax = detectors.size();
	dacs_t* iret[posmax - posmin];

	for (int idet = posmin; idet < posmax; ++idet) {
		iret[idet] = new dacs_t(-1);
		Task* task = new Task(new func2_t<dacs_t, dacIndex, int>(&slsDetector::getADC,
				detectors[idet], idac, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (int idet = posmin; idet < posmax; ++idet) {
		if (detectors[idet]) {
			if (iret[idet] != NULL) {
				if (ret == -100)
					ret = *iret[idet];
				else if (ret != *iret[idet])
					ret = -1;
				delete iret[idet];
			} else
				ret = -1;
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	return ret;
}


slsDetectorDefs::externalCommunicationMode multiSlsDetector::setExternalCommunicationMode(
		externalCommunicationMode pol) {
	externalCommunicationMode ret, ret1;
	//(Dhanya) FIXME: why first detector or is it the master one?
	if (detectors.size())
		ret = detectors[0]->setExternalCommunicationMode(pol);
	if (detectors[0]->getErrorMask())
		setErrorMask(getErrorMask() | (1 << 0));

	for (unsigned int idet = 1; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->setExternalCommunicationMode(pol);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret != ret1)
			ret = GET_EXTERNAL_COMMUNICATION_MODE;
	}
	setMaster();
	setSynchronization();
	return ret;
}

slsDetectorDefs::externalSignalFlag multiSlsDetector::setExternalSignalFlags(
		externalSignalFlag pol, int signalindex) {
	externalSignalFlag ret, ret1;
	//(Dhanya) FIXME: why first detector or is it the master one?
	if (detectors.size())
		ret = detectors[0]->setExternalSignalFlags(pol, signalindex);
	if (detectors[0]->getErrorMask())
		setErrorMask(getErrorMask() | (1 << 0));

	for (unsigned int idet = 1; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->setExternalSignalFlags(pol, signalindex);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret != ret1)
			ret = GET_EXTERNAL_SIGNAL_FLAG;
	}
	setMaster();
	setSynchronization();
	return ret;
}

int multiSlsDetector::setReadOutFlags(readOutFlags flag) {
	int ret = -100, ret1;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->setReadOutFlags(flag);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret == -100)
			ret = ret1;
		else if (ret != ret1)
			ret = -1;
	}

	return ret;
}


uint32_t multiSlsDetector::writeRegister(uint32_t addr, uint32_t val) {
	uint32_t ret, ret1;
	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->writeRegister(addr, val);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (i == 0)
			ret1 = ret;
		else if (ret != ret1) {
			// not setting it to -1 as it is a possible value
			std::cout << "Error: Different Values for function writeRegister ["
					<< ret << "," << ret1 << "]" << endl;
			setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
		}
	}

	return ret1;
}


uint32_t multiSlsDetector::readRegister(uint32_t addr) {
	uint32_t ret, ret1;
	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->readRegister(addr);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (i == 0)
			ret1 = ret;
		else if (ret != ret1) {
			// not setting it to -1 as it is a possible value
			std::cout << "Error: Different Values for function readRegister ["
					<< ret << "," << ret1 << "]" << endl;
			setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
		}
	}

	return ret1;
}




uint32_t multiSlsDetector::setBit(uint32_t addr, int n) {
	uint32_t ret, ret1;
	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret1 = detectors[i]->setBit(addr, n);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (i == 0)
			ret = ret1;
		else if (ret != ret1) {
			// not setting it to -1 as it is a possible value
			std::cout << "Error: Different Values for function setBit ["
					<< ret << "," << ret1 << "]" << endl;
			setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
		}
	}

	return ret;
}

uint32_t multiSlsDetector::clearBit(uint32_t addr, int n) {
	uint32_t ret, ret1;
	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret1 = detectors[i]->clearBit(addr, n);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (i == 0)
			ret = ret1;
		else if (ret != ret1) {
			// not setting it to -1 as it is a possible value
			std::cout << "Error: Different Values for function clearBit ["
					<< ret << "," << ret1 << "]" << endl;
			setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
		}
	}

	return ret;
}

std::string multiSlsDetector::setNetworkParameter(networkParameter p, std::string s) {

	if (s.find('+') == std::string::npos) {

		if (!threadpool) {
			cout << "Error in creating threadpool. Exiting" << endl;
			return getNetworkParameter(p);
		} else {
			std::string* sret[detectors.size()];
			for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
				if (p == RECEIVER_STREAMING_PORT || p == CLIENT_STREAMING_PORT)
					s.append("multi\0");
				sret[idet] = new std::string("error");
				Task* task = new Task(new func2_t<std::string, networkParameter,
						std::string>(&slsDetector::setNetworkParameter,
								detectors[idet], p, s, sret[idet]));
				threadpool->add_task(task);
			}
			threadpool->startExecuting();
			threadpool->wait_for_tasks_to_complete();
			for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
				if (sret[idet] != NULL)
					delete sret[idet];
				//doing nothing with the return values
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}

	} else {
		size_t p1 = 0;
		size_t p2 = s.find('+', p1);
		int id    = 0;
		while (p2 != std::string::npos) {
			detectors[id]->setNetworkParameter(p, s.substr(p1, p2 - p1));
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			++id;
			s  = s.substr(p2 + 1);
			p2 = s.find('+');
			if (id >= (int)detectors.size())
				break;
		}
	}

	return getNetworkParameter(p);
}

std::string multiSlsDetector::getNetworkParameter(networkParameter p) {
	std::string s0 = "", s1 = "", s;
	std::string ans = "";
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		s = detectors[idet]->getNetworkParameter(p);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));

		if (s0 == "")
			s0 = s + std::string("+");
		else
			s0 += s + std::string("+");

		if (s1 == "")
			s1 = s;
		else if (s1 != s)
			s1 = "bad";
	}
	if (s1 == "bad")
		ans = s0;
	else
		ans = s1;
	return ans;
}

int multiSlsDetector::digitalTest(digitalTestMode mode, int imod) {

	int id, im, ret;

	if (decodeNMod(imod, id, im) >= 0) {
		if (id < 0 || id >= (int)detectors.size())
			return -1;
		ret = detectors[id]->digitalTest(mode, im);
		if (detectors[id]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << id));
		return ret;
	}

	return -1;
}

int multiSlsDetector::executeTrimming(trimMode mode, int par1, int par2, int imod) {
	int ret = 100;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->executeTrimming(mode, par1, par2, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int* iret[detectors.size()];
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		iret[idet] = new int(-1);
		Task* task = new Task(new func4_t<int, trimMode, int, int,
				int>(&slsDetector::executeTrimming,
						detectors[idet], mode, par1, par2, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (iret[idet] != NULL) {
			if (ret == -100)
				ret = *iret[idet];
			else if (ret != *iret[idet])
				ret = -1;
			delete iret[idet];
		} else
			ret = -1;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}

	return ret;
}


int multiSlsDetector::loadImageToDetector(imageType index, std::string const fname) {

	int ret = -100, ret1;
	short int imageVals[thisMultiDetector->numberOfChannels];

	std::ifstream infile;
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {
#ifdef VERBOSE
		std::cout << std::endl
				<< "Loading ";
		if (!index)
			std::cout << "Dark";
		else
			std::cout << "Gain";
		std::cout << " image from file " << fname << std::endl;
#endif
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			if (detectors[idet]->readDataFile(infile, imageVals) >= 0) {
				ret1 = detectors[idet]->sendImageToDetector(index, imageVals);
				if (ret == -100)
					ret = ret1;
				else if (ret != ret1)
					ret = -1;
			}
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
		infile.close();
	} else {
		std::cout << "Could not open file " << fname << std::endl;
		return -1;
	}
	return ret;
}

int multiSlsDetector::writeCounterBlockFile(std::string const fname, int startACQ) {
	int ret = OK, ret1 = OK;
	short int arg[thisMultiDetector->numberOfChannels];
	std::ofstream outfile;
	outfile.open(fname.c_str(), std::ios_base::out);
	if (outfile.is_open()) {
#ifdef VERBOSE
		std::cout << std::endl
				<< "Reading Counter to \"" << fname;
		if (startACQ == 1)
			std::cout << "\" and Restarting Acquisition";
		std::cout << std::endl;
#endif
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			ret1 = detectors[idet]->getCounterBlock(arg, startACQ);
			if (ret1 != OK)
				ret = FAIL;
			else {
				ret1 = detectors[idet]->writeDataFile(outfile, arg);
				if (ret1 != OK)
					ret = FAIL;
			}
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
		outfile.close();
	} else {
		std::cout << "Could not open file " << fname << std::endl;
		return -1;
	}
	return ret;
}

int multiSlsDetector::resetCounterBlock(int startACQ) {
	return callDetectorMember(&slsDetector::resetCounterBlock, startACQ);
}

int multiSlsDetector::setCounterBit(int i) {
	return callDetectorMember(&slsDetector::setCounterBit, i);
}

void multiSlsDetector::verifyMinMaxROI(int n, ROI r[]) {
	int temp;
	for (int i = 0; i < n; ++i) {
		if ((r[i].xmax) < (r[i].xmin)) {
			temp      = r[i].xmax;
			r[i].xmax = r[i].xmin;
			r[i].xmin = temp;
		}
		if ((r[i].ymax) < (r[i].ymin)) {
			temp      = r[i].ymax;
			r[i].ymax = r[i].ymin;
			r[i].ymin = temp;
		}
	}
}

int multiSlsDetector::setROI(int n, ROI roiLimits[], int imod) {
    if (imod > 0 && imod < (int)detectors.size()) {
        return detectors[imod]->setROI(n, roiLimits, imod);
    }
	int ret1 = -100, ret;
	int i, xmin, xmax, ymin, ymax, channelX, channelY, idet, lastChannelX,
	lastChannelY, index, offsetX, offsetY;
	bool invalidroi = false;
	int ndet = detectors.size();
	ROI allroi[ndet][n];
	int nroi[ndet];
	for (i = 0; i < ndet; ++i)
		nroi[i] = 0;

	if ((n < 0) || (roiLimits == NULL))
		return FAIL;

	//ensures min < max
	verifyMinMaxROI(n, roiLimits);
#ifdef VERBOSE
	cout << "Setting ROI for " << n << "rois:" << endl;
	for (i = 0; i < n; ++i)
		cout << i << ":" << roiLimits[i].xmin << "\t" << roiLimits[i].xmax
		<< "\t" << roiLimits[i].ymin << "\t" << roiLimits[i].ymax << endl;
#endif
	//for each roi
	for (i = 0; i < n; ++i) {
		xmin = roiLimits[i].xmin;
		xmax = roiLimits[i].xmax;
		ymin = roiLimits[i].ymin;
		ymax = roiLimits[i].ymax;

		//check roi max values
		idet = decodeNChannel(xmax, ymax, channelX, channelY);
#ifdef VERBOSE
		cout << "Decoded Channel max vals: " << endl;
		cout << "det:" << idet << "\t" << xmax << "\t" << ymax << "\t"
				<< channelX << "\t" << channelY << endl;
#endif
		if (idet == -1) {
			cout << "invalid roi" << endl;
			continue;
		}

		//split in x dir
		while (xmin <= xmax) {
			invalidroi = false;
			ymin       = roiLimits[i].ymin;
			//split in y dir
			while (ymin <= ymax) {
				//get offset for each detector
				idet = decodeNChannel(xmin, ymin, channelX, channelY);
#ifdef VERBOSE
				cout << "Decoded Channel min vals: " << endl;
				cout << "det:" << idet << "\t" << xmin << "\t" << ymin
						<< "\t" << channelX << "\t" << channelY << endl;
#endif
				if (idet < 0 || idet >= (int)detectors.size()) {
					cout << "invalid roi" << endl;
					invalidroi = true;
					break;
				}
				//get last channel for each det in x and y dir
				lastChannelX = (detectors[idet]->getMaxNumberOfChannelsInclGapPixels(X)) - 1;
				lastChannelY = (detectors[idet]->getMaxNumberOfChannelsInclGapPixels(Y)) - 1;

				offsetX = detectors[idet]->getDetectorOffset(X);
				offsetY = detectors[idet]->getDetectorOffset(Y);
				//at the end in x dir
				if ((offsetX + lastChannelX) >= xmax)
					lastChannelX = xmax - offsetX;
				//at the end in y dir
				if ((offsetY + lastChannelY) >= ymax)
					lastChannelY = ymax - offsetY;

#ifdef VERBOSE
				cout << "lastChannelX:" << lastChannelX << "\t"
						<< "lastChannelY:" << lastChannelY << endl;
#endif

				//creating the list of roi for corresponding detector
				index                    = nroi[idet];
				allroi[idet][index].xmin = channelX;
				allroi[idet][index].xmax = lastChannelX;
				allroi[idet][index].ymin = channelY;
				allroi[idet][index].ymax = lastChannelY;
				nroi[idet]               = nroi[idet] + 1;

				ymin = lastChannelY + offsetY + 1;
				if ((lastChannelY + offsetY) == ymax)
					ymin = ymax + 1;

#ifdef VERBOSE
				cout << "nroi[idet]:" << nroi[idet] << "\tymin:" << ymin << endl;
#endif

			}
			if (invalidroi)
				break;

			xmin = lastChannelX + offsetX + 1;
			if ((lastChannelX + offsetX) == xmax)
				xmin = xmax + 1;
		}
	}

#ifdef VERBOSE
	cout << "Setting ROI :" << endl;
	for (i = 0; i < detectors.size(); ++i) {
		cout << "detector " << i << endl;
		for (int j = 0; j < nroi[i]; ++j) {
			cout << allroi[i][j].xmin << "\t" << allroi[i][j].xmax << "\t"
					<< allroi[i][j].ymin << "\t" << allroi[i][j].ymax << endl;
		}
	}
#endif

	//settings the rois for each detector
	for (unsigned i = 0; i < detectors.size(); ++i) {
#ifdef VERBOSE
		cout << "detector " << i << ":" << endl;
#endif
		ret = detectors[i]->setROI(nroi[i], allroi[i]);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 == -100)
			ret1 = ret;
		else
			ret1 = FAIL;
	}

	return ret1;
}


slsDetectorDefs::ROI* multiSlsDetector::getROI(int& n, int imod) {
    if (imod > 0 && imod < (int)detectors.size()) {
        return detectors[imod]->getROI(n, imod);
    }
	n          = 0;
	int num    = 0, i, j;
	int ndet   = detectors.size();
	int maxroi = ndet * MAX_ROIS;
	ROI temproi;
	ROI roiLimits[maxroi];
	ROI* retval = new ROI[maxroi];
	ROI* temp   = 0;
	int index   = 0;

	//get each detector's roi array
	for (unsigned i = 0; i < detectors.size(); ++i) {
		temp = detectors[i]->getROI(index);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));

		if (temp) {
			//#ifdef VERBOSE
			if (index)
				cout << "detector " << i << ":" << endl;
			//#endif
			for (j = 0; j < index; ++j) {
				//#ifdef VERBOSE
				cout << temp[j].xmin << "\t" << temp[j].xmax << "\t"
						<< temp[j].ymin << "\t" << temp[j].ymax << endl;
				//#endif
				int x = detectors[i]->getDetectorOffset(X);
				int y = detectors[i]->getDetectorOffset(Y);
				roiLimits[n].xmin = temp[j].xmin + x;
				roiLimits[n].xmax = temp[j].xmax + x;
				roiLimits[n].ymin = temp[j].ymin + y;
				roiLimits[n].ymax = temp[j].ymin + y;
				++n;
			}
		}
	}

	//empty roi
	if (!n)
		return NULL;

#ifdef VERBOSE
	cout << "ROI :" << endl;
	for (int j = 0; j < n; ++j) {
		cout << roiLimits[j].xmin << "\t" << roiLimits[j].xmax << "\t"
				<< roiLimits[j].ymin << "\t" << roiLimits[j].ymax << endl;
	}
#endif

	//combine all the adjacent rois in x direction
	for (i = 0; i < n; ++i) {
		//since the ones combined are replaced by -1
		if ((roiLimits[i].xmin) == -1)
			continue;
		for (j = i + 1; j < n; ++j) {
			//since the ones combined are replaced by -1
			if ((roiLimits[j].xmin) == -1)
				continue;
			//if y values are same
			if (((roiLimits[i].ymin) == (roiLimits[j].ymin)) &&
					((roiLimits[i].ymax) == (roiLimits[j].ymax))) {
				//if adjacent, increase [i] range and replace all [j] with -1
				if ((roiLimits[i].xmax) + 1 == roiLimits[j].xmin) {
					roiLimits[i].xmax = roiLimits[j].xmax;
					roiLimits[j].xmin = -1;
					roiLimits[j].xmax = -1;
					roiLimits[j].ymin = -1;
					roiLimits[j].ymax = -1;
				}
				//if adjacent, increase [i] range and replace all [j] with -1
				else if ((roiLimits[i].xmin) - 1 == roiLimits[j].xmax) {
					roiLimits[i].xmin = roiLimits[j].xmin;
					roiLimits[j].xmin = -1;
					roiLimits[j].xmax = -1;
					roiLimits[j].ymin = -1;
					roiLimits[j].ymax = -1;
				}
			}
		}
	}

#ifdef VERBOSE
	cout << "Combined along x axis Getting ROI :" << endl;
	cout << "detector " << i << endl;
	for (int j = 0; j < n; ++j) {
		cout << roiLimits[j].xmin << "\t" << roiLimits[j].xmax << "\t"
				<< roiLimits[j].ymin << "\t" << roiLimits[j].ymax << endl;
	}
#endif

	//combine all the adjacent rois in y direction
	for (i = 0; i < n; ++i) {
		//since the ones combined are replaced by -1
		if ((roiLimits[i].ymin) == -1)
			continue;
		for (j = i + 1; j < n; ++j) {
			//since the ones combined are replaced by -1
			if ((roiLimits[j].ymin) == -1)
				continue;
			//if x values are same
			if (((roiLimits[i].xmin) == (roiLimits[j].xmin)) &&
					((roiLimits[i].xmax) == (roiLimits[j].xmax))) {
				//if adjacent, increase [i] range and replace all [j] with -1
				if ((roiLimits[i].ymax) + 1 == roiLimits[j].ymin) {
					roiLimits[i].ymax = roiLimits[j].ymax;
					roiLimits[j].xmin = -1;
					roiLimits[j].xmax = -1;
					roiLimits[j].ymin = -1;
					roiLimits[j].ymax = -1;
				}
				//if adjacent, increase [i] range and replace all [j] with -1
				else if ((roiLimits[i].ymin) - 1 == roiLimits[j].ymax) {
					roiLimits[i].ymin = roiLimits[j].ymin;
					roiLimits[j].xmin = -1;
					roiLimits[j].xmax = -1;
					roiLimits[j].ymin = -1;
					roiLimits[j].ymax = -1;
				}
			}
		}
	}

	// get rid of -1s
	for (i = 0; i < n; ++i) {
		if ((roiLimits[i].xmin) != -1) {
			retval[num] = roiLimits[i];
			++num;
		}
	}
	//sort final roi
	for (i = 0; i < num; ++i) {
		for (j = i + 1; j < num; ++j) {
			if (retval[j].xmin < retval[i].xmin) {
				temproi   = retval[i];
				retval[i] = retval[j];
				retval[j] = temproi;
			}
		}
	}
	n = num;

	cout << "\nxmin\txmax\tymin\tymax" << endl;
	for (i = 0; i < n; ++i)
		cout << retval[i].xmin << "\t" << retval[i].xmax << "\t"
		<< retval[i].ymin << "\t" << retval[i].ymax << endl;
	return retval;
}

int multiSlsDetector::writeAdcRegister(int addr, int val) {

	int ret, ret1 = -100;

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->writeAdcRegister(addr, val);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 == -100)
			ret1 = ret;
		else if (ret != ret1) {
			// not setting it to -1 as it is a possible value
			std::cout << "Error: Different Values for function "
					"writeAdcRegister [" << ret << "," << ret1 << "]" << endl;
			setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
		}
	}

	return ret1;
}

int multiSlsDetector::activate(int const enable) {
	return callDetectorMember(&slsDetector::activate, enable);
}

int multiSlsDetector::setDeactivatedRxrPaddingMode(int padding) {
	return callDetectorMember(&slsDetector::setDeactivatedRxrPaddingMode, padding);
}

int multiSlsDetector::getFlippedData(dimension d) {
	return callDetectorMember(&slsDetector::getFlippedData, d);
}

int multiSlsDetector::setFlippedData(dimension d, int value) {
	return callDetectorMember(&slsDetector::setFlippedData, d, value);
}

int multiSlsDetector::setAllTrimbits(int val, int imod) {
	int ret = -100;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->setAllTrimbits(val, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}
	int* iret[detectors.size()];
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		iret[idet] = new int(-1);
		Task* task = new Task(new func2_t<int, int, int>(&slsDetector::setAllTrimbits,
				detectors[idet], val, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (iret[idet] != NULL) {
			if (ret == -100)
				ret = *iret[idet];
			else if (ret != *iret[idet])
				ret = -1;
			delete iret[idet];
		} else
			ret = -1;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return ret;
}


int multiSlsDetector::enableGapPixels(int val) {
	if (val > 0 && getDetectorsType() != EIGER) {
		std::cout << "Not implemented for this detector" << std::endl;
		val = -1;
	}
	int ret = callDetectorMember(&slsDetector::enableGapPixels, val);

	if (val != -1) {
		// update data bytes incl gap pixels
		thisMultiDetector->dataBytesInclGapPixels = 0;
		for (unsigned int i = 0; i < detectors.size(); ++i) {
			thisMultiDetector->dataBytesInclGapPixels +=
					detectors[i]->getDataBytesInclGapPixels();
		}
		// update offsets and number of channels incl gap pixels in multi level
		updateOffsets();
	}
	return ret;
}

int multiSlsDetector::setTrimEn(int ne, int* ene) {
	int ret = -100, ret1;
	int* ene1 = 0;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->setTrimEn(ne, ene);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret == -100)
			ret = ret1;
		else if (ret != ret1)
			ret = -1;

		if (ene != NULL) {
			if (ene1 == 0) {
				ene1 = new int[ret1];
				for (int i = 0; i < ret1; ++i){
					ene1[i] = ene[i];
				}
			}else if (ret != -1) {
				// only check if it is not already a fail
				for (int i = 0; i < ret; ++i){
					if (ene1[i] != ene[i])
						ret = -1;
				}
			}
		}
	}
	return ret;
}

int multiSlsDetector::getTrimEn(int* ene) {
	int ret = -100, ret1;
	int* ene1 = 0;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->getTrimEn(ene);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret == -100)
			ret = ret1;
		else if (ret != ret1)
			ret = -1;

		if (ene != NULL) {
			if (ene1 == 0) {
				ene1 = new int[ret1];
				for (int i = 0; i < ret1; ++i){
					ene1[i] = ene[i];
				}
			}else if (ret != -1) {
				// only check if it is not already a fail
				for (int i = 0; i < ret; ++i){
					if (ene1[i] != ene[i])
						ret = -1;
				}
			}
		}
	}

	if (ene1)
		delete [] ene1;
	return ret;
}

int multiSlsDetector::pulsePixel(int n, int x, int y) {
	return parallelCallDetectorMember(&slsDetector::pulsePixel, n, x, y);
}

int multiSlsDetector::pulsePixelNMove(int n, int x, int y) {
	return parallelCallDetectorMember(&slsDetector::pulsePixelNMove, n, x, y);
}

int multiSlsDetector::pulseChip(int n) {
	return parallelCallDetectorMember(&slsDetector::pulseChip, n);
}

int multiSlsDetector::setThresholdTemperature(int val, int imod) {
	int ret = -100;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->setThresholdTemperature(val, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int posmin = 0, posmax = detectors.size();
	int* iret[posmax - posmin];

	for (int idet = posmin; idet < posmax; ++idet) {
		iret[idet] = new dacs_t(-1);
		Task* task = new Task(new func2_t<int, int, int>(&slsDetector::setThresholdTemperature,
				detectors[idet], val, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (int idet = posmin; idet < posmax; ++idet) {
		if (iret[idet] != NULL) {
			if (ret == -100)
				ret = *iret[idet];
			else if (ret != *iret[idet])
				ret = -1;
			delete iret[idet];
		} else
			ret = -1;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}

	return ret;
}

int multiSlsDetector::setTemperatureControl(int val, int imod) {
	int ret = -100;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->setTemperatureControl(val, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int posmin = 0, posmax = detectors.size();
	int* iret[posmax - posmin];

	for (int idet = posmin; idet < posmax; ++idet) {
		iret[idet] = new dacs_t(-1);
		Task* task = new Task(new func2_t<int, int, int>(&slsDetector::setTemperatureControl,
				detectors[idet], val, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (int idet = posmin; idet < posmax; ++idet) {
		if (iret[idet] != NULL) {
			if (ret == -100)
				ret = *iret[idet];
			else if (ret != *iret[idet])
				ret = -1;
			delete iret[idet];
		} else
			ret = -1;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}

	return ret;
}

int multiSlsDetector::setTemperatureEvent(int val, int imod) {
	int ret = -100;

	// single
	{
		int id = -1, im = -1;
		if (decodeNMod(imod, id, im) >= 0) {
			if (id < 0 || id >= (int)detectors.size())
				return -1;
			ret = detectors[id]->setTemperatureEvent(val, im);
			if (detectors[id]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << id));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}

	int posmin = 0, posmax = detectors.size();
	int* iret[posmax - posmin];

	for (int idet = posmin; idet < posmax; ++idet) {
		iret[idet] = new dacs_t(-1);
		Task* task = new Task(new func2_t<int, int, int>(&slsDetector::setTemperatureEvent,
				detectors[idet], val, imod, iret[idet]));
		threadpool->add_task(task);
	}
	threadpool->startExecuting();
	threadpool->wait_for_tasks_to_complete();
	for (int idet = posmin; idet < posmax; ++idet) {
		if (iret[idet] != NULL) {
			if (ret == -100)
				ret = *iret[idet];
			else if (ret != *iret[idet])
				ret = -1;
			delete iret[idet];
		} else
			ret = -1;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}

	return ret;
}

int multiSlsDetector::setStoragecellStart(int pos) {
	return parallelCallDetectorMember(&slsDetector::setStoragecellStart, pos);
}

int multiSlsDetector::programFPGA(std::string fname) {
	int ret = OK, ret1 = OK;

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->programFPGA(fname);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret == FAIL)
			ret1 = FAIL;
	}
	return ret1;
}

int multiSlsDetector::resetFPGA() {
	int ret = OK, ret1 = OK;

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->resetFPGA();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret == FAIL)
			ret1 = FAIL;
	}
	return ret1;
}

int multiSlsDetector::powerChip(int ival) {
	int ret = OK, ret1 = OK;

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->powerChip(ival);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret == FAIL)
			ret1 = FAIL;
	}
	return ret1;
}

int multiSlsDetector::setAutoComparatorDisableMode(int ival) {
	int ret = OK, ret1 = OK;

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->setAutoComparatorDisableMode(ival);
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret == FAIL)
			ret1 = FAIL;
	}
	return ret1;
}

double multiSlsDetector::getScanStep(int index, int istep) {
	return thisMultiDetector->scanSteps[index][istep];
}

int multiSlsDetector::getChanRegs(double* retval, bool fromDetector) {
	//nChansDet and currentNumChans is because of varying channel size per detector
	int n = thisMultiDetector->numberOfChannels, nChansDet, currentNumChans = 0;
	double retval1[n];

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		nChansDet = detectors[idet]->getChanRegs(retval1, fromDetector);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));

		memcpy(retval + (currentNumChans), retval1, nChansDet * sizeof(double));
		currentNumChans += nChansDet;
	}
	return n;
}



int multiSlsDetector::setChannel(int64_t reg, int ichan, int ichip, int imod) {
	int ret, ret1 = -100;
	int id = -1, im = -1;
	int dmi = 0, dma = detectors.size();

	if (decodeNMod(imod, id, im) >= 0) {
		if (id < 0 || id >= (int)detectors.size())
			return -1;
		dmi = id;
		dma = id + 1;
	}
	for (int idet = dmi; idet < dma; ++idet) {
		ret = detectors[idet]->setChannel(reg, ichan, ichip, im);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret1 == -100)
			ret1 = ret;
		else if (ret != ret1)
			ret1 = -1;
	}
	return ret1;
}

int multiSlsDetector::getMoveFlag(int imod) {
	int id = -1, im = -1;
	if (decodeNMod(imod, id, im) >= 0) {
		if (id < 0 || id >= (int)detectors.size())
			return -1;
		return detectors[id]->getMoveFlag(im);
	}

	//default!!!
	return 1;
}

int multiSlsDetector::fillModuleMask(int* mM) {
	int imod = 0, off = 0;
	if (mM) {
		for (unsigned int i = 0; i < detectors.size(); ++i) {
			for (int im = 0; im < detectors[i]->getNMods(); ++im) {
				mM[imod] = im + off;
				++imod;
			}
			off += detectors[i]->getMaxMods();
		}
	}
	return getNMods();
}


int multiSlsDetector::calibratePedestal(int frames) {
	return callDetectorMember(&slsDetector::calibratePedestal, frames);
}


int multiSlsDetector::setRateCorrection(double t) {

#ifdef VERBOSE
	std::cout << "Setting rate correction with dead time " <<
			thisMultiDetector->tDead << std::endl;
#endif
	int ret    = OK;
	int posmax = detectors.size();

	// eiger return value is ok/fail
	if (getDetectorsType() == EIGER) {
		if (!threadpool) {
			cout << "Error in creating threadpool. Exiting" << endl;
			return FAIL;
		} else {
			int* iret[posmax];
			for (int idet = 0; idet < posmax; ++idet) {
				iret[idet] = new int(OK);
				Task* task = new Task(new func1_t<int, double>
				(&slsDetector::setRateCorrection,
						detectors[idet], t, iret[idet]));
				threadpool->add_task(task);
			}
			threadpool->startExecuting();
			threadpool->wait_for_tasks_to_complete();
			for (int idet = 0; idet < posmax; ++idet) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
		return ret;
	}

	// mythen, others
	if (t == 0) {
		thisMultiDetector->correctionMask &= ~(1 << RATE_CORRECTION);
		return thisMultiDetector->correctionMask & (1 << RATE_CORRECTION);
	} else
		thisMultiDetector->correctionMask |= (1 << RATE_CORRECTION);

	ret = -100;
	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		int* iret[posmax];
		for (int idet = 0; idet < posmax; ++idet) {
			iret[idet] = new int(-1);
			Task* task = new Task(new func1_t<int, double>
			(&slsDetector::setRateCorrection,
					detectors[idet], t, iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = 0; idet < posmax; ++idet) {
			if (iret[idet] != NULL)
				delete iret[idet];
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}
	return thisMultiDetector->correctionMask & (1 << RATE_CORRECTION);
}

int multiSlsDetector::getRateCorrection(double& t) {
	if (getDetectorsType() == EIGER) {
		t = getRateCorrectionTau();
		return t;
	}

	if (thisMultiDetector->correctionMask & (1 << RATE_CORRECTION)) {
#ifdef VERBOSE
		std::cout << "Rate correction is enabled with dead time " <<
				thisMultiDetector->tDead << std::endl;
#endif
		return 1;
	} else
		t = 0;
#ifdef VERBOSE
	std::cout << "Rate correction is disabled " << std::endl;
#endif
	return 0;
}

double multiSlsDetector::getRateCorrectionTau() {

	double ret = -100.0;
	int posmax = detectors.size();

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		double* iret[posmax];
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			iret[idet] = new double(-1);
			Task* task = new Task(new func0_t<double>
			(&slsDetector::getRateCorrectionTau,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();

		for (int idet = 0; idet < posmax; ++idet) {
			if (iret[idet] != NULL) {
				if (ret == -100.0)
					ret = *iret[idet];
				else if ((ret - *iret[idet]) > 0.000000001) {
					std::cout << "Rate correction is different for "
							"different readouts " << std::endl;
					ret = -1;
				}
				delete iret[idet];
			} else
				ret = -1;
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	if (getDetectorsType() == EIGER)
		return ret;

	//only mythen
	if (thisMultiDetector->correctionMask & (1 << RATE_CORRECTION)) {
#ifdef VERBOSE
		std::cout << "Rate correction is enabled with dead time " <<
				thisMultiDetector->tDead << std::endl;
#endif
	} else {
#ifdef VERBOSE
		std::cout << "Rate correction is disabled " << std::endl;
#endif
		ret = 0;
	}
	return ret;
}

int multiSlsDetector::getRateCorrection() {
	if (getDetectorsType() == EIGER) {
		return getRateCorrectionTau();
	}
	if (thisMultiDetector->correctionMask & (1 << RATE_CORRECTION)) {
		return 1;
	} else
		return 0;
};

int multiSlsDetector::rateCorrect(double* datain, double* errin, double* dataout, double* errout) {
	int ichdet   = 0;
	double* perr = errin;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (errin)
			perr += ichdet;
		detectors[idet]->rateCorrect(datain + ichdet, perr, dataout + ichdet,
				errout + ichdet);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		ichdet += detectors[idet]->getTotalNumberOfChannels();
	}
	return 0;
}


int multiSlsDetector::setFlatFieldCorrection(std::string fname) {
	double* data           = new double[thisMultiDetector->numberOfChannels];
	double* ffcoefficients = new double[thisMultiDetector->numberOfChannels];
	double* fferrors       = new double[thisMultiDetector->numberOfChannels];
	char ffffname[MAX_STR_LENGTH * 2];
	int nch;
	if (fname == "default") {
		fname = std::string(thisMultiDetector->flatFieldFile);
	}

	thisMultiDetector->correctionMask &= ~(1 << FLAT_FIELD_CORRECTION);

	if (fname == "") {
#ifdef VERBOSE
		std::cout << "disabling flat field correction" << std::endl;
#endif
		thisMultiDetector->correctionMask &= ~(1 << FLAT_FIELD_CORRECTION);
		for (unsigned int i = 0; i < detectors.size(); ++i) {
			detectors[i]->setFlatFieldCorrection(NULL, NULL);
			if (detectors[i]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << i));
		}
	} else {
#ifdef VERBOSE
		std::cout << "Setting flat field correction from file " << fname << std::endl;
#endif
		sprintf(ffffname, "%s/%s", thisMultiDetector->flatFieldDir, fname.c_str());
		nch = readDataFile(std::string(ffffname), data);

		if (nch > thisMultiDetector->numberOfChannels)
			nch = thisMultiDetector->numberOfChannels;

		if (nch > 0) {

			int nm = getNMods();
			int chpm[nm];
			int mMask[nm];
			for (int i = 0; i < nm; ++i) {
				chpm[i]  = getChansPerMod(i);
				mMask[i] = i;
			}
			fillModuleMask(mMask);
			fillBadChannelMask();
			if ((postProcessingFuncs::calculateFlatField(&nm, chpm, mMask,
					badChannelMask, data, ffcoefficients, fferrors)) >= 0) {
				strcpy(thisMultiDetector->flatFieldFile, fname.c_str());

				thisMultiDetector->correctionMask |= (1 << FLAT_FIELD_CORRECTION);

				setFlatFieldCorrection(ffcoefficients, fferrors);

			} else
				std::cout << "Calculated flat field from file " << fname <<
				" is not valid " << nch << std::endl;
		} else {
			std::cout << "Flat field from file " << fname << " is not valid "
					<< nch << std::endl;
		}
	}
	return thisMultiDetector->correctionMask & (1 << FLAT_FIELD_CORRECTION);
}


int multiSlsDetector::setFlatFieldCorrection(double* corr, double* ecorr) {
	int ichdet = 0;
	double *p, *ep;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (corr != NULL)
			p = corr + ichdet;
		else
			p = NULL;
		if (ecorr != NULL)
			ep = ecorr + ichdet;
		else
			ep = NULL;
		detectors[idet]->setFlatFieldCorrection(p, ep);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		ichdet += detectors[idet]->getTotalNumberOfChannels();
	}
	return 0;
}

int multiSlsDetector::getFlatFieldCorrection(double* corr, double* ecorr) {
	int ichdet = 0;
	double *p, *ep;
	int ff = 1, dff;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (corr != NULL)
			p = corr + ichdet;
		else
			p = NULL;
		if (ecorr != NULL)
			ep = ecorr + ichdet;
		else
			ep = NULL;
		dff = detectors[idet]->getFlatFieldCorrection(p, ep);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (dff == 0)
			ff = 0;
		ichdet += detectors[idet]->getTotalNumberOfChannels();
	}
	return ff;
}

int multiSlsDetector::flatFieldCorrect(double* datain, double* errin, double* dataout,
		double* errout) {
	int ichdet   = 0;
	double* perr = errin; //*pdata,
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
#ifdef VERBOSE
		cout << " detector " << idet << " offset " << ichdet << endl;
#endif
		if (errin)
			perr += ichdet;
		detectors[idet]->flatFieldCorrect(datain + ichdet, perr,
				dataout + ichdet, errout + ichdet);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		ichdet += detectors[idet]->getTotalNumberOfChannels();
	}
	return 0;
}

int multiSlsDetector::setBadChannelCorrection(std::string fname) {
	int badlist[MAX_BADCHANS];
	int nbad = 0;
	int ret  = 0;

	cout << thisMultiDetector->badChanFile << endl;

	if (fname == "default")
		fname = std::string(thisMultiDetector->badChanFile);

	ret = setBadChannelCorrection(fname, nbad, badlist);
	//#ifdef VERBOSE
	cout << "multi: file contained " << ret << " badchans" << endl;
	//#endif
	if (ret == 0) {
		thisMultiDetector->correctionMask &= ~(1 << DISCARD_BAD_CHANNELS);
		nbad = 0;
	} else {
		thisMultiDetector->correctionMask |= (1 << DISCARD_BAD_CHANNELS);
		strcpy(thisMultiDetector->badChanFile, fname.c_str());
	}

	return setBadChannelCorrection(nbad, badlist, 0);
}

int multiSlsDetector::setBadChannelCorrection(int nbad, int* badlist, int ff) {
	int badlistdet[MAX_BADCHANS];
	int nbaddet = 0, choff = 0, idet = 0;
	if (nbad < 1)
		badlistdet[0] = 0;
	else
		badlistdet[0] = badlist[0];

	if (nbad > 0) {
		thisMultiDetector->correctionMask |= (1 << DISCARD_BAD_CHANNELS);

		for (int ich = 0; ich < nbad; ++ich) {
			if ((badlist[ich] - choff) >= detectors[idet]->getMaxNumberOfChannels()) {
				//#ifdef VERBOSE
				cout << "setting " << nbaddet << " badchans to detector "
						<< idet << endl;
				//#endif
				detectors[idet]->setBadChannelCorrection(nbaddet, badlistdet, 0);
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
				choff += detectors[idet]->getMaxNumberOfChannels();
				nbaddet = 0;
				++idet;
				if (detectors[idet] == NULL)
					break;
			}
			badlistdet[nbaddet] = (badlist[ich] - choff);
			++nbaddet;
#ifdef VERBOSE
			cout << nbaddet << " " << badlist[ich] << " "
					<< badlistdet[nbaddet - 1] << endl;
#endif
		}
		if (nbaddet > 0) {
#ifdef VERBOSE
			cout << "setting " << nbaddet << " badchans to detector "
					<< idet << endl;
#endif
			detectors[idet]->setBadChannelCorrection(nbaddet, badlistdet, 0);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
			choff += detectors[idet]->getMaxNumberOfChannels();
			nbaddet = 0;
			++idet;
		}
		nbaddet = 0;
		for (unsigned int i = idet; i < detectors.size(); ++i) {
#ifdef VERBOSE
			cout << "setting " << 0 << " badchans to detector " << i << endl;
#endif
			detectors[i]->setBadChannelCorrection(nbaddet, badlistdet, 0);
			if (detectors[i]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << i));
		}

	} else {
		nbaddet = 0;
		for (unsigned int i = 0; i < detectors.size(); ++i) {
#ifdef VERBOSE
			cout << "setting " << 0 << " badchans to detector " << idet << endl;
#endif
			detectors[idet]->setBadChannelCorrection(nbaddet, badlistdet, 0);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
		thisMultiDetector->correctionMask &= ~(1 << DISCARD_BAD_CHANNELS);
	}
#ifdef VERBOSE
	cout << (thisMultiDetector->correctionMask & (1 << DISCARD_BAD_CHANNELS)) << endl;
#endif
	return thisMultiDetector->correctionMask & (1 << DISCARD_BAD_CHANNELS);
}


int multiSlsDetector::getBadChannelCorrection(int* bad) {
	//int ichan;
	int *bd, nd, ntot = 0, choff = 0;
	;

	if (((thisMultiDetector->correctionMask) & (1 << DISCARD_BAD_CHANNELS)) == 0)
		return 0;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		nd = detectors[idet]->getBadChannelCorrection();
		if (nd > 0) {
			bd = new int[nd];
			nd = detectors[idet]->getBadChannelCorrection(bd);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));

			for (int id = 0; id < nd; ++id) {
				if (bd[id] < detectors[idet]->getTotalNumberOfChannels()) {
					if (bad)
						bad[ntot] = choff + bd[id];
					++ntot;
				}
			}
			choff += detectors[idet]->getTotalNumberOfChannels();
			delete[] bd;
		} else
			ntot += nd;
	}
	return ntot;
}


int multiSlsDetector::readAngularConversionFile(std::string fname) {

	std::ifstream infile;
	//int nm=0;
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {

		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
#ifdef VERBOSE
			cout << " detector " << idet << endl;
#endif
			detectors[idet]->readAngularConversion(infile);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
		infile.close();
	} else {
		std::cout << "Could not open calibration file " << fname << std::endl;
		return -1;
	}
	return 0;
}

int multiSlsDetector::writeAngularConversion(std::string fname) {

	std::ofstream outfile;
	//  int nm=0;
	outfile.open(fname.c_str(), std::ios_base::out);
	if (outfile.is_open()) {

		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			detectors[idet]->writeAngularConversion(outfile);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
		outfile.close();
	} else {
		std::cout << "Could not open calibration file " << fname << std::endl;
		return -1;
	}
	return 0;
}

int multiSlsDetector::getAngularConversion(int& direction, angleConversionConstant* angconv) {

	int dir                     = -100, dir1;
	angleConversionConstant* a1 = angconv;

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		detectors[idet]->getAngularConversion(dir1, a1);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (dir == -100)
			dir = dir1;
		if (dir != dir1)
			dir = 0;
		if (angconv) {
			a1 += detectors[idet]->getNMods();
		}
	}
	direction = dir;

	if (thisMultiDetector->correctionMask & (1 << ANGULAR_CONVERSION)) {
		return 1;
	}
	return 0;
}

double multiSlsDetector::setAngularConversionParameter(angleConversionParameter c, double v) {
	double ret = slsDetectorUtils::setAngularConversionParameter(c, v);
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		detectors[idet]->setAngularConversionParameter(c, v);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return ret;
}

angleConversionConstant* multiSlsDetector::getAngularConversionPointer(int imod) {
	int id = -1, im = -1;
#ifdef VERBOSE
	cout << "get angular conversion pointer " << endl;
#endif
	if (decodeNMod(imod, id, im) >= 0) {
		if (id < 0 || id >= (int)detectors.size())
			return NULL;
		return detectors[id]->getAngularConversionPointer(im);
	}
	return NULL;
}


int multiSlsDetector::printReceiverConfiguration() {
	int ret, ret1 = -100;
	std::cout << "Printing Receiver configurations for all detectors..." << std::endl;
	for (unsigned int i = 0; i < detectors.size(); ++i) {
		std::cout << std::endl
				<< "#Detector " << i << ":" << std::endl;

		ret = detectors[i]->printReceiverConfiguration();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 == -100)
			ret1 = ret;
		else if (ret != ret1)
			ret1 = -1;
	}

	return ret1;
}


int multiSlsDetector::setReceiverOnline(int off) {
	if (off != GET_ONLINE_FLAG) {
		thisMultiDetector->receiverOnlineFlag = parallelCallDetectorMember(
				&slsDetector::setReceiverOnline, off);
	}
	return thisMultiDetector->receiverOnlineFlag;
}

std::string multiSlsDetector::checkReceiverOnline() {
	std::string retval1 = "", retval;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		retval = detectors[idet]->checkReceiverOnline();
		if (!retval.empty()) {
			retval1.append(retval);
			retval1.append("+");
		}
	}
	return retval1;
}

int multiSlsDetector::lockReceiver(int lock) {
	return callDetectorMember(&slsDetector::lockReceiver, lock);
}

std::string multiSlsDetector::getReceiverLastClientIP() {
	return callDetectorMember(&slsDetector::getReceiverLastClientIP);
}

int multiSlsDetector::exitReceiver() {
	//(Erik) logic is flawed should return fail if any fails?
	int ival = FAIL, iv;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		iv = detectors[idet]->exitReceiver();
		if (iv == OK)
			ival = iv;
	}
	return ival;
}

std::string multiSlsDetector::getFilePath() {
	return setFilePath();
}

std::string multiSlsDetector::setFilePath(std::string s) {

	std::string ret = "errorerror", ret1;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->setFilePath(s);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret == "errorerror")
			ret = ret1;
		else if (ret != ret1)
			ret = "";
	}
	fileIO::setFilePath(ret);
	return fileIO::getFilePath();
}

std::string multiSlsDetector::getFileName() {
	return setFileName();
}

std::string multiSlsDetector::setFileName(std::string s) {

	std::string ret = "error";
	int posmax = detectors.size();

	if (!s.empty()) {
		fileIO::setFileName(s);
		if (thisMultiDetector->receiverOnlineFlag == ONLINE_FLAG)
			s = createReceiverFilePrefix();
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return std::string("");
	} else {
		std::string* sret[detectors.size()];
		for (int idet = 0; idet < posmax; ++idet) {
			sret[idet] = new std::string("error");
			Task* task = new Task(new func1_t<std::string, std::string>(&slsDetector::setFileName,
					detectors[idet], s, sret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = 0; idet < posmax; ++idet) {
			if (sret[idet] != NULL) {
				if (ret == "error")
					ret = *sret[idet];
				else if (ret != *sret[idet])
					ret = "";
				delete sret[idet];
			} else
				ret = "";
			//doing nothing with the return values
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}

	if ((thisMultiDetector->receiverOnlineFlag == ONLINE_FLAG) &&
			((ret != "error") || (ret != ""))) {
#ifdef VERBOSE
		std::cout << "Complete file prefix from receiver: " << ret << std::endl;
#endif
		fileIO::setFileName(getNameFromReceiverFilePrefix(ret));
	}

	return ret;
}

int multiSlsDetector::setReceiverFramesPerFile(int f) {
	return parallelCallDetectorMember(&slsDetector::setReceiverFramesPerFile, f);
}

slsReceiverDefs::frameDiscardPolicy multiSlsDetector::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
	return callDetectorMember(&slsDetector::setReceiverFramesDiscardPolicy, f);
}

int multiSlsDetector::setReceiverPartialFramesPadding(int f) {
	return callDetectorMember(&slsDetector::setReceiverPartialFramesPadding, f);
}

slsReceiverDefs::fileFormat multiSlsDetector::getFileFormat() {
	return setFileFormat();
}

slsReceiverDefs::fileFormat multiSlsDetector::setFileFormat(fileFormat f) {
	return callDetectorMember(&slsDetector::setFileFormat, f);
}

int multiSlsDetector::getFileIndex() {
	return setFileIndex();
}

int multiSlsDetector::setFileIndex(int i) {
	return parallelCallDetectorMember(&slsDetector::setFileIndex, i);
}


int multiSlsDetector::startReceiver() {
	int i      = 0;
	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition){
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::startReceiver,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition)  {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	//master
	int ret1 = OK;
	i        = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->startReceiver();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}
	return ret;
}

int multiSlsDetector::stopReceiver() {
	int i   = 0;
	int ret = OK, ret1 = OK;
	int posmin = 0, posmax = detectors.size();

	i = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret1 = detectors[i]->stopReceiver();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret1 != OK)
			ret = FAIL;
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				iret[idet] = new int(OK);
				Task* task = new Task(new func0_t<int>(&slsDetector::stopReceiver,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				if (iret[idet] != NULL) {
					if (*iret[idet] != OK)
						ret = FAIL;
					delete iret[idet];
				} else
					ret = FAIL;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	return ret;
}

slsDetectorDefs::runStatus multiSlsDetector::startReceiverReadout() {
	unsigned int i = 0;
	runStatus s = IDLE, s1 = IDLE;
	i = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		s1 = detectors[i]->startReceiverReadout();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
	}
	for (i = 0; i < detectors.size(); ++i) {
		s = detectors[i]->startReceiverReadout();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (s == ERROR)
			s1 = ERROR;
		if (s != IDLE)
			s1 = s;
	}

	return s1;
}

slsDetectorDefs::runStatus multiSlsDetector::getReceiverStatus() {
	int i         = 0;
	runStatus ret = IDLE;
	int posmin = 0, posmax = detectors.size();

	i = thisMultiDetector->masterPosition;
	if (thisMultiDetector->masterPosition >= 0) {
		ret = detectors[i]->getReceiverStatus();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		return ret;
	}

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return ERROR;
	} else {
		runStatus* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition) {
				iret[idet] = new runStatus(ERROR);
				Task* task = new Task(new func0_t<runStatus>(&slsDetector::getReceiverStatus,
						detectors[idet], iret[idet]));
				threadpool->add_task(task);
			}
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
			if (idet != thisMultiDetector->masterPosition){
				if (iret[idet] != NULL) {
					if (*iret[idet] == (int)ERROR)
						ret = ERROR;
					if (*iret[idet] != IDLE)
						ret = *iret[idet];
					delete iret[idet];
				} else
					ret = ERROR;
				if (detectors[idet]->getErrorMask())
					setErrorMask(getErrorMask() | (1 << idet));
			}
		}
	}

	return ret;
}

int multiSlsDetector::getFramesCaughtByAnyReceiver() {
	int ret = 0;
	int i   = thisMultiDetector->masterPosition;
	if (i >= 0) {
		ret = detectors[i]->getFramesCaughtByReceiver();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		// return master receivers frames caught
		return ret;
	}

	// return the first one that works
	if (detectors.size()) {
		ret = detectors[0]->getFramesCaughtByReceiver();
		if (detectors[0]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		return ret;
	}

	return -1;
}

int multiSlsDetector::getFramesCaughtByReceiver() {

	int ret = 0, ret1 = 0;
	int posmax = detectors.size();

	if (!threadpool) {
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	} else {
		int* iret[posmax];
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			iret[idet] = new int(0);
			Task* task = new Task(new func0_t<int>(&slsDetector::getFramesCaughtByReceiver,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();

		for (int idet = 0; idet < posmax; ++idet) {
			if (iret[idet] != NULL) {
				if (*iret[idet] == -1) // could not connect
					ret = -1;
				else
					ret1 += (*iret[idet]);
				delete iret[idet];
			} else
				ret = -1;
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
		}
	}
	if ((!detectors.size()) || (ret == -1))
		return ret;

	ret = (int)(ret1 / detectors.size());
	return ret;
}


int multiSlsDetector::getReceiverCurrentFrameIndex() {
	int ret = 0, ret1 = 0;
	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret1 += detectors[i]->getReceiverCurrentFrameIndex();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
	}
	if (!detectors.size())
		return ret;
	ret = (int)(ret1 / detectors.size());

	return ret;
}

int multiSlsDetector::resetFramesCaught() {
	return parallelCallDetectorMember(&slsDetector::resetFramesCaught);
}

int multiSlsDetector::createReceivingDataSockets(const bool destroy) {
	if (destroy) {
		cprintf(MAGENTA, "Going to destroy data sockets\n");
		//close socket
		for (std::vector<ZmqSocket*>::const_iterator it = zmqSocket.begin(); it != zmqSocket.end(); ++it) {
			(*it)->Close();
			delete(*it);
		}
		zmqSocket.clear();

		client_downstream = false;
		cout << "Destroyed Receiving Data Socket(s)" << endl;
		return OK;
	}

	cprintf(MAGENTA, "Going to create data sockets\n");

	int numSockets            = detectors.size();
	int numSocketsPerDetector = 1;
	if (getDetectorsType() == EIGER) {
		numSocketsPerDetector = 2;
	}
	numSockets *= numSocketsPerDetector;

	for (int i = 0; i < numSockets; ++i) {
		uint32_t portnum = 0;
		sscanf(detectors[i / numSocketsPerDetector]->getClientStreamingPort().c_str(),
				"%d", &portnum);
		portnum += (i % numSocketsPerDetector);
		//cout<<"ip to be set to :"<<detectors[i/numSocketsPerDetector]
		//->getClientStreamingIP().c_str()<<endl;
		try {
			ZmqSocket* z = new ZmqSocket(
					detectors[i / numSocketsPerDetector]->getClientStreamingIP().c_str(),
					portnum);
			zmqSocket.push_back(z);
			printf("Zmq Client[%d] at %s\n", i, z->GetZmqServerAddress());
		} catch (...) {
			cprintf(RED, "Error: Could not create Zmq socket on port %d\n", portnum);
			createReceivingDataSockets(true);
			return FAIL;
		}
	}

	client_downstream = true;
	cout << "Receiving Data Socket(s) created" << endl;
	return OK;
}

void multiSlsDetector::readFrameFromReceiver() {

	int nX               = thisMultiDetector->numberOfDetector[X]; // to copy data in multi module
	int nY               = thisMultiDetector->numberOfDetector[Y]; // for eiger, to reverse the data
	bool gappixelsenable = false;
	bool eiger           = false;
	if (getDetectorsType() == EIGER) {
		eiger = true;
		nX *= 2;
		gappixelsenable = detectors[0]->enableGapPixels(-1) >= 1 ? true : false;
	}

	bool runningList[zmqSocket.size()], connectList[zmqSocket.size()];
	int numRunning = 0;
	for (unsigned int i = 0; i < zmqSocket.size(); ++i) {
		if (!zmqSocket[i]->Connect()) {
			connectList[i] = true;
			runningList[i] = true;
			++numRunning;
		} else {
			// to remember the list it connected to, to disconnect later
			connectList[i] = false;
			cprintf(RED, "Error: Could not connect to socket  %s\n",
					zmqSocket[i]->GetZmqServerAddress());
			runningList[i] = false;
		}
	}
	int numConnected     = numRunning;
	bool data            = false;
	char* image          = NULL;
	char* multiframe     = NULL;
	char* multigappixels = NULL;
	int multisize        = 0;
	// only first message header
	uint32_t size = 0, nPixelsX = 0, nPixelsY = 0, dynamicRange = 0;
	float bytesPerPixel = 0;
	// header info every header
	std::string currentFileName           = "";
	uint64_t currentAcquisitionIndex = -1, currentFrameIndex = -1, currentFileIndex = -1;
	uint32_t currentSubFrameIndex = -1, coordX = -1, coordY = -1, flippedDataX = -1;

	//wait for real time acquisition to start
	bool running = true;
	sem_wait(&sem_newRTAcquisition);
	if (checkJoinThread())
		running = false;

	//exit when checkJoinThread() (all sockets done)
	while (running) {

		// reset data
		data = false;
		if (multiframe != NULL)
			memset(multiframe, 0xFF, multisize);

		//get each frame
		for (unsigned int isocket = 0; isocket < zmqSocket.size(); ++isocket) {

			//if running
			if (runningList[isocket]) {

				// HEADER
				{
					rapidjson::Document doc;
					if (!zmqSocket[isocket]->ReceiveHeader(isocket, doc,
							SLS_DETECTOR_JSON_HEADER_VERSION)) {
						// parse error, version error or end of acquisition for socket
						runningList[isocket] = false;
						--numRunning;
						continue;
					}

					// if first message, allocate (all one time stuff)
					if (image == NULL) {
						// allocate
						size       = doc["size"].GetUint();
						multisize  = size * zmqSocket.size();
						image      = new char[size];
						multiframe = new char[multisize];
						memset(multiframe, 0xFF, multisize);
						// dynamic range
						dynamicRange  = doc["bitmode"].GetUint();
						bytesPerPixel = (float)dynamicRange / 8;
						// shape
						nPixelsX = doc["shape"][0].GetUint();
						nPixelsY = doc["shape"][1].GetUint();

#ifdef VERBOSE
						cprintf(BLUE, "(Debug) One Time Header Info:\n"
								"size: %u\n"
								"multisize: %u\n"
								"dynamicRange: %u\n"
								"bytesPerPixel: %f\n"
								"nPixelsX: %u\n"
								"nPixelsY: %u\n",
								size, multisize, dynamicRange, bytesPerPixel,
								nPixelsX, nPixelsY);
#endif
					}
					// each time, parse rest of header
					currentFileName         = doc["fname"].GetString();
					currentAcquisitionIndex = doc["acqIndex"].GetUint64();
					currentFrameIndex       = doc["fIndex"].GetUint64();
					currentFileIndex        = doc["fileIndex"].GetUint64();
					currentSubFrameIndex    = doc["expLength"].GetUint();
					coordY                  = doc["row"].GetUint();
					coordX                  = doc["column"].GetUint();
					if (eiger)
						coordY = (nY - 1) - coordY;
						//cout << "X:" << doc["row"].GetUint() <<" Y:"<<doc["column"].GetUint();
					flippedDataX = doc["flippedDataX"].GetUint();
#ifdef VERBOSE
					cprintf(BLUE, "(Debug) Header Info:\n"
							"currentFileName: %s\n"
							"currentAcquisitionIndex: %lu\n"
							"currentFrameIndex: %lu\n"
							"currentFileIndex: %lu\n"
							"currentSubFrameIndex: %u\n"
							"coordX: %u\n"
							"coordY: %u\n"
							"flippedDataX: %u\n",
							currentFileName.c_str(), currentAcquisitionIndex,
							currentFrameIndex, currentFileIndex, currentSubFrameIndex,
							coordX, coordY,
							flippedDataX);
#endif
				}

				// DATA
				data = true;
				zmqSocket[isocket]->ReceiveData(isocket, image, size);

				// creating multi image
				{
					uint32_t xoffset            = coordX * nPixelsX * bytesPerPixel;
					uint32_t yoffset            = coordY * nPixelsY;
					uint32_t singledetrowoffset = nPixelsX * bytesPerPixel;
					uint32_t rowoffset          = nX * singledetrowoffset;
#ifdef VERBOSE
					cprintf(BLUE, "(Debug) Multi Image Info:\n"
							"xoffset: %u\n"
							"yoffset: %u\n"
							"singledetrowoffset: %u\n"
							"rowoffset: %u\n",
							xoffset, yoffset, singledetrowoffset, rowoffset);
#endif
					if (eiger && flippedDataX) {
						for (uint32_t i = 0; i < nPixelsY; ++i) {
							memcpy(((char*)multiframe) +
									((yoffset + (nPixelsY - 1 - i)) * rowoffset) + xoffset,
									(char*)image + (i * singledetrowoffset),
									singledetrowoffset);
						}
					} else {
						for (uint32_t i = 0; i < nPixelsY; ++i) {
							memcpy(((char*)multiframe) +
									((yoffset + i) * rowoffset) + xoffset,
									(char*)image + (i * singledetrowoffset),
									singledetrowoffset);
						}
					}
				}
			}
		}

		//send data to callback
		if (data) {
			// 4bit gap pixels
			if (dynamicRange == 4 && gappixelsenable) {
				int n    = processImageWithGapPixels(multiframe, multigappixels);
				nPixelsX = thisMultiDetector->numberOfChannelInclGapPixels[X];
				nPixelsY = thisMultiDetector->numberOfChannelInclGapPixels[Y];
				thisData = new detectorData(NULL, NULL, NULL, getCurrentProgress(),
						currentFileName.c_str(), nPixelsX, nPixelsY,
						multigappixels, n, dynamicRange, currentFileIndex);
			}
			// normal pixels
			else {
				thisData = new detectorData(NULL, NULL, NULL, getCurrentProgress(),
						currentFileName.c_str(), nPixelsX, nPixelsY,
						multiframe, multisize, dynamicRange, currentFileIndex);
			}
			dataReady(thisData, currentFrameIndex,
					((dynamicRange == 32) ? currentSubFrameIndex : -1),
					pCallbackArg);
			delete thisData;
			setCurrentProgress(currentAcquisitionIndex + 1);
		}

		//all done
		if (!numRunning) {
			// let main thread know that all dummy packets have been received
			//(also from external process),
			// main thread can now proceed to measurement finished call back
			sem_post(&sem_endRTAcquisition);
			// wait for next scan/measurement, else join thread
			sem_wait(&sem_newRTAcquisition);
			//done with complete acquisition
			if (checkJoinThread())
				running = false;
			else {
				//starting a new scan/measurement (got dummy data)
				for (unsigned int i = 0; i < zmqSocket.size(); ++i)
					runningList[i] = connectList[i];
				numRunning = numConnected;
			}
		}
	}

	// Disconnect resources
	for (unsigned int i = 0; i < zmqSocket.size(); ++i)
		if (connectList[i])
			zmqSocket[i]->Disconnect();

	//free resources
	if (image != NULL)
		delete[] image;
	if (multiframe != NULL)
		delete[] multiframe;
	if (multigappixels != NULL)
		delete[] multigappixels;
}

int multiSlsDetector::processImageWithGapPixels(char* image, char*& gpImage) {
	// eiger 4 bit mode
	int nxb          = thisMultiDetector->numberOfDetector[X] * (512 + 3);
	int nyb          = thisMultiDetector->numberOfDetector[Y] * (256 + 1);
	int gapdatabytes = nxb * nyb;

	int nxchip = thisMultiDetector->numberOfDetector[X] * 4;
	int nychip = thisMultiDetector->numberOfDetector[Y] * 1;

	// allocate
	if (gpImage == NULL)
		gpImage = new char[gapdatabytes];
	// fill value
	memset(gpImage, 0xFF, gapdatabytes);

	const int b1chipx = 128;
	const int b1chipy = 256;
	char* src         = 0;
	char* dst         = 0;

	// copying line by line
	src = image;
	dst = gpImage;
	for (int row = 0; row < nychip; ++row) { // for each chip in a row
		for (int ichipy = 0; ichipy < b1chipy; ++ichipy) { //for each row in a chip
			for (int col = 0; col < nxchip; ++col) {
				memcpy(dst, src, b1chipx);
				src += b1chipx;
				dst += b1chipx;
				if ((col + 1) % 4)
					++dst;
			}
		}

		dst += (2 * nxb);
	}

	// vertical filling of values
	{
		uint8_t temp, g1, g2;
		int mod;
		dst = gpImage;
		for (int row = 0; row < nychip; ++row) { // for each chip in a row
			for (int ichipy = 0; ichipy < b1chipy; ++ichipy) { //for each row in a chip
				for (int col = 0; col < nxchip; ++col) {
					dst += b1chipx;
					mod = (col + 1) % 4;
					// copy gap pixel(chip 0, 1, 2)
					if (mod) {
						// neighbouring gap pixels to left
						temp                     = (*((uint8_t*)(dst - 1)));
						g1                       = ((temp & 0xF) / 2);
						(*((uint8_t*)(dst - 1))) = (temp & 0xF0) + g1;

						// neighbouring gap pixels to right
						temp                     = (*((uint8_t*)(dst + 1)));
						g2                       = ((temp >> 4) / 2);
						(*((uint8_t*)(dst + 1))) = (g2 << 4) + (temp & 0x0F);

						// gap pixels
						(*((uint8_t*)dst)) = (g1 << 4) + g2;

						// increment to point to proper chip destination
						++dst;
					}
				}
			}

			dst += (2 * nxb);
		}
	}

	//return gapdatabytes;
	// horizontal filling
	{
		uint8_t temp, g1, g2;
		char* dst_prevline = 0;
		dst                = gpImage;
		for (int row = 0; row < nychip; ++row) { // for each chip in a row
			dst += (b1chipy * nxb);
			// horizontal copying of gap pixels from neighboring past line (bottom parts)
			if (row < nychip - 1) {
				dst_prevline = dst - nxb;
				for (int gapline = 0; gapline < nxb; ++gapline) {
					temp                        = (*((uint8_t*)dst_prevline));
					g1                          = ((temp >> 4) / 2);
					g2                          = ((temp & 0xF) / 2);
					(*((uint8_t*)dst_prevline)) = (g1 << 4) + g2;
					(*((uint8_t*)dst))          = (*((uint8_t*)dst_prevline));
					++dst;
					++dst_prevline;
				}
			}

			// horizontal copying of gap pixels from neihboring future line (top part)
			if (row > 0) {
				dst -= ((b1chipy + 1) * nxb);
				dst_prevline = dst + nxb;
				for (int gapline = 0; gapline < nxb; ++gapline) {
					temp                        = (*((uint8_t*)dst_prevline));
					g1                          = ((temp >> 4) / 2);
					g2                          = ((temp & 0xF) / 2);
					temp                        = (g1 << 4) + g2;
					(*((uint8_t*)dst_prevline)) = temp;
					(*((uint8_t*)dst))          = temp;
					++dst;
					++dst_prevline;
				}
				dst += ((b1chipy + 1) * nxb);
			}

			dst += nxb;
		}
	}

	return gapdatabytes;
}


int multiSlsDetector::enableWriteToFile(int enable) {
	return callDetectorMember(&slsDetector::enableWriteToFile, enable);
}

int multiSlsDetector::overwriteFile(int enable) {
	return callDetectorMember(&slsDetector::overwriteFile, enable);
}


int multiSlsDetector::setReadReceiverFrequency(int freq) {
	return callDetectorMember(&slsDetector::setReadReceiverFrequency, freq);
}

int multiSlsDetector::setReceiverReadTimer(int time_in_ms) {
	return callDetectorMember(&slsDetector::setReceiverReadTimer, time_in_ms);
}

int multiSlsDetector::enableDataStreamingToClient(int enable) {
	if (enable >= 0) {
		//destroy data threads
		if (!enable)
			createReceivingDataSockets(true);
		//create data threads
		else {
			if (createReceivingDataSockets() == FAIL) {
				std::cout << "Could not create data threads in client." << std::endl;
				//only for the first det as theres no general one
				setErrorMask(getErrorMask() | (1 << 0));
				detectors[0]->setErrorMask((detectors[0]->getErrorMask()) |
						(DATA_STREAMING));
			}
		}
	}
	return client_downstream;
}

int multiSlsDetector::enableDataStreamingFromReceiver(int enable) {
	if (enable >= 0) {
		thisMultiDetector->receiver_upstream = parallelCallDetectorMember(
				&slsDetector::enableDataStreamingFromReceiver, enable);
	}
	return thisMultiDetector->receiver_upstream;
}

int multiSlsDetector::enableReceiverCompression(int i) {
	return callDetectorMember(&slsDetector::enableReceiverCompression, i);
}

int multiSlsDetector::enableTenGigabitEthernet(int i) {
	return callDetectorMember(&slsDetector::enableTenGigabitEthernet, i);
}

int multiSlsDetector::setReceiverFifoDepth(int i) {
	return callDetectorMember(&slsDetector::setReceiverFifoDepth, i);
}

int multiSlsDetector::setReceiverSilentMode(int i) {
	return callDetectorMember(&slsDetector::setReceiverSilentMode, i);
}

int multiSlsDetector::setCTBPattern(std::string fname) {
	uint64_t word;
	int addr = 0;
	FILE* fd = fopen(fname.c_str(), "r");
	if (fd) {
		while (fread(&word, sizeof(word), 1, fd)) {
			for (unsigned int idet = 0; idet < detectors.size(); ++idet)
				detectors[idet]->setCTBWord(addr, word);
			++addr;
		}

		fclose(fd);
	} else
		return -1;

	return addr;
}


uint64_t multiSlsDetector::setCTBWord(int addr, uint64_t word) {
	return callDetectorMember(&slsDetector::setCTBWord, addr, word);
}

int multiSlsDetector::setCTBPatLoops(int level, int& start, int& stop, int& n) {
	int ret = -100, ret1;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->setCTBPatLoops(level, start, stop, n);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
		if (ret == -100)
			ret = ret1;
		else if (ret != ret1)
			ret = -1;
	}
	return ret;
}

int multiSlsDetector::setCTBPatWaitAddr(int level, int addr) {
	return callDetectorMember(&slsDetector::setCTBPatWaitAddr, level, addr);
}

int multiSlsDetector::setCTBPatWaitTime(int level, uint64_t t) {
	return callDetectorMember(&slsDetector::setCTBPatWaitTime, level, t);
}
