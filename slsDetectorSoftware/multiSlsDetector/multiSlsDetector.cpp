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

std::string multiSlsDetector::callDetectorMember(std::string (slsDetector::*somefunc)(std::string),
		std::string s0) {
	std::vector<std::string> return_values(detectors.size(), "");
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		return_values[idet] = (detectors[idet]->*somefunc)(s0);
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return concatenateIfDifferent(return_values);
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return (T)-1;
	} else {
		std::vector<T> return_values(detectors.size(), (T)-1);
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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

std::string multiSlsDetector::parallelCallDetectorMember(std::string (slsDetector::*somefunc)(std::string),
		std::string s0) {
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return "";
	} else {
		std::vector<std::string> return_values(detectors.size(), "");
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			Task* task = new Task(new func1_t<std::string, std::string>(somefunc,
					detectors[idet], s0, &return_values[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();

		return concatenateIfDifferent(return_values);
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

std::string multiSlsDetector::concatenateIfDifferent(const std::vector<std::string>& return_values) {
	std::string concatenatedValue, firstValue;
	bool valueNotSame = false;

	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		if (firstValue.empty()) {
			concatenatedValue = return_values[idet];
			firstValue        = return_values[idet];
		} else {
			concatenatedValue += "+" + return_values[idet];
		}
		if (firstValue != return_values[idet])
			valueNotSame = true;
	}
	if (valueNotSame)
		return concatenatedValue;
	else
		return firstValue;
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
				(offsetX < (x +	detectors[i]->getTotalNumberOfChannelsInclGapPixels(X)))) {
			if (offsetY == -1) {
				channelX = offsetX - x;
				return i;
			} else {
				//check y offset range
				if ((offsetY >= y) &&
						(offsetY < (y +	detectors[i]->getTotalNumberOfChannelsInclGapPixels(Y)))) {
					channelX = offsetX - x;
					channelY = offsetY - y;
					return i;
				}
			}
		}
	}
	return -1;
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

	// single
	{
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			int64_t ret = detectors[imod]->getId(mode, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	return callDetectorMember(&slsDetector::getId, mode, imod);
}


slsDetector* multiSlsDetector::getSlsDetector(unsigned int pos) {
	if (pos >= 0 && pos < detectors.size()) {
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
			<< "\nDate: " << thisMultiDetector->lastDate << std::endl;

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
	thisMultiDetector->dataBytes = 0;
	thisMultiDetector->dataBytesInclGapPixels = 0;
	thisMultiDetector->numberOfChannels = 0;
	thisMultiDetector->numberOfChannel[X] = 0;
	thisMultiDetector->numberOfChannel[Y] = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[X] = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[Y] = 0;
	thisMultiDetector->maxNumberOfChannelsPerDetector[X] = 0;
	thisMultiDetector->maxNumberOfChannelsPerDetector[Y] = 0;
	for (int i = 0; i < MAX_TIMERS; ++i) {
		thisMultiDetector->timerValue[i] = 0;
	}
	thisMultiDetector->currentSettings = GET_SETTINGS;
	thisMultiDetector->currentThresholdEV = -1;
	thisMultiDetector->threadedProcessing = 1;
	thisMultiDetector->tDead = 0;
	thisMultiDetector->acquiringFlag = false;
	thisMultiDetector->receiverOnlineFlag = OFFLINE_FLAG;
	thisMultiDetector->receiver_upstream = false;
}

void multiSlsDetector::initializeMembers(bool verify) {
	//slsDetectorUtils
	stoppedFlag = &thisMultiDetector->stoppedFlag;
	timerValue = thisMultiDetector->timerValue;
	currentSettings = &thisMultiDetector->currentSettings;
	currentThresholdEV = &thisMultiDetector->currentThresholdEV;

	//postprocessing
	threadedProcessing = &thisMultiDetector->threadedProcessing;
	fdata = NULL;
	thisData = NULL;

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
	std::cout << "Adding detector " << s << std::endl;
#endif
	for (std::vector<slsDetector*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		if ((*it)->getHostname((it-detectors.begin())) == s) {
			std::cout << "Detector " << s << "already part of the multiDetector!" << std::endl
					<< "Remove it before adding it back in a new position!" << std::endl;
			return;
		}
	}

	//check entire shared memory if it doesnt exist?? needed?
	//could be that detectors not loaded completely cuz of crash in new slsdetector in initsharedmemory

	// get type by connecting
	detectorType type = slsDetector::getDetectorType(s.c_str(), DEFAULT_PORTNO);
	if (type == GENERIC) {
		std::cout << "Could not connect to Detector " << s << " to determine the type!" << std::endl;
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
		std::cerr << "Failed to initialize thread pool!" << std::endl;
		throw ThreadpoolException();
	case 1:
#ifdef VERBOSE
		std::cout << "Not initializing threads, not multi detector" << std::endl;
#endif
		break;
	default:
#ifdef VERBOSE
		std::cout << "Initialized Threadpool " << threadpool << std::endl;
#endif
		break;
	}
}


void multiSlsDetector::destroyThreadPool() {
	if (threadpool) {
		delete threadpool;
		threadpool = 0;
#ifdef VERBOSE
		std::cout << "Destroyed Threadpool " << threadpool << std::endl;
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
	std::cout << std::endl
			<< "Updating Multi-Detector Offsets" << std::endl;
#endif
	int offsetX = 0, offsetY = 0, numX = 0, numY = 0;
	int maxChanX   = thisMultiDetector->maxNumberOfChannelsPerDetector[X];
	int maxChanY   = thisMultiDetector->maxNumberOfChannelsPerDetector[Y];
	int prevChanX  = 0;
	int prevChanY  = 0;
	bool firstTime = true;

	thisMultiDetector->numberOfChannel[X]    = 0;
	thisMultiDetector->numberOfChannel[Y]    = 0;
	thisMultiDetector->numberOfDetector[X]   = 0;
	thisMultiDetector->numberOfDetector[Y]   = 0;

	// gap pixels
	int offsetX_gp = 0, offsetY_gp = 0, numX_gp = 0, numY_gp = 0;
	int prevChanX_gp = 0, prevChanY_gp = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[X]    = 0;
	thisMultiDetector->numberOfChannelInclGapPixels[Y]    = 0;


	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
#ifdef VERBOSE
		std::cout << "offsetX:" << offsetX << " prevChanX:" << prevChanX << " "
				"offsetY:" << offsetY << " prevChanY:" << prevChanY << std::endl;
		std::cout << "offsetX_gp:" << offsetX_gp << " "
				"prevChanX_gp:" << prevChanX_gp << " "
				"offsetY_gp:" << offsetY_gp << " "
				"prevChanY_gp:" << prevChanY_gp << std::endl;
#endif
		//std::cout<<" totalchan:"<< detectors[idet]->getTotalNumberOfChannels(Y)
		//<<" maxChanY:"<<maxChanY<<std::endl;
		//incrementing in both direction
		if (firstTime) {
			//incrementing in both directions
			firstTime = false;
			if ((maxChanX > 0) && ((offsetX + detectors[idet]->getTotalNumberOfChannels(X))
					> maxChanX))
				std::cout << "\nDetector[" << idet << "] exceeds maximum channels "
				"allowed for complete detector set in X dimension!" << std::endl;
			if ((maxChanY > 0) && ((offsetY + detectors[idet]->getTotalNumberOfChannels(Y))
					> maxChanY))
				std::cout << "\nDetector[" << idet << "] exceeds maximum channels "
				"allowed for complete detector set in Y dimension!" << std::endl;
			prevChanX    = detectors[idet]->getTotalNumberOfChannels(X);
			prevChanY    = detectors[idet]->getTotalNumberOfChannels(Y);
			prevChanX_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			prevChanY_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			numX += detectors[idet]->getTotalNumberOfChannels(X);
			numY += detectors[idet]->getTotalNumberOfChannels(Y);
			numX_gp += detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			numY_gp += detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			++thisMultiDetector->numberOfDetector[X];
			++thisMultiDetector->numberOfDetector[Y];
#ifdef VERBOSE
			std::cout << "incrementing in both direction" << std::endl;
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
			++thisMultiDetector->numberOfDetector[Y];
#ifdef VERBOSE
			std::cout << "incrementing in y direction" << std::endl;
#endif
		}

		//incrementing in x direction
		else {
			if ((maxChanX > 0) &&
					((offsetX + prevChanX + detectors[idet]->getTotalNumberOfChannels(X))
							> maxChanX))
				std::cout << "\nDetector[" << idet << "] exceeds maximum channels "
				"allowed for complete detector set in X dimension!" << std::endl;
			offsetY      = 0;
			offsetY_gp   = 0;
			prevChanY    = detectors[idet]->getTotalNumberOfChannels(Y);
			prevChanY_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
			numY         = 0; //assuming symmetry with this statement.
			//whats on 1st column should be on 2nd column
			numY_gp      = 0;
			offsetX += prevChanX;
			offsetX_gp += prevChanX_gp;
			prevChanX    = detectors[idet]->getTotalNumberOfChannels(X);
			prevChanX_gp = detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			numX += detectors[idet]->getTotalNumberOfChannels(X);
			numX_gp += detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
			++thisMultiDetector->numberOfDetector[X];
#ifdef VERBOSE
			std::cout << "incrementing in x direction" << std::endl;
#endif
		}

		double bytesperchannel = (double)detectors[idet]->getDataBytes() /
				(double)(detectors[idet]->getTotalNumberOfChannels(X)
						* detectors[idet]->getTotalNumberOfChannels(Y));
		detectors[idet]->setDetectorOffset(X, (bytesperchannel >= 1.0) ? offsetX_gp : offsetX);
		detectors[idet]->setDetectorOffset(Y, (bytesperchannel >= 1.0) ? offsetY_gp : offsetY);

#ifdef VERBOSE
		std::cout << "Detector[" << idet << "] has offsets (" <<
				detectors[idet]->getDetectorOffset(X) << ", " <<
				detectors[idet]->getDetectorOffset(Y) << ")" << std::endl;
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
	}
#ifdef VERBOSE
	std::cout << "Number of Channels in X direction:" << thisMultiDetector->numberOfChannel[X] << std::endl;
	std::cout << "Number of Channels in Y direction:" << thisMultiDetector->numberOfChannel[Y] << std::endl
			<< std::endl;
	std::cout << "Number of Channels in X direction with Gap Pixels:" <<
			thisMultiDetector->numberOfChannelInclGapPixels[X] << std::endl;
	std::cout << "Number of Channels in Y direction with Gap Pixels:" <<
			thisMultiDetector->numberOfChannelInclGapPixels[Y] << std::endl
			<< std::endl;
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


int multiSlsDetector::setPort(portType t, int num) {
	return callDetectorMember(&slsDetector::setPort, t, num);
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
				std::cout << std::endl;
				for (int ia = 0; ia < iargval; ia++)
					std::cout << args[ia] << " ??????? ";
				std::cout << std::endl;
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
			"outdir",
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
		std::cout << iline << " " << names[iline] << std::endl;
		strcpy(args[0], names[iline].c_str());
		outfile << names[iline] << " " << cmd->executeLine(1, args, GET_ACTION) << std::endl;
		++iline;

		// hostname of the detectors
		std::cout << iline << " " << names[iline] << std::endl;
		strcpy(args[0], names[iline].c_str());
		outfile << names[iline] << " " << cmd->executeLine(1, args, GET_ACTION) << std::endl;
		++iline;

		// single detector configuration
		for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
			outfile << std::endl;
			ret1 = detectors[idet]->writeConfigurationFile(outfile, idet);
			if (detectors[idet]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << idet));
			if (ret1 == FAIL)
				ret = FAIL;
		}

		outfile << std::endl;
		//other configurations
		while (iline < nvar) {
			std::cout << iline << " " << names[iline] << std::endl;
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->loadSettingsFile(fname, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
	int ret = OK;

	// single
	{
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->saveSettingsFile(fname, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	int ret1 = OK;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->saveSettingsFile(fname, imod);
		if (ret1 == FAIL)
			ret = FAIL;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return ret;
}


int multiSlsDetector::loadCalibrationFile(std::string fname, int imod) {
	int ret = OK;

	// single
	{
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->loadCalibrationFile(fname, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
	int ret = OK;

	// single
	{
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->saveCalibrationFile(fname, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	int ret1 = OK;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->saveCalibrationFile(fname, imod);
		if (ret1 == FAIL)
			ret = FAIL;
		if (detectors[idet]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << idet));
	}
	return ret;
}



slsDetectorDefs::runStatus multiSlsDetector::getRunStatus() {
	runStatus s = IDLE, s1 = IDLE;
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

	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new int(OK);
			Task* task = new Task(new func0_t<int>(&slsDetector::prepareAcquisition,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
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


int multiSlsDetector::startAcquisition() {
	if (getDetectorsType() == EIGER) {
		if (prepareAcquisition() == FAIL)
			return FAIL;
	}

	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new int(OK);
			Task* task = new Task(new func0_t<int>(&slsDetector::startAcquisition,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
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


int multiSlsDetector::stopAcquisition() {
	pthread_mutex_lock(&mg); // locks due to processing thread using threadpool when in use

	int ret = OK;
	int posmin = 0, posmax = detectors.size();


	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new int(OK);
			Task* task = new Task(new func0_t<int>(&slsDetector::stopAcquisition,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
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

	*stoppedFlag = 1;
	pthread_mutex_unlock(&mg);
	return ret;
}



int multiSlsDetector::sendSoftwareTrigger() {
	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new int(OK);
			Task* task = new Task(new func0_t<int>(&slsDetector::sendSoftwareTrigger,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
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



int multiSlsDetector::startAndReadAll() {
	pthread_mutex_lock(&mg);	// locks due to processing thread using threadpool when in use
	if (getDetectorsType() == EIGER) {
		if (prepareAcquisition() == FAIL)
			return FAIL;
	}

	int ret = parallelCallDetectorMember(&slsDetector::startAndReadAll);
	pthread_mutex_unlock(&mg);
	return ret;
}




int multiSlsDetector::startReadOut() {
	int ret = OK, ret1 = OK;

	for (unsigned int i = 0; i < detectors.size(); ++i) {
		ret = detectors[i]->startReadOut();
		if (detectors[i]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << i));
		if (ret != OK)
			ret1 = FAIL;
	}

	return ret1;
}


int multiSlsDetector::readAll() {
	return callDetectorMember(&slsDetector::readAll);
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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

	// single
	{
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			int64_t ret = detectors[imod]->getTimeLeft(index, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
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
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->setDAC(val, idac, mV, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->getADC(idac, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
					<< ret << "," << ret1 << "]" << std::endl;
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
					<< ret << "," << ret1 << "]" << std::endl;
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
					<< ret << "," << ret1 << "]" << std::endl;
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
					<< ret << "," << ret1 << "]" << std::endl;
			setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
		}
	}

	return ret;
}

std::string multiSlsDetector::setNetworkParameter(networkParameter p, std::string s) {

	if (s.find('+') == std::string::npos) {

		if (!threadpool) {
			std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
	int ret = OK;

	// single
	{
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->digitalTest(mode, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	int ret1 = OK;
	for (unsigned int idet = 0; idet < detectors.size(); ++idet) {
		ret1 = detectors[idet]->digitalTest(mode, imod);
		if (ret1 == FAIL)
			ret = FAIL;
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

int multiSlsDetector::setROI(int n, ROI roiLimits[]) {
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
	std::cout << "Setting ROI for " << n << "rois:" << std::endl;
	for (i = 0; i < n; ++i)
		std::cout << i << ":" << roiLimits[i].xmin << "\t" << roiLimits[i].xmax
		<< "\t" << roiLimits[i].ymin << "\t" << roiLimits[i].ymax << std::endl;
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
		std::cout << "Decoded Channel max vals: " << std::endl;
		std::cout << "det:" << idet << "\t" << xmax << "\t" << ymax << "\t"
				<< channelX << "\t" << channelY << std::endl;
#endif
		if (idet == -1) {
			std::cout << "invalid roi" << std::endl;
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
				std::cout << "Decoded Channel min vals: " << std::endl;
				std::cout << "det:" << idet << "\t" << xmin << "\t" << ymin
						<< "\t" << channelX << "\t" << channelY << std::endl;
#endif
				if (idet < 0 || idet >= (int)detectors.size()) {
					std::cout << "invalid roi" << std::endl;
					invalidroi = true;
					break;
				}
				//get last channel for each det in x and y dir
				lastChannelX = (detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X)) - 1;
				lastChannelY = (detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y)) - 1;

				offsetX = detectors[idet]->getDetectorOffset(X);
				offsetY = detectors[idet]->getDetectorOffset(Y);
				//at the end in x dir
				if ((offsetX + lastChannelX) >= xmax)
					lastChannelX = xmax - offsetX;
				//at the end in y dir
				if ((offsetY + lastChannelY) >= ymax)
					lastChannelY = ymax - offsetY;

#ifdef VERBOSE
				std::cout << "lastChannelX:" << lastChannelX << "\t"
						<< "lastChannelY:" << lastChannelY << std::endl;
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
				std::cout << "nroi[idet]:" << nroi[idet] << "\tymin:" << ymin << std::endl;
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
	std::cout << "Setting ROI :" << std::endl;
	for (i = 0; i < detectors.size(); ++i) {
		std::cout << "detector " << i << std::endl;
		for (int j = 0; j < nroi[i]; ++j) {
			std::cout << allroi[i][j].xmin << "\t" << allroi[i][j].xmax << "\t"
					<< allroi[i][j].ymin << "\t" << allroi[i][j].ymax << std::endl;
		}
	}
#endif

	//settings the rois for each detector
	for (unsigned i = 0; i < detectors.size(); ++i) {
#ifdef VERBOSE
		std::cout << "detector " << i << ":" << std::endl;
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


slsDetectorDefs::ROI* multiSlsDetector::getROI(int& n) {

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
				std::cout << "detector " << i << ":" << std::endl;
			//#endif
			for (j = 0; j < index; ++j) {
				//#ifdef VERBOSE
				std::cout << temp[j].xmin << "\t" << temp[j].xmax << "\t"
						<< temp[j].ymin << "\t" << temp[j].ymax << std::endl;
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
	std::cout << "ROI :" << std::endl;
	for (int j = 0; j < n; ++j) {
		std::cout << roiLimits[j].xmin << "\t" << roiLimits[j].xmax << "\t"
				<< roiLimits[j].ymin << "\t" << roiLimits[j].ymax << std::endl;
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
	std::cout << "Combined along x axis Getting ROI :" << std::endl;
	std::cout << "detector " << i << std::endl;
	for (int j = 0; j < n; ++j) {
		std::cout << roiLimits[j].xmin << "\t" << roiLimits[j].xmax << "\t"
				<< roiLimits[j].ymin << "\t" << roiLimits[j].ymax << std::endl;
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

	std::cout << "\nxmin\txmax\tymin\tymax" << std::endl;
	for (i = 0; i < n; ++i)
		std::cout << retval[i].xmin << "\t" << retval[i].xmax << "\t"
		<< retval[i].ymin << "\t" << retval[i].ymax << std::endl;
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
					"writeAdcRegister [" << ret << "," << ret1 << "]" << std::endl;
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
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->setAllTrimbits(val, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->setThresholdTemperature(val, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->setTemperatureControl(val, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		if (imod >= 0) {
			if (imod < 0 || imod >= (int)detectors.size())
				return -1;
			ret = detectors[imod]->setTemperatureEvent(val, imod);
			if (detectors[imod]->getErrorMask())
				setErrorMask(getErrorMask() | (1 << imod));
			return ret;
		}
	}

	// multi
	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
			std::cout << "Error in creating threadpool. Exiting" << std::endl;
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

	printf("Unknown detector for rate correction\n");
	return -1;

}

int multiSlsDetector::getRateCorrection(double& t) {
	if (getDetectorsType() == EIGER) {
		t = getRateCorrectionTau();
		return t;
	}

	printf("Unknown detector for rate correction\n");
	return -1;
}

double multiSlsDetector::getRateCorrectionTau() {

	double ret = -100.0;
	int posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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

	printf("Unknown detector for rate correction\n");
	return -1;
}

int multiSlsDetector::getRateCorrection() {
	if (getDetectorsType() == EIGER) {
		return getRateCorrectionTau();
	}

	printf("Unknown detector for rate correction\n");
	return -1;
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
	std::string s = "";
	return callDetectorMember(&slsDetector::setFilePath, s);
}

std::string multiSlsDetector::setFilePath(std::string s) {
	if (s.empty())
		return getFilePath();
	return parallelCallDetectorMember(&slsDetector::setFilePath, s);
}

std::string multiSlsDetector::getFileName() {
	std::string s = "";
	return callDetectorMember(&slsDetector::setFileName, s);
}

std::string multiSlsDetector::setFileName(std::string s) {
	if (s.empty())
		return getFileName();
	return parallelCallDetectorMember(&slsDetector::setFileName, s);
}

int multiSlsDetector::setReceiverFramesPerFile(int f) {
	if (f < 0)
		callDetectorMember(&slsDetector::setReceiverFramesPerFile, -1);
	return parallelCallDetectorMember(&slsDetector::setReceiverFramesPerFile, f);
}

slsReceiverDefs::frameDiscardPolicy multiSlsDetector::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
	return callDetectorMember(&slsDetector::setReceiverFramesDiscardPolicy, f);
}

int multiSlsDetector::setReceiverPartialFramesPadding(int f) {
	return callDetectorMember(&slsDetector::setReceiverPartialFramesPadding, f);
}

slsReceiverDefs::fileFormat multiSlsDetector::getFileFormat() {
	return callDetectorMember(&slsDetector::setFileFormat, GET_FILE_FORMAT);
}

slsReceiverDefs::fileFormat multiSlsDetector::setFileFormat(fileFormat f) {
	return parallelCallDetectorMember(&slsDetector::setFileFormat, f);
}

int multiSlsDetector::getFileIndex() {
	return callDetectorMember(&slsDetector::setFileIndex, -1);
}

int multiSlsDetector::incrementFileIndex() {
	return parallelCallDetectorMember(&slsDetector::incrementFileIndex);
}

int multiSlsDetector::setFileIndex(int i) {
	return parallelCallDetectorMember(&slsDetector::setFileIndex, i);
}


int multiSlsDetector::startReceiver() {
	int ret    = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new int(OK);
			Task* task = new Task(new func0_t<int>(&slsDetector::startReceiver,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
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

int multiSlsDetector::stopReceiver() {
	int ret = OK;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return FAIL;
	} else {
		int* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new int(OK);
			Task* task = new Task(new func0_t<int>(&slsDetector::stopReceiver,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
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

slsDetectorDefs::runStatus multiSlsDetector::getReceiverStatus() {
	runStatus ret = IDLE;
	int posmin = 0, posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
		return ERROR;
	} else {
		runStatus* iret[posmax - posmin];
		for (int idet = posmin; idet < posmax; ++idet) {
			iret[idet] = new runStatus(ERROR);
			Task* task = new Task(new func0_t<runStatus>(&slsDetector::getReceiverStatus,
					detectors[idet], iret[idet]));
			threadpool->add_task(task);
		}
		threadpool->startExecuting();
		threadpool->wait_for_tasks_to_complete();
		for (int idet = posmin; idet < posmax; ++idet) {
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

	return ret;
}

int multiSlsDetector::getFramesCaughtByAnyReceiver() {
	int ret = 0;

	// return the first one that works
	if (detectors.size()) {
		ret = detectors[0]->getFramesCaughtByReceiver();
		if (detectors[0]->getErrorMask())
			setErrorMask(getErrorMask() | (1 << 0));
		return ret;
	}

	return -1;
}

int multiSlsDetector::getFramesCaughtByReceiver() {

	int ret = 0, ret1 = 0;
	int posmax = detectors.size();

	if (!threadpool) {
		std::cout << "Error in creating threadpool. Exiting" << std::endl;
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
		std::cout << "Destroyed Receiving Data Socket(s)" << std::endl;
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
		//std::cout<<"ip to be set to :"<<detectors[i/numSocketsPerDetector]
		//->getClientStreamingIP().c_str()<<std::endl;
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
	std::cout << "Receiving Data Socket(s) created" << std::endl;
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
						//std::cout << "X:" << doc["row"].GetUint() <<" Y:"<<doc["column"].GetUint();
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
				thisData = new detectorData(getCurrentProgress(),
						currentFileName.c_str(), nPixelsX, nPixelsY,
						multigappixels, n, dynamicRange, currentFileIndex);
			}
			// normal pixels
			else {
				thisData = new detectorData(getCurrentProgress(),
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
	if (enable < 0)
		callDetectorMember(&slsDetector::enableWriteToFile, -1);
	return parallelCallDetectorMember(&slsDetector::enableWriteToFile, enable);
}

int multiSlsDetector::overwriteFile(int enable) {
	if (enable < 0)
		callDetectorMember(&slsDetector::overwriteFile, -1);
	return parallelCallDetectorMember(&slsDetector::overwriteFile, enable);
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
				std::cout  << "Could not create data threads in client." << std::endl;
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
	if (fd > 0) {
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
