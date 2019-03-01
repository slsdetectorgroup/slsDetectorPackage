#pragma once

/* Definitions for FPGA*/
#define MEM_MAP_SHIFT 1

/* FPGA Version register */
#define FPGA_VERSION_REG      			(0x00 << MEM_MAP_SHIFT)

#define BOARD_REVISION_OFST				(0)
#define BOARD_REVISION_MSK				(0x00FFFFFF << BOARD_REVISION_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)



/* Fix pattern register */
#define FIX_PATT_REG          			(0x01 << MEM_MAP_SHIFT)

#define FIX_PATT_VAL                    (0xACDC2014)

/* Status register */
#define STATUS_REG            			(0x02 << MEM_MAP_SHIFT)

#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)
#define WAITING_FOR_TRIGGER_OFST  		(3)
#define WAITING_FOR_TRIGGER_MSK  		(0x00000001 << WAITING_FOR_TRIGGER_OFST)
#define DELAYBEFORE_OFST  				(4)											//Not used in software
#define DELAYBEFORE_MSK  				(0x00000001 << DELAYBEFORE_OFST)			//Not used in software
#define DELAYAFTER_OFST  				(5)											//Not used in software
#define DELAYAFTER_MSK  				(0x00000001 << DELAYAFTER_OFST)				//Not used in software
#define STOPPED_OFST  					(15)
#define STOPPED_MSK  					(0x00000001 << STOPPED_OFST)
#define RUNMACHINE_BUSY_OFST  			(17)
#define RUNMACHINE_BUSY_MSK  			(0x00000001 << RUNMACHINE_BUSY_OFST)


/* Look at me register */
#define LOOK_AT_ME_REG          		(0x03 << MEM_MAP_SHIFT)								//Not used in firmware or software

/* System Status register */
#define SYSTEM_STATUS_REG       		(0x04 << MEM_MAP_SHIFT)								//Not used in software

#define DDR3_CAL_DONE_OFST				(0)											//Not used in software
#define DDR3_CAL_DONE_MSK 				(0x00000001 << DDR3_CAL_DONE_OFST)			//Not used in software
#define DDR3_CAL_FAIL_OFST				(1)											//Not used in software
#define DDR3_CAL_FAIL_MSK 				(0x00000001 << DDR3_CAL_FAIL_OFST)			//Not used in software
#define DDR3_INIT_DONE_OFST				(2)											//Not used in software
#define DDR3_INIT_DONE_MSK 				(0x00000001 << DDR3_INIT_DONE_OFST)			//Not used in software
#define RECONFIG_PLL_LCK_OFST			(3)											//Not used in software
#define RECONFIG_PLL_LCK_MSK 			(0x00000001 << RECONFIG_PLL_LCK_OFST)		//Not used in software
#define PLL_A_LCK_OFST					(4)											//Not used in software
#define PLL_A_LCK_MSK 					(0x00000001 << PLL_A_LCK_OFST)				//Not used in software
#define DD3_PLL_LCK_OFST				(5)											//Not used in software
#define DD3_PLL_LCK_MSK 				(0x00000001 << DD3_PLL_LCK_OFST)			//Not used in software


/* Module Control Board Serial Number Register */
#define MOD_SERIAL_NUM_REG				(0x0A << MEM_MAP_SHIFT)								//Not used in software

#define HARDWARE_SERIAL_NUM_OFST		(0)											//Not used in software
#define HARDWARE_SERIAL_NUM_MSK			(0x000000FF << HARDWARE_SERIAL_NUM_OFST)	//Not used in software
#define HARDWARE_VERSION_NUM_OFST		(16)										//Not used in software
#define HARDWARE_VERSION_NUM_MSK		(0x0000003F << HARDWARE_VERSION_NUM_OFST)	//Not used in software


/* API Version Register */
#define API_VERSION_REG                 (0x0F << MEM_MAP_SHIFT)

#define API_VERSION_OFST                (0)
#define API_VERSION_MSK                 (0x00FFFFFF << API_VERSION_OFST)
#define API_VERSION_DETECTOR_TYPE_OFST  (24)                                            //Not used in software
#define API_VERSION_DETECTOR_TYPE_MSK   (0x000000FF << API_VERSION_DETECTOR_TYPE_OFST)  //Not used in software

/* Time from Start 64 bit register */
#define TIME_FROM_START_LSB_REG   		(0x10 << MEM_MAP_SHIFT)
#define TIME_FROM_START_MSB_REG   		(0x11 << MEM_MAP_SHIFT)

/* Get Delay 64 bit register */
#define GET_DELAY_LSB_REG     			(0x12 << MEM_MAP_SHIFT)						// different kind of delay
#define GET_DELAY_MSB_REG     			(0x13 << MEM_MAP_SHIFT)						// different kind of delay

/* Get Cycles 64 bit register */
#define GET_CYCLES_LSB_REG    			(0x14 << MEM_MAP_SHIFT)
#define GET_CYCLES_MSB_REG    			(0x15 << MEM_MAP_SHIFT)

/* Get Frames 64 bit register */
#define GET_FRAMES_LSB_REG   			(0x16 << MEM_MAP_SHIFT)
#define GET_FRAMES_MSB_REG    			(0x17 << MEM_MAP_SHIFT)

/* Get Period 64 bit register tT = T x 50 ns */
#define GET_PERIOD_LSB_REG    			(0x18 << MEM_MAP_SHIFT)
#define GET_PERIOD_MSB_REG    			(0x19 << MEM_MAP_SHIFT)

/** Get Temperature Carlos, incorrectl as get gates */
#define GET_TEMPERATURE_TMP112_REG		(0x1c << MEM_MAP_SHIFT)							// (after multiplying by 625) in 10ths of millidegrees of TMP112

#define TEMPERATURE_VALUE_BIT           (0)
#define TEMPERATURE_VALUE_MSK           (0x000007FF << TEMPERATURE_VALUE_BIT)
#define TEMPERATURE_POLARITY_BIT		(11)
#define TEMPERATURE_POLARITY_MSK		(0x00000001 << TEMPERATURE_POLARITY_BIT)



/* Get Frames from Start 64 bit register (frames from last reset using CONTROL_CRST) */
#define FRAMES_FROM_START_PG_LSB_REG	(0x24 << MEM_MAP_SHIFT)
#define FRAMES_FROM_START_PG_MSB_REG 	(0x25 << MEM_MAP_SHIFT)

/* Measurement Time 64 bit register (timestamp at a frame start until reset)*/
#define START_FRAME_TIME_LSB_REG		(0x26 << MEM_MAP_SHIFT)
#define START_FRAME_TIME_MSB_REG 		(0x27 << MEM_MAP_SHIFT)

/* SPI (Serial Peripheral Interface) Register */
#define SPI_REG							(0x40 << MEM_MAP_SHIFT)

#define SPI_DAC_SRL_DGTL_OTPT_OFST          (0)
#define SPI_DAC_SRL_DGTL_OTPT_MSK           (0x00000001 << SPI_DAC_SRL_DGTL_OTPT_OFST)
#define SPI_DAC_SRL_CLK_OTPT_OFST           (1)
#define SPI_DAC_SRL_CLK_OTPT_MSK            (0x00000001 << SPI_DAC_SRL_CLK_OTPT_OFST)
#define SPI_DAC_SRL_CS_OTPT_OFST            (2)
#define SPI_DAC_SRL_CS_OTPT_MSK             (0x00000001 << SPI_DAC_SRL_CS_OTPT_OFST)
#define SPI_HV_SRL_DGTL_OTPT_OFST           (8)
#define SPI_HV_SRL_DGTL_OTPT_MSK            (0x00000001 << SPI_HV_SRL_DGTL_OTPT_OFST)
#define SPI_HV_SRL_CLK_OTPT_OFST            (9)
#define SPI_HV_SRL_CLK_OTPT_MSK             (0x00000001 << SPI_HV_SRL_CLK_OTPT_OFST)
#define SPI_HV_SRL_CS_OTPT_OFST             (10)
#define SPI_HV_SRL_CS_OTPT_MSK              (0x00000001 << SPI_HV_SRL_CS_OTPT_OFST)

/* ADC SPI (Serial Peripheral Interface) Register */
#define ADC_SPI_REG   					(0x41 << MEM_MAP_SHIFT)

#define ADC_SPI_SRL_CLK_OTPT_OFST       (0)
#define ADC_SPI_SRL_CLK_OTPT_MSK        (0x00000001 << ADC_SPI_SRL_CLK_OTPT_OFST)
#define ADC_SPI_SRL_DT_OTPT_OFST        (1)
#define ADC_SPI_SRL_DT_OTPT_MSK         (0x00000001 << ADC_SPI_SRL_DT_OTPT_OFST)
#define ADC_SPI_SRL_CS_OTPT_OFST        (2)
#define ADC_SPI_SRL_CS_OTPT_MSK         (0x0000000F << ADC_SPI_SRL_CS_OTPT_OFST)

/* ADC offset Register */
#define ADC_OFST_REG 					(0x42 << MEM_MAP_SHIFT)

/* ADC Port Invert Register */
#define ADC_PORT_INVERT_REG   			(0x43 << MEM_MAP_SHIFT)

#define ADC_PORT_INVERT_ADC_0_OFST      (0)
#define ADC_PORT_INVERT_ADC_0_MSK       (0x000000FF << ADC_PORT_INVERT_ADC_0_OFST)
#define ADC_PORT_INVERT_ADC_1_OFST      (8)
#define ADC_PORT_INVERT_ADC_1_MSK       (0x000000FF << ADC_PORT_INVERT_ADC_1_OFST)
#define ADC_PORT_INVERT_ADC_2_OFST      (16)
#define ADC_PORT_INVERT_ADC_2_MSK       (0x000000FF << ADC_PORT_INVERT_ADC_2_OFST)
#define ADC_PORT_INVERT_ADC_3_OFST      (24)
#define ADC_PORT_INVERT_ADC_3_MSK       (0x000000FF << ADC_PORT_INVERT_ADC_3_OFST)

/* Receiver IP Address Register */
#define RX_IP_REG    					(0x45 << MEM_MAP_SHIFT)

/* UDP Port */
#define UDP_PORT_REG    				(0x46 << MEM_MAP_SHIFT)

#define UDP_PORT_RX_OFST				(0)
#define UDP_PORT_RX_MSK					(0x0000FFFF << UDP_PORT_RX_OFST)
#define UDP_PORT_TX_OFST				(16)
#define UDP_PORT_TX_MSK					(0x0000FFFF << UDP_PORT_TX_OFST)

/* Receiver Mac Address 64 bit Register */
#define RX_MAC_LSB_REG					(0x47 << MEM_MAP_SHIFT)
#define RX_MAC_MSB_REG					(0x48 << MEM_MAP_SHIFT)

#define RX_MAC_LSB_OFST					(0)
#define RX_MAC_LSB_MSK					(0xFFFFFFFF << RX_MAC_LSB_OFST)
#define RX_MAC_MSB_OFST					(0)
#define RX_MAC_MSB_MSK					(0x0000FFFF << RX_MAC_MSB_OFST)

/* Detector/ Transmitter Mac Address 64 bit Register */
#define TX_MAC_LSB_REG					(0x49 << MEM_MAP_SHIFT)
#define TX_MAC_MSB_REG					(0x4A << MEM_MAP_SHIFT)

#define TX_MAC_LSB_OFST					(0)
#define TX_MAC_LSB_MSK					(0xFFFFFFFF << TX_MAC_LSB_OFST)
#define TX_MAC_MSB_OFST					(0)
#define TX_MAC_MSB_MSK					(0x0000FFFF << TX_MAC_MSB_OFST)

/* Detector/ Transmitter IP Address Register */
#define TX_IP_REG						(0x4B << MEM_MAP_SHIFT)

/* Detector/ Transmitter IP Checksum Register */
#define TX_IP_CHECKSUM_REG				(0x4C << MEM_MAP_SHIFT)

#define TX_IP_CHECKSUM_OFST				(0)
#define TX_IP_CHECKSUM_MSK				(0x0000FFFF << TX_IP_CHECKSUM_OFST)

/* Configuration Register */
#define CONFIG_REG            			(0x4D << MEM_MAP_SHIFT)

// readout timer (from chip) to stabilize (esp in burst acquisition mode) tRDT = (RDT + 1) * 25ns
#define CONFIG_RDT_TMR_OFST             (0)
#define CONFIG_RDT_TMR_MSK              (0x0000FFFF << CONFIG_RDT_TMR_OFST)
#define CONFIG_OPRTN_MDE_2_X_10GbE_OFST (16)
#define CONFIG_OPRTN_MDE_2_X_10GbE_MSK  (0x00000001 << CONFIG_OPRTN_MDE_2_X_10GbE_OFST)
#define CONFIG_OPRTN_MDE_1_X_10GBE_VAL  ((0x0 << CONFIG_OPRTN_MDE_2_X_10GbE_OFST) & CONFIG_OPRTN_MDE_2_X_10GbE_MSK)
#define CONFIG_READOUT_SPEED_OFST		(20)
#define CONFIG_READOUT_SPEED_MSK		(0x00000003 << CONFIG_READOUT_SPEED_OFST)
#define CONFIG_QUARTER_SPEED_10MHZ_VAL	((0x0 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_HALF_SPEED_20MHZ_VAL		((0x1 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_FULL_SPEED_40MHZ_VAL		((0x2 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_TDMA_OFST				(24)
#define CONFIG_TDMA_MSK					(0x00000001 << CONFIG_TDMA_OFST)
#define CONFIG_TDMA_DISABLE_VAL         ((0x0 << CONFIG_TDMA_OFST) & CONFIG_TDMA_MSK)
#define CONFIG_TDMA_TIMESLOT_OFST       (25)  // 1ms
#define CONFIG_TDMA_TIMESLOT_MSK		(0x0000001F << CONFIG_TDMA_TIMESLOT_OFST)
#define CONFIG_ETHRNT_FLW_CNTRL_OFST    (31)
#define CONFIG_ETHRNT_FLW_CNTRL_MSK     (0x00000001 << CONFIG_ETHRNT_FLW_CNTRL_OFST)

/* External Signal Register */
#define EXT_SIGNAL_REG        			(0x4E << MEM_MAP_SHIFT)

#define EXT_SIGNAL_OFST					(0)
#define EXT_SIGNAL_MSK					(0x00000001 << EXT_SIGNAL_OFST)

/* Control Register */
#define CONTROL_REG           			(0x4F << MEM_MAP_SHIFT)

#define CONTROL_START_ACQ_OFST       	(0)
#define CONTROL_START_ACQ_MSK			(0x00000001 << CONTROL_START_ACQ_OFST)
#define CONTROL_STOP_ACQ_OFST			(1)
#define CONTROL_STOP_ACQ_MSK			(0x00000001 << CONTROL_STOP_ACQ_OFST)
#define CONTROL_CORE_RST_OFST			(10)
#define CONTROL_CORE_RST_MSK			(0x00000001 << CONTROL_CORE_RST_OFST)
#define CONTROL_PERIPHERAL_RST_OFST		(11)										//DDR3 HMem Ctrlr, GBE, Temp
#define CONTROL_PERIPHERAL_RST_MSK		(0x00000001 << CONTROL_PERIPHERAL_RST_OFST)	//DDR3 HMem Ctrlr, GBE, Temp
#define CONTROL_DDR3_MEM_RST_OFST		(12)										//only PHY, not DDR3 PLL ,Not used in software
#define CONTROL_DDR3_MEM_RST_MSK		(0x00000001 << CONTROL_DDR3_MEM_RST_OFST)	//only PHY, not DDR3 PLL ,Not used in software
#define CONTROL_ACQ_FIFO_CLR_OFST		(14)
#define CONTROL_ACQ_FIFO_CLR_MSK		(0x00000001 << CONTROL_ACQ_FIFO_CLR_OFST)
#define CONTROL_STORAGE_CELL_NUM_OFST   (16)
#define CONTROL_STORAGE_CELL_NUM_MSK    (0x0000000F << CONTROL_STORAGE_CELL_NUM_OFST)

/* Reconfiguratble PLL Paramater Register */
#define PLL_PARAM_REG					(0x50 << MEM_MAP_SHIFT)

/* Reconfiguratble PLL Control Regiser */
#define PLL_CNTRL_REG					(0x51 << MEM_MAP_SHIFT)

#define PLL_CNTRL_RCNFG_PRMTR_RST_OFST	(0)											//parameter reset
#define PLL_CNTRL_RCNFG_PRMTR_RST_MSK	(0x00000001 << PLL_CNTRL_RCNFG_PRMTR_RST_OFST)	//parameter reset
#define PLL_CNTRL_WR_PRMTR_OFST		    (2)
#define PLL_CNTRL_WR_PRMTR_MSK		    (0x00000001 << PLL_CNTRL_WR_PRMTR_OFST)
#define PLL_CNTRL_PLL_RST_OFST			(3)
#define PLL_CNTRL_PLL_RST_MSK			(0x00000001 << PLL_CNTRL_PLL_RST_OFST)
#define PLL_CNTRL_ADDR_OFST				(16)
#define PLL_CNTRL_ADDR_MSK				(0x0000003F << PLL_CNTRL_ADDR_OFST)

/* Sample Register (Obsolete) */
#define SAMPLE_REG 						(0x59 << MEM_MAP_SHIFT)

#define SAMPLE_ADC_SAMPLE_SEL_OFST		(0)
#define SAMPLE_ADC_SAMPLE_SEL_MSK		(0x00000007 << SAMPLE_ADC_SAMPLE_SEL_OFST)
#define SAMPLE_ADC_SAMPLE_0_VAL			((0x0 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_1_VAL			((0x1 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_2_VAL			((0x2 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_3_VAL			((0x3 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_4_VAL			((0x4 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_5_VAL			((0x5 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_6_VAL			((0x6 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_7_VAL			((0x7 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
// Decimation = ADF + 1
#define SAMPLE_ADC_DECMT_FACTOR_OFST	(4)
#define SAMPLE_ADC_DECMT_FACTOR_MSK		(0x00000007 << SAMPLE_ADC_DECMT_FACTOR_OFST)
#define SAMPLE_ADC_DECMT_FACTOR_0_VAL	((0x0 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_1_VAL	((0x1 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_2_VAL	((0x2 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_3_VAL	((0x3 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_4_VAL	((0x4 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_5_VAL	((0x5 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_6_VAL	((0x6 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_7_VAL	((0x7 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)

#define SAMPLE_DGTL_SAMPLE_SEL_OFST		(8)
#define SAMPLE_DGTL_SAMPLE_SEL_MSK		(0x0000000F << SAMPLE_DGTL_SAMPLE_SEL_OFST)
#define SAMPLE_DGTL_SAMPLE_0_VAL		((0x0 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_1_VAL		((0x1 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_2_VAL		((0x2 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_3_VAL		((0x3 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_4_VAL		((0x4 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_5_VAL		((0x5 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_6_VAL		((0x6 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_7_VAL		((0x7 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_8_VAL		((0x8 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_9_VAL		((0x9 << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_10_VAL		((0xa << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_11_VAL		((0xb << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_12_VAL		((0xc << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_13_VAL		((0xd << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_14_VAL		((0xe << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)
#define SAMPLE_DGTL_SAMPLE_15_VAL		((0xf << SAMPLE_DGTL_SAMPLE_SEL_OFST) & SAMPLE_DGTL_SAMPLE_SEL_MSK)

#define SAMPLE_DGTL_DECMT_FACTOR_OFST	(12)
#define SAMPLE_DGTL_DECMT_FACTOR_MSK	(0x00000003 << SAMPLE_DGTL_DECMT_FACTOR_OFST)
// 1 = full speed, 2 = half speed, 4 = quarter speed
#define SAMPLE_DECMT_FACTOR_1_VAL		((0x0 << SAMPLE_DGTL_DECMT_FACTOR_OFST) & SAMPLE_DGTL_DECMT_FACTOR_MSK)
#define SAMPLE_DECMT_FACTOR_2_VAL		((0x1 << SAMPLE_DGTL_DECMT_FACTOR_OFST) & SAMPLE_DGTL_DECMT_FACTOR_MSK)
#define SAMPLE_DECMT_FACTOR_4_VAL		((0x2 << SAMPLE_DGTL_DECMT_FACTOR_OFST) & SAMPLE_DGTL_DECMT_FACTOR_MSK)

/** Vref Comp Mod Register */
#define VREF_COMP_MOD_REG				(0x5C << MEM_MAP_SHIFT)

#define VREF_COMP_MOD_OFST              (0)
#define VREF_COMP_MOD_MSK               (0x00000FFF << VREF_COMP_MOD_OFST)
#define VREF_COMP_MOD_ENABLE_OFST       (31)
#define VREF_COMP_MOD_ENABLE_MSK        (0x00000001 << VREF_COMP_MOD_ENABLE_OFST)


/** DAQ Register */
#define DAQ_REG							(0x5D << MEM_MAP_SHIFT)

#define DAQ_SETTINGS_MSK                (DAQ_HIGH_GAIN_MSK | DAQ_FIX_GAIN_MSK | DAQ_FRCE_SWTCH_GAIN_MSK)
#define DAQ_HIGH_GAIN_OFST              (0)
#define DAQ_HIGH_GAIN_MSK               (0x00000001 << DAQ_HIGH_GAIN_OFST)
#define DAQ_FIX_GAIN_DYNMC_VAL          ((0x0 << DAQ_HIGH_GAIN_OFST) & DAQ_HIGH_GAIN_MSK)
#define DAQ_FIX_GAIN_HIGHGAIN_VAL       ((0x1 << DAQ_HIGH_GAIN_OFST) & DAQ_HIGH_GAIN_MSK)
#define DAQ_FIX_GAIN_OFST               (1)
#define DAQ_FIX_GAIN_MSK                (0x00000003 << DAQ_FIX_GAIN_OFST)
#define DAQ_FIX_GAIN_STG_1_VAL          ((0x1 << DAQ_FIX_GAIN_OFST) & DAQ_FIX_GAIN_MSK)
#define DAQ_FIX_GAIN_STG_2_VAL          ((0x3 << DAQ_FIX_GAIN_OFST) & DAQ_FIX_GAIN_MSK)
#define DAQ_CMP_RST_OFST                (4)
#define DAQ_CMP_RST_MSK                 (0x00000001 << DAQ_CMP_RST_OFST)
#define DAQ_STRG_CELL_SLCT_OFST         (8)
#define DAQ_STRG_CELL_SLCT_MSK          (0x0000000F << DAQ_STRG_CELL_SLCT_OFST)
#define DAQ_FRCE_SWTCH_GAIN_OFST        (12)
#define DAQ_FRCE_SWTCH_GAIN_MSK         (0x00000003 << DAQ_FRCE_SWTCH_GAIN_OFST)
#define DAQ_FRCE_GAIN_STG_1_VAL         ((0x1 << DAQ_FRCE_SWTCH_GAIN_OFST) & DAQ_FRCE_SWTCH_GAIN_MSK)
#define DAQ_FRCE_GAIN_STG_2_VAL         ((0x3 << DAQ_FRCE_SWTCH_GAIN_OFST) & DAQ_FRCE_SWTCH_GAIN_MSK)
#define DAQ_ELCTRN_CLLCTN_MDE_OFST      (14)
#define DAQ_ELCTRN_CLLCTN_MDE_MSK       (0x00000001 << DAQ_ELCTRN_CLLCTN_MDE_OFST)
#define DAQ_G2_CNNT_OFST                (15)
#define DAQ_G2_CNNT_MSK                 (0x00000001 << DAQ_G2_CNNT_OFST)
#define DAQ_CRRNT_SRC_ENBL_OFST         (16)
#define DAQ_CRRNT_SRC_ENBL_MSK          (0x00000001 << DAQ_CRRNT_SRC_ENBL_OFST)
#define DAQ_CRRNT_SRC_CLMN_FIX_OFST     (17)
#define DAQ_CRRNT_SRC_CLMN_FIX_MSK      (0x00000001 << DAQ_CRRNT_SRC_CLMN_FIX_OFST)
#define DAQ_CRRNT_SRC_CLMN_SLCT_OFST    (20)
#define DAQ_CRRNT_SRC_CLMN_SLCT_MSK     (0x0000003F << DAQ_CRRNT_SRC_CLMN_SLCT_OFST)

/** Chip Power Register */
#define CHIP_POWER_REG					(0x5E << MEM_MAP_SHIFT)

#define CHIP_POWER_ENABLE_OFST			(0)
#define CHIP_POWER_ENABLE_MSK			(0x00000001 << CHIP_POWER_ENABLE_OFST)
#define CHIP_POWER_STATUS_OFST          (1)
#define CHIP_POWER_STATUS_MSK           (0x00000001 << CHIP_POWER_STATUS_OFST)


/** Temperature Control Register */
#define TEMP_CTRL_REG                   (0x5F << MEM_MAP_SHIFT)

#define TEMP_CTRL_PROTCT_THRSHLD_OFST   (0)
#define TEMP_CTRL_PROTCT_THRSHLD_MSK    (0x000007FF << TEMP_CTRL_PROTCT_THRSHLD_OFST)
#define TEMP_CTRL_PROTCT_ENABLE_OFST    (16)
#define TEMP_CTRL_PROTCT_ENABLE_MSK     (0x00000001 << TEMP_CTRL_PROTCT_ENABLE_OFST)
// set when temp higher than over threshold, write 1 to clear it
#define TEMP_CTRL_OVR_TMP_EVNT_OFST     (31)
#define TEMP_CTRL_OVR_TMP_EVNT_MSK      (0x00000001 << TEMP_CTRL_OVR_TMP_EVNT_OFST)


/* Set Delay 64 bit register */
#define SET_DELAY_LSB_REG     			(0x60 << MEM_MAP_SHIFT)					// different kind of delay
#define SET_DELAY_MSB_REG     			(0x61 << MEM_MAP_SHIFT)					// different kind of delay

/* Set Cycles 64 bit register */
#define SET_CYCLES_LSB_REG    			(0x62 << MEM_MAP_SHIFT)
#define SET_CYCLES_MSB_REG    			(0x63 << MEM_MAP_SHIFT)

/* Set Frames 64 bit register */
#define SET_FRAMES_LSB_REG   			(0x64 << MEM_MAP_SHIFT)
#define SET_FRAMES_MSB_REG    			(0x65 << MEM_MAP_SHIFT)

/* Set Period 64 bit register tT = T x 50 ns */
#define SET_PERIOD_LSB_REG    			(0x66 << MEM_MAP_SHIFT)
#define SET_PERIOD_MSB_REG    			(0x67 << MEM_MAP_SHIFT)

/* Set Exptime 64 bit register eEXP = Exp x 25 ns */
#define SET_EXPTIME_LSB_REG    			(0x68 << MEM_MAP_SHIFT)
#define SET_EXPTIME_MSB_REG    			(0x69 << MEM_MAP_SHIFT)

/* Trigger Delay 32 bit register */
#define SET_TRIGGER_DELAY_LSB_REG       (0x70 << MEM_MAP_SHIFT)
#define SET_TRIGGER_DELAY_MSB_REG       (0x71 << MEM_MAP_SHIFT)

/* Module Coordinates Register 0 */
#define COORD_0_REG						(0x7C << MEM_MAP_SHIFT)

#define COORD_0_Y_OFST					(0)
#define COORD_0_Y_MSK					(0x0000FFFF << COORD_0_Y_OFST)
#define COORD_0_X_OFST					(16)
#define COORD_0_X_MSK					(0x0000FFFF << COORD_0_X_OFST)

/* Module Coordinates Register 1 */
#define COORD_1_REG						(0x7D << MEM_MAP_SHIFT)

#define COORD_0_Z_OFST					(0)
#define COORD_0_Z_MSK					(0x0000FFFF << COORD_0_Z_OFST)

/* ASIC Control Register */
#define ASIC_CTRL_REG                   (0x7F << MEM_MAP_SHIFT)
// tPC = (PCT + 1) * 25ns
#define ASIC_CTRL_PRCHRG_TMR_OFST       (0)
#define ASIC_CTRL_PRCHRG_TMR_MSK        (0x000000FF << ASIC_CTRL_PRCHRG_TMR_OFST)
#define ASIC_CTRL_PRCHRG_TMR_VAL        ((0x1F << ASIC_CTRL_PRCHRG_TMR_OFST) & ASIC_CTRL_PRCHRG_TMR_MSK)
// tDS = (DST + 1) * 25ns
#define ASIC_CTRL_DS_TMR_OFST           (8)
#define ASIC_CTRL_DS_TMR_MSK            (0x000000FF << ASIC_CTRL_DS_TMR_OFST)
#define ASIC_CTRL_DS_TMR_VAL            ((0x1F << ASIC_CTRL_DS_TMR_OFST) & ASIC_CTRL_DS_TMR_MSK)
// tET = (ET + 1) * 25ns (increase timeout range between 2 consecutive storage cells)
#define ASIC_CTRL_EXPSRE_TMR_STEPS		(25)
#define ASIC_CTRL_EXPSRE_TMR_OFST       (16)
#define ASIC_CTRL_EXPSRE_TMR_MSK        (0x0000FFFF << ASIC_CTRL_EXPSRE_TMR_OFST)















