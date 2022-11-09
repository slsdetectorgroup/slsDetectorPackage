// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "clogger.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#include "ALTERA_PLL.h" // pll
#include "LTC2620.h"    // dacs
#include "MAX1932.h"    // hv
#include "UDPPacketHeaderGenerator.h"
#include "common.h"
#include "communication_funcs_UDP.h"
#include "loadPattern.h"

#include <netinet/in.h>
#include <string.h>
#include <unistd.h> // usleep
#ifdef VIRTUAL
#include <math.h> //ceil
#include <pthread.h>
#include <time.h>
#endif

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern udpStruct udpDetails[MAX_UDP_DESTINATION];
extern const enum detectorType myDetectorType;

// Global variable from UDPPacketHeaderGenerator
extern uint64_t udpFrameNumber;
extern uint32_t udpPacketNumber;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char *cmac, int size, uint64_t mac);
extern void getIpAddressinString(char *cip, uint32_t ip);

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
#endif

// 1g readout
int dataBytes = 0;
int analogDataBytes = 0;
int digitalDataBytes = 0;
char *analogData = 0;
char *digitalData = 0;
char volatile *analogDataPtr = 0;
char volatile *digitalDataPtr = 0;
char udpPacketData[UDP_PACKET_DATA_BYTES + sizeof(sls_detector_header)];
uint32_t adcEnableMask_1g = BIT32_MSK;

// 10g readout
uint8_t adcEnableMask_10g = 0xFF;

int32_t clkPhase[NUM_CLOCKS] = {};
uint32_t clkFrequency[NUM_CLOCKS] = {40, 20, 20, 200};
int dacValues[NDAC] = {};
int defaultDacValues[NDAC] = DEFAULT_DAC_VALS;
// software limit that depends on the current chip on the ctb
int vLimit = 0;
enum detectorSettings thisSettings = UNINITIALIZED;
int highvoltage = 0;

// getNumberofchannels return 0 for y in --update mode (virtual servers)
#ifdef VIRTUAL
int nSamples = DEFAULT_NUM_SAMPLES;
#else
int nSamples = 1;
#endif
int detPos[2] = {0, 0};

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
    LOG(logINFOBLUE, ("********* Moench Detector Virtual Server *********\n"));
#else
    LOG(logINFOBLUE, ("************* Moench Detector Server *************\n"));

    initError = defineGPIOpins(initErrorMessage);
    if (initError == FAIL) {
        return;
    }
    initError = resetFPGA(initErrorMessage);
    if (initError == FAIL) {
        return;
    }
#endif
    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Dangerous to continue.\n");
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }
#ifndef VIRTUAL
    // does check only if flag is 0 (by default), set by command line
    if ((!debugflag) && (!updateFlag) &&
        ((checkType() == FAIL) || (testFpga() == FAIL) ||
         (testBus() == FAIL))) {
        strcpy(initErrorMessage, "Could not pass basic tests of FPGA and bus. "
                                 "Dangerous to continue.\n");
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }
#endif
    uint16_t hversion = getHardwareVersionNumber();
    uint16_t hsnumber = getHardwareSerialNumber();
    uint32_t ipadd = getDetectorIP();
    uint64_t macadd = getDetectorMAC();
    int64_t fwversion = getFirmwareVersion();
    char swversion[MAX_STR_LENGTH] = {0};
    memset(swversion, 0, MAX_STR_LENGTH);
    getServerVersion(swversion);
    int64_t sw_fw_apiversion = 0;

    if (fwversion >= MIN_REQRD_VRSN_T_RD_API)
        sw_fw_apiversion = getFirmwareAPIVersion();
    LOG(logINFOBLUE,
        ("**************************************************\n"
         "Hardware Version:\t\t 0x%x\n"
         "Hardware Serial Nr:\t\t 0x%x\n"

         "Detector IP Addr:\t\t 0x%x\n"
         "Detector MAC Addr:\t\t 0x%llx\n\n"

         "Firmware Version:\t\t 0x%llx\n"
         "Software Version:\t\t %s\n"
         "F/w-S/w API Version:\t\t 0x%llx\n"
         "Required Firmware Version:\t 0x%x\n"
         "********************************************************\n",
         hversion, hsnumber, ipadd, (long long unsigned int)macadd,
         (long long int)fwversion, swversion, (long long int)sw_fw_apiversion,
         REQRD_FRMWR_VRSN));

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
    if (sw_fw_apiversion > REQRD_FRMWR_VRSN) {
        sprintf(initErrorMessage,
                "This firmware-software api version (0x%llx) is incompatible "
                "with the software's minimum required firmware version "
                "(0x%llx).\nPlease update detector software to be compatible "
                "with this firmware.\n",
                (long long int)sw_fw_apiversion,
                (long long int)REQRD_FRMWR_VRSN);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for firmware compatibility - old firmware
    if (REQRD_FRMWR_VRSN > fwversion) {
        sprintf(initErrorMessage,
                "This firmware version (0x%llx) is incompatible.\n"
                "Please update firmware (min. 0x%llx) to be compatible with "
                "this server.\n",
                (long long int)fwversion, (long long int)REQRD_FRMWR_VRSN);
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
    uint32_t type = ((bus_r(FPGA_VERSION_REG) & FPGA_VERSION_DTCTR_TYP_MSK) >>
                     FPGA_VERSION_DTCTR_TYP_OFST);
    uint32_t expectedType =
        (((FPGA_VERSION_DTCTR_TYP_MOENCH_VAL)&FPGA_VERSION_DTCTR_TYP_MSK) >>
         FPGA_VERSION_DTCTR_TYP_OFST);

    if (type != expectedType) {
        LOG(logERROR, ("(Type Fail) - This is not a Moench Detector firmware "
                       "(read %d, expected %d)\n",
                       type, expectedType));
        return FAIL;
    }
    return OK;
}

int testFpga() {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Testing FPGA:\n"));

    // fixed pattern
    int ret = OK;
    uint32_t val = bus_r(FIX_PATT_REG);
    if (val == FIX_PATT_VAL) {
        LOG(logINFO, ("\tFixed pattern: successful match (0x%08x)\n", val));
    } else {
        LOG(logERROR,
            ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n",
             val, FIX_PATT_VAL));
        ret = FAIL;
    }

    if (ret == OK) {
        // Delay LSB reg
        LOG(logINFO, ("\tTesting Delay LSB Register:\n"));
        uint32_t addr = DELAY_LSB_REG;

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

int testBus() {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Testing Bus:\n"));

    int ret = OK;
    uint32_t addr = DELAY_LSB_REG;

    // store previous delay value
    uint32_t previousValue = bus_r(addr);

    volatile uint32_t val = 0, readval = 0;
    int times = 1000 * 1000;

    for (int i = 0; i < times; ++i) {
        val += 0xbbbbb;
        bus_w(addr, val);
        readval = bus_r(addr);
        if (readval != val) {
            LOG(logERROR, ("Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n", i,
                           val, readval));
            ret = FAIL;
        }
    }

    // write back previous value
    bus_w(addr, previousValue);

    if (ret == OK) {
        LOG(logINFO, ("\tSuccessfully tested bus %d times\n", times));
    }
    return ret;
}

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIMOENCH); }

uint64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(FPGA_VERSION_REG) & FPGA_VERSION_BRD_RVSN_MSK) >>
            FPGA_VERSION_BRD_RVSN_OFST);
}

uint64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(API_VERSION_REG) & API_VERSION_MSK) >> API_VERSION_OFST);
}

uint16_t getHardwareVersionNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(MOD_SERIAL_NUMBER_REG) & MOD_SERIAL_NUMBER_VRSN_MSK) >>
            MOD_SERIAL_NUMBER_VRSN_OFST);
}

uint16_t getHardwareSerialNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(MOD_SERIAL_NUMBER_REG) & MOD_SERIAL_NUMBER_MSK) >>
            MOD_SERIAL_NUMBER_OFST);
}

uint32_t getDetectorNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return bus_r(MOD_SERIAL_NUMBER_REG);
}

uint64_t getDetectorMAC() {
#ifdef VIRTUAL
    return 0;
#else
    char output[255], mac[255] = "";
    uint64_t res = 0;
    FILE *sysFile =
        popen("ifconfig eth0 | grep HWaddr | cut -d \" \" -f 11", "r");
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);
    // getting rid of ":"
    char *pch;
    pch = strtok(output, ":");
    while (pch != NULL) {
        strcat(mac, pch);
        pch = strtok(NULL, ":");
    }
    sscanf(mac, "%llx", &res);
    return res;
#endif
}

uint32_t getDetectorIP() {
#ifdef VIRTUAL
    return 0;
#endif
    char temp[INET_ADDRSTRLEN] = "";
    uint32_t res = 0;
    // execute and get address
    char output[255];
    FILE *sysFile = popen(
        "ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2",
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
                   "Stop Server: Map Fail. Dangerous to continue. Goodbye!\n");
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
    LOG(logINFO, ("This Server is for 1 Moench Board module\n"));

    // default variables
    dataBytes = 0;
    analogDataBytes = 0;
    digitalDataBytes = 0;
    if (analogData) {
        free(analogData);
        analogData = 0;
    }
    if (digitalData) {
        free(digitalData);
        digitalData = 0;
    }
    analogDataPtr = 0;
    digitalDataPtr = 0;
    for (int i = 0; i < NUM_CLOCKS; ++i) {
        clkPhase[i] = 0;
    }
    clkFrequency[RUN_CLK] = DEFAULT_RUN_CLK_AT_STARTUP;
    clkFrequency[ADC_CLK] = DEFAULT_ADC_CLK_AT_STARTUP;
    clkFrequency[SYNC_CLK] = DEFAULT_SYNC_CLK_AT_STARTUP;
    clkFrequency[DBIT_CLK] = DEFAULT_DBIT_CLK_AT_STARTUP;
    // default adc phase in deg
    /*
            {
                int phase_shifts = 0;
                ConvertToDifferentRange(0, 359, 0, getMaxPhase(ADC_CLK) - 1,
        DEFAULT_ADC_PHASE_DEG, &phase_shifts); clkPhase[ADC_CLK] =
        phase_shifts;
            }
            LOG(logINFO, ("Default Run clk: %d MHz\n",
        clkFrequency[RUN_CLK])); LOG(logINFO, ("Default Adc clk: %d MHz\n",
        clkFrequency[ADC_CLK])); LOG(logINFO, ("Default Sync clk: %d MHz\n",
        clkFrequency[SYNC_CLK])); LOG(logINFO, ("Default Dbit clk: %d MHz\n",
        clkFrequency[DBIT_CLK])); LOG(logINFO, ("Default Adc Phase: %d (%d
        deg)\n", clkPhase[ADC_CLK], getPhase(ADC_CLK, 1)));
    */
    for (int i = 0; i < NDAC; ++i)
        dacValues[i] = -1;
    vLimit = DEFAULT_VLIMIT;
    highvoltage = 0;
    adcEnableMask_1g = BIT32_MSK;
    adcEnableMask_10g = 0xFF;
    nSamples = 1;
#ifdef VIRTUAL
    sharedMemory_setStatus(IDLE);
    initializePatternWord();
#endif
    setupUDPCommParameters();

    // altera pll
    ALTERA_PLL_SetDefines(PLL_CNTRL_REG, PLL_PARAM_REG,
                          PLL_CNTRL_RCNFG_PRMTR_RST_MSK, PLL_CNTRL_WR_PRMTR_MSK,
                          PLL_CNTRL_PLL_RST_MSK, PLL_CNTRL_ADDR_MSK,
                          PLL_CNTRL_ADDR_OFST);
    ALTERA_PLL_ResetPLLAndReconfiguration();

    resetCore();
    resetPeripheral();
    cleanFifos();

    initializePatternAddresses();

    // hv
    MAX1932_SetDefines(SPI_REG, SPI_HV_SRL_CS_OTPT_MSK, SPI_HV_SRL_CLK_OTPT_MSK,
                       SPI_HV_SRL_DGTL_OTPT_MSK, SPI_HV_SRL_DGTL_OTPT_OFST,
                       HIGHVOLTAGE_MIN, HIGHVOLTAGE_MAX);
    MAX1932_Disable();
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);

    // power off chip
    powerChip(0);

    // adcs
    AD9257_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK,
                      ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK,
                      ADC_SPI_SRL_DT_OTPT_OFST);
    AD9257_Disable();
    AD9257_Configure();

    // dacs
    LTC2620_SetDefines(SPI_REG, SPI_DAC_SRL_CS_OTPT_MSK,
                       SPI_DAC_SRL_CLK_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_MSK,
                       SPI_DAC_SRL_DGTL_OTPT_OFST, NDAC, DAC_MIN_MV,
                       DAC_MAX_MV); // has to be before setvchip
    LTC2620_Disable();
    LTC2620_Configure();
    resetToDefaultDacs(0);

    // not using setADCInvertRegister command (as it xors the default)
    bus_w(ADC_PORT_INVERT_REG, ADC_PORT_INVERT_VAL);

    LOG(logINFOBLUE, ("Setting Default parameters\n"));
    cleanFifos(); // FIXME: why twice?
    resetCore();

    // 1G UDP
    enableTenGigabitEthernet(0);

    // Initialization of acquistion parameters
    setNumAnalogSamples(DEFAULT_NUM_SAMPLES);
    setNumFrames(DEFAULT_NUM_FRAMES);
    setExpTime(DEFAULT_EXPTIME);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setPeriod(DEFAULT_PERIOD);
    setDelayAfterTrigger(DEFAULT_DELAY);
    setTiming(DEFAULT_TIMING_MODE);
    setADCEnableMask(BIT32_MSK);
    setADCEnableMask_10G(BIT32_MSK);
    if (setAnalogOnlyReadout() == FAIL) {
        strcpy(initErrorMessage,
               "Could not set readout mode to analog only.\n");
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
    }
    setADCPipeline(DEFAULT_PIPELINE);
    if (initError != FAIL) {
        initError = loadPatternFile(DEFAULT_PATTERN_FILE, initErrorMessage);
    }
    setSettings(DEFAULT_SETTINGS);

    setFrequency(RUN_CLK, DEFAULT_RUN_CLK);
    setFrequency(ADC_CLK, DEFAULT_ADC_CLK);
    setFrequency(DBIT_CLK, DEFAULT_DBIT_CLK);
    setPhase(ADC_CLK, DEFAULT_ADC_PHASE_DEG, 1);
    setNextFrameNumber(DEFAULT_STARTING_FRAME_NUMBER);
}

int updateDatabytesandAllocateRAM() {

    int oldDataBytes = analogDataBytes;
    updateDataBytes();

    // update only if change in databytes
    if (analogDataBytes == oldDataBytes) {
        LOG(logDEBUG1,
            ("RAM size (Databytes:%d) already allocated. Nothing to be done.\n",
             dataBytes));
        return OK;
    }
    // Zero databytes
    if (analogDataBytes == 0) {
        LOG(logERROR, ("Can not allocate RAM for 0 bytes.\n"));
        return FAIL;
    }
    // clear RAM
    if (analogData) {
        free(analogData);
        analogData = 0;
    }
    // allocate RAM
    analogData = malloc(analogDataBytes);
    // cannot malloc
    if (analogData == NULL) {
        LOG(logERROR, ("Can not allocate data RAM for even 1 frame. "
                       "Probable cause: Memory Leak.\n"));
        return FAIL;
    }
    LOG(logINFO, ("\tRAM allocated to %d bytes\n", analogDataBytes));
    return OK;
}

void updateDataBytes() {
    int nchans = 0;
    analogDataBytes = 0;

    if (adcEnableMask_1g == BIT32_MSK)
        nchans = 32;
    else {
        for (int ichan = 0; ichan < NCHAN; ++ichan) {
            if (adcEnableMask_1g & (1 << ichan))
                ++nchans;
        }
    }
    analogDataBytes = nchans * (DYNAMIC_RANGE / 8) * nSamples;
    LOG(logINFO, ("\t#Channels:%d, Databytes:%d\n", nchans, analogDataBytes));

    dataBytes = analogDataBytes;
}

int resetToDefaultDacs(int hardReset) {
    // reset defaults to hardcoded defaults
    if (hardReset) {
        const int vals[] = DEFAULT_DAC_VALS;
        for (int i = 0; i < NDAC; ++i) {
            defaultDacValues[i] = vals[i];
        }
    }
    // reset dacs to defaults
    int ret = OK;
    LOG(logINFOBLUE, ("Setting Default Dac values\n"));
    for (int i = 0; i < NDAC; ++i) {
        setDAC((enum DACINDEX)i, defaultDacValues[i], 0);
        if (dacValues[i] != defaultDacValues[i]) {
            ret = FAIL;
            LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n", i,
                           defaultDacValues[i], dacValues[i]));
        }
    }
    return ret;
}
int getDefaultDac(enum DACINDEX index, enum detectorSettings sett,
                  int *retval) {
    if (sett != UNDEFINED) {
        return FAIL;
    }
    if (index < 0 || index >= NDAC)
        return FAIL;
    *retval = defaultDacValues[index];
    return OK;
}

int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value) {
    if (sett != UNDEFINED) {
        return FAIL;
    }
    if (index < 0 || index >= NDAC)
        return FAIL;

    char *dac_names[] = {DAC_NAMES};
    LOG(logINFO, ("Setting Default Dac [%d - %s]: %d\n", (int)index,
                  dac_names[index], value));
    defaultDacValues[index] = value;
    return OK;
}

/* firmware functions (resets) */

void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Clearing Acquisition Fifos\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CLR_ACQSTN_FIFO_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CLR_ACQSTN_FIFO_MSK);
}

void resetCore() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Resetting Core\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CRE_RST_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CRE_RST_MSK);
}

void resetPeripheral() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Resetting Peripheral\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PRPHRL_RST_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PRPHRL_RST_MSK);
}

/* set parameters -  dr, adcenablemask */

int setDynamicRange(int dr) {
    if (dr == 16)
        return OK;
    return FAIL;
}

int getDynamicRange(int *retval) {
    *retval = DYNAMIC_RANGE;
    return OK;
}

int setADCEnableMask(uint32_t mask) {
    if (mask == 0u) {
        LOG(logERROR, ("Cannot set 1gb adc mask to 0\n"));
        return FAIL;
    }
    int topAdcs = __builtin_popcount(mask & 0xF0F0F0F0);
    int bottomAdcs = __builtin_popcount(mask & 0x0F0F0F0F);
    if (topAdcs > 0 && bottomAdcs > 0 && topAdcs != bottomAdcs) {
        LOG(logERROR,
            ("Invalid mask. Top and bottom number of adcs do not match\n"));
        return FAIL;
    }
    LOG(logINFO, ("Setting adcEnableMask 1G to 0x%08x\n", mask));
    adcEnableMask_1g = mask;
    // 1Gb enabled
    if (!enableTenGigabitEthernet(-1)) {
        if (updateDatabytesandAllocateRAM() == FAIL) {
            return FAIL;
        }
    }
    return OK;
}

uint32_t getADCEnableMask() { return adcEnableMask_1g; }

void setADCEnableMask_10G(uint32_t mask) {
    if (mask == 0u) {
        LOG(logERROR, ("Cannot set 10gb adc mask to 0\n"));
        return;
    }
    int topAdcs = __builtin_popcount(mask & 0xF0F0F0F0);
    int bottomAdcs = __builtin_popcount(mask & 0x0F0F0F0F);
    if (topAdcs > 0 && bottomAdcs > 0 && topAdcs != bottomAdcs) {
        LOG(logERROR,
            ("Invalid mask. Top and bottom number of adcs do not match\n"));
        return;
    }
    // convert 32 bit mask to 8 bit mask
    uint8_t actualMask = 0;
    if (mask != 0) {
        int ival = 0;
        for (int ich = 0; ich < NCHAN; ich = ich + 4) {
            if ((1 << ich) & mask) {
                actualMask |= (1 << ival);
            }
            ++ival;
        }
    }

    LOG(logINFO, ("Setting adcEnableMask 10G to 0x%x (from 0x%08x)\n",
                  actualMask, mask));
    adcEnableMask_10g = actualMask;
    uint32_t addr = READOUT_10G_ENABLE_REG;
    bus_w(addr, bus_r(addr) & (~READOUT_10G_ENABLE_ANLG_MSK));
    bus_w(addr,
          bus_r(addr) | ((adcEnableMask_10g << READOUT_10G_ENABLE_ANLG_OFST) &
                         READOUT_10G_ENABLE_ANLG_MSK));
}

uint32_t getADCEnableMask_10G() {
    adcEnableMask_10g =
        ((bus_r(READOUT_10G_ENABLE_REG) & READOUT_10G_ENABLE_ANLG_MSK) >>
         READOUT_10G_ENABLE_ANLG_OFST);

    // convert 8 bit mask to 32 bit mask
    uint32_t retval = 0;
    if (adcEnableMask_10g) {
        for (int ival = 0; ival < 8; ++ival) {
            // if bit in 8 bit mask set
            if ((1 << ival) & adcEnableMask_10g) {
                // set it for 4 bits in 32 bit mask
                for (int iloop = 0; iloop < 4; ++iloop) {
                    retval |= (1 << (ival * 4 + iloop));
                }
            }
        }
    }
    return retval;
}

void setADCInvertRegister(uint32_t val) {
    LOG(logINFO, ("Setting ADC Port Invert Reg to 0x%x\n", val));
    uint32_t defaultValue = ADC_PORT_INVERT_VAL;
    uint32_t changeValue = defaultValue ^ val;
    LOG(logINFO, ("\t default: 0x%x, final:0x%x\n", defaultValue, changeValue));
    bus_w(ADC_PORT_INVERT_REG, changeValue);
}

uint32_t getADCInvertRegister() {
    uint32_t readValue = bus_r(ADC_PORT_INVERT_REG);
    int32_t defaultValue = ADC_PORT_INVERT_VAL;
    uint32_t val = defaultValue ^ readValue;
    LOG(logDEBUG1, ("\tread:0x%x, default:0x%x returned:0x%x\n", readValue,
                    defaultValue, val));
    return val;
}

/* parameters - timer */
int setNextFrameNumber(uint64_t value) {
    LOG(logINFO,
        ("Setting next frame number: %llu\n", (long long unsigned int)value));
    setU64BitReg(value, NEXT_FRAME_NUMB_LOCAL_LSB_REG,
                 NEXT_FRAME_NUMB_LOCAL_MSB_REG);
#ifndef VIRTUAL
    // for 1g udp interface
    setUDPFrameNumber(value);
#endif
    return OK;
}

int getNextFrameNumber(uint64_t *retval) {
    *retval = getU64BitReg(NEXT_FRAME_NUMB_LOCAL_LSB_REG,
                           NEXT_FRAME_NUMB_LOCAL_MSB_REG);
    return OK;
}

void setNumFrames(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of frames %lld\n", (long long int)val));
        set64BitReg(val, FRAMES_LSB_REG, FRAMES_MSB_REG);
    }
}

int64_t getNumFrames() { return get64BitReg(FRAMES_LSB_REG, FRAMES_MSB_REG); }

void setNumTriggers(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of triggers %lld\n", (long long int)val));
        set64BitReg(val, CYCLES_LSB_REG, CYCLES_MSB_REG);
    }
}

int64_t getNumTriggers() { return get64BitReg(CYCLES_LSB_REG, CYCLES_MSB_REG); }

int setNumAnalogSamples(int val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid analog samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of analog samples %d\n", val));
    nSamples = val;
    bus_w(SAMPLES_REG, bus_r(SAMPLES_REG) & ~SAMPLES_ANALOG_MSK);
    bus_w(SAMPLES_REG, bus_r(SAMPLES_REG) |
                           ((val << SAMPLES_ANALOG_OFST) & SAMPLES_ANALOG_MSK));

    // 1Gb
    if (!enableTenGigabitEthernet(-1)) {
        if (updateDatabytesandAllocateRAM() == FAIL) {
            return FAIL;
        }
    }
    return OK;
}

int getNumAnalogSamples() { return nSamples; }

int setExpTime(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid exptime: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
    val *= (1E-3 * clkFrequency[RUN_CLK]);
    setPatternWaitTime(0, val);

    // validate for tolerance
    int64_t retval = getExpTime();
    val /= (1E-3 * clkFrequency[RUN_CLK]);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getExpTime() {
    return getPatternWaitTime(0) / (1E-3 * clkFrequency[RUN_CLK]);
}

int setPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid period: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    val *= (1E-3 * clkFrequency[SYNC_CLK]);
    set64BitReg(val, PERIOD_LSB_REG, PERIOD_MSB_REG);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-3 * clkFrequency[SYNC_CLK]);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return get64BitReg(PERIOD_LSB_REG, PERIOD_MSB_REG) /
           (1E-3 * clkFrequency[SYNC_CLK]);
}

int setDelayAfterTrigger(int64_t val) {
    if (val < 0) {
        LOG(logERROR,
            ("Invalid delay after trigger: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting delay after trigger %lld ns\n", (long long int)val));
    val *= (1E-3 * clkFrequency[SYNC_CLK]);
    set64BitReg(val, DELAY_LSB_REG, DELAY_MSB_REG);

    // validate for tolerance
    int64_t retval = getDelayAfterTrigger();
    val /= (1E-3 * clkFrequency[SYNC_CLK]);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getDelayAfterTrigger() {
    return get64BitReg(DELAY_LSB_REG, DELAY_MSB_REG) /
           (1E-3 * clkFrequency[SYNC_CLK]);
}

int64_t getNumFramesLeft() {
    return get64BitReg(FRAMES_LEFT_LSB_REG, FRAMES_LEFT_MSB_REG);
}

int64_t getNumTriggersLeft() {
    return get64BitReg(CYCLES_LEFT_LSB_REG, CYCLES_LEFT_MSB_REG);
}

int64_t getDelayAfterTriggerLeft() {
    return get64BitReg(DELAY_LEFT_LSB_REG, DELAY_LEFT_MSB_REG) /
           (1E-3 * clkFrequency[SYNC_CLK]);
}

int64_t getPeriodLeft() {
    return get64BitReg(PERIOD_LEFT_LSB_REG, PERIOD_LEFT_MSB_REG) /
           (1E-3 * clkFrequency[SYNC_CLK]);
}

int64_t getFramesFromStart() {
    return get64BitReg(FRAMES_FROM_START_PG_LSB_REG,
                       FRAMES_FROM_START_PG_MSB_REG);
}

int64_t getActualTime() {
    return get64BitReg(TIME_FROM_START_LSB_REG, TIME_FROM_START_MSB_REG) /
           (1E-3 * CLK_FREQ);
}

int64_t getMeasurementTime() {
    return get64BitReg(START_FRAME_TIME_LSB_REG, START_FRAME_TIME_MSB_REG) /
           (1E-3 * CLK_FREQ);
}

/* parameters - settings */

enum detectorSettings setSettings(enum detectorSettings sett) {
    if (sett == UNINITIALIZED)
        return thisSettings;

    // set settings
    switch (sett) {
    case G1_HIGHGAIN:
        LOG(logINFO, ("Set settings - G1_HIGHGAIN\n"));
        setPatternBitMask(G1_HIGHGAIN_PATSETBIT);
        break;
    case G1_LOWGAIN:
        LOG(logINFO, ("Set settings - G1_LOWGAIN\n"));
        setPatternBitMask(G1_LOWGAIN_PATSETBIT);
        break;
    case G2_HIGHCAP_HIGHGAIN:
        LOG(logINFO, ("Set settings - G2_HIGHCAP_HIGHGAIN\n"));
        setPatternBitMask(G2_HIGHCAP_HIGHGAIN_PATSETBIT);
        break;
    case G2_HIGHCAP_LOWGAIN:
        LOG(logINFO, ("Set settings - G2_HIGHCAP_LOWGAIN\n"));
        setPatternBitMask(G2_HIGHCAP_LOWGAIN_PATSETBIT);
        break;
    case G2_LOWCAP_HIGHGAIN:
        LOG(logINFO, ("Set settings - G2_LOWCAP_HIGHGAIN\n"));
        setPatternBitMask(G2_LOWCAP_HIGHGAIN_PATSETBIT);
        break;
    case G2_LOWCAP_LOWGAIN:
        LOG(logINFO, ("Set settings - G2_LOWCAP_LOWGAIN\n"));
        setPatternBitMask(G2_LOWCAP_LOWGAIN_PATSETBIT);
        break;
    case G4_HIGHGAIN:
        LOG(logINFO, ("Set settings - G4_HIGHGAIN\n"));
        setPatternBitMask(G4_HIGHGAIN_PATSETBIT);
        break;
    case G4_LOWGAIN:
        LOG(logINFO, ("Set settings - G4_LOWGAIN\n"));
        setPatternBitMask(G4_LOWGAIN_PATSETBIT);
        break;
    default:
        LOG(logERROR,
            ("This settings is not defined for this detector %d\n", (int)sett));
        return -1;
    }
    setPatternMask(DEFAULT_PATMASK);
    thisSettings = sett;

    return getSettings();
}

enum detectorSettings getSettings() {

    uint64_t patmask = getPatternMask();
    if (patmask != DEFAULT_PATMASK) {
        LOG(logERROR,
            ("Patmask is 0x%llx, and not 0x%llx. Undefined Settings!\n",
             patmask, DEFAULT_PATMASK));
        thisSettings = UNDEFINED;
        return thisSettings;
    }

    uint64_t patsetbit = getPatternBitMask();
    switch (patsetbit) {
    case G1_HIGHGAIN_PATSETBIT:
        thisSettings = G1_HIGHGAIN;
        break;
    case G1_LOWGAIN_PATSETBIT:
        thisSettings = G1_LOWGAIN;
        break;
    case G2_HIGHCAP_HIGHGAIN_PATSETBIT:
        thisSettings = G2_HIGHCAP_HIGHGAIN;
        break;
    case G2_HIGHCAP_LOWGAIN_PATSETBIT:
        thisSettings = G2_HIGHCAP_LOWGAIN;
        break;
    case G2_LOWCAP_HIGHGAIN_PATSETBIT:
        thisSettings = G2_LOWCAP_HIGHGAIN;
        break;
    case G2_LOWCAP_LOWGAIN_PATSETBIT:
        thisSettings = G2_LOWCAP_LOWGAIN;
        break;
    case G4_HIGHGAIN_PATSETBIT:
        thisSettings = G4_HIGHGAIN;
        break;
    case G4_LOWGAIN_PATSETBIT:
        thisSettings = G4_LOWGAIN;
        break;
    default:
        LOG(logERROR,
            ("Patsetbit is 0x%llx. Undefined Settings!\n", patsetbit));
        thisSettings = UNDEFINED;
        break;
    }
    return thisSettings;
}

/* parameters - dac, adc, hv */

void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0 && val != LTC2620_GetPowerDownValue()) {
        return;
    }

    char *dac_names[] = {DAC_NAMES};
    LOG(logINFO, ("Setting DAC %s\n", dac_names[ind]));
    LOG(logDEBUG, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                   val, (mV ? "mV" : "dac units")));
    int dacval = val;
#ifdef VIRTUAL
    LOG(logINFO, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                  val, (mV ? "mV" : "dac units")));
    if (!mV) {
        dacValues[ind] = val;
    }
    // convert to dac units
    else if (LTC2620_VoltageToDac(val, &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#else
    if (LTC2620_SetDACValue((int)ind, val, mV, &dacval) == OK)
        dacValues[ind] = dacval;
#endif
}

int getDAC(enum DACINDEX ind, int mV) {
    if (!mV) {
        LOG(logDEBUG1, ("Getting DAC %d : %d dac\n", ind, dacValues[ind]));
        return dacValues[ind];
    }
    int voltage = -1;
    LTC2620_DacToVoltage(dacValues[ind], &voltage);
    LOG(logDEBUG1,
        ("Getting DAC %d : %d dac (%d mV)\n", ind, dacValues[ind], voltage));
    return voltage;
}

int getMaxDacSteps() { return LTC2620_GetMaxNumSteps(); }

int dacToVoltage(int dac) {
    int val;
    LTC2620_DacToVoltage(dac, &val);
    return val;
}

int checkVLimitCompliant(int mV) {
    if (vLimit > 0 && mV > vLimit)
        return FAIL;
    return OK;
}

int checkVLimitDacCompliant(int dac) {
    if (vLimit > 0) {
        int mv = 0;
        // could not convert
        if (LTC2620_DacToVoltage(dac, &mv) == FAIL)
            return FAIL;
        if (mv > vLimit)
            return FAIL;
    }
    return OK;
}

int getVLimit() { return vLimit; }

void setVLimit(int l) {
    if (l >= 0)
        vLimit = l;
}

int setHighVoltage(int val) {
    // setting hv
    if (val >= 0) {
        LOG(logINFO, ("Setting High voltage: %d V\n", val));
        uint32_t addr = POWER_REG;

        // switch to external high voltage
        bus_w(addr, bus_r(addr) & (~POWER_HV_INTERNAL_SLCT_MSK));

        MAX1932_Set(&val);

        // switch on internal high voltage, if set
        if (val > 0)
            bus_w(addr, bus_r(addr) | POWER_HV_INTERNAL_SLCT_MSK);

        highvoltage = val;
    }
    return highvoltage;
}

/* parameters - timing, extsig */

void setTiming(enum timingMode arg) {
    switch (arg) {
    case AUTO_TIMING:
        LOG(logINFO, ("Set Timing: Auto\n"));
        bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) & ~EXT_SIGNAL_MSK);
        break;
    case TRIGGER_EXPOSURE:
        LOG(logINFO, ("Set Timing: Trigger\n"));
        bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) | EXT_SIGNAL_MSK);
        break;
    default:
        LOG(logERROR, ("Unknown timing mode %d\n", arg));
    }
}

enum timingMode getTiming() {
    if (bus_r(EXT_SIGNAL_REG) == EXT_SIGNAL_MSK)
        return TRIGGER_EXPOSURE;
    return AUTO_TIMING;
}

/* configure mac */

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
    LOG(logINFO, ("\tIP checksum is 0x%lx\n", checksum));
    udp->ip_checksum = checksum;
}

int configureMAC() {
    uint32_t srcip = udpDetails[0].srcip;
    uint32_t dstip = udpDetails[0].dstip;
    uint64_t srcmac = udpDetails[0].srcmac;
    uint64_t dstmac = udpDetails[0].dstmac;
    int srcport = udpDetails[0].srcport;
    int dstport = udpDetails[0].dstport;

    LOG(logINFOBLUE, ("Configuring MAC\n"));
    char src_mac[MAC_ADDRESS_SIZE], src_ip[INET_ADDRSTRLEN],
        dst_mac[MAC_ADDRESS_SIZE], dst_ip[INET_ADDRSTRLEN];
    getMacAddressinString(src_mac, MAC_ADDRESS_SIZE, srcmac);
    getMacAddressinString(dst_mac, MAC_ADDRESS_SIZE, dstmac);
    getIpAddressinString(src_ip, srcip);
    getIpAddressinString(dst_ip, dstip);

    LOG(logINFO, ("\tSource IP   : %s\n"
                  "\tSource MAC  : %s\n"
                  "\tSource Port : %d\n"
                  "\tDest IP     : %s\n"
                  "\tDest MAC    : %s\n"
                  "\tDest Port   : %d\n",
                  src_ip, src_mac, srcport, dst_ip, dst_mac, dstport));

    // 1 giga udp
    if (!enableTenGigabitEthernet(-1)) {
        LOG(logINFOBLUE, ("\t1G MAC\n"));
        if (updateDatabytesandAllocateRAM() == FAIL)
            return -1;
        if (setUDPDestinationDetails(0, 0, dst_ip, dstport) == FAIL) {
            LOG(logERROR, ("could not set udp 1G destination IP and port\n"));
            return FAIL;
        }
        return OK;
    }

    // 10 G
    LOG(logINFOBLUE, ("\t10G MAC\n"));

    // start addr
    uint32_t addr = RXR_ENDPOINT_START_REG;
    // get struct memory
    udp_header *udp = (udp_header *)(Blackfin_getBaseAddress() + addr / 2);
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

    cleanFifos(); // FIXME: resetPerpheral() for ctb?
    resetPeripheral();
    LOG(logINFO, ("Waiting for %d s for mac to be up\n",
                  WAIT_TIME_CONFIGURE_MAC / (1000 * 1000)));
    usleep(WAIT_TIME_CONFIGURE_MAC); // todo maybe without

    return OK;
}

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));
    return OK;
}

int *getDetectorPosition() { return detPos; }

int enableTenGigabitEthernet(int val) {
    uint32_t addr = CONFIG_REG;

    // set
    if (val != -1) {
        LOG(logINFO, ("Setting 10Gbe: %d\n", (val > 0) ? 1 : 0));
        if (val > 0) {
            bus_w(addr, bus_r(addr) | CONFIG_GB10_SND_UDP_MSK);
        } else {
            bus_w(addr, bus_r(addr) & (~CONFIG_GB10_SND_UDP_MSK));
        }
        // configuremac called from client
    }
    return ((bus_r(addr) & CONFIG_GB10_SND_UDP_MSK) >>
            CONFIG_GB10_SND_UDP_OFST);
}

/* moench specific - powerchip, configure frequency, phase, pll*/
int powerChip(int on) {
    uint32_t addr = POWER_REG;
    if (on > 0) {
        LOG(logINFOBLUE, ("Powering on chip\n"));
        bus_w(addr, bus_r(addr) | POWER_CHIP_MSK);
    } else if (on == 0) {
        LOG(logINFOBLUE, ("Powering off chip\n"));
        bus_w(addr, bus_r(addr) & ~POWER_CHIP_MSK);
    }
    return ((bus_r(addr) & POWER_CHIP_MSK) >> POWER_CHIP_OFST);
}

/* parameters - readout */

int setAnalogOnlyReadout() {
    LOG(logINFOBLUE, ("Setting Number of Digital samples to 0\n"));

    // digital num samples = 0
    bus_w(SAMPLES_REG, bus_r(SAMPLES_REG) & ~SAMPLES_DIGITAL_MSK);

    LOG(logINFOBLUE, ("Setting Analog Only Readout\n"));
    // analog only readout
    uint32_t addr = CONFIG_REG;
    uint32_t addr_readout_10g = READOUT_10G_ENABLE_REG;
    //  default: analog only
    bus_w(addr, bus_r(addr) & (~CONFIG_DSBL_ANLG_OTPT_MSK) &
                    (~CONFIG_ENBLE_DGTL_OTPT_MSK));
    bus_w(addr_readout_10g, bus_r(addr_readout_10g) &
                                (~READOUT_10G_ENABLE_ANLG_MSK) &
                                ~(READOUT_10G_ENABLE_DGTL_MSK));
    bus_w(addr_readout_10g,
          bus_r(addr_readout_10g) |
              ((adcEnableMask_10g << READOUT_10G_ENABLE_ANLG_OFST) &
               READOUT_10G_ENABLE_ANLG_MSK));

    // 1Gb
    if (!enableTenGigabitEthernet(-1)) {
        if (updateDatabytesandAllocateRAM() == FAIL) {
            return FAIL;
        }
    }

    // 10Gb
    else {
        // validate adcenablemask for 10g
        if (adcEnableMask_10g !=
            ((bus_r(READOUT_10G_ENABLE_REG) & READOUT_10G_ENABLE_ANLG_MSK) >>
             READOUT_10G_ENABLE_ANLG_OFST)) {
            LOG(logERROR, ("Setting readout mode failed. Could not set 10g adc "
                           "enable mask to 0x%x\n.",
                           adcEnableMask_10g));
            return FAIL;
        }
    }
    return OK;
}

int setPhase(enum CLKINDEX ind, int val, int degrees) {
    if (ind != ADC_CLK && ind != DBIT_CLK) {
        LOG(logERROR, ("Unknown clock index %d to set phase\n", ind));
        return FAIL;
    }
    char *clock_names[] = {CLK_NAMES};
    LOG(logINFO, ("Setting %s clock (%d) phase to %d %s\n", clock_names[ind],
                  ind, val, degrees == 0 ? "" : "degrees"));
    int maxShift = getMaxPhase(ind);
    // validation
    if (degrees && (val < 0 || val > 359)) {
        LOG(logERROR, ("\tPhase outside limits (0 - 359°C)\n"));
        return FAIL;
    }
    if (!degrees && (val < 0 || val > maxShift - 1)) {
        LOG(logERROR,
            ("\tPhase outside limits (0 - %d phase shifts)\n", maxShift - 1));
        return FAIL;
    }

    int valShift = val;
    // convert to phase shift
    if (degrees) {
        ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
    }
    LOG(logDEBUG1, ("phase shift: %d (degrees/shift: %d)\n", valShift, val));

    int relativePhase = valShift - clkPhase[ind];
    LOG(logDEBUG1, ("relative phase shift: %d (Current phase: %d)\n",
                    relativePhase, clkPhase[ind]));

    // same phase
    if (!relativePhase) {
        LOG(logINFO, ("\tNothing to do in Phase Shift\n"));
        return OK;
    }
    LOG(logINFOBLUE, ("Configuring Phase\n"));

    int phase = 0;
    if (relativePhase > 0) {
        phase = (maxShift - relativePhase);
    } else {
        phase = (-1) * relativePhase;
    }
    LOG(logDEBUG1, ("[Single Direction] Phase:%d (0x%x). Max Phase shifts:%d\n",
                    phase, phase, maxShift));

    ALTERA_PLL_SetPhaseShift(phase, (int)ind, 0);

    clkPhase[ind] = valShift;
    return OK;
}

int getPhase(enum CLKINDEX ind, int degrees) {
    if (ind != ADC_CLK && ind != DBIT_CLK) {
        LOG(logERROR, ("Unknown clock index %d to get phase\n", ind));
        return -1;
    }
    if (!degrees)
        return clkPhase[ind];
    // convert back to degrees
    int val = 0;
    ConvertToDifferentRange(0, getMaxPhase(ind) - 1, 0, 359, clkPhase[ind],
                            &val);
    return val;
}

int getMaxPhase(enum CLKINDEX ind) {
    if (ind != ADC_CLK && ind != DBIT_CLK) {
        LOG(logERROR, ("Unknown clock index %d to get max phase\n", ind));
        return -1;
    }
    int ret = ((double)PLL_VCO_FREQ_MHZ / (double)clkFrequency[ind]) *
              MAX_PHASE_SHIFTS_STEPS;

    char *clock_names[] = {CLK_NAMES};
    LOG(logDEBUG1,
        ("Max Phase Shift (%s): %d (Clock: %d MHz, VCO:%d MHz)\n",
         clock_names[ind], ret, clkFrequency[ind], PLL_VCO_FREQ_MHZ));

    return ret;
}

int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval) {
    if (ind != ADC_CLK && ind != DBIT_CLK) {
        LOG(logERROR,
            ("Unknown clock index %d to validate phase in degrees\n", ind));
        return FAIL;
    }
    if (val == -1) {
        return OK;
    }
    LOG(logDEBUG1, ("validating phase in degrees for clk %d\n", ind));
    int maxShift = getMaxPhase(ind);
    // convert degrees to shift
    int valShift = 0;
    ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
    // convert back to degrees
    ConvertToDifferentRange(0, maxShift - 1, 0, 359, valShift, &val);

    if (val == retval)
        return OK;
    return FAIL;
}

int setFrequency(enum CLKINDEX ind, int val) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to set frequency\n", ind));
        return FAIL;
    }
    if (val <= 0) {
        return FAIL;
    }
    char *clock_names[] = {CLK_NAMES};
    LOG(logINFOBLUE, ("Setting %s clock (%d) frequency to %d MHz\n",
                      clock_names[ind], ind, val));

    // check adc clk too high
    if (ind == ADC_CLK && val > MAXIMUM_ADC_CLK) {
        LOG(logERROR, ("Frequency %d MHz too high for ADC\n", val));
        return FAIL;
    }

    // Remembering adcphase/ dbit phase in degrees
    int adcPhase = getPhase(ADC_CLK, 1);
    LOG(logDEBUG1, ("\tRemembering ADC phase: %d degrees\n", adcPhase));
    int dbitPhase = getPhase(DBIT_CLK, 1);
    LOG(logDEBUG1, ("\tRemembering DBIT phase: %d degrees\n", dbitPhase));

    // Calculate and set output frequency
    clkFrequency[ind] =
        ALTERA_PLL_SetOuputFrequency(ind, PLL_VCO_FREQ_MHZ, val);
    LOG(logINFO, ("\t%s clock (%d) frequency set to %d MHz\n", clock_names[ind],
                  ind, clkFrequency[ind]));

    // phase reset by pll (when setting output frequency)
    clkPhase[ADC_CLK] = 0;
    clkPhase[DBIT_CLK] = 0;

    // set the phase (reset by pll)
    LOG(logINFO, ("\tCorrecting ADC phase to %d degrees\n", adcPhase));
    setPhase(ADC_CLK, adcPhase, 1);
    LOG(logINFO, ("\tCorrecting DBIT phase to %d degrees\n", dbitPhase));
    setPhase(DBIT_CLK, dbitPhase, 1);

    // required to reconfigure as adc clock is stopped temporarily when
    // resetting pll (in changing output frequency)
    AD9257_Configure();

    if (ind != SYNC_CLK) {
        configureSyncFrequency(ind);
    }
    return OK;
}

int getFrequency(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get frequency\n", ind));
        return -1;
    }
    return clkFrequency[ind];
}

void configureSyncFrequency(enum CLKINDEX ind) {
    char *clock_names[] = {CLK_NAMES};
    int clka = 0, clkb = 0;
    switch (ind) {
    case ADC_CLK:
        clka = DBIT_CLK;
        clkb = RUN_CLK;
        break;
    case DBIT_CLK:
        clka = ADC_CLK;
        clkb = RUN_CLK;
        break;
    case RUN_CLK:
        clka = DBIT_CLK;
        clkb = ADC_CLK;
        break;
    default:
        LOG(logERROR,
            ("Unknown clock index %d to configure sync frequcny\n", ind));
        return;
    }

    int syncFreq = getFrequency(SYNC_CLK);
    int retval = getFrequency(ind);
    int aFreq = getFrequency(clka);
    int bFreq = getFrequency(clkb);
    LOG(logDEBUG1,
        ("Sync Frequncy:%d, RetvalFreq(%s):%d, aFreq(%s):%d, bFreq(%s):%d\n",
         syncFreq, clock_names[ind], retval, clock_names[clka], aFreq,
         clock_names[clkb], bFreq));

    int configure = 0;

    // find the smallest frequency
    int min = (aFreq < bFreq) ? aFreq : bFreq;
    min = (retval < min) ? retval : min;

    // sync is greater than min
    if (syncFreq > retval) {
        LOG(logINFO, ("\t--Configuring Sync Clock\n"));
        configure = 1;
    }

    // sync is smaller than min
    else if (syncFreq < min) {
        LOG(logINFO, ("\t++Configuring Sync Clock\n"));
        configure = 1;
    }

    // configure sync to current
    if (configure)
        setFrequency(SYNC_CLK, min);
}

void setADCPipeline(int val) {
    if (val < 0) {
        return;
    }
    LOG(logINFO, ("Setting adc pipeline to %d\n", val));
    uint32_t addr = ADC_OFFSET_REG;
    bus_w(addr, bus_r(addr) & ~ADC_OFFSET_ADC_PPLN_MSK);
    bus_w(addr, bus_r(addr) | ((val << ADC_OFFSET_ADC_PPLN_OFST) &
                               ADC_OFFSET_ADC_PPLN_MSK));
}

int getADCPipeline() {
    return ((bus_r(ADC_OFFSET_REG) & ADC_OFFSET_ADC_PPLN_MSK) >>
            ADC_OFFSET_ADC_PPLN_OFST);
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
    int send_to_10g = enableTenGigabitEthernet(-1);
    // 1 giga udp
    if (send_to_10g == 0) {
        // create udp socket
        if (createUDPSocket(0) != OK) {
            return FAIL;
        }
        // update header with modId, detType and version. Reset offset and fnum
        createUDPPacketHeader(udpPacketData, getHardwareSerialNumber());
    }

    LOG(logINFOBLUE, ("Starting State Machine\n"));
    cleanFifos();
    if (send_to_10g == 0) {
        unsetFifoReadStrobes(); // FIXME: unnecessary to write bus_w(dumm, 0) as
                                // it is 0 in the beginnig and the strobes are
                                // always unset if set
    }

    // start state machine
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STRT_ACQSTN_MSK |
                           CONTROL_STRT_EXPSR_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STRT_ACQSTN_MSK &
                           ~CONTROL_STRT_EXPSR_MSK);

    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    return OK;
}

#ifdef VIRTUAL
void *start_timer(void *arg) {
    if (!isControlServer) {
        return NULL;
    }

    int64_t periodNs = getPeriod();
    int numFrames = (getNumFrames() * getNumTriggers());
    int64_t expNs = getExpTime();

    int imageSize = dataBytes;
    int dataSize = UDP_PACKET_DATA_BYTES;
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
        usleep(expNs / 1000);

        int srcOffset = 0;
        // loop packet
        for (int i = 0; i != packetsPerFrame; ++i) {
            // set header
            char packetData[packetSize];
            memset(packetData, 0, packetSize);
            sls_detector_header *header = (sls_detector_header *)(packetData);
            header->detType = (uint16_t)myDetectorType;
            header->version = SLS_DETECTOR_HEADER_VERSION - 1;
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
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STP_ACQSTN_MSK);
    usleep(WAIT_TIME_US_STP_ACQ);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STP_ACQSTN_MSK);

    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));

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

    // error
    // if (retval & STATUS_SM_FF_FLL_MSK) { This bit is high when a analog fifo
    // is full Or when external stop
    if (retval & STATUS_ANY_FF_FLL_MSK) { // if adc or digital fifo is full
        LOG(logINFORED, ("Status: Error (Any fifo full)\n"));
        return ERROR;
    }

    // running
    if (retval & STATUS_RN_BSY_MSK) {
        if (retval & STATUS_WTNG_FR_TRGGR_MSK) {
            LOG(logINFOBLUE, ("Status: Waiting for Trigger\n"));
            return WAITING;
        }

        LOG(logINFOBLUE, ("Status: Running\n"));
        return RUNNING;

    }

    // not running
    else {
        if (retval & STATUS_STPPD_MSK) {
            LOG(logINFOBLUE, ("Status: Stopped\n"));
            return STOPPED;
        }

        if (retval & STATUS_FRM_RN_BSY_MSK) {
            LOG(logINFOBLUE, ("Status: Transmitting (Read machine busy)\n"));
            return TRANSMITTING;
        }

        if (!(retval & STATUS_IDLE_MSK)) {
            LOG(logINFOBLUE, ("Status: Idle\n"));
            return IDLE;
        }

        LOG(logERROR, ("Status: Unknown status %08x\n", retval));
        return ERROR;
    }
}

void readandSendUDPFrames(int *ret, char *mess) {
    LOG(logDEBUG1, ("Reading from 1G UDP\n"));

    // validate udp socket
    if (getUdPSocketDescriptor(0, 0) <= 0) {
        *ret = FAIL;
        sprintf(mess, "UDP Socket not created. sockfd:%d\n",
                getUdPSocketDescriptor(0, 0));
        LOG(logERROR, (mess));
        return;
    }

    // every frame read
    while (readFrameFromFifo() == OK) {
        int bytesToSend = 0, n = 0;
        while ((bytesToSend = fillUDPPacket(udpPacketData))) {
            n += sendUDPPacket(0, 0, udpPacketData, bytesToSend);
        }
        if (n >= dataBytes) {
            LOG(logINFO, (" Frame %lld sent (%d packets, %d databytes, n:%d "
                          "bytes sent)\n",
                          udpFrameNumber, udpPacketNumber + 1, dataBytes, n));
        }
    }
    closeUDPSocket(0);
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

void readFrames(int *ret, char *mess) {
#ifdef VIRTUAL
    while (runBusy()) {
        usleep(500);
    }
#else
    // 1G force reading of frames
    if (!enableTenGigabitEthernet(-1)) {
        readandSendUDPFrames(ret, mess);
        LOG(logINFOBLUE, ("Transmitting frames done\n"));
    }
#endif
}

void unsetFifoReadStrobes() {
    bus_w(DUMMY_REG, bus_r(DUMMY_REG) & (~DUMMY_ANLG_FIFO_RD_STRBE_MSK) &
                         (~DUMMY_DGTL_FIFO_RD_STRBE_MSK));
}

void readSample(int ns) {
    uint32_t addr = DUMMY_REG;

    // read adcs
    if (ns < nSamples) {

        uint32_t fifoAddr = FIFO_DATA_REG;

        // read strobe to all analog fifos
        bus_w(addr, bus_r(addr) | DUMMY_ANLG_FIFO_RD_STRBE_MSK);
        bus_w(addr, bus_r(addr) & (~DUMMY_ANLG_FIFO_RD_STRBE_MSK));

        // wait for 1 us to latch different clocks of read and read strobe
        for (int i = 0; i < WAIT_TIME_1US_FOR_LOOP_CNT; ++i)
            ;

        if (!(ns % 1000)) {
            LOG(logDEBUG1, ("Reading sample ns:%d of %d AEmtpy:0x%x AFull:0x%x "
                            "Status:0x%x\n",
                            ns, nSamples, bus_r(FIFO_EMPTY_REG),
                            bus_r(FIFO_FULL_REG), bus_r(STATUS_REG)));
        }

        // loop through all channels
        for (int ich = 0; ich < NCHAN; ++ich) {

            // if channel is in enable mask
            if ((1 << ich) & (adcEnableMask_1g)) {

                // unselect channel
                bus_w(addr, bus_r(addr) & ~(DUMMY_FIFO_CHNNL_SLCT_MSK));

                // select channel
                bus_w(addr, bus_r(addr) | ((ich << DUMMY_FIFO_CHNNL_SLCT_OFST) &
                                           DUMMY_FIFO_CHNNL_SLCT_MSK));

                // read fifo and write it to current position of data pointer
                *((uint16_t *)analogDataPtr) = bus_r16(fifoAddr);

                // keep reading till the value is the same
                /* while (*((uint16_t*)analogDataPtr) != bus_r16(fifoAddr)) {
                     LOG(logDEBUG1, ("%d ", ich));
                     *((uint16_t*)analogDataPtr) = bus_r16(fifoAddr);
                 }*/

                // increment pointer to data out destination
                analogDataPtr += 2;
            }
        }
    }
}

uint32_t checkDataInFifo() {
    uint32_t dataPresent = 0;
    uint32_t fifoEmpty = bus_r(FIFO_EMPTY_REG);
    LOG(logINFO, ("Analog Fifo Empty (32 channels): 0x%08x\n", fifoEmpty));
    dataPresent = (~fifoEmpty);
    LOG(logDEBUG2, ("Data in Fifo :0x%x\n", dataPresent));
    return dataPresent;
}

// only called for starting of a new frame
int checkFifoForEndOfAcquisition() {
    uint32_t dataPresent = checkDataInFifo();
    LOG(logDEBUG2, ("status:0x%x\n", bus_r(STATUS_REG)));

    // as long as no data
    while (!dataPresent) {
        // acquisition done
        if (!runBusy()) {
            // wait to be sure there is no data in fifo
            usleep(WAIT_TME_US_FR_ACQDONE_REG);

            // still no data
            if (!checkDataInFifo()) {
                LOG(logINFO, ("Acquisition Finished (State: 0x%08x), "
                              "no frame found .\n",
                              bus_r(STATUS_REG)));
                return FAIL;
            }
            // got data, exit
            else {
                break;
            }
        }
        // check if data in fifo again
        dataPresent = checkDataInFifo();
    }
    LOG(logDEBUG1, ("Got data :0x%x\n", dataPresent));
    return OK;
}

int readFrameFromFifo() {
    int ns = 0;
    // point the data pointer to the starting position of data
    analogDataPtr = analogData;

    // no data for this frame
    if (checkFifoForEndOfAcquisition() == FAIL) {
        return FAIL;
    }

    // read Sample
    while (ns < nSamples) {
        readSample(ns);
        ns++;
    }

    // got frame
    return OK;
}

uint32_t runBusy() {
#ifdef VIRTUAL
    return ((sharedMemory_getStatus() == RUNNING) ? 1 : 0);
#endif
    uint32_t s = (bus_r(STATUS_REG) & STATUS_RN_BSY_MSK);
    // LOG(logDEBUG1, ("Status Register: %08x\n", s));
    return s;
}

/* common */

int calculateDataBytes() { return dataBytes; }

int getTotalNumberOfChannels() {
    int nchanx = 0, nchany = 0;
    getNumberOfChannels(&nchanx, &nchany);
    return nchanx * nchany;
}

void getNumberOfChannels(int *nchanx, int *nchany) {
    uint32_t mask =
        enableTenGigabitEthernet(-1) ? adcEnableMask_10g : adcEnableMask_1g;
    // count number of channels in x, each adc has 25 channels each
    int nchanTop = __builtin_popcount(mask & 0xF0F0F0F0) * NCHANS_PER_ADC;
    int nchanBot = __builtin_popcount(mask & 0x0F0F0F0F) * NCHANS_PER_ADC;
    *nchanx = nchanTop > 0 ? nchanTop : nchanBot;
    // if both top and bottom adcs enabled, rows = 2
    int nrows = 1;
    if (nchanTop > 0 && nchanBot > 0) {
        nrows = 2;
    }
    *nchany = nSamples / NSAMPLES_PER_ROW * nrows;
}

int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }
