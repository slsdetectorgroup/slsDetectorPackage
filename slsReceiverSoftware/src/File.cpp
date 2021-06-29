#include "File.h"

#include <iostream>

File::File(int index, slsDetectorDefs::fileFormat type)
    : index_(index), type_(type) {}

File::~File() {}

slsDetectorDefs::fileFormat File::GetFileType() { return type_; }
