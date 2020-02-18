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
#include <mutex>


class HDF5File : private virtual slsDetectorDefs, public File, public HDF5FileStatic {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @param ind self index
	 * @param maxf pointer to max frames per file
	 * @param nd pointer to number of detectors in each dimension
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of images in acquisition
	 * @param dr pointer to dynamic range
	 * @param portno pointer to udp port number for logging
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 * @param smode pointer to silent mode
	 */
	HDF5File(int ind, uint32_t* maxf,
			int* nd, std::string* fname, std::string* fpath, uint64_t* findex, bool* owenable,
			int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
			uint32_t nx, uint32_t ny,
			bool* smode);

	/**
	 * Destructor
	 */
	~HDF5File();

	/**
	 * Print all member values
	 */
	void PrintMembers(TLogLevel level = logDEBUG1);

	/**
	 * Set Number of pixels
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 */
	void SetNumberofPixels(uint32_t nx, uint32_t ny);

	/**
	 * Create file
	 * @param fnum current frame index to include in file name
	 */
	void CreateFile();

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
	 * @param nump number of packets caught
	 */
	void WriteToFile(char* buffer, int bsize, uint64_t fnum, uint32_t nump);

	/**
	 * Create master file
	 * @param mfwenable master file write enable
	 * @param attr master file attributes
	 */
	void CreateMasterFile(bool mfwenable, masterAttributes& attr);

	/**
	 * End of Acquisition
	 * @param anyPacketsCaught true if any packets are caught, else false
	 * @param numf number of images caught
	 */
	void EndofAcquisition(bool anyPacketsCaught, uint64_t numf);


 private:

	/**
	 * Create Virtual File
	 * @param numf number of images caught
	 */
	void CreateVirtualFile(uint64_t numf);

	/**
	 * Link virtual file in master file
	 * Only for Jungfrau at the moment for 1 module and 1 data file
	 */
	void LinkVirtualFileinMasterFile();

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
	static mutable std::mutex mutex;

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

	/** Number of actual packets caught in file */
	uint64_t numActualPacketsInFile;

	/** Number of files in an acquisition - to verify need of virtual file */
	int numFilesinAcquisition;

	/** parameter names */
	std::vector <const char*> parameterNames;

	/** parameter data types */
	std::vector <DataType> parameterDataTypes;

	/** Dataspace of parameters */
	DataSpace* dataspace_para;

	/** Dataset array for parameters */
	std::vector <DataSet*> dataset_para;

	/** Number of Images (including extended during acquisition) */
	uint64_t extNumImages;

};
#endif
