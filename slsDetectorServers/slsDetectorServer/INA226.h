#pragma once

#include "I2C.h"

/** INA226 defines */

/** Register set */
#define INA226_CONFIGURATION_REG    (0x00)  //R/W
#define INA226_SHUNT_VOLTAGE_REG    (0x01)  //R
#define INA226_BUS_VOLTAGE_REG      (0x02)  //R
#define INA226_POWER_REG            (0x03)  //R
#define INA226_CURRENT_REG          (0x04)  //R
#define INA226_CALIBRATION_REG      (0x05)  //R/W
#define INA226_MASK_ENABLE_REG      (0x06)  //R/W
#define INA226_ALERT_LIMIT_REG      (0x07)  //R/W
#define INA226_MANUFACTURER_ID_REG  (0xFE)  //R
#define INA226_DIE_ID_REG           (0xFF)  //R

/** bus voltage register */
#define INA226_BUS_VOLTAGE_VMIN_UV  (1250)     // 1.25mV
#define INA226_BUS_VOLTAGE_MX_STPS  (0x7FFF + 1)
#define INA226_BUS_VOLTAGE_VMAX_UV  (INA226_BUS_VOLTAGE_VMIN_UV * INA226_BUS_VOLTAGE_MX_STPS) // 40960000uV, 40.96V

/** current register */
#define INA226_CURRENT_IMIN_UA      (100) //100uA can be changed
#define INA226_CURRENT_MX_STPS      (0x7FFF + 1)
#define INA226_CURRENT_IMAX_UA      (INA226_CURRENT_IMIN_UA * INA226_CURRENT_MX_STPS)

/** calibration register */
#define INA226_CALIBRATION_MSK      (0x7FFF)

/** get calibration register value to be set */
#define INA226_getCalibrationValue(rOhm) (0.00512 /(INA226_CURRENT_IMIN_UA * 1e-6 * rohm))

/** get current unit */
#define INA226_getConvertedCurrentUnits(shuntVReg, calibReg) (shuntVReg * calibReg / 2048)

/**
 * Configure the I2C core and Enable core
 * @param sclLowCountReg register to set low count of the serial clock (defined in Registerdefs.h)
 * @param sclHighCountReg register to set high count of the serial clock (defined in Registerdefs.h)
 * @param sdaHoldTimeReg register to set hold time of the serial data (defined in Registerdefs.h)
 * @param controlReg register to set control reg (bus speed and enabling core) (defined in Registerdefs.h)
 */
void INA226_ConfigureI2CCore(uint32_t sclLowCountReg, uint32_t sclHighCountReg, uint32_t sdaHoldTimeReg, uint32_t controlReg) {
    I2C_ConfigureI2CCore(sclLowCountReg, sclHighCountReg, sdaHoldTimeReg, controlReg);
}

/**
 * Calibrate resolution of current register
 * @param shuntResisterOhm shunt resister value in Ohms
 * @param transferCommandReg transfer command fifo register (defined in RegisterDefs.h)
 * @param deviceId device Id (defined in slsDetectorServer_defs.h)
 */
void INA226_CalibrateCurrentRegister(uint32_t shuntResisterOhm, uint32_t transferCommandReg, uint32_t deviceId) {

    // get calibration value based on shunt resistor
    uint16_t calVal = INA226_getCalibrationValue(shuntResisterOhm) & INA226_CALIBRATION_MSK;
    FILE_LOG(logINFO, ("\tWriting to Calibration reg: 0x%0x\n", calVal));

    // calibrate current register
    I2C_Write(transferCommandReg, deviceId, INA226_CALIBRATION_REG, calVal);
}

/**
 * Read voltage of device
 * @param transferCommandReg transfer command fifo register (defined in RegisterDefs.h)
 * @param rxDataFifoLevelReg receive data fifo level register (defined in RegisterDefs.h)
 * @param deviceId device Id (defined in slsDetectorServer_defs.h)
 * @returns voltage in mV
 */
int INA226_ReadVoltage(uint32_t transferCommandReg, uint32_t rxDataFifoLevelReg, uint32_t deviceId) {
    FILE_LOG(logDEBUG1, ("\tReading voltage\n"));
    uint32_t regval = I2C_Read(transferCommandReg, rxDataFifoLevelReg, deviceId, INA226_BUS_VOLTAGE_REG);
    FILE_LOG(logDEBUG1, ("\tvoltage read: 0x%08x\n", regval));

    // value converted in mv
    uint32_t vmin = INA226_BUS_VOLTAGE_VMIN_UV;
    uint32_t vmax = INA226_BUS_VOLTAGE_VMAX_UV;
    uint32_t nsteps = INA226_BUS_VOLTAGE_MX_STPS;

    // value in uV
    int retval = (vmin + (vmax - vmin) * regval / (nsteps - 1));
    FILE_LOG(logDEBUG1, ("\tvoltage read: 0x%d uV\n", retval));

    // value in mV
    retval /= 1000;
    FILE_LOG(logDEBUG1, ("\tvoltage read: %d mV\n", retval));

    return retval;
}

/**
 * Read current
 * @param transferCommandReg transfer command fifo register (defined in RegisterDefs.h)
 * @param rxDataFifoLevelReg receive data fifo level register (defined in RegisterDefs.h)
 * @param deviceId device Id (should be defined in slsDetectorServer_defs.h)
 * @returns current in mA
 */
int INA226_ReadCurrent(uint32_t transferCommandReg, uint32_t rxDataFifoLevelReg, uint32_t deviceId) {
    FILE_LOG(logDEBUG1, ("\tReading current\n"));

    // read shunt voltage register
    FILE_LOG(logDEBUG1, ("\tReading shunt voltage reg\n"));
    uint32_t shuntVoltageRegVal = I2C_Read(transferCommandReg, rxDataFifoLevelReg, deviceId, INA226_SHUNT_VOLTAGE_REG);
    FILE_LOG(logDEBUG1, ("\tshunt voltage reg: 0x%08x\n", regval));

    // read calibration register
    FILE_LOG(logDEBUG1, ("\tReading calibration reg\n"));
    uint32_t calibrationRegVal = I2C_Read(transferCommandReg, rxDataFifoLevelReg, deviceId, INA226_CALIBRATION_REG);
    FILE_LOG(logDEBUG1, ("\tcalibration reg: 0x%08x\n", regval));

    // value for current
    uint32_t retval = INA226_getConvertedCurrentUnits(shuntVoltageRegVal, calibrationRegVal);
    FILE_LOG(logDEBUG1, ("\tcurrent unit value: %d\n", retval));

    // current in uA
    retval *= INA226_CURRENT_IMIN_UA;
    FILE_LOG(logDEBUG1, ("\tcurrent: %d uA\n", retval));

    // current in mA
    retval /= 1000;
    FILE_LOG(logDEBUG1, ("\tcurrent: %d mA\n", retval));

    return retval;
}
