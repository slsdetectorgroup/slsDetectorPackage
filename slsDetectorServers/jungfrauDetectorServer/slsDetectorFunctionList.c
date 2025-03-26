// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "clogger.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#include "ALTERA_PLL.h" // pll
#include "LTC2620.h"    // dacs
#include "MAX1932.h"    // hv
#include "common.h"
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h> // usleep
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif
extern int portno;
// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern udpStruct udpDetails[MAX_UDP_DESTINATION];
extern int numUdpDestinations;
extern const enum detectorType myDetectorType;
extern int ignoreConfigFileFlag;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char *cmac, int size, uint64_t mac);
extern void getIpAddressinString(char *cip, uint32_t ip);

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_image_test_mode = 0;
int virtual_moduleid = 0;
#endif

enum detectorSettings thisSettings = UNINITIALIZED;
int highvoltage = 0;
int dacValues[NDAC] = {};
int defaultDacValues[] = DEFAULT_DAC_VALS;
int defaultDacValue_G0[] = SPECIAL_DEFAULT_DYNAMIC_GAIN_VALS;
int defaultDacValue_HG0[] = SPECIAL_DEFAULT_DYNAMICHG0_GAIN_VALS;
int32_t clkPhase[NUM_CLOCKS] = {};
int detPos[4] = {};
int chipConfigured = 0;

uint64_t normal_mode_frames = -1;
uint64_t normal_mode_triggers = -1;

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
    LOG(logINFOBLUE, ("******** Jungfrau Virtual Server *****************\n"));
#else
    LOG(logINFOBLUE, ("************ Jungfrau Server *********************\n"));

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
    uint16_t hsnumber = getHardwareSerialNumber();
    uint32_t ipadd = getDetectorIP();
    uint64_t macadd = getDetectorMAC();
    int64_t fwversion = getFirmwareVersion();
    char swversion[MAX_STR_LENGTH] = {0};
    memset(swversion, 0, MAX_STR_LENGTH);
    getServerVersion(swversion);
    int64_t sw_fw_apiversion = 0;
    uint32_t requiredFirmwareVersion =
        (isHardwareVersion_1_0() ? REQRD_FRMWRE_VRSN_BOARD2
                                 : REQRD_FRMWRE_VRSN);

    if (fwversion >= MIN_REQRD_VRSN_T_RD_API)
        sw_fw_apiversion = getFirmwareAPIVersion();

    LOG(logINFOBLUE,
        ("************ Jungfrau Server *********************\n"
         "Hardware Version:\t\t %s\n"
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
    if (type != JUNGFRAU) {
        LOG(logERROR,
            ("This is not a Jungfrau firmware (read %d, expected %d)\n", type,
             JUNGFRAU));
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
    u_int32_t addr = SET_TRIGGER_DELAY_LSB_REG;
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

#ifdef VIRTUAL
void setTestImageMode(int ival) {
    if (ival >= 0) {
        if (ival == 0) {
            LOG(logINFO, ("Switching off Image Test Mode\n"));
            virtual_image_test_mode = 0;
        } else {
            LOG(logINFO, ("Switching on Image Test Mode\n"));
            virtual_image_test_mode = 1;
        }
    }
}

int getTestImageMode() { return virtual_image_test_mode; }
#endif

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIJUNGFRAU); }

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
    return 0x3;
#endif
    return ((bus_r(MOD_SERIAL_NUM_REG) & HARDWARE_VERSION_NUM_MSK) >>
            HARDWARE_VERSION_NUM_OFST);
}

u_int16_t getHardwareSerialNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(MOD_SERIAL_NUM_REG) & HARDWARE_SERIAL_NUM_MSK) >>
            HARDWARE_SERIAL_NUM_OFST);
}

int isHardwareVersion_1_0() {
    const int hwNumberList[] = HARDWARE_VERSION_NUMBERS;
    return ((getHardwareVersionNumber() == hwNumberList[0]) ? 1 : 0);
}

int getChipVersion() {
    // chip v1.1
    if (bus_r(DAQ_REG) & DAQ_CHIP11_VRSN_MSK) {
        return 11;
    }
    // chip v1.0
    return 10;
}

void setChipVersion(int version) {
    LOG(logINFO,
        ("Setting chip version to %0.1f in FPGA\n", (double)version / 10.0));
    if (version == 11) {
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_CHIP11_VRSN_MSK);
    } else {
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_CHIP11_VRSN_MSK);
    }
}

u_int32_t getDetectorNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return bus_r(MOD_SERIAL_NUM_REG);
}

int getModuleId(int *ret, char *mess) {
    return ((bus_r(MOD_ID_REG) & MOD_ID_MSK) >> MOD_ID_OFST);
}

void setModuleId(int modid) {
    LOG(logINFOBLUE, ("Setting module id in fpga: %d\n", modid));
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
        // chip version is a variable
        if (readConfigFile() == FAIL) {
            initCheckDone = 1;
            return;
        }
#endif
    }
    initCheckDone = 1;
}

/* set up detector */

void setupDetector() {
    LOG(logINFO, ("This Server is for 1 Jungfrau module (500k)\n"));

    for (int i = 0; i < NUM_CLOCKS; ++i) {
        clkPhase[i] = 0;
    }
    chipConfigured = 0;
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

    // altera pll
    ALTERA_PLL_SetDefines(
        PLL_CNTRL_REG, PLL_PARAM_REG, PLL_CNTRL_RCNFG_PRMTR_RST_MSK,
        PLL_CNTRL_WR_PRMTR_MSK, PLL_CNTRL_PLL_RST_MSK, PLL_CNTRL_ADDR_MSK,
        PLL_CNTRL_ADDR_OFST, PLL_CNTRL_DBIT_WR_PRMTR_MSK, DBIT_CLK_INDEX);
    ALTERA_PLL_ResetPLL();

    resetCore();
    resetPeripheral();
    cleanFifos();

    // hv
    MAX1932_SetDefines(SPI_REG, SPI_HV_SRL_CS_OTPT_MSK, SPI_HV_SRL_CLK_OTPT_MSK,
                       SPI_HV_SRL_DGTL_OTPT_MSK, SPI_HV_SRL_DGTL_OTPT_OFST,
                       HIGHVOLTAGE_MIN, HIGHVOLTAGE_MAX);
    MAX1932_Disable();
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);

    // adc
    AD9257_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK,
                      ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK,
                      ADC_SPI_SRL_DT_OTPT_OFST);
    AD9257_Set_Jungfrau_Hardware_Version_1_0(isHardwareVersion_1_0());
    AD9257_Disable();
    AD9257_Configure();

    // dac
    LTC2620_SetDefines(SPI_REG, SPI_DAC_SRL_CS_OTPT_MSK,
                       SPI_DAC_SRL_CLK_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_MSK,
                       SPI_DAC_SRL_DGTL_OTPT_OFST, NDAC, DAC_MIN_MV,
                       DAC_MAX_MV);
    LTC2620_Disable();
    LTC2620_Configure();
    resetToDefaultDacs(0);

    /* Only once at server startup */
    bus_w(DAQ_REG, 0x0);

    LOG(logINFOBLUE, ("Setting Default parameters\n"));

    // get chip version
    if (readConfigFile() == FAIL) {
        return;
    }

    if (updateModuleId() == FAIL) {
        return;
    }

    setReadoutSpeed(HALF_SPEED);
    cleanFifos();
    resetCore();

    alignDeserializer();
    configureASICTimer();
    bus_w(ADC_PORT_INVERT_REG,
          (isHardwareVersion_1_0() ? ADC_PORT_INVERT_BOARD2_VAL
                                   : ADC_PORT_INVERT_VAL));

    initReadoutConfiguration();

    // Initialization of acquistion parameters
    disableCurrentSource();
    setSettings(DEFAULT_SETTINGS);
    setGainMode(DEFAULT_GAINMODE);

    setNumFrames(DEFAULT_NUM_FRAMES);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setExpTime(DEFAULT_EXPTIME);
    setPeriod(DEFAULT_PERIOD);
    setDelayAfterTrigger(DEFAULT_DELAY);
    if (getChipVersion() == 11) {
        selectStoragecellStart(DEFAULT_STRG_CLL_STRT_CHIP11);
    } else {
        setNumAdditionalStorageCells(DEFAULT_NUM_STRG_CLLS);
        selectStoragecellStart(DEFAULT_STRG_CLL_STRT);
        // not applicable for chipv1.1
        setStorageCellDelay(DEFAULT_STRG_CLL_DLY);
    }
    setTiming(DEFAULT_TIMING_MODE);
    setNextFrameNumber(DEFAULT_STARTING_FRAME_NUMBER);

    // temp threshold and reset event
    setTemperatureControl(DEFAULT_TMP_CNTRL);
    setThresholdTemperature(DEFAULT_TMP_THRSHLD);
    setTemperatureEvent(0);
    if (getChipVersion() == 11) {
        setFilterResistor(DEFAULT_FILTER_RESISTOR);
        setNumberOfFilterCells(DEFAULT_FILTER_CELL);
    }
    if (!isHardwareVersion_1_0()) {
        setFlipRows(DEFAULT_FLIP_ROWS);
        setReadNRows(MAX_ROWS_PER_READOUT);
    }
#ifdef VIRTUAL
    // setting pedestalmode depends on previous values
    bus_w(PEDESTAL_MODE_REG,
          bus_r(PEDESTAL_MODE_REG) & ~PEDESTAL_MODE_ENBLE_MSK);
#endif
    setPedestalMode(DEFAULT_PEDESTAL_MODE, DEFAULT_PEDESTAL_FRAMES,
                    DEFAULT_PEDESTAL_LOOPS);
    setTimingInfoDecoder(DEFAULT_TIMING_INFO_DECODER);
    setElectronCollectionMode(DEFAULT_ELECTRON_COLLECTION_MODE);
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
        const int vals_G0[] = SPECIAL_DEFAULT_DYNAMIC_GAIN_VALS;
        for (int i = 0; i < NSPECIALDACS; ++i) {
            defaultDacValue_G0[i] = vals_G0[i];
        }
        const int vals_HG0[] = SPECIAL_DEFAULT_DYNAMICHG0_GAIN_VALS;
        for (int i = 0; i < NSPECIALDACS; ++i) {
            defaultDacValue_HG0[i] = vals_HG0[i];
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
                case GAIN0:
                    value = defaultDacValue_G0[j];
                    break;
                case HIGHGAIN0:
                    value = defaultDacValue_HG0[j];
                    break;
                default:
                    break;
                }
                break;
            }
        }

        // set to defualt
        setDAC((enum DACINDEX)i, value, 0);
        if (dacValues[i] != value) {
            LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n", i,
                           value, dacValues[i]));
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
                case GAIN0:
                    *retval = defaultDacValue_G0[i];
                    return OK;
                case HIGHGAIN0:
                    *retval = defaultDacValue_HG0[i];
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
                case GAIN0:
                    LOG(logINFO, ("Setting Default Dac [%d - %s, gain0]: %d\n",
                                  (int)index, dac_names[index], value));
                    defaultDacValue_G0[i] = value;
                    return OK;
                case HIGHGAIN0:
                    LOG(logINFO,
                        ("Setting Default Dac [%d - %s, highgain0]: %d\n",
                         (int)index, dac_names[index], value));
                    defaultDacValue_HG0[i] = value;
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

    // file doesnt exist (give warning and assume chipv1.0)
    if (access(fname, F_OK) != 0) {
        LOG(logWARNING, ("Could not find config file. Assuming chipv1.0\n"));
        return OK;
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

        // chipversion command
        if (!strncmp(line, "chipversion", strlen("chipversion"))) {
            int version = 0;

            // cannot scan values
            if (sscanf(line, "%s %d", command, &version) != 2) {
                sprintf(
                    initErrorMessage,
                    "Could not scan chipversion commands from on-board server "
                    "config file. Line:[%s].\n",
                    line);
                break;
            }
            // validations
            if (version != 10 && version != 11) {
                sprintf(initErrorMessage,
                        "Could not set chip version from on-board server "
                        "config file. Invalid chip version %d. Line:[%s].\n",
                        version, line);
                break;
            }
            // chipversion 1.1 and HW 1.0 is incompatible
            if (version == 11 && isHardwareVersion_1_0()) {
                strcpy(initErrorMessage,
                       "Chip version 1.1 (from on-board config file) is "
                       "incompatible with hardware version v1.0. Please update "
                       "board or correct on-board config file.\n");
                break;
            }

            setChipVersion(version);
        }

        // other commands
        else {
            sprintf(initErrorMessage,
                    "Could not scan command from on-board server "
                    "config file. Line:[%s].\n",
                    line);
            break;
        }

        memset(line, 0, LZ);
    }
    fclose(fd);

    if (strlen(initErrorMessage)) {
        initError = FAIL;
        LOG(logERROR, ("%s\n\n", initErrorMessage));
    } else {
        LOG(logINFOBLUE, ("Successfully read config file\n"));
    }
    return initError;
}

/* firmware functions (resets) */

void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Clearing Acquisition Fifos\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_ACQ_FIFO_CLR_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_ACQ_FIFO_CLR_MSK);
}

void resetCore() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Resetting Core\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CORE_RST_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CORE_RST_MSK);
    usleep(1000 * 1000);
}

void resetPeripheral() {
#ifdef VIRTUAL
    return;
#endif
    LOG(logINFO, ("Resetting Peripheral\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PERIPHERAL_RST_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PERIPHERAL_RST_MSK);
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

void setADCInvertRegister(uint32_t val) {
    LOG(logINFO, ("Setting ADC Port Invert Reg to 0x%x\n", val));
    uint32_t defaultValue =
        (isHardwareVersion_1_0() ? ADC_PORT_INVERT_BOARD2_VAL
                                 : ADC_PORT_INVERT_VAL);
    uint32_t changeValue = defaultValue ^ val;
    LOG(logINFO, ("\t default: 0x%x, final:0x%x\n", defaultValue, changeValue));
    bus_w(ADC_PORT_INVERT_REG, changeValue);
}

uint32_t getADCInvertRegister() {
    uint32_t readValue = bus_r(ADC_PORT_INVERT_REG);
    int32_t defaultValue = (isHardwareVersion_1_0() ? ADC_PORT_INVERT_BOARD2_VAL
                                                    : ADC_PORT_INVERT_VAL);
    uint32_t val = defaultValue ^ readValue;
    LOG(logDEBUG1, ("\tread:0x%x, default:0x%x returned:0x%x\n", readValue,
                    defaultValue, val));
    return val;
}

/* parameters - timer */
int selectStoragecellStart(int pos) {
    int value = pos;
    uint32_t addr = DAQ_REG;
    uint32_t mask = DAQ_STRG_CELL_SLCT_MSK;
    int offset = DAQ_STRG_CELL_SLCT_OFST;
    if (getChipVersion() == 11) {
        // set the bit
        value = 1 << pos;
        addr = CONFIG_V11_REG;
        mask = CONFIG_V11_STRG_CLL_MSK;
        offset = CONFIG_V11_STRG_CLL_OFST;
    }
    if (pos >= 0) {
        LOG(logINFO, ("Setting storage cell start: %d\n", pos));
        bus_w(addr, bus_r(addr) & ~mask);
        bus_w(addr, bus_r(addr) | ((value << offset) & mask));
        // should not do a get to verify (status register does not update
        // immediately during acquisition)
        if (getChipVersion() == 11) {
            return pos;
        }
    }

    // read value back
    // chipv1.1, writing and reading registers are different
#ifndef VIRTUAL
    if (getChipVersion() == 11) {
        addr = CONFIG_V11_STATUS_REG;
        mask = CONFIG_V11_STATUS_STRG_CLL_MSK;
        offset = CONFIG_V11_STATUS_STRG_CLL_OFST;
    }
#endif
    uint32_t regval = bus_r(addr);
#ifndef VIRTUAL
    // flip all contents of register //TODO FIRMWARE FIX
    if (getChipVersion() == 11) {
        regval ^= BIT32_MASK;
    }
#endif
    uint32_t retval = ((regval & mask) >> offset);
    if (getChipVersion() == 11) {
        // get which bit
        int max = getMaxStoragecellStart();
        for (int i = 0; i != max + 1; ++i) {
            if (retval & (1 << i)) {
                return i;
            }
        }
    }
    // chip v1.0
    return retval;
}

int getMaxStoragecellStart() {
    if (getChipVersion() == 11) {
        return MAX_STORAGE_CELL_CHIP11_VAL;
    } else {
        return MAX_STORAGE_CELL_VAL;
    }
}

int setNextFrameNumber(uint64_t value) {
    LOG(logINFO,
        ("Setting next frame number: %llu\n", (long long unsigned int)value));
#ifdef VIRTUAL
    setU64BitReg(value, FRAME_NUMBER_LSB_REG, FRAME_NUMBER_MSB_REG);
#else
    // decrement is for firmware
    setU64BitReg(value - 1, FRAME_NUMBER_LSB_REG, FRAME_NUMBER_MSB_REG);
    // need to set it twice for the firmware to catch
    setU64BitReg(value - 1, FRAME_NUMBER_LSB_REG, FRAME_NUMBER_MSB_REG);
#endif
    return OK;
}

int getNextFrameNumber(uint64_t *retval) {
#ifdef VIRTUAL
    *retval = getU64BitReg(FRAME_NUMBER_LSB_REG, FRAME_NUMBER_MSB_REG);
#else
    // increment is for firmware
    *retval =
        (getU64BitReg(GET_FRAME_NUMBER_LSB_REG, GET_FRAME_NUMBER_MSB_REG) + 1);
#endif
    return OK;
}

void setNumFrames(int64_t val) {
    if (getPedestalMode()) {
        return;
    }
    if (val > 0) {
        LOG(logINFO, ("Setting number of frames %lld\n", (long long int)val));
        set64BitReg(val, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
    }
}

int64_t getNumFrames() {
    return get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}

void setNumTriggers(int64_t val) {
    if (getPedestalMode()) {
        return;
    }
    if (val > 0) {
        LOG(logINFO, ("Setting number of triggers %lld\n", (long long int)val));
        set64BitReg(val, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
    }
}

int64_t getNumTriggers() {
    return get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
}

int setExpTime(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid exptime: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
    val *= (1E-3 * CLK_RUN);
    val -= ACQ_TIME_MIN_CLOCK;
    if (val < 0) {
        val = 0;
    }
    set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG);

    // validate for tolerance
    val += ACQ_TIME_MIN_CLOCK;
    int64_t retval = getExpTime();
    val /= (1E-3 * CLK_RUN);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getExpTime() {
    return (get64BitReg(SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) +
            ACQ_TIME_MIN_CLOCK) /
           (1E-3 * CLK_RUN);
}

int setPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid period: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    val *= (1E-3 * CLK_SYNC);
    set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-3 * CLK_SYNC);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return get64BitReg(SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
           (1E-3 * CLK_SYNC);
}

int setDelayAfterTrigger(int64_t val) {
    if (val < 0) {
        LOG(logERROR,
            ("Invalid delay after trigger: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting delay after trigger %lld ns\n", (long long int)val));
    val *= (1E-3 * CLK_SYNC);
    set64BitReg(val, SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG);

    // validate for tolerance
    int64_t retval = getDelayAfterTrigger();
    val /= (1E-3 * CLK_SYNC);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getDelayAfterTrigger() {
    return get64BitReg(SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG) /
           (1E-3 * CLK_SYNC);
}

void setNumAdditionalStorageCells(int val) {
    if (val >= 0) {
        LOG(logINFO, ("Setting number of addl. storage cells %d\n", val));
        bus_w(CONTROL_REG,
              (bus_r(CONTROL_REG) & ~CONTROL_STORAGE_CELL_NUM_MSK) |
                  ((val << CONTROL_STORAGE_CELL_NUM_OFST) &
                   CONTROL_STORAGE_CELL_NUM_MSK));
    }
}

int getNumAdditionalStorageCells() {
    return ((bus_r(CONTROL_REG) & CONTROL_STORAGE_CELL_NUM_MSK) >>
            CONTROL_STORAGE_CELL_NUM_OFST);
}

int setStorageCellDelay(int64_t val) {
    if (val < 0) {
        LOG(logERROR,
            ("Invalid delay after trigger: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting storage cell delay %lld ns\n", (long long int)val));
    val *= (1E-3 * CLK_RUN);
    bus_w(ASIC_CTRL_REG,
          (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_EXPSRE_TMR_MSK) |
              ((val << ASIC_CTRL_EXPSRE_TMR_OFST) & ASIC_CTRL_EXPSRE_TMR_MSK));

    // validate for tolerance
    int64_t retval = getStorageCellDelay();
    val /= (1E-3 * CLK_RUN);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getStorageCellDelay() {
    return (((int64_t)((bus_r(ASIC_CTRL_REG) & ASIC_CTRL_EXPSRE_TMR_MSK) >>
                       ASIC_CTRL_EXPSRE_TMR_OFST)) /
            (1E-3 * CLK_RUN));
}

int64_t getNumFramesLeft() {
    return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t getNumTriggersLeft() {
    return get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
}

int64_t getPeriodLeft() {
    return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) /
           (1E-3 * CLK_SYNC);
}

int64_t getDelayAfterTriggerLeft() {
    return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) /
           (1E-3 * CLK_SYNC);
}

int64_t getFramesFromStart() {
    return get64BitReg(FRAMES_FROM_START_LSB_REG, FRAMES_FROM_START_MSB_REG);
}

int64_t getActualTime() {
    return get64BitReg(TIME_FROM_START_LSB_REG, TIME_FROM_START_MSB_REG) /
           (1E-3 * CLK_SYNC);
}

int64_t getMeasurementTime() {
    return get64BitReg(START_FRAME_TIME_LSB_REG, START_FRAME_TIME_MSB_REG) /
           (1E-3 * CLK_SYNC);
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

    int *dacVals = NULL;
    // set settings
    switch (sett) {
    case GAIN0:
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_HIGH_GAIN_MSK);
        LOG(logINFO,
            ("Set settings - Gain 0 [DAQ Reg:0x%x]\n", bus_r(DAQ_REG)));
        dacVals = defaultDacValue_G0;
        break;
    case HIGHGAIN0:
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_HIGH_GAIN_MSK);
        LOG(logINFO,
            ("Set settings - High Gain 0 [DAQ Reg:0x%x]\n", bus_r(DAQ_REG)));
        dacVals = defaultDacValue_HG0;
        break;
    default:
        LOG(logERROR, ("This settings %d is not defined\n", (int)sett));
        return -1;
    }

    thisSettings = sett;

    // set special dacs
    const int specialDacs[] = SPECIALDACINDEX;
    for (int i = 0; i < NSPECIALDACS; ++i) {
        setDAC(specialDacs[i], dacVals[i], 0);
    }

    // if chipv1.1 and powered on
    configureChip();

    return getSettings();
}

enum detectorSettings getSettings() {
    if (bus_r(DAQ_REG) & DAQ_HIGH_GAIN_MSK)
        return HIGHGAIN0;
    return GAIN0;
}

enum gainMode getGainMode() {
    uint32_t regval = bus_r(DAQ_REG);
    uint32_t retval_force = regval & DAQ_FRCE_SWTCH_GAIN_MSK;
    uint32_t retval_fix = regval & DAQ_FIX_GAIN_MSK;
    uint32_t retval_cmp_rst = regval & DAQ_CMP_RST_MSK;

    // only one set should be valid
    if ((retval_force && retval_fix) || (retval_fix && retval_cmp_rst) ||
        (retval_force && retval_cmp_rst)) {
        LOG(logERROR, ("undefined gain mode. DAQ reg: 0x%x\n", regval));
    }

    // dynamic gain, when nothing is set
    if (retval_force == 0 && retval_fix == 0 && retval_cmp_rst == 0) {
        return DYNAMIC;
    }

    switch (retval_force) {
    case DAQ_FRCE_GAIN_STG_1_VAL:
        return FORCE_SWITCH_G1;
    case DAQ_FRCE_GAIN_STG_2_VAL:
        return FORCE_SWITCH_G2;
    default:
        break;
    }

    switch (retval_fix) {
    case DAQ_FIX_GAIN_STG_1_VAL:
        return FIX_G1;
    case DAQ_FIX_GAIN_STG_2_VAL:
        return FIX_G2;
    default:
        break;
    }

    if (retval_cmp_rst) {
        return FIX_G0;
    }

    LOG(logERROR, ("This gain mode is undefined [DAQ reg: %d]\n", regval));
    return -1;
}

void setGainMode(enum gainMode mode) {
    uint32_t addr = DAQ_REG;
    uint32_t value = bus_r(addr);

    switch (mode) {
    case DYNAMIC:
        value &= ~(DAQ_GAIN_MODE_MASK);
        bus_w(addr, value);
        LOG(logINFO,
            ("Set gain mode - Dynamic Gain [DAQ Reg:0x%x]\n", bus_r(DAQ_REG)));
        break;
    case FORCE_SWITCH_G1:
        value &= ~(DAQ_GAIN_MODE_MASK);
        value |= DAQ_FRCE_GAIN_STG_1_VAL;
        bus_w(addr, value);
        LOG(logINFO, ("Set gain mode - Force Switch G1 [DAQ Reg:0x%x]\n",
                      bus_r(DAQ_REG)));
        break;
    case FORCE_SWITCH_G2:
        value &= ~(DAQ_GAIN_MODE_MASK);
        value |= DAQ_FRCE_GAIN_STG_2_VAL;
        bus_w(addr, value);
        LOG(logINFO, ("Set gain mode - Force Switch G2 [DAQ Reg:0x%x]\n",
                      bus_r(DAQ_REG)));
        break;
    case FIX_G1:
        value &= ~(DAQ_GAIN_MODE_MASK);
        value |= DAQ_FIX_GAIN_STG_1_VAL;
        bus_w(addr, value);
        LOG(logINFO,
            ("Set gain mode - Fix G1 [DAQ Reg:0x%x]\n", bus_r(DAQ_REG)));
        break;
    case FIX_G2:
        value &= ~(DAQ_GAIN_MODE_MASK);
        value |= DAQ_FIX_GAIN_STG_2_VAL;
        bus_w(addr, value);
        LOG(logINFO,
            ("Set gain mode - Fix G2 [DAQ Reg:0x%x]\n", bus_r(DAQ_REG)));
        break;
    case FIX_G0:
        value &= ~(DAQ_GAIN_MODE_MASK);
        value |= DAQ_CMP_RST_MSK;
        bus_w(addr, value);
        LOG(logINFO,
            ("Set gain mode - Fix G0 [DAQ Reg:0x%x]\n", bus_r(DAQ_REG)));
        break;
    default:
        LOG(logERROR, ("This gain mode %d is not defined\n", (int)mode));
    }
}

/* parameters - dac, adc, hv */
void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0)
        return;

    char *dac_names[] = {DAC_NAMES};
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
    LOG(logINFO, ("Setting DAC %s\n", dac_names[ind]));
    if (LTC2620_SetDACValue((int)ind, val, mV, &dacval) == OK) {
        dacValues[ind] = dacval;
        if (ind == J_VREF_COMP &&
            (val >= 0)) { // FIXME: if val == pwr down value, write 0?
            bus_w(EXT_DAQ_CTRL_REG,
                  (bus_r(EXT_DAQ_CTRL_REG) &
                   ~(EXT_DAQ_CTRL_VREF_COMP_MSK)) // reset
                      | ((val << EXT_DAQ_CTRL_VREF_COMP_OFST) &
                         EXT_DAQ_CTRL_VREF_COMP_MSK)); // or it with value
        }
    }
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
    u_int32_t addr = GET_TEMPERATURE_TMP112_REG;
    uint32_t regvalue = bus_r(addr);
    uint32_t value = regvalue & TEMPERATURE_VALUE_MSK;
    double retval = value;

    // negative
    if (regvalue & TEMPERATURE_POLARITY_MSK) {
        // 2s complement
        int ret = (~value) + 1;
        // attach negative sign
        ret = 0 - value;
        retval = ret;
    }

    // conversion
    retval *= 625.0 / 10.0;
    LOG(logINFO, ("Temperature %s: %f °C\n", tempnames[ind], retval / 1000.00));
    return retval;
}

int setHighVoltage(int val) {
    // setting hv
    if (val >= 0) {
        LOG(logINFO, ("Setting High voltage: %d V", val));
        MAX1932_Set(&val);
        highvoltage = val;
    }
    return highvoltage;
}

/* parameters - timing, extsig */

int setMaster(enum MASTERINDEX m) {
    char *master_names[] = {MASTER_NAMES};
    LOG(logINFOBLUE, ("Setting up as %s in (%s server)\n", master_names[m],
                      (isControlServer ? "control" : "stop")));

    int prevSync = getSynchronization();
    setSynchronization(0);
    int retval = -1;
    int retMaster = OK;
    switch (m) {
    case OW_MASTER:
        bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_MASTER_MSK);
        isMaster(&retval);
        if (retval != 1) {
            LOG(logERROR, ("Could not set master\n"));
            retMaster = FAIL;
        }
        break;
    case OW_SLAVE:
        bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_MASTER_MSK);
        isMaster(&retval);
        if (retval != 0) {
            LOG(logERROR, ("Could not set slave\n"));
            retMaster = FAIL;
        }
        break;
    default:
        LOG(logERROR, ("Cannot reset to hardware settings from client. Restart "
                       "detector server.\n"));
        retMaster = FAIL;
    }
    setSynchronization(prevSync);
    return retMaster;
}

int isMaster(int *retval) {
    *retval =
        ((bus_r(CONTROL_REG) & CONTROL_MASTER_MSK) >> CONTROL_MASTER_OFST);
    return OK;
}

int getSynchronization() {
    return ((bus_r(EXT_SIGNAL_REG) & EXT_SYNC_MSK) >> EXT_SYNC_OFST);
}

void setSynchronization(int enable) {
    LOG(logINFOBLUE,
        ("%s Synchronization\n", (enable ? "Enabling" : "Disabling")));
    if (enable)
        bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) | EXT_SYNC_MSK);
    else
        bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) & ~EXT_SYNC_MSK);
}

void setTiming(enum timingMode arg) {
    if (getPedestalMode()) {
        return;
    }
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
    if ((bus_r(EXT_SIGNAL_REG) & EXT_SIGNAL_MSK) >> EXT_SIGNAL_OFST)
        return TRIGGER_EXPOSURE;
    return AUTO_TIMING;
}

/* configure mac */
void setNumberofUDPInterfaces(int val) {
    uint32_t addr = CONFIG_REG;

    // enable 2 interfaces
    if (val > 1) {
        LOG(logINFOBLUE, ("Setting #Interfaces: 2\n"));
        bus_w(addr, bus_r(addr) | CONFIG_OPRTN_MDE_2_X_10GbE_MSK);
    }
    // enable only 1 interface
    else {
        LOG(logINFOBLUE, ("Setting #Interfaces: 1\n"));
        bus_w(addr, bus_r(addr) & ~CONFIG_OPRTN_MDE_2_X_10GbE_MSK);
    }
    LOG(logDEBUG, ("config reg:0x%x\n", bus_r(addr)));
}

int getNumberofUDPInterfaces() {
    LOG(logDEBUG, ("config reg:0x%x\n", bus_r(CONFIG_REG)));
    // return 2 if enabled, else 1
    return ((bus_r(CONFIG_REG) & CONFIG_OPRTN_MDE_2_X_10GbE_MSK) ? 2 : 1);
}

int getNumberofDestinations(int *retval) {
    *retval = (((bus_r(CONTROL_REG) & CONTROL_RX_ADDTNL_ENDPTS_NUM_MSK) >>
                CONTROL_RX_ADDTNL_ENDPTS_NUM_OFST) +
               1);
    return OK;
}

int setNumberofDestinations(int value) {
    LOG(logINFO, ("Setting number of entries to %d\n", value));
    --value;
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_RX_ADDTNL_ENDPTS_NUM_MSK);
    bus_w(CONTROL_REG,
          bus_r(CONTROL_REG) | ((value << CONTROL_RX_ADDTNL_ENDPTS_NUM_OFST) &
                                CONTROL_RX_ADDTNL_ENDPTS_NUM_MSK));
    return OK;
}

int getFirstUDPDestination() {
    return ((bus_r(CONTROL_REG) & CONTROL_RX_ENDPTS_START_MSK) >>
            CONTROL_RX_ENDPTS_START_OFST);
}

void setFirstUDPDestination(int value) {
    LOG(logINFO, ("Setting first entry to %d\n", value));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_RX_ENDPTS_START_MSK);
    bus_w(CONTROL_REG,
          bus_r(CONTROL_REG) | ((value << CONTROL_RX_ENDPTS_START_OFST) &
                                CONTROL_RX_ENDPTS_START_MSK));
}

void selectPrimaryInterface(int val) {
    uint32_t addr = CONFIG_REG;

    // outer (user input: 0)
    if (val == 0) {
        LOG(logINFOBLUE, ("Setting Primary Interface: 0 (Outer)\n"));
        bus_w(addr, bus_r(addr) & ~CONFIG_INNR_PRIMRY_INTRFCE_MSK);
    }
    // inner (user input: 1)
    else {
        LOG(logINFOBLUE, ("Setting Secondary Interface: 1 (Inner)\n"));
        bus_w(addr, bus_r(addr) | CONFIG_INNR_PRIMRY_INTRFCE_MSK);
    }
}

int getPrimaryInterface() {
    return ((bus_r(CONFIG_REG) & CONFIG_INNR_PRIMRY_INTRFCE_MSK) ? 1 : 0);
}

void setupHeader(int iRxEntry, enum interfaceType type, uint32_t destip,
                 uint64_t destmac, uint16_t destport, uint64_t sourcemac,
                 uint32_t sourceip, uint16_t sourceport) {

    // start addr
    uint32_t addr = (type == INNER ? RXR_ENDPOINT_INNER_START_REG
                                   : RXR_ENDPOINT_OUTER_START_REG);
    // calculate rxr endpoint offset
    addr += (iRxEntry * RXR_ENDPOINT_OFST);
    // get struct memory
    udp_header *udp = (udp_header *)(Blackfin_getBaseAddress() + addr / 2);
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

    int numInterfaces = getNumberofUDPInterfaces();
    int selInterface = getPrimaryInterface();
    LOG(logINFO, ("\t#Interfaces : %d\n", numInterfaces));
    LOG(logINFO, ("\tInterface   : %d %s\n\n", selInterface,
                  (selInterface ? "Inner" : "Outer")));

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

        if (iRxEntry < numUdpDestinations) {
            LOG(logINFOBLUE, ("\tEntry %d\n", iRxEntry));

            LOG(logINFO,
                ("\tOuter %s\n", (numInterfaces == 2)
                                     ? "(Bottom)"
                                     : (selInterface ? "Not Used" : "Used")));
            LOG(logINFO, ("\tSource IP   : %s\n"
                          "\tSource MAC  : %s\n"
                          "\tSource Port : %hu\n"
                          "\tDest IP     : %s\n"
                          "\tDest MAC    : %s\n"
                          "\tDest Port   : %hu\n\n",
                          src_ip, src_mac, srcport, dst_ip, dst_mac, dstport));

            LOG(logINFO,
                ("\tInner %s\n", (numInterfaces == 2)
                                     ? "(Top)"
                                     : (selInterface ? "Used" : "Not Used")));
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
                           "interface 1 [entry:%d] \n",
                           iRxEntry));
            return FAIL;
        }
        if (numInterfaces == 2 &&
            setUDPDestinationDetails(iRxEntry, 1, dst_ip2, dstport2) == FAIL) {
            LOG(logERROR, ("could not set udp destination IP and port for "
                           "interface 2 [entry:%d]\n",
                           iRxEntry));
            return FAIL;
        }
#endif
        if (numInterfaces == 2) {
            // bottom
            setupHeader(iRxEntry, OUTER, dstip, dstmac, dstport, srcmac, srcip,
                        srcport);
            // top
            setupHeader(iRxEntry, INNER, dstip2, dstmac2, dstport2, srcmac2,
                        srcip2, srcport2);
        }
        // single interface
        else {
            // default
            if (selInterface == 0) {
                setupHeader(iRxEntry, OUTER, dstip, dstmac, dstport, srcmac,
                            srcip, srcport);
            } else {
                setupHeader(iRxEntry, INNER, dstip, dstmac, dstport, srcmac,
                            srcip, srcport2);
            }
        }
    }
    setNumberofUDPInterfaces(numInterfaces);
    selectPrimaryInterface(selInterface);

    cleanFifos();
    resetCore();
    alignDeserializer();
    return OK;
}

int setDetectorPosition(int pos[]) {
    int ret = OK;
    // row, col
    uint32_t innerPos[2] = {pos[X], pos[Y]};
    uint32_t outerPos[2] = {pos[X], pos[Y]};
    int selInterface = getPrimaryInterface();

    if (getNumberofUDPInterfaces() == 1) {
        LOG(logDEBUG,
            ("Setting detector position: 1 Interface %s \n(%d, %d)\n",
             (selInterface ? "Inner" : "Outer"), innerPos[X], innerPos[Y]));
    } else {
        // top has row incremented by 1
        ++innerPos[Y];
        LOG(logDEBUG, ("Setting detector position: 2 Interfaces \n"
                       "  inner top(%d, %d), outer bottom(%d, %d)\n",
                       innerPos[X], innerPos[Y], outerPos[X], outerPos[Y]));
    }
    detPos[0] = innerPos[X];
    detPos[1] = innerPos[Y];
    detPos[2] = outerPos[X];
    detPos[3] = outerPos[Y];

    // row [Y]
    // outer
    uint32_t addr = COORD_ROW_REG;
    bus_w(addr,
          (bus_r(addr) & ~COORD_ROW_OUTER_MSK) |
              ((outerPos[Y] << COORD_ROW_OUTER_OFST) & COORD_ROW_OUTER_MSK));
    if (((bus_r(addr) & COORD_ROW_OUTER_MSK) >> COORD_ROW_OUTER_OFST) !=
        outerPos[Y])
        ret = FAIL;
    // inner
    bus_w(addr,
          (bus_r(addr) & ~COORD_ROW_INNER_MSK) |
              ((innerPos[Y] << COORD_ROW_INNER_OFST) & COORD_ROW_INNER_MSK));
    if (((bus_r(addr) & COORD_ROW_INNER_MSK) >> COORD_ROW_INNER_OFST) !=
        innerPos[Y])
        ret = FAIL;

    // col [X]
    // outer
    addr = COORD_COL_REG;
    bus_w(addr,
          (bus_r(addr) & ~COORD_COL_OUTER_MSK) |
              ((outerPos[X] << COORD_COL_OUTER_OFST) & COORD_COL_OUTER_MSK));
    if (((bus_r(addr) & COORD_COL_OUTER_MSK) >> COORD_COL_OUTER_OFST) !=
        outerPos[X])
        ret = FAIL;
    // inner
    bus_w(addr,
          (bus_r(addr) & ~COORD_COL_INNER_MSK) |
              ((innerPos[X] << COORD_COL_INNER_OFST) & COORD_COL_INNER_MSK));
    if (((bus_r(addr) & COORD_COL_INNER_MSK) >> COORD_COL_INNER_OFST) !=
        innerPos[X])
        ret = FAIL;

    if (ret == OK) {
        if (getNumberofUDPInterfaces() == 1) {
            LOG(logINFOBLUE, ("Position set to [%d, %d] #(col, row)\n",
                              innerPos[X], innerPos[Y]));
        } else {
            LOG(logINFOBLUE, (" Inner (top) position set to [%d, %d]\n",
                              innerPos[X], innerPos[Y]));
            LOG(logINFOBLUE, (" Outer (bottom) position set to [%d, %d]\n",
                              outerPos[X], outerPos[Y]));
        }
    }
    return ret;
}

int *getDetectorPosition() { return detPos; }

/* jungfrau specific - powerchip, autocompdisable, asictimer, clockdiv, pll,
 * flashing fpga */

int setReadNRows(int value) {
    if (value < 0 || (value % READ_N_ROWS_MULTIPLE != 0)) {
        LOG(logERROR, ("Invalid number of rows %d\n", value));
        return FAIL;
    }
    if (isHardwareVersion_1_0()) {
        LOG(logERROR, ("Could not set number of rows. Only available for "
                       "Hardware Board version v2.0.\n"));
        return FAIL;
    }

    // regval is numpackets - 1
    int regval = (value / READ_N_ROWS_MULTIPLE) - 1;
    uint32_t addr = READ_N_ROWS_REG;
    LOG(logINFO, ("Setting number of rows: %d (regval:%d)\n", value, regval));
    bus_w(addr, bus_r(addr) & ~READ_N_ROWS_NUM_ROWS_MSK);
    bus_w(addr, bus_r(addr) | ((regval << READ_N_ROWS_NUM_ROWS_OFST) &
                               READ_N_ROWS_NUM_ROWS_MSK));
    if (value == MAX_ROWS_PER_READOUT) {
        LOG(logINFO, ("Disabling Partial Readout (#rows)\n"));
        bus_w(addr, bus_r(addr) & ~READ_N_ROWS_ENBL_MSK);
    } else {
        LOG(logINFO, ("Enabling Partial Readout (#rows)\n"));
        bus_w(addr, bus_r(addr) | READ_N_ROWS_ENBL_MSK);
    }
    return OK;
}

int getReadNRows() {
    // cannot set it in old board
    if (isHardwareVersion_1_0()) {
        return MAX_ROWS_PER_READOUT;
    }
    int enable = (bus_r(READ_N_ROWS_REG) & READ_N_ROWS_ENBL_MSK);
    int regval = ((bus_r(READ_N_ROWS_REG) & READ_N_ROWS_NUM_ROWS_MSK) >>
                  READ_N_ROWS_NUM_ROWS_OFST);

    int maxRegval = (MAX_ROWS_PER_READOUT / READ_N_ROWS_MULTIPLE) - 1;
    if ((regval == maxRegval && enable) || (regval != maxRegval && !enable)) {
        return -1;
    }

    return (regval + 1) * READ_N_ROWS_MULTIPLE;
}

void initReadoutConfiguration() {

    LOG(logINFO, ("Initializing Readout Configuration:\n"
                  "\t Reset readout Timer\n"
                  "\t 1 x 10G mode\n"
                  "\t outer interface is primary\n"
                  "\t half speed\n"
                  "\t TDMA disabled, 0 as TDMA slot\n"
                  "\t Ethernet overflow disabled\n"
                  "\t Reset Round robin entries\n"));

    uint32_t val = 0;
    // reset readouttimer
    val &= ~CONFIG_RDT_TMR_MSK;
    // 1 x 10G mode
    val &= ~CONFIG_OPRTN_MDE_2_X_10GbE_MSK;
    // outer interface
    val &= ~CONFIG_INNR_PRIMRY_INTRFCE_MSK;
    // half speed
    val &= ~CONFIG_READOUT_SPEED_MSK;
    val |= CONFIG_HALF_SPEED_20MHZ_VAL;
    // tdma disable
    val &= ~CONFIG_TDMA_ENABLE_MSK;
    // tdma slot 0
    val &= ~CONFIG_TDMA_TIMESLOT_MSK;
    // no ethernet overflow
    val &= ~CONFIG_ETHRNT_FLW_CNTRL_MSK;
    bus_w(CONFIG_REG, val);

    val = bus_r(CONTROL_REG);
    // reset (addtional round robin entry) rx endpoints num
    val &= CONTROL_RX_ADDTNL_ENDPTS_NUM_MSK;
    // reset start of round robin entry to 0
    val &= CONTROL_RX_ENDPTS_START_MSK;
    bus_w(CONTROL_REG, val);
}

int powerChip(int on) {
    if (on != -1) {
        if (on) {
            LOG(logINFOBLUE, ("Powering chip: on\n"));
            bus_w(CHIP_POWER_REG,
                  bus_r(CHIP_POWER_REG) | CHIP_POWER_ENABLE_MSK);

            configureChip();
        } else {
            LOG(logINFOBLUE, ("Powering chip: off\n"));
            bus_w(CHIP_POWER_REG,
                  bus_r(CHIP_POWER_REG) & ~CHIP_POWER_ENABLE_MSK);

            chipConfigured = 0;
        }
    }
#ifdef VIRTUAL
    return ((bus_r(CHIP_POWER_REG) & CHIP_POWER_ENABLE_MSK) >>
            CHIP_POWER_ENABLE_OFST);
#endif
    return ((bus_r(CHIP_POWER_REG) & CHIP_POWER_STATUS_MSK) >>
            CHIP_POWER_STATUS_OFST);
}

int isChipConfigured() { return chipConfigured; }

void configureChip() {
    // only for chipv1.1 and chip is powered on
    if (getChipVersion() == 11 && powerChip(-1)) {
        LOG(logINFOBLUE, ("\tConfiguring chip\n"));

        // waiting 500 ms before configuring selection
        usleep(500 * 1000);

        // write same values to configure selection
        // if (chip was powered off earlier)
        LOG(logINFO, ("\tRewriting values for selection\n"))
        bus_w(CRRNT_SRC_COL_LSB_REG, bus_r(CRRNT_SRC_COL_LSB_REG));
        bus_w(CRRNT_SRC_COL_MSB_REG, bus_r(CRRNT_SRC_COL_MSB_REG));

        // waiting 500 ms before configuring chip
        usleep(500 * 1000);

        // write same register values back to configure chip
        bus_w(CONFIG_V11_REG, bus_r(CONFIG_V11_REG));

        LOG(logINFOBLUE, ("\tChip configured\n"));
        chipConfigured = 1;
    }
}

int autoCompDisable(int on) {
    if (on != -1) {
        if (on) {
            LOG(logINFO, ("Auto comp disable mode: on\n"));
            bus_w(EXT_DAQ_CTRL_REG,
                  bus_r(EXT_DAQ_CTRL_REG) | EXT_DAQ_CTRL_CMP_LGC_ENBL_MSK);
        } else {
            LOG(logINFO, ("Auto comp disable mode: off\n"));
            bus_w(EXT_DAQ_CTRL_REG,
                  bus_r(EXT_DAQ_CTRL_REG) & ~EXT_DAQ_CTRL_CMP_LGC_ENBL_MSK);
        }
    }

    return ((bus_r(EXT_DAQ_CTRL_REG) & EXT_DAQ_CTRL_CMP_LGC_ENBL_MSK) >>
            EXT_DAQ_CTRL_CMP_LGC_ENBL_OFST);
}

int setComparatorDisableTime(int64_t val) {
    if (val < 0) {
        LOG(logERROR,
            ("Invalid comp disable time: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting comp disable time %lld ns\n", (long long int)val));
    val *= (1E-3 * CLK_RUN);
    bus_w(COMP_DSBLE_TIME_REG, val);

    // validate for tolerance
    int64_t retval = getComparatorDisableTime();
    val /= (1E-3 * CLK_RUN);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getComparatorDisableTime() {
    return bus_r(COMP_DSBLE_TIME_REG) / (1E-3 * CLK_RUN);
}

void configureASICTimer() {
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_PRCHRG_TMR_MSK) |
                             ASIC_CTRL_PRCHRG_TMR_VAL);

    uint32_t val = ASIC_CTRL_DS_TMR_VAL;
    if (getChipVersion() == 11) {
        val = ASIC_CTRL_DS_TMR_CHIP1_1_VAL;
    }
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_DS_TMR_MSK) | val);
    LOG(logINFO, ("Configured ASIC Timer [0x%x]\n", bus_r(ASIC_CTRL_REG)));
}

int setReadoutSpeed(int val) {
    // stop state machine if running
    if (runBusy()) {
        stopStateMachine();
    }

    uint32_t adcOfst = 0;
    uint32_t sampleAdcSpeed = 0;
    uint32_t adcPhase = 0;
    uint32_t dbitPhase = 0;
    uint32_t config = CONFIG_FULL_SPEED_40MHZ_VAL;

    switch (val) {

    case FULL_SPEED:
        if (isHardwareVersion_1_0()) {
            LOG(logERROR, ("Cannot set full speed. Should not be here\n"));
            return FAIL;
        }
        LOG(logINFO, ("Setting Full Speed (40 MHz):\n"));
        if (getChipVersion() == 10) {
            sampleAdcSpeed = SAMPLE_ADC_FULL_SPEED_CHIP10;
            adcPhase = ADC_PHASE_FULL_SPEED_CHIP10;
            dbitPhase = DBIT_PHASE_FULL_SPEED_CHIP10;
            adcOfst = ADC_OFST_FULL_SPEED_VAL_CHIP10;
        } else {
            sampleAdcSpeed = SAMPLE_ADC_FULL_SPEED_CHIP11;
            adcPhase = ADC_PHASE_FULL_SPEED_CHIP11;
            dbitPhase = DBIT_PHASE_FULL_SPEED_CHIP11;
            adcOfst = ADC_OFST_FULL_SPEED_VAL_CHIP11;
        }
        config = CONFIG_FULL_SPEED_40MHZ_VAL;
        break;

    case HALF_SPEED:
        LOG(logINFO, ("Setting Half Speed (20 MHz):\n"));
        if (isHardwareVersion_1_0()) {
            adcOfst = ADC_OFST_HALF_SPEED_BOARD2_VAL;
            sampleAdcSpeed = SAMPLE_ADC_HALF_SPEED_BOARD2;
            adcPhase = ADC_PHASE_HALF_SPEED_BOARD2;
            dbitPhase = DBIT_PHASE_HALF_SPEED_BOARD2;
        } else if (getChipVersion() == 10) {
            adcOfst = ADC_OFST_HALF_SPEED_VAL_CHIP10;
            sampleAdcSpeed = SAMPLE_ADC_HALF_SPEED_CHIP10;
            adcPhase = ADC_PHASE_HALF_SPEED_CHIP10;
            dbitPhase = DBIT_PHASE_HALF_SPEED_CHIP10;
        } else {
            adcOfst = ADC_OFST_HALF_SPEED_VAL_CHIP11;
            sampleAdcSpeed = SAMPLE_ADC_HALF_SPEED_CHIP11;
            adcPhase = ADC_PHASE_HALF_SPEED_CHIP11;
            dbitPhase = DBIT_PHASE_HALF_SPEED_CHIP11;
        }
        config = CONFIG_HALF_SPEED_20MHZ_VAL;
        break;

    case QUARTER_SPEED:
        LOG(logINFO, ("Setting Half Speed (10 MHz):\n"));
        if (isHardwareVersion_1_0()) {
            adcOfst = ADC_OFST_QUARTER_SPEED_BOARD2_VAL;
            sampleAdcSpeed = SAMPLE_ADC_QUARTER_SPEED_BOARD2;
            adcPhase = ADC_PHASE_QUARTER_SPEED_BOARD2;
            dbitPhase = DBIT_PHASE_QUARTER_SPEED_BOARD2;
        } else if (getChipVersion() == 10) {
            adcOfst = ADC_OFST_QUARTER_SPEED_VAL_CHIP10;
            sampleAdcSpeed = SAMPLE_ADC_QUARTER_SPEED_CHIP10;
            adcPhase = ADC_PHASE_QUARTER_SPEED_CHIP10;
            dbitPhase = DBIT_PHASE_QUARTER_SPEED_CHIP10;
        } else {
            adcOfst = ADC_OFST_QUARTER_SPEED_VAL_CHIP11;
            sampleAdcSpeed = SAMPLE_ADC_QUARTER_SPEED_CHIP11;
            adcPhase = ADC_PHASE_QUARTER_SPEED_CHIP11;
            dbitPhase = DBIT_PHASE_QUARTER_SPEED_CHIP11;
        }
        config = CONFIG_QUARTER_SPEED_10MHZ_VAL;
        break;

    default:
        LOG(logERROR, ("Unknown speed val %d\n", val));
        return FAIL;
    }

    bus_w(CONFIG_REG, (bus_r(CONFIG_REG) & ~CONFIG_READOUT_SPEED_MSK) | config);
    LOG(logINFO, ("\tSet Config Reg to 0x%x\n", bus_r(CONFIG_REG)));

    bus_w(ADC_OFST_REG, adcOfst);
    LOG(logINFO, ("\tSet ADC Ofst Reg to 0x%x\n", bus_r(ADC_OFST_REG)));

    bus_w(SAMPLE_REG, sampleAdcSpeed);
    LOG(logINFO, ("\tSet Sample Reg to 0x%x\n", bus_r(SAMPLE_REG)));

    setPhase(ADC_CLK, adcPhase, 0);
    LOG(logINFO, ("\tSet ADC Phase Reg to %d\n", adcPhase));

    setPhase(DBIT_CLK, dbitPhase, 0);
    LOG(logINFO, ("\tSet DBIT Phase Reg to %d\n", dbitPhase));

    return OK;
}

int getReadoutSpeed(int *retval) {
    u_int32_t speed = bus_r(CONFIG_REG) & CONFIG_READOUT_SPEED_MSK;
    switch (speed) {
    case CONFIG_FULL_SPEED_40MHZ_VAL:
        *retval = FULL_SPEED;
        break;
    case CONFIG_HALF_SPEED_20MHZ_VAL:
        *retval = HALF_SPEED;
        break;
    case CONFIG_QUARTER_SPEED_10MHZ_VAL:
        *retval = QUARTER_SPEED;
        break;
    default:
        LOG(logERROR, ("Unknown speed val: %d\n", speed));
        *retval = -1;
        return FAIL;
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
    int maxShift = MAX_PHASE_SHIFTS;
    // validation
    if (degrees && (val < 0 || val > 359)) {
        LOG(logERROR, ("\tPhase provided outside limits (0 - 359°C)\n"));
        return FAIL;
    }
    if (!degrees && (val < 0 || val > MAX_PHASE_SHIFTS - 1)) {
        LOG(logERROR,
            ("\tPhase provided outside limits (0 - %d phase shifts)\n",
             maxShift - 1));
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
        LOG(logINFO, ("Nothing to do in Phase Shift\n"));
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

    ALTERA_PLL_SetPhaseShift(
        phase, (ind == ADC_CLK ? ADC_CLK_INDEX : DBIT_CLK_INDEX), 0);

    clkPhase[ind] = valShift;

    alignDeserializer();
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
    ConvertToDifferentRange(0, MAX_PHASE_SHIFTS - 1, 0, 359, clkPhase[ind],
                            &val);
    return val;
}

int getMaxPhase(enum CLKINDEX ind) {
    if (ind != ADC_CLK && ind != DBIT_CLK) {
        LOG(logERROR, ("Unknown clock index %d to get max phase\n", ind));
        return -1;
    }
    return MAX_PHASE_SHIFTS;
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
    LOG(logDEBUG1, ("validating phase in degrees\n"));
    int maxShift = MAX_PHASE_SHIFTS;
    // convert degrees to shift
    int valShift = 0;
    ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
    // convert back to degrees
    ConvertToDifferentRange(0, maxShift - 1, 0, 359, valShift, &val);

    if (val == retval)
        return OK;
    return FAIL;
}

int setThresholdTemperature(int val) {

    if (val >= 0) {
        LOG(logINFO, ("Setting Threshold Temperature: %f °C\n", val / 1000.00));
        val *= (10.0 / 625.0);
        LOG(logDEBUG1, ("Converted Threshold Temperature: %d\n", val));
        bus_w(TEMP_CTRL_REG,
              (bus_r(TEMP_CTRL_REG) & ~(TEMP_CTRL_PROTCT_THRSHLD_MSK) &
               ~(TEMP_CTRL_OVR_TMP_EVNT_MSK)) |
                  (((val << TEMP_CTRL_PROTCT_THRSHLD_OFST) &
                    TEMP_CTRL_PROTCT_THRSHLD_MSK)));
        LOG(logDEBUG1,
            ("Converted Threshold Temperature set to %d\n",
             ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_THRSHLD_MSK) >>
              TEMP_CTRL_PROTCT_THRSHLD_OFST)));
    }
    uint32_t temp = ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_THRSHLD_MSK) >>
                     TEMP_CTRL_PROTCT_THRSHLD_OFST);

    // conversion
    temp = (temp * (625.0 / 10.0));

    double ftemp = (double)temp / 1000.00;
    LOG(logDEBUG1, ("Threshold Temperature read %f °C\n", ftemp));
    return temp;
}

int setTemperatureControl(int val) {
    if (val >= 0) {
        // binary value
        if (val > 0)
            val = 1;
        LOG(logINFO, ("Setting Temperature control: %d\n", val));
        bus_w(TEMP_CTRL_REG,
              (bus_r(TEMP_CTRL_REG) & ~(TEMP_CTRL_PROTCT_ENABLE_MSK) &
               ~(TEMP_CTRL_OVR_TMP_EVNT_MSK)) |
                  (((val << TEMP_CTRL_PROTCT_ENABLE_OFST) &
                    TEMP_CTRL_PROTCT_ENABLE_MSK)));
        LOG(logDEBUG1, ("Temperature control read: %d\n",
                        ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_ENABLE_MSK) >>
                         TEMP_CTRL_PROTCT_ENABLE_OFST)));
    }
    return ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_ENABLE_MSK) >>
            TEMP_CTRL_PROTCT_ENABLE_OFST);
}

int setTemperatureEvent(int val) {
#ifdef VIRTUAL
    return 0;
#endif
    if (val >= 0) {
        // set bit to clear it
        val = 1;
        LOG(logINFO, ("Setting Temperature Event (clearing): %d\n", val));
        bus_w(TEMP_CTRL_REG,
              (bus_r(TEMP_CTRL_REG) & ~TEMP_CTRL_OVR_TMP_EVNT_MSK) |
                  (((val << TEMP_CTRL_OVR_TMP_EVNT_OFST) &
                    TEMP_CTRL_OVR_TMP_EVNT_MSK)));
        LOG(logDEBUG1, ("Temperature Event read %d\n",
                        ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_OVR_TMP_EVNT_MSK) >>
                         TEMP_CTRL_OVR_TMP_EVNT_OFST)));
    }
    return ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_OVR_TMP_EVNT_MSK) >>
            TEMP_CTRL_OVR_TMP_EVNT_OFST);
}

void alignDeserializer() {
    // refresh alignment
    bus_w(ADC_DSRLZR_0_REG,
          bus_r(ADC_DSRLZR_0_REG) | ADC_DSRLZR_0_RFRSH_ALGNMNT_MSK);
    bus_w(ADC_DSRLZR_1_REG,
          bus_r(ADC_DSRLZR_1_REG) | ADC_DSRLZR_1_RFRSH_ALGNMNT_MSK);
    bus_w(ADC_DSRLZR_2_REG,
          bus_r(ADC_DSRLZR_2_REG) | ADC_DSRLZR_2_RFRSH_ALGNMNT_MSK);
    bus_w(ADC_DSRLZR_3_REG,
          bus_r(ADC_DSRLZR_3_REG) | ADC_DSRLZR_3_RFRSH_ALGNMNT_MSK);

    usleep(1 * 1000 * 1000);

    // disable the refresh
    bus_w(ADC_DSRLZR_0_REG,
          bus_r(ADC_DSRLZR_0_REG) & (~(ADC_DSRLZR_0_RFRSH_ALGNMNT_MSK)));
    bus_w(ADC_DSRLZR_1_REG,
          bus_r(ADC_DSRLZR_1_REG) & (~(ADC_DSRLZR_1_RFRSH_ALGNMNT_MSK)));
    bus_w(ADC_DSRLZR_2_REG,
          bus_r(ADC_DSRLZR_2_REG) & (~(ADC_DSRLZR_2_RFRSH_ALGNMNT_MSK)));
    bus_w(ADC_DSRLZR_3_REG,
          bus_r(ADC_DSRLZR_3_REG) & (~(ADC_DSRLZR_3_RFRSH_ALGNMNT_MSK)));
}

int getFlipRows() {
    return ((bus_r(CONFIG_REG) & CONFIG_BOTTOM_INVERT_STREAM_MSK) >>
            CONFIG_BOTTOM_INVERT_STREAM_OFST);
}

void setFlipRows(int arg) {
    if (isHardwareVersion_1_0()) {
        LOG(logERROR, ("Could not set flip rows. Only available for "
                       "Hardware Board version 2.0.\n"));
        return;
    }
    if (arg >= 0) {
        if (arg == 0) {
            LOG(logINFO, ("Switching off bottom row flipping\n"));
            bus_w(CONFIG_REG,
                  bus_r(CONFIG_REG) & ~CONFIG_BOTTOM_INVERT_STREAM_MSK);
        } else {
            LOG(logINFO, ("Switching on bottom row flipping\n"));
            bus_w(CONFIG_REG,
                  bus_r(CONFIG_REG) | CONFIG_BOTTOM_INVERT_STREAM_MSK);
        }
    }
}

int getFilterResistor() {
#ifdef VIRTUAL
    uint32_t addr = CONFIG_V11_REG;
#else
    uint32_t addr = CONFIG_V11_STATUS_REG;
#endif
    // return 0 for lower value, 1 for higher value
    if (bus_r(addr) & CONFIG_V11_STATUS_FLTR_RSSTR_SMLR_MSK) {
        return 0;
    }
    return 1;
}

int setFilterResistor(int value) {
    // lower resistor
    if (value == 0) {
        LOG(logINFO, ("Setting Lower Filter Resistor\n"));
        bus_w(CONFIG_V11_REG,
              bus_r(CONFIG_V11_REG) | CONFIG_V11_FLTR_RSSTR_SMLR_MSK);
        return OK;
    }
    // higher resistor
    else if (value == 1) {
        LOG(logINFO, ("Setting Higher Filter Resistor\n"));
        bus_w(CONFIG_V11_REG,
              bus_r(CONFIG_V11_REG) & ~CONFIG_V11_FLTR_RSSTR_SMLR_MSK);
        return OK;
    }
    LOG(logERROR, ("Could not set Filter Resistor. Invalid value %d\n", value));
    return FAIL;
}

int getNumberOfFilterCells() {
#ifdef VIRTUAL
    uint32_t addr = CONFIG_V11_REG;
#else
    uint32_t addr = CONFIG_V11_STATUS_REG;
#endif
    uint32_t regval = bus_r(addr);
#ifndef VIRTUAL
    // flip all contents of register //TODO FIRMWARE FIX
    regval ^= BIT32_MASK;
#endif
    uint32_t retval =
        (regval & CONFIG_V11_FLTR_CLL_MSK) >> CONFIG_V11_FLTR_CLL_OFST;
    // count number of bits = which icell
    return (__builtin_popcount(retval));
}

void setNumberOfFilterCells(int iCell) {
    if (iCell > MAX_FILTER_CELL_VAL) {
        return;
    }

    uint32_t addr = CONFIG_V11_REG;
    bus_w(addr, bus_r(addr) & ~CONFIG_V11_FLTR_CLL_MSK);

    if (iCell > 0) {
        // enables as many cells
        uint32_t value = 0;
        for (int i = 0; i != iCell; ++i) {
            value |= (1 << i);
        }
        bus_w(addr, bus_r(addr) | ((value << CONFIG_V11_FLTR_CLL_OFST) &
                                   CONFIG_V11_FLTR_CLL_MSK));
    }
    LOG(logINFO, ("Setting Number of Filter Cells to %d [Reg:0x%x]\n", iCell,
                  bus_r(addr)));
}

void disableCurrentSource() {
    LOG(logINFO, ("Disabling Current Source\n"));

    // set default values for current source first
    if (getChipVersion() == 11) {
        LOG(logINFO, ("\tSetting default values for selection\n"))
        bus_w(CRRNT_SRC_COL_LSB_REG, BIT32_MASK);
        bus_w(CRRNT_SRC_COL_MSB_REG, BIT32_MASK);
    }

    bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_CRRNT_SRC_ENBL_MSK);
    LOG(logINFO, ("\tCurrent Source disabled\n"));

    configureChip();
}

void enableCurrentSource(int fix, uint64_t select, int normal) {
    disableCurrentSource();

    if (getChipVersion() == 11) {
        LOG(logINFO, ("Enabling current source [fix:%d, select:0x%lx]\n", fix,
                      (long unsigned int)select));
    } else {
        LOG(logINFO,
            ("Enabling current source [fix:%d, select:%ld, normal:%d]\n", fix,
             (long int)select, normal));
    }
    // fix
    if (fix) {
        LOG(logINFO, ("\tEnabling fix\n"));
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_CRRNT_SRC_CLMN_FIX_MSK);
    } else {
        LOG(logINFO, ("\tDisabling fix\n"));
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_CRRNT_SRC_CLMN_FIX_MSK);
    }
    if (getChipVersion() == 10) {
        // select
        LOG(logINFO, ("\tSetting selection to %ld\n", (long int)select));
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_CRRNT_SRC_CLMN_SLCT_MSK);
        bus_w(DAQ_REG,
              bus_r(DAQ_REG) | ((select << DAQ_CRRNT_SRC_CLMN_SLCT_OFST) &
                                DAQ_CRRNT_SRC_CLMN_SLCT_MSK));

    } else {
        // select
        // invert select first
        uint64_t tmp = select;
        uint64_t inverted = 0;
        for (int i = 0; i != 64; ++i) {
            // get each bit from LSB side
            uint64_t bit = (tmp >> i) & 0x1;
            // push the bit into MSB side
            inverted |= (bit << (63 - i));
        }
        LOG(logINFO, ("\tSetting selection to 0x%lx (inverted from 0x%lx)\n",
                      (long unsigned int)inverted, (long unsigned int)select));
        set64BitReg(inverted, CRRNT_SRC_COL_LSB_REG, CRRNT_SRC_COL_MSB_REG);

        // normal
        if (normal) {
            LOG(logINFO, ("\tEnabling normal\n"))
            bus_w(CONFIG_V11_REG,
                  bus_r(CONFIG_V11_REG) & ~CONFIG_V11_CRRNT_SRC_LOW_MSK);
        } else {
            LOG(logINFO, ("\tEnabling low\n"))
            bus_w(CONFIG_V11_REG,
                  bus_r(CONFIG_V11_REG) | CONFIG_V11_CRRNT_SRC_LOW_MSK);
        }
    }
    // validating before enabling current source
    if (getFixCurrentSource() != fix || getSelectCurrentSource() != select) {
        LOG(logERROR,
            ("Could not set fix or select parameters for current source.\n"))
        return;
    }
    // not validating normal because the status register might not update during
    // acquisition

    // enabling current source
    LOG(logINFO, ("\tEnabling Current Source\n"));
    bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_CRRNT_SRC_ENBL_MSK);

    configureChip();
}

int getCurrentSource() {
    return ((bus_r(DAQ_REG) & DAQ_CRRNT_SRC_ENBL_MSK) >>
            DAQ_CRRNT_SRC_ENBL_OFST);
}

int getFixCurrentSource() {
    return ((bus_r(DAQ_REG) & DAQ_CRRNT_SRC_CLMN_FIX_MSK) >>
            DAQ_CRRNT_SRC_CLMN_FIX_OFST);
}

int getNormalCurrentSource() {
    if (getChipVersion() == 11) {
        int low = ((bus_r(CONFIG_V11_STATUS_REG) &
                    CONFIG_V11_STATUS_CRRNT_SRC_LOW_MSK) >>
                   CONFIG_V11_STATUS_CRRNT_SRC_LOW_OFST);
        return (low == 0 ? 1 : 0);
    }
    return -1;
}

uint64_t getSelectCurrentSource() {
    if (getChipVersion() == 10) {
        return ((bus_r(DAQ_REG) & DAQ_CRRNT_SRC_CLMN_SLCT_MSK) >>
                DAQ_CRRNT_SRC_CLMN_SLCT_OFST);
    } else {
        // invert the select
        uint64_t retval =
            get64BitReg(CRRNT_SRC_COL_LSB_REG, CRRNT_SRC_COL_MSB_REG);

        uint64_t tmp = retval;
        uint64_t inverted = 0;
        for (int i = 0; i != 64; ++i) {
            // get each bit from LSB side
            uint64_t bit = (tmp >> i) & 0x1;
            // push the bit into MSB side
            inverted |= (bit << (63 - i));
        }
        return inverted;
    }
}

int getPedestalMode() {
    return ((bus_r(PEDESTAL_MODE_REG) & PEDESTAL_MODE_ENBLE_MSK) >>
            PEDESTAL_MODE_ENBLE_OFST);
}

void getPedestalParameters(uint8_t *frames, uint16_t *loops) {
    uint32_t addr = PEDESTAL_MODE_REG;
    *frames =
        ((bus_r(addr) & PEDESTAL_MODE_LNGTH_MSK) >> PEDESTAL_MODE_LNGTH_OFST);
    *loops = ((bus_r(PEDESTAL_MODE_REG) & PEDESTAL_MODE_ITRTNS_MSK) >>
              PEDESTAL_MODE_ITRTNS_OFST);
}

void setPedestalMode(int enable, uint8_t frames, uint16_t loops) {
    // Note: loops is 8 bit in firmware as a bug.To be fixed in next version
    int prevPedestalEnable = getPedestalMode();
    uint32_t addr = PEDESTAL_MODE_REG;

    if (enable) {
        LOG(logINFOBLUE, ("Enabling pedestal mode [frames: %hhu, loops: %hu]\n",
                          frames, loops));

        // frames
        bus_w(addr, bus_r(addr) & ~PEDESTAL_MODE_LNGTH_MSK);
        bus_w(addr, bus_r(addr) | ((frames << PEDESTAL_MODE_LNGTH_OFST) &
                                   PEDESTAL_MODE_LNGTH_MSK));
        // loops
        bus_w(addr, bus_r(addr) & ~PEDESTAL_MODE_ITRTNS_MSK);
        bus_w(addr, bus_r(addr) | ((loops << PEDESTAL_MODE_ITRTNS_OFST) &
                                   PEDESTAL_MODE_ITRTNS_MSK));
        // enable
        bus_w(addr, bus_r(addr) | PEDESTAL_MODE_ENBLE_MSK);

        // if it was switched off before, remember the #frames and #triggers
        if (prevPedestalEnable == 0) {
            normal_mode_frames = getNumFrames();
            normal_mode_triggers = getNumTriggers();
            LOG(logINFO, ("\tRemembering Normal mode #frames and "
                          "#triggers[%lld, %lld]\n",
                          normal_mode_frames, normal_mode_triggers));
        }

        // overwrite #frames and #triggers to new values
        int64_t expFrames = -1;
        int64_t expTriggers = -1;
        enum timingMode timing = getTiming();
        if (timing == AUTO_TIMING ||
            (timing == TRIGGER_EXPOSURE && normal_mode_frames > 1)) {
            expFrames = frames * loops * 2;
            expTriggers = 1;
        } else {
            expFrames = 1;
            expTriggers = frames * loops * 2;
        }
        LOG(logINFO, ("\tOverwriting [#frames: %lld, #triggers: %lld]\n",
                      expFrames, expTriggers));
        set64BitReg(expFrames, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
        set64BitReg(expTriggers, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);

    } else {
        LOG(logINFOBLUE, ("Disabling pedestal mode\n"));
        bus_w(addr, bus_r(addr) & ~PEDESTAL_MODE_ENBLE_MSK);

        // if it was switched on before, reset the normal mode #frames and
        // #triggers
        if (prevPedestalEnable == 1) {
            LOG(logINFO,
                ("\tResetting to Normal mode [#frames:%lld, #triggers:%lld\n",
                 normal_mode_frames, normal_mode_triggers));
            set64BitReg(normal_mode_frames, SET_FRAMES_LSB_REG,
                        SET_FRAMES_MSB_REG);
            set64BitReg(normal_mode_triggers, SET_CYCLES_LSB_REG,
                        SET_CYCLES_MSB_REG);
        }
    }
}

int setTimingInfoDecoder(enum timingInfoDecoder val) {
    switch (val) {
    case SWISSFEL:
        LOG(logINFO, ("Setting Timing Info Decoder to SWISSFEL\n"));
        break;
    case SHINE:
        LOG(logINFO, ("Setting Timing Info Decoder to SHINE\n"));
        break;
    default:
        LOG(logERROR, ("Unknown Timing Info Decoder %d\n", val));
        return FAIL;
    }

    int decodeValue = (int)val;
    uint32_t addr = EXT_SIGNAL_REG;
    bus_w(addr, bus_r(addr) & ~EXT_TIMING_INFO_DECODER_MSK);
    bus_w(addr, bus_r(addr) | ((decodeValue << EXT_TIMING_INFO_DECODER_OFST) &
                               EXT_TIMING_INFO_DECODER_MSK));

    return OK;
}

int getTimingInfoDecoder(enum timingInfoDecoder *retval) {
    int decodeValue = ((bus_r(EXT_SIGNAL_REG) & EXT_TIMING_INFO_DECODER_MSK) >>
                       EXT_TIMING_INFO_DECODER_OFST);
    if (decodeValue == (int)SWISSFEL) {
        *retval = SWISSFEL;
    } else if (decodeValue == (int)SHINE) {
        *retval = SHINE;
    } else {
        return FAIL;
    }
    return OK;
}

int getElectronCollectionMode() {
    return ((bus_r(DAQ_REG) & DAQ_ELCTRN_CLLCTN_MDE_MSK) >>
            DAQ_ELCTRN_CLLCTN_MDE_OFST);
}

void setElectronCollectionMode(int enable) {
    LOG(logINFO,
        ("Setting Collection Mode to %s\n", enable == 0 ? "Hole" : "Electron"));
    if (enable) {
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_ELCTRN_CLLCTN_MDE_MSK);
    } else {
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_ELCTRN_CLLCTN_MDE_MSK);
    }
    configureChip();
}

int getTenGigaFlowControl() {
    return ((bus_r(CONFIG_REG) & CONFIG_ETHRNT_FLW_CNTRL_MSK) >>
            CONFIG_ETHRNT_FLW_CNTRL_OFST);
}

int setTenGigaFlowControl(int value) {
    if (value >= 0) {
        if (value == 0) {
            LOG(logINFO, ("Switching off 10G flow control\n"));
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) & ~CONFIG_ETHRNT_FLW_CNTRL_MSK);
        } else {
            LOG(logINFO, ("Switching on 10G flow control\n"));
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) | CONFIG_ETHRNT_FLW_CNTRL_MSK);
        }
    }
    return OK;
}

int getTransmissionDelayFrame() {
    return ((bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK) >>
            CONFIG_TDMA_TIMESLOT_OFST);
}

int setTransmissionDelayFrame(int value) {
    if (value >= 0) {
        LOG(logINFO, ("Setting transmission delay: %d\n", value));
        bus_w(CONFIG_REG, (bus_r(CONFIG_REG) & ~CONFIG_TDMA_TIMESLOT_MSK) |
                              (((value << CONFIG_TDMA_TIMESLOT_OFST) &
                                CONFIG_TDMA_TIMESLOT_MSK)));
        if (value == 0) {
            LOG(logINFO, ("Switching off transmission delay\n"));
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) & ~CONFIG_TDMA_ENABLE_MSK);
        } else {
            LOG(logINFO, ("Switching on transmission delay\n"));
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) | CONFIG_TDMA_ENABLE_MSK);
        }
        LOG(logDEBUG1, ("Transmission delay read %d\n",
                        ((bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK) >>
                         CONFIG_TDMA_TIMESLOT_OFST)));
    }
    return OK;
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
    LOG(logINFOBLUE, ("starting state machine\n"));
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
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_START_ACQ_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_START_ACQ_MSK);

    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    return OK;
}

#ifdef VIRTUAL
void *start_timer(void *arg) {
    if (!isControlServer) {
        return NULL;
    }
    int firstDest = getFirstUDPDestination();
    int transmissionDelayUs = getTransmissionDelayFrame() * 1000;
    int numInterfaces = getNumberofUDPInterfaces();
    int64_t periodNs = getPeriod();
    int numFrames = (getNumFrames() * getNumTriggers() *
                     (getNumAdditionalStorageCells() + 1));
    int64_t expUs = getExpTime() / 1000;
    const int dataSize = 8192;
    const int packetsize = dataSize + sizeof(sls_detector_header);
    const int maxPacketsPerFrame = 128;
    const int maxRows = MAX_ROWS_PER_READOUT;
    int readNRows = getReadNRows();
    if (readNRows == -1) {
        LOG(logERROR,
            ("number of rows is -1. Assuming no partial readout (#rows).\n"));
        readNRows = MAX_ROWS_PER_READOUT;
    }
    const int packetsPerFrame =
        ((maxPacketsPerFrame / 2) * readNRows) / (maxRows / 2);

    // Generate data
    char imageData[DATA_BYTES];
    memset(imageData, 0, DATA_BYTES);
    {
        const int npixels = (NCHAN * NCHIP);
        const int pixelsPerPacket = dataSize / NUM_BYTES_PER_PIXEL;
        int dataVal = 0;
        int gainVal = 0;
        int pixelVal = 0;
        for (int i = 0; i < npixels; ++i) {
            if (i % pixelsPerPacket == 0) {
                ++dataVal;
            }

            if ((i % 1024) < 300) {
                gainVal = 0;
            } else if ((i % 1024) < 600) {
                gainVal = 1;
            } else {
                gainVal = 3;
            }
            pixelVal = (dataVal & ~GAIN_VAL_MSK) | (gainVal << GAIN_VAL_OFST);
// to debug multi module geometry (row, column) in virtual servers (all pixels
// in a module set to particular value)
#ifdef TEST_MOD_GEOMETRY
            *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
                portno % 1900 + (i >= npixels / 2 ? 1 : 0);
#else
            *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
                virtual_image_test_mode ? 0x0FFE : (uint16_t)pixelVal;

#endif
        }
    }

    // Send data
    {
        uint64_t frameNr = 0;
        getNextFrameNumber(&frameNr);
        int iRxEntry = firstDest;
        for (int iframes = 0; iframes != numFrames; ++iframes) {
            if (transmissionDelayUs)
                usleep(transmissionDelayUs);

            // check if manual stop
            if (sharedMemory_getStop() == 1) {
                setNextFrameNumber(frameNr + iframes + 1);
                break;
            }

            // sleep for exposure time
            struct timespec begin, end;
            clock_gettime(CLOCK_REALTIME, &begin);
            usleep(expUs);

#ifdef TEST_CHANGE_GAIN_EVERY_FRAME
            // change gain and data for every frame
            {
                const int npixels = (NCHAN * NCHIP);

                // random gain values, 2 becomes 3 as 2 is invalid
                int randomGainValues[3] = {0};
                srand(time(0));
                for (int i = 0; i != 3; ++i) {
                    int r = rand() % 3;
                    if (r == 2)
                        r = 3;
                    randomGainValues[i] = r;
                }

                for (int i = 0; i < npixels; ++i) {
                    int gainVal = 0;
                    if ((i % 1024) < 300) {
                        gainVal = randomGainValues[0];
                    } else if ((i % 1024) < 600) {
                        gainVal = randomGainValues[1];
                    } else {
                        gainVal = randomGainValues[2];
                    }
                    int dataVal =
                        *((uint16_t *)(imageData + i * sizeof(uint16_t)));
                    dataVal += iframes;
                    int pixelVal =
                        (dataVal & ~GAIN_VAL_MSK) | (gainVal << GAIN_VAL_OFST);
                    *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
                        (uint16_t)pixelVal;
                }
            }
#endif
            int srcOffset = 0;
            int srcOffset2 = DATA_BYTES / 2;
            int row0 = (numInterfaces == 1 ? detPos[1] : detPos[3]);
            int col0 = (numInterfaces == 1 ? detPos[0] : detPos[2]);
            int row1 = detPos[1];
            int col1 = detPos[0];
            // loop packet (128 packets)
            for (int i = 0; i != maxPacketsPerFrame; ++i) {

                const int startval =
                    (maxPacketsPerFrame / 2) - (packetsPerFrame / 2);
                const int endval = startval + packetsPerFrame - 1;
                int pnum = i;

                // first interface
                if (numInterfaces == 1 || i < (maxPacketsPerFrame / 2)) {
                    char packetData[packetsize];
                    memset(packetData, 0, packetsize);
                    sls_detector_header *header =
                        (sls_detector_header *)(packetData);
                    header->detType = (uint16_t)myDetectorType;
                    header->version = SLS_DETECTOR_HEADER_VERSION;
                    header->frameNumber = frameNr + iframes;
                    header->packetNumber = pnum;
                    header->modId = virtual_moduleid;
                    header->row = row0;
                    header->column = col0;

                    // fill data
                    memcpy(packetData + sizeof(sls_detector_header),
                           imageData + srcOffset, dataSize);
                    srcOffset += dataSize;

                    if (i >= startval && i <= endval) {
                        sendUDPPacket(iRxEntry, 0, packetData, packetsize);
                        LOG(logDEBUG1, ("Sent packet: %d [interface 0]\n", i));
                    }
                }

                // second interface
                else if (numInterfaces == 2 && i >= (maxPacketsPerFrame / 2)) {
                    pnum = i % (maxPacketsPerFrame / 2);

                    char packetData2[packetsize];
                    memset(packetData2, 0, packetsize);
                    sls_detector_header *header =
                        (sls_detector_header *)(packetData2);
                    header->detType = (uint16_t)myDetectorType;
                    header->version = SLS_DETECTOR_HEADER_VERSION;
                    header->frameNumber = frameNr + iframes;
                    header->packetNumber = pnum;
                    header->modId = virtual_moduleid;
                    header->row = row1;
                    header->column = col1;

                    // fill data
                    memcpy(packetData2 + sizeof(sls_detector_header),
                           imageData + srcOffset2, dataSize);
                    srcOffset2 += dataSize;

                    if (i >= startval && i <= endval) {
                        sendUDPPacket(iRxEntry, 1, packetData2, packetsize);
                        LOG(logDEBUG1,
                            ("Sent packet: %d [interface 1]\n", pnum));
                    }
                }
            }
            LOG(logINFO, ("Sent frame %d [#%ld] to E%d\n", iframes,
                          frameNr + iframes, iRxEntry));
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
        setNextFrameNumber(frameNr + numFrames);
    }

    closeUDPSocket(0);
    if (numInterfaces == 2) {
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
    // read till status is idle
    while (sharedMemory_getStatus() == RUNNING)
        usleep(500);
    sharedMemory_setStop(0);
    LOG(logINFO, ("Stopped State Machine\n"));
    return OK;
#endif
    // stop state machine
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STOP_ACQ_MSK);
    usleep(100);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STOP_ACQ_MSK);

    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    return OK;
}

int softwareTrigger(int block) {
#ifndef VIRTUAL
    // ready for trigger
    if (getRunStatus() != WAITING) {
        LOG(logWARNING, ("Not yet ready for trigger!\n"));
        return 0;
    }
#endif

    LOG(logINFO, ("Sending Software Trigger\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_SOFTWARE_TRIGGER_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_SOFTWARE_TRIGGER_MSK);
    // wait to make sure its out of this state and even 'wait for start frame'
    usleep(100);

#ifndef VIRTUAL
    // block till frame sent out & back to wait for trigger (or not busy
    // anymore)
    if (block) {
        uint32_t retval = bus_r(STATUS_REG);
        while ((retval & RUN_BUSY_MSK) && !(retval & WAITING_FOR_TRIGGER_MSK)) {
            usleep(5000);
            retval = bus_r(STATUS_REG);
        }
    }
    LOG(logINFO, ("Ready for Next Trigger...\n"));
#endif

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

    enum runStatus s;
    u_int32_t retval = bus_r(STATUS_REG);
    LOG(logINFO, ("Status Register: %08x\n", retval));

    // error
    if (retval & INTERNAL_STOP_MSK) {
        LOG(logINFOBLUE, ("Status: ERROR\n"));
        s = ERROR;
    }

    // running
    else if (retval & RUN_BUSY_MSK) {
        if ((retval & WAITING_FOR_TRIGGER_MSK) ||
            (retval & WAITING_FOR_START_FRAME_MSK)) {
            LOG(logINFOBLUE, ("Status: WAITING\n"));
            s = WAITING;
        } else {
            LOG(logINFOBLUE, ("Status: RUNNING\n"));
            s = RUNNING;
        }
    }

    // not running
    else {
        // stopped or idle
        if (retval & STOPPED_MSK) {
            LOG(logINFOBLUE, ("Status: STOPPED\n"));
            s = STOPPED;
        } else {
            LOG(logINFOBLUE, ("Status: IDLE\n"));
            s = IDLE;
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
    u_int32_t s = (bus_r(STATUS_REG) & RUN_BUSY_MSK);
    LOG(logDEBUG1, ("Status Register: %08x\n", s));
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
