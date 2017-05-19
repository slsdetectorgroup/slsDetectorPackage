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
using namespace std;

int Fifo::NumberofFifoClassObjects(0);

Fifo::Fifo(uint32_t fifoItemSize, uint32_t fifoDepth, bool &success):
		memory(0),
		fifoBound(0),
		fifoFree(0),
		fifoStream(0),
		status_fifoBound(0){
	FILE_LOG (logDEBUG) << __AT__ << " called";
	index = NumberofFifoClassObjects++;
	if(CreateFifos(fifoItemSize, fifoDepth) == FAIL)
		success = false;
}


Fifo::~Fifo() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	//cprintf(BLUE,"Fifo Object %d: Goodbye\n", index);
	DestroyFifos();
	NumberofFifoClassObjects--;
}



int Fifo::CreateFifos(uint32_t fifoItemSize, uint32_t fifoDepth) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	//destroy if not already
	DestroyFifos();
	//create fifos
	fifoBound = new CircularFifo<char>(fifoDepth);
	fifoFree = new CircularFifo<char>(fifoDepth);
	fifoStream = new CircularFifo<char>(fifoDepth);
	//allocate memory
	memory = (char*) calloc (fifoItemSize * fifoDepth, sizeof(char));
	memset(memory,0,fifoItemSize * fifoDepth* sizeof(char));
	if (memory == NULL){
		FILE_LOG (logERROR) << "Could not allocate memory for fifos";
		memory = 0;
		return FAIL;
	}

	{ //push free addresses into fifoFree fifo
		char* buffer = memory;
		while (buffer < (memory + fifoItemSize * (fifoDepth-1))) {
			//sprintf(buffer,"memory");
#ifdef FIFODEBUG
			cprintf(MAGENTA,"Fifofree %d: value:%d, pop 0x%p\n", index, fifoFree->getSemValue(), (void*)(buffer));
#endif
			FreeAddress(buffer);
			buffer += fifoItemSize;
		}
	}
	FILE_LOG (logINFO) << "Fifo Structure " << index << " reconstructed";
	return OK;
}


void Fifo::DestroyFifos(){
	FILE_LOG (logDEBUG) << __AT__ << " called";


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
	while(!fifoFree->push(address));
}

void Fifo::GetNewAddress(char*& address) {
	fifoFree->pop(address);
}

void Fifo::PushAddress(char*& address) {
	while(!fifoBound->push(address));
	int temp = fifoBound->getSemValue();
	if (temp > status_fifoBound)
		status_fifoBound = temp;
}

void Fifo::PopAddress(char*& address) {
	fifoBound->pop(address);
}

void Fifo::PushAddressToStream(char*& address) {
	while(!fifoStream->push(address));
}

void Fifo::PopAddressToStream(char*& address) {
	fifoStream->pop(address);
}

int Fifo::GetMaxLevelForFifoBound() {
	int temp = status_fifoBound;
	status_fifoBound = 0;
	return temp;
}

