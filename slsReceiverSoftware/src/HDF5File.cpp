/************************************************
 * @file HDF5File.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/


//#define HDF5C	/*********************?????????????????* Need to remove this in the header file to o*************************************?????????????????????***/
//#ifdef HDF5C
#include "HDF5File.h"
#include "receiver_defs.h"

#include <iostream>
#include <iomanip>
#include <stdlib.h>	 //malloc
using namespace std;


pthread_mutex_t HDF5File::Mutex = PTHREAD_MUTEX_INITIALIZER;
H5File* HDF5File::masterfd = 0;
hid_t HDF5File::virtualfd = 0;

HDF5File::HDF5File(int ind, int* nd, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr,
		int nx, int ny):
		File(ind, nd, fname, fpath, findex, frindexenable, owenable, dindex, nunits, nf, dr),
		filefd(0),
		dataspace(0),
		dataset(0),
		datatype(PredType::STD_U16LE),
		nPixelsX(nx),
		nPixelsY(ny)
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
	case 4:
		datatype = PredType::STD_U8LE;
		break;
	case 8:
		datatype = PredType::STD_U8LE;
		break;
	case 16:
		datatype = PredType::STD_U16LE;
		break;
	case 32:
		datatype = PredType::STD_U32LE;
		break;
	default:
		cprintf(BG_RED,"unknown dynamic range\n");
		datatype = PredType::STD_U16LE;
		break;
	}
}

int HDF5File::CreateFile(uint64_t fnum) {
	currentFileName = CreateFileName(filePath, fileNamePrefix, *fileIndex,
			*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, index);

	UpdateDataType();

	if (CreateDataFile(index, *overWriteEnable, *numImages, currentFileName, *frameIndexEnable, fnum,
			((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX), nPixelsY,
			datatype, filefd, dataspace, dataset) == FAIL)
		return FAIL;
	printf("%d HDF5 File: %s\n", index, currentFileName.c_str());

	//virtual file
	if (master && (*detIndex==0))
		return CreateVirtualFile(fnum);

	return OK;
}

void HDF5File::CloseCurrentFile() {
	CloseDataFile(index, filefd, dataspace, dataset);
}

void HDF5File::CloseAllFiles() {
	CloseDataFile(index, filefd, dataspace, dataset);
	if (master && (*detIndex==0)) {
		CloseMasterDataFile();
		CloseVirtualDataFile();
	}
}


int HDF5File::WriteToFile(char* buffer, int buffersize, uint64_t fnum) {

	if (WriteDataFile(index, buffer + FILE_FRAME_HEADER_SIZE, *numImages, /** ignoring bunchid?????????? */
			((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX), nPixelsY, fnum,
			dataspace, dataset, datatype) == OK)
		return OK;
	cprintf(RED,"%d Error: Write to file failed\n", index);
	return FAIL;
}


int HDF5File::CreateMasterFile(bool en, uint32_t size,
		uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap) {
	if (master && (*detIndex==0)) {
		masterFileName = CreateMasterFileName(filePath, fileNamePrefix, *fileIndex);
		printf("Master File: %s\n", masterFileName.c_str());
		return CreateMasterDataFile(masterFileName, *overWriteEnable, *dynamicRange, en, size, nx, ny, *numImages, at, ap);
	}
	return OK;
}


int HDF5File::CreateVirtualFile(uint64_t fnum) {
	if (master && (*detIndex==0)) {

		//file name
		string virtualFileName = CreateVirtualFileName(filePath, fileNamePrefix, *fileIndex, *frameIndexEnable, fnum);
		printf("Virtual File: %s\n", virtualFileName.c_str());

		//source file names
		int numReadouts = numDetX * numDetY;
		string fileNames[numReadouts];
		for (int i = 0; i < numReadouts; ++i) {
			fileNames[i] = 	CreateFileName(filePath, fileNamePrefix, *fileIndex,
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
			cdatatype = H5T_STD_U16LE;
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

		//create virtual file
		return CreateVirtualDataFile(virtualFileName, virtualDatasetName, srcDatasetName,
				numReadouts, fileNames, *overWriteEnable, fnum, cdatatype,
				*numImages, nPixelsY, ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
				*numImages, numDetY * nPixelsY, numDetX * ((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX));
	}
	return OK;
}


/*** static functions ***/
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


string HDF5File::CreateMasterFileName(char* fpath, char* fnameprefix, uint64_t findex) {
	ostringstream osfn;
	osfn << fpath << "/" << fnameprefix;
	osfn << "_master";
	osfn << "_" << findex;
	osfn << ".h5";
	return osfn.str();
}


string HDF5File::CreateVirtualFileName(char* fpath, char* fnameprefix, uint64_t findex, bool frindexenable, uint64_t fnum) {
	ostringstream osfn;
	osfn << fpath << "/" << fnameprefix;
	osfn << "_virtual";
	if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << fnum;
	osfn << "_" << findex;
	osfn << ".h5";
	return osfn.str();
}


void HDF5File::CloseDataFile(int ind, H5File*& fd, DataSpace*& dp, DataSet*& ds) {
	pthread_mutex_lock(&Mutex);
	try {
		Exception::dontPrint(); //to handle errors
		if(dp) {delete dp; 	dp = 0;}
		if(ds) {delete ds; 	ds = 0;}
		if(fd) {delete fd; 	fd = 0;}
	} catch(Exception error) {
		cprintf(RED,"Error in closing HDF5 handles\n");
		error.printError();
	}
	pthread_mutex_unlock(&Mutex);
}


void HDF5File::CloseMasterDataFile() {
	pthread_mutex_lock(&Mutex);
	try {
		Exception::dontPrint(); //to handle errors
		if(masterfd) {delete masterfd; 	masterfd = 0;}
	} catch(Exception error) {
		cprintf(RED,"Error in closing master HDF5 handles\n");
		error.printError();
	}
	pthread_mutex_unlock(&Mutex);
}


/**(in C because H5Pset_virtual doesnt exist yet in C++)*/
void HDF5File::CloseVirtualDataFile() {
	pthread_mutex_lock(&Mutex);
	if(virtualfd) {
		if (H5Fclose(virtualfd) < 0 )
			cprintf(RED,"Error in closing virtual HDF5 handles\n");
		virtualfd = 0;
	}
	pthread_mutex_unlock(&Mutex);
}


int HDF5File::WriteDataFile(int ind, char* buf, uint64_t numImages, int nx, int ny, uint64_t fnum,
		DataSpace* dspace, DataSet* dset, DataType dtype) {
	pthread_mutex_lock(&Mutex);
	hsize_t count[3] = {1,ny,nx};
	hsize_t start[3] = {fnum%numImages,0,0};
	hsize_t dims2[2]={ny,nx};
	try{
		Exception::dontPrint(); //to handle errors

		dspace->selectHyperslab( H5S_SELECT_SET, count, start);
		DataSpace memspace(2,dims2);
		dset->write(buf, dtype, memspace, *dspace);
		memspace.close();
	}
	catch(Exception error){
		cprintf(RED,"Error in writing to file in object %d\n",ind);
		error.printError();
		pthread_mutex_unlock(&Mutex);
		return FAIL;
	}
	pthread_mutex_unlock(&Mutex);
	return OK;
}




int HDF5File::CreateMasterDataFile(string fname, bool owenable,
		uint32_t dr, bool tenE,	uint32_t size, uint32_t nx, uint32_t ny, uint64_t nf,
		uint64_t acquisitionTime, uint64_t acquisitionPeriod) {
	pthread_mutex_lock(&Mutex);
	try {
		Exception::dontPrint(); //to handle errors

		FileAccPropList flist;
		flist.setFcloseDegree(H5F_CLOSE_STRONG);
		if(!owenable)
			masterfd = new H5File( fname.c_str(), H5F_ACC_EXCL, NULL, flist );
		else
			masterfd = new H5File( fname.c_str(), H5F_ACC_TRUNC, NULL, flist );

		//variables
		DataSpace dataspace = DataSpace (H5S_SCALAR);
		Attribute attribute;
		DataSet dataset;
		int iValue=0;
		double dValue=0;
		StrType strdatatype(PredType::C_S1,256);

		//create attributes
		//version
		dValue=HDF5_WRITER_VERSION;
		attribute = masterfd->createAttribute("version",PredType::NATIVE_DOUBLE, dataspace);
		attribute.write(PredType::NATIVE_DOUBLE, &dValue);

		//Create a group in the file
		Group group1( masterfd->createGroup( "entry" ) );
		Group group2( group1.createGroup("data") );
		Group group3( group1.createGroup("instrument") );
		Group group4( group3.createGroup("beam") );
		Group group5( group3.createGroup("detector") );
		Group group6( group1.createGroup("sample") );

		//Dynamic Range
		dataset = group5.createDataSet ( "dynamic range", PredType::NATIVE_INT, dataspace );
		dataset.write ( &dr, PredType::NATIVE_INT);
		attribute = dataset.createAttribute("unit",strdatatype, dataspace);
		attribute.write(strdatatype, string("bits"));

		//Ten Giga
		iValue = tenE;
		dataset = group5.createDataSet ( "ten giga enable", PredType::NATIVE_INT, dataspace );
		dataset.write ( &iValue, PredType::NATIVE_INT);

		//Image Size
		dataset = group5.createDataSet ( "image size", PredType::NATIVE_INT, dataspace );
		dataset.write (  &size, PredType::NATIVE_INT);
		attribute = dataset.createAttribute("unit",strdatatype, dataspace);
		attribute.write(strdatatype, string("bytes"));

		//x
		dataset = group5.createDataSet ( "number of pixels in x axis", PredType::NATIVE_INT, dataspace );
		dataset.write ( &nx, PredType::NATIVE_INT);

		//y
		dataset = group5.createDataSet ( "number of pixels in y axis", PredType::NATIVE_INT, dataspace );
		dataset.write ( &ny, PredType::NATIVE_INT);

		//Total Frames
		dataset = group5.createDataSet ( "total frames", PredType::STD_U64LE, dataspace );
		dataset.write ( &nf, PredType::STD_U64LE);

		//Exptime
		dataset = group5.createDataSet ( "exposure time", PredType::STD_U64LE, dataspace );
		dataset.write ( &acquisitionTime, PredType::STD_U64LE);
		attribute = dataset.createAttribute("unit",strdatatype, dataspace);
		attribute.write(strdatatype, string("ns"));

		//Period
		dataset = group5.createDataSet ( "acquisition period", PredType::STD_U64LE, dataspace );
		dataset.write ( &acquisitionPeriod, PredType::STD_U64LE);
		attribute = dataset.createAttribute("unit",strdatatype, dataspace);
		attribute.write(strdatatype, string("ns"));

		//Timestamp
		time_t t = time(0);
		dataset = group5.createDataSet ( "timestamp", strdatatype, dataspace );
		dataset.write ( string(ctime(&t)), strdatatype );

		masterfd->close();

	} catch(Exception error) {
		cprintf(RED,"Error in creating master HDF5 handles\n");
		error.printError();
		pthread_mutex_unlock(&Mutex);
		return FAIL;
	}
	pthread_mutex_unlock(&Mutex);

	return OK;
}



int HDF5File::CreateDataFile(int ind, bool owenable, uint64_t numf, string fname, bool frindexenable, uint64_t fnum, int nx, int ny,
			DataType dtype, H5File*& fd, DataSpace*& dspace, DataSet*& dset) {
	pthread_mutex_lock(&Mutex);
	try {
		Exception::dontPrint(); //to handle errors

		//file
		FileAccPropList fapl;
		fapl.setFcloseDegree(H5F_CLOSE_STRONG);
		if(!owenable)
			fd = new H5File( fname.c_str(), H5F_ACC_EXCL, NULL,fapl );
		else
			fd = new H5File( fname.c_str(), H5F_ACC_TRUNC, NULL, fapl );

		//attributes - version
		double dValue=HDF5_WRITER_VERSION;
		DataSpace dataspace_attr = DataSpace (H5S_SCALAR);
		Attribute attribute = fd->createAttribute("version",PredType::NATIVE_DOUBLE, dataspace_attr);
		attribute.write(PredType::NATIVE_DOUBLE, &dValue);

		//dataspace
		hsize_t srcdims[3] = {numf,ny,nx};
		dspace = new DataSpace (3,srcdims);

		//dataset name
		ostringstream osfn;
		osfn << "/data";
		if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << fnum;
		string dsetname = osfn.str();

		//dataset
		//chunked dataset if greater than max_chunked_images
		if(numf > MAX_CHUNKED_IMAGES){
			DSetCreatPropList plist;
			hsize_t chunk_dims[3] ={MAX_CHUNKED_IMAGES, ny, nx};
			plist.setChunk(3, chunk_dims);
			dset = new DataSet (fd->createDataSet(dsetname.c_str(), dtype, *dspace, plist));
		}else
			dset = new DataSet (fd->createDataSet(dsetname.c_str(), dtype, *dspace));

	}
	catch(Exception error){
		cprintf(RED,"Error in creating HDF5 handles in object %d\n",ind);
		error.printError();
		fd->close();
		pthread_mutex_unlock(&Mutex);
		return FAIL;
	}
	pthread_mutex_unlock(&Mutex);
	return OK;
}



/**(in C because H5Pset_virtual doesnt exist yet in C++)*/
int HDF5File::CreateVirtualDataFile(string virtualfname, string virtualDatasetname, string srcDatasetname,
		int numFiles, string fileNames[], bool owenable, uint64_t fnum, hid_t dtype,
		int srcNDimx, int srcNDimy, int srcNDimz, int dstNDimx, int dstNDimy, int dstNDimz) {

	pthread_mutex_lock(&Mutex);

	//file
	hid_t dfal = H5Pcreate (H5P_FILE_ACCESS);
	if (dfal >= 0) {
		if (H5Pset_fclose_degree (dfal, H5F_CLOSE_STRONG) >= 0) {
			if(!owenable)		virtualfd = H5Fcreate( virtualfname.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, dfal);
			else				virtualfd = H5Fcreate( virtualfname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, dfal);
			if (virtualfd >= 0) {

				//attributes - version
				hid_t dataspace_attr = H5Screate (H5S_SCALAR);
				if (dataspace_attr >= 0) {
					hid_t attrid = H5Acreate2 (virtualfd, "version", H5T_NATIVE_DOUBLE, dataspace_attr, H5P_DEFAULT, H5P_DEFAULT);
					if (attrid >= 0) {
						double attr_data = HDF5_WRITER_VERSION;
						if (H5Awrite (attrid, H5T_NATIVE_DOUBLE, &attr_data) >= 0) {
							if (H5Aclose (attrid) >= 0) {

								//dataspace
								hsize_t vdsdims[3] = {dstNDimx, dstNDimy, dstNDimz};
								hid_t vdsDataspace = H5Screate_simple(3, vdsdims ,NULL);
								if (vdsDataspace >= 0) {
									hsize_t srcdims[3] = {srcNDimx, srcNDimy, srcNDimz};
									hid_t srcDataspace = H5Screate_simple(3, srcdims, NULL);
									if (srcDataspace >= 0) {

										//fill values
										hid_t dcpl = H5Pcreate (H5P_DATASET_CREATE);
										if (dcpl >= 0) {
											int fill_value = -1;
											if (H5Pset_fill_value (dcpl, dtype, &fill_value) >= 0) {

												//hyperslab
												hsize_t offset[3]={0,0,0},count[3]={srcNDimx,srcNDimy, srcNDimz};
												bool error = false;
												for (int i = 0; i < numFiles; i++) {
													//cout<<"("<<offset[0]<<","<<offset[1]<<","<<offset[2]<<")"<<endl;
													if (H5Sselect_hyperslab (vdsDataspace, H5S_SELECT_SET, offset, NULL, count, NULL) < 0) {
														cprintf(RED,"could not select hyperslab\n");
														error = true;
														break;
													}
													if (H5Pset_virtual(dcpl, vdsDataspace, fileNames[i].c_str(), srcDatasetname.c_str(), srcDataspace) < 0) {
														cprintf(RED,"could not set mapping\n");
														error = true;
														break;
													}
													offset[2] += srcNDimz;
													if(offset[2] >= (unsigned int) dstNDimz){
														offset[2] = 0;
														offset[1] += srcNDimy;
													}
												}
												if (!error) {

													//dataset
													hid_t vdsdataset = H5Dcreate2 (virtualfd, virtualDatasetname.c_str(), dtype, vdsDataspace, H5P_DEFAULT, dcpl, H5P_DEFAULT);
													if (vdsdataset >= 0){

														H5Sclose(vdsDataspace);
														H5Sclose(srcDataspace);
														H5Dclose(vdsdataset);
														H5Fclose(virtualfd);

														pthread_mutex_unlock(&Mutex);
														return OK;

													} else cprintf(RED, "could not create virtual dataset in virtual file %s\n", virtualfname.c_str());
												} else cprintf(RED, "could not map files in virtual file %s\n", virtualfname.c_str());
											} else cprintf(RED, "could not fill values in virtual file %s\n", virtualfname.c_str());
										} else cprintf(RED, "could not create dcpl in virtual file %s\n", virtualfname.c_str());
									} else cprintf(RED, "could not create source dataspace in virtual file %s\n", virtualfname.c_str());
								} else cprintf(RED, "could not create virtual dataspace in virtual file %s\n", virtualfname.c_str());
							} else cprintf(RED, "could not close attribute in virtual file %s\n", virtualfname.c_str());
						} else cprintf(RED, "could not write attribute in virtual file %s\n", virtualfname.c_str());
					} else cprintf(RED, "could not create attribute in virtual file %s\n", virtualfname.c_str());
				} else cprintf(RED, "could not create dataspace for attribute in virtual file %s\n", virtualfname.c_str());
			} else cprintf(RED, "could not create virtual file %s\n", virtualfname.c_str());
		} else cprintf(RED, "could not set strong file close degree for virtual file %s\n", virtualfname.c_str());
	} else cprintf(RED, "could not create  dfal for virtual file %s\n", virtualfname.c_str());


	H5Fclose(virtualfd);

	pthread_mutex_unlock(&Mutex);
	return FAIL;
}




template <typename T>
int HDF5File::CopyVirtualFile(bool owenable, string oldFileName, string oldDatasetName,
		string newFileName, string newDatasetName, int nDimx, int nDimy, int nDimz, T datatype) {

	T *data_out	= (T*)malloc(sizeof(T)*(nDimx*nDimy*nDimz));

	H5File* oldfd;
	H5File* newfd;
	pthread_mutex_lock(&Mutex);
	try {
		Exception::dontPrint(); //to handle errors

		//open old file
		oldfd = new H5File( oldFileName.c_str(), H5F_ACC_RDONLY);
		DataSet oldDataset = oldfd->openDataSet( oldDatasetName.c_str());
		//read dataset
		oldDataset.read( data_out, datatype);
		//new file
		FileAccPropList fapl;
		fapl.setFcloseDegree(H5F_CLOSE_STRONG);
		if(!owenable)
			newfd = new H5File( newFileName.c_str(), H5F_ACC_EXCL, NULL,fapl );
		else
			newfd = new H5File( newFileName.c_str(), H5F_ACC_TRUNC, NULL, fapl );
		//dataspace and dataset
		hsize_t dims[3] = {nDimx, nDimy, nDimz};
		DataSpace* newDataspace = new DataSpace (3,dims);
		DataSet* newDataset = new DataSet( newfd->createDataSet(newDatasetName.c_str(), datatype, *newDataspace));
		//write and close
		newDataset->write(data_out,datatype);
		newDataspace->close();
		newDataset->close();
		newfd->close();
		oldDataset.close();
		oldfd->close();
	} catch(Exception error){
		cprintf(RED,"Error in copying virtual files\n");
		error.printError();
		free(data_out);
		oldfd->close();
		newfd->close();
		pthread_mutex_unlock(&Mutex);
		return FAIL;
	}
	free(data_out);
	pthread_mutex_unlock(&Mutex);
	return OK;
}



void  HDF5File::CreateFinalFile() {
	if (master && (*detIndex==0)) {
			CopyVirtualFile(*overWriteEnable, "/home/l_maliakal_d/Software/scratch/run_virtual_f000000000000_0.h5",
					"virtual_data_f000000000000",
					"/home/l_maliakal_d/Software/scratch/copy.h5",
					"run_copy",
					*numImages, numDetY * nPixelsY, numDetX * 	((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX),
					datatype);
	}
}

//#endif
