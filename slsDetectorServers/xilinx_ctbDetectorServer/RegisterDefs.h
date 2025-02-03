// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#define CTRL_REG (0x0)

#define POWER_VIO_OFST   (0)
#define POWER_VIO_MSK    (0x00000001 << POWER_VIO_OFST)
#define POWER_VCC_A_OFST (1)
#define POWER_VCC_A_MSK  (0x00000001 << POWER_VCC_A_OFST)
#define POWER_VCC_B_OFST (2)
#define POWER_VCC_B_MSK  (0x00000001 << POWER_VCC_B_OFST)
#define POWER_VCC_C_OFST (3)
#define POWER_VCC_C_MSK  (0x00000001 << POWER_VCC_C_OFST)
#define POWER_VCC_D_OFST (4)
#define POWER_VCC_D_MSK  (0x00000001 << POWER_VCC_D_OFST)

#define EMPTY4REG (0x4)

#define STATUS_REG (0x8)

#define PATTERN_RUNNING_OFST  (0)
#define PATTERN_RUNNING_MSK   (0x00000001 << PATTERN_RUNNING_OFST)
#define RX_BUSY_OFST          (1)
#define RX_BUSY_MSK           (0x00000001 << RX_BUSY_OFST)
#define PROCESSING_BUSY_OFST  (2)
#define PROCESSING_BUSY_MSK   (0x00000001 << PROCESSING_BUSY_OFST)
#define UDP_GEN_BUSY_OFST     (3)
#define UDP_GEN_BUSY_MSK      (0x00000001 << UDP_GEN_BUSY_OFST)
#define NETWORK_BUSY_OFST     (4)
#define NETWORK_BUSY_MSK      (0x00000001 << NETWORK_BUSY_OFST)
#define WAIT_FOR_TRIGGER_OFST (5)
#define WAIT_FOR_TRIGGER_MSK  (0x00000001 << WAIT_FOR_TRIGGER_OFST)
#define RX_NOT_GOOD_OFST      (6)
#define RX_NOT_GOOD_MSK       (0x00000001 << RX_NOT_GOOD_OFST)

#define STATUS_REG2 (0xC)

#define FPGAVERSIONREG (0x10)

#define FPGACOMPDATE_OFST (0)
#define FPGACOMPDATE_MSK  (0x00ffffff << FPGACOMPDATE_OFST)
#define FPGADETTYPE_OFST  (24)
#define FPGADETTYPE_MSK   (0x000000ff << FPGADETTYPE_OFST)

#define FPGA_GIT_HEAD (0x14)

#define FIXEDPATTERNREG (0x18)
#define FIXEDPATTERNVAL (0xACDC2016)

#define EMPTY1CREG (0x1C)

#define APIVERSIONREG (0x20)

#define APICOMPDATE_OFST (0)
#define APICOMPDATE_MSK  (0x00ffffff << APICOMPDATE_OFST)
#define APIDETTYPE_OFST  (24)
#define APIDETTYPE_MSK   (0x000000ff << APIDETTYPE_OFST)

#define EMPTY24REG (0x24)

#define PKTPACKETLENGTHREG (0x28)

#define PACKETLENGTH1G_OFST  (0)
#define PACKETLENGTH1G_MSK   (0x0000ffff << PACKETLENGTH1G_OFST)
#define PACKETLENGTH10G_OFST (16)
#define PACKETLENGTH10G_MSK  (0x0000ffff << PACKETLENGTH10G_OFST)

#define EMPTY2CREG (0x2C)

#define PKTNOPACKETSREG (0x30)

#define NOPACKETS1G_OFST  (0)
#define NOPACKETS1G_MSK   (0x0000003f << NOPACKETS1G_OFST)
#define NOPACKETS10G_OFST (16)
#define NOPACKETS10G_MSK  (0x0000003f << NOPACKETS10G_OFST)

#define EMPTY34REG (0x34)

#define PKTCTRLREG (0x38)

#define NOSERVERS_OFST   (0)
#define NOSERVERS_MSK    (0x0000003f << NOSERVERS_OFST)
#define SERVERSTART_OFST (8)
#define SERVERSTART_MSK  (0x0000001f << SERVERSTART_OFST)
#define ETHINTERF_OFST   (16)
#define ETHINTERF_MSK    (0x00000001 << ETHINTERF_OFST)

#define EMPTY3CREG (0x3C)

#define PKTCOORDREG1 (0x40)

#define COORDX_OFST (0)
#define COORDX_MSK  (0x0000ffff << COORDX_OFST)
#define COORDY_OFST (16)
#define COORDY_MSK  (0x0000ffff << COORDY_OFST)

#define EMPTY44REG (0x44)

#define PKTCOORDREG2 (0x48)

#define COORDZ_OFST (0)
#define COORDZ_MSK  (0x0000ffff << COORDZ_OFST)

#define EMPTY4CREG (0x4C)

#define EMPTY50REG (0x50)

#define EMPTY54REG (0x54)

#define EMPTY58REG (0x58)

#define EMPTY5CREG (0x5C)

#define EMPTY60REG (0x60)

#define EMPTY64REG (0x64)

#define EMPTY68REG (0x68)

#define EMPTY6CREG (0x6C)

#define EMPTY70REG (0x70)

#define EMPTY74REG (0x74)

#define EMPTY78REG (0x78)

#define EMPTY7CREG (0x7C)

#define EMPTY80REG (0x80)

#define EMPTY84REG (0x84)

#define EMPTY88REG (0x88)

#define EMPTY8CREG (0x8C)

#define EMPTY90REG (0x90)

#define EMPTY94REG (0x94)

#define EMPTY98REG (0x98)

#define EMPTY9CREG (0x9C)

#define FLOW_STATUS_REG (0x100)

#define RSM_BUSY_OFST     (0)
#define RSM_BUSY_MSK      (0x00000001 << RSM_BUSY_OFST)
#define RSM_TRG_WAIT_OFST (3)
#define RSM_TRG_WAIT_MSK  (0x00000001 << RSM_TRG_WAIT_OFST)
#define CSM_BUSY_OFST     (17)
#define CSM_BUSY_MSK      (0x00000001 << CSM_BUSY_OFST)

#define EMPTY104REG (0x104)

#define FLOW_CONTROL_REG (0x108)

#define START_F_OFST        (0)
#define START_F_MSK         (0x00000001 << START_F_OFST)
#define STOP_F_OFST         (1)
#define STOP_F_MSK          (0x00000001 << STOP_F_OFST)
#define RST_F_OFST          (2)
#define RST_F_MSK           (0x00000001 << RST_F_OFST)
#define SW_TRIGGER_F_OFST   (3)
#define SW_TRIGGER_F_MSK    (0x00000001 << SW_TRIGGER_F_OFST)
#define TRIGGER_ENABLE_OFST (4)
#define TRIGGER_ENABLE_MSK  (0x00000001 << TRIGGER_ENABLE_OFST)

#define EMPTY10CREG (0x10C)

#define TIME_FROM_START_OUT_REG_1 (0x110)

#define TIME_FROM_START_OUT_REG_2 (0x114)

#define FRAMES_FROM_START_OUT_REG_1 (0x118)

#define FRAMES_FROM_START_OUT_REG_2 (0x11C)

#define FRAME_TIME_OUT_REG_1 (0x120)

#define FRAME_TIME_OUT_REG_2 (0x124)

#define DELAY_OUT_REG_1 (0x128)

#define DELAY_OUT_REG_2 (0x12C)

#define CYCLES_OUT_REG_1 (0x130)

#define CYCLES_OUT_REG_2 (0x134)

#define FRAMES_OUT_REG_1 (0x138)

#define FRAMES_OUT_REG_2 (0x13C)

#define PERIOD_OUT_REG_1 (0x140)

#define PERIOD_OUT_REG_2 (0x144)

#define DELAY_IN_REG_1 (0x148)

#define DELAY_IN_REG_2 (0x14C)

#define CYCLES_IN_REG_1 (0x150)

#define CYCLES_IN_REG_2 (0x154)

#define FRAMES_IN_REG_1 (0x158)

#define FRAMES_IN_REG_2 (0x15C)

#define PERIOD_IN_REG_1 (0x160)

#define PERIOD_IN_REG_2 (0x164)

#define EMPTY168REG (0x168)

#define EMPTY16CREG (0x16C)

#define EMPTY170REG (0x170)

#define EMPTY174REG (0x174)

#define EMPTY178REG (0x178)

#define EMPTY17CREG (0x17C)

#define EMPTY180REG (0x180)

#define EMPTY184REG (0x184)

#define EMPTY188REG (0x188)

#define EMPTY18CREG (0x18C)

#define EMPTY190REG (0x190)

#define EMPTY194REG (0x194)

#define EMPTY198REG (0x198)

#define EMPTY19CREG (0x19C)

#define PATTERN_OUT_LSB_REG (0x200)

#define PATTERN_OUT_MSB_REG (0x204)

#define PATTERN_IN_LSB_REG (0x208)

#define PATTERN_IN_MSB_REG (0x20C)

#define PATTERN_MASK_LSB_REG (0x210)

#define PATTERN_MASK_MSB_REG (0x214)

#define PATTERN_SET_LSB_REG (0x218)

#define PATTERN_SET_MSB_REG (0x21C)

#define PATTERN_CNTRL_REG (0x220)

#define PATTERN_CNTRL_WR_OFST   (0)
#define PATTERN_CNTRL_WR_MSK    (0x00000001 << PATTERN_CNTRL_WR_OFST)
#define PATTERN_CNTRL_RD_OFST   (1)
#define PATTERN_CNTRL_RD_MSK    (0x00000001 << PATTERN_CNTRL_RD_OFST)
#define PATTERN_CNTRL_ADDR_OFST (16)
#define PATTERN_CNTRL_ADDR_MSK  (0x00001fff << PATTERN_CNTRL_ADDR_OFST)

#define EMPTY224REG (0x224)

#define PATTERN_LIMIT_REG (0x228)

#define PATTERN_LIMIT_STRT_OFST (0)
#define PATTERN_LIMIT_STRT_MSK  (0x00001fff << PATTERN_LIMIT_STRT_OFST)
#define PATTERN_LIMIT_STP_OFST  (16)
#define PATTERN_LIMIT_STP_MSK   (0x00001fff << PATTERN_LIMIT_STP_OFST)

#define EMPTY22CREG (0x22C)

#define PATTERN_LOOP_0_ADDR_REG (0x230)

#define PATTERN_LOOP_0_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_0_ADDR_STRT_MSK                                           \
    (0x00001fff << PATTERN_LOOP_0_ADDR_STRT_OFST)
#define PATTERN_LOOP_0_ADDR_STP_OFST (16)
#define PATTERN_LOOP_0_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_0_ADDR_STP_OFST)

#define EMPTY234REG (0x234)

#define PATTERN_LOOP_0_ITERATION_REG (0x238)

#define EMPTY23CREG (0x23C)

#define PATTERN_WAIT_0_ADDR_REG (0x240)

#define PATTERN_WAIT_0_ADDR_OFST (0)
#define PATTERN_WAIT_0_ADDR_MSK  (0x00001fff << PATTERN_WAIT_0_ADDR_OFST)

#define EMPTY244REG (0x244)

#define PATTERN_WAIT_TIMER_0_LSB_REG (0x248)

#define PATTERN_WAIT_TIMER_0_MSB_REG (0x24C)

#define PATTERN_LOOP_1_ADDR_REG (0x250)

#define PATTERN_LOOP_1_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_1_ADDR_STRT_MSK                                           \
    (0x00001fff << PATTERN_LOOP_1_ADDR_STRT_OFST)
#define PATTERN_LOOP_1_ADDR_STP_OFST (16)
#define PATTERN_LOOP_1_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_1_ADDR_STP_OFST)

#define EMPTY254REG (0x254)

#define PATTERN_LOOP_1_ITERATION_REG (0x258)

#define EMPTY25CREG (0x25C)

#define PATTERN_WAIT_1_ADDR_REG (0x260)

#define PATTERN_WAIT_1_ADDR_OFST (0)
#define PATTERN_WAIT_1_ADDR_MSK  (0x00001fff << PATTERN_WAIT_1_ADDR_OFST)

#define EMPTY264REG (0x264)

#define PATTERN_WAIT_TIMER_1_LSB_REG (0x268)

#define PATTERN_WAIT_TIMER_1_MSB_REG (0x26C)

#define PATTERN_LOOP_2_ADDR_REG (0x270)

#define PATTERN_LOOP_2_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_2_ADDR_STRT_MSK                                           \
    (0x00001fff << PATTERN_LOOP_2_ADDR_STRT_OFST)
#define PATTERN_LOOP_2_ADDR_STP_OFST (16)
#define PATTERN_LOOP_2_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_2_ADDR_STP_OFST)

#define EMPTY274REG (0x274)

#define PATTERN_LOOP_2_ITERATION_REG (0x278)

#define EMPTY27CREG (0x27C)

#define PATTERN_WAIT_2_ADDR_REG (0x280)

#define PATTERN_WAIT_2_ADDR_OFST (0)
#define PATTERN_WAIT_2_ADDR_MSK  (0x00001fff << PATTERN_WAIT_2_ADDR_OFST)

#define EMPTY284REG (0x284)

#define PATTERN_WAIT_TIMER_2_LSB_REG (0x288)

#define PATTERN_WAIT_TIMER_2_MSB_REG (0x28C)

#define PATTERN_LOOP_3_ADDR_REG (0x290)

#define PATTERN_LOOP_3_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_3_ADDR_STRT_MSK                                           \
    (0x00001fff << PATTERN_LOOP_3_ADDR_STRT_OFST)
#define PATTERN_LOOP_3_ADDR_STP_OFST (16)
#define PATTERN_LOOP_3_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_3_ADDR_STP_OFST)

#define EMPTY294REG (0x294)

#define PATTERN_LOOP_3_ITERATION_REG (0x298)

#define EMPTY29CREG (0x29C)

#define PATTERN_WAIT_3_ADDR_REG (0x300)

#define PATTERN_WAIT_3_ADDR_OFST (0)
#define PATTERN_WAIT_3_ADDR_MSK  (0x00001fff << PATTERN_WAIT_3_ADDR_OFST)

#define EMPTY304REG (0x304)

#define PATTERN_WAIT_TIMER_3_LSB_REG (0x308)

#define PATTERN_WAIT_TIMER_3_MSB_REG (0x30C)

#define PATTERN_LOOP_4_ADDR_REG (0x310)

#define PATTERN_LOOP_4_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_4_ADDR_STRT_MSK                                           \
    (0x00001fff << PATTERN_LOOP_4_ADDR_STRT_OFST)
#define PATTERN_LOOP_4_ADDR_STP_OFST (16)
#define PATTERN_LOOP_4_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_4_ADDR_STP_OFST)

#define EMPTY314REG (0x314)

#define PATTERN_LOOP_4_ITERATION_REG (0x318)

#define EMPTY31CREG (0x31C)

#define PATTERN_WAIT_4_ADDR_REG (0x320)

#define PATTERN_WAIT_4_ADDR_OFST (0)
#define PATTERN_WAIT_4_ADDR_MSK  (0x00001fff << PATTERN_WAIT_4_ADDR_OFST)

#define EMPTY324REG (0x324)

#define PATTERN_WAIT_TIMER_4_LSB_REG (0x328)

#define PATTERN_WAIT_TIMER_4_MSB_REG (0x32C)

#define PATTERN_LOOP_5_ADDR_REG (0x330)

#define PATTERN_LOOP_5_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_5_ADDR_STRT_MSK                                           \
    (0x00001fff << PATTERN_LOOP_5_ADDR_STRT_OFST)
#define PATTERN_LOOP_5_ADDR_STP_OFST (16)
#define PATTERN_LOOP_5_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_5_ADDR_STP_OFST)

#define EMPTY334REG (0x334)

#define PATTERN_LOOP_5_ITERATION_REG (0x338)

#define EMPTY33CREG (0x33C)

#define PATTERN_WAIT_5_ADDR_REG (0x340)

#define PATTERN_WAIT_5_ADDR_OFST (0)
#define PATTERN_WAIT_5_ADDR_MSK  (0x00001fff << PATTERN_WAIT_5_ADDR_OFST)

#define EMPTY344REG (0x344)

#define PATTERN_WAIT_TIMER_5_LSB_REG (0x348)

#define PATTERN_WAIT_TIMER_5_MSB_REG (0x34C)

#define PINIOCTRLREG (0x350)

#define EMPTY354REG (0x354)

#define EMPTY358REG (0x358)

#define EMPTY35CREG (0x35C)

#define EMPTY360REG (0x360)

#define EMPTY364REG (0x364)

#define EMPTY368REG (0x368)

#define EMPTY36CREG (0x36C)

#define EMPTY370REG (0x370)

#define EMPTY374REG (0x374)

#define EMPTY378REG (0x378)

#define EMPTY37CREG (0x37C)

#define EMPTY380REG (0x380)

#define EMPTY384REG (0x384)

#define EMPTY388REG (0x388)

#define EMPTY38CREG (0x38C)

#define EMPTY390REG (0x390)

#define EMPTY394REG (0x394)

#define EMPTY398REG (0x398)

#define EMPTY39CREG (0x39C)

#define EMPTY3A0REG (0x3A0)

#define EMPTY3A4REG (0x3A4)

#define EMPTY3A8REG (0x3A8)

#define EMPTY3ACREG (0x3AC)

#define EMPTY3B0REG (0x3B0)

#define EMPTY3B4REG (0x3B4)

#define EMPTY3B8REG (0x3B8)

#define EMPTY3BCREG (0x3BC)

#define EMPTY3C0REG (0x3C0)

#define EMPTY3C4REG (0x3C4)

#define EMPTY3C8REG (0x3C8)

#define EMPTY3CCREG (0x3CC)

#define EMPTY3D0REG (0x3D0)

#define EMPTY3D4REG (0x3D4)

#define EMPTY3D8REG (0x3D8)

#define EMPTY3DCREG (0x3DC)

#define EMPTY3E0REG (0x3E0)

#define EMPTY3E4REG (0x3E4)

#define EMPTY3E8REG (0x3E8)

#define EMPTY3ECREG (0x3EC)

#define EMPTY3F0REG (0x3F0)

#define EMPTY3F4REG (0x3F4)

#define EMPTY3F8REG (0x3F8)

#define EMPTY3FCREG (0x3FC)

#define EMPTY400REG (0x400)

#define EMPTY404REG (0x404)

#define EMPTY408REG (0x408)

#define EMPTY40CREG (0x40C)

#define EMPTY410REG (0x410)

#define EMPTY414REG (0x414)

#define EMPTY418REG (0x418)

#define EMPTY41CREG (0x41C)

#define EMPTY420REG (0x420)

#define EMPTY424REG (0x424)

#define EMPTY428REG (0x428)

#define EMPTY42CREG (0x42C)

#define EMPTY430REG (0x430)

#define EMPTY434REG (0x434)

#define EMPTY438REG (0x438)

#define EMPTY43CREG (0x43C)

#define EMPTY440REG (0x440)

#define EMPTY444REG (0x444)

#define EMPTY448REG (0x448)

#define EMPTY44CREG (0x44C)

#define EMPTY450REG (0x450)

#define EMPTY454REG (0x454)

#define EMPTY458REG (0x458)

#define EMPTY45CREG (0x45C)

#define EMPTY460REG (0x460)

#define EMPTY464REG (0x464)

#define EMPTY468REG (0x468)

#define EMPTY46CREG (0x46C)

#define EMPTY470REG (0x470)

#define EMPTY474REG (0x474)

#define EMPTY478REG (0x478)

#define EMPTY47CREG (0x47C)

#define EMPTY480REG (0x480)

#define EMPTY484REG (0x484)

#define EMPTY488REG (0x488)

#define EMPTY48CREG (0x48C)

#define EMPTY490REG (0x490)

#define EMPTY494REG (0x494)

#define EMPTY498REG (0x498)

#define EMPTY49CREG (0x49C)

#define EMPTY4A0REG (0x4A0)

#define EMPTY4A4REG (0x4A4)

#define EMPTY4A8REG (0x4A8)

#define EMPTY4ACREG (0x4AC)

#define EMPTY4B0REG (0x4B0)

#define EMPTY4B4REG (0x4B4)

#define EMPTY4B8REG (0x4B8)

#define EMPTY4BCREG (0x4BC)

#define EMPTY4C0REG (0x4C0)

#define EMPTY4C4REG (0x4C4)

#define EMPTY4C8REG (0x4C8)

#define EMPTY4CCREG (0x4CC)

#define EMPTY4D0REG (0x4D0)

#define EMPTY4D4REG (0x4D4)

#define EMPTY4D8REG (0x4D8)

#define EMPTY4DCREG (0x4DC)

#define EMPTY4E0REG (0x4E0)

#define EMPTY4E4REG (0x4E4)

#define EMPTY4E8REG (0x4E8)

#define EMPTY4ECREG (0x4EC)

#define EMPTY4F0REG (0x4F0)

#define EMPTY4F4REG (0x4F4)

#define EMPTY4F8REG (0x4F8)

#define EMPTY4FCREG (0x4FC)

#define FIFO_TO_GB_CONTROL_REG (0x500)

#define ENABLED_CHANNELS_ADC_OFST     (0)
#define ENABLED_CHANNELS_ADC_MSK      (0x000000ff << ENABLED_CHANNELS_ADC_OFST)
#define ENABLED_CHANNELS_D_OFST       (8)
#define ENABLED_CHANNELS_D_MSK        (0x00000001 << ENABLED_CHANNELS_D_OFST)
#define ENABLED_CHANNELS_X_OFST       (9)
#define ENABLED_CHANNELS_X_MSK        (0x0000000f << ENABLED_CHANNELS_X_OFST)
#define RO_MODE_ADC_OFST              (13)
#define RO_MODE_ADC_MSK               (0x00000001 << RO_MODE_ADC_OFST)
#define RO_MODE_D_OFST                (14)
#define RO_MODE_D_MSK                 (0x00000001 << RO_MODE_D_OFST)
#define RO_MODE_X_OFST                (15)
#define RO_MODE_X_MSK                 (0x00000001 << RO_MODE_X_OFST)
#define COUNT_FRAMES_FROM_UPDATE_OFST (16)
#define COUNT_FRAMES_FROM_UPDATE_MSK                                           \
    (0x00000001 << COUNT_FRAMES_FROM_UPDATE_OFST)
#define START_STREAMING_P_OFST (17)
#define START_STREAMING_P_MSK  (0x00000001 << START_STREAMING_P_OFST)

#define EMPTY504REG (0x504)

#define NO_SAMPLES_D_REG (0x508)

#define NO_SAMPLES_D_OFST (0)
#define NO_SAMPLES_D_MSK  (0x00003fff << NO_SAMPLES_D_OFST)

#define EMPTY50CREG (0x50C)

#define NO_SAMPLES_A_REG (0x510)

#define NO_SAMPLES_A_OFST (0)
#define NO_SAMPLES_A_MSK  (0x00003fff << NO_SAMPLES_A_OFST)

#define EMPTY514REG (0x514)

#define NO_SAMPLES_X_REG (0x518)

#define NO_SAMPLES_X_OFST (0)
#define NO_SAMPLES_X_MSK  (0x00001fff << NO_SAMPLES_X_OFST)

#define EMPTY51CREG (0x51C)

#define COUNT_FRAMES_FROM_REG_1 (0x520)

#define COUNT_FRAMES_FROM_REG_2 (0x524)

#define LOCAL_FRAME_NUMBER_REG_1 (0x528)

#define LOCAL_FRAME_NUMBER_REG_2 (0x52C)

#define EMPTY530REG (0x530)

#define EMPTY534REG (0x534)

#define EMPTY538REG (0x538)

#define EMPTY53CREG (0x53C)

#define EMPTY540REG (0x540)

#define EMPTY544REG (0x544)

#define EMPTY548REG (0x548)

#define EMPTY54CREG (0x54C)

#define EMPTY550REG (0x550)

#define EMPTY554REG (0x554)

#define EMPTY558REG (0x558)

#define EMPTY55CREG (0x55C)

#define EMPTY560REG (0x560)

#define EMPTY564REG (0x564)

#define EMPTY568REG (0x568)

#define EMPTY56CREG (0x56C)

#define EMPTY570REG (0x570)

#define EMPTY574REG (0x574)

#define EMPTY578REG (0x578)

#define EMPTY57CREG (0x57C)

#define A_FIFO_OVERFLOW_STATUS_REG (0x580)

#define EMPTY584REG (0x584)

#define A_FIFO_EMPTY_STATUS_REG (0x588)

#define EMPTY58CREG (0x58C)

#define A_FIFO_FULL_STATUS_REG (0x590)

#define EMPTY594REG (0x594)

#define D_FIFO_OVERFLOW_STATUS_REG (0x598)

#define D_FIFO_OVERFLOW_STATUS_OFST (0)
#define D_FIFO_OVERFLOW_STATUS_MSK  (0x00000001 << D_FIFO_OVERFLOW_STATUS_OFST)

#define EMPTY59CREG (0x59C)

#define D_FIFO_EMPTY_STATUS_REG (0x5A0)

#define D_FIFO_EMPTY_STATUS_OFST (0)
#define D_FIFO_EMPTY_STATUS_MSK  (0x00000001 << D_FIFO_EMPTY_STATUS_OFST)

#define EMPTY5A4REG (0x5A4)

#define D_FIFO_FULL_STATUS_REG (0x5A8)

#define D_FIFO_FULL_STATUS_OFST (0)
#define D_FIFO_FULL_STATUS_MSK  (0x00000001 << D_FIFO_FULL_STATUS_OFST)

#define EMPTY5ACREG (0x5AC)

#define X_FIFO_OVERFLOW_STATUS_REG (0x5B0)

#define X_FIFO_OVERFLOW_STATUS_OFST (0)
#define X_FIFO_OVERFLOW_STATUS_MSK  (0x0000000f << X_FIFO_OVERFLOW_STATUS_OFST)

#define EMPTY5B4REG (0x5B4)

#define X_FIFO_EMPTY_STATUS_REG (0x5B8)

#define X_FIFO_EMPTY_STATUS_OFST (0)
#define X_FIFO_EMPTY_STATUS_MSK  (0x0000000f << X_FIFO_EMPTY_STATUS_OFST)

#define EMPTY5BCREG (0x5BC)

#define X_FIFO_FULL_STATUS_REG (0x5C0)

#define X_FIFO_FULL_STATUS_OFST (0)
#define X_FIFO_FULL_STATUS_MSK  (0x0000000f << X_FIFO_FULL_STATUS_OFST)

#define EMPTY5C4REG (0x5C4)

#define A_FIFO_CLEAN_REG (0x5C8)

#define EMPTY5CCREG (0x5CC)

#define D_FIFO_CLEAN_REG (0x5D0)

#define D_FIFO_CLEAN_OFST (0)
#define D_FIFO_CLEAN_MSK  (0x00000001 << D_FIFO_CLEAN_OFST)

#define EMPTY5D4REG (0x5D4)

#define X_FIFO_CLEAN_REG (0x5D8)

#define X_FIFO_CLEAN_OFST (0)
#define X_FIFO_CLEAN_MSK  (0x0000000f << X_FIFO_CLEAN_OFST)

#define EMPTY5DCREG (0x5DC)

#define EMPTY5E0REG (0x5E0)

#define EMPTY5E4REG (0x5E4)

#define EMPTY5E8REG (0x5E8)

#define EMPTY5ECREG (0x5EC)

#define EMPTY5F0REG (0x5F0)

#define EMPTY5F4REG (0x5F4)

#define EMPTY5F8REG (0x5F8)

#define EMPTY5FCREG (0x5FC)

#define MATTERHORNSPIREG1 (0x600)

#define MATTERHORNSPIREG2 (0x604)

#define MATTERHORNSPICTRL (0x608)

#define CONFIGSTART_P_OFST   (0)
#define CONFIGSTART_P_MSK    (0x00000001 << CONFIGSTART_P_OFST)
#define PERIPHERYRST_P_OFST  (1)
#define PERIPHERYRST_P_MSK   (0x00000001 << PERIPHERYRST_P_OFST)
#define STARTREAD_P_OFST     (2)
#define STARTREAD_P_MSK      (0x00000001 << STARTREAD_P_OFST)
#define BUSY_OFST            (3)
#define BUSY_MSK             (0x00000001 << BUSY_OFST)
#define READOUTFROMASIC_OFST (4)
#define READOUTFROMASIC_MSK  (0x00000001 << READOUTFROMASIC_OFST)

#define EMPTY60CREG (0x60C)

#define EMPTY610REG (0x610)

#define EMPTY614REG (0x614)

#define EMPTY618REG (0x618)

#define EMPTY61CREG (0x61C)

#define EMPTY620REG (0x620)

#define EMPTY624REG (0x624)

#define EMPTY628REG (0x628)

#define EMPTY62CREG (0x62C)

#define TRANSCEIVERRXCTRL0REG1 (0x630)

#define TRANSCEIVERRXCTRL0REG2 (0x634)

#define TRANSCEIVERRXCTRL1REG1 (0x638)

#define TRANSCEIVERRXCTRL1REG2 (0x63C)

#define TRANSCEIVERRXCTRL2REG (0x640)

#define EMPTY644REG (0x644)

#define TRANSCEIVERRXCTRL3REG (0x648)

#define EMPTY64CREG (0x64C)

#define TRANSCEIVERSTATUS (0x650)

#define LINKDOWNLATCHEDOUT_OFST (0)
#define LINKDOWNLATCHEDOUT_MSK  (0x00000001 << LINKDOWNLATCHEDOUT_OFST)
#define TXUSERCLKACTIVE_OFST    (1)
#define TXUSERCLKACTIVE_MSK     (0x00000001 << TXUSERCLKACTIVE_OFST)
#define RXUSERCLKACTIVE_OFST    (2)
#define RXUSERCLKACTIVE_MSK     (0x00000001 << RXUSERCLKACTIVE_OFST)
#define RXCOMMADET_OFST         (3)
#define RXCOMMADET_MSK          (0x0000000f << RXCOMMADET_OFST)
#define RXBYTEREALIGN_OFST      (7)
#define RXBYTEREALIGN_MSK       (0x0000000f << RXBYTEREALIGN_OFST)
#define RXBYTEISALIGNED_OFST    (11)
#define RXBYTEISALIGNED_MSK     (0x0000000f << RXBYTEISALIGNED_OFST)
#define GTWIZRXCDRSTABLE_OFST   (15)
#define GTWIZRXCDRSTABLE_MSK    (0x00000001 << GTWIZRXCDRSTABLE_OFST)
#define RESETTXDONE_OFST        (16)
#define RESETTXDONE_MSK         (0x00000001 << RESETTXDONE_OFST)
#define RESETRXDONE_OFST        (17)
#define RESETRXDONE_MSK         (0x00000001 << RESETRXDONE_OFST)
#define RXPMARESETDONE_OFST     (18)
#define RXPMARESETDONE_MSK      (0x0000000f << RXPMARESETDONE_OFST)
#define TXPMARESETDONE_OFST     (22)
#define TXPMARESETDONE_MSK      (0x0000000f << TXPMARESETDONE_OFST)
#define GTTPOWERGOOD_OFST       (26)
#define GTTPOWERGOOD_MSK        (0x0000000f << GTTPOWERGOOD_OFST)

#define TRANSCEIVERSTATUS2 (0x654)

#define RXLOCKED_OFST (0)
#define RXLOCKED_MSK  (0x0000000f << RXLOCKED_OFST)

#define TRANSCEIVERCONTROL (0x658)

#define GTWIZRESETALL_OFST         (0)
#define GTWIZRESETALL_MSK          (0x00000001 << GTWIZRESETALL_OFST)
#define RESETTXPLLANDDATAPATH_OFST (1)
#define RESETTXPLLANDDATAPATH_MSK  (0x00000001 << RESETTXPLLANDDATAPATH_OFST)
#define RESETTXDATAPATHIN_OFST     (2)
#define RESETTXDATAPATHIN_MSK      (0x00000001 << RESETTXDATAPATHIN_OFST)
#define RESETRXPLLANDDATAPATH_OFST (3)
#define RESETRXPLLANDDATAPATH_MSK  (0x00000001 << RESETRXPLLANDDATAPATH_OFST)
#define RESETRXDATAPATHIN_OFST     (4)
#define RESETRXDATAPATHIN_MSK      (0x00000001 << RESETRXDATAPATHIN_OFST)
#define RXPOLARITY_OFST            (5)
#define RXPOLARITY_MSK             (0x0000000f << RXPOLARITY_OFST)
#define RXERRORCNTRESET_OFST       (9)
#define RXERRORCNTRESET_MSK        (0x0000000f << RXERRORCNTRESET_OFST)
#define RXMSBLSBINVERT_OFST        (13)
#define RXMSBLSBINVERT_MSK         (0x0000000f << RXMSBLSBINVERT_OFST)

#define TRANSCEIVERERRCNT_REG0 (0x65C)

#define TRANSCEIVERERRCNT_REG1 (0x660)

#define TRANSCEIVERERRCNT_REG2 (0x664)

#define TRANSCEIVERERRCNT_REG3 (0x668)

#define TRANSCEIVERALIGNCNT_REG0 (0x66C)

#define RXALIGNCNTCH0_OFST (0)
#define RXALIGNCNTCH0_MSK  (0x0000ffff << RXALIGNCNTCH0_OFST)

#define TRANSCEIVERALIGNCNT_REG1 (0x670)

#define RXALIGNCNTCH1_OFST (0)
#define RXALIGNCNTCH1_MSK  (0x0000ffff << RXALIGNCNTCH1_OFST)

#define TRANSCEIVERALIGNCNT_REG2 (0x674)

#define RXALIGNCNTCH2_OFST (0)
#define RXALIGNCNTCH2_MSK  (0x0000ffff << RXALIGNCNTCH2_OFST)

#define TRANSCEIVERALIGNCNT_REG3 (0x678)

#define RXALIGNCNTCH3_OFST (0)
#define RXALIGNCNTCH3_MSK  (0x0000ffff << RXALIGNCNTCH3_OFST)

#define TRANSCEIVERLASTWORD_REG0 (0x67C)

#define RXDATACH0_OFST (0)
#define RXDATACH0_MSK  (0x0000ffff << RXDATACH0_OFST)

#define TRANSCEIVERLASTWORD_REG1 (0x680)

#define RXDATACH1_OFST (0)
#define RXDATACH1_MSK  (0x0000ffff << RXDATACH1_OFST)

#define TRANSCEIVERLASTWORD_REG2 (0x684)

#define RXDATACH2_OFST (0)
#define RXDATACH2_MSK  (0x0000ffff << RXDATACH2_OFST)

#define TRANSCEIVERLASTWORD_REG3 (0x688)

#define RXDATACH3_OFST (0)
#define RXDATACH3_MSK  (0x0000ffff << RXDATACH3_OFST)

#define EMPTY68CREG (0x68C)

#define EMPTY690REG (0x690)

#define EMPTY694REG (0x694)

#define EMPTY698REG (0x698)

#define EMPTY69CREG (0x69C)

#define EMPTY6A0REG (0x6A0)

#define EMPTY6A4REG (0x6A4)

#define EMPTY6A8REG (0x6A8)

#define EMPTY6ACREG (0x6AC)

#define EMPTY6B0REG (0x6B0)

#define EMPTY6B4REG (0x6B4)

#define EMPTY6B8REG (0x6B8)

#define EMPTY6BCREG (0x6BC)

#define EMPTY6C0REG (0x6C0)

#define EMPTY6C4REG (0x6C4)

#define EMPTY6C8REG (0x6C8)

#define EMPTY6CCREG (0x6CC)

#define EMPTY6D0REG (0x6D0)

#define EMPTY6D4REG (0x6D4)

#define EMPTY6D8REG (0x6D8)

#define EMPTY6DCREG (0x6DC)

#define EMPTY6E0REG (0x6E0)

#define EMPTY6E4REG (0x6E4)

#define EMPTY6E8REG (0x6E8)

#define EMPTY6ECREG (0x6EC)

#define EMPTY6F0REG (0x6F0)

#define EMPTY6F4REG (0x6F4)

#define EMPTY6F8REG (0x6F8)

#define EMPTY6FCREG (0x6FC)

#define DBITFIFOCTRLREG (0x700)

#define DBITRD_OFST        (0)
#define DBITRD_MSK         (0x00000001 << DBITRD_OFST)
#define DBITRST_OFST       (1)
#define DBITRST_MSK        (0x00000001 << DBITRST_OFST)
#define DBITFULL_OFST      (2)
#define DBITFULL_MSK       (0x00000001 << DBITFULL_OFST)
#define DBITEMPTY_OFST     (3)
#define DBITEMPTY_MSK      (0x00000001 << DBITEMPTY_OFST)
#define DBITUNDERFLOW_OFST (4)
#define DBITUNDERFLOW_MSK  (0x00000001 << DBITUNDERFLOW_OFST)
#define DBITOVERFLOW_OFST  (5)
#define DBITOVERFLOW_MSK   (0x00000001 << DBITOVERFLOW_OFST)

#define EMPTYREG (0x704)

#define DBITFIFODATAREG1 (0x708)

#define DBITFIFODATAREG2 (0x70C)

#define EMPTY710REG (0x710)

#define EMPTY714REG (0x714)

#define EMPTY718REG (0x718)

#define EMPTY71CREG (0x71C)

#define EMPTY720REG (0x720)
