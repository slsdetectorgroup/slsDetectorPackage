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


#include <string>
#include <iomanip>
#include <stdlib.h>	 //malloc
#include <sstream>
using namespace std;

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
	static string CreateFileName(char* fpath, char* fnameprefix, uint64_t findex, bool frindexenable,
			uint64_t fnum = 0, int dindex = -1, int numunits = 1, int unitindex = 0)
	{
		ostringstream osfn;
		osfn << fpath << "/" << fnameprefix;
		if (dindex >= 0) osfn << "_d" << (dindex * numunits + unitindex);
		if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << fnum;
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
	static string CreateMasterFileName(char* fpath, char* fnameprefix, uint64_t findex)
	{
		ostringstream osfn;
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
	static string CreateVirtualFileName(char* fpath, char* fnameprefix, uint64_t findex)
	{
		ostringstream osfn;
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
			error.printError();
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
			error.printError();
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
			error.printError();
			return 1;
		}
		return 0;
	}



	/**
	 * Write Parameter Arrays as datasets (to virtual file)
	 */
	static int WriteParameterDatasets(int ind, DataSpace* dspace_para, uint64_t fnum,
			DataSet* dset_para[],sls_detector_header* header)
	{
		hsize_t count[1] = {1};
		hsize_t start[1] = {fnum};
		try{
			Exception::dontPrint(); //to handle errors
			dspace_para->selectHyperslab( H5S_SELECT_SET, count, start);
			DataSpace memspace(H5S_SCALAR);
			dset_para[0]->write(&header->frameNumber, 	ParameterDataTypes[0], memspace, *dspace_para);
			dset_para[1]->write(&header->expLength, 	ParameterDataTypes[1], memspace, *dspace_para);
			dset_para[2]->write(&header->packetNumber, 	ParameterDataTypes[2], memspace, *dspace_para);
			dset_para[3]->write(&header->bunchId, 		ParameterDataTypes[3], memspace, *dspace_para);
			dset_para[4]->write(&header->timestamp, 	ParameterDataTypes[4], memspace, *dspace_para);
			dset_para[5]->write(&header->modId, 		ParameterDataTypes[5], memspace, *dspace_para);
			dset_para[6]->write(&header->xCoord, 		ParameterDataTypes[6], memspace, *dspace_para);
			dset_para[7]->write(&header->yCoord, 		ParameterDataTypes[7], memspace, *dspace_para);
			dset_para[8]->write(&header->zCoord, 		ParameterDataTypes[8], memspace, *dspace_para);
			dset_para[9]->write(&header->debug, 		ParameterDataTypes[9], memspace, *dspace_para);
			dset_para[10]->write(&header->roundRNumber, ParameterDataTypes[10], memspace, *dspace_para);
			dset_para[11]->write(&header->detType, 		ParameterDataTypes[11], memspace, *dspace_para);
			dset_para[12]->write(&header->version, 		ParameterDataTypes[12], memspace, *dspace_para);
		}
		catch(Exception error){
			cprintf(RED,"Error in writing parameters to file in object %d\n",ind);
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
	 * @param acquisitionTime acquisition time
	 * @param acquisitionPeriod acquisition period
	 * @param version version of software for hdf5 writing
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateMasterDataFile(H5File*& fd, string fname, bool owenable,
			uint32_t dr, bool tenE,	uint32_t size, uint32_t nPixelsx, uint32_t nPixelsy, uint64_t nf,
			uint64_t acquisitionTime, uint64_t acquisitionPeriod, double version)
	{
		try {
			Exception::dontPrint(); //to handle errors

			FileAccPropList flist;
			flist.setFcloseDegree(H5F_CLOSE_STRONG);
			if(!owenable)
				fd = new H5File( fname.c_str(), H5F_ACC_EXCL, NULL, flist );
			else
				fd = new H5File( fname.c_str(), H5F_ACC_TRUNC, NULL, flist );

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
			dataset.write ( &nPixelsx, PredType::NATIVE_INT);

			//y
			dataset = group5.createDataSet ( "number of pixels in y axis", PredType::NATIVE_INT, dataspace );
			dataset.write ( &nPixelsy, PredType::NATIVE_INT);

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

			fd->close();

		} catch(Exception error) {
			cprintf(RED,"Error in creating master HDF5 handles\n");
			error.printError();
			return 1;
		}
		return 0;
	}



	/**
	 * Create File
	 * @param ind object index for debugging
	 * @param owenable overwrite enable
	 * @param numf number of images
	 * @param fname complete file name
	 * @param frindexenable frame index enable
	 * @param fnum current image number
	 * @param nx number of pixels in x dir
	 * @param ny number of pixels in y dir
	 * @param dtype data type
	 * @param fd file pointer
	 * @param dspace dataspace pointer
	 * @param dset dataset pointer
	 * @param version version of software for hdf5 writing
	 * @param maxchunkedimages maximum chunked images
	 * @param dspace_para dataspace of parameters
	 * @param para1 parameter 1 name
	 * @param dset_para1 dataset of parameter 1
	 * @param dtype_para1 datatype of parameter 1
	 * @param para2 parameter 2 name
	 * @param dset_para2 dataset of parameter 2
	 * @param dtype_para2 datatype of parameter 2
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateDataFile(int ind, bool owenable, string fname, bool frindexenable,
			uint64_t fnum, uint64_t nDimx, uint32_t nDimy, uint32_t nDimz,
			DataType dtype, H5File*& fd, DataSpace*& dspace, DataSet*& dset,
			double version, uint64_t maxchunkedimages,
			DataSpace*& dspace_para, DataSet* dset_para[])
	{
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
			double dValue=version;
			DataSpace dataspace_attr = DataSpace (H5S_SCALAR);
			Attribute attribute = fd->createAttribute("version",PredType::NATIVE_DOUBLE, dataspace_attr);
			attribute.write(PredType::NATIVE_DOUBLE, &dValue);

			//dataspace
			hsize_t srcdims[3] = {nDimx, nDimy, nDimz};
			dspace = new DataSpace (3,srcdims);

			//dataset name
			ostringstream osfn;
			osfn << "/data";
			if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << fnum;
			string dsetname = osfn.str();

			//dataset
			//chunked dataset if greater than max_chunked_images
			if(nDimx > maxchunkedimages){
				DSetCreatPropList plist;
				hsize_t chunk_dims[3] ={maxchunkedimages, nDimy, nDimz};
				plist.setChunk(3, chunk_dims);
				dset = new DataSet (fd->createDataSet(dsetname.c_str(), dtype, *dspace, plist));
			}else
				dset = new DataSet (fd->createDataSet(dsetname.c_str(), dtype, *dspace));

			//create parameter datasets
			hsize_t dims[1] = {nDimx};
			dspace_para = new DataSpace (1,dims);
			for (int i = 0; i < NumberofParameters; ++i)
				dset_para[i] = new DataSet(fd->createDataSet(ParameterNames[i], ParameterDataTypes[i], *dspace_para));
		}
		catch(Exception error){
			cprintf(RED,"Error in creating HDF5 handles in object %d\n",ind);
			error.printError();
			fd->close();
			return 1;
		}
		return 0;
	}


	/**
	 * Create virtual file
	 * (in C because H5Pset_virtual doesnt exist yet in C++)
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
	 * @param srcP1DatasetName source parameter 1 dataset name
	 * @param srcP2DatasetName source parameter 2 dataset name
	 * @param dataType datatype of data dataset
	 * @param p1DataType datatype of parameter 1
	 * @param p2DataType datatype of parameter 2
	 * @param numDety number of readouts in Y dir
	 * @param numDetz number of readouts in Z dir
	 * @param nDimy number of objects in y dimension in source file (Number of pixels in y dir)
	 * @param nDimz number of objects in z dimension in source file (Number of pixels in x dir)
	 * @param version version of software for hdf5 writing
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateVirtualDataFile(
			hid_t& fd, string masterFileName,
			char* fpath, char* fnameprefix, uint64_t findex, bool frindexenable,
			int dindex, int numunits,
			uint32_t maxFramesPerFile, uint64_t numf,
			string srcDataseName, DataType dataType,
			int numDety, int numDetz, uint32_t nDimy, uint32_t nDimz,
			double version)
	{
		//virtual names
		string virtualFileName = CreateVirtualFileName(fpath, fnameprefix, findex);
		printf("Virtual File: %s\n", virtualFileName.c_str());

		//file
		hid_t dfal = H5Pcreate (H5P_FILE_ACCESS);
		if (dfal < 0)
			return CloseFileOnError(fd, string("Error in creating file access property for virtual file ") + virtualFileName + string("\n"));
		if (H5Pset_fclose_degree (dfal, H5F_CLOSE_STRONG) < 0)
			return CloseFileOnError(fd, string("Error in setting strong file close degree for virtual file ") + virtualFileName + string("\n"));
		fd = H5Fcreate( virtualFileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, dfal);
		if (fd < 0)
			return CloseFileOnError(fd, string("Error in creating virtual file ") + virtualFileName + string("\n"));

		//attributes - version
		hid_t dataspace_attr = H5Screate (H5S_SCALAR);
		if (dataspace_attr < 0)
			return CloseFileOnError(fd, string("Error in creating dataspace for attribute in virtual file ") + virtualFileName + string("\n"));
		hid_t attrid = H5Acreate2 (fd, "version", H5T_NATIVE_DOUBLE, dataspace_attr, H5P_DEFAULT, H5P_DEFAULT);
		if (attrid < 0)
			return CloseFileOnError(fd, string("Error in creating attribute in virtual file ") + virtualFileName + string("\n"));
		double attr_data = version;
		if (H5Awrite (attrid, H5T_NATIVE_DOUBLE, &attr_data) < 0)
			return CloseFileOnError(fd, string("Error in writing attribute in virtual file ") + virtualFileName + string("\n"));
		if (H5Aclose (attrid) < 0)
			return CloseFileOnError(fd, string("Error in closing attribute in virtual file ") + virtualFileName + string("\n"));


		//virtual dataspace
		hsize_t vdsdims[3] = {numf, numDety * nDimy, numDetz * nDimz};
		hid_t vdsDataspace = H5Screate_simple(3, vdsdims ,NULL);
		if (vdsDataspace < 0)
			return CloseFileOnError(fd, string("Error in creating virtual dataspace in virtual file ") + virtualFileName + string("\n"));
		hsize_t vdsdims_para[2] = {numf, numDety * numDetz};
		hid_t vdsDataspace_para = H5Screate_simple(2, vdsdims_para, NULL);
		if (vdsDataspace_para < 0)
			return CloseFileOnError(fd, string("Error in creating virtual dataspace (parameters) in virtual file ") + virtualFileName + string("\n"));


		//fill values
		hid_t dcpl = H5Pcreate (H5P_DATASET_CREATE);
		if (dcpl < 0)
			return CloseFileOnError(fd, string("Error in creating file creation properties in virtual file ") + virtualFileName + string("\n"));
		int fill_value = -1;
		if (H5Pset_fill_value (dcpl, GetDataTypeinC(dataType), &fill_value) < 0)
			return CloseFileOnError(fd, string("Error in creating fill value in virtual file ") + virtualFileName + string("\n"));
		hid_t dcpl_para[NumberofParameters];
		for (int i = 0; i < NumberofParameters; ++i) {
			dcpl_para[i] = H5Pcreate (H5P_DATASET_CREATE);
			if (dcpl_para[i] < 0)
				return CloseFileOnError(fd, string("Error in creating file creation properties (parameters) in virtual file ") + virtualFileName + string("\n"));
			if (H5Pset_fill_value (dcpl_para[i], GetDataTypeinC(ParameterDataTypes[i]), &fill_value) < 0)
				return CloseFileOnError(fd, string("Error in creating fill value (parameters) in virtual file ") + virtualFileName + string("\n"));
		}

		//hyperslab
		int numMajorHyperslab = numf/maxFramesPerFile;
		if (numf%maxFramesPerFile) numMajorHyperslab++;
		bool error = false;
		uint64_t framesSaved = 0;
		for (int j = 0; j < numMajorHyperslab; j++) {

			uint64_t nDimx = ((numf - framesSaved) > maxFramesPerFile) ? maxFramesPerFile : (numf-framesSaved);
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
				if (H5Sselect_hyperslab (vdsDataspace_para, H5S_SELECT_SET, offset_para, NULL, count_para, NULL) < 0) {
					cprintf(RED,"could not select hyperslab for parameters\n");
					error = true;
					break;
				}

				//source file name
				string srcFileName = HDF5FileStatic::CreateFileName(fpath, fnameprefix, findex,
						frindexenable, framesSaved, dindex, numunits, i);

				//source dataset name
				ostringstream osfn;
				osfn << "/data";
				if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << framesSaved;
				string srcDatasetName = osfn.str();

				//source dataspace
				hsize_t srcdims[3] = {nDimx, nDimy, nDimz};
				hid_t srcDataspace = H5Screate_simple(3, srcdims, NULL);
				if (srcDataspace < 0)
					return CloseFileOnError(fd, string("Error in creating source dataspace in virtual file ") + virtualFileName + string("\n"));
				hsize_t srcdims_para[1] = {nDimx};
				hid_t srcDataspace_para = H5Screate_simple(1, srcdims_para, NULL);
				if (srcDataspace_para < 0)
					return CloseFileOnError(fd, string("Error in creating source dataspace (parameters) in virtual file ") + virtualFileName + string("\n"));

				//mapping
				if (H5Pset_virtual(dcpl, vdsDataspace, srcFileName.c_str(), srcDatasetName.c_str(), srcDataspace) < 0) {
					cprintf(RED,"could not set mapping for paramter 1\n");
					error = true;
					break;
				}

				for (int k = 0; k < NumberofParameters; ++k) {
					if (H5Pset_virtual(dcpl_para[k], vdsDataspace_para, srcFileName.c_str(), ParameterNames[k], srcDataspace_para) < 0) {
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
			return CloseFileOnError(fd, string("Error in mapping files in virtual file ") + virtualFileName + string("\n"));

		//dataset
		string virtualDatasetName = string("/virtual_") + srcDataseName;
		hid_t vdsdataset = H5Dcreate2 (fd, virtualDatasetName.c_str(), GetDataTypeinC(dataType), vdsDataspace, H5P_DEFAULT, dcpl, H5P_DEFAULT);
		if (vdsdataset < 0)
			return CloseFileOnError(fd, string("Error in creating virutal dataset in virtual file ") + virtualFileName + string("\n"));


		//virtual parameter dataset
		for (int i = 0; i < NumberofParameters; ++i) {
			hid_t vdsdataset_para = H5Dcreate2 (fd,
					(string("/virtual_") + string (ParameterNames[i])).c_str(),
					GetDataTypeinC(ParameterDataTypes[i]), vdsDataspace_para, H5P_DEFAULT, dcpl_para[i], H5P_DEFAULT);
			if (vdsdataset_para < 0)
				return CloseFileOnError(fd, string("Error in creating virutal dataset (parameters) in virtual file ") + virtualFileName + string("\n"));
		}

		//close
		H5Fclose(fd); fd = 0;

		//link
		return LinkVirtualInMaster(masterFileName, virtualFileName, virtualDatasetName);
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
	static int CopyVirtualFile(T datatype, bool owenable, string oldFileName, string oldDatasetName,
			string newFileName, string newDatasetName, int rank,
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
			printf("datatype:16\n");
		} else if (datatype == PredType::STD_U32LE) {
			printf("datatype:32\n");
		} else if (datatype == PredType::STD_U64LE) {
			printf("datatype:64\n");
		} else if (datatype == PredType::STD_U8LE) {
			printf("datatype:8\n");
		} else {
			cprintf(RED, "unknown datatype\n");
			return 1;
		}
		printf("owenable:%d\n"
				"oldFileName:%s\n"
				"oldDatasetName:%s\n"
				"newFileName:%s\n"
				"newDatasetName:%s\n"
				"rank:%d\n"
				"nDimx:%llu\n"
				"nDimy:%u\n"
				"nDimz:%u\n",
						owenable?1:0,
						oldFileName.c_str(),
						oldDatasetName.c_str(),
						newFileName.c_str(),
						newDatasetName.c_str(),
						rank,
						(long long unsigned int)nDimx,
						nDimy,
						nDimz);

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
			if(!owenable)
				newfd = new H5File( newFileName.c_str(), H5F_ACC_EXCL, NULL,fapl );
			else
				newfd = new H5File( newFileName.c_str(), H5F_ACC_TRUNC, NULL, fapl );
			//dataspace and dataset
			DataSpace* newDataspace;
			if (rank == 3) {
				hsize_t dims[3] = {nDimx, nDimy, nDimz};
				newDataspace = new DataSpace (3,dims);
			} else if (rank == 2) {
				hsize_t dims[2] = {nDimx, nDimy};
				newDataspace = new DataSpace (2,dims);
			}
			DataSet* newDataset = new DataSet( newfd->createDataSet(newDatasetName.c_str(), datatype, *newDataspace));
			//write and close
			newDataset->write(data_out,datatype);
			newfd->close();
			oldfd->close();
		} catch(Exception error){
			cprintf(RED,"Error in copying virtual files\n");
			error.printError();
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
	 * @returns 0 for success and 1 for fail
	 */
	static int LinkVirtualInMaster(string masterFileName, string virtualfname, string virtualDatasetname) {
		char linkname[100];
		hid_t vfd = 0;

		hid_t dfal = H5Pcreate (H5P_FILE_ACCESS);
		if (dfal < 0)
			return CloseFileOnError( vfd, string("Error in creating file access property for link\n"));
		if (H5Pset_fclose_degree (dfal, H5F_CLOSE_STRONG) < 0)
			return CloseFileOnError( vfd, string("Error in setting strong file close degree for link\n"));

		//open master file
		hid_t mfd = H5Fopen( masterFileName.c_str(), H5F_ACC_RDWR, dfal);
		if (mfd < 0)
			return CloseFileOnError( vfd, string("Error in opening master file\n"));

		//open virtual file
		vfd = H5Fopen( virtualfname.c_str(), H5F_ACC_RDWR, dfal);
		if (vfd < 0) {
			H5Fclose(mfd); mfd = 0;
			return CloseFileOnError( vfd, string("Error in opening virtual file\n"));
		}

		//**data dataset**
		hid_t vdset = H5Dopen2( vfd, virtualDatasetname.c_str(), H5P_DEFAULT);
		if (vdset < 0) {
			H5Fclose(mfd);
			return CloseFileOnError( vfd, string("Error in opening virtual data dataset\n"));
		}
		sprintf(linkname, "/entry/data/%s",virtualDatasetname.c_str());
		if(H5Lcreate_external( virtualfname.c_str(), virtualDatasetname.c_str(),
										mfd, linkname, H5P_DEFAULT, H5P_DEFAULT) < 0) {
			H5Fclose(mfd); mfd = 0;
			return CloseFileOnError( vfd, string("Error in creating link to data dataset\n"));
		}
		H5Dclose(vdset);

		//**paramter datasets**
		for (int i = 0; i < NumberofParameters; ++i){
			hid_t vdset_para = H5Dopen2( vfd, (string("/virtual_") + string (ParameterNames[i])).c_str(), H5P_DEFAULT);
			if (vdset_para < 0) {
				H5Fclose(mfd); mfd = 0;
				return CloseFileOnError( vfd, string("Error in opening virtual parameter dataset to create link\n"));
			}
			sprintf(linkname, "/entry/data/%s",(string("/virtual_") + string (ParameterNames[i])).c_str());
			if(H5Lcreate_external( virtualfname.c_str(), (string("/virtual_") + string (ParameterNames[i])).c_str(),
					mfd, linkname, H5P_DEFAULT, H5P_DEFAULT) < 0) {
				H5Fclose(mfd); mfd = 0;
				return CloseFileOnError( vfd, string("Error in creating link to virtual parameter dataset\n"));
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
	static int CloseFileOnError(hid_t& fd, const string msg) {
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
			cprintf(RED, "Invalid Data type\n");
			return H5T_STD_U64LE;
		}
	}


	static const int NumberofParameters = 13;

private:
	static const char * const ParameterNames[];
	static const DataType ParameterDataTypes[];

};



#endif
