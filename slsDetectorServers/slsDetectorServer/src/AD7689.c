// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "AD7689.h"
#include "blackfin.h"
#include "clogger.h"
#include "common.h"
#include "commonServerFunctions.h" // blackfin.h, ansi.h

/* AD7689 ADC DEFINES */

/** Read back CFG Register */
#define AD7689_CFG_RB_OFST (0)
#define AD7689_CFG_RB_MSK  (0x00000001 << AD7689_CFG_RB_OFST)

/** Channel sequencer */
#define AD7689_CFG_SEQ_OFST (1)
#define AD7689_CFG_SEQ_MSK  (0x00000003 << AD7689_CFG_SEQ_OFST)
#define AD7689_CFG_SEQ_DSBLE_VAL                                               \
    ((0x0 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)
#define AD7689_CFG_SEQ_UPDTE_DRNG_SQNCE_VAL                                    \
    ((0x1 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)
#define AD7689_CFG_SEQ_SCN_WTH_TMP_VAL                                         \
    ((0x2 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)
#define AD7689_CFG_SEQ_SCN_WTHT_TMP_VAL                                        \
    ((0x3 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)

/** Reference/ buffer selection */
#define AD7689_CFG_REF_OFST (3)
#define AD7689_CFG_REF_MSK  (0x00000007 << AD7689_CFG_REF_OFST)
/** Internal reference. REF = 2.5V buffered output. Temperature sensor enabled.
 */
#define AD7689_CFG_REF_INT_2500MV_VAL                                          \
    ((0x0 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_OFST)
/** Internal reference. REF = 4.096V buffered output. Temperature sensor
 * enabled. */
#define AD7689_CFG_REF_INT_4096MV_VAL                                          \
    ((0x1 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor enabled. Internal buffer disabled. */
#define AD7689_CFG_REF_EXT_TMP_VAL                                             \
    ((0x2 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor enabled. Internal buffer enabled. */
#define AD7689_CFG_REF_EXT_TMP_INTBUF_VAL                                      \
    ((0x3 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor disabled. Internal buffer disabled.
 */
#define AD7689_CFG_REF_EXT_VAL                                                 \
    ((0x6 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor disabled. Internal buffer enabled. */
#define AD7689_CFG_REF_EXT_INTBUF_VAL                                          \
    ((0x7 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)

/** bandwidth of low pass filter */
#define AD7689_CFG_BW_OFST (6)
#define AD7689_CFG_BW_MSK  (0x00000001 << AD7689_CFG_REF_OFST)
#define AD7689_CFG_BW_ONE_FOURTH_VAL                                           \
    ((0x0 << AD7689_CFG_BW_OFST) & AD7689_CFG_BW_MSK)
#define AD7689_CFG_BW_FULL_VAL ((0x1 << AD7689_CFG_BW_OFST) & AD7689_CFG_BW_MSK)

/** input channel selection IN0 - IN7 */
#define AD7689_CFG_IN_OFST (7)
#define AD7689_CFG_IN_MSK  (0x00000007 << AD7689_CFG_IN_OFST)

/** input channel configuration */
#define AD7689_CFG_INCC_OFST (10)
#define AD7689_CFG_INCC_MSK  (0x00000007 << AD7689_CFG_INCC_OFST)
#define AD7689_CFG_INCC_BPLR_DFFRNTL_PRS_VAL                                   \
    ((0x0 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_BPLR_IN_COM_VAL                                        \
    ((0x2 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_TMP_VAL                                                \
    ((0x3 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_UNPLR_DFFRNTL_PRS_VAL                                  \
    ((0x4 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_UNPLR_IN_COM_VAL                                       \
    ((0x6 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_UNPLR_IN_GND_VAL                                       \
    ((0x7 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)

/** configuration update */
#define AD7689_CFG_CFG_OFST (13)
#define AD7689_CFG_CFG_MSK  (0x00000001 << AD7689_CFG_CFG_OFST)
#define AD7689_CFG_CFG_NO_UPDATE_VAL                                           \
    ((0x0 << AD7689_CFG_CFG_OFST) & AD7689_CFG_CFG_MSK)
#define AD7689_CFG_CFG_OVRWRTE_VAL                                             \
    ((0x1 << AD7689_CFG_CFG_OFST) & AD7689_CFG_CFG_MSK)

#define AD7689_ADC_CFG_NUMBITS         (14)
#define AD7689_ADC_DATA_NUMBITS        (16)
#define AD7689_NUM_CHANNELS            (8)
#define AD7689_NUM_INVALID_CONVERSIONS (3)

#define AD7689_INT_REF_MAX_MV                                                  \
    (2500) // chosen using reference buffer selection in config reg
#define AD7689_INT_REF_MIN_MV (0)
#define AD7689_INT_REF_MAX_UV (2500 * 1000)
#define AD7689_INT_REF_MIN_UV (0)
#define AD7689_INT_MAX_STEPS  (0xFFFF + 1)
#define AD7689_TMP_C_FOR_1_MV (25.00 / 283.00)


#define AD7689_SLEEP(x)  for (int i = 0; i!= x; ++i) ;
#define AD7689_40NS_SLEEP   (1)
#define AD7689_2US_SLEEP    (50)


// Definitions from the fpga
uint32_t AD7689_Reg = 0x0;
uint32_t AD7689_ROReg = 0x0;
uint32_t AD7689_CnvMask = 0x0;
uint32_t AD7689_ClkMask = 0x0;
uint32_t AD7689_DigMask = 0x0;
int AD7689_DigOffset = 0x0;

void AD7689_SetDefines(uint32_t reg, uint32_t roreg, uint32_t cmsk,
                       uint32_t clkmsk, uint32_t dmsk, int dofst) {
    LOG(logDEBUG, ("AD7689: reg:0x%x roreg:0x%x cmsk:0x%x  clkmsk:0x%x  "
                   "dmsk:0x%x  dofst:%d\n",
                   reg, roreg, cmsk, clkmsk, dmsk, dofst));
    AD7689_Reg = reg;
    AD7689_ROReg = roreg;
    AD7689_CnvMask = cmsk;
    AD7689_ClkMask = clkmsk;
    AD7689_DigMask = dmsk;
    AD7689_DigOffset = dofst;
}

int AD7689_GetTemperature() {
    AD7689_Set(
        // don't read back config reg
        AD7689_CFG_RB_MSK |
        // disable sequencer (different from config)
        AD7689_CFG_SEQ_DSBLE_VAL |
        // Internal reference. REF = 2.5V buffered output. Temperature sensor
        // enabled.
        AD7689_CFG_REF_INT_2500MV_VAL |
        // full bandwidth of low pass filter
        AD7689_CFG_BW_FULL_VAL |
        // all channel (different from config)
        AD7689_CFG_IN_MSK |
        // temperature sensor (different from config)
        AD7689_CFG_INCC_TMP_VAL |
        // overwrite configuration
        AD7689_CFG_CFG_OVRWRTE_VAL);

    int regval = AD7689_Get();

    // value in mV FIXME: page 17? reference voltage temperature coefficient or
    // t do with -40 to 85 °C
    int retval = 0;
    ConvertToDifferentRange(0, AD7689_INT_MAX_STEPS, AD7689_INT_REF_MIN_MV,
                            AD7689_INT_REF_MAX_MV, regval, &retval);
    LOG(logDEBUG1, ("voltage read for temp: %d mV\n", retval));

    // value in °C
    double tempValue = AD7689_TMP_C_FOR_1_MV * (double)retval;
    LOG(logINFO, ("\tTemp slow adc : %f °C (reg: %d)\n", tempValue, regval));

    return tempValue;
}

int AD7689_GetChannel(int ichan) {
    // filter channels val
    if (ichan < 0 || ichan >= AD7689_NUM_CHANNELS) {
        LOG(logERROR, ("Cannot get slow adc channel. "
                       "%d out of bounds (0 to %d)\n",
                       ichan, AD7689_NUM_CHANNELS - 1));
        return -1;
    }

    AD7689_Set(
        // don't read back config reg
        AD7689_CFG_RB_MSK |
        // disable sequencer (different from config)
        AD7689_CFG_SEQ_DSBLE_VAL |
        // Internal reference. REF = 2.5V buffered output. Temperature sensor
        // enabled.
        AD7689_CFG_REF_INT_2500MV_VAL |
        // full bandwidth of low pass filter
        AD7689_CFG_BW_FULL_VAL |
        // specific channel (different from config)
        ((ichan << AD7689_CFG_IN_OFST) & AD7689_CFG_IN_MSK) |
        // input channel configuration (unipolar. inx to gnd)
        AD7689_CFG_INCC_UNPLR_IN_GND_VAL |
        // overwrite configuration
        AD7689_CFG_CFG_OVRWRTE_VAL);

    int regval = AD7689_Get();

    // value in uV
    int retval = ((double)(regval - 0) *
                  (double)(AD7689_INT_REF_MAX_UV - AD7689_INT_REF_MIN_UV)) /
                     (double)(AD7689_INT_MAX_STEPS - 0) +
                 AD7689_INT_REF_MIN_UV;

    /*ConvertToDifferentRange(0, AD7689_INT_MAX_STEPS,
           AD7689_INT_REF_MIN_MV, AD7689_INT_REF_MAX_MV,
           regval, &retval);*/

    LOG(logINFO, ("\tRead slow adc [%d]: %d uV (reg: %d)\n", ichan,
                  retval, regval));
    return retval;
}

void AD7689_Configure() {
    LOG(logINFOBLUE, ("Configuring AD7689 (Slow ADCs): \n"));

    bus_w(AD7689_Reg, (bus_r(AD7689_Reg) & ~(AD7689_CnvMask) & ~AD7689_ClkMask &
                       ~(AD7689_DigMask)));

    // from power up, 3 invalid conversions
    LOG(logINFO,
        ("\tConfiguring %d x due to invalid conversions from power up\n",
         AD7689_NUM_INVALID_CONVERSIONS));
    for (int i = 0; i < AD7689_NUM_INVALID_CONVERSIONS; ++i) {
        AD7689_Set(
            // don't read back config reg
            AD7689_CFG_RB_MSK |
            // disable sequencer (different from config)
            AD7689_CFG_SEQ_DSBLE_VAL |
            // Internal reference. REF = 2.5V buffered output. Temperature
            // sensor enabled.
            AD7689_CFG_REF_INT_2500MV_VAL |
            // full bandwidth of low pass filter
            AD7689_CFG_BW_FULL_VAL |
            // scan upto channel 7
            AD7689_CFG_IN_MSK |
            // input channel configuration (unipolar. inx to gnd)
            AD7689_CFG_INCC_UNPLR_IN_GND_VAL |
            // overwrite configuration
            AD7689_CFG_CFG_OVRWRTE_VAL);
    }
}

void AD7689_ConvPulse(int x) {
    
    // conv bit high
    AD7689_SLEEP(AD7689_40NS_SLEEP);
    bus_w(AD7689_Reg, (bus_r(AD7689_Reg) | AD7689_CnvMask));
    AD7689_SLEEP(x);

    // conv bit low
    bus_w(AD7689_Reg, (bus_r(AD7689_Reg) &~AD7689_CnvMask));
}

void AD7689_StartConvPulse() {
    AD7689_ConvPulse(AD7689_40NS_SLEEP);
}

void AD7689_EndConvPulse() {
    AD7689_ConvPulse(AD7689_2US_SLEEP);
}

void AD7689_Set(uint32_t codata) {
    
    LOG(logDEBUG2,
        ("\tSetting Slow ADC SPI Register. Writing 0x%04x to Config Reg\n", codata));
    
    AD7689_StartConvPulse();

    uint32_t addr = AD7689_Reg;
    uint32_t value = bus_r(addr);
    int nBits = AD7689_ADC_CFG_NUMBITS;

    // trailing edge
    for (int i = 0; i != nBits; ++i) {

        // clk down
        value &= ~AD7689_ClkMask;
        bus_w(addr, value);

        // write data (push out each bit from msb)
        value = ((value & ~AD7689_DigMask) + // unset bit
                   (((codata >> (nBits - 1 - i)) & 0x1) // set bit if needed
                    << AD7689_DigOffset)); 
        bus_w(addr, value);

        // clk up
        value |= AD7689_ClkMask;
        bus_w(addr, value);
    }

    // clk down
    value &= ~AD7689_ClkMask;
    bus_w(addr, value);

    AD7689_EndConvPulse();
}

uint32_t AD7689_Get() {
    LOG(logDEBUG2, ("\tGetting Slow ADC SPI Register.\n"));
                                
    uint32_t addr = AD7689_Reg;
    uint32_t value = bus_r(addr);
    int nBits = AD7689_ADC_DATA_NUMBITS;

    uint32_t retval = 0;
    // rising edge
    for (int i = 0; i != nBits; ++i) {

        // clk up
        value |= AD7689_ClkMask;
        bus_w(addr, value);

        // read data (i)
        retval |= ((bus_r(AD7689_ROReg) & 0x1) << (nBits - 1 - i));

        // clk down
        value &= ~AD7689_ClkMask;
        bus_w(addr, value);
    }

    LOG(logDEBUG2, ("Read From SPI Register: 0x%08x\n", retval));
    return retval;
}
