#pragma once
/************************************************
 * @file zmqSocket.h
 * @short functions to open/close zmq sockets
 ***********************************************/
/**
 *@short functions to open/close zmq sockets
 */

#include "ansi.h"
//#include "sls_receiver_defs.h"

#include <zmq.h>
#include <errno.h>
#include <netdb.h>				//gethostbyname()
#include <arpa/inet.h>			//inet_ntoa
#include <rapidjson/document.h> //json header in zmq stream
using namespace rapidjson;

#define DEFAULT_ZMQ_PORTNO 	40001

class ZmqSocket {

public:

	//Socket Options for optimization
	//ZMQ_LINGER default is already -1 means no messages discarded. use this options if optimizing required
	//ZMQ_SNDHWM default is 0 means no limit. use this to optimize if optimizing required
	// eg. int value = -1;
	// if (zmq_setsockopt(socketDescriptor, ZMQ_LINGER, &value,sizeof(value))) {
	//	Close();
	// }

	/**
	 * Constructor for a client
	 * Creates socket, context and connects to server
	 * @param hostname hostname or ip of server
	 * @param portnumber port number
	 */
	ZmqSocket (const char* const hostname_or_ip, const uint32_t  portnumber):
		portno (portnumber),
		server (false),
		contextDescriptor (NULL),
		socketDescriptor (NULL)
	{
		char ip[MAX_STR_LENGTH] = "";
		strcpy(ip, hostname_or_ip);

		// construct address
		if (strchr (hostname_or_ip, '.') != NULL) {
			// convert hostname to ip
			char* ptr = ConvertHostnameToIp (hostname_or_ip);
			if (ptr == NULL)
				return;
			strcpy(ip, ptr);
			delete ptr;
		}
		sprintf (serverAddress, "tcp://%s:%d", ip, portno);

		// create context
		contextDescriptor = zmq_ctx_new();
		if (contextDescriptor == NULL)
			return;

		// create publisher
		socketDescriptor = zmq_socket (contextDescriptor, ZMQ_PULL);
		if (socketDescriptor == NULL) {
			PrintError ();
			Close ();
		}

		//Socket Options provided above

		//connect socket
		if (zmq_connect(socketDescriptor, serverAddress) < 0) {
			PrintError ();
			Close ();
		}
	};

	/**
	 * Constructor for a server
	 * Creates socket, context and connects to server
	 * @param hostname hostname or ip of server
	 * @param portnumber port number
	 */
	ZmqSocket (const uint32_t portnumber):
		portno (portnumber),
		server (true),
		contextDescriptor (NULL),
		socketDescriptor (NULL)
	{
		// create context
		contextDescriptor = zmq_ctx_new();
		if (contextDescriptor == NULL)
			return;
		// create publisher
		socketDescriptor = zmq_socket (contextDescriptor, ZMQ_PUSH);
		if (socketDescriptor == NULL) {
			PrintError ();
			Close ();
		}

		//Socket Options provided above

		// construct address
		sprintf (serverAddress,"tcp://*:%d", portno);
		// bind address
		if (zmq_bind (socketDescriptor, serverAddress) < 0) {
			PrintError ();
			Close ();
		}
	};

	/**
	 * Destructor
	 */
	~ZmqSocket () {
		Disconnect();
		Close();
	};

	/**
	 * Returns error status
	 * @returns true if error else false
	 */
	bool IsError() { if (socketDescriptor == NULL) return true; return false; };

	/**
	 * Returns Server Address
	 * @returns Server Address
	 */
	char* GetZmqServerAddress () { return serverAddress; };

	/**
	 * Returns Port Number
	 * @returns Port Number
	 */
	uint32_t GetPortNumber () { return portno; };

	/**
	 * Returns Socket Descriptor
	 * @reutns Socket descriptor
	 */

	void* GetsocketDescriptor () { return socketDescriptor; };

	/**
	 * Unbinds the Socket
	 */
	void Disconnect () {
		if (server)
			zmq_unbind (socketDescriptor, serverAddress);
		else
			zmq_disconnect (socketDescriptor, serverAddress);
	};

	/**
	 * Close Socket and destroy Context
	 */
	void Close () {
		if (socketDescriptor != NULL) {
			zmq_close (socketDescriptor);
			socketDescriptor = NULL;
		}
		if (contextDescriptor != NULL) {
			zmq_ctx_destroy (contextDescriptor);
			contextDescriptor = NULL;
		}
	};

	/**
	 * Convert Hostname to ip
	 * @param hostname hostname
	 * @returns string with ip or NULL if error
	 */
	char* ConvertHostnameToIp (const char* const hostname) {
		struct hostent *he = gethostbyname (hostname);
		if (he == NULL){
			cprintf (RED,"Error: Could not convert hostname to ip (zmq)\n");
			return NULL;
		}
		return inet_ntoa (*(struct in_addr*)he->h_addr);
	};

	/**
	 * Send Message Header
	 * @param buf message
	 * @param length length of message
	 * @param dummy true if end of acquistion else false
	 * @returns 0 if error, else 1
	 */
	int SendHeaderData (uint32_t jsonversion, uint32_t dynamicrange, uint32_t npixelsx, uint32_t npixelsy,
			uint64_t acqIndex, uint64_t fIndex, char* fname, bool dummy,
			uint64_t frameNumber, uint32_t expLength, uint32_t packetNumber, uint64_t bunchId, uint64_t timestamp,
			uint16_t modId, uint16_t xCoord, uint16_t yCoord, uint16_t zCoord, uint32_t debug, uint16_t roundRNumber,
			uint8_t detType, uint8_t version) {

		char buf[MAX_STR_LENGTH] = "";
		int length = sprintf(buf, jsonHeaderFormat,
				jsonversion, dynamicrange, npixelsx, npixelsy,
				acqIndex, fIndex, fname, dummy?1:0,
				frameNumber, expLength, packetNumber, bunchId, timestamp,
				modId, xCoord, yCoord, zCoord, debug, roundRNumber,
				detType, version);
#ifdef VERBOSE
	printf("%d Streamer: buf:%s\n", index, buf);
#endif

		if(zmq_send (socketDescriptor, buf, length, ZMQ_SNDMORE) < 0) {
			PrintError ();
			return 0;
		}
		return 1;
	};

	/**
	 * Send Message Body
	 * @param buf message
	 * @param length length of message
	 * @returns 0 if error, else 1
	 */
	int SendData (char* buf, int length) {
		if(zmq_send (socketDescriptor, buf, length, 0) < 0) {
			PrintError ();
			return 0;
		}
		return 1;
	};


	/**
	 * Receive Message
	 * @param index self index for debugging
	 * @param message message
	 * @returns length of message, -1 if error
	 */
	int ReceiveMessage(const int index, zmq_msg_t& message) {
		int length = zmq_msg_recv (&message, socketDescriptor, 0);
		if (length == -1) {
			PrintError ();
			cprintf (BG_RED,"Error: Could not read header for socket %d\n",index);
		}
		return length;
	};


	/**
	 * Receive Header
	 * @param index self index for debugging
	 * @param acqIndex address of acquisition index
	 * @param frameIndex address of frame index
	 * @param subframeIndex address of subframe index
	 * @param filename address of file name
	 * @returns 0 if error or end of acquisition, else 1
	 */
	int ReceiveHeader(const int index, uint64_t &acqIndex,
			uint64_t &frameIndex, uint32_t &subframeIndex, string &filename)
	{
		zmq_msg_t message;
		zmq_msg_init (&message);
		int len = ReceiveMessage(index, message);
		if ( len > 0 ) {
			bool dummy = false;
			if ( ParseHeader (index, len, message, acqIndex, frameIndex, subframeIndex, filename, dummy)) {
				zmq_msg_close (&message);
#ifdef VERBOSE
				cprintf( RED,"%d Length: %d Header:%s \n", index, length, (char*) zmq_msg_data (&message) );
#endif
				if (dummy) {
#ifdef VERBOSE
					cprintf(RED,"%d Received end of acquisition\n", index);
#endif
					return 0;
				}
				return 1;
			}
		}
		zmq_msg_close(&message);
		return 0;
	};

	/**
	 * Receive Data
	 * @param index self index for debugging
	 * @param buf buffer to copy image data to
	 * @param size size of image
	 * @returns length of data received
	 */
	int ReceiveData(const int index, int* buf, const int size)
	{
		zmq_msg_t message;
		zmq_msg_init (&message);
		int length = ReceiveMessage(index, message);

		//actual data
		if (length == size) {
#ifdef VERBOSE
			cprintf(BLUE,"%d actual data\n", index);
#endif
			memcpy((char*)buf, (char*)zmq_msg_data(&message), size);
		}

		//incorrect size
		else {
			cprintf(RED,"Error: Received weird packet size %d for socket %d\n", length, index);
			memset((char*)buf,0xFF,size);
		}

		zmq_msg_close(&message);
		return length;
	};


	/**
	 * Parse Header
	 * @param index self index for debugging
	 * @param length length of message
	 * @param message message
	 * @param acqIndex address of acquisition index
	 * @param frameIndex address of frame index
	 * @param subframeIndex address of subframe index
	 * @param filename address of file name
	 * @param dummy true if end of acquisition else false
	 * @returns true if successfull else false
	 */
	int ParseHeader(const int index, int length, zmq_msg_t& message, uint64_t &acqIndex,
			uint64_t &frameIndex, uint32_t &subframeIndex, string &filename, bool& dummy)
	{
		Document d;
		if ( d.Parse( (char*) zmq_msg_data (&message), zmq_msg_size (&message)).HasParseError() ) {
			cprintf( RED,"%d Could not parse. len:%d: Message:%s \n", index, length, (char*) zmq_msg_data (&message) );
			fflush ( stdout );
			char* buf =  (char*) zmq_msg_data (&message);
			for ( int i= 0; i < length; ++i ) {
				cprintf(RED,"%02x ",buf[i]);
			}
			printf("\n");
			fflush( stdout );
			return 0;
		}

		int temp = d["data"].GetUint();
		dummy = temp ? true : false;
		if (dummy) {
			acqIndex 		= d["acqIndex"].GetUint64();
			frameIndex 		= d["fIndex"].GetUint64();
			subframeIndex 	= -1;
			if(d["bitmode"].GetInt()==32 && d["detType"].GetUint() == slsReceiverDefs::EIGER) {
				subframeIndex 	= d["expLength"].GetUint();
			}
			filename 		= d["fname"].GetString();
#ifdef VERYVERBOSE
			cout << "Data: " << temp << endl;
			cout << "Acquisition index: " << acqIndex << endl;
			cout << "Frame index: " << frameIndex << endl;
			cout << "Subframe index: " << subframeIndex << endl;
			cout << "File name: " << filename << endl;
#endif
		}
		return 1;
	};


	/**
	 * Print error
	 */
	void PrintError () {
		switch (errno) {
		case EINVAL:
			cprintf(RED, "Error: The socket type/option or value/endpoint supplied is invalid (zmq)\n");
			break;
		case EAGAIN:
			cprintf(RED, "Error: Non-blocking mode was requested and the message cannot be sent/available at the moment (zmq)\n");
			break;
		case ENOTSUP:
			cprintf(RED, "Error: The zmq_send()/zmq_msg_recv() operation is not supported by this socket type (zmq)\n");
			break;
		case EFSM:
			cprintf(RED, "Error: The zmq_send()/zmq_msg_recv() unavailable now as socket in inappropriate state (eg. ZMQ_REP). Look up messaging patterns (zmq)\n");
			break;
		case EFAULT:
			cprintf(RED, "Error: The provided context/message is invalid (zmq)\n");
			break;
		case EMFILE:
			cprintf(RED, "Error: The limit on the total number of open ØMQ sockets has been reached (zmq)\n");
			break;
		case EPROTONOSUPPORT:
			cprintf(RED, "Error: The requested transport protocol is not supported (zmq)\n");
			break;
		case ENOCOMPATPROTO:
			cprintf(RED, "Error: The requested transport protocol is not compatible with the socket type (zmq)\n");
			break;
		case EADDRINUSE:
			cprintf(RED, "Error: The requested address is already in use (zmq)\n");
			break;
		case EADDRNOTAVAIL:
			cprintf(RED, "Error: The requested address was not local (zmq)\n");
			break;
		case ENODEV:
			cprintf(RED, "Error: The requested address specifies a nonexistent interface (zmq)\n");
			break;
		case ETERM:
			cprintf(RED, "Error: The ØMQ context associated with the specified socket was terminated (zmq)\n");
			break;
		case ENOTSOCK:
			cprintf(RED, "Error: The provided socket was invalid (zmq)\n");
			break;
		case EINTR:
			cprintf(RED, "Error: The operation was interrupted by delivery of a signal (zmq)\n");
			break;
		case EMTHREAD:
			cprintf(RED, "Error: No I/O thread is available to accomplish the task (zmq)\n");
			break;
		default:
			cprintf(RED, "Error: Unknown socket error (zmq)\n");
			break;
		}
	};


private:
	/** Port Number */
	uint32_t portno;

	/** true if server, else false */
	bool server;

	/** Context Descriptor */
	void* contextDescriptor;

	/** Socket Descriptor */
	void* socketDescriptor;

	/** Server Address */
	char serverAddress[1000];

	/** Json Header Format */
	static const char* jsonHeaderFormat =
			"{"
			"\"jsonversion\":%u, "
			"\"bitmode\":%d, "
			"\"shape\":[%d, %d], "
			"\"acqIndex\":%llu, "
			"\"fIndex\":%llu, "
			"\"fname\":\"%s\", "
			 "\"data\": %d, "

			"\"frameNumber\":%llu, "
			"\"expLength\":%u, "
			"\"packetNumber\":%u, "
			"\"bunchId\":%llu, "
			"\"timestamp\":%llu, "
			"\"modId\":%u, "
			"\"xCoord\":%u, "
			"\"yCoord\":%u, "
			"\"zCoord\":%u, "
			"\"debug\":%u, "
			"\"roundRNumber\":%u, "
			"\"detType\":%u, "
			"\"version\":%u"
			"}\n\0";

};
