#pragma once
#include "sls_detector_defs.h"
#include "RegisterDefs.h"


#define GOODBYE 					(-200)
#define MIN_REQRD_VRSN_T_RD_API     0x181130
#define REQRD_FRMWR_VRSN            0x181130

#define PROGRAMMING_MODE            (0x2)
#define CTRL_SRVR_INIT_TIME_US      (1 * 1000 * 1000)

/* Struct Definitions */
typedef struct ip_header_struct {
	uint16_t     ip_len;
	uint8_t      ip_tos;
	uint8_t      ip_ihl:4 ,ip_ver:4;
	uint16_t     ip_offset:13,ip_flag:3;
	uint16_t     ip_ident;
	uint16_t     ip_chksum;
	uint8_t      ip_protocol;
	uint8_t      ip_ttl;
	uint32_t     ip_sourceip;
	uint32_t     ip_destip;
} ip_header;

/* Enums */
enum CLKINDEX               {RUN_CLK, ADC_CLK, SYNC_CLK, DBIT_CLK, NUM_CLOCKS};
enum ADCINDEX				{V_PWR_IO, V_PWR_A, V_PWR_B, V_PWR_C, V_PWR_D, I_PWR_IO, I_PWR_A, I_PWR_B, I_PWR_C, I_PWR_D};
enum DACINDEX               {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
                             D10, D11, D12, D13, D14, D15, D16, D17,
                             D_PWR_D, D_PWR_CHIP, D_PWR_C, D_PWR_B, D_PWR_A, D_PWR_IO};

/* Hardware Definitions */
#define NCHAN 						(36)
#define NCHAN_ANALOG                (32)
#define NCHAN_DIGITAL               (4)
#define NCHIP 						(1)
#define NDAC 						(24)
#define NPWR                        (6)
#define NDAC_ONLY                   (NDAC - NPWR)
#define DYNAMIC_RANGE               (16)
#define NUM_BYTES_PER_PIXEL         (DYNAMIC_RANGE / 8)
#define CLK_FREQ					(156.25)	/* MHz */
#define I2C_POWER_VIO_DEVICE_ID     (0x40)
#define I2C_POWER_VA_DEVICE_ID      (0x41)
#define I2C_POWER_VB_DEVICE_ID      (0x42)
#define I2C_POWER_VC_DEVICE_ID      (0x43)
#define I2C_POWER_VD_DEVICE_ID      (0x44)
#define I2C_SHUNT_RESISTER_OHMS     (0.005)

/** Default Parameters */
#define DEFAULT_DATA_BYTES          (NCHIP * NCHAN * NUM_BITS_PER_PIXEL)
#define DEFAULT_NUM_SAMPLES         (1)
#define DEFAULT_NUM_FRAMES			(100 * 1000 * 1000)
#define DEFAULT_EXPTIME				(0)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_PERIOD				(1 * 1000 * 1000)	//ns
#define DEFAULT_DELAY				(0)
#define DEFAULT_HIGH_VOLTAGE		(0)
#define DEFAULT_VLIMIT              (-100)
#define DEFAULT_TIMING_MODE			(AUTO_TIMING)
#define DEFAULT_TX_UDP_PORT			(0x7e9a)
#define DEFAULT_RUN_CLK             (40)
#define DEFAULT_ADC_CLK             (20)
#define DEFAULT_SYNC_CLK            (20)
#define DEFAULT_DBIT_CLK            (200)

#define HIGHVOLTAGE_MIN             (60)
#define HIGHVOLTAGE_MAX             (200) // min dac val
#define DAC_MIN_MV                  (0)
#define DAC_MAX_MV                  (2500)
#define VCHIP_MIN_MV                (1673)
#define VCHIP_MAX_MV                (2668) // min dac val
#define POWER_RGLTR_MIN             (636)
#define POWER_RGLTR_MAX             (2638) // min dac val (not vchip-max) because of dac conversions
#define VCHIP_POWER_INCRMNT         (200)
#define VIO_MIN_MV                  (1200) // for fpga to function

/* Defines in the Firmware */
#define MAX_PATTERN_LENGTH  		(0xFFFF)
#define DIGITAL_IO_DELAY_MAXIMUM_PS	((OUTPUT_DELAY_0_OTPT_STTNG_MSK >> OUTPUT_DELAY_0_OTPT_STTNG_OFST) * OUTPUT_DELAY_0_OTPT_STTNG_STEPS)


#define WAIT_TME_US_FR_ACQDONE_REG 	(100) // wait time in us after acquisition done to ensure there is no data in fifo
#define WAIT_TIME_US_PLL            (10 * 1000)
#define WAIT_TIME_US_STP_ACQ        (100)
#define WAIT_TIME_CONFIGURE_MAC     (500 * 1000)
#define WAIT_TIME_PATTERN_READ     	(10)
#define WAIT_TIME_FIFO_RD_STROBE  	(10)

/* MSB & LSB DEFINES */
#define MSB_OF_64_BIT_REG_OFST		(32)
#define LSB_OF_64_BIT_REG_OFST		(0)
#define BIT_32_MSK					(0xFFFFFFFF)

#define IP_PACKETSIZE               (0x2032)
#define ADC_PORT_INVERT_VAL         (0x453b2593)
#define MAXIMUM_ADC_CLK             (40)
#define PLL_VCO_FREQ_MHZ            (400)

