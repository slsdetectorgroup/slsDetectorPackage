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
#include "ansi.h"


#include <string>
#include <iomanip>
#include <stdlib.h>	 //malloc
#include <sstream>
using namespace std;


class HDF5FileStatic {

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
	 * @param frindexenable frame index enable
	 * @param fnum frame number index
	 * @returns virtual file name
	 */
	static string CreateVirtualFileName(char* fpath, char* fnameprefix, uint64_t findex, bool frindexenable,
			uint64_t fnum)
	{
		ostringstream osfn;
		osfn << fpath << "/" << fnameprefix;
		osfn << "_virtual";
		if (frindexenable) osfn << "_f" << setfill('0') << setw(12) << fnum;
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
	 */

	static void CloseDataFile(int ind, H5File*& fd, DataSpace*& dp, DataSet*& ds,
			 DataSet*& ds_p1, DataSet*& ds_p2)
	{
		try {
			Exception::dontPrint(); //to handle errors
			if(dp) 		{delete dp; 	dp = 0;}
			if(ds) 		{delete ds; 	ds = 0;}
			if(ds_p1) 	{delete ds_p1; 	ds_p1 = 0;}
			if(ds_p2) 	{delete ds_p2; 	ds_p2 = 0;}
			if(fd) 		{delete fd; 	fd = 0;}
		} catch(Exception error) {
			cprintf(RED,"Error in closing HDF5 handles\n");
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
			if(fd) {delete fd; 	fd = 0;}
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
	template <typename P1, typename P2>
	static int WriteParameterDatasets(int ind, DataSpace* dspace_para, uint64_t fnum,
			DataSet*& dset_para1, DataType dtype_para1, P1* para1,
			DataSet*& dset_para2, DataType dtype_para2, P2* para2)
	{
		hsize_t count[1] = {1};
		hsize_t start[1] = {fnum};
		hsize_t dims2[2] = {1};
		try{
			Exception::dontPrint(); //to handle errors
			dspace_para->selectHyperslab( H5S_SELECT_SET, count, start);
			DataSpace memspace(H5S_SCALAR);
			dset_para1->write(para1, dtype_para1, memspace, *dspace_para);
			dset_para2->write(para2, dtype_para2, memspace, *dspace_para);
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
			DataSpace*& dspace_para,
			string para1, DataSet*& dset_para1, DataType dtype_para1,
			string para2, DataSet*& dset_para2, DataType dtype_para2)
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
			dset_para1 = new DataSet(fd->createDataSet(para1.c_str(), dtype_para1, *dspace_para));
			dset_para2 = new DataSet(fd->createDataSet(para2.c_str(), dtype_para2, *dspace_para));
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
	 * @param virtualfname virtual file name
	 * @param virtualDatasetname virtual dataset name
	 * @param srcDatasetname source dataset name
	 * @param spara1Datasetname source dataset name of parameter 1
	 * @param vpara1DatasetName virtual dataset name of parameter 1
	 * @param dtype_para1 datatype of parameter 1
	 * @param spara1Datasetname source dataset name of parameter 1
	 * @param vpara2DatasetName virtual dataset name of parameter 2
	 * @param dtype_para2 datatype of parameter 2
	 * @param numFiles number of files
	 * @param fileNames array of file names
	 * @param owenable overwrite enable
	 * @param dtype datatype
	 * @param srcNDimx source number of objects in x dimension (Number of images)
	 * @param srcNDimy source number of objects in y dimension (Number of pixels in y dir)
	 * @param srcNDimz source number of objects in z dimension (Number of pixels in x dir)
	 * @param dstNDimx destination number of objects in x dimension (Number of images)
	 * @param dstNDimy destination number of objects in y dimension (Number of pixels in y dir)
	 * @param dstNDimz destination number of objects in z dimension (Number of pixels in x dir)
	 * @param version version of software for hdf5 writing
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateVirtualDataFile(hid_t& fd, string virtualfname, string virtualDatasetname, string srcDatasetname,
			string spara1DatasetName, string vpara1DatasetName, hid_t dtype_para1,
			string spara2DatasetName, string vpara2DatasetName, hid_t dtype_para2,
			int numFiles, string fileNames[], bool owenable, hid_t dtype,
			uint64_t srcNDimx, uint32_t srcNDimy, uint32_t srcNDimz,
			uint64_t dstNDimx, uint32_t dstNDimy, uint32_t dstNDimz, double version)
	{
		//file
		hid_t dfal = H5Pcreate (H5P_FILE_ACCESS);
		if (dfal < 0)
			return CloseFileOnError(fd, string("Error in creating file access property for virtual file ") + virtualfname + string("\n"));
		if (H5Pset_fclose_degree (dfal, H5F_CLOSE_STRONG) < 0)
			return CloseFileOnError(fd, string("Error in setting strong file close degree for virtual file ") + virtualfname + string("\n"));
		if(!owenable)
			fd = H5Fcreate( virtualfname.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, dfal);
		else
			fd = H5Fcreate( virtualfname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, dfal);
		if (fd < 0)
			return CloseFileOnError(fd, string("Error in creating virtual file ") + virtualfname + string("\n"));


		//attributes - version
		hid_t dataspace_attr = H5Screate (H5S_SCALAR);
		if (dataspace_attr < 0)
			return CloseFileOnError(fd, string("Error in creating dataspace for attribute in virtual file ") + virtualfname + string("\n"));
		hid_t attrid = H5Acreate2 (fd, "version", H5T_NATIVE_DOUBLE, dataspace_attr, H5P_DEFAULT, H5P_DEFAULT);
		if (attrid < 0)
			return CloseFileOnError(fd, string("Error in creating attribute in virtual file ") + virtualfname + string("\n"));
		double attr_data = version;
		if (H5Awrite (attrid, H5T_NATIVE_DOUBLE, &attr_data) < 0)
			return CloseFileOnError(fd, string("Error in writing attribute in virtual file ") + virtualfname + string("\n"));
				if (H5Aclose (attrid) < 0)
			return CloseFileOnError(fd, string("Error in closing attribute in virtual file ") + virtualfname + string("\n"));



		//**data dataset**
		//dataspace
		hsize_t vdsdims[3] = {dstNDimx, dstNDimy, dstNDimz};
		hid_t vdsDataspace = H5Screate_simple(3, vdsdims ,NULL);
		if (vdsDataspace < 0)
			return CloseFileOnError(fd, string("Error in creating virtual dataspace in virtual file ") + virtualfname + string("\n"));
		hsize_t srcdims[3] = {srcNDimx, srcNDimy, srcNDimz};
		hid_t srcDataspace = H5Screate_simple(3, srcdims, NULL);
		if (srcDataspace < 0)
			return CloseFileOnError(fd, string("Error in creating source dataspace in virtual file ") + virtualfname + string("\n"));

		//fill values
		hid_t dcpl = H5Pcreate (H5P_DATASET_CREATE);
		if (dcpl < 0)
			return CloseFileOnError(fd, string("Error in creating file creation properties in virtual file ") + virtualfname + string("\n"));
		int fill_value = -1;
		if (H5Pset_fill_value (dcpl, dtype, &fill_value) < 0)
			return CloseFileOnError(fd, string("Error in creating fill value in virtual file ") + virtualfname + string("\n"));


		//hyperslab
		hsize_t offset[3]={0, 0, 0},
				count[3]={srcNDimx, srcNDimy, srcNDimz};
		bool error = false;
		for (int i = 0; i < numFiles; i++) {
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
			if (offset[2] >=  dstNDimz) {
				offset[2] = 0;
				offset[1] += srcNDimy;
			}
		}
		if (error)
			return CloseFileOnError(fd, string("Error in mapping files in virtual file ") + virtualfname + string("\n"));

		//dataset
		hid_t vdsdataset = H5Dcreate2 (fd, virtualDatasetname.c_str(), dtype, vdsDataspace, H5P_DEFAULT, dcpl, H5P_DEFAULT);
		if (vdsdataset < 0)
			return CloseFileOnError(fd, string("Error in creating virutal dataset in virtual file ") + virtualfname + string("\n"));

		//close
		H5Sclose(vdsDataspace);
		H5Sclose(srcDataspace);
		H5Dclose(vdsdataset);



		//**paramter datasets**
		//dataspace
		hsize_t vdsdims_para[2] = {dstNDimx, numFiles};
		hid_t vdsDataspace_para = H5Screate_simple(2, vdsdims_para, NULL);
		if (vdsDataspace_para < 0)
			return CloseFileOnError(fd, string("Error in creating virtual dataspace (parameters) in virtual file ") + virtualfname + string("\n"));
		hsize_t srcdims_para[1] = {srcNDimx};
		hid_t srcDataspace_para = H5Screate_simple(1, srcdims_para, NULL);
		if (srcDataspace_para < 0)
			return CloseFileOnError(fd, string("Error in creating source dataspace (parameters) in virtual file ") + virtualfname + string("\n"));

		//fill values
		hid_t dcpl1 = H5Pcreate (H5P_DATASET_CREATE);
		hid_t dcpl2 = H5Pcreate (H5P_DATASET_CREATE);
		if ((dcpl1 < 0) || (dcpl2 < 0))
			return CloseFileOnError(fd, string("Error in creating file creation properties (parameters) in virtual file ") + virtualfname + string("\n"));
		fill_value = -1;
		if ((H5Pset_fill_value (dcpl1, dtype_para1, &fill_value) < 0) || (H5Pset_fill_value (dcpl2, dtype_para2, &fill_value) < 0))
			return CloseFileOnError(fd, string("Error in creating fill value (parameters) in virtual file ") + virtualfname + string("\n"));

		//hyperslab
		hsize_t offset_para[2] = {0,0},
				count_para[2] = {dstNDimx,1};
		error = false;
		for (int i = 0; i < numFiles; i++) {
			//cout<<"("<<offset[0]<<","<<offset[1]<<","<<offset[2]<<")"<<endl;
			if (H5Sselect_hyperslab (vdsDataspace_para, H5S_SELECT_SET, offset_para, NULL, count_para, NULL) < 0) {
				cprintf(RED,"could not select hyperslab\n");
				error = true;
				break;
			}
			if (H5Pset_virtual(dcpl1, vdsDataspace_para, fileNames[i].c_str(), spara1DatasetName.c_str(), srcDataspace_para) < 0) {
				cprintf(RED,"could not set mapping for paramter 1\n");
				error = true;
				break;
			}
			if (H5Pset_virtual(dcpl2, vdsDataspace_para, fileNames[i].c_str(), spara2DatasetName.c_str(), srcDataspace_para) < 0) {
				cprintf(RED,"could not set mapping for paramter 2\n");
				error = true;
				break;
			}
			offset_para[1] ++;
		}
		if (error)
			return CloseFileOnError(fd, string("Error in mapping files (parameters) in virtual file ") + virtualfname + string("\n"));

		//dataset
		hid_t vdsdataset1 = H5Dcreate2 (fd, vpara1DatasetName.c_str(), dtype_para1, vdsDataspace_para, H5P_DEFAULT, dcpl1, H5P_DEFAULT);
		hid_t vdsdataset2 = H5Dcreate2 (fd, vpara2DatasetName.c_str(), dtype_para2, vdsDataspace_para, H5P_DEFAULT, dcpl2, H5P_DEFAULT);
		if ((vdsdataset1 < 0) && (vdsdataset1 < 0))
			return CloseFileOnError(fd, string("Error in creating virutal dataset (parameters) in virtual file ") + virtualfname + string("\n"));

		//close
		H5Sclose(vdsDataspace_para);
		H5Sclose(srcDataspace_para);
		H5Dclose(vdsdataset1);
		H5Dclose(vdsdataset2);

		H5Fclose(fd); fd = 0;
		return 0;
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
	int LinkVirtualInMaster(string masterFileName, string virtualfname, string virtualDatasetname,
			string virtualpara1DatasetName, string virtualpara2DatasetName) {
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
		hid_t vdset1 = H5Dopen2( vfd, virtualpara1DatasetName.c_str(), H5P_DEFAULT);
		hid_t vdset2 = H5Dopen2( vfd, virtualpara2DatasetName.c_str(), H5P_DEFAULT);
		if ((vdset1 < 0) || (vdset2 < 0)) {
			H5Fclose(mfd); mfd = 0;
			return CloseFileOnError( vfd, string("Error in opening virtual parameter dataet to create link\n"));
		}
		sprintf(linkname, "/entry/data/%s",virtualpara1DatasetName.c_str());
		if(H5Lcreate_external( virtualfname.c_str(), virtualpara1DatasetName.c_str(),
				mfd, linkname, H5P_DEFAULT, H5P_DEFAULT) < 0) {
			H5Fclose(mfd); mfd = 0;
			return CloseFileOnError( vfd, string("Error in creating link to virtual parameter 1 dataset\n"));
		}
		sprintf(linkname, "/entry/data/%s",virtualpara2DatasetName.c_str());
		if(H5Lcreate_external( virtualfname.c_str(), virtualpara2DatasetName.c_str(),
				mfd, linkname, H5P_DEFAULT, H5P_DEFAULT) < 0) {
			H5Fclose(mfd); mfd = 0;
			return CloseFileOnError( vfd, string("Error in creating link to virtual parameter 2 dataset\n"));
		}
		H5Dclose(vdset1);
		H5Dclose(vdset2);
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
		if(fd > 0) H5Fclose(fd); fd = 0;
		return 1;
	}


};




#endif
