 
/**
 * @author Ian Johnson
 * @version 1.0
 */



/*#include <iostream>
#include <iomanip>*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "xparameters.h"
#include "Feb.h"



void Feb_Feb(){

	Feb_nfebs    = 0;
	Feb_feb_numb = 0;

	Feb_send_ndata = 0;
	Feb_send_buffer_size = 1026;
	Feb_send_data_raw = malloc((Feb_send_buffer_size+1)*sizeof(int));
	Feb_send_data     = &Feb_send_data_raw[1];

	Feb_recv_ndata = 0;
	Feb_recv_buffer_size = 1026;
	Feb_recv_data_raw = malloc((Feb_recv_buffer_size+1)*sizeof(int));
	Feb_recv_data     = &Feb_recv_data_raw[1];

	Local_LocalLinkInterface1(ll,XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_RIGHT_BASEADDR);

}
/*
~Feb(){
  delete ll;
  if(feb_numb) delete [] feb_numb;
  delete [] send_data_raw;
  delete [] recv_data_raw;
}
*/

void Feb_SendCompleteFebList(unsigned int n,unsigned int* list){
	unsigned int i;
	if(Feb_feb_numb) free(Feb_feb_numb);
  Feb_nfebs = n;
  Feb_feb_numb = malloc(n*sizeof(unsigned int));
  for(i=0;i<n;i++) Feb_feb_numb[i] = list[i];
}

int Feb_WriteTo(unsigned int ch){
  if(ch>0xfff) return 0;

  Feb_send_data_raw[0] = 0x90000000 | (ch<<16); //we
    if(Local_Write(ll,4,Feb_send_data_raw)!=4) return 0;

    Feb_send_data_raw[0] = 0xc0000000; //data
    return 1;//((Feb_send_ndata+1)*4==Local_Write(ll,(Feb_send_ndata+1)*4,Feb_send_data_raw));
}

int Feb_ReadFrom(unsigned int ch, unsigned int ntrys){
	unsigned int t;
  if(ch>=0xfff) return 0;

  Feb_recv_data_raw[0] = 0xa0000000 | (ch<<16); //read data
  Local_Write(ll,4,Feb_recv_data_raw);
    usleep(20);
    
    Feb_recv_ndata=-1;
  for(t=0;t<ntrys;t++){
    if((Feb_recv_ndata=Local_Read(ll,Feb_recv_buffer_size*4,Feb_recv_data_raw)/4)>0){
    	Feb_recv_ndata--;
      break;
    }
    printf("\t Read try number: %d\n",t);
    usleep(1000);
  }


  return (Feb_recv_ndata>=0);
}

void Feb_PrintData(){
	int i;
  printf("Sent data: %d\n",Feb_send_ndata);
  for(i=0;i<Feb_send_ndata;i++) printf("\t%d)%d  (0x%x)\n",i,Feb_send_data[i],Feb_send_data[i]);
  printf("Receive data: %d\n",Feb_recv_ndata);
  for(i=0;i<Feb_recv_ndata;i++) printf("\t%d)%d   (0x%x)\n",i,Feb_recv_data[i],Feb_recv_data[i]);
  printf("\n\n");
}


int Feb_CheckHeader(unsigned int valid_bit_mask, int print_error_info){

  int header_returned_is_ok = (Feb_send_data[0] & valid_bit_mask)==(Feb_recv_data[0] & valid_bit_mask);

  if(print_error_info && !header_returned_is_ok){
	  printf("Error: Command received not the same as command recieved.\n");
	  printf("\t\t Header sent: %d (0x%x) received: %d (0x%x)\n",Feb_send_data[0], Feb_send_data[0], Feb_recv_data[0], Feb_recv_data[0]);
    if(Feb_send_ndata>1&&Feb_recv_ndata>1){
    	printf("\t\t Tail sent:   %d (0x%x) receiver: %d (0x%x)\n",Feb_send_data[Feb_send_ndata-1],Feb_send_data[Feb_send_ndata-1],Feb_recv_data[Feb_recv_ndata-1],Feb_recv_data[Feb_recv_ndata-1]);
    }else{
    	printf("Error printing tail, too little data nsent = 0x%x, nrecv = 0x%x.\n",Feb_send_ndata, Feb_recv_ndata);
    }
    Feb_PrintData();
  }
  return header_returned_is_ok;
}


int Feb_CheckTail(unsigned int valid_bit_mask){
  if(Feb_send_ndata<=1&&Feb_recv_ndata<=1){
    printf("Error checking tail, too little data nsent = %d, nrecv = %d.\n",Feb_send_ndata, Feb_recv_ndata);
    return 0;
  }

  unsigned int the_tail = Feb_recv_data[Feb_recv_ndata-1]&valid_bit_mask;
  if(the_tail!=0){
    printf("Error returned in tail:  0x%x (%d)\n",the_tail,the_tail);
    if(the_tail&0x10000000) printf("\t\tBusy flag address error.\n");
    if(the_tail&0x20000000) printf("\t\tRead register address error.\n");
    if(the_tail&0x40000000) printf("\t\tWrite register address error.\n");
    if(the_tail&0x80000000) printf("\t\tBram number error.\n");
    if(the_tail&0x08000000) printf("\t\tFifo to read from error.\n");
    if(the_tail&0x3ff)      printf("\t\tNumber of data send error.\n");
    return 0; //error
  }

  return 1;
}  


int Feb_CheckCommunication(){
	Feb_send_data_raw[0] = 0x8fff0000; //rst-all serial coms and lls
  if(Local_Write(ll,4,Feb_send_data_raw)!=4) return 0;
  
  printf("CheckingCommunication ....\n");
  while((Local_Read(ll,Feb_recv_buffer_size*4,Feb_recv_data_raw)/4)>0) printf("\t) Cleanning buffer ...\n");

  return Feb_SetByteOrder();
}


int Feb_SetByteOrder(){

	unsigned int i;
	Feb_send_ndata   = 2;
	Feb_send_data[0] = 0; //header
	Feb_send_data[1] = 0; //tail
    
  unsigned int dst = 0xff;
  for( i=0;i<Feb_nfebs;i++) dst = (dst | Feb_feb_numb[i]); //get sub dst bits (left right in this case)
  int passed = Feb_WriteTo(dst);

  for(i=0;i<Feb_nfebs;i++){
    printf("\t%d) Set Byte Order ..............  ",i);
    unsigned int current_passed = Feb_ReadFrom(Feb_feb_numb[i],20)&&(Feb_recv_ndata==2)&&Feb_CheckHeader(0xffffffff,1);
    if(current_passed) printf("passed.\n");
    else               printf("failed.\n");
    passed&=current_passed;
  }
  printf("\n");

  return passed;
}

/* feb_ needed
int Feb_CheckSubNumber(unsigned int Feb_sub_num){
  if(sub_num>=nfebs){
    cout<<"Error invalid sub number "<<sub_num<<" must be less than "<<nfebs<<"."<<endl;
    return 0;
  }
  return 1;
}

int Feb_SetStartOnEndOnFebs(int sub_num_s, unsigned int& start_on, unsigned int& end_on){
   // -1 means write to all

  if(sub_num_s<=-2){
    cout<<"Error bad subnumber "<<sub_num_s<<"."<<endl;
    return 0;
  }

  start_on = sub_num_s!=-1 ? sub_num_s : 0;
  end_on   = sub_num_s!=-1 ? sub_num_s : nfebs - 1;

  return Feb_CheckSubNumber(start_on);
}  
*/


/*
int Feb_ReadRegister(unsigned int Feb_sub_num, unsigned int Feb_reg_num,unsigned int& Feb_value_read){
  return Feb_ReadRegisters(Feb_sub_num,1,&Feb_reg_num,&Feb_value_read);
}
*/
int Feb_ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int* value_read){
  return Feb_ReadRegisters(sub_num,1,&reg_num,value_read);
}

int Feb_ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read){
  //here   cout<<"Reading Register ...."<<endl;
unsigned int i;
	nreads &= 0x3ff; //10 bits
  if(!nreads||nreads>Feb_send_buffer_size-2) return 0;

  Feb_send_ndata   = nreads+2;
  Feb_send_data[0] = 0x20000000 | nreads << 14; //cmd -> read "00" , nreads
  
  for(i=0;i<nreads;i++) Feb_send_data[i+1]=reg_nums[i];
  Feb_send_data[nreads+1] = 0; //tail

  if(!Feb_WriteTo(sub_num)||!Feb_ReadFrom(sub_num,20)||Feb_recv_ndata!=(int)(nreads+2)||!Feb_CheckHeader(0xffffffff,1)||!Feb_CheckTail(0xffffffff)){
	  Feb_PrintData();
    printf("Error reading register.\n");
    return 0;
  }

  for(i=0;i<nreads;i++) values_read[i] = Feb_recv_data[i+1];

  return 1;
}

int Feb_WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, int wait_on, unsigned int wait_on_address){
  return Feb_WriteRegisters(sub_num,1,&reg_num,&value,&wait_on,&wait_on_address);
}

int Feb_WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, int* wait_ons, unsigned int* wait_on_addresses){
unsigned int i;
   // sub_num == 0xfff means write to all

	nwrites &= 0x3ff; //10 bits
  if(!nwrites||nwrites>Feb_send_buffer_size-2) return 0;

  //cout<<"Write register : "<<this<<"  "<<s_num<<" "<<nwrites<<" "<<reg_nums<<" "<<values<<" "<<wait_ons<<" "<<wait_on_addresses<<endl;
  Feb_send_ndata             = 2*nwrites+2;
  Feb_send_data[0]           = 0x80000000 | nwrites << 14; //cmd -> write nwrites and how many
  Feb_send_data[2*nwrites+1] = 0; //tail

    for(i=0;i<nwrites;i++) Feb_send_data[2*i+1] = 0x3fff&reg_nums[i]; // register address data_in(13 downto 0)
    for(i=0;i<nwrites;i++) Feb_send_data[2*i+2] = values[i];          // value is data_in(31 downto 0)
    // wait on busy data(28), address of busy flag data(27 downto 14)
    if(wait_ons&&wait_on_addresses) for(i=0;i<nwrites;i++) Feb_send_data[2*i+1] |= (wait_ons[i]<<28 | (0x3fff&wait_on_addresses[i])<<14);

  if(!Feb_WriteTo(sub_num)){
    printf("%d) Error writing register(s).\n",sub_num);
    Feb_PrintData();
    return 0;
  }

  int passed        = 1;
  unsigned int  n    = (sub_num&0xff)==0xff ? Feb_nfebs    : 1;
  unsigned int* nums = (sub_num&0xff)==0xff ? Feb_feb_numb : &sub_num;
  for(i=0;i<n;i++){
    if((sub_num&0xf00&(nums[i]))==0) continue;
    if(!Feb_ReadFrom(nums[i],20)||Feb_recv_ndata!=2||!Feb_CheckHeader(0xffffffff,1)){
      printf("%d) Error writing register(s) response.\n",nums[i]);
      Feb_PrintData();
      passed = 0;
    }else{
    	passed = passed && Feb_CheckTail(0xffffffff);
    }
  }

  return passed;
}


int Feb_WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values){
   // -1 means write to all
  unsigned int i;
	mem_num       &= 0x3f;   //6 bits
	start_address &= 0x3fff; //14 bits
	nwrites       &= 0x3ff;  //10 bits
  if(!nwrites||nwrites>Feb_send_buffer_size-2) return 0;

  Feb_send_ndata           =  nwrites+2;
  Feb_send_data[0]         = 0xc0000000 | mem_num << 24 | nwrites << 14 | start_address; //cmd -> write to memory, nwrites, mem number, start address
  Feb_send_data[nwrites+1] = 0; //tail
  for(i=0;i<nwrites;i++) Feb_send_data[i+1] = values[i];


  if(!Feb_WriteTo(sub_num)){
    printf("%d)  Error writing memory.\n",sub_num);
    return 0;
  }

  int passed        = 1;
  unsigned int  n    = (sub_num&0xff)==0xff ? Feb_nfebs    : 1;
  unsigned int* nums = (sub_num&0xff)==0xff ? Feb_feb_numb : &sub_num;
  for(i=0;i<n;i++){
    if((sub_num&0xf00&(nums[i]))==0) continue;
    if(!Feb_ReadFrom(nums[i],20)||Feb_recv_ndata!=2||!Feb_CheckHeader(0xffffffff,1)){
      printf("%d)  Error writing memory response. \n",nums[i]);
      Feb_PrintData();
      passed = 0;
    }else{
    	passed = passed && Feb_CheckTail(0xffffffff);
    }
  }
    //  unsigned int  n    = sub_num==0xfff ? nfebs    : 1;
    //  unsigned int* nums = sub_num==0xfff ? feb_numb : &sub_num;
    //  for(unsigned int i=0;i<n;i++){

  return passed;
}



int Feb_Test(){//int sub_num_s, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values){
   // -1 means write to all
unsigned int i;
  unsigned int reg_nums[10]={0,1,2,3,1,2,3,1,2,3};
  
  printf("Test\n\n\n\n");

  unsigned int value = 0;
  for(i=0;i<10;i++){
	  Feb_WriteRegister(0xfff,reg_nums[i%10],i,0,0);
	  Feb_ReadRegister(256,reg_nums[i%10],&value);
    printf("%d  %d\n",i,value);
    Feb_ReadRegister(512,reg_nums[i%10],&value);
    printf("%d  %d\n",i,value);
    Feb_WriteMemory(0xfff,0,0,10,reg_nums);
  }

  return 0;
}

