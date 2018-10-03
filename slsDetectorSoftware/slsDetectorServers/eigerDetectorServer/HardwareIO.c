
//Class initially from Gerd and was called mmap_test.c 
//return reversed 1 means good, 0 means failed


//#include <stdio.h>
//#include <unistd.h>
//#include <string.h>
//#include <sys/mman.h>
//#include <fcntl.h>

#include "HardwareIO.h"

xfs_u8 HWIO_xfs_in8(xfs_u32 InAddress)
{
    /* read the contents of the I/O location and then synchronize the I/O
     * such that the I/O operation completes before proceeding on
     */

    xfs_u8 IoContents;
    __asm__ volatile ("eieio; lbz %0,0(%1)":"=r" (IoContents):"b"
              (InAddress));
    return IoContents;
}

/*****************************************************************************/

xfs_u16 HWIO_xfs_in16(xfs_u32 InAddress)
{
    /* read the contents of the I/O location and then synchronize the I/O
     * such that the I/O operation completes before proceeding on
     */

    xfs_u16 IoContents;
    __asm__ volatile ("eieio; lhz %0,0(%1)":"=r" (IoContents):"b"
              (InAddress));
    return IoContents;
}

/*****************************************************************************/

xfs_u32 HWIO_xfs_in32(xfs_u32 InAddress)
{
    /* read the contents of the I/O location and then synchronize the I/O
     * such that the I/O operation completes before proceeding on
     */

    xfs_u32 IoContents;
    __asm__ volatile ("eieio; lwz %0,0(%1)":"=r" (IoContents):"b"
              (InAddress));
    return IoContents;
}

/*****************************************************************************/

void HWIO_xfs_out8(xfs_u32 OutAddress, xfs_u8 Value)
{
    /* write the contents of the I/O location and then synchronize the I/O
     * such that the I/O operation completes before proceeding on
     */

    __asm__ volatile ("stb %0,0(%1); eieio"::"r" (Value), "b"(OutAddress));
}

/*****************************************************************************/
void HWIO_xfs_out16(xfs_u32 OutAddress, xfs_u16 Value)
{
    /* write the contents of the I/O location and then synchronize the I/O
     * such that the I/O operation completes before proceeding on
     */

    __asm__ volatile ("sth %0,0(%1); eieio"::"r" (Value), "b"(OutAddress));
}

/*****************************************************************************/

void HWIO_xfs_out32(xfs_u32 OutAddress, xfs_u32 Value)
{
    /* write the contents of the I/O location and then synchronize the I/O
     * such that the I/O operation completes before proceeding on
     */

    __asm__ volatile ("stw %0,0(%1); eieio"::"r" (Value), "b"(OutAddress));
}



