

#include "communication_funcs.h" 
//#include <sys/socket.h> 
#include <netinet/tcp.h> /* for TCP_NODELAY */ 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <sys/time.h>
char lastClientIP[INET_ADDRSTRLEN];
char thisClientIP[INET_ADDRSTRLEN];
int lockStatus;
int differentClients;

//int socketDescriptor, file_des;
const int send_rec_max_size=SEND_REC_MAX_SIZE;
extern int errno;


char dummyClientIP[INET_ADDRSTRLEN];


fd_set readset, tempset;
int isock=0, maxfd;


int myport=-1;

//struct sockaddr_in address;
//#define VERBOSE


int bindSocket(unsigned short int port_number) {
  int i;

  struct sockaddr_in addressS;
  int socketDescriptor;
  //int file_des;

  //file_des= -1;










  if (myport==port_number)
    return -10;





  socketDescriptor = socket(AF_INET, SOCK_STREAM,0); //tcp

  //socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);



     if (socketDescriptor < 0) {
       printf("Can not create socket\n");
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

	 printf("Can not create socket\n");

	 socketDescriptor=-1;
       } else {
	 if (listen(socketDescriptor, 5)==0) {

	   if (isock==0) {
	     FD_ZERO(&readset);
	   }
	   
	   
	   FD_SET(socketDescriptor, &readset);
	   isock++;
	   maxfd = socketDescriptor;
	   printf ("%d port %d fd %d\n",isock, port_number,socketDescriptor);
	   myport=port_number;
	 } else
	   printf("error on listen");
       }
     }
     


     //int getrlimit(int resource, struct rlimit *rlim);



  return socketDescriptor;

}





int getServerError(int socketDescriptor)
{
  if (socketDescriptor<0) return 1; 
  else return 0;
};


int acceptConnection(int socketDescriptor) {
  

  int j;


  struct sockaddr_in addressC;
  int file_des=-1;
  struct timeval tv;
  int result;


  //socklen_t address_length;
  size_t address_length=sizeof(struct sockaddr_in);
  
  if (socketDescriptor<0)
    return -1;

   memcpy(&tempset, &readset, sizeof(tempset));
   tv.tv_sec = 10000000;
   tv.tv_usec = 0;
   result = select(maxfd + 1, &tempset, NULL, NULL, &tv);

   if (result == 0) {
      printf("select() timed out!\n");
   }  else if (result < 0 && errno != EINTR) {
      printf("Error in select(): %s\n", strerror(errno));
   } else if (result > 0) {
#ifdef VERBOSE
     printf("select returned!\n");
#endif
     for (j=0; j<maxfd+1; j++) {
       if (FD_ISSET(j, &tempset)) {
#ifdef VERBOSE
	 printf("fd %d is set\n",j);
#endif
	 FD_CLR(j, &tempset);


	 if ((file_des = accept(j,(struct sockaddr *) &addressC, &address_length)) < 0) {
	 printf("Error in accept(): %s\n", strerror(errno));
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
	 
	 // should remove descriptor

	 socketDescriptor=-1;
	 }  else {
	   
	   inet_ntop(AF_INET, &(addressC.sin_addr), dummyClientIP, INET_ADDRSTRLEN);
#ifdef VERBOSE
	   printf("connection accepted %d\n",file_des);
#endif
	   FD_SET(file_des, &readset);
	   maxfd = (maxfd < file_des)?file_des:maxfd;
	 }

       }
     }
   }
 


   return file_des;
}







void closeConnection(int file_des) {
#ifdef VERY_VERBOSE
#endif 
   if(file_des>=0)
     close(file_des);
  FD_CLR(file_des, &readset);
}

void exitServer(int socketDescriptor) {
  if (socketDescriptor>=0)
    close(socketDescriptor);
#ifdef VERY_VERBOSE
  printf("Closing server\n");
#endif
  FD_CLR(socketDescriptor, &readset);
  socketDescriptor=-1;
  isock--;
}




void swapData(void* val,int length,intType itype){
	int i;
	int16_t* c= (int16_t*)val;
	int32_t* a= (int32_t*)val;
	int64_t* b= (int64_t*)val;
	for(i=0; length > 0; i++){
		switch(itype){
		case INT16:
			c[i] = ((c[i] & 0x00FF) << 8) | ((c[i] & 0xFF00) >> 8);
			length -= sizeof(int16_t);
			break;
		case INT32:
			a[i]=((a[i] << 8) & 0xFF00FF00) | ((a[i] >> 8) & 0xFF00FF );
			a[i]=(a[i] << 16) 			  | ((a[i] >> 16) & 0xFFFF);
			length -= sizeof(int32_t);
			break;
		case INT64:
			b[i] = ((b[i] << 8) & 0xFF00FF00FF00FF00ULL ) | ((b[i] >> 8) & 0x00FF00FF00FF00FFULL );
			b[i] = ((b[i] << 16) & 0xFFFF0000FFFF0000ULL ) | ((b[i] >> 16) & 0x0000FFFF0000FFFFULL );
			b[i] =  (b[i] << 32) | ((b[i] >> 32) & 0xFFFFFFFFULL);
			length -= sizeof(int64_t);
			break;
		default:
			length = 0;
			break;
		}
	}
}

int sendData(int file_des, void* buf,int length, intType itype){
#ifndef PCCOMPILE
#ifdef EIGERD
	swapData(buf, length, itype);
#endif
#endif
	return sendDataOnly(file_des, buf, length);
}


int receiveData(int file_des, void* buf,int length, intType itype){
	int ret = receiveDataOnly(file_des, buf, length);
#ifndef PCCOMPILE
#ifdef EIGERD
	swapData(buf, length, itype);
#endif
#endif
	return ret;
}


 int sendDataOnly(int file_des, void* buf,int length) {


  return   write(file_des, buf, length);  
}


 int receiveDataOnly(int file_des, void* buf,int length) {

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

  if (total_received>0)
    strcpy(thisClientIP,dummyClientIP);
  
  //if (strcmp(lastClientIP,"none")==0)
  //strcpy(lastClientIP,thisClientIP);
  
  if (strcmp(lastClientIP,thisClientIP))
    differentClients=1;
  else
    differentClients=0;

  return total_received;
}















int sendChannel(int file_des, sls_detector_channel *myChan) {
  int ts=0;
  //sendDataOnly(file_des,myChan, sizeof(sls_detector_channel));
  ts+=sendData(file_des,&(myChan->chan),sizeof(myChan->chan),INT32);
  ts+=sendData(file_des,&(myChan->chip),sizeof(myChan->chip),INT32);
  ts+=sendData(file_des,&(myChan->module),sizeof(myChan->module),INT32);
  ts+=sendData(file_des,&(myChan->reg),sizeof(myChan->reg),INT64);
  return  ts;
}

int sendChip(int file_des, sls_detector_chip *myChip) {
  int ts=0;
  //ts+=sendDataOnly(file_des,myChip,sizeof(sls_detector_chip));
  ts+=sendData(file_des,&(myChip->chip),sizeof(myChip->chip),INT32);
  ts+=sendData(file_des,&(myChip->module),sizeof(myChip->module),INT32);
  ts+=sendData(file_des,&(myChip->nchan),sizeof(myChip->nchan),INT32);
  ts+=sendData(file_des,&(myChip->reg),sizeof(myChip->reg),INT32);
  ts+=sendData(file_des,(myChip->chanregs),sizeof(myChip->chanregs),INT32);
  ts+=sendData(file_des,myChip->chanregs,myChip->nchan*sizeof(int),INT32);
  return ts;			  
}


int sendModule(int file_des, sls_detector_module *myMod) {
	return sendModuleGeneral(file_des, myMod, 1);
}


int sendModuleGeneral(int file_des, sls_detector_module *myMod, int sendAll) {
  int ts=0;
#ifdef VERBOSE
  int idac;
#endif
  int nChips=myMod->nchip;
  int nChans=myMod->nchan;
  int nAdcs=myMod->nadc;
  int nDacs=myMod->ndac;
  //ts+= sendDataOnly(file_des,myMod,sizeof(sls_detector_module));
  ts+=sendData(file_des,&(myMod->module),sizeof(myMod->module),INT32);
  ts+=sendData(file_des,&(myMod->serialnumber),sizeof(myMod->serialnumber),INT32);
  ts+=sendData(file_des,&(myMod->nchan),sizeof(myMod->nchan),INT32);
  ts+=sendData(file_des,&(myMod->nchip),sizeof(myMod->nchip),INT32);
  ts+=sendData(file_des,&(myMod->ndac),sizeof(myMod->ndac),INT32);
  ts+=sendData(file_des,&(myMod->nadc),sizeof(myMod->nadc),INT32);
  ts+=sendData(file_des,&(myMod->reg),sizeof(myMod->reg),INT32);
  ts+=sendData(file_des,myMod->dacs,sizeof(myMod->ndac),OTHER);
  ts+=sendData(file_des,myMod->adcs,sizeof(myMod->nadc),OTHER);
  /*some detectors dont require sending all trimbits etc.*/
  if(sendAll){
    ts+=sendData(file_des,myMod->chipregs,sizeof(myMod->nchip),OTHER);
    ts+=sendData(file_des,myMod->chanregs,sizeof(myMod->nchan),OTHER);
  }
  ts+=sendData(file_des,&(myMod->gain), sizeof(myMod->gain),OTHER);
  ts+=sendData(file_des,&(myMod->offset), sizeof(myMod->offset),OTHER);

#ifdef VERBOSE
  printf("module %d of size %d sent\n",myMod->module, ts);
#endif
  ts+= sendData(file_des,myMod->dacs,sizeof(dacs_t)*nDacs,INT32);
#ifdef VERBOSE
  printf("dacs %d of size %d sent\n",myMod->module, ts);
  for (idac=0; idac< nDacs; idac++) 
    printf("dac %d is %d\n",idac,(int)myMod->dacs[idac]);
#endif
  ts+= sendData(file_des,myMod->adcs,sizeof(dacs_t)*nAdcs,INT32);
#ifdef VERBOSE
  printf("adcs %d of size %d sent\n",myMod->module, ts);
#endif

  /*some detectors dont require sending all trimbits etc.*/
  if(sendAll){
    ts+=sendData(file_des,myMod->chipregs,sizeof(int)*nChips,INT32);
#ifdef VERBOSE
    printf("chips %d of size %d sent\n",myMod->module, ts);
#endif
    ts+=sendData(file_des,myMod->chanregs,sizeof(int)*nChans,INT32);
#ifdef VERBOSE
    printf("chans %d of size %d sent - %d\n",myMod->module, ts, myMod->nchan);
#endif
  }

#ifdef VERBOSE
  printf("module %d of size %d sent register %x\n",myMod->module, ts, myMod->reg);
#endif
  return ts;
}

int receiveChannel(int file_des, sls_detector_channel *myChan) {
  int ts=0;
  //receiveDataOnly(file_des,myChan,sizeof(sls_detector_channel));
  ts+=receiveData(file_des,&(myChan->chan),sizeof(myChan->chan),INT32);
  ts+=receiveData(file_des,&(myChan->chip),sizeof(myChan->chip),INT32);
  ts+=receiveData(file_des,&(myChan->module),sizeof(myChan->module),INT32);
  ts+=receiveData(file_des,&(myChan->reg),sizeof(myChan->reg),INT32);
  return ts;
}

int receiveChip(int file_des, sls_detector_chip* myChip) {

  int *ptr=myChip->chanregs;
  int ts=0;
  int nChans, nchanold=myChip->nchan, chdiff;

  //ts+= receiveDataOnly(file_des,myChip,sizeof(sls_detector_chip));
  ts+=receiveData(file_des,&(myChip->chip),sizeof(myChip->chip),INT32);
  ts+=receiveData(file_des,&(myChip->module),sizeof(myChip->module),INT32);
  ts+=receiveData(file_des,&(myChip->nchan),sizeof(myChip->nchan),INT32);
  ts+=receiveData(file_des,&(myChip->reg),sizeof(myChip->reg),INT32);
  ts+=receiveData(file_des,(myChip->chanregs),sizeof(myChip->chanregs),INT32);

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
    ts+=receiveData(file_des,myChip->chanregs, sizeof(int)*nChans,INT32);
  else {
    ptr=(int*)malloc(chdiff*sizeof(int));
    myChip->nchan=nchanold;
    ts+=receiveData(file_des,myChip->chanregs, sizeof(int)*nchanold,INT32);
    ts+=receiveData(file_des,ptr, sizeof(int)*chdiff,INT32);
    free(ptr);
    return FAIL;
  }

#ifdef VERBOSE
  printf("chip's channels received\n");
#endif
  return ts;
}


int  receiveModule(int file_des, sls_detector_module* myMod) {
	return receiveModuleGeneral(file_des,myMod,1);
}

int  receiveModuleGeneral(int file_des, sls_detector_module* myMod, int receiveAll) {
  int ts=0;
  dacs_t *dacptr=myMod->dacs;
  dacs_t *adcptr=myMod->adcs;
  int *chipptr=myMod->chipregs, *chanptr=myMod->chanregs;
  int nChips, nchipold=myMod->nchip, nchipdiff;
  int nChans, nchanold=myMod->nchan, nchandiff;
  int nDacs, ndold=myMod->ndac, ndacdiff;
  int nAdcs, naold=myMod->nadc, nadcdiff;
#ifdef VERBOSE
  int id=0;
#endif
  // ts+= receiveDataOnly(file_des,myMod,sizeof(sls_detector_module));
  ts+=receiveData(file_des,&(myMod->module),sizeof(myMod->module),INT32);
  ts+=receiveData(file_des,&(myMod->serialnumber),sizeof(myMod->serialnumber),INT32);
  ts+=receiveData(file_des,&(myMod->nchan),sizeof(myMod->nchan),INT32);
  ts+=receiveData(file_des,&(myMod->nchip),sizeof(myMod->nchip),INT32);
  ts+=receiveData(file_des,&(myMod->ndac),sizeof(myMod->ndac),INT32);
  ts+=receiveData(file_des,&(myMod->nadc),sizeof(myMod->nadc),INT32);
  ts+=receiveData(file_des,&(myMod->reg),sizeof(myMod->reg),INT32);
  ts+=receiveData(file_des,myMod->dacs,sizeof(myMod->ndac),INT32);
  ts+=receiveData(file_des,myMod->adcs,sizeof(myMod->nadc),INT32);
  /*some detectors dont require sending all trimbits etc.*/
  if(receiveAll){
    ts+=receiveData(file_des,myMod->chipregs,sizeof(myMod->nchip),INT32);
    ts+=receiveData(file_des,myMod->chanregs,sizeof(myMod->nchan),INT32);
  }
  ts+=receiveData(file_des,&(myMod->gain), sizeof(myMod->gain),OTHER);
  ts+=receiveData(file_des,&(myMod->offset), sizeof(myMod->offset),OTHER);

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
    ts+=receiveData(file_des,myMod->dacs, sizeof(dacs_t)*nDacs,INT32);
#ifdef VERBOSE
    printf("dacs received\n");
    int id;
    for (id=0; id<nDacs; id++)
      printf("dac %d val %d\n",id,  (int)myMod->dacs[id]);


#endif
  } else {
    dacptr=(dacs_t*)malloc(ndacdiff*sizeof(dacs_t));
    myMod->ndac=ndold;
    ts+=receiveData(file_des,myMod->dacs, sizeof(dacs_t)*ndold,INT32);
    ts+=receiveData(file_des,dacptr, sizeof(dacs_t)*ndacdiff,INT32);
    free(dacptr);
    return FAIL;
  }

  if (nadcdiff<=0) {
    ts+=receiveData(file_des,myMod->adcs, sizeof(dacs_t)*nAdcs,INT32);
#ifdef VERBOSE
    printf("adcs received\n");
#endif
  } else {
    adcptr=(dacs_t*)malloc(nadcdiff*sizeof(dacs_t));
    myMod->nadc=naold;
    ts+=receiveData(file_des,myMod->adcs, sizeof(dacs_t)*naold,INT32);
    ts+=receiveData(file_des,adcptr, sizeof(dacs_t)*nadcdiff,INT32);
    free(adcptr);
    return FAIL;
  }


  /*some detectors dont require sending all trimbits etc.*/
  if(receiveAll){

    if (nchipdiff<=0) {
      ts+=receiveData(file_des,myMod->chipregs, sizeof(int)*nChips,INT32);
#ifdef VERBOSE
      printf("chips received\n");
#endif
    } else {
      chipptr=(int*)malloc(nchipdiff*sizeof(int));
      myMod->nchip=nchipold;
      ts+=receiveData(file_des,myMod->chipregs, sizeof(int)*nchipold,INT32);
      ts+=receiveData(file_des,chipptr, sizeof(int)*nchipdiff,INT32);
      free(chipptr);
      return FAIL;
    }

    if (nchandiff<=0) {
      ts+=receiveData(file_des,myMod->chanregs, sizeof(int)*nChans,INT32);
#ifdef VERBOSE
      printf("chans received\n");
#endif
    } else {
      chanptr=(int*)malloc(nchandiff*sizeof(int));
      myMod->nchan=nchanold;
      ts+=receiveData(file_des,myMod->chanregs, sizeof(int)*nchanold,INT32);
      ts+=receiveData(file_des,chanptr, sizeof(int)*nchandiff,INT32);
      free(chanptr);
      return FAIL;
    }
  }
#ifdef VERBOSE
  printf("received module %d of size %d register %x\n",myMod->module,ts,myMod->reg);
#endif

  return ts;
}
