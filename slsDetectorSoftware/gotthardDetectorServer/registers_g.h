#ifndef REGISTERS_G_H
#define REGISTERS_G_H


/* Definitions for FPGA*/
#define CSP0 0x20200000
#define MEM_SIZE 0x100000 


/* registers defined in FPGA */
#define GAIN_REG         0x10<<11
#define DAQ_REG          0x1b<<11
#define MULTI_PURPOSE_REG 0x94<<11

#define DUMMY_REG        0x13<<11
#define FIX_PATT_REG     0x45<<11
#define FPGA_VERSION_REG 0x47<<11

#define CONTROL_REG      0x5d<<11
#define STATUS_REG       0x5e<<11
#define CONFIG_REG       0x5f<<11
#define EXT_SIGNAL_REG   0x6a<<11

//temperature
#define TEMP_IN_REG      0x81<<11
#define TEMP_OUT_REG     0x82<<11

//HV
#define HV_REG           0x93<<11

//not used so far
#define SPEED_REG        0x006000
#define SET_NBITS_REG    0x008000
#define LOOK_AT_ME_REG   0x009000

//user entered
#define SET_DELAY_LSB_REG     0x44<<11
#define SET_DELAY_MSB_REG     0x45<<11
#define GET_DELAY_LSB_REG     0x46<<11
#define GET_DELAY_MSB_REG     0x47<<11

#define SET_TRAINS_LSB_REG    0x48<<11
#define SET_TRAINS_MSB_REG    0x49<<11 
#define GET_TRAINS_LSB_REG    0x4a<<11
#define GET_TRAINS_MSB_REG    0x4b<<11

#define SET_FRAMES_LSB_REG    0x4c<<11
#define SET_FRAMES_MSB_REG    0x4d<<11
#define GET_FRAMES_LSB_REG    0x4e<<11
#define GET_FRAMES_MSB_REG    0x4f<<11

#define SET_PERIOD_LSB_REG    0x51<<11
#define SET_PERIOD_MSB_REG    0x52<<11
#define GET_PERIOD_LSB_REG    0x53<<11
#define GET_PERIOD_MSB_REG    0x54<<11

#define SET_EXPTIME_LSB_REG   0x55<<11
#define SET_EXPTIME_MSB_REG   0x56<<11
#define GET_EXPTIME_LSB_REG   0x57<<11
#define GET_EXPTIME_MSB_REG   0x58<<11

#define SET_GATES_LSB_REG     0x59<<11
#define SET_GATES_MSB_REG     0x5a<<11
#define GET_GATES_LSB_REG     0x5b<<11
#define GET_GATES_MSB_REG     0x5c<<11


//not used
#define GET_SHIFT_IN_REG      0x022000   

//to read back dac registers
#define MOD_DACS1_REG         0x41<<11
#define MOD_DACS2_REG         0x42<<11
#define MOD_DACS3_REG         0x43<<11 

#define MCB_CNTRL_REG_OFF     0x37<<11//used to control the dacs

//not used
#define MCB_DOUT_REG_OFF      0x200000
#define FIFO_CNTRL_REG_OFF    0x300000
#define FIFO_COUNTR_REG_OFF   0x400000
#define FIFO_DATA_REG_OFF     0x800000

#define SHIFTMOD 2
#define SHIFTFIFO 9



/* values defined for FPGA */
#define MCSNUM        0x0
#define MCSVERSION    0x101
#define FIXED_PATT_VAL 0xacdc1980
#define FPGA_VERSION_VAL 0x01110825  //0x00090514
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
#define SYNC_RESET          0x00000400

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

/* for fifo status register */
#define FIFO_ENABLED_BIT         0x80000000
#define FIFO_DISABLED_BIT        0x01000000
#define FIFO_ERROR_BIT           0x08000000
#define FIFO_EMPTY_BIT           0x04000000
#define FIFO_DATA_READY_BIT      0x02000000
#define FIFO_COUNTER_MASK        0x000001ff
#define FIFO_NM_MASK             0x00e00000
#define FIFO_NM_OFF              21
#define FIFO_NC_MASK             0x001ffe00
#define FIFO_NC_OFF              9

/* for config register *///not really used yet
#define TOT_ENABLE_BIT           0x00000002
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
#define TOTCLK_DUTYCYCLE_MASK         0x00ff0000 
#define TOTCLK_DUTYCYCLE_OFFSET       16

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


/* for temperature register */
#define T1_CLK_BIT             0x00000001
#define T1_CS_BIT              0x00000002
#define T2_CLK_BIT             0x00000004
#define T2_CS_BIT              0x00000008



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

