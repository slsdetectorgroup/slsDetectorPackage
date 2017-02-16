/************************************************
 * @file BinaryFile.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFile.h"
#include "receiver_defs.h"

#include <iostream>
#include <iomanip>
#include <string.h>
using namespace std;


FILE* BinaryFile::masterfd = 0;

BinaryFile::BinaryFile(int ind, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t maxf):
		File(ind, fname, fpath, findex, frindexenable, owenable, dindex, nunits, nf, dr),
		maxFramesPerFile(maxf),
		filefd(0)
{
#ifdef VERBOSE
	PrintMembers();
#endif
}

BinaryFile::~BinaryFile() {
}

void BinaryFile::PrintMembers() {
	File::PrintMembers();
	printf("Max Frames Per File: %d\n",maxFramesPerFile);
}

slsReceiverDefs::fileFormat BinaryFile::GetFileType() {
	return BINARY;
}

void BinaryFile::SetMaxFramesPerFile(uint32_t maxf) {
	maxFramesPerFile = maxf;
}

int BinaryFile::CreateFile(uint64_t fnum) {
	currentFileName = CreateFileName(filePath, fileNamePrefix, *fileIndex,
			*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, index);

	if (CreateDataFile(filefd, *overWriteEnable, currentFileName) == FAIL)
		return FAIL;

	printf("%d Binary File created: %s\n", index, currentFileName.c_str());
	return OK;
}

void BinaryFile::CloseCurrentFile() {
	CloseDataFile(filefd);
}

void BinaryFile::CloseAllFiles() {
	CloseDataFile(filefd);
	if (master)
		CloseCommonDataFiles();
}

int BinaryFile::WriteToFile(char* buffer, int buffersize, uint64_t fnum) {
	if (WriteDataFile(filefd, buffer, buffersize, fnum) == buffersize)
		return OK;
	cprintf(RED,"%d Error: Write to file failed for image number %lld\n", index, (long long int)fnum);
	return FAIL;
}


int BinaryFile::CreateCommonFiles(bool en, uint32_t size,
		uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap) {
	if (master) {
		string masterFileName="";
		CreateCommonFileNames(masterFileName, filePath, fileNamePrefix, *fileIndex);
		printf("Master HDF5 File: %s\n", masterFileName.c_str());
		//create common files
		return CreateCommonDataFiles(masterFileName, *overWriteEnable,
				en, size, nx, ny, at, ap);
	}
	return OK;
}

/*** static function ***/
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

/*** static function ***/
int BinaryFile::CreateDataFile(FILE*& fd, bool owenable, string fname) {
	if(!owenable){
		if (NULL == (fd = fopen((const char *) fname.c_str(), "wx"))){
			FILE_LOG(logERROR) << "Could not create/overwrite file" << fname;
			fd = 0;
			return FAIL;
		}
	}else if (NULL == (fd = fopen((const char *) fname.c_str(), "w"))){
		FILE_LOG(logERROR) << "Could not create file" << fname;
		fd = 0;
		return FAIL;
	}
	//setting file buffer size to 16mb
	setvbuf(fd,NULL,_IOFBF,FILE_BUFFER_SIZE);
	return OK;
}

/*** static function ***/
void BinaryFile::CloseDataFile(FILE*& fd) {
	if (fd)
		fclose(fd);
	fd = 0;
}

/*** static function ***/
int BinaryFile::WriteDataFile(FILE* fd, char* buf, int bsize, uint64_t fnum) {
	if (!fd)
		return 0;
	return fwrite(buf, 1, bsize, fd);
}

void BinaryFile::CreateCommonFileNames(string& m, char* fpath, char* fnameprefix, uint64_t findex) {
	ostringstream osfn;
	osfn << fpath << "/" << fnameprefix;
	osfn << "_master";
	osfn << "_" << findex;
	osfn << ".raw";
	m = osfn.str();
}

void BinaryFile::CloseCommonDataFiles() {
	if(masterfd)
		delete masterfd;
	masterfd = 0;
}


int BinaryFile::CreateCommonDataFiles(string m, bool owenable,
				bool tengigaEnable,	uint32_t imageSize, uint32_t nPixelsX, uint32_t nPixelsY,
				uint64_t acquisitionTime, uint64_t acquisitionPeriod) {
	if(!owenable){
		if (NULL == (masterfd = fopen((const char *) m.c_str(), "wx"))){
			cprintf(RED,"Error in creating binary master file %s\n",m.c_str());
			masterfd = 0;
			return FAIL;
		}
	}else if (NULL == (masterfd = fopen((const char *) m.c_str(), "w"))){
		cprintf(RED,"Error in creating binary master file %s\n",m.c_str());
		masterfd = 0;
		return FAIL;
	}
	time_t t = time(0);
	char message[MAX_STR_LENGTH];
	sprintf(message,
			"Version\t\t: %.1f\n"
			"Dynamic Range\t: %d\n"
			"Ten Giga\t: %d\n"
			"Image Size\t: %d bytes\n"
			"x\t\t: %d pixels\n"
			"y\t\t: %d pixels\n"
			"Total Frames\t: %lld\n"
			"Exptime (ns)\t: %lld\n"
			"Period (ns)\t: %lld\n"
			"Timestamp\t: %s\n\n",
			BINARY_WRITER_VERSION,
			*dynamicRange,
			tengigaEnable,
			imageSize,
			nPixelsX,
			nPixelsY,
			(long long int)numImages,
			(long long int)acquisitionTime,
			(long long int)acquisitionPeriod,
			ctime(&t));
	if (strlen(message) > MAX_STR_LENGTH) {
		cprintf(BG_RED,"Master File Size %d is greater than max str size %d\n",
				(int)strlen(message), MAX_STR_LENGTH);
		return FAIL;
	}

	if (fwrite((void*)message, 1, strlen(message), masterfd) !=  strlen(message))
		return FAIL;

	CloseDataFile(masterfd);
	return OK;
}
