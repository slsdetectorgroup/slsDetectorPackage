
//version 1.0, base development ij 19/01/09

#include <iostream>
#include "MySocketTCP.h"

using namespace std;

int main(){

  char data[50000];
  int  length=50000;

  unsigned short int portnum = 1952;
  MySocketTCP* sock = new MySocketTCP(portnum);

    cout<<"\tReceived :"<<sock->ReceiveDataAndKeepConnection(data,23000)<<endl;
    cout<<"\tReceived :"<<sock->ReceiveData(data,32200)<<endl;
    cout<<"\tReceived :"<<sock->ReceiveData(data,33300)<<endl;


    
    cout<<"\tReceived :"<<sock->ReceiveData(data,30000)<<endl;


    cout<<"\tReceived :"<<sock->ReceiveData(data,3222)<<endl;

  delete sock;

  return 0;
}
