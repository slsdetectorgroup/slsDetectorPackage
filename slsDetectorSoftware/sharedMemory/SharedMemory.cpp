#include "SharedMemory.h"
#include "sls_detector_exceptions.h"
#include "ansi.h"

#include <iostream>
#include <stdio.h>      // printf
#include <cerrno>       // errno
#include <cstring>      // strerror
#include <unistd.h>
#include <fcntl.h>      // O_CREAT, O_TRUNC..
#include <sys/stat.h>   // fstat
#include <sys/mman.h>   // shared memory
#include <sstream>


SharedMemory::SharedMemory(int multiId, int singleId):
    fd(-1),
	shmSize(0)
{
	name = ConstructSharedMemoryName(multiId, singleId);
}



SharedMemory::~SharedMemory(){
	if (fd >= 0)
		close(fd);
}


bool SharedMemory::IsExisting(std::string name) {
    bool ret = true;
    int fd = shm_open(name.c_str(), O_RDWR, 0);
    if ((fd < 0) && (errno == ENOENT)) {
       ret = false;
    }
    close(fd);
    return ret;
}

std::string SharedMemory::GetName() {
    return name;
}


void* SharedMemory::CreateSharedMemory(size_t sz){
    // create
    fd = shm_open(name.c_str(), O_CREAT | O_TRUNC | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        cprintf(RED, "Error: Create shared memory %s failed: %s\n",
        		name.c_str(), strerror(errno));
        throw SharedMemoryException();
    }

    // resize
    if (ftruncate(fd, sz) < 0) {
        cprintf(RED, "Error: Create shared memory %s failed at ftruncate: %s\n",
        		name.c_str(), strerror(errno));
        close(fd);
        throw SharedMemoryException();
    }

    // map
    return MapSharedMemory(sz);
}

void* SharedMemory::OpenSharedMemory(size_t sz){
    // open
    fd = shm_open(name.c_str(), O_RDWR, 0);
    if (fd < 0) {
    	cprintf(RED, "Error: Open existing shared memory %s failed: %s\n",
    			name.c_str(), strerror(errno));
        throw SharedMemoryException();
    }

    return MapSharedMemory(sz);
}


void SharedMemory::UnmapSharedMemory(void* addr) {
    if (munmap(addr, shmSize) < 0) {
        cprintf(RED, "Error: Unmapping shared memory %s failed: %s\n",
        		name.c_str(), strerror(errno));
        close(fd);
        throw SharedMemoryException();
    }
}

void SharedMemory::RemoveSharedMemory() {
	RemoveSharedMemory(name.c_str());
}


void* SharedMemory::MapSharedMemory(size_t sz) {
    void* addr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        cprintf(RED, "Error: Mapping shared memory %s failed: %s\n",
        		name.c_str(), strerror(errno));
        close(fd);
        throw SharedMemoryException();
    }
    shmSize = sz;
    close(fd);
    return addr;
}


std::string SharedMemory::ConstructSharedMemoryName(int multiId, int singleId) {
	stringstream ss;
	if (singleId < 0)
		ss << "/slsDetectorPackage_multi_" << multiId;
	else
		ss << "/slsDetectorPackage_multi_" << multiId << "_single_" << singleId;

	std::string temp = ss.str();
	if (temp.length() > NAME_MAX) {
		 cprintf(RED, "Error: Shared memory initialization %s failed: %s\n",
				 name.c_str(), strerror(errno));
		 throw SharedMemoryException();
	}
	return temp;
}


int SharedMemory::VerifySizeMatch(size_t expectedSize) {
    struct stat sb;
    // could not fstat
    if (fstat(fd, &sb) < 0) {
        cprintf(RED, "Error: Could not verify existing shared memory %s size match "
        		"(could not fstat): %s\n", name.c_str(), strerror(errno));
        close(fd);
        throw SharedMemoryException();
    }

    //size does not match
    long unsigned int sz = (long unsigned int)sb.st_size;
    if (sz != expectedSize) {
    	cprintf(RED, "Warning: Existing shared memory %s size does not match.\n",
    			name.c_str());
#ifdef VERBOSE
    	cprintf(RED, " Expected %ld, found %ld\n", expectedSize, sz);
#endif
    	throw SharedMemoryException();
    	return 1;
    }
    return 0;
}

void SharedMemory::RemoveSharedMemory(std::string name) {
    if (shm_unlink(name.c_str()) < 0) {
        // silent exit if shm did not exist anyway
        if (errno == ENOENT)
            return;
        cprintf(RED, "Error: Free Shared Memory %s Failed: %s\n",
        		name.c_str(), strerror(errno));
        throw SharedMemoryException();
    }
    printf("Shared memory deleted %s \n", name.c_str());
}

