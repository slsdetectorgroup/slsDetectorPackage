// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "RegisterDefs.h"
#include "clogger.h"
#include "common.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#include "LTC2620.h" // dacs
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include "string.h"
#include <netinet/in.h>
#include <unistd.h> // usleep
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern udpStruct udpDetails[MAX_UDP_DESTINATION];
extern const enum detectorType myDetectorType;
extern int ignoreConfigFileFlag;

// Variables that will be exported
int phaseShift = DEFAULT_PHASE_SHIFT;
int masterCommandLine = -1;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char *cmac, int size, uint64_t mac);
extern void getIpAddressinString(char *cip, uint32_t ip);

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int highvoltage = 0;
int64_t virtual_currentFrameNumber = 2;
#endif
int detPos[2] = {};

int detectorFirstServer = 1;
int dacValues[NDAC] = {};
int defaultDacValues[NDAC] = DEFAULT_DAC_VALS;
enum detectorSettings thisSettings = UNINITIALIZED;
enum externalSignalFlag signalMode = 0;

// roi configuration
int adcConfigured = -1;
ROI rois;
int ipPacketSize = 0;
int udpPacketSize = 0;

// master slave configuration (for 25um)
int master = 0;
int masterdefaultdelay = 62;
int patternphase = 0;
int adcphase = 0;
int slavepatternphase = 0;
int slaveadcphase = 0;
int rsttosw1delay = 2;
int startacqdelay = 1;

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
    LOG(logINFOBLUE, ("******** Gotthard Virtual Server *****************\n"));
#else
    LOG(logINFOBLUE, ("**************** Gotthard Server *****************\n"));
#endif
    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Cannot proceed. Check Firmware.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }
#ifndef VIRTUAL
    // does check only if flag is 0 (by default), set by command line
    if ((!debugflag) && (!updateFlag) &&
        ((checkType() == FAIL) || (testFpga() == FAIL) ||
         (testBus() == FAIL))) {
        strcpy(initErrorMessage, "Could not pass basic tests of FPGA and bus. "
                                 "Cannot proceed. Check Firmware.\n");
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

    LOG(logINFOBLUE,
        ("**************************************************\n"
         "Hardware Revision      : %s\n"

         "Detector IP Addr       : 0x%x\n"
         "Detector MAC Addr      : 0x%llx\n\n"

         "Firmware Version       : 0x%llx\n"
         "Software Version       : %s\n"
         "********************************************************\n",
         hversion,

         ipadd, (long long unsigned int)macadd,

         (long long int)fwversion, swversion));

#ifndef VIRTUAL
    if (!debugflag || updateFlag) {
        LOG(logINFO, ("Basic Tests - success\n"));
    }
#endif
}

int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
    u_int32_t type =
        ((bus_r(BOARD_REVISION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
    if (type == DETECTOR_TYPE_MOENCH_VAL) {
        LOG(logERROR,
            ("This is not a Gotthard firmware (read %d, expected ?)\n", type));
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
    u_int32_t val = bus_r(FIX_PATT_REG);
    if (val == FIX_PATT_VAL) {
        LOG(logINFO, ("Fixed pattern: successful match (0x%08x)\n", val));
    } else {
        LOG(logERROR,
            ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n",
             val, FIX_PATT_VAL));
        ret = FAIL;
    }

    if (ret == OK) {
        // dummy reg
        LOG(logINFO, ("\tTesting Dummy Register:\n"));
        u_int32_t addr = DUMMY_REG;
        volatile u_int32_t val = 0, readval = 0;
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
        bus_w(addr, 0);
        if (ret == OK) {
            LOG(logINFO,
                ("Successfully tested FPGA Dummy Register %d times\n", times));
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
    u_int32_t addr = DUMMY_REG;
    volatile u_int32_t val = 0, readval = 0;
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

    bus_w(addr, 0);

    if (ret == OK) {
        LOG(logINFO, ("Successfully tested bus %d times\n", times));
    }
    return ret;
}

void setTestImageMode(int ival) {
    uint32_t addr = MULTI_PURPOSE_REG;
    if (ival >= 0) {
        if (ival == 0) {
            LOG(logINFO, ("Switching off Image Test Mode\n"));
            bus_w(addr, bus_r(addr) & ~DGTL_TST_MSK);
        } else {
            LOG(logINFO, ("Switching on Image Test Mode\n"));
            bus_w(addr, bus_r(addr) | DGTL_TST_MSK);
        }
    }
}

int getTestImageMode() {
    return ((bus_r(MULTI_PURPOSE_REG) & DGTL_TST_MSK) >> DGTL_TST_OFST);
}

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIGOTTHARD); }

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 1;
#endif
    return ((bus_r(FPGA_VERSION_REG) & FPGA_VERSION_MSK) >> FPGA_VERSION_OFST);
}

u_int32_t getDetectorNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return 0;
}

u_int64_t getDetectorMAC() {
#ifdef VIRTUAL
    return 0;
#else
    char output[255], mac[255] = "";
    u_int64_t res = 0;
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

u_int32_t getDetectorIP() {
#ifdef VIRTUAL
    return 0;
#endif
    char temp[INET_ADDRSTRLEN] = "";
    u_int32_t res = 0;
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

void getHardwareVersion(char *version) {
    strcpy(version, "unknown");
    int hwversion = getHardwareVersionNumber();
    const int hwNumberList[] = HARDWARE_VERSION_NUMBERS;
    const char *hwNamesList[] = HARDWARE_VERSION_NAMES;
    for (int i = 0; i != NUM_HARDWARE_VERSIONS; ++i) {
        LOG(logDEBUG, ("0x%x %d 0x%x %s\n", hwversion, i, hwNumberList[i],
                       hwNamesList[i]));
        if (hwNumberList[i] == hwversion) {
            strcpy(version, hwNamesList[i]);
            return;
        }
    }
}

u_int16_t getHardwareVersionNumber() {
#ifdef VIRTUAL
    return 0x2;
#endif
    return ((bus_r(BOARD_REVISION_REG) & BOARD_REVISION_MSK) >>
            BOARD_REVISION_OFST);
}

int isHardwareVersion_1_0() {
    const int hwNumberList[] = HARDWARE_VERSION_NUMBERS;
    return ((getHardwareVersionNumber() == hwNumberList[0]) ? 1 : 0);
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
#else
        // ismaster from variable in stop server
        if (readConfigFile() == FAIL ||
            checkCommandLineConfiguration() == FAIL) {
            initCheckDone = 1;
            return;
        }
#endif
    }
    initCheckDone = 1;
}

/* set up detector */

void setupDetector() {
    LOG(logINFO, ("This Server is for 1 Gotthard module (1280 channels)\n"));

#ifdef VIRTUAL
    if (isControlServer) {
        sharedMemory_setStatus(IDLE);
        setupUDPCommParameters();
    } else {
        sharedMemory_setStop(0);
    }
#endif

    // Initialization
    setPhaseShiftOnce();

    // hv
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);

    // adc
    if (isHardwareVersion_1_0()) {
        AD9252_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK,
                          ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK,
                          ADC_SPI_SRL_DT_OTPT_OFST);
        AD9252_Disable();
        AD9252_Configure();
    } else {
        AD9257_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK,
                          ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK,
                          ADC_SPI_SRL_DT_OTPT_OFST);
        AD9257_Disable();
        AD9257_Configure();
    }

    // dac
    LTC2620_SetDefines(SPI_REG, SPI_DAC_SRL_CS_OTPT_MSK,
                       SPI_DAC_SRL_CLK_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_MSK,
                       SPI_DAC_SRL_DGTL_OTPT_OFST, NDAC, DAC_MIN_MV,
                       DAC_MAX_MV);
    LTC2620_Disable();
    LTC2620_Configure();
    resetToDefaultDacs(0);

    // temp
    bus_w(TEMP_SPI_IN_REG, TEMP_SPI_IN_IDLE_MSK);
    bus_w(TEMP_SPI_OUT_REG, 0x0);

    // roi, gbit readout
    rois.xmin = -1;
    rois.xmax = -1;
    rois.ymin = -1;
    rois.ymax = -1;
    setROI(rois); // set adcsyncreg, daqreg, chipofinterestreg, cleanfifos,
    setGbitReadout();

    // no config file or not first time server
    if (readConfigFile() == FAIL)
        return;

    if (checkCommandLineConfiguration() == FAIL)
        return;

    // master, slave (25um)
    setMasterSlaveConfiguration();

    // Default Parameters
    LOG(logINFOBLUE, ("Setting Default parameters\n"));

    setSettings(DEFAULT_SETTINGS);
    setExtSignal(0, DEFAULT_TRIGGER_MODE);
    setTiming(DEFAULT_TIMING_MODE);
    setNumFrames(DEFAULT_NUM_FRAMES);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setExpTime(DEFAULT_EXPTIME);
    setPeriod(DEFAULT_PERIOD);
    setDelayAfterTrigger(DEFAULT_DELAY);
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

void writeRegister16And32(uint32_t offset, uint32_t data) {
    if (((offset << MEM_MAP_SHIFT) == CONTROL_REG) ||
        ((offset << MEM_MAP_SHIFT) == FIFO_DATA_REG)) {
        writeRegister16(offset, data);
    } else
        writeRegister(offset, data);
}

uint32_t readRegister16And32(uint32_t offset) {
    if (((offset << MEM_MAP_SHIFT) == CONTROL_REG) ||
        ((offset << MEM_MAP_SHIFT) == FIFO_DATA_REG)) {
        return readRegister16(offset);
    } else
        return readRegister(offset);
}

/* firmware functions (resets) */

void setPhaseShiftOnce() {
    u_int32_t addr = MULTI_PURPOSE_REG;
    volatile u_int32_t val = bus_r(addr);
    LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));

    // first time detector has switched on
    if (!val) {
        detectorFirstServer = 1;
        LOG(logINFO,
            ("Implementing the first phase shift of %d\n", phaseShift));
        for (int times = 1; times < phaseShift; ++times) {
            bus_w(addr, (INT_RSTN_MSK | ENT_RSTN_MSK | SW1_MSK |
                         PHS_STP_MSK)); // 0x1821
            bus_w(addr, (INT_RSTN_MSK | ENT_RSTN_MSK |
                         (SW1_MSK & ~PHS_STP_MSK))); // 0x1820
        }
        LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));
    } else
        detectorFirstServer = 0;
}

void setPhaseShift(int numphaseshift) {
    LOG(logINFO, ("Implementing phase shift of %d\n", numphaseshift));
    u_int32_t addr = MULTI_PURPOSE_REG;

    volatile u_int32_t val = bus_r(addr);
    LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));
    for (int times = 1; times < numphaseshift; ++times) {
        bus_w(addr, val | PHS_STP_MSK);
        bus_w(addr, val & (~PHS_STP_MSK));
    }
    LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));
}

void cleanFifos() {
    LOG(logINFO, ("Cleaning FIFOs\n"));
    bus_w(ADC_SYNC_REG, bus_r(ADC_SYNC_REG) | ADC_SYNC_CLEAN_FIFOS_MSK);
    bus_w(ADC_SYNC_REG, bus_r(ADC_SYNC_REG) & ~ADC_SYNC_CLEAN_FIFOS_MSK);
}

void setADCSyncRegister() {
    LOG(logINFO, ("\tSetting ADC Sync and Token Delays:\n"));
    u_int32_t addr = ADC_SYNC_REG;

    // 0x88(no roi), 0x1b(roi) (MSB)
    u_int32_t tokenDelay =
        ((adcConfigured == -1) ? ADC_SYNC_ENET_DELAY_NO_ROI_VAL
                               : ADC_SYNC_ENET_DELAY_ROI_VAL);

    // 0x88032214(no roi), 0x1b032214(with roi)
    u_int32_t val = (ADC_SYNC_TKN_VAL | tokenDelay);

    bus_w(addr, val);
    LOG(logINFO, ("\tADC Sync Reg: 0x%x\n", bus_r(addr)));
}

void setDAQRegister() {
    LOG(logINFO, ("\tSetting Packet Length and DAQ Token Timing:\n"));
    u_int32_t addr = DAQ_REG;

    // 0x1f16(board rev 1) 0x1f0f(board rev 2)
    u_int32_t tokenTiming =
        ((isHardwareVersion_1_0()) ? DAQ_TKN_TMNG_BRD_RVSN_1_VAL
                                   : DAQ_TKN_TMNG_BRD_RVSN_2_VAL);

    // 0x13f(no roi), 0x7f(roi)
    u_int32_t packetLength = ((adcConfigured == -1) ? DAQ_PCKT_LNGTH_NO_ROI_VAL
                                                    : DAQ_PCKT_LNGTH_ROI_VAL);

    // MSB: packetLength LSB: tokenTiming
    u_int32_t val = (tokenTiming | packetLength);

    bus_w(addr, val);
    LOG(logINFO, ("\tDAQ Reg: 0x%x\n", bus_r(addr)));
}

void setChipOfInterestRegister(int adc) {
    LOG(logINFO, ("\tSelecting Chips of Interst:\n"));
    u_int32_t addr = CHIP_OF_INTRST_REG;

    // 0x1f(no roi), 0xXX(roi)
    u_int32_t adcSelect =
        ((adcConfigured == -1) ? CHIP_OF_INTRST_ADC_SEL_MSK
                               : (((1 << adc) << CHIP_OF_INTRST_ADC_SEL_OFST) &
                                  CHIP_OF_INTRST_ADC_SEL_MSK));

    // 0x0500(no roi), 0x0100(roi)
    u_int32_t numChannels =
        (adcConfigured == -1) ? (NCHIP * NCHAN) : (NCHIPS_PER_ADC * NCHAN);
    numChannels = ((numChannels << CHIP_OF_INTRST_NUM_CHNNLS_OFST) &
                   CHIP_OF_INTRST_NUM_CHNNLS_MSK);

    // 0x500001f(no roi), 0x10000xx(roi) MSB:num channels, LSB: selected ADC
    u_int32_t val = (numChannels | adcSelect);

    bus_w(addr, val);
    LOG(logINFO, ("\tChip Of Interest Reg: 0x%x\n", bus_r(addr)));
}

void setROIADC(int adc) {
    LOG(logINFO, ("\tSetting ROI ADC: %d\n", adc));
    adcConfigured = adc;

    setADCSyncRegister();           // adc sync & token delays
    setDAQRegister();               // packet length & token timing
    cleanFifos();                   // clean fifos
    setChipOfInterestRegister(adc); // num channels & select adc

    ipPacketSize =
        ((adcConfigured == -1) ? IP_PACKET_SIZE_NO_ROI : IP_PACKET_SIZE_ROI);
    udpPacketSize =
        ((adcConfigured == -1) ? UDP_PACKETSIZE_NO_ROI : UDP_PACKETSIZE_ROI);
}

void setGbitReadout() {
    LOG(logINFO, ("Setting Gbit Readout\n"));
    u_int32_t addr = CONFIG_REG;
    bus_w(addr, bus_r(addr) & ~CONFIG_CPU_RDT_MSK);
    LOG(logINFO, ("\tConfig Reg 0x%x\n", bus_r(addr)));
}

int readConfigFile() {

    if (initError == FAIL) {
        return initError;
    }

    if (ignoreConfigFileFlag) {
        LOG(logWARNING, ("Ignoring Config file\n"));
        return OK;
    }

    const int fileNameSize = 128;
    char fname[fileNameSize];
    if (getAbsPath(fname, fileNameSize, CONFIG_FILE) == FAIL) {
        return FAIL;
    }

    // open config file
    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        LOG(logWARNING, ("Could not find config file %s\n", CONFIG_FILE));
        return FAIL;
    }
    LOG(logINFO, ("\tConfig file %s opened\n", CONFIG_FILE));

    // Initialization
    const size_t lineSize = 256;
    char line[lineSize];
    memset(line, 0, lineSize);
    const size_t keySize = lineSize / 2;
    char key[keySize];
    memset(key, 0, keySize);
    char value[keySize];
    memset(value, 0, keySize);

    // keep reading a line
    while (fgets(line, lineSize, fd)) {
        // ignore comments
        if (line[0] == '#')
            continue;
        // read key & value
        sscanf(line, "%s %s\n", key, value);

        // key is master/ slave flag
        if (!strcasecmp(key, "masterflags")) {
            if (!strcasecmp(value, "is_master")) {
                master = 1;
                LOG(logINFOBLUE, ("\tMaster\n"));
            } else if ((!strcasecmp(value, "is_slave")) ||
                       (!strcasecmp(value, "no_master"))) {
                master = 0;
                LOG(logINFOBLUE, ("\tSlave or No Master\n"));
            } else {
                initError = FAIL;
                sprintf(
                    initErrorMessage,
                    "Could not scan masterflags %s value from config file\n",
                    value);
                LOG(logERROR, (initErrorMessage))
                fclose(fd);
                return FAIL;
            }

            // not first server since detector power on
            if (!detectorFirstServer) {
                LOG(logWARNING, ("\tServer has been started up before. "
                                 "Ignoring rest of config file\n"));
                fclose(fd);
                return OK;
            }
        }

        // key is not master/ slave flag
        else {
            // convert value to int
            int ival = 0;
            if (sscanf(value, "%d", &ival) <= 0) {
                initError = FAIL;
                sprintf(initErrorMessage,
                        "Could not scan parameter %s value %s from "
                        "config file\n",
                        key, value);
                LOG(logERROR, (initErrorMessage))
                fclose(fd);
                return FAIL;
            }
            // set value
            if (!strcasecmp(key, "masterdefaultdelay"))
                masterdefaultdelay = ival;
            else if (!strcasecmp(key, "patternphase"))
                patternphase = ival;
            else if (!strcasecmp(key, "adcphase"))
                adcphase = ival;
            else if (!strcasecmp(key, "slavepatternphase"))
                slavepatternphase = ival;
            else if (!strcasecmp(key, "slaveadcphase"))
                slaveadcphase = ival;
            else if (!strcasecmp(key, "rsttosw1delay"))
                rsttosw1delay = ival;
            else if (!strcasecmp(key, "startacqdelay"))
                startacqdelay = ival;
            else {
                initError = FAIL;
                sprintf(initErrorMessage,
                        "Could not scan parameter %s from config file\n", key);
                LOG(logERROR, (initErrorMessage))
                fclose(fd);
                return FAIL;
            }
        }
    }
    fclose(fd);

    LOG(logINFOBLUE,
        ("\tmasterdefaultdelay:%d\n"
         "\tpatternphase:%d\n"
         "\tadcphase:%d\n"
         "\tslavepatternphase:%d\n"
         "\tslaveadcphase:%d\n"
         "\trsttosw1delay:%d\n"
         "\tstartacqdelay:%d\n",
         masterdefaultdelay, patternphase, adcphase, slavepatternphase,
         slaveadcphase, rsttosw1delay, startacqdelay));
    return OK;
}

int checkCommandLineConfiguration() {
    if (masterCommandLine != -1) {
#ifdef VIRTUAL
        master = masterCommandLine;
#else
        initError = FAIL;
        strcpy(initErrorMessage,
               "Cannot set Master from command line for this detector. "
               "Should have been caught before!\n");
        return FAIL;
#endif
    }
    return OK;
}

void setMasterSlaveConfiguration() {
    // not the first time its being read
    if (!detectorFirstServer) {
        return;
    }

    LOG(logINFO, ("Reading Master Slave Configuration\n"));
    // master configuration
    if (master) {
        // master default delay set, so reset delay
        setDelayAfterTrigger(0);

        // Set pattern phase for the master module
        u_int32_t val =
            (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_MSTR_VAL);
        setPhaseShift(patternphase);

        // Set adc phase for the master module
        val = (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_MSTR_ADC_VAL);
        setPhaseShift(adcphase);

        // Set pattern phase for the slave module
        val = (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_SLV_VAL);
        setPhaseShift(slavepatternphase);

        // Set adc phase for the slave module
        val = (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_SLV_ADC_VAL);
        setPhaseShift(slaveadcphase);

        // Set start acq delay
        val = (bus_r(MULTI_PURPOSE_REG) & (~(STRT_ACQ_DLY_MSK))); // unset mask
        val = val | ((startacqdelay << STRT_ACQ_DLY_OFST) &
                     STRT_ACQ_DLY_MSK); // set val
        bus_w(MULTI_PURPOSE_REG, val);
        LOG(logDEBUG1, ("\tMultipurpose reg: 0x%x\n", val));
    }

    // all configuration - Set RST to SW1 delay
    u_int32_t val =
        (bus_r(MULTI_PURPOSE_REG) & (~(RST_TO_SW1_DLY_MSK))); // unset mask
    val = val | ((rsttosw1delay << RST_TO_SW1_DLY_OFST) &
                 RST_TO_SW1_DLY_MSK); // set val
    bus_w(MULTI_PURPOSE_REG, val);
    LOG(logDEBUG1, ("\tMultipurpose reg: 0x%x\n", val));

    LOG(logINFO, ("\tMaster Slave Configuration has been set up\n"));
}

/* set parameters -  dr, roi */

int setDynamicRange(int dr) {
    if (dr == 16)
        return OK;
    return FAIL;
}

int getDynamicRange(int *retval) {
    *retval = DYNAMIC_RANGE;
    return OK;
}

int setROI(ROI arg) {

    int adc = -1;
    if (arg.xmin == -1) {
        LOG(logINFO, ("Clearing ROI\n"));
        rois.xmin = -1;
        rois.xmax = -1;
        rois.ymin = -1;
        rois.ymax = -1;
    } else {
        LOG(logINFO, ("Setting ROI:(%d, %d, %d, %d)\n", arg.xmin, arg.xmax,
                      arg.ymin, arg.ymax));
        // validation
        // xmin divisible by 256 and less than 1280
        if (((arg.xmin % NCHAN_PER_ADC) != 0) ||
            (arg.xmin >= (NCHAN * NCHIP))) {
            LOG(logERROR, ("Could not set roi. xmin is invalid\n"));
            return FAIL;
        }
        // xmax must be 255 more than xmin
        if (arg.xmax != (arg.xmin + NCHAN_PER_ADC - 1)) {
            LOG(logERROR, ("Could not set roi. xmax is invalid\n"));
            return FAIL;
        }
        rois.xmin = arg.xmin;
        rois.xmax = arg.xmax;
        adc = arg.xmin / NCHAN_PER_ADC;
    }
    LOG(logINFO, ("\tAdc to be configured: %d\n", adc));
    LOG(logINFO,
        ("\tROI to be configured: (%d, %d)\n", (adc == -1) ? 0 : (rois.xmin),
         (adc == -1) ? (NCHIP * NCHAN - 1) : (rois.xmax)));

    // set adc of interest
    setROIADC(adc);
    return OK;
}

ROI getROI() {
    LOG(logINFO, ("Getting ROI:\n"));

    // print
    if (rois.xmin == -1) {
        LOG(logINFO, ("\tROI: None\n"));
    } else {
        LOG(logINFO, ("ROI: (%d,%d,%d,%d)\n", rois.xmin, rois.xmax, rois.ymin,
                      rois.ymax));
    }
    return rois;
}

/* parameters - timer */
void setNumFrames(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of frames %lld\n", (long long int)val));
        set64BitReg(val, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
    }
}

int64_t getNumFrames() {
    return get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}

void setNumTriggers(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of triggers %lld\n", (long long int)val));
        set64BitReg(val, SET_TRAINS_LSB_REG, SET_TRAINS_MSB_REG);
    }
}

int64_t getNumTriggers() {
    return get64BitReg(SET_TRAINS_LSB_REG, SET_TRAINS_MSB_REG);
}

int setExpTime(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid exptime: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
    val = (val * 1E-9 * CLK_FREQ) + 0.5;
    set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG);

    // validate for tolerance
    int64_t retval = getExpTime();
    val /= (1E-9 * CLK_FREQ);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getExpTime() {
    return get64BitReg(SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) /
           (1E-9 * CLK_FREQ);
}

int setPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid period: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    val = (val * 1E-9 * CLK_FREQ) + 0.5;
    set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-9 * CLK_FREQ);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return get64BitReg(SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
           (1E-9 * CLK_FREQ);
}

int setDelayAfterTrigger(int64_t val) {
    if (val < 0) {
        LOG(logERROR,
            ("Invalid delay after trigger: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting delay after trigger %lld ns\n", (long long int)val));
    if (master) {
        val += masterdefaultdelay;
        LOG(logINFO, ("\tActual Delay (master): %lld\n", (long long int)val));
    }
    val =
        (val * 1E-9 * CLK_FREQ) +
        0.5; // because of the master delay of 62 ns (not really double of
             // clkfreq), losing precision and 0 delay becomes -31ns, so adding
             // +0.5. Also adding +0.5 for more tolerance for gotthard1.
    set64BitReg(val, SET_DELAY_LSB_REG, SET_DELAY_MSB_REG);

    // validate for tolerance
    int64_t retval = getDelayAfterTrigger();
    val /= (1E-9 * CLK_FREQ);
    if (master) {
        val -= masterdefaultdelay;
    }
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getDelayAfterTrigger() {
    int64_t retval =
        get64BitReg(SET_DELAY_LSB_REG, SET_DELAY_MSB_REG) / (1E-9 * CLK_FREQ);
    if (master) {
        LOG(logDEBUG1,
            ("\tActual Delay read (master): %lld\n", (long long int)retval));
        retval -= masterdefaultdelay;
    }
    return retval;
}

int64_t getNumFramesLeft() {
    return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t getNumTriggersLeft() {
    return get64BitReg(GET_TRAINS_LSB_REG, GET_TRAINS_MSB_REG);
}

int64_t getPeriodLeft() {
    return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) /
           (1E-9 * CLK_FREQ);
}

int64_t getDelayAfterTriggerLeft() {
    int64_t retval =
        get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) / (1E-9 * CLK_FREQ);
    if (master) {
        LOG(logDEBUG1,
            ("\tGetting Actual delay (master): %lld\n", (long long int)retval));
        retval -= masterdefaultdelay;
    }
    return retval;
}

int64_t getExpTimeLeft() {
    return get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG) /
           (1E-9 * CLK_FREQ);
}

/* parameters - channel, chip, module, settings */

int setModule(sls_detector_module myMod, char *mess) {

    LOG(logINFO, ("Setting module with settings %d\n", myMod.reg));

    // settings
    setSettings((enum detectorSettings)myMod.reg);

    // set dac values
    for (int i = 0; i < NDAC; ++i)
        setDAC((enum DACINDEX)i, myMod.dacs[i], 0);
    return OK;
}

enum detectorSettings setSettings(enum detectorSettings sett) {
    if (sett == UNINITIALIZED)
        return thisSettings;

    // set settings
    u_int32_t addr = GAIN_REG;

    // find conf gain value
    uint32_t confgain = 0x0;
    switch (sett) {
    case DYNAMICGAIN:
        LOG(logINFO, ("Set settings - Dyanmic Gain\n"));
        confgain = GAIN_CONFGAIN_DYNMC_GAIN_VAL;
        break;
    case HIGHGAIN:
        LOG(logINFO, ("Set settings - High Gain\n"));
        confgain = GAIN_CONFGAIN_HGH_GAIN_VAL;
        break;
    case LOWGAIN:
        LOG(logINFO, ("Set settings - Low Gain\n"));
        confgain = GAIN_CONFGAIN_LW_GAIN_VAL;
        break;
    case MEDIUMGAIN:
        LOG(logINFO, ("Set settings - Medium Gain\n"));
        confgain = GAIN_CONFGAIN_MDM_GAIN_VAL;
        break;
    case VERYHIGHGAIN:
        LOG(logINFO, ("Set settings - Very High Gain\n"));
        confgain = GAIN_CONFGAIN_VRY_HGH_GAIN_VAL;
        break;
    default:
        LOG(logERROR,
            ("This settings is not defined for this detector %d\n", (int)sett));
        return -1;
    }
    // set conf gain
    bus_w(addr, bus_r(addr) & ~GAIN_CONFGAIN_MSK);
    bus_w(addr, bus_r(addr) | confgain);
    LOG(logINFO, ("\tGain Reg: 0x%x\n", bus_r(addr)));
    thisSettings = sett;

    return getSettings();
}

enum detectorSettings getSettings() {
    uint32_t regval = bus_r(GAIN_REG);
    uint32_t val = regval & GAIN_CONFGAIN_MSK;
    switch (val) {
    case GAIN_CONFGAIN_DYNMC_GAIN_VAL:
        LOG(logDEBUG1,
            ("Settings read: Dynamic Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = DYNAMICGAIN;
        break;
    case GAIN_CONFGAIN_HGH_GAIN_VAL:
        LOG(logDEBUG1, ("Settings read: High Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = HIGHGAIN;
        break;
    case GAIN_CONFGAIN_LW_GAIN_VAL:
        LOG(logDEBUG1, ("Settings read: Low Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = LOWGAIN;
        break;
    case GAIN_CONFGAIN_MDM_GAIN_VAL:
        LOG(logDEBUG1,
            ("Settings read: Medium Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = MEDIUMGAIN;
        break;
    case GAIN_CONFGAIN_VRY_HGH_GAIN_VAL:
        LOG(logDEBUG1,
            ("Settings read: Very High Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = VERYHIGHGAIN;
        break;
    default:
        thisSettings = UNDEFINED;
        LOG(logERROR, ("Settings read: Undefined. Gain Reg: 0x%x\n", regval));
    }

    return thisSettings;
}

/* parameters - dac, adc, hv */

void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0)
        return;

    char *dac_names[] = {DAC_NAMES};
    LOG(logINFO, ("Setting DAC %s\n", dac_names[ind]));
    LOG(logDEBUG1, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                    val, (mV ? "mV" : "dac units")));
    int dacval = val;
#ifdef VIRTUAL
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

int getADC(enum ADCINDEX ind) {
#ifdef VIRTUAL
    return 0;
#endif
    char tempnames[2][40] = {"VRs/FPGAs Temperature", "ADCs/ASICs Temperature"};
    LOG(logDEBUG1, ("Getting Temperature for %s\n", tempnames[ind]));

    u_int32_t addr = TEMP_SPI_IN_REG;
    uint32_t addrout = TEMP_SPI_OUT_REG;
    const int repeats = 6; // number of register writes for delay
    const int reads = 20;
    u_int32_t value = 0;

    // standby, high clk, high cs
    bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T1_CS_MSK |
                 TEMP_SPI_IN_T2_CLK_MSK | TEMP_SPI_IN_T2_CS_MSK));

    // high clk low cs
    bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T2_CLK_MSK));

    for (int i = 0; i < reads; ++i) {

        // low clk low cs
        for (int j = 0; j < repeats; ++j)
            bus_w(addr, 0x0);
        // high clk low cs
        for (int j = 0; j < repeats; ++j)
            bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T2_CLK_MSK));

        // only the first time
        if (i <= 10) {
            if (ind == TEMP_ADC)
                value =
                    (value << 1) + (bus_r(addrout) & TEMP_SPI_OUT_T1_DT_MSK);
            else
                value =
                    (value << 1) + (bus_r(addrout) & TEMP_SPI_OUT_T2_DT_MSK);
        }
    }

    // standby high clk, high cs
    bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T1_CS_MSK |
                 TEMP_SPI_IN_T2_CLK_MSK | TEMP_SPI_IN_T2_CS_MSK));

    LOG(logDEBUG1, ("\tInitial Temperature value: %u\n", value));
    // conversion
    value = value / 4.0;
    LOG(logINFO, ("\tTemperature %s: %f °C\n", tempnames[ind], value));
    return value;
}

int setHighVoltage(int val) {
    u_int32_t addr = HV_REG;
    u_int32_t sel = 0x0;

    // set
    if (val >= 0) {
        LOG(logINFO, ("Setting High Voltage to %d\n", val));
        switch (val) {
        case 0:
            break;
        case 90:
            sel = HV_SEL_90_VAL;
            break;
        case 110:
            sel = HV_SEL_110_VAL;
            break;
        case 120:
            sel = HV_SEL_120_VAL;
            break;
        case 150:
            sel = HV_SEL_150_VAL;
            break;
        case 180:
            sel = HV_SEL_180_VAL;
            break;
        case 200:
            sel = HV_SEL_200_VAL;
            break;
        default:
            LOG(logERROR,
                ("%d high voltage is not defined for this detector\n", val));
            return setHighVoltage(-1);
        }
        LOG(logDEBUG1, ("\tHigh voltage value to be sent: 0x%x\n", sel));

        // switch off high voltage
        bus_w(addr, (bus_r(addr) & ~HV_ENBL_MSK));

        // unset mask and set value
        bus_w(addr, (bus_r(addr) & ~HV_SEL_MSK) | sel);

        // switch on high voltage
        if (val > 0)
            bus_w(addr, bus_r(addr) | HV_ENBL_MSK);
    }

    // get
    u_int32_t retval = 0;
    u_int32_t regval = bus_r(addr);
    LOG(logDEBUG1, ("\tHigh voltage value read: 0x%x\n", regval));

    // if high voltage was enabled, find value
    if (regval & HV_ENBL_MSK) {
        switch (regval & HV_SEL_MSK) {
        case HV_SEL_90_VAL:
            retval = 90;
            break;
        case HV_SEL_110_VAL:
            retval = 110;
            break;
        case HV_SEL_120_VAL:
            retval = 120;
            break;
        case HV_SEL_150_VAL:
            retval = 150;
            break;
        case HV_SEL_180_VAL:
            retval = 180;
            break;
        default:
            retval = 200;
            break;
        }
    }
    LOG(logDEBUG1, ("\tHigh Voltage: %d\n", retval));
    return retval;
}

/* parameters - timing, extsig */

int isMaster(int *retval) {
    *retval = master;
    return OK;
}

void setTiming(enum timingMode arg) {
    u_int32_t addr = EXT_SIGNAL_REG;
    switch (arg) {
    case AUTO_TIMING:
        LOG(logINFO, ("Set Timing: Auto\n"));
        bus_w(addr, EXT_SIGNAL_OFF_VAL);
        break;
    case TRIGGER_EXPOSURE:
        if (signalMode == TRIGGER_IN_FALLING_EDGE) {
            LOG(logINFO, ("Set Timing: Trigger (Falling Edge)\n"));
            bus_w(addr, EXT_SIGNAL_TRGGR_IN_FLLNG_VAL);
        } else {
            LOG(logINFO, ("Set Timing: Trigger (Rising Edge)\n"));
            bus_w(addr, EXT_SIGNAL_TRGGR_IN_RSNG_VAL);
        }
        break;
    default:
        LOG(logERROR, ("Unknown timing mode %d for this detector\n", (int)arg));
    }
}

enum timingMode getTiming() {
    u_int32_t regval = bus_r(EXT_SIGNAL_REG);
    switch (regval) {
    case EXT_SIGNAL_TRGGR_IN_RSNG_VAL:
    case EXT_SIGNAL_TRGGR_IN_FLLNG_VAL:
        return TRIGGER_EXPOSURE;
    default:
        return AUTO_TIMING;
    }
}

void setExtSignal(int signalIndex, enum externalSignalFlag mode) {
    LOG(logDEBUG1, ("Setting signal flag[%d] to %d\n", signalIndex, mode));
    switch (mode) {
    case TRIGGER_IN_RISING_EDGE:
        LOG(logINFO,
            ("Setting External Signal flag: Trigger in Rising Edge\n"));
        break;
    case TRIGGER_IN_FALLING_EDGE:
        LOG(logINFO,
            ("Setting External Signal flag: Trigger in Falling Edge\n"));
        break;
    default:
        LOG(logERROR,
            ("Extsig (signal mode) %d not defined for this detector\n", mode));
        return;
    }
    signalMode = mode;
    setTiming(getTiming());
}

int getExtSignal(int signalIndex) {
    LOG(logDEBUG1, ("Getting signal flag[%d]\n", signalIndex));
    return signalMode;
}

/* configure mac */

int getNumberofUDPInterfaces() { return 1; }

void calcChecksum(mac_conf *mac, int sourceip, int destip) {
    mac->ip.ip_ver = 0x4;
    mac->ip.ip_ihl = 0x5;
    mac->ip.ip_tos = 0x0;
    mac->ip.ip_len = ipPacketSize;
    mac->ip.ip_ident = 0x0000;
    mac->ip.ip_flag = 0x2; // not nibble aligned (flag& offset
    mac->ip.ip_offset = 0x00;
    mac->ip.ip_ttl = 0x70;
    mac->ip.ip_protocol = 0x11;
    mac->ip.ip_chksum = 0x0000; // pseudo
    mac->ip.ip_sourceip = sourceip;
    mac->ip.ip_destip = destip;
    LOG(logDEBUG1, ("\tIP TTL: 0x%x\n", mac->ip.ip_ttl));

    int count = sizeof(mac->ip);
    unsigned short *addr;
    addr = (unsigned short *)(&(
        mac->ip)); /* warning: assignment from incompatible pointer type */

    long int sum = 0;
    while (count > 1) {
        sum += *addr++;
        count -= 2;
    }
    if (count > 0)
        sum += *addr; // Add left-over byte, if any
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16); // Fold 32-bit sum to 16 bits
    long int checksum = (~sum) & 0xffff;
    LOG(logINFO, ("\tIP checksum : 0x%lx\n", checksum));
    mac->ip.ip_chksum = checksum;
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
        LOG(logERROR, ("could not set udp 1G destination IP and port\n"));
        return FAIL;
    }
    return OK;
#endif
    u_int32_t addr = MULTI_PURPOSE_REG;

    LOG(logDEBUG1, ("\tRoi: %d, Ip Packet size: %d UDP Packet size: %d\n",
                    adcConfigured, ipPacketSize, udpPacketSize));

    // reset mac
    bus_w(addr, bus_r(addr) | RST_MSK);
    LOG(logDEBUG1, ("\tReset Mac. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    usleep(500000);

    // release reset
    bus_w(addr, bus_r(addr) & (~RST_MSK));
    LOG(logDEBUG1, ("\tReset released. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    // write shadow regs
    bus_w(addr, bus_r(addr) | (ENT_RSTN_MSK | WRT_BCK_MSK));
    LOG(logDEBUG1,
        ("\tWrite shadow regs. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    // release write back
    bus_w(addr, bus_r(addr) & (~WRT_BCK_MSK));
    LOG(logDEBUG1,
        ("\tWrite back released. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    LOG(logDEBUG1, ("\tConfiguring MAC CONF\n"));
    mac_conf *mac_conf_regs = (mac_conf *)(Blackfin_getBaseAddress() +
                                           ENET_CONF_REG / 2); // direct write
    mac_conf_regs->mac.mac_dest_mac1 = ((dstmac >> (8 * 5)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac2 = ((dstmac >> (8 * 4)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac3 = ((dstmac >> (8 * 3)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac4 = ((dstmac >> (8 * 2)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac5 = ((dstmac >> (8 * 1)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac6 = ((dstmac >> (8 * 0)) & 0xFF);
    LOG(logDEBUG1,
        ("\tDestination Mac: %llx %x:%x:%x:%x:%x:%x\n", dstmac,
         mac_conf_regs->mac.mac_dest_mac1, mac_conf_regs->mac.mac_dest_mac2,
         mac_conf_regs->mac.mac_dest_mac3, mac_conf_regs->mac.mac_dest_mac4,
         mac_conf_regs->mac.mac_dest_mac5, mac_conf_regs->mac.mac_dest_mac6));
    mac_conf_regs->mac.mac_src_mac1 = ((srcmac >> (8 * 5)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac2 = ((srcmac >> (8 * 4)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac3 = ((srcmac >> (8 * 3)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac4 = ((srcmac >> (8 * 2)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac5 = ((srcmac >> (8 * 1)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac6 = ((srcmac >> (8 * 0)) & 0xFF);
    LOG(logDEBUG1,
        ("\tSource Mac: %llx %x:%x:%x:%x:%x:%x\n", srcmac,
         mac_conf_regs->mac.mac_src_mac1, mac_conf_regs->mac.mac_src_mac2,
         mac_conf_regs->mac.mac_src_mac3, mac_conf_regs->mac.mac_src_mac4,
         mac_conf_regs->mac.mac_src_mac5, mac_conf_regs->mac.mac_src_mac6));
    mac_conf_regs->mac.mac_ether_type = 0x0800; // ipv4

    calcChecksum(mac_conf_regs, srcip, dstip);
    mac_conf_regs->udp.udp_srcport = srcport;
    mac_conf_regs->udp.udp_destport = dstport;
    mac_conf_regs->udp.udp_len = udpPacketSize;
    mac_conf_regs->udp.udp_chksum = 0x0000;

    LOG(logDEBUG1, ("\tConfiguring TSE\n"));
    tse_conf *tse_conf_regs = (tse_conf *)(Blackfin_getBaseAddress() +
                                           TSE_CONF_REG / 2); // direct write
    tse_conf_regs->rev = 0xA00;
    tse_conf_regs->scratch = 0xCCCCCCCC;
    tse_conf_regs->command_config = 0xB;
    tse_conf_regs->mac_0 = 0x17231C00;
    tse_conf_regs->mac_1 = 0xCB4A;
    tse_conf_regs->frm_length =
        0x5DC; // max frame length (1500 bytes) (was 0x41C)
    tse_conf_regs->pause_quant = 0x0;
    tse_conf_regs->rx_section_empty = 0x7F0;
    tse_conf_regs->rx_section_full = 0x10;
    tse_conf_regs->tx_section_empty = 0x3F8; // was 0x7F0;
    tse_conf_regs->tx_section_full = 0x16;
    tse_conf_regs->rx_almost_empty = 0x8;
    tse_conf_regs->rx_almost_full = 0x8;
    tse_conf_regs->tx_almost_empty = 0x8;
    tse_conf_regs->tx_almost_full = 0x3;
    tse_conf_regs->mdio_addr0 = 0x12;
    tse_conf_regs->mdio_addr1 = 0x0;
    mac_conf_regs->cdone = 0xFFFFFFFF;

    bus_w(addr, bus_r(addr) | (INT_RSTN_MSK | WRT_BCK_MSK));
    LOG(logDEBUG1,
        ("\tWrite shadow regs with int reset. MultiPurpose reg: 0x%x\n",
         bus_r(addr)));

    usleep(100000);

    // release write back
    bus_w(addr, bus_r(addr) & (~WRT_BCK_MSK));
    LOG(logDEBUG1,
        ("\tWrite back released. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    bus_w(addr, bus_r(addr) | SW1_MSK);
    LOG(logDEBUG1, ("\tSw1. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    usleep(1000 * 1000);
    LOG(logDEBUG1, ("\tConfigure Mac Done\n"));
    {
        /** send out first image as first packet does not give 0xcacacaca
         * (needed to know if first image when switching back and forth between
         * roi and no roi
         */
        LOG(logINFOBLUE, ("Sending an image to counter the packet numbers\n"));
        // remember old parameters
        enum timingMode oldtiming = getTiming();
        uint64_t oldframes = getNumFrames();
        uint64_t oldtriggers = getNumTriggers();
        uint64_t oldPeriod = getPeriod();
        uint64_t oldExptime = getExpTime();

        // set to basic parameters
        LOG(logINFO, ("\tSetting basic parameters\n"
                      "\tTiming: auto\n"
                      "\tframes: 1\n"
                      "\ttriggers: 1\n"
                      "\tperiod: 1s\n"
                      "\texptime: 900ms\n"));
        setTiming(AUTO_TIMING);
        setNumFrames(1);
        setNumTriggers(1);
        setPeriod(1e9); // important to keep this until we have to wait for
                        // acquisition to start
        setExpTime(900 * 1000);

        // take an image
        if (master)
            usleep(1 * 1000 * 1000); // required to ensure master starts
                                     // acquisition only after slave has changed
                                     // to basic parameters and is waiting

        int loop = 0;
        startStateMachine();
        // wait for acquisition to start (trigger from master)
        LOG(logINFO, ("\tWaiting for acquisition to start\n"));
        while (!runBusy()) {
            usleep(0);
            ++loop;
        }

        LOG(logINFO, ("\twaited %d loops to start\n", loop));
        LOG(logINFO, ("\tWaiting for acquisition to end (frames left: %lld)\n",
                      (long long int)getNumFramesLeft()));
        // wait for status to be done
        while (runBusy()) {
            usleep(500);
        }

        // set to previous parameters
        LOG(logINFO, ("\tSetting previous parameters:\n"
                      "\tTiming: %d\n"
                      "\tframes: %lld\n"
                      "\ttriggers: %lld\n"
                      "\tperiod: %lld ns\n"
                      "\texptime:%lld ns\n",
                      (int)oldtiming, (long long int)oldframes,
                      (long long int)oldtriggers, (long long int)oldPeriod,
                      (long long int)oldExptime));
        setTiming(oldtiming);
        setNumFrames(oldframes);
        setNumTriggers(oldtriggers);
        setPeriod(oldPeriod);
        setExpTime(oldExptime);
        LOG(logINFOBLUE, ("Done sending a frame at configuration\n"));
    }
    return OK;
}

int getAdcConfigured() { return adcConfigured; }

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));
    return OK;
}

int *getDetectorPosition() { return detPos; }

/* gotthard specific - adc phase */
int setPhase(enum CLKINDEX ind, int val, int degrees) {
    if (ind != ADC_CLK) {
        LOG(logERROR, ("Unknown clock index: %d\n", ind));
        return FAIL;
    }
    if (degrees != 0) {
        LOG(logERROR, ("Cannot set phase in degrees\n"));
        return FAIL;
    }
    setPhaseShift(val);
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
    LOG(logINFOBLUE, ("Starting State Machine\n"));
    LOG(logINFO, ("#frames to acquire:%lld\n", (long long int)getNumFrames()));

    cleanFifos();

    // start state machine
    bus_w16(CONTROL_REG, CONTROL_STRT_ACQ_MSK | CONTROL_STRT_EXPSR_MSK);
    bus_w16(CONTROL_REG, 0x0);
    runState(logINFO);
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

    int imageSize =
        adcConfigured == -1 ? DATA_BYTES : NCHAN_PER_ADC * NUM_BITS_PER_PIXEL;
    int dataSize = adcConfigured == -1 ? 1280 : 512;
    int packetSize = adcConfigured == -1 ? 1286 : 518;
    int packetsPerFrame = adcConfigured == -1 ? 2 : 1;

    // Generate Data
    char imageData[imageSize];
    memset(imageData, 0, imageSize);
    if (adcConfigured == -1) {
        // split dereferencing for rhel7 warnings
        uint32_t *start = (uint32_t *)imageData;
        *start = 0xCACACACA;
    }
    for (int i = sizeof(uint32_t); i < imageSize; i += sizeof(uint16_t)) {
        *((uint16_t *)(imageData + i)) = (uint16_t)i;
    }

    // Send data
    // loop over number of frames
    for (int frameNr = 0; frameNr != numFrames; ++frameNr) {

        // check if manual stop
        if (sharedMemory_getStop() == 1) {
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
            // split dereferencing for rhel7 warnings
            uint16_t *fnum = (uint16_t *)packetData;
            *fnum = virtual_currentFrameNumber;
            ++virtual_currentFrameNumber;

            // fill data
            memcpy(packetData + 4, imageData + srcOffset, dataSize);
            srcOffset += dataSize;

            sendUDPPacket(0, 0, packetData, packetSize);
        }
        LOG(logINFO,
            ("Sent frame: %d [%d]\n", frameNr, virtual_currentFrameNumber));
        clock_gettime(CLOCK_REALTIME, &end);
        int64_t timeNs =
            ((end.tv_sec - begin.tv_sec) * 1E9 + (end.tv_nsec - begin.tv_nsec));

        // sleep for (period - exptime)
        if (frameNr < numFrames) { // if there is a next frame
            if (periodNs > timeNs) {
                usleep((periodNs - timeNs) / 1000);
            }
        }
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
    if (!isControlServer) {
        sharedMemory_setStop(1);
        // read till status is idle
        while (sharedMemory_getStatus() == RUNNING)
            usleep(500);
        sharedMemory_setStop(0);
        LOG(logINFO, ("Stopped State Machine\n"));
    }
    return OK;
#endif
    // stop state machine
    bus_w16(CONTROL_REG, CONTROL_STP_ACQ_MSK);
    usleep(100);
    bus_w16(CONTROL_REG, 0x0);

    // check
    usleep(500);
    if ((runState(logDEBUG1) & STATUS_RN_MSHN_BSY_MSK)) {
        LOG(logERROR, ("\tFailed to stop state machine.\n"));
        runState(logINFORED);
        return FAIL;
    }

    runState(logINFO);
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

    enum runStatus s = IDLE;
    u_int32_t retval = runState(logINFO);

    // finished (external stop or fifo full)
    if (retval & STATUS_RN_FNSHD_MSK) {
        LOG(logINFORED, ("Status: Stopped\n"));
        s = STOPPED;

        LOG(logINFO, ("\t Reading status reg again\n"));
        retval = runState(logINFO);
        // fifo full
        if (runState(logDEBUG1) & STATUS_RN_FNSHD_MSK) {
            LOG(logINFORED, ("Status: Error\n"));
            runState(logINFORED);
            s = ERROR;
        }
    }

    // error (fifo full)
    else if (retval & STATUS_SM_FF_FLL_MSK) {
        LOG(logINFORED, ("Status: Error\n"));
        s = ERROR;
    }

    // not running
    else if (!(retval & STATUS_RN_BSY_MSK)) {
        // read last frames
        if (retval & STATUS_RD_MSHN_BSY_MSK) {
            LOG(logINFOBLUE, ("Status: Read Machine Busy\n"));
            s = TRANSMITTING;
        }
        // ???
        else if (retval & STATUS_ALL_FF_EMPTY_MSK) {
            LOG(logINFOBLUE, ("Status: Data in Fifo\n"));
            s = TRANSMITTING;
        }
        // idle, unknown
        else if (!(retval & STATUS_IDLE_MSK)) {
            LOG(logINFOBLUE, ("Status: IDLE\n"));
            s = IDLE;
        } else {
            LOG(logINFORED,
                ("Status: Unknown Status: 0x%x. Trying again.\n", retval));
            for (int iloop = 0; iloop < 10; ++iloop) {
                usleep(1000 * 1000);
                if (runState(logDEBUG1) != retval)
                    return getRunStatus();
            }
            s = ERROR;
        }
    }

    // running
    else {
        if (retval & STATUS_WTNG_FR_TRGGR_MSK) {
            LOG(logINFOBLUE, ("Status: Waiting\n"));
            s = WAITING;
        } else {
            LOG(logINFOBLUE, ("Status: Running\n"));
            s = RUNNING;
        }
    }

    return s;
}

void waitForAcquisitionEnd() {
    while (runBusy()) {
        usleep(500);
    }
#ifndef VIRTUAL
    int64_t retval = getNumFramesLeft() + 1;
    if (retval > -1) {
        LOG(logINFORED, ("%lld frames left\n", (long long int)retval));
    }
#endif
    LOG(logINFOGREEN, ("Blocking Acquisition done\n"));
}

u_int32_t runBusy() {
#ifdef VIRTUAL
    return ((sharedMemory_getStatus() == RUNNING) ? 1 : 0);
#endif
    return runState(logDEBUG1) & STATUS_RN_BSY_MSK;
}

u_int32_t runState(enum TLogLevel lev) {
#ifdef VIRTUAL
    return (int)sharedMemory_getStatus();
#endif
    u_int32_t s = bus_r(STATUS_REG);
    LOG(lev, ("Status Register: 0x%08x\n", s));
    return s;
}

/* common */

int calculateDataBytes() { return DATA_BYTES; }

int getTotalNumberOfChannels() {
    return (getNumberOfChannelsPerChip() * getNumberOfChips());
}
int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }
