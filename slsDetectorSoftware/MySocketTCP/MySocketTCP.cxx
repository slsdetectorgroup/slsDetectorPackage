
//version 1.0, base development, Ian 19/01/09


#include "MySocketTCP.h"
#include <string.h>
#include <iostream>
#include <math.h>

using namespace std;

MySocketTCP::~MySocketTCP(){
  Disconnect();
  if (socketDescriptor >= 0){
    close(socketDescriptor);
  }
  file_des=-1;
}


MySocketTCP::MySocketTCP(unsigned short int const port_number): last_keep_connection_open_action_was_a_send(0), file_des(-1), send_rec_max_size(SEND_REC_MAX_SIZE), is_a_server(1), portno(DEFAULT_PORTNO), socketDescriptor(-1)
{ // receiver (server) local no need for ip 
  //is_a_server = 1;

  portno=port_number;
  strcpy(hostname,"localhost");
  //  SetupParameters();

  socketDescriptor = socket(AF_INET, SOCK_STREAM,0); //tcp

  if (socketDescriptor < 0) {
    cerr << "Can not create socket "<<endl;
  } else {
  
  // Set some fields in the serverAddress structure.  
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port_number);
    
    if(bind(socketDescriptor,(struct sockaddr *) &serverAddress,sizeof(serverAddress))<0){
      cerr << "Can not bind socket "<<endl;
    socketDescriptor=-1;
    } else {
      listen(socketDescriptor, 5);
    }
  }
 }



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





int MySocketTCP::Connect(){

  if(file_des>0) return file_des;


  if(is_a_server){ //server; the server will wait for the clients connection


    if (socketDescriptor>0) {
      if ((file_des = accept(socketDescriptor,(struct sockaddr *) &clientAddress, &clientAddress_length)) < 0) {

	cerr << "Error: with server accept, connection refused"<<endl;

	socketDescriptor=-1;
      }
    }
    file_des = socketDescriptor;
  } else {  
    socketDescriptor = socket(AF_INET, SOCK_STREAM,0);  //tcp
    
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

int MySocketTCP::SendDataOnly(void* buf,int length){//length in characters


#ifdef VERY_VERBOSE

  cout << "want to send "<< length << " Bytes" << endl; 
#endif

  if (file_des<0) return -1;
  int total_sent=0;
  while(length>0){
    int nsending = (length>send_rec_max_size) ? send_rec_max_size:length;
    int nsent = write(file_des,(char*)buf+total_sent,nsending); 
    if(!nsent) break;
    length-=nsent;
    total_sent+=nsent;
    //    cout<<"nsent: "<<nsent<<endl;
  }

#ifdef VERY_VERBOSE
  cout << "sent "<< total_sent << " Bytes" << endl; 
#endif
  return total_sent;
}




int MySocketTCP::SendData(void* buf,int length){//length in characters
  int ndata = SendDataAndKeepConnection(buf,length);
  Disconnect();
  return ndata;
}

int MySocketTCP::SendDataAndKeepConnection(void* buf,int length){//length in characters
  if(last_keep_connection_open_action_was_a_send) Disconnect(); //to keep a structured data flow;

  Connect();
  int total_sent=SendDataOnly(buf,length);
  last_keep_connection_open_action_was_a_send=1;
  return total_sent;
}


int MySocketTCP::ReceiveDataOnly(void* buf,int length){//length in characters
  int total_received=0;
  if (file_des<0) return -1;
#ifdef VERY_VERBOSE
  cout << "want to receive "<< length << " Bytes" << endl; 
#endif

  while(length>0){
    int nreceiving = (length>send_rec_max_size) ? send_rec_max_size:length;
    int nreceived = read(file_des,(char*)buf+total_received,nreceiving); 
    if(!nreceived) break;
    length-=nreceived;
    total_received+=nreceived;
    //    cout<<"nrec: "<<nreceived<<" waiting for ("<<length<<")"<<endl;
  }
 
#ifdef VERY_VERBOSE
  cout << "received "<< total_received << " Bytes" << endl;

#endif
  
  return total_received;
}




int MySocketTCP::ReceiveData(void* buf,int length){//length in characters
  int ndata = ReceiveDataAndKeepConnection(buf,length);
  Disconnect();
  return ndata;
}

int MySocketTCP::ReceiveDataAndKeepConnection(void* buf,int length){//length in characters
  if(!last_keep_connection_open_action_was_a_send) Disconnect(); //to a keep structured data flow;

  Connect();
  //  should preform two reads one to receive incomming char count
  int total_received=ReceiveDataOnly(buf,length);
  last_keep_connection_open_action_was_a_send=0;
  return total_received;
}

