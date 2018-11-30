#pragma once

//#include "commonServerFunctions.h" // blackfin.h, ansi.h



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

int getAD7689(int ind) {

}

void setAD7689(int addr, int val) {
    u_int32_t codata;
    codata = val + (addr << 8);
    FILE_LOG(logINFO, ("\tSetting ADC SPI Register. Wrote 0x%04x at 0x%04x\n", val, addr));
    serializeToSPI(ADC_SPI_REG, codata, ADC_SERIAL_CS_OUT_MSK, AD9257_ADC_NUMBITS,
            ADC_SERIAL_CLK_OUT_MSK, ADC_SERIAL_DATA_OUT_MSK, ADC_SERIAL_DATA_OUT_OFST);
}

void prepareAD7689(){
    FILE_LOG(logINFOBLUE, ("Preparing AD7689 (Slow ADCs):\n"));

    uint16_t codata = (
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
