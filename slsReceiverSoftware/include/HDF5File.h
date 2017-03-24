#ifdef HDF5C
#pragma once
/************************************************
 * @file HDF5File.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the HDF5 file, creates/closes the file and writes data to it
 */


#include "File.h"
#include "HDF5FileStatic.h"

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif
#include <string>

class HDF5File : private virtual slsReceiverDefs, public File, public HDF5FileStatic {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @param ind self index
	 * @param nd pointer to number of detectors in each dimension
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param frindexenable pointer to frame index enable
	 * @param owenable pointer to over write enable
	 * @param maxf max frames per file
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of frames
	 * @param dr dynamic range
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 */
	HDF5File(int ind, int* nd, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable, uint32_t maxf, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr,
			uint32_t nx, uint32_t ny);

	/**
	 * Destructor
	 */
	~HDF5File();

	/**
	 * Print all member values
	 */
	void PrintMembers();

	/**
	 * Set Number of pixels
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 */
	void SetNumberofPixels(uint32_t nx, uint32_t ny);

	/**
	 * Create file
	 * @param fnum current frame index to include in file name
	 * @returns OK or FAIL
	 */
	int CreateFile(uint64_t fnum);

	/**
	 * Close Current File
	 */
	void CloseCurrentFile();

	/**
	 * Close all Files
	 */
	void CloseAllFiles();

	/**
	 * Write data to file
	 * @param buffer buffer to write from
	 * @param bsize size of buffer (not used)
	 * @param fnum current image number
	 * @returns OK or FAIL
	 */
	int WriteToFile(char* buffer, int bsize, uint64_t fnum);

	/**
	 * Create master file
	 * @param en ten giga enable
	 * @param size image size
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 * @param at acquisition time
	 * @param ap acquisition period
	 * @returns OK or FAIL
	 */
	int CreateMasterFile(bool en, uint32_t size,
			uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap);

	/**
	 * End of Acquisition
	 * @param numf number of images caught
	 */
	void EndofAcquisition(uint64_t numf);

	/**
	 * Create Virtual File
	 * @param numf number of images caught
	 * @returns OK or FAIL
	 */
	int CreateVirtualFile(uint64_t numf);


 private:

	/**
	 * Get Type
	 * @return type
	 */
	fileFormat GetFileType();

	/**
	 * Updates data type depending on current dynamic range
	 */
	void UpdateDataType();



	/** mutex to update static items among objects (threads)*/
	static pthread_mutex_t Mutex;

	/** Master File handle */
	static H5File* masterfd;

	/** Virtual File handle ( only file name because code in C as H5Pset_virtual doesnt exist yet in C++) */
	static hid_t virtualfd;

	/** File handle */
	H5File* filefd;

	/** Dataspace handle */
	DataSpace* dataspace;

	/** DataSet handle */
	DataSet* dataset;

	/** Datatype of dataset */
	DataType datatype;

	/** Number of pixels in x direction */
	uint32_t nPixelsX;

	/** Number of pixels in y direction */
	uint32_t nPixelsY;

	/** Number of frames in file */
	uint32_t numFramesInFile;

	/** Number of files in an acquisition - to verify need of virtual file */
	int numFilesinAcquisition;

	//parameters
	/** Dataspace of parameters */
	DataSpace* dataspace_para;

	/** parameter1 */
	std::string para1;

	/** Dataset of parameter1 */
	DataSet* dataset_para1;

	/** Datatype of parameter1 */
	DataType datatype_para1;

	/** parameter2 */
	std::string para2;

	/** Dataset of parameter2 */
	DataSet* dataset_para2;

	/** Datatype of parameter2 */
	DataType datatype_para2;


};
#endif
