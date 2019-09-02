#pragma once

#include <inttypes.h>

/** 
 * Get current udp packet number 
 */
uint32_t getUDPPacketNumber();

/** 
 * Get current udp frame number 
 */
uint64_t getUDPFrameNumber();

/**
 * Called for each UDP packet header creation
 * @param buffer pointer to header
 * @param id module id
 */
void createUDPPacketHeader(char* buffer, uint16_t id);

/** 
 * fill up the udp packet with data till its full 
 * @param buffer pointer to memory
 */
int fillUDPPacket(char* buffer);
