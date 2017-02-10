/************************************************
 * @file HDF5File.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "HDF5File.h"

#include <iostream>
#include <iomanip>
using namespace std;


HDF5File::HDF5File(int ind, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits):
		File(ind, fname, fpath, findex, frindexenable, owenable, dindex, nunits)
{
	printf("%d HDF5File constructor\n",index);
	PrintMembers();
}


HDF5File::~HDF5File() {
	printf("%d HDF5File destructor\n",index);
}

void HDF5File::PrintMembers() {
	File::PrintMembers();
	printf("\nHDF5 Print Members \n");
}


slsReceiverDefs::fileFormat HDF5File::GetType() {
	return HDF5;
}

int HDF5File::CreateFile(uint64_t fnum) {
	currentFileName = CreateFileName(filePath, fileNamePrefix, *fileIndex,
			*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, index);
	printf("%d HDF5 File: %s\n", index, currentFileName.c_str());
	return OK;
}


void HDF5File::CloseFile() {
}

string HDF5File::CreateFileName(char* fpath, char* fnameprefix, uint64_t findex,
		bool frindexenable,	uint64_t fnum, int dindex, int numunits, int unitindex) {
	ostringstream osfn;
	osfn << fpath << "/" << fnameprefix;
	if (dindex >= 0) osfn << "_d" << (dindex * numunits + unitindex);
	if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << fnum;
	osfn << "_" << findex;
	osfn << ".h5";
	return osfn.str();
}

int HDF5File::CreateDataFile(bool owenable, char* fname) {
	return OK;
}

void HDF5File::CloseDataFile() {

}
