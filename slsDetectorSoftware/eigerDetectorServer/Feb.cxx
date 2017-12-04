 
/**
 * @author Ian Johnson
 * @version 1.0
 */


//return reversed 1 means good, 0 means failed


#include <iostream>
#include <iomanip>
//#include <unistd.h>
//#include <string.h>
//#include <sys/mman.h>
//#include <fcntl.h>

#include "xparameters.h"

#include "Feb.h"

using namespace std;

Feb::Feb(){

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

Feb::~Feb(){
  delete ll;
  if(feb_numb) delete [] feb_numb;
  delete [] send_data_raw;
  delete [] recv_data_raw;
}

void Feb::SendCompleteFebList(unsigned int n,unsigned int* list){
  if(feb_numb) delete [] feb_numb;
  nfebs = n;
  feb_numb = new unsigned int [n];
  for(unsigned int i=0;i<n;i++) feb_numb[i] = list[i];
}

bool Feb::WriteTo(unsigned int ch){
  if(ch>0xfff) return 0;

  send_data_raw[0] = 0x90000000 | (ch<<16); //we
    if(ll->Write(4,send_data_raw)!=4) return 0;

  send_data_raw[0] = 0xc0000000; //data
    return ((send_ndata+1)*4==ll->Write((send_ndata+1)*4,send_data_raw));
}

bool Feb::ReadFrom(unsigned int ch, unsigned int ntrys){
  if(ch>=0xfff) return 0;

  recv_data_raw[0] = 0xa0000000 | (ch<<16); //read data
    ll->Write(4,recv_data_raw);
    usleep(20);
    
  recv_ndata=-1;
  for(unsigned int t=0;t<ntrys;t++){
    if((recv_ndata=ll->Read(recv_buffer_size*4,recv_data_raw)/4)>0){
      recv_ndata--;
      break;
    }
    cout<<"\t Read try number: "<<t<<endl;
    usleep(1000);
  }

  return (recv_ndata>=0);
}

void Feb::PrintData(){
  cout<<"Sent data: "<<send_ndata<<endl;
  for(int i=0;i<send_ndata;i++) cout<<"\t"<<i<<")"<<setw(14)<<send_data[i]<<"    ("<<hex<<send_data[i]<<")"<<dec<<endl;
  cout<<"Receive data: "<<recv_ndata<<endl;
  for(int i=0;i<recv_ndata;i++) cout<<"\t"<<i<<")"<<setw(14)<<recv_data[i]<<"    ("<<hex<<recv_data[i]<<")"<<dec<<endl;
  cout<<endl<<endl;
}


bool Feb::CheckHeader(unsigned int valid_bit_mask, bool print_error_info){

  bool header_returned_is_ok = (send_data[0] & valid_bit_mask)==(recv_data[0] & valid_bit_mask);

  if(print_error_info && !header_returned_is_ok){
    cout<<"Error: Command received not the same as command recieved."<<endl;
    cout<<"\t\t Header sent: "<<dec<<send_data[0]<<" ("<<hex<<send_data[0]<<")  recieved: "<<dec<<recv_data[0]<<" ("<<hex<<recv_data[0]<<")"<<dec<<endl;
    if(send_ndata>1&&recv_ndata>1){
      cout<<"\t\t Tail sent:   "<<dec<<send_data[send_ndata-1]<<" ("<<hex<<send_data[send_ndata-1]<<")  recieved: "<<dec<<recv_data[recv_ndata-1]<<" ("<<hex<<recv_data[recv_ndata-1]<<")"<<dec<<endl;
    }else{
      cout<<"Error printing tail, too little data nsent = "<<send_ndata<<", nrecv = "<<recv_ndata<<"."<<endl;
    }
    PrintData();
  }
  return header_returned_is_ok;
}


bool Feb::CheckTail(unsigned int valid_bit_mask){
  if(send_ndata<=1&&recv_ndata<=1){
    cout<<"Error checking tail, too little data nsent = "<<send_ndata<<", nrecv = "<<recv_ndata<<"."<<endl;
    return 0;
  }

  unsigned int the_tail = recv_data[recv_ndata-1]&valid_bit_mask;
  if(the_tail!=0){
    cout<<"Error returned in tail:  "<<hex<<the_tail<<"  "<<dec<<"("<<the_tail<<")"<<endl;
    if(the_tail&0x10000000) cout<<"\t\tBusy flag address error."<<endl;
    if(the_tail&0x20000000) cout<<"\t\tRead register address error."<<endl;
    if(the_tail&0x40000000) cout<<"\t\tWrite register address error."<<endl;
    if(the_tail&0x80000000) cout<<"\t\tBram number error."<<endl;
    if(the_tail&0x08000000) cout<<"\t\tFifo to read from error."<<endl;
    if(the_tail&0x3ff)      cout<<"\t\tNumber of data send error."<<endl;
    return 0; //error
  }

  return 1;
}  


bool Feb::CheckCommunication(){
  send_data_raw[0] = 0x8fff0000; //rst-all serial coms and lls
  if(ll->Write(4,send_data_raw)!=4) return 0;
  
  cout<<"Feb::CheckingCommunication ...."<<endl;
  while((ll->Read(recv_buffer_size*4,recv_data_raw)/4)>0) cout<<"\t) Cleanning buffer ..."<<endl;

  return SetByteOrder();
}


bool Feb::SetByteOrder(){

  send_ndata   = 2;
    send_data[0] = 0; //header
    send_data[1] = 0; //tail
    
  unsigned int dst = 0xff;
  for(unsigned int i=0;i<nfebs;i++) dst = (dst | feb_numb[i]); //get sub dst bits (left right in this case)
  bool passed = WriteTo(dst);

  for(unsigned int i=0;i<nfebs;i++){
    cout<<"\t"<<i<<") Set Byte Order ..............  ";
    unsigned int current_passed = ReadFrom(feb_numb[i])&&(recv_ndata==2)&&CheckHeader();
    if(current_passed) cout<<"passed."<<endl; 
    else               cout<<"failed."<<endl;
    passed&=current_passed;
  }
  cout<<endl; 

  return passed;
}

/*
bool Feb::CheckSubNumber(unsigned int sub_num){
  if(sub_num>=nfebs){
    cout<<"Error invalid sub number "<<sub_num<<" must be less than "<<nfebs<<"."<<endl;
    return 0;
  }
  return 1;
}

bool Feb::SetStartOnEndOnFebs(int sub_num_s, unsigned int& start_on, unsigned int& end_on){
   // -1 means write to all

  if(sub_num_s<=-2){
    cout<<"Error bad subnumber "<<sub_num_s<<"."<<endl;
    return 0;
  }

  start_on = sub_num_s!=-1 ? sub_num_s : 0;
  end_on   = sub_num_s!=-1 ? sub_num_s : nfebs - 1;

  return CheckSubNumber(start_on);
}  
*/

bool Feb::ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int& value_read){
  return ReadRegisters(sub_num,1,&reg_num,&value_read);
}


bool Feb::ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read){
  //here   cout<<"Reading Register ...."<<endl;

  nreads &= 0x3ff; //10 bits
  if(!nreads||nreads>send_buffer_size-2) return 0;

  send_ndata   = nreads+2;
  send_data[0] = 0x20000000 | nreads << 14; //cmd -> read "00" , nreads
  
  for(unsigned int i=0;i<nreads;i++) send_data[i+1]=reg_nums[i];
  send_data[nreads+1] = 0; //tail

  if(!WriteTo(sub_num)||!ReadFrom(sub_num)||recv_ndata!=int(nreads+2)||!CheckHeader()||!CheckTail()){
    PrintData();
    cout<<"Error reading register."<<endl;
    return 0;
  }

  for(unsigned int i=0;i<nreads;i++) values_read[i] = recv_data[i+1];

  return 1;
}

bool Feb::WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, bool wait_on, unsigned int wait_on_address){
  return WriteRegisters(sub_num,1,&reg_num,&value,&wait_on,&wait_on_address);
}

bool Feb::WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, bool* wait_ons, unsigned int* wait_on_addresses){

   // sub_num == 0xfff means write to all

  nwrites &= 0x3ff; //10 bits
  if(!nwrites||nwrites>send_buffer_size-2) return 0;

  //cout<<"Write register : "<<this<<"  "<<s_num<<" "<<nwrites<<" "<<reg_nums<<" "<<values<<" "<<wait_ons<<" "<<wait_on_addresses<<endl;
  send_ndata             = 2*nwrites+2;
    send_data[0]           = 0x80000000 | nwrites << 14; //cmd -> write nwrites and how many
    send_data[2*nwrites+1] = 0; //tail

    for(unsigned int i=0;i<nwrites;i++) send_data[2*i+1] = 0x3fff&reg_nums[i]; // register address data_in(13 downto 0)
    for(unsigned int i=0;i<nwrites;i++) send_data[2*i+2] = values[i];          // value is data_in(31 downto 0)
    // wait on busy data(28), address of busy flag data(27 downto 14)
    if(wait_ons&&wait_on_addresses) for(unsigned int i=0;i<nwrites;i++) send_data[2*i+1] |= (wait_ons[i]<<28 | (0x3fff&wait_on_addresses[i])<<14);

  if(!WriteTo(sub_num)){
    cout<<sub_num<<") Error writing register(s)."<<endl;
    PrintData();
    return 0;
  }

  bool passed        = 1;
  unsigned int  n    = (sub_num&0xff)==0xff ? nfebs    : 1;
  unsigned int* nums = (sub_num&0xff)==0xff ? feb_numb : &sub_num;
  for(unsigned int i=0;i<n;i++){
    if((sub_num&0xf00&(nums[i]))==0) continue;
    if(!ReadFrom(nums[i])||recv_ndata!=2||!CheckHeader()){
      cout<<nums[i]<<") Error writing register(s) response."<<endl;
      PrintData();
      passed = 0;
    }else{
      passed = passed && CheckTail();
    }
  }

  return passed;
}


bool Feb::WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values){
   // -1 means write to all
  
  mem_num       &= 0x3f;   //6 bits
  start_address &= 0x3fff; //14 bits
  nwrites       &= 0x3ff;  //10 bits
  if(!nwrites||nwrites>send_buffer_size-2) return 0;

  send_ndata           =  nwrites+2;
  send_data[0]         = 0xc0000000 | mem_num << 24 | nwrites << 14 | start_address; //cmd -> write to memory, nwrites, mem number, start address
  send_data[nwrites+1] = 0; //tail
  for(unsigned int i=0;i<nwrites;i++) send_data[i+1] = values[i];


  if(!WriteTo(sub_num)){
    cout<<sub_num<<")  Error writing memory."<<endl;
    return 0;
  }

  bool passed        = 1;
  unsigned int  n    = (sub_num&0xff)==0xff ? nfebs    : 1;
  unsigned int* nums = (sub_num&0xff)==0xff ? feb_numb : &sub_num;
  for(unsigned int i=0;i<n;i++){
    if((sub_num&0xf00&(nums[i]))==0) continue;
    if(!ReadFrom(nums[i])||recv_ndata!=2||!CheckHeader()){
      cout<<nums[i]<<")  Error writing memory response."<<endl;
      PrintData();
      passed = 0;
    }else{
      passed = passed && CheckTail();
    }
  }
    //  unsigned int  n    = sub_num==0xfff ? nfebs    : 1;
    //  unsigned int* nums = sub_num==0xfff ? feb_numb : &sub_num;
    //  for(unsigned int i=0;i<n;i++){

  return passed;
}



bool Feb::Test(){//int sub_num_s, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values){
   // -1 means write to all

  unsigned int reg_nums[10]={0,1,2,3,1,2,3,1,2,3};
  
  cout<<"Test"<<endl<<endl<<endl<<endl;

  unsigned int value = 0;
  for(unsigned int i=0;i<10;i++){
    WriteRegister(0xfff,reg_nums[i%10],i);
    ReadRegister(256,reg_nums[i%10],value); 
    cout<<i<<"  "<<value<<endl;
    ReadRegister(512,reg_nums[i%10],value);
    cout<<i<<"  "<<value<<endl;
    WriteMemory(0xfff,0,0,10,reg_nums);
  }

  return 0;
}

