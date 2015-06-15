
//Class initially from Gerd and was called mmap_test.c 

#ifndef LOCALLINKINTERFACE_H
#define LOCALLINKINTERFACE_H

#include "xfs_types.h"
#include "HardwareIO.h"
#include <sys/types.h>

#include "ansi.h"
#include <sys/mman.h>
#include <fcntl.h>

/*class LocalLinkInterface: public HardwareIO{ //*/


struct LocalLinkInterface{
  xfs_u32      ll_fifo_base;
  unsigned int ll_fifo_ctrl_reg;
};



  int Local_Init(struct LocalLinkInterface* ll,unsigned int ll_fifo_badr);
  int Local_Reset1(struct LocalLinkInterface* ll,unsigned int rst_mask);

  int Local_ctrl_reg_write_mask(struct LocalLinkInterface* ll,unsigned int mask, unsigned int val);
  void Local_llfifo_print_frame(struct LocalLinkInterface* ll,unsigned char* fbuff, int len);


  void Local_LocalLinkInterface1(struct LocalLinkInterface* ll,unsigned int ll_fifo_badr);
 /* virtual ~LocalLinkInterface();*/

  unsigned int Local_StatusVector(struct LocalLinkInterface* ll);
  int Local_Reset(struct LocalLinkInterface* ll);
  int  Local_Write(struct LocalLinkInterface* ll,unsigned int buffer_len, void *buffer);
  int  Local_Read(struct LocalLinkInterface* ll,unsigned int buffer_len, void *buffer);

  int  Local_Test(struct LocalLinkInterface* ll,unsigned int buffer_len, void *buffer);

  void Local_LocalLinkInterface(struct LocalLinkInterface* ll);

  /*
  int FiFoReset(unsigned int numb);
  int  FifoSend(unsigned int numb, unsigned int frame_len, void *buffer);
  int  FifoReceive(unsigned int numb, unsigned int frame_len, void *buffer);
  int  FifoTest(unsigned int numb,unsigned int send_len, char *send_str);
  */




#endif 
