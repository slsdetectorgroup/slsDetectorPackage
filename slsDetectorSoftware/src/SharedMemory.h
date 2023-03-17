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

#include <cerrno> // errno
#include <cstdlib>
#include <cstring> // strerror
#include <fcntl.h> // O_CREAT, O_TRUNC..
#include <iostream>
#include <sstream>
#include <string>
#include <sys/mman.h> // shared memory
#include <sys/stat.h> // fstat
#include <unistd.h>

namespace sls {

#define SHM_DETECTOR_PREFIX "/slsDetectorPackage_detector_"
#define SHM_MODULE_PREFIX   "_module_"
#define SHM_ENV_NAME        "SLSDETNAME"

template <typename T> class SharedMemory {
    static constexpr int NAME_MAX_LENGTH = 255;
    std::string name;
    T *shared_struct{nullptr};

  public:
    // moduleid of -1 creates a detector only shared memory
    SharedMemory(int detectorId, int moduleIndex, const std::string &tag = "") {
        name = constructSharedMemoryName(detectorId, moduleIndex, tag);
    }

    // Disable copy, since we refer to a unique location
    SharedMemory(const SharedMemory &) = delete;
    SharedMemory &operator=(const SharedMemory &other) = delete;

    SharedMemory(SharedMemory &&other)
        : name(other.name), shared_struct(other.shared_struct) {
        other.shared_struct = nullptr;
    }

    SharedMemory &operator=(SharedMemory &&other) {
        name = other.name;
        if (shared_struct != nullptr)
            unmapSharedMemory();
        shared_struct = other.shared_struct;
        other.shared_struct = nullptr;
        return *this;
    }

    ~SharedMemory() {
        if (shared_struct)
            unmapSharedMemory();
    }

    T *operator()() {
        if (shared_struct)
            return shared_struct;
        throw SharedMemoryError(getNoShmAccessMessage());
    }

    const T *operator()() const {
        if (shared_struct)
            return shared_struct;
        throw SharedMemoryError(getNoShmAccessMessage());
    }

    std::string getName() const { return name; }

    bool exists() {
        int tempfd = shm_open(name.c_str(), O_RDWR, 0);
        if ((tempfd < 0) && (errno == ENOENT)) {
            return false;
        }
        close(tempfd);
        return true;
    }

    void createSharedMemory() {
        int fd = shm_open(name.c_str(), O_CREAT | O_TRUNC | O_EXCL | O_RDWR,
                          S_IRUSR | S_IWUSR);
        if (fd < 0) {
            std::string msg =
                "Create shared memory " + name + " failed: " + strerror(errno);
            throw SharedMemoryError(msg);
        }

        if (ftruncate(fd, sizeof(T)) < 0) {
            std::string msg = "Create shared memory " + name +
                              " failed at ftruncate: " + strerror(errno);
            close(fd);
            removeSharedMemory();
            throw SharedMemoryError(msg);
        }
        shared_struct = mapSharedMemory(fd);
        new (shared_struct) T{};
        LOG(logINFO) << "Shared memory created " << name;
    }

    void openSharedMemory(bool verifySize) {
        int fd = shm_open(name.c_str(), O_RDWR, 0);
        if (fd < 0) {
            std::string msg = "Open existing shared memory " + name +
                              " failed: " + strerror(errno);
            throw SharedMemoryError(msg);
        }
        if (verifySize)
            checkSize(fd);
        shared_struct = mapSharedMemory(fd);
    }

    void unmapSharedMemory() {
        if (shared_struct != nullptr) {
            if (munmap(shared_struct, sizeof(T)) < 0) {
                std::string msg = "Unmapping shared memory " + name +
                                  " failed: " + strerror(errno);
                throw SharedMemoryError(msg);
            }
            shared_struct = nullptr;
        }
    }

    void removeSharedMemory() {
        unmapSharedMemory();
        if (shm_unlink(name.c_str()) < 0) {
            // silent exit if shm did not exist anyway
            if (errno == ENOENT)
                return;
            std::string msg =
                "Free Shared Memory " + name + " Failed: " + strerror(errno);
            throw SharedMemoryError(msg);
        }
        LOG(logINFO) << "Shared memory deleted " << name;
    }

  private:
    std::string constructSharedMemoryName(int detectorId, int moduleIndex,
                                          const std::string &tag) {

        // using environment variable
        std::string slsdetname;
        char *envpath = getenv(SHM_ENV_NAME);
        if (envpath != nullptr) {
            slsdetname = envpath;
            slsdetname.insert(0, "_");
        }

        std::stringstream ss;
        if (moduleIndex < 0) {
            ss << SHM_DETECTOR_PREFIX << detectorId << slsdetname;
            if (!tag.empty())
                ss << "_" << tag;
        } else {
            ss << SHM_DETECTOR_PREFIX << detectorId << SHM_MODULE_PREFIX
               << moduleIndex << slsdetname;
        }

        std::string shm_name = ss.str();
        if (shm_name.length() > NAME_MAX_LENGTH) {
            std::string msg =
                "Shared memory initialization failed. " + shm_name + " has " +
                std::to_string(shm_name.length()) + " characters. \n" +
                "Maximum is " + std::to_string(NAME_MAX_LENGTH) +
                ". Change the environment variable " + SHM_ENV_NAME;
            throw SharedMemoryError(msg);
        }
        return shm_name;
    }

    // from the Linux manual:
    // After the mmap() call has returned, the file descriptor, fd, can
    // be closed immediately without invalidating the mapping.
    T *mapSharedMemory(int fd) {
        void *addr =
            mmap(nullptr, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        if (addr == MAP_FAILED) {
            std::string msg =
                "Mapping shared memory " + name + " failed: " + strerror(errno);
            throw SharedMemoryError(msg);
        }
        return static_cast<T *>(addr);
    }

    void checkSize(int fd) {
        struct stat sb;
        if (fstat(fd, &sb) < 0) {
            std::string msg = "Could not verify existing shared memory " +
                              name + " size match " +
                              "(could not fstat): " + strerror(errno);
            close(fd);
            throw SharedMemoryError(msg);
        }

        auto actual_size = static_cast<size_t>(sb.st_size);
        auto expected_size = sizeof(T);
        if (actual_size != expected_size) {
            std::string msg =
                "Existing shared memory " + name + " size does not match. " +
                "Expected " + std::to_string(expected_size) + ", found " +
                std::to_string(actual_size) +
                ". Detector software mismatch? Try freeing shared memory.";
            throw SharedMemoryError(msg);
        }
    }

    const char *getNoShmAccessMessage() const {
        return ("No shared memory to access. Create it first with "
                "hostname or config command.");
    };
};

} // namespace sls
