
#ifndef MY_SOCKET_TCP_H
#define MY_SOCKET_TCP_H 




/**
 * 
 * @libdoc The MySocketTCP class provides a simple interface for creating and sending/receiving data over a TCP socket.
 *
 * @short This class provides a simple interface for creating and sending/receiving data over a TCP socket.
 * @author Ian Johnson
 * @version 1.0
 */



//version 1.0, base development, Ian 19/01/09

/* Modified by anna on 19.01.2009 */
/*
  canceled SetupParameters() and varaibles intialized in the constructors' headers;
  defined SEND_REC_MAX_SIZE (for compatibilty with mythen (and possibly other)  pure C servers (i would move it to the common header file)

  added #ifndef C_ONLY... to cutout class definition when including in pure C servers (can be removed if SEND_REC_MAX_SIZE is moved to the common header file)

  defined private variables char hostname[1000] and int portno to store connection informations;

  defined public functions  int getHostname(char *name) and  int getPortNumber() to retrieve connection informations
  
  added public function int getErrorStatus() returning 1 if socketDescriptor<0

  remove exits in the constructors and replace them with socketDescriptor=-1

  replaced the argument of send/receive data with void (to avoid too much casting or compiler errors/warnings)

  added a function which really does not close the socket between send/receive (senddataonly, receivedataonly)

*/


/* Modified by Anna on 31.10.2012

developed and 

*/


#include "genericSocket.h"
#define TCP_PACKET_SIZE 4096

class MySocketTCP: public genericSocket {

 public:
  MySocketTCP(const char* const host_ip_or_name, unsigned short int const port_number):  genericSocket(host_ip_or_name, port_number,TCP), last_keep_connection_open_action_was_a_send(0){setPacketSize(TCP_PACKET_SIZE);}; // sender (client): where to? ip
    MySocketTCP(unsigned short int const port_number):genericSocket(port_number,TCP), last_keep_connection_open_action_was_a_send(0) {setPacketSize(TCP_PACKET_SIZE);}; // receiver (server) local no need for ip


  //The following two functions will connectioned->send/receive->disconnect
  int  SendData(void* buf,int length);//length in characters
  int  ReceiveData(void* buf,int length);
 

  //The following two functions stay connected, blocking other connections, and must be manually disconnected,
  //          when the last call is a SendData() or ReceiveData() the disconnection will be done automatically
  //These function will also automatically disconnect->reconnect if
  //          two reads (or two writes) are called in a row to preserve the data send/receive structure 
  int  SendDataAndKeepConnection(void* buf,int length);
  int  ReceiveDataAndKeepConnection(void* buf,int length);

 private:


  bool last_keep_connection_open_action_was_a_send;


};
#endif
