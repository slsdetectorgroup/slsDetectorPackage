/********************************************//**
 * @file FileWriter.h
 * @short sets/gets properties for the file, creates/closes the file and writes data to it
 ***********************************************/
#ifndef FILE_WRITER_H
#define FILE_WRITER_H



/**
 *@short sets/gets properties for the file, creates/closes the file and writes data to it
 */

class FileWriter : private virtual slsReceiverDefs {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 */
	FileWriter();

	/**
	 * Destructor
	 */
	~FileWriter();

 private:

};

#endif
