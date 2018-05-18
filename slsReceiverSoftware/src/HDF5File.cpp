/************************************************
 * @file HDF5File.cpp
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
#include "HDF5File.h"
#include "receiver_defs.h"
#include "Fifo.h"

#include <iostream>
#include <iomanip>
#include <libgen.h>			//basename
#include <string.h>
using namespace std;



pthread_mutex_t HDF5File::Mutex = PTHREAD_MUTEX_INITIALIZER;
H5File* HDF5File::masterfd = 0;
hid_t HDF5File::virtualfd = 0;



HDF5File::HDF5File(int ind, uint32_t* maxf,
		int* nd, char* fname, char* fpath, uint64_t* findex, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
		uint32_t nx, uint32_t ny,
		bool* smode):

		File(ind, maxf, nd, fname, fpath, findex, owenable, dindex, nunits, nf, dr, portno, smode),
		filefd(0),
		dataspace(0),
		dataset(0),
		datatype(PredType::STD_U16LE),
		nPixelsX(nx),
		nPixelsY(ny),
		numFramesInFile(0),
		numActualPacketsInFile(0),
		numFilesinAcquisition(0),
		dataspace_para(0)
{
#ifdef VERBOSE
	PrintMembers();
#endif
	for (int i = 0; i < HDF5FileStatic::NumberofParameters; ++i)
		dataset_para[i] = 0;
}


HDF5File::~HDF5File() {
	CloseAllFiles();
}

void HDF5File::PrintMembers() {
	File::PrintMembers();
	UpdateDataType();
	if (datatype == PredType::STD_U8LE) {
		FILE_LOG(logINFO) << "Data Type: 4 or 8";
	} else if (datatype == PredType::STD_U16LE) {
		FILE_LOG(logINFO) << "Data Type: 16";
	} else if (datatype == PredType::STD_U32LE) {
		FILE_LOG(logINFO) << "Data Type: 32";
	} else {
		FILE_LOG(logERROR) << "unknown data type";
	}
}


void HDF5File::SetNumberofPixels(uint32_t nx, uint32_t ny) {
	nPixelsX = nx;
	nPixelsY = ny;
}


slsReceiverDefs::fileFormat HDF5File::GetFileType() {
	return HDF5;
}


void HDF5File::UpdateDataType() {
	switch(*dynamicRange){
	case 16:	datatype = PredType::STD_U16LE;		break;
	case 32:	datatype = PredType::STD_U32LE;		break;
	default:	datatype = PredType::STD_U8LE;		break;
	}
}


int HDF5File::CreateFile(uint64_t fnum) {
	numFilesinAcquisition++;
	numFramesInFile = 0;
	numActualPacketsInFile = 0;
	currentFileName = HDF5FileStatic::CreateFileName(filePath, fileNamePrefix, *fileIndex,
			(*numImages > 1), fnum, *detIndex, *numUnitsPerDetector, index);

	//first time
	if(!fnum) UpdateDataType();

	uint64_t framestosave = ((*maxFramesPerFile == 0) ? *numImages : // infinite images
			(((*numImages - fnum) > (*maxFramesPerFile)) ?  // save up to maximum at a time
					(*maxFramesPerFile) : (*numImages-fnum)));
	pthread_mutex_lock(&Mutex);
	if (HDF5FileStatic::CreateDataFile(index, *overWriteEnable, currentFileName, (*numImages > 1),
			fnum, framestosave, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
			datatype, filefd, dataspace, dataset,
			HDF5_WRITER_VERSION, MAX_CHUNKED_IMAGES,
			dataspace_para,	dataset_para) == FAIL) {
		pthread_mutex_unlock(&Mutex);
		return FAIL;
	}
	pthread_mutex_unlock(&Mutex);
	if (dataspace == NULL)
		cprintf(RED,"Got nothing!\n");

	if(!silentMode)
		FILE_LOG(logINFO) << *udpPortNumber << ": HDF5 File created: " << currentFileName;

	return OK;
}


void HDF5File::CloseCurrentFile() {
	pthread_mutex_lock(&Mutex);
	HDF5FileStatic::CloseDataFile(index, filefd);
	pthread_mutex_unlock(&Mutex);
}


void HDF5File::CloseAllFiles() {
	numFilesinAcquisition = 0;
	pthread_mutex_lock(&Mutex);
	HDF5FileStatic::CloseDataFile(index, filefd);
	if (master && (*detIndex==0)) {
		HDF5FileStatic::CloseMasterDataFile(masterfd);
		HDF5FileStatic::CloseVirtualDataFile(virtualfd);
	}
	pthread_mutex_unlock(&Mutex);
}


int HDF5File::WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) {
	// check if maxframesperfile = 0 for infinite
	if ((*maxFramesPerFile) && (numFramesInFile >= (*maxFramesPerFile))) {
		CloseCurrentFile();
		CreateFile(fnum);
	}
	numFramesInFile++;
	numActualPacketsInFile += nump;
	pthread_mutex_lock(&Mutex);
	if (HDF5FileStatic::WriteDataFile(index, buffer + sizeof(sls_detector_header),
			// infinite then no need for %maxframesperfile
			((*maxFramesPerFile == 0) ? fnum : fnum%(*maxFramesPerFile)),
			nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
			dataspace, dataset, datatype) == OK) {
		sls_detector_header* header = (sls_detector_header*) (buffer);
		/*header->xCoord = ((*detIndex) * (*numUnitsPerDetector) + index); */
		if (HDF5FileStatic::WriteParameterDatasets(index, dataspace_para,
				// infinite then no need for %maxframesperfile
				((*maxFramesPerFile == 0) ? fnum : fnum%(*maxFramesPerFile)),
				dataset_para, header) == OK) {
			pthread_mutex_unlock(&Mutex);
			return OK;
		}
	}
	pthread_mutex_unlock(&Mutex);
	cprintf(RED,"%d Error: Write to file failed\n", index);
	return FAIL;
}


int HDF5File::CreateMasterFile(bool en, uint32_t size,
		uint32_t nx, uint32_t ny, uint64_t at, uint64_t st, uint64_t sp,
		uint64_t ap) {

	//beginning of every acquisition
	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	if (master && (*detIndex==0)) {
		virtualfd = 0;
		masterFileName = HDF5FileStatic::CreateMasterFileName(filePath,
				fileNamePrefix, *fileIndex);
		if(!silentMode)
			FILE_LOG(logINFO) << "Master File: " << masterFileName;
		pthread_mutex_lock(&Mutex);
		int ret = HDF5FileStatic::CreateMasterDataFile(masterfd, masterFileName,
				*overWriteEnable,
				*dynamicRange, en, size, nx, ny, *numImages, at, st, sp, ap,
				HDF5_WRITER_VERSION);
		pthread_mutex_unlock(&Mutex);
		return ret;
	}
	return OK;
}


void HDF5File::EndofAcquisition(bool anyPacketsCaught, uint64_t numf) {
	//not created before
	if (!virtualfd && anyPacketsCaught) {

		//only one file and one sub image (link current file in master)
		if (((numFilesinAcquisition == 1) && (numDetY*numDetX) == 1)) {
			//dataset name
			ostringstream osfn;
			osfn << "/data";
			if ((*numImages > 1)) osfn << "_f" << setfill('0') << setw(12) << 0;
			string dsetname = osfn.str();
			pthread_mutex_lock(&Mutex);
			HDF5FileStatic::LinkVirtualInMaster(masterFileName, currentFileName, dsetname);
			pthread_mutex_unlock(&Mutex);
		}

		//create virutal file
		else
			CreateVirtualFile(numf);
	}
	numFilesinAcquisition = 0;
}


int HDF5File::CreateVirtualFile(uint64_t numf) {
	if (master && (*detIndex==0)) {
		pthread_mutex_lock(&Mutex);
		int ret = HDF5FileStatic::CreateVirtualDataFile(
				virtualfd, masterFileName,
				filePath, fileNamePrefix, *fileIndex, (*numImages > 1),
				*detIndex, *numUnitsPerDetector,
				// infinite images in 1 file, then maxfrperfile = numf
				((*maxFramesPerFile == 0) ? numf+1 : *maxFramesPerFile),
				numf+1,
				"data",	datatype,
				numDetY, numDetX, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
				HDF5_WRITER_VERSION);
		pthread_mutex_unlock(&Mutex);
		return ret;
	}
	return OK;
}
