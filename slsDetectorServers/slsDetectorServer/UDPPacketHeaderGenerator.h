#pragma once

#include "logger.h"
#include "sls_detector_defs.h"


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

extern const enum detectorType myDetectorType;
extern int nSamples;
extern int dataBytes;
extern int nframes;
extern char* ramValues;

#define UDP_PACKET_HEADER_VERSION 	(0x1)


uint32_t udpPacketNumber = 0;
uint64_t udpFrameNumber = 0;

int numSamplesPerPacket = 0;
int dataBytesPerSample = 0;
int dataBytesPerPacket = 0;
int udpHeaderOffset = 0;

uint32_t getUDPPacketNumber() {
	return udpPacketNumber;
}

uint64_t getUDPFrameNumber() {
	return udpFrameNumber;
}


/**
 * Called for each UDP packet header creation
 *
 */
void createUDPPacketHeader(char* buffer, uint16_t id) {
	memset(buffer, 0, sizeof(sls_detector_header));
	sls_detector_header* header = (sls_detector_header*)(buffer);

	header->modId = id;
	// row and column remains 0 (only used by ctb now)
	// uint64_t timestamp FIXME: needed?
	header->detType = (uint16_t)myDetectorType;
	header->version = UDP_PACKET_HEADER_VERSION;

	// reset offset
	udpHeaderOffset = 0;
	// reset frame number
	udpFrameNumber = 0;
}


int fillUDPPacket(char* buffer) {
	FILE_LOG(logDEBUG2, ("Databytes:%d offset:%d\n", dataBytes, udpHeaderOffset));
	// reached end of data for one frame
	if (udpHeaderOffset >= dataBytes) {
		// reset offset
		udpHeaderOffset = 0;
		return 0;
	}

	sls_detector_header* header = (sls_detector_header*)(buffer);

	// update frame number, starts at 1 (reset packet number)
	if (udpHeaderOffset == 0) {
		++udpFrameNumber;
		header->frameNumber = udpFrameNumber;
		udpPacketNumber = -1;
	}

	// increment  and copy udp packet number (starts at 0)
	++udpPacketNumber;
	header->packetNumber = udpPacketNumber;
	FILE_LOG(logDEBUG2, ("Creating packet number %d (fnum:%lld)\n", udpPacketNumber, (long long int) udpFrameNumber));

	// calculate number of bytes to copy
	int numBytesToCopy = ((udpHeaderOffset + UDP_PACKET_DATA_BYTES) <= dataBytes) ?
			UDP_PACKET_DATA_BYTES : (dataBytes - udpHeaderOffset);
	header->reserved = numBytesToCopy;

	// copy data
	memcpy(buffer + sizeof(sls_detector_header), ramValues + udpHeaderOffset, numBytesToCopy);
	// pad last packet if extra space
	if (numBytesToCopy < UDP_PACKET_DATA_BYTES) {
		int bytes = UDP_PACKET_DATA_BYTES - numBytesToCopy;
		FILE_LOG(logDEBUG1, ("Padding %d bytes for fnum:%lld pnum:%d\n", bytes, (long long int)udpFrameNumber, udpPacketNumber));
		memset(buffer + sizeof(sls_detector_header) + numBytesToCopy, 0, bytes);
	}

	// increment offset
	udpHeaderOffset += numBytesToCopy;

	return UDP_PACKET_DATA_BYTES + sizeof(sls_detector_header);
}
