/************************************************
 * @file HDF5File.cpp
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
#include "HDF5FileStatic.h"

const char * const HDF5FileStatic::ParameterNames[] = {
			"frameNumber",
			"expLength",
			"packetNumber",
			"bunchId",
			"timestamp",
			"modId",
			"xCoord",
			"yCoord",
			"zCoord",
			"debug",
			"roundRNumber",
			"detType",
			"version"};
const DataType HDF5FileStatic::ParameterDataTypes[] = {
			PredType::STD_U64LE,
			PredType::STD_U32LE,
			PredType::STD_U32LE,
			PredType::STD_U64LE,
			PredType::STD_U64LE,
			PredType::STD_U16LE,
			PredType::STD_U16LE,
			PredType::STD_U16LE,
			PredType::STD_U16LE,
			PredType::STD_U32LE,
			PredType::STD_U16LE,
			PredType::STD_U8LE,
			PredType::STD_U8LE};


