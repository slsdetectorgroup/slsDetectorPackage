/************************************************
 * @file BinaryFile.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/
#ifndef BINARY_FILE_H
#define BINARY_FILE_H
/**
 *@short sets/gets properties for the binary file, creates/closes the file and writes data to it
 */


#include "File.h"

#include <string>

class BinaryFile : private virtual slsReceiverDefs, public File {
	
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
	 * @param nf pointer to number of frames
	 * @param dr dynamic range
	 * @param maxf max frames per file
	 */
	BinaryFile(int ind, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t maxf);

	/**
	 * Destructor
	 */
	~BinaryFile();

	/**
	 * Print all member values
	 */
	void PrintMembers();

	/**
	 * Set Max frames per file
	 * @param maxf maximum frames per file
	 */
	void SetMaxFramesPerFile(uint32_t maxf);

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
	 * @param buffersize size of buffer
	 * @param fnum current image number
	 * @returns OK or FAIL
	 */
	 int WriteToFile(char* buffer, int buffersize, uint64_t fnum);

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
	 * Create File
	 * @param fd file pointer
	 * @param owenable overwrite enable
	 * @param fname complete file name
	 * @returns OK or FAIL
	 */
	static int CreateDataFile(FILE*& fd, bool owenable, std::string fname);

	/**
	 * Close File
	 * @param fd file pointer
	 */
	static void CloseDataFile(FILE*& fd);

	/**
	 * Write data to file
	 * @param fd file pointer
	 * @param buf buffer to write from
	 * @param bsize size of buffer
	 * @param fnum current image number
	 * @returns number of elements written
	 */
	static int WriteDataFile(FILE* fd, char* buf, int bsize, uint64_t fnum);

	 /**
	  * Create common files
	 * @param en ten giga enable
	 * @param size image size
	 * @param nx number of pixels in x direction
	 * @param ny number of pixels in y direction
	 * @param at acquisition time
	 * @param ap acquisition period
	  * @returns OK or FAIL
	  */
	 int CreateCommonFiles(bool en, uint32_t size,
				uint32_t nx, uint32_t ny, uint64_t at, uint64_t ap);


 private:

	/**
	 * Get Type
	 * @return type
	 */
	 fileFormat GetFileType();

	/** Maximum frames per file */
	uint32_t maxFramesPerFile;

	/** File Descriptor */
	FILE* filefd;

	/** Master File Descriptor */
	static FILE* masterfd;

	/**
	 * Create file names for master and virtual file
	 * @param m master file name
	 * @param fpath file path
	 * @param fnameprefix file name prefix (includes scan and position variables)
	 * @param findex file index
	 */
	void CreateCommonFileNames(std::string& m, char* fpath, char* fnameprefix, uint64_t findex);

	 /*
	  * Close master and virtual files
	 */
	void CloseCommonDataFiles();

	/**
	 * Create master and virtual files
	 * @param m master file name
	 * @param owenable overwrite enable
	 * @param tengigaEnable ten giga enable
	 * @param imageSize image size
	 * @param nPixelsX number of pixels in x direction
	 * @param nPixelsY number of pixels in y direction
	 * @param acquisitionTime acquisition time
	 * @param acquisitionPeriod acquisition period
	 * @returns OK or FAIL
	 */
	int CreateCommonDataFiles(std::string m, bool owenable,
			bool tengigaEnable, uint32_t imageSize, uint32_t nPixelsX, uint32_t nPixelsY,
			uint64_t acquisitionTime, uint64_t acquisitionPeriod);


};

#endif
