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
#include "sls_detector_defs.h"
#include "logger.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdlib.h>	 //malloc
#include <sstream>
#include <cstring>	//memset

class HDF5FileStatic: public virtual slsDetectorDefs {

public:










/**
	* Create virtual file
	* (in C because H5Pset_virtual doesnt exist yet in C++)
	* @param virtualFileName virtual file name
	* @param fd virtual file handle
	* @param masterFileName master file name
	* @param fpath file path
	* @param fnameprefix file name prefix 
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
	*/


};



#endif
