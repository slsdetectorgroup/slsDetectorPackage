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

#include "sls/TypeTraits.h"
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

// ********************** Defines for shared memory. **********************
// WARNING! before chaning these search the codebase for their usage!

// date when IsValid boolean introduced into every shm structure
// (to look out for shm that still exists due to other mapped resources)
#define SHM_IS_VALID_CHECK_VERSION 0x250820

// Max shared memory name length in macOS is 31 characters
#ifdef __APPLE__
#define SHM_DETECTOR_PREFIX "/sls_"
#define SHM_MODULE_PREFIX   "_mod_"
#else
#define SHM_DETECTOR_PREFIX "/slsDetectorPackage_detector_"
#define SHM_MODULE_PREFIX   "_module_"
#endif

#define SHM_ENV_NAME "SLSDETNAME"
// ************************************************************************

namespace sls {

class CtbConfig;

template <typename T, typename U> constexpr bool is_type() {
    return std::is_same_v<std::decay_t<U>, T>;
}

template <typename T> class SharedMemory {

#ifndef DISABLE_STATIC_ASSERT
    static_assert(has_bool_isValid<T>::value,
                  "SharedMemory requires the struct to have a bool member "
                  "named 'isValid'");
#endif

    static constexpr int NAME_MAX_LENGTH = 255;
    std::string name;
    T *shared_struct{nullptr};

  public:
    // moduleid of -1 creates a detector only shared memory
    SharedMemory(int detectorId, int moduleIndex, const std::string &tag = "") {
        name = constructSharedMemoryName(detectorId, moduleIndex, tag);
    }

    explicit SharedMemory(const std::string &shm_name) : name(shm_name) {}

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
            unmapSharedMemory(); // only unmapped as resued in command line
    }

    /** memory is valid if it has the IsValid flag and is true */
    bool memoryHasValidFlag() const {
        if (shared_struct == nullptr) {
            throw SharedMemoryError(
                "Shared memory not mapped. Cannot check validity.");
        }
        // CtbConfig also works (shmversion didnt exist prior, but it would read
        // "20" = length size) so shmversion should always be >= isValid
        // introduced date (0x250820)
        if (shared_struct->shmversion >= SHM_IS_VALID_CHECK_VERSION) {
            return true;
        }
        return false;
    }

    bool checkValidity() const {
        if (memoryHasValidFlag() && !shared_struct->isValid)
            return false;
        return true;
    }

    void invalidate() {
        bool wasValid = true;
        if (shared_struct == nullptr) {
            wasValid = false;
            openSharedMemory(false);
        }
        // also called by obsolete shm test structure (so check added)
        if constexpr (has_bool_isValid<T>::value) {
            if (memoryHasValidFlag())
                shared_struct->isValid = false;
        }
        if (!wasValid)
            unmapSharedMemory();
    }

    T *operator()() {
        if (!shared_struct)
            throw SharedMemoryError(getNoShmAccessMessage());

        if (!checkValidity())
            throw SharedMemoryError(getInvalidShmMessage());

        return shared_struct;
    }

    const T *operator()() const {
        if (!shared_struct)
            throw SharedMemoryError(getNoShmAccessMessage());

        if (!checkValidity())
            throw SharedMemoryError(getInvalidShmMessage());

        return shared_struct;
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
            if (errno == EEXIST) {
                throw SharedMemoryAlreadyExistsError(msg);
            } else {
                throw SharedMemoryError(msg);
            }
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
        invalidate();
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

#ifdef __APPLE__
        // On macOS, fstat returns the allocated size and not the requested
        // size. This means we can't check for size since we always get for
        // example 16384 bytes.
        return;
#endif
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

    inline const char *getNoShmAccessMessage() const {
        return ("No shared memory to access. Create it first with "
                "hostname or config command.");
    };

    inline const char *getInvalidShmMessage() const {
        return ("Shared memory is invalid or freed. Close resources before "
                "access.");
    };
};

} // namespace sls
