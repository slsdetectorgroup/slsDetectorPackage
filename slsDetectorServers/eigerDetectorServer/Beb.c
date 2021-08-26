#include "Beb.h"
#include "FebRegisterDefs.h"
#include "clogger.h"
#include "xparameters.h"

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

struct BebInfo beb_infos[10];
int bebInfoSize = 0;

struct LocalLinkInterface ll_beb_local, *ll_beb;

struct udp_header_type udp_header;

int Beb_send_ndata;
unsigned int Beb_send_buffer_size;
unsigned int *Beb_send_data_raw;
unsigned int *Beb_send_data;

int Beb_recv_ndata;
unsigned int Beb_recv_buffer_size;
unsigned int *Beb_recv_data_raw;
unsigned int *Beb_recv_data;

short Beb_bit_mode;
int BEB_MMAP_SIZE = 0x1000;

int Beb_activated = 1;

uint32_t Beb_detid = 0;
int Beb_top = 0;

uint64_t Beb_deactivatedNextFrameNumber = 0;
int Beb_quadEnable = 0;
int Beb_positions[2] = {0, 0};
int Beb_partialReadout = MAX_ROWS_PER_READOUT;
int Beb_deactivated_transmission_flowcontrol_10g = 0;
int Beb_deactivated_transmission_delay_frame = 0;
int Beb_deactivated_transmission_delay_left = 0;
int Beb_deactivated_transmission_delay_right = 0;
int Beb_deactivated_left_datastream = 1;
int Beb_deactivated_right_datastream = 1;
int Beb_deactivated_num_destinations = 1;

void BebInfo_BebInfo(struct BebInfo *bebInfo, unsigned int beb_num) {
    bebInfo->beb_number = beb_num;
    bebInfo->serial_address = 0;
    strcpy(bebInfo->src_mac_1GbE, "");
    strcpy(bebInfo->src_mac_10GbE, "");
    strcpy(bebInfo->src_ip_1GbE, "");
    strcpy(bebInfo->src_ip_10GbE, "");
    bebInfo->src_port_1GbE = bebInfo->src_port_10GbE = 0;
}

int BebInfo_SetSerialAddress(struct BebInfo *bebInfo, unsigned int a) {
    // address pre shifted
    if (a > 0xff)
        return 0;
    bebInfo->serial_address = 0x04000000 | ((a & 0xff) << 16);
    return 1;
}

int BebInfo_SetHeaderInfo(struct BebInfo *bebInfo, int ten_gig, char *src_mac,
                          char *src_ip, unsigned int src_port) {
    if (ten_gig) {
        strcpy(bebInfo->src_mac_10GbE, src_mac);
        strcpy(bebInfo->src_ip_10GbE, src_ip);
        bebInfo->src_port_10GbE = src_port;
    } else {
        strcpy(bebInfo->src_mac_1GbE, src_mac);
        strcpy(bebInfo->src_ip_1GbE, src_ip);
        bebInfo->src_port_1GbE = src_port;
    }
    return 1;
}

unsigned int BebInfo_GetBebNumber(struct BebInfo *bebInfo) {
    return bebInfo->beb_number;
}
unsigned int BebInfo_GetSerialAddress(struct BebInfo *bebInfo) {
    return bebInfo->serial_address;
}
char *BebInfo_GetSrcMAC(struct BebInfo *bebInfo, int ten_gig) {
    return ten_gig ? bebInfo->src_mac_10GbE : bebInfo->src_mac_1GbE;
}
char *BebInfo_GetSrcIP(struct BebInfo *bebInfo, int ten_gig) {
    return ten_gig ? bebInfo->src_ip_10GbE : bebInfo->src_ip_1GbE;
}
unsigned int BebInfo_GetSrcPort(struct BebInfo *bebInfo, int ten_gig) {
    return ten_gig ? bebInfo->src_port_10GbE : bebInfo->src_port_1GbE;
}

void BebInfo_Print(struct BebInfo *bebInfo) {
    LOG(logINFO,
        ("%d) Beb Info:\n"
         "\tSerial Add: 0x%x\n"
         "\tMAC   1GbE: %s\n"
         "\tIP    1GbE: %s\n"
         "\tPort  1GbE: %d\n"
         "\tMAC  10GbE: %s\n"
         "\tIP   10GbE: %s\n"
         "\tPort 10GbE: %d\n",
         bebInfo->beb_number, bebInfo->serial_address, bebInfo->src_mac_1GbE,
         bebInfo->src_ip_1GbE, bebInfo->src_port_1GbE, bebInfo->src_mac_10GbE,
         bebInfo->src_ip_10GbE, bebInfo->src_port_10GbE));
}

void Beb_Beb(int id) {
    Beb_detid = id;
    Beb_send_ndata = 0;
    Beb_send_buffer_size = 1026;
    Beb_send_data_raw =
        malloc((Beb_send_buffer_size + 1) * sizeof(unsigned int));
    Beb_send_data = &Beb_send_data_raw[1];

    Beb_recv_ndata = 0;
    Beb_recv_buffer_size = 1026;
    Beb_recv_data_raw =
        malloc((Beb_recv_buffer_size + 1) * sizeof(unsigned int));
    Beb_recv_data = &Beb_recv_data_raw[1];

    udp_header = (struct udp_header_type){
        {0x00, 0x50, 0xc5, 0xb2, 0xcb, 0x46}, // DST MAC
        {0x00, 0x50, 0xc2, 0x46, 0xd9, 0x02}, // SRC MAC
        {0x08, 0x00},
        {0x45},
        {0x00},
        {0x00, 0x00},
        {0x00, 0x00},
        {0x40},
        {0x00},
        {0xff},
        {0x11},
        {0x00, 0x00},
        {129, 205, 205, 128}, // Src IP
        {129, 205, 205, 122}, // Dst IP
        {0x0f, 0xa1},
        {0x13, 0x89},
        {0x00, 0x00}, //{0x00, 0x11},
        {0x00, 0x00}};

    if (!Beb_InitBebInfos())
        exit(1);

    LOG(logDEBUG1, ("Printing Beb infos:\n"));
    for (unsigned int i = 1; i < bebInfoSize; i++)
        BebInfo_Print(&beb_infos[i]);

    Beb_bit_mode = 4;

    //  ll_beb = &ll_beb_local;
    //  Local_LocalLinkInterface(ll_beb,XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_LEFT_BASEADDR);

    //  Beb_SetByteOrder();
}

void Beb_GetModuleConfiguration(int *master, int *top, int *normal) {
    *top = 0;
    *master = 0;
    // mapping new memory to read master top module configuration
    u_int32_t *csp0base = 0;
    int ret;
    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Module Configuration FAIL\n"));
    } else {
        // read data
        ret = Beb_Read32(csp0base, BEB_CONFIG_RD_OFST);
        LOG(logDEBUG1, ("Module Configuration OK\n"));
        LOG(logDEBUG1, ("Beb: value =0x%x\n", ret));
        if (ret & BEB_CONFIG_TOP_RD_MSK) {
            *top = 1;
        }
        if (ret & BEB_CONFIG_MASTER_RD_MSK)
            *master = 1;
        if (ret & BEB_CONFIG_NORMAL_RD_MSK)
            *normal = 1;
        // close file pointer
        Beb_close(fd, csp0base);
    }
}

int Beb_IsTransmitting(int *retval, int tengiga, int waitForDelay) {
    if (!Beb_activated) {
        // not transmitting
        *retval = 0;
        return OK;
    }
    // mapping new memory
    u_int32_t *csp0base = 0;
    int addr_l_txndelaycounter = 0, addr_l_framedelaycounter = 0;
    int addr_r_txndelaycounter = 0, addr_r_framedelaycounter = 0;
    int addr_l_framepktLsbcounter = 0, addr_l_framepktMsbcounter = 0;
    int addr_r_framepktLsbcounter = 0, addr_r_framepktMsbcounter = 0;
    if (tengiga) {
        addr_l_txndelaycounter = TEN_GIGA_LEFT_TXN_DELAY_COUNTER;
        addr_l_framedelaycounter = TEN_GIGA_LEFT_FRAME_DELAY_COUNTER;
        addr_r_txndelaycounter = TEN_GIGA_RIGHT_TXN_DELAY_COUNTER;
        addr_r_framedelaycounter = TEN_GIGA_RIGHT_FRAME_DELAY_COUNTER;
        addr_l_framepktLsbcounter = TEN_GIGA_LEFT_INDEX_LSB_COUNTER;
        addr_l_framepktMsbcounter = TEN_GIGA_LEFT_INDEX_MSB_COUNTER;
        addr_r_framepktLsbcounter = TEN_GIGA_RIGHT_INDEX_LSB_COUNTER;
        addr_r_framepktMsbcounter = TEN_GIGA_RIGHT_INDEX_MSB_COUNTER;
    } else {
        addr_l_txndelaycounter = ONE_GIGA_LEFT_TXN_DELAY_COUNTER;
        addr_l_framedelaycounter = ONE_GIGA_LEFT_FRAME_DELAY_COUNTER;
        addr_r_txndelaycounter = ONE_GIGA_RIGHT_TXN_DELAY_COUNTER;
        addr_r_framedelaycounter = ONE_GIGA_RIGHT_FRAME_DELAY_COUNTER;
        addr_l_framepktLsbcounter = ONE_GIGA_LEFT_INDEX_LSB_COUNTER;
        addr_l_framepktMsbcounter = ONE_GIGA_LEFT_INDEX_MSB_COUNTER;
        addr_r_framepktLsbcounter = ONE_GIGA_RIGHT_INDEX_LSB_COUNTER;
        addr_r_framepktMsbcounter = ONE_GIGA_RIGHT_INDEX_MSB_COUNTER;
    }

    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_COUNTER_BASEADDR);
    if (fd < 0) {
        cprintf(BG_RED, "Could not read Beb Delay read counter\n");
        return FAIL;
    } else {
        // read data first time
        int l_txndelaycounter = Beb_Read32(csp0base, addr_l_txndelaycounter);
        int l_framedelaycounter =
            Beb_Read32(csp0base, addr_l_framedelaycounter);
        int r_txndelaycounter = Beb_Read32(csp0base, addr_r_txndelaycounter);
        int r_framedelaycounter =
            Beb_Read32(csp0base, addr_r_framedelaycounter);
        int l_framepktLsbcounter =
            Beb_Read32(csp0base, addr_l_framepktLsbcounter);
        int l_framepktMsbcounter =
            Beb_Read32(csp0base, addr_l_framepktMsbcounter);
        int r_framepktLsbcounter =
            Beb_Read32(csp0base, addr_r_framepktLsbcounter);
        int r_framepktMsbcounter =
            Beb_Read32(csp0base, addr_r_framepktMsbcounter);
#ifdef VERBOSE
        printf("\nFirst Read:\n"
               "\tLeft [Txndelaycounter:%d, Framedelaycounter:%d]\n"
               "\tRight [Txndelaycounter:%d, Framedelaycounter:%d]\n",
               "\tLeft [FramepacketLsbcounter:%d, FramepacketMsbcounter:%d]\n"
               "\tRight [FramepacketLsbcounter:%d, FramepacketMsbcounter:%d]\n",
               l_txndelaycounter, l_framedelaycounter, r_txndelaycounter,
               r_framedelaycounter, l_framepktLsbcounter, l_framepktMsbcounter,
               r_framepktLsbcounter, r_framepktMsbcounter);
#endif
        // wait for max counter delay
        if (waitForDelay) {
            int maxtimer = (MAX(MAX(l_txndelaycounter, l_framedelaycounter),
                                MAX(r_txndelaycounter, r_framedelaycounter))) /
                           100; // counter values in 10 ns
            printf("Beb: Will wait for %d us\n", maxtimer);
            usleep(maxtimer);
        }
        // wait for 1 ms
        else {
            printf("Beb: Will wait for 1 ms\n");
            usleep(1 * 1000);
        }

        // read values again
        int l_txndelaycounter2 = Beb_Read32(csp0base, addr_l_txndelaycounter);
        int l_framedelaycounter2 =
            Beb_Read32(csp0base, addr_l_framedelaycounter);
        int r_txndelaycounter2 = Beb_Read32(csp0base, addr_r_txndelaycounter);
        int r_framedelaycounter2 =
            Beb_Read32(csp0base, addr_r_framedelaycounter);
        int l_framepktLsbcounter2 =
            Beb_Read32(csp0base, addr_l_framepktLsbcounter);
        int l_framepktMsbcounter2 =
            Beb_Read32(csp0base, addr_l_framepktMsbcounter);
        int r_framepktLsbcounter2 =
            Beb_Read32(csp0base, addr_r_framepktLsbcounter);
        int r_framepktMsbcounter2 =
            Beb_Read32(csp0base, addr_r_framepktMsbcounter);
#ifdef VERBOSE
        printf("\nSecond Read:\n"
               "\tLeft [Txndelaycounter:%d, Framedelaycounter:%d]\n"
               "\tRight [Txndelaycounter:%d, Framedelaycounter:%d]\n",
               "\tLeft [FramepacketLsbcounter:%d, FramepacketMsbcounter:%d]\n"
               "\tRight [FramepacketLsbcounter:%d, FramepacketMsbcounter:%d]\n",
               l_txndelaycounter2, l_framedelaycounter2, r_txndelaycounter2,
               r_framedelaycounter2, l_framepktLsbcounter2,
               l_framepktMsbcounter2, r_framepktLsbcounter2,
               r_framepktMsbcounter2);
#endif
        // any change in values, it is still transmitting
        if (l_txndelaycounter != l_txndelaycounter2 ||
            l_framedelaycounter != l_framedelaycounter2 ||
            r_txndelaycounter != r_txndelaycounter2 ||
            r_framedelaycounter != r_framedelaycounter2 ||
            l_framepktLsbcounter != l_framepktLsbcounter2 ||
            l_framepktMsbcounter != l_framepktMsbcounter2 ||
            r_framepktLsbcounter != r_framepktLsbcounter2 ||
            r_framepktMsbcounter != r_framepktMsbcounter2) {
            *retval = 1;
        } else {
            *retval = 0;
        }
        // close file pointer
        Beb_close(fd, csp0base);
    }
    return OK;
}

void Beb_SetTopVariable(int val) { Beb_top = val; }

int Beb_SetTop(enum TOPINDEX ind) {
    if (!Beb_activated)
        return 0;

    u_int32_t *csp0base = 0;
    u_int32_t value = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Set Top FAIL, could not open fd in Beb\n"));
        return 0;
    }
    value = Beb_Read32(csp0base, BEB_CONFIG_WR_OFST);
    switch (ind) {
    case TOP_HARDWARE:
        value &= ~BEB_CONFIG_OW_TOP_MSK;
        break;
    case OW_TOP:
        value |= BEB_CONFIG_OW_TOP_MSK;
        value |= BEB_CONFIG_TOP_MSK;
        break;
    case OW_BOTTOM:
        value |= BEB_CONFIG_OW_TOP_MSK;
        value &= ~BEB_CONFIG_TOP_MSK;
        break;
    default:
        LOG(logERROR, ("Unknown top index in Beb: %d\n", ind));
        Beb_close(fd, csp0base);
        return 0;
    }

    char *top_names[] = {TOP_NAMES};
    int newval = Beb_Write32(csp0base, BEB_CONFIG_WR_OFST, value);
    if (newval != value) {
        LOG(logERROR,
            ("Could not set Top flag to %s in Beb\n", top_names[ind]));
        Beb_close(fd, csp0base);
        return 0;
    }
    LOG(logINFOBLUE,
        ("%s Top flag to %s in Beb\n",
         (ind == TOP_HARDWARE ? "Resetting" : "Overwriting"), top_names[ind]));
    Beb_close(fd, csp0base);
    return 1;
}

int Beb_SetMaster(enum MASTERINDEX ind) {
    if (!Beb_activated)
        return 0;

    u_int32_t *csp0base = 0;
    u_int32_t value = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Set Master FAIL, could not open fd in Beb\n"));
        return 0;
    }
    value = Beb_Read32(csp0base, BEB_CONFIG_WR_OFST);
    switch (ind) {
    case MASTER_HARDWARE:
        value &= ~BEB_CONFIG_OW_MASTER_MSK;
        break;
    case OW_MASTER:
        value |= BEB_CONFIG_OW_MASTER_MSK;
        value |= BEB_CONFIG_MASTER_MSK;
        break;
    case OW_SLAVE:
        value |= BEB_CONFIG_OW_MASTER_MSK;
        value &= ~BEB_CONFIG_MASTER_MSK;
        break;
    default:
        LOG(logERROR, ("Unknown master index in Beb: %d\n", ind));
        Beb_close(fd, csp0base);
        return 0;
    }

    char *master_names[] = {MASTER_NAMES};
    int newval = Beb_Write32(csp0base, BEB_CONFIG_WR_OFST, value);
    if (newval != value) {
        LOG(logERROR,
            ("Could not set Master flag to %s in Beb\n", master_names[ind]));
        Beb_close(fd, csp0base);
        return 0;
    }
    LOG(logINFOBLUE, ("%s Master flag to %s in Beb\n",
                      (ind == MASTER_HARDWARE ? "Resetting" : "Overwriting"),
                      master_names[ind]));

    Beb_close(fd, csp0base);
    return 1;
}

int Beb_SetActivate(int enable) {
    if (enable < 0) {
        LOG(logERROR, ("Invalid enable value\n"));
        return 0;
    }
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Activate FAIL, could not open fd\n"));
        return 0;
    } else {
        u_int32_t value = Beb_Read32(csp0base, BEB_CONFIG_WR_OFST);
        LOG(logDEBUG, ("Activate register value before:%d\n", value));
        if (enable)
            value |= BEB_CONFIG_ACTIVATE_MSK;
        else
            value &= ~BEB_CONFIG_ACTIVATE_MSK;

        u_int32_t retval = Beb_Write32(csp0base, BEB_CONFIG_WR_OFST, value);
        if (retval != value) {
            LOG(logERROR,
                ("Could not %s. WRote 0x%x, read 0x%x\n",
                 (enable ? "activate" : "deactivate"), value, retval));
            Beb_close(fd, csp0base);
        }
    }
    Beb_activated = enable;
    Beb_close(fd, csp0base);
    return 1;
}

int Beb_GetActivate(int *retval) {
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Activate FAIL, could not open fd\n"));
        return 0;
    } else {
        u_int32_t value = Beb_Read32(csp0base, BEB_CONFIG_WR_OFST);
        Beb_activated = (value & BEB_CONFIG_ACTIVATE_MSK) ? 1 : 0;
        if (Beb_activated) {
            LOG(logINFOBLUE, ("Detector is active\n"));
        } else {
            LOG(logINFORED, ("Detector is deactivated!\n"));
        }
    }
    Beb_close(fd, csp0base);
    *retval = Beb_activated;
    return 1;
}

int Beb_SetDataStream(enum portPosition port, int enable) {
    if (!Beb_activated) {
        if (port == LEFT) {
            Beb_deactivated_left_datastream = enable;
        } else {
            Beb_deactivated_right_datastream = enable;
        }
        return 1;
    }
    if (enable < 0) {
        LOG(logERROR, ("Invalid enable value\n"));
        return 0;
    }
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Activate FAIL, could not open fd\n"));
        return 0;
    } else {
        u_int32_t reg = XPAR_GPIO_P15_STREAMING_REG;
        u_int32_t mask = (port == LEFT ? XPAR_GPIO_LFT_STRM_DSBL_MSK
                                       : XPAR_GPIO_RGHT_STRM_DSBL_MSK);

        u_int32_t value = Beb_Read32(csp0base, reg);
        // disabling in firmware 
        if (!enable)
            value |= mask;
        else
            value &= ~mask;
        u_int32_t retval = Beb_Write32(csp0base, reg, value);
        if (retval != value) {
            LOG(logERROR,
                ("Could not %s %s fpga datastream. Wrote 0x%x, read 0x%x\n",
                 (enable ? "enable" : "disable"),
                 (port == LEFT ? "left" : "right"), value, retval));
            Beb_close(fd, csp0base);
        }
    }
    Beb_close(fd, csp0base);
    return 1;
}

int Beb_GetDataStream(enum portPosition port, int *retval) {
    if (!Beb_activated) {
        if (port == LEFT) {
            return Beb_deactivated_left_datastream;
        } else {
            return Beb_deactivated_right_datastream;
        }
    }
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Activate FAIL, could not open fd\n"));
        return 0;
    } else {
        u_int32_t reg = XPAR_GPIO_P15_STREAMING_REG;
        u_int32_t mask = (port == LEFT ? XPAR_GPIO_LFT_STRM_DSBL_MSK
                                       : XPAR_GPIO_RGHT_STRM_DSBL_MSK);

        u_int32_t value = Beb_Read32(csp0base, reg);
        // disabling in firmware
        *retval = (value & mask) ? 0 : 1;
    }
    Beb_close(fd, csp0base);
    return 1;
}

int Beb_Set32bitOverflow(int val) {
    if (!Beb_activated)
        return val;

    // mapping new memory
    u_int32_t *csp0base = 0;
    u_int32_t valueread = 0;
    u_int32_t offset = FLOW_REG_OFFSET;
    if (val > 0)
        val = 1;

    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Could not read register to set overflow flag in 32 bit "
                       "mode. FAIL\n"));
        return -1;
    } else {
        if (val > -1) {
            // reset bit
            valueread = Beb_Read32(csp0base, offset);
            Beb_Write32(csp0base, offset,
                        valueread & ~FLOW_REG_OVERFLOW_32_BIT_MSK);

            // set bit
            valueread = Beb_Read32(csp0base, offset);
            Beb_Write32(csp0base, offset,
                        valueread | ((val << FLOW_REG_OVERFLOW_32_BIT_OFST) &
                                     FLOW_REG_OVERFLOW_32_BIT_MSK));
        }

        valueread =
            (Beb_Read32(csp0base, offset) & FLOW_REG_OVERFLOW_32_BIT_MSK) >>
            FLOW_REG_OVERFLOW_32_BIT_OFST;
    }
    // close file pointer
    Beb_close(fd, csp0base);

    return valueread;
}

int Beb_GetTenGigaFlowControl() {
    if (!Beb_activated)
        return Beb_deactivated_transmission_flowcontrol_10g;

    u_int32_t offset = FLOW_REG_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to get ten giga flow "
                       "control. FAIL\n"));
        return -1;
    } else {
        u_int32_t retval = Beb_Read32(csp0base, offset);
        retval = (retval & FLOW_REG_TXM_FLOW_CNTRL_10G_MSK) >>
                 FLOW_REG_TXM_FLOW_CNTRL_10G_OFST;

        Beb_close(fd, csp0base);
        return retval;
    }
}

int Beb_SetTenGigaFlowControl(int value) {
    LOG(logINFO, ("Setting ten giga flow control to %d\n", value));
    if (!Beb_activated) {
        Beb_deactivated_transmission_flowcontrol_10g = value;
        return 1;
    }

    value = value == 0 ? 0 : 1;
    u_int32_t offset = FLOW_REG_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to set ten giga flow "
                       "control. FAIL\n"));
        return 0;
    } else {
        // reset bit
        u_int32_t retval = Beb_Read32(csp0base, offset);
        Beb_Write32(csp0base, offset,
                    retval & ~FLOW_REG_TXM_FLOW_CNTRL_10G_MSK);

        // set bit
        retval = Beb_Read32(csp0base, offset);
        Beb_Write32(csp0base, offset,
                    retval | ((value << FLOW_REG_TXM_FLOW_CNTRL_10G_OFST) &
                              FLOW_REG_TXM_FLOW_CNTRL_10G_MSK));

        Beb_deactivated_transmission_flowcontrol_10g = value;
        Beb_close(fd, csp0base);
        return 1;
    }
}

int Beb_GetTransmissionDelayFrame() {
    if (!Beb_activated) {
        return Beb_deactivated_transmission_delay_frame;
    }

    u_int32_t offset = TXM_DELAY_FRAME_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to get transmission delay "
                       "frame. FAIL\n"));
        return -1;
    } else {
        u_int32_t retval = Beb_Read32(csp0base, offset);
        Beb_close(fd, csp0base);
        return retval;
    }
}

int Beb_SetTransmissionDelayFrame(int value) {
    LOG(logINFO, ("Setting transmission delay frame to %d\n", value));
    if (value < 0) {
        LOG(logERROR, ("Invalid transmission delay frame value %d\n", value));
        return 0;
    }
    if (!Beb_activated) {
        Beb_deactivated_transmission_delay_frame = value;
        return 1;
    }
    u_int32_t offset = TXM_DELAY_FRAME_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to set transmission delay "
                       "frame. FAIL\n"));
        return 0;
    } else {
        Beb_Write32(csp0base, offset, value);
        Beb_deactivated_transmission_delay_frame = value;
        Beb_close(fd, csp0base);
        return 1;
    }
}

int Beb_GetTransmissionDelayLeft() {
    if (!Beb_activated) {
        return Beb_deactivated_transmission_delay_left;
    }
    u_int32_t offset = TXM_DELAY_LEFT_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to get transmission delay "
                       "left. FAIL\n"));
        return -1;
    } else {
        u_int32_t retval = Beb_Read32(csp0base, offset);
        Beb_close(fd, csp0base);
        return retval;
    }
}

int Beb_SetTransmissionDelayLeft(int value) {
    LOG(logINFO, ("Setting transmission delay left to %d\n", value));
    if (value < 0) {
        LOG(logERROR, ("Invalid transmission delay left value %d\n", value));
        return 0;
    }
    if (!Beb_activated) {
        Beb_deactivated_transmission_delay_left = value;
        return 1;
    }
    u_int32_t offset = TXM_DELAY_LEFT_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to set transmission delay "
                       "left. FAIL\n"));
        return 0;
    } else {
        Beb_Write32(csp0base, offset, value);
        Beb_deactivated_transmission_delay_left = value;
        Beb_close(fd, csp0base);
        return 1;
    }
}

int Beb_GetTransmissionDelayRight() {
    if (!Beb_activated) {
        return Beb_deactivated_transmission_delay_right;
    }

    u_int32_t offset = TXM_DELAY_RIGHT_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to get transmission delay "
                       "right. FAIL\n"));
        return -1;
    } else {
        u_int32_t retval = Beb_Read32(csp0base, offset);
        Beb_close(fd, csp0base);
        return retval;
    }
}

int Beb_SetTransmissionDelayRight(int value) {
    LOG(logINFO, ("Setting transmission delay right to %d\n", value));
    if (value < 0) {
        LOG(logERROR, ("Invalid transmission delay right value %d\n", value));
        return 0;
    }
    if (!Beb_activated) {
        Beb_deactivated_transmission_delay_right = value;
        return 1;
    }
    u_int32_t offset = TXM_DELAY_RIGHT_OFFSET;
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to set transmission delay "
                       "right. FAIL\n"));
        return 0;
    } else {
        Beb_Write32(csp0base, offset, value);
        Beb_deactivated_transmission_delay_right = value;
        Beb_close(fd, csp0base);
        return 1;
    }
}

u_int32_t Beb_GetFirmwareRevision() {
    // mapping new memory
    u_int32_t *csp0base = 0;
    u_int32_t value = 0;

    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_VERSION);
    if (fd < 0) {
        LOG(logERROR, ("Firmware Revision Read FAIL\n"));
    } else {
        value = Beb_Read32(csp0base, FIRMWARE_VERSION_OFFSET);
        if (!value) {
            LOG(logERROR, ("Firmware Revision Number does not exist in "
                           "this version\n"));
        }
    }

    // close file pointer
    if (fd > 0)
        Beb_close(fd, csp0base);

    return value;
}

u_int32_t Beb_GetFirmwareSoftwareAPIVersion() {
    // mapping new memory
    u_int32_t *csp0base = 0;
    u_int32_t value = 0;

    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_VERSION);
    if (fd < 0) {
        LOG(logERROR, ("Firmware Software API Version Read FAIL\n"));
    } else {
        value = Beb_Read32(csp0base, FIRMWARESOFTWARE_API_OFFSET);
        if (!value) {
            LOG(logERROR, ("Firmware Software API Version does not exist in "
                           "this version\n"));
        }
    }

    // close file pointer
    if (fd > 0)
        Beb_close(fd, csp0base);

    return value;
}

void Beb_ResetFrameNumber() {

    if (!Beb_activated)
        return;

    // mapping new memory to read master top module configuration
    u_int32_t *csp0base = 0;
    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_SYS_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Reset Frame Number FAIL\n"));
    } else {
        // write a 1
        Beb_Write32(csp0base, FRAME_NUM_RESET_OFFSET, 1);
        usleep(100000); // 100ms
        // write a 0
        Beb_Write32(csp0base, FRAME_NUM_RESET_OFFSET, 0);
        LOG(logINFO, ("Frame Number Reset OK\n"));
        // close file pointer
        Beb_close(fd, csp0base);
    }
}

void Beb_ClearBebInfos() { bebInfoSize = 0; }

int Beb_InitBebInfos() { // file name at some point
    Beb_ClearBebInfos();

    struct BebInfo b0;
    BebInfo_BebInfo(&b0, 0);
    if (BebInfo_SetSerialAddress(
            &b0,
            0xff)) { // all bebs for reset and possibly get request data?
        beb_infos[bebInfoSize] = b0;
        bebInfoSize++;
    }

    int i0 = Beb_detid, i1 = 0;
    if (Beb_GetBebInfoIndex(i0)) {
        LOG(logERROR,
            ("cant add beb. adding beb %d, beb number %d already added.\n",
             Beb_detid, i0));
        exit(0);
    }
    struct BebInfo b1;
    BebInfo_BebInfo(&b1, i0);
    BebInfo_SetSerialAddress(&b1, i1);
    BebInfo_SetHeaderInfo(&b1, 0, (char *)"00:50:c2:46:d9:34",
                          (char *)"129.129.205.78", 42000 + i0);
    BebInfo_SetHeaderInfo(&b1, 1, (char *)"00:50:c2:46:d9:35",
                          (char *)"10.0.26.1", 52000 + i0);
    beb_infos[bebInfoSize] = b1;
    bebInfoSize++;

    /*
//loop through file to fill vector.
BebInfo* b = new BebInfo(26);
b->SetSerialAddress(0); //0xc4000000
b->SetHeaderInfo(0,"00:50:c2:46:d9:34","129.129.205.78",42000 + 26); // 1
GbE, ip address can be acquire from the network "arp"
b->SetHeaderInfo(1,"00:50:c2:46:d9:35","10.0.26.1",52000 + 26); //10 GbE,
everything calculable/setable beb_infos.push_back(b);
        */

    return Beb_CheckSourceStuffBebInfo();
}

int Beb_SetBebSrcHeaderInfos(unsigned int beb_number, int ten_gig,
                             char *src_mac, char *src_ip,
                             unsigned int src_port) {
    // so that the values can be reset externally for now....

    unsigned int i = 1; /*Beb_GetBebInfoIndex(beb_number);*/
    /******* if (!i) return 0;****************************/ // i must be
                                                            // greater than
                                                            // 0, zero is
                                                            // the global
                                                            // send
    BebInfo_SetHeaderInfo(&beb_infos[i], ten_gig, src_mac, src_ip, src_port);

    LOG(logINFO, ("Printing Beb info number (%d) :\n", i));
    BebInfo_Print(&beb_infos[i]);

    return 1;
}

int Beb_CheckSourceStuffBebInfo() {
    for (unsigned int i = 1; i < bebInfoSize;
         i++) { // header stuff always starts from 1
        if (!Beb_SetHeaderData(BebInfo_GetBebNumber(&beb_infos[i]), 0,
                               "00:00:00:00:00:00", "10.0.0.1", 20000) ||
            !Beb_SetHeaderData(BebInfo_GetBebNumber(&beb_infos[i]), 1,
                               "00:00:00:00:00:00", "10.0.0.1", 20000)) {
            LOG(logINFO, ("Error in BebInfo for module number %d.\n",
                          BebInfo_GetBebNumber(&beb_infos[i])));
            BebInfo_Print(&beb_infos[i]);
            return 0;
        }
    }
    return 1;
}

unsigned int Beb_GetBebInfoIndex(unsigned int beb_numb) {
    for (unsigned int i = 1; i < bebInfoSize; i++)
        if (beb_numb == BebInfo_GetBebNumber(&beb_infos[i])) {
            LOG(logDEBUG1,
                ("*****found beb index:%d, for beb number:%d\n", i, beb_numb));
            return i;
        }
    LOG(logDEBUG1, ("*****Returning 0\n"));
    return 0;
}

int Beb_WriteTo(unsigned int index) {

    if (!Beb_activated)
        return 1;

    if (index >= bebInfoSize) {
        LOG(logERROR, ("WriteTo index error.\n"));
        return 0;
    }

    Beb_send_data_raw[0] =
        0x90000000 | BebInfo_GetSerialAddress(&beb_infos[index]);
    if (Local_Write(ll_beb, 4, Beb_send_data_raw) != 4)
        return 0;

    Beb_send_data_raw[0] = 0xc0000000;
    if ((Beb_send_ndata + 1) * 4 !=
        Local_Write(ll_beb, (Beb_send_ndata + 1) * 4, Beb_send_data_raw))
        return 0;

    return 1;
}

void Beb_SwapDataFun(int little_endian, unsigned int n, unsigned int *d) {
    if (little_endian)
        for (unsigned int i = 0; i < n; i++)
            d[i] = (((d[i] & 0xff) << 24) | ((d[i] & 0xff00) << 8) |
                    ((d[i] & 0xff0000) >> 8) |
                    ((d[i] & 0xff000000) >> 24)); // little_endian
    else
        for (unsigned int i = 0; i < n; i++)
            d[i] = (((d[i] & 0xffff) << 16) | ((d[i] & 0xffff0000) >> 16));
}

int Beb_SetByteOrder() { return 1; }

int Beb_SetUpUDPHeader(unsigned int beb_number, int ten_gig,
                       unsigned int header_number, char *dst_mac, char *dst_ip,
                       unsigned int dst_port) {

    if (!Beb_activated)
        return 1;

    u_int32_t bram_phy_addr;
    u_int32_t *csp0base = 0;
    /*u_int32_t* bram_ptr = NULL;*/
    if (ten_gig)
        bram_phy_addr = 0xC6002000;
    else
        bram_phy_addr = 0xC6001000;

    if (!Beb_SetHeaderData(beb_number, ten_gig, dst_mac, dst_ip, dst_port))
        return 0;

    int fd = Beb_open(&csp0base, bram_phy_addr);
    if (fd < 0) {
        LOG(logERROR, ("Set up UDP Header FAIL\n"));
    } else {
        // read data
        memcpy(csp0base + header_number * 16, &udp_header, sizeof(udp_header));
        // close file pointer
        Beb_close(fd, csp0base);
    }
    return 1;
}

int Beb_SetHeaderData(unsigned int beb_number, int ten_gig, char *dst_mac,
                      char *dst_ip, unsigned int dst_port) {
    unsigned int i = 1; /*Beb_GetBebInfoIndex(beb_number);*/
    /***********************************if (!i) return 0;
     * *************************************///i must be greater than 0, zero is the global send
    return Beb_SetHeaderData1(BebInfo_GetSrcMAC(&beb_infos[i], ten_gig),
                              BebInfo_GetSrcIP(&beb_infos[i], ten_gig),
                              BebInfo_GetSrcPort(&beb_infos[i], ten_gig),
                              dst_mac, dst_ip, dst_port);
}

int Beb_SetHeaderData1(char *src_mac, char *src_ip, unsigned int src_port,
                       char *dst_mac, char *dst_ip, unsigned int dst_port) {
    /* example header*/
    // static unsigned int*   word_ptr   = new unsigned int [16];
    /*static*/
    /*
udp_header_type udp_header = {
            {0x00, 0x50, 0xc5, 0xb2, 0xcb, 0x46},  // DST MAC
            {0x00, 0x50, 0xc2, 0x46, 0xd9, 0x02},  // SRC MAC
            {0x08, 0x00},
            {0x45},
            {0x00},
            {0x00, 0x00},
            {0x00, 0x00},
            {0x40},
            {0x00},
            {0xff},
            {0x11},
            {0x00, 0x00},
            {129, 205, 205, 128},  // Src IP
            {129, 205, 205, 122},  // Dst IP
            {0x0f, 0xa1},
            {0x13, 0x89},
            {0x00, 0x00}, //{0x00, 0x11},
            {0x00, 0x00}
    };
        */

    if (!Beb_SetMAC(src_mac, &(udp_header.src_mac[0])))
        return 0;
    LOG(logINFO, ("Setting Source MAC to %s\n", src_mac));
    if (!Beb_SetIP(src_ip, &(udp_header.src_ip[0])))
        return 0;
    LOG(logINFO, ("Setting Source IP to %s\n", src_ip));
    if (!Beb_SetPortNumber(src_port, &(udp_header.src_port[0])))
        return 0;
    LOG(logINFO, ("Setting Source port to %d\n", src_port));

    if (!Beb_SetMAC(dst_mac, &(udp_header.dst_mac[0])))
        return 0;
    LOG(logINFO, ("Setting Destination MAC to %s\n", dst_mac));
    if (!Beb_SetIP(dst_ip, &(udp_header.dst_ip[0])))
        return 0;
    LOG(logINFO, ("Setting Destination IP to %s\n", dst_ip));
    if (!Beb_SetPortNumber(dst_port, &(udp_header.dst_port[0])))
        return 0;
    LOG(logINFO, ("Setting Destination port to %d\n", dst_port));

    Beb_AdjustIPChecksum(&udp_header);

    unsigned int *base_ptr = (unsigned int *)&udp_header;
    unsigned int num_words = (sizeof(struct udp_header_type) + 3) / 4;
    for (unsigned int i = 0; i < num_words; i++)
        Beb_send_data[i + 2] = base_ptr[i];
    for (unsigned int i = num_words; i < 16; i++)
        Beb_send_data[i + 2] = 0;

    return 1;
}

int Beb_SetMAC(char *mac, uint8_t *dst_ptr) {
    char macVal[50];
    strcpy(macVal, mac);

    int i = 0;
    char *pch = strtok(macVal, ":");
    while (pch != NULL) {
        if (strlen(pch) != 2) {
            LOG(logERROR, ("Error: in mac address -> %s\n", macVal));
            return 0;
        }

        int itemp;
        sscanf(pch, "%x", &itemp);
        dst_ptr[i] = (u_int8_t)itemp;
        pch = strtok(NULL, ":");
        i++;
    }
    return 1;
}

int Beb_SetIP(char *ip, uint8_t *dst_ptr) {
    char ipVal[50];
    strcpy(ipVal, ip);
    int i = 0;
    char *pch = strtok(ipVal, ".");
    while (pch != NULL) {
        if (((i != 3) && ((strlen(pch) > 3) || (strlen(pch) < 1))) ||
            ((i == 3) && ((strlen(pch) < 1) || (strlen(pch) > 3)))) {
            LOG(logERROR, ("Error: in ip address -> %s\n", ipVal));
            return 0;
        }

        int itemp;
        sscanf(pch, "%d", &itemp);
        dst_ptr[i] = (u_int8_t)itemp;
        pch = strtok(NULL, ".");
        i++;
    }
    return 1;
}

int Beb_SetPortNumber(unsigned int port_number, uint8_t *dst_ptr) {
    dst_ptr[0] = (port_number >> 8) & 0xff;
    dst_ptr[1] = port_number & 0xff;
    return 1;
}

void Beb_AdjustIPChecksum(struct udp_header_type *ip) {
    unsigned char *cptr = (unsigned char *)ip->ver_headerlen;

    ip->ip_header_checksum[0] = 0;
    ip->ip_header_checksum[1] = 0;
    ip->total_length[0] = 0;
    ip->total_length[1] = 28; // IP + UDP Header Length

    // calc ip checksum
    unsigned int ip_checksum = 0;
    for (unsigned int i = 0; i < 10; i++) {
        ip_checksum += ((cptr[2 * i] << 8) + (cptr[2 * i + 1]));
        if (ip_checksum & 0x00010000)
            ip_checksum = (ip_checksum + 1) & 0x0000ffff;
    }

    ip->ip_header_checksum[0] = (ip_checksum >> 8) & 0xff;
    ip->ip_header_checksum[1] = ip_checksum & 0xff;
}

int Beb_SetUpTransferParameters(short the_bit_mode) {
    if (the_bit_mode != 4 && the_bit_mode != 8 && the_bit_mode != 16 &&
        the_bit_mode != 32)
        return 0;
    Beb_bit_mode = the_bit_mode;
    return 1;
}

int Beb_StopAcquisition() {
    if (!Beb_activated)
        return 1;

    u_int32_t *csp0base = 0;
    volatile u_int32_t valuel, valuer;
    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_CMD_GENERATOR);
    if (fd < 0) {
        LOG(logERROR, ("Beb Reset FAIL\n"));
        return 0;
    } else {
        // find value
        valuel = Beb_Read32(csp0base, (LEFT_OFFSET + STOP_ACQ_OFFSET));
        valuer = Beb_Read32(csp0base, (RIGHT_OFFSET + STOP_ACQ_OFFSET));
        // high
        Beb_Write32(csp0base, (LEFT_OFFSET + STOP_ACQ_OFFSET),
                    (valuel | STOP_ACQ_BIT));
        Beb_Write32(csp0base, (RIGHT_OFFSET + STOP_ACQ_OFFSET),
                    (valuer | STOP_ACQ_BIT));
        // low
        Beb_Write32(csp0base, (LEFT_OFFSET + STOP_ACQ_OFFSET),
                    (valuel & (~STOP_ACQ_BIT)));
        Beb_Write32(csp0base, (RIGHT_OFFSET + STOP_ACQ_OFFSET),
                    (valuer & (~STOP_ACQ_BIT)));

        LOG(logINFO, ("Beb: Reset done\n"));
        // close file pointer
        Beb_close(fd, csp0base);
    }
    return 1;
}

int Beb_RequestNImages(int ten_gig, unsigned int nimages, int test_just_send_out_packets_no_wait) {
    if (!Beb_activated)
        return 1;

     unsigned int maxnl = MAX_ROWS_PER_READOUT;
    unsigned int maxnp = (ten_gig ? 4 : 16) * Beb_bit_mode;
    unsigned int nl = Beb_partialReadout;
    unsigned int npackets = (nl * maxnp) / maxnl;
    if ((nl * maxnp) % maxnl) {
        LOG(logERROR, ("Read N Lines is incorrect. Switching to Full Image "
                       "Readout\n"));
        npackets = maxnp;
    }
    int in_two_requests = (npackets > MAX_PACKETS_PER_REQUEST) ? 1 : 0;
    if (in_two_requests) {
        npackets /= 2;
    }
    unsigned int header_size = 4;                      // 4*64 bits
    unsigned int packet_size = ten_gig ? 0x200 : 0x80; // 4k or  1k packets

    LOG(logDEBUG1, ("----Beb_RequestNImages Start----\n"));
    LOG(logINFO, ("ten_gig:%d, npackets:%d, "
                  "Beb_bit_mode:%d, header_size:%d, nimages:%d, "
                  "test_just_send_out_packets_no_wait:%d\n",
                  ten_gig, npackets, Beb_bit_mode,
                  header_size, nimages, test_just_send_out_packets_no_wait));

    u_int32_t right_port_value = 0x2000;
    u_int32_t *csp0base = 0;
    volatile u_int32_t value;
    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_CMD_GENERATOR);
    if (fd < 0) {
        LOG(logERROR, ("Beb Request N Images FAIL\n"));
        return 0;
    } else {
        for (int i = 0; i < 10; i++) {
            LOG(logDEBUG1,
                ("%X\n", Beb_Read32(csp0base, (LEFT_OFFSET + i * 4))));
        }
        // Generating commands
        u_int32_t send_header_command =
            0x62000000 | (!test_just_send_out_packets_no_wait) << 27 |
            (ten_gig == 1) << 24 | header_size << 14 | 0;
        u_int32_t send_frame_command =
            0x62000000 | (!test_just_send_out_packets_no_wait) << 27 |
            (ten_gig == 1) << 24 | packet_size << 14 | (npackets - 1);
        for (int i = 0; i < 10; i++) {
            LOG(logDEBUG1,
                ("%X\n", Beb_Read32(csp0base, (LEFT_OFFSET + i * 4))));
        }
        LOG(logDEBUG1, ("%d\n", in_two_requests));
        // Left
        Beb_Write32(csp0base, (LEFT_OFFSET + FIRST_CMD_PART1_OFFSET), 0);
        Beb_Write32(csp0base, (LEFT_OFFSET + FIRST_CMD_PART2_OFFSET),
                    send_header_command);
        Beb_Write32(csp0base, (LEFT_OFFSET + SECOND_CMD_PART1_OFFSET), 0);
        Beb_Write32(csp0base, (LEFT_OFFSET + SECOND_CMD_PART2_OFFSET),
                    send_frame_command);
        value = Beb_Read32(csp0base, (LEFT_OFFSET + TWO_REQUESTS_OFFSET));
        if (in_two_requests)
            Beb_Write32(csp0base, (LEFT_OFFSET + TWO_REQUESTS_OFFSET),
                        (value | TWO_REQUESTS_BIT));
        else
            Beb_Write32(csp0base, (LEFT_OFFSET + TWO_REQUESTS_OFFSET),
                        (value & ~(TWO_REQUESTS_BIT)));

        // Right
        Beb_Write32(csp0base, (RIGHT_OFFSET + FIRST_CMD_PART1_OFFSET), 0);
        Beb_Write32(csp0base, (RIGHT_OFFSET + FIRST_CMD_PART2_OFFSET),
                    send_header_command | right_port_value);
        Beb_Write32(csp0base, (RIGHT_OFFSET + SECOND_CMD_PART1_OFFSET), 0);
        Beb_Write32(csp0base, (RIGHT_OFFSET + SECOND_CMD_PART2_OFFSET),
                    send_frame_command | right_port_value);
        value = Beb_Read32(csp0base, (RIGHT_OFFSET + TWO_REQUESTS_OFFSET));
        if (in_two_requests)
            Beb_Write32(csp0base, (RIGHT_OFFSET + TWO_REQUESTS_OFFSET),
                        (value | TWO_REQUESTS_BIT));
        else
            Beb_Write32(csp0base, (RIGHT_OFFSET + TWO_REQUESTS_OFFSET),
                        (value & ~(TWO_REQUESTS_BIT)));

        // Set number of frames
        Beb_Write32(csp0base, (LEFT_OFFSET + COMMAND_COUNTER_OFFSET),
                    nimages * (2 + in_two_requests));
        Beb_Write32(csp0base, (RIGHT_OFFSET + COMMAND_COUNTER_OFFSET),
                    nimages * (2 + in_two_requests));
        for (int i = 0; i < 10; i++) {
            LOG(logDEBUG1,
                ("%X\n", Beb_Read32(csp0base,
                                    (LEFT_OFFSET + i * 4)))); //*(ptrl+i));
        }
        LOG(logDEBUG1, ("%d\n", in_two_requests));
        Beb_close(fd, csp0base);

        LOG(logDEBUG1, ("----Beb_RequestNImages----\n"));
    }

    return 1;
}

// Returns the FPGA temperature from the xps sysmon ip core
// Temperature value is cropped and not well rounded
int Beb_GetBebFPGATemp() {

    u_int32_t *csp0base = 0;
    int temperature = 0;
    int ret;
    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_SYSMON_0_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Module Configuration FAIL\n"));
    } else {
        // read data
        ret = Beb_Read32(csp0base, FPGA_TEMP_OFFSET);
        temperature = ((((float)(ret) / 65536.0f) / 0.00198421639f) - 273.15f) *
                      1000; // Static conversation, copied from xps sysmon
                            // standalone driver
        // close file pointer
        Beb_close(fd, csp0base);
    }

    return temperature;
}

void Beb_SetDetectorNumber(uint32_t detid) {
    if (!Beb_activated)
        return;

    uint32_t swapid = Beb_swap_uint16(detid);
    // LOG(logINFO, "detector id %d swapped %d\n", detid, swapid));
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_TEST_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Set Detector ID FAIL\n"));
        return;
    } else {
        uint32_t value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
        value &= UDP_HEADER_X_MSK; // to keep previous x value
        Beb_Write32(csp0base, UDP_HEADER_A_LEFT_OFST,
                    value |
                        ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK));
        value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
        if ((value & UDP_HEADER_ID_MSK) !=
            ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK)) {
            LOG(logERROR, ("Set Detector ID FAIL\n"));
        }
        value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
        value &= UDP_HEADER_X_MSK; // to keep previous x value
        Beb_Write32(csp0base, UDP_HEADER_A_RIGHT_OFST,
                    value |
                        ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK));
        value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
        if ((value & UDP_HEADER_ID_MSK) !=
            ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK)) {
            LOG(logERROR, ("Set Detector ID FAIL\n"));
        }
        Beb_close(fd, csp0base);
    }
    LOG(logINFO, ("Detector id %d set in UDP Header\n\n", detid));
}

int Beb_SetQuad(int value) {
    if (value < 0)
        return OK;
    LOG(logINFO, ("Setting Quad to %d in Beb\n", value));
    Beb_quadEnable = (value == 0 ? 0 : 1);
    return Beb_SetDetectorPosition(Beb_positions);
}

int Beb_GetQuad() { return Beb_quadEnable; }

int *Beb_GetDetectorPosition() { return Beb_positions; }

int Beb_SetDetectorPosition(int pos[]) {
    if (!Beb_activated)
        return OK;
    LOG(logINFO, ("Got Position values %d %d...\n", pos[0], pos[1]));

    // save positions
    Beb_positions[0] = pos[0];
    Beb_positions[1] = pos[1];

    // get left and right
    int posLeft[2] = {pos[0], Beb_top ? pos[1] : pos[1] + 1};
    int posRight[2] = {pos[0], Beb_top ? pos[1] + 1 : pos[1]};

    if (Beb_quadEnable) {
        posRight[0] = 1; // right is next row
        posRight[1] = 0; // right same first column
    }

    int ret = FAIL;
    // mapping new memory to read master top module configuration
    u_int32_t *csp0base = 0;
    // open file pointer
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_TEST_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Set Detector Position FAIL\n"));
        return FAIL;
    } else {
        uint32_t value = 0;
        ret = OK;
        // x left
        int posval = Beb_swap_uint16(posLeft[0]);
        value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
        value &= UDP_HEADER_ID_MSK; // to keep previous id value
        Beb_Write32(csp0base, UDP_HEADER_A_LEFT_OFST,
                    value | ((posval << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK));
        value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
        if ((value & UDP_HEADER_X_MSK) !=
            ((posval << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK)) {
            LOG(logERROR, ("Could not set row position for left port\n"));
            ret = FAIL;
        }
        // x right
        posval = Beb_swap_uint16(posRight[0]);
        value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
        value &= UDP_HEADER_ID_MSK; // to keep previous id value
        Beb_Write32(csp0base, UDP_HEADER_A_RIGHT_OFST,
                    value | ((posval << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK));
        value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
        if ((value & UDP_HEADER_X_MSK) !=
            ((posval << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK)) {
            LOG(logERROR, ("Could not set row position for right port\n"));
            ret = FAIL;
        }

        // y left (column)
        posval = Beb_swap_uint16(posLeft[1]);
        value = Beb_Read32(csp0base, UDP_HEADER_B_LEFT_OFST);
        value &= UDP_HEADER_Z_MSK; // to keep previous z value
        Beb_Write32(csp0base, UDP_HEADER_B_LEFT_OFST,
                    value | ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK));
        value = Beb_Read32(csp0base, UDP_HEADER_B_LEFT_OFST);
        if ((value & UDP_HEADER_Y_MSK) !=
            ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK)) {
            LOG(logERROR, ("Could not set column position for left port\n"));
            ret = FAIL;
        }

        // y right
        posval = Beb_swap_uint16(posRight[1]);
        value = Beb_Read32(csp0base, UDP_HEADER_B_RIGHT_OFST);
        value &= UDP_HEADER_Z_MSK; // to keep previous z value
        Beb_Write32(csp0base, UDP_HEADER_B_RIGHT_OFST,
                    value | ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK));
        value = Beb_Read32(csp0base, UDP_HEADER_B_RIGHT_OFST);
        if ((value & UDP_HEADER_Y_MSK) !=
            ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK)) {
            LOG(logERROR, ("Could not set column position for right port\n"));
            ret = FAIL;
        }

        // close file pointer
        Beb_close(fd, csp0base);
    }
    if (ret == OK) {
        LOG(logINFO, ("Position set to...\n"
                      "\tLeft: [%d, %d]\n"
                      "\tRight:[%d, %d]\n",
                      posLeft[0], posLeft[1], posRight[0], posRight[1]));
    }

    return ret;
}

int Beb_SetNextFrameNumber(uint64_t value) {
    if (!Beb_activated) {
        Beb_deactivatedNextFrameNumber = value;
        return OK;
    }
    LOG(logINFO,
        ("Setting next frame number: %llu\n", (long long unsigned int)value));

    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_PLB_GPIO_TEST_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Set next Frame Number FAIL\n"));
        return FAIL;
    }
    // since the read is not implemented in firmware yet
    Beb_deactivatedNextFrameNumber = value;

    // decrement for firmware
    uint64_t valueInFirmware = value - 1;
    Beb_Write32(csp0base, UDP_HEADER_FRAME_NUMBER_LSB_OFST,
                valueInFirmware & (0xffffffff));
    Beb_Write32(csp0base, UDP_HEADER_FRAME_NUMBER_MSB_OFST,
                (valueInFirmware >> 32) & (0xffffffff));
    Beb_close(fd, csp0base);

    LOG(logINFO, ("Going to reset Frame Number\n"));
    Beb_ResetFrameNumber();
    return OK;
}

int Beb_GetNextFrameNumber(uint64_t *retval, int tengigaEnable) {
    if (!Beb_activated) {
        *retval = Beb_deactivatedNextFrameNumber;
        return OK;
    }

    LOG(logDEBUG1, ("Getting next frame number\n"));
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_COUNTER_BASEADDR);
    if (fd < 0) {
        LOG(logERROR, ("Get next Frame Number FAIL\n"));
        return FAIL;
    }

    uint32_t temp = 0;
    if (!tengigaEnable) {
        uint64_t left1g =
            Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_MSB_OFST);
        temp = Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_LSB_OFST);
        left1g = ((left1g << 32) | temp) >> 16;
        ++left1g; // increment for firmware

        uint64_t right1g =
            Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_MSB_OFST);
        temp = Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_LSB_OFST);
        right1g = ((right1g << 32) | temp) >> 16;
        ++right1g; // increment for firmware

        Beb_close(fd, csp0base);
        if (left1g != right1g) {
            LOG(logERROR, ("Retrieved inconsistent frame numbers from 1g left "
                           "%llu and right %llu\n",
                           (long long int)left1g, (long long int)right1g));
            *retval = (left1g > right1g)
                          ? left1g
                          : right1g; // give max to set it to when stopping
                                     // acq & different value
            return -2; // to differentiate between failed address mapping
        }
        *retval = left1g;
    }

    else {
        uint64_t left10g =
            Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_MSB_OFST);
        temp = Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_LSB_OFST);
        left10g = ((left10g << 32) | temp) >> 16;
        ++left10g; // increment for firmware

        uint64_t right10g =
            Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_MSB_OFST);
        temp = Beb_Read32(csp0base, UDP_HEADER_GET_FNUM_1G_LEFT_LSB_OFST);
        right10g = ((right10g << 32) | temp) >> 16;
        Beb_close(fd, csp0base);
        ++right10g; // increment for firmware

        if (left10g != right10g) {
            LOG(logERROR, ("Retrieved inconsistent frame numbers from `0g left "
                           "%llu and right %llu\n",
                           (long long int)left10g, (long long int)right10g));
            *retval = (left10g > right10g)
                          ? left10g
                          : right10g; // give max to set it to when stopping
                                      // acq & different value
            return -2; // to differentiate between failed address mapping
        }
        *retval = left10g;
    }
    return OK;
}

void Beb_SetPartialReadout(int value) { Beb_partialReadout = value; }

int Beb_GetNumberofDestinations(int *retval) {
    if (!Beb_activated) {
        *retval = Beb_deactivated_num_destinations;
        return OK;
    }
    u_int32_t offset[2] = {LEFT_OFFSET + NUM_UDP_DEST_OFFSET,
                           RIGHT_OFFSET + NUM_UDP_DEST_OFFSET};
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_CMD_GENERATOR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to get number of udp "
                       "destinations. FAIL\n"));
        return FAIL;
    } else {
        int retval1[2] = {0, 0};
        retval1[0] = Beb_Read32(csp0base, offset[0]);
        retval1[1] = Beb_Read32(csp0base, offset[1]);
        Beb_close(fd, csp0base);
        if (retval1[0] != retval1[1]) {
            LOG(logERROR, ("Inconsistent values on left (%d) and right (%d) "
                           "fpga for number of destinations. FAIL\n",
                           retval1[0], retval1[1]));
            return FAIL;
        }
        *retval = ++retval1[0];
        return OK;
    }
}

int Beb_SetNumberofDestinations(int value) {
    LOG(logINFO, ("Setting number of destinations to %d\n", value));
    if (value < 0 || value >= MAX_UDP_DESTINATION) {
        LOG(logERROR, ("Invalid number of destinations %d\n", value));
        return FAIL;
    }
    if (!Beb_activated) {
        Beb_deactivated_num_destinations = value;
        return FAIL;
    }
    u_int32_t offset[2] = {LEFT_OFFSET + NUM_UDP_DEST_OFFSET,
                           RIGHT_OFFSET + NUM_UDP_DEST_OFFSET};
    u_int32_t *csp0base = 0;
    int fd = Beb_open(&csp0base, XPAR_CMD_GENERATOR);
    if (fd <= 0) {
        LOG(logERROR, ("Could not read register to set number of udp "
                       "destinations. FAIL\n"));
        return FAIL;
    } else {
        Beb_deactivated_num_destinations = value;
        --value;
        Beb_Write32(csp0base, offset[0], value);
        Beb_Write32(csp0base, offset[1], value);
        Beb_close(fd, csp0base);
        return OK;
    }
}

uint16_t Beb_swap_uint16(uint16_t val) { return (val << 8) | (val >> 8); }

int Beb_open(u_int32_t **csp0base, u_int32_t offset) {

    int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
    if (fd == -1) {
        LOG(logERROR, ("\nCan't find /dev/mem!\n"));
    } else {
        LOG(logDEBUG1, ("/dev/mem opened\n"));
        *csp0base = (u_int32_t *)mmap(0, BEB_MMAP_SIZE, PROT_READ | PROT_WRITE,
                                      MAP_FILE | MAP_SHARED, fd, offset);
        if (*csp0base == MAP_FAILED) {
            LOG(logERROR, ("\nCan't map memmory area!!\n"));
            fd = -1;
        } else
            LOG(logDEBUG1, ("CSP0 mapped %p\n", (void *)*csp0base));
    }
    return fd;
}

u_int32_t Beb_Read32(u_int32_t *baseaddr, u_int32_t offset) {
    volatile u_int32_t value;
    value = *(u_int32_t *)(baseaddr + offset / (sizeof(u_int32_t)));
    return value;
}

u_int32_t Beb_Write32(u_int32_t *baseaddr, u_int32_t offset, u_int32_t data) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(baseaddr + offset / (sizeof(u_int32_t)));
    *ptr1 = data;
    return *ptr1;
}

void Beb_close(int fd, u_int32_t *csp0base) {
    if (fd >= 0)
        close(fd);
    munmap(csp0base, BEB_MMAP_SIZE);
}
