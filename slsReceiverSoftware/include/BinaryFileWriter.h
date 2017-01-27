/************************************************
 * @file BinaryFileWriter.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/
#ifndef BINARY_FILE_WRITER_H
#define BINARY_FILE_WRITER_H

#include "FileWriter.h"

/**
 *@short sets/gets properties for the binary file, creates/closes the file and writes data to it
 */

class BinaryFileWriter : private virtual slsReceiverDefs, public FileWriter {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @fname pointer to file name prefix
	 */
	BinaryFileWriter(char* fname);

	/**
	 * Destructor
	 */
	~BinaryFileWriter();

 private:

};

#endif
