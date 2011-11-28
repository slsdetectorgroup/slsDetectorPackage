

#include "communication_funcs.h" 
#include <sys/socket.h> 
#include <netinet/tcp.h> /* for TCP_NODELAY */ 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>



//int socketDescriptor, file_des;
int socketDescriptor, file_des;
const int send_rec_max_size=SEND_REC_MAX_SIZE;
extern int errno;

//struct sockaddr_in address;
//#define VERBOSE


int bindSocket(unsigned short int port_number) {
  int i;

  struct sockaddr_in addressS;


  file_des= -1;
  socketDescriptor = socket(AF_INET, SOCK_STREAM,0); //tcp

  //socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);



     if (socketDescriptor < 0) {
       printf("Cannot create socket..socketdescript less than 0\n");
     } else {
  
       i = 1;					
       setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)); 
       //    setsockopt(socketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char *) &i, sizeof(i));  
       //  TCP_CORK 

  // Set some fields in the serverAddress structure.  
       addressS.sin_family = AF_INET;
       addressS.sin_addr.s_addr = htonl(INADDR_ANY);
       addressS.sin_port = htons(port_number);
    
       //    memset(&address.sin_addr, 0, sizeof(address.sin_addr));


       if(bind(socketDescriptor,(struct sockaddr *) &addressS,sizeof(addressS))<0){

	 printf("Cannot create socket..did not bind\n");

	 socketDescriptor=-1;
       } else {
	 listen(socketDescriptor, 5);
       }
     }
 


     


     //int getrlimit(int resource, struct rlimit *rlim);






  return socketDescriptor;

}


/*
only client funcs
*/
/*
#ifndef C_ONLY

MySocketTCP::MySocketTCP(const char* const host_ip_or_name, unsigned short int const port_number):
  last_keep_connection_open_action_was_a_send(0), file_des(-1), send_rec_max_size(SEND_REC_MAX_SIZE), is_a_server(0), portno(DEFAULT_PORTNO), socketDescriptor(-1)
{ // sender (client): where to? ip 
  //is_a_server = 0;
  // SetupParameters();
  strcpy(hostname,host_ip_or_name);
  portno=port_number;
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


int MySocketTCP::getHostname(char *name) {
  if (is_a_server==0) {
    strcpy(name,hostname);
  }
  return is_a_server;
};
#endif
*/




int getServerError()
{
  if (socketDescriptor<0) return 1; 
  else return 0;
};


int acceptConnection() {
  struct sockaddr_in addressC;
  //socklen_t address_length;
  size_t address_length=sizeof(struct sockaddr_in);
  
  if(file_des>0) return file_des;


  //#ifndef C_ONLY
  //  if(is_a_server){ //server; the server will wait for the clients connection
  //#endif


    if (socketDescriptor>0) {
       if ((file_des = accept(socketDescriptor,(struct sockaddr *) &addressC, &address_length)) < 0) {



	printf("Error: with server accept, connection refused %d\n", errno);


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
#ifdef VERBOSE


struct sockaddr_in6 sa;
char str[INET6_ADDRSTRLEN];

// store this IP address in sa:
inet_pton(AF_INET6, "2001:db8:8714:3a90::12", &(sa.sin6_addr));

// now get it back and print it
inet_ntop(AF_INET6, &(sa.sin6_addr), str, INET6_ADDRSTRLEN);

//printf("%s\n\n", str); // prints "2001:db8:8714:3a90::12"


      printf("client connected %d\n", file_des);
      printf("addressC family %d port %d addr %s\n",addressC.sin_family,addressC.sin_port,inet_ntoa(addressC.sin_addr));//edited
#endif

      //strcpy(lastClientIP,inet_ntoa(addressC.sin_addr)); //In case you want to lock the server...


      // struct sockaddr_in
      //{
      //short int sin_family; /* Famyly of the address*/
      //unsigned short int sin_port; /* Port */
      //struct in_addr sin_addr; /* Network address */
      //unsigned char sin_zero[8]; /* Same size of struct sockaddr */
      //};
    }
 

  return file_des;
}







void closeConnection() {
  //fflush(stdout);
  //printf("Closing file_des %d\n", file_des);
  //sleep(1);
#ifdef VERY_VERBOSE
#endif 
  if(file_des>=0)
    close(file_des);
  file_des=-1;
}

void exitServer() {
  if (socketDescriptor>=0)
    close(socketDescriptor);
#ifdef VERY_VERBOSE
  printf("Closing server\n");
#endif
  socketDescriptor=-1;
}




/* client close conenction */
/*
#ifndef C_ONLY
void MySocketTCP::Disconnect(){
  
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

}
#endif
*/


int sendDataOnly(void* buf,int length) {
  /*
 int total_sent=0;
 int nsending;
 int nsent;
 

#ifdef VERY_VERBOSE
  printf("want to send %d Bytes\n", length);
#endif
  if (file_des<0) return -1;
 
  while(length>0){
    nsending = (length>send_rec_max_size) ? send_rec_max_size:length;
    nsent = write(file_des,(char*)buf+total_sent,nsending); 
    if(!nsent) break;
    length-=nsent;
    total_sent+=nsent;
    //    cout<<"nsent: "<<nsent<<endl;
  }
  
  */

  return   write(file_des, buf, length);  
}


 int receiveDataOnly(void* buf,int length) {

  int total_received=0;
  int nreceiving;
  int nreceived;
  if (file_des<0) return -1;
#ifdef VERY_VERBOSE
  printf("want to receive %d Bytes\n", length); 
#endif

  while(length>0){
    nreceiving = (length>send_rec_max_size) ? send_rec_max_size:length;

#ifdef VERY_VERBOSE
  printf("want to receive %d Bytes\n", nreceiving); 
#endif
    nreceived = read(file_des,(char*)buf+total_received,nreceiving); 
#ifdef VERY_VERBOSE
  printf("read %d \n", nreceived); 
#endif
  if(!nreceived) break;
  //  if(nreceived<0) break;
    length-=nreceived;
    total_received+=nreceived;
    //    cout<<"nrec: "<<nreceived<<" waiting for ("<<length<<")"<<endl;
  }
 
#ifdef VERY_VERBOSE
  printf("received %d Bytes\n", total_received); 
#endif
  
  return total_received;
}















int sendChannel(sls_detector_channel *myChan) {
  return  sendDataOnly(myChan, sizeof(sls_detector_channel));
}

int sendChip(sls_detector_chip *myChip) {
  int ts=0;
  int nChans=myChip->nchan;
  ts+=sendDataOnly(myChip,sizeof(sls_detector_chip));
  ts+=sendDataOnly(myChip->chanregs,nChans*sizeof(int));
  return ts;			  
}

int sendModule(sls_detector_module *myMod) {
  int ts=0;
  int idac;
  int nChips=myMod->nchip;
  int nChans=myMod->nchan;
  int nAdcs=myMod->nadc;
  int nDacs=myMod->ndac;
  ts+= sendDataOnly(myMod,sizeof(sls_detector_module));
#ifdef VERBOSE
  printf("module %d of size %d sent\n",myMod->module, ts);
#endif
  ts+= sendDataOnly(myMod->dacs,sizeof(float)*nDacs);
#ifdef VERBOSE
  printf("dacs %d of size %d sent\n",myMod->module, ts);
  for (idac=0; idac< nDacs; idac++) 
    printf("dac %d is %d\n",idac,myMod->dacs[idac]);
#endif
  ts+= sendDataOnly(myMod->adcs,sizeof(float)*nAdcs);
#ifdef VERBOSE
  printf("adcs %d of size %d sent\n",myMod->module, ts);
#endif
  ts+=sendDataOnly(myMod->chipregs,sizeof(int)*nChips);
#ifdef VERBOSE
  printf("chips %d of size %d sent\n",myMod->module, ts);
#endif
  ts+=sendDataOnly(myMod->chanregs,sizeof(int)*nChans);
#ifdef VERBOSE
  printf("chans %d of size %d sent - %d\n",myMod->module, ts, myMod->nchan);
#endif
#ifdef VERBOSE
  printf("module %d of size %d sent register %x\n",myMod->module, ts, myMod->reg);
#endif
  return ts;
}

int receiveChannel(sls_detector_channel *myChan) {
  return  receiveDataOnly(myChan,sizeof(sls_detector_channel));
}

int receiveChip(sls_detector_chip* myChip) {

  int *ptr=myChip->chanregs;
  int ts=0;
  int nChans, nchanold=myChip->nchan, chdiff;

  ts+= receiveDataOnly(myChip,sizeof(sls_detector_chip));


  myChip->chanregs=ptr;
  nChans=myChip->nchan;
  chdiff=nChans-nchanold;
  if (nchanold!=nChans) {
    printf("wrong number of channels received!\n");
  } 
  

#ifdef VERBOSE
  printf("chip structure received\n");
  printf("now receiving %d channels\n", nChans);
#endif

  if (chdiff<=0)
    ts+=receiveDataOnly(myChip->chanregs, sizeof(int)*nChans);
  else {
    ptr=malloc(chdiff*sizeof(int));
    myChip->nchan=nchanold;
    ts+=receiveDataOnly(myChip->chanregs, sizeof(int)*nchanold);
    ts+=receiveDataOnly(ptr, sizeof(int)*chdiff);
    free(ptr);
    return FAIL;
  }

#ifdef VERBOSE
  printf("chip's channels received\n");
#endif
  return ts;
}

int  receiveModule(sls_detector_module* myMod) {

 
  float *dacptr=myMod->dacs;
  float *adcptr=myMod->adcs;
  int *chipptr=myMod->chipregs, *chanptr=myMod->chanregs;
  int ts=0;
  int nChips, nchipold=myMod->nchip, nchipdiff;
  int nChans, nchanold=myMod->nchan, nchandiff;
  int nDacs, ndold=myMod->ndac, ndacdiff;
  int nAdcs, naold=myMod->nadc, nadcdiff;


  ts+= receiveDataOnly(myMod,sizeof(sls_detector_module));

  myMod->dacs=dacptr;
  myMod->adcs=adcptr;
  myMod->chipregs=chipptr;
  myMod->chanregs=chanptr;
    
  nChips=myMod->nchip;
  nchipdiff=nChips-nchipold;
  if (nchipold!=nChips) {
    printf("received wrong number of chips\n");
  } 
#ifdef VERBOSE 
  else
    printf("received %d chips\n",nChips);
#endif

  nChans=myMod->nchan;
  nchandiff=nChans-nchanold;
  if (nchanold!=nChans) {
    printf("received wrong number of channels\n");
  } 
#ifdef VERBOSE 
  else
    printf("received %d chans\n",nChans);
#endif


  nDacs=myMod->ndac;
  ndacdiff=nDacs-ndold;
  if (ndold!=nDacs) {
    printf("received wrong number of dacs\n");
  } 
#ifdef VERBOSE 
  else
    printf("received %d dacs\n",nDacs);
#endif
  
  nAdcs=myMod->nadc;
  nadcdiff=nAdcs-naold;
  if (naold!=nAdcs) {
    printf("received wrong number of adcs\n");
  }
#ifdef VERBOSE 
  else
    printf("received %d adcs\n",nAdcs);
#endif
  if (ndacdiff<=0) {
    ts+=receiveDataOnly(myMod->dacs, sizeof(float)*nDacs);
#ifdef VERBOSE 
    printf("dacs received\n");
#endif
  } else {
    dacptr=malloc(ndacdiff*sizeof(float));
    myMod->ndac=ndold;
    ts+=receiveDataOnly(myMod->dacs, sizeof(float)*ndold);
    ts+=receiveDataOnly(dacptr, sizeof(float)*ndacdiff);
    free(dacptr);
    return FAIL;
  }

  if (nadcdiff<=0) {
    ts+=receiveDataOnly(myMod->adcs, sizeof(float)*nAdcs);
#ifdef VERBOSE 
    printf("adcs received\n");
#endif
  } else {
    adcptr=malloc(nadcdiff*sizeof(float));
    myMod->nadc=naold;
    ts+=receiveDataOnly(myMod->adcs, sizeof(float)*naold);
    ts+=receiveDataOnly(adcptr, sizeof(float)*nadcdiff);
    free(adcptr);
    return FAIL;
  }

  if (nchipdiff<=0) {
    ts+=receiveDataOnly(myMod->chipregs, sizeof(int)*nChips);
#ifdef VERBOSE 
    printf("chips received\n");
#endif
  } else {
    chipptr=malloc(nchipdiff*sizeof(int));
    myMod->nchip=nchipold;
    ts+=receiveDataOnly(myMod->chipregs, sizeof(int)*nchipold);
    ts+=receiveDataOnly(chipptr, sizeof(int)*nchipdiff);
    free(chipptr);
    return FAIL;
  }

  if (nchandiff<=0) {
    ts+=receiveDataOnly(myMod->chanregs, sizeof(int)*nChans);
#ifdef VERBOSE 
    printf("chans received\n");
#endif
  } else {
    chanptr=malloc(nchandiff*sizeof(int));
    myMod->nchan=nchanold;
    ts+=receiveDataOnly(myMod->chanregs, sizeof(int)*nchanold);
    ts+=receiveDataOnly(chanptr, sizeof(int)*nchandiff);
    free(chanptr);
    return FAIL;
  }
#ifdef VERBOSE
  printf("received module %d of size %d register %x\n",myMod->module,ts,myMod->reg);
#endif
  return ts;
}
