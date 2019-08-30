#pragma once
/**
 * Get UDP socket desicriptor
 * @param udp port index
 */
int getUdPSocketDescriptor(int index);

/**
 * Set udp destination
 * @param index udp port index
 * @param ip udp destination ip
 * @param port udp destination port
 */
int setUDPDestinationDetails(int index, const char* ip, unsigned short int port);

/**
 * Create udp socket
 * @param index udp port index
 */
int createUDPSocket(int index);

/**
 * Writes to socket file descriptor
 * @param index udp port index
 * @param buf pointer to memory to write
 * @param length length of buffer to write to socket
 */
int sendUDPPacket(int index, const char* buf, int length);

/**
 * Close udp socket
 * @index udp port index
 */
void closeUDPSocket(int index);
