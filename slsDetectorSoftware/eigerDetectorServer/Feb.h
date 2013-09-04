
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef FEB_H
#define FEB_H

#include "LocalLinkInterface.h"

class Feb{ //

 private:
  LocalLinkInterface* ll;
  
  unsigned int  nfebs;
  unsigned int* feb_numb;

  int           send_ndata;
  unsigned int  send_buffer_size;
  unsigned int* send_data_raw;
  unsigned int* send_data;

  int           recv_ndata;
  unsigned int  recv_buffer_size;
  unsigned int* recv_data_raw;
  unsigned int* recv_data;

  bool WriteTo(unsigned int ch);
  bool ReadFrom(unsigned int ch, unsigned int ntrys=20);
  bool CheckHeader(unsigned int valid_bit_mask=0xffffffff, bool print_error_info=1);
  bool CheckTail(unsigned int valid_bit_mask=0xffffffff);


  bool SetByteOrder();
  //bool CheckSubNumber(unsigned int sub_num);
  //bool SetStartOnEndOnFebs(int sub_num_s, unsigned int& start_on, unsigned int& end_on);
  void PrintData();

 public:
  Feb();
  virtual ~Feb();

  void SendCompleteFebList(unsigned int n,unsigned int* list);
  bool CheckCommunication();

  bool ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int& value_read);
  bool ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read);

  bool WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, bool wait_on=0, unsigned int wait_on_address=0);
  bool WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, bool* wait_ons=0, unsigned int* wait_on_addresses=0);

  bool WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values);
  
  bool Test();

};


#endif 
