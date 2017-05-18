/************************************************
 * @file File.cpp
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "File.h"
#include "Fifo.h"

#include <iostream>
using namespace std;


File::File(int ind, uint32_t maxf, const uint32_t* ppf,
		int* nd, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno, Fifo*& f):
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
	printf("\nGeneral Writer Variables:"
			"Index: %d\n"
			"Max Frames Per File: %u\n"
			"Packets per Frame: %u\n"
			"Number of Detectors in x dir: %d\n"
			"Number of Detectors in y dir: %d\n"
			"File Name Prefix: %s\n"
			"File Path: %s\n"
			"File Index: %lu\n"
			"Frame Index Enable: %d\n"
			"Over Write Enable: %d\n"

			"Detector Index: %d\n"
			"Number of Units Per Detector: %d\n"
			"Number of Images in Acquisition: %lu\n"
			"Dynamic Range: %u\n"
			"UDP Port number: %u\n"
			"Master File Name: %s\n"
			"Current File Name: %s\n",
			index,
			maxFramesPerFile,
			*packetsPerFrame,
			numDetX,
			numDetY,
			fileNamePrefix,
			filePath,
			*fileIndex,
			(int)*frameIndexEnable,
			(int)*overWriteEnable,

			*detIndex,
			*numUnitsPerDetector,
			*numImages,
			*dynamicRange,
			*udpPortNumber,
			masterFileName.c_str(),
			currentFileName.c_str());
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

void File::SetFifo(Fifo*& f) {
	fifo = f;
}
