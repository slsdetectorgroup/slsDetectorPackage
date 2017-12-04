 
/**
 * @author Ian Johnson
 * @version 1.0
 */



//#include <iostream>
//#include <iomanip>
//#include <unistd.h>
//#include <string.h>
//#include <sys/mman.h>
//#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xparameters.h"

#include "FebInterface.h"



	struct LocalLinkInterface ll_local,* ll;

  unsigned int  Feb_Interface_nfebs;
  unsigned int* Feb_Interface_feb_numb;

  int           Feb_Interface_send_ndata;
  unsigned int  Feb_Interface_send_buffer_size;
  unsigned int* Feb_Interface_send_data_raw;
  unsigned int* Feb_Interface_send_data;

  int           Feb_Interface_recv_ndata;
  unsigned int  Feb_Interface_recv_buffer_size;
  unsigned int* Feb_Interface_recv_data_raw;
  unsigned int* Feb_Interface_recv_data;


void Feb_Interface_FebInterface(){
	ll = &ll_local;
	Feb_Interface_nfebs    = 0;
	Feb_Interface_feb_numb = 0;

	Feb_Interface_send_ndata = 0;
	Feb_Interface_send_buffer_size = 1026;
	Feb_Interface_send_data_raw = malloc((Feb_Interface_send_buffer_size+1) * sizeof(unsigned int));
	Feb_Interface_send_data     = &Feb_Interface_send_data_raw[1];

	Feb_Interface_recv_ndata = 0;
	Feb_Interface_recv_buffer_size = 1026;
	Feb_Interface_recv_data_raw = malloc((Feb_Interface_recv_buffer_size+1) * sizeof(unsigned int));
	Feb_Interface_recv_data     = &Feb_Interface_recv_data_raw[1];

  Local_LocalLinkInterface1(ll,XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_RIGHT_BASEADDR);

}



void Feb_Interface_SendCompleteList(unsigned int n,unsigned int* list){
	unsigned int i;
  if(Feb_Interface_feb_numb) free(Feb_Interface_feb_numb);
  Feb_Interface_nfebs = n;
  Feb_Interface_feb_numb = malloc(n * sizeof(unsigned int));
  for(i=0;i<n;i++) Feb_Interface_feb_numb[i] = list[i];
}

int Feb_Interface_WriteTo(unsigned int ch){
  if(ch>0xfff) return 0;

#ifdef MARTIN
  cprintf(YELLOW, "FIW ch %d\n", ch);
#endif

  Feb_Interface_send_data_raw[0] = 0x8fff0000;
  if(Local_Write(ll,4,Feb_Interface_send_data_raw)!=4) return 0;

  Feb_Interface_send_data_raw[0] = 0x90000000 | (ch<<16);
    if(Local_Write(ll,4,Feb_Interface_send_data_raw)!=4) return 0;

    Feb_Interface_send_data_raw[0] = 0xc0000000;
    return ((Feb_Interface_send_ndata+1)*4==Local_Write(ll,(Feb_Interface_send_ndata+1)*4,Feb_Interface_send_data_raw));
}

int Feb_Interface_ReadFrom(unsigned int ch, unsigned int ntrys){
	unsigned int t;
  if(ch>=0xfff) return 0;

  Feb_Interface_recv_data_raw[0] = 0xa0000000 | (ch<<16);
    Local_Write(ll,4,Feb_Interface_recv_data_raw);
    usleep(20);
    
    Feb_Interface_recv_ndata=-1;
  for(t=0;t<ntrys;t++){
    if((Feb_Interface_recv_ndata=Local_Read(ll,Feb_Interface_recv_buffer_size*4,Feb_Interface_recv_data_raw)/4)>0){
    	Feb_Interface_recv_ndata--;
      break;
    }
    usleep(1000);
  }

  return (Feb_Interface_recv_ndata>=0);
}



int Feb_Interface_SetByteOrder(){
	Feb_Interface_send_data_raw[0] = 0x8fff0000;
  if(Local_Write(ll,4,Feb_Interface_send_data_raw)!=4) return 0;
  Feb_Interface_send_ndata   = 2;
  Feb_Interface_send_data[0] = 0;
  Feb_Interface_send_data[1] = 0;
  unsigned int i;
  unsigned int dst = 0xff;
  for(i=0;i<Feb_Interface_nfebs;i++) dst = (dst | Feb_Interface_feb_numb[i]);
  int passed = Feb_Interface_WriteTo(dst);

  return passed;
}


int Feb_Interface_ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int* value_read){
  return Feb_Interface_ReadRegisters(sub_num,1,&reg_num,value_read);
}


int Feb_Interface_ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read){
  //here   cout<<"Reading Register ...."<<endl;
	unsigned int i;
  nreads &= 0x3ff;
  if(!nreads||nreads>Feb_Interface_send_buffer_size-2) return 0;

  Feb_Interface_send_ndata   = nreads+2;
  Feb_Interface_send_data[0] = 0x20000000 | nreads << 14;

  for(i=0;i<nreads;i++) Feb_Interface_send_data[i+1]=reg_nums[i];
  Feb_Interface_send_data[nreads+1] = 0;

  if(!Feb_Interface_WriteTo(sub_num)||!Feb_Interface_ReadFrom(sub_num,20)||Feb_Interface_recv_ndata!=(int)(nreads+2)) return 0;

  for(i=0;i<nreads;i++) values_read[i] = Feb_Interface_recv_data[i+1];

  return 1;
}

int Feb_Interface_WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, int wait_on, unsigned int wait_on_address){
  return Feb_Interface_WriteRegisters(sub_num,1,&reg_num,&value,&wait_on,&wait_on_address);
}

int Feb_Interface_WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, int* wait_ons, unsigned int* wait_on_addresses){
	unsigned int i;
  nwrites &= 0x3ff; //10 bits
  if(!nwrites||2*nwrites>Feb_Interface_send_buffer_size-2) return 0;

  //cout<<"Write register : "<<this<<"  "<<s_num<<" "<<nwrites<<" "<<reg_nums<<" "<<values<<" "<<wait_ons<<" "<<wait_on_addresses<<endl;
  Feb_Interface_send_ndata               = 2*nwrites+2;
  Feb_Interface_send_data[0]           = 0x80000000 | nwrites << 14;
  Feb_Interface_send_data[2*nwrites+1] = 0;

    for(i=0;i<nwrites;i++) Feb_Interface_send_data[2*i+1] = 0x3fff&reg_nums[i];
    for(i=0;i<nwrites;i++) Feb_Interface_send_data[2*i+2] = values[i];
    // wait on busy data(28), address of busy flag data(27 downto 14)
    if(wait_ons&&wait_on_addresses) for(i=0;i<nwrites;i++) Feb_Interface_send_data[2*i+1] |= (wait_ons[i]<<28 | (0x3fff&wait_on_addresses[i])<<14);

    if(!Feb_Interface_WriteTo(sub_num)) return 0;

  return 1;
}

int Feb_Interface_WriteMemoryInLoops(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values){
	unsigned int max_single_packet_size = 352;
	int passed=1;
	unsigned int n_to_send  = max_single_packet_size;
	unsigned int ndata_sent = 0;
	unsigned int ndata_countdown = nwrites;
	while(ndata_countdown>0){
		n_to_send = ndata_countdown<max_single_packet_size ? ndata_countdown:max_single_packet_size;
		if(!Feb_Interface_WriteMemory(sub_num,mem_num,start_address,n_to_send,&(values[ndata_sent]))){passed=0; break;}
		ndata_countdown-=n_to_send;
		ndata_sent     +=n_to_send;
		start_address  +=n_to_send;
		usleep(500);//500 works
	}
	return passed;
}

int Feb_Interface_WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values){
   // -1 means write to all
	unsigned int i;
  mem_num       &= 0x3f;
  start_address &= 0x3fff;
  nwrites       &= 0x3ff;
  if(!nwrites||nwrites>Feb_Interface_send_buffer_size-2) {printf("error herer: nwrites:%d\n",nwrites);return 0;}//*d-1026

  Feb_Interface_send_ndata           =  nwrites+2;//*d-1026
  Feb_Interface_send_data[0]         = 0xc0000000 | mem_num << 24 | nwrites << 14 | start_address; //cmd -> write to memory, nwrites, mem number, start address
  Feb_Interface_send_data[nwrites+1] = 0;
  for(i=0;i<nwrites;i++) Feb_Interface_send_data[i+1] = values[i];


  if(!Feb_Interface_WriteTo(sub_num)) return 0;

  return 1;
}



