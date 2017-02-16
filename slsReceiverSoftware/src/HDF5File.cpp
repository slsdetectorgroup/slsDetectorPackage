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
using namespace std;


pthread_mutex_t HDF5File::Mutex = PTHREAD_MUTEX_INITIALIZER;
H5File* HDF5File::masterfd = 0;
H5File* HDF5File::virtualfd = 0;

HDF5File::HDF5File(int ind, char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr,
		int nx, int ny):
		File(ind, fname, fpath, findex, frindexenable, owenable, dindex, nunits, nf, dr),
		filefd(0),
		dataspace(0),
		dataset(0),
		datatype(PredType::STD_U16LE),
		nPixelsX(nx),
		nPixelsY(ny)
{
	printf("%d HDF5File constructor\n",index);
#ifdef VERBOSE
	PrintMembers();
#endif
}


HDF5File::~HDF5File() {
	printf("%d HDF5File destructor\n",index);
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
	case 4:		datatype = PredType::STD_U8LE; break;
	case 8: 	datatype = PredType::STD_U8LE; break;
	case 16:	datatype = PredType::STD_U16LE; break;
	case 32:	datatype = PredType::STD_U32LE; break;
	default:	cprintf(BG_RED,"unknown dynamic range\n");
				datatype = PredType::STD_U16LE; break;
	}
}

int HDF5File::CreateFile(uint64_t fnum) {
	currentFileName = CreateFileName(filePath, fileNamePrefix, *fileIndex,
			*frameIndexEnable, fnum, *detIndex, *numUnitsPerDetector, index);

	//create file
	UpdateDataType();
	if (CreateDataFile(index, *overWriteEnable, *numImages, currentFileName, fnum,
			((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX), nPixelsY,
			datatype, filefd, dataspace, dataset) == FAIL)
		return FAIL;

	printf("%d HDF5 File: %s\n", index, currentFileName.c_str());
	return OK;
}

void HDF5File::CloseCurrentFile() {
	CloseDataFile(filefd, dataspace, dataset);
}

void HDF5File::CloseAllFiles() {
	CloseDataFile(filefd, dataspace, dataset);
	if (master)
		CloseCommonDataFiles();
}


int HDF5File::WriteToFile(char* buffer, int buffersize, uint64_t fnum) {
	if (WriteDataFile(index, buffer, *numImages,
			((*dynamicRange==4) ? (nPixelsX/2) : nPixelsX), nPixelsY, fnum,
			dataspace, dataset, datatype) == OK)
		return OK;
	cprintf(RED,"%d Error: Write to file failed\n", index);
	return FAIL;
}


int HDF5File::CreateCommonFiles(bool en, uint32_t size,
		uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap) {
	if (master) {
		string masterFileName="", virtualFileName="";
		CreateCommonFileNames(masterFileName, virtualFileName, filePath, fileNamePrefix, *fileIndex);
		printf("Master HDF5 File: %s\nVirtual HDF5 File: %s\n", masterFileName.c_str(), virtualFileName.c_str());
		//create common files
		return CreateCommonDataFiles(masterFileName, virtualFileName, *overWriteEnable,
				en, size, nx, ny, at, ap);
	}
	return OK;
}

/*** static function ***/
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

/*** static function ***/
int HDF5File::CreateDataFile(int ind, bool owenable, uint64_t numf, string fname, uint64_t fnum, int nx, int ny,
			DataType dtype, H5File*& fd, DataSpace*& dspace, DataSet*& dset) {
	pthread_mutex_lock(&Mutex);
	try {
		Exception::dontPrint(); //to handle errors

		//file
		FileAccPropList flist;
		flist.setFcloseDegree(H5F_CLOSE_STRONG);
		if(!owenable)
			fd = new H5File( fname.c_str(), H5F_ACC_EXCL, NULL,flist );
		else
			fd = new H5File( fname.c_str(), H5F_ACC_TRUNC, NULL, flist );

		//attributes - version
		double dValue=HDF5_WRITER_VERSION;
		DataSpace dataspace_attr = DataSpace (H5S_SCALAR);
		Attribute attribute = fd->createAttribute("version",PredType::NATIVE_DOUBLE, dataspace_attr);
		attribute.write(PredType::NATIVE_DOUBLE, &dValue);

		//dataspace
		char dsetname[100];
		hsize_t srcdims[3] = {numf,ny,nx};
		dspace = new DataSpace (3,srcdims);
		sprintf(dsetname, "/data_%012lld", (long long int)fnum);

		//dataset
		//create chunked dataset if greater than max_chunked_images
		if(numf > MAX_CHUNKED_IMAGES){
			DSetCreatPropList plist;
			hsize_t chunk_dims[3] ={MAX_CHUNKED_IMAGES, ny, nx};
			plist.setChunk(3, chunk_dims);
			dset = new DataSet (fd->createDataSet(dsetname, dtype, *dspace, plist));
		}else
			dset = new DataSet (fd->createDataSet(dsetname, dtype, *dspace));

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



/*** static function ***/
void HDF5File::CloseDataFile(H5File*& fd, DataSpace*& dp, DataSet*& ds) {
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

/*** static function ***/
int HDF5File::WriteDataFile(int ind, char* buf, uint64_t numImages, int nx, int ny, uint64_t fnum,
		DataSpace* dspace, DataSet* dset, DataType dtype) {
	pthread_mutex_lock(&Mutex);
	hsize_t count[3] = {1, ny,nx};
	hsize_t start[3] = {fnum%numImages, 0 , 0};
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

void HDF5File::CreateCommonFileNames(string& m, string& v, char* fpath, char* fnameprefix, uint64_t findex) {
	ostringstream osfn;
	osfn << fpath << "/" << fnameprefix;
	osfn << "_master";
	osfn << "_" << findex;
	osfn << ".h5";
	m = osfn.str();
	v.assign(m);
	v.replace(v.find("_master", 0), 7, "_virtual");
}

void HDF5File::CloseCommonDataFiles() {
	pthread_mutex_lock(&Mutex);
	try {
		Exception::dontPrint(); //to handle errors
		if(masterfd) {delete masterfd; 	masterfd = 0;}
		if(virtualfd) {delete virtualfd; 	virtualfd = 0;}
	} catch(Exception error) {
		cprintf(RED,"Error in closing common HDF5 handles\n");
		error.printError();
	}
	pthread_mutex_unlock(&Mutex);
}


int HDF5File::CreateCommonDataFiles(string m, string v, bool owenable,
		bool tengigaEnable,	uint32_t imageSize, uint32_t nPixelsX, uint32_t nPixelsY,
		uint64_t acquisitionTime, uint64_t acquisitionPeriod) {
	pthread_mutex_lock(&Mutex);
	try {
		//Exception::dontPrint(); //to handle errors

		FileAccPropList flist;
		flist.setFcloseDegree(H5F_CLOSE_STRONG);
		if(!owenable)
			masterfd = new H5File( m.c_str(), H5F_ACC_EXCL, NULL, flist );
		else
			masterfd = new H5File( m.c_str(), H5F_ACC_TRUNC, NULL, flist );

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
		dataset.write ( dynamicRange, PredType::NATIVE_INT);
		attribute = dataset.createAttribute("unit",strdatatype, dataspace);
		attribute.write(strdatatype, string("bits"));
		//Ten Giga
		iValue = tengigaEnable;
		dataset = group5.createDataSet ( "ten giga enable", PredType::NATIVE_INT, dataspace );
		dataset.write ( &iValue, PredType::NATIVE_INT);
		//Image Size
		dataset = group5.createDataSet ( "image size", PredType::NATIVE_INT, dataspace );
		dataset.write (  &imageSize, PredType::NATIVE_INT);
		attribute = dataset.createAttribute("unit",strdatatype, dataspace);
		attribute.write(strdatatype, string("bytes"));
		//x
		dataset = group5.createDataSet ( "number of pixels in x axis", PredType::NATIVE_INT, dataspace );
		dataset.write ( &nPixelsX, PredType::NATIVE_INT);
		//y
		dataset = group5.createDataSet ( "number of pixels in y axis", PredType::NATIVE_INT, dataspace );
		dataset.write ( &nPixelsY, PredType::NATIVE_INT);
		//Total Frames
		dataset = group5.createDataSet ( "total frames", PredType::STD_U64LE, dataspace );
		dataset.write ( &numImages, PredType::STD_U64LE);
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
		cprintf(RED,"Error in creating common HDF5 handles\n");
		error.printError();
		pthread_mutex_unlock(&Mutex);
		return FAIL;
	}
	pthread_mutex_unlock(&Mutex);

	return OK;
}


//#endif
