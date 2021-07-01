#include "File.h"

#include <iostream>

File::File(const slsDetectorDefs::fileFormat format) : format_(format) {}

File::~File() {}

slsDetectorDefs::fileFormat File::GetFileFormat() const { return format_; }
