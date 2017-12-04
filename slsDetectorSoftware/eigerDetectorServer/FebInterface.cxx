 
/**
 * @author Ian Johnson
 * @version 1.0
 */



#include <iostream>
#include <iomanip>
//#include <unistd.h>
//#include <string.h>
//#include <sys/mman.h>
//#include <fcntl.h>

#include "xparameters.h"

#include "FebInterface.h"

using namespace std;

FebInterface::FebInterface(){

  nfebs    = 0;
  feb_numb = 0;

  send_ndata = 0;
  send_buffer_size = 1026;
  send_data_raw = new unsigned int [send_buffer_size+1];
  send_data     = &send_data_raw[1];

  recv_ndata = 0;
  recv_buffer_size = 1026;
  recv_data_raw = new unsigned int [recv_buffer_size+1];
  recv_data     = &recv_data_raw[1];

  ll = new LocalLinkInterface(XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_RIGHT_BASEADDR);

}

FebInterface::~FebInterface(){
  delete ll;
  if(feb_numb) delete [] feb_numb;
  delete [] send_data_raw;
  delete [] recv_data_raw;
}

void FebInterface::SendCompleteList(unsigned int n,unsigned int* list){
  if(feb_numb) delete [] feb_numb;
  nfebs = n;
  feb_numb = new unsigned int [n];
  for(unsigned int i=0;i<n;i++) feb_numb[i] = list[i];
}

bool FebInterface::WriteTo(unsigned int ch){
  if(ch>0xfff) return 0;

  send_data_raw[0] = 0x8fff0000;
  if(ll->Write(4,send_data_raw)!=4) return 0;

  send_data_raw[0] = 0x90000000 | (ch<<16);
    if(ll->Write(4,send_data_raw)!=4) return 0;

  send_data_raw[0] = 0xc0000000;
    return ((send_ndata+1)*4==ll->Write((send_ndata+1)*4,send_data_raw));
}

bool FebInterface::ReadFrom(unsigned int ch, unsigned int ntrys){
  if(ch>=0xfff) return 0;

  recv_data_raw[0] = 0xa0000000 | (ch<<16);
    ll->Write(4,recv_data_raw);
    usleep(20);
    
  recv_ndata=-1;
  for(unsigned int t=0;t<ntrys;t++){
    if((recv_ndata=ll->Read(recv_buffer_size*4,recv_data_raw)/4)>0){
      recv_ndata--;
      break;
    }
    usleep(1000);
  }

  return (recv_ndata>=0);
}



bool FebInterface::SetByteOrder(){

  send_data_raw[0] = 0x8fff0000;
  if(ll->Write(4,send_data_raw)!=4) return 0;

  send_ndata   = 2;
    send_data[0] = 0;
    send_data[1] = 0;
    
  unsigned int dst = 0xff;
  for(unsigned int i=0;i<nfebs;i++) dst = (dst | feb_numb[i]);
  bool passed = WriteTo(dst);

  return passed;
}


bool FebInterface::ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int& value_read){
  return ReadRegisters(sub_num,1,&reg_num,&value_read);
}


bool FebInterface::ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read){
  //here   cout<<"Reading Register ...."<<endl;

  nreads &= 0x3ff;
  if(!nreads||nreads>send_buffer_size-2) return 0;

  send_ndata   = nreads+2;
  send_data[0] = 0x20000000 | nreads << 14;
  
  for(unsigned int i=0;i<nreads;i++) send_data[i+1]=reg_nums[i];
  send_data[nreads+1] = 0;

  if(!WriteTo(sub_num)||!ReadFrom(sub_num)||recv_ndata!=int(nreads+2)) return 0;

  for(unsigned int i=0;i<nreads;i++) values_read[i] = recv_data[i+1];

  return 1;
}

bool FebInterface::WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, bool wait_on, unsigned int wait_on_address){
  return WriteRegisters(sub_num,1,&reg_num,&value,&wait_on,&wait_on_address);
}

bool FebInterface::WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, bool* wait_ons, unsigned int* wait_on_addresses){

  nwrites &= 0x3ff; //10 bits
  if(!nwrites||2*nwrites>send_buffer_size-2) return 0;

  //cout<<"Write register : "<<this<<"  "<<s_num<<" "<<nwrites<<" "<<reg_nums<<" "<<values<<" "<<wait_ons<<" "<<wait_on_addresses<<endl;
  send_ndata               = 2*nwrites+2;
    send_data[0]           = 0x80000000 | nwrites << 14;
    send_data[2*nwrites+1] = 0;

    for(unsigned int i=0;i<nwrites;i++) send_data[2*i+1] = 0x3fff&reg_nums[i];
    for(unsigned int i=0;i<nwrites;i++) send_data[2*i+2] = values[i];
    // wait on busy data(28), address of busy flag data(27 downto 14)
    if(wait_ons&&wait_on_addresses) for(unsigned int i=0;i<nwrites;i++) send_data[2*i+1] |= (wait_ons[i]<<28 | (0x3fff&wait_on_addresses[i])<<14);

    if(!WriteTo(sub_num)) return 0;

  return 1;
}


bool FebInterface::WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values){
   // -1 means write to all
  
  mem_num       &= 0x3f;
  start_address &= 0x3fff;
  nwrites       &= 0x3ff;
  if(!nwrites||nwrites>send_buffer_size-2) {cout<<"error herer: nwrites:"<<nwrites<<endl;return 0;}//*d-1026

  send_ndata           =  nwrites+2;//*d-1025
  send_data[0]         = 0xc0000000 | mem_num << 24 | nwrites << 14 | start_address; //cmd -> write to memory, nwrites, mem number, start address
  send_data[nwrites+1] = 0;
  for(unsigned int i=0;i<nwrites;i++) send_data[i+1] = values[i];


  if(!WriteTo(sub_num)) return 0;

  return 1;
}



