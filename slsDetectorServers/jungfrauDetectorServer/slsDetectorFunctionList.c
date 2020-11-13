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

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
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
int virtual_image_test_mode = 0;
#endif

enum detectorSettings thisSettings = UNINITIALIZED;
int highvoltage = 0;
int dacValues[NDAC] = {};
int32_t clkPhase[NUM_CLOCKS] = {};
int detPos[4] = {};

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
    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Dangerous to continue.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
    }
    return;
#else
    defineGPIOpins();
    resetFPGA();
    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Dangerous to continue.\n");
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }

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

    uint16_t hversion = getHardwareVersionNumber();
    uint16_t hsnumber = getHardwareSerialNumber();
    uint32_t ipadd = getDetectorIP();
    uint64_t macadd = getDetectorMAC();
    int64_t fwversion = getFirmwareVersion();
    int64_t swversion = getServerVersion();
    int64_t sw_fw_apiversion = 0;
    int64_t client_sw_apiversion = getClientServerAPIVersion();
    uint32_t requiredFirmwareVersion =
        (isHardwareVersion2() ? REQRD_FRMWRE_VRSN_BOARD2 : REQRD_FRMWRE_VRSN);

    if (fwversion >= MIN_REQRD_VRSN_T_RD_API)
        sw_fw_apiversion = getFirmwareAPIVersion();
    LOG(logINFOBLUE,
        ("************ Jungfrau Server *********************\n"
         "Hardware Version:\t\t 0x%x\n"
         "Hardware Serial Nr:\t\t 0x%x\n"

         "Detector IP Addr:\t\t 0x%x\n"
         "Detector MAC Addr:\t\t 0x%llx\n\n"

         "Firmware Version:\t\t 0x%llx\n"
         "Software Version:\t\t 0x%llx\n"
         "F/w-S/w API Version:\t\t 0x%llx\n"
         "Required Firmware Version:\t 0x%x\n"
         "Client-Software API Version:\t 0x%llx\n"
         "********************************************************\n",
         hversion, hsnumber, ipadd, (long long unsigned int)macadd,
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

uint64_t getServerVersion() { return APIJUNGFRAU; }

uint64_t getClientServerAPIVersion() { return APIJUNGFRAU; }

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

// is board 1.0?, with value 2 (resistor network)
int isHardwareVersion2() {
    return (((bus_r(MOD_SERIAL_NUM_REG) & HARDWARE_VERSION_NUM_MSK) ==
             HARDWARE_VERSION_2_VAL)
                ? 1
                : 0);
}

u_int32_t getDetectorNumber() {
#ifdef VIRTUAL
    return 0;
#endif
    return bus_r(MOD_SERIAL_NUM_REG);
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
    if (!updateFlag && initError == OK) {
        setupDetector();
    }
    initCheckDone = 1;
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
    // temp threshold and reset event (read by stop server)
    setThresholdTemperature(DEFAULT_TMP_THRSHLD);
    setTemperatureEvent(0);
#endif
}

/* set up detector */

void setupDetector() {
    LOG(logINFO, ("This Server is for 1 Jungfrau module (500k)\n"));

    for (int i = 0; i < NUM_CLOCKS; ++i) {
        clkPhase[i] = 0;
    }
#ifdef VIRTUAL
    sharedMemory_setStatus(IDLE);
#endif

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
    AD9257_Disable();
    AD9257_Configure();

    // dac
    LTC2620_SetDefines(SPI_REG, SPI_DAC_SRL_CS_OTPT_MSK,
                       SPI_DAC_SRL_CLK_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_MSK,
                       SPI_DAC_SRL_DGTL_OTPT_OFST, NDAC, DAC_MIN_MV,
                       DAC_MAX_MV);
    LTC2620_Disable();
    LTC2620_Configure();
    setDefaultDacs();

    // altera pll
    ALTERA_PLL_SetDefines(
        PLL_CNTRL_REG, PLL_PARAM_REG, PLL_CNTRL_RCNFG_PRMTR_RST_MSK,
        PLL_CNTRL_WR_PRMTR_MSK, PLL_CNTRL_PLL_RST_MSK, PLL_CNTRL_ADDR_MSK,
        PLL_CNTRL_ADDR_OFST, PLL_CNTRL_DBIT_WR_PRMTR_MSK, DBIT_CLK_INDEX);

    bus_w(DAQ_REG, 0x0); /* Only once at server startup */

    LOG(logINFOBLUE, ("Setting Default parameters\n"));
    setClockDivider(RUN_CLK, HALF_SPEED);
    cleanFifos();
    resetCore();

    alignDeserializer();
    configureASICTimer();
    bus_w(ADC_PORT_INVERT_REG,
          (isHardwareVersion2() ? ADC_PORT_INVERT_BOARD2_VAL
                                : ADC_PORT_INVERT_VAL));

    initReadoutConfiguration();

    // Initialization of acquistion parameters
    setSettings(DEFAULT_SETTINGS);

    setNumFrames(DEFAULT_NUM_FRAMES);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setExpTime(DEFAULT_EXPTIME);
    setPeriod(DEFAULT_PERIOD);
    setDelayAfterTrigger(DEFAULT_DELAY);
    setNumAdditionalStorageCells(DEFAULT_NUM_STRG_CLLS);
    setStorageCellDelay(DEFAULT_STRG_CLL_DLY);
    selectStoragecellStart(DEFAULT_STRG_CLL_STRT);
    /*setClockDivider(RUN_CLK, HALF_SPEED); depends if all the previous stuff
     * works*/
    setTiming(DEFAULT_TIMING_MODE);
    setNextFrameNumber(DEFAULT_STARTING_FRAME_NUMBER);

    // temp threshold and reset event
    setThresholdTemperature(DEFAULT_TMP_THRSHLD);
    setTemperatureEvent(0);
}

int setDefaultDacs() {
    int ret = OK;
    LOG(logINFOBLUE, ("Setting Default Dac values\n"));
    const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
    for (int i = 0; i < NDAC; ++i) {
        setDAC((enum DACINDEX)i, defaultvals[i], 0);
        if (dacValues[i] != defaultvals[i]) {
            ret = FAIL;
            LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n", i,
                           defaultvals[i], dacValues[i]));
        }
    }
    return ret;
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

int setDynamicRange(int dr) { return DYNAMIC_RANGE; }

void setADCInvertRegister(uint32_t val) {
    LOG(logINFO, ("Setting ADC Port Invert Reg to 0x%x\n", val));
    uint32_t defaultValue = (isHardwareVersion2() ? ADC_PORT_INVERT_BOARD2_VAL
                                                  : ADC_PORT_INVERT_VAL);
    uint32_t changeValue = defaultValue ^ val;
    LOG(logINFO, ("\t default: 0x%x, final:0x%x\n", defaultValue, changeValue));
    bus_w(ADC_PORT_INVERT_REG, changeValue);
}

uint32_t getADCInvertRegister() {
    uint32_t readValue = bus_r(ADC_PORT_INVERT_REG);
    int32_t defaultValue = (isHardwareVersion2() ? ADC_PORT_INVERT_BOARD2_VAL
                                                 : ADC_PORT_INVERT_VAL);
    uint32_t val = defaultValue ^ readValue;
    LOG(logDEBUG1, ("\tread:0x%x, default:0x%x returned:0x%x\n", readValue,
                    defaultValue, val));
    return val;
}

/* parameters - timer */
int selectStoragecellStart(int pos) {
    if (pos >= 0) {
        LOG(logINFO, ("Setting storage cell start: %d\n", pos));
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_STRG_CELL_SLCT_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | ((pos << DAQ_STRG_CELL_SLCT_OFST) &
                                         DAQ_STRG_CELL_SLCT_MSK));
    }
    return ((bus_r(DAQ_REG) & DAQ_STRG_CELL_SLCT_MSK) >>
            DAQ_STRG_CELL_SLCT_OFST);
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

    // set settings
    switch (sett) {
    case DYNAMICGAIN:
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
        LOG(logINFO,
            ("Set settings - Dyanmic Gain, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
        break;
    case DYNAMICHG0:
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_HIGHGAIN_VAL);
        LOG(logINFO, ("Set settings - Dyanmic High Gain 0, DAQ Reg: 0x%x\n",
                      bus_r(DAQ_REG)));
        break;
    case FIXGAIN1:
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_STG_1_VAL);
        LOG(logINFO,
            ("Set settings - Fix Gain 1, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
        break;
    case FIXGAIN2:
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_STG_2_VAL);
        LOG(logINFO,
            ("Set settings - Fix Gain 2, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
        break;
    case FORCESWITCHG1:
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FRCE_GAIN_STG_1_VAL);
        LOG(logINFO, ("Set settings - Force Switch Gain 1, DAQ Reg: 0x%x\n",
                      bus_r(DAQ_REG)));
        break;
    case FORCESWITCHG2:
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FRCE_GAIN_STG_2_VAL);
        LOG(logINFO, ("Set settings - Force Switch Gain 2, DAQ Reg: 0x%x\n",
                      bus_r(DAQ_REG)));
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

    uint32_t regval = bus_r(DAQ_REG);
    uint32_t val = regval & DAQ_SETTINGS_MSK;
    LOG(logDEBUG1, ("Getting Settings\n Reading DAQ Register :0x%x\n", val));

    switch (val) {
    case DAQ_FIX_GAIN_DYNMC_VAL:
        thisSettings = DYNAMICGAIN;
        LOG(logDEBUG1,
            ("Settings read: Dynamic Gain. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FIX_GAIN_HIGHGAIN_VAL:
        thisSettings = DYNAMICHG0;
        LOG(logDEBUG1,
            ("Settings read: Dynamig High Gain. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FIX_GAIN_STG_1_VAL:
        thisSettings = FIXGAIN1;
        LOG(logDEBUG1, ("Settings read: Fix Gain 1. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FIX_GAIN_STG_2_VAL:
        thisSettings = FIXGAIN2;
        LOG(logDEBUG1, ("Settings read: Fix Gain 2. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FRCE_GAIN_STG_1_VAL:
        thisSettings = FORCESWITCHG1;
        LOG(logDEBUG1,
            ("Settings read: Force Switch Gain 1. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FRCE_GAIN_STG_2_VAL:
        thisSettings = FORCESWITCHG2;
        LOG(logDEBUG1,
            ("Settings read: Force Switch Gain 2. DAQ Reg: 0x%x\n", regval));
        break;
    default:
        thisSettings = UNDEFINED;
        LOG(logERROR, ("Settings read: Undefined. DAQ Reg: 0x%x\n", regval));
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
                 uint64_t destmac, uint32_t destport, uint64_t sourcemac,
                 uint32_t sourceip, uint32_t sourceport) {

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

int configureMAC() {

    uint32_t srcip = udpDetails.srcip;
    uint32_t srcip2 = udpDetails.srcip2;
    uint32_t dstip = udpDetails.dstip;
    uint32_t dstip2 = udpDetails.dstip2;
    uint64_t srcmac = udpDetails.srcmac;
    uint64_t srcmac2 = udpDetails.srcmac2;
    uint64_t dstmac = udpDetails.dstmac;
    uint64_t dstmac2 = udpDetails.dstmac2;
    int srcport = udpDetails.srcport;
    int srcport2 = udpDetails.srcport2;
    int dstport = udpDetails.dstport;
    int dstport2 = udpDetails.dstport2;

    LOG(logINFOBLUE, ("Configuring MAC\n"));
    char src_mac[50], src_ip[INET_ADDRSTRLEN], dst_mac[50],
        dst_ip[INET_ADDRSTRLEN];
    getMacAddressinString(src_mac, 50, srcmac);
    getMacAddressinString(dst_mac, 50, dstmac);
    getIpAddressinString(src_ip, srcip);
    getIpAddressinString(dst_ip, dstip);
    char src_mac2[50], src_ip2[INET_ADDRSTRLEN], dst_mac2[50],
        dst_ip2[INET_ADDRSTRLEN];
    getMacAddressinString(src_mac2, 50, srcmac2);
    getMacAddressinString(dst_mac2, 50, dstmac2);
    getIpAddressinString(src_ip2, srcip2);
    getIpAddressinString(dst_ip2, dstip2);

    int numInterfaces = getNumberofUDPInterfaces();
    int selInterface = getPrimaryInterface();
    LOG(logINFO, ("\t#Interfaces : %d\n", numInterfaces));
    LOG(logINFO, ("\tInterface   : %d %s\n\n", selInterface,
                  (selInterface ? "Inner" : "Outer")));

    LOG(logINFO, ("\tOuter %s\n", (numInterfaces == 2)
                                      ? "(Bottom)"
                                      : (selInterface ? "Not Used" : "Used")));
    LOG(logINFO, ("\tSource IP   : %s\n"
                  "\tSource MAC  : %s\n"
                  "\tSource Port : %d\n"
                  "\tDest IP     : %s\n"
                  "\tDest MAC    : %s\n"
                  "\tDest Port   : %d\n",
                  src_ip, src_mac, srcport, dst_ip, dst_mac, dstport));

    LOG(logINFO, ("\tInner %s\n", (numInterfaces == 2)
                                      ? "(Top)"
                                      : (selInterface ? "Used" : "Not Used")));
    LOG(logINFO, ("\tSource IP2  : %s\n"
                  "\tSource MAC2 : %s\n"
                  "\tSource Port2: %d\n"
                  "\tDest IP2    : %s\n"
                  "\tDest MAC2   : %s\n"
                  "\tDest Port2  : %d\n",
                  src_ip2, src_mac2, srcport2, dst_ip2, dst_mac2, dstport2));

#ifdef VIRTUAL
    if (setUDPDestinationDetails(0, dst_ip, dstport) == FAIL) {
        LOG(logERROR,
            ("could not set udp destination IP and port for interface 1\n"));
        return FAIL;
    }
    if (numInterfaces == 2 &&
        setUDPDestinationDetails(1, dst_ip2, dstport2) == FAIL) {
        LOG(logERROR,
            ("could not set udp destination IP and port for interface 2\n"));
        return FAIL;
    }
    return OK;
#endif
    // default one rxr entry (others not yet implemented in client yet)
    int iRxEntry = 0;

    if (numInterfaces == 2) {
        // bottom
        setupHeader(iRxEntry, OUTER, dstip, dstmac, dstport, srcmac, srcip,
                    srcport);
        // top
        setupHeader(iRxEntry, INNER, dstip2, dstmac2, dstport2, srcmac2, srcip2,
                    srcport2);
    }
    // single interface
    else {
        // default
        if (selInterface == 0) {
            setupHeader(iRxEntry, OUTER, dstip, dstmac, dstport, srcmac, srcip,
                        srcport);
        } else {
            setupHeader(iRxEntry, INNER, dstip, dstmac, dstport, srcmac, srcip,
                        srcport2);
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
    uint32_t innerPos[2] = {pos[X], pos[Y]};
    uint32_t outerPos[2] = {pos[X], pos[Y]};
    int selInterface = getPrimaryInterface();

    if (getNumberofUDPInterfaces() == 1) {
        LOG(logDEBUG,
            ("Setting detector position: 1 Interface %s \n(%d, %d)\n",
             (selInterface ? "Inner" : "Outer"), innerPos[X], innerPos[Y]));
    } else {
        ++outerPos[X];
        LOG(logDEBUG, ("Setting detector position: 2 Interfaces \n"
                       "  inner top(%d, %d), outer bottom(%d, %d)\n",
                       innerPos[X], innerPos[Y], outerPos[X], outerPos[Y]));
    }
    detPos[0] = innerPos[0];
    detPos[1] = innerPos[1];
    detPos[2] = outerPos[0];
    detPos[3] = outerPos[1];

    // row
    // outer
    uint32_t addr = COORD_ROW_REG;
    bus_w(addr,
          (bus_r(addr) & ~COORD_ROW_OUTER_MSK) |
              ((outerPos[X] << COORD_ROW_OUTER_OFST) & COORD_ROW_OUTER_MSK));
    if (((bus_r(addr) & COORD_ROW_OUTER_MSK) >> COORD_ROW_OUTER_OFST) !=
        outerPos[X])
        ret = FAIL;
    // inner
    bus_w(addr,
          (bus_r(addr) & ~COORD_ROW_INNER_MSK) |
              ((innerPos[X] << COORD_ROW_INNER_OFST) & COORD_ROW_INNER_MSK));
    if (((bus_r(addr) & COORD_ROW_INNER_MSK) >> COORD_ROW_INNER_OFST) !=
        innerPos[X])
        ret = FAIL;

    // col
    // outer
    addr = COORD_COL_REG;
    bus_w(addr,
          (bus_r(addr) & ~COORD_COL_OUTER_MSK) |
              ((outerPos[Y] << COORD_COL_OUTER_OFST) & COORD_COL_OUTER_MSK));
    if (((bus_r(addr) & COORD_COL_OUTER_MSK) >> COORD_COL_OUTER_OFST) !=
        outerPos[Y])
        ret = FAIL;
    // inner
    bus_w(addr,
          (bus_r(addr) & ~COORD_COL_INNER_MSK) |
              ((innerPos[Y] << COORD_COL_INNER_OFST) & COORD_COL_INNER_MSK));
    if (((bus_r(addr) & COORD_COL_INNER_MSK) >> COORD_COL_INNER_OFST) !=
        innerPos[Y])
        ret = FAIL;

    if (ret == OK) {
        if (getNumberofUDPInterfaces() == 1) {
            LOG(logINFOBLUE,
                ("Position set to [%d, %d]\n", innerPos[X], innerPos[Y]));
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
            LOG(logINFO, ("Powering chip: on\n"));
            bus_w(CHIP_POWER_REG,
                  bus_r(CHIP_POWER_REG) | CHIP_POWER_ENABLE_MSK);
        } else {
            LOG(logINFO, ("Powering chip: off\n"));
            bus_w(CHIP_POWER_REG,
                  bus_r(CHIP_POWER_REG) & ~CHIP_POWER_ENABLE_MSK);
        }
    }
#ifdef VIRTUAL
    return ((bus_r(CHIP_POWER_REG) & CHIP_POWER_ENABLE_MSK) >>
            CHIP_POWER_ENABLE_OFST);
#endif
    return ((bus_r(CHIP_POWER_REG) & CHIP_POWER_STATUS_MSK) >>
            CHIP_POWER_STATUS_OFST);
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

void configureASICTimer() {
    LOG(logINFO, ("Configuring ASIC Timer\n"));
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_PRCHRG_TMR_MSK) |
                             ASIC_CTRL_PRCHRG_TMR_VAL);
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_DS_TMR_MSK) |
                             ASIC_CTRL_DS_TMR_VAL);
}

int setClockDivider(enum CLKINDEX ind, int val) {
    if (ind != RUN_CLK) {
        LOG(logERROR, ("Unknown clock index %d to set speed\n", ind));
        return FAIL;
    }
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
        if (isHardwareVersion2()) {
            LOG(logERROR, ("Cannot set full speed. Should not be here\n"));
            return FAIL;
        }
        LOG(logINFO, ("Setting Full Speed (40 MHz):\n"));
        adcOfst = ADC_OFST_FULL_SPEED_VAL;
        sampleAdcSpeed = SAMPLE_ADC_FULL_SPEED;
        adcPhase = ADC_PHASE_FULL_SPEED;
        dbitPhase = DBIT_PHASE_FULL_SPEED;
        config = CONFIG_FULL_SPEED_40MHZ_VAL;
        break;

    case HALF_SPEED:
        LOG(logINFO, ("Setting Half Speed (20 MHz):\n"));
        adcOfst = isHardwareVersion2() ? ADC_OFST_HALF_SPEED_BOARD2_VAL
                                       : ADC_OFST_HALF_SPEED_VAL;
        sampleAdcSpeed = isHardwareVersion2() ? SAMPLE_ADC_HALF_SPEED_BOARD2
                                              : SAMPLE_ADC_HALF_SPEED;
        adcPhase = isHardwareVersion2() ? ADC_PHASE_HALF_SPEED_BOARD2
                                        : ADC_PHASE_HALF_SPEED;
        dbitPhase = isHardwareVersion2() ? DBIT_PHASE_HALF_SPEED_BOARD2
                                         : DBIT_PHASE_HALF_SPEED;
        config = CONFIG_HALF_SPEED_20MHZ_VAL;
        break;

    case QUARTER_SPEED:
        LOG(logINFO, ("Setting Half Speed (10 MHz):\n"));
        adcOfst = isHardwareVersion2() ? ADC_OFST_QUARTER_SPEED_BOARD2_VAL
                                       : ADC_OFST_QUARTER_SPEED_VAL;
        sampleAdcSpeed = isHardwareVersion2() ? SAMPLE_ADC_QUARTER_SPEED_BOARD2
                                              : SAMPLE_ADC_QUARTER_SPEED;
        adcPhase = isHardwareVersion2() ? ADC_PHASE_QUARTER_SPEED_BOARD2
                                        : ADC_PHASE_QUARTER_SPEED;
        dbitPhase = isHardwareVersion2() ? DBIT_PHASE_QUARTER_SPEED_BOARD2
                                         : DBIT_PHASE_QUARTER_SPEED;
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

    // only implemented in the new boards now
    if (!isHardwareVersion2()) {
        setPhase(DBIT_CLK, dbitPhase, 0);
        LOG(logINFO, ("\tSet DBIT Phase Reg to %d\n", dbitPhase));
    }

    return OK;
}

int getClockDivider(enum CLKINDEX ind) {
    if (ind != RUN_CLK) {
        LOG(logERROR, ("Unknown clock index %d to get speed\n", ind));
        return -1;
    }
    u_int32_t speed = bus_r(CONFIG_REG) & CONFIG_READOUT_SPEED_MSK;
    switch (speed) {
    case CONFIG_FULL_SPEED_40MHZ_VAL:
        return FULL_SPEED;
    case CONFIG_HALF_SPEED_20MHZ_VAL:
        return HALF_SPEED;
    case CONFIG_QUARTER_SPEED_10MHZ_VAL:
        return QUARTER_SPEED;
    default:
        LOG(logERROR, ("Unknown speed val: %d\n", speed));
        return -1;
    }
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
    }
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

    int numInterfaces = getNumberofUDPInterfaces();
    int64_t periodNs = getPeriod();
    int numFrames = (getNumFrames() * getNumTriggers() *
                     (getNumAdditionalStorageCells() + 1));
    int64_t expUs = getExpTime() / 1000;
    const int npixels = 256 * 256 * 8;
    const int dataSize = 8192;
    const int packetsize = dataSize + sizeof(sls_detector_header);
    const int packetsPerFrame = numInterfaces == 1 ? 128 : 64;
    int transmissionDelayUs = getTransmissionDelayFrame() * 1000;

    // Generate data
    char imageData[DATA_BYTES];
    memset(imageData, 0, DATA_BYTES);
    for (int i = 0; i < npixels; ++i) {
        // avoiding gain also being divided when gappixels enabled in call
        // back
        *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
            virtual_image_test_mode ? 0x0FFE : (uint16_t)i;
    }

    // Send data
    {
        uint64_t frameNr = 0;
        getNextFrameNumber(&frameNr);
        for (int iframes = 0; iframes != numFrames; ++iframes) {

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

            int srcOffset = 0;
            int srcOffset2 = DATA_BYTES / 2;
            // loop packet
            for (int i = 0; i != packetsPerFrame; ++i) {
                // set header
                char packetData[packetsize];
                memset(packetData, 0, packetsize);
                sls_detector_header *header =
                    (sls_detector_header *)(packetData);
                header->detType = (uint16_t)myDetectorType;
                header->version = SLS_DETECTOR_HEADER_VERSION - 1;
                header->frameNumber = frameNr + iframes;
                header->packetNumber = i;
                header->modId = 0;
                header->row = detPos[2];
                header->column = detPos[3];

                // fill data
                memcpy(packetData + sizeof(sls_detector_header),
                       imageData + srcOffset, dataSize);
                srcOffset += dataSize;

                sendUDPPacket(0, packetData, packetsize);

                // second interface
                char packetData2[packetsize];
                memset(packetData2, 0, packetsize);
                if (numInterfaces == 2) {
                    header = (sls_detector_header *)(packetData2);
                    header->detType = (uint16_t)myDetectorType;
                    header->version = SLS_DETECTOR_HEADER_VERSION - 1;
                    header->frameNumber = frameNr + iframes;
                    header->packetNumber = i;
                    header->modId = 0;
                    header->row = detPos[0];
                    header->column = detPos[1];

                    // fill data
                    memcpy(packetData2 + sizeof(sls_detector_header),
                           imageData + srcOffset2, dataSize);
                    srcOffset2 += dataSize;

                    sendUDPPacket(1, packetData2, packetsize);
                }
            }
            LOG(logINFO, ("Sent frame: %d\n", iframes));
            clock_gettime(CLOCK_REALTIME, &end);
            int64_t timeNs = ((end.tv_sec - begin.tv_sec) * 1E9 +
                              (end.tv_nsec - begin.tv_nsec));

            // sleep for (period - exptime)
            if (iframes < numFrames) { // if there is a next frame
                if (periodNs > timeNs) {
                    usleep((periodNs - timeNs) / 1000);
                }
            }
        }
        setNextFrameNumber(frameNr + numFrames);
    }

    closeUDPSocket(0);
    if (numInterfaces == 2) {
        closeUDPSocket(1);
    }

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

    // running
    if (retval & RUN_BUSY_MSK) {
        if (retval & WAITING_FOR_TRIGGER_MSK) {
            LOG(logINFOBLUE, ("Status: WAITING\n"));
            s = WAITING;
        } else {
            LOG(logINFOBLUE, ("Status: RUNNING\n"));
            s = RUNNING;
        }
    }

    // not running
    else {
        // stopped or error
        if (retval & STOPPED_MSK) {
            LOG(logINFOBLUE, ("Status: STOPPED\n"));
            s = STOPPED;
        } else if (retval & RUNMACHINE_BUSY_MSK) {
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
