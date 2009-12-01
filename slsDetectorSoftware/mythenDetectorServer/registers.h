#ifndef REGISTERS_H
#define REGISTERS_H



/* Definitions for FPGA*/
#define CSP0 0x90000000 // Base Addresse CSP0
#define CSP4 0xa0000000 // Base Addresse CSP4

//#define MEM_SIZE 0xFFFFFF // map so much memory
#define MEM_SIZE 0xFFFFFFF // map so much memory



/* registers defined in FPGA */
#define FIX_PATT_REG     0x000000
#define FPGA_VERSION_REG 0x001000
#define DUMMY_REG        0x002000
#define CONTROL_REG      0x003000
#define STATUS_REG       0x004000
#define CONFIG_REG       0x005000
#define SPEED_REG        0x006000
#define EXT_SIGNAL_REG   0x007000
#define SET_NBITS_REG    0x008000
#define LOOK_AT_ME_REG   0x009000

#define SET_FRAMES_LSB_REG    0x00A000   
#define SET_FRAMES_MSB_REG    0x00B000   
#define GET_FRAMES_LSB_REG    0x00C000   
#define GET_FRAMES_MSB_REG    0x00D000   

#define SET_EXPTIME_LSB_REG    0x00E000   
#define SET_EXPTIME_MSB_REG    0x00F000   
#define GET_EXPTIME_LSB_REG    0x010000   
#define GET_EXPTIME_MSB_REG    0x011000   

#define SET_GATES_LSB_REG    0x012000   
#define SET_GATES_MSB_REG    0x013000   
#define GET_GATES_LSB_REG    0x014000   
#define GET_GATES_MSB_REG    0x015000   

#define SET_PERIOD_LSB_REG    0x016000   
#define SET_PERIOD_MSB_REG    0x017000   
#define GET_PERIOD_LSB_REG    0x018000   
#define GET_PERIOD_MSB_REG    0x019000   

#define SET_DELAY_LSB_REG    0x01A000   
#define SET_DELAY_MSB_REG    0x01B000   
#define GET_DELAY_LSB_REG    0x01C000   
#define GET_DELAY_MSB_REG    0x01D000   

#define SET_TRAINS_LSB_REG    0x01E000   
#define SET_TRAINS_MSB_REG    0x01F000   
#define GET_TRAINS_LSB_REG    0x020000   
#define GET_TRAINS_MSB_REG    0x021000   

#define GET_SHIFT_IN_REG      0x022000   

#define MCB_CNTRL_REG_OFF       0x100000
#define MCB_DOUT_REG_OFF        0x200000
#define FIFO_CNTRL_REG_OFF      0x300000
#define FIFO_COUNTR_REG_OFF     0x400000
#define FIFO_DATA_REG_OFF       0x800000

#define SHIFTMOD 2
#define SHIFTFIFO 9


/* values defined for FPGA */
#define MCSNUM        0x0
#define MCSVERSION    0x101
#define FIXED_PATT_VAL 0xacdc1980
#define FPGA_VERSION_VAL 0x00090514
#define FPGA_INIT_PAT 0x60008
#define FPGA_INIT_ADDR 0xb0000000

/* for control register */
#define START_ACQ_BIT      0x00000001
#define STOP_ACQ_BIT       0x00000002
#define START_FIFOTEST_BIT 0x00000004 // ?????
#define STOP_FIFOTEST_BIT  0x00000008  // ??????
#define START_READOUT_BIT  0x00000010  
#define STOP_READOUT_BIT   0x00000020 
#define START_EXPOSURE_BIT  0x00000040  
#define STOP_EXPOSURE_BIT   0x00000080  
#define START_TRAIN_BIT     0x00000100  
#define STOP_TRAIN_BIT      0x00000200  
#define SYNC_RESET          0x80000000

/* for status register */
#define RUN_BUSY_BIT             0x00000001
#define READOUT_BUSY_BIT         0x00000002
#define FIFOTEST_BUSY_BIT        0x00000004 //????
#define WAITING_FOR_TRIGGER_BIT  0x00000008
#define DELAYBEFORE_BIT          0x00000010
#define DELAYAFTER_BIT           0x00000020
#define EXPOSING_BIT             0x00000040
#define COUNT_ENABLE_BIT         0x00000080
#define SOME_FIFO_FULL_BIT       0x00008000 // error!
#define ALL_FIFO_EMPTY_BIT       0x00010000 // data ready



/* for config register */

#define TOT_ENABLE_BIT           0x00000002
#define TIMED_GATE_BIT           0x00000004
#define TIMED_GATE_BIT           0x00000004
#define CONT_RO_ENABLE_BIT       0x00080000  



/* for speed register */

#define CLK_DIVIDER_MASK              0x000000ff 
#define CLK_DIVIDER_OFFSET            0 
#define SET_LENGTH_MASK               0x00000f00 
#define SET_LENGTH_OFFSET             8 
#define WAIT_STATES_MASK              0x0000f000
#define WAIT_STATES_OFFSET            12  
#define TOTCLK_DIVIDER_MASK           0xff000000 
#define TOTCLK_DIVIDER_OFFSET         24  


/* for external signal register */

#define SIGNAL_OFFSET                 4
#define SIGNAL_MASK                   0xF
#define EXT_SIG_OFF                   0x0
#define EXT_GATE_IN_ACTIVEHIGH        0x1
#define EXT_GATE_IN_ACTIVELOW         0x2
#define EXT_TRIG_IN_RISING            0x3
#define EXT_TRIG_IN_FALLING           0x4
#define EXT_RO_TRIG_IN_RISING         0x5
#define EXT_RO_TRIG_IN_FALLING        0x6
#define EXT_GATE_OUT_ACTIVEHIGH        0x7
#define EXT_GATE_OUT_ACTIVELOW         0x8
#define EXT_TRIG_OUT_RISING            0x9
#define EXT_TRIG_OUT_FALLING           0xA
#define EXT_RO_TRIG_OUT_RISING         0xB
#define EXT_RO_TRIG_OUT_FALLING        0xC



/* fifo control register */
#define FIFO_RESET_BIT              0x00000001 
#define FIFO_DISABLE_TOGGLE_BIT     0x00000002 


//chip shiftin register meaning

#define OUTMUX_OFF 20
#define OUTMUX_MASK   0x1f
#define PROBES_OFF 4
#define PROBES_MASK 0x7f
#define OUTBUF_OFF 0
#define OUTBUF_MASK 1


#endif
