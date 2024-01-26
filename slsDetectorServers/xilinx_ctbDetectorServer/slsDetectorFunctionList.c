// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "arm64.h"
#include "programViaArm.h"
#include "clogger.h"
#include "common.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#include "loadPattern.h"
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include <arpa/inet.h> // INET_ADDRSTRLEN
#include <string.h>
#include <unistd.h> // usleep

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
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

int chipConfigured = 0;
int analogEnable = 0;
int digitalEnable = 0;
int transceiverEnable = 0;

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
    if (val == FIXEDPATTERNVAL) {
        LOG(logINFO, ("\tFixed pattern: successful match (0x%08x)\n", val));
    } else {
        LOG(logERROR,
            ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n",
             val, FIXEDPATTERNVAL));
        return FAIL;
    }
#endif
    LOG(logINFO, ("\tSuccessfully read FPGA Fixed Pattern (0x%x)\n", FIXEDPATTERNVAL));
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
        sharedMemory_setStop(0);
#endif
    }
    initCheckDone = 1;
}

/* set up detector */

void setupDetector() {
    LOG(logINFO, ("Setting up Server for 1 Xilinx Chip Test Board\n"));

    // default variables
    chipConfigured = 0;
    analogEnable = 0;
    digitalEnable = 0;
    transceiverEnable = 0;
    
#ifdef VIRTUAL
    sharedMemory_setStatus(IDLE);
    setupUDPCommParameters();
    initializePatternWord();
#endif
    // initialization only at start up (restart fpga)
    initError = waitTranseiverReset(initErrorMessage);
    if (initError == FAIL) {
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
    setExpTime(DEFAULT_EXPTIME);
    setPeriod(DEFAULT_PERIOD);
}

/* firmware functions (resets) */

void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Clearing Acquisition Fifos\n"));
    bus_w(A_FIFO_CLEAN_REG, bus_r(A_FIFO_CLEAN_REG) | BIT32_MSK);
    bus_w(D_FIFO_CLEAN_REG, bus_r(D_FIFO_CLEAN_REG) | D_FIFO_CLEAN_MSK);
    bus_w(X_FIFO_CLEAN_REG, bus_r(X_FIFO_CLEAN_REG) | X_FIFO_CLEAN_MSK);

    bus_w(A_FIFO_CLEAN_REG, 0);
    bus_w(D_FIFO_CLEAN_REG, bus_r(D_FIFO_CLEAN_REG) & ~D_FIFO_CLEAN_MSK);
    bus_w(X_FIFO_CLEAN_REG, bus_r(X_FIFO_CLEAN_REG) & ~X_FIFO_CLEAN_MSK);

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

int waitTranseiverReset(char* mess) {
#ifndef VIRTUAL
    int resetTransceiverDone = (bus_r(TRANSCEIVERSTATUS) & RESETRXDONE_MSK);
	int times = 0;
    while (resetTransceiverDone == 0) {
        if (times++ > WAIT_TIME_OUT_0US_TIMES) {
            sprintf(mess, "Resetting transceiver timed out\n");
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

int isTransceiverAligned() {return (bus_r(TRANSCEIVERSTATUS) & RXBYTEISALIGNED_MSK);}

int waitTransceiverAligned(char* mess) {
#ifndef VIRTUAL
    int transceiverWordAligned = isTransceiverAligned();
	int times = 0;
    while (transceiverWordAligned == 0) {
        if (times++ > WAIT_TIME_OUT_0US_TIMES) {
            sprintf(mess, "Transceiver alignment timed out\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
        usleep(0);
        transceiverWordAligned = isTransceiverAligned();
    }
#endif
    LOG(logINFOBLUE, ("Transceiver alignment done\n"));
    return OK;
}

int configureTransceiver(char* mess) {
    LOG(logINFOBLUE, ("\tConfiguring transceiver\n"));
    
    if (chipConfigured == 0) {
        sprintf(mess, "Chip not configured. Use powerchip to power on chip first.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    return waitTransceiverAligned(mess);
}

int isChipConfigured() { return chipConfigured; }

//TODO powerchip and configurechip should be separate commands (not requirement) in the future
int powerChip(int on, char* mess) {
    uint32_t addr = CTRL_REG;
    uint32_t mask = POWER_VIO_MSK | POWER_VCC_A_MSK | POWER_VCC_B_MSK |
                    POWER_VCC_C_MSK | POWER_VCC_D_MSK;
    if (on) {
        LOG(logINFOBLUE, ("Powering chip: on\n"));
        bus_w(addr, bus_r(addr) | mask);

        if (configureChip(mess) == FAIL)
            return FAIL;

        startPeriphery();

        chipConfigured = 1;
    } else {
        LOG(logINFOBLUE, ("Powering chip: off\n"));
        bus_w(addr, bus_r(addr) & ~mask);

        chipConfigured = 0;

#ifndef VIRTUAL
        // transceiver alignment should be reset at power off
        if (isTransceiverAligned()) {
            sprintf(mess, "Transceiver alignment not reset\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tTransceiver alignment has been reset\n"));
    }
#endif
    return OK;
}

int getPowerChip() {
    uint32_t addr = CTRL_REG;
    uint32_t mask = POWER_VIO_MSK | POWER_VCC_A_MSK | POWER_VCC_B_MSK |
                    POWER_VCC_C_MSK | POWER_VCC_D_MSK;
    return ((bus_r(addr) & mask) == mask);
}

int configureChip(char* mess) {
    LOG(logINFOBLUE, ("\tConfiguring chip\n"));
    
    // enable correct endianness (Only for MH_PR_2)
	//uint32_t addr = MATTERHORNSPIREG1;
    //bus_w(addr, bus_r(addr) &~MATTERHORNSPI1_MSK);
    //bus_w(addr, bus_r(addr) | ((0x40000 << MATTERHORNSPI1_OFST) & MATTERHORNSPI1_MSK));
	
    // start configuration
    uint32_t addr = MATTERHORNSPICTRL;
    bus_w(addr, bus_r(addr) | CONFIGSTART_MSK);
    bus_w(addr, bus_r(addr) & ~CONFIGSTART_MSK);

	// wait until configuration is done
#ifndef VIRTUAL
	int configDone =  (bus_r(MATTERHORNSPICTRL) & BUSY_MSK);
	int times = 0;
	while (configDone == 0) {
        if (times++ > WAIT_TIME_OUT_0US_TIMES) {
            sprintf(mess, "Configuration of chip timed out\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
        usleep(0);
        configDone = (bus_r(MATTERHORNSPICTRL) & BUSY_MSK);
    }
#endif
    LOG(logINFOBLUE, ("\tChip configured\n"));
    return OK;
}

void startPeriphery() {
    LOG(logINFOBLUE, ("\tStarting periphery\n"));
    //TODO
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
        bus_w(addr, bus_r(addr) |
                        ((actualMask << ENABLED_CHANNELS_ADC_OFST) &
                         ENABLED_CHANNELS_ADC_MSK));
}

uint32_t getADCEnableMask_10G() {
    uint32_t mask = ((bus_r(FIFO_TO_GB_CONTROL_REG) & ENABLED_CHANNELS_ADC_MSK) >>
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
    if (mask < 0 || mask > MAX_TRANSCEIVER_MASK) {
        LOG(logERROR, ("Invalid transceiver mask: 0x%x\n", mask));
        return FAIL;
    }
    LOG(logINFO, ("Setting transceivermask to 0x%x\n", mask));

    uint32_t addr = FIFO_TO_GB_CONTROL_REG;
    bus_w(addr, bus_r(addr) & ~ENABLED_CHANNELS_X_MSK);
    bus_w(addr, bus_r(addr) | ((mask  << ENABLED_CHANNELS_X_OFST) & ENABLED_CHANNELS_X_MSK));

    return OK;
}

uint32_t getTransceiverEnableMask() { 
    return ((bus_r(FIFO_TO_GB_CONTROL_REG) & ENABLED_CHANNELS_X_MSK) >> ENABLED_CHANNELS_X_OFST);
}

/* parameters - readout */

int setReadoutMode(enum readoutMode mode) {
    int analogEnable = 0, digitalEnable = 0, transceiverEnable = 0;
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
    bus_w(addr, bus_r(addr) & ~(RO_MODE_ADC_MSK | RO_MODE_D_MSK | RO_MODE_X_MSK));
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

void setNumFrames(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of frames %ld\n", val));
        setU64BitReg(val, FRAMES_IN_REG_1, FRAMES_IN_REG_2);
    }
}

int64_t getNumFrames() { return getU64BitReg(FRAMES_IN_REG_1, FRAMES_IN_REG_2); }

void setNumTriggers(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of triggers %ld\n", val));
        setU64BitReg(val, CYCLES_IN_REG_1, CYCLES_IN_REG_2);
    }
}

int64_t getNumTriggers() { return getU64BitReg(CYCLES_IN_REG_1, CYCLES_IN_REG_2); }

int setNumAnalogSamples(int val) {
    if (val < 0 || val > MAX_ANALOG_SAMPLES) {
        LOG(logERROR, ("Invalid analog samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of analog samples %d\n", val));

    uint32_t addr = NO_SAMPLES_A_REG;
    bus_w(addr, bus_r(addr) & ~NO_SAMPLES_A_MSK);
    bus_w(addr, bus_r(addr) | ((val << NO_SAMPLES_A_OFST) &
                               NO_SAMPLES_A_MSK));
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
    bus_w(addr, bus_r(addr) | ((val << NO_SAMPLES_D_OFST) &
                               NO_SAMPLES_D_MSK));
    return OK;
}

int getNumDigitalSamples() { 
    return ((bus_r(NO_SAMPLES_X_REG) & NO_SAMPLES_D_MSK) >> NO_SAMPLES_D_OFST); 
}

int setNumTransceiverSamples(int val) {
    if (val < 0 || val > MAX_TRANSCEIVER_SAMPLES) {
        LOG(logERROR, ("Invalid transceiver samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of transceiver samples %d\n", val));

    uint32_t addr = NO_SAMPLES_X_REG;
    bus_w(addr, bus_r(addr) & ~NO_SAMPLES_X_MSK);
    bus_w(addr, bus_r(addr) | ((val << NO_SAMPLES_X_OFST) &
                               NO_SAMPLES_X_MSK));
    return OK;
}

int getNumTransceiverSamples() { 
    return ((bus_r(NO_SAMPLES_X_REG) & NO_SAMPLES_X_MSK) >> NO_SAMPLES_X_OFST); 
}

int setExpTime(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid exptime: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
    val *= (1E-3 * RUN_CLK);
    setPatternWaitTime(0, val);

    // validate for tolerance
    int64_t retval = getExpTime();
    val /= (1E-3 * RUN_CLK);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getExpTime() { return getPatternWaitTime(0) / (1E-3 * RUN_CLK); }

int setPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid period: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    val *= (1E-3 * RUN_CLK);
    setU64BitReg(val, PERIOD_IN_REG_1, PERIOD_IN_REG_2);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-3 * RUN_CLK);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return getU64BitReg(PERIOD_IN_REG_1, PERIOD_IN_REG_2) / (1E-3 * RUN_CLK);
}

int64_t getNumFramesLeft() {
    return getU64BitReg(FRAMES_OUT_REG_1, FRAMES_OUT_REG_2);
}

int64_t getNumTriggersLeft() {
    return getU64BitReg(CYCLES_OUT_REG_1, CYCLES_OUT_REG_2);
}

/* parameters - timing, extsig */

void setTiming(enum timingMode arg) {
    switch (arg) {
    case AUTO_TIMING:
        LOG(logINFO, ("Set Timing: Auto\n"));
        bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) & ~TRIGGER_ENABLE_MSK);
        break;
    case TRIGGER_EXPOSURE:
        LOG(logINFO, ("Set Timing: Trigger\n"));
        bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | TRIGGER_ENABLE_MSK);
        break;
    default:
        LOG(logERROR, ("Unknown timing mode %d\n", arg));
    }
}

enum timingMode getTiming() {
    if (bus_r(FLOW_CONTROL_REG) == TRIGGER_ENABLE_MSK)
        return TRIGGER_EXPOSURE;
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

    // from identification to srcip_lsb
    while (count > 2) {
        sum += *addr++;
        count -= 2;
    }

    // ignore src udp port (from udp header)
    addr++;

    if (count > 0)
        sum += *addr; // Add left-over byte, if any
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16); // Fold 32-bit sum to 16 bits
    long int checksum = sum & 0xffff;
    checksum += UDP_IP_HEADER_LENGTH_BYTES;
    udp->ip_checksum = checksum;
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
    }
    LOG(logINFOGREEN, ("Virtual Acquisition started\n"));
    return OK;
#endif

    LOG(logINFOBLUE, ("Starting State Machine\n"));
    cleanFifos();

    // start state machine
    bus_w(FLOW_CONTROL_REG, bus_r(FLOW_CONTROL_REG) | START_F_MSK);
    
    LOG(logINFORED, ("Waiting for exposing to be done\n"));
    int exposingDone = (bus_r(FLOW_STATUS_REG) & RSM_BUSY_MSK);
	while (exposingDone != 0) {
        usleep(0);
        exposingDone = (bus_r(FLOW_STATUS_REG) & RSM_BUSY_MSK);
    }
    
    LOG(logINFORED, ("Starting readout of chip to fifo\n"));
    bus_w(MATTERHORNSPICTRL, bus_r(MATTERHORNSPICTRL) | STARTREAD_P_MSK);
    
    LOG(logINFORED, ("Waiting until k-words or end of acquisition\n"));
    usleep(0);
    int commaDet = (bus_r(TRANSCEIVERSTATUS) & RXCOMMADET_MSK);
	while (commaDet == 0) {
        usleep(0);
        commaDet = (bus_r(TRANSCEIVERSTATUS) & RXCOMMADET_MSK);
    }

    return OK;
}

#ifdef VIRTUAL
void *start_timer(void *arg) {
    if (!isControlServer) {
        return NULL;
    }

    int64_t periodNs = getPeriod();
    int numFrames = (getNumFrames() * getNumTriggers());
    int64_t expUs = getExpTime() / 1000;

    int imageSize = calculateDataBytes();
    int dataSize = 8192;
    int packetSize = sizeof(sls_detector_header) + dataSize;
    int packetsPerFrame = ceil((double)imageSize / (double)dataSize);

    // Generate Data
    char imageData[imageSize];
    memset(imageData, 0, imageSize);
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
            memcpy(packetData + sizeof(sls_detector_header),
                   imageData + srcOffset, dataSize);
            srcOffset += dataSize;

            sendUDPPacket(0, 0, packetData, packetSize);
            // LOG(logINFOBLUE, ("packetsize:%d\n", packetSize));
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
        setNextFrameNumber(frameNr + numFrames);
    }
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

    return OK;
}

int startReadOut() {
    LOG(logINFOBLUE, ("Starting Readout\n"));
#ifdef VIRTUAL
    // cannot set #frames and exptiem temporarily to 1 and 0,
    // because have to set it back after readout (but this is non blocking)
    return startStateMachine();
#endif
    // check if data in fifo
    if (transceiverEnable) {
        if ((bus_r(X_FIFO_EMPTY_STATUS_REG) & X_FIFO_EMPTY_STATUS_MSK) == X_FIFO_EMPTY_STATUS_MSK) {
            LOG(logERROR, ("No data in fifo\n"));
            return FAIL;
        }
    }

    LOG(logINFOBLUE, ("Streaming data from fifo\n"));
    bus_w(FIFO_TO_GB_CONTROL_REG, bus_r(FIFO_TO_GB_CONTROL_REG) | START_STREAMING_P_MSK);

    // wait until streaming is done (not same as fifo empty)
    /*TODO int streamingBusy = (bus_r(FIFO_TO_GB_CONTROL_REG) & STREAMING_BSY_MSK);
    while (streamingBusy != 0) {
        usleep(0);
        streamingBusy = (bus_r(FIFO_TO_GB_CONTROL_REG) & STREAMING_BSY_MSK);
    }
    */
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
   //TODO: usleep(100);

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
    uint32_t retval = bus_r(FLOW_STATUS_REG);
    LOG(logINFO, ("Flow Status Register: %08x\n", retval));

    if (retval & RSM_BUSY_MSK) {
        if (retval & RSM_TRG_WAIT_MSK) {

        } else if (retval & RSM_TRG_WAIT_MSK) {
            LOG(logINFOBLUE, ("Status: WAITING\n"));
            return WAITING;
        }
    } 
    
    /* TODO else if (bus_r(FIFO_TO_GB_CONTROL_REG) & STREAMING_BSY_MSK) {
        LOG(logINFOBLUE, ("Status: TRANSMITTING\n"));
        return TRANSMITTING;
    }
    */
    LOG(logINFOBLUE, ("Status: IDLE\n"));
    return IDLE;
    // TODO: STOPPED, ERROR?
}

void waitForAcquisitionEnd() {
   /* uint32_t busy = bus_r(FLOW_STATUS_REG) & RSM_BUSY_MSK;
    TODO
    uint32_t streaming = bus_r(FIFO_TO_GB_CONTROL_REG) & STREAMING_BSY_MSK;
    while (busy != 0 && streaming != 0) {
        usleep(100);
        busy = bus_r(FLOW_STATUS_REG) & RSM_BUSY_MSK;
        bus_r(FIFO_TO_GB_CONTROL_REG) & STREAMING_BSY_MSK;
    }
    */
#ifndef VIRTUAL
    int64_t retval = getNumFramesLeft() + 1;
    if (retval > 0) {
        LOG(logINFORED, ("%lld frames left\n", (long long int)retval));
    }
#endif
    LOG(logINFOGREEN, ("Blocking Acquisition done\n"));
}

int calculateDataBytes() { 
    int nchanx = 0, nchany = 0;
    getNumberOfChannels(&nchanx, &nchany);

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
    LOG(logDEBUG, ("Total #Channels: %d\n", *nchanx));
    *nchany = 1;
}

int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }