/************************************************
 * @file FileWriter.h
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/
#ifndef FILE_WRITER_H
#define FILE_WRITER_H
/**
 *@short sets/gets properties for the file, creates/closes the file and writes data to it
 */

#include "sls_receiver_defs.h"
#include "logger.h"


class FileWriter : private virtual slsReceiverDefs {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @fname pointer to file name prefix
	 */
	FileWriter(char* fname);

	/**
	 * Destructor
	 */
	~FileWriter();

	/**
	 * Get File Name prefix
	 * @returns file name prefix
	 */
	char* GetFileName();

 protected:

	char* fileName;



};

#endif
