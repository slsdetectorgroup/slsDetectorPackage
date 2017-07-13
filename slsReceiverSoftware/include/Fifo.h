#pragma once
/************************************************
 * @file Fifo.h
 * @short constructs the fifo structure
 * which is a circular buffer with pointers to
 * parts of allocated memory
 ***********************************************/
/**
 *@short constructs the fifo structure
 */

#include "sls_receiver_defs.h"
#include "logger.h"

#include "circularFifo.h"

class Fifo : private virtual slsReceiverDefs {
	
 public:
	/**
	 * Constructor
	 * Calls CreateFifos that creates fifos and allocates memory
	 * @param fifoItemSize size of each fifo item
	 * @param fifoDepth fifo depth
	 * @param success true if successful, else false
	 */
	Fifo(uint32_t fifoItemSize, uint32_t fifoDepth, bool &success);

	/**
	 * Destructor
	 */
	~Fifo();

	/**
	 * Frees the bound address by pushing into fifoFree
	 */
	void FreeAddress(char*& address);

	/**
	 * Pops free address from fifoFree
	 */
	void GetNewAddress(char*& address);

	/**
	 * Pushes bound address into fifoBound
	 */
	void PushAddress(char*& address);

	/**
	 * Pops bound address from fifoBound to process data
	 */
	void PopAddress(char*& address);

	/**
	 * Pushes bound address into fifoStream
	 */
	void PushAddressToStream(char*& address);

	/**
	 * Pops bound address from fifoStream to stream data
	 */
	void PopAddressToStream(char*& address);

	/**
	 * Get Maximum Level filled in Fifo Bound
	 * and reset this value for next intake
	 */
	int GetMaxLevelForFifoBound();

 private:

	/**
	 * Create Fifos, allocate memory & push addresses into fifo
	 * @param fifoItemSize size of each fifo item
	 * @param fifoDepth fifo depth
	 * @return OK if successful, else FAIL
	 */
	int CreateFifos(uint32_t fifoItemSize, uint32_t fifoDepth);

	/**
	 * Destroy Fifos and deallocate memory
	 */
	void DestroyFifos();


	/** Total Number of Fifo Class Objects */
	static int NumberofFifoClassObjects;

	/** Self Index */
	int index;

	/** Memory allocated, whose addresses are pushed into the fifos */
	char* memory;

	/** Circular Fifo pointing to addresses of bound data in memory */
	CircularFifo<char>* fifoBound;

	/** Circular Fifo pointing to addresses of freed data in memory */
	CircularFifo<char>* fifoFree;

	/** Circular Fifo pointing to addresses of to be streamed data in memory */
	CircularFifo<char>* fifoStream;

	int status_fifoBound;
};
