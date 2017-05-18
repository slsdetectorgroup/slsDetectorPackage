/************************************************
 * @file BinaryFile.cpp
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFile.h"
#include "receiver_defs.h"
#include "Fifo.h"

#include <iostream>
using namespace std;


FILE* BinaryFile::masterfd = 0;

BinaryFile::BinaryFile(int ind, uint32_t maxf, const uint32_t* ppf,
		int* nd, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno, Fifo*& f):

		File(ind, maxf, ppf, nd, fname, fpath, findex, frindexenable, owenable, dindex, nunits, nf, dr, portno, f),
		filefd(0),
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

void BinaryFile::PrintMembers() {
	File::PrintMembers();
	printf("Max Frames Per File: %d\n",maxFramesPerFile);
	printf("Number of Frames in File: %d\n",numFramesInFile);
}

slsReceiverDefs::fileFormat BinaryFile::GetFileType() {
	return BINARY;
}


int BinaryFile::CreateFile(uint64_t fnum) {
	//calculate packet loss
	int64_t loss = -1;
	if (numFramesInFile)
		loss = (numFramesInFile*(*packetsPerFrame)) - numActualPacketsInFile;

	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	currentFileName = BinaryFileStatic::CreateFileName(filePath, fileNamePrefix, *fileIndex,
			*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, index);

	if (BinaryFileStatic::CreateDataFile(filefd, *overWriteEnable, currentFileName, FILE_BUFFER_SIZE) == FAIL)
		return FAIL;

	//first file, print entrire path
	if (loss == -1)
		printf("[%u]: Binary File created: %s\n", *udpPortNumber, currentFileName.c_str());
	//other files
	else {
		if (loss)
			cprintf(RED,"[%u]:  Packet_Loss:%lu  Fifo_Max_Level:%d  \tNew_File:%s\n",
					*udpPortNumber,loss, fifo->GetMaxLevelForFifoBound() , basename(currentFileName.c_str()));
		else
			cprintf(GREEN,"[%u]:  Packet_Loss:%lu  Fifo_Max_Level:%d  \tNew_File:%s\n",
					*udpPortNumber,loss, fifo->GetMaxLevelForFifoBound(), basename(currentFileName.c_str()));
	}

	return OK;
}

void BinaryFile::CloseCurrentFile() {
	BinaryFileStatic::CloseDataFile(filefd);
}

void BinaryFile::CloseAllFiles() {
	BinaryFileStatic::CloseDataFile(filefd);
	if (master && (*detIndex==0))
		BinaryFileStatic::CloseDataFile(masterfd);
}

int BinaryFile::WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) {
	if (numFramesInFile >= maxFramesPerFile) {
		CloseCurrentFile();
		CreateFile(fnum);
	}
	numFramesInFile++;
	numActualPacketsInFile += nump;
	if (BinaryFileStatic::WriteDataFile(filefd, buffer, buffersize, fnum) == buffersize)
		return OK;
	cprintf(RED,"%d Error: Write to file failed for image number %lld\n", index, (long long int)fnum);
	return FAIL;
}


int BinaryFile::CreateMasterFile(bool en, uint32_t size,
		uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap) {
	//beginning of every acquisition
	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	if (master && (*detIndex==0)) {
		masterFileName = BinaryFileStatic::CreateMasterFileName(filePath, fileNamePrefix, *fileIndex);
		printf("Master File: %s\n", masterFileName.c_str());
		return BinaryFileStatic::CreateMasterDataFile(masterfd, masterFileName, *overWriteEnable,
				*dynamicRange, en, size, nx, ny, *numImages,
				at, ap, BINARY_WRITER_VERSION);
	}
	return OK;
}


