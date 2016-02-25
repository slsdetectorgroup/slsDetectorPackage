
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef FEBINTERFACE_H
#define FEBINTERFACE_H

#include "LocalLinkInterface.h"






  int Feb_Interface_WriteTo(unsigned int ch);
  /*int Feb_Interface_ReadFrom(unsigned int ch, unsigned int ntrys=20);*/
  int Feb_Interface_ReadFrom(unsigned int ch, unsigned int ntrys);

  void Feb_Interface_FebInterface();


  void Feb_Interface_SendCompleteList(unsigned int n,unsigned int* list);
  int Feb_Interface_SetByteOrder();

  int Feb_Interface_ReadRegister(unsigned int sub_num, unsigned int reg_num,unsigned int* value_read);
  int Feb_Interface_ReadRegisters(unsigned int sub_num, unsigned int nreads, unsigned int* reg_nums,unsigned int* values_read);
  /*int Feb_Interface_WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, int wait_on=0, unsigned int wait_on_address=0);*/
  int Feb_Interface_WriteRegister(unsigned int sub_num, unsigned int reg_num,unsigned int value, int wait_on, unsigned int wait_on_address);
  /*int Feb_Interface_WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, int* wait_ons=0, unsigned int* wait_on_addresses=0);*/
  int Feb_Interface_WriteRegisters(unsigned int sub_num, unsigned int nwrites, unsigned int* reg_nums, unsigned int* values, int* wait_ons, unsigned int* wait_on_addresses);

  //mem_num is 0 for trimbit BRAM and 1 for rate correction BRAM
  int Feb_Interface_WriteMemoryInLoops(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values);

  int Feb_Interface_WriteMemory(unsigned int sub_num, unsigned int mem_num, unsigned int start_address, unsigned int nwrites, unsigned int *values);
  



#endif 
