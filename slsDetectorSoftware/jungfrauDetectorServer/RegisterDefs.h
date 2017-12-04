#ifndef REGISTER_DEFS_H
#define REGISTER_DEFS_H

/* Definitions for FPGA*/

/* FPGA Version register */
#define FPGA_VERSION_REG      			(0x00 << 11)

#define BOARD_REVISION_OFST				(0)
#define BOARD_REVISION_MSK				(0x00FFFFFF << BOARD_REVISION_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)



/* Fix pattern register */
#define FIX_PATT_REG          			(0x01 << 11)

/* Status register */
#define STATUS_REG            			(0x02 << 11)

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
#define LOOK_AT_ME_REG          		(0x03 << 11)								//Not used in firmware or software

/* System Status register */
#define SYSTEM_STATUS_REG       		(0x04 << 11)								//Not used in software

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
#define MOD_SERIAL_NUM_REG				(0x0A << 11)								//Not used in software

#define HARDWARE_SERIAL_NUM_OFST		(0)											//Not used in software
#define HARDWARE_SERIAL_NUM_MSK			(0x000000FF << HARDWARE_SERIAL_NUM_OFST)	//Not used in software
#define HARDWARE_VERSION_NUM_OFST		(16)										//Not used in software
#define HARDWARE_VERSION_NUM_MSK		(0x0000003F << HARDWARE_VERSION_NUM_OFST)	//Not used in software


/* Time from Start 64 bit register */
#define TIME_FROM_START_LSB_REG   		(0x10 << 11)
#define TIME_FROM_START_MSB_REG   		(0x11 << 11)

/* Get Delay 64 bit register */
#define GET_DELAY_LSB_REG     			(0x12 << 11)
#define GET_DELAY_MSB_REG     			(0x13 << 11)

/* Get Cycles 64 bit register */
#define GET_CYCLES_LSB_REG    			(0x14 << 11)
#define GET_CYCLES_MSB_REG    			(0x15 << 11)

/* Get Frames 64 bit register */
#define GET_FRAMES_LSB_REG   			(0x16 << 11)
#define GET_FRAMES_MSB_REG    			(0x17 << 11)

/* Get Period 64 bit register */
#define GET_PERIOD_LSB_REG    			(0x18 << 11)
#define GET_PERIOD_MSB_REG    			(0x19 << 11)

/** Get Temperature Carlos, incorrectl as get gates */
#define GET_TEMPERATURE_TMP112_REG		(0x1c << 11)							// (after multiplying by 625) in 10ths of millidegrees of TMP112

#define TEMPERATURE_POLARITY_BIT		(15)
#define TEMPERATURE_POLARITY_MSK		(0x00000001 << TEMPERATURE_POLARITY_BIT)
#define TEMPERATURE_VALUE_BIT			(0)
#define TEMPERATURE_VALUE_MSK			(0x00007FFF << TEMPERATURE_VALUE_BIT)


/* Get Frames from Start 64 bit register (frames from start Run Control) */
#define FRAMES_FROM_START_PG_LSB_REG	(0x24 << 11)
#define FRAMES_FROM_START_PG_MSB_REG 	(0x25 << 11)

/* Measurement Time 64 bit register (timestamp at a frame start until reset)*/
#define START_FRAME_TIME_LSB_REG		(0x26 << 11)
#define START_FRAME_TIME_MSB_REG 		(0x27 << 11)

/* SPI (Serial Peripheral Interface) Register */
#define SPI_REG							(0x40 << 11)

#define DAC_SERIAL_DIGITAL_OUT_OFST		(0)
#define DAC_SERIAL_DIGITAL_OUT_MSK		(0x00000001 << DAC_SERIAL_DIGITAL_OUT_OFST)
#define DAC_SERIAL_CLK_OUT_OFST			(1)
#define DAC_SERIAL_CLK_OUT_MSK			(0x00000001 << DAC_SERIAL_CLK_OUT_OFST)
#define DAC_SERIAL_CS_OUT_OFST			(2)
#define DAC_SERIAL_CS_OUT_MSK			(0x00000001 << DAC_SERIAL_CS_OUT_OFST)
#define HV_SERIAL_DIGITAL_OUT_OFST		(8)
#define HV_SERIAL_DIGITAL_OUT_MSK		(0x00000001 << HV_SERIAL_DIGITAL_OUT_OFST)
#define HV_SERIAL_CLK_OUT_OFST			(9)
#define HV_SERIAL_CLK_OUT_MSK			(0x00000001 << HV_SERIAL_CLK_OUT_OFST)
#define HV_SERIAL_CS_OUT_OFST			(10)
#define HV_SERIAL_CS_OUT_MSK			(0x00000001 << HV_SERIAL_CS_OUT_OFST)


/* ADC SPI (Serial Peripheral Interface) Register */
#define ADC_SPI_REG   					(0x41 << 11)

#define ADC_SERIAL_CLK_OUT_OFST			(0)
#define ADC_SERIAL_CLK_OUT_MSK			(0x00000001 << ADC_SERIAL_CLK_OUT_OFST)
#define ADC_SERIAL_DATA_OUT_OFST		(1)
#define ADC_SERIAL_DATA_OUT_MSK			(0x00000001 << ADC_SERIAL_DATA_OUT_OFST)
#define ADC_SERIAL_CS_OUT_OFST			(2)
#define ADC_SERIAL_CS_OUT_MSK			(0x0000000F << ADC_SERIAL_CS_OUT_OFST)

/* ADC offset Register */
#define ADC_OFST_REG 					(0x42 << 11)

/* ADC Port Invert Register */
#define ADC_PORT_INVERT_REG   			(0x43 << 11)

/* Receiver IP Address Register */
#define RX_IP_REG    					(0x45 << 11)

/* UDP Port */
#define UDP_PORT_REG    				(0x46 << 11)

#define UDP_PORT_RX_OFST				(0)
#define UDP_PORT_RX_MSK					(0x0000FFFF << UDP_PORT_RX_OFST)
#define UDP_PORT_TX_OFST				(16)
#define UDP_PORT_TX_MSK					(0x0000FFFF << UDP_PORT_TX_OFST)

/* Receiver Mac Address 64 bit Register */
#define RX_MAC_LSB_REG					(0x47 << 11)
#define RX_MAC_MSB_REG					(0x48 << 11)

#define RX_MAC_LSB_OFST					(0)
#define RX_MAC_LSB_MSK					(0xFFFFFFFF << RX_MAC_LSB_OFST)
#define RX_MAC_MSB_OFST					(0)
#define RX_MAC_MSB_MSK					(0x0000FFFF << RX_MAC_MSB_OFST)

/* Detector/ Transmitter Mac Address 64 bit Register */
#define TX_MAC_LSB_REG					(0x49 << 11)
#define TX_MAC_MSB_REG					(0x4A << 11)

#define TX_MAC_LSB_OFST					(0)
#define TX_MAC_LSB_MSK					(0xFFFFFFFF << TX_MAC_LSB_OFST)
#define TX_MAC_MSB_OFST					(0)
#define TX_MAC_MSB_MSK					(0x0000FFFF << TX_MAC_MSB_OFST)

/* Detector/ Transmitter IP Address Register */
#define TX_IP_REG						(0x4B << 11)

/* Detector/ Transmitter IP Checksum Register */
#define TX_IP_CHECKSUM_REG				(0x4C << 11)

#define TX_IP_CHECKSUM_OFST				(0)
#define TX_IP_CHECKSUM_MSK				(0x0000FFFF << TX_IP_CHECKSUM_OFST)

/* Configuration Register */
#define CONFIG_REG            			(0x4D << 11)

#define CONFIG_OPERATION_MODE_OFST		(16)
#define CONFIG_OPERATION_MODE_MSK		(0x00000001 << CONFIG_OPERATION_MODE_OFST)
#define CONFIG_MODE_1_X_10GBE_VAL		((0x0 << CONFIG_OPERATION_MODE_OFST) & CONFIG_OPERATION_MODE_MSK)
#define CONFIG_MODE_2_X_10GBE_VAL		((0x1 << CONFIG_OPERATION_MODE_OFST) & CONFIG_OPERATION_MODE_MSK)
#define CONFIG_READOUT_SPEED_OFST		(20)
#define CONFIG_READOUT_SPEED_MSK		(0x00000003 << CONFIG_READOUT_SPEED_OFST)
#define CONFIG_QUARTER_SPEED_10MHZ_VAL	((0x0 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_HALF_SPEED_20MHZ_VAL		((0x1 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_FULL_SPEED_40MHZ_VAL		((0x2 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_TDMA_OFST				(24)
#define CONFIG_TDMA_MSK					(0x00000001 << CONFIG_TDMA_OFST)
#define CONFIG_TDMA_DISABLE_VAL			((0x0 << CONFIG_TDMA_OFST) & CONFIG_TDMA_MSK)
#define CONFIG_TDMA_ENABLE_VAL			((0x1 << CONFIG_TDMA_OFST) & CONFIG_TDMA_MSK)
#define CONFIG_TDMA_TIMESLOT_OFST		(25)
#define CONFIG_TDMA_TIMESLOT_MSK		(0x0000001F << CONFIG_TDMA_TIMESLOT_OFST)
#define CONFIG_TDMA_TIMESLOT_0_VAL		((0x0 << CONFIG_TDMA_TIMESLOT_OFST) & CONFIG_TDMA_TIMESLOT_MSK)

/* External Signal Register */
#define EXT_SIGNAL_REG        			(0x4E << 11)

#define EXT_SIGNAL_OFST					(0)
#define EXT_SIGNAL_MSK					(0x00000003 << EXT_SIGNAL_OFST)				//enabled when both bits high

/* Control Register */
#define CONTROL_REG           			(0x4F << 11)

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

/* Reconfiguratble PLL Paramater Register */
#define PLL_PARAM_REG					(0x50 << 11)

/* Reconfiguratble PLL Control Regiser */
#define PLL_CONTROL_REG					(0x51 << 11)

#define PLL_CTRL_RECONFIG_RST_OFST		(0)											//parameter reset
#define PLL_CTRL_RECONFIG_RST_MSK		(0x00000001 << PLL_CTRL_RECONFIG_RST_OFST)	//parameter reset
#define PLL_CTRL_WR_PARAMETER_OFST		(2)
#define PLL_CTRL_WR_PARAMETER_MSK		(0x00000001 << PLL_CTRL_WR_PARAMETER_OFST)
#define PLL_CTRL_RST_OFST				(3)
#define PLL_CTRL_RST_MSK				(0x00000001 << PLL_CTRL_RST_OFST)
#define PLL_CTRL_ADDR_OFST				(16)
#define PLL_CTRL_ADDR_MSK				(0x0000003F << PLL_CTRL_ADDR_OFST)

/* Sample Register (Obsolete) */
#define SAMPLE_REG 						(0x59 << 11)

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
#define SAMPLE_DECMT_FACTOR_1_VAL		((0x0 << SAMPLE_DGTL_DECMT_FACTOR_OFST) & SAMPLE_DGTL_DECMT_FACTOR_MSK)
#define SAMPLE_DECMT_FACTOR_2_VAL		((0x1 << SAMPLE_DGTL_DECMT_FACTOR_OFST) & SAMPLE_DGTL_DECMT_FACTOR_MSK)
#define SAMPLE_DECMT_FACTOR_4_VAL		((0x2 << SAMPLE_DGTL_DECMT_FACTOR_OFST) & SAMPLE_DGTL_DECMT_FACTOR_MSK)

/** Vref Comp Mod Register */
#define VREF_COMP_MOD_REG				(0x5C << 11)								//Not used in software, TBD in firmware

/** DAQ Register */
#define DAQ_REG							(0x5D << 11)								//TBD in firmware

/** Chip Power Register */
#define CHIP_POWER_REG					(0x5E << 11)

#define CHIP_POWER_ENABLE_OFST			(0)
#define CHIP_POWER_ENABLE_MSK			(0x00000001 << CHIP_POWER_ENABLE_OFST)

/* Set Delay 64 bit register */
#define SET_DELAY_LSB_REG     			(0x60 << 11)
#define SET_DELAY_MSB_REG     			(0x61 << 11)

/* Set Cycles 64 bit register */
#define SET_CYCLES_LSB_REG    			(0x62 << 11)
#define SET_CYCLES_MSB_REG    			(0x63 << 11)

/* Set Frames 64 bit register */
#define SET_FRAMES_LSB_REG   			(0x64 << 11)
#define SET_FRAMES_MSB_REG    			(0x65 << 11)

/* Set Period 64 bit register */
#define SET_PERIOD_LSB_REG    			(0x66 << 11)
#define SET_PERIOD_MSB_REG    			(0x67 << 11)

/* Set Period 64 bit register */
#define SET_EXPTIME_LSB_REG    			(0x68 << 11)
#define SET_EXPTIME_MSB_REG    			(0x69 << 11)

/* Module Coordinates Register 0 */
#define COORD_0							(0x7C << 11)

#define COORD_0_Y_OFST					(0)
#define COORD_0_Y_MSK					(0x0000FFFF << COORD_0_Y_OFST)
#define COORD_0_X_OFST					(16)
#define COORD_0_X_MSK					(0x0000FFFF << COORD_0_X_OFST)

/* Module Coordinates Register 1 */
#define COORD_1							(0x7D << 11)

#define COORD_0_Z_OFST					(0)
#define COORD_0_Z_MSK					(0x0000FFFF << COORD_0_Z_OFST)


#endif  //REGISTERS_G_H















