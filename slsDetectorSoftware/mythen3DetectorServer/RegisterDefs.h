#ifndef REGISTERS_G_H
#define REGISTERS_G_H

/* Definitions for FPGA*/

/* FPGA Version register */
#define FPGA_VERSION_REG                (0x00 << 11)

#define BOARD_REVISION_OFST             (0)
#define BOARD_REVISION_MSK              (0x00FFFFFF << BOARD_REVISION_OFST)
#define DETECTOR_TYPE_OFST              (24)
#define DETECTOR_TYPE_MSK               (0x000000FF << DETECTOR_TYPE_OFST)

/* Fix pattern register */
#define FIX_PATT_REG                    (0x01 << 11)


/* Timer 64 bit Regiser */
#define SET_DELAY_LSB_REG               (0x60 << 11) //96<<11 //0x68<<11
#define SET_DELAY_MSB_REG               (0x61 << 11) //97<<11 //0x69<<11
#define SET_CYCLES_LSB_REG              (0x62 << 11) //98<<11//0x6c<<11
#define SET_CYCLES_MSB_REG              (0x63 << 11) //99<<11//0x6d<<11
#define SET_FRAMES_LSB_REG              (0x64 << 11) //(100<<11)/** to hex */
#define SET_FRAMES_MSB_REG              (0x65 << 11) //101<<11//0x71<<11
#define SET_PERIOD_LSB_REG              (0x66 << 11) //102<<11//0x74<<11
#define SET_PERIOD_MSB_REG              (0x67 << 11) //103<<11//0x75<<11
#define SET_GATES_LSB_REG               (0x6A << 11) /*check in firmware*///106<<11//0x7c<<11
#define SET_GATES_MSB_REG               (0x6B << 11) //107<<11//0x7d<<11
#define SET_EXPTIME_LSB_REG             (0x72 << 11) /** check in firmware *///114<<11//0x78<<11
#define SET_EXPTIME_MSB_REG             (0x73 << 11) //115<<11//0x79<<11

#define GET_ACTUAL_TIME_LSB_REG         (0x10 << 11) //16<<11
#define GET_ACTUAL_TIME_MSB_REG         (0x11 << 11) //17<<11
#define GET_DELAY_LSB_REG               (0x12 << 11) //18<<11//0x6a<<11
#define GET_DELAY_MSB_REG               (0x13 << 11) //19<<11//0x6b<<11
#define GET_CYCLES_LSB_REG              (0x14 << 11) //20<<11//0x6e<<11
#define GET_CYCLES_MSB_REG              (0x15 << 11) //21<<11//0x6f<<11
#define GET_FRAMES_LSB_REG              (0x16 << 11) //22<<11//0x72<<11
#define GET_FRAMES_MSB_REG              (0x17 << 11) //23<<11//0x73<<11
#define GET_PERIOD_LSB_REG              (0x18 << 11) //24<<11//0x76<<11
#define GET_PERIOD_MSB_REG              (0x19 << 11) //25<<11//0x77<<11
#define GET_EXPTIME_LSB_REG             (0x1A << 11) //26<<11//0x7a<<11
#define GET_EXPTIME_MSB_REG             (0x1B << 11) //27<<11//0x7b<<11
#define GET_GATES_LSB_REG               (0x1C << 11) //28<<11//0x7e<<11
#define GET_GATES_MSB_REG               (0x1D << 11) //29<<11//0x7f<<11

#define FRAMES_FROM_START_LSB_REG       (0x22 << 11) //34<<11
#define FRAMES_FROM_START_MSB_REG       (0x23 << 11) //35<<11
#define FRAMES_FROM_START_PG_LSB_REG    (0x24 << 11) //36<<11
#define FRAMES_FROM_START_PG_MSB_REG    (0x25 << 11) //37<<11
#define GET_MEASUREMENT_TIME_LSB_REG    (0x26 << 11) //38<<11
#define GET_MEASUREMENT_TIME_MSB_REG    (0x27 << 11) //39<<11


/* SPI (Serial Peripheral Interface) Register */
#define SPI_REG                         (0x40 << 11)

#define DAC_SERIAL_DIGITAL_OUT_OFST     (0)
#define DAC_SERIAL_DIGITAL_OUT_MSK      (0x00000001 << DAC_SERIAL_DIGITAL_OUT_OFST)
#define DAC_SERIAL_CLK_OUT_OFST         (1)
#define DAC_SERIAL_CLK_OUT_MSK          (0x00000001 << DAC_SERIAL_CLK_OUT_OFST)
#define DAC_SERIAL_CS_OUT_OFST          (2)
#define DAC_SERIAL_CS_OUT_MSK           (0x00000001 << DAC_SERIAL_CS_OUT_OFST)
#define HV_SERIAL_DIGITAL_OUT_OFST      (8)
#define HV_SERIAL_DIGITAL_OUT_MSK       (0x00000001 << HV_SERIAL_DIGITAL_OUT_OFST)
#define HV_SERIAL_CLK_OUT_OFST          (9)
#define HV_SERIAL_CLK_OUT_MSK           (0x00000001 << HV_SERIAL_CLK_OUT_OFST)
#define HV_SERIAL_CS_OUT_OFST           (10)
#define HV_SERIAL_CS_OUT_MSK            (0x00000001 << HV_SERIAL_CS_OUT_OFST)

/* Control Register */
#define CONTROL_REG                     (0x4F << 11) //(79 << 11) /** to hex */


/* Reconfiguratble PLL Control Regiser */
#define PLL_CONTROL_REG                 (0x51 << 11) //(81 << 11)/** to hex */

//#define PLL_CTRL_RECONFIG_RST_OFST      (0)                                         //parameter reset
//#define PLL_CTRL_RECONFIG_RST_MSK       (0x00000001 << PLL_CTRL_RECONFIG_RST_OFST)  //parameter reset
//#define PLL_CTRL_WR_PARAMETER_OFST      (2)
//#define PLL_CTRL_WR_PARAMETER_MSK       (0x00000001 << PLL_CTRL_WR_PARAMETER_OFST)
#define PLL_CTRL_RST_OFST               (3)
#define PLL_CTRL_RST_MSK                (0x00000001 << PLL_CTRL_RST_OFST)
//#define PLL_CTRL_ADDR_OFST              (16)
//#define PLL_CTRL_ADDR_MSK               (0x0000003F << PLL_CTRL_ADDR_OFST)

/* Samples Register */
#define NSAMPLES_REG                    (0x5D << 11) //93<<11

/* Power On Register */
#define POWER_ON_REG                    (0x5e<<11)

#define POWER_ENABLE_OFST               (16)

/* Dac Registers */
#define DAC_VAL_REG                     (0x79 << 11) //121<<11
#define DAC_NUM_REG                     (0x80 << 11) //122<<11
#define DAC_VAL_OUT_REG                 (0x2A << 11) //42<<11

#endif

