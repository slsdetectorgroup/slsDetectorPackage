#include "SharedMemory.h"
#include "sls_detector_exceptions.h"
#include "ansi.h"
#include "logger.h"

#include <iostream>
#include <stdio.h>      // printf
#include <cerrno>       // errno
#include <cstring>      // strerror
#include <unistd.h>
#include <fcntl.h>      // O_CREAT, O_TRUNC..
#include <sys/stat.h>   // fstat
#include <sys/mman.h>   // shared memory
#include <sstream>
#include "stdlib.h"

#define SHM_MULTI_PREFIX "/slsDetectorPackage_multi_"
#define SHM_SLS_PREFIX "_sls_"
#define SHM_ENV_NAME	"SLSDETNAME"

SharedMemory::SharedMemory(int multiId, int slsId):
    fd(-1),
	shmSize(0)
{
	name = ConstructSharedMemoryName(multiId, slsId);
}



SharedMemory::~SharedMemory(){
	if (fd >= 0)
		close(fd);
}


bool SharedMemory::IsExisting() {
    bool ret = true;
    int tempfd = shm_open(name.c_str(), O_RDWR, 0);
    if ((tempfd < 0) && (errno == ENOENT)) {
       ret = false;
    }
    close(tempfd);
    return ret;
}

std::string SharedMemory::GetName() {
    return name;
}


void* SharedMemory::CreateSharedMemory(size_t sz){
    // create
    fd = shm_open(name.c_str(), O_CREAT | O_TRUNC | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        FILE_LOG(logERROR) << "Create shared memory " << name << " failed: " << strerror(errno);
        throw SharedMemoryException();
    }

    // resize
    if (ftruncate(fd, sz) < 0) {
    	FILE_LOG(logERROR) << "Create shared memory " << name << " failed at ftruncate: " << strerror(errno);
        close(fd);
        RemoveSharedMemory();
        throw SharedMemoryException();
    }

    // map
    void* addr = MapSharedMemory(sz);
    FILE_LOG(logINFO) << "Shared memory created " << name;
    return addr;
}

void* SharedMemory::OpenSharedMemory(size_t sz){
    // open
    fd = shm_open(name.c_str(), O_RDWR, 0);
    if (fd < 0) {
    	FILE_LOG(logERROR) << "Open existing shared memory " << name << " failed: " << strerror(errno);
        throw SharedMemoryException();
    }

    return MapSharedMemory(sz);
}


void SharedMemory::UnmapSharedMemory(void* addr) {
    if (munmap(addr, shmSize) < 0) {
    	FILE_LOG(logERROR) << "Unmapping shared memory " << name << " failed: " << strerror(errno);
        close(fd);
        throw SharedMemoryException();
    }
}

void SharedMemory::RemoveSharedMemory() {
    if (shm_unlink(name.c_str()) < 0) {
        // silent exit if shm did not exist anyway
        if (errno == ENOENT)
            return;
        FILE_LOG(logERROR) << "Free Shared Memory " << name << " Failed: " << strerror(errno);
        throw SharedMemoryException();
    }
    FILE_LOG(logINFO) << "Shared memory deleted " << name;
}


void* SharedMemory::MapSharedMemory(size_t sz) {
    void* addr = mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
    	FILE_LOG(logERROR) << "Mapping shared memory " << name << " failed: " << strerror(errno);
        close(fd);
        throw SharedMemoryException();
    }
    shmSize = sz;
    close(fd);
    return addr;
}


std::string SharedMemory::ConstructSharedMemoryName(int multiId, int slsId) {

	// using environment path
	std::string sEnvPath = "";
	char* envpath = getenv(SHM_ENV_NAME);
	if (envpath != nullptr) {
		sEnvPath.assign(envpath);
		sEnvPath.insert(0,"_");
	}

	std::stringstream ss;
	if (slsId < 0)
		ss << SHM_MULTI_PREFIX << multiId << sEnvPath;
	else
		ss << SHM_MULTI_PREFIX << multiId << SHM_SLS_PREFIX << slsId << sEnvPath;

	std::string temp = ss.str();
	if (temp.length() > NAME_MAX) {
		FILE_LOG(logERROR) << "Shared memory initialization failed. " <<
				 temp << " has " << temp.length() << " characters. \n"
				 "Maximum is " << NAME_MAX << ". Change the environment variable " << SHM_ENV_NAME;
		 throw SharedMemoryException();
	}
	return temp;
}


int SharedMemory::VerifySizeMatch(size_t expectedSize) {
    struct stat sb;
    // could not fstat
    if (fstat(fd, &sb) < 0) {
    	FILE_LOG(logERROR) << "Could not verify existing shared memory " << name << " size match "
        		"(could not fstat): " << strerror(errno);
        close(fd);
        throw SharedMemoryException();
    }

    //size does not match
    long unsigned int sz = (long unsigned int)sb.st_size;
    if (sz != expectedSize) {
    	FILE_LOG(logERROR) << "Existing shared memory " << name << " size does not match";
    	FILE_LOG(logDEBUG1) << "Expected " << expectedSize << ", found " << sz;
    	throw SharedMemoryException();
    	return 1;
    }
    return 0;
}

