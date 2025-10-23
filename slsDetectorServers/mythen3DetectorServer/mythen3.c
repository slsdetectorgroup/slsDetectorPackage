// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "mythen3.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <string.h>

int chipStatusRegister = 0;

int setBit(int ibit, int patword) { return patword |= (1 << ibit); }

int clearBit(int ibit, int patword) { return patword &= ~(1 << ibit); }

int getChipStatusRegister() { return chipStatusRegister; }

patternParameters *setChipStatusRegisterPattern(int csr) {
    int iaddr = 0;
    int nbits = 18;
    int error = 0;
    // int start=0, stop=MAX_PATTERN_LENGTH, loop=0;
    int patword = 0;

    patternParameters *pat = malloc(sizeof(patternParameters));
    memset(pat, 0, sizeof(patternParameters));

    patword = setBit(SIGNAL_STATLOAD, patword);
    for (int i = 0; i < 2; i++)
        pat->word[iaddr++] = patword;
    patword = setBit(SIGNAL_resStorage, patword);
    patword = setBit(SIGNAL_resCounter, patword);
    for (int i = 0; i < 8; i++)
        pat->word[iaddr++] = patword;
    patword = clearBit(SIGNAL_resStorage, patword);
    patword = clearBit(SIGNAL_resCounter, patword);
    for (int i = 0; i < 8; i++)
        pat->word[iaddr++] = patword;
    // #This version of the serializer pushes in the MSB first (compatible with
    //  the CSR bit numbering)
    for (int ib = nbits - 1; ib >= 0; ib--) {
        if (csr & (1 << ib))
            patword = setBit(SIGNAL_serialIN, patword);
        else
            patword = clearBit(SIGNAL_serialIN, patword);
        for (int i = 0; i < 4; i++)
            pat->word[iaddr++] = patword;
        patword = setBit(SIGNAL_CHSclk, patword);
        pat->word[iaddr++] = patword;
        patword = clearBit(SIGNAL_CHSclk, patword);
        pat->word[iaddr++] = patword;
    }

    patword = clearBit(SIGNAL_serialIN, patword);
    for (int i = 0; i < 2; i++)
        pat->word[iaddr++] = patword;
    patword = setBit(SIGNAL_STO, patword);
    for (int i = 0; i < 5; i++)
        pat->word[iaddr++] = patword;
    patword = clearBit(SIGNAL_STO, patword);
    for (int i = 0; i < 5; i++)
        pat->word[iaddr++] = patword;
    patword = clearBit(SIGNAL_STATLOAD, patword);
    for (int i = 0; i < 5; i++)
        pat->word[iaddr++] = patword;

    if (iaddr >= MAX_PATTERN_LENGTH) {
        LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n", iaddr,
                       MAX_PATTERN_LENGTH));
        error = 1;
    }
    // set pattern wait address
    for (int i = 0; i < M3_MAX_PATTERN_LEVELS; i++)
        pat->wait[i] = MAX_PATTERN_LENGTH - 1;
    // pattern loop
    for (int i = 0; i < M3_MAX_PATTERN_LEVELS; i++) {
        // int stop = MAX_PATTERN_LENGTH - 1, nloop = 0;
        pat->startloop[i] = MAX_PATTERN_LENGTH - 1;
        pat->stoploop[i] = MAX_PATTERN_LENGTH - 1;
        pat->nloop[i] = 0;
    }

    // pattern limits
    {
        pat->limits[0] = 0;
        pat->limits[1] = iaddr;
    }

    if (error != 0) {
        free(pat);
        return NULL;
    }
    chipStatusRegister = csr;
    return pat;
}

void flipNegativePolarityBits(int *csr) {
    (*csr) ^= ((1 << _CSR_C10pre) | (1 << _CSR_C15pre));
}

int getGainCaps() {
    int csr = chipStatusRegister;
    // Translates bit representation
    int caps = 0;
    if (!(csr & (1 << _CSR_C10pre)))
        caps |= M3_C10pre;
    if (csr & (1 << CSR_C15sh))
        caps |= M3_C15sh;
    if (csr & (1 << CSR_C30sh))
        caps |= M3_C30sh;
    if (csr & (1 << CSR_C50sh))
        caps |= M3_C50sh;
    if (csr & (1 << CSR_C225ACsh))
        caps |= M3_C225ACsh;
    if (!(csr & (1 << _CSR_C15pre)))
        caps |= M3_C15pre;

    return caps;
}

int M3SetGainCaps(int caps) {
    int csr = chipStatusRegister & ~GAIN_MASK;

    // Translates bit representation
    if (!(caps & M3_C10pre))
        csr |= 1 << _CSR_C10pre;
    if (caps & M3_C15sh)
        csr |= 1 << CSR_C15sh;
    if (caps & M3_C30sh)
        csr |= 1 << CSR_C30sh;
    if (caps & M3_C50sh)
        csr |= 1 << CSR_C50sh;
    if (caps & M3_C225ACsh)
        csr |= 1 << CSR_C225ACsh;
    if (!(caps & M3_C15pre))
        csr |= 1 << _CSR_C15pre;

    return csr;
}

int getInterpolation() {
    return ((chipStatusRegister & CSR_interp_MSK) >> CSR_interp);
}

int M3SetInterpolation(int enable) {
    int csr = 0;
    if (enable)
        csr = chipStatusRegister | CSR_interp_MSK;
    else
        csr = chipStatusRegister & ~CSR_interp_MSK;
    return csr;
}

int getPumpProbe() {
    return ((chipStatusRegister & CSR_pumprobe_MSK) >> CSR_pumprobe);
}

int M3SetPumpProbe(int enable) {
    LOG(logINFO, ("%s Pump Probe\n", enable == 0 ? "Disabling" : "Enabling"));
    int csr = 0;
    if (enable)
        csr = chipStatusRegister | CSR_pumprobe_MSK;
    else
        csr = chipStatusRegister & ~CSR_pumprobe_MSK;
    return csr;
}

int getDigitalPulsing() {
    return ((chipStatusRegister & CSR_dpulse_MSK) >> CSR_dpulse);
}

int M3SetDigitalPulsing(int enable) {
    LOG(logINFO,
        ("%s Digital Pulsing\n", enable == 0 ? "Disabling" : "Enabling"));
    int csr = 0;
    if (enable)
        csr = chipStatusRegister | CSR_dpulse_MSK;
    else
        csr = chipStatusRegister & ~CSR_dpulse_MSK;
    return csr;
}

int getAnalogPulsing() {
    return ((chipStatusRegister & CSR_apulse_MSK) >> CSR_apulse);
}

int M3SetAnalogPulsing(int enable) {
    LOG(logINFO,
        ("%s Analog Pulsing\n", enable == 0 ? "Disabling" : "Enabling"));
    int csr = 0;
    if (enable)
        csr = chipStatusRegister | CSR_apulse_MSK;
    else
        csr = chipStatusRegister & ~CSR_apulse_MSK;
    return csr;
}

int getNegativePolarity() {
    return ((chipStatusRegister & CSR_invpol_MSK) >> CSR_invpol);
}

int M3SetNegativePolarity(int enable) {
    LOG(logINFO,
        ("%s Negative Polarity\n", enable == 0 ? "Disabling" : "Enabling"));
    int csr = 0;
    if (enable)
        csr = chipStatusRegister | CSR_invpol_MSK;
    else
        csr = chipStatusRegister & ~CSR_invpol_MSK;
    return csr;
}

patternParameters *setChannelRegisterChip(int ichip, char *mask,
                                          int *trimbits) {

    patternParameters *pat = malloc(sizeof(patternParameters));
    memset(pat, 0, sizeof(patternParameters));

    // validate
    for (int ichan = ichip * NCHAN_1_COUNTER * NCOUNTERS;
         ichan <
         ichip * NCHAN_1_COUNTER * NCOUNTERS + NCHAN_1_COUNTER * NCOUNTERS;
         ichan++) {
        if (trimbits[ichan] < 0) {
            LOG(logERROR, ("Trimbit value (%d) for channel %d is invalid - "
                           "setting it to 0\n",
                           trimbits[ichan], ichan));
            trimbits[ichan] = 0;
        }
        if (trimbits[ichan] > 63) {
            LOG(logERROR, ("Trimbit value (%d) for channel %d is invalid - "
                           "settings it to 63\n",
                           trimbits[ichan], ichan));
            trimbits[ichan] = 63;
        }
    }
    LOG(logINFO, ("Trimbits validated\n"));

    // trimming
    int error = 0;
    uint64_t patword = 0;
    int iaddr = 0;

    LOG(logDEBUG1, (" Chip %d\n", ichip));
    iaddr = 0;
    patword = 0;
    pat->word[iaddr++] = patword;

    // chip select
    patword = setBit(SIGNAL_TBLoad_1 + ichip, patword);
    pat->word[iaddr++] = patword;

    // reset trimbits
    patword = setBit(SIGNAL_resStorage, patword);
    patword = setBit(SIGNAL_resCounter, patword);
    pat->word[iaddr++] = patword;
    pat->word[iaddr++] = patword;
    patword = clearBit(SIGNAL_resStorage, patword);
    patword = clearBit(SIGNAL_resCounter, patword);
    pat->word[iaddr++] = patword;
    pat->word[iaddr++] = patword;

    // select first channel
    patword = setBit(SIGNAL_CHSserialIN, patword);
    pat->word[iaddr++] = patword;
    // 1 clk pulse
    patword = setBit(SIGNAL_CHSclk, patword);
    pat->word[iaddr++] = patword;
    patword = clearBit(SIGNAL_CHSclk, patword);
    // clear 1st channel
    pat->word[iaddr++] = patword;
    patword = clearBit(SIGNAL_CHSserialIN, patword);
    // 2 clk pulses
    for (int i = 0; i < 2; i++) {
        patword = setBit(SIGNAL_CHSclk, patword);
        pat->word[iaddr++] = patword;
        patword = clearBit(SIGNAL_CHSclk, patword);
        pat->word[iaddr++] = patword;
    }

    // for each channel (all chips)
    for (int ich = 0; ich < NCHAN_1_COUNTER; ich++) {
        LOG(logDEBUG1, (" Chip %d, Channel %d\n", ichip, ich));
        int chanReg =
            64 *
            (trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS + NCOUNTERS * ich] +
             trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS + NCOUNTERS * ich +
                      1] *
                 64 +
             trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS + NCOUNTERS * ich +
                      2] *
                 64 * 64);

        for (int icounter = 0; icounter != 3; ++icounter) {
            if (mask[ichip * NCHAN + ich * NCOUNTERS + icounter]) {
                LOG(logDEBUG1,
                    ("badchannel [modCounter:%d, modChan:%d, ichip:%d, ich:%d, "
                     "icounter:%d]\n",
                     ichip * NCHAN + ich * NCOUNTERS + icounter,
                     ichip * NCHAN_1_COUNTER + ich, ichip, ich, icounter));
                chanReg |= (0x1 << (3 + icounter));
            }
        }
        chanReg /= 2;
        // deserialize
        if (chanReg & CHAN_REG_BAD_CHANNEL_MSK) {
            LOG(logINFOBLUE,
                ("badchannel [chanReg:0x%x modCounter:%d, modChan:%d, "
                 "ichip:%d, ich:%d]\n",
                 chanReg, ichip * NCHAN + ich * NCOUNTERS,
                 ichip * NCHAN_1_COUNTER + ich, ichip, ich));
        }
        for (int i = 0; i < 23; i++) {
            patword = clearBit(SIGNAL_clk, patword);
            pat->word[iaddr++] = patword;

            if (chanReg & (1 << (i + 1))) {
                patword = setBit(SIGNAL_serialIN, patword);
            } else {
                patword = clearBit(SIGNAL_serialIN, patword);
            }

            patword = setBit(SIGNAL_clk, patword);
            pat->word[iaddr++] = patword;
        }
        pat->word[iaddr++] = patword;
        pat->word[iaddr++] = patword;

        // move to next channel
        for (int i = 0; i < 3; i++) {
            patword = setBit(SIGNAL_CHSclk, patword);
            pat->word[iaddr++] = patword;
            patword = clearBit(SIGNAL_CHSclk, patword);
            pat->word[iaddr++] = patword;
        }
    }
    // chip unselect
    patword = clearBit(SIGNAL_TBLoad_1 + ichip, patword);
    pat->word[iaddr++] = patword;

    // last iaddr check
    if (iaddr >= MAX_PATTERN_LENGTH) {
        LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n", iaddr,
                       MAX_PATTERN_LENGTH));
        error = 1;
    }

    if (iaddr >= MAX_PATTERN_LENGTH) {
        LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n", iaddr,
                       MAX_PATTERN_LENGTH));
        error = 1;
    }
    // set pattern wait address
    for (int i = 0; i < M3_MAX_PATTERN_LEVELS; i++)
        pat->wait[i] = MAX_PATTERN_LENGTH - 1;
    // pattern loop
    for (int i = 0; i < M3_MAX_PATTERN_LEVELS; i++) {
        // int stop = MAX_PATTERN_LENGTH - 1, nloop = 0;
        pat->startloop[i] = MAX_PATTERN_LENGTH - 1;
        pat->stoploop[i] = MAX_PATTERN_LENGTH - 1;
        pat->nloop[i] = 0;
    }

    // pattern limits
    {
        pat->limits[0] = 0;
        pat->limits[1] = iaddr;
    }

    if (error == 0) {

        LOG(logINFO, ("All trimbits have been loaded\n"));
    } else {
        free(pat);
        return NULL;
    }
    return pat;
}
