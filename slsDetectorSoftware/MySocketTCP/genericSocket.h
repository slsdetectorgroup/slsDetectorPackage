
#ifndef GENERIC_SOCKET_H
#define GENERIC_SOCKET_H





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
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

#endif

#include <unistd.h>
#include <string.h>
#include <iostream>

#include <math.h>
#include <errno.h>
using namespace std;

#define DEFAULT_PACKET_SIZE 1286
#define DEFAULT_PORTNO    1952
#define DEFAULT_BACKLOG 5

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
   //   portno(port_number), 
protocol(p), is_a_server(0), socketDescriptor(-1),file_des(-1), packet_size(DEFAULT_PACKET_SIZE)// sender (client): where to? ip 
   { 
/* 	  pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER; */
/* 	  mp=mp1; */
/* 	  pthread_mutex_init(&mp, NULL); */

	  //   strcpy(hostname,host_ip_or_name);
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
     clientAddress_length=sizeof(clientAddress);
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
      \param port_number port number to listen to
      \param p TCP or UDP
      \param eth interface name or IP address to listen to (if NULL, listen to all interfaces)

  */
  
   genericSocket(unsigned short int const port_number, communicationProtocol p, const char *eth=NULL): 
     //portno(port_number),
     protocol(p), is_a_server(1),socketDescriptor(-1), file_des(-1), packet_size(DEFAULT_PACKET_SIZE){

/* // you can specify an IP address: */
/*  */

/* // or you can let it automatically select one: */
/* myaddr.sin_addr.s_addr = INADDR_ANY; */


     char ip[20];

     strcpy(ip,"0.0.0.0");
     clientAddress_length=sizeof(clientAddress);
     
     if (eth) {
       strcpy(ip,nameToIp(string(eth)).c_str());
       if (string(ip)==string("0.0.0.0"))
	 strcpy(ip,eth);
     }
    
     // strcpy(hostname,"localhost"); //needed?!?!?!?
    

     socketDescriptor = socket(AF_INET, getProtocol(),0); //tcp

     if (socketDescriptor < 0) {
       cerr << "Can not create socket "<<endl;
       return;
     } 
     
     // Set some fields in the serverAddress structure.  
     serverAddress.sin_family = AF_INET;
     serverAddress.sin_port = htons(port_number);
     serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
     

     if (string(ip)!=string("0.0.0.0")) {
       if (inet_pton(AF_INET, ip, &(serverAddress.sin_addr)))
	 ;
       else
	 serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
     }


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
     

/*      /\** @short if client returns hostname for connection */
/* 	 \param name string to write the hostname to */
/* 	 \returns 0 if client, 1 if server (in this case ignore name return value) */

/*      *\/ */
/*      int getHostname(char *name){ */
/*        if (is_a_server==0) { */
/* 	 strcpy(name,getHostname().c_str()); */
/*        } */
/*        return is_a_server; */
/*      }; */
/*       /\** @short if client returns hostname for connection */
/* 	 \returns hostname */

/*       *\/ */
/*      string getHostname(){return string(hostname);}; */

/*      /\** @short returns port number for connection */
/* 	 \returns port number */
/*      *\/ */
/*      int getPortNumber(){return portno;}; */

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
     int  Connect(){//cout<<"connect"<<endl;

     if(file_des>0) return file_des;
       if (protocol==UDP) return -1;

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
    		 } else{
    			 file_des = socketDescriptor;
    			 /*cout<<"locking"<<endl;
    			 pthread_mutex_lock(&mp);
    			 cout<<"locked"<<endl;*/
    		 }
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
    		 /*cout<<"unlocking"<<endl;
    		 pthread_mutex_unlock(&mp);
    		 cout<<"unlocked"<<endl;*/
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

   
     int setPacketSize(int i=-1) { if (i>=0) packet_size=i; return packet_size;};

  

     static string ipToName(string ip) {
       struct ifaddrs *addrs, *iap;
       struct sockaddr_in *sa;
       char buf[32];

       strcpy(buf,"none");

       getifaddrs(&addrs);
       for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
	 if (iap->ifa_addr && (iap->ifa_flags & IFF_UP) && iap->ifa_addr->sa_family == AF_INET) {
	   sa = (struct sockaddr_in *)(iap->ifa_addr);
	   inet_ntop(iap->ifa_addr->sa_family, (void *)&(sa->sin_addr), buf, sizeof(buf));
	   if (ip==string(buf)) {
	     printf("%s\n", iap->ifa_name);
	   }
	 }
       }
       freeifaddrs(addrs);
       return string(buf);
     };
     
     static string nameToMac(string inf) {
       struct ifreq ifr;
       int sock, j, k;
       char mac[32];
       
       sock=getSock(inf,&ifr);
       
       if (-1==ioctl(sock, SIOCGIFHWADDR, &ifr)) {
	 perror("ioctl(SIOCGIFHWADDR) ");
	 return string("00:00:00:00:00:00");
       }
       for (j=0, k=0; j<6; j++) {
	 k+=snprintf(mac+k, sizeof(mac)-k-1, j ? ":%02X" : "%02X",
		     (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
       }
       mac[sizeof(mac)-1]='\0';
       
       return string(mac);
       
     };
    

     
     static string nameToIp(string inf){
       struct ifreq ifr;
       int sock;
       char *p, addr[32];
       
       sock=getSock(inf,&ifr);
       
       if (-1==ioctl(sock, SIOCGIFADDR, &ifr)) {
	 perror("ioctl(SIOCGIFADDR) ");
	 return string("0.0.0.0");
       }
       p=inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);
       strncpy(addr,p,sizeof(addr)-1);
       addr[sizeof(addr)-1]='\0';
       
       return string(addr);
       
     };
     
     static int getSock(string inf, struct ifreq *ifr) {
   
       int sock;
       sock=socket(PF_INET, SOCK_STREAM, 0);
       if (-1==sock) {
        perror("socket() ");
        return 1;
       }
       strncpy(ifr->ifr_name,inf.c_str(),sizeof(ifr->ifr_name)-1);
       ifr->ifr_name[sizeof(ifr->ifr_name)-1]='\0';
       
       return sock;

     };

    
     int ReceiveDataOnly(void* buf,int length){
   

       if (buf==NULL) return -1;
       

       total_sent=0;
       
       switch(protocol) {
       case TCP:
	 if (file_des<0) return -1;
	 while(length>0){
	   nsending = (length>packet_size) ? packet_size:length;
	   nsent = read(file_des,(char*)buf+total_sent,nsending); 
	   if(!nsent) break;
	   length-=nsent;
	   total_sent+=nsent;
	 }
	 break;
       case UDP:
	 if (socketDescriptor<0) return -1;
	 while(length>0){
	   nsending = (length>packet_size) ? packet_size:length;
	   nsent = recvfrom(socketDescriptor,(char*)buf+total_sent,nsending, 0, (struct sockaddr *) &clientAddress, &clientAddress_length); 
	   if(!nsent) break;
	   length-=nsent;
	   total_sent+=nsent;
	 }
	 
	 break;
       default:
	 ;
       }
#ifdef VERY_VERBOSE
       cout << "sent "<< total_sent << " Bytes" << endl; 
#endif
       return total_sent;
       


     }


     int SendDataOnly(void *buf, int length) {
#ifdef VERY_VERBOSE
       cout << "want to send "<< length << " Bytes" << endl; 
#endif
       if (buf==NULL) return -1;
       
       total_sent=0;
       

       switch(protocol) {
       case TCP:
	 if (file_des<0) return -1;
	 while(length>0){
	   nsending = (length>packet_size) ? packet_size:length;
	   nsent = write(file_des,(char*)buf+total_sent,nsending); 
	   if(!nsent) break;
	   length-=nsent;
	   total_sent+=nsent;
	 }
	 break;
       case UDP:
	 if (socketDescriptor<0) return -1;
	 while(length>0){
	   nsending = (length>packet_size) ? packet_size:length;
	   nsent = sendto(socketDescriptor,(char*)buf+total_sent,nsending, 0, (struct sockaddr *) &clientAddress, clientAddress_length); 
	   if(!nsent) break;
	   length-=nsent;
	   total_sent+=nsent;
	 }
	 
	 break;
       default:
	 ;
       }
#ifdef VERY_VERBOSE
       cout << "sent "<< total_sent << " Bytes" << endl; 
#endif
       return total_sent;
       

     }



 protected:

  communicationProtocol protocol; 


  
  int is_a_server;


  int socketDescriptor;
  int file_des;

  int packet_size;

  struct sockaddr_in clientAddress, serverAddress;
  socklen_t clientAddress_length;

 private:
  
  int nsending;
  int nsent;
  int total_sent;
  
       

  // pthread_mutex_t mp;
};
#endif
