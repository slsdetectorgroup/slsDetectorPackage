/************************************************
 * @file BinaryFile.cpp
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFile.h"
#include "Fifo.h"
#include "receiver_defs.h"

#include <iostream>
#include <iomanip>

FILE* BinaryFile::masterfd = nullptr;

BinaryFile::BinaryFile(int ind, uint32_t* maxf,
		int* nd, std::string* fname, std::string* fpath, uint64_t* findex, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
		bool* smode):
		File(ind, maxf, nd, fname, fpath, findex, owenable, dindex, nunits, nf, dr, portno, smode),
		filefd(nullptr),
		numFramesInFile(0),
		numActualPacketsInFile(0),
		maxMasterFileSize(2000)
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
	LOG(logINFO) << "Max Frames Per File: " << *maxFramesPerFile;
	LOG(logINFO) << "Number of Frames in File: " << numFramesInFile;
}

slsDetectorDefs::fileFormat BinaryFile::GetFileType() {
	return BINARY;
}

void BinaryFile::CreateFile() {
	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	std::ostringstream os;
	os << *filePath << "/" << *fileNamePrefix << "_d"
		<< (*detIndex * (*numUnitsPerDetector) + index) << "_f" << subFileIndex << '_'
		<< *fileIndex << ".raw";
	currentFileName = os.str();

	if (!(*overWriteEnable)){
		if (NULL == (filefd = fopen((const char *) currentFileName.c_str(), "wx"))){
			filefd = 0;
			throw sls::RuntimeError("Could not create/overwrite file " + currentFileName);
		}
	} else if (NULL == (filefd = fopen((const char *) currentFileName.c_str(), "w"))){
		filefd = 0;
		throw sls::RuntimeError("Could not create file " + currentFileName);
	}
	//setting to no file buffering
	setvbuf(filefd, NULL, _IONBF, 0);

	if(!(*silentMode)) {
		LOG(logINFO) << "[" << *udpPortNumber << "]: Binary File created: " << currentFileName;
	}
}

void BinaryFile::CloseCurrentFile() {
	if (filefd)
		fclose(filefd);
	filefd = 0;	
}

void BinaryFile::CloseAllFiles() {
	CloseCurrentFile();
	if (master && (*detIndex==0)) {
		if (masterfd)
			fclose(masterfd);
		masterfd = 0;
	}
}

int BinaryFile::WriteData(char* buf, int bsize) {
	if (!filefd)
		return 0;
	return fwrite(buf, 1, bsize, filefd);
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
		ret = BinaryFileStatic::WriteData(buffer, buffersize);
	}

	// not contiguous bitset
	else {
		// write detector header
		ret = BinaryFileStatic::WriteData(buffer, sizeof(sls_detector_header));

		// get contiguous representation of bit mask
		bitset_storage storage;
		memset(storage, 0 , sizeof(bitset_storage));
		sls_bitset bits = *(sls_bitset*)(buffer + sizeof(sls_detector_header));
		for (int i = 0; i < MAX_NUM_PACKETS; ++i)
			storage[i >> 3] |= (bits[i] << (i & 7));
		// write bitmask
		ret += BinaryFileStatic::WriteData((char*)storage, sizeof(bitset_storage));

		// write data
		ret += BinaryFileStatic::WriteData(buffer + sizeof(sls_detector_header), 
			buffersize - sizeof(sls_receiver_header));
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

		std::ostringstream os;
		os << *filePath << "/" << *fileNamePrefix << "_master"
			<< "_" << *fileIndex << ".raw";
		masterFileName = os.str();
		if(!(*silentMode)) {
			LOG(logINFO) << "Master File: " << masterFileName;
		}
		attr.version = BINARY_WRITER_VERSION;
		BinaryFileStatic::CreateMasterDataFile(masterfd, masterFileName,
				*overWriteEnable, attr);
	}
}


