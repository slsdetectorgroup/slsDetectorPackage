#pragma once
/************************************************
 * @file SharedMemory.h
 * @short functions basic implemenation of
 * shared memory
 ***********************************************/
/**
 *@short functions basic implemenation of shared memory
 */

#include "ansi.h"
#include "logger.h"
#include "sls_detector_exceptions.h"

#include "stdlib.h"
#include <cerrno>  // errno
#include <cstring> // strerror
#include <fcntl.h> // O_CREAT, O_TRUNC..
#include <iostream>
#include <sstream>
#include <stdio.h>    // printf
#include <sys/mman.h> // shared memory
#include <sys/stat.h> // fstat
#include <unistd.h>

#define SHM_MULTI_PREFIX "/slsDetectorPackage_multi_"
#define SHM_SLS_PREFIX "_sls_"
#define SHM_ENV_NAME "SLSDETNAME"

#include <iostream>
#include <string>

template <typename T>
class SharedMemory {
  public:
    /**
	 * Constructor
	 * creates the single/multi detector shared memory name
	 * @param multiId multi detector id
     * @param slsId sls detector id, -1 if a multi detector shared memory
 	 */
    SharedMemory(int multiId, int slsId){
        name = ConstructSharedMemoryName(multiId, slsId);
    }

    /**
	 * Destructor
	 */
    ~SharedMemory() {
        if (fd >= 0)
            close(fd);

        if (shared_struct) {
            UnmapSharedMemory();
        }
    }

    /**
     * Verify if it exists
     * @param name of shared memory
     * @return true if exists, else false
     */
    bool IsExisting() {
        bool ret = true;
        int tempfd = shm_open(name.c_str(), O_RDWR, 0);
        if ((tempfd < 0) && (errno == ENOENT)) {
            ret = false;
        }
        close(tempfd);
        return ret;
    }

    /**
	 * Get shared memory name
	 */
    std::string GetName() {
        return name;
    }

    /**
     * Create Shared memory and call MapSharedMemory to map it to an address
     * throws a SharedMemoryException exception on failure to create, ftruncate or map
     * @param sz of shared memory
     */
    void CreateSharedMemory(size_t sz = 0) {
        // create
        if (sz == 0) {
            sz = sizeof(T);
        }

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
        // void* addr = MapSharedMemory(sz);
        shared_struct = MapSharedMemory(sz);
        FILE_LOG(logINFO) << "Shared memory created " << name;

        // return addr;
    }

    /**
     * Open existing Shared memory and call MapSharedMemory to map it to an address
     * throws a SharedMemoryException exception on failure to open or map
     * @param sz of shared memory
     */
    void OpenSharedMemory(size_t sz = 0) {
        // open
        if (sz == 0) {
            sz = sizeof(T);
        }

        fd = shm_open(name.c_str(), O_RDWR, 0);
        if (fd < 0) {
            FILE_LOG(logERROR) << "Open existing shared memory " << name << " failed: " << strerror(errno);
            throw SharedMemoryException();
        }

        shared_struct = MapSharedMemory(sz);
        // return MapSharedMemory(sz);
    }

    /**
     * Unmap shared memory from an address
     * throws a SharedMemoryException exception on failure
     */
    void UnmapSharedMemory() {
        if (shared_struct != nullptr) {
            if (munmap(shared_struct, shmSize) < 0) {
                FILE_LOG(logERROR) << "Unmapping shared memory " << name << " failed: " << strerror(errno);
                close(fd);
                throw SharedMemoryException();
            }
            shared_struct = nullptr;
        }
    }

    /**
	 * Remove existing Shared memory
	 */
    void RemoveSharedMemory() {
        UnmapSharedMemory();
        if (shm_unlink(name.c_str()) < 0) {
            // silent exit if shm did not exist anyway
            if (errno == ENOENT)
                return;
            FILE_LOG(logERROR) << "Free Shared Memory " << name << " Failed: " << strerror(errno);
            throw SharedMemoryException();
        }
        FILE_LOG(logINFO) << "Shared memory deleted " << name;
    }

    /**
     * Maximum length of name as from man pages
     */
    static const int NAME_MAX = 255;

    /*
    Using the call operator to access the pointer

    */

    T *operator()() {
        return shared_struct;
    }

    const T *operator()() const {
        return shared_struct;
    }

  private:
    /**
     * Create Shared memory name
     * throws exception if name created is longer than required 255(manpages)
     * @param multiId multi detector id
     * @param slsId sls detector id, -1 if a multi detector shared memory
     * @returns shared memory name
     */
    std::string ConstructSharedMemoryName(int multiId, int slsId) {

        // using environment path
        std::string sEnvPath = "";
        char *envpath = getenv(SHM_ENV_NAME);
        if (envpath != nullptr) {
            sEnvPath.assign(envpath);
            sEnvPath.insert(0, "_");
        }

        std::stringstream ss;
        if (slsId < 0)
            ss << SHM_MULTI_PREFIX << multiId << sEnvPath;
        else
            ss << SHM_MULTI_PREFIX << multiId << SHM_SLS_PREFIX << slsId << sEnvPath;

        std::string temp = ss.str();
        if (temp.length() > NAME_MAX) {
            FILE_LOG(logERROR) << "Shared memory initialization failed. " << temp << " has " << temp.length() << " characters. \n"
                                                                                                                 "Maximum is "
                               << NAME_MAX << ". Change the environment variable " << SHM_ENV_NAME;
            throw SharedMemoryException();
        }
        return temp;
    }

    /**
     * Map shared memory to an address
     * throws a SharedMemoryException exception on failure
     * @param sz of shared memory
     */

    T *MapSharedMemory(size_t sz) {
        void *addr = mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            FILE_LOG(logERROR) << "Mapping shared memory " << name << " failed: " << strerror(errno);
            close(fd);
            throw SharedMemoryException();
        }
        shmSize = sz;
        close(fd);
        return (T *)addr;
    }

    /**
     * Verify if existing shared memory size matches expected size
     * @param expectedSize expected size of shared memory, replaced with smaller size if size does not match
     * @return 0 for success, 1 for fail
     */
    int VerifySizeMatch(size_t expectedSize) {
        struct stat sb;
        // could not fstat
        if (fstat(fd, &sb) < 0) {
            FILE_LOG(logERROR) << "Could not verify existing shared memory " << name << " size match "
                                                                                        "(could not fstat): "
                               << strerror(errno);
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

    /** Shared memory name */
    std::string name;

    /** File descriptor */
    int fd{-1};

    /** shm size */
    size_t shmSize{0};

    T *shared_struct{nullptr};
};
