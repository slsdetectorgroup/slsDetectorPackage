// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once



#define CTRLREG1REG (0x0)

#define CTRLREG1_OFST (0)
#define CTRLREG1_MSK  (0xffffffff << CTRLREG1_OFST)


#define CTRLREG2REG (0x4)

#define CTRLREG2_OFST (0)
#define CTRLREG2_MSK  (0xffffffff << CTRLREG2_OFST)


#define STATUSREG1REG (0x8)

#define STATUSREG1_OFST (0)
#define STATUSREG1_MSK  (0xffffffff << STATUSREG1_OFST)


#define STATUSREG2REG (0xC)

#define STATUSREG2_OFST (0)
#define STATUSREG2_MSK  (0xffffffff << STATUSREG2_OFST)


#define EXPCTRLREG (0x10)

#define STARTP_OFST (0)
#define STARTP_MSK  (0x00000001 << STARTP_OFST)


#define EMPTY14REG (0x14)

#define EMPTY14_OFST (0)
#define EMPTY14_MSK  (0xffffffff << EMPTY14_OFST)


#define EXPFRAMESREG (0x18)

#define NOFRAMES_OFST (0)
#define NOFRAMES_MSK  (0xffffffff << NOFRAMES_OFST)


#define EMPTY1CREG (0x1C)

#define EMPTY1C_OFST (0)
#define EMPTY1C_MSK  (0xffffffff << EMPTY1C_OFST)


#define EXPTIMEREG (0x20)

#define EXPTIME_OFST (0)
#define EXPTIME_MSK  (0xffffffff << EXPTIME_OFST)


#define EMPTY24REG (0x24)

#define EMPTY24_OFST (0)
#define EMPTY24_MSK  (0xffffffff << EMPTY24_OFST)


#define PKTPACKETLENGTHREG (0x28)

#define PACKETLENGTH1G_OFST (0)
#define PACKETLENGTH1G_MSK  (0x0000ffff << PACKETLENGTH1G_OFST)
#define PACKETLENGTH10G_OFST (16)
#define PACKETLENGTH10G_MSK  (0x0000ffff << PACKETLENGTH10G_OFST)


#define EMPTY2CREG (0x2C)

#define EMPTY2C_OFST (0)
#define EMPTY2C_MSK  (0xffffffff << EMPTY2C_OFST)


#define PKTNOPACKETSREG (0x30)

#define NOPACKETS1G_OFST (0)
#define NOPACKETS1G_MSK  (0x0000003f << NOPACKETS1G_OFST)
#define NOPACKETS10G_OFST (8)
#define NOPACKETS10G_MSK  (0x0000003f << NOPACKETS10G_OFST)


#define EMPTY34REG (0x34)

#define EMPTY34_OFST (0)
#define EMPTY34_MSK  (0xffffffff << EMPTY34_OFST)


#define PKTCTRLREG (0x38)

#define NSERVERS_OFST (0)
#define NSERVERS_MSK  (0x0000003f << NSERVERS_OFST)
#define SERVERSTART_OFST (6)
#define SERVERSTART_MSK  (0x0000001f << SERVERSTART_OFST)
#define ETHINTERF_OFST (11)
#define ETHINTERF_MSK  (0x00000001 << ETHINTERF_OFST)


#define EMPTY3CREG (0x3C)

#define EMPTY3C_OFST (0)
#define EMPTY3C_MSK  (0xffffffff << EMPTY3C_OFST)


#define PKTCOORD1 (0x40)

#define COORDX_OFST (0)
#define COORDX_MSK  (0x0000ffff << COORDX_OFST)
#define COORDY_OFST (16)
#define COORDY_MSK  (0x0000ffff << COORDY_OFST)


#define EMPTY44REG (0x44)

#define EMPTY44_OFST (0)
#define EMPTY44_MSK  (0xffffffff << EMPTY44_OFST)


#define PKTCOORD2 (0x48)

#define COORDZ_OFST (0)
#define COORDZ_MSK  (0x0000ffff << COORDZ_OFST)


#define EMPTY4CREG (0x4C)

#define EMPTY4C_OFST (0)
#define EMPTY4C_MSK  (0xffffffff << EMPTY4C_OFST)


#define EMPTY50REG (0x50)

#define EMPTY50_OFST (0)
#define EMPTY50_MSK  (0xffffffff << EMPTY50_OFST)


#define EMPTY54REG (0x54)

#define EMPTY54_OFST (0)
#define EMPTY54_MSK  (0xffffffff << EMPTY54_OFST)


#define EMPTY58REG (0x58)

#define EMPTY58_OFST (0)
#define EMPTY58_MSK  (0xffffffff << EMPTY58_OFST)


#define EMPTY5CREG (0x5C)

#define EMPTY5C_OFST (0)
#define EMPTY5C_MSK  (0xffffffff << EMPTY5C_OFST)


#define EMPTY60REG (0x60)

#define EMPTY60_OFST (0)
#define EMPTY60_MSK  (0xffffffff << EMPTY60_OFST)


#define EMPTY64REG (0x64)

#define EMPTY64_OFST (0)
#define EMPTY64_MSK  (0xffffffff << EMPTY64_OFST)


#define EMPTY68REG (0x68)

#define EMPTY68_OFST (0)
#define EMPTY68_MSK  (0xffffffff << EMPTY68_OFST)


#define EMPTY6CREG (0x6C)

#define EMPTY6C_OFST (0)
#define EMPTY6C_MSK  (0xffffffff << EMPTY6C_OFST)


#define EMPTY70REG (0x70)

#define EMPTY70_OFST (0)
#define EMPTY70_MSK  (0xffffffff << EMPTY70_OFST)


#define EMPTY74REG (0x74)

#define EMPTY74_OFST (0)
#define EMPTY74_MSK  (0xffffffff << EMPTY74_OFST)


#define EMPTY78REG (0x78)

#define EMPTY78_OFST (0)
#define EMPTY78_MSK  (0xffffffff << EMPTY78_OFST)


#define EMPTY7CREG (0x7C)

#define EMPTY7C_OFST (0)
#define EMPTY7C_MSK  (0xffffffff << EMPTY7C_OFST)


#define EMPTY80REG (0x80)

#define EMPTY80_OFST (0)
#define EMPTY80_MSK  (0xffffffff << EMPTY80_OFST)


#define EMPTY84REG (0x84)

#define EMPTY84_OFST (0)
#define EMPTY84_MSK  (0xffffffff << EMPTY84_OFST)


#define EMPTY88REG (0x88)

#define EMPTY88_OFST (0)
#define EMPTY88_MSK  (0xffffffff << EMPTY88_OFST)


#define EMPTY8CREG (0x8C)

#define EMPTY8C_OFST (0)
#define EMPTY8C_MSK  (0xffffffff << EMPTY8C_OFST)


#define EMPTY90REG (0x90)

#define EMPTY90_OFST (0)
#define EMPTY90_MSK  (0xffffffff << EMPTY90_OFST)


#define EMPTY94REG (0x94)

#define EMPTY94_OFST (0)
#define EMPTY94_MSK  (0xffffffff << EMPTY94_OFST)


#define EMPTY98REG (0x98)

#define EMPTY98_OFST (0)
#define EMPTY98_MSK  (0xffffffff << EMPTY98_OFST)


#define EMPTY9CREG (0x9C)

#define EMPTY9C_OFST (0)
#define EMPTY9C_MSK  (0xffffffff << EMPTY9C_OFST)


#define FLOWSTATUSREG (0x100)

#define RSMBUSY_OFST (0)
#define RSMBUSY_MSK  (0x00000001 << RSMBUSY_OFST)
#define RSMTRGWAIT_OFST (3)
#define RSMTRGWAIT_MSK  (0x00000001 << RSMTRGWAIT_OFST)
#define RSMDLYBEFOREACQ_OFST (4)
#define RSMDLYBEFOREACQ_MSK  (0x00000001 << RSMDLYBEFOREACQ_OFST)
#define RSMFIFOFULL_OFST (5)
#define RSMFIFOFULL_MSK  (0x00000001 << RSMFIFOFULL_OFST)
#define CSMDLYAFTERACQ_OFST (15)
#define CSMDLYAFTERACQ_MSK  (0x00000001 << CSMDLYAFTERACQ_OFST)
#define CSMBUSY_OFST (17)
#define CSMBUSY_MSK  (0x00000001 << CSMBUSY_OFST)


#define EMPTY104REG (0x104)

#define EMPTY104_OFST (0)
#define EMPTY104_MSK  (0xffffffff << EMPTY104_OFST)


#define FLOWCONTROLREG (0x108)

#define FLOWCONTROL_OFST (0)
#define FLOWCONTROL_MSK  (0xffffffff << FLOWCONTROL_OFST)


#define EMPTY10CREG (0x10C)

#define EMPTY10C_OFST (0)
#define EMPTY10C_MSK  (0xffffffff << EMPTY10C_OFST)


#define TIMEFROMSTARTOUTREG1 (0x110)

#define TIMEFROMSTARTOUT1_OFST (0)
#define TIMEFROMSTARTOUT1_MSK  (0xffffffff << TIMEFROMSTARTOUT1_OFST)


#define TIMEFROMSTARTOUTREG2 (0x114)

#define TIMEFROMSTARTOUT2_OFST (0)
#define TIMEFROMSTARTOUT2_MSK  (0xffffffff << TIMEFROMSTARTOUT2_OFST)


#define FRAMESFROMSTARTOUTREG1 (0x118)

#define FRAMESFROMSTARTOUT1_OFST (0)
#define FRAMESFROMSTARTOUT1_MSK  (0xffffffff << FRAMESFROMSTARTOUT1_OFST)


#define FRAMESFROMSTARTOUTREG2 (0x11C)

#define FRAMESFROMSTARTOUT2_OFST (0)
#define FRAMESFROMSTARTOUT2_MSK  (0xffffffff << FRAMESFROMSTARTOUT2_OFST)


#define FRAMETIMEOUTREG1 (0x120)

#define FRAMETIMEOUT1_OFST (0)
#define FRAMETIMEOUT1_MSK  (0xffffffff << FRAMETIMEOUT1_OFST)


#define FRAMETIMEOUTREG2 (0x124)

#define FRAMETIMEOUT2_OFST (0)
#define FRAMETIMEOUT2_MSK  (0xffffffff << FRAMETIMEOUT2_OFST)


#define DELAYOUTREG1 (0x128)

#define DELAYOUT1_OFST (0)
#define DELAYOUT1_MSK  (0xffffffff << DELAYOUT1_OFST)


#define DELAYOUTREG2 (0x12C)

#define DELAYOUT2_OFST (0)
#define DELAYOUT2_MSK  (0xffffffff << DELAYOUT2_OFST)


#define CYCLESOUTREG1 (0x130)

#define CYCLESOUT1_OFST (0)
#define CYCLESOUT1_MSK  (0xffffffff << CYCLESOUT1_OFST)


#define CYCLESOUTREG2 (0x134)

#define CYCLESOUT2_OFST (0)
#define CYCLESOUT2_MSK  (0xffffffff << CYCLESOUT2_OFST)


#define FRAMESOUTREG1 (0x138)

#define FRAMESOUT1_OFST (0)
#define FRAMESOUT1_MSK  (0xffffffff << FRAMESOUT1_OFST)


#define FRAMESOUTREG2 (0x13C)

#define FRAMESOUT2_OFST (0)
#define FRAMESOUT2_MSK  (0xffffffff << FRAMESOUT2_OFST)


#define PERIODOUTREG1 (0x140)

#define PERIODOUT1_OFST (0)
#define PERIODOUT1_MSK  (0xffffffff << PERIODOUT1_OFST)


#define PERIODOUTREG2 (0x144)

#define PERIODOUT2_OFST (0)
#define PERIODOUT2_MSK  (0xffffffff << PERIODOUT2_OFST)


#define DELAYINREG1 (0x148)

#define DELAYIN1_OFST (0)
#define DELAYIN1_MSK  (0xffffffff << DELAYIN1_OFST)


#define DELAYINREG2 (0x14C)

#define DELAYIN2_OFST (0)
#define DELAYIN2_MSK  (0xffffffff << DELAYIN2_OFST)


#define CYCLESINREG1 (0x150)

#define CYCLESIN1_OFST (0)
#define CYCLESIN1_MSK  (0xffffffff << CYCLESIN1_OFST)


#define CYCLESINREG2 (0x154)

#define CYCLESIN2_OFST (0)
#define CYCLESIN2_MSK  (0xffffffff << CYCLESIN2_OFST)


#define FRAMESINREG1 (0x158)

#define FRAMESIN1_OFST (0)
#define FRAMESIN1_MSK  (0xffffffff << FRAMESIN1_OFST)


#define FRAMESINREG2 (0x15C)

#define FRAMESIN2_OFST (0)
#define FRAMESIN2_MSK  (0xffffffff << FRAMESIN2_OFST)


#define PERIODINREG1 (0x160)

#define PERIODIN1_OFST (0)
#define PERIODIN1_MSK  (0xffffffff << PERIODIN1_OFST)


#define PERIODINREG2 (0x164)

#define PERIODIN2_OFST (0)
#define PERIODIN2_MSK  (0xffffffff << PERIODIN2_OFST)


#define EMPTY168REG (0x168)

#define EMPTY168_OFST (0)
#define EMPTY168_MSK  (0xffffffff << EMPTY168_OFST)


#define EMPTY16CREG (0x16C)

#define EMPTY16C_OFST (0)
#define EMPTY16C_MSK  (0xffffffff << EMPTY16C_OFST)


#define EMPTY170REG (0x170)

#define EMPTY170_OFST (0)
#define EMPTY170_MSK  (0xffffffff << EMPTY170_OFST)


#define EMPTY174REG (0x174)

#define EMPTY174_OFST (0)
#define EMPTY174_MSK  (0xffffffff << EMPTY174_OFST)


#define EMPTY178REG (0x178)

#define EMPTY178_OFST (0)
#define EMPTY178_MSK  (0xffffffff << EMPTY178_OFST)


#define EMPTY17CREG (0x17C)

#define EMPTY17C_OFST (0)
#define EMPTY17C_MSK  (0xffffffff << EMPTY17C_OFST)


#define EMPTY180REG (0x180)

#define EMPTY180_OFST (0)
#define EMPTY180_MSK  (0xffffffff << EMPTY180_OFST)


#define EMPTY184REG (0x184)

#define EMPTY184_OFST (0)
#define EMPTY184_MSK  (0xffffffff << EMPTY184_OFST)


#define EMPTY188REG (0x188)

#define EMPTY188_OFST (0)
#define EMPTY188_MSK  (0xffffffff << EMPTY188_OFST)


#define EMPTY18CREG (0x18C)

#define EMPTY18C_OFST (0)
#define EMPTY18C_MSK  (0xffffffff << EMPTY18C_OFST)


#define EMPTY190REG (0x190)

#define EMPTY190_OFST (0)
#define EMPTY190_MSK  (0xffffffff << EMPTY190_OFST)


#define EMPTY194REG (0x194)

#define EMPTY194_OFST (0)
#define EMPTY194_MSK  (0xffffffff << EMPTY194_OFST)


#define EMPTY198REG (0x198)

#define EMPTY198_OFST (0)
#define EMPTY198_MSK  (0xffffffff << EMPTY198_OFST)


#define EMPTY19CREG (0x19C)

#define EMPTY19C_OFST (0)
#define EMPTY19C_MSK  (0xffffffff << EMPTY19C_OFST)


#define PATTERNOUT0REG (0x200)

#define PATTERNOUT0_OFST (0)
#define PATTERNOUT0_MSK  (0xffffffff << PATTERNOUT0_OFST)


#define PATTERNOUT1REG (0x204)

#define PATTERNOUT1_OFST (0)
#define PATTERNOUT1_MSK  (0xffffffff << PATTERNOUT1_OFST)


#define PATTERNIN0REG (0x208)

#define PATTERNIN0_OFST (0)
#define PATTERNIN0_MSK  (0xffffffff << PATTERNIN0_OFST)


#define PATTERNIN1REG (0x20C)

#define PATTERNIN1_OFST (0)
#define PATTERNIN1_MSK  (0xffffffff << PATTERNIN1_OFST)


#define PATTERNIOMASK0REG (0x210)

#define PATTERNIOMASK0_OFST (0)
#define PATTERNIOMASK0_MSK  (0xffffffff << PATTERNIOMASK0_OFST)


#define PATTERNIOMASK1REG (0x214)

#define PATTERNIOMASK1_OFST (0)
#define PATTERNIOMASK1_MSK  (0xffffffff << PATTERNIOMASK1_OFST)


#define PATTERNIOSET0REG (0x218)

#define PATTERNIOSET0_OFST (0)
#define PATTERNIOSET0_MSK  (0xffffffff << PATTERNIOSET0_OFST)


#define PATTERNIOSET1REG (0x21C)

#define PATTERNIOSET1_OFST (0)
#define PATTERNIOSET1_MSK  (0xffffffff << PATTERNIOSET1_OFST)


#define PATTERNCONTROLREG (0x220)

#define PATTERNCONTROL_OFST (0)
#define PATTERNCONTROL_MSK  (0xffffffff << PATTERNCONTROL_OFST)


#define EMPTY224REG (0x224)

#define EMPTY224_OFST (0)
#define EMPTY224_MSK  (0xffffffff << EMPTY224_OFST)


#define PATTERNLIMITADDRESSREG (0x228)

#define PATTERNLIMITADDRESS_OFST (0)
#define PATTERNLIMITADDRESS_MSK  (0xffffffff << PATTERNLIMITADDRESS_OFST)


#define EMPTY22CREG (0x22C)

#define EMPTY22C_OFST (0)
#define EMPTY22C_MSK  (0xffffffff << EMPTY22C_OFST)


#define PATTERNLOOP1ADDRESSREG (0x230)

#define PATTERNLOOP1ADDRESS_OFST (0)
#define PATTERNLOOP1ADDRESS_MSK  (0xffffffff << PATTERNLOOP1ADDRESS_OFST)


#define EMPTY234REG (0x234)

#define EMPTY234_OFST (0)
#define EMPTY234_MSK  (0xffffffff << EMPTY234_OFST)


#define PATTERNNLOOPS1REG (0x238)

#define PATTERNNLOOPS1_OFST (0)
#define PATTERNNLOOPS1_MSK  (0xffffffff << PATTERNNLOOPS1_OFST)


#define EMPTY23CREG (0x23C)

#define EMPTY23C_OFST (0)
#define EMPTY23C_MSK  (0xffffffff << EMPTY23C_OFST)


#define PATTERNWAIT1ADDRESSREG (0x240)

#define PATTERNWAIT1ADDRESS_OFST (0)
#define PATTERNWAIT1ADDRESS_MSK  (0xffffffff << PATTERNWAIT1ADDRESS_OFST)


#define EMPTY244REG (0x244)

#define EMPTY244_OFST (0)
#define EMPTY244_MSK  (0xffffffff << EMPTY244_OFST)


#define PATTERNWAIT1TIME1REG (0x248)

#define PATTERNWAIT1TIME1_OFST (0)
#define PATTERNWAIT1TIME1_MSK  (0xffffffff << PATTERNWAIT1TIME1_OFST)


#define PATTERNWAIT1TIME2REG (0x24C)

#define PATTERNWAIT1TIME2_OFST (0)
#define PATTERNWAIT1TIME2_MSK  (0xffffffff << PATTERNWAIT1TIME2_OFST)


#define PATTERNLOOP2ADDRESSREG (0x250)

#define PATTERNLOOP2ADDRESS_OFST (0)
#define PATTERNLOOP2ADDRESS_MSK  (0xffffffff << PATTERNLOOP2ADDRESS_OFST)


#define EMPTY254REG (0x254)

#define EMPTY254_OFST (0)
#define EMPTY254_MSK  (0xffffffff << EMPTY254_OFST)


#define PATTERNNLOOPS2REG (0x258)

#define PATTERNNLOOPS2_OFST (0)
#define PATTERNNLOOPS2_MSK  (0xffffffff << PATTERNNLOOPS2_OFST)


#define EMPTY25CREG (0x25C)

#define EMPTY25C_OFST (0)
#define EMPTY25C_MSK  (0xffffffff << EMPTY25C_OFST)


#define PATTERNWAIT2ADDRESSREG (0x260)

#define PATTERNWAIT2ADDRESS_OFST (0)
#define PATTERNWAIT2ADDRESS_MSK  (0xffffffff << PATTERNWAIT2ADDRESS_OFST)


#define EMPTY264REG (0x264)

#define EMPTY264_OFST (0)
#define EMPTY264_MSK  (0xffffffff << EMPTY264_OFST)


#define PATTERNWAIT2TIME1REG (0x268)

#define PATTERNWAIT2TIME1_OFST (0)
#define PATTERNWAIT2TIME1_MSK  (0xffffffff << PATTERNWAIT2TIME1_OFST)


#define PATTERNWAIT2TIME2REG (0x26C)

#define PATTERNWAIT2TIME2_OFST (0)
#define PATTERNWAIT2TIME2_MSK  (0xffffffff << PATTERNWAIT2TIME2_OFST)


#define PATTERNLOOP3ADDRESSREG (0x270)

#define PATTERNLOOP3ADDRESS_OFST (0)
#define PATTERNLOOP3ADDRESS_MSK  (0xffffffff << PATTERNLOOP3ADDRESS_OFST)


#define EMPTY274REG (0x274)

#define EMPTY274_OFST (0)
#define EMPTY274_MSK  (0xffffffff << EMPTY274_OFST)


#define PATTERNNLOOPS3REG (0x278)

#define PATTERNNLOOPS3_OFST (0)
#define PATTERNNLOOPS3_MSK  (0xffffffff << PATTERNNLOOPS3_OFST)


#define EMPTY27CREG (0x27C)

#define EMPTY27C_OFST (0)
#define EMPTY27C_MSK  (0xffffffff << EMPTY27C_OFST)


#define PATTERNWAIT3ADDRESSREG (0x280)

#define PATTERNWAIT3ADDRESS_OFST (0)
#define PATTERNWAIT3ADDRESS_MSK  (0xffffffff << PATTERNWAIT3ADDRESS_OFST)


#define EMPTY284REG (0x284)

#define EMPTY284_OFST (0)
#define EMPTY284_MSK  (0xffffffff << EMPTY284_OFST)


#define PATTERNWAIT3TIME1REG (0x288)

#define PATTERNWAIT3TIME1_OFST (0)
#define PATTERNWAIT3TIME1_MSK  (0xffffffff << PATTERNWAIT3TIME1_OFST)


#define PATTERNWAIT3TIME2REG (0x28C)

#define PATTERNWAIT3TIME2_OFST (0)
#define PATTERNWAIT3TIME2_MSK  (0xffffffff << PATTERNWAIT3TIME2_OFST)


#define PATTERNLOOP4ADDRESSREG (0x290)

#define PATTERNLOOP4ADDRESS_OFST (0)
#define PATTERNLOOP4ADDRESS_MSK  (0xffffffff << PATTERNLOOP4ADDRESS_OFST)


#define EMPTY294REG (0x294)

#define EMPTY294_OFST (0)
#define EMPTY294_MSK  (0xffffffff << EMPTY294_OFST)


#define PATTERNNLOOPS4REG (0x298)

#define PATTERNNLOOPS4_OFST (0)
#define PATTERNNLOOPS4_MSK  (0xffffffff << PATTERNNLOOPS4_OFST)


#define EMPTY29CREG (0x29C)

#define EMPTY29C_OFST (0)
#define EMPTY29C_MSK  (0xffffffff << EMPTY29C_OFST)


#define PATTERNWAIT4ADDRESSREG (0x300)

#define PATTERNWAIT4ADDRESS_OFST (0)
#define PATTERNWAIT4ADDRESS_MSK  (0xffffffff << PATTERNWAIT4ADDRESS_OFST)


#define EMPTY304REG (0x304)

#define EMPTY304_OFST (0)
#define EMPTY304_MSK  (0xffffffff << EMPTY304_OFST)


#define PATTERNWAIT4TIME1REG (0x308)

#define PATTERNWAI4TIME1_OFST (0)
#define PATTERNWAI4TIME1_MSK  (0xffffffff << PATTERNWAI4TIME1_OFST)


#define PATTERNWAIT4TIME2REG (0x30C)

#define PATTERNWAIT4TIME2_OFST (0)
#define PATTERNWAIT4TIME2_MSK  (0xffffffff << PATTERNWAIT4TIME2_OFST)


#define PATTERNLOOP5ADDRESSREG (0x310)

#define PATTERNLOOP5ADDRESS_OFST (0)
#define PATTERNLOOP5ADDRESS_MSK  (0xffffffff << PATTERNLOOP5ADDRESS_OFST)


#define EMPTY314REG (0x314)

#define EMPTY314_OFST (0)
#define EMPTY314_MSK  (0xffffffff << EMPTY314_OFST)


#define PATTERNNLOOPS5REG (0x318)

#define PATTERNNLOOPS5_OFST (0)
#define PATTERNNLOOPS5_MSK  (0xffffffff << PATTERNNLOOPS5_OFST)


#define EMPTY31CREG (0x31C)

#define EMPTY31C_OFST (0)
#define EMPTY31C_MSK  (0xffffffff << EMPTY31C_OFST)


#define PATTERNWAIT5ADDRESSREG (0x320)

#define PATTERNWAIT5ADDRESS_OFST (0)
#define PATTERNWAIT5ADDRESS_MSK  (0xffffffff << PATTERNWAIT5ADDRESS_OFST)


#define EMPTY324REG (0x324)

#define EMPTY324_OFST (0)
#define EMPTY324_MSK  (0xffffffff << EMPTY324_OFST)


#define PATTERNWAIT5TIME1REG (0x328)

#define PATTERNWAIT5TIME1_OFST (0)
#define PATTERNWAIT5TIME1_MSK  (0xffffffff << PATTERNWAIT5TIME1_OFST)


#define PATTERNWAIT5TIME2REG (0x32C)

#define PATTERNWAIT5TIME2_OFST (0)
#define PATTERNWAIT5TIME2_MSK  (0xffffffff << PATTERNWAIT5TIME2_OFST)


#define PATTERNLOOP6ADDRESSREG (0x330)

#define PATTERNLOOP6ADDRESS_OFST (0)
#define PATTERNLOOP6ADDRESS_MSK  (0xffffffff << PATTERNLOOP6ADDRESS_OFST)


#define EMPTY334REG (0x334)

#define EMPTY334_OFST (0)
#define EMPTY334_MSK  (0xffffffff << EMPTY334_OFST)


#define PATTERNNLOOPS6REG (0x338)

#define PATTERNNLOOPS6_OFST (0)
#define PATTERNNLOOPS6_MSK  (0xffffffff << PATTERNNLOOPS6_OFST)


#define EMPTY33CREG (0x33C)

#define EMPTY33C_OFST (0)
#define EMPTY33C_MSK  (0xffffffff << EMPTY33C_OFST)


#define PATTERNWAIT6ADDRESSREG (0x340)

#define PATTERNWAIT6ADDRESS_OFST (0)
#define PATTERNWAIT6ADDRESS_MSK  (0xffffffff << PATTERNWAIT6ADDRESS_OFST)


#define EMPTY344REG (0x344)

#define EMPTY344_OFST (0)
#define EMPTY344_MSK  (0xffffffff << EMPTY344_OFST)


#define PATTERNWAIT6TIME1REG (0x348)

#define PATTERNWAIT6TIME1_OFST (0)
#define PATTERNWAIT6TIME1_MSK  (0xffffffff << PATTERNWAIT6TIME1_OFST)


#define PATTERNWAIT6TIME2REG (0x34C)

#define PATTERNWAIT6TIME2_OFST (0)
#define PATTERNWAIT6TIME2_MSK  (0xffffffff << PATTERNWAIT6TIME2_OFST)


#define EMPTY350REG (0x350)

#define EMPTY350_OFST (0)
#define EMPTY350_MSK  (0xffffffff << EMPTY350_OFST)


#define EMPTY354REG (0x354)

#define EMPTY354_OFST (0)
#define EMPTY354_MSK  (0xffffffff << EMPTY354_OFST)


#define EMPTY358REG (0x358)

#define EMPTY358_OFST (0)
#define EMPTY358_MSK  (0xffffffff << EMPTY358_OFST)


#define EMPTY35CREG (0x35C)

#define EMPTY35C_OFST (0)
#define EMPTY35C_MSK  (0xffffffff << EMPTY35C_OFST)


#define EMPTY360REG (0x360)

#define EMPTY360_OFST (0)
#define EMPTY360_MSK  (0xffffffff << EMPTY360_OFST)


#define EMPTY364REG (0x364)

#define EMPTY364_OFST (0)
#define EMPTY364_MSK  (0xffffffff << EMPTY364_OFST)


#define EMPTY368REG (0x368)

#define EMPTY368_OFST (0)
#define EMPTY368_MSK  (0xffffffff << EMPTY368_OFST)


#define EMPTY36CREG (0x36C)

#define EMPTY36C_OFST (0)
#define EMPTY36C_MSK  (0xffffffff << EMPTY36C_OFST)


#define EMPTY370REG (0x370)

#define EMPTY370_OFST (0)
#define EMPTY370_MSK  (0xffffffff << EMPTY370_OFST)


#define EMPTY374REG (0x374)

#define EMPTY374_OFST (0)
#define EMPTY374_MSK  (0xffffffff << EMPTY374_OFST)


#define EMPTY378REG (0x378)

#define EMPTY378_OFST (0)
#define EMPTY378_MSK  (0xffffffff << EMPTY378_OFST)
