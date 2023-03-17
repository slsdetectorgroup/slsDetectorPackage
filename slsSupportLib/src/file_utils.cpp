// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/file_utils.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"

#include <errno.h>
#include <ios>
#include <iostream>
#include <libgen.h> // dirname
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> //readlink

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
        throw RuntimeError(errorPrefix + std::string(" (file does not exist)"));
    }

    FILE *fp = fopen(fname.c_str(), "rb");
    if (fp == nullptr) {
        throw RuntimeError(errorPrefix +
                           std::string(" (Could not open file: ") + fname +
                           std::string(")"));
    }

    // get file size to print progress
    ssize_t filesize = getFileSize(fp, errorPrefix);

    std::vector<char> buffer(filesize, 0);
    if ((ssize_t)fread(buffer.data(), sizeof(char), filesize, fp) != filesize) {
        throw RuntimeError(errorPrefix + std::string(" (Could not read file)"));
    }

    if (fclose(fp) != 0) {
        throw RuntimeError(errorPrefix +
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
            throw RuntimeError("Could not create: " + dir);
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

ssize_t getFileSize(FILE *fd, const std::string &prependErrorString) {
    if (fseek(fd, 0, SEEK_END) != 0) {
        throw RuntimeError(prependErrorString +
                           std::string(" (Seek error in src file)"));
    }
    size_t fileSize = ftell(fd);
    if (fileSize <= 0) {
        throw RuntimeError(
            prependErrorString +
            std::string(" (Could not get length of source file)"));
    }
    rewind(fd);
    return fileSize;
}

std::vector<int>
getChannelsFromStringList(const std::vector<std::string> list) {
    std::vector<int> channels;
    for (auto line : list) {

        // replace comma with space
        std::replace_if(
            begin(line), end(line), [](char c) { return (c == ','); }, ' ');

        // split line (delim space)
        std::vector<std::string> vec = split(line, ' ');

        // for every channel separated by space
        for (auto it : vec) {
            // find range and replace with sequence of x to y
            auto result = it.find(':');
            if (result != std::string::npos) {
                try {
                    int istart = StringTo<int>(it.substr(0, result));
                    int istop = StringTo<int>(
                        it.substr(result + 1, it.length() - result - 1));
                    LOG(logDEBUG1) << "istart:" << istart << " istop:" << istop;
                    std::vector<int> range(istop - istart);
                    std::generate(range.begin(), range.end(),
                                  [n = istart]() mutable { return n++; });
                    for (auto range_it : range) {
                        channels.push_back(range_it);
                    }
                } catch (std::exception &e) {
                    throw RuntimeError(
                        "Could not load channels. Invalid channel range: " +
                        it);
                }
            }

            // else convert to int
            else {
                int ival = 0;
                try {
                    ival = StringTo<int>(it);
                } catch (std::exception &e) {
                    throw RuntimeError(
                        "Could not load channels. Invalid channel number: " +
                        it);
                }
                channels.push_back(ival);
            }
        }
    }

    if (removeDuplicates(channels)) {
        LOG(logWARNING) << "Removed duplicates from channel file";
    }
    LOG(logDEBUG1) << "list:" << ToString(channels);
    return channels;
}

std::vector<int> getChannelsFromFile(const std::string &fname) {
    // read bad channels file
    std::ifstream input_file(fname);
    if (!input_file) {
        throw RuntimeError("Could not open bad channels file " + fname +
                           " for reading");
    }
    std::vector<std::string> lines;
    for (std::string line; std::getline(input_file, line);) {
        // ignore comments
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        // ignore empty lines
        if (line.empty()) {
            continue;
        }
        lines.push_back(line);
    }
    return getChannelsFromStringList(lines);
}

std::string getAbsolutePathFromCurrentProcess(const std::string &fname) {
    if (fname[0] == '/') {
        return fname;
    }

    // get path of current binary
    char path[MAX_STR_LENGTH];
    memset(path, 0, MAX_STR_LENGTH);
    ssize_t len = readlink("/proc/self/exe", path, MAX_STR_LENGTH - 1);
    if (len < 0) {
        throw RuntimeError("Could not get absolute path for " + fname);
    }
    path[len] = '\0';

    // get dir path and attach file name
    std::string absPath = (std::string(dirname(path)) + '/' + fname);
    return absPath;
}

} // namespace sls
