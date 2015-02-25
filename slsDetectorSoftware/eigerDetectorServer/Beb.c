 
/**
 * @author Ian Johnson
 * @version 1.0
 */


//return reversed 1 means good, 0 means failed



#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "xfs_types.h"
#include "xparameters.h"

#include "Beb.h"



	 struct BebInfo beb_infos[10];
	 int bebInfoSize = 0;

		struct LocalLinkInterface ll_beb_local,* ll_beb;
		struct LocalLinkInterface ll_beb_new_memory_local,* ll_beb_new_memory;

		 struct udp_header_type udp_header;

		  int           Beb_send_ndata;
		  unsigned int  Beb_send_buffer_size;
		  unsigned int* Beb_send_data_raw;
		  unsigned int* Beb_send_data;

		  int           Beb_recv_ndata;
		  unsigned int  Beb_recv_buffer_size;
		  unsigned int* Beb_recv_data_raw;
		  unsigned int* Beb_recv_data;

		  short Beb_bit_mode;



void BebInfo_BebInfo(struct BebInfo* bebInfo, unsigned int beb_num){
	bebInfo->beb_number=beb_num;
	bebInfo->serial_address=0;
	strcpy(bebInfo->src_mac_1GbE,"");
	strcpy(bebInfo->src_mac_10GbE,"");
	strcpy(bebInfo->src_ip_1GbE,"");
	strcpy(bebInfo->src_ip_10GbE,"");
	bebInfo->src_port_1GbE=bebInfo->src_port_10GbE=0;
}


int BebInfo_SetSerialAddress(struct BebInfo* bebInfo, unsigned int a){
  //address pre shifted
  if(a>0xff) return 0;
  bebInfo->serial_address = 0x04000000 | ((a&0xff)<<16);
  return 1;
}


int BebInfo_SetHeaderInfo(struct BebInfo* bebInfo, int ten_gig, char* src_mac, char* src_ip, unsigned int src_port){
  if(ten_gig){ strcpy(bebInfo->src_mac_10GbE,src_mac); strcpy(bebInfo->src_ip_10GbE,src_ip); bebInfo->src_port_10GbE = src_port;}
  else       { strcpy(bebInfo->src_mac_1GbE,src_mac); strcpy(bebInfo->src_ip_1GbE,src_ip); bebInfo->src_port_1GbE  = src_port;}
  return 1;
}



unsigned int BebInfo_GetBebNumber(struct BebInfo* bebInfo)           {return bebInfo->beb_number;}
unsigned int BebInfo_GetSerialAddress(struct BebInfo* bebInfo)       {return bebInfo->serial_address;}
char*  BebInfo_GetSrcMAC(struct BebInfo* bebInfo, int ten_gig)  {return ten_gig ? bebInfo->src_mac_10GbE  : bebInfo->src_mac_1GbE;}
char*  BebInfo_GetSrcIP(struct BebInfo* bebInfo, int ten_gig)   {return ten_gig ? bebInfo->src_ip_10GbE   : bebInfo->src_ip_1GbE;}
unsigned int BebInfo_GetSrcPort(struct BebInfo* bebInfo, int ten_gig) {return ten_gig ? bebInfo->src_port_10GbE : bebInfo->src_port_1GbE;}


void BebInfo_Print(struct BebInfo* bebInfo){
  printf("\t%d) Beb Info.\n",bebInfo->beb_number);
  printf("\t\tSerial Add: 0x%x\n",bebInfo->serial_address);
  printf("\t\tMAC   1GbE: %s\n",bebInfo->src_mac_1GbE);
  printf("\t\tIP    1GbE: %s\n",bebInfo->src_ip_1GbE);
  printf("\t\tport  1GbE: %d\n",bebInfo->src_port_1GbE);
  printf("\t\tMAC  10GbE: %s\n",bebInfo->src_mac_10GbE);
  printf("\t\tIP   10GbE: %s\n",bebInfo->src_ip_10GbE);
  printf("\t\tport 10GbE: %d\n",bebInfo->src_port_10GbE);
}


void Beb_Beb(){

	Beb_send_ndata = 0;
	Beb_send_buffer_size = 1026;
	Beb_send_data_raw = malloc((Beb_send_buffer_size+1) * sizeof(unsigned int));
	Beb_send_data     = &Beb_send_data_raw[1];

	Beb_recv_ndata = 0;
	Beb_recv_buffer_size = 1026;
	Beb_recv_data_raw = malloc((Beb_recv_buffer_size+1) * sizeof(unsigned int));
	Beb_recv_data     = &Beb_recv_data_raw[1];

	udp_header= (struct udp_header_type){
				     	{0x00, 0x50, 0xc5, 0xb2, 0xcb, 0x46},  // DST MAC
					{0x00, 0x50, 0xc2, 0x46, 0xd9, 0x02},  // SRC MAC
					{0x08, 0x00},
			 		{0x45},
					{0x00},
					{0x00, 0x00},
					{0x00, 0x00},
					{0x40},
					{0x00},
					{0xff},
					{0x11},
					{0x00, 0x00},
					{129, 205, 205, 128},  // Src IP
				 	{129, 205, 205, 122},  // Dst IP
					{0x0f, 0xa1},
					{0x13, 0x89},
					{0x00, 0x00}, //{0x00, 0x11},
					{0x00, 0x00}
				};


  if(!Beb_InitBebInfos()) exit(1);

  printf("Printing Beb infos:\n");
  unsigned int i;
  for(i=1;i<bebInfoSize;i++) BebInfo_Print(&beb_infos[i]);
  printf("\n");

  Beb_bit_mode = 4;
  
  ll_beb = &ll_beb_local;
  Local_LocalLinkInterface1(ll_beb,XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_LEFT_BASEADDR);

  Beb_SetByteOrder();

}



void Beb_GetModuleCopnfiguration(int* master, int* top){
	*top = 0;
	*master = 0;
	  //mapping new memory to read master top module configuration
	  ll_beb_new_memory = &ll_beb_new_memory_local;
	  Local_LocalLinkInterface(ll_beb_new_memory);
	  int ret  = Local_GetModuleConfiguration(ll_beb_new_memory,XPAR_PLB_GPIO_SYS_BASEADDR, MODULE_CONFIGURATION);
	  if(!ret)
		  printf("Module Configuration FAIL\n");
	  else{
		  printf("Module Configuration OK\n");
		  printf("Beb: value =0x%x\n",ret);
		  if(ret&0xf){
			  *top = 1;
			 // printf("Beb.c: TOP\n\n\n\n");
		  }//else  printf("Beb.c: BOTTOM\n\n\n\n");

		  if(ret&0x200){
			  *master = 1;
			 // printf("Beb.c: MASTER\n\n\n\n");
		  }//else  printf("Beb.c: SLAVE\n\n\n\n");
	  }
}



void Beb_ClearBebInfos(){
	//unsigned int i;
  //for(i=0;i<bebInfoSize;i++) free(beb_infos[i]);
	bebInfoSize=0;
}

int Beb_InitBebInfos(){//file name at some point
	Beb_ClearBebInfos();


  struct BebInfo b0;
  BebInfo_BebInfo(&b0,0);
    if(BebInfo_SetSerialAddress(&b0,0xff)) { //all bebs for reset and possibly get request data?
    	beb_infos[bebInfoSize] = b0;
    	bebInfoSize++;
    }
    
    if(!Beb_ReadSetUpFromFile("/home/root/executables/setup_beb.txt")) return 0;
/*
  //loop through file to fill vector.
  BebInfo* b = new BebInfo(26);
    b->SetSerialAddress(0); //0xc4000000
    b->SetHeaderInfo(0,"00:50:c2:46:d9:34","129.129.205.78",42000 + 26); // 1 GbE, ip address can be acquire from the network "arp"
    b->SetHeaderInfo(1,"00:50:c2:46:d9:35","10.0.26.1",52000 + 26); //10 GbE, everything calculable/setable
    beb_infos.push_back(b);
*/


  return Beb_CheckSourceStuffBebInfo();
}



int Beb_SetBebSrcHeaderInfos(unsigned int beb_number, int ten_gig, char* src_mac, char* src_ip,unsigned int src_port){
  //so that the values can be reset externally for now....

  unsigned int i = 1;/*Beb_GetBebInfoIndex(beb_number);*/
 /******* if(!i) return 0;****************************/ //i must be greater than 0, zero is the global send
  BebInfo_SetHeaderInfo(&beb_infos[i],ten_gig,src_mac,src_ip,src_port);

  printf("Printing Beb info number (%d) :\n",i);
    BebInfo_Print(&beb_infos[i]);
  printf("\n");

  return 1;
}


int Beb_ReadSetUpFromFile(char* file_name){
	char line[100];
	char str[100];

	int i0,i1;
	char mac0[50],mac1[50],ip0[50],ip1[0];
	FILE* fp = fopen(file_name, "r");
	if( fp == NULL ){
		perror("Error while opening the beb setup file.\n");
		return 0;
	}

	printf("Setting up beb side of detector:\n");
	while ( fgets (line , 255 , fp) != NULL ){
		if(strlen(line)<=1)
			continue;
		sscanf (line, "%s", str);
		if (str[0]=='#')
			continue;

		if(!strcmp(str,"add_beb")){
			if( sscanf (line,"%s %d %d %s %s %s %s",str,&i0,&i1,mac0,ip0,mac1,ip1) < 7){
				printf("Error adding beb from %s.\n",file_name);
				exit(0);
			}

			printf ("Read: %s %d %d %s %s %s %s\n", str,i0,i1,mac0,ip0,mac1,ip1);

			if(Beb_GetBebInfoIndex(i0)){
				printf("Error adding beb from %s, beb number %d already added.\n",file_name,i0);
				exit(0);
			}

			struct BebInfo b0;
			BebInfo_BebInfo(&b0,i0);
			BebInfo_SetSerialAddress(&b0,i1);
			BebInfo_SetHeaderInfo(&b0,0,mac0,ip0,42000+i0);
			BebInfo_SetHeaderInfo(&b0,1,mac1,ip1,52000+i0);
			beb_infos[bebInfoSize] = b0;
			bebInfoSize++;
		}
	}
	fclose(fp);
	return 1;
}



int Beb_CheckSourceStuffBebInfo(){
	unsigned int i;
  for(i=1;i<bebInfoSize;i++){ //header stuff always starts from 1
    if(!Beb_SetHeaderData(BebInfo_GetBebNumber(&beb_infos[i]),0,"00:00:00:00:00:00","10.0.0.1",20000)||!Beb_SetHeaderData(BebInfo_GetBebNumber(&beb_infos[i]),1,"00:00:00:00:00:00","10.0.0.1",20000)){
      printf("Error in BebInfo for module number %d.\n",BebInfo_GetBebNumber(&beb_infos[i]));
      BebInfo_Print(&beb_infos[i]);
      return 0;
    }
  }
  return 1;
}

unsigned int Beb_GetBebInfoIndex(unsigned int beb_numb){
/******************** if(!beb_numb) return 0;******************************/
  unsigned int i;
  for(i=1;i<bebInfoSize;i++)
	  if(beb_numb==BebInfo_GetBebNumber(&beb_infos[i])){
		  printf("*****found beb index:%d, for beb number:%d\n",i,beb_numb);
		  return i;
	  }
  printf("*****Returning 0\n");
  return 0;
}



int Beb_WriteTo(unsigned int index){
  if(index>=bebInfoSize){
    printf("WriteTo index error.\n");
    return 0;
  }

  Beb_send_data_raw[0] = 0x90000000 | BebInfo_GetSerialAddress(&beb_infos[index]);
    if(Local_Write(ll_beb,4,Beb_send_data_raw)!=4) return 0;

    Beb_send_data_raw[0] = 0xc0000000;
    if((Beb_send_ndata+1)*4!=Local_Write(ll_beb,(Beb_send_ndata+1)*4,Beb_send_data_raw)) return 0;

  return 1;
}


void Beb_SwapDataFun(int little_endian, unsigned int n, unsigned int *d){
	unsigned int i;
  if(little_endian) for(i=0;i<n;i++) d[i] = (((d[i]&0xff)<<24) | ((d[i]&0xff00)<<8) | ((d[i]&0xff0000)>>8) | ((d[i]&0xff000000)>>24)); //little_endian
  else              for(i=0;i<n;i++) d[i] = (((d[i]&0xffff)<<16) | ((d[i]&0xffff0000)>>16));
}


int Beb_SetByteOrder(){
	Beb_send_data_raw[0] = 0x8fff0000;
  if(Local_Write(ll_beb,4,Beb_send_data_raw)!=4) return 0;
  
  while((Local_Read(ll_beb,Beb_recv_buffer_size*4,Beb_recv_data_raw)/4)>0) printf("\t) Cleanning buffer ...\n");

  if(bebInfoSize<2) return 0;

  Beb_send_ndata   = 3;
  Beb_send_data[0] = 0x000c0000;
  Beb_send_data[1] = 0;
  Beb_send_data[2] = 0;
  Beb_WriteTo(0);

  //using little endian for data, big endian not fully tested, swap on 16 bit boundary.
  Beb_send_ndata   = 3;
  Beb_send_data[0] = 0x000c0000;
  Beb_send_data[1] = 1;
  Beb_send_data[2] = 0;
  Beb_SwapDataFun(0,2,&(Beb_send_data[1]));
  Beb_WriteTo(0);
   
  printf("\tSetting Byte Order ..............        ok\n");

  return 1;
}




int Beb_SetUpUDPHeader(unsigned int beb_number, int ten_gig, unsigned int header_number, char* dst_mac, char* dst_ip, unsigned int dst_port){
  unsigned int i = 1;/*Beb_GetBebInfoIndex(beb_number);*/

  /***********************************if(!i) return 0; *************************************///i must be greater than 0, zero is the global send

  Beb_send_ndata   = 14;
  Beb_send_data[0] = ten_gig ? 0x00020000 : 0x00010000; //write to fanout numbers 1 or 2
  Beb_send_data[1] = ((header_number*8)<<16);
    if(!Beb_SetHeaderData(beb_number,ten_gig,dst_mac,dst_ip,dst_port)) return 0;

    Beb_SwapDataFun(1,12,&(Beb_send_data[2]));

  if(!Beb_WriteTo(i)) return 0;
  printf("beb dst_port:%d\n",dst_port);
  return 1;
}


int Beb_SetHeaderData(unsigned int beb_number, int ten_gig, char* dst_mac, char* dst_ip, unsigned int dst_port){
  unsigned int i = 1;/*Beb_GetBebInfoIndex(beb_number);*/
  /***********************************if(!i) return 0; *************************************///i must be greater than 0, zero is the global send
  return Beb_SetHeaderData1(BebInfo_GetSrcMAC(&beb_infos[i],ten_gig),BebInfo_GetSrcIP(&beb_infos[i],ten_gig),BebInfo_GetSrcPort(&beb_infos[i],ten_gig),dst_mac,dst_ip,dst_port);
}

int Beb_SetHeaderData1(char* src_mac, char* src_ip, unsigned int src_port, char* dst_mac, char* dst_ip, unsigned int dst_port){
  /* example header*/
  //static unsigned int*   word_ptr   = new unsigned int [16];
  /*static*/
	/*
  udp_header_type udp_header = {
	     	{0x00, 0x50, 0xc5, 0xb2, 0xcb, 0x46},  // DST MAC
		{0x00, 0x50, 0xc2, 0x46, 0xd9, 0x02},  // SRC MAC
		{0x08, 0x00},
 		{0x45},
		{0x00},
		{0x00, 0x00},
		{0x00, 0x00},
		{0x40},
		{0x00},
		{0xff},
		{0x11},
		{0x00, 0x00}, 
		{129, 205, 205, 128},  // Src IP
	 	{129, 205, 205, 122},  // Dst IP
		{0x0f, 0xa1}, 
		{0x13, 0x89}, 		
		{0x00, 0x00}, //{0x00, 0x11},
		{0x00, 0x00}
	};
*/

  if(!Beb_SetMAC(src_mac,&(udp_header.src_mac[0])))           return 0;
  printf("Setting Source MAC to %s\n",src_mac);
  if(!Beb_SetIP(src_ip,&(udp_header.src_ip[0])))              return 0;
  printf("Setting Source IP to %s\n",src_ip);
  if(!Beb_SetPortNumber(src_port,&(udp_header.src_port[0])))  return 0;
  printf("Setting Source port to %d\n",src_port);

  if(!Beb_SetMAC(dst_mac,&(udp_header.dst_mac[0])))           return 0;
  printf("Setting Destination MAC to %s\n",dst_mac);
  if(!Beb_SetIP(dst_ip,&(udp_header.dst_ip[0])))              return 0;
  printf("Setting Destination IP to %s\n",dst_ip);
  if(!Beb_SetPortNumber(dst_port,&(udp_header.dst_port[0])))  return 0;
  printf("Setting Destination port to %d\n",dst_port);


  Beb_AdjustIPChecksum(&udp_header);

  unsigned int* base_ptr  = (unsigned int *) &udp_header;
  unsigned int  num_words = ( sizeof(struct udp_header_type) + 3 ) / 4;
  //  for(unsigned int i=0; i<num_words; i++)  word_ptr[i] = base_ptr[i];
  //  for(unsigned int i=num_words; i<16; i++) word_ptr[i] = 0;
  //  return word_ptr;
  unsigned int i;
  for(i=0; i<num_words; i++)  Beb_send_data[i+2] = base_ptr[i];
  for(i=num_words; i<16; i++) Beb_send_data[i+2] = 0;

  return 1;
}


int Beb_SetMAC(char* mac, uint8_t* dst_ptr){
	char macVal[50];strcpy(macVal,mac);

	int i = 0;
	char *pch = strtok (macVal,":");
	while (pch != NULL){
		if(strlen(pch)!=2){
			printf("Error: in mac address -> %s\n",macVal);
			return 0;
		}

		int itemp;
		sscanf(pch,"%x",&itemp);
		dst_ptr[i] = (u_int8_t)itemp;
		pch = strtok (NULL, ":");
		i++;
	}
	return 1;
}

int Beb_SetIP(char* ip, uint8_t* dst_ptr){
	char ipVal[50];strcpy(ipVal,ip);
	int i = 0;
	char *pch = strtok (ipVal,".");
	while (pch != NULL){
			if(((i!=3) && ((strlen(pch)>3) || (strlen(pch)<1))) || ((i==3)&&((strlen(pch)<1) || (strlen(pch) > 3)))){
				printf("Error: in ip address -> %s\n",ipVal);
				return 0;
			}

		int itemp;
		sscanf(pch,"%d",&itemp);
		dst_ptr[i] = (u_int8_t)itemp;
		pch = strtok (NULL, ".");
		i++;
	}
	return 1;
}

int Beb_SetPortNumber(unsigned int port_number, uint8_t* dst_ptr){
  dst_ptr[0] = (port_number >> 8) & 0xff ;
  dst_ptr[1] = port_number & 0xff;
  return 1;
}


void Beb_AdjustIPChecksum(struct udp_header_type *ip){
  unsigned char *cptr = (unsigned char *) ip->ver_headerlen;

  ip->ip_header_checksum[0] = 0;
  ip->ip_header_checksum[1] = 0;
  ip->total_length[0] = 0;
  ip->total_length[1] = 28; // IP + UDP Header Length
  
  // calc ip checksum  
  unsigned int ip_checksum = 0;
  unsigned int i;
  for(i=0; i<10; i++){
    ip_checksum += ( (cptr[2*i] << 8)  + (cptr[2*i + 1]) );
    if (ip_checksum & 0x00010000) ip_checksum = (ip_checksum + 1) & 0x0000ffff;
  }   
  
  ip->ip_header_checksum[0] = (ip_checksum >> 8) & 0xff ;
  ip->ip_header_checksum[1] = ip_checksum & 0xff ;
}



int Beb_SendMultiReadRequest(unsigned int beb_number, unsigned int left_right, int ten_gig, unsigned int dst_number, unsigned int npackets, unsigned int packet_size, int stop_read_when_fifo_empty){

  unsigned int i = 1;/*Beb_GetBebInfoIndex(beb_number); //zero is the global send*/

  Beb_send_ndata   = 3;
    if(left_right == 1)      Beb_send_data[0] = 0x00040000;
    else if(left_right == 2) Beb_send_data[0] = 0x00080000;
    else if(left_right == 3) Beb_send_data[0] = 0x000c0000;
    else                     return 0;

    //packet_size/=2;
    if(dst_number>0x3f)   return 0;
    if(packet_size>0x3ff) return 0;
    if(npackets==0||npackets>0x100) return 0;
    npackets--; 


    Beb_send_data[1] = 0x62000000 | (!stop_read_when_fifo_empty) << 27 | (ten_gig==1) << 24 | packet_size << 14 | dst_number << 8 | npackets;
    Beb_send_data[2] = 0;
    
    Beb_SwapDataFun(0,2,&(Beb_send_data[1]));

    if(!Beb_WriteTo(i)) return 0;

  return 1;
}


int Beb_SetUpTransferParameters(short the_bit_mode){
  if(the_bit_mode!=4&&the_bit_mode!=8&&the_bit_mode!=16&&the_bit_mode!=32) return 0;
  Beb_bit_mode = the_bit_mode;
  
  //nimages = the_number_of_images;
  //  on_dst = 0;

  return 1;
}

int Beb_RequestNImages(unsigned int beb_number, unsigned int left_right, int ten_gig, unsigned int dst_number, unsigned int nimages, int test_just_send_out_packets_no_wait){
  if(dst_number>64) return 0;

  unsigned int     header_size  = 4; //4*64 bits
  unsigned int     packet_size  = ten_gig ? 0x200 : 0x80; // 4k or  1k packets 
  unsigned int         npackets = ten_gig ?  Beb_bit_mode*4 : Beb_bit_mode*16;
  int          in_two_requests = (!ten_gig&&Beb_bit_mode==32);
  if(in_two_requests) npackets/=2;
 // printf("npackets:%d\n",npackets);
  //usleep needed after acquisition start, else you miss the single images
  usleep(10000);//less than this and it starts sending half stuff sometimes

  //printf("beb no:%d left_right:%d ten_gig:%d dst_number:%d #images:%d header_size:%d test_just_send_out_packets_no_wait:%d\n",beb_number,left_right,ten_gig,dst_number,nimages, header_size,test_just_send_out_packets_no_wait);
  //printf("here: "<<beb_number<<","<<left_right<<","<<ten_gig<<","<<dst_number<<","<<1<<","<<header_size<<","<<test_just_send_out_packets_no_wait\n");
/*
  unsigned int i;
  for(i=0;i<nimages;i++){
    //header then data request
	  //  usleep(10000);
    if(!Beb_SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,1,header_size,test_just_send_out_packets_no_wait)){printf("Send failed\n");return 0;}
   // usleep(10000);
    if(!Beb_SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,npackets,packet_size,test_just_send_out_packets_no_wait)){printf("Send failed\n");return 0;}
    usleep(1000);
    if(in_two_requests){if(!Beb_SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,npackets,packet_size,test_just_send_out_packets_no_wait)){printf("Send failed\n");return 0;}
    }
  }
*/



  unsigned int i;
  for(i=0;i<nimages;i++){
    //header then data request
    if(!Beb_SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,1,header_size,test_just_send_out_packets_no_wait) ||
       !Beb_SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,npackets,packet_size,test_just_send_out_packets_no_wait) ||
       (in_two_requests&&!Beb_SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,npackets,packet_size,test_just_send_out_packets_no_wait))){
       	printf("SendMultiReadRequest failed\n");
    	return 0;
    }
  }


  return 1;
}


int Beb_Test(unsigned int beb_number){
  printf("Testing module number: %d\n",beb_number);
  

  //int SetUpUDPHeader(unsigned int beb_number, int ten_gig, unsigned int header_number, string dst_mac, string dst_ip, unsigned int dst_port){
  //SetUpUDPHeader(26,0,0,"60:fb:42:f4:e3:d2","129.129.205.186",22000);

  unsigned int index = Beb_GetBebInfoIndex(beb_number);
  if(!index){
    printf("Error beb number (%d)not in list????\n",beb_number);
    return 0;
  }

  unsigned int i;
  for(i=0;i<64;i++){
    if(!Beb_SetUpUDPHeader(beb_number,0,i,"60:fb:42:f4:e3:d2","129.129.205.186",22000+i)){
      printf("Error setting up header table....\n");
      return 0;
    }
  }

  //  SendMultiReadRequest(unsigned int beb_number, unsigned int left_right, int ten_gig, unsigned int dst_number, unsigned int npackets, unsigned int packet_size, int stop_read_when_fifo_empty=1);
  for(i=0;i<64;i++){
    if(!Beb_SendMultiReadRequest(beb_number,i%3+1,0,i,1,0,1)){
      printf("Error requesting data....\n");
      return 0;
    }
  }
  

  return 1;
}

