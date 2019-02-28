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
#define UDP_PACKET_MAX_DATA_BYTES	(1280)


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

void calculateDataBytesPerSample() {
	dataBytesPerSample = dataBytes / nSamples;
	numSamplesPerPacket = (double)UDP_PACKET_MAX_DATA_BYTES / (double)dataBytesPerSample;
	dataBytesPerPacket = dataBytesPerSample * numSamplesPerPacket;
	FILE_LOG(logDEBUG2, ("Databytes/Sample = %d, numSamples/Packet:%d dataBytes/Packet:%d\n",
			dataBytesPerSample, numSamplesPerPacket, dataBytesPerPacket));
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

	// first packet (update frame number)
	if (udpHeaderOffset == 0) {
		// increment frame number (starts at 1)
		++udpFrameNumber;
		header->frameNumber = udpFrameNumber;
		udpPacketNumber = -1;
	}
	// increment udp packet number (starts at 0)
	++udpPacketNumber;

	// copy packet number
	FILE_LOG(logDEBUG2, ("Creating packet number %d (fnum:%lld)\n", udpPacketNumber, (long long int) udpFrameNumber));
	header->packetNumber = udpPacketNumber;

	// calculate number of bytes to write
	int numBytesToWrite = ((udpHeaderOffset + dataBytesPerPacket) <= dataBytes) ?
			dataBytesPerPacket : (dataBytes - udpHeaderOffset);

	// copy number of samples in current packet
	header->reserved = (numBytesToWrite / dataBytesPerSample);
	if (numBytesToWrite % dataBytesPerSample) {
		FILE_LOG(logERROR, ("fillUDPPacketHeader: numBytesToWrite is not divisible by dataBytesPerSample! Calculation error\n"));
	}

	// copy date
	memcpy(buffer + sizeof(sls_detector_header), ramValues + udpHeaderOffset, numBytesToWrite);

	// increment offset
	udpHeaderOffset += numBytesToWrite;

	return numBytesToWrite + sizeof(sls_detector_header);
}
