/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "dummyUDPInterface.h"
#include "slsReceiverTCPIPInterface.h"

#include <iostream>
#include <string.h>


using namespace std;


int main(int argc, char *argv[]) {
  int success; 
  int tcpip_port_no;
  bool bottom = false; 
  cout << "CCCCCC" << endl;
  dummyUDPInterface *udp=new dummyUDPInterface();
//   slsReceiverTCPIPInterface *tcpipInterface = new slsReceiverTCPIPInterface(success, udp, tcpip_port_no, bottom);

  
  
  
// 	if(tcpipInterface->start() == slsReceiverDefs::OK){
// 		cout << "DONE!" << endl;
// 		string str;
// 		cin>>str;
// 		//wait and look for an exit keyword
// 		while(str.find("exit") == string::npos)
// 			cin>>str;
// 		//stop tcp server thread, stop udp socket
// 		tcpipInterface->stop();
// 	}

//   if (tcpipInterface)
//     delete tcpipInterface;

  udp->startReceiver();
   if(udp)
     delete udp;
  return 0;



}

