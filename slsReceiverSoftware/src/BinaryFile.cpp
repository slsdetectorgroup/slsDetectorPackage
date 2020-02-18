/************************************************
 * @file BinaryFile.cpp
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFile.h"
#include "Fifo.h"
#include "receiver_defs.h"

#include <iostream>


FILE* BinaryFile::masterfd = nullptr;

BinaryFile::BinaryFile(int ind, uint32_t* maxf,
		int* nd, std::string* fname, std::string* fpath, uint64_t* findex, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
		bool* smode):
		File(ind, maxf, nd, fname, fpath, findex, owenable, dindex, nunits, nf, dr, portno, smode),
		filefd(nullptr),
		numFramesInFile(0),
		numActualPacketsInFile(0)
{
#ifdef VERBOSE
	PrintMembers();
#endif
}

BinaryFile::~BinaryFile() {
	CloseAllFiles();
}

void BinaryFile::PrintMembers(TLogLevel level) {
	File::PrintMembers(level);
	FILE_LOG(logINFO) << "Max Frames Per File: " << *maxFramesPerFile;
	FILE_LOG(logINFO) << "Number of Frames in File: " << numFramesInFile;
}

slsDetectorDefs::fileFormat BinaryFile::GetFileType() {
	return BINARY;
}


void BinaryFile::CreateFile() {
	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	currentFileName = BinaryFileStatic::CreateFileName(*filePath, *fileNamePrefix, *fileIndex,
			 subFileIndex, *detIndex, *numUnitsPerDetector, index);

	BinaryFileStatic::CreateDataFile(filefd, *overWriteEnable, currentFileName);

	if(!(*silentMode)) {
		FILE_LOG(logINFO) << "[" << *udpPortNumber << "]: Binary File created: " << currentFileName;
	}
}

void BinaryFile::CloseCurrentFile() {
	BinaryFileStatic::CloseDataFile(filefd);
}

void BinaryFile::CloseAllFiles() {
	BinaryFileStatic::CloseDataFile(filefd);
	if (master && (*detIndex==0))
		BinaryFileStatic::CloseDataFile(masterfd);
}

void BinaryFile::WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) {
	// check if maxframesperfile = 0 for infinite
	if ((*maxFramesPerFile) && (numFramesInFile >= (*maxFramesPerFile))) {
		CloseCurrentFile();
		++subFileIndex;
		CreateFile();
	}
	numFramesInFile++;
	numActualPacketsInFile += nump;

	// write to file
	int ret = 0;

	// contiguous bitset
	if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
		ret = BinaryFileStatic::WriteDataFile(filefd, buffer, buffersize);
	}

	// not contiguous bitset
	else {
		// write detector header
		ret = BinaryFileStatic::WriteDataFile(filefd, buffer, sizeof(sls_detector_header));

		// get contiguous representation of bit mask
		bitset_storage storage;
		memset(storage, 0 , sizeof(bitset_storage));
		sls_bitset bits = *(sls_bitset*)(buffer + sizeof(sls_detector_header));
		for (int i = 0; i < MAX_NUM_PACKETS; ++i)
			storage[i >> 3] |= (bits[i] << (i & 7));
		// write bitmask
		ret += BinaryFileStatic::WriteDataFile(filefd, (char*)storage, sizeof(bitset_storage));

		// write data
		ret += BinaryFileStatic::WriteDataFile(filefd,
		buffer + sizeof(sls_detector_header), buffersize - sizeof(sls_receiver_header));
}

	// if write error
    if (ret != buffersize) {
    	throw sls::RuntimeError(std::to_string(index) + " : Write to file failed for image number " + std::to_string(fnum));
    }
}


void BinaryFile::CreateMasterFile(bool mfwenable, masterAttributes& attr) {
	//beginning of every acquisition
	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	if (mfwenable && master && (*detIndex==0)) {
		masterFileName = BinaryFileStatic::CreateMasterFileName(*filePath,
				*fileNamePrefix, *fileIndex);
		if(!(*silentMode)) {
			FILE_LOG(logINFO) << "Master File: " << masterFileName;
		}
		attr.version = BINARY_WRITER_VERSION;
		BinaryFileStatic::CreateMasterDataFile(masterfd, masterFileName,
				*overWriteEnable, attr);
	}
}


