// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "arm64.h"
#include "clogger.h"
#include "common.h"
#include "programViaArm.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#include "LTC2620_Driver.h"
#include "XILINX_FMC.h"
#include "XILINX_PLL.h"
#include "loadPattern.h"
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#include <math.h> //ceil
#include <pthread.h>
#endif

#include <arpa/inet.h> // INET_ADDRSTRLEN
#include <string.h>
#include <unistd.h> // usleep

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern int checkModuleFlag;
extern udpStruct udpDetails[MAX_UDP_DESTINATION];
extern const enum detectorType myDetectorType;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char *cmac, int size, uint64_t mac);
extern void getIpAddressinString(char *cip, uint32_t ip);

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

int detPos[2] = {0, 0};

uint32_t clkFrequency[NUM_CLOCKS] = {};
int analogEnable = 0;
int digitalEnable = 0;
int transceiverEnable = 0;
int dacValues[NDAC_ONLY] = {};
int powerValues[NPWR - 1] = {}; // powerIndex (A->IO)
// software limit that depends on the current chip on the ctb
int vLimit = 0;

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
#endif

int isInitCheckDone() { return initCheckDone; }

int getInitResult(char **mess) {
    *mess = initErrorMessage;
    return initError;
}

void basictests() {
    initError = OK;
    initCheckDone = 0;
    memset(initErrorMessage, 0, MAX_STR_LENGTH);
#ifdef VIRTUAL
    LOG(logINFOBLUE, ("****** Xilinx Chip Test Board Virtual Server ******\n"));
#else
    LOG(logINFOBLUE, ("********** Xilinx Chip Test Board Server **********\n"));
#endif

    initError = resetFPGA(initErrorMessage);
    if (initError == FAIL) {
        return;
    }

    initError = loadDeviceTree(initErrorMessage);
    if (initError == FAIL) {
        return;
    }

    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Cannot proceed. Check Firmware.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

#ifndef VIRTUAL
    if ((!debugflag) && (!updateFlag) &&
        ((validateKernelVersion(KERNEL_DATE_VRSN) == FAIL) ||
         (checkType() == FAIL) || (testFixedFPGAPattern() == FAIL))) {
        sprintf(initErrorMessage,
                "Could not pass basic tests of FPGA and bus. Cannot proceed. "
                "Check Firmware. (Firmware version:0x%lx) \n",
                getFirmwareVersion());
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }
#endif
    char hversion[MAX_STR_LENGTH] = {0};
    memset(hversion, 0, MAX_STR_LENGTH);
    getHardwareVersion(hversion);
    uint32_t ipadd = getDetectorIP();
    uint64_t macadd = getDetectorMAC();
    int64_t fwversion = getFirmwareVersion();
    char swversion[MAX_STR_LENGTH] = {0};
    memset(swversion, 0, MAX_STR_LENGTH);
    getServerVersion(swversion);
    int64_t sw_fw_apiversion = getFirmwareAPIVersion();
    uint64_t requiredfwversion = REQRD_FRMWRE_VRSN;

    LOG(logINFOBLUE,
        ("**************************************************\n"
         "Hardware Version:\t\t %s\n"

         "Detector IP Addr:\t\t 0x%x\n"
         "Detector MAC Addr:\t\t 0x%lx\n\n"

         "Firmware Version:\t\t 0x%lx\n"
         "Software Version:\t\t %s\n"
         "F/w-S/w API Version:\t\t 0x%lx\n"
         "Required Firmware Version:\t 0x%lx\n"
         "********************************************************\n",
         hversion, ipadd, macadd, fwversion, swversion, sw_fw_apiversion,
         requiredfwversion));

#ifndef VIRTUAL
    // return if flag is not zero, debug mode
    if (debugflag || updateFlag) {
        return;
    }

    // cant read versions
    LOG(logINFO, ("Testing Firmware-software compatibility:\n"));
    if (!fwversion || !sw_fw_apiversion) {
        strcpy(initErrorMessage,
               "Cant read versions from FPGA. Please update firmware.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for API compatibility - old server
    if (sw_fw_apiversion > requiredfwversion) {
        sprintf(initErrorMessage,
                "This firmware-software api version (0x%lx) is incompatible "
                "with the software's minimum required firmware version "
                "(0x%lx).\nPlease update detector software to be compatible "
                "with this firmware.\n",
                sw_fw_apiversion, requiredfwversion);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for firmware compatibility - old firmware
    if (requiredfwversion > fwversion) {
        sprintf(initErrorMessage,
                "This firmware version (0x%lx) is incompatible.\n"
                "Please update firmware (min. 0x%lx) to be compatible with "
                "this server.\n",
                fwversion, requiredfwversion);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }
    LOG(logINFO, ("\tCompatibility - success\n"));
#endif
}

int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
    u_int32_t type =
        ((bus_r(FPGAVERSIONREG) & FPGADETTYPE_MSK) >> FPGADETTYPE_OFST);
    if (type != XILINX_CHIPTESTBOARD) {
        LOG(logERROR,
            ("This is not a Xilinx CTB firmware (read %d, expected %d)\n", type,
             XILINX_CHIPTESTBOARD));
        return FAIL;
    }
    return OK;
}

int testFpga() {
    LOG(logINFO, ("Testing FPGA:\n"));

    // fixed pattern
    int ret = testFixedFPGAPattern();

    if (ret == OK) {
        // Delay LSB reg
        LOG(logINFO, ("\tTesting Delay LSB Register:\n"));
        uint32_t addr = DELAY_IN_REG_1;

        // store previous delay value
        uint32_t previousValue = bus_r(addr);

        volatile uint32_t val = 0, readval = 0;
        int times = 1000 * 1000;
        for (int i = 0; i < times; ++i) {
            val = 0x5A5A5A5A - i;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("1:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
            val = (i + (i << 10) + (i << 20));
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("2:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
            val = 0x0F0F0F0F;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("3:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
            val = 0xF0F0F0F0;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("4:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
        }

        // write back previous value
        bus_w(addr, previousValue);
        if (ret == OK) {
            LOG(logINFO,
                ("\tSuccessfully tested FPGA Delay LSB Register %d times\n",
                 times));
        }
    }
    return ret;
}

int testFixedFPGAPattern() {
    LOG(logINFO, ("Testing FPGA Fixed Pattern:\n"));
#ifndef VIRTUAL
    uint32_t val = bus_r(FIXEDPATTERNREG);
    if (val == FIXEDPATTERNREG_PRESET) {
        LOG(logINFO, ("\tFixed pattern: successful match (0x%08x)\n", val));
    } else {
        LOG(logERROR,
            ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n",
             val, FIXEDPATTERNREG_PRESET));
        return FAIL;
    }
#endif
    LOG(logINFO, ("\tSuccessfully read FPGA Fixed Pattern (0x%x)\n",
                  FIXEDPATTERNREG_PRESET));
    return OK;
}

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIXILINXCTB); }

uint64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return REQRD_FRMWRE_VRSN;
#endif
    return ((bus_r(FPGAVERSIONREG) & FPGACOMPDATE_MSK) >> FPGACOMPDATE_OFST);
}

uint64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(APIVERSIONREG) & APICOMPDATE_MSK) >> APICOMPDATE_OFST);
}

void getHardwareVersion(char *version) { strcpy(version, "Not applicable"); }

u_int64_t getDetectorMAC() {
#ifdef VIRTUAL
    return 0;
#else
    char output[255], mac[255] = "";
    u_int64_t res = 0;
    FILE *sysFile =
        popen("ifconfig eth0 | grep ether |  awk '{ print $2 }'", "r");
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);
    // getting rid of ":"
    char *pch;
    pch = strtok(output, ":");
    while (pch != NULL) {
        strcat(mac, pch);
        pch = strtok(NULL, ":");
    }
    sscanf(mac, "%lx", &res);
    return res;
#endif
}

u_int32_t getDetectorIP() {
#ifdef VIRTUAL
    return 0;
#endif
    char temp[INET_ADDRSTRLEN] = "";
    u_int32_t res = 0;
    // execute and get address
    char output[255];
    FILE *sysFile = popen(
        "ifconfig  | grep 'inet '| grep -v '127.0.0.1' | awk '{ print $2 }'",
        "r");
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);

    // converting IPaddress to hex.
    char *pcword = strtok(output, ".");
    while (pcword != NULL) {
        sprintf(output, "%02x", atoi(pcword));
        strcat(temp, output);
        pcword = strtok(NULL, ".");
    }
    strcpy(output, temp);
    sscanf(output, "%x", &res);
    // LOG(logINFO, ("ip:%x\n",res);

    return res;
}

/* initialization */

void initControlServer() {
    if (!updateFlag && initError == OK) {
        setupDetector();
    }
    initCheckDone = 1;
}

void initStopServer() {
    if (!updateFlag && initError == OK) {
        usleep(CTRL_SRVR_INIT_TIME_US);
        LOG(logINFOBLUE, ("Configuring Stop server\n"));
        if (mapCSP0() == FAIL) {
            initError = FAIL;
            strcpy(initErrorMessage,
                   "Stop Server: Map Fail. Cannot proceed. Check Firmware.\n");
            LOG(logERROR, (initErrorMessage));
            initCheckDone = 1;
            return;
        }
#ifdef VIRTUAL
        setupDetector();
#endif
    }
    initCheckDone = 1;
}

/* set up detector */

void setupDetector() {
    LOG(logINFO, ("Setting up Server for 1 Xilinx Chip Test Board\n"));

    // default variables
    clkFrequency[RUN_CLK] = DEFAULT_RUN_CLK;
    clkFrequency[ADC_CLK] = DEFAULT_ADC_CLK;
    clkFrequency[SYNC_CLK] = DEFAULT_SYNC_CLK;
    clkFrequency[DBIT_CLK] = DEFAULT_DBIT_CLK;
    analogEnable = 0;
    digitalEnable = 0;
    transceiverEnable = 0;
    for (int i = 0; i != NDAC_ONLY; ++i)
        dacValues[i] = -1;
    for (int i = 0; i != NPWR - 1; ++i)
        powerValues[i] = -1;
    vLimit = DEFAULT_VLIMIT;

#ifdef VIRTUAL
    if (isControlServer) {
        sharedMemory_setStatus(IDLE);
        setupUDPCommParameters();
        initializePatternWord();
    } else {
        sharedMemory_setStop(0);
    }
#endif
    // initialization only at start up (restart fpga)
    initError = waitTransceiverReset(initErrorMessage);
    if (initError == FAIL) {
        return;
    }

    powerOff();

    LTC2620_D_SetDefines(DAC_MIN_MV, DAC_MAX_MV, DAC_DRIVER_FILE_NAME, NDAC,
                         NPWR, DAC_POWERDOWN_DRIVER_FILE_NAME);

    // power LTC2620 before talking to it
    initError = XILINX_FMC_enable_all(initErrorMessage, MAX_STR_LENGTH);
    if (initError == FAIL) {
        return;
    }
    // dacs only
    LOG(logINFOBLUE, ("Setting all dacs to min (0 mV)\n"));
    for (int idac = 0; idac < NDAC_ONLY; ++idac) {
        initError = setDAC(idac, 0, false, initErrorMessage);
        if (initError == FAIL)
            return;
    }

    // power regulators
    LOG(logINFOBLUE,
        ("Setting power dacs to min dac value (power disabled)\n"));
    for (int iPower = 0; iPower != (NPWR - 1); ++iPower) {
        int min = (iPower == (int)V_POWER_IO) ? VIO_MIN_MV : POWER_RGLTR_MIN;
        initError = setPowerDAC(iPower, min, initErrorMessage);
        if (initError == FAIL)
            return;
    }

    resetFlow();
    cleanFifos();

    initializePatternAddresses();

    LOG(logINFOBLUE, ("Setting Default readout\n"));
    setNumAnalogSamples(DEFAULT_NUM_ASAMPLES);
    setNumDigitalSamples(DEFAULT_NUM_DSAMPLES);
    setADCEnableMask_10G(BIT32_MSK);
    setTransceiverEnableMask(DEFAULT_TRANSCEIVER_MASK);
    setNumTransceiverSamples(DEFAULT_NUM_TSAMPLES);
    setReadoutMode(DEFAULT_READOUT_MODE);

    LOG(logINFOBLUE, ("Setting Default parameters\n"));
    setNumFrames(DEFAULT_NUM_FRAMES);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setTiming(DEFAULT_TIMING_MODE);
    initError = setExpTime(DEFAULT_EXPTIME, initErrorMessage);
    if (initError == FAIL)
        return;
    initError = setPeriod(DEFAULT_PERIOD, initErrorMessage);
    if (initError == FAIL)
        return;
    initError = setDelayAfterTrigger(DEFAULT_DELAY, initErrorMessage);
    if (initError == FAIL)
        return;

    setNextFrameNumber(DEFAULT_STARTING_FRAME_NUMBER);
}

/* firmware functions (resets) */

void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
    uint32_t t_enable_mask = getTransceiverEnableMask();
    uint32_t tclean_msk =
        ((t_enable_mask << X_FIFO_CLEAN_OFST) & X_FIFO_CLEAN_MSK);
    uint32_t t_before_reg = bus_r(X_FIFO_CLEAN_REG);
    LOG(logINFO, ("Clearing Acquisition Fifos\n"));
    bus_w(A_FIFO_CLEAN_REG, bus_r(A_FIFO_CLEAN_REG) | BIT32_MSK);
    bus_w(D_FIFO_CLEAN_REG, bus_r(D_FIFO_CLEAN_REG) | D_FIFO_CLEAN_MSK);
    bus_w(X_FIFO_CLEAN_REG, t_before_reg | tclean_msk);

    bus_w(A_FIFO_CLEAN_REG, 0);
    bus_w(D_FIFO_CLEAN_REG, bus_r(D_FIFO_CLEAN_REG) & ~D_FIFO_CLEAN_MSK);
    bus_w(X_FIFO_CLEAN_REG, t_before_reg);
}

void resetFlow() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Resetting Core\n"));
    bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | RST_F_MSK);
    usleep(0);
    bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) & ~RST_F_MSK);
}

int waitTransceiverReset(char *mess) {
#ifndef VIRTUAL
    int resetTransceiverDone = (bus_r(TRANSCEIVERSTATUS) & RESETRXDONE_MSK);
    int times = 0;
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);
    while (resetTransceiverDone == 0) {
        if (times++ > WAIT_TIME_OUT_0US_TIMES) {
            clock_gettime(CLOCK_REALTIME, &end);
            int64_t timeNs = ((end.tv_sec - begin.tv_sec) * 1E9 +
                              (end.tv_nsec - begin.tv_nsec));

            sprintf(mess, "Resetting transceiver timed out, time:%.2fs\n",
                    (timeNs / (1E9)));
            LOG(logERROR, (mess));
            return FAIL;
        }
        usleep(0);
        resetTransceiverDone = (bus_r(TRANSCEIVERSTATUS) & RESETRXDONE_MSK);
    }
#endif
    LOG(logINFOBLUE, ("Transceiver reset done\n"));
    return OK;
}

int isTransceiverAligned() {
#ifdef VIRTUAL
    return 1;
#endif
    int times = 0;
    int retval = bus_r(TRANSCEIVERSTATUS2) & RXLOCKED_MSK;
    while (retval) {
        retval = bus_r(TRANSCEIVERSTATUS2) & RXLOCKED_MSK;
        times++;
        usleep(10);
        if (times == 5)
            return 1;
    }
    return retval;
}

/* set parameters -  dr */

int setDynamicRange(int dr) {
    if (dr == 16)
        return OK;
    return FAIL;
}

int getDynamicRange(int *retval) {
    *retval = DYNAMIC_RANGE;
    return OK;
}

void setADCEnableMask_10G(uint32_t mask) {
    // convert 32 bit mask to 8 bit mask
    uint8_t actualMask = 0;
    int ival = 0;
    for (int ich = 0; ich < NCHAN_ANALOG; ich = ich + 4) {
        if ((1 << ich) & mask) {
            actualMask |= (1 << ival);
        }
        ++ival;
    }

    LOG(logINFO, ("Setting adcEnableMask 10G to 0x%x (from 0x%08x)\n",
                  actualMask, mask));
    uint32_t addr = FIFO_TO_GB_CONTROL_REG;
    bus_w(addr, bus_r(addr) & (~ENABLED_CHANNELS_ADC_MSK));
    bus_w(addr, bus_r(addr) | ((actualMask << ENABLED_CHANNELS_ADC_OFST) &
                               ENABLED_CHANNELS_ADC_MSK));
}

uint32_t getADCEnableMask_10G() {
    uint32_t mask =
        ((bus_r(FIFO_TO_GB_CONTROL_REG) & ENABLED_CHANNELS_ADC_MSK) >>
         ENABLED_CHANNELS_ADC_OFST);

    // convert 8 bit mask to 32 bit mask
    uint32_t retval = 0;
    if (mask) {
        for (int ival = 0; ival < 8; ++ival) {
            // if bit in 8 bit mask set
            if ((1 << ival) & mask) {
                // set it for 4 bits in 32 bit mask
                for (int iloop = 0; iloop < 4; ++iloop) {
                    retval |= (1 << (ival * 4 + iloop));
                }
            }
        }
    }
    return retval;
}

int setTransceiverEnableMask(uint32_t mask) {
    if (mask > MAX_TRANSCEIVER_MASK) {
        LOG(logERROR, ("Invalid transceiver mask: 0x%x\n", mask));
        return FAIL;
    }
    LOG(logINFO, ("Setting transceivermask to 0x%x\n", mask));

    uint32_t addr = FIFO_TO_GB_CONTROL_REG;
    bus_w(addr, bus_r(addr) & ~ENABLED_CHANNELS_X_MSK);
    bus_w(addr, bus_r(addr) | ((mask << ENABLED_CHANNELS_X_OFST) &
                               ENABLED_CHANNELS_X_MSK));

    return OK;
}

uint32_t getTransceiverEnableMask() {
    return ((bus_r(FIFO_TO_GB_CONTROL_REG) & ENABLED_CHANNELS_X_MSK) >>
            ENABLED_CHANNELS_X_OFST);
}

/* parameters - readout */

int setReadoutMode(enum readoutMode mode) {
    analogEnable = 0, digitalEnable = 0, transceiverEnable = 0;
    switch (mode) {
    case ANALOG_ONLY:
        LOG(logINFO, ("Setting Analog Only Readout\n"));
        analogEnable = 1;
        break;
    case DIGITAL_ONLY:
        LOG(logINFO, ("Setting Digital Only Readout\n"));
        digitalEnable = 1;
        break;
    case ANALOG_AND_DIGITAL:
        LOG(logINFO, ("Setting Analog & Digital Readout\n"));
        analogEnable = 1;
        digitalEnable = 1;
        break;
    case TRANSCEIVER_ONLY:
        LOG(logINFO, ("Setting Transceiver Only Readout\n"));
        transceiverEnable = 1;
        break;
    case DIGITAL_AND_TRANSCEIVER:
        LOG(logINFO, ("Setting Digital & Transceiver Readout\n"));
        digitalEnable = 1;
        transceiverEnable = 1;
        break;
    default:
        LOG(logERROR, ("Cannot set unknown readout flag. 0x%x\n", mode));
        return FAIL;
    }

    uint32_t val = 0;
    if (analogEnable == 1) {
        val |= RO_MODE_ADC_MSK;
    }
    if (digitalEnable == 1) {
        val |= RO_MODE_D_MSK;
    }
    if (transceiverEnable == 1) {
        val |= RO_MODE_X_MSK;
    }

    uint32_t addr = FIFO_TO_GB_CONTROL_REG;
    bus_w(addr,
          bus_r(addr) & ~(RO_MODE_ADC_MSK | RO_MODE_D_MSK | RO_MODE_X_MSK));
    bus_w(addr, bus_r(addr) | val);

    return OK;
}

int getReadoutMode() {
    uint32_t retval = bus_r(FIFO_TO_GB_CONTROL_REG);
    if (retval & RO_MODE_ADC_MSK) {
        analogEnable = 1;
    }
    if (retval & RO_MODE_D_MSK) {
        digitalEnable = 1;
    }
    if (retval & RO_MODE_X_MSK) {
        transceiverEnable = 1;
    }

    if (analogEnable && digitalEnable && !transceiverEnable) {
        LOG(logDEBUG1, ("Getting readout: Analog & Digita\n"));
        return ANALOG_AND_DIGITAL;
    } else if (analogEnable && !digitalEnable && !transceiverEnable) {
        LOG(logDEBUG1, ("Getting readout: Analog Only\n"));
        return ANALOG_ONLY;
    } else if (!analogEnable && digitalEnable && !transceiverEnable) {
        LOG(logDEBUG1, ("Getting readout: Digital Only\n"));
        return DIGITAL_ONLY;
    } else if (!analogEnable && !digitalEnable && transceiverEnable) {
        LOG(logDEBUG1, ("Getting readout: Transceiver Only\n"));
        return TRANSCEIVER_ONLY;
    } else if (!analogEnable && digitalEnable && transceiverEnable) {
        LOG(logDEBUG1, ("Getting readout: Digital & Transceiver\n"));
        return DIGITAL_AND_TRANSCEIVER;
    } else {
        LOG(logERROR, ("Read unknown readout (analog enable:%d digital "
                       "enable:%d transceiver enable:%d)\n",
                       analogEnable, digitalEnable, transceiverEnable));
        return -1;
    }
}

/* parameters - timer */
int setNextFrameNumber(uint64_t value) {
    LOG(logINFO, ("Setting next frame number: %lu\n", value));
    setU64BitReg(value, LOCAL_FRAME_NUMBER_REG_1, LOCAL_FRAME_NUMBER_REG_2);
    return OK;
}

int getNextFrameNumber(uint64_t *retval) {
    *retval = getU64BitReg(LOCAL_FRAME_NUMBER_REG_1, LOCAL_FRAME_NUMBER_REG_2);
    return OK;
}

void setNumFrames(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of frames %ld\n", val));
        setU64BitReg(val, FRAMES_IN_REG_1, FRAMES_IN_REG_2);
    }
}

int64_t getNumFrames() {
    return getU64BitReg(FRAMES_IN_REG_1, FRAMES_IN_REG_2);
}

void setNumTriggers(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of triggers %ld\n", val));
        setU64BitReg(val, CYCLES_IN_REG_1, CYCLES_IN_REG_2);
    }
}

int64_t getNumTriggers() {
    return getU64BitReg(CYCLES_IN_REG_1, CYCLES_IN_REG_2);
}

int setNumAnalogSamples(int val) {
    if (val < 0 || val > MAX_ANALOG_SAMPLES) {
        LOG(logERROR, ("Invalid analog samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of analog samples %d\n", val));

    uint32_t addr = NO_SAMPLES_A_REG;
    bus_w(addr, bus_r(addr) & ~NO_SAMPLES_A_MSK);
    bus_w(addr, bus_r(addr) | ((val << NO_SAMPLES_A_OFST) & NO_SAMPLES_A_MSK));
    return OK;
}

int getNumAnalogSamples() {
    return ((bus_r(NO_SAMPLES_A_REG) & NO_SAMPLES_A_MSK) >> NO_SAMPLES_A_OFST);
}

int setNumDigitalSamples(int val) {
    if (val < 0 || val > MAX_DIGITAL_SAMPLES) {
        LOG(logERROR, ("Invalid digital samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of digital samples %d\n", val));

    uint32_t addr = NO_SAMPLES_D_REG;
    bus_w(addr, bus_r(addr) & ~NO_SAMPLES_D_MSK);
    bus_w(addr, bus_r(addr) | ((val << NO_SAMPLES_D_OFST) & NO_SAMPLES_D_MSK));
    return OK;
}

int getNumDigitalSamples() {
    return ((bus_r(NO_SAMPLES_D_REG) & NO_SAMPLES_D_MSK) >> NO_SAMPLES_D_OFST);
}

int setNumTransceiverSamples(int val) {
    if (val < 0 || val > MAX_TRANSCEIVER_SAMPLES) {
        LOG(logERROR, ("Invalid transceiver samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of transceiver samples %d\n", val));

    uint32_t addr = NO_SAMPLES_X_REG;
    bus_w(addr, bus_r(addr) & ~NO_SAMPLES_X_MSK);
    bus_w(addr, bus_r(addr) | ((val << NO_SAMPLES_X_OFST) & NO_SAMPLES_X_MSK));
    return OK;
}

int getNumTransceiverSamples() {
    return ((bus_r(NO_SAMPLES_X_REG) & NO_SAMPLES_X_MSK) >> NO_SAMPLES_X_OFST);
}

int setExpTime(int64_t val, char *mess) {
    setPatternWaitInterval(0, val);

    // validate
    uint64_t arg_clocks = ns_to_clocks(val, clkFrequency[RUN_CLK]);
    uint64_t retval_clocks = getPatternWaitClocks(0);
    if (arg_clocks != retval_clocks) {
        sprintf(mess,
                "Failed to set exposure time. Could not set number of clocks "
                "to %lld, read %lld\n",
                (long long int)arg_clocks, (long long int)retval_clocks);
        LOG(logERROR, (mess));
        return FAIL;
    }

    // log rounding if any
    int64_t retval = getPatternWaitInterval(0);
    if (val != retval) {
        LOG(logWARNING, ("Rounding to %lld ns due to clock frequency\n",
                         (long long int)retval));
    }

    return OK;
}

int getExpTime(int64_t *retval, char *mess) {
    *retval = getPatternWaitInterval(0);
    if (*retval == -1) {
        sprintf(mess, "Failed to get exposure time.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int setPeriod(int64_t val, char *mess) {
    if (val < 0) {
        sprintf(mess, "Invalid period: %lld ns\n", (long long int)val);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    uint64_t arg_clocks = ns_to_clocks(val, clkFrequency[RUN_CLK]);
    setU64BitReg(arg_clocks, PERIOD_IN_REG_1, PERIOD_IN_REG_2);

    // validate
    uint64_t retval_clocks = getU64BitReg(PERIOD_IN_REG_1, PERIOD_IN_REG_2);
    if (arg_clocks != retval_clocks) {
        sprintf(mess,
                "Failed to set period. Could not set number of clocks "
                "to %lld, red %lld\n",
                (long long int)arg_clocks, (long long int)retval_clocks);
        LOG(logERROR, (mess));
        return FAIL;
    }

    // log rounding if any
    int64_t retval = 0;
    if (getPeriod(&retval, mess) == FAIL) {
        return FAIL;
    }
    if (val != retval) {
        LOG(logWARNING, ("Rounding to %lld ns due to clock frequency\n",
                         (long long int)retval));
    }

    return OK;
}

int getPeriod(int64_t *retval, char *mess) {
    if (clkFrequency[RUN_CLK] == 0) {
        sprintf(mess, "Cannot get period. Run clock frequency is 0.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    uint64_t numClocks = getU64BitReg(PERIOD_IN_REG_1, PERIOD_IN_REG_2);
    *retval = clocks_to_ns(numClocks, clkFrequency[RUN_CLK]);
    return OK;
}

int setDelayAfterTrigger(int64_t val, char *mess) {
    if (val < 0) {
        sprintf(mess, "Invalid delay after trigger: %lld ns\n",
                (long long int)val);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("Setting delay after trigger %lld ns\n", (long long int)val));
    uint64_t arg_clocks = ns_to_clocks(val, clkFrequency[RUN_CLK]);
    setU64BitReg(arg_clocks, DELAY_IN_REG_1, DELAY_IN_REG_2);

    // validate
    uint64_t retval_clocks = getU64BitReg(DELAY_IN_REG_1, DELAY_IN_REG_2);
    if (arg_clocks != retval_clocks) {
        sprintf(
            mess,
            "Failed to set delay after trigger. Could not set number of clocks "
            "to %lld, read %lld\n",
            (long long int)arg_clocks, (long long int)retval_clocks);
        LOG(logERROR, (mess));
        return FAIL;
    }

    // log rounding if any
    int64_t retval = 0;
    if (getDelayAfterTrigger(&retval, mess) == FAIL) {
        return FAIL;
    }
    if (val != retval) {
        LOG(logWARNING, ("Rounding to %lld ns due to clock frequency\n",
                         (long long int)retval));
    }

    return OK;
}

int getDelayAfterTrigger(int64_t *retval, char *mess) {
    if (clkFrequency[RUN_CLK] == 0) {
        sprintf(mess,
                "Cannot get delay after trigger. Run clock frequency is 0.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    uint64_t numClocks = getU64BitReg(DELAY_IN_REG_1, DELAY_IN_REG_2);
    *retval = clocks_to_ns(numClocks, clkFrequency[RUN_CLK]);
    return OK;
}

int64_t getNumFramesLeft() {
    return getU64BitReg(FRAMES_OUT_REG_1, FRAMES_OUT_REG_2);
}

int64_t getNumTriggersLeft() {
    return getU64BitReg(CYCLES_OUT_REG_1, CYCLES_OUT_REG_2);
}

int getDelayAfterTriggerLeft(int64_t *retval, char *mess) {
    if (clkFrequency[RUN_CLK] == 0) {
        sprintf(mess, "Cannot get delay after trigger left. Run clock "
                      "frequency is 0.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    uint64_t numClocks = getU64BitReg(DELAY_OUT_REG_1, DELAY_OUT_REG_2);
    *retval = clocks_to_ns(numClocks, clkFrequency[RUN_CLK]);
    return OK;
}

int getPeriodLeft(int64_t *retval, char *mess) {
    if (clkFrequency[RUN_CLK] == 0) {
        sprintf(mess, "Cannot get period left. Run clock frequency is 0.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    uint64_t numClocks = getU64BitReg(PERIOD_OUT_REG_1, PERIOD_OUT_REG_2);
    *retval = clocks_to_ns(numClocks, clkFrequency[RUN_CLK]);
    return OK;
}

int64_t getFramesFromStart() {
    return getU64BitReg(FRAMES_FROM_START_OUT_REG_1,
                        FRAMES_FROM_START_OUT_REG_2);
}

int64_t getActualTime() {
    // in unit of 100ns
    return getU64BitReg(TIME_FROM_START_OUT_REG_1, TIME_FROM_START_OUT_REG_2) *
           100;
}

int64_t getMeasurementTime() {
    // in unit of 100ns
    return getU64BitReg(FRAME_TIME_OUT_REG_1, FRAME_TIME_OUT_REG_2) * 100;
}

/* parameters - dac, adc, hv */

int getVLimit() { return vLimit; }

int setVLimit(int val, char *mess) {
    if (val < 0) {
        sprintf(mess, "Could not set vlimit. Invalid value %d\n", val);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("Setting vlimit to %d mV\n", val));
    vLimit = val;
    return OK;
}

int validateDACIndex(enum DACINDEX ind, char *mess) {
    if (ind < 0 || ind >= NDAC_ONLY) {
        sprintf(mess, "Could not set DAC. Invalid index %d\n", ind);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int validateDACVoltage(enum DACINDEX ind, int voltage, char *mess) {
    // validate min value
    if (voltage < 0) {
        sprintf(mess,
                "Could not set DAC %d. Input value %d cannot be negative\n",
                ind, voltage);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // validate max value
    if (voltage > DAC_MAX_MV) {
        sprintf(
            mess,
            "Could not set DAC %d. Input value %d mV exceeds maximum %d mV\n",
            ind, voltage, DAC_MAX_MV);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // validate vlimit
    if (vLimit > 0 && voltage > vLimit) {
        sprintf(mess,
                "Could not set DAC %d. Input %d mV exceeds vLimit %d mV\n", ind,
                voltage, vLimit);
        LOG(logERROR, (mess))
        return FAIL;
    }
    return OK;
}

int convertVoltageToDAC(enum DACINDEX ind, int voltage, int *retval_dacval,
                        char *mess) {
    *retval_dacval = -1;
    if (LTC2620_D_VoltageToDac(voltage, retval_dacval) == FAIL) {
        sprintf(mess,
                "Could not set DAC %d. Could not convert %d mV to dac units.\n",
                (int)ind, voltage);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int convertDACToVoltage(enum DACINDEX ind, int dacval, int *retval_voltage,
                        char *mess) {
    *retval_voltage = -1;
    if (LTC2620_D_DacToVoltage(dacval, retval_voltage) == FAIL) {
        sprintf(mess,
                "Could not get DAC %d. Could not convert %d dac units to mV\n",
                (int)ind, dacval);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int getDAC(enum DACINDEX ind, bool mV, int *retval, char *mess) {
    *retval = -1;
    if (validateDACIndex(ind, mess) == FAIL)
        return FAIL;

    int dacval = dacValues[ind];
    if (dacval == LTC2620_D_GetPowerDownValue()) {
        LOG(logWARNING, ("DAC %d is in power down mode.\n", ind));
        *retval = dacval;
        return OK;
    }

    if (mV) {
        if (convertDACToVoltage(ind, dacval, retval, mess) == FAIL)
            return FAIL;
        return OK;
    }

    *retval = dacval;
    return OK;
}

int setDAC(enum DACINDEX ind, int val, bool mV, char *mess) {
    LOG(logINFO,
        ("Setting DAC %d: %d %s \n", ind, val, (mV ? "mV" : "dac units")));

    if (validateDACIndex(ind, mess) == FAIL)
        return FAIL;

    int dacval = val;
    if (mV) {
        if (validateDACVoltage(ind, val, mess) == FAIL)
            return FAIL;

        if (convertVoltageToDAC(ind, val, &dacval, mess) == FAIL)
            return FAIL;
    }

    {
        char dacName[20] = {0};
        snprintf(dacName, sizeof(dacName), "dac %d", ind);
        if (LTC2620_D_SetDacValue((int)ind, dacval, dacName, mess) == FAIL)
            return FAIL;
    }
    dacValues[ind] = dacval;
    return OK;
}

int validatePowerDACIndex(enum powerIndex ind, char *mess) {
    if (ind < 0 || ind > V_POWER_IO) {
        sprintf(mess, "Could not set Power DAC. Invalid index %d\n", ind);
        LOG(logERROR, (mess));
        return FAIL;
    }

    return OK;
}

int validatePower(enum powerIndex ind, int voltage, char *mess) {
    char *powerNames[] = {PWR_NAMES};

    // validate min value
    int min = (ind == V_POWER_IO) ? VIO_MIN_MV : POWER_RGLTR_MIN;
    if (voltage < min && voltage != 0) {
        sprintf(
            mess,
            "Could not set %s. Input value %d mV must be greater than %d mV.\n",
            powerNames[ind], voltage, min);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // validate max value
    if (voltage > POWER_RGLTR_MAX) {
        sprintf(
            mess,
            "Could not set %s. Input value %d mV must be less than %d mV.\n",
            powerNames[ind], voltage, POWER_RGLTR_MAX);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // validate vlimit
    if (vLimit > 0 && voltage > vLimit) {
        sprintf(mess, "Could not set %s. Input %d mV exceeds vLimit %d mV\n",
                powerNames[ind], voltage, vLimit);
        LOG(logERROR, (mess))
        return FAIL;
    }
    return OK;
}

int convertVoltageToPowerDAC(enum powerIndex ind, int voltage,
                             int *retval_dacval, char *mess) {
    *retval_dacval = -1;
    if (ConvertToDifferentRange(
            POWER_RGLTR_MIN, POWER_RGLTR_MAX, LTC2620_D_GetMaxInput(),
            LTC2620_D_GetMinInput(), voltage, retval_dacval) == FAIL) {
        char *powerNames[] = {PWR_NAMES};
        sprintf(mess,
                "Could not set %s. Could not convert %d mV to dac units.\n",
                powerNames[ind], voltage);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int convertPowerDACToVoltage(enum powerIndex ind, int dacval,
                             int *retval_voltage, char *mess) {
    *retval_voltage = -1;
    if (ConvertToDifferentRange(
            LTC2620_D_GetMaxInput(), LTC2620_D_GetMinInput(), POWER_RGLTR_MIN,
            POWER_RGLTR_MAX, dacval, retval_voltage) == FAIL) {
        char *powerNames[] = {PWR_NAMES};
        sprintf(mess,
                "Could not get %s. Could not convert %d dac units to mV\n",
                powerNames[ind], dacval);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int getPowerDAC(enum powerIndex ind, int *retval, char *mess) {
    *retval = -1;
    if (validatePowerDACIndex(ind, mess) == FAIL)
        return FAIL;

    int dacval = powerValues[ind];
    if (convertPowerDACToVoltage(ind, dacval, retval, mess) == FAIL)
        return FAIL;

    return OK;
}

int setPowerDAC(enum powerIndex ind, int voltage, char *mess) {
    if (validatePowerDACIndex(ind, mess) == FAIL)
        return FAIL;

    char *powerNames[] = {PWR_NAMES};
    LOG(logINFO, ("Setting DAC %s: %d mV\n", powerNames[ind], voltage));

    if (validatePower(ind, voltage, mess) == FAIL)
        return FAIL;

    int dacval = -1;
    if (convertVoltageToPowerDAC(ind, voltage, &dacval, mess) == FAIL)
        return FAIL;

    {
        enum DACINDEX dacIndex = D_PWR_IO;
        if (getDACIndexForPower(ind, &dacIndex, mess) == FAIL) {
            return FAIL;
        }

        if (LTC2620_D_SetDacValue(dacIndex, dacval, powerNames[ind], mess) ==
            FAIL)
            return FAIL;
    }

    powerValues[ind] = dacval;
    return OK;
}

int getDACIndexForPower(enum powerIndex pind, enum DACINDEX *dacIndex,
                        char *mess) {
    switch (pind) {
    case V_POWER_IO:
        *dacIndex = D_PWR_IO;
        break;
    case V_POWER_A:
        *dacIndex = D_PWR_A;
        break;
    case V_POWER_B:
        *dacIndex = D_PWR_B;
        break;
    case V_POWER_C:
        *dacIndex = D_PWR_C;
        break;
    case V_POWER_D:
        *dacIndex = D_PWR_D;
        break;
    default:
        *dacIndex = -1;
        sprintf(mess, "Power index %d has no corresponding dac index\n", pind);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int getPowerMask(enum powerIndex index, uint32_t *mask, char *mess) {
    switch (index) {
    case V_POWER_IO:
        *mask |= POWER_VIO_MSK;
        break;
    case V_POWER_A:
        *mask |= POWER_VCC_A_MSK;
        break;
    case V_POWER_B:
        *mask |= POWER_VCC_B_MSK;
        break;
    case V_POWER_C:
        *mask |= POWER_VCC_C_MSK;
        break;
    case V_POWER_D:
        *mask |= POWER_VCC_D_MSK;
        break;
    default:
        sprintf(mess, "Index %d has no power rail index\n", index);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

void powerOff() {
    LOG(logINFOBLUE, ("Powering OFF all rails\n"));
    uint32_t mask = POWER_VIO_MSK | POWER_VCC_A_MSK | POWER_VCC_B_MSK |
                    POWER_VCC_C_MSK | POWER_VCC_D_MSK;
    bus_w(CTRL_REG, bus_r(CTRL_REG) & ~(mask));
}

int setPowerEnabled(enum powerIndex indices[], int count, bool enable,
                    char *mess) {
    uint32_t mask = 0;
    for (int i = 0; i != count; ++i) {
        if (getPowerMask(indices[i], &mask, mess) == FAIL)
            return FAIL;
    }
    // log message
    {
        char *powerNames[] = {PWR_NAMES};
        char message[256] = {0};
        sprintf(message, "Switching %s power for [", enable ? "on" : "off");
        for (int i = 0; i != count; ++i) {
            strcat(message, powerNames[indices[i]]);
            strcat(message, ", ");
        }
        strcat(message, "]\n");
        LOG(logINFO, ("%s", message));
    }
    // enable/disable power rails
    uint32_t addr = CTRL_REG;
    if (enable) {
        bus_w(addr, bus_r(addr) | mask);
    } else {
        bus_w(addr, bus_r(addr) & ~(mask));
    }
    return OK;
}

int isPowerEnabled(enum powerIndex ind, bool *retval, char *mess) {
    uint32_t mask = 0;
    if (getPowerMask(ind, &mask, mess) == FAIL)
        return FAIL;

    *retval = (bus_r(CTRL_REG) & mask) != 0;
    LOG(logDEBUG1, ("get power %d:%d\n", ind, *retval));
    return OK;
}

int getADC(enum ADCINDEX ind, int *value, char *mess) {
    *value = 0;
    switch (ind) {
        // slow adcs
    case S_ADC0:
    case S_ADC1:
    case S_ADC2:
    case S_ADC3:
    case S_ADC4:
    case S_ADC5:
    case S_ADC6:
    case S_ADC7:
        LOG(logDEBUG1, ("Reading Slow ADC Channel %d\n", (int)ind - S_ADC0));
        return getSlowADC((int)ind - S_ADC0, value, mess);
    case TEMP_FPGA:
        LOG(logDEBUG1, ("Reading FPGA Temperature\n"));
        return getTemperature(value, mess);
    default:
        snprintf(mess, MAX_STR_LENGTH, "Adc Index %d not defined\n", (int)ind);
        LOG(logERROR, (mess));
        return FAIL;
    }
}

int getSlowADC(int ichan, int *retval, char *mess) {
    *retval = 0;
    int fval = 0;

#ifdef VIRTUAL
    fval = 1;
#else
    char fname[MAX_STR_LENGTH];
    memset(fname, 0, MAX_STR_LENGTH);
    sprintf(fname, SLOWADC_DRIVER_FILE_NAME, ichan);
    LOG(logDEBUG1, ("fname %s\n", fname));
    if (readParameterFromFile(fname, "slow adc", &fval) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH, "Could not read slow adc channel %d\n",
                 ichan);
        LOG(logERROR, (mess));
        return FAIL;
    }
#endif

    // value in uV
    int refMaxuv = SLOW_ADC_MAX_MV * 1000;
    int regMinuv = 0;
    int maxSteps = SLOW_ADC_MAX_STEPS;
    if (ConvertToDifferentRange(0, maxSteps, regMinuv, refMaxuv, fval,
                                retval) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not convert slow adc channel (fval:0x%x) to uv\n",
                 fval);
        LOG(logERROR, (mess));
        return -1;
    }

    LOG(logINFO,
        ("\tSlow adc [%d]: %d uV (reg: 0x%x)\n", ichan, *retval, fval));
    return OK;
}

int getTemperature(int *retval, char *mess) {
    *retval = 0;
#ifndef VIRTUAL
    if (readParameterFromFile(TEMP_DRIVER_FILE_NAME, "temperature", retval) ==
        FAIL) {
        snprintf(mess, MAX_STR_LENGTH, "Could not read temperature\n");
        LOG(logERROR, ("Could not get temperature\n"));
        return FAIL;
    }
    LOG(logINFO, ("Temperature: %.2f °C\n", (double)(*retval) / 1000.00));
#endif
    return OK;
}

/* parameters - timing, extsig */

void setTiming(enum timingMode arg) {
    switch (arg) {
    case AUTO_TIMING:
        LOG(logINFO, ("Set Timing: Auto\n"));
        bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) & ~TRIGGER_ENABLE_MSK);
        bus_w(GATE_CTRL, bus_r(GATE_CTRL) & ~GATE_ENABLE_MSK);
        break;
    case TRIGGER_EXPOSURE:
        LOG(logINFO, ("Set Timing: Trigger\n"));
        bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | TRIGGER_ENABLE_MSK);
        bus_w(GATE_CTRL, bus_r(GATE_CTRL) & ~GATE_ENABLE_MSK);
        break;
    case GATED:
        //   chip-specific config needed, see example reg definitions in matterhorn chiptesting repo
        // - a gate mask setting to specify which bits of the pattern are gated (GATE_MASK_LSB & GATE_MASK_MSB)
        // - value of the pattern bit when gating is enabled but gate is closed (GATE_VALUE_WHEN_CLOSED)
        // - a gate inversion setting (GATE_INVERT_MSK)
        LOG(logINFO, ("Set Timing: Gated\n"));
        bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) & ~TRIGGER_ENABLE_MSK);
        bus_w(GATE_CTRL, bus_r(GATE_CTRL) | GATE_ENABLE_MSK);
        break;
    case TRIGGER_GATED:
        // trigger + gate --> trigger will start a pattern and the output of the pattern is gated.
        // NOT A GATED TRIGGER !! (this is not: "trigger starts a pattern only if the gate is open")
        LOG(logINFO, ("Set Timing: Trigger Gated\n"));
        bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | TRIGGER_ENABLE_MSK);
        bus_w(GATE_CTRL, bus_r(GATE_CTRL) | GATE_ENABLE_MSK);
        break;
    default:
        LOG(logERROR, ("Unknown timing mode %d\n", arg));
    }
}

enum timingMode getTiming() {
    uint32_t extTrigger = (bus_r(FLOW_CONTROL_REG) & TRIGGER_ENABLE_MSK);
    uint32_t extGate = (bus_r(GATE_CTRL) & GATE_ENABLE_MSK);

    if (extTrigger) {
        if (extGate) {
            return TRIGGER_GATED;
        }
        return TRIGGER_EXPOSURE;
    }

    if (extGate) {
        return GATED;
    }
    return AUTO_TIMING;
}

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));
    // TODO
    return OK;
}

int *getDetectorPosition() { return detPos; }

int getNumberofUDPInterfaces() { return 1; }

void calcChecksum(udp_header *udp) {
    int count = IP_HEADER_SIZE;
    long int sum = 0;

    // start at ip_tos as the memory is not continous for ip header
    uint16_t *addr = (uint16_t *)(&(udp->ip_tos));

    sum += *addr++;
    count -= 2;

    // ignore ethertype (from udp header)
    addr++;
    // ignore udp_srcmac_lsb (from udp header)
    addr++;
    addr++;

    // from ip_protocol to ip_checksum
    while (count > 2) {
        sum += *addr++;
        count -= 2;
    }

    // ignore udp_checksum (from udp header)
    addr++;
    // ignore udp_destport (from udp header)
    addr++;
    // ignore udp_srcport (from udp header)
    addr++;

    if (count > 0)
        sum += *addr; // Add left-over byte, if any
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16); // Fold 32-bit sum to 16 bits
    long int checksum = sum & 0xffff;
    checksum += UDP_IP_HEADER_LENGTH_BYTES;
    udp->ip_checksum = checksum;
    LOG(logINFO, ("\tIP checksum: 0x%x\n", checksum));
}

int configureMAC() {
    uint32_t srcip = udpDetails[0].srcip;
    uint32_t dstip = udpDetails[0].dstip;
    uint64_t srcmac = udpDetails[0].srcmac;
    uint64_t dstmac = udpDetails[0].dstmac;
    uint16_t srcport = udpDetails[0].srcport;
    uint16_t dstport = udpDetails[0].dstport;

    LOG(logINFOBLUE, ("Configuring MAC\n"));
    char src_mac[MAC_ADDRESS_SIZE], src_ip[INET_ADDRSTRLEN],
        dst_mac[MAC_ADDRESS_SIZE], dst_ip[INET_ADDRSTRLEN];
    getMacAddressinString(src_mac, MAC_ADDRESS_SIZE, srcmac);
    getMacAddressinString(dst_mac, MAC_ADDRESS_SIZE, dstmac);
    getIpAddressinString(src_ip, srcip);
    getIpAddressinString(dst_ip, dstip);

    LOG(logINFO, ("\tSource IP   : %s\n"
                  "\tSource MAC  : %s\n"
                  "\tSource Port : %hu\n"
                  "\tDest IP     : %s\n"
                  "\tDest MAC    : %s\n"
                  "\tDest Port   : %hu\n",
                  src_ip, src_mac, srcport, dst_ip, dst_mac, dstport));

#ifdef VIRTUAL
    if (setUDPDestinationDetails(0, 0, dst_ip, dstport) == FAIL) {
        LOG(logERROR, ("could not set udp destination IP and port\n"));
        return FAIL;
    }
#endif

    // get struct memory
    udp_header *udp = (udp_header *)(Arm_getUDPBaseAddress());
    memset(udp, 0, sizeof(udp_header));

    //  mac addresses
    // msb (32) + lsb (16)
    udp->udp_destmac_msb = ((dstmac >> 16) & BIT32_MASK);
    udp->udp_destmac_lsb = ((dstmac >> 0) & BIT16_MASK);
    // msb (16) + lsb (32)
    udp->udp_srcmac_msb = ((srcmac >> 32) & BIT16_MASK);
    udp->udp_srcmac_lsb = ((srcmac >> 0) & BIT32_MASK);

    // ip addresses
    udp->ip_srcip_msb = ((srcip >> 16) & BIT16_MASK);
    udp->ip_srcip_lsb = ((srcip >> 0) & BIT16_MASK);
    udp->ip_destip_msb = ((dstip >> 16) & BIT16_MASK);
    udp->ip_destip_lsb = ((dstip >> 0) & BIT16_MASK);

    // source port
    udp->udp_srcport = srcport;
    udp->udp_destport = dstport;

    // other defines
    udp->udp_ethertype = 0x800;
    udp->ip_ver = 0x4;
    udp->ip_ihl = 0x5;
    udp->ip_flags = 0x2; // FIXME
    udp->ip_ttl = 0x40;
    udp->ip_protocol = 0x11;
    // total length is redefined in firmware

    calcChecksum(udp);

    return OK;
}

/* aquisition */

int startStateMachine() {
#ifdef VIRTUAL
    // create udp socket
    if (createUDPSocket(0) != OK) {
        return FAIL;
    }
    LOG(logINFOBLUE, ("Starting State Machine\n"));
    if (sharedMemory_getStop() != 0) {
        LOG(logERROR, ("Cant start acquisition. "
                       "Stop server has not updated stop status to 0\n"));
        return FAIL;
    }
    sharedMemory_setStatus(RUNNING);
    if (pthread_create(&pthread_virtual_tid, NULL, &start_timer, NULL)) {
        LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
        sharedMemory_setStatus(IDLE);
        return FAIL;
    } else
        pthread_detach(pthread_virtual_tid);
    LOG(logINFOGREEN, ("Virtual Acquisition started\n"));
    return OK;
#endif

    LOG(logINFOBLUE, ("Starting readout\n"));
    bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | START_F_MSK);
    return OK;
}

#ifdef VIRTUAL
void *start_timer(void *arg) {
    if (!isControlServer) {
        return NULL;
    }
    int64_t periodNs = 0;
    int64_t expUs = 0;
    {
        char mess[MAX_STR_LENGTH] = {0};
        if (getPeriod(&periodNs, mess) == FAIL) {
            LOG(logERROR, ("Failed to get period.\n"));
            return NULL;
        }
        if (getExpTime(&expUs, mess) == FAIL) {
            LOG(logERROR, ("Failed to get exposure time.\n"));
            return NULL;
        }
        expUs /= 1000;
    }

    int numFrames = (getNumFrames() * getNumTriggers());
    int imageSize = calculateDataBytes();
    int maxDataSize = MAX_DATA_SIZE_IN_PACKET;
    int packetSize = sizeof(sls_detector_header) + maxDataSize;
    int packetsPerFrame = ceil((double)imageSize / (double)maxDataSize);

    LOG(logDEBUG1, ("period: %lld ns, exp: %lld us, numFrames: %d, "
                    "imageSize: %d, maxDataSize: %d, packetsize: %d, "
                    "packetsPerFrame: %d\n",
                    periodNs, expUs, numFrames, imageSize, maxDataSize,
                    packetSize, packetsPerFrame));

    // Generate Data
    char *imageData = (char *)malloc(imageSize);
    memset(imageData, 0, imageSize);
    if (imageData == NULL) {
        LOG(logERROR, ("Can not allocate image.\n"));
        return NULL;
    }
    for (int i = 0; i < imageSize; i += sizeof(uint16_t)) {
        *((uint16_t *)(imageData + i)) = i;
    }

    // Send data
    uint64_t frameNr = 0;
    getNextFrameNumber(&frameNr);
    // loop over number of frames
    for (int iframes = 0; iframes != numFrames; ++iframes) {

        // check if manual stop
        if (sharedMemory_getStop() == 1) {
            setNextFrameNumber(frameNr + iframes + 1);
            break;
        }

        // sleep for exposure time
        struct timespec begin, end;
        clock_gettime(CLOCK_REALTIME, &begin);
        usleep(expUs);

        int srcOffset = 0;
        int dataSent = 0;
        // loop packet
        for (int i = 0; i != packetsPerFrame; ++i) {

            char packetData[packetSize];
            memset(packetData, 0, packetSize);
            // set header
            sls_detector_header *header = (sls_detector_header *)(packetData);
            header->detType = (uint16_t)myDetectorType;
            header->version = SLS_DETECTOR_HEADER_VERSION;
            header->frameNumber = frameNr + iframes;
            header->packetNumber = i;
            header->modId = 0;
            header->row = detPos[Y];
            header->column = detPos[X];

            // fill data
            int remaining = imageSize - dataSent;
            int dataSize = remaining < maxDataSize ? remaining : maxDataSize;
            memcpy(packetData + sizeof(sls_detector_header),
                   imageData + srcOffset, dataSize);
            srcOffset += dataSize;
            dataSent += dataSize;

            dataSize = (srcOffset + dataSize > imageSize)
                           ? (imageSize - srcOffset)
                           : dataSize;

            sendUDPPacket(0, 0, packetData, packetSize);
        }
        LOG(logINFO, ("Sent frame: %d [%lld]\n", iframes, frameNr + iframes));
        clock_gettime(CLOCK_REALTIME, &end);
        int64_t timeNs =
            ((end.tv_sec - begin.tv_sec) * 1E9 + (end.tv_nsec - begin.tv_nsec));

        // sleep for (period - exptime)
        if (iframes < numFrames) { // if there is a next frame
            if (periodNs > timeNs) {
                usleep((periodNs - timeNs) / 1000);
            }
        }
    }
    setNextFrameNumber(frameNr + numFrames);
    closeUDPSocket(0);

    sharedMemory_setStatus(IDLE);
    LOG(logINFOBLUE, ("Transmitting frames done\n"));
    return NULL;
}
#endif

int stopStateMachine() {
    LOG(logINFORED, ("Stopping State Machine\n"));
    // if scan active, stop scan
    if (sharedMemory_getScanStatus() == RUNNING) {
        sharedMemory_setScanStop(1);
    }
#ifdef VIRTUAL
    sharedMemory_setStop(1);
    // read till status is idle
    while (sharedMemory_getStatus() == RUNNING)
        usleep(500);
    sharedMemory_setStop(0);
    LOG(logINFO, ("Stopped State Machine\n"));
    return OK;
#endif
    // stop state machine
    bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | STOP_F_MSK);
    cleanFifos();
    return OK;
}

int softwareTrigger() {
#ifndef VIRTUAL
    // ready for trigger
    if (getRunStatus() != WAITING) {
        LOG(logWARNING, ("Not yet ready for trigger!\n"));
        return FAIL;
    }
#endif
    LOG(logINFO, ("Sending Software Trigger\n"));
    bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | SW_TRIGGER_F_MSK);
    // wait to make sure its out of this state and even 'wait for start frame'
    // TODO: usleep(100);

    return OK;
}

enum runStatus getRunStatus() {
    LOG(logDEBUG1, ("Getting status\n"));
    // scan error or running
    if (sharedMemory_getScanStatus() == ERROR) {
        LOG(logINFOBLUE, ("Status: scan ERROR\n"));
        return ERROR;
    }
    if (sharedMemory_getScanStatus() == RUNNING) {
        LOG(logINFOBLUE, ("Status: scan RUNNING\n"));
        return RUNNING;
    }
#ifdef VIRTUAL
    if (sharedMemory_getStatus() == RUNNING) {
        LOG(logINFOBLUE, ("Status: RUNNING\n"));
        return RUNNING;
    }
    LOG(logINFOBLUE, ("Status: IDLE\n"));
    return IDLE;
#endif
    uint32_t retval = bus_r(STATUS_REG);
    LOG(logINFO, ("Status Register: %08x\n", retval));

    if (retval == 0x0) {
        return IDLE;
    }

    if (retval & RX_NOT_GOOD_MSK) {
        LOG(logINFOBLUE, ("Status: ERROR\n"));
        return ERROR;
    }

    if (retval & WAIT_FOR_TRIGGER_MSK) {
        LOG(logINFOBLUE, ("Status: WAITING\n"));
        return WAITING;
    }

    LOG(logINFOBLUE, ("Status: RUNNING\n"));
    return RUNNING;
    // TODO: STOPPED?
}

u_int32_t runBusy() {
#ifdef VIRTUAL
    return ((sharedMemory_getStatus() == RUNNING) ? 1 : 0);
#endif
    return (bus_r(STATUS_REG));
}

void waitForAcquisitionEnd() {
    while (runBusy()) {
        usleep(500);
    }
#ifndef VIRTUAL
    int64_t retval = getNumFramesLeft() + 1;
    if (retval > 0) {
        LOG(logINFORED, ("%lld frames left\n", (long long int)retval));
    }
#endif
    LOG(logINFOGREEN, ("Blocking Acquisition done\n"));
}

int calculateDataBytes() {
    int analogDataBytes = 0;
    int digitalDataBytes = 0;
    int transceiverDataBytes = 0;
    int nachans = 0, ndchans = 0, ntchans = 0;

    if (analogEnable) {
        nachans = __builtin_popcount(getADCEnableMask_10G());
        analogDataBytes = nachans * (DYNAMIC_RANGE / 8) * getNumAnalogSamples();
        LOG(logINFO, ("\t#Analog Channels:%d, Databytes:%d\n", nachans,
                      analogDataBytes));
    }

    if (digitalEnable) {
        ndchans = 64;
        digitalDataBytes = (sizeof(uint64_t) * getNumDigitalSamples());
        LOG(logINFO, ("\t#Digital Channels:%d, Databytes:%d\n", ndchans,
                      digitalDataBytes));
    }

    if (transceiverEnable) {
        ntchans = __builtin_popcount(getTransceiverEnableMask());
        transceiverDataBytes =
            ntchans * (NBITS_PER_TRANSCEIVER / 8) * getNumTransceiverSamples();
        LOG(logINFO, ("\t#Transceiver Channels:%d, Databytes:%d\n", ntchans,
                      transceiverDataBytes));
    }

    // total
    int nchans = nachans + ndchans + ntchans;
    int dataBytes = analogDataBytes + digitalDataBytes + transceiverDataBytes;

    LOG(logINFO,
        ("\t#Total Channels:%d, Total Databytes:%d\n", nchans, dataBytes));
    return dataBytes;
}

int getTotalNumberOfChannels() {
    int nchanx = 0, nchany = 0;
    getNumberOfChannels(&nchanx, &nchany);
    return nchanx * nchany;
}

void getNumberOfChannels(int *nchanx, int *nchany) {
    int nachans = 0, ndchans = 0, ntchans = 0;

    if (analogEnable) {
        nachans = __builtin_popcount(getADCEnableMask_10G());
        LOG(logDEBUG1, ("Analog Channels: %d\n", nachans));
    }

    if (digitalEnable) {
        ndchans = 64;
        LOG(logDEBUG, ("Digital Channels: %d\n", ndchans));
    }

    if (transceiverEnable) {
        ntchans = __builtin_popcount(getTransceiverEnableMask());
        LOG(logDEBUG1, ("Transceiver Channels: %d\n", ntchans));
    }
    *nchanx = nachans + ndchans + ntchans;
    LOG(logDEBUG1, ("Total #Channels: %d\n", *nchanx));
    *nchany = 1;
}

int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }

int setFrequency(enum CLKINDEX ind, int val) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to set frequency\n", ind));
        return FAIL;
    }
    if (val <= 0) {
        return FAIL;
    }

    char *clock_names[] = {CLK_NAMES};
    LOG(logINFO, ("\tSetting %s clock (%d) frequency to %d Hz\n",
                  clock_names[ind], ind, val));

    if (XILINX_PLL_setFrequency(ind, val) == FAIL) {
        LOG(logERROR, ("\tCould not set %s clock (%d) frequency to %d Hz\n",
                       clock_names[ind], ind, val));
        return FAIL;
    }
    clkFrequency[ind] = val;
    // TODO later: connect setPhase as phase gets reset on freq change
    return OK;
}

int getFrequency(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get frequency\n", ind));
        return -1;
    }
#ifndef VIRTUAL
    clkFrequency[ind] = XILINX_PLL_getFrequency(ind);
#endif
    return clkFrequency[ind];
}
