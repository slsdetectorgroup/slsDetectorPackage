#pragma once
/************************************************
 * @file File.h
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the file, creates/closes the file and writes data to it
 */

#include "sls_receiver_defs.h"
#include "logger.h"

#include <string>

class Fifo;

class File : private virtual slsReceiverDefs {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @param ind self index
	 * @param maxf max frames per file
	 * @param ppf packets per frame
	 * @param nd pointer to number of detectors in each dimension
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param frindexenable pointer to frame index enable
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of images in acquisition
	 * @param dr pointer to dynamic range
	 * @param portno pointer to udp port number for logging
	 * @param fifo for logging fill level
	 */
	File(int ind, uint32_t maxf, const uint32_t* ppf,
			int* nd, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable,
			int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno, Fifo*& f);

	/**
	 * Destructor
	 */
	virtual ~File();

	/**
	 * Get Current File Name
	 * @returns current file name
	 */
	std::string GetCurrentFileName();

	/**
	 * Print all member values
	 */
	virtual void PrintMembers();

	/**
	 * Get Type
	 * @return type
	 */
	virtual fileFormat GetFileType() = 0;

	/**
	 * Get Member Pointer Values before the object is destroyed
	 * @param nd pointer to number of detectors in each dimension
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param frindexenable pointer to frame index enable
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of images in acquisition
	 * @param dr pointer to dynamic range
	 * @param portno pointer to dynamic range
	 */
	void GetMemberPointerValues(int* nd, char*& fname, char*& fpath, uint64_t*& findex,
			bool*& frindexenable, bool*& owenable,
			int*& dindex, int*& nunits, uint64_t*& nf, uint32_t*& dr, uint32_t*& portno);

	/**
	 * Set Max frames per file
	 * @param maxf maximum frames per file
	 */
	void SetMaxFramesPerFile(uint32_t maxf);

	/**
	 * Set Packets per frame (called only for each generalData construction)
	 * @param ppf pointer to packets per frame
	 */
	void SetPacketsPerFrame(const uint32_t* ppf);

	/**
	 * Set Fifo for logging fill level
	 * @param f fifo reference
	 */
	void SetFifo(Fifo*& f);


	/**
	 * Create file
	 * @param fnum current frame index to include in file name
	 * @returns OK or FAIL
	 */
	virtual int CreateFile(uint64_t fnum){
		cprintf(RED,"This is a generic function CreateFile that should be overloaded by a derived class\n");
		return OK;
	}

	/**
	 * Close Current File
	 */
	virtual void CloseCurrentFile() {
		cprintf(RED,"This is a generic function CloseCurrentFile that should be overloaded by a derived class\n");
	}

	/**
	 * Close Files
	 */
	virtual void CloseAllFiles() {
		cprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	}

	/**
	 * Write data to file
	 * @param buffer buffer to write from
	 * @param fnum current image number
	 * @param nump number of packets caught
	 * @param OK or FAIL
	 */
	virtual int WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) {
		cprintf(RED,"This is a generic function WriteToFile that should be overloaded by a derived class\n");
		return FAIL;
	}

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
	virtual int CreateMasterFile(bool en, uint32_t size,
				uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap) {
		cprintf(RED,"This is a generic function CreateMasterFile that should be overloaded by a derived class\n");
		return OK;
	}

	// HDf5 specific
	/**
	 * Set Number of pixels
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 */
	virtual void SetNumberofPixels(uint32_t nx, uint32_t ny) {
		cprintf(RED,"This is a generic function SetNumberofPixels that should be overloaded by a derived class\n");
	}

	/**
	 * End of Acquisition
	 * @param numf number of images caught
	 */
	virtual void EndofAcquisition(uint64_t numf) {
		cprintf(RED,"This is a generic function EndofAcquisition that should be overloaded by a derived class\n");
	}

 protected:

	/** master file writer/reader */
	bool master;

	/** Self Index */
	int index;

	/** Maximum frames per file */
	uint32_t maxFramesPerFile;

	/** Packets per frame for logging */
	//pointer because value in generalData could change
	const uint32_t* packetsPerFrame;

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

	/** Frame Index */
	bool* frameIndexEnable;

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

	/** Fifo structure for logging fill level*/
	Fifo* fifo;

};

