
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>



#include "slsDetectorServer_defs.h" //include port number

struct sockaddr_in eiger_back_socket_addr;
int  eiger_back_max_message_length = 1024;
char eiger_back_message[1024];
int  eiger_back_message_length = 0;
int  eiger_back_ret_val=0;


int bit_mode=0;
int ten_giga=0;

int EigerBackInit(){
  static int passed = 0;

  if(!passed){
    struct hostent *dst_host;
    if((dst_host = gethostbyname("localhost")) == NULL){  //or look into getaddrinfo(3)
      fprintf(stderr,"ERROR, no such host\n");
      return 0;
    }else{
      //struct sockaddr_in eiger_back_socket_addr;
      int port = BEB_PORT;
      bzero((char *) &eiger_back_socket_addr, sizeof(eiger_back_socket_addr));
      eiger_back_socket_addr.sin_family = AF_INET;
      bcopy((char *)dst_host->h_addr,(char *)&eiger_back_socket_addr.sin_addr.s_addr,dst_host->h_length);
      eiger_back_socket_addr.sin_port = htons(port);
      passed = 1;
    }
  }

  return passed;
}


int EigerBackSendCMD(){
  if(!EigerBackInit()||eiger_back_message_length<=0) return 0;

  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd<0){
    fprintf(stderr,"ERROR opening socket\n");
    return 0;
  }

  if(connect(sockfd,(struct sockaddr *) &eiger_back_socket_addr,sizeof(eiger_back_socket_addr))<0){
    fprintf(stderr,"ERROR connecting\n");
    return 0;
  }

  int n = write(sockfd,eiger_back_message,eiger_back_message_length);
  int ret_length = read(sockfd,eiger_back_message,eiger_back_max_message_length);

  close(sockfd);

  if(n<0||ret_length<0) return 0;


  //fprintf(stdout,"%s\n",eiger_back_message);
  if(eiger_back_ret_val>0){
    int i=0;
    eiger_back_message[1]='\0';
    if(atoi(eiger_back_message)!=0) return 0;

    for(i=2;i<ret_length;i++){
      if(eiger_back_message[i] == ' '){
	//fprintf(stdout," in : %d \n",i);
	eiger_back_message[i]='\0';
	break;
      }
    }
    eiger_back_ret_val = atoi(&eiger_back_message[2]);
    //fprintf(stdout," the \"%s\" %d\n",&eiger_back_message[2],eiger_back_ret_val);
  }

  eiger_back_message_length = 0;

  return 1;
}


int EigerSetBitMode(int i){
  eiger_back_ret_val=0;
  eiger_back_message_length = sprintf(eiger_back_message,"setbitmode %d",i);
  bit_mode = i;
  return EigerBackSendCMD();
}

int EigerGetBitMode(){
	return bit_mode;
}

//SetupTableEntry <beb_number> <1GbE(0) or 10GbE(1)> <dst_number> <src_mac> <src_ip> <src_port> <dst_mac> <dst_ip> <dst_port>
int EigerSetupTableEntryLeft(int ipad, long long int macad, long long int detectormacadd, int detipad, int udpport){
	char src_mac[50], src_ip[50],dst_mac[50], dst_ip[50];

	int src_port = 0xE185;
	int dst_port = udpport;
	sprintf(src_ip,"%d.%d.%d.%d",(detipad>>24)&0xff,(detipad>>16)&0xff,(detipad>>8)&0xff,(detipad)&0xff);
	sprintf(dst_ip,"%d.%d.%d.%d",(ipad>>24)&0xff,(ipad>>16)&0xff,(ipad>>8)&0xff,(ipad)&0xff);
	sprintf(src_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((detectormacadd>>40)&0xFF),
										(unsigned int)((detectormacadd>>32)&0xFF),
										(unsigned int)((detectormacadd>>24)&0xFF),
										(unsigned int)((detectormacadd>>16)&0xFF),
										(unsigned int)((detectormacadd>>8)&0xFF),
										(unsigned int)((detectormacadd>>0)&0xFF));
	sprintf(dst_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((macad>>40)&0xFF),
										(unsigned int)((macad>>32)&0xFF),
										(unsigned int)((macad>>24)&0xFF),
										(unsigned int)((macad>>16)&0xFF),
										(unsigned int)((macad>>8)&0xFF),
										(unsigned int)((macad>>0)&0xFF));

	printf("Seting up Table Entry Left:\n");
	printf("src_port:%d\n",src_port);
	printf("dst_port:%d\n",dst_port);
	printf("src_ip:%s\n",src_ip);
	printf("dst_ip:%s\n",dst_ip);
	printf("src_mac:%s\n",src_mac);
	printf("dst_mac:%s\n\n",dst_mac);



  eiger_back_ret_val=0;
  eiger_back_message_length = sprintf(eiger_back_message,"setuptableentry %d %d %d %s %s %d %s %s %d",34,ten_giga,0,src_mac,src_ip,src_port,dst_mac,dst_ip,dst_port);
  return EigerBackSendCMD();
}


int EigerSetupTableEntryRight(int ipad, long long int macad, long long int detectormacadd, int detipad, int udpport){
	char src_mac[50], src_ip[50],dst_mac[50], dst_ip[50];

	int src_port = 0xE185;
	int dst_port = udpport+1;
	sprintf(src_ip,"%d.%d.%d.%d",(detipad>>24)&0xff,(detipad>>16)&0xff,(detipad>>8)&0xff,(detipad)&0xff);
	sprintf(dst_ip,"%d.%d.%d.%d",(ipad>>24)&0xff,(ipad>>16)&0xff,(ipad>>8)&0xff,(ipad)&0xff);
	sprintf(src_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((detectormacadd>>40)&0xFF),
										(unsigned int)((detectormacadd>>32)&0xFF),
										(unsigned int)((detectormacadd>>24)&0xFF),
										(unsigned int)((detectormacadd>>16)&0xFF),
										(unsigned int)((detectormacadd>>8)&0xFF),
										(unsigned int)((detectormacadd>>0)&0xFF));
	sprintf(dst_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((macad>>40)&0xFF),
										(unsigned int)((macad>>32)&0xFF),
										(unsigned int)((macad>>24)&0xFF),
										(unsigned int)((macad>>16)&0xFF),
										(unsigned int)((macad>>8)&0xFF),
										(unsigned int)((macad>>0)&0xFF));

	printf("Seting up Table Entry Right:\n");
	printf("src_port:%d\n",src_port);
	printf("dst_port:%d\n",dst_port);
	printf("src_ip:%s\n",src_ip);
	printf("dst_ip:%s\n",dst_ip);
	printf("src_mac:%s\n",src_mac);
	printf("dst_mac:%s\n\n",dst_mac);

  eiger_back_ret_val=0;
  eiger_back_message_length = sprintf(eiger_back_message,"setuptableentry %d %d %d %s %s %d %s %s %d",34,ten_giga,32,src_mac,src_ip,src_port,dst_mac,dst_ip,dst_port);
  return EigerBackSendCMD();
}




int RequestImages(){
	printf("Going to request images\n");
  eiger_back_ret_val=0;
  eiger_back_message_length = sprintf(eiger_back_message,"requestimages %d",0); // dst_number
  return EigerBackSendCMD();
}

int SetDestinationParameters(int i){
	eiger_back_ret_val=0;
	eiger_back_message_length = sprintf(eiger_back_message,"setdstparameters %d %d %d",ten_giga,1,i);// number of dsts
	return EigerBackSendCMD();
}


void SetTenGigbaBitEthernet(int val){
	ten_giga = val;
}


int GetTenGigbaBitEthernet(){
	return ten_giga;
}
