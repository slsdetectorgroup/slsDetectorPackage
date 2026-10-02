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

#include "sls/CircularFifo.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <atomic>
#include <mutex>

namespace sls {

class Fifo : private virtual slsDetectorDefs {

  public:
    Fifo(int index, size_t fifoItemSize, uint32_t fifoDepth);
    ~Fifo();

    void FreeAddress(char *&address);
    void GetNewAddress(char *&address);

    /** to process data */
    void PushAddress(char *&address);
    void PopAddress(char *&address);

    void PushAddressToStream(char *&address);
    void PopAddressToStream(char *&address);

    int GetMaxLevelForFifoBound();
    int GetMinLevelForFifoFree();

  private:
    /** also allocate memory & push addresses into free fifo */
    void CreateFifos(size_t fifoItemSize);
    /** also deallocate memory */
    void DestroyFifos();

    int index;
    char *memory;
    CircularFifo<char> *fifoBound;
    CircularFifo<char> *fifoFree;
    CircularFifo<char> *fifoStream;
    int fifoDepth;
    std::atomic<int> status_fifoBound;
    std::atomic<int> status_fifoFree;
    /** fifoFree is filled by the listener (discarded frames), the data
     * processor and the data streamer, CircularFifo allows only one producer.
     * On its own cache line: it is written for every freed buffer, the
     * members above are read by the listener for every frame
     */
    alignas(64) std::mutex fifoFreeMutex;
};

} // namespace sls
