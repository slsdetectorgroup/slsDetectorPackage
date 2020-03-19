#include "file_utils.h"
#include "logger.h"
#include "sls_detector_exceptions.h"

#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int readDataFile(std::ifstream &infile, short int *data, int nch, int offset) {
	int ichan, iline=0;
	short int idata;
	int interrupt=0;
	std::string str;
	while (infile.good() and interrupt==0) {
		getline(infile,str);
		std::istringstream ssstr(str);
		ssstr >> ichan >> idata;
		if (ssstr.fail() || ssstr.bad()) {
			interrupt=1;
			break;
		}
		if (iline<nch) {
			if (ichan>=offset) {
				data[iline]=idata;
				iline++;
			}
		} else {
			interrupt=1;
			break;
		}
		return iline;
	};
	return iline;
}



int readDataFile(std::string fname, short int *data, int nch) {
	std::ifstream infile;
	int iline=0;
	std::string str;
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {
		iline=readDataFile(infile, data, nch, 0);
		infile.close();
	} else {
		LOG(logERROR) << "Could not read file " << fname;
		return -1;
	}
	return iline;
}



int writeDataFile(std::ofstream &outfile,int nch,  short int *data, int offset) {
	if (data==nullptr)
		return slsDetectorDefs::FAIL;
	for (int ichan=0; ichan<nch; ichan++)
		outfile << ichan+offset << " " << *(data+ichan) << std::endl;
	return slsDetectorDefs::OK;
}


int writeDataFile(std::string fname,int nch, short int *data) {
	std::ofstream outfile;
	if (data==nullptr)
		return slsDetectorDefs::FAIL;
	outfile.open (fname.c_str(),std::ios_base::out);
	if (outfile.is_open())       {
		writeDataFile(outfile, nch, data, 0);
		outfile.close();
		return slsDetectorDefs::OK;
	} else {
		LOG(logERROR) << "Could not open file " << fname << "for writing";
		return slsDetectorDefs::FAIL;
	}
}



void mkdir_p(const std::string& path, std::string dir) {
    if (path.length() == 0)
        return;

    size_t i = 0;
    for (; i < path.length(); i++) {
        dir += path[i];
        if (path[i] == '/')
            break;
    }
    if(mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
        if (errno != EEXIST)
            throw sls::RuntimeError("Could not create: " + dir);
    }

    if (i + 1 < path.length())
        mkdir_p(path.substr(i + 1), dir);
}




