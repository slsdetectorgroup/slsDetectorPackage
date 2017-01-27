/************************************************
 * @file BinaryFileWriter.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFileWriter.h"

#include <iostream>
using namespace std;


BinaryFileWriter::BinaryFileWriter(char* fname):
		FileWriter(fname) {

}


BinaryFileWriter::~BinaryFileWriter() {

}

