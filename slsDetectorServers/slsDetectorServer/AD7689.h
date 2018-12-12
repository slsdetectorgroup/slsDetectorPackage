#pragma once

#include "commonServerFunctions.h" // blackfin.h, ansi.h

/* AD7689 ADC DEFINES */

/** Read back CFG Register */
#define AD7689_CFG_RB_OFST                  (0)
#define AD7689_CFG_RB_MSK                   (0x00000001 << AD7689_CFG_RB_OFST)

/** Channel sequencer */
#define AD7689_CFG_SEQ_OFST                 (1)
#define AD7689_CFG_SEQ_MSK                  (0x00000003 << AD7689_CFG_SEQ_OFST)
#define AD7689_CFG_SEQ_DSBLE_VAL            ((0x0 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)
#define AD7689_CFG_SEQ_UPDTE_DRNG_SQNCE_VAL ((0x1 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)
#define AD7689_CFG_SEQ_SCN_WTH_TMP_VAL      ((0x2 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)
#define AD7689_CFG_SEQ_SCN_WTHT_TMP_VAL     ((0x3 << AD7689_CFG_SEQ_OFST) & AD7689_CFG_SEQ_MSK)

/** Reference/ buffer selection */
#define AD7689_CFG_REF_OFST                 (3)
#define AD7689_CFG_REF_MSK                  (0x00000007 << AD7689_CFG_REF_OFST)
/** Internal reference. REF = 2.5V buffered output. Temperature sensor enabled. */
#define AD7689_CFG_REF_INT_2500MV_VAL       ((0x0 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_OFST)
/** Internal reference. REF = 4.096V buffered output. Temperature sensor enabled. */
#define AD7689_CFG_REF_INT_4096MV_VAL       ((0x1 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor enabled. Internal buffer disabled. */
#define AD7689_CFG_REF_EXT_TMP_VAL          ((0x2 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor enabled. Internal buffer enabled. */
#define AD7689_CFG_REF_EXT_TMP_INTBUF_VAL   ((0x3 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor disabled. Internal buffer disabled. */
#define AD7689_CFG_REF_EXT_VAL              ((0x6 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)
/** External reference. Temperature sensor disabled. Internal buffer enabled. */
#define AD7689_CFG_REF_EXT_INTBUF_VAL       ((0x7 << AD7689_CFG_REF_OFST) & AD7689_CFG_REF_MSK)

/** bandwidth of low pass filter */
#define AD7689_CFG_BW_OFST                  (6)
#define AD7689_CFG_BW_MSK                   (0x00000001 << AD7689_CFG_REF_OFST)
#define AD7689_CFG_BW_ONE_FOURTH_VAL        ((0x0 << AD7689_CFG_BW_OFST) & AD7689_CFG_BW_MSK)
#define AD7689_CFG_BW_FULL_VAL              ((0x1 << AD7689_CFG_BW_OFST) & AD7689_CFG_BW_MSK)

/** input channel selection IN0 - IN7 */
#define AD7689_CFG_IN_OFST                  (7)
#define AD7689_CFG_IN_MSK                   (0x00000007 << AD7689_CFG_IN_OFST)

/** input channel configuration */
#define AD7689_CFG_INCC_OFST                (10)
#define AD7689_CFG_INCC_MSK                 (0x00000007 << AD7689_CFG_INCC_OFST)
#define AD7689_CFG_INCC_BPLR_DFFRNTL_PRS_VAL ((0x0 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_BPLR_IN_COM_VAL     ((0x2 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_TMP_VAL             ((0x3 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_UNPLR_DFFRNTL_PRS_VAL ((0x4 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_UNPLR_IN_COM_VAL    ((0x6 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)
#define AD7689_CFG_INCC_UNPLR_IN_GND_VAL    ((0x7 << AD7689_CFG_INCC_OFST) & AD7689_CFG_INCC_MSK)

/** configuration update */
#define AD7689_CFG_CFG_OFST                 (13)
#define AD7689_CFG_CFG_MSK                  (0x00000001 << AD7689_CFG_CFG_OFST)
#define AD7689_CFG_CFG_NO_UPDATE_VAL        ((0x0 << AD7689_CFG_CFG_OFST) & AD7689_CFG_CFG_MSK)
#define AD7689_CFG_CFG_OVRWRTE_VAL          ((0x1 << AD7689_CFG_CFG_OFST) & AD7689_CFG_CFG_MSK)

#define AD7689_ADC_CFG_NUMBITS              (14)
#define AD7689_ADC_DATA_NUMBITS             (16)
#define AD7689_NUM_CHANNELS                 (7)
#define AD7689_NUM_INVALID_CONVERSIONS      (3)
#define AD7689_INT_REF_MAX_MV               (2500) // chosen using reference buffer selection in config reg
#define AD7689_INT_REF_MIN_MV               (0)
#define AD7689_TMP_C_FOR_1_MV               (25.00 / 283)


uint32_t AD7689_Reg = 0x0;
uint32_t AD7689_ROReg = 0x0;
uint32_t AD7689_CnvMask = 0x0;
uint32_t AD7689_ClkMask = 0x0;
uint32_t AD7689_DigMask = 0x0;
int AD7689_DigOffset = 0x0;

/**
 * Set Defines
 * @param reg spi register
 * @param roreg spi readout register
 * @param cmsk conversion mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 */
void AD7689_SetDefines(uint32_t reg, uint32_t roreg, uint32_t cmsk, uint32_t clkmsk, uint32_t dmsk, int dofst) {
    AD7689_Reg = reg;
    AD7689_ROReg = roreg;
    AD7689_CnvMask = cmsk;
    AD7689_ClkMask = clkmsk;
    AD7689_DigMask = dmsk;
    AD7689_DigOffset = dofst;
}

/**
 * Disable SPI
 */
void AD7689_Disable() {
    bus_w(AD7689_Reg, bus_r(AD7689_Reg)
            | AD7689_CnvMask
            | AD7689_ClkMask
            &~(AD7689_DigMask));
}

/**
 * Set SPI reg value
 * @param codata value to be set
 */
void AD7689_Set(u_int32_t codata) {
    FILE_LOG(logINFO, ("\tSetting ADC SPI Register. Writing 0x%08x to Config Reg\n", codata));
    serializeToSPI(AD7689_Reg, codata, AD7689_CnvMask, AD7689_ADC_CFG_NUMBITS,
            AD7689_ClkMask, AD7689_DigMask, AD7689_DigOffset);
}

/**
 * Get SPI reg value
 * @returns SPI reg value
 */
uint16_t AD7689_Get() {
    FILE_LOG(logINFO, ("\tGetting ADC SPI Register.\n"));
    return (uint16_t)serializeFromSPI(AD7689_ROReg, AD7689_Reg, AD7689_CnvMask, AD7689_ADC_DATA_NUMBITS,
            AD7689_ClkMask, AD7689_DigMask);
}

/**
 * Get temperature
 * @returns temperature in °C
 */
int AD7689_GetTemperature() {
    AD7689_Set(
            // read back
            AD7689_CFG_RB_MSK |
            // disable sequencer (different from config)
            AD7689_CFG_SEQ_DSBLE_VAL |
            // Internal reference. REF = 2.5V buffered output. Temperature sensor enabled.
            AD7689_CFG_REF_INT_2500MV_VAL |
            // full bandwidth of low pass filter
            AD7689_CFG_BW_FULL_VAL |
            // all channel (different from config)
            AD7689_CFG_IN_MSK |
            // temperature sensor (different from config)
            AD7689_CFG_INCC_TMP_VAL |
            // overwrite configuration
            AD7689_CFG_CFG_OVRWRTE_VAL);

    // FIXME: do we have to read it 8 times?? (sequencer is disabled anyway) or are we sequencing, then we read only last channel
    uint16_t regval = AD7689_Get();

    // value in mV FIXME: page 17? reference voltage temperature coefficient or t do with -40 to 85 °C
    uint16_t vmin = AD7689_INT_REF_MIN_MV;
    uint16_t vmax = AD7689_INT_REF_MAX_MV;
    uint16_t nsteps = (2 ^ AD7689_ADC_DATA_NUMBITS);
    uint16_t retval = (vmin + (vmax - vmin) * regval / (nsteps - 1));
    FILE_LOG(logDEBUG1, ("\tvoltage read for temp: 0x%d mV\n", retval));

    // value in °C
    uint16_t temp = AD7689_TMP_C_FOR_1_MV * retval;
    FILE_LOG(logDEBUG1, ("\ttemp read: 0x%d °C\n", temp));

    return temp;

}

/**
 * Reads channels voltage
 * @param ichan channel number from 0 to 7
 * @returns channel voltage in mV
 */
int AD7689_GetChannel(int ichan) {
   // filter channels val
   if (ichan < 0 || ichan >= AD7689_NUM_CHANNELS) {
       FILE_LOG(logERROR, ("Cannot get slow adc channel. "
               "%d out of bounds (0 to %d)\n", ichan, AD7689_NUM_CHANNELS - 1));
       return -1;
   }

   AD7689_Set(
           // read back
           AD7689_CFG_RB_MSK |
           // disable sequencer (different from config)
           AD7689_CFG_SEQ_DSBLE_VAL |
           // Internal reference. REF = 2.5V buffered output. Temperature sensor enabled.
           AD7689_CFG_REF_INT_2500MV_VAL |
           // full bandwidth of low pass filter
           AD7689_CFG_BW_FULL_VAL |
           // specific channel (different from config)
           ((ichan << AD7689_CFG_IN_OFST) & AD7689_CFG_IN_MSK) |
           // input channel configuration (unipolar. inx to gnd)
           AD7689_CFG_INCC_UNPLR_IN_GND_VAL |
           // overwrite configuration
           AD7689_CFG_CFG_OVRWRTE_VAL);

   // FIXME: do we have to read it 8 times?? (sequencer is disabled anyway) or are we sequencing, then we read only last channel
   uint16_t regval = AD7689_Get();

   // value in mV
   uint16_t vmin = AD7689_INT_REF_MIN_MV;
   uint16_t vmax = AD7689_INT_REF_MAX_MV;
   uint16_t nsteps = (2 ^ AD7689_ADC_DATA_NUMBITS);
   uint16_t retval = (vmin + (vmax - vmin) * regval / (nsteps - 1));
   FILE_LOG(logINFO, ("\tvoltage read for chan %d: 0x%d mV\n", retval));

   return retval;
}

/**
 * Configure
 */
void AD7689_Configure(){
    FILE_LOG(logINFOBLUE, ("Configuring AD7689 (Slow ADCs):\n"));

    // from power up, 3 invalid conversions
    int i = 0;
    for (i = 0; i < AD7689_NUM_INVALID_CONVERSIONS; ++i) {
        AD7689_Set(
            // read back
            AD7689_CFG_RB_MSK |
            // scan sequence IN0-IN7 then temperature sensor
            AD7689_CFG_SEQ_SCN_WTH_TMP_VAL |
            // Internal reference. REF = 2.5V buffered output. Temperature sensor enabled.
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
