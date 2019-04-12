

//from Gerd and was called mmap_test.h

#ifndef __PLB_LL_FIFO_H__
#define __PLB_LL_FIFO_H__


/******************************************************************************/
/* definitions                                                                */
/******************************************************************************/

#define PLB_LL_FIFO_REG_CTRL   0
#define PLB_LL_FIFO_REG_STATUS 1
#define PLB_LL_FIFO_REG_FIFO   2

#define PLB_LL_FIFO_CTRL_LL_REM_SHIFT    30
#define PLB_LL_FIFO_CTRL_LL_REM          0xC0000000
#define PLB_LL_FIFO_CTRL_LL_EOF          0x20000000
#define PLB_LL_FIFO_CTRL_LL_SOF          0x10000000
#define PLB_LL_FIFO_CTRL_LL_MASK         0xF0000000


#define PLB_LL_FIFO_CTRL_TX_RESET        0x08000000
#define PLB_LL_FIFO_CTRL_RX_RESET        0x04000000

#define PLB_LL_FIFO_CTRL_RESET_STATUS    0x00800000
#define PLB_LL_FIFO_CTRL_RESET_USER      0x00400000
#define PLB_LL_FIFO_CTRL_RESET_LINK      0x00200000
#define PLB_LL_FIFO_CTRL_RESET_GT        0x00100000

#define PLB_LL_FIFO_CTRL_RESET_ALL       0x0CF00000

// do not reset complete gtx dual in std. case
// cause this would reset PLL and stop LL clk
#define PLB_LL_FIFO_CTRL_RESET_STD       0x0CE00000

// reset Rx and Tx Fifo and set User Reset
#define PLB_LL_FIFO_CTRL_RESET_FIFO      0x0C400000


#define PLB_LL_FIFO_CTRL_CONFIG_VECTOR   0x000FFFFF


#define PLB_LL_FIFO_STATUS_LL_REM_SHIFT  30
#define PLB_LL_FIFO_STATUS_LL_REM        0xC0000000
#define PLB_LL_FIFO_STATUS_LL_EOF        0x20000000
#define PLB_LL_FIFO_STATUS_LL_SOF        0x10000000

#define PLB_LL_FIFO_STATUS_EMPTY         0x08000000
#define PLB_LL_FIFO_STATUS_ALMOSTEMPTY   0x04000000
#define PLB_LL_FIFO_STATUS_FULL          0x02000000
#define PLB_LL_FIFO_STATUS_ALMOSTFULL    0x01000000

#define PLB_LL_FIFO_STATUS_VECTOR        0x000FFFFF

#define PLB_LL_FIFO_ALMOST_FULL_THRESHOLD_WORDS    100


#endif // __PLB_LL_FIFO_H__


