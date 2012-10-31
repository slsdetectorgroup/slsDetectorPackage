
//version 1.0, base development, Ian 19/01/09


#include "MySocketTCP.h"
#include <string.h>
#include <iostream>
#include <cstdio>

using namespace std;





int MySocketTCP::SendDataOnly(void* buf,int length){//length in characters


#ifdef VERY_VERBOSE

  cout << "want to send "<< length << " Bytes" << endl; 
#endif
  int nsending;
  int nsent;
  if (file_des<0) return -1;
  int total_sent=0;
  
  while(length>0){
    nsending = (length>send_rec_max_size) ? send_rec_max_size:length;
    nsent = write(file_des,(char*)buf+total_sent,nsending); 
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
  int nreceiving;
  int nreceived;
  while(length>0){
    nreceiving = (length>send_rec_max_size) ? send_rec_max_size:length;
#ifdef VERY_VERBOSE
  cout << "start to receive "<< nreceiving << " Bytes" << endl; 
#endif
    nreceived = read(file_des,(char*)buf+total_received,nreceiving);
#ifdef VERY_VERBOSE
    cout << "received "<< nreceived << " Bytes on fd " << file_des  << endl; 
#endif 
    if(nreceived<0) break;
    length-=nreceived;
#ifdef VERY_VERBOSE
  cout << "length left "<< length << " Bytes" << endl; 
#endif
    total_received+=nreceived;
#ifdef VERY_VERBOSE
  cout << "total "<< total_received << " Bytes" << endl; 
#endif
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

