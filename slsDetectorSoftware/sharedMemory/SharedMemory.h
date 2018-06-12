#pragma once
/************************************************
 * @file SharedMemory.h
 * @short functions basic implemenation of
 * shared memory
 ***********************************************/
/**
 *@short functions basic implemenation of shared memory
 */

#include <iostream>
#include <string>

class SharedMemory{
public:
	/**
	 * Constructor
	 * creates the single/multi detector shared memory name
	 * @param multiId multi detector id
     * @param singleId sls detector id, -1 if a multi detector shared memory
 	 */
	SharedMemory(int multiId, int singleId);

	/**
	 * Destructor
	 */
	~SharedMemory();

    /**
     * Verify if it exists
     * @param name of shared memory
     * @return true if exists, else false
     */
    static bool IsExisting(std::string name);

	/**
	 * Get shared memory name
	 */
	std::string GetName();

    /**
     * Create Shared memory and call MapSharedMemory to map it to an address
     * throws a SharedMemoryException exception on failure to create, ftruncate or map
     * @param sz of shared memory
     * @param addr double pointer to address to be mapped
     */
    void* CreateSharedMemory(size_t sz);

    /**
     * Open existing Shared memory and call MapSharedMemory to map it to an address
     * throws a SharedMemoryException exception on failure to open, incorrect size if verify true, or map
     * @param sz of shared memory
     * @param addr double pointer to address to be mapped
     * @param verify true to verify if the size matches existing one and return fail if does not match
     */
    void* OpenSharedMemory(size_t sz, bool verify = true);

    /**
     * Unmap shared memory from an address
     * throws a SharedMemoryException exception on failure
     * @param addr double pointer to address to be mapped
     */
    void UnmapSharedMemory(void* addr);

	/**
	 * Remove existing Shared memory
	 */
	void RemoveSharedMemory();

    /**
     * Maximum length of name as from man pages
     */
    static const int NAME_MAX = 255;

private:
    /**
     * Create Shared memory name
     * throws exception if name created is longer than required 255(manpages)
     * @param multiId multi detector id
     * @param singleId sls detector id, -1 if a multi detector shared memory
     * @returns shared memory name
     */
    std::string ConstructSharedMemoryName(int multiId, int singleId);

    /**
     * Map shared memory to an address
     * throws a SharedMemoryException exception on failure
     * @param addr double pointer to address to be mapped
     * @param sz of shared memory
     */
	//template <typename myType>
   // void MapSharedMemory(myType*& addr, size_t sz);
    void* MapSharedMemory(size_t sz);

    /**
     * Verify if existing shared memory size matches expected size
     * @param sz expected size of shared memory, replaced with smaller size if size does not match
     * @return 0 for success, 1 for fail
     */
    int VerifySizeMatch(size_t expectedSize);

    /**
     * Remove existing Shared memory
     * @param name name of shared memory (should be less than NAME_MAX)
     */
	void RemoveSharedMemory(std::string name);

	/** Shared memory name */
	std::string name;

	/** File descriptor */
	int fd;

	/** shm size */
	size_t shmSize;

};
