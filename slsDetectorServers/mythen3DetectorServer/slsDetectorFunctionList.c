#include "slsDetectorFunctionList.h"
#include "ALTERA_PLL_CYCLONE10.h"
#include "DAC6571.h"
#include "LTC2620_Driver.h"
#include "RegisterDefs.h"
#include "clogger.h"
#include "common.h"
#include "versionAPI.h"
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#include "communication_virtual.h"
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
int virtual_status = 0;
int virtual_stop = 0;
#endif

sls_detector_module *detectorModules = NULL;
int *detectorChans = NULL;
int *detectorDacs = NULL;

enum TLogLevel trimmingPrint = logINFO;
int32_t clkPhase[NUM_CLOCKS] = {};
uint32_t clkDivider[NUM_CLOCKS] = {};

int highvoltage = 0;
int detPos[2] = {};
uint32_t countermask =
    0; // will be removed later when in firmware converted to mask

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
    if ((!debugflag) && ((checkType() == FAIL) || (testFpga() == FAIL) ||
                         (testBus() == FAIL))) {
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
    if (debugflag) {
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
                "This detector software software version (0x%llx) is "
                "incompatible.\n"
                "Please update detector software (min. 0x%llx) to be "
                "compatible with this firmware.\n",
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
    return bus_r(MCB_SERIAL_NO_REG);
}

u_int32_t getDetectorNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(MCB_SERIAL_NO_REG) & MCB_SERIAL_NO_VRSN_MSK) >>
            MCB_SERIAL_NO_VRSN_OFST);
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
    if (initError == OK) {
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
    virtual_stop = 0;
    if (!isControlServer) {
        ComVirtual_setStop(virtual_stop);
    }
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
    (detectorModules)->eV = 0;
    // thisSettings = UNINITIALIZED;

    // initialize dacs
    for (int idac = 0; idac < (detectorModules)->ndac; ++idac) {
        detectorDacs[idac] = 0;
    }

    // if trimval requested, should return -1 to acknowledge unknown
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        *((detectorModules->chanregs) + ichan) = -1;
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

    highvoltage = 0;
    trimmingPrint = logINFO;
    for (int i = 0; i < NUM_CLOCKS; ++i) {
        clkPhase[i] = 0;
    }
#ifdef VIRTUAL
    virtual_status = 0;
    if (isControlServer) {
        ComVirtual_setStatus(virtual_status);
    }
#endif

    // pll defines
    ALTERA_PLL_C10_SetDefines(REG_OFFSET, BASE_READOUT_PLL, BASE_SYSTEM_PLL,
                              PLL_RESET_REG, PLL_RESET_REG,
                              PLL_RESET_READOUT_MSK, PLL_RESET_SYSTEM_MSK,
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
    for (int i = 0; i != 3; ++i) {
        setExpTime(i, DEFAULT_GATE_WIDTH);
        setGateDelay(i, DEFAULT_GATE_DELAY);
    }
}

int setDefaultDacs() {
    int ret = OK;
    LOG(logINFOBLUE, ("Setting Default Dac values\n"));
    {
        const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
        for (int i = 0; i < NDAC; ++i) {
            setDAC((enum DACINDEX)i, defaultvals[i], 0);
            if (detectorDacs[i] != defaultvals[i]) {
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
        case 1:
            regval = CONFIG_DYNAMIC_RANGE_1_VAL;
            break;
        case 4:
            regval = CONFIG_DYNAMIC_RANGE_4_VAL;
            break;
        case 16:
            regval = CONFIG_DYNAMIC_RANGE_16_VAL;
            break;
        case 24:
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
    }

    uint32_t regval = bus_r(CONFIG_REG) & CONFIG_DYNAMIC_RANGE_MSK;
    switch (regval) {
    case CONFIG_DYNAMIC_RANGE_1_VAL:
        return 1;
    case CONFIG_DYNAMIC_RANGE_4_VAL:
        return 4;
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

/* parameters - module, speed, readout */

int setModule(sls_detector_module myMod, char *mess) {

    LOG(logINFO, ("Setting module\n"));

    /* future implementation
    // settings (not yet implemented)
    setSettings((enum detectorSettings)myMod.reg);
    if (myMod.reg >= 0) {
        detectorModules->reg = myMod.reg;
    }

    // threshold
    if (myMod.eV >= 0)
        setThresholdEnergy(myMod.eV);
    else {
        // (loading a random trim file) (dont return fail)
        setSettings(UNDEFINED);
        LOG(logERROR,
            ("Settings has been changed to undefined (random trim
            file)\n"));
    }
    */

    // dacs
    for (int i = 0; i < NDAC; ++i) {
        // ignore dacs with -1
        if (myMod.dacs[i] != -1) {
            setDAC((enum DACINDEX)i, myMod.dacs[i], 0);
            if (myMod.dacs[i] != detectorDacs[i]) {
                sprintf(mess, "Could not set module. Could not set dac %d\n",
                        i);
                LOG(logERROR, (mess));
                // setSettings(UNDEFINED);
                // LOG(logERROR, ("Settings has been changed to undefined\n"));
                return FAIL;
            }
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
            // setSettings(UNDEFINED);
            // LOG(logERROR, ("Settings has been changed to undefined (random "
            //               "trim file)\n"));
            return FAIL;
        }
    }

    return OK;
}

int getModule(sls_detector_module *myMod) {
    // copy local module to myMod
    if (detectorModules) {
        if (copyModule(myMod, detectorModules) == FAIL)
            return FAIL;
    } else
        return FAIL;
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

    uint64_t patword = 0;
    int iaddr = 0;
    for (int ichip = 0; ichip < NCHIP; ichip++) {
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
            trimmingPrint = logINFO;
            return FAIL;
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

    // copy trimbits locally
    for (int ichan = 0; ichan < ((detectorModules)->nchan); ++ichan) {
        detectorChans[ichan] = trimbits[ichan];
    }
    trimmingPrint = logINFO;
    LOG(logINFO, ("All trimbits have been loaded\n"));
    return OK;
}

int setAllTrimbits(int val) {
    int *trimbits = malloc(sizeof(int) * ((detectorModules)->nchan));
    for (int ichan = 0; ichan < ((detectorModules)->nchan); ++ichan) {
        trimbits[ichan] = val;
    }
    if (setTrimbits(trimbits) == FAIL) {
        LOG(logERROR, ("Could not set all trimbits to %d\n", val));
        free(trimbits);
        return FAIL;
    }
    // setSettings(UNDEFINED);
    // LOG(logERROR, ("Settings has been changed to undefined (random "
    //               "trim file)\n"));
    LOG(logINFO, ("All trimbits have been set to %d\n", val));
    free(trimbits);
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
    val *= (1E-9 * getFrequency(SYSTEM_C2));
    set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-9 * getFrequency(SYSTEM_C2));
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return get64BitReg(SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C2));
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
    for (int i = 0; i != 3; ++i) {
        // TODO: only those counters enabled (when updated to mask in firmware)
        uint64_t sum = getExpTime(i) + getGateDelay(i);
        if (sum > max) {
            max = sum;
        }
    }
    LOG(logINFO, ("\tSetting Gate Period to %lld ns\n", (long long int)max));
    max *= (1E-9 * getFrequency(SYSTEM_C2));
    set64BitReg(max, ASIC_EXP_GATE_PERIOD_LSB_REG,
                ASIC_EXP_GATE_PERIOD_MSB_REG);
}

int64_t getGatePeriod() {
    return get64BitReg(ASIC_EXP_GATE_PERIOD_LSB_REG,
                       ASIC_EXP_GATE_PERIOD_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C2));
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
        LOG(logERROR, ("Invalid exptime (index:%d): %lld ns\n", gateIndex,
                       (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting exptime %lld ns (index:%d)\n", (long long int)val,
                  gateIndex));
    val *= (1E-9 * getFrequency(SYSTEM_C2));
    set64BitReg(val, alsb, amsb);

    // validate for tolerance
    int64_t retval = getExpTime(gateIndex);
    val /= (1E-9 * getFrequency(SYSTEM_C2));
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
    return get64BitReg(alsb, amsb) / (1E-9 * getFrequency(SYSTEM_C2));
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
        LOG(logERROR, ("Invalid gate delay (index:%d): %lld ns\n", gateIndex,
                       (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting gate delay %lld ns (index:%d)\n", (long long int)val,
                  gateIndex));
    val *= (1E-9 * getFrequency(SYSTEM_C2));
    set64BitReg(val, alsb, amsb);

    // validate for tolerance
    int64_t retval = getGateDelay(gateIndex);
    val /= (1E-9 * getFrequency(SYSTEM_C2));
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
    return get64BitReg(alsb, amsb) / (1E-9 * getFrequency(SYSTEM_C2));
}

void setCounterMask(uint32_t arg) {
    if (arg == 0 || arg > MAX_COUNTER_MSK) {
        return;
    }
    countermask = arg;
    // convert mask into number of counters (until firmware converts to mask)
    int ncounters = __builtin_popcount(countermask);
    LOG(logINFO, ("Setting number of counters to %d\n", ncounters));
    uint32_t val = 0;
    switch (ncounters) {
    case 1:
        val = CONFIG_COUNTER_ENA_1_VAL;
        break;
    case 2:
        val = CONFIG_COUNTER_ENA_2_VAL;
        break;
    default:
        val = CONFIG_COUNTER_ENA_ALL_VAL;
        break;
    }
    uint32_t addr = CONFIG_REG;
    bus_w(addr, bus_r(addr) & ~CONFIG_COUNTER_ENA_MSK);
    bus_w(addr, bus_r(addr) | val);
    LOG(logDEBUG, ("Config Reg: 0x%x\n", bus_r(addr)));

    updateGatePeriod();
}

uint32_t getCounterMask() {
    uint32_t addr = CONFIG_REG;
    uint32_t regval = (bus_r(addr) & CONFIG_COUNTER_ENA_MSK);
    int ncounters = 0;
    switch (regval) {
    case CONFIG_COUNTER_ENA_1_VAL:
        ncounters = 1;
        break;
    case CONFIG_COUNTER_ENA_2_VAL:
        ncounters = 2;
        break;
    default:
        ncounters = 3;
        break;
    }
    // confirm ncounters work with mask saved in server (until firmware converts
    // to mask)
    int nc = __builtin_popcount(countermask);
    // if not equal, make a mask of what is in register (will change once
    // firmware changes)
    if (nc != ncounters) {
        switch (ncounters) {
        case 1:
            countermask = 0x1;
            break;
        case 2:
            countermask = 0x3;
            break;
        default:
            countermask = 0x7;
            break;
        }
    }
    return countermask;
}

int setDelayAfterTrigger(int64_t val) {
    if (val < 0) {
        LOG(logERROR,
            ("Invalid delay after trigger: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting delay after trigger %lld ns\n", (long long int)val));
    val *= (1E-9 * getFrequency(SYSTEM_C2));
    set64BitReg(val, SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG);

    // validate for tolerance
    int64_t retval = getDelayAfterTrigger();
    val /= (1E-9 * getFrequency(SYSTEM_C2));
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getDelayAfterTrigger() {
    return get64BitReg(SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C2));
}

int64_t getNumFramesLeft() {
    return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t getNumTriggersLeft() {
    return get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
}

int64_t getDelayAfterTriggerLeft() {
    return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C2));
}

int64_t getPeriodLeft() {
    return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) /
           (1E-9 * getFrequency(SYSTEM_C2));
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

/* parameters - dac, hv */
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
}

int getDAC(enum DACINDEX ind, int mV) {
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
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif

    // setting hv
    if (val >= 0) {
        LOG(logINFO, ("Setting High voltage: %d V\n", val));
        DAC6571_Set(val);
        highvoltage = val;
    }
    return highvoltage;
}

/* parameters - timing */
void setTiming(enum timingMode arg) {
    if (arg != GET_TIMING_MODE) {
        switch (arg) {
        case AUTO_TIMING:
            LOG(logINFO, ("Set Timing: Auto (Int. Trigger, Int. Gating)\n"));
            bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) & ~EXT_SIGNAL_MSK);
            bus_w(ASIC_EXP_STATUS_REG,
                  bus_r(ASIC_EXP_STATUS_REG) & ~ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
            break;
        case TRIGGER_EXPOSURE:
            LOG(logINFO, ("Set Timing: Trigger (Ext. Trigger, Int. Gating)\n"));
            bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) | EXT_SIGNAL_MSK);
            bus_w(ASIC_EXP_STATUS_REG,
                  bus_r(ASIC_EXP_STATUS_REG) & ~ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
            break;
        case GATED:
            LOG(logINFO, ("Set Timing: Gating (Int. Trigger, Ext. Gating)\n"));
            bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) & ~EXT_SIGNAL_MSK);
            bus_w(ASIC_EXP_STATUS_REG,
                  bus_r(ASIC_EXP_STATUS_REG) | ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
            break;
        case TRIGGER_GATED:
            LOG(logINFO,
                ("Set Timing: Trigger_Gating (Ext. Trigger, Ext. Gating)\n"));
            bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) | EXT_SIGNAL_MSK);
            bus_w(ASIC_EXP_STATUS_REG,
                  bus_r(ASIC_EXP_STATUS_REG) | ASIC_EXP_STAT_GATE_SRC_EXT_MSK);
            break;
        default:
            LOG(logERROR, ("Unknown timing mode %d\n", arg));
            return;
        }
    }
}

enum timingMode getTiming() {
    uint32_t extTrigger = (bus_r(EXT_SIGNAL_REG) & EXT_SIGNAL_MSK);
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

    LOG(trimmingPrint, ("  Reading Pattern Word (addr:0x%x)\n", addr));
    uint32_t reg_lsb =
        PATTERN_STEP0_LSB_REG +
        addr * REG_OFFSET * 2; // the first word in RAM as base plus the offset
                               // of the word to write (addr)
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
    LOG(trimmingPrint, ("Setting Pattern Word (addr:0x%x, word:0x%llx)\n", addr,
                        (long long int)word));

    // write word
    uint32_t reg_lsb =
        PATTERN_STEP0_LSB_REG +
        addr * REG_OFFSET * 2; // the first word in RAM as base plus the offset
                               // of the word to write (addr)
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

    // get
    else {
        *startAddr = ((bus_r(addr) & startMask) >> startOffset);
        LOG(logDEBUG1, ("Getting Pattern Loop Start Address (level:%d, Read "
                        "startAddr:0x%x)\n",
                        level, *startAddr));

        *stopAddr = ((bus_r(addr) & stopMask) >> stopOffset);
        LOG(logDEBUG1, ("Getting Pattern Loop Stop Address (level:%d, Read "
                        "stopAddr:0x%x)\n",
                        level, *stopAddr));
    }
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
    if (ind >= READOUT_C0) {
        clkPhase[READOUT_C0] = 0;
        clkPhase[READOUT_C1] = 0;
    } else {
        clkPhase[SYSTEM_C0] = 0;
        clkPhase[SYSTEM_C1] = 0;
        clkPhase[SYSTEM_C2] = 0;
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

/* aquisition */

int startStateMachine() {
#ifdef VIRTUAL
    // create udp socket
    if (createUDPSocket(0) != OK) {
        return FAIL;
    }
    LOG(logINFOBLUE, ("Starting State Machine\n"));
    // set status to running
    virtual_status = 1;
    if (isControlServer) {
        ComVirtual_setStatus(virtual_status);
        virtual_stop = ComVirtual_getStop();
        if (virtual_stop != 0) {
            LOG(logERROR, ("Cant start acquisition. "
                           "Stop server has not updated stop status to 0\n"));
            return FAIL;
        }
    }
    if (pthread_create(&pthread_virtual_tid, NULL, &start_timer, NULL)) {
        LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
        virtual_status = 0;
        if (isControlServer) {
            ComVirtual_setStatus(virtual_status);
        }
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

    int64_t periodNs = getPeriod();
    int numFrames = (getNumFrames() * getNumTriggers());
    int64_t expUs = getGatePeriod() / 1000;

    // int dr = setDynamicRange(-1);
    int imagesize = calculateDataBytes();
    int dataSize = imagesize / PACKETS_PER_FRAME;
    int packetSize = dataSize + sizeof(sls_detector_header);

    // Generate data
    char imageData[imagesize];
    memset(imageData, 0, imagesize);
    for (int i = 0; i < imagesize; i += sizeof(uint8_t)) {
        *((uint8_t *)(imageData + i)) = i;
    }

    // Send data
    // loop over number of frames
    for (int frameNr = 0; frameNr != numFrames; ++frameNr) {

        // update the virtual stop from stop server
        virtual_stop = ComVirtual_getStop();
        // check if virtual_stop is high
        if (virtual_stop == 1) {
            break;
        }

        // sleep for exposure time
        struct timespec begin, end;
        clock_gettime(CLOCK_REALTIME, &begin);
        usleep(expUs);

        int srcOffset = 0;
        // loop packet
        for (int i = 0; i != PACKETS_PER_FRAME; ++i) {
            char packetData[packetSize];
            memset(packetData, 0, packetSize);

            // set header
            sls_detector_header *header = (sls_detector_header *)(packetData);
            header->detType = (uint16_t)myDetectorType;
            header->version = SLS_DETECTOR_HEADER_VERSION - 1;
            header->frameNumber = frameNr + 1;
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
        LOG(logINFO, ("Sent frame: %d\n", frameNr));
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

    virtual_status = 0;
    if (isControlServer) {
        ComVirtual_setStatus(virtual_status);
    }
    LOG(logINFOBLUE, ("Finished Acquiring\n"));
    return NULL;
}
#endif

int stopStateMachine() {
    LOG(logINFORED, ("Stopping State Machine\n"));
#ifdef VIRTUAL
    if (!isControlServer) {
        virtual_stop = 1;
        ComVirtual_setStop(virtual_stop);
        // read till status is idle
        int tempStatus = 1;
        while (tempStatus == 1) {
            tempStatus = ComVirtual_getStatus();
        }
        virtual_stop = 0;
        ComVirtual_setStop(virtual_stop);
        LOG(logINFO, ("Stopped State Machine\n"));
    }
    return OK;
#endif
    // stop state machine
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STP_ACQSTN_MSK);
    LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    return OK;
}

enum runStatus getRunStatus() {
#ifdef VIRTUAL
    if (!isControlServer) {
        virtual_status = ComVirtual_getStatus();
    }
    if (virtual_status == 0) {
        LOG(logINFOBLUE, ("Status: IDLE\n"));
        return IDLE;
    } else {
        LOG(logINFOBLUE, ("Status: RUNNING\n"));
        return RUNNING;
    }
#endif
    LOG(logDEBUG1, ("Getting status\n"));
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
    if (!isControlServer) {
        virtual_status = ComVirtual_getStatus();
    }
    return virtual_status;
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
    if (srcMod->eV >= 0)
        destMod->eV = srcMod->eV;
    */
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
    int databytes = NCHAN_1_COUNTER * NCHIP * numCounters *
                    ((dr > 16) ? 4 :            // 32 bit
                         ((dr > 8) ? 2 :        // 16 bit
                              ((dr > 4) ? 0.5 : // 4 bit
                                   0.125)));    // 1 bit

    return databytes;
}

int getTotalNumberOfChannels() {
    return (getNumberOfChannelsPerChip() * getNumberOfChips());
}
int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }
