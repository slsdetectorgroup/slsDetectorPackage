#include "File.h"

#include <iostream>

File::File(slsDetectorDefs::fileFormat type) : type_(type) {}

File::~File() {}

slsDetectorDefs::fileFormat File::GetFileType() { return type_; }
