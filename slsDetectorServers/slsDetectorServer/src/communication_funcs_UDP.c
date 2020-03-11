#include "communication_funcs_UDP.h"
#include "clogger.h"
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

int udpSockfd[2] = {-1, -1};
struct addrinfo* udpServerAddrInfo[2] = {0, 0};
unsigned short int udpDestinationPort[2] = {0, 0};
char udpDestinationIp[2][MAX_STR_LENGTH] = {"", ""};

//DEFAULT_TX_UDP_PORT;// src port
int getUdPSocketDescriptor(int index) {
	return udpSockfd[index];
}

int setUDPDestinationDetails(int index, const char* ip, unsigned short int port) {
	udpDestinationPort[index] = port;
	size_t len = strlen(ip);
	memset(udpDestinationIp[index], 0, MAX_STR_LENGTH);
	strncpy(udpDestinationIp[index], ip, len > MAX_STR_LENGTH ? MAX_STR_LENGTH : len );

	if (udpServerAddrInfo[index]) {
		freeaddrinfo(udpServerAddrInfo[index]);
		udpServerAddrInfo[index] = 0;
	}

	// convert ip to internet address
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	char sport[100];
	memset(sport, 0, 100);
	sprintf(sport, "%d", udpDestinationPort[index]);
	int err = getaddrinfo(udpDestinationIp[index], sport, &hints, &udpServerAddrInfo[index]);
	if (err != 0) {
		LOG(logERROR, ("Failed to resolve remote socket address %s at port %d. "
				"(Error code:%d, %s)\n", udpDestinationIp[index], udpDestinationPort[index], err, gai_strerror(err)));
		return FAIL;
	}
	if (udpServerAddrInfo[index] == NULL) {
		LOG(logERROR, ("Failed to resolve remote socket address %s at port %d "
				"(getaddrinfo returned NULL)\n", udpDestinationIp[index], udpDestinationPort[index]));
		udpServerAddrInfo[index] = 0;
		return FAIL;
	}

	return OK;
}

int createUDPSocket(int index) {
	LOG(logDEBUG2, ("Creating UDP Socket %d\n", index));
	if (!strlen(udpDestinationIp[index])) {
		LOG(logERROR, ("No destination UDP ip specified.\n"));
		return FAIL;
	}

	if (udpSockfd[index] != -1) {
		LOG(logERROR, ("Strange that Udp socket was still open. Closing it to create a new one\n"));
		close(udpSockfd[index]);
		udpSockfd[index] = -1;
	}

	// Creating socket file descriptor
	udpSockfd[index] = socket(udpServerAddrInfo[index]->ai_family, udpServerAddrInfo[index]->ai_socktype, udpServerAddrInfo[index]->ai_protocol);
	if (udpSockfd[index]  == -1 ) {
		LOG(logERROR, ("UDP socket at port %d failed. (Error code:%d, %s)\n",
				udpDestinationPort[index], errno, gai_strerror(errno)));
		return FAIL;
	}
	LOG(logINFO, ("Udp client socket created for server (port %d, ip:%s)\n",
			udpDestinationPort[index], udpDestinationIp[index]));

	// connecting allows to use "send/write" instead of "sendto", avoiding checking for server address for each packet
	// using write without a connect will end in segv
	if (connect(udpSockfd[index],udpServerAddrInfo[index]->ai_addr, udpServerAddrInfo[index]->ai_addrlen)==-1) {
		LOG(logERROR, ("Could not connect to UDP server at ip:%s, port:%d. (Error code:%d, %s)\n",
				udpDestinationIp[index], udpDestinationPort[index],  errno, gai_strerror(errno)));
	}
	LOG(logINFO, ("Udp client socket connected\n",
				udpDestinationPort[index], udpDestinationIp[index]));
	return OK;
}

int sendUDPPacket(int index, const char* buf, int length) {
	int n = write(udpSockfd[index], buf, length);
	// udp sends atomically, no need to handle partial data
	if (n == -1) {
		LOG(logERROR, ("Could not send udp packet for socket %d. (Error code:%d, %s)\n",
				index, n, errno, gai_strerror(errno)));
	} else {
		LOG(logDEBUG2, ("%d bytes sent\n", n));
	}
	return n;
}

void closeUDPSocket(int index) {
	if (udpSockfd[index] != -1) {
		LOG(logINFO, ("Udp client socket closed\n"));
		close(udpSockfd[index]);
		udpSockfd[index] = -1;
	}
}
