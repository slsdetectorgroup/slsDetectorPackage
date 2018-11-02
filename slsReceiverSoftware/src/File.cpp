/************************************************
 * @file File.cpp
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "File.h"

#include <iostream>


File::File(int ind, uint32_t* maxf,
		int* nd, char* fname, char* fpath, uint64_t* findex, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
		bool* smode):
			index(ind),
			maxFramesPerFile(maxf),
			numDetX(nd[0]),
			numDetY(nd[1]),
			fileNamePrefix(fname),
			filePath(fpath),
			fileIndex(findex),
			overWriteEnable(owenable),
			detIndex(dindex),
			numUnitsPerDetector(nunits),
			numImages(nf),
			dynamicRange(dr),
			udpPortNumber(portno),
			silentMode(smode)

{
	master = index?false:true;
}

File::~File() {}

std::string File::GetCurrentFileName() {
	return currentFileName;
}

void File::PrintMembers() {
	FILE_LOG(logINFO) << "\nGeneral Writer Variables:" << std::endl
			<< "Index: " << index << std::endl
			<< "Max Frames Per File: " << *maxFramesPerFile << std::endl
			<< "Number of Detectors in x dir: " << numDetX << std::endl
			<< "Number of Detectors in y dir: " << numDetY << std::endl
			<< "File Name Prefix: " << fileNamePrefix << std::endl
			<< "File Path: " << filePath << std::endl
			<< "File Index: " << *fileIndex << std::endl
			<< "Over Write Enable: " << *overWriteEnable << std::endl

			<< "Detector Index: " << *detIndex << std::endl
			<< "Number of Units Per Detector: " << *numUnitsPerDetector << std::endl
			<< "Number of Images in Acquisition: " << *numImages << std::endl
			<< "Dynamic Range: " << *dynamicRange << std::endl
			<< "UDP Port number: " << *udpPortNumber << std::endl
			<< "Master File Name: " << masterFileName << std::endl
			<< "Current File Name: " << currentFileName << std::endl
			<< "Silent Mode: " << *silentMode;
}


void File::GetMemberPointerValues(int* nd, uint32_t*& maxf, char*& fname, char*& fpath, uint64_t*& findex, bool*& owenable,
		int*& dindex, int*& nunits, uint64_t*& nf, uint32_t*& dr, uint32_t*& portno)
{
	nd[0] = numDetX;
	nd[1] = numDetY;
	maxf = maxFramesPerFile;
	fname = fileNamePrefix;
	fpath = filePath;
	findex = fileIndex;
	owenable = overWriteEnable;
	dindex = detIndex;
	nunits = numUnitsPerDetector;
	nf = numImages;
	dr = dynamicRange;
	portno = udpPortNumber;
}


