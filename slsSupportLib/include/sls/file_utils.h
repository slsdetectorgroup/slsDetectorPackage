#pragma once

#include "sls/sls_detector_defs.h"

#include <cstdio>
#include <fstream>
#include <string>

/** (used by multi and sls)
 * reads a short int raw data file
 * @param infile input file stream
 * @param data array of data values
 * @param nch number of channels
 * @param offset start channel value
 * @returns OK or FAIL if it could not read the file or data=NULL
 */
int readDataFile(std::ifstream &infile, short int *data, int nch,
                 int offset = 0);

/** (used by multi and sls)
 * reads a short int rawdata file
 * @param fname name of the file to be read
 * @param data array of data value
 * @param nch number of channels
 * @returns OK or FAIL if it could not read the file or data=NULL
 */
int readDataFile(std::string fname, short int *data, int nch);

/** (used by multi and sls)
 * writes a short int raw data file
 * @param outfile output file stream
 * @param nch number of channels
 * @param data array of data values
 * @param offset start channel number
 * @returns OK or FAIL if it could not write the file or data=NULL
 */
int writeDataFile(std::ofstream &outfile, int nch, short int *data,
                  int offset = 0);

/** (used by multi and sls)
 * writes a short int raw data file
 * @param fname of the file to be written
 * @param nch number of channels
 * @param data array of data values
 * @returns OK or FAIL if it could not write the file or data=NULL
 */
int writeDataFile(std::string fname, int nch, short int *data);

// mkdir -p path implemented by recursive calls
void mkdir_p(const std::string &path, std::string dir = "");

namespace sls {
int getFileSize(std::ifstream &ifs);
}
