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

namespace sls {

template <typename T>
class SharedMemory {

  public:
    /**
	 * Constructor
	 * creates the single/multi detector shared memory name
	 * @param multiId multi detector id
     * @param slsId sls detector id, -1 if a multi detector shared memory
 	 */
    SharedMemory(int multiId, int slsId) {
        name = ConstructSharedMemoryName(multiId, slsId);
    }

    /** 
     * Delete the copy constructor and copy assignment since we don't want two 
     * objects managing the same resource
     */
    SharedMemory(const SharedMemory &) = delete;
    SharedMemory &operator=(const SharedMemory &other) = delete;

    //Move constructor
    SharedMemory(SharedMemory &&other) : name(other.name),
                                         fd(other.fd),
                                         shmSize(other.shmSize),
                                         shared_struct(other.shared_struct) {

        other.fd = -1;
        other.shared_struct = nullptr;
        other.shmSize = 0;
    }

    //Move assignment
    SharedMemory &operator=(SharedMemory &&other) {
        name = other.name;
        if (fd) {
            close(fd);
        }
        fd = other.fd;
        other.fd = -1;

        if (shared_struct != nullptr) {
            UnmapSharedMemory();
        }
        shared_struct = other.shared_struct;
        other.shared_struct = nullptr;

        shmSize = other.shmSize;
        other.shmSize = 0;
        return *this;
    }

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
    std::string GetName() const {
        return name;
    }

    size_t size() const {
        return shmSize;
    }

    /**
     * Create Shared memory and call MapSharedMemory to map it to an address
     * throws a SharedMemoryError exception on failure to create, ftruncate or map
     * @param sz of shared memory
     */
    void CreateSharedMemory() {
        fd = shm_open(name.c_str(), O_CREAT | O_TRUNC | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            std::string msg = "Create shared memory " + name + " failed: " + strerror(errno);
            FILE_LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
        }

        if (ftruncate(fd, sizeof(T)) < 0) {
            std::string msg = "Create shared memory " + name + " failed at ftruncate: " + strerror(errno);
            FILE_LOG(logERROR) << msg;
            close(fd);
            RemoveSharedMemory();
            throw SharedMemoryError(msg);
        }

        shared_struct = MapSharedMemory();
        FILE_LOG(logINFO) << "Shared memory created " << name;
    }

    /**
     * Open existing Shared memory and call MapSharedMemory to map it to an address
     * throws a SharedMemoryError exception on failure to open or map
     * @param sz of shared memory
     */
    void OpenSharedMemory() {
        fd = shm_open(name.c_str(), O_RDWR, 0);
        if (fd < 0) {
            std::string msg = "Open existing shared memory " + name + " failed: " + strerror(errno);
            FILE_LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
        }

        shared_struct = MapSharedMemory();
    }

    /**
     * Unmap shared memory from an address
     * throws a SharedMemoryError exception on failure
     */
    void UnmapSharedMemory() {
        if (shared_struct != nullptr) {
            if (munmap(shared_struct, shmSize) < 0) {
                std::string msg = "Unmapping shared memory " + name + " failed: " + strerror(errno);
                FILE_LOG(logERROR) << msg;
                close(fd);
                throw SharedMemoryError(msg);
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
            std::string msg = "Free Shared Memory " + name + " Failed: " + strerror(errno);
            FILE_LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
        }
        FILE_LOG(logINFO) << "Shared memory deleted " << name;
    }

    /**
     * Maximum length of name as from man pages
     */
    static const int name_max_length = 255;

    /**
    *Using the call operator to access the pointer
    */
    T *operator()() {
        return shared_struct;
    }

    /**
    *Using the call operator to access the pointer, const overload
    */
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
        if (temp.length() > name_max_length) {
            std::string msg = "Shared memory initialization failed. " + temp + " has " + std::to_string(temp.length()) + " characters. \n" + "Maximum is " + std::to_string(name_max_length) + ". Change the environment variable " + SHM_ENV_NAME;
            FILE_LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
        }
        return temp;
    }

    /**
     * Map shared memory to an address
     * throws a SharedMemoryException exception on failure
     * @param sz of shared memory
     */

    T *MapSharedMemory() {
        void *addr = mmap(nullptr, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            std::string msg = "Mapping shared memory " + name + " failed: " + strerror(errno);
            FILE_LOG(logERROR) << msg;
            close(fd);
            throw SharedMemoryError(msg);
        }
        shmSize = sizeof(T);
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
            std::string msg = "Could not verify existing shared memory " + name + " size match " + "(could not fstat): " + strerror(errno);
            FILE_LOG(logERROR) << msg;
            close(fd);
            throw SharedMemoryError(msg);
        }

        //size does not match
        long unsigned int sz = (long unsigned int)sb.st_size;
        if (sz != expectedSize) {
            std::string msg = "Existing shared memory " + name + " size does not match" + "Expected " + std::to_string(expectedSize) + ", found " + std::to_string(sz);
            FILE_LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
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

} // namespace sls