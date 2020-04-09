#pragma once
/************************************************
 * @file zmqSocket.h
 * @short functions to open/close zmq sockets
 ***********************************************/
/**
 *@short functions to open/close zmq sockets
 */

#include "sls_detector_exceptions.h"
#include <rapidjson/document.h> //json header in zmq stream

#define MAX_STR_LENGTH 1000

// #define ZMQ_DETAIL
#define ROIVERBOSITY


class zmq_msg_t;
#include <map>

/** zmq header structure */
struct zmqHeader {
  /** true if incoming data, false if end of acquisition */
  bool data{true};
  uint32_t jsonversion{0};
  uint32_t dynamicRange{0};
  uint64_t fileIndex{0};
  /** number of detectors in x axis */
  uint32_t ndetx{0}; 
  /** number of detectors in y axis */
  uint32_t ndety{0}; 
  /** number of pixels/channels in x axis for this zmq socket */
  uint32_t npixelsx{0};
  /** number of pixels/channels in y axis for this zmq socket */
  uint32_t npixelsy{0}; 
  /** number of bytes for an image in this socket */
  uint32_t imageSize{0}; 
  /** frame number from detector */
  uint64_t acqIndex{0}; 
  /** frame index (starting at 0 for each acquisition) */
  uint64_t frameIndex{0};
  /** progress in percentage */
  int progress{0};
  /** file name prefix */
  std::string fname{""};
  /** header from detector */ 
  uint64_t frameNumber{0}; 
  uint32_t expLength{0};
  uint32_t packetNumber{0}; 
  uint64_t bunchId{0}; 
  uint64_t timestamp{0}; 
  uint16_t modId{0};
  uint16_t row{0}; 
  uint16_t column{0}; 
  uint16_t reserved{0}; 
  uint32_t debug{0};
  uint16_t roundRNumber{0}; 
  uint8_t detType{0}; 
  uint8_t version{0};
  /** if image should be flipped across x axis */
  int flippedDataX{0}; 
  /** quad type (eiger hardware specific) */
  uint32_t quad{0}; 
  /** true if complete image, else missing packets */
  bool completeImage{false};
  /** additional json header */
  std::map<std::string, std::string> addJsonHeader;
};

class ZmqSocket {

  public:

    // Socket Options for optimization
    // ZMQ_LINGER default is already -1 means no messages discarded. use this
    // options if optimizing required ZMQ_SNDHWM default is 0 means no limit. use
    // this to optimize if optimizing required
    // eg. int value = -1;
    // if (zmq_setsockopt(socketDescriptor, ZMQ_LINGER, &value,sizeof(value))) {
    //	Close();
    /**
     * Constructor for a client
     * Creates socket, context and connects to server
     * @param hostname hostname or ip of server
     * @param portnumber port number
     */
    ZmqSocket(const char *const hostname_or_ip, const uint32_t portnumber);

    /**
     * Constructor for a server
     * Creates socket, context and connects to server
     * @param hostname hostname or ip of server
     * @param portnumber port number
     * @param ethip is the ip of the ethernet interface to stream zmq from
     */
    ZmqSocket(const uint32_t portnumber, const char *ethip);

    /**
     * Destructor
     */
    ~ZmqSocket() = default;

    /**
     * Returns Port Number
     * @returns Port Number
     */
    uint32_t GetPortNumber() { return portno; }

    /**
     * Returns Server Address
     * @returns Server Address
     */
    char *GetZmqServerAddress() { return sockfd.serverAddress; }

    /**
     * Returns Socket Descriptor
     * @reutns Socket descriptor
     */

    void *GetsocketDescriptor() { return sockfd.socketDescriptor; }

    /**
     * Connect client socket to server socket
     * @returns 1 for fail, 0 for success
     */
    int Connect();

    /**
     * Unbinds the Socket
     */
    void Disconnect() { sockfd.Disconnect(); };

    /**
     * Close Socket and destroy Context
     */
    void Close() { sockfd.Close(); };

    /**
     * Convert Hostname to Internet address info structure
     * One must use freeaddrinfo(res) after using it
     * @param hostname hostname
     * @param res address of pointer to address info structure
     * @return 1 for fail, 0 for success
     */
    // Do not make this static (for multi threading environment)
    int ConvertHostnameToInternetAddress(const char *const hostname,
                                         struct addrinfo **res);

    /**
     * Convert Internet Address structure pointer to ip string (char*)
     * Clears the internet address structure as well
     * @param res pointer to internet address structure
     * @param ip pointer to char array to store result in
     * @param ipsize size available in ip buffer
     * @return 1 for fail, 0 for success
     */
    // Do not make this static (for multi threading environment)
    int ConvertInternetAddresstoIpString(struct addrinfo *res, char *ip,
                                         const int ipsize);

    /**
     * Send Message Header
     * @param index self index for debugging
     * @param header zmq header (from json)
     * @returns 0 if error, else 1
     */
    int SendHeader(int index, zmqHeader header);

    /**
     * Send Message Body
     * @param buf message
     * @param length length of message
     * @returns 0 if error, else 1
     */
    int SendData(char *buf, int length);
  
    /**
     * Receive Header
     * @param index self index for debugging
     * @param zHeader filled out zmqHeader structure (parsed from json header)
     * @param version version that has to match, -1 to not care
     * @returns 0 if error or end of acquisition, else 1 (call
     * CloseHeaderMessage after parsing header)
     */
    int ReceiveHeader(const int index, zmqHeader& zHeader, uint32_t version);

    /**
     * Receive Data
     * @param index self index for debugging
     * @param buf buffer to copy image data to
     * @param size size of image
     * @returns length of data received
     */
    int ReceiveData(const int index, char *buf, const int size);

    /**
     * Print error
     */
    void PrintError();

  private:

	/**
     * Receive Message
     * @param index self index for debugging
     * @param message message
     * @returns length of message, -1 if error
     */
    int ReceiveMessage(const int index, zmq_msg_t &message);

    /**
     * Parse Header
     * @param index self index for debugging
     * @param length length of message
     * @param message message
     * @param zHeader filled out zmqHeader structure (parsed from json header)
     * @param version version that has to match, -1 to not care
     * @returns true if successful else false
     */
    int ParseHeader(const int index, int length, char *buff, 
      zmqHeader& zHeader, uint32_t version);

    /**
     * Class to close socket descriptors automatically
     * upon encountering exceptions in the ZmqSocket constructor
     */
    class mySocketDescriptors {
      public:
        /** Constructor */
        mySocketDescriptors();
        /** Destructor */
        ~mySocketDescriptors();
        /** Unbinds the Socket */
        void Disconnect();
        /** Close Socket and destroy Context */
        void Close();
        /** true if server, else false */
        bool server;
        /** Server Address */
        char serverAddress[1000];
        /** Context Descriptor */
        void *contextDescriptor;
        /** Socket Descriptor */
        void *socketDescriptor;
    };

  private:
    /** Port Number */
    uint32_t portno;

    /** Socket descriptor */
    mySocketDescriptors sockfd;


};
