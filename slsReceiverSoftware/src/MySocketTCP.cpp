
//version 1.0, base development, Ian 19/01/09


#include "MySocketTCP.h"
#include <string.h>
#include <iostream>
#include <cstdio>

using namespace std;









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

