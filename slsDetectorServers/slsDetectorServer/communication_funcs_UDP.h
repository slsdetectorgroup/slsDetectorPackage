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

int udpSockfd = -1;
struct addrinfo* udpServerAddrInfo = 0;
unsigned short int udpDestinationPort = 0;
char udpDestinationIp[MAX_STR_LENGTH] = "";

//DEFAULT_TX_UDP_PORT;// src port
int getUdPSocketDescriptor() {
	return udpSockfd;
}

int setUDPDestinationDetails(const char* ip, unsigned short int port) {
	udpDestinationPort = port;
	size_t len = strlen(ip);
	strncpy(udpDestinationIp, ip, len > MAX_STR_LENGTH ? MAX_STR_LENGTH : len );

	if (udpServerAddrInfo) {
		freeaddrinfo(udpServerAddrInfo);
		udpServerAddrInfo = 0;
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
	sprintf(sport, "%d", udpDestinationPort);
	int err = getaddrinfo(udpDestinationIp, sport, &hints, &udpServerAddrInfo);
	if (err != 0) {
		FILE_LOG(logERROR, ("Failed to resolve remote socket address %s at port %d. "
				"(Error code:%d, %s)\n", udpDestinationIp, udpDestinationPort, err, gai_strerror(err)));
		return FAIL;
	}
	if (udpServerAddrInfo == NULL) {
		FILE_LOG(logERROR, ("Failed to resolve remote socket address %s at port %d "
				"(getaddrinfo returned NULL)\n", udpDestinationIp, udpDestinationPort));
		udpServerAddrInfo = 0;
		return FAIL;
	}

	return OK;
}

int createUDPSocket() {
	if (!strlen(udpDestinationIp)) {
		FILE_LOG(logERROR, ("No destination UDP ip specified.\n"));
		return FAIL;
	}

	if (udpSockfd != -1) {
		FILE_LOG(logERROR, ("Strange that Udp socket was still open. Closing it to create a new one\n"));
		close(udpSockfd);
		udpSockfd = -1;
	}

	// Creating socket file descriptor
	udpSockfd = socket(udpServerAddrInfo->ai_family, udpServerAddrInfo->ai_socktype, udpServerAddrInfo->ai_protocol);
	if (udpSockfd  == -1 ) {
		FILE_LOG(logERROR, ("UDP socket at port %d failed. (Error code:%d, %s)\n",
				udpDestinationPort, errno, gai_strerror(errno)));
		return FAIL;
	}
	FILE_LOG(logINFO, ("Udp client socket created for server (port %d, ip:%s)\n",
			udpDestinationPort, udpDestinationIp));

	// connecting allows to use "send/write" instead of "sendto", avoiding checking for server address for each packet
	// using write without a connect will end in segv
	if (connect(udpSockfd,udpServerAddrInfo->ai_addr, udpServerAddrInfo->ai_addrlen)==-1) {
		FILE_LOG(logERROR, ("Could not connect to UDP server at ip:%s, port:%d. (Error code:%d, %s)\n",
				udpDestinationIp, udpDestinationPort,  errno, gai_strerror(errno)));
	}
	FILE_LOG(logINFO, ("Udp client socket connected\n",
				udpDestinationPort, udpDestinationIp));
	return OK;
}

int sendUDPPacket(const char* buf, int length) {
	int n = write(udpSockfd, buf, length);
	// udp sends atomically, no need to handle partial data
	if (n == -1) {
		FILE_LOG(logERROR, ("Could not send udp packet. (Error code:%d, %s)\n",
				n, errno, gai_strerror(errno)));
	} else {
		FILE_LOG(logDEBUG2, ("%d bytes sent\n", n));
	}
	return n;
}

void closeUDPSocket() {
	close(udpSockfd);
	udpSockfd = -1;
	if (udpServerAddrInfo) {
		freeaddrinfo(udpServerAddrInfo);
		udpServerAddrInfo = 0;
	}
}
