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



 
/*   constant FPGAVersionReg_c  : integer:= 0; */
/*   constant FixedPatternReg_c : integer:= 1; */
/*   constant StatusReg_c     : integer:= 2; */
/*   constant LookAtMeReg_c   : integer:= 3; */
/*   constant SystemStatusReg_c : integer:= 4; */
 
/*   constant PLL_ParamOutReg_c : integer:=5;  -- RO register to check control signals going to the chip */
 
 
/*  --time registers use only even numbers! */
/*   constant TimeFromStartReg_c : integer:= 16; */
/*   --constant TimeFromStartReg_c : integer:= 17; MSB */
/*   constant GetDelayReg_c   : integer:= 18; */
/*   --constant GetDelayReg_c   : integer:= 19; MSB */
/*   constant GetCyclesReg_c  : integer:= 20; */
/*   --constant GetTrainsReg_c   : integer:= 21; MSB */
/*   constant GetFramesReg_c  : integer:= 22; */
/*   --constant GetFramesReg_c   : integer:= 23; MSB */
/*   constant GetPeriodReg_c  : integer:= 24; */
/*   --constant GetPeriodReg_c   : integer:= 25; MSB */
/*   constant GetExpTimeReg_c  : integer:= 26; */
/*   --constant GetExpTimeReg_c   : integer:= 27; MSB */
/*   constant GetGatesReg_c   : integer:= 28;  */
/*   --constant GetGatesReg_c   : integer:= 29; MSB */
  
  
  
  
  
/*  -----rw: */
 
/*   constant DACReg_c          : integer:= 64; */
/*   constant ADCWriteReg_c     : integer:= 65; */
/*   constant ADCsyncReg_c      : integer:= 66; */
/*   constant HVReg_c           : integer:= 67; */
/*   constant DummyReg_c        : integer:= 68; */

/*   constant rx_udpip_AReg_c     : integer:= 69; */
/*   constant udpports_AReg_c   : integer:= 70; */
/*   constant rx_udpmacL_AReg_c   : integer:= 71; */
/*   constant rx_udpmacH_AReg_c   : integer:= 72; */
/*   constant detectormacL_AReg_c : integer:= 73; */
/*   constant detectormacH_AReg_c : integer:= 74; */
/*   constant detectorip_AReg_c   : integer:= 75; */
/*   constant ipchksum_AReg_c : integer:= 76; */

/*   constant ConfigReg_c     : integer:= 77; */
/*   constant ExtSignalReg_c  : integer:= 78; */
/*   constant ControlReg_c    : integer:= 79; */
  
  
 
/*   constant PLL_ParamReg_c : integer:= 80; */
/*   constant PLL_CntrlReg_c : integer:=81; */

   
  

/*   --time registers use only even numbers! */
/* --     DELAY_AFTER_TRIGGER, */
/*   constant SetDelayReg_c   : integer:= 96; */
/*   --constant SetDelayReg_c   : integer:= 97; MSB */
/* --    CYCLES_NUMBER, */
/*   constant SetCyclesReg_c  : integer:= 98; */
/*   --constant SetCyclesReg_c : integer:= 99;MSB */
/*   -- FRAME_NUMBER, */
/*   constant SetFramesReg_c  : integer:= 100; */
/*   --constant SetFramesReg_c : integer:= 101; MSB */
/* --     FRAME_PERIOD, */
/*   constant SetPeriodReg_c   : integer:= 102; */
/*   --constant SetPeriodReg_c : integer:= 103; MSB */
/* --     ACQUISITION_TIME, */
/*   constant SetExpTimeReg_c : integer:= 104; */
/*   --constant SetExpTimeReg_c : integer:= 105; MSB */
/* --     GATES_NUMBER, */
/*   constant SetGatesReg_c   : integer:= 106; */
/*   --constant SetGatesReg_c : integer:= 107; MSB */











#define DAC_REG     64<<11//0x17<<11// control the dacs
//ADC
#define ADC_WRITE_REG         65<<11//0x18<<11
//#define ADC_SYNC_REG          66<<11//0x19<<11
//#define HV_REG                67<<11//0x20<<11

#define ADC_OFFSET_REG      66<<11
#define ADC_INVERSION_REG      67<<11

//#define MUTIME_REG            0x1a<<11
//temperature
#define TEMP_IN_REG           0x1b<<11
#define TEMP_OUT_REG          0x1c<<11
//configure MAC
#define TSE_CONF_REG          0x1d<<11
#define ENET_CONF_REG         0x1e<<11
//#define WRTSE_SHAD_REG        0x1f<<11
//HV


#define DUMMY_REG             68<<11//0x21<<11
#define FPGA_VERSION_REG      0<<11 //0x22<<11
#define PCB_REV_REG			  0<<11
#define FIX_PATT_REG          1<<11 //0x23<<11
#define CONTROL_REG           79<<11//0x24<<11
#define STATUS_REG            2<<11 //0x25<<11
#define CONFIG_REG            77<<11//0x26<<11
#define EXT_SIGNAL_REG        78<<11//	  0x27<<11
#define FPGA_SVN_REG    	  0x29<<11


#define CHIP_OF_INTRST_REG    0x2A<<11

//FIFO
#define LOOK_AT_ME_REG          3<<11 //0x28<<11
#define SYSTEM_STATUS_REG       4<<11

#define FIFO_DATA_REG 6<<11
#define FIFO_STATUS_REG  7<<11


#define FIFO_DATA_REG_OFF     0x50<<11 ///////
//to read back dac registers
#define MOD_DACS1_REG         0x65<<11
#define MOD_DACS2_REG         0x66<<11
#define MOD_DACS3_REG         0x67<<11

//user entered





#define GET_ACTUAL_TIME_LSB_REG     16<<11
#define GET_ACTUAL_TIME_MSB_REG     17<<11

#define GET_MEASUREMENT_TIME_LSB_REG     38<<11
#define GET_MEASUREMENT_TIME_MSB_REG     38<<11


#define SET_DELAY_LSB_REG     96<<11 //0x68<<11
#define SET_DELAY_MSB_REG     97<<11 //0x69<<11
#define GET_DELAY_LSB_REG     18<<11//0x6a<<11
#define GET_DELAY_MSB_REG     19<<11//0x6b<<11

#define SET_CYCLES_LSB_REG    98<<11//0x6c<<11
#define SET_CYCLES_MSB_REG    99<<11//0x6d<<11
#define GET_CYCLES_LSB_REG    20<<11//0x6e<<11
#define GET_CYCLES_MSB_REG    21<<11//0x6f<<11

#define SET_FRAMES_LSB_REG    100<<11//0x70<<11
#define SET_FRAMES_MSB_REG    101<<11//0x71<<11
#define GET_FRAMES_LSB_REG    22<<11//0x72<<11
#define GET_FRAMES_MSB_REG    23<<11//0x73<<11

#define SET_PERIOD_LSB_REG    102<<11//0x74<<11
#define SET_PERIOD_MSB_REG    103<<11//0x75<<11
#define GET_PERIOD_LSB_REG    24<<11//0x76<<11
#define GET_PERIOD_MSB_REG    25<<11//0x77<<11

#define SET_EXPTIME_LSB_REG   104<<11//0x78<<11
#define SET_EXPTIME_MSB_REG   105<<11//0x79<<11
#define GET_EXPTIME_LSB_REG   26<<11//0x7a<<11
#define GET_EXPTIME_MSB_REG   27<<11//0x7b<<11

#define SET_GATES_LSB_REG     106<<11//0x7c<<11
#define SET_GATES_MSB_REG     107<<11//0x7d<<11
#define GET_GATES_LSB_REG     28<<11//0x7e<<11
#define GET_GATES_MSB_REG     29<<11//0x7f<<11

#define DATA_IN_LSB_REG 30<<11
#define DATA_IN_MSB_REG 31<<11

#define PATTERN_OUT_LSB_REG 32<<11
#define PATTERN_OUT_MSB_REG 33<<11

#define FRAMES_FROM_START_LSB_REG 34<<11
#define FRAMES_FROM_START_MSB_REG 35<<11

#define FRAMES_FROM_START_PG_LSB_REG 36<<11
#define FRAMES_FROM_START_PG_MSB_REG 37<<11


 
   

#define PLL_PARAM_REG  80<<11//0x37<<11
#define PLL_PARAM_OUT_REG 5<<11 //0x38<<11
#define PLL_CNTRL_REG 81<<11//0x34<<11




#define RX_UDP_AREG    69<<11 //rx_udpip_AReg_c     : integer:= 69; */
#define UDPPORTS_AREG 70<<11// udpports_AReg_c   : integer:= 70; */
#define RX_UDPMACL_AREG 71<<11//rx_udpmacL_AReg_c   : integer:= 71; */
#define RX_UDPMACH_AREG 72<<11//rx_udpmacH_AReg_c   : integer:= 72; */
#define DETECTORMACL_AREG 73<<11//detectormacL_AReg_c : integer:= 73; */
#define DETECTORMACH_AREG 74<<11//detectormacH_AReg_c : integer:= 74; */
#define DETECTORIP_AREG 75<<11//detectorip_AReg_c   : integer:= 75; */
#define IPCHKSUM_AREG 76<<11//ipchksum_AReg_c : integer:= 76; */

#define PATTERN_CNTRL_REG 82<<11
#define PATTERN_LIMITS_AREG 83<<11

#define PATTERN_LOOP0_AREG 84<<11
#define PATTERN_N_LOOP0_REG 85<<11

#define PATTERN_LOOP1_AREG 86<<11
#define PATTERN_N_LOOP1_REG 87<<11

#define PATTERN_LOOP2_AREG 88<<11
#define PATTERN_N_LOOP2_REG 89<<11

#define PATTERN_WAIT0_AREG 90<<11
#define PATTERN_WAIT1_AREG 91<<11
#define PATTERN_WAIT2_AREG 92<<11



#define DAQ_REG   93<<11
#define ADC_LATCH_DISABLE_REG   94<<11

   
#define PATTERN_IOCTRL_REG_LSB 108<<11
#define PATTERN_IOCTRL_REG_MSB 109<<11

#define PATTERN_IOCLKCTRL_REG_LSB 110<<11
#define PATTERN_IOCLKCTRL_REG_MSB 111<<11
#define PATTERN_IN_REG_LSB 112<<11
#define PATTERN_IN_REG_MSB 113<<11
#define PATTERN_WAIT0_TIME_REG_LSB 114<<11
#define PATTERN_WAIT0_TIME_REG_MSB 115<<11
#define PATTERN_WAIT1_TIME_REG_LSB 116<<11
#define PATTERN_WAIT1_TIME_REG_MSB 117<<11
#define PATTERN_WAIT2_TIME_REG_LSB 118<<11
#define PATTERN_WAIT2_TIME_REG_MSB 119<<11
   
#define DAC_REG_OFF 120 
#define DAC_0_1_VAL_REG 120<<11 
#define DAC_2_3_VAL_REG 121<<11  
#define DAC_4_5_VAL_REG 122<<11  
#define DAC_6_7_VAL_REG 123<<11  
#define DAC_8_9_VAL_REG 124<<11  
#define DAC_10_11_VAL_REG 125<<11  
#define DAC_12_13_VAL_REG 126<<11  
#define DAC_14_15_VAL_REG 127<<11  
   
 
 






/* registers defined in FPGA */
#define GAIN_REG              0
//#define FLOW_CONTROL_REG      0x11<<11
//#define FLOW_STATUS_REG       0x12<<11
//#define FRAME_REG             0x13<<11
#define MULTI_PURPOSE_REG     0
//#define TIME_FROM_START_REG   0x16<<11


#define ROI_REG 0 // 0x35<<11
#define OVERSAMPLING_REG 0 // 0x36<<11
#define MOENCH_CNTR_REG 0 // 0x31<<11
#define MOENCH_CNTR_OUT_REG 0 // 0x33<<11
#define MOENCH_CNTR_CONF_REG 0 // 0x32<<11



//image
#define DARK_IMAGE_REG     0 // 0x81<<11
#define GAIN_IMAGE_REG     0 // 0x82<<11

//counter block memory
#define COUNTER_MEMORY_REG 0 // 0x85<<11


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
#define PLL_RECONFIG_BUSY     		 0x00100000
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
#define GB10_NOT_CPU_BIT		 0x00001000



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

#define PLL_VCO_FREQ_MHZ 480//800




/**------------------
-- pattern registers definitions
--------------------------------------------- */
#define  IOSIGNALS_MASK 0xfffffffffffff
#define ADC_ENABLE_BIT 63
#define  APATTERN_MASK 0x3ff
#define ASTART_OFFSET 0
#define ASTOP_OFFSET 16
#define PATTERN_CTRL_WRITE_BIT 0
#define PATTERN_CTRL_READ_BIT 1
#define PATTERN_CTRL_ADDR_OFFSET 16
#define MAX_PATTERN_LENGTH 1024


#endif

