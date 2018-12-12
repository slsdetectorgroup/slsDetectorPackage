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


#define I2C_DATA_RATE_KBPS              (200)
#define I2C_SCL_PERIOD_NS               ((1000 * 1000) / I2C_DATA_RATE_KBPS)
#define I2C_SCL_LOW_PERIOD_NS           (I2C_SCL_PERIOD_NS / 2)
#define I2C_SDA_DATA_HOLD_TIME_NS       (I2C_SCL_HIGH_PERIOD_NS / 2)
#define I2C_SCL_LOW_COUNT               ((I2C_SCL_LOW_PERIOD_NS / 1000) * I2C_CLOCK_MHZ) // convert to us, then to clock (defined in blackfin.h)
#define I2C_SDA_DATA_HOLD_COUNT         ((I2C_SDA_DATA_HOLD_TIME_NS / 1000) * I2C_CLOCK_MHZ) // convert to us, then to clock (defined in blackfin.h)

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


/**
 * Configure the I2C core,
 * Enable core and
 * Calibrate the calibration register for current readout
 */
void I2C_ConfigureI2CCore() {
    FILE_LOG(logINFOBLUE, ("\tConfiguring I2C Core for %d kbps:\n", I2C_DATA_RATE_KBPS));

    FILE_LOG(logINFOBLUE, ("\tSetting SCL Low Period: %d ns (0x%x clocks)\n", I2C_SCL_LOW_PERIOD_NS, I2C_SCL_LOW_COUNT));
    bus_w(I2C_SCL_LOW_COUNT_REG, (uint32_t)I2C_SCL_LOW_COUNT);

    FILE_LOG(logINFOBLUE, ("\tSetting SCL High Period: %d ns (0x%x clocks)\n", I2C_SCL_HIGH_PERIOD_NS, I2C_SCL_LOW_COUNT));
    bus_w(I2C_SCL_HIGH_COUNT_REG, (uint32_t)I2C_SCL_LOW_COUNT);

    FILE_LOG(logINFOBLUE, ("\tSetting SDA Hold Time: %d ns (0x%x clocks)\n", I2C_SDA_DATA_HOLD_TIME_NS, I2C_SDA_DATA_HOLD_COUNT));
    bus_w(I2C_SDA_HOLD_REG, (uint32_t)I2C_SDA_DATA_HOLD_COUNT);

    FILE_LOG(logINFOBLUE, ("\tEnabling core\n"));
    bus_w(I2C_CONTROL_REG, I2C_CTRL_ENBLE_CORE_MSK | I2C_CTRL_BUS_SPEED_FAST_400_VAL);// fixme: (works?)
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
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, (devIdMask & ~(I2C_TFR_CMD_RW_MSK)));

    // write register addr
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, addr);

    // repeated start with read
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, (devIdMask | I2C_TFR_CMD_RPTD_STRT_MSK | I2C_TFR_CMD_RW_READ_VAL));

    // continue reading
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, 0x0);

    // stop reading
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, I2C_TFR_CMD_STOP_MSK);

    // read value
    return bus_r(I2C_RX_DATA_FIFO_LEVEL_REG);
}

/**
 * Write register (16 bit value)
 * @param deviceId device Id
 * @param addr register address
 * @param data data to be written (16 bit)
 */
void I2C_Write(uint32_t devId, uint32_t addr, uint16_t data) {
    FILE_LOG(logDEBUG1, ("\tWriting data %d to I2C device 0x%x and reg 0x%x\n", data, devId, addr));
    // device Id mask
    uint32_t devIdMask =  ((devId << I2C_TFR_CMD_ADDR_OFST) & I2C_TFR_CMD_ADDR_MSK);

    // write I2C ID
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, (devIdMask & ~(I2C_TFR_CMD_RW_MSK)));

    // write register addr
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, addr);

    // repeated start with write
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, (devIdMask | I2C_TFR_CMD_RPTD_STRT_MSK & ~(I2C_TFR_CMD_RW_MSK)));

    uint8_t msb = data & 0xFF00;
    uint8_t lsb = data & 0x00FF;

    // writing data MSB
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG, ((msb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK));

    // writing data LSB and stop writing bit
    bus_w(I2C_TRANSFER_COMMAND_FIFO_REG,  ((lsb << I2C_TFR_CMD_DATA_FR_WR_OFST) & I2C_TFR_CMD_DATA_FR_WR_MSK) | I2C_TFR_CMD_STOP_MSK);
}


