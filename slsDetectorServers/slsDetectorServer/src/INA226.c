// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "INA226.h"
#include "I2C.h"
#include "clogger.h"
#include "common.h"

#include "math.h"

/**
 * To be defined in
 *
 * (slsDetectorServer_defs.h)
 * I2C_SHUNT_RESISTER_OHMS
 * device ids that are passed as arguments
 */

/** INA226 defines */

/** Register set */
#define INA226_CONFIGURATION_REG   (0x00) // R/W
#define INA226_SHUNT_VOLTAGE_REG   (0x01) // R
#define INA226_BUS_VOLTAGE_REG     (0x02) // R
#define INA226_POWER_REG           (0x03) // R
#define INA226_CURRENT_REG         (0x04) // R
#define INA226_CALIBRATION_REG     (0x05) // R/W
#define INA226_MASK_ENABLE_REG     (0x06) // R/W
#define INA226_ALERT_LIMIT_REG     (0x07) // R/W
#define INA226_MANUFACTURER_ID_REG (0xFE) // R
#define INA226_DIE_ID_REG          (0xFF) // R

/** bus voltage register */
#define INA226_BUS_VOLTAGE_VMIN_UV (1250) // 1.25mV
#define INA226_BUS_VOLTAGE_MX_STPS (0x7FFF + 1)
#define INA226_BUS_VOLTAGE_VMAX_UV                                             \
    (INA226_BUS_VOLTAGE_VMIN_UV *                                              \
     INA226_BUS_VOLTAGE_MX_STPS) // 40960000uV, 40.96V

/** shunt voltage register */
#define INA226_SHUNT_VOLTAGE_VMIN_NV (2500) // 2.5uV
#define INA226_SHUNT_VOLTAGE_MX_STPS (0x7FFF + 1)
#define INA226_SHUNT_VOLTAGE_VMAX_NV                                           \
    (INA226_SHUNT_VOLTAGE_VMIN_NV *                                            \
     INA226_SHUNT_VOLTAGE_MX_STPS) // 81920000nV, 81.92mV
#define INA226_SHUNT_NEGATIVE_MSK  (1 << 15)
#define INA226_SHUNT_ABS_VALUE_MSK (0x7FFF)

/** current precision for calibration register */
#define INA226_CURRENT_IMIN_UA (100) // 100uA can be changed

/** calibration register */
#define INA226_CALIBRATION_MSK (0x7FFF)

/** get calibration register value to be set */
#define INA226_getCalibrationValue(rOhm)                                       \
    (0.00512 / (INA226_CURRENT_IMIN_UA * 1e-6 * rOhm))

/** get current unit */
#define INA226_getConvertedCurrentUnits(shuntV, calibReg)                      \
    ((double)shuntV * (double)calibReg / (double)2048)

// defines from the fpga
double INA226_Shunt_Resistor_Ohm = 0.0;
int INA226_Calibration_Register_Value = 0;

#define INA226_CALIBRATION_CURRENT_TOLERANCE (1.2268)

void INA226_ConfigureI2CCore(double rOhm, uint32_t creg, uint32_t sreg,
                             uint32_t rreg, uint32_t rlvlreg, uint32_t slreg,
                             uint32_t shreg, uint32_t sdreg, uint32_t treg) {
    LOG(logINFOBLUE, ("Configuring INA226\n"));
    LOG(logDEBUG1, ("Shunt ohm resistor: %f\n", rOhm));
    INA226_Shunt_Resistor_Ohm = rOhm;

    I2C_ConfigureI2CCore(creg, sreg, rreg, rlvlreg, slreg, shreg, sdreg, treg);
}

void INA226_CalibrateCurrentRegister(uint32_t deviceId) {
    LOG(logINFO,
        ("Calibrating Current Register for Device ID: 0x%x\n", deviceId));
    // get calibration value based on shunt resistor
    uint16_t calVal =
        ((uint16_t)INA226_getCalibrationValue(INA226_Shunt_Resistor_Ohm)) &
        INA226_CALIBRATION_MSK;
    LOG(logINFO,
        ("\tCalculated calibration reg value: 0x%0x (%d)\n", calVal, calVal));

    calVal = ((double)calVal / INA226_CALIBRATION_CURRENT_TOLERANCE) + 0.5;
    LOG(logINFO,
        ("\tRealculated (for tolerance) calibration reg value: 0x%0x (%d)\n",
         calVal, calVal));
    INA226_Calibration_Register_Value = calVal;

    // calibrate current register
    I2C_Write(deviceId, INA226_CALIBRATION_REG, calVal);

    // read back calibration register
    int retval = I2C_Read(deviceId, INA226_CALIBRATION_REG);
    if (retval != calVal) {
        LOG(logERROR,
            ("Cannot set calibration register for I2C. Set 0x%x, read 0x%x\n",
             calVal, retval));
    }
}

int INA226_ReadVoltage(uint32_t deviceId) {
    LOG(logDEBUG1, (" Reading voltage\n"));
    uint32_t regval = I2C_Read(deviceId, INA226_BUS_VOLTAGE_REG);
    LOG(logDEBUG1, (" bus voltage reg: 0x%08x\n", regval));

    // value in uV
    int voltageuV = 0;
    ConvertToDifferentRange(0, INA226_BUS_VOLTAGE_MX_STPS,
                            INA226_BUS_VOLTAGE_VMIN_UV,
                            INA226_BUS_VOLTAGE_VMAX_UV, regval, &voltageuV);
    LOG(logDEBUG1, (" voltage: 0x%d uV\n", voltageuV));

    // value in mV
    int voltagemV = voltageuV / 1000;
    LOG(logDEBUG1, (" voltage: %d mV\n", voltagemV));
    LOG(logINFO,
        ("Voltage via I2C (Device: 0x%x): %d mV\n", deviceId, voltagemV));

    return voltagemV;
}

int INA226_ReadCurrent(uint32_t deviceId) {
    LOG(logDEBUG1, (" Reading current\n"));

    // read shunt voltage register
    LOG(logDEBUG1, (" Reading shunt voltage reg\n"));
    uint32_t shuntVoltageRegVal = I2C_Read(deviceId, INA226_SHUNT_VOLTAGE_REG);
    LOG(logDEBUG1, (" shunt voltage reg: %d\n", shuntVoltageRegVal));

    // read it once more as this error has occured once
    if (shuntVoltageRegVal == 0xFFFF) {
        LOG(logDEBUG1, (" Reading shunt voltage reg again\n"));
        shuntVoltageRegVal = I2C_Read(deviceId, INA226_SHUNT_VOLTAGE_REG);
        LOG(logDEBUG1, (" shunt voltage reg: %d\n", shuntVoltageRegVal));
    }
    // value for current
    int retval = INA226_getConvertedCurrentUnits(
        shuntVoltageRegVal, INA226_Calibration_Register_Value);
    LOG(logDEBUG1, (" current unit value: %d\n", retval));

    // reading directly the current reg
    LOG(logDEBUG1, (" Reading current reg\n"));
    int cuurentRegVal = I2C_Read(deviceId, INA226_CURRENT_REG);
    LOG(logDEBUG1, (" current reg: %d\n", cuurentRegVal));
    // read it once more as this error has occured once
    if (cuurentRegVal >= 0xFFF0) {
        LOG(logDEBUG1, (" Reading current reg again\n"));
        cuurentRegVal = I2C_Read(deviceId, INA226_CURRENT_REG);
        LOG(logDEBUG1, (" current reg: %d\n", cuurentRegVal));
    }

    // should be the same
    LOG(logDEBUG1, (" ===============current reg: %d, current unit "
                    "cal:%d=================================\n",
                    cuurentRegVal, retval));
    // current in uA
    int currentuA = cuurentRegVal * INA226_CURRENT_IMIN_UA;
    LOG(logDEBUG1, (" current: %d uA\n", currentuA));

    // current in mA
    int currentmA = (currentuA / 1000.00) + 0.5;
    LOG(logDEBUG1, (" current: %d mA\n", currentmA));

    LOG(logINFO,
        ("Current via I2C (Device: 0x%x): %d mA\n", deviceId, currentmA));

    return currentmA;
}
