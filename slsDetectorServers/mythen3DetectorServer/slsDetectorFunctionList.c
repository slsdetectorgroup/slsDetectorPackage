#include "slsDetectorFunctionList.h"
#include "ALTERA_PLL_CYCLONE10.h"
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

/// NOT the right place to put it!
int setChipStatusRegister(int csr);

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern int checkModuleFlag;
extern udpStruct udpDetails;
extern const enum detectorType myDetectorType;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char *cmac, int size, uint64_t mac);
extern void getIpAddressinString(char *cip, uint32_t ip);

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int64_t virtual_currentFrameNumber = 2;
#endif

enum detectorSettings thisSettings;
sls_detector_module *detectorModules = NULL;
int *detectorChans = NULL;
int *detectorDacs = NULL;

enum TLogLevel trimmingPrint = logINFO;
int32_t clkPhase[NUM_CLOCKS] = {};
uint32_t clkDivider[NUM_CLOCKS] = {};

int highvoltage = 0;
int detPos[2] = {};
int64_t exptimeReg[NCOUNTERS] = {0, 0, 0};
int64_t gateDelayReg[NCOUNTERS] = {0, 0, 0};
int vthEnabledVals[NCOUNTERS] = {0, 0, 0};
int detID = 0;

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
    LOG(logINFOBLUE, ("******** Mythen3 Virtual Server *****************\n"));
    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Dangerous to continue.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
    }
    return;
#else
    LOG(logINFOBLUE, ("************ Mythen3 Server *********************\n"));
    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Dangerous to continue.\n");
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }
    // does check only if flag is 0 (by default), set by command line
    if ((!debugflag) && (!updateFlag) &&
        ((checkKernelVersion() == FAIL) || (checkType() == FAIL) ||
         (testFpga() == FAIL) || (testBus() == FAIL))) {
        strcpy(initErrorMessage, "Could not pass basic tests of FPGA and bus. "
                                 "Dangerous to continue.\n");
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }
    uint16_t hversion = getHardwareVersionNumber();
    uint32_t ipadd = getDetectorIP();
    uint64_t macadd = getDetectorMAC();
    int64_t fwversion = getFirmwareVersion();
    int64_t swversion = getServerVersion();
    int64_t sw_fw_apiversion = getFirmwareAPIVersion();
    ;
    int64_t client_sw_apiversion = getClientServerAPIVersion();
    uint32_t requiredFirmwareVersion = REQRD_FRMWRE_VRSN;

    LOG(logINFOBLUE,
        ("*************************************************\n"
         "Hardware Version:\t\t 0x%x\n"

         "Detector IP Addr:\t\t 0x%x\n"
         "Detector MAC Addr:\t\t 0x%llx\n\n"

         "Firmware Version:\t\t 0x%llx\n"
         "Software Version:\t\t 0x%llx\n"
         "F/w-S/w API Version:\t\t 0x%llx\n"
         "Required Firmware Version:\t 0x%x\n"
         "Client-Software API Version:\t 0x%llx\n"
         "********************************************************\n",
         hversion, ipadd, (long long unsigned int)macadd,
         (long long int)fwversion, (long long int)swversion,
         (long long int)sw_fw_apiversion, requiredFirmwareVersion,
         (long long int)client_sw_apiversion));

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

int checkKernelVersion() {
#ifdef VIRTUAL
    return OK;
#endif
    return Nios_checkKernelVersion(KERNEL_DATE_VRSN);
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

uint64_t getServerVersion() { return APIMYTHEN3; }

uint64_t getClientServerAPIVersion() { return APIMYTHEN3; }

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
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

u_int16_t getHardwareVersionNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(MCB_SERIAL_NO_REG) & MCB_SERIAL_NO_VRSN_MSK) >>
            MCB_SERIAL_NO_VRSN_OFST);
}

void readDetectorNumber() {
#ifndef VIRTUAL
    if (initError == FAIL) {
        return;
    }
    FILE *fd = fopen(ID_FILE, "r");
    if (fd == NULL) {
        sprintf(initErrorMessage, "No %s file found.\n", ID_FILE);
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }
    char output[255];
    fgets(output, sizeof(output), fd);
    sscanf(output, "%u", &detID);
    if (isControlServer) {
        LOG(logINFOBLUE, ("Detector ID: %u\n", detID));
    }
#endif
}

u_int32_t getDetectorNumber() { return detID; }

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
    char temp[50] = "";
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

    usleep(CTRL_SRVR_INIT_TIME_US);
    if (mapCSP0() == FAIL) {
        LOG(logERROR,
            ("Stop Server: Map Fail. Dangerous to continue. Goodbye!\n"));
        exit(EXIT_FAILURE);
    }
#ifdef VIRTUAL
    sharedMemory_setStop(0);
#endif
}

/* set up detector */

void allocateDetectorStructureMemory() {
    // Allocation of memory
    detectorModules = malloc(sizeof(sls_detector_module));
    detectorChans = malloc(NCHIP * NCHAN * sizeof(int));
    detectorDacs = malloc(NDAC * sizeof(int));
    LOG(logDEBUG1,
        ("modules from 0x%x to 0x%x\n", detectorModules, detectorModules));
    LOG(logDEBUG1, ("chans from 0x%x to 0x%x\n", detectorChans, detectorChans));
    LOG(logDEBUG1, ("dacs from 0x%x to 0x%x\n", detectorDacs, detectorDacs));
    (detectorModules)->dacs = detectorDacs;
    (detectorModules)->chanregs = detectorChans;
    (detectorModules)->ndac = NDAC;
    (detectorModules)->nchip = NCHIP;
    (detectorModules)->nchan = NCHIP * NCHAN;
    (detectorModules)->reg = UNINITIALIZED;
    (detectorModules)->iodelay = 0;
    (detectorModules)->tau = 0;
    (detectorModules)->eV[0] = 0;
    (detectorModules)->eV[1] = 0;
    (detectorModules)->eV[2] = 0;
    thisSettings = UNINITIALIZED;

    // initialize dacs
    for (int idac = 0; idac < (detectorModules)->ndac; ++idac) {
        detectorDacs[idac] = 0;
    }

    // trimbits start at 0 //TODO: restart server will not have 0 always
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        *((detectorModules->chanregs) + ichan) = 0;
    }
}

void setupDetector() {
    LOG(logINFO, ("This Server is for 1 Mythen3 module \n"));

    allocateDetectorStructureMemory();

    clkDivider[READOUT_C0] = DEFAULT_READOUT_C0;
    clkDivider[READOUT_C1] = DEFAULT_READOUT_C1;
    clkDivider[SYSTEM_C0] = DEFAULT_SYSTEM_C0;
    clkDivider[SYSTEM_C1] = DEFAULT_SYSTEM_C1;
    clkDivider[SYSTEM_C2] = DEFAULT_SYSTEM_C2;
    clkDivider[SYSTEM_C3] = DEFAULT_SYSTEM_C3;

    highvoltage = 0;
    trimmingPrint = logINFO;
    for (int i = 0; i < NUM_CLOCKS; ++i) {
        clkPhase[i] = 0;
    }
#ifdef VIRTUAL
    sharedMemory_setStatus(IDLE);
#endif

    // pll defines
    ALTERA_PLL_C10_SetDefines(
        REG_OFFSET, BASE_READOUT_PLL, BASE_SYSTEM_PLL, PLL_RESET_REG,
        PLL_RESET_READOUT_MSK, PLL_RESET_SYSTEM_MSK, SYSTEM_STATUS_REG,
        SYSTEM_STATUS_RDO_PLL_LCKD_MSK, SYSTEM_STATUS_R_PLL_LCKD_MSK,
        READOUT_PLL_VCO_FREQ_HZ, SYSTEM_PLL_VCO_FREQ_HZ);
    ALTERA_PLL_C10_ResetPLL(READOUT_PLL);
    ALTERA_PLL_C10_ResetPLL(SYSTEM_PLL);
    // hv
    DAC6571_SetDefines(HV_HARD_MAX_VOLTAGE, HV_DRIVER_FILE_NAME);
    // dac
    LTC2620_D_SetDefines(DAC_MAX_MV, DAC_DRIVER_FILE_NAME, NDAC);

    resetCore();
    resetPeripheral();
    cleanFifos();

    // defaults
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);
    setDefaultDacs();
    setASICDefaults();
    setADIFDefaults();

    // set trigger flow for m3 (for all timing modes)
    bus_w(FLOW_TRIGGER_REG, bus_r(FLOW_TRIGGER_REG) | FLOW_TRIGGER_MSK);

    // dynamic range
    setDynamicRange(DEFAULT_DYNAMIC_RANGE);
    // enable all counters
    setCounterMask(MAX_COUNTER_MSK);

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
#ifdef VIRTUAL
    enableTenGigabitEthernet(0);
#endif
    readDetectorNumber();
    if (initError == FAIL) {
        return;
    }
    setSettings(DEFAULT_SETTINGS);

    // check module type attached if not in debug mode
    {
        int ret = checkDetectorType();
        if (checkModuleFlag) {
            switch (ret) {
            case -1:
                sprintf(initErrorMessage,
                        "Could not get the module type attached.\n");
                initError = FAIL;
                LOG(logERROR, ("Aborting startup!\n\n", initErrorMessage));
                return;
            case -2:
                sprintf(initErrorMessage,
                        "No Module attached! Run server with -nomodule.\n");
                initError = FAIL;
                LOG(logERROR, ("Aborting startup!\n\n", initErrorMessage));
                return;
            case FAIL:
                sprintf(initErrorMessage,
                        "Wrong Module (Not Mythen3) attached!\n");
                initError = FAIL;
                LOG(logERROR, ("Aborting startup!\n\n", initErrorMessage));
                return;
            default:
                break;
            }
        } else {
            LOG(logINFOBLUE,
                ("In No-Module mode: Ignoring module type. Continuing.\n"));
        }
    }

    powerChip(1);
    if (initError != FAIL) {
      initError = setChipStatusRegister(CSR_default);
      //loadDefaultPattern(DEFAULT_PATTERN_FILE, initErrorMessage);
      //startStateMachine(); //this was missing in previous code! runs the default pattern
    }
    setAllTrimbits(DEFAULT_TRIMBIT_VALUE);
}

int setDefaultDacs() {
    int ret = OK;
    LOG(logINFOBLUE, ("Setting Default Dac values\n"));
    {
        const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
        for (int i = 0; i < NDAC; ++i) {
            setDAC((enum DACINDEX)i, defaultvals[i], 0);
            if (detectorDacs[i] != defaultvals[i]) {
                ret = FAIL;
                LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n", i,
                               defaultvals[i], detectorDacs[i]));
            }
        }
    }
    return ret;
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
    if (dr > 0) {
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
    }

    uint32_t regval = bus_r(CONFIG_REG) & CONFIG_DYNAMIC_RANGE_MSK;
    switch (regval) {
    /*case CONFIG_DYNAMIC_RANGE_1_VAL: TODO:Not implemented in firmware yet
        return 1;*/
    case CONFIG_DYNAMIC_RANGE_8_VAL:
        return 8;
    case CONFIG_DYNAMIC_RANGE_16_VAL:
        return 16;
    case CONFIG_DYNAMIC_RANGE_24_VAL:
        return 32;
    default:
        LOG(logERROR, ("Invalid dynamic range %d read back\n",
                       regval >> CONFIG_DYNAMIC_RANGE_OFST));
        return -1;
    }
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

void setCounterMask(uint32_t arg) {
    if (arg == 0 || arg > MAX_COUNTER_MSK) {
        return;
    }
    uint32_t oldmask = getCounterMask();
    LOG(logINFO, ("Setting counter mask to  0x%x\n", arg));
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

    LOG(logINFO, ("\tUpdating Vth dacs\n"));
    for (int i = 0; i < NCOUNTERS; ++i) {
        // if change in enable
        if ((arg & (1 << i)) ^ (oldmask & (1 << i))) {
            // will disable if counter disabled
            setDAC(VTH1, vthEnabledVals[i], 0);
        }
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
        const int dr = setDynamicRange(-1);
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

int setModule(sls_detector_module myMod, char *mess) {

    LOG(logINFO, ("Setting module\n"));

    // settings
    if (myMod.reg >= 0) {
        setSettings((enum detectorSettings)myMod.reg);
        if (getSettings() != (enum detectorSettings)myMod.reg) {
            sprintf(
                mess,
                "Could not set module. Could not set settings to %d, read %d\n",
                myMod.reg, (int)getSettings());
            LOG(logERROR, (mess));
            return FAIL;
        }
        detectorModules->reg = myMod.reg;
    }
    // custom trimbit file
    else {
        // changed for setsettings (direct),
        // custom trimbit file (setmodule with myMod.reg as -1),
        // change of dac (direct)
        for (int i = 0; i < NCOUNTERS; ++i) {
            setThresholdEnergy(i, -1);
        }
    }

    // dacs
    for (int i = 0; i < NDAC; ++i) {
        // ignore dacs with -1
        if (myMod.dacs[i] != -1) {
            setDAC((enum DACINDEX)i, myMod.dacs[i], 0);
            if (myMod.dacs[i] != detectorDacs[i]) {
                // dont complain if that counter was disabled
                if ((i == M_VTH1 || i == M_VTH2 || i == M_VTH3) &&
                    (detectorDacs[i] == DEFAULT_COUNTER_DISABLED_VTH_VAL)) {
                    continue;
                }
                sprintf(mess,
                        "Could not set module. Could not set dac %d, wrote %d, "
                        "read %d\n",
                        i, myMod.dacs[i], detectorDacs[i]);
                LOG(logERROR, (mess));
                return FAIL;
            }
        }
    }

    // if settings given and cannot be validated (after setting dacs), return
    // error
    if (myMod.reg >= 0) {
        if (getSettings() != (enum detectorSettings)myMod.reg) {
            sprintf(
                mess,
                "Could not set module. The dacs in file do not correspond to "
                "settings %d\n",
                myMod.reg);
            LOG(logERROR, (mess));
            return FAIL;
        }
    }

    // threshold
    for (int i = 0; i < NCOUNTERS; ++i) {
        if (myMod.eV[i] >= 0) {
            setThresholdEnergy(i, myMod.eV[i]);
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

int setBit(int ibit, int patword) { return patword |= (1 << ibit); }

int clearBit(int ibit, int patword) { return patword &= ~(1 << ibit); }

int setTrimbits(int *trimbits) {
    LOG(logINFOBLUE, ("Setting trimbits\n"));

    // validate
    for (int ichan = 0; ichan < ((detectorModules)->nchan); ++ichan) {
        if (trimbits[ichan] < 0 || trimbits[ichan] > 63) {
            LOG(logERROR, ("Trimbit value (%d) for channel %d is invalid\n",
                           trimbits[ichan], ichan));
            return FAIL;
        }
    }
    LOG(logINFO, ("Trimbits validated\n"));
    trimmingPrint = logDEBUG5;

    // remember previous run clock
    uint32_t prevRunClk = clkDivider[SYSTEM_C0];

    // set to trimming clock
    if (setClockDivider(SYSTEM_C0, DEFAULT_TRIMMING_RUN_CLKDIV) == FAIL) {
        LOG(logERROR,
            ("Could not start trimming. Could not set to trimming clock\n"));
        return FAIL;
    }

    // trimming
    int error = 0;
    uint64_t patword = 0;
    int iaddr = 0;
    for (int ichip = 0; ichip < NCHIP; ichip++) {
        if (error != 0) {
            break;
        }
        LOG(logDEBUG1, (" Chip %d\n", ichip));
        iaddr = 0;
        patword = 0;
        writePatternWord(iaddr++, patword);

        // chip select
        patword = setBit(SIGNAL_TBLoad_1 + ichip, patword);
        writePatternWord(iaddr++, patword);

        // reset trimbits
        patword = setBit(SIGNAL_resStorage, patword);
        patword = setBit(SIGNAL_resCounter, patword);
        writePatternWord(iaddr++, patword);
        writePatternWord(iaddr++, patword);
        patword = clearBit(SIGNAL_resStorage, patword);
        patword = clearBit(SIGNAL_resCounter, patword);
        writePatternWord(iaddr++, patword);
        writePatternWord(iaddr++, patword);

        // select first channel
        patword = setBit(SIGNAL_CHSserialIN, patword);
        writePatternWord(iaddr++, patword);
        // 1 clk pulse
        patword = setBit(SIGNAL_CHSclk, patword);
        writePatternWord(iaddr++, patword);
        patword = clearBit(SIGNAL_CHSclk, patword);
        // clear 1st channel
        writePatternWord(iaddr++, patword);
        patword = clearBit(SIGNAL_CHSserialIN, patword);
        // 2 clk pulses
        for (int i = 0; i < 2; i++) {
            patword = setBit(SIGNAL_CHSclk, patword);
            writePatternWord(iaddr++, patword);
            patword = clearBit(SIGNAL_CHSclk, patword);
            writePatternWord(iaddr++, patword);
        }

        // for each channel (all chips)
        for (int ich = 0; ich < NCHAN_1_COUNTER; ich++) {
            LOG(logDEBUG1, (" Chip %d, Channel %d\n", ichip, ich));
            int val = trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS +
                               NCOUNTERS * ich] +
                      trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS +
                               NCOUNTERS * ich + 1] *
                          64 +
                      trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS +
                               NCOUNTERS * ich + 2] *
                          64 * 64;

            // push 6 0 bits
            for (int i = 0; i < 6; i++) {
                patword = clearBit(SIGNAL_serialIN, patword);
                patword = clearBit(SIGNAL_clk, patword);
                writePatternWord(iaddr++, patword);
                patword = setBit(SIGNAL_clk, patword);
                writePatternWord(iaddr++, patword);
            }

            // deserialize
            for (int i = 0; i < 18; i++) {
                if (val & (1 << i)) {
                    patword = setBit(SIGNAL_serialIN, patword);
                } else {
                    patword = clearBit(SIGNAL_serialIN, patword);
                }
                patword = clearBit(SIGNAL_clk, patword);
                writePatternWord(iaddr++, patword);

                patword = setBit(SIGNAL_clk, patword);
                writePatternWord(iaddr++, patword);
            }
            writePatternWord(iaddr++, patword);
            writePatternWord(iaddr++, patword);

            // move to next channel
            for (int i = 0; i < 3; i++) {
                patword = setBit(SIGNAL_CHSclk, patword);
                writePatternWord(iaddr++, patword);
                patword = clearBit(SIGNAL_CHSclk, patword);
                writePatternWord(iaddr++, patword);
            }
        }
        // chip unselect
        patword = clearBit(SIGNAL_TBLoad_1 + ichip, patword);
        writePatternWord(iaddr++, patword);

        // last iaddr check
        if (iaddr >= MAX_PATTERN_LENGTH) {
            LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n",
                           iaddr, MAX_PATTERN_LENGTH));
            error = 1;
            break;
        }

        // set pattern wait address
        for (int i = 0; i <= 2; i++)
            setPatternWaitAddress(i, MAX_PATTERN_LENGTH - 1);

        // pattern loop
        for (int i = 0; i <= 2; i++) {
            int stop = MAX_PATTERN_LENGTH - 1, nloop = 0;
            setPatternLoop(i, &stop, &stop, &nloop);
        }

        // pattern limits
        {
            int start = 0, nloop = 0;
            setPatternLoop(-1, &start, &iaddr, &nloop);
        }
        // send pattern to the chips
        startPattern();
    }

    if (error == 0) {
        // copy trimbits locally
        for (int ichan = 0; ichan < ((detectorModules)->nchan); ++ichan) {
            detectorChans[ichan] = trimbits[ichan];
        }
        LOG(logINFO, ("All trimbits have been loaded\n"));
    }

    trimmingPrint = logINFO;
    // set back to previous clock
    if (setClockDivider(SYSTEM_C0, prevRunClk) == FAIL) {
        LOG(logERROR, ("Could not set to previous run clock after trimming\n"));
        return FAIL;
    }

    if (error != 0) {
        return FAIL;
    }

    return OK;
}

int setAllTrimbits(int val) {
    LOG(logINFO, ("Setting all trimbits to %d\n", val));
    int *trimbits = malloc(sizeof(int) * ((detectorModules)->nchan));
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
    switch (sett) {
    case STANDARD:
        LOG(logINFOBLUE, ("Setting to standard settings\n"));
        thisSettings = sett;
        setDAC(M_VRPREAMP, DEFAULT_STANDARD_VRPREAMP, 0);
        setDAC(M_VRSHAPER, DEFAULT_STANDARD_VRSHAPER, 0);
        break;
    case FAST:
        LOG(logINFOBLUE, ("Setting to fast settings\n"));
        thisSettings = sett;
        setDAC(M_VRPREAMP, DEFAULT_FAST_VRPREAMP, 0);
        setDAC(M_VRSHAPER, DEFAULT_FAST_VRSHAPER, 0);
        break;
    case HIGHGAIN:
        LOG(logINFOBLUE, ("Setting to high gain settings\n"));
        thisSettings = sett;
        setDAC(M_VRPREAMP, DEFAULT_HIGHGAIN_VRPREAMP, 0);
        setDAC(M_VRSHAPER, DEFAULT_HIGHGAIN_VRSHAPER, 0);
        break;
    default:
        LOG(logERROR,
            ("Settings %d not defined for this detector\n", (int)sett));
        return thisSettings;
    }

    LOG(logINFO, ("Settings: %d\n", thisSettings));
    return thisSettings;
}

void validateSettings() {
    if (detectorDacs[M_VRPREAMP] == DEFAULT_STANDARD_VRPREAMP &&
        detectorDacs[M_VRSHAPER] == DEFAULT_STANDARD_VRSHAPER) {
        if (thisSettings != STANDARD) {
            thisSettings = STANDARD;
            LOG(logINFOBLUE, ("Validated Settings changed to standard!\n"));
        }
    } else if (detectorDacs[M_VRPREAMP] == DEFAULT_FAST_VRPREAMP &&
               detectorDacs[M_VRSHAPER] == DEFAULT_FAST_VRSHAPER) {
        if (thisSettings != FAST) {
            thisSettings = FAST;
            LOG(logINFOBLUE, ("Validated Settings changed to fast!\n"));
        }
    } else if (detectorDacs[M_VRPREAMP] == DEFAULT_HIGHGAIN_VRPREAMP &&
               detectorDacs[M_VRSHAPER] == DEFAULT_HIGHGAIN_VRSHAPER) {
        if (thisSettings != HIGHGAIN) {
            thisSettings = HIGHGAIN;
            LOG(logINFOBLUE, ("Validated Settings changed to highgain!\n"));
        }
    } else {
        thisSettings = UNDEFINED;
        LOG(logWARNING,
            ("Settings set to undefined [vrpreamp: %d, vrshaper: %d]\n",
             detectorDacs[M_VRPREAMP], detectorDacs[M_VRSHAPER]));
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
void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0) {
        return;
    }

    if (ind == M_VTHRESHOLD) {
        LOG(logINFO,
            ("Setting Threshold voltages to %d %s\n", val, (mV ? "mv" : "")));
        setDAC(M_VTH1, val, mV);
        setDAC(M_VTH2, val, mV);
        setDAC(M_VTH3, val, mV);
        return;
    }
    char *dac_names[] = {DAC_NAMES};

    // remember vthx values and set 2800 if counter disabled
    uint32_t counters = getCounterMask();
    int vthdacs[] = {M_VTH1, M_VTH2, M_VTH3};
    for (int i = 0; i < NCOUNTERS; ++i) {
        if (vthdacs[i] == (int)ind) {
            // remember enabled values for vthx
            if (val != DEFAULT_COUNTER_DISABLED_VTH_VAL) {
                int vthval = val;
                if (mV) {
                    if (LTC2620_D_VoltageToDac(val, &vthval) == FAIL) {
                        return;
                    }
                }
                vthEnabledVals[i] = vthval;
                LOG(logINFO, ("Remembering %s [%d]\n", dac_names[ind], vthval));
            }
            // set vthx to disable val, if counter disabled
            if (!(counters & (1 << i))) {
                LOG(logINFO, ("Disabling %s\n", dac_names[ind]));
                val = DEFAULT_COUNTER_DISABLED_VTH_VAL;
                mV = 0;
            }
        }
    }

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
    if (ind == M_VRPREAMP || ind == M_VRSHAPER) {
        validateSettings();
    }
}

int getDAC(enum DACINDEX ind, int mV) {
    if (ind == M_VTHRESHOLD) {
        int ret[NCOUNTERS] = {0};
        ret[0] = getDAC(M_VTH1, mV);
        ret[1] = getDAC(M_VTH2, mV);
        ret[2] = getDAC(M_VTH3, mV);

        if ((ret[0] == ret[1]) && (ret[1] == ret[2])) {
            LOG(logINFO, ("\tvthreshold match\n"));
            return ret[0];
        } else {
            LOG(logERROR, ("\tvthreshold mismatch vth1:%d vth2:%d "
                           "vth3:%d\n",
                           ret[0], ret[1], ret[2]));
            return -1;
        }
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

int setHighVoltage(int val) {
    // limit values
    if (val > HV_SOFT_MAX_VOLTAGE) {
        val = HV_SOFT_MAX_VOLTAGE;
    }

    // setting hv
    if (val >= 0) {
        LOG(logINFO, ("Setting High voltage: %d V\n", val));
        if (DAC6571_Set(val) == OK)
            highvoltage = val;
    }
    return highvoltage;
}

/* parameters - timing */
void setTiming(enum timingMode arg) {
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

int configureMAC() {

    uint32_t srcip = udpDetails.srcip;
    uint32_t dstip = udpDetails.dstip;
    uint64_t srcmac = udpDetails.srcmac;
    uint64_t dstmac = udpDetails.dstmac;
    int srcport = udpDetails.srcport;
    int dstport = udpDetails.dstport;

    LOG(logINFOBLUE, ("Configuring MAC\n"));
    char src_mac[50], src_ip[INET_ADDRSTRLEN], dst_mac[50],
        dst_ip[INET_ADDRSTRLEN];
    getMacAddressinString(src_mac, 50, srcmac);
    getMacAddressinString(dst_mac, 50, dstmac);
    getIpAddressinString(src_ip, srcip);
    getIpAddressinString(dst_ip, dstip);

    LOG(logINFO, ("\tSource IP   : %s\n"
                  "\tSource MAC  : %s\n"
                  "\tSource Port : %d\n"
                  "\tDest IP     : %s\n"
                  "\tDest MAC    : %s\n"
                  "\tDest Port   : %d\n",
                  src_ip, src_mac, srcport, dst_ip, dst_mac, dstport));

#ifdef VIRTUAL
    if (setUDPDestinationDetails(0, dst_ip, dstport) == FAIL) {
        LOG(logERROR, ("could not set udp destination IP and port\n"));
        return FAIL;
    }
#endif

    // start addr
    uint32_t addr = BASE_UDP_RAM;
    // calculate rxr endpoint offset
    // addr += (iRxEntry * RXR_ENDPOINT_OFST);//TODO: is there round robin
    // already implemented?
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
    LOG(logINFO, ("\tIP checksum is 0x%lx\n", checksum));
    udp->ip_checksum = checksum;
}

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));

    uint32_t addr = COORD_0_REG;
    int value = 0;
    int valueRead = 0;
    int ret = OK;

    // row
    value = detPos[X];
    bus_w(addr, (bus_r(addr) & ~COORD_ROW_MSK) |
                    ((value << COORD_ROW_OFST) & COORD_ROW_MSK));
    valueRead = ((bus_r(addr) & COORD_ROW_MSK) >> COORD_ROW_OFST);
    if (valueRead != value) {
        LOG(logERROR,
            ("Could not set row. Set %d, read %d\n", value, valueRead));
        ret = FAIL;
    }

    // col
    value = detPos[Y];
    bus_w(addr, (bus_r(addr) & ~COORD_COL_MSK) |
                    ((value << COORD_COL_OFST) & COORD_COL_MSK));
    valueRead = ((bus_r(addr) & COORD_COL_MSK) >> COORD_COL_OFST);
    if (valueRead != value) {
        LOG(logERROR,
            ("Could not set column. Set %d, read %d\n", value, valueRead));
        ret = FAIL;
    }

    if (ret == OK) {
        LOG(logINFO, ("\tPosition set to [%d, %d]\n", detPos[X], detPos[Y]));
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

/* pattern */

void startPattern() {
    LOG(logINFOBLUE, ("Starting Pattern\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STRT_PATTERN_MSK);
    usleep(1);
    while (bus_r(PAT_STATUS_REG) & PAT_STATUS_RUN_BUSY_MSK) {
        usleep(1);
    }
    LOG(logINFOBLUE, ("Pattern done\n"));
}

uint64_t readPatternWord(int addr) {
    // error (handled in tcp)
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        LOG(logERROR, ("Cannot get Pattern - Word. Invalid addr 0x%x. "
                       "Should be between 0 and 0x%x\n",
                       addr, MAX_PATTERN_LENGTH));
        return -1;
    }

    LOG(logDEBUG1, ("  Reading Pattern Word (addr:0x%x)\n", addr));
    uint32_t reg_lsb =
        PATTERN_STEP0_LSB_REG +
        addr * REG_OFFSET * 2; // the first word in RAM as base plus the
                               // offset of the word to write (addr)
    uint32_t reg_msb = PATTERN_STEP0_MSB_REG + addr * REG_OFFSET * 2;

    // read value
    uint64_t retval = get64BitReg(reg_lsb, reg_msb);
    LOG(logDEBUG1,
        ("  Word(addr:0x%x) retval: 0x%llx\n", addr, (long long int)retval));

    return retval;
}

uint64_t writePatternWord(int addr, uint64_t word) {

    // get
    if ((int64_t)word == -1)
        return readPatternWord(addr);

    // error (handled in tcp)
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        LOG(logERROR, ("Cannot set Pattern - Word. Invalid addr 0x%x. "
                       "Should be between 0 and 0x%x\n",
                       addr, MAX_PATTERN_LENGTH));
        return -1;
    }
    LOG(logDEBUG1, ("Setting Pattern Word (addr:0x%x, word:0x%llx)\n", addr,
                    (long long int)word));

    // write word
    uint32_t reg_lsb =
        PATTERN_STEP0_LSB_REG +
        addr * REG_OFFSET * 2; // the first word in RAM as base plus the
                               // offset of the word to write (addr)
    uint32_t reg_msb = PATTERN_STEP0_MSB_REG + addr * REG_OFFSET * 2;
    set64BitReg(word, reg_lsb, reg_msb);

    LOG(logDEBUG1, ("  Wrote word. PatternIn Reg: 0x%llx\n",
                    get64BitReg(reg_lsb, reg_msb)));
    return readPatternWord(addr);
}

int setPatternWaitAddress(int level, int addr) {
    // error (handled in tcp)
    if (addr >= MAX_PATTERN_LENGTH) {
        LOG(logERROR, ("Cannot set Pattern Wait Address. Invalid addr 0x%x. "
                       "Should be between 0 and 0x%x\n",
                       addr, MAX_PATTERN_LENGTH));
        return -1;
    }

    uint32_t reg = 0;
    uint32_t offset = 0;
    uint32_t mask = 0;

    switch (level) {
    case 0:
        reg = PATTERN_WAIT_0_ADDR_REG;
        offset = PATTERN_WAIT_0_ADDR_OFST;
        mask = PATTERN_WAIT_0_ADDR_MSK;
        break;
    case 1:
        reg = PATTERN_WAIT_1_ADDR_REG;
        offset = PATTERN_WAIT_1_ADDR_OFST;
        mask = PATTERN_WAIT_1_ADDR_MSK;
        break;
    case 2:
        reg = PATTERN_WAIT_2_ADDR_REG;
        offset = PATTERN_WAIT_2_ADDR_OFST;
        mask = PATTERN_WAIT_2_ADDR_MSK;
        break;
    default:
        LOG(logERROR, ("Cannot set Pattern Wait Address. Invalid level 0x%x. "
                       "Should be between 0 and 2.\n",
                       level));
        return -1;
    }

    // set
    if (addr >= 0) {
        LOG(trimmingPrint,
            ("Setting Pattern Wait Address (level:%d, addr:0x%x)\n", level,
             addr));
        bus_w(reg, ((addr << offset) & mask));
    }

    // get
    uint32_t regval = ((bus_r(reg) & mask) >> offset);
    LOG(logDEBUG1,
        ("  Wait Address retval (level:%d, addr:0x%x)\n", level, regval));
    return regval;
}

uint64_t setPatternWaitTime(int level, uint64_t t) {
    uint32_t regl = 0;
    uint32_t regm = 0;

    switch (level) {
    case 0:
        regl = PATTERN_WAIT_TIMER_0_LSB_REG;
        regm = PATTERN_WAIT_TIMER_0_MSB_REG;
        break;
    case 1:
        regl = PATTERN_WAIT_TIMER_1_LSB_REG;
        regm = PATTERN_WAIT_TIMER_1_MSB_REG;
        break;
    case 2:
        regl = PATTERN_WAIT_TIMER_2_LSB_REG;
        regm = PATTERN_WAIT_TIMER_2_MSB_REG;
        break;
    default:
        LOG(logERROR, ("Cannot set Pattern Wait Time. Invalid level %d. "
                       "Should be between 0 and 2.\n",
                       level));
        return -1;
    }

    // set
    if ((int64_t)t >= 0) {
        LOG(trimmingPrint, ("Setting Pattern Wait Time (level:%d, t:%lld)\n",
                            level, (long long int)t));
        set64BitReg(t, regl, regm);
    }

    // get
    uint64_t regval = get64BitReg(regl, regm);
    LOG(logDEBUG1, ("  Wait Time retval (level:%d, t:%lld)\n", level,
                    (long long int)regval));
    return regval;
}

void setPatternLoop(int level, int *startAddr, int *stopAddr, int *nLoop) {

    // (checked at tcp)
    if (*startAddr >= MAX_PATTERN_LENGTH || *stopAddr >= MAX_PATTERN_LENGTH) {
        LOG(logERROR, ("Cannot set Pattern Loop, Address (startaddr:0x%x, "
                       "stopaddr:0x%x) must be "
                       "less than 0x%x\n",
                       *startAddr, *stopAddr, MAX_PATTERN_LENGTH));
        *startAddr = -1;
        *stopAddr = -1;
        *nLoop = -1;
        return;
    }

    uint32_t addr = 0;
    uint32_t nLoopReg = 0;
    uint32_t startOffset = 0;
    uint32_t startMask = 0;
    uint32_t stopOffset = 0;
    uint32_t stopMask = 0;

    switch (level) {
    case 0:
        addr = PATTERN_LOOP_0_ADDR_REG;
        nLoopReg = PATTERN_LOOP_0_ITERATION_REG;
        startOffset = PATTERN_LOOP_0_ADDR_STRT_OFST;
        startMask = PATTERN_LOOP_0_ADDR_STRT_MSK;
        stopOffset = PATTERN_LOOP_0_ADDR_STP_OFST;
        stopMask = PATTERN_LOOP_0_ADDR_STP_MSK;
        break;
    case 1:
        addr = PATTERN_LOOP_1_ADDR_REG;
        nLoopReg = PATTERN_LOOP_1_ITERATION_REG;
        startOffset = PATTERN_LOOP_1_ADDR_STRT_OFST;
        startMask = PATTERN_LOOP_1_ADDR_STRT_MSK;
        stopOffset = PATTERN_LOOP_1_ADDR_STP_OFST;
        stopMask = PATTERN_LOOP_1_ADDR_STP_MSK;
        break;
    case 2:
        addr = PATTERN_LOOP_2_ADDR_REG;
        nLoopReg = PATTERN_LOOP_2_ITERATION_REG;
        startOffset = PATTERN_LOOP_2_ADDR_STRT_OFST;
        startMask = PATTERN_LOOP_2_ADDR_STRT_MSK;
        stopOffset = PATTERN_LOOP_2_ADDR_STP_OFST;
        stopMask = PATTERN_LOOP_2_ADDR_STP_MSK;
        break;
    case -1:
        // complete pattern
        addr = PATTERN_LIMIT_REG;
        nLoopReg = -1;
        startOffset = PATTERN_LIMIT_STRT_OFST;
        startMask = PATTERN_LIMIT_STRT_MSK;
        stopOffset = PATTERN_LIMIT_STP_OFST;
        stopMask = PATTERN_LIMIT_STP_MSK;
        break;
    default:
        // already checked at tcp interface
        LOG(logERROR, ("Cannot set Pattern loop. Invalid level %d. "
                       "Should be between -1 and 2.\n",
                       level));
        *startAddr = 0;
        *stopAddr = 0;
        *nLoop = 0;
    }

    // set iterations
    if (level >= 0) {
        // set iteration
        if (*nLoop >= 0) {
            LOG(trimmingPrint,
                ("Setting Pattern Loop (level:%d, nLoop:%d)\n", level, *nLoop));
            bus_w(nLoopReg, *nLoop);
        }
        *nLoop = bus_r(nLoopReg);
    }

    // set
    if (*startAddr >= 0 && *stopAddr >= 0) {
        // writing start and stop addr
        LOG(trimmingPrint, ("Setting Pattern Loop (level:%d, startaddr:0x%x, "
                            "stopaddr:0x%x)\n",
                            level, *startAddr, *stopAddr));
        bus_w(addr, ((*startAddr << startOffset) & startMask) |
                        ((*stopAddr << stopOffset) & stopMask));
    }

    *startAddr = ((bus_r(addr) & startMask) >> startOffset);
    LOG(logDEBUG1, ("Getting Pattern Loop Start Address (level:%d, Read "
                    "startAddr:0x%x)\n",
                    level, *startAddr));

    *stopAddr = ((bus_r(addr) & stopMask) >> stopOffset);
    LOG(logDEBUG1, ("Getting Pattern Loop Stop Address (level:%d, Read "
                    "stopAddr:0x%x)\n",
                    level, *stopAddr));
}

void setPatternMask(uint64_t mask) {
    set64BitReg(mask, PATTERN_MASK_LSB_REG, PATTERN_MASK_MSB_REG);
}

uint64_t getPatternMask() {
    return get64BitReg(PATTERN_MASK_LSB_REG, PATTERN_MASK_MSB_REG);
}

void setPatternBitMask(uint64_t mask) {
    set64BitReg(mask, PATTERN_SET_LSB_REG, PATTERN_SET_MSB_REG);
}

uint64_t getPatternBitMask() {
    return get64BitReg(PATTERN_SET_LSB_REG, PATTERN_SET_MSB_REG);
}

int checkDetectorType() {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Checking type of module\n"));
    FILE *fd = fopen(TYPE_FILE_NAME, "r");
    if (fd == NULL) {
        LOG(logERROR,
            ("Could not open file %s to get type of the module attached\n",
             TYPE_FILE_NAME));
        return -1;
    }
    char buffer[MAX_STR_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fread(buffer, MAX_STR_LENGTH, sizeof(char), fd);
    fclose(fd);
    if (strlen(buffer) == 0) {
        LOG(logERROR,
            ("Could not read file %s to get type of the module attached\n",
             TYPE_FILE_NAME));
        return -1;
    }
    int type = atoi(buffer);
    if (type > TYPE_NO_MODULE_STARTING_VAL) {
        LOG(logERROR, ("No Module attached! Expected %d for Mythen, got %d\n",
                       TYPE_MYTHEN3_MODULE_VAL, type));
        return -2;
    }

    if (abs(type - TYPE_MYTHEN3_MODULE_VAL) > TYPE_TOLERANCE) {
        LOG(logERROR,
            ("Wrong Module attached! Expected %d for Mythen3, got %d\n",
             TYPE_MYTHEN3_MODULE_VAL, type));
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
    return (getVCOFrequency(ind) / clkDivider[ind]);
}

int getVCOFrequency(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get vco frequency\n", ind));
        return -1;
    }
    int pllIndex = (int)(ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL);
    return ALTERA_PLL_C10_GetVCOFrequency(pllIndex);
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
    return OK;
}

int getClockDivider(enum CLKINDEX ind) {
    if (ind < 0 || ind >= NUM_CLOCKS) {
        LOG(logERROR, ("Unknown clock index %d to get clock divider\n", ind));
        return -1;
    }
    return clkDivider[ind];
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
    }
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

    const int64_t periodNs = getPeriod();
    const int numFrames = (getNumFrames() * getNumTriggers());
    const int64_t expUs = getGatePeriod() / 1000;

    const int imageSize = calculateDataBytes();
    const int tgEnable = enableTenGigabitEthernet(-1);
    const int dr = setDynamicRange(-1);
    int ncounters = __builtin_popcount(getCounterMask());
    int dataSize = 0;
    int packetsPerFrame = 0;
    // 10g
    if (tgEnable) {
        packetsPerFrame = 1;
        if (dr == 32 && ncounters > 1) {
            packetsPerFrame = 2;
        }
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

        switch (dr) {
        /*case 1: // TODO: Not implemented in firmware yet
                break;*/
        case 8:
            for (int i = 0; i < nchannels; ++i) {
                *((uint8_t *)(imageData + i)) = (uint8_t)i;
            }
            break;
        case 16:
            for (int i = 0; i < nchannels; ++i) {
                *((uint16_t *)(imageData + i * sizeof(uint16_t))) = (uint16_t)i;
            }
            break;
        case 32:
            for (int i = 0; i < nchannels; ++i) {
                *((uint32_t *)(imageData + i * sizeof(uint32_t))) =
                    ((uint32_t)i & 0xFFFFFF); // 24 bit
            }
            break;
        default:
            break;
        }
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
            sls_detector_header *header = (sls_detector_header *)(packetData);
            header->detType = (uint16_t)myDetectorType;
            header->version = SLS_DETECTOR_HEADER_VERSION - 1;
            header->frameNumber = virtual_currentFrameNumber;
            header->packetNumber = i;
            header->modId = 0;
            header->row = detPos[X];
            header->column = detPos[Y];

            // fill data
            memcpy(packetData + sizeof(sls_detector_header),
                   imageData + srcOffset, dataSize);
            srcOffset += dataSize;

            sendUDPPacket(0, packetData, packetSize);
        }
        LOG(logINFO, ("Sent frame: %d [%lld]\n", frameNr,
                      (long long unsigned int)virtual_currentFrameNumber));
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
    }

    closeUDPSocket(0);

    sharedMemory_setStatus(IDLE);
    LOG(logINFOBLUE, ("Finished Acquiring\n"));
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

void readFrame(int *ret, char *mess) {
    // wait for status to be done
    while (runBusy()) {
        usleep(500);
    }

#ifdef VIRTUAL
    LOG(logINFOGREEN, ("acquisition successfully finished\n"));
    return;
#endif

    *ret = (int)OK;
    // frames left to give status
    int64_t retval = getNumFramesLeft() + 1;

    if (retval > 0) {
        LOG(logERROR, ("No data and run stopped: %lld frames left\n",
                       (long long int)retval));
    } else {
        LOG(logINFOGREEN, ("Acquisition successfully finished\n"));
    }
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

int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod) {
    LOG(logDEBUG1, ("Copying module\n"));

    if (srcMod->serialnumber >= 0) {
        destMod->serialnumber = srcMod->serialnumber;
    }
    // no trimbit feature
    if (destMod->nchan && ((srcMod->nchan) > (destMod->nchan))) {
        LOG(logINFO, ("Number of channels of source is larger than number of "
                      "channels of destination\n"));
        return FAIL;
    }
    if ((srcMod->ndac) > (destMod->ndac)) {
        LOG(logINFO, ("Number of dacs of source is larger than number of dacs "
                      "of destination\n"));
        return FAIL;
    }

    LOG(logDEBUG1, ("DACs: src %d, dest %d\n", srcMod->ndac, destMod->ndac));
    LOG(logDEBUG1, ("Chans: src %d, dest %d\n", srcMod->nchan, destMod->nchan));
    destMod->ndac = srcMod->ndac;
    destMod->nchip = srcMod->nchip;
    destMod->nchan = srcMod->nchan;
    if (srcMod->reg >= 0)
        destMod->reg = srcMod->reg;
    /*
    if (srcMod->iodelay >= 0)
        destMod->iodelay = srcMod->iodelay;
    if (srcMod->tau >= 0)
        destMod->tau = srcMod->tau;
    */
    for (int i = 0; i < NCOUNTERS; ++i) {
        if (srcMod->eV[i] >= 0)
            destMod->eV[i] = srcMod->eV[i];
    }

    LOG(logDEBUG1, ("Copying register %x (%x)\n", destMod->reg, srcMod->reg));

    if (destMod->nchan != 0) {
        for (int ichan = 0; ichan < (srcMod->nchan); ichan++) {
            *((destMod->chanregs) + ichan) = *((srcMod->chanregs) + ichan);
        }
    } else
        LOG(logINFO, ("Not Copying trimbits\n"));

    for (int idac = 0; idac < (srcMod->ndac); idac++) {
        if (*((srcMod->dacs) + idac) >= 0) {
            *((destMod->dacs) + idac) = *((srcMod->dacs) + idac);
        }
    }
    return OK;
}

int calculateDataBytes() {
    int numCounters = __builtin_popcount(getCounterMask());
    int dr = setDynamicRange(-1);
    return (NCHAN_1_COUNTER * NCHIP * numCounters * ((double)dr / 8.00));
}

int getTotalNumberOfChannels() {
    return (getNumberOfChannelsPerChip() * getNumberOfChips());
}
int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }

int setChipStatusRegister(int csr) {
  int iaddr=0;
  int  nbits=18;
  int error=0;
  //int start=0, stop=MAX_PATTERN_LENGTH, loop=0;
  int patword=0;
  patword=setBit(SIGNAL_STATLOAD,patword);
  for (int i=0; i<2; i++)
    writePatternWord(iaddr++, patword);
  patword=setBit(SIGNAL_resStorage,patword);
  patword=setBit(SIGNAL_resCounter,patword);
  for (int i=0; i<8; i++)
    writePatternWord(iaddr++, patword);
  patword=clearBit(SIGNAL_resStorage,patword);
  patword=clearBit(SIGNAL_resCounter,patword);
  for (int i=0; i<8; i++)
    writePatternWord(iaddr++, patword);
  //#This version of the serializer pushes in the MSB first (compatible with the CSR bit numbering)
  for (int ib=nbits-1; ib>=0; ib--) {
    if (csr&(1<<ib))
      patword=setBit(SIGNAL_serialIN,patword);
    else
      patword=clearBit(SIGNAL_serialIN,patword);
    for (int i=0; i<4; i++)
      writePatternWord(iaddr++, patword);
    patword=setBit(SIGNAL_CHSclk,patword);
    writePatternWord(iaddr++, patword);
    patword=clearBit(SIGNAL_CHSclk,patword);
    writePatternWord(iaddr++, patword);
  }

  patword=clearBit(SIGNAL_serialIN,patword);
  for (int i=0; i<2; i++)
      writePatternWord(iaddr++, patword);
  patword=setBit(SIGNAL_STO,patword);
  for (int i=0; i<5; i++)
      writePatternWord(iaddr++, patword);
  patword=clearBit(SIGNAL_STO,patword);
  for (int i=0; i<5; i++)
    writePatternWord(iaddr++, patword);
  patword=clearBit(SIGNAL_STATLOAD,patword);
  for (int i=0; i<5; i++)
    writePatternWord(iaddr++, patword);

  if (iaddr >= MAX_PATTERN_LENGTH) {
    LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n",
		   iaddr, MAX_PATTERN_LENGTH));
    error = 1;
  }
  // set pattern wait address
  for (int i = 0; i <= 2; i++)
    setPatternWaitAddress(i, MAX_PATTERN_LENGTH - 1);
  
  // pattern loop
  for (int i = 0; i <= 2; i++) {
    int stop = MAX_PATTERN_LENGTH - 1, nloop = 0;
    setPatternLoop(i, &stop, &stop, &nloop);
  }
  
  // pattern limits
  {
    int start = 0, nloop = 0;
    setPatternLoop(-1, &start, &iaddr, &nloop);
  }
  // send pattern to the chips
  startPattern();
  
  if (error != 0) {
    return FAIL;
  }
  
  return OK;

}
