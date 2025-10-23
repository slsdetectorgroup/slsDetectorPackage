// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "ALTERA_PLL_CYCLONE10.h"
#include "DAC6571.h"
#include "LTC2620_Driver.h"
#include "RegisterDefs.h"
#include "clogger.h"
#include "common.h"
#include "loadPattern.h"
#include "mythen3.h"
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
int64_t virtual_currentFrameNumber = 2;
int virtual_moduleid = 0;
#endif

enum detectorSettings thisSettings = UNINITIALIZED;
sls_detector_module *detectorModules = NULL;
int *detectorChans = NULL;
int *detectorDacs = NULL;
char *badChannelMask = NULL;
int defaultDacValues[NDAC] = DEFAULT_DAC_VALS;
int defaultDacValue_standard[] = SPECIAL_DEFAULT_STANDARD_DAC_VALS;
int defaultDacValue_fast[] = SPECIAL_DEFAULT_FAST_DAC_VALS;
int defaultDacValue_highgain[] = SPECIAL_DEFAULT_HIGHGAIN_DAC_VALS;

int32_t clkPhase[NUM_CLOCKS] = {};
uint32_t clkDivider[NUM_CLOCKS] = {};

enum TLogLevel trimmingPrint = logINFO;
int detPos[2] = {};
int64_t exptimeReg[NCOUNTERS] = {0, 0, 0};
int64_t gateDelayReg[NCOUNTERS] = {0, 0, 0};
int vthEnabledVals[NCOUNTERS] = {0, 0, 0};
int detID = 0;
int counterMask = 0x0;

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
    LOG(logINFOBLUE, ("************* Mythen3 Virtual Server *************\n"));
#else
    LOG(logINFOBLUE, ("***************** Mythen3 Server *****************\n"));
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
    if (type != MYTHEN3) {
        LOG(logERROR,
            ("This is not a Mythen3 firmware (read %d, expected %d)\n", type,
             MYTHEN3));
        return FAIL;
    }

    return OK;
}

int testFpga() {
    LOG(logINFO, ("Testing FPGA:\n"));
#ifdef VIRTUAL
    return OK;
#endif
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
    LOG(logINFO, ("Testing Bus:\n"));
#ifdef VIRTUAL
    return OK;
#endif

    int ret = OK;
    u_int32_t addr = DTA_OFFSET_REG;
    u_int32_t times = 1000 * 1000;

    for (u_int32_t i = 0; i < times; ++i) {
        bus_w(addr, i * 100);
        if (i * 100 != bus_r(addr)) {
            LOG(logERROR,
                ("Mismatch! Wrote 0x%x, read 0x%x\n", i * 100, bus_r(addr)));
            ret = FAIL;
        }
    }

    bus_w(addr, 0);

    if (ret == OK) {
        LOG(logINFO, ("Successfully tested bus %d times\n", times));
    }
    return ret;
}

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIMYTHEN3); }

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
                   "Stop Server: Map Fail. Cannot proceed. Check Firmware. \n");
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

int allocateDetectorStructureMemory() {
    detectorModules = malloc(sizeof(sls_detector_module));
    detectorChans = malloc(NCHAN_PER_MODULE * sizeof(int));
    detectorDacs = malloc(NDAC * sizeof(int));
    badChannelMask = malloc(NCHAN_PER_MODULE * sizeof(char));
    if (detectorModules == NULL || detectorChans == NULL ||
        detectorDacs == NULL || badChannelMask == NULL) {
        initError = FAIL;
        strcpy(initErrorMessage, "Could not allocate memory for dacs, channels "
                                 "or bad channel mask in detector\n");
        LOG(logERROR, (initErrorMessage));
        return FAIL;
    }
    LOG(logDEBUG1,
        ("modules from 0x%x to 0x%x\n", detectorModules, detectorModules));
    LOG(logDEBUG1, ("chans from 0x%x to 0x%x\n", detectorChans, detectorChans));
    LOG(logDEBUG1, ("dacs from 0x%x to 0x%x\n", detectorDacs, detectorDacs));

    (detectorModules)->dacs = detectorDacs;
    (detectorModules)->chanregs = detectorChans;
    (detectorModules)->ndac = NDAC;
    (detectorModules)->nchip = NCHIP;
    (detectorModules)->nchan = NCHAN_PER_MODULE;
    (detectorModules)->reg = UNINITIALIZED;
    (detectorModules)->iodelay = 0;
    (detectorModules)->tau = 0;
    (detectorModules)->eV[0] = 0;
    (detectorModules)->eV[1] = 0;
    (detectorModules)->eV[2] = 0;
    thisSettings = UNINITIALIZED;

    // initialize
    for (int idac = 0; idac < (detectorModules)->ndac; ++idac) {
        detectorDacs[idac] = 0;
    }
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        *((detectorModules->chanregs) + ichan) = 0;
    }
    memset(badChannelMask, 0, NCHAN_PER_MODULE * sizeof(char));
    return OK;
}

void setupDetector() {
    LOG(logINFO, ("This Server is for 1 Mythen3 module \n"));

    if (allocateDetectorStructureMemory() == FAIL)
        return;

    if (checkCommandLineConfiguration() == FAIL)
        return;

    if (updateModuleId() == FAIL)
        return;

    clkDivider[SYSTEM_C0] = DEFAULT_SYSTEM_C0;
    clkDivider[SYSTEM_C1] = DEFAULT_SYSTEM_C1;
    clkDivider[SYSTEM_C2] = DEFAULT_SYSTEM_C2;

    trimmingPrint = logINFO;
    for (int i = 0; i < NUM_CLOCKS; ++i) {
        clkPhase[i] = 0;
    }
#ifdef VIRTUAL
    if (isControlServer) {
        sharedMemory_setStatus(IDLE);
        setupUDPCommParameters();
    } else {
        sharedMemory_setStop(0);
    }
#endif

    // pll defines
    ALTERA_PLL_C10_SetDefines(
        REG_OFFSET, 0, BASE_SYSTEM_PLL, PLL_RESET_REG, 0, PLL_RESET_SYSTEM_MSK,
        SYSTEM_STATUS_REG, SYSTEM_STATUS_RDO_PLL_LCKD_MSK,
        SYSTEM_STATUS_R_PLL_LCKD_MSK, 0, SYSTEM_PLL_VCO_FREQ_HZ);
    ALTERA_PLL_C10_ResetPLL(SYSTEM_PLL);

    // hv
    DAC6571_SetDefines(HV_HARD_MAX_VOLTAGE, HV_DRIVER_FILE_NAME);
    // dac
    LTC2620_D_SetDefines(DAC_MIN_MV, DAC_MAX_MV, DAC_DRIVER_FILE_NAME, NDAC, 0,
                         "");

    resetCore();
    resetPeripheral();
    cleanFifos();

    // defaults
    initError = setHighVoltage(DEFAULT_HIGH_VOLTAGE);
    if (initError == FAIL) {
        sprintf(initErrorMessage, "Could not set high voltage to %d\n",
                DEFAULT_HIGH_VOLTAGE);
        return;
    }

    setASICDefaults();
    setADIFDefaults();

    initializePatternAddresses();

    // enable all counters before setting dacs (vthx)
    setCounterMask(MAX_COUNTER_MSK);
    resetToDefaultDacs(0);

    // set trigger flow for m3 (for all timing modes)
    bus_w(FLOW_TRIGGER_REG, bus_r(FLOW_TRIGGER_REG) | FLOW_TRIGGER_MSK);

    // dynamic range
    setDynamicRange(DEFAULT_DYNAMIC_RANGE);

    // Initialization of acquistion parameters
    setNumFrames(DEFAULT_NUM_FRAMES);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setPeriod(DEFAULT_PERIOD);
    setDelayAfterTrigger(DEFAULT_DELAY_AFTER_TRIGGER);
    setTiming(DEFAULT_TIMING_MODE);
    setNumIntGates(DEFAULT_INTERNAL_GATES);
    setNumGates(DEFAULT_EXTERNAL_GATES);
    for (int i = 0; i != NCOUNTERS; ++i) {
        setExpTime(i, DEFAULT_GATE_WIDTH);
        setGateDelay(i, DEFAULT_GATE_DELAY);
    }
    setInitialExtSignals();
    // 10G UDP
    enableTenGigabitEthernet(1);
    setSettings(DEFAULT_SETTINGS);

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

    powerChip(1);

    if (!initError) {
        setChipStatusRegister(CSR_default);
    }

    setAllTrimbits(DEFAULT_TRIMBIT_VALUE);
    setReadoutSpeed(DEFAULT_READOUT_SPEED);
}

int resetToDefaultDacs(int hardReset) {
    LOG(logINFOBLUE, ("Resetting %s to Default Dac values\n",
                      (hardReset == 1 ? "hard" : "")));

    // reset defaults to hardcoded defaults
    if (hardReset) {
        const int vals[] = DEFAULT_DAC_VALS;
        for (int i = 0; i < NDAC; ++i) {
            defaultDacValues[i] = vals[i];
        }
        const int vals_standard[] = SPECIAL_DEFAULT_STANDARD_DAC_VALS;
        for (int i = 0; i < NSPECIALDACS; ++i) {
            defaultDacValue_standard[i] = vals_standard[i];
        }
        const int vals_fast[] = SPECIAL_DEFAULT_FAST_DAC_VALS;
        for (int i = 0; i < NSPECIALDACS; ++i) {
            defaultDacValue_fast[i] = vals_fast[i];
        }
        const int vals_highgain[] = SPECIAL_DEFAULT_HIGHGAIN_DAC_VALS;
        for (int i = 0; i < NSPECIALDACS; ++i) {
            defaultDacValue_highgain[i] = vals_highgain[i];
        }
    }

    // remember settings
    enum detectorSettings oldSettings = thisSettings;

    // reset dacs to defaults
    const int specialDacs[] = SPECIALDACINDEX;
    for (int i = 0; i < NDAC; ++i) {
        int value = defaultDacValues[i];

        for (int j = 0; j < NSPECIALDACS; ++j) {
            // special dac: replace default value
            if (specialDacs[j] == i) {
                switch (oldSettings) {
                case STANDARD:
                    value = defaultDacValue_standard[j];
                    break;
                case FAST:
                    value = defaultDacValue_fast[j];
                    break;
                case HIGHGAIN:
                    value = defaultDacValue_highgain[j];
                    break;
                default:
                    break;
                }
                break;
            }
        }

        // set to default (last arg to ensure counter check)
        setDAC((enum DACINDEX)i, value, 0, 1);
        if (detectorDacs[i] != value) {
            LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n", i,
                           value, detectorDacs[i]));
            return FAIL;
        }
    }
    return OK;
}

int getDefaultDac(enum DACINDEX index, enum detectorSettings sett,
                  int *retval) {

    // settings only for special dacs
    if (sett != UNDEFINED) {
        const int specialDacs[] = SPECIALDACINDEX;
        // find special dac index
        for (int i = 0; i < NSPECIALDACS; ++i) {
            if ((int)index == specialDacs[i]) {
                switch (sett) {
                case STANDARD:
                    *retval = defaultDacValue_standard[i];
                    return OK;
                case FAST:
                    *retval = defaultDacValue_fast[i];
                    return OK;
                case HIGHGAIN:
                    *retval = defaultDacValue_highgain[i];
                    return OK;
                    // unknown settings
                default:
                    return FAIL;
                }
            }
        }
        // not a special dac
        return FAIL;
    }

    if (index < 0 || index >= NDAC)
        return FAIL;
    *retval = defaultDacValues[index];
    return OK;
}

int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value) {
    char *dac_names[] = {DAC_NAMES};

    // settings only for special dacs
    if (sett != UNDEFINED) {
        const int specialDacs[] = SPECIALDACINDEX;
        // find special dac index
        for (int i = 0; i < NSPECIALDACS; ++i) {
            if ((int)index == specialDacs[i]) {
                switch (sett) {
                case STANDARD:
                    LOG(logINFO,
                        ("Setting Default Dac [%d - %s, standard]: %d\n",
                         (int)index, dac_names[index], value));
                    defaultDacValue_standard[i] = value;
                    return OK;
                case FAST:
                    LOG(logINFO, ("Setting Default Dac [%d - %s, fast]: %d\n",
                                  (int)index, dac_names[index], value));
                    defaultDacValue_fast[i] = value;
                    return OK;
                case HIGHGAIN:
                    LOG(logINFO,
                        ("Setting Default Dac [%d - %s, highgain]: %d\n",
                         (int)index, dac_names[index], value));
                    defaultDacValue_highgain[i] = value;
                    return OK;
                    // unknown settings
                default:
                    return FAIL;
                }
            }
        }
        // not a special dac
        return FAIL;
    }

    if (index < 0 || index >= NDAC)
        return FAIL;
    LOG(logINFO, ("Setting Default Dac [%d - %s]: %d\n", (int)index,
                  dac_names[index], value));
    defaultDacValues[index] = value;
    return OK;
}

void setASICDefaults() {
    uint32_t val = bus_r(ASIC_EXP_STATUS_REG);
    val &= (~ASIC_EXP_STAT_STO_LNGTH_MSK);
    val |= ((DEFAULT_ASIC_LATCHING_NUM_PULSES << ASIC_EXP_STAT_STO_LNGTH_OFST) &
            ASIC_EXP_STAT_STO_LNGTH_MSK);
    val &= (~ASIC_EXP_STAT_RSCNTR_LNGTH_MSK);
    val |=
        ((DEFAULT_ASIC_LATCHING_NUM_PULSES << ASIC_EXP_STAT_RSCNTR_LNGTH_OFST) &
         ASIC_EXP_STAT_RSCNTR_LNGTH_MSK);
    bus_w(ASIC_EXP_STATUS_REG, val);

    val = bus_r(ASIC_RDO_CONFIG_REG);
    val &= (~ASICRDO_CNFG_RESSTRG_LNGTH_MSK);
    val |=
        ((DEFAULT_ASIC_LATCHING_NUM_PULSES << ASICRDO_CNFG_RESSTRG_LNGTH_OFST) &
         ASICRDO_CNFG_RESSTRG_LNGTH_MSK);
    bus_w(ASIC_RDO_CONFIG_REG, val);
}

void setADIFDefaults() {
    uint32_t addr = ADIF_CONFIG_REG;
    bus_w(addr, ((bus_r(addr) & ~ADIF_ADDTNL_OFST_MSK) & ~ADIF_PIPELINE_MSK));
    bus_w(addr,
          (bus_r(addr) | ((DEFAULT_ADIF_PIPELINE_VAL << ADIF_PIPELINE_OFST) &
                          ADIF_PIPELINE_MSK)));
    bus_w(addr,
          (bus_r(addr) | ((DEFAULT_ADIF_ADD_OFST_VAL << ADIF_ADDTNL_OFST_OFST) &
                          ADIF_ADDTNL_OFST_MSK)));
}

int checkCommandLineConfiguration() {
    if (masterCommandLine != -1) {
#ifdef VIRTUAL
        if (masterCommandLine == 1) {
            bus_w(SYSTEM_STATUS_REG,
                  bus_r(SYSTEM_STATUS_REG) & ~SYSTEM_STATUS_SLV_BRD_DTCT_MSK);
        } else {
            bus_w(SYSTEM_STATUS_REG,
                  bus_r(SYSTEM_STATUS_REG) | SYSTEM_STATUS_SLV_BRD_DTCT_MSK);
        }
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

/* set parameters -  dr, roi */

int setDynamicRange(int dr) {
    if (dr <= 0) {
        return FAIL;
    }
    uint32_t regval = 0;
    switch (dr) {
    /*case 1: TODO:Not implemented in firmware yet
        regval = CONFIG_DYNAMIC_RANGE_1_VAL;
        break;*/
    case 8:
        regval = CONFIG_DYNAMIC_RANGE_8_VAL;
        break;
    case 16:
        regval = CONFIG_DYNAMIC_RANGE_16_VAL;
        break;
    case 32:
        regval = CONFIG_DYNAMIC_RANGE_24_VAL;
        break;
    default:
        LOG(logERROR, ("Invalid dynamic range %d\n", dr));
        return -1;
    }
    // set it
    bus_w(CONFIG_REG, bus_r(CONFIG_REG) & ~CONFIG_DYNAMIC_RANGE_MSK);
    bus_w(CONFIG_REG, bus_r(CONFIG_REG) | regval);
    updatePacketizing();
    return OK;
}

int getDynamicRange(int *retval) {
    uint32_t regval = bus_r(CONFIG_REG) & CONFIG_DYNAMIC_RANGE_MSK;
    switch (regval) {
    /*case CONFIG_DYNAMIC_RANGE_1_VAL: TODO:Not implemented in firmware yet
        return 1;*/
    case CONFIG_DYNAMIC_RANGE_8_VAL:
        *retval = 8;
        break;
    case CONFIG_DYNAMIC_RANGE_16_VAL:
        *retval = 16;
        break;
    case CONFIG_DYNAMIC_RANGE_24_VAL:
        *retval = 32;
        break;
    default:
        LOG(logERROR, ("Invalid dynamic range %d read back\n",
                       regval >> CONFIG_DYNAMIC_RANGE_OFST));
        return FAIL;
    }
    return OK;
}

/* set parameters -  readout */

int setParallelMode(int mode) {
    if (mode < 0)
        return FAIL;
    LOG(logINFO, ("Setting %s mode\n", (mode ? "Parallel" : "Non Parallel")));
    uint32_t addr = DEADTIME_CONFIG_REG;
    if (mode) {
        bus_w(addr, bus_r(addr) | DEADTIME_FREE_MODE_ENBL_MSK);
    } else {
        bus_w(addr, bus_r(addr) & ~DEADTIME_FREE_MODE_ENBL_MSK);
    }
    return OK;
}

int getParallelMode() {
    return ((bus_r(DEADTIME_CONFIG_REG) & DEADTIME_FREE_MODE_ENBL_MSK) >>
            DEADTIME_FREE_MODE_ENBL_OFST);
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
        set64BitReg(val, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
    }
}

int64_t getNumTriggers() {
    return get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
}

int setPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid period: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    val *= (1E-9 * getFrequency(SYSTEM_C0));
    set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-9 * getFrequency(SYSTEM_C0));
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return get64BitReg(SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C0));
}

void setNumIntGates(int val) {
    if (val > 0) {
        LOG(logINFO,
            ("Setting number of Internal Gates %lld\n", (long long int)val));
        bus_w(ASIC_EXP_INT_GATE_NUMBER_REG, val);
    }
}

void setNumGates(int val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of Gates %lld\n", (long long int)val));
        bus_w(ASIC_EXP_EXT_GATE_NUMBER_REG, val);
    }
}

int getNumGates() { return bus_r(ASIC_EXP_EXT_GATE_NUMBER_REG); }

void updateGatePeriod() {
    uint64_t max = 0;
    uint32_t countermask = getCounterMask();
    for (int i = 0; i != NCOUNTERS; ++i) {
        // only if counter enabled
        if (countermask & (1 << i)) {
            uint64_t sum = getExpTime(i) + getGateDelay(i);
            if (sum > max) {
                max = sum;
            }
        }
    }
    LOG(logINFO, ("\tUpdating Gate Period to %lld ns\n", (long long int)max));
    max *= (1E-9 * getFrequency(SYSTEM_C0));
    set64BitReg(max, ASIC_EXP_GATE_PERIOD_LSB_REG,
                ASIC_EXP_GATE_PERIOD_MSB_REG);
}

int64_t getGatePeriod() {
    return get64BitReg(ASIC_EXP_GATE_PERIOD_LSB_REG,
                       ASIC_EXP_GATE_PERIOD_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C0));
}

int setExpTime(int gateIndex, int64_t val) {
    uint32_t alsb = 0;
    uint32_t amsb = 0;
    switch (gateIndex) {
    case 0:
        alsb = ASIC_EXP_GATE_0_WIDTH_LSB_REG;
        amsb = ASIC_EXP_GATE_0_WIDTH_MSB_REG;
        break;
    case 1:
        alsb = ASIC_EXP_GATE_1_WIDTH_LSB_REG;
        amsb = ASIC_EXP_GATE_1_WIDTH_MSB_REG;
        break;
    case 2:
        alsb = ASIC_EXP_GATE_2_WIDTH_LSB_REG;
        amsb = ASIC_EXP_GATE_2_WIDTH_MSB_REG;
        break;
    default:
        LOG(logERROR, ("Invalid gate index: %d\n", gateIndex));
        return FAIL;
    }
    if (val < 0) {
        LOG(logERROR,
            ("Invalid exptime%d : %lld ns\n", gateIndex, (long long int)val));
        return FAIL;
    }
    LOG(logINFO,
        ("Setting exptime%d to %lld ns\n", gateIndex, (long long int)val));
    val *= (1E-9 * getFrequency(SYSTEM_C0));

    // set in register only if counter enabled
    exptimeReg[gateIndex] = val;
    if (getCounterMask() & (1 << gateIndex)) {
        set64BitReg(val, alsb, amsb);
    } else {
        LOG(logWARNING, ("Writing 0 to reg (counter disabled)\n"));
        set64BitReg(0, alsb, amsb);
    }

    // validate for tolerance
    int64_t retval = getExpTime(gateIndex);
    val /= (1E-9 * getFrequency(SYSTEM_C0));
    if (val != retval) {
        return FAIL;
    }

    updateGatePeriod();

    return OK;
}

int64_t getExpTime(int gateIndex) {
    uint32_t alsb = 0;
    uint32_t amsb = 0;
    switch (gateIndex) {
    case 0:
        alsb = ASIC_EXP_GATE_0_WIDTH_LSB_REG;
        amsb = ASIC_EXP_GATE_0_WIDTH_MSB_REG;
        break;
    case 1:
        alsb = ASIC_EXP_GATE_1_WIDTH_LSB_REG;
        amsb = ASIC_EXP_GATE_1_WIDTH_MSB_REG;
        break;
    case 2:
        alsb = ASIC_EXP_GATE_2_WIDTH_LSB_REG;
        amsb = ASIC_EXP_GATE_2_WIDTH_MSB_REG;
        break;
    default:
        LOG(logERROR, ("Invalid gate index: %d\n", gateIndex));
        return -1;
    }

    uint64_t retval = 0;
    // read from register if counter enabled
    if (getCounterMask() & (1 << gateIndex)) {
        retval = get64BitReg(alsb, amsb);
    } else {
        retval = exptimeReg[gateIndex];
    }
    return retval / (1E-9 * getFrequency(SYSTEM_C0));
}

int setGateDelay(int gateIndex, int64_t val) {
    uint32_t alsb = 0;
    uint32_t amsb = 0;
    switch (gateIndex) {
    case 0:
        alsb = ASIC_EXP_GATE_0_DELAY_LSB_REG;
        amsb = ASIC_EXP_GATE_0_DELAY_MSB_REG;
        break;
    case 1:
        alsb = ASIC_EXP_GATE_1_DELAY_LSB_REG;
        amsb = ASIC_EXP_GATE_1_DELAY_MSB_REG;
        break;
    case 2:
        alsb = ASIC_EXP_GATE_2_DELAY_LSB_REG;
        amsb = ASIC_EXP_GATE_2_DELAY_MSB_REG;
        break;
    default:
        LOG(logERROR, ("Invalid gate index: %d\n", gateIndex));
        return FAIL;
    }
    if (val < 0) {
        LOG(logERROR, ("Invalid gate delay%d : %lld ns\n", gateIndex,
                       (long long int)val));
        return FAIL;
    }
    LOG(logINFO,
        ("Setting gate delay%d to %lld ns\n", gateIndex, (long long int)val));
    val *= (1E-9 * getFrequency(SYSTEM_C0));

    // set in register only if counter enabled
    gateDelayReg[gateIndex] = val;
    if (getCounterMask() & (1 << gateIndex)) {
        set64BitReg(val, alsb, amsb);
    } else {
        LOG(logWARNING, ("Writing 0 to reg (counter disabled)\n"));
        set64BitReg(0, alsb, amsb);
    }

    // validate for tolerance
    int64_t retval = getGateDelay(gateIndex);
    val /= (1E-9 * getFrequency(SYSTEM_C0));
    if (val != retval) {
        return FAIL;
    }

    updateGatePeriod();

    return OK;
}

int64_t getGateDelay(int gateIndex) {
    uint32_t alsb = 0;
    uint32_t amsb = 0;
    switch (gateIndex) {
    case 0:
        alsb = ASIC_EXP_GATE_0_DELAY_LSB_REG;
        amsb = ASIC_EXP_GATE_0_DELAY_MSB_REG;
        break;
    case 1:
        alsb = ASIC_EXP_GATE_1_DELAY_LSB_REG;
        amsb = ASIC_EXP_GATE_1_DELAY_MSB_REG;
        break;
    case 2:
        alsb = ASIC_EXP_GATE_2_DELAY_LSB_REG;
        amsb = ASIC_EXP_GATE_2_DELAY_MSB_REG;
        break;
    default:
        LOG(logERROR, ("Invalid gate index: %d\n", gateIndex));
        return -1;
    }

    uint64_t retval = 0;
    // read from register if counter enabled
    if (getCounterMask() & (1 << gateIndex)) {
        retval = get64BitReg(alsb, amsb);
    } else {
        retval = gateDelayReg[gateIndex];
    }
    return retval / (1E-9 * getFrequency(SYSTEM_C0));
}

void updateVthAndCounterMask() {
    LOG(logINFO, ("\tUpdating Vth and countermask\n"));
    int interpolation = getInterpolation();
    int pumpProbe = getPumpProbe();

    if (interpolation) {
        // enable all counters
        setCounterMaskWithUpdateFlag(MAX_COUNTER_MSK, 0);
        // disable vth3
        setVthDac(2, 0);
    } else {
        // previous counter values
        setCounterMaskWithUpdateFlag(counterMask, 0);
    }
    if (pumpProbe) {
        // enable only vth2
        setVthDac(0, 0);
        setVthDac(1, 1);
        setVthDac(2, 0);
    } else {
        setVthDac(0, (counterMask & (1 << 0)));
        setVthDac(1, (counterMask & (1 << 1)));
    }
    if (!interpolation && !pumpProbe) {
        setVthDac(2, (counterMask & (1 << 2)));
    }
}

void setCounterMask(uint32_t arg) {
    setCounterMaskWithUpdateFlag(arg, 1);
    updateVthAndCounterMask();
}

void setCounterMaskWithUpdateFlag(uint32_t arg, int updateMaskFlag) {
    if (arg == 0 || arg > MAX_COUNTER_MSK) {
        return;
    }
    LOG(logINFO, ("\tSetting counter mask to  0x%x\n", arg));
    uint32_t addr = CONFIG_REG;
    bus_w(addr, bus_r(addr) & ~CONFIG_COUNTERS_ENA_MSK);
    bus_w(addr, bus_r(addr) | ((arg << CONFIG_COUNTERS_ENA_OFST) &
                               CONFIG_COUNTERS_ENA_MSK));
    LOG(logDEBUG, ("Config Reg: 0x%x\n", bus_r(addr)));

    updatePacketizing();
    LOG(logINFO, ("\tUpdating Exptime and Gate Delay\n"));
    for (int i = 0; i < NCOUNTERS; ++i) {
        uint64_t ns = exptimeReg[i] / (1E-9 * getFrequency(SYSTEM_C0));
        setExpTime(i, ns);
        ns = gateDelayReg[i] / (1E-9 * getFrequency(SYSTEM_C0));
        setGateDelay(i, ns);
    }

    if (updateMaskFlag) {
        counterMask = arg;
    }
}

uint32_t getCounterMask() {
    return ((bus_r(CONFIG_REG) & CONFIG_COUNTERS_ENA_MSK) >>
            CONFIG_COUNTERS_ENA_OFST);
}

void updatePacketizing() {
    LOG(logINFO, ("\tUpdating Packetizing\n"));
    const int ncounters = __builtin_popcount(getCounterMask());
    const int tgEnable = enableTenGigabitEthernet(-1);
    int packetsPerFrame = 0;

    // 10g
    if (tgEnable) {
        int dr = 0;
        getDynamicRange(&dr);
        packetsPerFrame = 1;
        if (dr == 32 && ncounters > 1) {
            packetsPerFrame = 2;
        }
    }
    // 1g
    else {
        int dataSize = 1280;
        if (ncounters == 3) {
            dataSize = 768;
        }
        packetsPerFrame = calculateDataBytes() / dataSize;
    }
    const int deserializersPerPacket = MAX_NUM_DESERIALIZERS / packetsPerFrame;

    // bus_w()
    LOG(logINFO,
        ("[#Packets/Frame: %d, #Deserializers/Packet: %d] for %s\n",
         packetsPerFrame, deserializersPerPacket, (tgEnable ? "10g" : "1g")));
    const uint32_t addr = PKT_FRAG_REG;
    if (tgEnable) {
        bus_w(addr, bus_r(addr) & ~PKT_FRAG_10G_NUM_PACKETS_MSK);
        bus_w(addr, bus_r(addr) |
                        ((packetsPerFrame << PKT_FRAG_10G_NUM_PACKETS_OFST) &
                         PKT_FRAG_10G_NUM_PACKETS_MSK));
        bus_w(addr, bus_r(addr) & ~PKT_FRAG_10G_N_DSR_PER_PKT_MSK);
        bus_w(addr, bus_r(addr) | ((deserializersPerPacket
                                    << PKT_FRAG_10G_N_DSR_PER_PKT_OFST) &
                                   PKT_FRAG_10G_N_DSR_PER_PKT_MSK));
    } else {
        bus_w(addr, bus_r(addr) & ~PKT_FRAG_1G_NUM_PACKETS_MSK);
        bus_w(addr,
              bus_r(addr) | ((packetsPerFrame << PKT_FRAG_1G_NUM_PACKETS_OFST) &
                             PKT_FRAG_1G_NUM_PACKETS_MSK));
        bus_w(addr, bus_r(addr) & ~PKT_FRAG_1G_N_DSR_PER_PKT_MSK);
        bus_w(addr, bus_r(addr) | ((deserializersPerPacket
                                    << PKT_FRAG_1G_N_DSR_PER_PKT_OFST) &
                                   PKT_FRAG_1G_N_DSR_PER_PKT_MSK));
    }
}

int setDelayAfterTrigger(int64_t val) {
    if (val < 0) {
        LOG(logERROR,
            ("Invalid delay after trigger: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting delay after trigger %lld ns\n", (long long int)val));
    val *= (1E-9 * getFrequency(SYSTEM_C0));
    set64BitReg(val, SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG);

    // validate for tolerance
    int64_t retval = getDelayAfterTrigger();
    val /= (1E-9 * getFrequency(SYSTEM_C0));
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getDelayAfterTrigger() {
    return get64BitReg(SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C0));
}

int64_t getNumFramesLeft() {
    return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t getNumTriggersLeft() {
    return get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
}

int64_t getDelayAfterTriggerLeft() {
    return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C0));
}

int64_t getPeriodLeft() {
    return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C0));
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

/* parameters - module, speed, readout */

int setDACS(int *dacs) {
    for (int i = 0; i < NDAC; ++i) {
        if (dacs[i] != -1) {
            // set to default (last arg to ensure counter check)
            setDAC((enum DACINDEX)i, dacs[i], 0, 1);
            if (dacs[i] != detectorDacs[i]) {
                // dont complain if that counter was disabled
                if ((i == M_VTH1 || i == M_VTH2 || i == M_VTH3) &&
                    (detectorDacs[i] == DEFAULT_COUNTER_DISABLED_VTH_VAL)) {
                    continue;
                }
                return FAIL;
            }
        }
    }
    return OK;
}

void getModule(sls_detector_module *myMod) {
    // serial number
    myMod->serialnumber = detectorModules->serialnumber;
    // csr reg
    myMod->reg = detectorModules->reg;
    // eV
    myMod->eV[0] = detectorModules->eV[0];
    myMod->eV[1] = detectorModules->eV[1];
    myMod->eV[2] = detectorModules->eV[2];
    // dacs
    for (int idac = 0; idac < (detectorModules->ndac); idac++) {
        *((myMod->dacs) + idac) = *((detectorModules->dacs) + idac);
    }
    // trimbits
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        *((myMod->chanregs) + ichan) = *((detectorModules->chanregs) + ichan);
    }
}

int setModule(sls_detector_module myMod, char *mess) {
    LOG(logINFO, ("Setting module\n"));

    if (((myMod.nchan) > (detectorModules->nchan)) ||
        ((myMod.ndac) > (detectorModules->ndac))) {
        strcpy(mess, "Could not set module as the number of channels or dacs "
                     "do not match to the one in the detector server\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    // serial number (pointless)
    detectorModules->serialnumber = myMod.serialnumber;

    // csr reg
    flipNegativePolarityBits(&myMod.reg);
    if (setChipStatusRegister(myMod.reg)) {
        sprintf(mess, "Could not CSR from module\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    // dacs
    if (setDACS(myMod.dacs)) {
        sprintf(mess, "Could not set dacs\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    // update vth and countermask
    updateVthAndCounterMask();

    // threshold energy
    for (int i = 0; i < NCOUNTERS; ++i) {
        if (myMod.eV[i] >= 0) {
            setThresholdEnergy(i, myMod.eV[i]);
        } else {
            setThresholdEnergy(i, -1);
        }
    }

    // trimbits
    if (myMod.nchan == 0) {
        LOG(logINFO, ("Setting module without trimbits\n"));
    } else {
        // set trimbits
        if (setTrimbits(myMod.chanregs) == FAIL) {
            sprintf(mess, "Could not set module. Could not set trimbits\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
    }

    return OK;
}

int setTrimbits(int *trimbits) {
    LOG(logINFOBLUE, ("Setting trimbits\n"));

    // remember previous run clock
    uint32_t prevRunClk = clkDivider[SYSTEM_C0];

    // set to trimming clock
    if (setClockDividerWithTimeUpdateOption(
            SYSTEM_C0, DEFAULT_TRIMMING_RUN_CLKDIV, 0) == FAIL) {
        LOG(logERROR,
            ("Could not start trimming. Could not set to trimming clock\n"));
        return FAIL;
    }

    // for every chip
    int error = 0;
    char cmess[MAX_STR_LENGTH];
    for (int ichip = 0; ichip < NCHIP; ichip++) {
        patternParameters *pat =
            setChannelRegisterChip(ichip, badChannelMask, trimbits);
        if (pat == NULL) {
            error = 1;
        } else {
            memset(cmess, 0, MAX_STR_LENGTH);
            error |= loadPattern(cmess, logDEBUG5, pat, "");
            if (!error)
                startPattern();
            free(pat);
        }
    }

    // copy trimbits locally
    if (error == 0) {
        for (int ichan = 0; ichan < ((detectorModules)->nchan); ++ichan) {
            detectorChans[ichan] = trimbits[ichan];
        }
        LOG(logINFO, ("All trimbits have been loaded\n"));
    }

    // set back to previous clock
    if (setClockDividerWithTimeUpdateOption(SYSTEM_C0, prevRunClk, 0) == FAIL) {
        LOG(logERROR, ("Could not set to previous run clock after trimming\n"));
        return FAIL;
    }

    // copying trimbits locally (if tirmbit value > -1)
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        if (*(trimbits + ichan) >= 0)
            *((detectorModules->chanregs) + ichan) = *(trimbits + ichan);
    }

    return (error ? FAIL : OK);
}

int setAllTrimbits(int val) {
    LOG(logINFO, ("Setting all trimbits to %d\n", val));
    int *trimbits = malloc(sizeof(int) * ((detectorModules)->nchan));
    if (trimbits == NULL) {
        LOG(logERROR, ("Could not allocate memory to set all trimbits\n"));
        return FAIL;
    }
    for (int ichan = 0; ichan < ((detectorModules)->nchan); ++ichan) {
        trimbits[ichan] = val;
    }
    if (setTrimbits(trimbits) == FAIL) {
        LOG(logERROR, ("Could not set all trimbits to %d\n", val));
        free(trimbits);
        return FAIL;
    }
    LOG(logINFO, ("All trimbits have been set to %d\n", val));
    free(trimbits);
    // changed for setsettings (direct),
    // custom trimbit file (setmodule with myMod.reg as -1),
    // change of dac (direct)
    for (int i = 0; i < NCOUNTERS; ++i) {
        setThresholdEnergy(i, -1);
    }
    return OK;
}

int getAllTrimbits() {
    int value = detectorChans[0];
    if (detectorModules) {
        for (int ichan = 0; ichan < ((detectorModules)->nchan); ichan++) {
            if (detectorChans[ichan] != value) {
                value = -1;
                break;
            }
        }
    }
    LOG(logINFO, ("Value of all Trimbits: %d\n", value));
    return value;
}

enum detectorSettings setSettings(enum detectorSettings sett) {
    int *dacVals = NULL;
    switch (sett) {
    case STANDARD:
        LOG(logINFOBLUE, ("Setting to standard settings\n"));
        dacVals = defaultDacValue_standard;
        break;
    case FAST:
        LOG(logINFOBLUE, ("Setting to fast settings\n"));
        dacVals = defaultDacValue_fast;
        break;
    case HIGHGAIN:
        LOG(logINFOBLUE, ("Setting to high gain settings\n"));
        dacVals = defaultDacValue_highgain;
        break;
    default:
        LOG(logERROR,
            ("Settings %d not defined for this detector\n", (int)sett));
        return thisSettings;
    }

    thisSettings = sett;

    // set special dacs
    const int specialDacs[] = SPECIALDACINDEX;
    for (int i = 0; i < NSPECIALDACS; ++i) {
        // set to default (last arg to ensure counter check)
        setDAC(specialDacs[i], dacVals[i], 0, 1);
    }

    LOG(logINFO, ("Settings: %d\n", thisSettings));
    return thisSettings;
}

void validateSettings() {
    LOG(logWARNING, ("Not validating dac settings temporarily"));
    return;

    // if any special dac value is changed individually => undefined
    const int specialDacs[NSPECIALDACS] = SPECIALDACINDEX;
    int *specialDacValues[] = {defaultDacValue_standard, defaultDacValue_fast,
                               defaultDacValue_highgain};
    int settList[] = {STANDARD, FAST, HIGHGAIN};

    enum detectorSettings sett = UNDEFINED;
    for (int isett = 0; isett != NUMSETTINGS; ++isett) {

        // assume it matches current setting in list
        sett = settList[isett];
        // if one value does not match, = undefined
        for (int i = 0; i < NSPECIALDACS; ++i) {
            if (getDAC(specialDacs[i], 0) != specialDacValues[isett][i]) {
                sett = UNDEFINED;
                break;
            }
        }

        // all values matchd a setting
        if (sett != UNDEFINED) {
            break;
        }
    }
    // update settings
    if (thisSettings != sett) {
        LOG(logINFOBLUE,
            ("Validated settings to %s (%d)\n",
             (sett == STANDARD
                  ? "standard"
                  : (sett == FAST
                         ? "fast"
                         : (sett == HIGHGAIN ? "highgain" : "undefined"))),
             sett));
        thisSettings = sett;
    }
}

enum detectorSettings getSettings() { return thisSettings; }

int getThresholdEnergy(int counterIndex) {
    return (detectorModules)->eV[counterIndex];
}

void setThresholdEnergy(int counterIndex, int eV) {
    (detectorModules)->eV[counterIndex] = eV;
}

/* parameters - dac, hv */
// counterEnableCheck false only if setDAC called directly
void setDAC(enum DACINDEX ind, int val, int mV, int counterEnableCheck) {
    // invalid value
    if (val < 0) {
        return;
    }
    // out of scope, NDAC + 1 for vthreshold
    if ((int)ind > NDAC + 1) {
        LOG(logERROR, ("Unknown dac index %d\n", ind));
        return;
    }

    // threshold dacs
    // remember value, vthreshold: skip disabled,
    // others: disable or enable dac if counter mask
    // setDAC called directly: will set independent of counter enable
    if (ind == M_VTHRESHOLD || ind == M_VTH1 || ind == M_VTH2 ||
        ind == M_VTH3) {
        char *dac_names[] = {DAC_NAMES};
        int vthdacs[] = {M_VTH1, M_VTH2, M_VTH3};
        uint32_t counters = getCounterMask();
        for (int i = 0; i < NCOUNTERS; ++i) {
            if ((int)ind == vthdacs[i] || ind == M_VTHRESHOLD) {
                int dacval = val;
                // if not disabled value, remember value
                if (dacval != DEFAULT_COUNTER_DISABLED_VTH_VAL) {
                    if (mV) {
                        if (LTC2620_D_VoltageToDac(val, &dacval) == FAIL) {
                            return;
                        }
                    }
                    vthEnabledVals[i] = dacval;
                    LOG(logINFO,
                        ("Remembering %s [%d]\n", dac_names[ind], dacval));
                }
                // disabled counter
                if (!(counters & (1 << i))) {
                    // skip setting vthx dac (value remembered anyway)
                    if (ind == M_VTHRESHOLD) {
                        continue;
                    }
                    // disable dac (except when setting dac directly)
                    if (counterEnableCheck) {
                        val = DEFAULT_COUNTER_DISABLED_VTH_VAL;
                    }
                }
                setGeneralDAC(vthdacs[i], val, mV);
            }
        }
        return;
    }

    setGeneralDAC(ind, val, mV);
}

void setGeneralDAC(enum DACINDEX ind, int val, int mV) {
    char *dac_names[] = {DAC_NAMES};
    LOG(logDEBUG1, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                    val, (mV ? "mV" : "dac units")));
    int dacval = val;

#ifdef VIRTUAL
    LOG(logINFO, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                  val, (mV ? "mV" : "dac units")));
    if (!mV) {
        detectorDacs[ind] = val;
    }
    // convert to dac units
    else if (LTC2620_D_VoltageToDac(val, &dacval) == OK) {
        detectorDacs[ind] = dacval;
    }
#else
    if (LTC2620_D_SetDACValue((int)ind, val, mV, dac_names[ind], &dacval) ==
        OK) {
        detectorDacs[ind] = dacval;
    }
#endif
    const int specialDacs[NSPECIALDACS] = SPECIALDACINDEX;
    for (int i = 0; i < NSPECIALDACS; ++i) {
        if ((int)ind == specialDacs[i]) {
            validateSettings();
        }
    }
}

void setVthDac(int index, int enable) {
    LOG(logINFO, ("\t%s vth%d\n", (enable ? "Enabling" : "Disabing"), index));
    // enables (from remembered values) or disables vthx
    enum DACINDEX vthdacs[] = {M_VTH1, M_VTH2, M_VTH3};
    // disable value
    int value = DEFAULT_COUNTER_DISABLED_VTH_VAL;
    // enable, set saved values
    if (enable) {
        value = vthEnabledVals[index];
    }
    setGeneralDAC(vthdacs[index], value, 0);
}

int getDAC(enum DACINDEX ind, int mV) {
    if (ind == M_VTHRESHOLD) {
        int ret = -1, ret1 = -1;
        // get only for enabled counters
        uint32_t counters = getCounterMask();
        int vthdacs[] = {M_VTH1, M_VTH2, M_VTH3};
        for (int i = 0; i < NCOUNTERS; ++i) {
            if (counters & (1 << i)) {
                ret1 = getDAC(vthdacs[i], mV);
                // first enabled counter
                if (ret == -1) {
                    ret = ret1;
                }
                // different values for enabled counters
                else if (ret1 != ret) {
                    return -1;
                }
            }
        }
        if (ret == -1) {
            LOG(logERROR, ("\tvthreshold mismatch (of enabled counters)\n"));
        } else {
            LOG(logINFO, ("\tvthreshold match %d\n", ret));
        }
        return ret;
    }

    if (!mV) {
        LOG(logDEBUG1, ("Getting DAC %d : %d dac\n", ind, detectorDacs[ind]));
        return detectorDacs[ind];
    }
    int voltage = -1;
    LTC2620_D_DacToVoltage(detectorDacs[ind], &voltage);
    LOG(logDEBUG1,
        ("Getting DAC %d : %d dac (%d mV)\n", ind, detectorDacs[ind], voltage));
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
    // limit values
    if (val > HV_SOFT_MAX_VOLTAGE) {
        val = HV_SOFT_MAX_VOLTAGE;
    }

    LOG(logINFO, ("Setting High voltage: %d V\n", val));
    return DAC6571_Set(val);
}

int getHighVoltage(int *retval) { return DAC6571_Get(retval); }

/* parameters - timing */

int isMaster(int *retval) {
    int slave = ((bus_r(SYSTEM_STATUS_REG) & SYSTEM_STATUS_SLV_BRD_DTCT_MSK) >>
                 SYSTEM_STATUS_SLV_BRD_DTCT_OFST);
    *retval = (slave == 0 ? 1 : 0);
    return OK;
}

void setTiming(enum timingMode arg) {

    int master = 0;
    isMaster(&master);
    if (!master && arg == AUTO_TIMING)
        arg = TRIGGER_EXPOSURE;

    uint32_t addr = CONFIG_REG;
    switch (arg) {
    case AUTO_TIMING:
        LOG(logINFO, ("Set Timing: Auto (Int. Trigger, Int. Gating)\n"));
        bus_w(addr, bus_r(addr) & ~CONFIG_TRIGGER_ENA_MSK);
        bus_w(ASIC_EXP_STATUS_REG,
              bus_r(ASIC_EXP_STATUS_REG) & ~ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
        break;
    case TRIGGER_EXPOSURE:
        LOG(logINFO, ("Set Timing: Trigger (Ext. Trigger, Int. Gating)\n"));
        bus_w(addr, bus_r(addr) | CONFIG_TRIGGER_ENA_MSK);
        bus_w(ASIC_EXP_STATUS_REG,
              bus_r(ASIC_EXP_STATUS_REG) & ~ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
        break;
    case GATED:
        LOG(logINFO, ("Set Timing: Gating (Int. Trigger, Ext. Gating)\n"));
        bus_w(addr, bus_r(addr) & ~CONFIG_TRIGGER_ENA_MSK);
        bus_w(ASIC_EXP_STATUS_REG,
              bus_r(ASIC_EXP_STATUS_REG) | ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
        break;
    case TRIGGER_GATED:
        LOG(logINFO,
            ("Set Timing: Trigger_Gating (Ext. Trigger, Ext. Gating)\n"));
        bus_w(addr, bus_r(addr) | CONFIG_TRIGGER_ENA_MSK);
        bus_w(ASIC_EXP_STATUS_REG,
              bus_r(ASIC_EXP_STATUS_REG) | ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
        break;
    default:
        LOG(logERROR, ("Unknown timing mode %d\n", arg));
        return;
    }
}

enum timingMode getTiming() {
    uint32_t extTrigger = (bus_r(CONFIG_REG) & CONFIG_TRIGGER_ENA_MSK);
    uint32_t extGate =
        (bus_r(ASIC_EXP_STATUS_REG) & ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
    if (extTrigger) {
        if (extGate) {
            // external trigger, external gating
            return TRIGGER_GATED;
        } else {
            // external trigger, internal gating
            return TRIGGER_EXPOSURE;
        }
    } else {
        if (extGate) {
            // internal trigger, external gating
            return GATED;
        } else {
            // internal trigger, internal gating
            return AUTO_TIMING;
        }
    }
}

void setInitialExtSignals() {
    LOG(logINFOBLUE, ("Setting Initial External Signals\n"));
    // default, everything is 0
    bus_w(DINF1_REG, 0);
    bus_w(DOUTIF1_REG, 0);
    bus_w(DINF2_REG, 0);
    bus_w(DOUTIF_RISING_LNGTH_REG, 0);

    // bypass everything
    // (except master triggers can edge detect)
    bus_w(DINF1_REG, DINF1_BYPASS_GATE_MSK);
    bus_w(DOUTIF1_REG, DOUTIF1_BYPASS_GATE_MSK);
    bus_w(DINF2_REG, DINF2_BYPASS_MSK);

    // master input/output  can edge detect, so rising is 1
    bus_w(DINF1_REG, bus_r(DINF1_REG) | DINF1_RISING_TRIGGER_MSK);
    bus_w(DOUTIF1_REG, bus_r(DOUTIF1_REG) | DOUTIF1_RISING_TRIGGER_MSK);

    // set default value for master output rising pulse length for port1
    bus_w(DOUTIF_RISING_LNGTH_REG,
          bus_r(DOUTIF_RISING_LNGTH_REG) & ~DOUTIF_RISING_LNGTH_PORT_1_MSK);
    bus_w(DOUTIF_RISING_LNGTH_REG, bus_r(DOUTIF_RISING_LNGTH_REG) |
                                       ((DEFAULT_MSTR_OTPT_P1_NUM_PULSES
                                         << DOUTIF_RISING_LNGTH_PORT_1_OFST) &
                                        DOUTIF_RISING_LNGTH_PORT_1_MSK));
}

int setGainCaps(int caps) {
    LOG(logINFO, ("Setting gain caps to: %u\n", caps));
    int csr = M3SetGainCaps(caps);
    return setChipStatusRegister(csr);
}

int setInterpolation(int enable) {
    LOG(logINFO,
        ("%s Interpolation\n", enable == 0 ? "Disabling" : "Enabling"));

    int csr = M3SetInterpolation(enable);
    int ret = setChipStatusRegister(csr);
    if (ret == OK) {
        updateVthAndCounterMask();
    }
    return ret;
}

int setPumpProbe(int enable) {
    LOG(logINFO, ("%s Pump Probe\n", enable == 0 ? "Disabling" : "Enabling"));

    int csr = M3SetPumpProbe(enable);
    int ret = setChipStatusRegister(csr);
    if (ret == OK) {
        updateVthAndCounterMask();
    }
    return ret;
}

int setDigitalPulsing(int enable) {
    LOG(logINFO,
        ("%s Digital Pulsing\n", enable == 0 ? "Disabling" : "Enabling"));
    int csr = M3SetDigitalPulsing(enable);
    return setChipStatusRegister(csr);
}

int setAnalogPulsing(int enable) {
    LOG(logINFO,
        ("%s Analog Pulsing\n", enable == 0 ? "Disabling" : "Enabling"));
    int csr = M3SetAnalogPulsing(enable);
    return setChipStatusRegister(csr);
}

int setNegativePolarity(int enable) {
    LOG(logINFO,
        ("%s Negative Polarity\n", enable == 0 ? "Disabling" : "Enabling"));
    int csr = M3SetNegativePolarity(enable);
    return setChipStatusRegister(csr);
}

void setExtSignal(int signalIndex, enum externalSignalFlag mode) {
    LOG(logDEBUG1, ("Setting signal flag[%d] to %d\n", signalIndex, mode));

    if (signalIndex == 0 && mode != TRIGGER_IN_RISING_EDGE &&
        mode != TRIGGER_IN_FALLING_EDGE) {
        return;
    }

    // getting addr and mask for each signal
    uint32_t addr = 0;
    uint32_t mask = 0;
    if (signalIndex <= 3) {
        addr = DINF1_REG;
        int offset = DINF1_INVERSION_OFST + signalIndex;
        mask = (1 << offset);
    } else {
        addr = DOUTIF1_REG;
        int offset = DOUTIF1_INVERSION_OFST + signalIndex - 4;
        mask = (1 << offset);
    }
    LOG(logDEBUG, ("addr: 0x%x mask:0x%x\n", addr, mask));

    switch (mode) {
    case TRIGGER_IN_RISING_EDGE:
        LOG(logINFO, ("Setting External Master Input Signal flag: Trigger in "
                      "Rising Edge\n"));
        bus_w(addr, bus_r(addr) & ~mask);
        break;
    case TRIGGER_IN_FALLING_EDGE:
        LOG(logINFO, ("Setting External Master Input Signal flag: Trigger in "
                      "Falling Edge\n"));
        bus_w(addr, bus_r(addr) | mask);
        break;
    case INVERSION_ON:
        LOG(logINFO, ("Setting External Master %s Signal flag: Inversion on\n",
                      (signalIndex <= 3 ? "Input" : "Output")));
        bus_w(addr, bus_r(addr) | mask);
        break;
    case INVERSION_OFF:
        LOG(logINFO, ("Setting External Master %s Signal flag: Inversion offn",
                      (signalIndex <= 3 ? "Input" : "Output")));
        bus_w(addr, bus_r(addr) & ~mask);
        break;
    default:
        LOG(logERROR,
            ("Extsig (signal mode) %d not defined for this detector\n", mode));
        return;
    }
}

int getExtSignal(int signalIndex) {
    // getting addr and mask for each signal
    uint32_t addr = 0;
    uint32_t mask = 0;
    if (signalIndex <= 3) {
        addr = DINF1_REG;
        int offset = DINF1_INVERSION_OFST + signalIndex;
        mask = (1 << offset);
    } else {
        addr = DOUTIF1_REG;
        int offset = DOUTIF1_INVERSION_OFST + signalIndex - 4;
        mask = (1 << offset);
    }
    LOG(logDEBUG, ("addr: 0x%x mask:0x%x\n", addr, mask));

    int val = bus_r(addr) & mask;
    // master input trigger signal
    if (signalIndex == 0) {
        if (val) {
            return TRIGGER_IN_FALLING_EDGE;
        } else {
            return TRIGGER_IN_RISING_EDGE;
        }
    } else {
        if (val) {
            return INVERSION_ON;
        } else {
            return INVERSION_OFF;
        }
    }
}

int getNumberofUDPInterfaces() { return 1; }

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

int configureMAC() {

    LOG(logINFOBLUE, ("Configuring MAC\n"));
    LOG(logINFO, ("Number of entries: %d\n\n", numUdpDestinations));
    for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION; ++iRxEntry) {

        uint32_t srcip = udpDetails[iRxEntry].srcip;
        uint32_t dstip = udpDetails[iRxEntry].dstip;
        uint64_t srcmac = udpDetails[iRxEntry].srcmac;
        uint64_t dstmac = udpDetails[iRxEntry].dstmac;
        uint16_t srcport = udpDetails[iRxEntry].srcport;
        uint16_t dstport = udpDetails[iRxEntry].dstport;

        char src_mac[MAC_ADDRESS_SIZE], src_ip[INET_ADDRSTRLEN],
            dst_mac[MAC_ADDRESS_SIZE], dst_ip[INET_ADDRSTRLEN];
        getMacAddressinString(src_mac, MAC_ADDRESS_SIZE, srcmac);
        getMacAddressinString(dst_mac, MAC_ADDRESS_SIZE, dstmac);
        getIpAddressinString(src_ip, srcip);
        getIpAddressinString(dst_ip, dstip);
        if (iRxEntry < numUdpDestinations) {
            LOG(logINFOBLUE, ("\tEntry %d\n", iRxEntry));
            LOG(logINFO, ("\tSource IP   : %s\n"
                          "\tSource MAC  : %s\n"
                          "\tSource Port : %hu\n"
                          "\tDest IP     : %s\n"
                          "\tDest MAC    : %s\n"
                          "\tDest Port   : %hu\n",
                          src_ip, src_mac, srcport, dst_ip, dst_mac, dstport));
        }
#ifdef VIRTUAL
        if (setUDPDestinationDetails(iRxEntry, 0, dst_ip, dstport) == FAIL) {
            LOG(logERROR, ("could not set udp destination IP and port for "
                           "data interface [entry:%d] \n",
                           iRxEntry));
            return FAIL;
        }
#endif

        // start addr
        uint32_t addr = BASE_UDP_RAM;
        // calculate rxr endpoint offset
        addr += (iRxEntry * RXR_ENDPOINT_OFST); // TODO: is there round robin
        // get struct memory
        udp_header *udp =
            (udp_header *)(Nios_getBaseAddress() + addr / (sizeof(u_int32_t)));
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
        if (iRxEntry < numUdpDestinations) {
            LOG(logINFO, ("\tIP checksum : 0x%lx\n\n", udp->ip_checksum));
        }
    }

    // TODO?
    cleanFifos();
    resetCore();
    // alignDeserializer();
    return OK;
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

int enableTenGigabitEthernet(int val) {
    uint32_t addr = PKT_CONFIG_REG;

    // set
    if (val != -1) {
        LOG(logINFO, ("Setting 10Gbe: %d\n", (val > 0) ? 1 : 0));
        // 1g
        if (val == 0) {
            bus_w(addr, bus_r(addr) | PKT_CONFIG_1G_INTERFACE_MSK);
        }
        // 10g
        else {
            bus_w(addr, bus_r(addr) & (~PKT_CONFIG_1G_INTERFACE_MSK));
        }
        // stop server does not know dr in virtual mode
#ifdef VIRTUAL
        if (isControlServer) {
#endif
            updatePacketizing();
#ifdef VIRTUAL
        }
#endif
    }
    int oneG = ((bus_r(addr) & PKT_CONFIG_1G_INTERFACE_MSK) >>
                PKT_CONFIG_1G_INTERFACE_OFST);
    return oneG ? 0 : 1;
}

int checkDetectorType(char *mess) {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Checking type of module\n"));
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
        return FAIL;
    }
    int type = atoi(buffer);
    if (type > TYPE_NO_MODULE_STARTING_VAL) {
        sprintf(mess, "No Module attached! Run server with -nomodule.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    if (abs(type - TYPE_MYTHEN3_MODULE_VAL) > TYPE_TOLERANCE) {
        sprintf(mess,
                "Wrong Module attached! Expected %d for Mythen3, got %d\n",
                TYPE_MYTHEN3_MODULE_VAL, type);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int powerChip(int on) {
    if (on != -1) {
        if (on) {
            LOG(logINFO, ("Powering chip: on\n"));
            bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PWR_CHIP_MSK);
        } else {
            LOG(logINFO, ("Powering chip: off\n"));
            bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PWR_CHIP_MSK);
        }
    }

    return ((bus_r(CONTROL_REG) & CONTROL_PWR_CHIP_MSK) >>
            CONTROL_PWR_CHIP_OFST);
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
    ALTERA_PLL_C10_SetPhaseShift(SYSTEM_PLL, ind, relativePhase, direction);

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
    return (getVCOFrequency(ind) / clkDivider[ind]);
}

int getVCOFrequency(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get vco frequency\n", ind));
        return -1;
    }
    return ALTERA_PLL_C10_GetVCOFrequency(SYSTEM_PLL);
}

int getMaxClockDivider() { return ALTERA_PLL_C10_GetMaxClockDivider(); }

int setClockDivider(enum CLKINDEX ind, int val) {
    char *clock_names[] = {CLK_NAMES};
    if (ind == SYSTEM_C1 || ind == SYSTEM_C2) {
        LOG(logERROR, ("Cannot set %s and %s for this detector\n",
                       clock_names[SYSTEM_C1], clock_names[SYSTEM_C2]));
        return FAIL;
    }
    return setClockDividerWithTimeUpdateOption(ind, val, 1);
}

int setClockDividerWithTimeUpdateOption(enum CLKINDEX ind, int val,
                                        int timeUpdate) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to set clock divider\n", ind));
        return FAIL;
    }
    if (val < 2 || val > getMaxClockDivider()) {
        LOG(logERROR, ("Invalid clock divider %d\n", val));
        return FAIL;
    }
    char *clock_names[] = {CLK_NAMES};

    LOG(logINFO, ("\tSetting %s clock (%d) divider from %d to %d\n",
                  clock_names[ind], ind, clkDivider[ind], val));

    // Remembering old phases in degrees
    int oldPhases[NUM_CLOCKS];
    for (int i = 0; i < NUM_CLOCKS; ++i) {
        oldPhases[i] = getPhase(i, 1);
        LOG(logDEBUG1, ("\tRemembering %s clock (%d) phase: %d degrees\n",
                        clock_names[ind], ind, oldPhases[i]));
    }

    // Calculate and set output frequency
    ALTERA_PLL_C10_SetOuputClockDivider(SYSTEM_PLL, ind, val);

    // Update time settings that depend on system frequency
    // timeUpdate = 0 for setChipRegister/setTrimbits etc
    // as clk reverted back again
    if (timeUpdate && ind == SYSTEM_C0) {
        LOG(logINFO, ("\tUpdating time settings (sys freq change)\n"));
        int64_t exptime[3] = {0, 0, 0};
        int64_t gateDelay[3] = {0, 0, 0};
        for (int i = 0; i != 3; ++i) {
            exptime[i] = getExpTime(i);
            gateDelay[i] = getGateDelay(i);
        }
        int64_t period = getPeriod();
        int64_t delayAfterTrigger = getDelayAfterTrigger();

        clkDivider[ind] = val;

        for (int i = 0; i != 3; ++i) {
            setExpTime(i, exptime[i]);
            setGateDelay(i, gateDelay[i]);
        }
        setPeriod(period);
        setDelayAfterTrigger(delayAfterTrigger);
        LOG(logINFO, ("\tDone updating time settings\n"));
    }

    clkDivider[ind] = val;
    LOG(logINFO, ("\t%s clock (%d) divider set to %d\n", clock_names[ind], ind,
                  clkDivider[ind]));

    // phase is reset by pll (when setting output frequency)
    clkPhase[SYSTEM_C0] = 0;
    clkPhase[SYSTEM_C1] = 0;
    clkPhase[SYSTEM_C2] = 0;

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
    return OK;
}

int getClockDivider(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get clock divider\n", ind));
        return -1;
    }
    return clkDivider[ind];
}

int setReadoutSpeed(int val) {
    enum speedLevel speed = FULL_SPEED;
    switch (val) {
    case FULL_SPEED:
        LOG(logINFO, ("Setting Full Speed (100 MHz):\n"));
        speed = FULL_SPEED_CLKDIV;
        break;
    case HALF_SPEED:
        LOG(logINFO, ("Setting Half Speed (50 MHz):\n"));
        speed = HALF_SPEED_CLKDIV;
        break;
    case QUARTER_SPEED:
        LOG(logINFO, ("Setting Quarter Speed (25 MHz):\n"));
        speed = QUARTER_SPEED_CLKDIV;
        break;
    default:
        LOG(logERROR, ("Unknown readout speed %d\n", val));
        return FAIL;
    }
    return setClockDivider(SYSTEM_C0, speed);
}

int getReadoutSpeed(int *retval) {
    int clkdiv = getClockDivider(SYSTEM_C0);
    if (clkdiv == FULL_SPEED_CLKDIV) {
        *retval = FULL_SPEED;
    } else if (clkdiv == HALF_SPEED_CLKDIV) {
        *retval = HALF_SPEED;
    } else if (clkdiv == QUARTER_SPEED_CLKDIV) {
        *retval = QUARTER_SPEED;
    } else {
        return FAIL;
    }
    return OK;
}

int setBadChannels(int numChannels, int *channelList) {
    LOG(logINFO, ("Setting %d bad channels\n", numChannels));
    memset(badChannelMask, 0, NCHAN_PER_MODULE * sizeof(char));
    for (int i = 0; i != numChannels; ++i) {
        LOG(logINFO, ("\t[%d]: %d\n", i, channelList[i]));
        for (int ich = channelList[i] * NCOUNTERS;
             ich != channelList[i] * NCOUNTERS + NCOUNTERS; ++ich) {
            badChannelMask[ich] = 1;
        }
    }
    for (int i = 0; i != NCHAN_PER_MODULE; ++i) {
        if (badChannelMask[i]) {
            LOG(logDEBUG1, ("[%d]:0x%02x\n", i, badChannelMask[i]));
        }
    }
    return setTrimbits(detectorChans);
}

int *getBadChannels(int *numChannels) {
    int *retvals = NULL;
    *numChannels = 0;
    for (int i = 0; i != NCHAN_PER_MODULE; i = i + NCOUNTERS) {
        if (badChannelMask[i]) {
            *numChannels += 1;
        }
    }
    if (*numChannels > 0) {
        retvals = malloc(*numChannels * sizeof(int));
        if (retvals == NULL) {
            LOG(logERROR, ("Could not allocate memory to get bad channels\n"));
            *numChannels = -1;
            return NULL;
        }
        memset(retvals, 0, *numChannels * sizeof(int));
        // return only 1 channel for all counters
        int ich = 0;
        for (int i = 0; i != NCHAN_PER_MODULE; i = i + NCOUNTERS) {
            if (badChannelMask[i]) {
                retvals[ich++] = i / NCOUNTERS;
            }
        }
    }
    // debugging
    LOG(logDEBUG1, ("Reading Bad channel list: %d\n", *numChannels));
    for (int i = 0; i != (*numChannels); ++i) {
        LOG(logDEBUG1, ("[%d]: %d\n", i, retvals[i]));
    }
    return retvals;
}

int getTransmissionDelayFrame() {
    return ((bus_r(FMT_CONFIG_REG) & FMT_CONFIG_TXN_DELAY_MSK) >>
            FMT_CONFIG_TXN_DELAY_OFST);
}

int setTransmissionDelayFrame(int value) {
    if (value < 0 || value > MAX_TIMESLOT_VAL) {
        LOG(logERROR, ("Transmission delay %d should be in range: 0 - %d\n",
                       value, MAX_TIMESLOT_VAL));
        return FAIL;
    }
    LOG(logINFO, ("Setting transmission delay: %d\n", value));
    uint32_t addr = FMT_CONFIG_REG;
    bus_w(addr, bus_r(addr) & ~FMT_CONFIG_TXN_DELAY_MSK);
    bus_w(addr, (bus_r(addr) | ((value << FMT_CONFIG_TXN_DELAY_OFST) &
                                FMT_CONFIG_TXN_DELAY_MSK)));
    LOG(logDEBUG1, ("Transmission delay read %d\n",
                    ((bus_r(addr) & FMT_CONFIG_TXN_DELAY_MSK) >>
                     FMT_CONFIG_TXN_DELAY_OFST)));
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

    const int64_t periodNs = getPeriod();
    const int numFrames = (getNumFrames() * getNumTriggers());
    const int64_t expUs = getGatePeriod() / 1000;

    const int imageSize = calculateDataBytes();
    const int tgEnable = enableTenGigabitEthernet(-1);
    int dr = 0;
    getDynamicRange(&dr);
    int ncounters = __builtin_popcount(getCounterMask());
    int dataSize = 0;
    int packetsPerFrame = 0;
    // 10g
    if (tgEnable) {
        packetsPerFrame = 1;
        if (dr == 32 && ncounters > 1) {
            packetsPerFrame = 2;
        }
        dataSize = imageSize / packetsPerFrame;
    }
    // 1g
    else {
        dataSize = 1280;
        if (ncounters == 3) {
            dataSize = 768;
        }
        packetsPerFrame = imageSize / dataSize;
    }
    const int packetSize = dataSize + sizeof(sls_detector_header);

    LOG(logDEBUG1,
        ("imageSize:%d tg:%d packets/Frame:%d datasize:%d packetSize:%d\n",
         imageSize, tgEnable, packetsPerFrame, dataSize, packetSize));

    // Generate data
    char imageData[imageSize];
    memset(imageData, 0, imageSize);
    {
        const int nchannels = NCHAN_1_COUNTER * NCHIP * ncounters;

        for (int i = 0; i < nchannels; ++i) {
            switch (dr) {
            // case 1: // TODO: Not implemented in firmware yet
            //  break;
            case 8:
                *((uint8_t *)(imageData + i)) = (uint8_t)i;
                break;
            case 16:
                *((uint16_t *)(imageData + i * sizeof(uint16_t))) = (uint16_t)i;
                break;
            case 32:
                *((uint32_t *)(imageData + i * sizeof(uint32_t))) =
                    ((uint32_t)i & 0xFFFFFF); // 24 bit
                break;
            default:
                break;
            }
        }
    }

    // Send data
    int iRxEntry = firstDest;
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
            sls_detector_header *header = (sls_detector_header *)(packetData);
            header->detType = (uint16_t)myDetectorType;
            header->version = SLS_DETECTOR_HEADER_VERSION;
            header->frameNumber = virtual_currentFrameNumber;
            header->packetNumber = i;
            header->modId = virtual_moduleid;
            header->row = detPos[Y];
            header->column = detPos[X];

            // fill data
            memcpy(packetData + sizeof(sls_detector_header),
                   imageData + srcOffset, dataSize);
            srcOffset += dataSize;
            sendUDPPacket(iRxEntry, 0, packetData, packetSize);
        }
        LOG(logINFO,
            ("Sent frame: %d [%lld] to E%d\n", frameNr,
             (long long unsigned int)virtual_currentFrameNumber, iRxEntry));
        clock_gettime(CLOCK_REALTIME, &end);
        int64_t timeNs =
            ((end.tv_sec - begin.tv_sec) * 1E9 + (end.tv_nsec - begin.tv_nsec));

        // sleep for (period - exptime)
        if (frameNr < numFrames) { // if there is a next frame
            if (periodNs > timeNs) {
                usleep((periodNs - timeNs) / 1000);
            }
        }
        ++virtual_currentFrameNumber;
        ++iRxEntry;
        if (iRxEntry == numUdpDestinations) {
            iRxEntry = 0;
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
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STP_ACQSTN_MSK);
    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    return OK;
}

int softwareTrigger() {
    LOG(logINFO, ("Sending Software Trigger\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STRT_SW_TRIGGER_MSK);
    return OK;
}

int startReadOut() {
    LOG(logINFOBLUE, ("Starting Readout\n"));
#ifdef VIRTUAL
    // cannot set #frames and exptiem temporarily to 1 and 0,
    // because have to set it back after readout (but this is non blocking)
    return startStateMachine();
#endif
    cleanFifos();

    // start readout
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STRT_READOUT_MSK);
    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    usleep(1);
    while (bus_r(ASIC_RDO_STATUS_REG) & ASIC_RDO_STATUS_BUSY_MSK) {
        usleep(1);
    }
    LOG(logINFOBLUE, ("Readout done\n"));
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
        // error from too short exptime in parallel mode
        uint32_t deadtimeReg = bus_r(DEADTIME_CONFIG_REG);
        if ((deadtimeReg & DEADTIME_EARLY_EXP_FIN_ERR_MSK) >>
            DEADTIME_EARLY_EXP_FIN_ERR_OFST) {
            LOG(logERROR, ("Status: ERROR in Dead Time Reg (too short "
                           "exptime) %08x\n",
                           deadtimeReg));
            s = ERROR;
        }
        // stopped or error
        else if (retval & FLOW_STATUS_FIFO_FULL_MSK) {
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

#ifndef VIRTUAL
    int64_t retval = getNumFramesLeft() + 1;
    if (retval > 0) {
        LOG(logINFORED, ("%lld frames left\n", (long long int)retval));
    }
#endif
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

int calculateDataBytes() {
    int numCounters = __builtin_popcount(getCounterMask());
    int dr = 0;
    getDynamicRange(&dr);
    return (NCHAN_1_COUNTER * NCHIP * numCounters * ((double)dr / 8.00));
}

int getTotalNumberOfChannels() {
    return (getNumberOfChannelsPerChip() * getNumberOfChips());
}
int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }

int setChipStatusRegister(int csr) {

    // remember previous run clock
    uint32_t prevRunClk = clkDivider[SYSTEM_C0];

    // set to trimming clock
    if (setClockDividerWithTimeUpdateOption(
            SYSTEM_C0, DEFAULT_TRIMMING_RUN_CLKDIV, 0) == FAIL) {
        LOG(logERROR,
            ("Could not set to trimming clock in order to change CSR\n"));
        return FAIL;
    }

    int iret = OK;
    char cmess[MAX_STR_LENGTH];
    patternParameters *pat = setChipStatusRegisterPattern(csr);
    if (pat == NULL) {
        iret = FAIL;
    } else {
        memset(cmess, 0, MAX_STR_LENGTH);
        iret = loadPattern(cmess, logDEBUG5, pat, "");
        if (iret == OK) {
            startPattern();
            LOG(logINFO, ("CSR is now: 0x%x\n", csr));
        }
        free(pat);
    }

    // set back to previous clock
    if (setClockDividerWithTimeUpdateOption(SYSTEM_C0, prevRunClk, 0) == FAIL) {
        LOG(logERROR,
            ("Could not set to previous run clock after changing CSR\n"));
        return FAIL;
    }

    detectorModules->reg = csr;
    return iret;
}
