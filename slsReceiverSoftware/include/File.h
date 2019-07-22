#pragma once
/************************************************
 * @file File.h
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the file, creates/closes the file and writes data to it
 */

#include "sls_detector_defs.h"
#include "logger.h"

#include <string>


class File : private virtual slsDetectorDefs {
	
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
	 * @param smode pointer to silent mode
	 */
	File(int ind, uint32_t* maxf,
			int* nd, char* fname, char* fpath, uint64_t* findex, bool* owenable,
			int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
			bool* smode);

	/**
	 * Destructor
	 */
	virtual ~File();

	/**
	 * Get Current File Name
	 * @returns current file name
	 */
	std::string GetCurrentFileName();


	void resetSubFileIndex();
	/**
	 * Print all member values
	 */
	virtual void PrintMembers(TLogLevel level = logDEBUG1);

	/**
	 * Get Type
	 * @return type
	 */
	virtual fileFormat GetFileType() = 0;

	/**
	 * Get Member Pointer Values before the object is destroyed
	 * @param nd pointer to number of detectors in each dimension
	 * @param maxf pointer to max frames per file
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of images in acquisition
	 * @param dr pointer to dynamic range
	 * @param portno pointer to dynamic range
	 */
	void GetMemberPointerValues(int* nd, uint32_t*& maxf, char*& fname, char*& fpath,
			uint64_t*& findex, bool*& owenable,
			int*& dindex, int*& nunits, uint64_t*& nf, uint32_t*& dr, uint32_t*& portno);

	/**
	 * Create file
	 * @returns OK or FAIL
	 */
	virtual int CreateFile() = 0;

	/**
	 * Close Current File
	 */
	virtual void CloseCurrentFile() = 0;

	/**
	 * Close Files
	 */
	virtual void CloseAllFiles() = 0;

	/**
	 * Write data to file
	 * @param buffer buffer to write from
	 * @param fnum current image number
	 * @param nump number of packets caught
	 * @param OK or FAIL
	 */
	virtual int WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) = 0;

	 /**
	  * Create master file
	  * @param mfwenable master file write enable
	  * @param en ten giga enable
	  * @param size image size
	  * @param nx number of pixels in x direction
	  * @param ny number of pixels in y direction
	  * @param at acquisition time
	  * @param st sub exposure time
	  * @param sp sub period
	  * @param ap acquisition period
	  * @returns OK or FAIL
	  */
	virtual int CreateMasterFile(bool mfwenable, bool en, uint32_t size,
				uint32_t nx, uint32_t ny, uint64_t at, uint64_t st,
				uint64_t sp, uint64_t ap) = 0;

	// HDf5 specific
	/**
	 * Set Number of pixels
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 */
	virtual void SetNumberofPixels(uint32_t nx, uint32_t ny) {
		FILE_LOG(logERROR) << "This is a generic function SetNumberofPixels that "
				"should be overloaded by a derived class";
	}
	

	/**
	 * End of Acquisition
	 * @param anyPacketsCaught true if any packets are caught, else false
	 * @param numf number of images caught
	 */
	virtual void EndofAcquisition(bool anyPacketsCaught, uint64_t numf) {
		FILE_LOG(logERROR) << "This is a generic function EndofAcquisition that "
				"should be overloaded by a derived class";
	}

 protected:

	/** master file writer/reader */
	bool master;

	/** Self Index */
	int index;

	/** Maximum frames per file */
	uint32_t* maxFramesPerFile;

	/** Master File Name */
	std::string masterFileName;

	/** Current File Name */
	std::string currentFileName;

	/** Number of Detectors in X dimension */
	int numDetX;

	/** Number of Detectors in Y dimension */
	int numDetY;

	/** File Name Prefix */
	char* fileNamePrefix;

	/** File Path */
	char* filePath;

	/** File Index */
	uint64_t* fileIndex;

	/** Sub file index */
	uint64_t subFileIndex{0};

	/** Over write enable */
	bool* overWriteEnable;

	/** Detector Index */
	int* detIndex;

	/** Number of units per detector. Eg. Eiger has 2, others 1 */
	int* numUnitsPerDetector;

	/** Number of images in acquisition */
	uint64_t* numImages;

	/** Dynamic Range */
	uint32_t* dynamicRange;

	/** UDP Port Number for logging */
	uint32_t* udpPortNumber;

	/** Silent Mode */
	bool* silentMode;

};

