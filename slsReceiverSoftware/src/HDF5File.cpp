/************************************************
 * @file HDF5File.cpp
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
#include "HDF5File.h"
#include "receiver_defs.h"
#include "Fifo.h"

#include <iostream>
#include <iomanip>
#include <libgen.h>			//basename
#include <string.h>


H5File* HDF5File::masterfd = 0;
hid_t HDF5File::virtualfd = 0;



HDF5File::HDF5File(int ind, uint32_t* maxf,
		int* nd, std::string* fname, std::string* fpath, uint64_t* findex, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
		uint32_t nx, uint32_t ny,
		bool* smode):

		File(ind, maxf, nd, fname, fpath, findex, owenable, dindex, nunits, nf, dr, portno, smode),
		filefd(0),
		dataspace(0),
		dataset(0),
		datatype(PredType::STD_U16LE),
		nPixelsX(nx),
		nPixelsY(ny),
		numFramesInFile(0),
		numActualPacketsInFile(0),
		numFilesinAcquisition(0),
		dataspace_para(0),
		extNumImages(0)
{
	PrintMembers();
	dataset_para.clear();
	parameterNames.clear();
	parameterDataTypes.clear();

	parameterNames.push_back("frame number");
	parameterDataTypes.push_back(PredType::STD_U64LE);

	parameterNames.push_back("exp length or sub exposure time");
	parameterDataTypes.push_back(PredType::STD_U32LE);

	parameterNames.push_back("packets caught");
	parameterDataTypes.push_back(PredType::STD_U32LE);

	parameterNames.push_back("bunch id");
	parameterDataTypes.push_back(PredType::STD_U64LE);

	parameterNames.push_back("timestamp");
	parameterDataTypes.push_back(PredType::STD_U64LE);

	parameterNames.push_back("mod id");
	parameterDataTypes.push_back(PredType::STD_U16LE);

	parameterNames.push_back("row");
	parameterDataTypes.push_back(PredType::STD_U16LE);

	parameterNames.push_back("column");
	parameterDataTypes.push_back(PredType::STD_U16LE);

	parameterNames.push_back("reserved");
	parameterDataTypes.push_back(PredType::STD_U16LE);

	parameterNames.push_back("debug");
	parameterDataTypes.push_back(PredType::STD_U32LE);

	parameterNames.push_back("round robin number");
	parameterDataTypes.push_back(PredType::STD_U16LE);

	parameterNames.push_back("detector type");
	parameterDataTypes.push_back(PredType::STD_U8LE);

	parameterNames.push_back("detector header version");
	parameterDataTypes.push_back(PredType::STD_U8LE);

	parameterNames.push_back("packets caught bit mask");
	StrType strdatatype(PredType::C_S1, sizeof(bitset_storage));
	parameterDataTypes.push_back(strdatatype);

}


HDF5File::~HDF5File() {
	CloseAllFiles();
}

void HDF5File::PrintMembers(TLogLevel level) {
	File::PrintMembers();
	UpdateDataType();
	if (datatype == PredType::STD_U8LE) {
		FILE_LOG(level) << "Data Type: 4 or 8";
	} else if (datatype == PredType::STD_U16LE) {
		FILE_LOG(level) << "Data Type: 16";
	} else if (datatype == PredType::STD_U32LE) {
		FILE_LOG(level) << "Data Type: 32";
	} else {
		FILE_LOG(logERROR) << "unknown data type";
	}
}


void HDF5File::SetNumberofPixels(uint32_t nx, uint32_t ny) {
	nPixelsX = nx;
	nPixelsY = ny;
}


slsDetectorDefs::fileFormat HDF5File::GetFileType() {
	return HDF5;
}


void HDF5File::UpdateDataType() {
	switch(*dynamicRange){
	case 16:	datatype = PredType::STD_U16LE;		break;
	case 32:	datatype = PredType::STD_U32LE;		break;
	default:	datatype = PredType::STD_U8LE;		break;
	}
}


void HDF5File::CreateFile() {
	numFilesinAcquisition++;
	numFramesInFile = 0;
	numActualPacketsInFile = 0;
	currentFileName = HDF5FileStatic::CreateFileName(*filePath, *fileNamePrefix, *fileIndex,
			subFileIndex, *detIndex, *numUnitsPerDetector, index);

	//first time
	if(subFileIndex == 0u) 
		UpdateDataType();

	uint64_t framestosave = ((*maxFramesPerFile == 0) ? *numImages : // infinite images
			(((extNumImages - subFileIndex) > (*maxFramesPerFile)) ?  // save up to maximum at a time
					(*maxFramesPerFile) : (extNumImages-subFileIndex)));

	std::lock_guard<std::mutex> lock(mutex);
	HDF5FileStatic::CreateDataFile(index, *overWriteEnable, currentFileName, (*numImages > 1),
			subFileIndex, framestosave, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
			datatype, filefd, dataspace, dataset,
			HDF5_WRITER_VERSION, MAX_CHUNKED_IMAGES,
			dataspace_para,	dataset_para,
			parameterNames, parameterDataTypes);

	if(!(*silentMode)) {
		FILE_LOG(logINFO) << *udpPortNumber << ": HDF5 File created: " << currentFileName;
	}
}


void HDF5File::CloseCurrentFile() {
	{
		std::lock_guard<std::mutex> lock(mutex);
		HDF5FileStatic::CloseDataFile(index, filefd);
	}
	for (unsigned int i = 0; i < dataset_para.size(); ++i)
		delete dataset_para[i];
	dataset_para.clear();
	if(dataspace_para) {delete dataspace_para;dataspace_para=0;}
	if(dataset) {delete dataset;dataset=0;}
	if(dataspace) {delete dataspace;dataspace=0;}
	if(filefd) {delete filefd;filefd=0;}
}


void HDF5File::CloseAllFiles() {
	numFilesinAcquisition = 0;
	{
		std::lock_guard<std::mutex> lock(mutex);
		HDF5FileStatic::CloseDataFile(index, filefd);
		if (master && (*detIndex==0)) {
			HDF5FileStatic::CloseMasterDataFile(masterfd);
			HDF5FileStatic::CloseVirtualDataFile(virtualfd);
		}
	}
	for (unsigned int i = 0; i < dataset_para.size(); ++i)
		delete dataset_para[i];
	dataset_para.clear();
	if(dataspace_para) delete dataspace_para;
	if(dataset) delete dataset;
	if(dataspace) delete dataspace;
	if(filefd) delete filefd;
}


void HDF5File::WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) {

	// check if maxframesperfile = 0 for infinite
	if ((*maxFramesPerFile) && (numFramesInFile >= (*maxFramesPerFile))) {
		CloseCurrentFile();
		++subFileIndex;
		CreateFile();
	}
	numFramesInFile++;
	numActualPacketsInFile += nump;

	std::lock_guard<std::mutex> lock(mutex);

	// extend dataset (when receiver start followed by many status starts (jungfrau)))
	if (fnum >= extNumImages) {
		HDF5FileStatic::ExtendDataset(index, dataspace, dataset,
				dataspace_para, dataset_para, *numImages);
		if (!(*silentMode)) {
			FILE_LOG(logINFO) << index << " Extending HDF5 dataset by " <<
					extNumImages << ", Total x Dimension: " << (extNumImages + *numImages);
		}
		extNumImages += *numImages;
	}

	HDF5FileStatic::WriteDataFile(index, buffer + sizeof(sls_receiver_header),
			// infinite then no need for %maxframesperfile
			((*maxFramesPerFile == 0) ? fnum : fnum%(*maxFramesPerFile)),
			nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
			dataspace, dataset, datatype);

	HDF5FileStatic::WriteParameterDatasets(index, dataspace_para,
				// infinite then no need for %maxframesperfile
				((*maxFramesPerFile == 0) ? fnum : fnum%(*maxFramesPerFile)),
				dataset_para, (sls_receiver_header*) (buffer),
				parameterDataTypes);
}


void HDF5File::CreateMasterFile(bool mfwenable, masterAttributes& attr) {

	//beginning of every acquisition
	numFramesInFile = 0;
	numActualPacketsInFile = 0;
	extNumImages = *numImages;

	if (mfwenable && master && (*detIndex==0)) {
		virtualfd = 0;
		masterFileName = HDF5FileStatic::CreateMasterFileName(*filePath,
				*fileNamePrefix, *fileIndex);
		if(!(*silentMode)) {
			FILE_LOG(logINFO) << "Master File: " << masterFileName;
		}
		std::lock_guard<std::mutex> lock(mutex);
		attr.version = HDF5_WRITER_VERSION;
		HDF5FileStatic::CreateMasterDataFile(masterfd, masterFileName,
				*overWriteEnable, attr);
	}
}


void HDF5File::EndofAcquisition(bool anyPacketsCaught, uint64_t numf) {
	//not created before
	if (!virtualfd && anyPacketsCaught) {

		// called only by the one maser receiver
		if (master && (*detIndex==0)) {

			//only one file and one sub image (link current file in master)
			if (((numFilesinAcquisition == 1) && (numDetY*numDetX) == 1)) {
				LinkVirtualFileinMasterFile();
			}
			//create virutal file
			else{
				CreateVirtualFile(numf);}
		}
	}
	numFilesinAcquisition = 0;
}


// called only by the one maser receiver
void HDF5File::CreateVirtualFile(uint64_t numf) {
	std::lock_guard<std::mutex> lock(mutex);

	std::string vname = HDF5FileStatic::CreateVirtualFileName(*filePath, *fileNamePrefix, *fileIndex);
	if(!(*silentMode)) {
		FILE_LOG(logINFO) << "Virtual File: " << vname;
	}
	HDF5FileStatic::CreateVirtualDataFile(vname,
			virtualfd, masterFileName,
			*filePath, *fileNamePrefix, *fileIndex, (*numImages > 1),
			*detIndex, *numUnitsPerDetector,
			// infinite images in 1 file, then maxfrperfile = numf
			((*maxFramesPerFile == 0) ? numf+1 : *maxFramesPerFile),
			numf+1,
			"data",	datatype,
			numDetY, numDetX, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
			HDF5_WRITER_VERSION,
			parameterNames, parameterDataTypes);
}

// called only by the one maser receiver
void HDF5File::LinkVirtualFileinMasterFile() {
	//dataset name
	std::ostringstream osfn;
	osfn << "/data";
	if ((*numImages > 1)) osfn << "_f" << std::setfill('0') << std::setw(12) << 0;
	std::string dsetname = osfn.str();

	std::lock_guard<std::mutex> lock(mutex);
	HDF5FileStatic::LinkVirtualInMaster(masterFileName, currentFileName,
			dsetname, parameterNames);
}
