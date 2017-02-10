/************************************************
 * @file File.h
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/
#ifndef FILE_H
#define FILE_H
/**
 *@short sets/gets properties for the file, creates/closes the file and writes data to it
 */

#include "sls_receiver_defs.h"
#include "logger.h"


#include <string>

class File : private virtual slsReceiverDefs {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @param ind self index
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param frindexenable pointer to frame index enable
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 */
	File(int ind, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable, int* dindex, int* nunits);

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
	virtual fileFormat GetType() = 0;

	/**
	 * Get Member Pointer Values before the object is destroyed
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param frindexenable pointer to frame index enable
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 */
	void GetMemberPointerValues(char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable, int* dindex, int* nunits);

	/**
	 * Create file
	 * @param fnum current frame index to include in file name
	 * @returns OK or FAIL
	 */
	virtual int CreateFile(uint64_t fnum){
		cprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
		return OK;
	}

	/**
	 * Close File
	 */
	virtual void CloseFile() {
		cprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	}


	// Binary specific
	/**
	 * Set Max frames per file
	 * @param maxf maximum frames per file
	 */
	virtual void SetMaxFramesPerFile(uint32_t maxf) {
		cprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	}



 protected:

	/** Self Index */
	int index;

	/** File Name Prefix */
	char* fileNamePrefix;

	/** File Path */
	char* filePath;

	/** File Index */
	uint64_t* fileIndex;

	/** Frame Index */
	bool* frameIndexEnable;

	/** File Write Enable */
	bool* fileWriteEnable;

	/** Over write enable */
	bool* overWriteEnable;

	/** Detector Index */
	int* detIndex;

	/** Number of units per detector. Eg. Eiger has 2, others 1 */
	int* numUnitsPerDetector;

	/** Current File Name */
	std::string currentFileName;
};

#endif
