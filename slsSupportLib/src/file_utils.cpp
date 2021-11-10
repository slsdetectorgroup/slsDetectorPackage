// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/file_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"

#include <errno.h>
#include <ios>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace sls {

int readDataFile(std::ifstream &infile, short int *data, int nch, int offset) {
    int ichan, iline = 0;
    short int idata;
    int interrupt = 0;
    std::string str;
    while (infile.good() and interrupt == 0) {
        getline(infile, str);
        std::istringstream ssstr(str);
        ssstr >> ichan >> idata;
        if (ssstr.fail() || ssstr.bad()) {
            interrupt = 1;
            break;
        }
        if (iline < nch) {
            if (ichan >= offset) {
                data[iline] = idata;
                iline++;
            }
        } else {
            interrupt = 1;
            break;
        }
        return iline;
    };
    return iline;
}

int readDataFile(std::string fname, short int *data, int nch) {
    std::ifstream infile;
    int iline = 0;
    std::string str;
    infile.open(fname.c_str(), std::ios_base::in);
    if (infile.is_open()) {
        iline = readDataFile(infile, data, nch, 0);
        infile.close();
    } else {
        LOG(logERROR) << "Could not read file " << fname;
        return -1;
    }
    return iline;
}

std::vector<char> readBinaryFile(const std::string &fname,
                                 const std::string &errorPrefix) {
    // check if it exists
    struct stat st;
    if (stat(fname.c_str(), &st) != 0) {
        throw sls::RuntimeError(errorPrefix +
                                std::string(" (file does not exist)"));
    }

    FILE *fp = fopen(fname.c_str(), "rb");
    if (fp == nullptr) {
        throw sls::RuntimeError(errorPrefix +
                                std::string(" (Could not open file: ") + fname + std::string(")"));
    }

    // get file size to print progress
    ssize_t filesize = sls::getFileSize(fp, errorPrefix);

    std::vector<char> buffer(filesize, 0);
    if ((ssize_t)fread(buffer.data(), sizeof(char), filesize, fp) != filesize) {
        throw sls::RuntimeError(errorPrefix +
                                std::string(" (Could not read file)"));
    }

    if (fclose(fp) != 0) {
        throw sls::RuntimeError(errorPrefix +
                                std::string(" (Could not close file)"));
    }

    LOG(logDEBUG1) << "Read file into memory";
    return buffer;
}

int writeDataFile(std::ofstream &outfile, int nch, short int *data,
                  int offset) {
    if (data == nullptr)
        return slsDetectorDefs::FAIL;
    for (int ichan = 0; ichan < nch; ichan++)
        outfile << ichan + offset << " " << *(data + ichan) << std::endl;
    return slsDetectorDefs::OK;
}

int writeDataFile(std::string fname, int nch, short int *data) {
    std::ofstream outfile;
    if (data == nullptr)
        return slsDetectorDefs::FAIL;
    outfile.open(fname.c_str(), std::ios_base::out);
    if (outfile.is_open()) {
        writeDataFile(outfile, nch, data, 0);
        outfile.close();
        return slsDetectorDefs::OK;
    } else {
        LOG(logERROR) << "Could not open file " << fname << "for writing";
        return slsDetectorDefs::FAIL;
    }
}

void mkdir_p(const std::string &path, std::string dir) {
    if (path.length() == 0)
        return;

    size_t i = 0;
    for (; i < path.length(); i++) {
        dir += path[i];
        if (path[i] == '/')
            break;
    }
    if (mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        if (errno != EEXIST)
            throw sls::RuntimeError("Could not create: " + dir);
    }

    if (i + 1 < path.length())
        mkdir_p(path.substr(i + 1), dir);
}

int getFileSize(std::ifstream &ifs) {
    auto current_pos = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    int file_size = ifs.tellg();
    ifs.seekg(current_pos);
    return file_size;
}

std::string getFileNameFromFilePath(const std::string &fpath) {
    std::string fname(fpath);
    std::size_t slashPos = fpath.rfind('/');
    if (slashPos != std::string::npos) {
        fname = fpath.substr(slashPos + 1, fpath.size() - 1);
    }
    return fname;
}

ssize_t getFileSize(FILE* fd, const std::string &prependErrorString) {
    if (fseek(fd, 0, SEEK_END) != 0) {
        throw RuntimeError(prependErrorString + std::string(" (Seek error in src file)"));
    }
    size_t fileSize = ftell(fd);
    if (fileSize <= 0) {
        throw RuntimeError(prependErrorString + std::string(" (Could not get length of source file)"));
    }
    rewind(fd);
    return fileSize;
}


} // namespace sls
