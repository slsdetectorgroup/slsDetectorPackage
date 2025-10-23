// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "I2C.h"
#include "blackfin.h"
#include "clogger.h"

#include <unistd.h> // usleep

/**
 * Intel: Embedded Peripherals IP User Guide
 * https://www.intel.com/content/dam/www/programmable/us/en/pdfs/literature/ug/ug_embedded_ip.pdf
 * To be defined
 *
 * (in blackfin.h)
 * I2C_CLOCK_MHZ
 *
 * (RegisterDefs.h)
 * I2C_SCL_LOW_COUNT_REG
 * I2C_SCL_HIGH_COUNT_REG
 * I2C_SDA_HOLD_REG
 * I2C_CONTROL_REG
 * I2C_STATUS_REG
 * I2C_TRANSFER_COMMAND_FIFO_REG
 * I2C_RX_DATA_FIFO_LEVEL_REG
 * I2C_RX_DATA_FIFO_REG
 */

#define I2C_DATA_RATE_KBPS (200)

/** Control Register */
#define I2C_CTRL_ENBLE_CORE_OFST (0)
#define I2C_CTRL_ENBLE_CORE_MSK  (0x00000001 << I2C_CTRL_ENBLE_CORE_OFST)
#define I2C_CTRL_BUS_SPEED_OFST  (1)
#define I2C_CTRL_BUS_SPEED_MSK   (0x00000001 << I2C_CTRL_BUS_SPEED_OFST)
#define I2C_CTRL_BUS_SPEED_STNDRD_100_VAL                                      \
    ((0x0 << I2C_CTRL_BUS_SPEED_OFST) &                                        \
     I2C_CTRL_BUS_SPEED_MSK) // standard mode (up to 100 kbps)
#define I2C_CTRL_BUS_SPEED_FAST_400_VAL                                        \
    ((0x1 << I2C_CTRL_BUS_SPEED_OFST) &                                        \
     I2C_CTRL_BUS_SPEED_MSK) // fast mode (up to 400 kbps)
/** if actual level of transfer command fifo <= thd level, TX_READY interrupt
 * asserted */
#define I2C_CTRL_TFR_CMD_FIFO_THD_OFST (2)
#define I2C_CTRL_TFR_CMD_FIFO_THD_MSK                                          \
    (0x00000003 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST)
#define I2C_CTRL_TFR_CMD_EMPTY_VAL                                             \
    ((0x0 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
#define I2C_CTRL_TFR_CMD_ONE_FOURTH_VAL                                        \
    ((0x1 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
#define I2C_CTRL_TFR_CMD_ONE_HALF_VAL                                          \
    ((0x2 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
#define I2C_CTRL_TFR_CMD_NOT_FULL_VAL                                          \
    ((0x3 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
/** if actual level of receive data fifo <= thd level, RX_READY interrupt
 * asserted */
#define I2C_CTRL_RX_DATA_FIFO_THD_OFST (4)
#define I2C_CTRL_RX_DATA_FIFO_THD_MSK                                          \
    (0x00000003 << I2C_CTRL_RX_DATA_FIFO_THD_OFST)
#define I2C_CTRL_RX_DATA_1_VALID_ENTRY_VAL                                     \
    ((0x0 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)
#define I2C_CTRL_RX_DATA_ONE_FOURTH_VAL                                        \
    ((0x1 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)
#define I2C_CTRL_RX_DATA_ONE_HALF_VAL                                          \
    ((0x2 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)
#define I2C_CTRL_RX_DATA_FULL_VAL                                              \
    ((0x3 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)

/** Transfer Command Fifo register */
#define I2C_TFR_CMD_RW_OFST (0)
#define I2C_TFR_CMD_RW_MSK  (0x00000001 << I2C_TFR_CMD_RW_OFST)
#define I2C_TFR_CMD_RW_WRITE_VAL                                               \
    ((0x0 << I2C_TFR_CMD_RW_OFST) & I2C_TFR_CMD_RW_MSK)
#define I2C_TFR_CMD_RW_READ_VAL                                                \
    ((0x1 << I2C_TFR_CMD_RW_OFST) & I2C_TFR_CMD_RW_MSK)
#define I2C_TFR_CMD_ADDR_OFST (1)
#define I2C_TFR_CMD_ADDR_MSK  (0x0000007F << I2C_TFR_CMD_ADDR_OFST)
/** when writing, rw and addr converts to data to be written mask */
#define I2C_TFR_CMD_DATA_FR_WR_OFST (0)
#define I2C_TFR_CMD_DATA_FR_WR_MSK  (0x000000FF << I2C_TFR_CMD_DATA_FR_WR_OFST)
#define I2C_TFR_CMD_STOP_OFST       (8)
#define I2C_TFR_CMD_STOP_MSK        (0x00000001 << I2C_TFR_CMD_STOP_OFST)
#define I2C_TFR_CMD_RPTD_STRT_OFST  (9)
#define I2C_TFR_CMD_RPTD_STRT_MSK   (0x00000001 << I2C_TFR_CMD_RPTD_STRT_OFST)

/** Receive DataFifo register */
#define I2C_RX_DATA_FIFO_RXDATA_OFST (0)
#define I2C_RX_DATA_FIFO_RXDATA_MSK  (0x000000FF << I2C_RX_DATA_FIFO_RXDATA_OFST)

/** Status register */
#define I2C_STATUS_BUSY_OFST (0)
#define I2C_STATUS_BUSY_MSK  (0x00000001 << I2C_STATUS_BUSY_OFST)

/** SCL Low Count register */
#define I2C_SCL_LOW_COUNT_PERIOD_OFST (0)
#define I2C_SCL_LOW_COUNT_PERIOD_MSK                                           \
    (0x0000FFFF << I2C_SCL_LOW_COUNT_PERIOD_OFST)

/** SCL High Count register */
#define I2C_SCL_HIGH_COUNT_PERIOD_OFST (0)
#define I2C_SCL_HIGH_COUNT_PERIOD_MSK                                          \
    (0x0000FFFF << I2C_SCL_HIGH_COUNT_PERIOD_OFST)

/** SDA Hold Count register */
#define I2C_SDA_HOLD_COUNT_PERIOD_OFST (0)
#define I2C_SDA_HOLD_COUNT_PERIOD_MSK                                          \
    (0x0000FFFF << I2C_SDA_HOLD_COUNT_PERIOD_OFST)

/** Receive Data Fifo Level register */
// #define I2C_RX_DATA_FIFO_LVL_OFST           (0)
// #define I2C_RX_DATA_FIFO_LVL_MSK            (0x000000FF <<
//  I2C_RX_DATA_FIFO_LVL_OFST)

// defines in the fpga
uint32_t I2C_Control_Reg = 0x0;
uint32_t I2C_Status_Reg = 0x0;
uint32_t I2C_Rx_Data_Fifo_Reg = 0x0;
uint32_t I2C_Rx_Data_Fifo_Level_Reg = 0x0;
uint32_t I2C_Scl_Low_Count_Reg = 0x0;
uint32_t I2C_Scl_High_Count_Reg = 0x0;
uint32_t I2C_Sda_Hold_Reg = 0x0;
uint32_t I2C_Transfer_Command_Fifo_Reg = 0x0;

void I2C_ConfigureI2CCore(uint32_t creg, uint32_t sreg, uint32_t rreg,
                          uint32_t rlvlreg, uint32_t slreg, uint32_t shreg,
                          uint32_t sdreg, uint32_t treg) {
    LOG(logINFO, ("\tConfiguring I2C Core for %d kbps:\n", I2C_DATA_RATE_KBPS));
    LOG(logDEBUG1, ("controlreg,:0x%x, statusreg,:0x%x, "
                    "rxrdatafiforeg: 0x%x, rxdatafifocountreg,:0x%x, "
                    "scllow,:0x%x, sclhighreg,:0x%x, sdaholdreg,:0x%x, "
                    "transfercmdreg,:0x%x\n",
                    creg, sreg, rreg, rlvlreg, slreg, shreg, sdreg, treg));

    I2C_Control_Reg = creg;
    I2C_Status_Reg = sreg;
    I2C_Rx_Data_Fifo_Reg = rreg;
    I2C_Rx_Data_Fifo_Level_Reg = rlvlreg;
    I2C_Scl_Low_Count_Reg = slreg;
    I2C_Scl_High_Count_Reg = shreg;
    I2C_Sda_Hold_Reg = sdreg;
    I2C_Transfer_Command_Fifo_Reg = treg;

    // calculate scl low and high period count
    uint32_t sclPeriodNs = ((1000.00 * 1000.00 * 1000.00) /
                            ((double)I2C_DATA_RATE_KBPS * 1000.00));
    // scl low period same as high period
    uint32_t sclLowPeriodNs = sclPeriodNs / 2;
    // convert to us, then to clock (defined in blackfin.h)
    uint32_t sclLowPeriodCount = (sclLowPeriodNs / 1000.00) * I2C_CLOCK_MHZ;

    // calculate sda hold data count
    uint32_t sdaDataHoldTimeNs =
        (sclLowPeriodNs / 2); //  scl low period same as high period
    // convert to us, then to clock (defined in blackfin.h)
    uint32_t sdaDataHoldCount = ((sdaDataHoldTimeNs / 1000.00) * I2C_CLOCK_MHZ);

    LOG(logINFO, ("\tSetting SCL Low Period: %d ns (%d clocks)\n",
                  sclLowPeriodNs, sclLowPeriodCount));
    bus_w(I2C_Scl_Low_Count_Reg,
          bus_r(I2C_Scl_Low_Count_Reg) |
              ((sclLowPeriodCount << I2C_SCL_LOW_COUNT_PERIOD_OFST) &
               I2C_SCL_LOW_COUNT_PERIOD_MSK));
    LOG(logDEBUG1, ("SCL Low reg:0x%x\n", bus_r(I2C_Scl_Low_Count_Reg)));

    LOG(logINFO, ("\tSetting SCL High Period: %d ns (%d clocks)\n",
                  sclLowPeriodNs, sclLowPeriodCount));
    bus_w(I2C_Scl_High_Count_Reg,
          bus_r(I2C_Scl_High_Count_Reg) |
              ((sclLowPeriodCount << I2C_SCL_HIGH_COUNT_PERIOD_OFST) &
               I2C_SCL_HIGH_COUNT_PERIOD_MSK));
    LOG(logDEBUG1, ("SCL High reg:0x%x\n", bus_r(I2C_Scl_High_Count_Reg)));

    LOG(logINFO, ("\tSetting SDA Hold Time: %d ns (%d clocks)\n",
                  sdaDataHoldTimeNs, sdaDataHoldCount));
    bus_w(I2C_Sda_Hold_Reg,
          bus_r(I2C_Sda_Hold_Reg) |
              ((sdaDataHoldCount << I2C_SDA_HOLD_COUNT_PERIOD_OFST) &
               I2C_SDA_HOLD_COUNT_PERIOD_MSK));
    LOG(logDEBUG1, ("SDA Hold reg:0x%x\n", bus_r(I2C_Sda_Hold_Reg)));

    LOG(logINFO, ("\tEnabling core and bus speed to fast (up to 400 kbps)\n"));
    bus_w(I2C_Control_Reg,
          bus_r(I2C_Control_Reg) | I2C_CTRL_ENBLE_CORE_MSK |
              I2C_CTRL_BUS_SPEED_FAST_400_VAL); // fixme: (works?)
    LOG(logDEBUG1, ("Control reg:0x%x\n", bus_r(I2C_Control_Reg)));
    // The INA226 supports the transmission protocol for fast mode (1 kHz to 400
    // kHz) and high-speed mode (1 kHz to 2.94 MHz).
}

uint32_t I2C_Read(uint32_t devId, uint32_t addr) {
    LOG(logDEBUG2, (" ================================================\n"));
    LOG(logDEBUG2,
        (" Reading from I2C device 0x%x and reg 0x%x\n", devId, addr));
    // device Id mask
    uint32_t devIdMask =
        ((devId << I2C_TFR_CMD_ADDR_OFST) & I2C_TFR_CMD_ADDR_MSK);
    LOG(logDEBUG2, (" devId:0x%x\n", devIdMask));

    // write I2C ID
    bus_w(I2C_Transfer_Command_Fifo_Reg, (devIdMask & ~(I2C_TFR_CMD_RW_MSK)));
    LOG(logDEBUG2,
        (" write devID and R/-W:0x%x\n", (devIdMask & ~(I2C_TFR_CMD_RW_MSK))));

    // write register addr
    bus_w(I2C_Transfer_Command_Fifo_Reg, addr);
    LOG(logDEBUG2, (" write addr:0x%x\n", addr));

    // repeated start with read (repeated start needed here because it was in
    // write operation mode earlier, for the device ID)
    bus_w(I2C_Transfer_Command_Fifo_Reg,
          (devIdMask | I2C_TFR_CMD_RPTD_STRT_MSK | I2C_TFR_CMD_RW_READ_VAL));
    LOG(logDEBUG2,
        (" repeated start:0x%x\n",
         (devIdMask | I2C_TFR_CMD_RPTD_STRT_MSK | I2C_TFR_CMD_RW_READ_VAL)));

    // continue reading
    bus_w(I2C_Transfer_Command_Fifo_Reg, 0x0);
    LOG(logDEBUG2, (" continue reading:0x%x\n", 0x0));

    // stop reading
    bus_w(I2C_Transfer_Command_Fifo_Reg, I2C_TFR_CMD_STOP_MSK);
    LOG(logDEBUG2, (" stop reading:0x%x\n", I2C_TFR_CMD_STOP_MSK));

    // read value
    uint32_t retval = 0;

    // In case one wants to do something more general (INA226 receives only 2
    // bytes)
    // wait till status is idle
    int status = 1;
    while (status) {
        status = bus_r(I2C_Status_Reg) & I2C_STATUS_BUSY_MSK;
        LOG(logDEBUG2, (" status:%d\n", status));
        usleep(0);
    }
    // get rx fifo level (get number of bytes to be received)
    int level = bus_r(I2C_Rx_Data_Fifo_Level_Reg);
    LOG(logDEBUG2, (" level:%d\n", level));

    // level bytes to read, read 1 byte at a time
    for (int iloop = level - 1; iloop >= 0; --iloop) {
        u_int16_t byte =
            bus_r(I2C_Rx_Data_Fifo_Reg) & I2C_RX_DATA_FIFO_RXDATA_MSK;
        LOG(logDEBUG2, (" byte nr %d:0x%x\n", iloop, byte));
        // push by 1 byte at a time
        retval |= (byte << (8 * iloop));
    }
    LOG(logDEBUG2, (" retval:0x%x\n", retval));
    LOG(logDEBUG2, (" ================================================\n"));
    return retval;
}

void I2C_Write(uint32_t devId, uint32_t addr, uint16_t data) {
    LOG(logDEBUG2, (" ================================================\n"));
    LOG(logDEBUG2, (" Writing to I2C (Device:0x%x, reg:0x%x, data:%d)\n", devId,
                    addr, data));
    // device Id mask
    uint32_t devIdMask =
        ((devId << I2C_TFR_CMD_ADDR_OFST) & I2C_TFR_CMD_ADDR_MSK);
    LOG(logDEBUG2, (" devId:0x%x\n", devId));

    // write I2C ID
    bus_w(I2C_Transfer_Command_Fifo_Reg, (devIdMask & ~(I2C_TFR_CMD_RW_MSK)));
    LOG(logDEBUG2,
        (" write devID and R/-W:0x%x\n", (devIdMask & ~(I2C_TFR_CMD_RW_MSK))));

    // write register addr
    bus_w(I2C_Transfer_Command_Fifo_Reg, addr);
    LOG(logDEBUG2, (" write addr:0x%x\n", addr));

    // do not do the repeated start as it is already in write operation mode
    // (else it wont work)

    uint8_t msb = (uint8_t)((data & 0xFF00) >> 8);
    uint8_t lsb = (uint8_t)(data & 0x00FF);
    LOG(logDEBUG2, (" msb:0x%02x, lsb:0x%02x\n", msb, lsb));

    // writing data MSB
    bus_w(I2C_Transfer_Command_Fifo_Reg,
          ((msb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK));
    LOG(logDEBUG2,
        (" write msb:0x%02x\n",
         ((msb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK)));

    // writing data LSB and stop writing bit
    bus_w(I2C_Transfer_Command_Fifo_Reg,
          ((lsb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK) |
              I2C_TFR_CMD_STOP_MSK);
    LOG(logDEBUG2,
        (" write lsb and stop writing:0x%x\n",
         ((lsb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK) |
             I2C_TFR_CMD_STOP_MSK));
    LOG(logDEBUG2, (" ================================================\n"));
}
