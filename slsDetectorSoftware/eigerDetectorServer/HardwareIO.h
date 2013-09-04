 

//Class initially from Gerd and was called mmap_test.c 

#ifndef HARDWAREIO_H
#define HARDWAREIO_H

#include "xfs_types.h"

class HardwareIO{ //

 protected:
  xfs_u8  xfs_in8(xfs_u32 InAddress);
  xfs_u16 xfs_in16(xfs_u32 InAddress);
  xfs_u32 xfs_in32(xfs_u32 InAddress);

  void xfs_out8(xfs_u32 OutAddress, xfs_u8 Value);
  void xfs_out16(xfs_u32 OutAddress, xfs_u16 Value);
  void xfs_out32(xfs_u32 OutAddress, xfs_u32 Value);


 public:
  HardwareIO(){};
  virtual ~HardwareIO(){};


};


#endif 
