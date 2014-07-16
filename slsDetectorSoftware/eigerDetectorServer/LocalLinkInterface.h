
//Class initially from Gerd and was called mmap_test.c 

#ifndef LOCALLINKINTERFACE_H
#define LOCALLINKINTERFACE_H

#include "xfs_types.h"
#include "HardwareIO.h"

class LocalLinkInterface: public HardwareIO{ //

 private:

  xfs_u32      ll_fifo_base;
  unsigned int ll_fifo_ctrl_reg;

  bool Init(unsigned int ll_fifo_badr);
  bool Reset(unsigned int rst_mask);

  bool ctrl_reg_write_mask(unsigned int mask, unsigned int val);
  void llfifo_print_frame(unsigned char* fbuff, int len);

 public:
  LocalLinkInterface(unsigned int ll_fifo_badr);
  virtual ~LocalLinkInterface();

  unsigned int StatusVector();
  bool Reset();
  int  Write(unsigned int buffer_len, void *buffer);
  int  Read(unsigned int buffer_len, void *buffer);

  int  Test(unsigned int buffer_len, void *buffer);

  LocalLinkInterface();
  int InitNewMemory (unsigned int addr, int ifg);

  /*
  bool FiFoReset(unsigned int numb);
  int  FifoSend(unsigned int numb, unsigned int frame_len, void *buffer);
  int  FifoReceive(unsigned int numb, unsigned int frame_len, void *buffer);
  int  FifoTest(unsigned int numb,unsigned int send_len, char *send_str);
  */

};


#endif 
