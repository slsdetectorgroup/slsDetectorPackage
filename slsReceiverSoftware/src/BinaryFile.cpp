/************************************************
 * @file BinaryFile.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFile.h"

#include <iostream>
#include <iomanip>
using namespace std;


BinaryFile::BinaryFile(int ind, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint32_t maxf):
		File(ind, fname, fpath, findex, frindexenable, owenable, dindex, nunits),
				maxFramesPerFile(maxf)
{
	printf("%d BinaryFile constructor\n",index);
	PrintMembers();
}

BinaryFile::~BinaryFile() {
	printf("%d BinaryFile destructor\n",index);
}

void BinaryFile::PrintMembers() {
	File::PrintMembers();
	printf("Max Frames Per File: %d\n",maxFramesPerFile);
}

slsReceiverDefs::fileFormat BinaryFile::GetType() {
	return BINARY;
}

void BinaryFile::SetMaxFramesPerFile(uint32_t maxf) {
	maxFramesPerFile = maxf;
}

int BinaryFile::CreateFile(uint64_t fnum) {
	currentFileName = CreateFileName(filePath, fileNamePrefix, *fileIndex,
			*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, index);

	printf("%d Binary File: %s\n", index, currentFileName.c_str());
	return OK;
}


void BinaryFile::CloseFile() {
	printf("%d Closing File: %s\n", index, currentFileName.c_str());
}

string BinaryFile::CreateFileName(char* fpath, char* fnameprefix, uint64_t findex,
		bool frindexenable,	uint64_t fnum, int dindex, int numunits, int unitindex) {
	ostringstream osfn;
	osfn << fpath << "/" << fnameprefix;
	if (dindex >= 0) osfn << "_d" << (dindex * numunits + unitindex);
	if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << fnum;
	osfn << "_" << findex;
	osfn << ".raw";
	return osfn.str();
}

int BinaryFile::CreateDataFile(bool owenable, char* fname) {
	return OK;
}

void BinaryFile::CloseDataFile() {

}
