/************************************************
 * @file HDF5File.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
//#define HDF5C
//#ifdef HDF5C
#ifndef HDF5_FILE_H
#define HDF5_FILE_H
/**
 *@short sets/gets properties for the HDF5 file, creates/closes the file and writes data to it
 */


#include "File.h"
#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

#include <string>

class HDF5File : private virtual slsReceiverDefs, public File {
	
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
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of frames
	 * @param dr dynamic range
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 */
	HDF5File(int ind, int* nd, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr,
			int nx, int ny);

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




	//*** static functions ***

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
			uint64_t fnum = 0, int dindex = -1, int numunits = 1, int unitindex = 0);

	/**
	 * Create master file name
	 * @param fpath file path
	 * @param fnameprefix file name prefix (includes scan and position variables)
	 * @param findex file index
	 * @returns master file name
	 */
	static std::string CreateMasterFileName(char* fpath, char* fnameprefix, uint64_t findex);

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
	static std::string CreateVirtualFileName(char* fpath, char* fnameprefix, uint64_t findex, bool frindexenable,
			uint64_t fnum);

	/**
	 * Close File
	 * @param ind index for debugging
	 * @param fd file pointer
	 * @param dp dataspace pointer
	 * @param ds dataset pointer
	 */

	static void CloseDataFile(int ind, H5File*& fd, DataSpace*& dp, DataSet*& ds);

	/*
	  * Close master file
	 */
	void CloseMasterDataFile();

	 /*
	  * Close virtual file
	 */
	void CloseVirtualDataFile();

	/**
	 * Write data to file
	 * @param ind object index for debugging
	 * @param buf buffer to write from
	 * @param numImages number of images
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 * @param fnum current image number
	 * @param dspace dataspace pointer
	 * @param dset dataset pointer
	 * @param dtype datatype
	 * @returns OK or FAIL
	 */
	static int WriteDataFile(int ind, char* buf, uint64_t numImages, int nx, int ny, uint64_t fnum,
			DataSpace* dspace, DataSet* dset, DataType dtype);

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
	 * @returns OK or FAIL
	 */
	int CreateMasterDataFile(std::string fname, bool owenable,
			uint32_t dr, bool tenE,	uint32_t size, uint32_t nx, uint32_t ny, uint64_t nf,
			uint64_t acquisitionTime, uint64_t acquisitionPeriod);

	/**
	 * Create Virtual File
	 * @param fnum frame number
	 */
	int CreateVirtualFile(uint64_t fnum);

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
	 * @returns OK or FAIL
	 */
	static int CreateDataFile(int ind, bool owenable, uint64_t numf, std::string fname, bool frindexenable, uint64_t fnum, int nx, int ny,
			DataType dtype, H5File*& fd, DataSpace*& dspace, DataSet*& dset);

	/**
	 * Create virtual file
	 * @param virtualfname virtual file name
	 * @param virtualDatasetname virtual dataset name
	 * @param srcDatasetname source dataset name
	 * @param numFiles number of files
	 * @param fileNames array of file names
	 * @param owenable overwrite enable
	 * @param fnum current frame number
	 * @param dtype datatype
	 * @param srcNDimx source number of objects in x dimension (Number of images)
	 * @param srcNDimy source number of objects in y dimension (Number of pixels in y dir)
	 * @param srcNDimz source number of objects in z dimension (Number of pixels in x dir)
	 * @param dstNDimx destination number of objects in x dimension (Number of images)
	 * @param dstNDimy destination number of objects in y dimension (Number of pixels in y dir)
	 * @param dstNDimz destination number of objects in z dimension (Number of pixels in x dir)
	 * @returns OK or FAIL
	 */
	static int CreateVirtualDataFile(std::string virtualfname, std::string virtualDatasetname, std::string srcDatasetname,
			int numFiles, std::string fileNames[], bool owenable, uint64_t fnum, hid_t dtype,
			int srcNDimx, int srcNDimy, int srcNDimz, int dstNDimx, int dstNDimy, int dstNDimz);

	/**
	 * Copy file to another file (mainly to view virutal files in hdfviewer)
	 * @param owenable overwrite enable
	 * @param oldFileName file name including path of file to copy
	 * @param oldDatasetName dataset name to copy
	 * @param newFileName  file name including path of file to copy to
	 * @param newDatasetName dataset name to copy to
	 * @param nDimx Number of objects in x dimension
	 * @param nDimy Number of objects in y dimension
	 * @param nDimz Number of objects in z dimension
	 * @param dataType data type
	 * @returns OK or FAIL
	 */
	template <typename T>
	static int CopyVirtualFile(bool owenable, std::string oldFileName, std::string oldDatasetName,
			std::string newFileName, std::string newDatasetName, int nDimx, int nDimy, int nDimz, T datatype);


	void  CreateFinalFile();
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
	int nPixelsX;

	/** Number of pixels in y direction */
	int nPixelsY;

};

//#endif
#endif
