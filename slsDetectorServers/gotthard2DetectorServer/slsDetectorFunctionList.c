// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "ALTERA_PLL_CYCLONE10.h"
#include "ASIC_Driver.h"
#include "DAC6571.h"
#include "LTC2620_Driver.h"
#include "RegisterDefs.h"
#include "clogger.h"
#include "common.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include <netinet/in.h>
#include <string.h>
#include <unistd.h> // usleep
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern int checkModuleFlag;
extern udpStruct udpDetails[MAX_UDP_DESTINATION];
extern int numUdpDestinations;
extern const enum detectorType myDetectorType;
extern int ignoreConfigFileFlag;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char *cmac, int size, uint64_t mac);
extern void getIpAddressinString(char *cip, uint32_t ip);

// Variables that will be exported
int masterCommandLine = -1;

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_moduleid = 0;
#endif

enum detectorSettings thisSettings = UNINITIALIZED;
int32_t clkPhase[NUM_CLOCKS] = {};
uint32_t clkDivider[NUM_CLOCKS] = {};
double systemFrequency = 0;
int dacValues[NDAC] = {};
int startupPowerChipConfigDone = 0;
int onChipdacValues[ONCHIP_NDAC][NCHIP] = {};
int defaultDacValues[NDAC] = {};
int hardCodedDefaultDacValues[NDAC] = {};
int defaultOnChipdacValues[ONCHIP_NDAC][NCHIP] = {};
int injectedChannelsOffset = 0;
int injectedChannelsIncrement = 0;
int vetoReference[NCHIP][NCHAN];
int vetoGainIndices[NCHIP][NCHAN];
uint8_t adcConfiguration[NCHIP][NADC];
int burstMode = BURST_INTERNAL;
int64_t numFramesReg = 1;
int64_t periodReg = 0;
int64_t numTriggersReg = 1;
int64_t delayReg = 0;
int64_t numBurstsReg = 1;
int64_t burstPeriodReg = 0;
int filterResistor = 0;
int cdsGain = 0;
int detPos[2] = {};
int chipConfigured = 0;

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
    LOG(logINFOBLUE, ("************ Gotthard2 Virtual Server ************\n"));
#else
    LOG(logINFOBLUE, ("**************** Gotthard2 Server ****************\n"));
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
        ((validateKernelVersion(KERNEL_DATE_VRSN) == FAIL) ||
         (checkType() == FAIL) || (testFpga() == FAIL) ||
         (testBus() == FAIL))) {
        sprintf(initErrorMessage,
                "Could not pass basic tests of FPGA and bus. Cannot proceed. "
                "Check Firmware. (Firmware version:0x%llx) \n",
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
    uint32_t requiredFirmwareVersion = REQRD_FRMWRE_VRSN;

    LOG(logINFOBLUE,
        ("**************************************************\n"
         "Hardware Version:\t\t %s\n"

         "Detector IP Addr:\t\t 0x%x\n"
         "Detector MAC Addr:\t\t 0x%llx\n\n"

         "Firmware Version:\t\t 0x%llx\n"
         "Software Version:\t\t %s\n"
         "F/w-S/w API Version:\t\t 0x%llx\n"
         "Required Firmware Version:\t 0x%x\n"
         "********************************************************\n",
         hversion, ipadd, (long long unsigned int)macadd,
         (long long int)fwversion, swversion, (long long int)sw_fw_apiversion,
         requiredFirmwareVersion));

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
    if (sw_fw_apiversion > requiredFirmwareVersion) {
        sprintf(initErrorMessage,
                "This firmware-software api version (0x%llx) is incompatible "
                "with the software's minimum required firmware version "
                "(0x%llx).\nPlease update detector software to be compatible "
                "with this firmware.\n",
                (long long int)sw_fw_apiversion,
                (long long int)requiredFirmwareVersion);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for firmware compatibility - old firmware
    if (requiredFirmwareVersion > fwversion) {
        sprintf(initErrorMessage,
                "This firmware version (0x%llx) is incompatible.\n"
                "Please update firmware (min. 0x%llx) to be compatible with "
                "this server.\n",
                (long long int)fwversion,
                (long long int)requiredFirmwareVersion);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }
    LOG(logINFO, ("Compatibility - success\n"));
#endif
}

int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
    u_int32_t type =
        ((bus_r(FPGA_VERSION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
    if (type != GOTTHARD2) {
        LOG(logERROR,
            ("This is not a Gotthard2 firmware (read %d, expected %d)\n", type,
             GOTTHARD2));
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
    volatile u_int32_t val = bus_r(FIX_PATT_REG);
    if (val == FIX_PATT_VAL) {
        LOG(logINFO, ("Fixed pattern: successful match 0x%08x\n", val));
    } else {
        LOG(logERROR,
            ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n",
             val, FIX_PATT_VAL));
        ret = FAIL;
    }
    return ret;
}

int testBus() {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Testing Bus:\n"));

    int ret = OK;
    u_int32_t addr = DTA_OFFSET_REG;
    u_int32_t prevValue = bus_r(addr);
    u_int32_t times = 1000 * 1000;

    for (u_int32_t i = 0; i < times; ++i) {
        bus_w(addr, i * 100);
        if (i * 100 != bus_r(addr)) {
            LOG(logERROR,
                ("Mismatch! Wrote 0x%x, read 0x%x\n", i * 100, bus_r(addr)));
            ret = FAIL;
        }
    }

    bus_w(addr, prevValue);

    if (ret == OK) {
        LOG(logINFO, ("Successfully tested bus %d times\n", times));
    }
    return ret;
}

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIGOTTHARD2); }

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return REQRD_FRMWRE_VRSN;
#endif
    return ((bus_r(FPGA_VERSION_REG) & FPGA_COMPILATION_DATE_MSK) >>
            FPGA_COMPILATION_DATE_OFST);
}

u_int64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(API_VERSION_REG) & API_VERSION_MSK) >> API_VERSION_OFST);
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
    return ((bus_r(MCB_SERIAL_NO_REG) & MCB_SERIAL_NO_VRSN_MSK) >>
            MCB_SERIAL_NO_VRSN_OFST);
}

int isHardwareVersion_1_0() {
    const int hwNumberList[] = HARDWARE_VERSION_NUMBERS;
    return ((getHardwareVersionNumber() == hwNumberList[0]) ? 1 : 0);
}

u_int32_t getDetectorNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return bus_r(MCB_SERIAL_NO_REG);
}

int getModuleId(int *ret, char *mess) {
    return ((bus_r(MOD_ID_REG) & MOD_ID_MSK) >> MOD_ID_OFST);
}

void setModuleId(int modid) {
    LOG(logINFOBLUE, ("Setting module id in fpga: %d\n", modid))
    bus_w(MOD_ID_REG, bus_r(MOD_ID_REG) & ~MOD_ID_MSK);
    bus_w(MOD_ID_REG,
          bus_r(MOD_ID_REG) | ((modid << MOD_ID_OFST) & MOD_ID_MSK));
}

int updateModuleId() {
    int modid = getModuleIdInFile(&initError, initErrorMessage, ID_FILE);
    if (initError == FAIL) {
        return FAIL;
    }
#ifdef VIRTUAL
    virtual_moduleid = modid;
#endif
    setModuleId(modid);
    return OK;
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

/* initialization */

void initControlServer() {
    if (!updateFlag && initError == OK) {
        setupDetector();
    }
    initCheckDone = 1;
    if (initError == OK) {
        NotifyServerStartSuccess();
    }
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
    LOG(logINFO, ("This Server is for 1 Gotthard2 module \n"));

    clkDivider[READOUT_C0] = DEFAULT_READOUT_C0;
    clkDivider[READOUT_C1] = DEFAULT_READOUT_C1;
    clkDivider[SYSTEM_C0] = DEFAULT_SYSTEM_C0;
    clkDivider[SYSTEM_C1] = DEFAULT_SYSTEM_C1;
    clkDivider[SYSTEM_C2] = DEFAULT_SYSTEM_C2;
    clkDivider[SYSTEM_C3] = DEFAULT_SYSTEM_C3;
    systemFrequency = INT_SYSTEM_C0_FREQUENCY;
    detPos[0] = 0;
    detPos[1] = 0;
    chipConfigured = 0;

    thisSettings = UNINITIALIZED;
    injectedChannelsOffset = 0;
    injectedChannelsIncrement = 0;
    burstMode = BURST_INTERNAL;
    numFramesReg = 1;
    periodReg = 0;
    numTriggersReg = 1;
    delayReg = 0;
    numBurstsReg = 1;
    burstPeriodReg = 0;
    filterResistor = 0;
    cdsGain = 0;
    startupPowerChipConfigDone = 0;
    memset(clkPhase, 0, sizeof(clkPhase));
    memset(dacValues, 0, sizeof(dacValues));
    for (int i = 0; i < NDAC; ++i) {
        defaultDacValues[i] = -1;
        hardCodedDefaultDacValues[i] = -1;
    }
    for (int i = 0; i < ONCHIP_NDAC; ++i) {
        for (int j = 0; j < NCHIP; ++j) {
            onChipdacValues[i][j] = -1;
            defaultOnChipdacValues[i][j] = -1;
        }
    }
    memset(vetoReference, 0, sizeof(vetoReference));
    memset(vetoGainIndices, 0, sizeof(vetoGainIndices));
    memset(adcConfiguration, 0, sizeof(adcConfiguration));
#ifdef VIRTUAL
    if (isControlServer) {
        sharedMemory_setStatus(IDLE);
        setupUDPCommParameters();
    } else {
        sharedMemory_setStop(0);
    }
    // ismaster from reg in stop server, so set it in virtual mode
    setMaster(OW_MASTER);
#endif
    // pll defines
    ALTERA_PLL_C10_SetDefines(REG_OFFSET, BASE_READOUT_PLL, BASE_SYSTEM_PLL,
                              PLL_RESET_REG, PLL_RESET_READOUT_MSK,
                              PLL_RESET_SYSTEM_MSK, READOUT_PLL_VCO_FREQ_HZ,
                              SYSTEM_PLL_VCO_FREQ_HZ);
    ALTERA_PLL_C10_ResetPLL(READOUT_PLL);
    ALTERA_PLL_C10_ResetPLL(SYSTEM_PLL);
    // hv
    DAC6571_SetDefines(HV_HARD_MAX_VOLTAGE, HV_DRIVER_FILE_NAME);
    // dacs
    LTC2620_D_SetDefines(DAC_MIN_MV, DAC_MAX_MV, DAC_DRIVER_FILE_NAME, NDAC, 0,
                         "");
    // on chip dacs
    ASIC_Driver_SetDefines(ONCHIP_DAC_DRIVER_FILE_NAME);
    setTimingSource(DEFAULT_TIMING_SOURCE);

    // Default values
    initError = setHighVoltage(DEFAULT_HIGH_VOLTAGE);
    if (initError == FAIL) {
        sprintf(initErrorMessage, "Could not set high voltage to %d\n",
                DEFAULT_HIGH_VOLTAGE);
        return;
    }

    // check module type attached if not in debug mode
    if (initError == FAIL)
        return;
    if (!checkModuleFlag) {
        LOG(logINFOBLUE, ("In No-Module mode: Ignoring module type...\n"));
    } else {
        initError = checkDetectorType(initErrorMessage);
    }
    if (initError == FAIL) {
        return;
    }

    // power on chip
    initError = powerChip(1, initErrorMessage);
    if (initError == FAIL)
        return;

    setASICDefaults();

    setPhase(READOUT_C1, DEFAULT_CLK1_PHASE_DEG, 1);
    setDBITPipeline(DEFAULT_DBIT_PIPELINE);

    // also sets default dac and on chip dac values
    if (readConfigFile() == FAIL) {
        return;
    }

    // master for virtual
    if (checkCommandLineConfiguration() == FAIL)
        return;

    if (updateModuleId() == FAIL) {
        return;
    }

    setBurstMode(DEFAULT_BURST_MODE);
    setFilterResistor(DEFAULT_FILTER_RESISTOR);
    setCDSGain(DEFAILT_CDS_GAIN);
    setSettings(DEFAULT_SETTINGS);

    // Initialization of acquistion parameters
    setNextFrameNumber(DEFAULT_FRAME_NUMBER);
    setNumFrames(DEFAULT_NUM_FRAMES);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setNumBursts(DEFAULT_NUM_BURSTS);
    setExpTime(DEFAULT_EXPTIME);
    setPeriod(DEFAULT_PERIOD);
    setDelayAfterTrigger(DEFAULT_DELAY_AFTER_TRIGGER);
    setBurstPeriod(DEFAULT_BURST_PERIOD);
    setTiming(DEFAULT_TIMING_MODE);
    setCurrentSource(DEFAULT_CURRENT_SOURCE);
    setVetoAlgorithm(DEFAULT_ALGORITHM, LOW_LATENCY_LINK);
    setVetoAlgorithm(DEFAULT_ALGORITHM, ETHERNET_10GB);
    setReadoutSpeed(DEFAULT_READOUT_SPEED);
}

void setASICDefaults() {
    uint32_t addr = ASIC_CONFIG_REG;

    // dout ready source
    bus_w(addr, bus_r(addr) & ~ASIC_CONFIG_DOUT_RDY_SRC_MSK);
    bus_w(addr, bus_r(addr) | ((DEFAULT_ASIC_DOUT_RDY_SRC
                                << ASIC_CONFIG_DOUT_RDY_SRC_OFST) &
                               ASIC_CONFIG_DOUT_RDY_SRC_MSK));
    // dout ready delay
    bus_w(addr, bus_r(addr) & ~ASIC_CONFIG_DOUT_RDY_DLY_MSK);
    bus_w(addr, bus_r(addr) | ((DEFAULT_ASIC_DOUT_RDY_DLY
                                << ASIC_CONFIG_DOUT_RDY_DLY_OFST) &
                               ASIC_CONFIG_DOUT_RDY_DLY_MSK));
    // config done
    bus_w(addr, bus_r(addr) | ASIC_CONFIG_DONE_MSK);
    LOG(logINFO, ("Setting ASIC Defaults (0x%x)\n", bus_r(addr)));
}

int resetToDefaultDacs(int hardReset) {
    // reset defaults to hardcoded defaults
    if (hardReset) {
        for (int i = 0; i < NDAC; ++i) {
            defaultDacValues[i] = hardCodedDefaultDacValues[i];
        }
    }
    // reset dacs to defaults
    int ret = OK;
    LOG(logINFOBLUE, ("Setting Default Dac values\n"));
    for (int i = 0; i < NDAC; ++i) {
        if (defaultDacValues[i] != -1) {
            setDAC((enum DACINDEX)i, defaultDacValues[i], 0);
            if (dacValues[i] != defaultDacValues[i]) {
                ret = FAIL;
                LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n", i,
                               defaultDacValues[i], dacValues[i]));
            }
        }
    }
    LOG(logINFOBLUE, ("Setting Default On-chip Dac values\n"));
    for (int ichip = 0; ichip < NCHIP; ++ichip) {
        for (int idac = 0; idac < ONCHIP_NDAC; ++idac) {
            if (defaultOnChipdacValues[idac][ichip] != -1) {
                setOnChipDAC((enum ONCHIP_DACINDEX)idac, ichip,
                             defaultOnChipdacValues[idac][ichip]);
                if (onChipdacValues[idac][ichip] !=
                    defaultOnChipdacValues[idac][ichip]) {
                    ret = FAIL;
                    LOG(logERROR,
                        ("Setting on-chip dac %d (ichip:%d) failed, "
                         "wrote %d, read %d\n",
                         idac, ichip, defaultOnChipdacValues[idac][ichip],
                         onChipdacValues[idac][ichip]));
                }
            }
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

int readConfigFile() {

    if (initError == FAIL) {
        return initError;
    }

    if (ignoreConfigFileFlag) {
        LOG(logWARNING, ("Ignoring Config file\n"));
        return OK;
    }

    // require a sleep before and after the rst dac signal
    usleep(INITIAL_STARTUP_WAIT);

    // inform FPGA that onchip dacs will be configured soon
    LOG(logINFO, ("Setting configuration starting bit\n"));
    bus_w(ASIC_CONFIG_REG, bus_r(ASIC_CONFIG_REG) | ASIC_CONFIG_RST_DAC_MSK);

    usleep(INITIAL_STARTUP_WAIT);

    const int fileNameSize = 128;
    char fname[fileNameSize];
    if (getAbsPath(fname, fileNameSize, CONFIG_FILE) == FAIL) {
        return FAIL;
    }

    // open config file
    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        sprintf(initErrorMessage,
                "Could not open on-board detector server config file [%s].\n",
                CONFIG_FILE);
        initError = FAIL;
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        return FAIL;
    }

    LOG(logINFOBLUE, ("Reading config file %s\n", CONFIG_FILE));

    // Initialization
    const size_t LZ = 256;
    char line[LZ];
    memset(line, 0, LZ);
    char command[LZ];

    int nAdcTotal = 0;

    // keep reading a line
    while (fgets(line, LZ, fd)) {

        // ignore comments
        if (line[0] == '#') {
            LOG(logDEBUG1, ("Ignoring Comment\n"));
            continue;
        }

        // ignore empty lines
        if (strlen(line) <= 1) {
            LOG(logDEBUG1, ("Ignoring Empty line\n"));
            continue;
        }

        // removing leading spaces
        if (line[0] == ' ' || line[0] == '\t') {
            int len = strlen(line);
            // find first valid character
            int i = 0;
            for (i = 0; i < len; ++i) {
                if (line[i] != ' ' && line[i] != '\t') {
                    break;
                }
            }
            // ignore the line full of spaces (last char \n)
            if (i >= len - 1) {
                LOG(logDEBUG1, ("Ignoring line full of spaces\n"));
                continue;
            }
            // copying only valid char
            char temp[LZ];
            memset(temp, 0, LZ);
            memcpy(temp, line + i, strlen(line) - i);
            memset(line, 0, LZ);
            memcpy(line, temp, strlen(temp));
            LOG(logDEBUG1, ("Removing leading spaces.\n"));
        }

        LOG(logDEBUG1, ("Command to process: (size:%d) %.*s\n", strlen(line),
                        strlen(line) - 1, line));
        memset(command, 0, LZ);

        // master command
        if (!strncmp(line, "master", strlen("master"))) {
            int m = -1;
            // cannot scan values
            if (sscanf(line, "%s %d", command, &m) != 2) {
                sprintf(initErrorMessage,
                        "Could not scan master commands from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            // validations
            if (m != 0 && m != 1) {
                sprintf(initErrorMessage,
                        "Invalid master argument from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            if (setMaster(m == 1 ? OW_MASTER : OW_SLAVE) == FAIL) {
                sprintf(initErrorMessage,
                        "Could not set master from config file. Line:[%s].\n",
                        line);
                break;
            }
        }

        // vetoref command
        else if (!strncmp(line, "vetoref", strlen("vetoref"))) {
            int igain = 0;
            int value = 0;

            // cannot scan values
            if (sscanf(line, "%s %d %d", command, &igain, &value) != 3) {
                sprintf(initErrorMessage,
                        "Could not scan vetoref commands from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            // validations
            if (igain < 0 || igain > 2) {
                sprintf(initErrorMessage,
                        "Could not set veto reference from on-board server "
                        "config file. Invalid gain index. Line:[%s].\n",
                        line);
                break;
            }
            // validations
            if (value > ADU_MAX_VAL) {
                sprintf(initErrorMessage,
                        "Could not set veto reference from on-board server "
                        "config file. Invalid value (max 0x%x). Line:[%s].\n",
                        ADU_MAX_VAL, line);
                break;
            }
            if (setVetoReference(igain, value) == FAIL) {
                sprintf(initErrorMessage,
                        "Could not set veto reference from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
        }

        // confadc command
        else if (!strncmp(line, "confadc", strlen("confadc"))) {
            int ichip = -1;
            int iadc = -1;
            int value = 0;

            // cannot scan values
            if (sscanf(line, "%s %d %d 0x%x", command, &ichip, &iadc, &value) !=
                4) {
                sprintf(initErrorMessage,
                        "Could not scan confadc commands from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            // validations
            if (ichip < -1 || ichip >= NCHIP) {
                sprintf(initErrorMessage,
                        "Could not configure adc from on-board server config "
                        "file. Invalid chip index. Line:[%s].\n",
                        line);
                break;
            }
            if (iadc < -1 || iadc >= NADC) {
                sprintf(initErrorMessage,
                        "Could not configure adc from on-board server config "
                        "file. Invalid adc index. Line:[%s].\n",
                        line);
                break;
            }
            // validations
            if (value < 0 || value > ASIC_ADC_MAX_VAL) {
                sprintf(initErrorMessage,
                        "Could not configure adc from on-board server config "
                        "file. Invalid value (max 0x%x). Line:[%s].\n",
                        ASIC_ADC_MAX_VAL, line);
                break;
            }

            if (setADCConfiguration(ichip, iadc, value) == FAIL) {
                sprintf(initErrorMessage,
                        "Could not configure adc from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }

            // to ensure all adcs are configured at start up
            int nadc = 1;
            if (iadc == -1) {
                nadc = NADC;
            }
            if (ichip == -1) {
                nadc *= NCHIP;
            }
            nAdcTotal += nadc;
        }

        // vchip command
        else if (!strncmp(line, "vchip_", strlen("vchip_"))) {

            enum ONCHIP_DACINDEX idac = 0;
            int ichip = -1;
            int value = 0;

            // cannot scan values
            if (sscanf(line, "%s %d 0x%x", command, &ichip, &value) != 3) {
                sprintf(initErrorMessage,
                        "Could not scan on-chip dac commands from on-board "
                        "server config file. Line:[%s].\n",
                        line);
                break;
            }

            if (!strcmp(command, "vchip_comp_fe")) {
                idac = G2_VCHIP_COMP_FE;
            } else if (!strcasecmp(command, "vchip_opa_1st")) {
                idac = G2_VCHIP_OPA_1ST;
            } else if (!strcasecmp(command, "vchip_opa_fd")) {
                idac = G2_VCHIP_OPA_FD;
            } else if (!strcasecmp(command, "vchip_comp_adc")) {
                idac = G2_VCHIP_COMP_ADC;
            } else if (!strcasecmp(command, "vchip_ref_comp_fe")) {
                idac = G2_VCHIP_REF_COMP_FE;
            } else if (!strcasecmp(command, "vchip_cs")) {
                idac = G2_VCHIP_CS;
            } else {
                sprintf(initErrorMessage,
                        "Unknown on-chip dac command in on-board server config "
                        "file. Command:[%s].\n",
                        command);
                break;
            }

            // set default on chip dac variable
            // specific chip
            if (ichip != -1) {
                defaultOnChipdacValues[idac][ichip] = value;
            }
            // all chips
            else {
                for (int i = 0; i < NCHIP; ++i) {
                    defaultOnChipdacValues[idac][i] = value;
                }
            }

            // set on chip dac
            if (setOnChipDAC(idac, ichip, value) == FAIL) {
                sprintf(initErrorMessage,
                        "Set on-chip dac failed from on-board server config "
                        "file. Command:[%s].\n",
                        command);
                break;
            }
        }

        // dac command
        else {
            enum DACINDEX idac = 0;
            int value = 0;

            // cannot scan values
            if (sscanf(line, "%s %d", command, &value) != 2) {
                sprintf(initErrorMessage,
                        "Could not scan dac commands from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }

            if (!strcmp(command, "vref_h_adc")) {
                idac = G2_VREF_H_ADC;
            } else if (!strcasecmp(command, "vb_comp_fe")) {
                idac = G2_VB_COMP_FE;
            } else if (!strcasecmp(command, "vb_comp_adc")) {
                idac = G2_VB_COMP_ADC;
            } else if (!strcasecmp(command, "vcom_cds")) {
                idac = G2_VCOM_CDS;
            } else if (!strcasecmp(command, "vref_rstore")) {
                idac = G2_VREF_RSTORE;
            } else if (!strcasecmp(command, "vb_opa_1st")) {
                idac = G2_VB_OPA_1ST;
            } else if (!strcasecmp(command, "vref_comp_fe")) {
                idac = G2_VREF_COMP_FE;
            } else if (!strcasecmp(command, "vcom_adc1")) {
                idac = G2_VCOM_ADC1;
            } else if (!strcasecmp(command, "vref_prech")) {
                idac = G2_VREF_PRECH;
            } else if (!strcasecmp(command, "vref_l_adc")) {
                idac = G2_VREF_L_ADC;
            } else if (!strcasecmp(command, "vref_cds")) {
                idac = G2_VREF_CDS;
            } else if (!strcasecmp(command, "vb_cs")) {
                idac = G2_VB_CS;
            } else if (!strcasecmp(command, "vb_opa_fd")) {
                idac = G2_VB_OPA_FD;
            } else if (!strcasecmp(command, "vcom_adc2")) {
                idac = G2_VCOM_ADC2;
            } else {
                sprintf(initErrorMessage,
                        "Unknown command in on-board server config file. "
                        "Command:[%s].\n",
                        command);
                break;
            }

            // set default dac variables
            defaultDacValues[idac] = value;
            hardCodedDefaultDacValues[idac] = value;

            // set dac
            setDAC(idac, value, 0);
            int retval = getDAC(idac, 0);
            if (retval != value) {
                sprintf(initErrorMessage,
                        "Set dac %s failed from on-board server config file. "
                        "Set %d, got %d.\n",
                        command, value, retval);
                break;
            }
        }
        memset(line, 0, LZ);
    }
    fclose(fd);

    if (!strlen(initErrorMessage)) {
        if (nAdcTotal != NADC * NCHIP) {
            sprintf(initErrorMessage,
                    "Could not configure adc from on-board server config file. "
                    "Insufficient adcconf commands. Read %d, expected %d\n",
                    nAdcTotal, NADC * NCHIP);
        }
    }

    if (strlen(initErrorMessage)) {
        initError = FAIL;
        LOG(logERROR, ("%s\n\n", initErrorMessage));
    } else {
        LOG(logINFOBLUE, ("Successfully read config file\n"));

        // inform FPGA that onchip dacs will be configured soon
        LOG(logINFO, ("Setting configuration done bit\n"));
        bus_w(ASIC_CONFIG_REG, bus_r(ASIC_CONFIG_REG) | ASIC_CONFIG_DONE_MSK);

        // to inform powerchip config parameters are set
        startupPowerChipConfigDone = 1;
        chipConfigured = 1;
        LOG(logINFOBLUE, ("Chip configured\n"));
    }
    return initError;
}

int checkCommandLineConfiguration() {
    if (masterCommandLine != -1) {
        LOG(logINFOBLUE, ("Setting %s from Command Line\n",
                          (masterCommandLine == 1 ? "Master" : "Slave")));
        if (setMaster(masterCommandLine == 1 ? OW_MASTER : OW_SLAVE) == FAIL) {
            initError = FAIL;
            sprintf(initErrorMessage, "Could not set %s from command line.\n",
                    (masterCommandLine == 1 ? "Master" : "Slave"));
            LOG(logERROR, (initErrorMessage));
            return FAIL;
        }
    }
    return OK;
}

/* firmware functions (resets) */

void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Clearing Acquisition Fifos\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CLR_ACQSTN_FIFO_MSK);
}

void resetCore() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Resetting Core\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CRE_RST_MSK);
}

void resetPeripheral() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Resetting Peripheral\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PRPHRL_RST_MSK);
}

/* set parameters -  readout */

int setParallelMode(int mode) {
    if (mode < 0)
        return FAIL;
    LOG(logINFO, ("Setting %s mode\n", (mode ? "Parallel" : "Non Parallel")));
    uint32_t addr = ASIC_CONFIG_REG;
    if (mode) {
        bus_w(addr, bus_r(addr) & ~ASIC_CONFIG_NON_PARALLEL_RD_MSK);
    } else {
        bus_w(addr, bus_r(addr) | ASIC_CONFIG_NON_PARALLEL_RD_MSK);
    }
    return OK;
}

int getParallelMode() {
    int nonparallel =
        ((bus_r(ASIC_CONFIG_REG) & ASIC_CONFIG_NON_PARALLEL_RD_MSK) >>
         ASIC_CONFIG_NON_PARALLEL_RD_OFST);
    return (nonparallel == 0 ? 1 : 0);
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

/* parameters - timer */

int setNextFrameNumber(uint64_t value) {
    LOG(logINFO, ("Setting next frame number: %lu\n", value));
#ifdef VIRTUAL
    setU64BitReg(value, FRAME_NUMBER_LSB_REG, FRAME_NUMBER_MSB_REG);
#else
    // decrement by 1 for firmware
    setU64BitReg(value - 1, FRAME_NUMBER_LSB_REG, FRAME_NUMBER_MSB_REG);
#endif
    return OK;
}

int getNextFrameNumber(uint64_t *value) {
#ifdef VIRTUAL
    *value = getU64BitReg(FRAME_NUMBER_LSB_REG, FRAME_NUMBER_MSB_REG);
#else
    // increment is for firmware
    *value =
        (getU64BitReg(GET_FRAME_NUMBER_LSB_REG, GET_FRAME_NUMBER_MSB_REG) + 1);
#endif
    return OK;
}

void setNumFrames(int64_t val) {
    if (val > 0) {
        numFramesReg = val;
        // continuous
        if (burstMode == CONTINUOUS_INTERNAL ||
            burstMode == CONTINUOUS_EXTERNAL) {
            LOG(logINFO,
                ("Setting number of frames %lld [Continuous mode]\n", val));
            // trigger
            if (getTiming() == TRIGGER_EXPOSURE) {
                LOG(logINFO,
                    ("\tCont. Trigger mode (not writing to register)\n", val));
                // #frames limited in cont trigger mode
            }
            // auto
            else {
                set64BitReg(val, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
                set64BitReg(val, ASIC_CONT_FRAMES_LSB_REG,
                            ASIC_CONT_FRAMES_MSB_REG);
            }
        }
        // burst
        else {
            LOG(logINFO,
                ("Setting number of frames %d [Burst mode]\n", (int)val));
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) & ~ASIC_INT_FRAMES_MSK);
            bus_w(ASIC_INT_FRAMES_REG, bus_r(ASIC_INT_FRAMES_REG) |
                                           (((int)val << ASIC_INT_FRAMES_OFST) &
                                            ASIC_INT_FRAMES_MSK));
        }
    }
}

int64_t getNumFrames() {
    // continuous
    if (burstMode == CONTINUOUS_INTERNAL || burstMode == CONTINUOUS_EXTERNAL) {
        // trigger
        if (getTiming() == TRIGGER_EXPOSURE) {
            // #frames limited in cont trigger mode
            return numFramesReg;
        }
        // auto
        return get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
    }
    // burst
    else {
        return ((bus_r(ASIC_INT_FRAMES_REG) & ASIC_INT_FRAMES_MSK) >>
                ASIC_INT_FRAMES_OFST);
    }
}

void setNumTriggers(int64_t val) {
    if (val > 0) {
        numTriggersReg = val;
        LOG(logINFO, ("Setting number of triggers %lld\n", val));
        // auto
        if (getTiming() == AUTO_TIMING) {
            LOG(logINFO, ("\tAuto mode (not writing to register)\n"));
        }
        // trigger
        else {
            set64BitReg(val, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
            // continuous
            if (burstMode == CONTINUOUS_INTERNAL ||
                burstMode == CONTINUOUS_EXTERNAL) {
                set64BitReg(val, ASIC_CONT_FRAMES_LSB_REG,
                            ASIC_CONT_FRAMES_MSB_REG);
            }
        }
    }
}

int64_t getNumTriggers() {
    // auto
    if (getTiming() == AUTO_TIMING) {
        return numTriggersReg;
    }
    // trigger
    return get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
}

void setNumBursts(int64_t val) {
    if (val > 0) {
        numBurstsReg = val;
        LOG(logINFO, ("Setting number of bursts %lld\n", val));
        // burst and auto
        if ((burstMode == BURST_INTERNAL || burstMode == BURST_EXTERNAL) &&
            getTiming() == AUTO_TIMING) {
            set64BitReg(val, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
        }
        // burst-trigger or continuous
        else {
            LOG(logINFO,
                ("\tNot (Burst and Auto mode): not writing to register\n"));
        }
    }
}

int64_t getNumBursts() {
    // burst and auto
    if ((burstMode == BURST_INTERNAL || burstMode == BURST_EXTERNAL) &&
        getTiming() == AUTO_TIMING) {
        return get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
    }
    // burst-trigger or continuous
    return numBurstsReg;
}

int setExpTime(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid exptime: %lld ns\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting exptime %lld ns\n", val));
    val = (val * 1E-9 * systemFrequency) + 0.5;
    set64BitReg(val, ASIC_INT_EXPTIME_LSB_REG, ASIC_INT_EXPTIME_MSB_REG);

    // validate for tolerance
    int64_t retval = getExpTime();
    val /= (1E-9 * systemFrequency);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getExpTime() {
    return get64BitReg(ASIC_INT_EXPTIME_LSB_REG, ASIC_INT_EXPTIME_MSB_REG) /
           (1E-9 * systemFrequency);
}

int setPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid period: %lld ns\n", val));
        return FAIL;
    }
    // continuous
    if (burstMode == CONTINUOUS_INTERNAL || burstMode == CONTINUOUS_EXTERNAL) {
        LOG(logINFO, ("Setting period %lld ns [Continuous mode]\n", val));
        val = (val * 1E-9 * systemFrequency) + 0.5;
        // trigger
        if (getTiming() == TRIGGER_EXPOSURE) {
            LOG(logINFO,
                ("\tCont. Trigger mode (not writing to register)\n", val));
            // #frames limited in cont trigger mode
        }
        // auto
        else {
            set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);
        }
    }
    // burst
    else {
        LOG(logINFO, ("Setting period %lld ns [Burst mode]\n", val));
        val = (val * 1E-9 * systemFrequency) + 0.5;
        set64BitReg(val, ASIC_INT_PERIOD_LSB_REG, ASIC_INT_PERIOD_MSB_REG);
    }
    periodReg = val;
    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-9 * systemFrequency);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    // continuous
    if (burstMode == CONTINUOUS_INTERNAL || burstMode == CONTINUOUS_EXTERNAL) {
        // trigger
        if (getTiming() == TRIGGER_EXPOSURE) {
            // #frames limited in cont trigger mode
            return periodReg / (1E-9 * systemFrequency);
        }
        // auto
        return get64BitReg(SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
               (1E-9 * systemFrequency);
    }
    // burst
    else {
        return get64BitReg(ASIC_INT_PERIOD_LSB_REG, ASIC_INT_PERIOD_MSB_REG) /
               (1E-9 * systemFrequency);
    }
}

int setDelayAfterTrigger(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid delay after trigger: %lld ns\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting delay after trigger %lld ns\n", val));
    val = (val * 1E-9 * systemFrequency) + 0.5;
    delayReg = val;
    if (getTiming() == AUTO_TIMING) {
        LOG(logINFO, ("\tAuto mode (not writing to register)\n"));
    } else {
        set64BitReg(val, SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG);
    }
    // validate for tolerance
    int64_t retval = getDelayAfterTrigger();
    val /= (1E-9 * systemFrequency);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getDelayAfterTrigger() {
    if (getTiming() == AUTO_TIMING) {
        return delayReg / (1E-9 * systemFrequency);
    }
    return get64BitReg(SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG) /
           (1E-9 * systemFrequency);
}

int setBurstPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid burst period: %lld ns\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting burst period %lld ns\n", val));
    val = (val * 1E-9 * systemFrequency) + 0.5;
    burstPeriodReg = val;

    // burst and auto
    if ((burstMode == BURST_INTERNAL || burstMode == BURST_EXTERNAL) &&
        getTiming() == AUTO_TIMING) {
        set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);
    }
    // burst-trigger, continuous
    else {
        LOG(logINFO, ("\tNot Burst and Auto mode (not writing to register)\n"));
    }

    // validate for tolerance
    int64_t retval = getBurstPeriod();
    val /= (1E-9 * systemFrequency);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getBurstPeriod() {
    // burst and auto
    if ((burstMode == BURST_INTERNAL || burstMode == BURST_EXTERNAL) &&
        getTiming() == AUTO_TIMING) {
        return get64BitReg(SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
               (1E-9 * systemFrequency);
    }
    // burst-trigger, continuous
    return burstPeriodReg / (1E-9 * systemFrequency);
}

int64_t getNumFramesLeft() {
    // continuous and auto
    if ((burstMode == CONTINUOUS_INTERNAL ||
         burstMode == CONTINUOUS_EXTERNAL) &&
        getTiming() == AUTO_TIMING) {
        return (get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG) + 1);
    }
    return -1;
}

int64_t getNumTriggersLeft() {
    // trigger
    if (getTiming() == TRIGGER_EXPOSURE) {
        return (get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG) + 1);
    }
    return -1;
}

int64_t getDelayAfterTriggerLeft() {
    return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) /
           (1E-9 * systemFrequency);
}

int64_t getPeriodLeft() {
    return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) /
           (1E-9 * systemFrequency);
}

int64_t getNumBurstsLeft() {
    // burst and auto
    if ((burstMode == BURST_INTERNAL || burstMode == BURST_EXTERNAL) &&
        getTiming() == AUTO_TIMING) {
        return (get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG) + 1);
    }
    return -1;
}

int64_t getFramesFromStart() {
    return get64BitReg(FRAMES_FROM_START_LSB_REG, FRAMES_FROM_START_MSB_REG);
}

int64_t getActualTime() {
    return get64BitReg(TIME_FROM_START_LSB_REG, TIME_FROM_START_MSB_REG) /
           (1E-9 * FIXED_PLL_FREQUENCY * 2);
}

int64_t getMeasurementTime() {
    return get64BitReg(START_FRAME_TIME_LSB_REG, START_FRAME_TIME_MSB_REG) /
           (1E-9 * FIXED_PLL_FREQUENCY);
}

/* parameters - module, settings */
enum detectorSettings setSettings(enum detectorSettings sett) {
    if (sett == UNINITIALIZED)
        return thisSettings;

    // set settings
    uint32_t addr = ASIC_CONFIG_REG;
    uint32_t mask = ASIC_CONFIG_GAIN_MSK;
    switch (sett) {
    case DYNAMICGAIN:
        bus_w(addr, bus_r(addr) & ~mask);
        bus_w(addr, bus_r(addr) | ASIC_CONFIG_DYNAMIC_GAIN_VAL);
        LOG(logINFO,
            ("Set settings - Dyanmic Gain, val: 0x%x\n", bus_r(addr) & mask));
        break;
    case FIXGAIN1:
        bus_w(addr, bus_r(addr) & ~mask);
        bus_w(addr, bus_r(addr) | ASIC_CONFIG_FIX_GAIN_1_VAL);
        LOG(logINFO,
            ("Set settings - Fix Gain 1, val: 0x%x\n", bus_r(addr) & mask));
        break;
    case FIXGAIN2:
        bus_w(addr, bus_r(addr) & ~mask);
        bus_w(addr, bus_r(addr) | ASIC_CONFIG_FIX_GAIN_2_VAL);
        LOG(logINFO,
            ("Set settings - Fix Gain 2, val: 0x%x\n", bus_r(addr) & mask));
        break;
    default:
        LOG(logERROR,
            ("This settings is not defined for this detector %d\n", (int)sett));
        return -1;
    }
    thisSettings = sett;

    return getSettings();
}

enum detectorSettings getSettings() {
    uint32_t regval = bus_r(ASIC_CONFIG_REG);
    uint32_t val = regval & ASIC_CONFIG_GAIN_MSK;
    LOG(logDEBUG1, ("Getting Settings\n Reading val :0x%x\n", val));

    switch (val) {
    case ASIC_CONFIG_RESERVED_VAL:
    case ASIC_CONFIG_DYNAMIC_GAIN_VAL:
        thisSettings = DYNAMICGAIN;
        LOG(logDEBUG1, ("Settings read: Dynamic Gain. val: 0x%x\n", val));
        break;
    case ASIC_CONFIG_FIX_GAIN_1_VAL:
        thisSettings = FIXGAIN1;
        LOG(logDEBUG1, ("Settings read: Fix Gain 1. val: 0x%x\n", val));
        break;
    case ASIC_CONFIG_FIX_GAIN_2_VAL:
        thisSettings = FIXGAIN2;
        LOG(logDEBUG1, ("Settings read: Fix Gain 2. val: 0x%x\n", val));
        break;
    default:
        thisSettings = UNDEFINED;
        LOG(logERROR, ("Settings read: Undefined. val: 0x%x\n", val));
    }
    return thisSettings;
}

/* parameters - dac, hv */
int setOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex, int val) {
    char *names[] = {ONCHIP_DAC_NAMES};
    LOG(logDEBUG1,
        ("Setting on chip dac[%d - %s]: 0x%x\n", (int)ind, names[ind], val));

    if (ind >= ONCHIP_NDAC) {
        LOG(logERROR, ("Invalid dac index %d\n", (int)ind));
        return FAIL;
    }
    if (chipIndex >= NCHIP) {
        LOG(logERROR, ("Invalid chip index %d\n", chipIndex));
        return FAIL;
    }
    if (val > ONCHIP_DAC_MAX_VAL) {
        LOG(logERROR, ("Invalid val %d\n", val));
        return FAIL;
    }
    LOG(logINFO,
        ("Setting on chip dac[%d - %s]: 0x%x\n", (int)ind, names[ind], val));

    char buffer[2];
    memset(buffer, 0, sizeof(buffer));
    buffer[1] =
        ((val & 0xF) << 4) | (((int)ind) & 0xF); // LSB (4 bits) + ADDR (4 bits)
    buffer[0] = (val >> 4) & 0x3F;               // MSB (6 bits)

    if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
        return FAIL;
    }
    // all chips
    if (chipIndex == -1) {
        for (int ichip = 0; ichip < NCHIP; ++ichip) {
            onChipdacValues[ind][ichip] = val;
        }
    }

    // specific chip
    else {
        onChipdacValues[ind][chipIndex] = val;
    }
    return OK;
}

int getOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex) {
    // all chips
    if (chipIndex == -1) {
        int retval = onChipdacValues[ind][0];
        // check if same value for remaining chips
        for (int ichip = 1; ichip < NCHIP; ++ichip) {
            if (onChipdacValues[ind][ichip] != retval) {
                return -1;
            }
        }
        return retval;
    }
    // specific chip
    return onChipdacValues[ind][chipIndex];
}

void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0) {
        return;
    }

    char *dac_names[] = {DAC_NAMES};
    LOG(logDEBUG1, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                    val, (mV ? "mV" : "dac units")));
    int dacval = val;
#ifdef VIRTUAL
    LOG(logINFO, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                  val, (mV ? "mV" : "dac units")));
    if (!mV) {
        dacValues[ind] = val;
    }
    // convert to dac units
    else if (LTC2620_D_VoltageToDac(val, &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#else
    if (LTC2620_D_SetDACValue((int)ind, val, mV, dac_names[ind], &dacval) ==
        OK) {
        dacValues[ind] = dacval;
    }
#endif
}

int getDAC(enum DACINDEX ind, int mV) {
    if (!mV) {
        LOG(logDEBUG1, ("Getting DAC %d : %d dac\n", ind, dacValues[ind]));
        return dacValues[ind];
    }
    int voltage = -1;
    LTC2620_D_DacToVoltage(dacValues[ind], &voltage);
    LOG(logDEBUG1,
        ("Getting DAC %d : %d dac (%d mV)\n", ind, dacValues[ind], voltage));
    return voltage;
}

int getMaxDacSteps() { return LTC2620_D_GetMaxNumSteps(); }

int getADC(enum ADCINDEX ind, int *value) {
    LOG(logDEBUG1, ("Reading FPGA temperature...\n"));
    if (readParameterFromFile(TEMPERATURE_FILE_NAME, "temperature", value) ==
        FAIL) {
        LOG(logERROR, ("Could not get temperature\n"));
        return FAIL;
    }
    LOG(logINFO, ("Temperature: %.2f °C\n", (double)(*value) / 1000.00));
    return OK;
}

int setHighVoltage(int val) {
    if (val > HV_SOFT_MAX_VOLTAGE) {
        LOG(logERROR, ("Invalid high voltage: %d V\n", val));
        return FAIL;
    }

    LOG(logINFO, ("Setting High voltage: %d V\n", val));
    int waitTime = WAIT_HIGH_VOLTAGE_SETTLE_TIME_S;

    // get current high voltage
    int prevHighVoltage = 0;
    // at startup (initCheck not done: to not wait 10s assuming hv = 0
    // otherwise as below, always check current hv to wait 10s if powering off
    if (initCheckDone) {
        if (getHighVoltage(&prevHighVoltage) == FAIL) {
            LOG(logERROR, ("Could not get current high voltage to determine if "
                           "%d s wait is required\n",
                           waitTime));
            return FAIL;
        }
    }

    int ret = DAC6571_Set(val);

    // only when powering off (from non zero value), wait 10s
    if (ret == OK) {
        if (prevHighVoltage > 0 && val == 0) {
            LOG(logINFO,
                ("\tSwitching off high voltage requires %d s...\n", waitTime));
            sleep(waitTime);
            LOG(logINFO, ("\tAssuming high voltage switched off\n"));
        }
    }
    return ret;
}

int getHighVoltage(int *retval) { return DAC6571_Get(retval); }

/* parameters - timing */

int setMaster(enum MASTERINDEX m) {
    char *master_names[] = {MASTER_NAMES};
    LOG(logINFOBLUE, ("Setting up as %s in (%s server)\n", master_names[m],
                      (isControlServer ? "control" : "stop")));
    int retval = -1;
    switch (m) {
    case OW_MASTER:
        bus_w(CONFIG_REG, bus_r(CONFIG_REG) & ~CONFIG_SLAVE_MSK);
        isMaster(&retval);
        if (retval != 1) {
            LOG(logERROR, ("Could not set master\n"));
            return FAIL;
        }
        break;
    case OW_SLAVE:
        bus_w(CONFIG_REG, bus_r(CONFIG_REG) | CONFIG_SLAVE_MSK);
        isMaster(&retval);
        if (retval != 0) {
            LOG(logERROR, ("Could not set slave\n"));
            return FAIL;
        }
        break;
    default:
        // hardware settings (do nothing)
        break;
    }
    return OK;
}

int isMaster(int *retval) {
    int slave = ((bus_r(CONFIG_REG) & CONFIG_SLAVE_MSK) >> CONFIG_SLAVE_OFST);
    *retval = (slave == 1 ? 0 : 1);
    return OK;
}

void updatingRegisters() {
    LOG(logINFO, ("\tUpdating registers\n"));
    // burst
    if (burstMode == BURST_INTERNAL || burstMode == BURST_EXTERNAL) {
        // auto
        if (getTiming() == AUTO_TIMING) {
            LOG(logINFO, ("\t[Burst, Auto mode]\n"))
            // trigger reg
            set64BitReg(1, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
            LOG(logINFO, ("\tTrigger reg: %lld\n",
                          get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG)));
            // delay reg
            set64BitReg(0, SET_TRIGGER_DELAY_LSB_REG,
                        SET_TRIGGER_DELAY_MSB_REG);
            LOG(logINFO,
                ("\tDelay reg: %lldns\n",
                 (long long int)(get64BitReg(SET_TRIGGER_DELAY_LSB_REG,
                                             SET_TRIGGER_DELAY_MSB_REG) /
                                 (1E-9 * systemFrequency))));
            // frame reg
            set64BitReg(numBurstsReg, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
            LOG(logINFO, ("\tFrame reg (bursts): %lld\n",
                          get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG)));
            // period reg
            set64BitReg(burstPeriodReg, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);
            LOG(logINFO, ("\tPeriod reg (burst period): %lldns\n",
                          (long long int)(get64BitReg(SET_PERIOD_LSB_REG,
                                                      SET_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // int. frame reg
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) & ~ASIC_INT_FRAMES_MSK);
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) |
                      (((int)numFramesReg << ASIC_INT_FRAMES_OFST) &
                       ASIC_INT_FRAMES_MSK));
            LOG(logINFO, ("\tInt. Frame reg (frames): %d\n",
                          ((bus_r(ASIC_INT_FRAMES_REG) & ASIC_INT_FRAMES_MSK) >>
                           ASIC_INT_FRAMES_OFST)));
            // int. period reg
            set64BitReg(periodReg, ASIC_INT_PERIOD_LSB_REG,
                        ASIC_INT_PERIOD_MSB_REG);
            LOG(logINFO, ("\tInt. Period reg (period): %lldns\n",
                          (long long int)(get64BitReg(ASIC_INT_PERIOD_LSB_REG,
                                                      ASIC_INT_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // extra frame reg (N.A)
        }
        // trigger
        else {
            LOG(logINFO, ("\t[Burst, Trigger mode]\n"))
            // trigger reg
            set64BitReg(numTriggersReg, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
            LOG(logINFO, ("\tTrigger reg (triggers): %lld\n",
                          get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG)));
            // delay reg
            set64BitReg(delayReg, SET_TRIGGER_DELAY_LSB_REG,
                        SET_TRIGGER_DELAY_MSB_REG);
            LOG(logINFO,
                ("\tDelay reg (delay): %lldns\n",
                 (long long int)(get64BitReg(SET_TRIGGER_DELAY_LSB_REG,
                                             SET_TRIGGER_DELAY_MSB_REG) /
                                 (1E-9 * systemFrequency))));
            // frame reg
            set64BitReg(1, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
            LOG(logINFO, ("\tFrame reg: %lld\n",
                          get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG)));
            // period reg
            set64BitReg(0, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);
            LOG(logINFO, ("\tPeriod reg: %lldns\n",
                          (long long int)(get64BitReg(SET_PERIOD_LSB_REG,
                                                      SET_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // int. frame reg
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) & ~ASIC_INT_FRAMES_MSK);
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) |
                      (((int)numFramesReg << ASIC_INT_FRAMES_OFST) &
                       ASIC_INT_FRAMES_MSK));
            LOG(logINFO, ("\tInt. Frame reg (frames): %d\n",
                          ((bus_r(ASIC_INT_FRAMES_REG) & ASIC_INT_FRAMES_MSK) >>
                           ASIC_INT_FRAMES_OFST)));
            // int. period reg
            set64BitReg(periodReg, ASIC_INT_PERIOD_LSB_REG,
                        ASIC_INT_PERIOD_MSB_REG);
            LOG(logINFO, ("\tInt. Period reg (period): %lldns\n",
                          (long long int)(get64BitReg(ASIC_INT_PERIOD_LSB_REG,
                                                      ASIC_INT_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // extra frame reg (N.A)
        }
    }
    // continuous
    else {
        // auto
        if (getTiming() == AUTO_TIMING) {
            LOG(logINFO, ("\t[Continuous, Auto mode]\n"))
            // trigger reg
            set64BitReg(1, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
            LOG(logINFO, ("\tTrigger reg: %lld\n",
                          get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG)));
            // delay reg
            set64BitReg(0, SET_TRIGGER_DELAY_LSB_REG,
                        SET_TRIGGER_DELAY_MSB_REG);
            LOG(logINFO,
                ("\tDelay reg: %lldns\n",
                 (long long int)(get64BitReg(SET_TRIGGER_DELAY_LSB_REG,
                                             SET_TRIGGER_DELAY_MSB_REG) /
                                 (1E-9 * systemFrequency))));
            // frame reg
            set64BitReg(numFramesReg, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
            LOG(logINFO, ("\tFrame reg (frames): %lld\n",
                          get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG)));
            // period reg
            set64BitReg(periodReg, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);
            LOG(logINFO, ("\tPeriod reg (period): %lldns\n",
                          (long long int)(get64BitReg(SET_PERIOD_LSB_REG,
                                                      SET_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // int. frame reg
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) & ~ASIC_INT_FRAMES_MSK);
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) |
                      ((1 << ASIC_INT_FRAMES_OFST) & ASIC_INT_FRAMES_MSK));
            LOG(logINFO, ("\tInt. Frame reg: %d\n",
                          ((bus_r(ASIC_INT_FRAMES_REG) & ASIC_INT_FRAMES_MSK) >>
                           ASIC_INT_FRAMES_OFST)));
            // int. period reg
            set64BitReg(0, ASIC_INT_PERIOD_LSB_REG, ASIC_INT_PERIOD_MSB_REG);
            LOG(logINFO, ("\tInt. Period reg: %lldns\n",
                          (long long int)(get64BitReg(ASIC_INT_PERIOD_LSB_REG,
                                                      ASIC_INT_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // extra frame reg
            set64BitReg(numFramesReg, ASIC_CONT_FRAMES_LSB_REG,
                        ASIC_CONT_FRAMES_MSB_REG);
            LOG(logINFO, ("\tExtra Frame reg (frames): %lld\n",
                          get64BitReg(ASIC_CONT_FRAMES_LSB_REG,
                                      ASIC_CONT_FRAMES_MSB_REG)));
        }
        // trigger
        else {
            LOG(logINFO, ("\t[Continuous, Trigger mode]\n"))
            // trigger reg
            set64BitReg(numTriggersReg, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
            LOG(logINFO, ("\tTrigger reg (triggers): %lld\n",
                          get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG)));
            // delay reg
            set64BitReg(delayReg, SET_TRIGGER_DELAY_LSB_REG,
                        SET_TRIGGER_DELAY_MSB_REG);
            LOG(logINFO,
                ("\tDelay reg (delay): %lldns\n",
                 (long long int)(get64BitReg(SET_TRIGGER_DELAY_LSB_REG,
                                             SET_TRIGGER_DELAY_MSB_REG) /
                                 (1E-9 * systemFrequency))));
            // frame reg
            set64BitReg(1, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
            LOG(logINFO, ("\tFrame reg: %lld\n",
                          get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG)));
            // period reg
            set64BitReg(0, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);
            LOG(logINFO, ("\tPeriod reg: %lld\n",
                          (long long int)(get64BitReg(SET_PERIOD_LSB_REG,
                                                      SET_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // int. frame reg
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) & ~ASIC_INT_FRAMES_MSK);
            bus_w(ASIC_INT_FRAMES_REG,
                  bus_r(ASIC_INT_FRAMES_REG) |
                      ((1 << ASIC_INT_FRAMES_OFST) & ASIC_INT_FRAMES_MSK));
            LOG(logINFO, ("\tInt. Frame reg: %d\n",
                          ((bus_r(ASIC_INT_FRAMES_REG) & ASIC_INT_FRAMES_MSK) >>
                           ASIC_INT_FRAMES_OFST)));
            // int. period reg
            set64BitReg(0, ASIC_INT_PERIOD_LSB_REG, ASIC_INT_PERIOD_MSB_REG);
            LOG(logINFO, ("\tInt. Period reg: %lldns\n",
                          (long long int)(get64BitReg(ASIC_INT_PERIOD_LSB_REG,
                                                      ASIC_INT_PERIOD_MSB_REG) /
                                          (1E-9 * systemFrequency))));
            // extra frame reg
            set64BitReg(numTriggersReg, ASIC_CONT_FRAMES_LSB_REG,
                        ASIC_CONT_FRAMES_MSB_REG);
            LOG(logINFO, ("\tExtra Frame reg (triggers): %lld\n",
                          get64BitReg(ASIC_CONT_FRAMES_LSB_REG,
                                      ASIC_CONT_FRAMES_MSB_REG)));
        }
    }
    LOG(logINFO, ("\tDone Updating registers\n\n"));
}

int updateClockDivs() {
    char *clock_names[] = {CLK_NAMES};
    switch (burstMode) {
    case BURST_INTERNAL:
    case BURST_EXTERNAL:
        if (setClockDivider(SYSTEM_C0, DEFAULT_BURST_SYSTEM_C0) == FAIL) {
            LOG(logERROR, ("Could not set clk %s speed to %d.\n",
                           clock_names[SYSTEM_C0], DEFAULT_BURST_SYSTEM_C0));
            return FAIL;
        }
        if (setClockDivider(SYSTEM_C1, DEFAULT_BURST_SYSTEM_C1) == FAIL) {
            LOG(logERROR, ("Could not set clk %s speed to %d.\n",
                           clock_names[SYSTEM_C0], DEFAULT_BURST_SYSTEM_C1));
            return FAIL;
        }
        if (setClockDivider(SYSTEM_C2, DEFAULT_BURST_SYSTEM_C2) == FAIL) {
            LOG(logERROR, ("Could not set clk %s speed to %d.\n",
                           clock_names[SYSTEM_C0], DEFAULT_BURST_SYSTEM_C2));
            return FAIL;
        }
        break;
    case CONTINUOUS_INTERNAL:
    case CONTINUOUS_EXTERNAL:
        if (setClockDivider(SYSTEM_C0, DEFAULT_CNTNS_SYSTEM_C0) == FAIL) {
            LOG(logERROR, ("Could not set clk %s speed to %d.\n",
                           clock_names[SYSTEM_C0], DEFAULT_CNTNS_SYSTEM_C0));
            return FAIL;
        }
        if (setClockDivider(SYSTEM_C1, DEFAULT_CNTNS_SYSTEM_C1) == FAIL) {
            LOG(logERROR, ("Could not set clk %s speed to %d.\n",
                           clock_names[SYSTEM_C0], DEFAULT_CNTNS_SYSTEM_C1));
            return FAIL;
        }
        if (setClockDivider(SYSTEM_C2, DEFAULT_CNTNS_SYSTEM_C2) == FAIL) {
            LOG(logERROR, ("Could not set clk %s speed to %d.\n",
                           clock_names[SYSTEM_C0], DEFAULT_CNTNS_SYSTEM_C2));
            return FAIL;
        }
        break;
    default:
        LOG(logERROR, ("Unknown burst mode. Cannot update clock divs.\n"));
        return FAIL;
    }
    return OK;
}

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

    updatingRegisters();
}

enum timingMode getTiming() {
    if (bus_r(EXT_SIGNAL_REG) == EXT_SIGNAL_MSK)
        return TRIGGER_EXPOSURE;
    return AUTO_TIMING;
}

/* configure mac */
void setNumberofUDPInterfaces(int val) {
    uint32_t addr = CONFIG_REG;

    // 2 rxr interfaces (enable debugging interface)
    if (val > 1) {
        LOG(logINFOBLUE, ("Enabling 10GbE (debugging) veto streaming\n"));
        bus_w(addr, bus_r(addr) | CONFIG_VETO_CH_10GBE_ENBL_MSK);
    }
    // 1 rxr interface (disable debugging interface)
    else {
        LOG(logINFOBLUE, ("Disabling 10GbE (debugging) veto streaming\n"));
        bus_w(addr, bus_r(addr) & ~CONFIG_VETO_CH_10GBE_ENBL_MSK);
    }
    LOG(logDEBUG, ("config reg:0x%x\n", bus_r(addr)));
}

int getNumberofUDPInterfaces() {
    LOG(logDEBUG, ("config reg:0x%x\n", bus_r(CONFIG_REG)));
    // return 2 if 10gbps veto streaming enabled, else 1
    return ((bus_r(CONFIG_REG) & CONFIG_VETO_CH_10GBE_ENBL_MSK) ? 2 : 1);
}

int getNumberofDestinations(int *retval) {
    *retval = (((bus_r(PKT_CONFIG_REG) & PKT_CONFIG_NRXR_MAX_MSK) >>
                PKT_CONFIG_NRXR_MAX_OFST) +
               1);
    return OK;
}

int setNumberofDestinations(int value) {
    LOG(logINFO, ("Setting number of entries to %d\n", value));
    --value;
    bus_w(PKT_CONFIG_REG, bus_r(PKT_CONFIG_REG) & ~PKT_CONFIG_NRXR_MAX_MSK);
    bus_w(PKT_CONFIG_REG,
          bus_r(PKT_CONFIG_REG) |
              ((value << PKT_CONFIG_NRXR_MAX_OFST) & PKT_CONFIG_NRXR_MAX_MSK));
    return OK;
}

int getFirstUDPDestination() {
    return ((bus_r(PKT_CONFIG_REG) & PKT_CONFIG_RXR_START_ID_MSK) >>
            PKT_CONFIG_RXR_START_ID_OFST);
}

void setFirstUDPDestination(int value) {
    LOG(logINFO, ("Setting first entry to %d\n", value));
    bus_w(PKT_CONFIG_REG, bus_r(PKT_CONFIG_REG) & ~PKT_CONFIG_RXR_START_ID_MSK);
    bus_w(PKT_CONFIG_REG,
          bus_r(PKT_CONFIG_REG) | ((value << PKT_CONFIG_RXR_START_ID_OFST) &
                                   PKT_CONFIG_RXR_START_ID_MSK));
}

void setupHeader(int iRxEntry, int vetoInterface, uint32_t destip,
                 uint64_t destmac, uint16_t destport, uint64_t sourcemac,
                 uint32_t sourceip, uint16_t sourceport) {

    // start addr
    uint32_t addr = BASE_UDP_RAM;
    // calculate rxr endpoint offset
    if (vetoInterface == 1) {
        iRxEntry += RXR_ENDPOINTS_MAX;
    }
    addr += (iRxEntry * RXR_ENDPOINT_OFST);
    // get struct memory
    udp_header *udp =
        (udp_header *)(Nios_getBaseAddress() + addr / (sizeof(u_int32_t)));
    memset(udp, 0, sizeof(udp_header));

    //  mac addresses
    // msb (32) + lsb (16)
    udp->udp_destmac_msb = ((destmac >> 16) & BIT32_MASK);
    udp->udp_destmac_lsb = ((destmac >> 0) & BIT16_MASK);
    // msb (16) + lsb (32)
    udp->udp_srcmac_msb = ((sourcemac >> 32) & BIT16_MASK);
    udp->udp_srcmac_lsb = ((sourcemac >> 0) & BIT32_MASK);

    // ip addresses
    udp->ip_srcip_msb = ((sourceip >> 16) & BIT16_MASK);
    udp->ip_srcip_lsb = ((sourceip >> 0) & BIT16_MASK);
    udp->ip_destip_msb = ((destip >> 16) & BIT16_MASK);
    udp->ip_destip_lsb = ((destip >> 0) & BIT16_MASK);

    // source port
    udp->udp_srcport = sourceport;
    udp->udp_destport = destport;

    // other defines
    udp->udp_ethertype = 0x800;
    udp->ip_ver = 0x4;
    udp->ip_ihl = 0x5;
    udp->ip_flags = 0x2; // FIXME
    udp->ip_ttl = 0x40;
    udp->ip_protocol = 0x11;
    // total length is redefined in firmware

    calcChecksum(udp);
    if (iRxEntry < numUdpDestinations) {
        LOG(logINFO, ("\tIP checksum : 0x%lx\n\n", udp->ip_checksum));
    }
}

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

    LOG(logINFOBLUE, ("Configuring MAC\n"));

    LOG(logINFO, ("Number of entries: %d\n\n", numUdpDestinations));
    for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION; ++iRxEntry) {
        uint32_t srcip = udpDetails[iRxEntry].srcip;
        uint32_t srcip2 = udpDetails[iRxEntry].srcip2;
        uint32_t dstip = udpDetails[iRxEntry].dstip;
        uint32_t dstip2 = udpDetails[iRxEntry].dstip2;
        uint64_t srcmac = udpDetails[iRxEntry].srcmac;
        uint64_t srcmac2 = udpDetails[iRxEntry].srcmac2;
        uint64_t dstmac = udpDetails[iRxEntry].dstmac;
        uint64_t dstmac2 = udpDetails[iRxEntry].dstmac2;
        uint16_t srcport = udpDetails[iRxEntry].srcport;
        uint16_t srcport2 = udpDetails[iRxEntry].srcport2;
        uint16_t dstport = udpDetails[iRxEntry].dstport;
        uint16_t dstport2 = udpDetails[iRxEntry].dstport2;

        char src_mac[MAC_ADDRESS_SIZE], src_ip[INET_ADDRSTRLEN],
            dst_mac[MAC_ADDRESS_SIZE], dst_ip[INET_ADDRSTRLEN];
        getMacAddressinString(src_mac, MAC_ADDRESS_SIZE, srcmac);
        getMacAddressinString(dst_mac, MAC_ADDRESS_SIZE, dstmac);
        getIpAddressinString(src_ip, srcip);
        getIpAddressinString(dst_ip, dstip);
        char src_mac2[MAC_ADDRESS_SIZE], src_ip2[INET_ADDRSTRLEN],
            dst_mac2[MAC_ADDRESS_SIZE], dst_ip2[INET_ADDRSTRLEN];
        getMacAddressinString(src_mac2, MAC_ADDRESS_SIZE, srcmac2);
        getMacAddressinString(dst_mac2, MAC_ADDRESS_SIZE, dstmac2);
        getIpAddressinString(src_ip2, srcip2);
        getIpAddressinString(dst_ip2, dstip2);

        int i10gbe = (getNumberofUDPInterfaces() == 2 ? 1 : 0);
        if (iRxEntry < numUdpDestinations) {
            LOG(logINFOBLUE, ("\tEntry %d\n", iRxEntry));

            LOG(logINFO, ("\tData Interface \n"));
            LOG(logINFO, ("\tSource IP   : %s\n"
                          "\tSource MAC  : %s\n"
                          "\tSource Port : %hu\n"
                          "\tDest IP     : %s\n"
                          "\tDest MAC    : %s\n"
                          "\tDest Port   : %hu\n\n",
                          src_ip, src_mac, srcport, dst_ip, dst_mac, dstport));

            if (getVetoStream()) {
                LOG(logINFOGREEN, ("\tVeto (lll) : enabled\n\n"));
            } else {
                LOG(logINFORED, ("\tVeto (lll) : disabled\n\n"));
            }
            if (i10gbe) {
                LOG(logINFOGREEN, ("\tVeto (10GbE): enabled\n"));
            } else {
                LOG(logINFORED, ("\tVeto (10GbE): disabled\n"));
            }
            LOG(logINFO,
                ("\tSource IP2  : %s\n"
                 "\tSource MAC2 : %s\n"
                 "\tSource Port2: %hu\n"
                 "\tDest IP2    : %s\n"
                 "\tDest MAC2   : %s\n"
                 "\tDest Port2  : %hu\n\n",
                 src_ip2, src_mac2, srcport2, dst_ip2, dst_mac2, dstport2));
        }
#ifdef VIRTUAL
        if (setUDPDestinationDetails(iRxEntry, 0, dst_ip, dstport) == FAIL) {
            LOG(logERROR, ("could not set udp destination IP and port for "
                           "data interface [entry:%d] \n",
                           iRxEntry));
            return FAIL;
        }
        if (i10gbe &&
            setUDPDestinationDetails(iRxEntry, 1, dst_ip2, dstport2) == FAIL) {
            LOG(logERROR, ("could not set udp destination IP and port for "
                           "veto interface [entry:%d] \n",
                           iRxEntry));
            return FAIL;
        }
#endif
        // data
        setupHeader(iRxEntry, 0, dstip, dstmac, dstport, srcmac, srcip,
                    srcport);

        // veto
        if (i10gbe) {
            setupHeader(iRxEntry, 1, dstip2, dstmac2, dstport2, srcmac2, srcip2,
                        srcport2);
        }
    }
    cleanFifos();
    resetCore();
    // alignDeserializer();
    return OK;
}

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));

    uint32_t addr = COORD_0_REG;
    int value = 0;
    int valueRead = 0;
    int ret = OK;

    // row
    value = detPos[Y];
    bus_w(addr, (bus_r(addr) & ~COORD_ROW_MSK) |
                    ((value << COORD_ROW_OFST) & COORD_ROW_MSK));
    valueRead = ((bus_r(addr) & COORD_ROW_MSK) >> COORD_ROW_OFST);
    if (valueRead != value) {
        LOG(logERROR,
            ("Could not set row. Set %d, read %d\n", value, valueRead));
        ret = FAIL;
    }

    // col
    value = detPos[X];
    bus_w(addr, (bus_r(addr) & ~COORD_COL_MSK) |
                    ((value << COORD_COL_OFST) & COORD_COL_MSK));
    valueRead = ((bus_r(addr) & COORD_COL_MSK) >> COORD_COL_OFST);
    if (valueRead != value) {
        LOG(logERROR,
            ("Could not set column. Set %d, read %d\n", value, valueRead));
        ret = FAIL;
    }

    if (ret == OK) {
        LOG(logINFO,
            ("\tPosition set to [%d, %d] #(col, row)\n", detPos[X], detPos[Y]));
    }

    return ret;
}

int *getDetectorPosition() { return detPos; }

// Detector Specific

int checkDetectorType(char *mess) {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Checking module type\n"));
    FILE *fd = fopen(TYPE_FILE_NAME, "r");
    if (fd == NULL) {
        sprintf(mess,
                "Could not open file %s to get type of the module attached\n",
                TYPE_FILE_NAME);
        return FAIL;
    }
    char buffer[MAX_STR_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fread(buffer, MAX_STR_LENGTH, sizeof(char), fd);
    fclose(fd);
    if (strlen(buffer) == 0) {
        sprintf(mess,
                "Could not read file %s to get type of the module attached\n",
                TYPE_FILE_NAME);
        LOG(logERROR, (mess));
        return FAIL;
    }

    {
        int type = atoi(buffer);
        int hdiSlave = 0;
        int hdiVersion = 0x1;
        int hdi25um = 1;
        if (abs(type - TYPE_GOTTHARD2_25UM_MASTER_HD1_V1_VAL) <=
            TYPE_TOLERANCE) {
            LOG(logINFOBLUE, ("MASTER 25um Module (HDI v1.0)\n"));
        } else if (abs(type - TYPE_GOTTHARD2_25UM_MASTER_HD1_V2_VAL) <=
                   TYPE_TOLERANCE) {
            LOG(logINFOBLUE, ("MASTER 25um Module (HDI v2.0)\n"));
            hdiVersion = 0x2;
        } else if (abs(type - TYPE_GOTTHARD2_25UM_SLAVE_HDI_V1_VAL) <=
                   TYPE_TOLERANCE) {
            LOG(logINFOBLUE, ("SLAVE 25um Module (HDI v1.0)\n"));
            hdiSlave = 1;
        } else if (abs(type - TYPE_GOTTHARD2_25UM_SLAVE_HDI_V2_VAL) <=
                   TYPE_TOLERANCE) {
            LOG(logINFOBLUE, ("SLAVE 25um Module (HDI v2.0)\n"));
            hdiSlave = 1;
            hdiVersion = 0x2;
        } else if (abs(type - TYPE_GOTTHARD2_MODULE_VAL) <= TYPE_TOLERANCE) {
            LOG(logINFOBLUE, ("50um Module\n"));
            hdi25um = 0;
        }
        // no module or invalid module
        else if (type > TYPE_NO_MODULE_STARTING_VAL) {
            sprintf(mess, "No Module attached! Run server with -nomodule.\n");
            LOG(logERROR, (mess));
            return FAIL;
        } else {
            sprintf(mess,
                    "Wrong Module attached! Expected %d, %d, %d, %d or %d for "
                    "Gotthard2, got %d\n",
                    TYPE_GOTTHARD2_MODULE_VAL,
                    TYPE_GOTTHARD2_25UM_MASTER_HD1_V1_VAL,
                    TYPE_GOTTHARD2_25UM_SLAVE_HDI_V1_VAL,
                    TYPE_GOTTHARD2_25UM_MASTER_HD1_V2_VAL,
                    TYPE_GOTTHARD2_25UM_SLAVE_HDI_V2_VAL, type);
            LOG(logERROR, (mess));
            return FAIL;
        }
        if (hdiSlave) {
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) | CONFIG_HDI_SLAVE_MSK);
        } else {
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) & ~CONFIG_HDI_SLAVE_MSK);
        }
        if (hdi25um) {
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) | CONFIG_HDI_25UM_MSK);
        } else {
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) & ~CONFIG_HDI_25UM_MSK);
        }
        bus_w(CONFIG_REG, bus_r(CONFIG_REG) & ~CONFIG_HDI_VERSION_MSK);
        bus_w(CONFIG_REG,
              bus_r(CONFIG_REG) | ((hdiVersion << CONFIG_HDI_VERSION_OFST) &
                                   CONFIG_HDI_VERSION_MSK));
    }

    {
        enum MASTERINDEX master =
            (bus_r(CONFIG_REG) & CONFIG_HDI_SLAVE_MSK) ? OW_SLAVE : OW_MASTER;
        if (setMaster(master) == FAIL) {
            strcpy(mess, "Could not set to master/slave.");
            LOG(logERROR, (mess));
            return FAIL;
        }
    }
    return OK;
}

int powerChip(int on, char *mess) {
    if (on) {
        LOG(logINFO, ("Powering chip: on\n"));
        bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PWR_CHIP_MSK);
        if (configureChip(mess) == FAIL)
            return FAIL;
    } else {
        // throw if high voltage on
        int highVoltage = 0;
        if (getHighVoltage(&highVoltage) == FAIL) {
            sprintf(mess, "Could not get high voltage status to do a safety "
                          "check first\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
        if (highVoltage > 0) {
            sprintf(mess, "High voltage is on. Turn off high voltage first\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("Powering chip: off\n"));
        bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PWR_CHIP_MSK);
        chipConfigured = 0;
    }
    return OK;
}

int getPowerChip() {
    return ((bus_r(CONTROL_REG) & CONTROL_PWR_CHIP_MSK) >>
            CONTROL_PWR_CHIP_OFST);
}

int isChipConfigured() { return chipConfigured; }

int configureChip(char *mess) {

    if (!startupPowerChipConfigDone) {
        LOG(logINFOBLUE,
            ("Startup: Chip to be configured when reading config file\n"));
        return OK;
    }
    LOG(logINFOBLUE, ("\tConfiguring chip\n"));

    // on chip dacs
    for (int idac = 0; idac != ONCHIP_NDAC; ++idac) {
        // ignore unused dacs
        if (idac == (int)G2_VCHIP_UNUSED)
            continue;
        for (int ichip = 0; ichip != NCHIP; ++ichip) {
            if (onChipdacValues[idac][ichip] == -1) {
                sprintf(mess, "On chip DAC [%d] value not set for chip %d\n",
                        idac, ichip);
                LOG(logERROR, (mess));
                return FAIL;
            }
            if (setOnChipDAC(idac, ichip, onChipdacValues[idac][ichip]) ==
                FAIL) {
                sprintf(mess, "Could not set on chip DAC for chip %d\n", ichip);
                LOG(logERROR, (mess));
                return FAIL;
            }
        }
    }

    // adc configuration
    for (int ichip = 0; ichip != NCHIP; ++ichip) {
        for (int iadc = 0; iadc != NADC; ++iadc) {
            if (setADCConfiguration(ichip, iadc,
                                    adcConfiguration[ichip][iadc]) == FAIL) {
                sprintf(mess, "Could not set ADC configuration for chip %d\n",
                        ichip);
                LOG(logERROR, (mess));
                return FAIL;
            }
        }
    }

    // veto reference
    for (int ichip = 0; ichip != NCHIP; ++ichip) {
        if (configureASICVetoReference(ichip, vetoGainIndices[ichip],
                                       vetoReference[ichip]) == FAIL) {
            sprintf(mess, "Could not configure veto reference for chip %d\n",
                    ichip);
            LOG(logERROR, (mess));
            return FAIL;
        }
    }

    // asic global settings (burst mode, cds gain, filter resistor)
    if (configureASICGlobalSettings() == FAIL) {
        sprintf(mess, "Could not configure asic global settings\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    LOG(logINFOBLUE, ("\tChip configured\n"));
    chipConfigured = 1;
    return OK;
}

void setDBITPipeline(int val) {
    if (val < 0) {
        return;
    }
    LOG(logINFO, ("Setting dbit pipeline to %d\n", val));
    uint32_t addr = ADIF_CONFIG_REG;
    bus_w(addr, bus_r(addr) & ~ADIF_CONFIG_DBIT_PIPELINE_MSK);
    bus_w(addr, bus_r(addr) | ((val << ADIF_CONFIG_DBIT_PIPELINE_OFST) &
                               ADIF_CONFIG_DBIT_PIPELINE_MSK));
}

int getDBITPipeline() {
    return ((bus_r(ADIF_CONFIG_REG) & ADIF_CONFIG_DBIT_PIPELINE_MSK) >>
            ADIF_CONFIG_DBIT_PIPELINE_OFST);
}

int setPhase(enum CLKINDEX ind, int val, int degrees) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to set phase\n", ind));
        return FAIL;
    }
    char *clock_names[] = {CLK_NAMES};
    LOG(logINFOBLUE,
        ("Setting %s clock (%d) phase to %d %s\n", clock_names[ind], ind, val,
         degrees == 0 ? "" : "degrees"));
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
    LOG(logDEBUG1, ("\tphase shift: %d (degrees/shift: %d)\n", valShift, val));

    int relativePhase = valShift - clkPhase[ind];
    LOG(logDEBUG1, ("\trelative phase shift: %d (Current phase: %d)\n",
                    relativePhase, clkPhase[ind]));

    // same phase
    if (!relativePhase) {
        LOG(logINFO, ("\tNothing to do in Phase Shift\n"));
        return OK;
    }

    int direction = 1;
    if (relativePhase < 0) {
        relativePhase *= -1;
        direction = 0;
    }
    int pllIndex = (int)(ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL);
    int clkIndex = (int)(ind >= SYSTEM_C0 ? ind - SYSTEM_C0 : ind);
    ALTERA_PLL_C10_SetPhaseShift(pllIndex, clkIndex, relativePhase, direction);

    clkPhase[ind] = valShift;
    return OK;
}

int getPhase(enum CLKINDEX ind, int degrees) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
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
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get max phase\n", ind));
        return -1;
    }
    int maxshiftstep = ALTERA_PLL_C10_GetMaxPhaseShiftStepsofVCO();
    int ret = clkDivider[ind] * maxshiftstep;

    char *clock_names[] = {CLK_NAMES};
    LOG(logDEBUG1, ("\tMax Phase Shift (%s): %d (Clock Div: %d)\n",
                    clock_names[ind], ret, clkDivider[ind]));

    return ret;
}

int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR,
            ("Unknown clock index %d to validate phase in degrees\n", ind));
        return FAIL;
    }
    if (val == -1) {
        return OK;
    }
    LOG(logDEBUG1, ("validating phase in degrees for clk %d\n", (int)ind));
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

int getFrequency(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get frequency\n", ind));
        return -1;
    }
    return (((double)getVCOFrequency(ind) / (double)clkDivider[ind]) + 0.5);
}

int getVCOFrequency(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get vco frequency\n", ind));
        return -1;
    }
    int pllIndex = (int)(ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL);
    return ALTERA_PLL_C10_GetVCOFrequency(pllIndex);
}

int setReadoutSpeed(int val) {
    switch (val) {
    case G2_108MHZ:
        LOG(logINFOBLUE, ("Setting readout speed to 108 MHz\n"));
        if (setClockDivider(READOUT_C0, SPEED_108_CLKDIV_0) == FAIL) {
            LOG(logERROR, ("Could not set readout speed to 108 MHz. Failed to "
                           "set readout clk 0 to %d\n",
                           SPEED_108_CLKDIV_0));
            return FAIL;
        }
        if (setClockDivider(READOUT_C1, SPEED_108_CLKDIV_1) == FAIL) {
            LOG(logERROR, ("Could not set readout speed to 108 MHz. Failed to "
                           "set readout clk 1 to %d\n",
                           SPEED_108_CLKDIV_1));
            return FAIL;
        }
        if (setPhase(READOUT_C1, SPEED_108_CLKPHASE_DEG_1, 1) == FAIL) {
            LOG(logERROR, ("Could not set readout speed to 108 MHz. Failed to "
                           "set clk phase 1 %d deg\n",
                           SPEED_108_CLKPHASE_DEG_1));
            return FAIL;
        }
        setDBITPipeline(SPEED_144_DBIT_PIPELINE);
        if (getDBITPipeline() != SPEED_144_DBIT_PIPELINE) {
            LOG(logERROR, ("Could not set readout speed to 108 MHz. Failed to "
                           "set dbitpipeline to %d \n",
                           SPEED_144_DBIT_PIPELINE));
            return FAIL;
        }
        break;
    case G2_144MHZ:
        LOG(logINFOBLUE, ("Setting readout speed to 144 MHz\n"));
        if (setClockDivider(READOUT_C0, SPEED_144_CLKDIV_0) == FAIL) {
            LOG(logERROR, ("Could not set readout speed to 144 MHz. Failed to "
                           "set readout clk 0 to %d\n",
                           SPEED_144_CLKDIV_0));
            return FAIL;
        }
        if (setClockDivider(READOUT_C1, SPEED_144_CLKDIV_1) == FAIL) {
            LOG(logERROR, ("Could not set readout speed to 144 MHz. Failed to "
                           "set readout clk 1 to %d\n",
                           SPEED_144_CLKDIV_1));
            return FAIL;
        }
        if (setPhase(READOUT_C1, SPEED_144_CLKPHASE_DEG_1, 1) == FAIL) {
            LOG(logERROR, ("Could not set readout speed to 144 MHz. Failed to "
                           "set clk phase 1 %d deg\n",
                           SPEED_144_CLKPHASE_DEG_1));
            return FAIL;
        }
        setDBITPipeline(SPEED_144_DBIT_PIPELINE);
        if (getDBITPipeline() != SPEED_144_DBIT_PIPELINE) {
            LOG(logERROR, ("Could not set readout speed to 144 MHz. Failed to "
                           "set dbitpipeline to %d \n",
                           SPEED_144_DBIT_PIPELINE));
            return FAIL;
        }
        break;
    default:
        LOG(logERROR, ("Unknown readout speed %d\n", val));
        return FAIL;
    }
    return OK;
}

int getReadoutSpeed(int *retval) {
    // TODO ASIC and ADIFreg need to check????
    // clkdiv 2, 3, 4, 5?
    if (clkDivider[READOUT_C0] == SPEED_108_CLKDIV_0 &&
        clkDivider[READOUT_C1] == SPEED_108_CLKDIV_1 &&
        getPhase(READOUT_C1, 1) == SPEED_108_CLKPHASE_DEG_1) {
        *retval = G2_108MHZ;
    }

    else if (clkDivider[READOUT_C0] == SPEED_144_CLKDIV_0 &&
             clkDivider[READOUT_C1] == SPEED_144_CLKDIV_1 &&
             getPhase(READOUT_C1, 1) == SPEED_144_CLKPHASE_DEG_1) {
        *retval = G2_144MHZ;
    }

    else {
        *retval = -1;
        return FAIL;
    }
    return OK;
}

int getMaxClockDivider() { return ALTERA_PLL_C10_GetMaxClockDivider(); }

int setClockDivider(enum CLKINDEX ind, int val) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to set clock divider\n", ind));
        return FAIL;
    }
    if (val < 2 || val > getMaxClockDivider()) {
        return FAIL;
    }
    char *clock_names[] = {CLK_NAMES};

    LOG(logINFOBLUE, ("Setting %s clock (%d) divider from %d to %d\n",
                      clock_names[ind], ind, clkDivider[ind], val));

    // Remembering old phases in degrees
    int oldPhases[NUM_CLOCKS];
    {
        for (int i = 0; i < NUM_CLOCKS; ++i) {
            oldPhases[i] = getPhase(i, 1);
            LOG(logDEBUG1, ("\tRemembering %s clock (%d) phase: %d degrees\n",
                            clock_names[ind], ind, oldPhases[i]));
        }
    }

    // Calculate and set output frequency
    int pllIndex = (int)(ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL);
    int clkIndex = (int)(ind >= SYSTEM_C0 ? ind - SYSTEM_C0 : ind);
    ALTERA_PLL_C10_SetOuputClockDivider(pllIndex, clkIndex, val);
    clkDivider[ind] = val;
    LOG(logINFO, ("\t%s clock (%d) divider set to %d\n", clock_names[ind], ind,
                  clkDivider[ind]));

    // phase is reset by pll (when setting output frequency)
    if (ind < SYSTEM_C0) {
        clkPhase[READOUT_C0] = 0;
        clkPhase[READOUT_C1] = 0;
    } else {
        clkPhase[SYSTEM_C0] = 0;
        clkPhase[SYSTEM_C1] = 0;
        clkPhase[SYSTEM_C2] = 0;
        clkPhase[SYSTEM_C3] = 0;
    }

    // set the phase in degrees (reset by pll)
    for (int i = 0; i < NUM_CLOCKS; ++i) {
        int currPhaseDeg = getPhase(i, 1);
        if (oldPhases[i] != currPhaseDeg) {
            LOG(logINFO,
                ("\tCorrecting %s clock (%d) phase from %d to %d degrees\n",
                 clock_names[i], i, currPhaseDeg, oldPhases[i]));
            setPhase(i, oldPhases[i], 1);
        }
    }

    // update system frequency and time settings that depend on it
    if (ind == SYSTEM_C0) {
        LOG(logINFO, ("\tUpdating time settings (sys freq change)\n"));
        int64_t exptime = getExpTime();
        int64_t period = getPeriod();
        int64_t delayAfterTrigger = getDelayAfterTrigger();
        int64_t burstPeriod = getBurstPeriod();

        systemFrequency = ((double)getVCOFrequency(SYSTEM_C0) /
                           (double)clkDivider[SYSTEM_C0]);

        setExpTime(exptime);
        setPeriod(period);
        setDelayAfterTrigger(delayAfterTrigger);
        setBurstPeriod(burstPeriod);
        LOG(logINFO, ("\tDone updating time settings\n"));
    }
    return OK;
}

int getClockDivider(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get clock divider\n", ind));
        return -1;
    }
    return clkDivider[ind];
}

int setInjectChannel(int offset, int increment) {
    if (offset < 0 || increment < 1) {
        LOG(logERROR,
            ("Cannot inject channel. Invalid offset %d or increment %d\n",
             offset, increment));
        return FAIL;
    }

    LOG(logINFO,
        ("Injecting channels [offset:%d, increment:%d]\n", offset, increment));

    // 4 bits of padding + 128 bits + 4 bits for address = 136 bits
    char buffer[17];
    memset(buffer, 0, sizeof(buffer));
    int startCh = 4; // 4 due to padding
    // reversing the channels sent (offset 0 is 127, 50 is 77 etc..)
    for (int ich = startCh + NCHAN - 1 - offset; ich >= startCh;
         ich -= increment) {
        int byteIndex = ich / 8;
        int bitIndex = ich % 8;
        buffer[byteIndex] |= (1 << (8 - 1 - bitIndex));
    }

    // address at the end
    buffer[16] |= (ASIC_CURRENT_INJECT_ADDR);

    int chipIndex = -1; // for all chips
    if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
        return FAIL;
    }

    injectedChannelsOffset = offset;
    injectedChannelsIncrement = increment;
    return OK;
}

void getInjectedChannels(int *offset, int *increment) {
    *offset = injectedChannelsOffset;
    *increment = injectedChannelsIncrement;
}

int setVetoReference(int gainIndex, int value) {
    LOG(logINFO, ("Setting veto reference [chip:-1, G%d, value:%d]\n",
                  gainIndex, value));
    int values[NCHAN];
    int gainIndices[NCHAN];
    for (int ich = 0; ich < NCHAN; ++ich) {
        values[ich] = value;
        gainIndices[ich] = gainIndex;
    }
    return configureASICVetoReference(-1, gainIndices, values);
}

int setVetoPhoton(int chipIndex, int *gainIndices, int *values) {
    return configureASICVetoReference(chipIndex, gainIndices, values);
}

int configureASICVetoReference(int chipIndex, int *gainIndices, int *values) {
    LOG(logINFO, ("Setting veto photon/file/reference [chip:%d]\n", chipIndex));

    // reversing the values sent to the chip
    int revValues[NCHAN] = {};
    for (int i = 0; i < NCHAN; ++i) {
        revValues[i] = values[NCHAN - 1 - i];
    }

    // correct gain bits and integrate into revValues
    for (int i = 0; i < NCHAN; ++i) {
        int gainValue = 0;
        switch (gainIndices[i]) {
        case 0:
            gainValue = ASIC_G0_VAL;
            break;
        case 1:
            gainValue = ASIC_G1_VAL;
            break;
        case 2:
            gainValue = ASIC_G2_VAL;
            break;
        default:
            LOG(logERROR,
                ("Unknown gain index %d for channel %d\n", gainIndices[i], i));
            return FAIL;
        }
        revValues[NCHAN - 1 - i] |=
            gainValue; // reversed list, so NCHAN - 1 - i
        LOG(logDEBUG2, ("Values[%d]: 0x%x\n", i, revValues[i]));
    }

    const int lenDataBitsPerchannel = ASIC_GAIN_MAX_BITS + ADU_MAX_BITS; // 14
    const int lenBits = lenDataBitsPerchannel * NCHAN;                   // 1792
    const int padding = 4; // due to address (4) to make it byte aligned
    const int lenTotalBits = padding + lenBits + ASIC_ADDR_MAX_BITS; // 1800
    const int len = lenTotalBits / 8;                                // 225

    // assign each bit into 4 + 1792  into byte array
    uint8_t commandBytes[lenTotalBits];
    memset(commandBytes, 0, sizeof(commandBytes));
    int offset = padding; // bit offset for commandbytes
    for (int ich = 0; ich < NCHAN; ++ich) {
        // loop through all bits in a value
        for (int iBit = 0; iBit < lenDataBitsPerchannel; ++iBit) {
            commandBytes[offset++] =
                ((revValues[ich] >> (lenDataBitsPerchannel - 1 - iBit)) & 0x1);
        }
    }

    // create command for 4 padding  + 1792 bits + 4 bits address = 1800
    // bits = 225 bytes
    char buffer[len];
    memset(buffer, 0, len);
    offset = 0;
    // loop through buffer elements
    for (int ich = 0; ich < len; ++ich) {
        // loop through each bit in buffer element
        for (int iBit = 0; iBit < 8; ++iBit) {
            buffer[ich] |= (commandBytes[offset++] << (8 - 1 - iBit));
        }
    }

    // address at the end
    buffer[len - 1] |= (ASIC_VETO_REF_ADDR);

    if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
        return FAIL;
    }

    // all chips (saving unreversed values)
    if (chipIndex == -1) {
        for (int ichan = 0; ichan < NCHAN; ++ichan) {
            for (int ichip = 0; ichip < NCHIP; ++ichip) {
                vetoReference[ichip][ichan] = values[ichan];
                vetoGainIndices[ichip][ichan] = gainIndices[ichan];
            }
        }
    }

    // specific chip
    else {
        for (int ichan = 0; ichan < NCHAN; ++ichan) {
            vetoReference[chipIndex][ichan] = values[ichan];
            vetoGainIndices[chipIndex][ichan] = gainIndices[ichan];
        }
    }
    return OK;
}

int getVetoPhoton(int chipIndex, int *retvals, int *gainRetvals) {
    if (chipIndex == -1) {
        // if chipindex is -1, check that all values and gain indices are
        // same
        for (int i = 0; i < NCHAN; ++i) {
            int val = vetoReference[0][i];
            int gval = vetoGainIndices[0][i];
            for (int j = 1; j < NCHIP; ++j) {
                if (vetoReference[j][i] != val) {
                    LOG(logERROR,
                        ("Get vet photon fail for chipIndex:%d. Different "
                         "values between [nchip:%d, nchan:%d, value:%d] "
                         "and "
                         "[nchip:0, nchan:%d, value:%d]\n",
                         chipIndex, j, i, vetoReference[j][i], i, val));
                    return FAIL;
                }
                if (vetoGainIndices[j][i] != gval) {
                    LOG(logERROR,
                        ("Get vet photon fail for chipIndex:%d. Different "
                         "gain indices between [nchip:%d, nchan:%d, gain "
                         "index:%d] and [nchip:0, nchan:%d, gain "
                         "index:%d]\n",
                         chipIndex, j, i, vetoGainIndices[j][i], i, gval));
                    return FAIL;
                }
            }
        }
        chipIndex = 0;
    }
    memcpy((char *)retvals,
           ((char *)vetoReference) + NCHAN * chipIndex * sizeof(int),
           sizeof(int) * NCHAN);
    memcpy((char *)gainRetvals,
           ((char *)vetoGainIndices) + NCHAN * chipIndex * sizeof(int),
           sizeof(int) * NCHAN);
    return OK;
}

int setADCConfiguration(int chipIndex, int adcIndex, int value) {
    LOG(logINFO, ("Configuring ADC [chipIndex:%d, adcIndex:%d, value:0x%x]\n",
                  chipIndex, adcIndex, value));

    // validations
    if (chipIndex < -1 || chipIndex >= NCHIP) {
        LOG(logERROR, ("Invalid chip index %d\n", chipIndex));
        return FAIL;
    }
    if (adcIndex < -1 || adcIndex >= NADC) {
        LOG(logERROR, ("Invalid adc index %d\n", adcIndex));
        return FAIL;
    }
    // validations
    if (value < 0 || value > ASIC_ADC_MAX_VAL) {
        LOG(logERROR, ("Invalid value 0x%x\n", value));
        return FAIL;
    }
    int chipmin = 0;
    int chipmax = NCHIP;
    int adcmin = 0;
    int adcmax = NADC;
    // specific chip
    if (chipIndex != -1) {
        chipmin = chipIndex;
        chipmax = chipIndex + 1;
    }
    // specific adc (reversing adc when sending to chip)
    if (adcIndex != -1) {
        adcmin = NADC - 1 - adcIndex;
        adcmax = NADC - adcIndex;
    }
    // update values
    for (int i = chipmin; i < chipmax; ++i) {
        for (int j = adcmin; j < adcmax; ++j) {
            adcConfiguration[i][j] = (uint8_t)value;
            LOG(logDEBUG1,
                ("Value [%d][%d]: 0x%02hhx\n", i, j, adcConfiguration[i][j]));
        }
    }
    // single chip configuration
    int ind = chipIndex;
    // all chips, take the first one as all equal
    if (ind == -1) {
        ind = 0;
    }

    const int lenDataBitsPerADC = ASIC_ADC_MAX_BITS; // 7
    const int lenBits = lenDataBitsPerADC * NADC;    // 224
    const int padding = 4; // due to address (4) to make it byte aligned
    const int lenTotalBits = padding + lenBits + ASIC_ADDR_MAX_BITS; // 232
    const int len = lenTotalBits / 8;                                // 29

    // assign each bit into 4 + 224  into byte array
    uint8_t commandBytes[lenTotalBits];
    memset(commandBytes, 0, sizeof(commandBytes));
    int offset = padding; // bit offset for commandbytes
    for (int ich = 0; ich < NADC; ++ich) {
        // loop through all bits in a value
        for (int iBit = 0; iBit < lenDataBitsPerADC; ++iBit) {
            commandBytes[offset++] = ((adcConfiguration[ind][ich] >>
                                       (lenDataBitsPerADC - 1 - iBit)) &
                                      0x1);
        }
    }

    // create command for 4 padding + 224 bits + 4 bits address = 232 bits =
    // 29 bytes
    char buffer[len];
    memset(buffer, 0, len);
    offset = 0;
    // loop through buffer elements
    for (int ich = 0; ich < len; ++ich) {
        // loop through each bit in buffer element
        for (int iBit = 0; iBit < 8; ++iBit) {
            buffer[ich] |= (commandBytes[offset++] << (8 - 1 - iBit));
        }
    }

    // address at the end
    buffer[len - 1] |= (ASIC_CONF_ADC_ADDR);

    if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
        return FAIL;
    }

    return OK;
}

int getADCConfiguration(int chipIndex, int adcIndex) {
    // already validated at tcp interface
    if (chipIndex < -1 || chipIndex >= NCHIP) {
        LOG(logERROR, ("Invalid chip index %d\n", chipIndex));
        return -1;
    }
    if (adcIndex < -1 || adcIndex >= NADC) {
        LOG(logERROR, ("Invalid adc index %d\n", adcIndex));
        return -1;
    }
    int chipmin = 0;
    int chipmax = NCHIP;
    int adcmin = 0;
    int adcmax = NADC;
    // specific chip
    if (chipIndex != -1) {
        chipmin = chipIndex;
        chipmax = chipIndex + 1;
    }
    // specific adc (reversing adc when sending to chip)
    if (adcIndex != -1) {
        adcmin = NADC - 1 - adcIndex;
        adcmax = NADC - adcIndex;
    }
    int val = adcConfiguration[chipmin][adcmin];

    // ensure same values for chip and adc in question
    for (int i = chipmin; i < chipmax; ++i) {
        for (int j = adcmin; j < adcmax; ++j) {
            if (adcConfiguration[i][j] != val) {
                LOG(logINFO,
                    ("\tADC configuration 0x%x at [%d][%d] differs from "
                     "0x%x "
                     "at "
                     "[%d][%d], returning -1\n",
                     adcConfiguration[i][j], i, j, val, chipmin, adcmin));
                return -1;
            }
        }
    }
    return val;
}

int setBurstModeinFPGA(enum burstMode value) {
    uint32_t addr = ASIC_CONFIG_REG;
    uint32_t runmode = 0;
    switch (value) {
    case BURST_INTERNAL:
        runmode = ASIC_CONFIG_RUN_MODE_INT_BURST_VAL;
        break;
    case BURST_EXTERNAL:
        runmode = ASIC_CONFIG_RUN_MODE_EXT_BURST_VAL;
        break;
    case CONTINUOUS_INTERNAL:
        runmode = ASIC_CONFIG_RUN_MODE_INT_CONT_VAL;
        break;
    case CONTINUOUS_EXTERNAL:
        runmode = ASIC_CONFIG_RUN_MODE_EXT_CONT_VAL;
        break;
    default:
        LOG(logERROR, ("Unknown burst mode %d\n", value));
        return FAIL;
    }
    LOG(logDEBUG1, ("Run mode (FPGA val): %d\n", runmode));
    bus_w(addr, bus_r(addr) & ~ASIC_CONFIG_RUN_MODE_MSK);
    bus_w(addr, bus_r(addr) | ((runmode << ASIC_CONFIG_RUN_MODE_OFST) &
                               ASIC_CONFIG_RUN_MODE_MSK));
    burstMode = value;
    return OK;
}

int setBurstMode(enum burstMode burst) {
    LOG(logINFO,
        ("Setting burst mode to %s\n",
         (burst == BURST_INTERNAL
              ? "burst_internal"
              : (burst == BURST_EXTERNAL ? "burst external"
                                         : (burst == CONTINUOUS_INTERNAL
                                                ? "continuous internal"
                                                : "continuous external")))));

    if (setBurstModeinFPGA(burst) == FAIL) {
        return FAIL;
    }

    updatingRegisters();

    updateClockDivs();

    return configureASICGlobalSettings();
}

int configureASICGlobalSettings() {
    int value = ((filterResistor << ASIC_FILTER_OFST) & ASIC_FILTER_MSK) |
                ((cdsGain << ASIC_CDS_GAIN_OFST) & ASIC_CDS_GAIN_MSK);
    switch (burstMode) {
    case BURST_INTERNAL:
        break;
    case BURST_EXTERNAL:
        value |= ASIC_EXT_TIMING_MSK;
        break;
    case CONTINUOUS_INTERNAL:
        value |= ASIC_CONT_MODE_MSK;
        break;
    case CONTINUOUS_EXTERNAL:
        value |= (ASIC_CONT_MODE_MSK | ASIC_EXT_TIMING_MSK);
        break;
    }
    LOG(logINFO, ("\tSending Global Chip settings:0x%x (filterResistor:%d, "
                  "cdsgain:%d)\n",
                  value, filterResistor, cdsGain));

    const int padding = 6; // due to address (4) to make it byte aligned
    const int lenTotalBits = padding + ASIC_GLOBAL_SETT_MAX_BITS +
                             ASIC_ADDR_MAX_BITS; // 4 + 6 + 4 = 16
    const int len = lenTotalBits / 8;            // 2

    // assign each bit into 4 + 224  into byte array
    uint8_t commandBytes[lenTotalBits];
    memset(commandBytes, 0, sizeof(commandBytes));
    int offset = padding; // bit offset for commandbytes
    // loop through all bits in a value
    for (int iBit = 0; iBit < ASIC_GLOBAL_SETT_MAX_BITS; ++iBit) {
        commandBytes[offset++] =
            ((value >> (ASIC_GLOBAL_SETT_MAX_BITS - 1 - iBit)) & 0x1);
    }

    // create command for 4 padding + 224 bits + 4 bits address = 232 bits =
    // 29 bytes
    char buffer[len];
    memset(buffer, 0, len);
    offset = 0;
    // loop through buffer elements
    for (int ich = 0; ich < len; ++ich) {
        // loop through each bit in buffer element
        for (int iBit = 0; iBit < 8; ++iBit) {
            buffer[ich] |= (commandBytes[offset++] << (8 - 1 - iBit));
        }
    }

    // address at the end
    buffer[len - 1] |= (ASIC_CONF_GLOBAL_SETT);

    int chipIndex = -1;
    if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
        return FAIL;
    }

    return OK;
}

enum burstMode getBurstMode() {
    uint32_t addr = ASIC_CONFIG_REG;
    int runmode = bus_r(addr) & ASIC_CONFIG_RUN_MODE_MSK;
    switch (runmode) {
    case ASIC_CONFIG_RUN_MODE_INT_BURST_VAL:
        burstMode = BURST_INTERNAL;
        break;
    case ASIC_CONFIG_RUN_MODE_EXT_BURST_VAL:
        burstMode = BURST_EXTERNAL;
        break;
    case ASIC_CONFIG_RUN_MODE_INT_CONT_VAL:
        burstMode = CONTINUOUS_INTERNAL;
        break;
    case ASIC_CONFIG_RUN_MODE_EXT_CONT_VAL:
        burstMode = CONTINUOUS_EXTERNAL;
        break;
    default:
        LOG(logERROR, ("Unknown run mode read from FPGA %d\n", runmode));
        return -1;
    }
    return burstMode;
}

int setCDSGain(int enable) {
    if (enable >= 0) {
        cdsGain = (enable == 0 ? 0 : 1);
        LOG(logINFO,
            ("%s CDS Gain\n", (cdsGain == 0 ? "Disabling" : "Enabling")));
        return configureASICGlobalSettings();
    }
    return FAIL;
}

int getCDSGain() { return cdsGain; }

int setFilterResistor(int value) {
    if (value < 0 || value > ASIC_FILTER_MAX_RES_VALUE) {
        LOG(logERROR, ("Invalid filter resistor value %d\n", value));
        return FAIL;
    }
    filterResistor = value;
    LOG(logINFO, ("Setting Filter Resistor to %d\n", filterResistor));
    return configureASICGlobalSettings();
}

int getFilterResistor() { return filterResistor; }

void setCurrentSource(int value) {
    uint32_t addr = ASIC_CONFIG_REG;
    if (value > 0) {
        bus_w(addr, (bus_r(addr) | ASIC_CONFIG_CURRENT_SRC_EN_MSK));
    } else if (value == 0) {
        bus_w(addr, (bus_r(addr) & ~ASIC_CONFIG_CURRENT_SRC_EN_MSK));
    }
}

int getCurrentSource() {
    return ((bus_r(ASIC_CONFIG_REG) & ASIC_CONFIG_CURRENT_SRC_EN_MSK) >>
            ASIC_CONFIG_CURRENT_SRC_EN_OFST);
}

void setTimingSource(enum timingSourceType value) {
    uint32_t addr = CONTROL_REG;
    switch (value) {
    case TIMING_INTERNAL:
        LOG(logINFO, ("Setting timing source to internal\n"));
        bus_w(addr, (bus_r(addr) & ~CONTROL_TIMING_SOURCE_EXT_MSK));
        break;
    case TIMING_EXTERNAL:
        LOG(logINFO, ("Setting timing source to exernal\n"));
        bus_w(addr, (bus_r(addr) | CONTROL_TIMING_SOURCE_EXT_MSK));
        break;
    default:
        LOG(logERROR, ("Unknown timing source %d\n", value));
        break;
    }
}

enum timingSourceType getTimingSource() {
    if (bus_r(CONTROL_REG) & CONTROL_TIMING_SOURCE_EXT_MSK) {
        return TIMING_EXTERNAL;
    }
    return TIMING_INTERNAL;
}

void setVeto(int enable) {
    if (enable >= 0) {
        uint32_t addr = CONFIG_REG;

        if (enable) {
            LOG(logINFOBLUE, ("Enabling veto streaming\n"));
            bus_w(addr, bus_r(addr) | CONFIG_VETO_ENBL_MSK);
        } else {
            LOG(logINFOBLUE, ("Disabling veto streaming\n"));
            bus_w(addr, bus_r(addr) & ~CONFIG_VETO_ENBL_MSK);
        }
        LOG(logDEBUG, ("config reg:0x%x\n", bus_r(addr)));
    }
}

int getVeto() {
    LOG(logDEBUG, ("config reg:0x%x\n", bus_r(CONFIG_REG)));
    return ((bus_r(CONFIG_REG) & CONFIG_VETO_ENBL_MSK) >>
            CONFIG_VETO_ENBL_OFST);
}

void setVetoStream(int value) {
    uint32_t addr = CONFIG_REG;

    if (value) {
        LOG(logINFOBLUE, ("Enabling lll veto streaming\n"));
        bus_w(addr, bus_r(addr) | CONFIG_VETO_CH_LLL_ENBL_MSK);
    } else {
        LOG(logINFOBLUE, ("Disabling lll veto streaming\n"));
        bus_w(addr, bus_r(addr) & ~CONFIG_VETO_CH_LLL_ENBL_MSK);
    }
    LOG(logDEBUG, ("config reg:0x%x\n", bus_r(addr)));
}

int getVetoStream() {
    return ((bus_r(CONFIG_REG) & CONFIG_VETO_CH_LLL_ENBL_MSK) ? 1 : 0);
}

enum vetoAlgorithm getVetoAlgorithm(enum streamingInterface interface) {
    int retval = 0;
    if (interface == LOW_LATENCY_LINK) {
        retval = ((bus_r(CONFIG_REG) & CONFIG_VETO_CH_LLL_ALG_MSK) >>
                  CONFIG_VETO_CH_LLL_ALG_OFST);
    } else {
        retval = ((bus_r(CONFIG_REG) & CONFIG_VETO_CH_10GBE_ALG_MSK) >>
                  CONFIG_VETO_CH_10GBE_ALG_OFST);
    }
    switch (retval) {
    case ALGORITHM_HITS_VAL:
        return ALG_HITS;
    case ALGORITHM_RAW_VAL:
        return ALG_RAW;
    default:
        LOG(logERROR, ("unknown algorithm %d\n", retval));
        return -1;
    }
}

void setVetoAlgorithm(enum vetoAlgorithm alg,
                      enum streamingInterface interface) {
    uint32_t addr = CONFIG_REG;
    uint32_t value = bus_r(addr);
    switch (alg) {
        // more to follow
    case ALG_HITS:
        if (interface == LOW_LATENCY_LINK) {
            LOG(logINFO, ("Setting veto algorithm [lll]: hits\n"));
            value &= (~CONFIG_VETO_CH_LLL_ALG_MSK);
            value |= ((ALGORITHM_HITS_VAL << CONFIG_VETO_CH_LLL_ALG_OFST) &
                      CONFIG_VETO_CH_LLL_ALG_MSK);
        } else {
            LOG(logINFO, ("Setting veto algorithm [10Gbe]: hits\n"));
            value &= (~CONFIG_VETO_CH_10GBE_ALG_MSK);
            value |= ((ALGORITHM_HITS_VAL << CONFIG_VETO_CH_10GBE_ALG_OFST) &
                      CONFIG_VETO_CH_10GBE_ALG_MSK);
        }
        break;
    case ALG_RAW:
        if (interface == LOW_LATENCY_LINK) {
            LOG(logINFO, ("Setting veto algorithm [lll]: raw\n"));
            value &= (~CONFIG_VETO_CH_LLL_ALG_MSK);
            value |= ((ALGORITHM_RAW_VAL << CONFIG_VETO_CH_LLL_ALG_OFST) &
                      CONFIG_VETO_CH_LLL_ALG_MSK);
        } else {
            LOG(logINFO, ("Setting veto algorithm [10Gbe]: raw\n"));
            value &= (~CONFIG_VETO_CH_10GBE_ALG_MSK);
            value |= ((ALGORITHM_RAW_VAL << CONFIG_VETO_CH_10GBE_ALG_OFST) &
                      CONFIG_VETO_CH_10GBE_ALG_MSK);
        }
        break;
    default:
        LOG(logERROR, ("unknown algorithm %d for lll\n", alg));
        return;
    }
    bus_w(addr, value);
}

int setBadChannels(int numChannels, int *channelList) {
    LOG(logINFO, ("Setting %d bad channels\n", numChannels));

    int numAddr = MASK_STRIP_NUM_REGS;
    int startAddr = MASK_STRIP_START_REG;

    // resetting all mask registers first
    for (int iaddr = 0; iaddr < numAddr; ++iaddr) {
        uint32_t addr = startAddr + iaddr * REG_OFFSET;
        bus_w(addr, 0);
    }

    // setting badchannels, loop through list
    for (int i = 0; i != numChannels; ++i) {
        LOG(logINFO, ("\t[%d]: %d\n", i, channelList[i]));
        int iaddr = channelList[i] / 32;
        int iBit = channelList[i] % 32;
        uint32_t addr = startAddr + iaddr * REG_OFFSET;
        LOG(logDEBUG1,
            ("val:%d iaddr:%d iBit:%d, addr:0x%x old:0x%x val:0x%x\n",
             channelList[i], iaddr, iBit, addr, bus_r(addr), (1 << iBit)));
        bus_w(addr, bus_r(addr) | (1 << iBit));
    }
    return OK;
}

int *getBadChannels(int *numChannels) {
    int *retvals = NULL;
    // count number of bad channels
    *numChannels = 0;
    for (int i = 0; i != MASK_STRIP_NUM_REGS; ++i) {
        uint32_t addr = MASK_STRIP_START_REG + i * REG_OFFSET;
        *numChannels += __builtin_popcount(bus_r(addr));
    }
    if (*numChannels > 0) {
        // get list of bad channels
        retvals = malloc(*numChannels * sizeof(int));
        if (retvals == NULL) {
            LOG(logERROR, ("Could not allocate memory to get bad channels\n"));
            *numChannels = -1;
            return NULL;
        }
        memset(retvals, 0, *numChannels * sizeof(int));
        int chIndex = 0;
        int numAddr = MASK_STRIP_NUM_REGS;
        // loop through registers
        for (int iaddr = 0; iaddr != numAddr; ++iaddr) {
            // calculate address and get value
            uint32_t addr = MASK_STRIP_START_REG + iaddr * REG_OFFSET;
            uint32_t val = bus_r(addr);
            // loop through 32 bits
            for (int iBit = 0; iBit != 32; ++iBit) {
                // masked, add to list
                if ((val >> iBit) & 0x1) {
                    LOG(logDEBUG1, ("iaddr:%d iBit:%d val:0x%x, ch:%d\n", iaddr,
                                    iBit, val, iaddr * 32 + iBit));
                    retvals[chIndex++] = iaddr * 32 + iBit;
                }
            }
        }
    }
    // debugging
    LOG(logDEBUG1, ("Reading Bad channel list\n"));
    for (int i = 0; i != (*numChannels); ++i) {
        LOG(logDEBUG1, ("[%d]: %d\n", i, retvals[i]));
    }
    return retvals;
}

/* aquisition */

int startStateMachine() {
#ifdef VIRTUAL
    // create udp socket
    if (createUDPSocket(0) != OK) {
        return FAIL;
    }
    if (getNumberofUDPInterfaces() == 2 && createUDPSocket(1) != OK) {
        return FAIL;
    }
    LOG(logINFOBLUE, ("Starting State Machine\n"));
    // set status to running
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
    cleanFifos();

    // start state machine
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STRT_ACQSTN_MSK);

    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    return OK;
}

#ifdef VIRTUAL
void *start_timer(void *arg) {
    if (!isControlServer) {
        return NULL;
    }

    int firstDest = getFirstUDPDestination();
    int i10gbe = (getNumberofUDPInterfaces() == 2 ? 1 : 0);

    int numRepeats = getNumTriggers();
    if (getTiming() == AUTO_TIMING) {
        // continuous
        if (burstMode == CONTINUOUS_INTERNAL ||
            burstMode == CONTINUOUS_EXTERNAL) {
            numRepeats = 1;
        }
        // burst
        else {
            numRepeats = getNumBursts();
        }
    }
    int repeatPeriodNs = getBurstPeriod();
    int numFrames = getNumFrames();
    // continuous trigger mode, #frames = 1
    int64_t periodNs = getPeriod();
    if (getTiming() == TRIGGER_EXPOSURE && (burstMode == CONTINUOUS_INTERNAL ||
                                            burstMode == CONTINUOUS_EXTERNAL)) {
        numFrames = 1;
        periodNs = 0;
    }
    int64_t expUs = getExpTime() / 1000;
    int imagesize = NCHAN * NCHIP * 2;
    int datasize = imagesize;
    int packetsize = datasize + sizeof(sls_detector_header);
    int vetodatasize = VETO_DATA_SIZE;
    int vetopacketsize = vetodatasize + sizeof(veto_header);

    // Generate data
    char imageData[imagesize];
    memset(imageData, 0, imagesize);
    const int nchannels = NCHIP * NCHAN;
    int gainVal = 0;
    int channelVal = 0;
    for (int i = 0; i < nchannels; ++i) {
        if ((i % nchannels) < 400) {
            gainVal = 1;
        } else if ((i % nchannels) < 800) {
            gainVal = 2;
        } else {
            gainVal = 3;
        }
        channelVal = (i & ~GAIN_VAL_MSK) | (gainVal << GAIN_VAL_OFST);

        *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
            (uint16_t)channelVal;
        LOG(logDEBUG, ("[%d]:0x%08x\n", i, channelVal));
    }
    char vetoData[vetodatasize];
    memset(vetoData, 0, sizeof(vetodatasize));
    for (int i = 0; i < vetodatasize; i += sizeof(uint8_t)) {
        *((uint16_t *)(vetoData + i)) = i;
    }

    {
        uint64_t frameNr = 0;
        getNextFrameNumber(&frameNr);

        int iRxEntry = firstDest;
        // loop over number of repeats
        for (int repeatNr = 0; repeatNr != numRepeats; ++repeatNr) {

            struct timespec rbegin, rend;
            clock_gettime(CLOCK_REALTIME, &rbegin);

            // loop over number of frames
            for (int iframes = 0; iframes != numFrames; ++iframes) {

                // check if manual stop
                if (sharedMemory_getStop() == 1) {
                    setNextFrameNumber(frameNr + (repeatNr * numFrames) +
                                       iframes);
                    break;
                }

                // change gain and data for every frame
                {
                    const int nchannels = NCHIP * NCHAN;
                    int gainVal = 0;
                    for (int i = 0; i < nchannels; ++i) {
                        if ((i % nchannels) < 400) {
                            gainVal = 1 + iframes;
                        } else if ((i % nchannels) < 800) {
                            gainVal = 2 + iframes;
                        } else {
                            gainVal = 3 + iframes;
                        }
                        int dataVal =
                            *((uint16_t *)(imageData + i * sizeof(uint16_t)));
                        dataVal += iframes;
                        int channelVal = (dataVal & ~GAIN_VAL_MSK) |
                                         (gainVal << GAIN_VAL_OFST);
                        *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
                            (uint16_t)channelVal;
                    }
                }
                // sleep for exposure time
                struct timespec begin, end;
                clock_gettime(CLOCK_REALTIME, &begin);
                usleep(expUs);

                // first interface
                char packetData[packetsize];
                memset(packetData, 0, packetsize);
                // set header
                sls_detector_header *header =
                    (sls_detector_header *)(packetData);
                header->detType = (uint16_t)myDetectorType;
                header->version = SLS_DETECTOR_HEADER_VERSION;
                header->frameNumber =
                    frameNr + (repeatNr * numFrames) + iframes;
                header->packetNumber = 0;
                header->modId = virtual_moduleid;
                header->row = detPos[Y];
                header->column = detPos[X];
                // fill data
                memcpy(packetData + sizeof(sls_detector_header), imageData,
                       datasize);
                // send 1 packet = 1 frame
                sendUDPPacket(iRxEntry, 0, packetData, packetsize);

                // second interface (veto)
                char packetData2[vetopacketsize];
                memset(packetData2, 0, vetopacketsize);
                if (i10gbe) {
                    // set header
                    veto_header *header = (veto_header *)(packetData2);
                    header->frameNumber =
                        frameNr + (repeatNr * numFrames) + iframes;
                    header->bunchId = 0;
                    // fill data
                    memcpy(packetData2 + sizeof(veto_header), vetoData,
                           vetodatasize);
                    // send 1 packet = 1 frame
                    sendUDPPacket(iRxEntry, 1, packetData2, vetopacketsize);
                }
                LOG(logINFO,
                    ("Sent frame %s: %d (bursts/ triggers: %d) [%lld] to E%d\n",
                     (i10gbe ? "(+veto)" : ""), frameNr, repeatNr,
                     (frameNr + (repeatNr * numFrames) + iframes), iRxEntry));
                clock_gettime(CLOCK_REALTIME, &end);
                int64_t timeNs = ((end.tv_sec - begin.tv_sec) * 1E9 +
                                  (end.tv_nsec - begin.tv_nsec));

                // sleep for (period - exptime)
                if (iframes < numFrames) { // if there is a next frame
                    if (periodNs > timeNs) {
                        usleep((periodNs - timeNs) / 1000);
                    }
                }
                ++iRxEntry;
                if (iRxEntry == numUdpDestinations) {
                    iRxEntry = 0;
                }
            }

            clock_gettime(CLOCK_REALTIME, &rend);
            int64_t timeNs = ((rend.tv_sec - rbegin.tv_sec) * 1E9 +
                              (rend.tv_nsec - rbegin.tv_nsec));

            // sleep for (repeatPeriodNs - time remaining)
            if (repeatNr < numRepeats) { // if there is a next repeat
                if (repeatPeriodNs > timeNs) {
                    usleep((repeatPeriodNs - timeNs) / 1000);
                }
            }
        }
        // already being set in the start acquisition (also for real detectors)
        setNextFrameNumber(frameNr + (numRepeats * numFrames));
    }

    closeUDPSocket(0);
    if (i10gbe) {
        closeUDPSocket(1);
    }

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
    while (sharedMemory_getStatus() == RUNNING)
        usleep(500);
    sharedMemory_setStop(0);
    LOG(logINFO, ("Stopped State Machine\n"));
    return OK;
#endif
    // stop state machine
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STP_ACQSTN_MSK);
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
    uint32_t retval = bus_r(FLOW_STATUS_REG);
    LOG(logINFO, ("Status Register: %08x\n", retval));

    enum runStatus s;

    // running
    if (retval & FLOW_STATUS_RUN_BUSY_MSK) {
        if (retval & FLOW_STATUS_WAIT_FOR_TRGGR_MSK) {
            LOG(logINFOBLUE, ("Status: WAITING\n"));
            s = WAITING;
        } else {
            if (retval & FLOW_STATUS_DLY_BFRE_TRGGR_MSK) {
                LOG(logINFO, ("Status: Delay before Trigger\n"));
            } else if (retval & FLOW_STATUS_DLY_AFTR_TRGGR_MSK) {
                LOG(logINFO, ("Status: Delay after Trigger\n"));
            }
            LOG(logINFOBLUE, ("Status: RUNNING\n"));
            s = RUNNING;
        }
    }

    // not running
    else {
        // stopped or error
        if (retval & FLOW_STATUS_FIFO_FULL_MSK) {
            LOG(logINFOBLUE, ("Status: STOPPED\n")); // FIFO FULL??
            s = STOPPED;
        } else if (retval & FLOW_STATUS_CSM_BUSY_MSK) {
            LOG(logINFOBLUE, ("Status: READ MACHINE BUSY\n"));
            s = TRANSMITTING;
        } else if (!retval) {
            LOG(logINFOBLUE, ("Status: IDLE\n"));
            s = IDLE;
        } else {
            LOG(logERROR, ("Status: Unknown status %08x\n", retval));
            s = ERROR;
        }
    }

    return s;
}

void waitForAcquisitionEnd() {
    while (runBusy()) {
        usleep(500);
    }
    LOG(logINFOGREEN, ("Blocking Acquisition done\n"));
}

u_int32_t runBusy() {
#ifdef VIRTUAL
    return ((sharedMemory_getStatus() == RUNNING) ? 1 : 0);
#endif
    u_int32_t s = (bus_r(FLOW_STATUS_REG) & FLOW_STATUS_RUN_BUSY_MSK);
    // LOG(logDEBUG1, ("Status Register: %08x\n", s));
    return s;
}

/* common */

int calculateDataBytes() { return getTotalNumberOfChannels() * DYNAMIC_RANGE; }

int getTotalNumberOfChannels() {
    return (getNumberOfChannelsPerChip() * getNumberOfChips());
}
int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }
