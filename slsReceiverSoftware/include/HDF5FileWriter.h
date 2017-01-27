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
	 * @fname pointer to file name prefix
	 */
	HDF5FileWriter(char* fname);

	/**
	 * Destructor
	 */
	~HDF5FileWriter();

 private:

};

#endif
