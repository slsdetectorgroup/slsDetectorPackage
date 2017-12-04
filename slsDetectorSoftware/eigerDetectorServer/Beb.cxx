 
/**
 * @author Ian Johnson
 * @version 1.0
 */


//return reversed 1 means good, 0 means failed



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>


#include "xfs_types.h"
#include "xparameters.h"

#include "Beb.h"

using namespace std;

BebInfo::BebInfo(unsigned int beb_num){beb_number=beb_num;serial_address=0;src_mac_1GbE="";src_mac_10GbE="";src_ip_1GbE="";src_ip_10GbE=""; src_port_1GbE=src_port_10GbE=0;}
bool BebInfo::SetHeaderInfo(bool ten_gig, string src_mac, string src_ip, unsigned int src_port){
  if(ten_gig){ src_mac_10GbE = src_mac; src_ip_10GbE = src_ip; src_port_10GbE = src_port;}
  else       { src_mac_1GbE  = src_mac; src_ip_1GbE  = src_ip; src_port_1GbE  = src_port;}
  return 1;
}

bool BebInfo::SetSerialAddress(unsigned int a){
  //address pre shifted
  if(a>0xff) return 0;
  serial_address = 0x04000000 | ((a&0xff)<<16);
  return 1;
}

void BebInfo::Print(){
  cout<<"\t"<<beb_number<<") Beb Info.:"<<endl;
  cout<<"\t\tSerial Add: 0x"<<hex<<serial_address<<dec<<endl;
  cout<<"\t\tMAC   1GbE: "<<src_mac_1GbE.c_str()<<endl;
  cout<<"\t\tIP    1GbE: "<<src_ip_1GbE.c_str()<<endl;
  cout<<"\t\tport  1GbE: "<<src_port_1GbE<<endl;
  cout<<"\t\tMAC  10GbE: "<<src_mac_10GbE.c_str()<<endl;
  cout<<"\t\tIP   10GbE: "<<src_ip_10GbE.c_str()<<endl;
  cout<<"\t\tport 10GbE: "<<src_port_10GbE<<endl;
}


Beb::Beb(int arg1){

  send_ndata = 0;
  send_buffer_size = 1026;
  send_data_raw = new unsigned int [send_buffer_size+1];
  send_data     = &send_data_raw[1];

  recv_ndata = 0;
  recv_buffer_size = 1026;
  recv_data_raw = new unsigned int [recv_buffer_size+1];
  recv_data     = &recv_data_raw[1];

  if(!InitBebInfos()) exit(1);

  cout<<"Printing Beb infos:"<<endl;
  for(unsigned int i=1;i<beb_infos.size();i++) beb_infos[i]->Print();
  cout<<endl<<endl;

  bit_mode = 4;
  
  ll = new LocalLinkInterface(XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_LEFT_BASEADDR);

  SetByteOrder();


  new_memory = new LocalLinkInterface();
  if(!new_memory->InitNewMemory(XPAR_PLB_LL_NEW_MEMORY, arg1))
	  printf("New Memory FAIL\n");
  else
	  printf("New Memory OK\n");

}

Beb::~Beb(){
  delete ll;
  delete [] send_data_raw;
  delete [] recv_data_raw;
}

void Beb::ClearBebInfos(){
  for(unsigned int i=0;i<beb_infos.size();i++) delete beb_infos[i];
  beb_infos.clear();
}

bool Beb::InitBebInfos(){//file name at some point
  ClearBebInfos();

  BebInfo* b0 = new BebInfo(0);
    if(b0->SetSerialAddress(0xff)) beb_infos.push_back(b0); //all bebs for reset and possibly get request data?
    
    if(!ReadSetUpFromFile("/home/root/executables/setup_beb.txt")) return 0;
/*
  //loop through file to fill vector.
  BebInfo* b = new BebInfo(26);
    b->SetSerialAddress(0); //0xc4000000
    b->SetHeaderInfo(0,"00:50:c2:46:d9:34","129.129.205.78",42000 + 26); // 1 GbE, ip address can be acquire from the network "arp"
    b->SetHeaderInfo(1,"00:50:c2:46:d9:35","10.0.26.1",52000 + 26); //10 GbE, everything calculable/setable
    beb_infos.push_back(b);
*/

  return CheckSourceStuffBebInfo();
}



bool Beb::SetBebSrcHeaderInfos(unsigned int beb_number, bool ten_gig, string src_mac, string src_ip,unsigned int src_port){
  //so that the values can be reset externally for now....

  unsigned int i = GetBebInfoIndex(beb_number);
  if(!i) return 0; //i must be greater than 0, zero is the global send
  beb_infos[i]->SetHeaderInfo(ten_gig,src_mac,src_ip,src_port);

  cout<<"Printing Beb info number ("<<i<<") :"<<endl;
    beb_infos[i]->Print();
  cout<<endl<<endl;

  return 1;
}


bool Beb::ReadSetUpFromFile(string file_name){

  static ifstream infile;
  static string   line;
  static char     cmd_st[2000],str_mac1[200],str_ip1[200],str_mac10[200],str_ip10[200];
  static int      value_i[2];
  
  infile.open(file_name.c_str(),ios::in);
  if(!infile.is_open()){
    cout<<"Error could not open setup file: "<<file_name<<"."<<endl;
    return 0;
  }

  cout<<endl;
  cout<<"Setting up detector:"<<endl;
  while(std::getline(infile,line)){
    if(line.length()<1) continue;
    istringstream iss(line);
    iss>>cmd_st;
    if(!strcmp("add_beb",cmd_st)){
      if(!(iss>>value_i[0]>>value_i[1]>>str_mac1>>str_ip1>>str_mac10>>str_ip10)){
	cout<<"Error adding beb from "<<file_name<<"."<<endl;
	exit(0);
      }

      if(GetBebInfoIndex(value_i[0])){
	cout<<"Error adding beb from "<<file_name<<", beb number "<<value_i[0]<<" already added."<<endl;
	exit(0);
      }

      BebInfo* b = new BebInfo(value_i[0]);
        b->SetSerialAddress(value_i[1]);
	b->SetHeaderInfo(0,str_mac1,str_ip1,42000+value_i[0]);
	b->SetHeaderInfo(1,str_mac10,str_ip10,52000+value_i[0]);
	beb_infos.push_back(b);
    }
  }

  infile.close();

  return 1;
}



bool Beb::CheckSourceStuffBebInfo(){
  for(unsigned int i=1;i<beb_infos.size();i++){ //header stuff always starts from 1
    if(!SetHeaderData(beb_infos[i]->GetBebNumber(),0,"00:00:00:00:00:00","10.0.0.1",20000)||!SetHeaderData(beb_infos[i]->GetBebNumber(),1,"00:00:00:00:00:00","10.0.0.1",20000)){
      cout<<"Error in BebInfo for module number "<<beb_infos[i]->GetBebNumber()<<"."<<endl; 
      beb_infos[i]->Print();
      return 0;
    }
  }
  return 1;
}

unsigned int Beb::GetBebInfoIndex(unsigned int beb_numb){
  if(!beb_numb) return 0;

  for(unsigned int i=1;i<beb_infos.size();i++) if(beb_numb==beb_infos[i]->GetBebNumber()) return i; 
  return 0;
}



bool Beb::WriteTo(unsigned int index){
  if(index>=beb_infos.size()){
    cout<<"WriteTo index error."<<endl;
    return 0;
  }

  send_data_raw[0] = 0x90000000 | beb_infos[index]->GetSerialAddress();
    if(ll->Write(4,send_data_raw)!=4) return 0;

  send_data_raw[0] = 0xc0000000;
    if((send_ndata+1)*4!=ll->Write((send_ndata+1)*4,send_data_raw)) return 0;

  return 1;
}


void Beb::SwapDataFun(bool little_endian, unsigned int n, unsigned int *d){
  if(little_endian) for(unsigned int i=0;i<n;i++) d[i] = (((d[i]&0xff)<<24) | ((d[i]&0xff00)<<8) | ((d[i]&0xff0000)>>8) | ((d[i]&0xff000000)>>24)); //little_endian
  else              for(unsigned int i=0;i<n;i++) d[i] = (((d[i]&0xffff)<<16) | ((d[i]&0xffff0000)>>16));
}


bool Beb::SetByteOrder(){
  send_data_raw[0] = 0x8fff0000;
  if(ll->Write(4,send_data_raw)!=4) return 0;
  
  while((ll->Read(recv_buffer_size*4,recv_data_raw)/4)>0) cout<<"\t) Cleanning buffer ..."<<endl;

  if(beb_infos.size()<2) return 0;

  send_ndata   = 3;
    send_data[0] = 0x000c0000;
    send_data[1] = 0;
    send_data[2] = 0;
    WriteTo(0);

  //using little endian for data, big endian not fully tested, swap on 16 bit boundary.
  send_ndata   = 3;
    send_data[0] = 0x000c0000;
    send_data[1] = 1;
    send_data[2] = 0;
    SwapDataFun(0,2,&(send_data[1]));
    WriteTo(0);
   
  cout<<"\tSetting Byte Order ..............        ok"<<endl<<endl;

  return 1;
}




bool Beb::SetUpUDPHeader(unsigned int beb_number, bool ten_gig, unsigned int header_number, string dst_mac, string dst_ip, unsigned int dst_port){
  unsigned int i = GetBebInfoIndex(beb_number);
  if(!i) return 0; //i must be greater than 0, zero is the global send

  send_ndata   = 14;
    send_data[0] = ten_gig ? 0x00020000 : 0x00010000; //write to fanout numbers 1 or 2
    send_data[1] = ((header_number*8)<<16);
    if(!SetHeaderData(beb_number,ten_gig,dst_mac,dst_ip,dst_port)) return 0;

    SwapDataFun(1,12,&(send_data[2]));

  if(!WriteTo(i)) return 0;
  
  return 1;
}


bool Beb::SetHeaderData(unsigned int beb_number, bool ten_gig, string dst_mac, string dst_ip, unsigned int dst_port){
  unsigned int i = GetBebInfoIndex(beb_number);
  if(!i) return 0; //i must be greater than 0, zero is the global send
  return SetHeaderData(beb_infos[i]->GetSrcMAC(ten_gig),beb_infos[i]->GetSrcIP(ten_gig),beb_infos[i]->GetSrcPort(ten_gig),dst_mac,dst_ip,dst_port);
}

bool Beb::SetHeaderData(string src_mac, string src_ip, unsigned int src_port, string dst_mac, string dst_ip, unsigned int dst_port){
  /* example header*/	
  //static unsigned int*   word_ptr   = new unsigned int [16];
  static udp_header_type udp_header = {
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

  if(!SetMAC(src_mac,&(udp_header.src_mac[0])))           return 0;
  if(!SetIP(src_ip,&(udp_header.src_ip[0])))              return 0;
  if(!SetPortNumber(src_port,&(udp_header.src_port[0])))  return 0;

  if(!SetMAC(dst_mac,&(udp_header.dst_mac[0])))           return 0;
  if(!SetIP(dst_ip,&(udp_header.dst_ip[0])))              return 0;
  if(!SetPortNumber(dst_port,&(udp_header.dst_port[0])))  return 0;


  AdjustIPChecksum(&udp_header);  

  unsigned int* base_ptr  = (unsigned int *) &udp_header;
  unsigned int  num_words = ( sizeof(udp_header_type) + 3 ) / 4;
  //  for(unsigned int i=0; i<num_words; i++)  word_ptr[i] = base_ptr[i];
  //  for(unsigned int i=num_words; i<16; i++) word_ptr[i] = 0;
  //  return word_ptr;

  for(unsigned int i=0; i<num_words; i++)  send_data[i+2] = base_ptr[i];
  for(unsigned int i=num_words; i<16; i++) send_data[i+2] = 0;

  return 1;
}


bool Beb::SetMAC(string mac, unsigned char* dst_ptr){
  for(int i=0;i<6;i++){
    size_t p0=mac.find(':');
    if((i!=5&&p0!=2)||(i==5&&mac.length()!=2)){
      cout<<"Error: in mac address -> "<<mac<<endl;
      return 0;
    }
    dst_ptr[i] = (unsigned char) strtoul(mac.substr(0,p0).c_str(),NULL,16); 
    mac=mac.substr(p0+1);
  }

  return 1;
}

bool Beb::SetIP(string ip, unsigned char* dst_ptr){
  for(int i=0;i<4;i++){
    size_t p0=ip.find('.');
    if((i!=3&&(p0<1||p0>3))||(i==3&&(ip.length()<1||ip.length()>3))){
      cout<<"Error: in ip address -> "<<ip<<endl;
      return 0;
    }
    dst_ptr[i] = atoi(ip.substr(0,p0).c_str());
    ip=ip.substr(p0+1);
  }

  return 1;
}

bool Beb::SetPortNumber(unsigned int port_number, unsigned char* dst_ptr){
  dst_ptr[0] = (port_number >> 8) & 0xff ;
  dst_ptr[1] = port_number & 0xff;
  return 1;
}


void Beb::AdjustIPChecksum(udp_header_type *ip){
  unsigned char *cptr = (unsigned char *) ip->ver_headerlen;

  ip->ip_header_checksum[0] = 0;
  ip->ip_header_checksum[1] = 0;
  ip->total_length[0] = 0;
  ip->total_length[1] = 28; // IP + UDP Header Length
  
  // calc ip checksum  
  unsigned int ip_checksum = 0;
  for(unsigned int i=0; i<10; i++){ 
    ip_checksum += ( (cptr[2*i] << 8)  + (cptr[2*i + 1]) );
    if (ip_checksum & 0x00010000) ip_checksum = (ip_checksum + 1) & 0x0000ffff;
  }   
  
  ip->ip_header_checksum[0] = (ip_checksum >> 8) & 0xff ;
  ip->ip_header_checksum[1] = ip_checksum & 0xff ;
}



bool Beb::SendMultiReadRequest(unsigned int beb_number, unsigned int left_right, bool ten_gig, unsigned int dst_number, unsigned int npackets, unsigned int packet_size, bool stop_read_when_fifo_empty){

  unsigned int i = GetBebInfoIndex(beb_number); //zero is the global send

  send_ndata   = 3;
    if(left_right == 1)      send_data[0] = 0x00040000;
    else if(left_right == 2) send_data[0] = 0x00080000;
    else if(left_right == 3) send_data[0] = 0x000c0000;
    else                     return 0;

    //packet_size/=2;
    if(dst_number>0x3f)   return 0;
    if(packet_size>0x3ff) return 0;
    if(npackets==0||npackets>0x100) return 0;
    npackets--; 


    send_data[1] = 0x62000000 | (!stop_read_when_fifo_empty) << 27 | (ten_gig==1) << 24 | packet_size << 14 | dst_number << 8 | npackets;
    send_data[2] = 0;
    
    SwapDataFun(0,2,&(send_data[1]));

    if(!WriteTo(i)) return 0;

  return 1;
}


bool Beb::SetUpTransferParameters(short the_bit_mode){
  if(the_bit_mode!=4&&the_bit_mode!=8&&the_bit_mode!=16&&the_bit_mode!=32) return 0;
  bit_mode = the_bit_mode;
  
  //nimages = the_number_of_images;
  //  on_dst = 0;

  return 1;
}

bool Beb::RequestNImages(unsigned int beb_number, unsigned int left_right, bool ten_gig, unsigned int dst_number, unsigned int nimages, bool test_just_send_out_packets_no_wait){
  if(dst_number>64) return 0;

  unsigned int     header_size  = 4; //4*64 bits
  unsigned int     packet_size  = ten_gig ? 0x200 : 0x80; // 4k or  1k packets 
  unsigned int         npackets = ten_gig ?  bit_mode*4 : bit_mode*16;
  bool          in_two_requests = (!ten_gig&&bit_mode==32);
  if(in_two_requests) npackets/=2;
 
  //cout<<"here: "<<beb_number<<","<<left_right<<","<<ten_gig<<","<<dst_number<<","<<1<<","<<header_size<<","<<test_just_send_out_packets_no_wait<<endl;

  for(unsigned int i=0;i<nimages;i++){
    //header then data request
    if(!SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,1,header_size,test_just_send_out_packets_no_wait) ||
       !SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,npackets,packet_size,test_just_send_out_packets_no_wait) ||
       (in_two_requests&&!SendMultiReadRequest(beb_number,left_right,ten_gig,dst_number,npackets,packet_size,test_just_send_out_packets_no_wait))) return 0;
  }

  return 1;
}


bool Beb::Test(unsigned int beb_number){
  cout<<"Testing module number: "<<beb_number<<endl;
  

  //bool Beb::SetUpUDPHeader(unsigned int beb_number, bool ten_gig, unsigned int header_number, string dst_mac, string dst_ip, unsigned int dst_port){
  //SetUpUDPHeader(26,0,0,"60:fb:42:f4:e3:d2","129.129.205.186",22000);

  unsigned int index = GetBebInfoIndex(beb_number);
  if(!index){
    cout<<"Error beb number ("<<beb_number<<")not in list????"<<endl;
    return 0;
  }


  for(unsigned int i=0;i<64;i++){
    if(!SetUpUDPHeader(beb_number,0,i,"60:fb:42:f4:e3:d2","129.129.205.186",22000+i)){
      cout<<"Error setting up header table...."<<endl;
      return 0;
    }
  }

  //  SendMultiReadRequest(unsigned int beb_number, unsigned int left_right, bool ten_gig, unsigned int dst_number, unsigned int npackets, unsigned int packet_size, bool stop_read_when_fifo_empty=1);
  for(unsigned int i=0;i<64;i++){
    if(!SendMultiReadRequest(beb_number,i%3+1,0,i,1,0)){
      cout<<"Error requesting data...."<<endl;
      return 0;
    }
  }
  

  return 1;
}

