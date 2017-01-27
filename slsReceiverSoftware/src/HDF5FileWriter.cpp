/************************************************
 * @file HDF5FileWriter.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "HDF5FileWriter.h"

#include <iostream>
using namespace std;


HDF5FileWriter::HDF5FileWriter(char* fname):
				FileWriter(fname) {

}


HDF5FileWriter::~HDF5FileWriter() {

}


