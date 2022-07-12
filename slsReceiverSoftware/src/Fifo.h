// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
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

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include "sls/CircularFifo.h"

namespace sls {

class Fifo : private virtual slsDetectorDefs {

  public:
    /**
     * Constructor
     * Calls CreateFifos that creates fifos and allocates memory
     * @param ind self index
     * @param fifoItemSize size of each fifo item
     * @param depth fifo depth
     */
    Fifo(int ind, size_t fifoItemSize, uint32_t depth);

    /**
     * Destructor
     */
    ~Fifo();

    /**
     * Frees the bound address by pushing into fifoFree
     */
    void FreeAddress(char *&address);

    /**
     * Pops free address from fifoFree
     */
    void GetNewAddress(char *&address);

    /**
     * Pushes bound address into fifoBound
     */
    void PushAddress(char *&address);

    /**
     * Pops bound address from fifoBound to process data
     */
    void PopAddress(char *&address);

    /**
     * Pushes bound address into fifoStream
     */
    void PushAddressToStream(char *&address);

    /**
     * Pops bound address from fifoStream to stream data
     */
    void PopAddressToStream(char *&address);

    /**
     * Get Maximum Level filled in Fifo Bound
     * and reset this value for next intake
     */
    int GetMaxLevelForFifoBound();

    /**
     * Get Minimum Level filled in Fifo Free
     * and reset this value to max for next intake
     */
    int GetMinLevelForFifoFree();

  private:
    /**
     * Create Fifos, allocate memory & push addresses into fifo
     * @param fifoItemSize size of each fifo item
     */
    void CreateFifos(size_t fifoItemSize);

    /**
     * Destroy Fifos and deallocate memory
     */
    void DestroyFifos();

    /** Self Index */
    int index;

    /** Memory allocated, whose addresses are pushed into the fifos */
    char *memory;

    /** Circular Fifo pointing to addresses of bound data in memory */
    CircularFifo<char> *fifoBound;

    /** Circular Fifo pointing to addresses of freed data in memory */
    CircularFifo<char> *fifoFree;

    /** Circular Fifo pointing to addresses of to be streamed data in memory */
    CircularFifo<char> *fifoStream;

    /** Fifo depth set */
    int fifoDepth;

    volatile int status_fifoBound;
    volatile int status_fifoFree;
};

} // namespace sls

