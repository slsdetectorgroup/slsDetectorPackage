/************************************************
 * @file File.cpp
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "File.h"

#include <iostream>
using namespace std;


File::File(int ind, uint32_t maxf, const uint32_t* ppf,
		int* nd, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno):
			index(ind),
			maxFramesPerFile(maxf),
			packetsPerFrame(ppf),
			numDetX(nd[0]),
			numDetY(nd[1]),
			fileNamePrefix(fname),
			filePath(fpath),
			fileIndex(findex),
			frameIndexEnable(frindexenable),
			overWriteEnable(owenable),
			detIndex(dindex),
			numUnitsPerDetector(nunits),
			numImages(nf),
			dynamicRange(dr),
			udpPortNumber(portno)

{
	master = index?false:true;
}

File::~File() {}

string File::GetCurrentFileName() {
	return currentFileName;
}

void File::PrintMembers() {
	FILE_LOG(logINFO) << "\nGeneral Writer Variables:" << endl
			<< "Index: " << index << endl
			<< "Max Frames Per File: " << maxFramesPerFile << endl
			<< "Packets per Frame: " << *packetsPerFrame << endl
			<< "Number of Detectors in x dir: " << numDetX << endl
			<< "Number of Detectors in y dir: " << numDetY << endl
			<< "File Name Prefix: " << fileNamePrefix << endl
			<< "File Path: " << filePath << endl
			<< "File Index: " << *fileIndex << endl
			<< "Frame Index Enable: " << *frameIndexEnable << endl
			<< "Over Write Enable: " << *overWriteEnable << endl

			<< "Detector Index: " << *detIndex << endl
			<< "Number of Units Per Detector: " << *numUnitsPerDetector << endl
			<< "Number of Images in Acquisition: " << *numImages << endl
			<< "Dynamic Range: " << *dynamicRange << endl
			<< "UDP Port number: " << *udpPortNumber << endl
			<< "Master File Name: " << masterFileName << endl
			<< "Current File Name: " << currentFileName;
}


void File::GetMemberPointerValues(int* nd, char*& fname, char*& fpath, uint64_t*& findex,
		bool*& frindexenable, bool*& owenable,
		int*& dindex, int*& nunits, uint64_t*& nf, uint32_t*& dr, uint32_t*& portno)
{
	nd[0] = numDetX;
	nd[1] = numDetY;
	fname = fileNamePrefix;
	fpath = filePath;
	findex = fileIndex;
	frindexenable = frameIndexEnable;
	owenable = overWriteEnable;
	dindex = detIndex;
	nunits = numUnitsPerDetector;
	nf = numImages;
	dr = dynamicRange;
	portno = udpPortNumber;
}

void File::SetMaxFramesPerFile(uint32_t maxf) {
	maxFramesPerFile = maxf;
}


void File::SetPacketsPerFrame(const uint32_t* ppf) {
	packetsPerFrame = ppf;
}
