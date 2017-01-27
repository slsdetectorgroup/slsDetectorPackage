/************************************************
 * @file FileWriter.h
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "FileWriter.h"

#include <iostream>
using namespace std;


FileWriter::FileWriter(char* fname):
		fileName(fname) {
		cout<<"fileName:"<<fileName<<endl;
}

FileWriter::~FileWriter() {

}

char* FileWriter::GetFileName() {
	return fileName;
}
