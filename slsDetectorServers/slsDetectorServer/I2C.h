#pragma once

#include "blackfin.h"

/**
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
 * I2C_TRANSFER_COMMAND_FIFO_REG
 * I2C_RX_DATA_FIFO_LEVEL_REG
 */


#define I2C_DATA_RATE_KBPS                  (200)

/** Control Register */
#define I2C_CTRL_ENBLE_CORE_OFST            (0)
#define I2C_CTRL_ENBLE_CORE_MSK             (0x00000001 << I2C_CTRL_ENBLE_CORE_OFST)
#define I2C_CTRL_BUS_SPEED_OFST             (1)
#define I2C_CTRL_BUS_SPEED_MSK              (0x00000001 << I2C_CTRL_BUS_SPEED_OFST)
#define I2C_CTRL_BUS_SPEED_STNDRD_100_VAL   ((0x0 << I2C_CTRL_BUS_SPEED_OFST) & I2C_CTRL_BUS_SPEED_MSK) // standard mode (up to 100 kbps)
#define I2C_CTRL_BUS_SPEED_FAST_400_VAL     ((0x1 << I2C_CTRL_BUS_SPEED_OFST) & I2C_CTRL_BUS_SPEED_MSK) // fast mode (up to 400 kbps)
/** if actual level of transfer command fifo <= thd level, TX_READY interrupt asserted */
#define I2C_CTRL_TFR_CMD_FIFO_THD_OFST      (2)
#define I2C_CTRL_TFR_CMD_FIFO_THD_MSK       (0x00000003 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST)
#define I2C_CTRL_TFR_CMD_EMPTY_VAL          ((0x0 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
#define I2C_CTRL_TFR_CMD_ONE_FOURTH_VAL     ((0x1 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
#define I2C_CTRL_TFR_CMD_ONE_HALF_VAL       ((0x2 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
#define I2C_CTRL_TFR_CMD_NOT_FULL_VAL       ((0x3 << I2C_CTRL_TFR_CMD_FIFO_THD_OFST) & I2C_CTRL_TFR_CMD_FIFO_THD_MSK)
/** if actual level of receive data fifo <= thd level, RX_READY interrupt asserted */
#define I2C_CTRL_RX_DATA_FIFO_THD_OFST      (4)
#define I2C_CTRL_RX_DATA_FIFO_THD_MSK       (0x00000003 << I2C_CTRL_RX_DATA_FIFO_THD_OFST)
#define I2C_CTRL_RX_DATA_1_VALID_ENTRY_VAL  ((0x0 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)
#define I2C_CTRL_RX_DATA_ONE_FOURTH_VAL     ((0x1 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)
#define I2C_CTRL_RX_DATA_ONE_HALF_VAL       ((0x2 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)
#define I2C_CTRL_RX_DATA_FULL_VAL           ((0x3 << I2C_CTRL_RX_DATA_FIFO_THD_OFST) & I2C_CTRL_RX_DATA_FIFO_THD_MSK)

/** Transfer Command Fifo register */
#define I2C_TFR_CMD_RW_OFST                 (0)
#define I2C_TFR_CMD_RW_MSK                  (0x00000001 << I2C_TFR_CMD_RW_OFST)
#define I2C_TFR_CMD_RW_WRITE_VAL            ((0x0 << I2C_TFR_CMD_RW_OFST) & I2C_TFR_CMD_RW_MSK)
#define I2C_TFR_CMD_RW_READ_VAL             ((0x1 << I2C_TFR_CMD_RW_OFST) & I2C_TFR_CMD_RW_MSK)
#define I2C_TFR_CMD_ADDR_OFST               (1)
#define I2C_TFR_CMD_ADDR_MSK                (0x0000007F << I2C_TFR_CMD_ADDR_OFST)
/** when writing, rw and addr converts to data to be written mask */
#define I2C_TFR_CMD_DATA_FR_WR_OFST         (0)
#define I2C_TFR_CMD_DATA_FR_WR_MSK          (0x000000FF << I2C_TFR_CMD_DATA_FR_WR_OFST)
#define I2C_TFR_CMD_STOP_OFST               (8)
#define I2C_TFR_CMD_STOP_MSK                (0x00000001 << I2C_TFR_CMD_ADDR_OFST)
#define I2C_TFR_CMD_RPTD_STRT_OFST          (9)
#define I2C_TFR_CMD_RPTD_STRT_MSK           (0x00000001 << I2C_TFR_CMD_RPTD_STRT_OFST)


uint32_t I2C_Control_Reg = 0x0;
uint32_t I2C_Rx_Data_Fifo_Level_Reg = 0x0;
uint32_t I2C_Scl_Low_Count_Reg = 0x0;
uint32_t I2C_Scl_High_Count_Reg = 0x0;
uint32_t I2C_Sda_Hold_Reg = 0x0;
uint32_t I2C_Transfer_Command_Fifo_Reg = 0x0;

/**
 * Configure the I2C core,
 * Enable core and
 * Calibrate the calibration register for current readout
 * @param creg control register (defined in RegisterDefs.h)
 * @param rreg rx data fifo level register (defined in RegisterDefs.h)
 * @param slreg scl low count register (defined in RegisterDefs.h)
 * @param shreg scl high count register (defined in RegisterDefs.h)
 * @param sdreg sda hold register (defined in RegisterDefs.h)
 * @param treg transfer command fifo register (defined in RegisterDefs.h)
 */
void I2C_ConfigureI2CCore(uint32_t creg, uint32_t rreg, uint32_t slreg, uint32_t shreg, uint32_t sdreg, uint32_t treg) {
    FILE_LOG(logINFO, ("\tConfiguring I2C Core for %d kbps:\n", I2C_DATA_RATE_KBPS));

    I2C_Control_Reg = creg;
    I2C_Rx_Data_Fifo_Level_Reg = rreg;
    I2C_Scl_Low_Count_Reg = slreg;
    I2C_Scl_High_Count_Reg = shreg;
    I2C_Sda_Hold_Reg = sdreg;
    I2C_Transfer_Command_Fifo_Reg = treg;

    // calculate scl low and high period count
    uint32_t sclPeriodNs = ((1000.00 * 1000.00) / (double)I2C_DATA_RATE_KBPS);
    // scl low period same as high period
    uint32_t sclLowPeriodNs = sclPeriodNs / 2;
    // convert to us, then to clock (defined in blackfin.h)
    uint32_t sclLowPeriodCount = (sclLowPeriodNs / 1000.00) * I2C_CLOCK_MHZ;

    // calculate sda hold data count
    uint32_t sdaDataHoldTimeNs =  (sclLowPeriodNs / 2); //  scl low period same as high period
    // convert to us, then to clock (defined in blackfin.h)
    uint32_t sdaDataHoldCount = ((sdaDataHoldTimeNs / 1000.00) * I2C_CLOCK_MHZ);

    FILE_LOG(logINFO, ("\tSetting SCL Low Period: %d ns (%d clocks)\n", sclLowPeriodNs, sclLowPeriodCount));
    bus_w(I2C_Scl_Low_Count_Reg, sclLowPeriodCount);

    FILE_LOG(logINFO, ("\tSetting SCL High Period: %d ns (%d clocks)\n", sclLowPeriodNs, sclLowPeriodCount));
    bus_w(I2C_Scl_High_Count_Reg, sclLowPeriodCount);

    FILE_LOG(logINFO, ("\tSetting SDA Hold Time: %d ns (%d clocks)\n", sdaDataHoldTimeNs, sdaDataHoldCount));
    bus_w(I2C_Sda_Hold_Reg, (uint32_t)sdaDataHoldCount);

    FILE_LOG(logINFO, ("\tEnabling core\n"));
    bus_w(I2C_Control_Reg, I2C_CTRL_ENBLE_CORE_MSK | I2C_CTRL_BUS_SPEED_FAST_400_VAL);// fixme: (works?)
}

/**
 * Read register
 * @param deviceId device Id
 * @param addr register address
 * @returns value read from register
 */
uint32_t I2C_Read(uint32_t devId, uint32_t addr) {
    FILE_LOG(logDEBUG1, ("\tReading from I2C device 0x%x and reg 0x%x\n", devId, addr));
    // device Id mask
    uint32_t devIdMask =  ((devId << I2C_TFR_CMD_ADDR_OFST) & I2C_TFR_CMD_ADDR_MSK);

    // write I2C ID
    bus_w(I2C_Transfer_Command_Fifo_Reg, (devIdMask & ~(I2C_TFR_CMD_RW_MSK)));

    // write register addr
    bus_w(I2C_Transfer_Command_Fifo_Reg, addr);

    // repeated start with read
    bus_w(I2C_Transfer_Command_Fifo_Reg, (devIdMask | I2C_TFR_CMD_RPTD_STRT_MSK | I2C_TFR_CMD_RW_READ_VAL));

    // continue reading
    bus_w(I2C_Transfer_Command_Fifo_Reg, 0x0);

    // stop reading
    bus_w(I2C_Transfer_Command_Fifo_Reg, I2C_TFR_CMD_STOP_MSK);

    // read value
    return bus_r(I2C_Rx_Data_Fifo_Level_Reg);
}

/**
 * Write register (16 bit value)
 * @param deviceId device Id
 * @param addr register address
 * @param data data to be written (16 bit)
 */
void I2C_Write(uint32_t devId, uint32_t addr, uint16_t data) {
    FILE_LOG(logDEBUG1, ("Writing to I2C (Device:0x%x, reg:0x%x, data:%d)\n", devId, addr, data));
    // device Id mask
    uint32_t devIdMask =  ((devId << I2C_TFR_CMD_ADDR_OFST) & I2C_TFR_CMD_ADDR_MSK);

    // write I2C ID
    bus_w(I2C_Transfer_Command_Fifo_Reg, (devIdMask & ~(I2C_TFR_CMD_RW_MSK)));

    // write register addr
    bus_w(I2C_Transfer_Command_Fifo_Reg, addr);

    // repeated start with write
    bus_w(I2C_Transfer_Command_Fifo_Reg, ((devIdMask | I2C_TFR_CMD_RPTD_STRT_MSK) & ~(I2C_TFR_CMD_RW_MSK)));

    uint8_t msb = (uint8_t)((data & 0xFF00) >> 8);
    uint8_t lsb = (uint8_t)(data & 0x00FF);

    // writing data MSB
    bus_w(I2C_Transfer_Command_Fifo_Reg, ((msb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK));

    // writing data LSB and stop writing bit
    bus_w(I2C_Transfer_Command_Fifo_Reg,  ((lsb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK) | I2C_TFR_CMD_STOP_MSK);
}


