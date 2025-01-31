// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MYTHEN3_H
#define MYTHEN3_H

#include "Pattern.h"

/** Signal Definitions */
#define SIGNAL_TBLoad_1    (0)
#define SIGNAL_TBLoad_2    (1)
#define SIGNAL_TBLoad_3    (2)
#define SIGNAL_TBLoad_4    (3)
#define SIGNAL_TBLoad_5    (4)
#define SIGNAL_TBLoad_6    (5)
#define SIGNAL_TBLoad_7    (6)
#define SIGNAL_TBLoad_8    (7)
#define SIGNAL_TBLoad_9    (8)
#define SIGNAL_TBLoad_10   (9)
#define SIGNAL_AnaMode     (10)
#define SIGNAL_CHSserialIN (11)
#define SIGNAL_READOUT     (12)
#define SIGNAL_pulse       (13)
#define SIGNAL_EN1         (14)
#define SIGNAL_EN2         (15)
#define SIGNAL_EN3         (16)
#define SIGNAL_clk         (17)
#define SIGNAL_SRmode      (18)
#define SIGNAL_serialIN    (19)
#define SIGNAL_STO         (20)
#define SIGNAL_STATLOAD    (21)
#define SIGNAL_resStorage  (22)
#define SIGNAL_resCounter  (23)
#define SIGNAL_CHSclk      (24)
#define SIGNAL_exposing    (25)

// CHIP STARTUS REGISTER BITS
#define CSR_spypads  0
#define CSR_invpol   4
#define CSR_dpulse   5
#define CSR_interp   6
#define _CSR_C10pre  7 // #default, negative polarity
#define CSR_pumprobe 8
#define CSR_apulse   9
#define CSR_C15sh    10
#define CSR_C30sh    11 // #default
#define CSR_C50sh    12
#define CSR_C225ACsh                                                           \
    13 // Connects 225fF SHAPER AC cap (1: 225 to shaper, 225 to GND. 0: 450 to
       // shaper)
#define _CSR_C15pre 14 // negative polarity

#define CSR_invpol_MSK   (0x1 << CSR_invpol)
#define CSR_dpulse_MSK   (0x1 << CSR_dpulse)
#define CSR_interp_MSK   (0x1 << CSR_interp)
#define CSR_pumprobe_MSK (0x1 << CSR_pumprobe)
#define CSR_apulse_MSK   (0x1 << CSR_apulse)

#define CSR_default (1 << _CSR_C10pre) | (1 << CSR_C30sh)

#define GAIN_MASK                                                              \
    ((1 << _CSR_C10pre) | (1 << CSR_C15sh) | (1 << CSR_C30sh) |                \
     (1 << CSR_C50sh) | (1 << CSR_C225ACsh) | (1 << _CSR_C15pre))

#define CHAN_REG_BAD_CHANNEL_MSK (0x38)

int setBit(int ibit, int patword);
int clearBit(int ibit, int patword);
int getChipStatusRegister();

patternParameters *setChipStatusRegisterPattern(int csr);
patternParameters *setChannelRegisterChip(int ichip, char *mask, int *trimbits);
void flipNegativePolarityBits(int *csr);
int getGainCaps();
int M3SetGainCaps(int caps);
int getInterpolation();
int M3SetInterpolation(int enable);
int getPumpProbe();
int M3SetPumpProbe(int enable);
int getDigitalPulsing();
int M3SetDigitalPulsing(int enable);
int getAnalogPulsing();
int M3SetAnalogPulsing(int enable);
int getNegativePolarity();
int M3SetNegativePolarity(int enable);

#endif
