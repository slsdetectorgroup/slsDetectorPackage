// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/************************************************
 * @file SharedMemory.h
 * @short functions basic implemenation of
 * shared memory
 ***********************************************/
/**
 *@short functions basic implemenation of shared memory
 */

#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"

#include "stdlib.h"
#include <cerrno>  // errno
#include <cstring> // strerror
#include <fcntl.h> // O_CREAT, O_TRUNC..
#include <iostream>
#include <sstream>
#include <sys/mman.h> // shared memory
#include <sys/stat.h> // fstat
#include <unistd.h>

#define SHM_DETECTOR_PREFIX "/slsDetectorPackage_detector_"
#define SHM_MODULE_PREFIX   "_module_"
#define SHM_ENV_NAME        "SLSDETNAME"

#include <iostream>
#include <string>

namespace sls {

template <typename T> class SharedMemory {

  public:
    //moduleid of -1 creates a detector only shared memory
    SharedMemory(int detectorId, int moduleIndex, const std::string& tag = "") {
        name = ConstructSharedMemoryName(detectorId, moduleIndex, tag);
    }

    /**
     * Delete the copy constructor and copy assignment since we don't want two
     * objects managing the same resource
     */
    SharedMemory(const SharedMemory &) = delete;
    SharedMemory &operator=(const SharedMemory &other) = delete;

    // Move constructor
    SharedMemory(SharedMemory &&other)
        : name(other.name), fd(other.fd), shmSize(other.shmSize),
          shared_struct(other.shared_struct) {

        other.fd = -1;
        other.shared_struct = nullptr;
        other.shmSize = 0;
    }

    // Move assignment
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

    std::string GetName() const { return name; }
    size_t size() const { return shmSize; }

    /**
     * Create Shared memory and call MapSharedMemory to map it to an address
     * throws a SharedMemoryError exception on failure to create, ftruncate or
     * map
     */
    void CreateSharedMemory() {
        fd = shm_open(name.c_str(), O_CREAT | O_TRUNC | O_EXCL | O_RDWR,
                      S_IRUSR | S_IWUSR);
        if (fd < 0) {
            std::string msg =
                "Create shared memory " + name + " failed: " + strerror(errno);
            LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
        }

        if (ftruncate(fd, sizeof(T)) < 0) {
            std::string msg = "Create shared memory " + name +
                              " failed at ftruncate: " + strerror(errno);
            LOG(logERROR) << msg;
            close(fd);
            RemoveSharedMemory();
            throw SharedMemoryError(msg);
        }
        shared_struct = MapSharedMemory();
        new (shared_struct) T{}; 
        LOG(logINFO) << "Shared memory created " << name;
    }

    /**
     * Open existing Shared memory and call MapSharedMemory to map it to an
     * address throws a SharedMemoryError exception on failure to open or map
     */
    void OpenSharedMemory() {
        fd = shm_open(name.c_str(), O_RDWR, 0);
        if (fd < 0) {
            std::string msg = "Open existing shared memory " + name +
                              " failed: " + strerror(errno);
            LOG(logERROR) << msg;
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
                std::string msg = "Unmapping shared memory " + name +
                                  " failed: " + strerror(errno);
                LOG(logERROR) << msg;
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
            std::string msg =
                "Free Shared Memory " + name + " Failed: " + strerror(errno);
            LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
        }
        LOG(logINFO) << "Shared memory deleted " << name;
    }

    /**
     * Maximum length of name as from man pages
     */
    static const int NAME_MAX_LENGTH = 255;

    /**
     *Using the call operator to access the pointer
     */
    T *operator()() { return shared_struct; }

    /**
     *Using the call operator to access the pointer, const overload
     */
    const T *operator()() const { return shared_struct; }

  private:
    /**
     * Create Shared memory name
     * throws exception if name created is longer than required 255(manpages)
     * @param detectorId  detector id
     * @param moduleIndex module id, -1 if a detector shared memory
     * @returns shared memory name
     */
    std::string ConstructSharedMemoryName(int detectorId, int moduleIndex, const std::string& tag) {

        // using environment path
        std::string sEnvPath;
        char *envpath = getenv(SHM_ENV_NAME);
        if (envpath != nullptr) {
            sEnvPath.assign(envpath);
            sEnvPath.insert(0, "_");
        }

        std::stringstream ss;
        if (moduleIndex < 0){
            ss << SHM_DETECTOR_PREFIX << detectorId << sEnvPath;
            if (!tag.empty())
                ss << "_" << tag;
        }
            
        else
            ss << SHM_DETECTOR_PREFIX << detectorId << SHM_MODULE_PREFIX
               << moduleIndex << sEnvPath;

        std::string temp = ss.str();
        if (temp.length() > NAME_MAX_LENGTH) {
            std::string msg =
                "Shared memory initialization failed. " + temp + " has " +
                std::to_string(temp.length()) + " characters. \n" +
                "Maximum is " + std::to_string(NAME_MAX_LENGTH) +
                ". Change the environment variable " + SHM_ENV_NAME;
            LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
        }
        return temp;
    }

    /**
     * Map shared memory to an address
     * throws a SharedMemoryException exception on failure
     */

    T *MapSharedMemory() {
        void *addr =
            mmap(nullptr, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            std::string msg =
                "Mapping shared memory " + name + " failed: " + strerror(errno);
            LOG(logERROR) << msg;
            close(fd);
            throw SharedMemoryError(msg);
        }
        shmSize = sizeof(T);
        close(fd);
        return (T *)addr;
    }

    /**
     * Verify if existing shared memory size matches expected size
     * @param expectedSize expected size of shared memory, replaced with smaller
     * size if size does not match
     * @return 0 for success, 1 for fail
     */
    int VerifySizeMatch(size_t expectedSize) {
        struct stat sb;
        // could not fstat
        if (fstat(fd, &sb) < 0) {
            std::string msg = "Could not verify existing shared memory " +
                              name + " size match " +
                              "(could not fstat): " + strerror(errno);
            LOG(logERROR) << msg;
            close(fd);
            throw SharedMemoryError(msg);
        }

        // size does not match
        auto sz = static_cast<size_t>(sb.st_size);
        if (sz != expectedSize) {
            std::string msg = "Existing shared memory " + name +
                              " size does not match" + "Expected " +
                              std::to_string(expectedSize) + ", found " +
                              std::to_string(sz);
            LOG(logERROR) << msg;
            throw SharedMemoryError(msg);
            return 1;
        }
        return 0;
    }


    std::string name;
    int fd{-1};
    size_t shmSize{0};
    T *shared_struct{nullptr};
};

} // namespace sls
