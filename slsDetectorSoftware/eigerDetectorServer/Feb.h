
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef FEB_H
#define FEB_H

#include "LocalLinkInterface.h"


 struct LocalLinkInterface* ll;
  
  unsigned int  Feb_nfebs;
  unsigned int* Feb_feb_numb;

  int           Feb_send_ndata;
  unsigned int  Feb_send_buffer_size;
  unsigned int* Feb_send_data_raw;
  unsigned int* Feb_send_data;

  int           Feb_recv_ndata;
  unsigned int  Feb_recv_buffer_size;
  unsigned int* Feb_recv_data_raw;
  unsigned int* Feb_recv_data;

  int Feb_WriteTo(unsigned int ch);
  /*int Feb_ReadFrom(unsigned int Feb_ch, unsigned int Feb_ntrys=20);*/
  int Feb_ReadFrom(unsigned int ch, unsigned int ntrys);
 /* int Feb_CheckHeader(unsigned int Feb_valid_bit_mask=0xffffffff, int Feb_print_error_info=1);*/
  int Feb_CheckHeader(unsigned int valid_bit_mask, int print_error_info);
  /*int Feb_CheckTail(unsigned int Feb_valid_bit_mask=0xffffffff);*/
  int Feb_CheckTail(unsigned int valid_bit_mask);

  int Feb_SetByteOrder();
  //int Feb_CheckSubNumber(unsigned int Feb_sub_num);
  //int Feb_SetStartOnEndOnFebs(int Feb_sub_num_s, unsigned int& Feb_start_on, unsigned int& Feb_end_on);
  void Feb_PrintData();


  void Feb_Feb();
  /*virtual ~Feb();*/
  void Feb_SendCompleteFebList(unsigned int n,unsigned int* list);
  int Feb_CheckCommunication();
  /*int Feb_ReadRegister(unsigned int Feb_sub_num, unsigned int Feb_reg_num,unsigned int& Feb_value_read);*/
  int Feb_ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int* value_read);
  int Feb_ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read);
  /*int WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, int wait_on=0, unsigned int wait_on_address=0);*/
  int Feb_WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, int wait_on, unsigned int wait_on_address);
  /*int WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, int* wait_ons=0, unsigned int* wait_on_addresses=0);*/
  int Feb_WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, int* wait_ons, unsigned int* wait_on_addresses);
  int Feb_WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values);
  int Feb_Test();



#endif 
