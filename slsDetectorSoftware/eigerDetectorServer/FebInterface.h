
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef FEBINTERFACE_H
#define FEBINTERFACE_H

#include "LocalLinkInterface.h"

class FebInterface{ //

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


 public:
  FebInterface();
  virtual ~FebInterface();

  void SendCompleteList(unsigned int n,unsigned int* list);
  bool SetByteOrder();

  bool ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int& value_read);
  bool ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read);

  bool WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, bool wait_on=0, unsigned int wait_on_address=0);
  bool WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, bool* wait_ons=0, unsigned int* wait_on_addresses=0);

  bool WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values);
  
};


#endif 
