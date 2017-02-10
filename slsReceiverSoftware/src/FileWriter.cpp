/************************************************
 * @file FileWriter.h
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "FileWriter.h"

#include <iostream>
using namespace std;


FileWriter::FileWriter(int ind, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits):
			index(ind),
			fileNamePrefix(fname),
			filePath(fpath),
			fileIndex(findex),
			frameIndexEnable(frindexenable),
			overWriteEnable(owenable),
			detIndex(dindex),
			numUnitsPerDetector(nunits)
{
	printf("%d FileWriter constructor\n",index);
}

FileWriter::~FileWriter() {
	printf("%d FileWriter Destructor\n", index);
}

string FileWriter::GetCurrentFileName() {
	return currentFileName;
}

void FileWriter::PrintMembers() {
	printf("\nGeneral Writer Variables:"
			"Index: %d\n"
			"File Name Prefix: %s\n"
			"File Path: %s\n"
			"File Index: %lld\n"
			"Frame Index Enable: %d\n"
			"Over Write Enable: %d\n"
			"Detector Index: %d\n"
			"Number of Units Per Detector: %d\n",
			index,
			fileNamePrefix,
			filePath,
			(long long int)*fileIndex,
			*frameIndexEnable,
			*overWriteEnable,
			*detIndex,
			*numUnitsPerDetector);
}


void FileWriter::GetMemberPointerValues(char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits)
{
	fname = fileNamePrefix;
	fpath = filePath;
	findex = fileIndex;
	frindexenable = frameIndexEnable;
	owenable = overWriteEnable;
	dindex = detIndex;
	nunits = numUnitsPerDetector;
}
