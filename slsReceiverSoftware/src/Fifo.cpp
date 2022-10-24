// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/************************************************
 * @file Fifo.cpp
 * @short constructs the fifo structure
 * which is a circular buffer with pointers to
 * parts of allocated memory
 ***********************************************/

#include "Fifo.h"
#include "sls/sls_detector_exceptions.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

namespace sls {

Fifo::Fifo(int index, size_t fifoItemSize, uint32_t fifoDepth)
    : index(index), memory(nullptr), fifoBound(nullptr), fifoFree(nullptr),
      fifoStream(nullptr), fifoDepth(fifoDepth), status_fifoBound(0),
      status_fifoFree(fifoDepth) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    CreateFifos(fifoItemSize);
}

Fifo::~Fifo() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    DestroyFifos();
}

void Fifo::CreateFifos(size_t fifoItemSize) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    // destroy if not already
    DestroyFifos();

    // create fifos
    fifoBound = new CircularFifo<char>(fifoDepth);
    fifoFree = new CircularFifo<char>(fifoDepth);
    fifoStream = new CircularFifo<char>(fifoDepth);
    // allocate memory
    size_t mem_len = fifoItemSize * (size_t)fifoDepth * sizeof(char);
    memory = (char *)malloc(mem_len);
    if (memory == nullptr) {
        throw RuntimeError("Could not allocate memory for fifos");
    }
    memset(memory, 0, mem_len);
    int pagesize = getpagesize();
    for (size_t i = 0; i < mem_len; i += pagesize) {
        strcpy(memory + i, "memory");
    }
    LOG(logDEBUG) << "Memory Allocated " << index << ": "
                  << (double)mem_len / (double)(1024 * 1024) << " MB";

    { // push free addresses into fifoFree fifo
        char *buffer = memory;
        for (int i = 0; i < fifoDepth; ++i) {
            // sprintf(buffer,"memory");
            FreeAddress(buffer);
            buffer += fifoItemSize;
        }
    }
    LOG(logINFO) << "Fifo " << index << " reconstructed Depth (rx_fifodepth): "
                 << fifoFree->getDataValue();
}

void Fifo::DestroyFifos() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (memory) {
        free(memory);
        memory = nullptr;
    }
    delete fifoBound;
    fifoBound = nullptr;
    delete fifoFree;
    fifoFree = nullptr;
    delete fifoStream;
    fifoStream = nullptr;
}

void Fifo::FreeAddress(char *&address) { fifoFree->push(address); }

void Fifo::GetNewAddress(char *&address) {
    int temp = fifoFree->getDataValue();
    if (temp < status_fifoFree)
        status_fifoFree = temp;
    fifoFree->pop(address);
}

void Fifo::PushAddress(char *&address) {
    int temp = fifoBound->getDataValue();
    if (temp > status_fifoBound)
        status_fifoBound = temp;
    while (!fifoBound->push(address))
        ;
    /*temp = fifoBound->getDataValue();
    if (temp > status_fifoBound)
            status_fifoBound = temp;*/
}

void Fifo::PopAddress(char *&address) { fifoBound->pop(address); }

void Fifo::PushAddressToStream(char *&address) { fifoStream->push(address); }

void Fifo::PopAddressToStream(char *&address) { fifoStream->pop(address); }

int Fifo::GetMaxLevelForFifoBound() {
    int temp = status_fifoBound;
    status_fifoBound = 0;
    return temp;
}

int Fifo::GetMinLevelForFifoFree() {
    int temp = status_fifoFree;
    status_fifoFree = fifoDepth;
    return temp;
}

} // namespace sls
