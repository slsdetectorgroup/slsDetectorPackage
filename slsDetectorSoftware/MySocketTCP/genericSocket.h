
#ifndef GENERIC_SOCKET_H
#define GENERIC_SOCKET_H

#define SEND_REC_MAX_SIZE 4096
#define DEFAULT_PORTNO    1952
#define DEFAULT_BACKLOG 5





/**
 * 
 * @libdoc genericSocket provides some functions to open/close sockets both TCP and UDP
 *
 * @short some functions to open/close sockets both TCP and UDP
 * @author Anna Bergamaschi
 * @version 0.0
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

#ifdef __CINT__
//class  sockaddr_in;
class socklen_t;
class uint32_t;
class uint32_t_ss; 
// CINT view of types:
class sockaddr_in;
// {
//     unsigned short int sa_family;
//    unsigned char sa_data[14];
//   };
#else

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#endif



#include <unistd.h>
#include <string.h>
#include <iostream>

#include <math.h>
#include <errno.h>
using namespace std;

class genericSocket{

 public:

  /** 
     Communication protocol 
*/
enum communicationProtocol{
  TCP,  /**< TCP/IP */
  UDP /**< UDP */
};


 genericSocket(const char* const host_ip_or_name, unsigned short int const port_number, communicationProtocol p) : 
   portno(port_number), protocol(p), is_a_server(0), socketDescriptor(-1),file_des(-1)// sender (client): where to? ip 
   { // sender (client): where to? ip 
     // SetupParameters();
     strcpy(hostname,host_ip_or_name);
     struct hostent *hostInfo = gethostbyname(host_ip_or_name);
     if (hostInfo == NULL){
       cerr << "Exiting: Problem interpreting host: " << host_ip_or_name << "\n";
     } else {
       // Set some fields in the serverAddress structure.  
       serverAddress.sin_family = hostInfo->h_addrtype;
       memcpy((char *) &serverAddress.sin_addr.s_addr,
	      hostInfo->h_addr_list[0], hostInfo->h_length);
       serverAddress.sin_port = htons(port_number);   
       socketDescriptor=0; //You can use send and recv, //would it work?????
     } 
   }

   
   int getProtocol(communicationProtocol p) {
     switch (p) {
     case TCP:
       return SOCK_STREAM;
       break;
     case UDP:
       return SOCK_DGRAM;
       
     default: 
       cerr << "unknow protocol " << p << endl;
       return -1;
     }
   }
   
   int getProtocol() {return getProtocol(protocol);};
   



  /** 
      The constructor for a server
      @short the contructor for a server
      \param protocol 

  */
  
   genericSocket(unsigned short int const port_number, communicationProtocol p): portno(port_number),protocol(p), is_a_server(1),socketDescriptor(-1), file_des(-1){
     strcpy(hostname,"localhost"); //needed?!?!?!?
    

     socketDescriptor = socket(AF_INET, getProtocol(),0); //tcp

     if (socketDescriptor < 0) {
       cerr << "Can not create socket "<<endl;
       return;
     } 
     
     // Set some fields in the serverAddress structure.  
     serverAddress.sin_family = AF_INET;
     serverAddress.sin_port = htons(portno);
     serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
     
     if(bind(socketDescriptor,(struct sockaddr *) &serverAddress,sizeof(serverAddress))<0){
       cerr << "Can not bind socket "<<endl;
       socketDescriptor=-1;
       return;
     }
     if (getProtocol()==SOCK_STREAM)
       listen(socketDescriptor, DEFAULT_BACKLOG); 
     
    
   }
  
     /** 
	 The destructor: disconnects and close the socket
	 @short the destructor 
	 
     */
     ~genericSocket(){				\
       Disconnect();
       if (socketDescriptor >= 0){		\
	 close(socketDescriptor);		\
       }					\
       file_des=-1;				\
     };
     

     /** @short if client returns hostname for connection
	 \param name string to write the hostname to
	 \returns 0 if client, 1 if server (in this case ignore name return value)

     */
     int getHostname(char *name){
       if (is_a_server==0) {
	 strcpy(name,getHostname().c_str());
       }
       return is_a_server;
     };
      /** @short if client returns hostname for connection
	 \returns hostname

      */
     string getHostname(){return string(hostname);};

     /** @short returns port number for connection
	 \returns port number
     */
     int getPortNumber(){return portno;};

    /** @short returns communication protocol
	 \returns TCP or UDP
     */
     int getCommunicationProtocol(){return protocol;};


    /** @short returns error status
	 \returns 1 if error
     */
     int getErrorStatus(){if (socketDescriptor<0) return 1; else return 0;};
     

 /** @short etablishes connection; disconnect should always follow
	 \returns 1 if error
     */
     int  Connect(){

       if(file_des>0) return file_des;

       if(is_a_server && protocol==TCP){ //server tcp; the server will wait for the clients connection
	 if (socketDescriptor>0) {
	   if ((file_des = accept(socketDescriptor,(struct sockaddr *) &clientAddress, &clientAddress_length)) < 0) {
	     cerr << "Error: with server accept, connection refused"<<endl;
	     switch(errno) {
	     case EWOULDBLOCK:
	       printf("ewouldblock eagain\n");
	       break;
	     case EBADF:
	       printf("ebadf\n");
	       break;
	     case ECONNABORTED:
	       printf("econnaborted\n");
	       break;
	     case EFAULT:
	       printf("efault\n");
	       break;
	     case EINTR:
	       printf("eintr\n");
	       break;
	     case EINVAL:
	       printf("einval\n");
	       break;
	     case EMFILE: 
	       printf("emfile\n");
	       break;
	     case ENFILE:
	       printf("enfile\n");
	       break;
	     case ENOTSOCK:
	       printf("enotsock\n");
	       break;
	     case EOPNOTSUPP:
	       printf("eOPNOTSUPP\n");
	       break;
	     case ENOBUFS:
	       printf("ENOBUFS\n");
	       break;
	     case ENOMEM:
	       printf("ENOMEM\n");
	       break;
	     case ENOSR:
	       printf("ENOSR\n");
	       break;
	     case EPROTO:
	       printf("EPROTO\n");
	       break;
	     default:
	       printf("unknown error\n");
	     }
	     socketDescriptor=-1;
	   } 
#ifdef VERY_VERBOSE
	   else 
	     cout << "client connected "<< file_des << endl;
#endif
	   
	 }
	 // file_des = socketDescriptor;
	 
#ifdef VERY_VERBOSE
	 cout << "fd " << file_des  << endl; 
#endif
       } else {  
	 if (socketDescriptor<=0)
	   socketDescriptor = socket(AF_INET, getProtocol(),0);  
	 //    SetTimeOut(10);
	 if (socketDescriptor < 0){
	   cerr << "Can not create socket "<<endl;
	   file_des = socketDescriptor;
	 } else {
	   
	   if(connect(socketDescriptor,(struct sockaddr *) &serverAddress,sizeof(serverAddress))<0){
	     cerr << "Can not connect to socket "<<endl;
	     file_des = -1;
	   } else
	     file_des = socketDescriptor;
	 }
	 
       }

       return file_des;
     }




     /** @short free connection */
     void Disconnect(){
       
       if(file_des>=0){ //then was open
	 if(is_a_server){ 
	   close(file_des);
	 }
	 else { 
      close(socketDescriptor);
      socketDescriptor=-1;
	 } 
	 file_des=-1;
       }  
     }; 



     /** Set the socket timeout ts is in seconds */
     int SetTimeOut(int ts){


       if (ts<=0)
	 return -1;
       
       //cout << "socketdescriptor "<< socketDescriptor << endl; 
       struct timeval tout;
       tout.tv_sec  = 0;
       tout.tv_usec = 0;
       if(::setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(struct timeval)) <0)
	 {
	   cerr << "Error in setsockopt SO_RCVTIMEO "<< 0 << endl;
	 }
       tout.tv_sec  = ts;
       tout.tv_usec = 0;
       if(::setsockopt(socketDescriptor, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(struct timeval)) < 0)
	 {
	   cerr << "Error in setsockopt SO_SNDTIMEO " << ts <<  endl;
	 }
       return 0;
       

     };

     
/*      //The following two functions will connectioned->send/receive->disconnect */
/*   int  SendData(void* buf,int length);//length in characters */
/*   int  ReceiveData(void* buf,int length); */
  

/*   //The following two functions stay connected, blocking other connections, and must be manually disconnected, */
/*   //          when the last call is a SendData() or ReceiveData() the disconnection will be done automatically */
/*   //These function will also automatically disconnect->reconnect if */
/*   //          two reads (or two writes) are called in a row to preserve the data send/receive structure  */
/*   int  SendDataAndKeepConnection(void* buf,int length); */
/*   int  ReceiveDataAndKeepConnection(void* buf,int length); */


/*   // Danger! These functions do not connect nor disconnect nor flush the data! be sure that send-receive match perfectly on both server and client side! */
/*   int  SendDataOnly(void* buf,int length); */
/*   int  ReceiveDataOnly(void* buf,int length); */

 protected:

  char hostname[1000];
  int portno;
  communicationProtocol protocol; 

  int is_a_server;
  int socketDescriptor;
  struct sockaddr_in clientAddress, serverAddress;
  socklen_t clientAddress_length;

  int file_des;


};
#endif
