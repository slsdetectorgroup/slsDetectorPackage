#ifdef HDF5C
#pragma once
/************************************************
 * @file HDF5FileStatic.h
 * @short creating, closing, writing and reading
 * from HDF5 files
 ***********************************************/
/**
 *@short creating, closing, writing and reading from HDF5 files
 */

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif
#include "sls_receiver_defs.h"
#include "logger.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdlib.h>	 //malloc
#include <sstream>
#include <cstring>	//memset

class HDF5FileStatic: public virtual slsReceiverDefs {

public:

	/** Constructor */
	HDF5FileStatic(){};
	/** Destructor */
	virtual ~HDF5FileStatic(){};


	/**
	 * Create File Name in format fpath/fnameprefix_fx_dy_z.raw,
	 * where x is fnum, y is (dindex * numunits + unitindex) and z is findex
	 * @param fpath file path
	 * @param fnameprefix file name prefix (includes scan and position variables)
	 * @param findex file index
	 * @param frindexenable frame index enable
	 * @param fnum frame number index
	 * @param dindex readout index
	 * @param numunits number of units per readout. eg. eiger has 2 udp units per readout
	 * @param unitindex unit index
	 * @returns complete file name created
	 */
	static std::string CreateFileName(char* fpath, char* fnameprefix, uint64_t findex, bool frindexenable,
			uint64_t fnum = 0, int dindex = -1, int numunits = 1, int unitindex = 0)
	{
		std::ostringstream osfn;
		osfn << fpath << "/" << fnameprefix;
		if (dindex >= 0) osfn << "_d" <<  (dindex * numunits + unitindex);
		if (frindexenable) osfn << "_f" << std::setfill('0') << std::setw(12) << fnum;
		osfn << "_" << findex;
		osfn << ".h5";
		return osfn.str();
	}

	/**
	 * Create master file name
	 * @param fpath file path
	 * @param fnameprefix file name prefix (includes scan and position variables)
	 * @param findex file index
	 * @returns master file name
	 */
	static std::string CreateMasterFileName(char* fpath, char* fnameprefix, uint64_t findex)
	{
		std::ostringstream osfn;
		osfn << fpath << "/" << fnameprefix;
		osfn << "_master";
		osfn << "_" << findex;
		osfn << ".h5";
		return osfn.str();
	}

	/**
	 * Create virtual file name
	 * @param fpath file path
	 * @param fnameprefix file name prefix (includes scan and position variables)
	 * @param fnum current frame number
	 * @param findex file index
	 * @returns virtual file name
	 */
	static std::string CreateVirtualFileName(char* fpath, char* fnameprefix, uint64_t findex)
	{
		std::ostringstream osfn;
		osfn << fpath << "/" << fnameprefix;
		osfn << "_virtual";
		osfn << "_" << findex;
		osfn << ".h5";
		return osfn.str();
	}

	/**
	 * Close File
	 * @param ind index for debugging
	 * @param fd file pointer
	 * @param dp dataspace pointer
	 * @param ds dataset pointer
	 * @param ds_para pointer to array of parameter datasets
	 */

	static void CloseDataFile(int ind, H5File*& fd)
	{
		try {
			Exception::dontPrint(); //to handle errors
			if (fd) {
				delete fd;
				fd = 0;
			}
		} catch(Exception error) {
			cprintf(RED,"Error in closing HDF5 handles of index %d\n", ind);
			error.printErrorStack();
		}
	}

	/*
	 * Close master file
	 * @param fd master hdf5 file object
	 */
	static void CloseMasterDataFile(H5File*& fd)
	{
		try {
			Exception::dontPrint(); //to handle errors
			if (fd) {
				delete fd;
				fd = 0;
			}
		} catch(Exception error) {
			cprintf(RED,"Error in closing master HDF5 handles\n");
			error.printErrorStack();
		}
	}

	/*
	 * Close virtual file
	 * (in C because H5Pset_virtual doesnt exist yet in C++)
	 * @param fd virtual hdf5 file handle
	 */
	static void CloseVirtualDataFile(hid_t& fd)
	{
		if(fd) {
			if (H5Fclose(fd) < 0 )
				cprintf(RED,"Error in closing virtual HDF5 handles\n");
			fd = 0;
		}
	}

	/**
	 * Write data to file
	 * @param ind object index for debugging
	 * @param buf buffer to write from
	 * @param nDimx image number in file (imagenumber%maxframesinfile)
	 * @param nDimy number of pixels in y direction
	 * @param nDimz number of pixels in x direction
	 * @param dspace dataspace pointer
	 * @param dset dataset pointer
	 * @param dtype datatype
	 * @returns 0 for success and 1 for fail
	 */
	static int WriteDataFile(int ind, char* buf,
			uint64_t nDimx, uint32_t nDimy, uint32_t nDimz,
			DataSpace* dspace, DataSet* dset, DataType dtype)
	{
		hsize_t count[3] = {1, nDimy, nDimz};
		hsize_t start[3] = {nDimx, 0, 0};
		hsize_t dims2[2] = {nDimy, nDimz};
		try{
			Exception::dontPrint(); //to handle errors

			dspace->selectHyperslab( H5S_SELECT_SET, count, start);
			DataSpace memspace(2,dims2);
			dset->write(buf, dtype, memspace, *dspace);
			memspace.close();
		}
		catch(Exception error){
			cprintf(RED,"Error in writing to file in object %d\n",ind);
			error.printErrorStack();
			return 1;
		}
		return 0;
	}



	/**
	 * Write Parameter Arrays as datasets (to virtual file)
	 * @param ind self index
	 * @param dspace_para parameter dataspace
	 * @param fnum current frame number
	 * @param dset_para vector or dataset pointers of parameters
	 * @param rheader sls_receiver_header pointer
	 * @param parameterDataTypes parameter datatypes
	 */
	static int WriteParameterDatasets(int ind, DataSpace* dspace_para, uint64_t fnum,
			std::vector <DataSet*> dset_para,sls_receiver_header* rheader,
			std::vector <DataType> parameterDataTypes)
	{
		sls_detector_header header = rheader->detHeader;
		hsize_t count[1] = {1};
		hsize_t start[1] = {fnum};
		int i = 0;
		try{
			Exception::dontPrint(); //to handle errors
			dspace_para->selectHyperslab( H5S_SELECT_SET, count, start);
			DataSpace memspace(H5S_SCALAR);
			dset_para[0]->write(&header.frameNumber, 	parameterDataTypes[0], memspace, *dspace_para);i=1;
			dset_para[1]->write(&header.expLength, 		parameterDataTypes[1], memspace, *dspace_para);i=2;
			dset_para[2]->write(&header.packetNumber, 	parameterDataTypes[2], memspace, *dspace_para);i=3;
			dset_para[3]->write(&header.bunchId, 		parameterDataTypes[3], memspace, *dspace_para);i=4;
			dset_para[4]->write(&header.timestamp, 		parameterDataTypes[4], memspace, *dspace_para);i=5;
			dset_para[5]->write(&header.modId, 			parameterDataTypes[5], memspace, *dspace_para);i=6;
			dset_para[6]->write(&header.row, 			parameterDataTypes[6], memspace, *dspace_para);i=7;
			dset_para[7]->write(&header.column, 		parameterDataTypes[7], memspace, *dspace_para);i=8;
			dset_para[8]->write(&header.reserved, 		parameterDataTypes[8], memspace, *dspace_para);i=9;
			dset_para[9]->write(&header.debug, 			parameterDataTypes[9], memspace, *dspace_para);i=10;
			dset_para[10]->write(&header.roundRNumber, 	parameterDataTypes[10], memspace, *dspace_para);i=11;
			dset_para[11]->write(&header.detType, 		parameterDataTypes[11], memspace, *dspace_para);i=12;
			dset_para[12]->write(&header.version, 		parameterDataTypes[12], memspace, *dspace_para);i=13;

			// contiguous bitset
			if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
				dset_para[13]->write((char*)&(rheader->packetsMask), parameterDataTypes[13], memspace, *dspace_para);
			}

			// not contiguous bitset
			else {
				// get contiguous representation of bit mask
				bitset_storage storage;
				memset(storage, 0 , sizeof(bitset_storage));
				sls_bitset bits = rheader->packetsMask;
				for (int i = 0; i < MAX_NUM_PACKETS; ++i)
					storage[i >> 3] |= (bits[i] << (i & 7));
				// write bitmask
				dset_para[13]->write((char*)storage,	parameterDataTypes[13], memspace, *dspace_para);
			}i=14;
		}
		catch(Exception error){
			cprintf(RED,"Error in writing parameters (index:%d) to file in object %d\n", i, ind);
			error.printErrorStack();
			return 1;
		}
		return 0;
	}



	/**
	 * Extend datasets in #images dimension (x dimension)
	 * @param ind self index
	 * @param dpace data space pointer address
	 * @param dset data set pointer
	 * @param dspace_para parameter dataspace address pointer
	 * @param dset dataset parameter pointer
	 * @param initialNumImages initial number of images
	 * @returns 0 for success and 1 for fail
	 */
	static int ExtendDataset(int ind, DataSpace*& dspace, DataSet* dset,
			DataSpace*& dspace_para, std::vector <DataSet*> dset_para,
			uint64_t initialNumImages) {
		try{
			Exception::dontPrint(); //to handle errors

			hsize_t dims[3];
			dspace->getSimpleExtentDims(dims);
			dims[0] += initialNumImages;

			dset->extend(dims);
			delete dspace;
			dspace = 0;
			dspace = new DataSpace(dset->getSpace());

			hsize_t dims_para[1] = {dims[0]};
			for (unsigned int i = 0; i < dset_para.size(); ++i)
				dset_para[i]->extend(dims_para);
			delete dspace_para;
			dspace_para = 0;
			dspace_para = new DataSpace(dset_para[0]->getSpace());

		}
		catch(Exception error){
			cprintf(RED,"Error in extending dataset in object %d\n",ind);
			error.printError();
			return 1;
		}
		return 0;
	}


	/**
	 * Create master file
	 * @param fname master file name
	 * @param owenable overwrite enable
	 * @param dr dynamic range
	 * @param tenE ten giga enable
	 * @param size image size
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 * @param nf number of images
	 * @param maxf maximum frames per file
	 * @param acquisitionTime acquisition time
	 * @param subexposuretime sub exposure time
	 * @param subperiod sub period
	 * @param acquisitionPeriod acquisition period
	 * @param version version of software for hdf5 writing
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateMasterDataFile(H5File*& fd, std::string fname, bool owenable,
			uint32_t dr, bool tenE,	uint32_t size,
			uint32_t nPixelsx, uint32_t nPixelsy, uint64_t nf,
			uint32_t maxf,
			uint64_t acquisitionTime, uint64_t subexposuretime,
			uint64_t subperiod, uint64_t acquisitionPeriod, double version)
	{
		try {
			Exception::dontPrint(); //to handle errors

			FileAccPropList flist;
			flist.setFcloseDegree(H5F_CLOSE_STRONG);
			fd = 0;
			if(!owenable)
				fd = new H5File( fname.c_str(), H5F_ACC_EXCL,
						FileCreatPropList::DEFAULT,
						flist );
			else
				fd = new H5File( fname.c_str(), H5F_ACC_TRUNC,
						FileCreatPropList::DEFAULT,
						flist );

			//variables
			DataSpace dataspace = DataSpace (H5S_SCALAR);
			Attribute attribute;
			DataSet dataset;
			int iValue=0;
			double dValue=0;
			StrType strdatatype(PredType::C_S1,256);

			//create attributes
			//version
			dValue=version;
			attribute = fd->createAttribute("version",PredType::NATIVE_DOUBLE, dataspace);
			attribute.write(PredType::NATIVE_DOUBLE, &dValue);

			//Create a group in the file
			Group group1( fd->createGroup( "entry" ) );
			 Group group2( group1.createGroup("data") );
			 Group group3( group1.createGroup("instrument") );
			  Group group4( group3.createGroup("beam") );
			  Group group5( group3.createGroup("detector") );
			 Group group6( group1.createGroup("sample") );

			//Dynamic Range
			dataset = group5.createDataSet ( "dynamic range", PredType::NATIVE_INT, dataspace );
			dataset.write ( &dr, PredType::NATIVE_INT);
			attribute = dataset.createAttribute("unit",strdatatype, dataspace);
			attribute.write(strdatatype, std::string("bits"));

			//Ten Giga
			iValue = tenE;
			dataset = group5.createDataSet ( "ten giga enable", PredType::NATIVE_INT, dataspace );
			dataset.write ( &iValue, PredType::NATIVE_INT);

			//Image Size
			dataset = group5.createDataSet ( "image size", PredType::NATIVE_INT, dataspace );
			dataset.write (  &size, PredType::NATIVE_INT);
			attribute = dataset.createAttribute("unit",strdatatype, dataspace);
			attribute.write(strdatatype, std::string("bytes"));

			//x
			dataset = group5.createDataSet ( "number of pixels in x axis", PredType::NATIVE_INT, dataspace );
			dataset.write ( &nPixelsx, PredType::NATIVE_INT);

			//y
			dataset = group5.createDataSet ( "number of pixels in y axis", PredType::NATIVE_INT, dataspace );
			dataset.write ( &nPixelsy, PredType::NATIVE_INT);

			//Maximum frames per file
			dataset = group5.createDataSet ( "maximum frames per file", PredType::NATIVE_INT, dataspace );
			dataset.write ( &maxf, PredType::NATIVE_INT);

			//Total Frames
			dataset = group5.createDataSet ( "total frames", PredType::STD_U64LE, dataspace );
			dataset.write ( &nf, PredType::STD_U64LE);

			//Exptime
			dataset = group5.createDataSet ( "exposure time", PredType::STD_U64LE, dataspace );
			dataset.write ( &acquisitionTime, PredType::STD_U64LE);
			attribute = dataset.createAttribute("unit",strdatatype, dataspace);
			attribute.write(strdatatype, std::string("ns"));

			//SubExptime
			dataset = group5.createDataSet ( "sub exposure time", PredType::STD_U64LE, dataspace );
			dataset.write ( &subexposuretime, PredType::STD_U64LE);
			attribute = dataset.createAttribute("unit",strdatatype, dataspace);
			attribute.write(strdatatype, std::string("ns"));

			//SubPeriod
			dataset = group5.createDataSet ( "sub period", PredType::STD_U64LE, dataspace );
			dataset.write ( &subperiod, PredType::STD_U64LE);
			attribute = dataset.createAttribute("unit",strdatatype, dataspace);
			attribute.write(strdatatype, std::string("ns"));

			//Period
			dataset = group5.createDataSet ( "acquisition period", PredType::STD_U64LE, dataspace );
			dataset.write ( &acquisitionPeriod, PredType::STD_U64LE);
			attribute = dataset.createAttribute("unit",strdatatype, dataspace);
			attribute.write(strdatatype, std::string("ns"));

			//Timestamp
			time_t t = time(0);
			dataset = group5.createDataSet ( "timestamp", strdatatype, dataspace );
			dataset.write ( std::string(ctime(&t)), strdatatype );

			fd->close();

		} catch(Exception error) {
			cprintf(RED,"Error in creating master HDF5 handles\n");
			error.printErrorStack();
			if (fd) fd->close();
			return 1;
		}
		return 0;
	}



	/**
	 * Create File
	 * @param ind object index for debugging
	 * @param owenable overwrite enable
	 * @param fname complete file name
	 * @param frindexenable frame index enable
	 * @param fnum current image number
	 * @param nDimx number of pixels in x dim (#frames)
	 * @param nDimy number of pixels in y dim (height y dir)
	 * @param nDimz number of pixels in z dim (width x dir)
	 * @param dtype data type
	 * @param fd file pointer
	 * @param dspace dataspace pointer
	 * @param dset dataset pointer
	 * @param version version of software for hdf5 writing
	 * @param maxchunkedimages maximum chunked images
	 * @param dspace_para dataspace of parameters
	 * @param dset_para vector of datasets of parameters
	 * @param parameterNames parameter names
	 * @param parameterDataTypes parameter datatypes
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateDataFile(int ind, bool owenable, std::string fname, bool frindexenable,
			uint64_t fnum, uint64_t nDimx, uint32_t nDimy, uint32_t nDimz,
			DataType dtype, H5File*& fd, DataSpace*& dspace, DataSet*& dset,
			double version, uint64_t maxchunkedimages,
			DataSpace*& dspace_para, std::vector<DataSet*>& dset_para,
			std::vector <const char*> parameterNames,
			std::vector <DataType> parameterDataTypes)
	{
		try {
			Exception::dontPrint(); //to handle errors

			//file
			FileAccPropList fapl;
			fapl.setFcloseDegree(H5F_CLOSE_STRONG);
			fd = 0;
			if(!owenable)
				fd = new H5File( fname.c_str(), H5F_ACC_EXCL,
						FileCreatPropList::DEFAULT,
						fapl );
			else
				fd = new H5File( fname.c_str(), H5F_ACC_TRUNC,
						FileCreatPropList::DEFAULT,
						fapl );

			//attributes - version
			double dValue=version;
			DataSpace dataspace_attr = DataSpace (H5S_SCALAR);
			Attribute attribute = fd->createAttribute("version",PredType::NATIVE_DOUBLE, dataspace_attr);
			attribute.write(PredType::NATIVE_DOUBLE, &dValue);

			//dataspace
			hsize_t srcdims[3] = {nDimx, nDimy, nDimz};
			hsize_t srcdimsmax[3] = {H5S_UNLIMITED, nDimy, nDimz};
			dspace = 0;
			dspace = new DataSpace (3,srcdims,srcdimsmax);


			//dataset name
			std::ostringstream osfn;
			osfn << "/data";
			if (frindexenable) osfn << "_f" << std::setfill('0') << std::setw(12) << fnum;
			std::string dsetname = osfn.str();

			//dataset
			//fill value
			DSetCreatPropList plist;
			int fill_value = -1;
			plist.setFillValue(dtype, &fill_value);
			// always create chunked dataset as unlimited is only supported with chunked layout
			hsize_t chunk_dims[3] ={maxchunkedimages, nDimy, nDimz};
			plist.setChunk(3, chunk_dims);
			dset = 0;
			dset = new DataSet (fd->createDataSet(dsetname.c_str(), dtype, *dspace, plist));

			//create parameter datasets
			hsize_t dims[1] = {nDimx};
			hsize_t dimsmax[1] = {H5S_UNLIMITED};
			dspace_para = 0;
			dspace_para = new DataSpace (1,dims,dimsmax);

			// always create chunked dataset as unlimited is only supported with chunked layout
			DSetCreatPropList paralist;
			hsize_t chunkpara_dims[3] = {maxchunkedimages};
			paralist.setChunk(1, chunkpara_dims);

			for (unsigned int i = 0; i < parameterNames.size(); ++i){
				DataSet* ds = new DataSet(fd->createDataSet(parameterNames[i],
						parameterDataTypes[i], *dspace_para, paralist));
				dset_para.push_back(ds);
			}
		}
		catch(Exception error){
			cprintf(RED,"Error in creating HDF5 handles in object %d\n",ind);
			error.printErrorStack();
			if (fd) fd->close();
			return 1;
		}
		return 0;
	}


	/**
	 * Create virtual file
	 * (in C because H5Pset_virtual doesnt exist yet in C++)
	 * @param virtualFileName virtual file name
	 * @param fd virtual file handle
	 * @param masterFileName master file name
	 * @param fpath file path
	 * @param fnameprefix file name prefix (includes scan and position variables)
	 * @param findex file index
	 * @param frindexenable frame index enable
	 * @param dindex readout index
	 * @param numunits number of units per readout. eg. eiger has 2 udp units per readout
	 * @param maxFramesPerFile maximum frames per file
	 * @param numf number of frames caught
	 * @param srcDataseName source dataset name
	 * @param dataType datatype of data dataset
	 * @param numDety number of readouts in Y dir
	 * @param numDetz number of readouts in Z dir
	 * @param nDimy number of objects in y dimension in source file (Number of pixels in y dir)
	 * @param nDimz number of objects in z dimension in source file (Number of pixels in x dir)
	 * @param version version of software for hdf5 writing
	 * @param parameterNames parameter names
	 * @param parameterDataTypes parameter datatypes
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateVirtualDataFile(
			std::string virtualFileName,
			hid_t& fd, std::string masterFileName,
			char* fpath, char* fnameprefix, uint64_t findex, bool frindexenable,
			int dindex, int numunits,
			uint32_t maxFramesPerFile, uint64_t numf,
			std::string srcDataseName, DataType dataType,
			int numDety, int numDetz, uint32_t nDimy, uint32_t nDimz,
			double version,
			std::vector <const char*> parameterNames,
			std::vector <DataType> parameterDataTypes)
	{
		//file
		hid_t dfal = H5Pcreate (H5P_FILE_ACCESS);
		if (dfal < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating file access property for virtual file ")
					+ virtualFileName + std::string("\n"));
		if (H5Pset_fclose_degree (dfal, H5F_CLOSE_STRONG) < 0)
			return CloseFileOnError(fd,
					std::string("Error in setting strong file close degree for virtual file ")
					+ virtualFileName + std::string("\n"));
		fd = H5Fcreate( virtualFileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, dfal);
		if (fd < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating virtual file ") + virtualFileName + std::string("\n"));

		//attributes - version
		hid_t dataspace_attr = H5Screate (H5S_SCALAR);
		if (dataspace_attr < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating dataspace for attribute in virtual file ")
					+ virtualFileName + std::string("\n"));
		hid_t attrid = H5Acreate2 (fd, "version", H5T_NATIVE_DOUBLE, dataspace_attr, H5P_DEFAULT, H5P_DEFAULT);
		if (attrid < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating attribute in virtual file ")
					+ virtualFileName + std::string("\n"));
		double attr_data = version;
		if (H5Awrite (attrid, H5T_NATIVE_DOUBLE, &attr_data) < 0)
			return CloseFileOnError(fd,
					std::string("Error in writing attribute in virtual file ")
					+ virtualFileName + std::string("\n"));
		if (H5Aclose (attrid) < 0)
			return CloseFileOnError(fd,
					std::string("Error in closing attribute in virtual file ")
					+ virtualFileName + std::string("\n"));


		//virtual dataspace
		hsize_t vdsdims[3] = {numf, numDety * nDimy, numDetz * nDimz};
		hid_t vdsDataspace = H5Screate_simple(3, vdsdims ,NULL);
		if (vdsDataspace < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating virtual dataspace in virtual file ")
					+ virtualFileName + std::string("\n"));
		hsize_t vdsdims_para[2] = {numf, (unsigned int) numDety * numDetz};
		hid_t vdsDataspace_para = H5Screate_simple(2, vdsdims_para, NULL);
		if (vdsDataspace_para < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating virtual dataspace (parameters) in virtual file ")
					+ virtualFileName + std::string("\n"));


		//fill values
		hid_t dcpl = H5Pcreate (H5P_DATASET_CREATE);
		if (dcpl < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating file creation properties in virtual file ")
					+ virtualFileName + std::string("\n"));
		int fill_value = -1;
		if (H5Pset_fill_value (dcpl, GetDataTypeinC(dataType), &fill_value) < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating fill value in virtual file ")
					+ virtualFileName + std::string("\n"));
		hid_t dcpl_para[parameterNames.size()];
		for (unsigned int i = 0; i < parameterNames.size(); ++i) {
			dcpl_para[i] = H5Pcreate (H5P_DATASET_CREATE);
			if (dcpl_para[i] < 0)
				return CloseFileOnError(fd,
						std::string("Error in creating file creation properties (parameters) in virtual file ")
						+ virtualFileName + std::string("\n"));
			if (H5Pset_fill_value (dcpl_para[i], GetDataTypeinC(parameterDataTypes[i]), &fill_value) < 0)
				return CloseFileOnError(fd,
						std::string("Error in creating fill value (parameters) in virtual file ")
						+ virtualFileName + std::string("\n"));
		}

		//hyperslab
		int numMajorHyperslab = numf/maxFramesPerFile;
		if (numf%maxFramesPerFile) numMajorHyperslab++;
		bool error = false;
		uint64_t framesSaved = 0;
		for (int j = 0; j < numMajorHyperslab; j++) {

			uint64_t nDimx = ((numf - framesSaved) > maxFramesPerFile)
					? maxFramesPerFile : (numf-framesSaved);
			hsize_t offset[3] 		= {framesSaved, 0, 0};
			hsize_t	count[3] 		= {nDimx, nDimy, nDimz};
			hsize_t offset_para[2] 	= {framesSaved, 0};
			hsize_t	count_para[2] 	= {nDimx, 1};

			for (int i = 0; i < numDety * numDetz; ++i) {

				//setect hyperslabs
				if (H5Sselect_hyperslab (vdsDataspace, H5S_SELECT_SET, offset, NULL, count, NULL) < 0) {
					cprintf(RED,"could not select hyperslab\n");
					error = true;
					break;
				}
				if (H5Sselect_hyperslab (vdsDataspace_para, H5S_SELECT_SET,
						offset_para, NULL, count_para, NULL) < 0) {
					cprintf(RED,"could not select hyperslab for parameters\n");
					error = true;
					break;
				}

				//source file name
				std::string srcFileName = HDF5FileStatic::CreateFileName(fpath, fnameprefix, findex,
						frindexenable, framesSaved, dindex, numunits, i);

				// find relative path
				std::string relative_srcFileName = srcFileName;
				{
					size_t i = srcFileName.rfind('/', srcFileName.length());
					if (i != std::string::npos)
						relative_srcFileName = (srcFileName.substr(i+1, srcFileName.length() - i));
				}

				//source dataset name
				std::ostringstream osfn;
				osfn << "/data";
				if (frindexenable) osfn << "_f" << std::setfill('0') << std::setw(12) << framesSaved;
				std::string srcDatasetName = osfn.str();

				//source dataspace
				hsize_t srcdims[3] = {nDimx, nDimy, nDimz};
				hsize_t srcdimsmax[3] = {H5S_UNLIMITED, nDimy, nDimz};
				hid_t srcDataspace = H5Screate_simple(3, srcdims, srcdimsmax);
				if (srcDataspace < 0)
					return CloseFileOnError(fd,
							std::string("Error in creating source dataspace in virtual file ")
							+ virtualFileName + std::string("\n"));
				hsize_t srcdims_para[1] = {nDimx};
				hsize_t srcdimsmax_para[1] = {H5S_UNLIMITED};
				hid_t srcDataspace_para = H5Screate_simple(1, srcdims_para, srcdimsmax_para);
				if (srcDataspace_para < 0)
					return CloseFileOnError(fd,
							std::string("Error in creating source dataspace (parameters) in virtual file ")
							+ virtualFileName + std::string("\n"));

				//mapping
				if (H5Pset_virtual(dcpl, vdsDataspace, relative_srcFileName.c_str(),
						srcDatasetName.c_str(), srcDataspace) < 0) {
					cprintf(RED,"could not set mapping for paramter 1\n");
					error = true;
					break;
				}

				for (unsigned int k = 0; k < parameterNames.size(); ++k) {
					if (H5Pset_virtual(dcpl_para[k], vdsDataspace_para, relative_srcFileName.c_str(),
							parameterNames[k], srcDataspace_para) < 0) {
						cprintf(RED,"could not set mapping for paramter %d\n", k);
						error = true;
						break;
					}
				}

				//H5Sclose(srcDataspace);
				//H5Sclose(srcDataspace_para);
				offset[2] += nDimz;
				if (offset[2] >=  (numDetz * nDimz)) {
					offset[2] = 0;
					offset[1] += nDimy;
				}
				offset_para[1]++;
			}
			framesSaved += nDimx;
		}
		if (error)
			return CloseFileOnError(fd,
					std::string("Error in mapping files in virtual file ")
					+ virtualFileName + std::string("\n"));

		//dataset
		std::string virtualDatasetName = srcDataseName;
		hid_t vdsdataset = H5Dcreate2 (fd, virtualDatasetName.c_str(),
				GetDataTypeinC(dataType), vdsDataspace, H5P_DEFAULT, dcpl, H5P_DEFAULT);
		if (vdsdataset < 0)
			return CloseFileOnError(fd,
					std::string("Error in creating virutal dataset in virtual file ")
					+ virtualFileName + std::string("\n"));


		//virtual parameter dataset
		for (unsigned int i = 0; i < parameterNames.size(); ++i) {
			hid_t vdsdataset_para = H5Dcreate2 (fd,
					parameterNames[i],
					GetDataTypeinC(parameterDataTypes[i]), vdsDataspace_para,
					H5P_DEFAULT, dcpl_para[i], H5P_DEFAULT);
			if (vdsdataset_para < 0)
				return CloseFileOnError(fd,
						std::string("Error in creating virutal dataset (parameters) in virtual file ")
						+ virtualFileName + std::string("\n"));
		}

		//close
		H5Fclose(fd); fd = 0;

		//link
		return LinkVirtualInMaster(masterFileName, virtualFileName, virtualDatasetName, parameterNames);
	}



	/**
	 * Copy file to another file (mainly to view virutal files in hdfviewer)
	 * @param dataType data type
	 * @param owenable overwrite enable
	 * @param oldFileName file name including path of file to copy
	 * @param oldDatasetName dataset name to copy
	 * @param newFileName  file name including path of file to copy to
	 * @param newDatasetName dataset name to copy to
	 * @param rank rank
	 * @param nDimx Number of objects in x dimension
	 * @param nDimy Number of objects in y dimension
	 * @param nDimz Number of objects in z dimension
	 * @returns 0 for success and 1 for fail
	 */
	template <typename T>
	static int CopyVirtualFile(T datatype, bool owenable, std::string oldFileName, std::string oldDatasetName,
			std::string newFileName, std::string newDatasetName, int rank,
			uint64_t nDimx, uint32_t nDimy, uint32_t nDimz=0)
	{
		T *data_out;
		switch (rank) {
		case 2:
			data_out = (T*)malloc(sizeof(T)*(nDimx*nDimy));
			break;
		case 3:
			data_out = (T*)malloc(sizeof(T)*(nDimx*nDimy*nDimz));
			break;
		default:
			cprintf(RED,"invalid rank. Options: 2 or 3\n");
			return 0;
		}
		if (datatype == PredType::STD_U16LE) {
			FILE_LOG(logINFO) << "datatype:16";
		} else if (datatype == PredType::STD_U32LE) {
			FILE_LOG(logINFO) << "datatype:32";
		} else if (datatype == PredType::STD_U64LE) {
			FILE_LOG(logINFO) << "datatype:64";
		} else if (datatype == PredType::STD_U8LE) {
			FILE_LOG(logINFO) << "datatype:8";
		} else {
			FILE_LOG(logERROR) <<  "unknown datatype";
			return 1;
		}
		FILE_LOG(logINFO) << "owenable:" << (owenable?1:0) << std::endl
				<< "oldFileName:" << oldFileName << std::endl
				<< "oldDatasetName:" << oldDatasetName << std::endl
				<< "newFileName:" << newFileName << std::endl
				<< "newDatasetName:" << newDatasetName << std::endl
				<< "rank:" << rank << std::endl
				<< "nDimx:" << nDimx << std::endl
				<< "nDimy:" << nDimy << std::endl
				<< "nDimz:" << nDimz;

		H5File* oldfd;
		H5File* newfd;
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
			newfd = 0;
			if(!owenable)
				newfd = new H5File( newFileName.c_str(), H5F_ACC_EXCL,
						FileCreatPropList::DEFAULT,
						fapl );
			else
				newfd = new H5File( newFileName.c_str(), H5F_ACC_TRUNC,
						FileCreatPropList::DEFAULT,
						fapl );
			//dataspace and dataset
			DataSpace* newDataspace = 0;
			if (rank == 3) {
				hsize_t dims[3] = {nDimx, nDimy, nDimz};
				newDataspace = new DataSpace (3,dims);
			} else if (rank == 2) {
				hsize_t dims[2] = {nDimx, nDimy};
				newDataspace = new DataSpace (2,dims);
			}
			DataSet* newDataset = 0;
			newDataset = new DataSet( newfd->createDataSet(newDatasetName.c_str(), datatype, *newDataspace));
			//write and close
			newDataset->write(data_out,datatype);
			newfd->close();
			oldfd->close();
		} catch(Exception error){
			cprintf(RED,"Error in copying virtual files\n");
			error.printErrorStack();
			free(data_out);
			oldfd->close();
			newfd->close();
			return 1;
		}
		free(data_out);
		return 0;
	}



	/**
	 * Link the Virtual File in the Master File
	 * In C because H5Lcreate_external exists only in C
	 * @param masterFileName master file name
	 * @param virtualfname virtual file name
	 * @param virtualDatasetname virtual dataset name
	 * @param parameterNames parameter names
	 * @returns 0 for success and 1 for fail
	 */
	static int LinkVirtualInMaster(std::string masterFileName, std::string virtualfname,
			std::string virtualDatasetname, std::vector <const char*> parameterNames) {
		char linkname[100];
		hid_t vfd = 0;

		hid_t dfal = H5Pcreate (H5P_FILE_ACCESS);
		if (dfal < 0)
			return CloseFileOnError( vfd, std::string("Error in creating file access property for link\n"));
		if (H5Pset_fclose_degree (dfal, H5F_CLOSE_STRONG) < 0)
			return CloseFileOnError( vfd, std::string("Error in setting strong file close degree for link\n"));

		//open master file
		hid_t mfd = H5Fopen( masterFileName.c_str(), H5F_ACC_RDWR, dfal);
		if (mfd < 0)
			return CloseFileOnError( vfd, std::string("Error in opening master file\n"));

		//open virtual file
		vfd = H5Fopen( virtualfname.c_str(), H5F_ACC_RDWR, dfal);
		if (vfd < 0) {
			H5Fclose(mfd); mfd = 0;
			return CloseFileOnError( vfd, std::string("Error in opening virtual file\n"));
		}

		// find relative path
		std::string relative_virtualfname = virtualfname;
		{
			size_t i = virtualfname.rfind('/', virtualfname.length());
			if (i != std::string::npos)
				relative_virtualfname = (virtualfname.substr(i+1, virtualfname.length() - i));
		}

		//**data dataset**
		hid_t vdset = H5Dopen2( vfd, virtualDatasetname.c_str(), H5P_DEFAULT);
		if (vdset < 0) {
			H5Fclose(mfd);
			return CloseFileOnError( vfd, std::string("Error in opening virtual data dataset\n"));
		}
		sprintf(linkname, "/entry/data/%s",virtualDatasetname.c_str());
		if(H5Lcreate_external( relative_virtualfname.c_str(), virtualDatasetname.c_str(),
										mfd, linkname, H5P_DEFAULT, H5P_DEFAULT) < 0) {
			H5Fclose(mfd); mfd = 0;
			return CloseFileOnError( vfd, std::string("Error in creating link to data dataset\n"));
		}
		H5Dclose(vdset);

		//**paramter datasets**
		for (unsigned int i = 0; i < parameterNames.size(); ++i){
			hid_t vdset_para = H5Dopen2( vfd, (std::string (parameterNames[i])).c_str(), H5P_DEFAULT);
			if (vdset_para < 0) {
				H5Fclose(mfd); mfd = 0;
				return CloseFileOnError( vfd, std::string("Error in opening virtual parameter dataset to create link\n"));
			}
			sprintf(linkname, "/entry/data/%s",(std::string (parameterNames[i])).c_str());

			if(H5Lcreate_external( relative_virtualfname.c_str(), (std::string (parameterNames[i])).c_str(),
					mfd, linkname, H5P_DEFAULT, H5P_DEFAULT) < 0) {
				H5Fclose(mfd); mfd = 0;
				return CloseFileOnError( vfd, std::string("Error in creating link to virtual parameter dataset\n"));
			}
		}

		H5Fclose(mfd); mfd = 0;
		H5Fclose(vfd); vfd = 0;
		return 0;
	}



	/**
	 * Print Error msg and Close File and called on error
	 * @returns 1 for fail
	 */
	static int CloseFileOnError(hid_t& fd, const std::string msg) {
		cprintf(RED, "%s", msg.c_str());
		if(fd > 0)
			H5Fclose(fd);
		fd = 0;
		return 1;
	}


	/**
	 * Get Data type in C
	 * @param dtype datatype in C++
	 * @returns datatype in C
	 */
	static hid_t GetDataTypeinC(DataType dtype) {
		if (dtype == PredType::STD_U8LE)
			return H5T_STD_U8LE;
		else if (dtype == PredType::STD_U16LE)
			return H5T_STD_U16LE;
		else if (dtype == PredType::STD_U32LE)
			return H5T_STD_U32LE;
		else if (dtype == PredType::STD_U64LE)
			return H5T_STD_U64LE;
		else {
			hid_t s = H5Tcopy(H5T_C_S1);
			H5Tset_size(s, MAX_NUM_PACKETS);
			return s;
		}
	}

};



#endif
