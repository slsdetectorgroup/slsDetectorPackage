#ifndef REGISTERS_G_H
#define REGISTERS_G_H


#include "sls_detector_defs.h"


/* Definitions for FPGA*/
#define CSP0 0x20200000
#define MEM_SIZE 0x100000 



/* values defined for FPGA */
#define MCSNUM            0x0
#define FIXED_PATT_VAL    0xacdc1980


#define FPGA_INIT_PAT     0x60008
#define FPGA_INIT_ADDR    0xb0000000




/* registers defined in FPGA */
#define PCB_REV_REG			  0x2c<<11
#define GAIN_REG              0x10<<11
//#define FLOW_CONTROL_REG      0x11<<11
//#define FLOW_STATUS_REG       0x12<<11
//#define FRAME_REG             0x13<<11
#define MULTI_PURPOSE_REG     0x14<<11
#define DAQ_REG               0x15<<11
//#define TIME_FROM_START_REG   0x16<<11
#define MCB_CNTRL_REG_OFF     0x17<<11// control the dacs
//ADC
#define ADC_WRITE_REG         0x18<<11
#define ADC_SYNC_REG          0x19<<11
//#define MUTIME_REG            0x1a<<11
//temperature
#define TEMP_IN_REG           0x1b<<11
#define TEMP_OUT_REG          0x1c<<11
//configure MAC
#define TSE_CONF_REG          0x1d<<11
#define ENET_CONF_REG         0x1e<<11
//#define WRTSE_SHAD_REG        0x1f<<11
//HV
#define HV_REG                0x20<<11


#define DUMMY_REG             0x21<<11
#define FPGA_VERSION_REG      0x22<<11
#define FIX_PATT_REG          0x23<<11
#define CONTROL_REG           0x24<<11
#define STATUS_REG            0x25<<11
#define CONFIG_REG            0x26<<11
#define EXT_SIGNAL_REG   	  0x27<<11
#define FPGA_SVN_REG    	  0x29<<11


#define CHIP_OF_INTRST_REG    0x2A<<11

//FIFO
#define LOOK_AT_ME_REG   	  0x28<<11

#define FIFO_DATA_REG_OFF     0x50<<11 ///////


//to read back dac registers
#define MOD_DACS1_REG         0x65<<11
#define MOD_DACS2_REG         0x66<<11
#define MOD_DACS3_REG         0x67<<11

//user entered
#define SET_DELAY_LSB_REG     0x68<<11
#define SET_DELAY_MSB_REG     0x69<<11
#define GET_DELAY_LSB_REG     0x6a<<11
#define GET_DELAY_MSB_REG     0x6b<<11

#define SET_TRAINS_LSB_REG    0x6c<<11
#define SET_TRAINS_MSB_REG    0x6d<<11
#define GET_TRAINS_LSB_REG    0x6e<<11
#define GET_TRAINS_MSB_REG    0x6f<<11

#define SET_FRAMES_LSB_REG    0x70<<11
#define SET_FRAMES_MSB_REG    0x71<<11
#define GET_FRAMES_LSB_REG    0x72<<11
#define GET_FRAMES_MSB_REG    0x73<<11

#define SET_PERIOD_LSB_REG    0x74<<11
#define SET_PERIOD_MSB_REG    0x75<<11
#define GET_PERIOD_LSB_REG    0x76<<11
#define GET_PERIOD_MSB_REG    0x77<<11

#define SET_EXPTIME_LSB_REG   0x78<<11
#define SET_EXPTIME_MSB_REG   0x79<<11
#define GET_EXPTIME_LSB_REG   0x7a<<11
#define GET_EXPTIME_MSB_REG   0x7b<<11

#define SET_GATES_LSB_REG     0x7c<<11
#define SET_GATES_MSB_REG     0x7d<<11
#define GET_GATES_LSB_REG     0x7e<<11
#define GET_GATES_MSB_REG     0x7f<<11



#define PLL_PARAM_REG  0x37<<11
#define PLL_PARAM_OUT_REG 0x38<<11
#define PLL_CNTRL_REG 0x34<<11

#define ROI_REG 0x35<<11
#define OVERSAMPLING_REG 0x36<<11
#define MOENCH_CNTR_REG 0x31<<11
#define MOENCH_CNTR_OUT_REG 0x33<<11
#define MOENCH_CNTR_CONF_REG 0x32<<11



//image
#define DARK_IMAGE_REG     0x81<<11
#define GAIN_IMAGE_REG     0x82<<11

//counter block memory
#define COUNTER_MEMORY_REG 0x85<<11


#define GET_MEASUREMENT_TIME_LSB_REG    0x023000   
#define GET_MEASUREMENT_TIME_MSB_REG    0x024000 
  
#define GET_ACTUAL_TIME_LSB_REG    0x025000   
#define GET_ACTUAL_TIME_MSB_REG    0x026000  


//not used
//#define MCB_DOUT_REG_OFF      0x200000
//#define FIFO_CNTRL_REG_OFF    0x300000
//#define FIFO_COUNTR_REG_OFF   0x400000
//not used so far
//#define SPEED_REG        0x006000
//#define SET_NBITS_REG    0x008000
//not used
//#define GET_SHIFT_IN_REG      0x022000



#define SHIFTMOD 2
#define SHIFTFIFO 9

/** for PCB_REV_REG */
#define DETECTOR_TYPE_MASK   	0xF0000
#define DETECTOR_TYPE_OFFSET   	16
#define BOARD_REVISION_MASK		0xFFFF
#define MOENCH_MODULE			2




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
#define READSTATE_0_BIT    		 0x00000100
#define READSTATE_1_BIT    		 0x00000200
#define READSTATE_2_BIT    		 0x00000400

#define RUNSTATE_0_BIT     		 0x00001000
#define RUNSTATE_1_BIT    		 0x00002000
#define RUNSTATE_2_BIT    		 0x00004000
#define SOME_FIFO_FULL_BIT       0x00008000 // error!
#define ALL_FIFO_EMPTY_BIT       0x00010000 // data ready
#define RUNMACHINE_BUSY_BIT      0x00020000
#define READMACHINE_BUSY_BIT     0x00040000



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
#define CPU_OR_RECEIVER_BIT		 0x00001000



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


/* multi purpose register */
#define PHASE_STEP_BIT               0x00000001
#define PHASE_STEP_OFFSET            0
// #define xxx_BIT                   0x00000002
#define RESET_COUNTER_BIT            0x00000004
#define RESET_COUNTER_OFFSET         2
//#define xxx_BIT                    0x00000008
//#define xxx_BIT                    0x00000010
#define SW1_BIT                      0x00000020
#define SW1_OFFSET                   5
#define WRITE_BACK_BIT               0x00000040
#define WRITE_BACK_OFFSET            6
#define RESET_BIT                    0x00000080
#define RESET_OFFSET                 7
#define ENET_RESETN_BIT              0x00000800
#define ENET_RESETN_OFFSET           11
#define INT_RSTN_BIT                 0x00002000
#define INT_RSTN_OFFSET              13
#define DIGITAL_TEST_BIT             0x00004000
#define DIGITAL_TEST_OFFSET          14
//#define CHANGE_AT_POWER_ON_BIT       0x00008000
//#define CHANGE_AT_POWER_ON_OFFSET    15


/* settings/conf gain register */
#define GAIN_MASK                    0x0000000f 
#define GAIN_OFFSET                  0  
#define SETTINGS_MASK                0x000000f0
#define SETTINGS_OFFSET              4


/* CHIP_OF_INTRST_REG */
#define CHANNEL_MASK 			    0xffff0000
#define CHANNEL_OFFSET				16
#define ACTIVE_ADC_MASK 		    0x0000001f



/**ADC SYNC CLEAN FIFO*/
#define ADCSYNC_CLEAN_FIFO_BITS     0x300000
#define CLEAN_FIFO_MASK				0x0fffff

#endif

