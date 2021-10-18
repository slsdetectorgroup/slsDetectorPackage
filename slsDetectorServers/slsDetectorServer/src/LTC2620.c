// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "LTC2620.h"
#include "blackfin.h"
#include "clogger.h"
#include "common.h"
#include "commonServerFunctions.h" // blackfin.h, ansi.h
#include "sls/sls_detector_defs.h"

#include <string.h>

/* LTC2620 DAC DEFINES */
// first 4 bits are 0 as this is a 12 bit dac
#define LTC2620_DAC_DATA_OFST (4)
#define LTC2620_DAC_DATA_MSK  (0x00000FFF << LTC2620_DAC_DATA_OFST)
#define LTC2620_DAC_ADDR_OFST (16)
#define LTC2620_DAC_ADDR_MSK  (0x0000000F << LTC2620_DAC_ADDR_OFST)
#define LTC2620_DAC_CMD_OFST  (20)
#define LTC2620_DAC_CMD_MSK   (0x0000000F << LTC2620_DAC_CMD_OFST)

#define LTC2620_DAC_CMD_WR_IN_VAL                                              \
    ((0x0 << LTC2620_DAC_CMD_OFST) &                                           \
     LTC2620_DAC_CMD_MSK) // write to input register
#define LTC2620_DAC_CMD_UPDTE_DAC_VAL                                          \
    ((0x1 << LTC2620_DAC_CMD_OFST) &                                           \
     LTC2620_DAC_CMD_MSK) // update dac (power up)
#define LTC2620_DAC_CMD_WR_IN_UPDTE_DAC_VAL                                    \
    ((0x2 << LTC2620_DAC_CMD_OFST) &                                           \
     LTC2620_DAC_CMD_MSK) // write to input register and update dac (power up)
#define LTC2620_DAC_CMD_WR_UPDTE_DAC_VAL                                       \
    ((0x3 << LTC2620_DAC_CMD_OFST) &                                           \
     LTC2620_DAC_CMD_MSK) // write to and update dac (power up)
#define LTC2620_DAC_CMD_PWR_DWN_VAL                                            \
    ((0x4 << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK)
#define LTC2620_DAC_CMD_NO_OPRTN_VAL                                           \
    ((0xF << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK)

#define LTC2620_NUMBITS (24)
#define LTC2620_DAISY_CHAIN_NUMBITS                                            \
    (32) // due to shift register FIXME: was 33 earlier
#define LTC2620_NUMCHANNELS  (8)
#define LTC2620_PWR_DOWN_VAL (-100)
#define LTC2620_MIN_VAL      (0)
#define LTC2620_MAX_VAL      (4095) // 12 bits
#define LTC2620_MAX_STEPS    (LTC2620_MAX_VAL + 1)

#ifdef CHIPTESTBOARDD
#include "slsDetectorServer_defs.h"
#endif

// defines from the fpga
uint32_t LTC2620_Reg = 0x0;
uint32_t LTC2620_CsMask = 0x0;
uint32_t LTC2620_ClkMask = 0x0;
uint32_t LTC2620_DigMask = 0x0;
int LTC2620_DigOffset = 0x0;
int LTC2620_Ndac = 0;
int LTC2620_MinVoltage = 0;
int LTC2620_MaxVoltage = 0;

void LTC2620_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk,
                        uint32_t dmsk, int dofst, int nd, int minMV,
                        int maxMV) {
    LTC2620_Reg = reg;
    LTC2620_CsMask = cmsk;
    LTC2620_ClkMask = clkmsk;
    LTC2620_DigMask = dmsk;
    LTC2620_DigOffset = dofst;
    LTC2620_Ndac = nd;
    LTC2620_MinVoltage = minMV;
    LTC2620_MaxVoltage = maxMV;
}

void LTC2620_Disable() {
    bus_w(LTC2620_Reg, (bus_r(LTC2620_Reg) | LTC2620_CsMask | LTC2620_ClkMask) &
                           ~(LTC2620_DigMask));
}

int LTC2620_GetPowerDownValue() { return LTC2620_PWR_DOWN_VAL; }

int LTC2620_GetMinInput() { return LTC2620_MIN_VAL; }

int LTC2620_GetMaxInput() { return LTC2620_MAX_VAL; }

int LTC2620_GetMaxNumSteps() { return LTC2620_MAX_STEPS; }

int LTC2620_VoltageToDac(int voltage, int *dacval) {
    return ConvertToDifferentRange(LTC2620_MinVoltage, LTC2620_MaxVoltage,
                                   LTC2620_MIN_VAL, LTC2620_MAX_VAL, voltage,
                                   dacval);
}

int LTC2620_DacToVoltage(int dacval, int *voltage) {
    return ConvertToDifferentRange(LTC2620_MIN_VAL, LTC2620_MAX_VAL,
                                   LTC2620_MinVoltage, LTC2620_MaxVoltage,
                                   dacval, voltage);
}

void LTC2620_SetSingle(int cmd, int data, int dacaddr) {
    LOG(logDEBUG2,
        ("(Single) dac addr:%d, dac value:%d, cmd:%d\n", dacaddr, data, cmd));

    uint32_t codata =
        (((data << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK) |
         ((dacaddr << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) | cmd);
    LOG(logDEBUG2, ("codata: 0x%x\n", codata));

    serializeToSPI(LTC2620_Reg, codata, LTC2620_CsMask, LTC2620_NUMBITS,
                   LTC2620_ClkMask, LTC2620_DigMask, LTC2620_DigOffset, 0);
}

void LTC2620_SendDaisyData(uint32_t *valw, uint32_t val) {
    sendDataToSPI(valw, LTC2620_Reg, val, LTC2620_DAISY_CHAIN_NUMBITS,
                  LTC2620_ClkMask, LTC2620_DigMask, LTC2620_DigOffset);
}

void LTC2620_SetDaisy(int cmd, int data, int dacaddr, int chipIndex) {

    int nchip = LTC2620_Ndac / LTC2620_NUMCHANNELS;
    uint32_t valw = 0;

    LOG(logDEBUG2, ("(Daisy) desired chip index:%d, nchip:%d, dac ch:%d, "
                    "val:%d, cmd:0x%x \n",
                    chipIndex, nchip, dacaddr, data, cmd));

    // data to be bit banged
    uint32_t codata =
        (((data << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK) |
         ((dacaddr << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) | cmd);
    LOG(logDEBUG2, ("codata: 0x%x\n", codata));

    // select all chips (ctb daisy chain; others 1 chip)
    LOG(logDEBUG2, ("Selecting LTC2620\n"));
    SPIChipSelect(&valw, LTC2620_Reg, LTC2620_CsMask, LTC2620_ClkMask,
                  LTC2620_DigMask, 0);

    // send same data to all
    if (chipIndex < 0) {
        LOG(logDEBUG2, ("Send same data to all\n"));
        for (int ichip = 0; ichip < nchip; ++ichip) {
            LOG(logDEBUG2, ("Send data  (0x%x) to ichip %d\n", codata, ichip));
            LTC2620_SendDaisyData(&valw, codata);
        }
    }

    // send to one chip, nothing to others
    else {
        // send nothing to subsequent ichips (daisy chain) (if any chips after
        // desired chip)
        for (int ichip = chipIndex + 1; ichip < nchip; ++ichip) {
            LOG(logDEBUG2, ("Send nothing to ichip %d\n", ichip));
            LTC2620_SendDaisyData(&valw, LTC2620_DAC_CMD_NO_OPRTN_VAL);
        }

        // send data to desired chip
        LOG(logDEBUG2, ("Send data  (0x%x) to ichip %d\n", codata, chipIndex));
        LTC2620_SendDaisyData(&valw, codata);

        // send nothing to preceding ichips (daisy chain) (if any chips in front
        // of desired chip)
        for (int ichip = 0; ichip < chipIndex; ++ichip) {
            LOG(logDEBUG2, ("Send nothing to ichip %d\n", ichip));
            LTC2620_SendDaisyData(&valw, LTC2620_DAC_CMD_NO_OPRTN_VAL);
        }
    }

    // deselect all chips (ctb daisy chain; others 1 chip)
    LOG(logDEBUG2, ("Deselecting LTC2620\n"));
    SPIChipDeselect(&valw, LTC2620_Reg, LTC2620_CsMask, LTC2620_ClkMask,
                    LTC2620_DigMask, 0);
}

void LTC2620_Set(int cmd, int data, int dacaddr, int chipIndex) {
    LOG(logDEBUG1, ("cmd:0x%x, data:%d, dacaddr:%d, chipIndex:%d\n", cmd, data,
                    dacaddr, chipIndex));
    LOG(logDEBUG2, (" ================================================\n"));
    // ctb
    if (LTC2620_Ndac > LTC2620_NUMCHANNELS)
        LTC2620_SetDaisy(cmd, data, dacaddr, chipIndex);
    // others
    else
        LTC2620_SetSingle(cmd, data, dacaddr);
    LOG(logDEBUG2, (" ================================================\n"));
}

void LTC2620_Configure() {
    LOG(logINFOBLUE, ("Configuring LTC2620\n"));

    // dac channel - all channels
    int addr = (LTC2620_DAC_ADDR_MSK >> LTC2620_DAC_ADDR_OFST);

    // data (any random low value, just writing to power up)
    int data = 0x6;

    // command
    int cmd = LTC2620_DAC_CMD_WR_IN_VAL; // FIXME: should be command update and
                                         // not write(does not power up)
    // also why do we need to power up (for jctb, we power down next)

    LTC2620_Set(cmd, data, addr, -1);
}

void LTC2620_SetDAC(int dacnum, int data) {
    LOG(logDEBUG1, ("Setting dac %d to %d\n", dacnum, data));
    // LTC2620 index
    int ichip = dacnum / LTC2620_NUMCHANNELS;

    // dac channel
    int addr = dacnum % LTC2620_NUMCHANNELS;

    // command
    int cmd = LTC2620_DAC_CMD_WR_UPDTE_DAC_VAL;

    // power down mode, value is ignored
    if (data == LTC2620_PWR_DOWN_VAL) {
        cmd = LTC2620_DAC_CMD_PWR_DWN_VAL;
        LOG(logDEBUG1, ("POWER DOWN\n"));
    } else {
        LOG(logDEBUG1, ("Write to Input Register and Update\n"));
    }

    LTC2620_Set(cmd, data, addr, ichip);
}

int LTC2620_SetDACValue(int dacnum, int val, int mV, int *dacval) {
    LOG(logDEBUG1, ("dacnum:%d, val:%d, ismV:%d\n", dacnum, val, mV));
    // validate index
    if (dacnum < 0 || dacnum >= LTC2620_Ndac) {
        LOG(logERROR, ("Dac index %d is out of bounds (0 to %d)\n", dacnum,
                       LTC2620_Ndac - 1));
        return FAIL;
    }

    // get
    if (val < 0 && val != LTC2620_PWR_DOWN_VAL)
        return FAIL;

    // convert to dac or get mV value
    *dacval = val;
    int dacmV = val;
    int ret = OK;
    int ndacsonly = LTC2620_Ndac;
#ifdef CHIPTESTBOARDD
    ndacsonly = NDAC_ONLY;
#endif
    if (mV) {
        ret = LTC2620_VoltageToDac(val, dacval);
    } else if (val >= 0 && dacnum <= ndacsonly) {
        // do not convert power down dac val
        //(if not ndacsonly (pwr/vchip): dont need to print mV value as it will
        // be wrong (wrong limits))
        ret = LTC2620_DacToVoltage(val, &dacmV);
    }

    // conversion out of bounds
    if (ret == FAIL) {
        LOG(logERROR, ("Setting Dac %d %s is out of bounds\n", dacnum,
                       (mV ? "mV" : "dac units")));
        return FAIL;
    }

    // set
    if ((*dacval >= 0) || (*dacval == LTC2620_PWR_DOWN_VAL)) {
        LOG(logINFO,
            ("Setting DAC %d: %d dac (%d mV)\n", dacnum, *dacval, dacmV));
        LTC2620_SetDAC(dacnum, *dacval);
    }
    return OK;
}
