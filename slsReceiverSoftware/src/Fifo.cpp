/************************************************
 * @file Fifo.cpp
 * @short constructs the fifo structure
 * which is a circular buffer with pointers to
 * parts of allocated memory
 ***********************************************/

#include "Fifo.h"
#include "sls_detector_exceptions.h"

#include <cstdlib>
#include <cstring>
#include <iostream>


Fifo::Fifo(int ind, uint32_t fifoItemSize, uint32_t depth):
		index(ind),
		memory(nullptr),
		fifoBound(nullptr),
		fifoFree(nullptr),
		fifoStream(nullptr),
		fifoDepth(depth),
		status_fifoBound(0),
		status_fifoFree(depth){
	FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
	if(CreateFifos(fifoItemSize) == FAIL)
	    throw sls::RuntimeError("Could not create FIFO");
}


Fifo::~Fifo() {
	FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";
	DestroyFifos();
}



int Fifo::CreateFifos(uint32_t fifoItemSize) {
	FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

	//destroy if not already
	DestroyFifos();

	//create fifos
	fifoBound = new CircularFifo<char>(fifoDepth);
	fifoFree = new CircularFifo<char>(fifoDepth);
	fifoStream = new CircularFifo<char>(fifoDepth);
	//allocate memory
	size_t mem_len = fifoItemSize * fifoDepth * sizeof(char);
	memory = (char*) malloc (mem_len);
	if (memory == nullptr){
		FILE_LOG(logERROR) << "Could not allocate memory for fifos";
		return FAIL;
	}
    memset(memory, 0, mem_len);
	FILE_LOG(logDEBUG) << "Memory Allocated " << index << ": " << mem_len << " bytes";

	{ //push free addresses into fifoFree fifo
		char* buffer = memory;
		for (int i = 0; i < fifoDepth; ++i) {
			//sprintf(buffer,"memory");
			FreeAddress(buffer);
			buffer += fifoItemSize;
		}
	}
	FILE_LOG(logINFO) << "Fifo " << index << " reconstructed Depth (rx_fifodepth): " << fifoFree->getDataValue();
	return OK;
}


void Fifo::DestroyFifos(){
	FILE_LOG(logDEBUG3) << __SHORT_AT__ << " called";

	if(memory) {
		free(memory);
		memory = nullptr;
	}
	if (fifoBound) {
		delete fifoBound;
		fifoBound = nullptr;
	}
	if (fifoFree) {
		delete fifoFree;
		fifoFree = nullptr;
	}
	if (fifoStream) {
		delete fifoStream;
		fifoStream = nullptr;
	}
}


void Fifo::FreeAddress(char*& address) {
	fifoFree->push(address);
}

void Fifo::GetNewAddress(char*& address) {
	int temp = fifoFree->getDataValue();
	if (temp < status_fifoFree)
		status_fifoFree = temp;
	fifoFree->pop(address);
}

void Fifo::PushAddress(char*& address) {
	int temp = fifoBound->getDataValue();
	if (temp > status_fifoBound)
		status_fifoBound = temp;
	while(!fifoBound->push(address));
	/*temp = fifoBound->getDataValue();
	if (temp > status_fifoBound)
		status_fifoBound = temp;*/
}

void Fifo::PopAddress(char*& address) {
	fifoBound->pop(address);
}

void Fifo::PushAddressToStream(char*& address) {
	fifoStream->push(address);
}

void Fifo::PopAddressToStream(char*& address) {
	fifoStream->pop(address);
}

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

