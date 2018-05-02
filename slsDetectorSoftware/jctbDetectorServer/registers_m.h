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

//#ifdef JUNGFRAU_DHANYA
#define POWER_ON_REG 			0x5e << MEM_MAP_SHIFT
//  Pwr_I2C_SDA <= PowerReg_s(1) when PowerReg_s(3)='1' else 'Z';
//  Pwr_I2C_SCL <= PowerReg_s(0) when PowerReg_s(2)='1' else 'Z';

#define PWR_I2C_SCL_BIT  0
#define PWR_I2C_SDA_BIT  1
#define PWR_I2C_SCL_EN_BIT  2
#define PWR_I2C_SDA_EN_BIT  3

#define POWER_STATUS_REG 		41 << MEM_MAP_SHIFT	

#define ADCREG1 			0x08  
#define ADCREG2 			0x14//20 
#define ADCREG3 			0x4  
#define ADCREG4 			0x5  
#define ADCREG_VREFS 			24 
#define DBIT_PIPELINE_REG 		89 << MEM_MAP_SHIFT //0x59 same PATTERN_N_LOOP2_REG
#define MEM_MACHINE_FIFOS_REG 		79 << MEM_MAP_SHIFT //from gotthard
#define CONFGAIN_REG 			93 << MEM_MAP_SHIFT //from gotthard
#define ADC_PIPELINE_REG 		66 << MEM_MAP_SHIFT //0x42 same as ADC_OFFSET_REG
//#endif

//#define ADC_OFFSET_REG      		93 << MEM_MAP_SHIFT //same as DAQ_REG
#define ADC_INVERSION_REG   		67 << MEM_MAP_SHIFT

#define DAC_REG     64 << MEM_MAP_SHIFT//0x17 << MEM_MAP_SHIFT// control the dacs
//ADC
#define ADC_WRITE_REG         65 << MEM_MAP_SHIFT//0x18 << MEM_MAP_SHIFT
//#define ADC_SYNC_REG          66 << MEM_MAP_SHIFT//0x19 << MEM_MAP_SHIFT
//#define HV_REG                67 << MEM_MAP_SHIFT//0x20 << MEM_MAP_SHIFT




//#define MUTIME_REG            0x1a << MEM_MAP_SHIFT
//temperature
#define TEMP_IN_REG           0x1b << MEM_MAP_SHIFT
#define TEMP_OUT_REG          0x1c << MEM_MAP_SHIFT
//configure MAC
#define TSE_CONF_REG          0x1d << MEM_MAP_SHIFT
#define ENET_CONF_REG         0x1e << MEM_MAP_SHIFT
//#define WRTSE_SHAD_REG        0x1f << MEM_MAP_SHIFT
//HV


#define DUMMY_REG             68 << MEM_MAP_SHIFT//0x21 << MEM_MAP_SHIFT
#define FPGA_VERSION_REG      0 << MEM_MAP_SHIFT //0x22 << MEM_MAP_SHIFT
#define PCB_REV_REG			  0 << MEM_MAP_SHIFT
#define FIX_PATT_REG          1 << MEM_MAP_SHIFT //0x23 << MEM_MAP_SHIFT
#define CONTROL_REG           79 << MEM_MAP_SHIFT//0x24 << MEM_MAP_SHIFT
#define STATUS_REG            2 << MEM_MAP_SHIFT //0x25 << MEM_MAP_SHIFT
#define CONFIG_REG            77 << MEM_MAP_SHIFT//0x26 << MEM_MAP_SHIFT
#define EXT_SIGNAL_REG        78 << MEM_MAP_SHIFT//	  0x27 << MEM_MAP_SHIFT
//#define FPGA_SVN_REG    	  0x29 << MEM_MAP_SHIFT


#define CHIP_OF_INTRST_REG    0x2A << MEM_MAP_SHIFT

//FIFO
#define LOOK_AT_ME_REG          3 << MEM_MAP_SHIFT //0x28 << MEM_MAP_SHIFT
#define SYSTEM_STATUS_REG       4 << MEM_MAP_SHIFT

#define FIFO_DATA_REG 6 << MEM_MAP_SHIFT
#define FIFO_STATUS_REG  7 << MEM_MAP_SHIFT

//   constant FifoDigitalInReg_c : integer := 60;
#define FIFO_DIGITAL_DATA_LSB_REG 60 << MEM_MAP_SHIFT
#define FIFO_DIGITAL_DATA_MSB_REG 61 << MEM_MAP_SHIFT

#define FIFO_DATA_REG_OFF     0x50 << MEM_MAP_SHIFT ///////
//to read back dac registers
//#define MOD_DACS1_REG         0x65 << MEM_MAP_SHIFT
//#define MOD_DACS2_REG         0x66 << MEM_MAP_SHIFT
//#define MOD_DACS3_REG         0x67 << MEM_MAP_SHIFT

//user entered






#define GET_ACTUAL_TIME_LSB_REG     16 << MEM_MAP_SHIFT
#define GET_ACTUAL_TIME_MSB_REG     17 << MEM_MAP_SHIFT

#define GET_MEASUREMENT_TIME_LSB_REG     38 << MEM_MAP_SHIFT
#define GET_MEASUREMENT_TIME_MSB_REG     39 << MEM_MAP_SHIFT


#define SET_DELAY_LSB_REG     96 << MEM_MAP_SHIFT //0x68 << MEM_MAP_SHIFT
#define SET_DELAY_MSB_REG     97 << MEM_MAP_SHIFT //0x69 << MEM_MAP_SHIFT
#define GET_DELAY_LSB_REG     18 << MEM_MAP_SHIFT//0x6a << MEM_MAP_SHIFT
#define GET_DELAY_MSB_REG     19 << MEM_MAP_SHIFT//0x6b << MEM_MAP_SHIFT

#define SET_CYCLES_LSB_REG    98 << MEM_MAP_SHIFT//0x6c << MEM_MAP_SHIFT
#define SET_CYCLES_MSB_REG    99 << MEM_MAP_SHIFT//0x6d << MEM_MAP_SHIFT
#define GET_CYCLES_LSB_REG    20 << MEM_MAP_SHIFT//0x6e << MEM_MAP_SHIFT
#define GET_CYCLES_MSB_REG    21 << MEM_MAP_SHIFT//0x6f << MEM_MAP_SHIFT

#define SET_FRAMES_LSB_REG    100 << MEM_MAP_SHIFT//0x70 << MEM_MAP_SHIFT
#define SET_FRAMES_MSB_REG    101 << MEM_MAP_SHIFT//0x71 << MEM_MAP_SHIFT
#define GET_FRAMES_LSB_REG    22 << MEM_MAP_SHIFT//0x72 << MEM_MAP_SHIFT
#define GET_FRAMES_MSB_REG    23 << MEM_MAP_SHIFT//0x73 << MEM_MAP_SHIFT

#define SET_PERIOD_LSB_REG    102 << MEM_MAP_SHIFT//0x74 << MEM_MAP_SHIFT
#define SET_PERIOD_MSB_REG    103 << MEM_MAP_SHIFT//0x75 << MEM_MAP_SHIFT
#define GET_PERIOD_LSB_REG    24 << MEM_MAP_SHIFT//0x76 << MEM_MAP_SHIFT
#define GET_PERIOD_MSB_REG    25 << MEM_MAP_SHIFT//0x77 << MEM_MAP_SHIFT

//#define PATTERN_WAIT0_TIME_REG_LSB 114 << MEM_MAP_SHIFT
//#define PATTERN_WAIT0_TIME_REG_MSB 115 << MEM_MAP_SHIFT
#define SET_EXPTIME_LSB_REG   114 << MEM_MAP_SHIFT//0x78 << MEM_MAP_SHIFT
#define SET_EXPTIME_MSB_REG   115 << MEM_MAP_SHIFT//0x79 << MEM_MAP_SHIFT
#define GET_EXPTIME_LSB_REG   26 << MEM_MAP_SHIFT//0x7a << MEM_MAP_SHIFT
#define GET_EXPTIME_MSB_REG   27 << MEM_MAP_SHIFT//0x7b << MEM_MAP_SHIFT

#define SET_GATES_LSB_REG     106 << MEM_MAP_SHIFT//0x7c << MEM_MAP_SHIFT
#define SET_GATES_MSB_REG     107 << MEM_MAP_SHIFT//0x7d << MEM_MAP_SHIFT
#define GET_GATES_LSB_REG     28 << MEM_MAP_SHIFT//0x7e << MEM_MAP_SHIFT
#define GET_GATES_MSB_REG     29 << MEM_MAP_SHIFT//0x7f << MEM_MAP_SHIFT

#define DATA_IN_LSB_REG 30 << MEM_MAP_SHIFT
#define DATA_IN_MSB_REG 31 << MEM_MAP_SHIFT

#define PATTERN_OUT_LSB_REG 32 << MEM_MAP_SHIFT
#define PATTERN_OUT_MSB_REG 33 << MEM_MAP_SHIFT

#define FRAMES_FROM_START_LSB_REG 34 << MEM_MAP_SHIFT
#define FRAMES_FROM_START_MSB_REG 35 << MEM_MAP_SHIFT

#define FRAMES_FROM_START_PG_LSB_REG 36 << MEM_MAP_SHIFT
#define FRAMES_FROM_START_PG_MSB_REG 37 << MEM_MAP_SHIFT

#define SLOW_ADC_REG 43 << MEM_MAP_SHIFT
 
   

#define PLL_PARAM_REG  80 << MEM_MAP_SHIFT//0x37 << MEM_MAP_SHIFT
#define PLL_PARAM_OUT_REG 5 << MEM_MAP_SHIFT //0x38 << MEM_MAP_SHIFT
#define PLL_CNTRL_REG 81 << MEM_MAP_SHIFT//0x34 << MEM_MAP_SHIFT


#ifdef NEW_GBE_INTERFACE
#define GBE_PARAM_OUT_REG 40 << MEM_MAP_SHIFT
#define GBE_PARAM_REG 69 << MEM_MAP_SHIFT
#define GBE_CNTRL_REG 70 << MEM_MAP_SHIFT
#else
#define RX_UDP_AREG    69 << MEM_MAP_SHIFT //rx_udpip_AReg_c     : integer:= 69; *\/ 
#define UDPPORTS_AREG 70 << MEM_MAP_SHIFT// udpports_AReg_c   : integer:= 70; *\/
#define RX_UDPMACL_AREG 71 << MEM_MAP_SHIFT//rx_udpmacL_AReg_c   : integer:= 71; *\/ 
#define RX_UDPMACH_AREG 72 << MEM_MAP_SHIFT//rx_udpmacH_AReg_c   : integer:= 72; *\/
#define DETECTORMACL_AREG 73 << MEM_MAP_SHIFT//detectormacL_AReg_c : integer:= 73; *\/
#define DETECTORMACH_AREG 74 << MEM_MAP_SHIFT//detectormacH_AReg_c : integer:= 74; *\/ 
#define DETECTORIP_AREG 75 << MEM_MAP_SHIFT//detectorip_AReg_c   : integer:= 75; *\/
#define IPCHKSUM_AREG 76 << MEM_MAP_SHIFT//ipchksum_AReg_c : integer:= 76; *\/ */
#endif


#define PATTERN_CNTRL_REG 82 << MEM_MAP_SHIFT
#define PATTERN_LIMITS_AREG 83 << MEM_MAP_SHIFT

#define PATTERN_LOOP0_AREG 84 << MEM_MAP_SHIFT
#define PATTERN_N_LOOP0_REG 85 << MEM_MAP_SHIFT

#define PATTERN_LOOP1_AREG 86 << MEM_MAP_SHIFT
#define PATTERN_N_LOOP1_REG 87 << MEM_MAP_SHIFT

#define PATTERN_LOOP2_AREG 88 << MEM_MAP_SHIFT
#define PATTERN_N_LOOP2_REG 89 << MEM_MAP_SHIFT

#define PATTERN_WAIT0_AREG 90 << MEM_MAP_SHIFT
#define PATTERN_WAIT1_AREG 91 << MEM_MAP_SHIFT
#define PATTERN_WAIT2_AREG 92 << MEM_MAP_SHIFT



//#define DAQ_REG   93 << MEM_MAP_SHIFT //unused
#define NSAMPLES_REG 93 << MEM_MAP_SHIFT 


#define HV_REG 95 << MEM_MAP_SHIFT
   


#define PATTERN_IOCTRL_REG_LSB 108 << MEM_MAP_SHIFT
#define PATTERN_IOCTRL_REG_MSB 109 << MEM_MAP_SHIFT

#define PATTERN_IOCLKCTRL_REG_LSB 110 << MEM_MAP_SHIFT
#define PATTERN_IOCLKCTRL_REG_MSB 111 << MEM_MAP_SHIFT
#define PATTERN_IN_REG_LSB 112 << MEM_MAP_SHIFT
#define PATTERN_IN_REG_MSB 113 << MEM_MAP_SHIFT
#define PATTERN_WAIT0_TIME_REG_LSB 114 << MEM_MAP_SHIFT
#define PATTERN_WAIT0_TIME_REG_MSB 115 << MEM_MAP_SHIFT
#define PATTERN_WAIT1_TIME_REG_LSB 116 << MEM_MAP_SHIFT
#define PATTERN_WAIT1_TIME_REG_MSB 117 << MEM_MAP_SHIFT
#define PATTERN_WAIT2_TIME_REG_LSB 118 << MEM_MAP_SHIFT
#define PATTERN_WAIT2_TIME_REG_MSB 119 << MEM_MAP_SHIFT
   
//#define DAC_REG_OFF 120 
//#define DAC_0_1_VAL_REG 120 << MEM_MAP_SHIFT 
//#define DAC_2_3_VAL_REG 121 << MEM_MAP_SHIFT  
//#define DAC_4_5_VAL_REG 122 << MEM_MAP_SHIFT  
//#define DAC_6_7_VAL_REG 123 << MEM_MAP_SHIFT  
//#define DAC_8_9_VAL_REG 124 << MEM_MAP_SHIFT  
//#define DAC_10_11_VAL_REG 125 << MEM_MAP_SHIFT  
//#define DAC_12_13_VAL_REG 126 << MEM_MAP_SHIFT  
//#define DAC_14_15_VAL_REG 127 << MEM_MAP_SHIFT  
#define DAC_VAL_REG 121 << MEM_MAP_SHIFT
#define DAC_NUM_REG 122 << MEM_MAP_SHIFT
#define DAC_VAL_OUT_REG 42 << MEM_MAP_SHIFT
#define ADC_LATCH_DISABLE_REG   120 << MEM_MAP_SHIFT
 
 






/* registers defined in FPGA */
#define GAIN_REG              0
//#define FLOW_CONTROL_REG      0x11 << MEM_MAP_SHIFT
//#define FLOW_STATUS_REG       0x12 << MEM_MAP_SHIFT
//#define FRAME_REG             0x13 << MEM_MAP_SHIFT
#define MULTI_PURPOSE_REG     0
//#define TIME_FROM_START_REG   0x16 << MEM_MAP_SHIFT


#define ROI_REG 0 // 0x35 << MEM_MAP_SHIFT
#define OVERSAMPLING_REG 0 // 0x36 << MEM_MAP_SHIFT
#define MOENCH_CNTR_REG 0 // 0x31 << MEM_MAP_SHIFT
#define MOENCH_CNTR_OUT_REG 0 // 0x33 << MEM_MAP_SHIFT
#define MOENCH_CNTR_CONF_REG 0 // 0x32 << MEM_MAP_SHIFT



//image
#define DARK_IMAGE_REG     0 // 0x81 << MEM_MAP_SHIFT
#define GAIN_IMAGE_REG     0 // 0x82 << MEM_MAP_SHIFT

//counter block memory
#define COUNTER_MEMORY_REG 0 // 0x85 << MEM_MAP_SHIFT


//not used
//#define MCB_DOUT_REG_OFF      0 // 0x200000
//#define FIFO_CNTRL_REG_OFF    0 // 0x300000
//#define FIFO_COUNTR_REG_OFF   0 // 0x400000
//not used so far
//#define SPEED_REG        0 // 0x006000
//#define SET_NBITS_REG    0 // 0x008000
//not used
//#define GET_SHIFT_IN_REG      0 // 0x022000



#define SHIFTMOD 2
#define SHIFTFIFO 9

/** for PCB_REV_REG */
#define DETECTOR_TYPE_MASK   	0xFF000000
#define DETECTOR_TYPE_OFFSET   	24
#define BOARD_REVISION_MASK		0xFFFFFF
#define MOENCH03_MODULE_ID		2
#define JUNGFRAU_MODULE_ID			1
#define JUNGFRAU_CTB_ID			3




/* for control register (16bit only)*/
#define START_ACQ_BIT       0x0001
#define STOP_ACQ_BIT        0x0002
#define START_FIFOTEST_BIT  0x0004 // ?????
#define STOP_FIFOTEST_BIT   0x0008  // ??????
#define START_READOUT_BIT   0x0010  
#define STOP_READOUT_BIT    0x0020 
#define START_EXPOSURE_BIT  0x0040  
#define STOP_EXPOSURE_BIT   0x0080  
#define START_TRAIN_BIT     0x0100  
#define STOP_TRAIN_BIT      0x0200   
#define FIFO_RESET_BIT      0x8000  
#define SYNC_RESET          0x0400  
#define GB10_RESET_BIT      0x0800   
#define MEM_RESET_BIT       0x1000   

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
#define LAM_BIT       0x00000400 // error!
#define SOME_FIFO_FULL_BIT       0x00000800 // error!



#define RUNSTATE_0_BIT     		 0x00001000
#define RUNSTATE_1_BIT    		 0x00002000
#define RUNSTATE_2_BIT    		 0x00004000
#define STOPPED_BIT       0x00008000 // stopped!
#define ALL_FIFO_EMPTY_BIT       0x00010000 // data ready
#define RUNMACHINE_BUSY_BIT      0x00020000
#define READMACHINE_BUSY_BIT     0x00040000
#define PLL_RECONFIG_BUSY     	 0x00100000



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
#define GB10_NOT_CPU_BIT		 0x00001000
#define ADC_OUTPUT_DISABLE_BIT 0x00100
#define DIGITAL_OUTPUT_ENABLE_BIT 0x00200


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
//#define FIFO_RESET_BIT              0x00000001 
//#define FIFO_DISABLE_TOGGLE_BIT     0x00000002 


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




enum {run_clk_c, adc_clk_c, sync_clk_c, dbit_clk_c};




#define PLL_CNTR_ADDR_OFF 16 //PLL_CNTR_REG bits 21 downto 16 represent the counter address

#define PLL_CNTR_RECONFIG_RESET_BIT 0
#define PLL_CNTR_READ_BIT 1
#define PLL_CNTR_WRITE_BIT 2
#define PLL_CNTR_PLL_RESET_BIT 3


#define PLL_CNTR_PHASE_EN_BIT 8
#define PLL_CNTR_UPDN_BIT 9
#define PLL_CNTR_CNTSEL_OFF 10
		




#define PLL_MODE_REG 0x0
#define PLL_STATUS_REG 0x1
#define PLL_START_REG 0x2
#define PLL_N_COUNTER_REG 0x3
#define PLL_M_COUNTER_REG 0x4
#define PLL_C_COUNTER_REG 0x5 //which ccounter stands in param 22:18; 7:0 lowcount 15:8 highcount; 16 bypassenable; 17 oddivision
#define PLL_PHASE_SHIFT_REG 0x6 // which ccounter stands in param 16:20; 21 updown (1 up, 0 down)
#define PLL_K_COUNTER_REG 0x7
#define PLL_BANDWIDTH_REG 0x8
#define PLL_CHARGEPUMP_REG 0x9
#define PLL_VCO_DIV_REG 0x1c
#define PLL_MIF_REG 0x1f

#define PPL_M_CNT_PARAM_DEFAULT 0x4040
#define PPL_N_CNT_PARAM_DEFAULT 0x20D0C
#define PPL_C0_CNT_PARAM_DEFAULT 0x20D0C
#define PPL_C1_CNT_PARAM_DEFAULT 0xA0A0
#define PPL_C2_CNT_PARAM_DEFAULT 0x20D0C
#define PPL_C3_CNT_PARAM_DEFAULT 0x0808
#define PPL_BW_PARAM_DEFAULT 0x2EE0
#define PPL_VCO_PARAM_DEFAULT 0x1

#define NEW_PLL_RECONFIG

#ifdef NEW_PLL_RECONFIG
#define PLL_VCO_FREQ_MHZ 400//480//800
#else
#define PLL_VCO_FREQ_MHZ 480//800
#endif





/*
  GBE parameter and control registers definitions
*/

#define GBE_CTRL_WSTROBE 0 
#define GBE_CTRL_VAR_OFFSET 16
#define GBE_CTRL_VAR_MASK 0XF
#define GBE_CTRL_RAMADDR_OFFSET 24
#define GBE_CTRL_RAMADDR_MASK 0X3F
#define GBE_CTRL_INTERFACE 23

#define RX_UDP_IP_ADDR 0
#define RX_UDP_PORTS_ADDR 1
#define RX_UDP_MAC_L_ADDR 2
#define RX_UDP_MAC_H_ADDR 3
#define IPCHECKSUM_ADDR 4
#define GBE_DELAY_ADDR 5
#define GBE_RESERVED1_ADDR 6
#define GBE_RESERVED2_ADDR 7
#define DETECTOR_MAC_L_ADDR 8
#define DETECTOR_MAC_H_ADDR 9
#define DETECTOR_IP_ADDR 10
   


/**------------------
-- pattern registers definitions
--------------------------------------------- */
#define  IOSIGNALS_MASK 0xfffffffffffff
#define ADC_ENABLE_BIT 63
#define  APATTERN_MASK 0xffff
#define ASTART_OFFSET 0
#define ASTOP_OFFSET 16
#define PATTERN_CTRL_WRITE_BIT 0
#define PATTERN_CTRL_READ_BIT 1
#define PATTERN_CTRL_ADDR_OFFSET 16
#define MAX_PATTERN_LENGTH 1024


#endif

