#pragma once
/************************************************
 * @file BinaryFile.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the binary file, creates/closes the file and writes data to it
 */

#include "File.h"
#include "BinaryFileStatic.h"

#include <string>

class Fifo;

class BinaryFile : private virtual slsReceiverDefs, public File, public BinaryFileStatic {
	
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
	BinaryFile(int ind, uint32_t maxf, const uint32_t* ppf,
			int* nd, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable,
			int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno, Fifo*& f);

	/**
	 * Destructor
	 */
	~BinaryFile();

	/**
	 * Print all member values
	 */
	void PrintMembers();

	/**
	 * Create file
	 * @param fnum current frame index to include in file name
	 * @returns OK or FAIL
	 */
	int CreateFile(uint64_t fnum);

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
	 * @param buffersize size of buffer
	 * @param fnum current image number
	 * @param nump number of packets caught
	 * @returns OK or FAIL
	 */
	 int WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump);



 private:

	/**
	 * Get Type
	 * @return type
	 */
	 fileFormat GetFileType();



	/** File Descriptor */
	FILE* filefd;

	/** Master File Descriptor */
	static FILE* masterfd;

	/** Number of frames in file */
	uint32_t numFramesInFile;

	/** Number of actual packets caught in file */
	uint64_t numActualPacketsInFile;

};

