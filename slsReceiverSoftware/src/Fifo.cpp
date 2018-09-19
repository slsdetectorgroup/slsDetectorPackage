/************************************************
 * @file Fifo.cpp
 * @short constructs the fifo structure
 * which is a circular buffer with pointers to
 * parts of allocated memory
 ***********************************************/

#include "Fifo.h"

#include <iostream>
#include <cstdlib>
#include <cstring>


Fifo::Fifo(int ind, uint32_t fifoItemSize, uint32_t depth):
		index(ind),
		memory(0),
		fifoBound(0),
		fifoFree(0),
		fifoStream(0),
		fifoDepth(depth),
		status_fifoBound(0),
		status_fifoFree(depth){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	if(CreateFifos(fifoItemSize) == FAIL)
	    throw std::exception();
}


Fifo::~Fifo() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	//cprintf(BLUE,"Fifo Object %d: Goodbye\n", index);
	DestroyFifos();
}



int Fifo::CreateFifos(uint32_t fifoItemSize) {
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//destroy if not already
	DestroyFifos();

	//create fifos
	fifoBound = new CircularFifo<char>(fifoDepth);
	fifoFree = new CircularFifo<char>(fifoDepth);
	fifoStream = new CircularFifo<char>(fifoDepth);
	//allocate memory
	size_t mem_len = fifoItemSize * fifoDepth * sizeof(char);
	memory = (char*) malloc (mem_len);
	if (memory == NULL){
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
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(memory) {
		free(memory);
		memory = 0;
	}
	if (fifoBound) {
		delete fifoBound;
		fifoBound = 0;
	}
	if (fifoFree) {
		delete fifoFree;
		fifoFree = 0;
	}
	if (fifoStream) {
		delete fifoStream;
		fifoStream = 0;
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

