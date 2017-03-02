/************************************************
 * @file HDF5File.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/

#pragma once
#include "HDF5File.h"
#include "receiver_defs.h"

#include <iostream>
#include <iomanip>
using namespace std;


pthread_mutex_t HDF5File::Mutex = PTHREAD_MUTEX_INITIALIZER;
H5File* HDF5File::masterfd = 0;
hid_t HDF5File::virtualfd = 0;

HDF5File::HDF5File(int ind, int* nd, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, uint32_t maxf, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr,
		uint32_t nx, uint32_t ny):

		File(ind, nd, fname, fpath, findex, frindexenable, owenable, maxf, dindex, nunits, nf, dr),
		filefd(0),
		dataspace(0),
		dataset(0),
		datatype(PredType::STD_U16LE),

		dataspace_para(0),

		para1("sub_frame_number"),
		dataset_para1(0),
		datatype_para1(PredType::STD_U32LE),

		para2("bunch_id"),
		dataset_para2(0),
		datatype_para2(PredType::STD_U64LE),

		nPixelsX(nx),
		nPixelsY(ny),
		numFramesInFile(0)
{
#ifdef VERBOSE
	PrintMembers();
#endif
}


HDF5File::~HDF5File() {
	CloseAllFiles();
}

void HDF5File::PrintMembers() {
	File::PrintMembers();
	UpdateDataType();
	if (datatype == PredType::STD_U8LE) {
		printf("Data Type: 4 or 8\n");
	} else if (datatype == PredType::STD_U16LE) {
		printf("Data Type: 16\n");
	} else if (datatype == PredType::STD_U32LE) {
		printf("Data Type: 32\n");
	} else {
		cprintf(BG_RED,"unknown data type\n");
	}
}

void HDF5File::SetNumberofPixels(uint32_t nx, uint32_t ny) {
	nPixelsX = nx;
	nPixelsY = ny;
}

slsReceiverDefs::fileFormat HDF5File::GetFileType() {
	return HDF5;
}

void HDF5File::UpdateDataType() {
	switch(*dynamicRange){
	case 16:
		datatype = PredType::STD_U16LE;
		break;
	case 32:
		datatype = PredType::STD_U32LE;
		break;
	default:
		datatype = PredType::STD_U8LE;
		break;
	}
}

int HDF5File::CreateFile(uint64_t fnum) {
	numFramesInFile = 0;
	currentFileName = HDF5FileStatic::CreateFileName(filePath, fileNamePrefix, *fileIndex,
			*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, index);

	//first time
	if(!fnum) UpdateDataType();

	uint64_t framestosave = ((*numImages - fnum) > maxFramesPerFile) ? maxFramesPerFile : (*numImages-fnum);
	pthread_mutex_lock(&Mutex);
	if (HDF5FileStatic::CreateDataFile(index, *overWriteEnable, currentFileName, *frameIndexEnable,
			fnum, framestosave, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
			datatype, filefd, dataspace, dataset,
			HDF5_WRITER_VERSION, MAX_CHUNKED_IMAGES,
			dataspace_para,
			para1, dataset_para1, datatype_para1,
			para2, dataset_para2, datatype_para2) == FAIL) {
		pthread_mutex_unlock(&Mutex);
		return FAIL;
	}
	pthread_mutex_unlock(&Mutex);
	printf("%d HDF5 File: %s\n", index, currentFileName.c_str());

	//virtual file
	if (master && (*detIndex==0))
		return CreateVirtualFile(fnum);

	return OK;
}

void HDF5File::CloseCurrentFile() {
	pthread_mutex_lock(&Mutex);
	HDF5FileStatic::CloseDataFile(index, filefd, dataspace, dataset, dataset_para1, dataset_para2);
	if (master && (*detIndex==0)) {
		HDF5FileStatic::CloseVirtualDataFile(virtualfd);
	}
	pthread_mutex_unlock(&Mutex);
}

void HDF5File::CloseAllFiles() {
	pthread_mutex_lock(&Mutex);
	HDF5FileStatic::CloseDataFile(index, filefd, dataspace, dataset, dataset_para1, dataset_para2);
	if (master && (*detIndex==0)) {
		HDF5FileStatic::CloseMasterDataFile(masterfd);
		HDF5FileStatic::CloseVirtualDataFile(virtualfd);
	}
	pthread_mutex_unlock(&Mutex);
}


int HDF5File::WriteToFile(char* buffer, int buffersize, uint64_t fnum) {
	if (numFramesInFile >= maxFramesPerFile) {/**max  multiply by 100?????????????*/
		CloseCurrentFile();
		CreateFile(fnum);
	}
	numFramesInFile++;

	uint32_t snum = (*((uint32_t*)(buffer + FILE_FRAME_HDR_FNUM_SIZE)));
	uint64_t bid = (*((uint64_t*)(buffer + FILE_FRAME_HDR_FNUM_SIZE + FILE_FRAME_HDR_SNUM_SIZE)));
	pthread_mutex_lock(&Mutex);
	if (HDF5FileStatic::WriteDataFile(index, buffer + FILE_FRAME_HEADER_SIZE,
			fnum%maxFramesPerFile, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
			dataspace, dataset, datatype) == OK) {
		if (HDF5FileStatic::WriteParameterDatasets(index, dataspace_para,
				fnum%maxFramesPerFile,
				dataset_para1, datatype_para1, &snum,
				dataset_para2, datatype_para2, &bid) == OK) {
			pthread_mutex_unlock(&Mutex);
			return OK;
		}
	}
	pthread_mutex_unlock(&Mutex);
	cprintf(RED,"%d Error: Write to file failed\n", index);
	return FAIL;
}


int HDF5File::CreateMasterFile(bool en, uint32_t size,
		uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap) {
	if (master && (*detIndex==0)) {
		masterFileName = HDF5FileStatic::CreateMasterFileName(filePath, fileNamePrefix, *fileIndex);
		printf("Master File: %s\n", masterFileName.c_str());
		pthread_mutex_lock(&Mutex);
		int ret = HDF5FileStatic::CreateMasterDataFile(masterfd, masterFileName, *overWriteEnable,
				*dynamicRange, en, size, nx, ny, *numImages, at, ap, HDF5_WRITER_VERSION);
		pthread_mutex_unlock(&Mutex);
		return ret;
	}
	return OK;
}


int HDF5File::CreateVirtualFile(uint64_t fnum) {
	if (master && (*detIndex==0)) {

		//file name
		string virtualFileName = HDF5FileStatic::CreateVirtualFileName(filePath, fileNamePrefix, *fileIndex, *frameIndexEnable, fnum);
		printf("Virtual File: %s\n", virtualFileName.c_str());

		//source file names
		int numReadouts = numDetX * numDetY;
		string fileNames[numReadouts];
		for (int i = 0; i < numReadouts; ++i) {
			fileNames[i] = 	HDF5FileStatic::CreateFileName(filePath, fileNamePrefix, *fileIndex,
					*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, i);
#ifdef VERBOSE
			printf("%d: File Name: %s\n", i, fileNames[i].c_str());
#endif
		}

		//datatype
		hid_t cdatatype;
		switch(*dynamicRange){
		case 16:
			cdatatype = H5T_STD_U16LE;
			break;
		case 32:
			cdatatype = H5T_STD_U32LE;
			break;
		default:
			cdatatype = H5T_STD_U8LE;
			break;
		}

		//source dataset name
		ostringstream osfn;
		osfn << "/data";
		if (*frameIndexEnable) osfn << "_f" << setfill('0') << setw(12) << fnum;
		string srcDatasetName = osfn.str();

		//virtual dataset name
		osfn.str(""); osfn.clear();
		osfn << "/virtual_data";
		if (*frameIndexEnable) osfn << "_f" << setfill('0') << setw(12) << fnum;
		string virtualDatasetName = osfn.str();
		//parameter 1 name
		osfn.str(""); osfn.clear();
		osfn << "/virtual_" << para1;
		if (*frameIndexEnable) osfn << "_f" << setfill('0') << setw(12) << fnum;
		string vpara1DatasetName = osfn.str();

		//parameter 2 name
		osfn.str(""); osfn.clear();
		osfn << "/virtual_" << para2;
		if (*frameIndexEnable) osfn << "_f" << setfill('0') << setw(12) << fnum;
		string vpara2DatasetName = osfn.str();

		uint64_t framestosave = ((*numImages - fnum) > maxFramesPerFile) ? maxFramesPerFile : (*numImages-fnum);

		//create virtual file
		pthread_mutex_lock(&Mutex);
		int ret = HDF5FileStatic::CreateVirtualDataFile(virtualfd, virtualFileName, virtualDatasetName, srcDatasetName,
				para1, vpara1DatasetName, H5T_STD_U32LE,
				para2, vpara2DatasetName, H5T_STD_U64LE,
				numReadouts, fileNames, *overWriteEnable, cdatatype,
				framestosave, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
				framestosave, numDetY * nPixelsY, numDetX * ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX), HDF5_WRITER_VERSION);

		if (ret == OK)
			ret = HDF5FileStatic::LinkVirtualInMaster(masterFileName, virtualFileName, virtualDatasetName, vpara1DatasetName, vpara2DatasetName);

		pthread_mutex_unlock(&Mutex);
		return ret;
	}
	return OK;
}
