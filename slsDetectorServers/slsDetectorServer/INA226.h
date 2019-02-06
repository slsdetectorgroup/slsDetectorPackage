#pragma once

#include "I2C.h"

/**
 * To be defined in
 *
 * (slsDetectorServer_defs.h)
 * I2C_SHUNT_RESISTER_OHMS
 * device ids that are passed as arguments
 */

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
#define INA226_getCalibrationValue(rOhm) (0.00512 /(INA226_CURRENT_IMIN_UA * 1e-6 * rOhm))

/** get current unit */
#define INA226_getConvertedCurrentUnits(shuntVReg, calibReg) (shuntVReg * calibReg / 2048)

double INA226_Shunt_Resistor_Ohm = 0.0;


/**
 * Configure the I2C core and Enable core
 * @param rOhm shunt resister value in Ohms (defined in slsDetectorServer_defs.h)
 * @param creg control register (defined in RegisterDefs.h)
 * @param rreg rx data fifo level register (defined in RegisterDefs.h)
 * @param slreg scl low count register (defined in RegisterDefs.h)
 * @param shreg scl high count register (defined in RegisterDefs.h)
 * @param sdreg sda hold register (defined in RegisterDefs.h)
 * @param treg transfer command fifo register (defined in RegisterDefs.h)
 */
void INA226_ConfigureI2CCore(double rOhm, uint32_t creg, uint32_t rreg, uint32_t slreg, uint32_t shreg, uint32_t sdreg, uint32_t treg) {
    FILE_LOG(logINFOBLUE, ("Configuring INA226\n"));

    INA226_Shunt_Resistor_Ohm = rOhm;

    I2C_ConfigureI2CCore(creg, rreg, slreg, shreg, sdreg, treg);
}

/**
 * Calibrate resolution of current register
 * @param deviceId device Id (defined in slsDetectorServer_defs.h)
 */
void INA226_CalibrateCurrentRegister(uint32_t deviceId) {
    FILE_LOG(logINFO, ("Calibrating Current Register for Device ID: 0x%x\n", deviceId));
    // get calibration value based on shunt resistor
    uint16_t calVal = ((uint16_t)INA226_getCalibrationValue(INA226_Shunt_Resistor_Ohm)) & INA226_CALIBRATION_MSK;
    FILE_LOG(logINFO, ("\tWriting to Calibration reg: 0x%0x\n", calVal));

    // calibrate current register
    I2C_Write(deviceId, INA226_CALIBRATION_REG, calVal);
}

/**
 * Read voltage of device
 * @param deviceId device Id
 * @returns voltage in mV
 */
int INA226_ReadVoltage(uint32_t deviceId) {
    FILE_LOG(logDEBUG1, ("\tReading voltage\n"));
    uint32_t regval = I2C_Read(deviceId, INA226_BUS_VOLTAGE_REG);
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
 * @param deviceId device Id
 * @returns current in mA
 */
int INA226_ReadCurrent(uint32_t deviceId) {
    FILE_LOG(logDEBUG1, ("\tReading current\n"));

    // read shunt voltage register
    FILE_LOG(logDEBUG1, ("\tReading shunt voltage reg\n"));
    uint32_t shuntVoltageRegVal = I2C_Read(deviceId, INA226_SHUNT_VOLTAGE_REG);
    FILE_LOG(logDEBUG1, ("\tshunt voltage reg: 0x%08x\n", shuntVoltageRegVal));

    // read calibration register
    FILE_LOG(logDEBUG1, ("\tReading calibration reg\n"));
    uint32_t calibrationRegVal = I2C_Read(deviceId, INA226_CALIBRATION_REG);
    FILE_LOG(logDEBUG1, ("\tcalibration reg: 0x%08x\n", calibrationRegVal));

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
