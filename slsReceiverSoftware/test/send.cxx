
//version 1.0, base development ij 19/01/09
 
#include <iostream>
#include "MySocketTCP.h"

using namespace std;

int main(int argc, char *argv[]){

  if(argc!=2){
    cout<<"Usage: send ip_addess/hostName"<<endl;
    return 1;
  }
  cout<<"rec function must be first called."<<endl;

  char ip_address[200];
    sprintf(ip_address,"%s",argv[1]);
  unsigned short int portnum = 1952;


  char data[50000];
  int  length=50000;

  MySocketTCP* sock = new MySocketTCP(ip_address,portnum);

    cout<<"\tSending :"<<sock->SendDataAndKeepConnection(data,2000)<<endl;
    cout<<"\tSending :"<<sock->SendData(data,2200)<<endl;
    cout<<"\tSending :"<<sock->SendData(data,1200)<<endl;


    cout<<"\tSending :"<<sock->SendData(data,25000)<<endl;

    cout<<"\tSending :"<<sock->SendData(data,222)<<endl;

  delete sock;


  return 0;
}
