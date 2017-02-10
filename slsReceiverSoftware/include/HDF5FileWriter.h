/************************************************
 * @file HDF5FileWriter.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
#ifndef HDF5_FILE_WRITER_H
#define HDF5_FILE_WRITER_H

#include "FileWriter.h"

/**
 *@short sets/gets properties for the HDF5 file, creates/closes the file and writes data to it
 */

class HDF5FileWriter : private virtual slsReceiverDefs, public FileWriter {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param frindexenable pointer to frame index enable
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 */
	HDF5FileWriter(int ind, char* fname, char* fpath, uint64_t* findex,
			bool* frindexenable, bool* owenable, int* dindex, int* nunits);

	/**
	 * Destructor
	 */
	~HDF5FileWriter();

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
	 * Close File
	 */
	void CloseFile();

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
	 * @param owenable overwrite enable
	 * @param fname complete file name
	 * @returns OK or FAIL
	 */
	static int CreateDataFile(bool owenable, char* fname);

	/**
	 * Close File
	 */
	static void CloseDataFile();

 private:

	/**
	 * Get Type
	 * @return type
	 */
	fileFormat GetType();

};

#endif
