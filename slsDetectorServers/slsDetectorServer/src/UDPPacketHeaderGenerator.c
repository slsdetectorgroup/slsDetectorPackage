// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "UDPPacketHeaderGenerator.h"
#include "clogger.h"
#include "sls/sls_detector_defs.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define UDP_PACKET_HEADER_VERSION (0x1)

extern const enum detectorType myDetectorType;

extern int analogDataBytes;
extern int digitalDataBytes;
extern int transceiverDataBytes;
extern char *analogData;
extern char *digitalData;
extern char *transceiverData;

int analogOffset = 0;
int digitalOffset = 0;
int transceiverOffset = 0;
uint32_t udpPacketNumber = 0;
uint64_t udpFrameNumber = 0;

uint32_t getUDPPacketNumber() { return udpPacketNumber; }

uint64_t getUDPFrameNumber() { return udpFrameNumber; }
void setUDPFrameNumber(uint64_t fnum) {
    LOG(logINFO, ("Setting next frame number also for 1g to %lld\n", fnum));
    // this gets incremented before setting it
    udpFrameNumber = fnum - 1;
}

void createUDPPacketHeader(char *buffer, uint16_t id) {
    memset(buffer, 0, sizeof(sls_detector_header));
    sls_detector_header *header = (sls_detector_header *)(buffer);

    header->modId = id;
    // row and column remains 0 (only used by ctb now)
    // uint64_t timestamp FIXME: needed?
    header->detType = (uint16_t)myDetectorType;
    header->version = UDP_PACKET_HEADER_VERSION;

    // reset offset
    analogOffset = 0;
    digitalOffset = 0;
    transceiverOffset = 0;
    // reset frame number
    udpFrameNumber = 0;
}

int fillUDPPacket(char *buffer) {
    LOG(logDEBUG2, ("Analog (databytes:%d, offset:%d)\n Digital (databytes:%d "
                    "offset:%d)\n\n Transceiver (databytes:%d offset:%d)\n",
                    analogDataBytes, analogOffset, digitalDataBytes,
                    digitalOffset, transceiverDataBytes, transceiverOffset));
    // reached end of data for one frame
    if (analogOffset >= analogDataBytes && digitalOffset >= digitalDataBytes &&
        transceiverOffset >= transceiverDataBytes) {
        // reset offset
        analogOffset = 0;
        digitalOffset = 0;
        transceiverOffset = 0;
        return 0;
    }

    sls_detector_header *header = (sls_detector_header *)(buffer);

    // update frame number, starts at 1 (reset packet number)
    if (analogOffset == 0 && digitalOffset == 0 && transceiverOffset == 0) {
        ++udpFrameNumber;
        header->frameNumber = udpFrameNumber;
        udpPacketNumber = -1;
    }

    // increment  and copy udp packet number (starts at 0)
    ++udpPacketNumber;
    header->packetNumber = udpPacketNumber;
    LOG(logDEBUG2, ("Creating packet number %d (fnum:%lld)\n", udpPacketNumber,
                    (long long int)udpFrameNumber));

    int freeBytes = UDP_PACKET_DATA_BYTES;

    // analog data
    int analogBytes = 0;
    if (analogOffset < analogDataBytes) {
        // bytes to copy
        analogBytes = ((analogOffset + freeBytes) <= analogDataBytes)
                          ? freeBytes
                          : (analogDataBytes - analogOffset);
        // copy
        memcpy(buffer + sizeof(sls_detector_header), analogData + analogOffset,
               analogBytes);
        // increment offset
        analogOffset += analogBytes;
        // decrement free bytes
        freeBytes -= analogBytes;
    }

    // digital data
    int digitalBytes = 0;
    if (freeBytes && digitalOffset < digitalDataBytes) {
        // bytes to copy
        digitalBytes = ((digitalOffset + freeBytes) <= digitalDataBytes)
                           ? freeBytes
                           : (digitalDataBytes - digitalOffset);
        // copy
        memcpy(buffer + sizeof(sls_detector_header) + analogBytes,
               digitalData + digitalOffset, digitalBytes);
        // increment offset
        digitalOffset += digitalBytes;
        // decrement free bytes
        freeBytes -= digitalBytes;
    }

    // transceiver data
    int transceiverBytes = 0;
    if (freeBytes && transceiverOffset < transceiverDataBytes) {
        // bytes to copy
        transceiverBytes =
            ((transceiverOffset + freeBytes) <= transceiverDataBytes)
                ? freeBytes
                : (transceiverDataBytes - transceiverOffset);
        // copy
        memcpy(buffer + sizeof(sls_detector_header) + analogBytes +
                   digitalBytes,
               transceiverData + transceiverOffset, transceiverBytes);
        // increment offset
        transceiverOffset += transceiverBytes;
        // decrement free bytes
        freeBytes -= transceiverBytes;
    }

    // pad data
    if (freeBytes) {
        memset(buffer + sizeof(sls_detector_header) + analogBytes +
                   digitalBytes + transceiverBytes,
               0, freeBytes);
        LOG(logDEBUG1, ("Padding %d bytes for fnum:%lld pnum:%d\n", freeBytes,
                        (long long int)udpFrameNumber, udpPacketNumber));
    }

    return UDP_PACKET_DATA_BYTES + sizeof(sls_detector_header);
}
