// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "clogger.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#include "ALTERA_PLL.h" // pll
#include "INA226.h"     // i2c
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
int transceiverDataBytes = 0;
char *analogData = NULL;
char *digitalData = NULL;
char *transceiverData = NULL;
char volatile *analogDataPtr = NULL;
char volatile *digitalDataPtr = NULL;
char volatile *transceiverDataPtr = NULL;
char udpPacketData[UDP_PACKET_DATA_BYTES + sizeof(sls_detector_header)];
uint32_t adcEnableMask_1g = BIT32_MSK;
// 10g readout
uint8_t adcEnableMask_10g = 0xFF;
uint32_t transceiverMask = DEFAULT_TRANSCEIVER_MASK;

int32_t clkPhase[NUM_CLOCKS] = {};
uint32_t clkFrequency[NUM_CLOCKS] = {40, 20, 20, 200};
int dacValues[NDAC] = {};
// software limit that depends on the current chip on the ctb
int vLimit = 0;
int highvoltage = 0;
int analogEnable = 1;
int digitalEnable = 0;
int transceiverEnable = 0;
int naSamples = 1;
int ndSamples = 1;
int ntSamples = 1;
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
    LOG(logINFOBLUE, ("********* Chip Test Board Virtual Server *********\n"));
#else
    LOG(logINFOBLUE, ("************* Chip Test Board Server *************\n"));
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

    if (fwversion >= MIN_REQRD_VRSN_T_RD_API)
        sw_fw_apiversion = getFirmwareAPIVersion();
    LOG(logINFOBLUE,
        ("**************************************************\n"
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
        (((FPGA_VERSION_DTCTR_TYP_CTB_VAL)&FPGA_VERSION_DTCTR_TYP_MSK) >>
         FPGA_VERSION_DTCTR_TYP_OFST);

    if (type != expectedType) {
        LOG(logERROR, ("(Type Fail) - This is not a Chip Test Board firmware "
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

void getServerVersion(char *version) { strcpy(version, APICTB); }

uint64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return REQRD_FRMWR_VRSN;
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

uint16_t getHardwareVersionNumber() {
#ifdef VIRTUAL
    return 0x3f;
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
    LOG(logINFO, ("This Server is for 1 Chip Test Board module\n"));

    // default variables
    dataBytes = 0;
    analogDataBytes = 0;
    digitalDataBytes = 0;
    transceiverDataBytes = 0;
    free(analogData);
    analogData = NULL;
    free(digitalData);
    digitalData = NULL;
    free(transceiverData);
    transceiverData = NULL;
    analogDataPtr = NULL;
    digitalDataPtr = NULL;
    transceiverData = NULL;
    {
        for (int i = 0; i < NUM_CLOCKS; ++i) {
            clkPhase[i] = 0;
        }
        clkFrequency[RUN_CLK] = DEFAULT_RUN_CLK;
        clkFrequency[ADC_CLK] = DEFAULT_ADC_CLK;
        clkFrequency[SYNC_CLK] = DEFAULT_SYNC_CLK;
        clkFrequency[DBIT_CLK] = DEFAULT_DBIT_CLK;
        for (int i = 0; i < NDAC; ++i)
            dacValues[i] = -1;
    }
    vLimit = DEFAULT_VLIMIT;
    highvoltage = 0;
    adcEnableMask_1g = BIT32_MSK;
    adcEnableMask_10g = 0xFF;
    transceiverMask = DEFAULT_TRANSCEIVER_MASK;
    analogEnable = 1;
    digitalEnable = 0;
    transceiverEnable = 0;
    naSamples = 1;
    ndSamples = 1;
    ntSamples = 1;
#ifdef VIRTUAL
    if (isControlServer) {
        sharedMemory_setStatus(IDLE);
        initializePatternWord();
    } else {
        sharedMemory_setStop(0);
    }
#endif
    if (isControlServer) {
        setupUDPCommParameters();
    }

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

    // power off voltage regulators
    powerOff();

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
    // switch off dacs (power regulators most likely only sets to minimum (if
    // power enable on))
    LOG(logINFOBLUE, ("Powering down all dacs\n"));
    for (int idac = 0; idac < NDAC; ++idac) {
        setDAC(idac, LTC2620_GetPowerDownValue(),
               0); // has to be before setvchip
    }

    // power regulators
    // I2C
    INA226_ConfigureI2CCore(I2C_SHUNT_RESISTER_OHMS, I2C_CONTROL_REG,
                            I2C_STATUS_REG, I2C_RX_DATA_FIFO_REG,
                            I2C_RX_DATA_FIFO_LEVEL_REG, I2C_SCL_LOW_COUNT_REG,
                            I2C_SCL_HIGH_COUNT_REG, I2C_SDA_HOLD_REG,
                            I2C_TRANSFER_COMMAND_FIFO_REG);
    INA226_CalibrateCurrentRegister(I2C_POWER_VIO_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VA_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VB_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VC_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VD_DEVICE_ID);
    setVchip(VCHIP_MIN_MV);

    setADCInvertRegister(0); // depends on chip

    LOG(logINFOBLUE, ("Setting Default parameters\n"));
    cleanFifos(); // FIXME: why twice?
    resetCore();

    // 1G UDP
    enableTenGigabitEthernet(0);

    // Initialization of acquistion parameters
    setNumAnalogSamples(DEFAULT_NUM_SAMPLES);
    setNumDigitalSamples(
        DEFAULT_NUM_SAMPLES); // update databytes and allocate ram
    setNumTransceiverSamples(DEFAULT_NUM_SAMPLES);
    setNumFrames(DEFAULT_NUM_FRAMES);
    setExpTime(DEFAULT_EXPTIME);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setPeriod(DEFAULT_PERIOD);
    setDelayAfterTrigger(DEFAULT_DELAY);
    setTiming(DEFAULT_TIMING_MODE);
    setADCEnableMask(BIT32_MSK);
    setADCEnableMask_10G(BIT32_MSK);
    setTransceiverEnableMask(DEFAULT_TRANSCEIVER_MASK);

    if (setReadoutMode(ANALOG_ONLY) == FAIL) {
        strcpy(initErrorMessage,
               "Could not set readout mode to analog only.\n");
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
    }
    setNextFrameNumber(DEFAULT_STARTING_FRAME_NUMBER);
}

int updateDatabytesandAllocateRAM() {

    int oldAnalogDataBytes = analogDataBytes;
    int oldDigitalDataBytes = digitalDataBytes;
    int oldTranceiverDataBytes = transceiverDataBytes;
    updateDataBytes();

    // update only if change in databytes
    if (analogDataBytes == oldAnalogDataBytes &&
        digitalDataBytes == oldDigitalDataBytes &&
        transceiverDataBytes == oldTranceiverDataBytes) {
        LOG(logDEBUG1,
            ("RAM size (Analog:%d, Digital:%d, Transceiver:%d) already "
             "allocated. Nothing to be done.\n",
             analogDataBytes, digitalDataBytes, transceiverDataBytes));
        return OK;
    }
    // Zero databytes
    if (analogDataBytes == 0 && digitalDataBytes == 0 &&
        transceiverDataBytes == 0) {
        LOG(logERROR, ("Can not allocate RAM for 0 bytes.\n"));
        return FAIL;
    }
    // clear RAM
    free(analogData);
    analogData = NULL;
    free(digitalData);
    digitalData = NULL;
    free(transceiverData);
    transceiverData = NULL;
    // allocate RAM
    if (analogDataBytes) {
        analogData = malloc(analogDataBytes);
        if (analogData == NULL) {
            LOG(logERROR, ("Can not allocate analog data RAM for even 1 frame. "
                           "Probable cause: Memory Leak.\n"));
            return FAIL;
        }
        LOG(logINFO, ("\tAnalog RAM allocated to %d bytes\n", analogDataBytes));
    }
    if (digitalDataBytes) {
        digitalData = malloc(digitalDataBytes);
        if (digitalData == NULL) {
            LOG(logERROR,
                ("Can not allocate digital data RAM for even 1 frame. "
                 "Probable cause: Memory Leak.\n"));
            return FAIL;
        }
        LOG(logINFO,
            ("\tDigital RAM allocated to %d bytes\n", digitalDataBytes));
    }
    if (transceiverDataBytes) {
        transceiverData = malloc(transceiverDataBytes);
        if (transceiverData == NULL) {
            LOG(logERROR,
                ("Can not allocate transceiver data RAM for even 1 frame. "
                 "Probable cause: Memory Leak.\n"));
            return FAIL;
        }
        LOG(logINFO, ("\tTransceiver RAM allocated to %d bytes\n",
                      transceiverDataBytes));
    }
    return OK;
}

void updateDataBytes() {
    int nachans = 0, ndchans = 0, ntchans = 0;
    analogDataBytes = 0;
    digitalDataBytes = 0;
    transceiverDataBytes = 0;

    // analog
    if (analogEnable) {
        if (adcEnableMask_1g == BIT32_MSK)
            nachans = 32;
        else {
            for (int ichan = 0; ichan < NCHAN_ANALOG; ++ichan) {
                if (adcEnableMask_1g & (1 << ichan))
                    ++nachans;
            }
        }
        analogDataBytes = nachans * (DYNAMIC_RANGE / 8) * naSamples;
        LOG(logINFO, ("\t#Analog Channels:%d, Databytes:%d\n", nachans,
                      analogDataBytes));
    }
    // digital
    if (digitalEnable) {
        ndchans = NCHAN_DIGITAL;
        digitalDataBytes = (sizeof(uint64_t) * ndSamples);
        LOG(logINFO, ("\t#Digital Channels:%d, Databytes:%d\n", ndchans,
                      digitalDataBytes));
    }
    // transceiver
    if (transceiverEnable) {
        for (int ichan = 0; ichan < NCHAN_TRANSCEIVER; ++ichan) {
            if (transceiverMask & (1 << ichan))
                ++ntchans;
        }
        transceiverDataBytes =
            ntchans * (NBITS_PER_TRANSCEIVER / 8) * ntSamples;
        LOG(logINFO, ("\t#Transceiver Channels:%d, Databytes:%d\n", ntchans,
                      transceiverDataBytes));
    }
    // total
    int nchans = nachans + ndchans + ntchans;
    dataBytes = analogDataBytes + digitalDataBytes + transceiverDataBytes;

    LOG(logINFO,
        ("\t#Total Channels:%d, Total Databytes:%d\n", nchans, dataBytes));
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
    adcEnableMask_10g = actualMask;
    if (analogEnable) {
        uint32_t addr = READOUT_10G_ENABLE_REG;
        bus_w(addr, bus_r(addr) & (~READOUT_10G_ENABLE_ANLG_MSK));
        bus_w(addr, bus_r(addr) |
                        ((adcEnableMask_10g << READOUT_10G_ENABLE_ANLG_OFST) &
                         READOUT_10G_ENABLE_ANLG_MSK));
    }
}

uint32_t getADCEnableMask_10G() {
    if (analogEnable) {
        adcEnableMask_10g =
            ((bus_r(READOUT_10G_ENABLE_REG) & READOUT_10G_ENABLE_ANLG_MSK) >>
             READOUT_10G_ENABLE_ANLG_OFST);
    }

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

int setTransceiverEnableMask(uint32_t mask) {
    LOG(logINFO, ("Setting transceivermask to 0x%08x\n", mask));
    transceiverMask = mask;
    // 1Gb enabled
    if (!enableTenGigabitEthernet(-1)) {
        if (updateDatabytesandAllocateRAM() == FAIL) {
            return FAIL;
        }
    }
    return OK;
}

uint32_t getTransceiverEnableMask() { return transceiverMask; }

void setADCInvertRegister(uint32_t val) {
    LOG(logINFO, ("Setting ADC Port Invert Reg to 0x%x\n", val));
    bus_w(ADC_PORT_INVERT_REG, val);
}

uint32_t getADCInvertRegister() { return bus_r(ADC_PORT_INVERT_REG); }

int setExternalSamplingSource(int val) {
    uint32_t addr = DBIT_EXT_TRG_REG;
    if (val >= 0) {
        LOG(logINFO, ("Setting External sampling source to %d\n", val));
        bus_w(addr, bus_r(addr) & ~DBIT_EXT_TRG_SRC_MSK);
        bus_w(addr, bus_r(addr) | ((val << DBIT_EXT_TRG_SRC_OFST) &
                                   DBIT_EXT_TRG_SRC_MSK));
    }
    return ((bus_r(addr) & DBIT_EXT_TRG_SRC_MSK) >> DBIT_EXT_TRG_SRC_OFST);
}

int setExternalSampling(int val) {
    uint32_t addr = DBIT_EXT_TRG_REG;
    if (val > 0) {
        LOG(logINFO, ("Enabling External sampling\n"));
        bus_w(addr, bus_r(addr) | DBIT_EXT_TRG_OPRTN_MD_MSK);
    } else if (val == 0) {
        LOG(logINFO, ("Disabling External sampling\n"));
        bus_w(addr, bus_r(addr) & ~DBIT_EXT_TRG_OPRTN_MD_MSK);
    }

    return ((bus_r(addr) & DBIT_EXT_TRG_OPRTN_MD_MSK) >>
            DBIT_EXT_TRG_OPRTN_MD_OFST);
}

/* parameters - readout */

int setReadoutMode(enum readoutMode mode) {
    analogEnable = 0;
    digitalEnable = 0;
    transceiverEnable = 0;
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

    uint32_t addr = CONFIG_REG;
    uint32_t addr_readout_10g = READOUT_10G_ENABLE_REG;

    bus_w(addr,
          (bus_r(addr) & (~CONFIG_ENBLE_ANLG_OTPT_MSK) &
           (~CONFIG_ENBLE_DGTL_OTPT_MSK) & (~CONFIG_ENBLE_TRNSCVR_OTPT_MSK)));
    bus_w(addr_readout_10g, bus_r(addr_readout_10g) &
                                (~READOUT_10G_ENABLE_ANLG_MSK) &
                                ~(READOUT_10G_ENABLE_DGTL_MSK) &
                                ~(READOUT_10G_ENABLE_TRNSCVR_MSK));
    if (analogEnable) {
        bus_w(addr, bus_r(addr) | (CONFIG_ENBLE_ANLG_OTPT_MSK));
        bus_w(addr_readout_10g,
              bus_r(addr_readout_10g) |
                  ((adcEnableMask_10g << READOUT_10G_ENABLE_ANLG_OFST) &
                   READOUT_10G_ENABLE_ANLG_MSK));
    }
    if (digitalEnable) {
        bus_w(addr, bus_r(addr) | CONFIG_ENBLE_DGTL_OTPT_MSK);
        bus_w(addr_readout_10g,
              bus_r(addr_readout_10g) | READOUT_10G_ENABLE_DGTL_MSK);
    }
    if (transceiverEnable) {
        bus_w(addr, bus_r(addr) | CONFIG_ENBLE_TRNSCVR_OTPT_MSK);
        bus_w(addr_readout_10g,
              bus_r(addr_readout_10g) |
                  ((transceiverMask << READOUT_10G_ENABLE_TRNSCVR_OFST) &
                   READOUT_10G_ENABLE_TRNSCVR_MSK));
    }

    if (isControlServer) {
        // 1Gb
        if (!enableTenGigabitEthernet(-1)) {
            if (updateDatabytesandAllocateRAM() == FAIL) {
                return FAIL;
            }
        }

        // 10Gb
        else {
            // validate adcenablemask for 10g
            if (analogEnable &&
                adcEnableMask_10g != ((bus_r(READOUT_10G_ENABLE_REG) &
                                       READOUT_10G_ENABLE_ANLG_MSK) >>
                                      READOUT_10G_ENABLE_ANLG_OFST)) {
                LOG(logERROR,
                    ("Setting readout mode failed. Could not set 10g adc "
                     "enable mask to 0x%x\n.",
                     adcEnableMask_10g));
                return FAIL;
            }
            // validate transceivermask for 10g
            if (transceiverEnable &&
                transceiverMask != ((bus_r(READOUT_10G_ENABLE_REG) &
                                     READOUT_10G_ENABLE_TRNSCVR_MSK) >>
                                    READOUT_10G_ENABLE_TRNSCVR_OFST)) {
                LOG(logERROR, ("Setting readout mode failed. Could not set 10g "
                               "transceiver enable mask to 0x%x\n.",
                               transceiverMask));
                return FAIL;
            }
        }
    }
    return OK;
}

int getReadoutMode() {
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
    naSamples = val;
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

int getNumAnalogSamples() { return naSamples; }

int setNumDigitalSamples(int val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid digital samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of digital samples %d\n", val));
    ndSamples = val;
    bus_w(SAMPLES_REG, bus_r(SAMPLES_REG) & ~SAMPLES_DIGITAL_MSK);
    bus_w(SAMPLES_REG, bus_r(SAMPLES_REG) | ((val << SAMPLES_DIGITAL_OFST) &
                                             SAMPLES_DIGITAL_MSK));
    // 1Gb
    if (!enableTenGigabitEthernet(-1)) {
        if (updateDatabytesandAllocateRAM() == FAIL) {
            return FAIL;
        }
    }
    return OK;
}

int getNumDigitalSamples() { return ndSamples; }

int setNumTransceiverSamples(int val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid transceiver samples: %d\n", val));
        return FAIL;
    }
    LOG(logINFO, ("Setting number of transceiver samples %d\n", val));
    ntSamples = val;
    uint32_t addr = SAMPLES_TRANSCEIVER_REG;
    bus_w(addr, bus_r(addr) & ~SAMPLES_TRANSCEIVER_MSK);
    bus_w(addr, bus_r(addr) | ((val << SAMPLES_TRANSCEIVER_OFST) &
                               SAMPLES_TRANSCEIVER_MSK));
    // 1Gb
    if (!enableTenGigabitEthernet(-1)) {
        if (updateDatabytesandAllocateRAM() == FAIL) {
            return FAIL;
        }
    }
    return OK;
}

int getNumTransceiverSamples() { return ntSamples; }

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
enum detectorSettings getSettings() { return UNDEFINED; }

/* parameters - dac, adc, hv */

void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0 && val != LTC2620_GetPowerDownValue())
        return;

    LOG(logDEBUG1, ("Setting dac[%d]: %d %s \n", (int)ind, val,
                    (mV ? "mV" : "dac units")));
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
    if (vLimit > 0 && dac != -1 && dac != LTC2620_GetPowerDownValue()) {
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

int isVchipValid(int val) {
    if (val < VCHIP_MIN_MV || val > VCHIP_MAX_MV) {
        return 0;
    }
    return 1;
}

int getVchip() {
    // not set yet
    if (dacValues[D_PWR_CHIP] == -1 ||
        dacValues[D_PWR_CHIP] == LTC2620_GetPowerDownValue())
        return dacValues[D_PWR_CHIP];
    int voltage = -1;
    // dac to voltage
    ConvertToDifferentRange(LTC2620_GetMaxInput(), LTC2620_GetMinInput(),
                            VCHIP_MIN_MV, VCHIP_MAX_MV, dacValues[D_PWR_CHIP],
                            &voltage);
    return voltage;
}

void setVchip(int val) {
    // set vchip
    if (val != -1) {
        LOG(logINFOBLUE, ("Setting Vchip to %d mV\n", val));

        int dacval = LTC2620_GetPowerDownValue();

        // validate & convert it to dac
        if (val != LTC2620_GetPowerDownValue()) {
            // convert voltage to dac
            if (ConvertToDifferentRange(
                    VCHIP_MIN_MV, VCHIP_MAX_MV, LTC2620_GetMaxInput(),
                    LTC2620_GetMinInput(), // min val is max V
                    val, &dacval) == FAIL) {
                LOG(logERROR,
                    ("\tVChip %d mV invalid. Is not between %d and %d mV\n",
                     val, VCHIP_MIN_MV, VCHIP_MAX_MV));
                return;
            }
        }
        LOG(logINFO, ("Setting Vchip (DAC %d): %d dac (%d mV)\n", D_PWR_CHIP,
                      dacval, val));
        // set
        setDAC(D_PWR_CHIP, dacval, 0);
    }
}

int getVChipToSet(enum DACINDEX ind, int val) {
    LOG(logDEBUG1, ("Calculating vchip to set\n"));
    // validate index & get adc index
    int adcIndex = getADCIndexFromDACIndex(ind);
    if (adcIndex == -1) {
        return -1;
    }

    // get maximum value of the adc values (minimum is 0)
    int max = 0;

    // loop through the adcs
    for (int ipwr = 0; ipwr < NPWR - 1; ++ipwr) {

        // get the dac values for each adc
        int dacmV = getPower(getDACIndexFromADCIndex(ipwr));

        // if current index, replace with value to be set
        if (ipwr == adcIndex) {
            dacmV = val;
        }

        // update max
        max = (dacmV > max) ? dacmV : max;
    }

    // increment to get vchip value
    max += VCHIP_POWER_INCRMNT;

    // validate with vchip minimum value
    if (max < VCHIP_MIN_MV)
        max = VCHIP_MIN_MV;
    // with correct calulations, vchip val should never be greater than vchipmax
    if (max > VCHIP_MAX_MV) {
        LOG(logERROR,
            ("Vchip value to set %d is beyond its maximum (WEIRD)\n", max));
        return -1;
    }
    return max;
}

int getDACIndexFromADCIndex(enum ADCINDEX ind) {
    switch (ind) {
    case V_PWR_IO:
        return D_PWR_IO;
    case V_PWR_A:
        return D_PWR_A;
    case V_PWR_B:
        return D_PWR_B;
    case V_PWR_C:
        return D_PWR_C;
    case V_PWR_D:
        return D_PWR_D;
    default:
        LOG(logERROR, ("ADC index %d is not defined to get DAC index\n", ind));
        return -1;
    }
}

int getADCIndexFromDACIndex(enum DACINDEX ind) {
    switch (ind) {
    case D_PWR_IO:
        return V_PWR_IO;
    case D_PWR_A:
        return V_PWR_A;
    case D_PWR_B:
        return V_PWR_B;
    case D_PWR_C:
        return V_PWR_C;
    case D_PWR_D:
        return V_PWR_D;
    default:
        LOG(logERROR, ("DAC index %d is not defined to get ADC index\n", ind));
        return -1;
    }
}

int isPowerValid(enum DACINDEX ind, int val) {
    int min = (ind == D_PWR_IO) ? VIO_MIN_MV : POWER_RGLTR_MIN;

    // not power_rgltr_max because it is allowed only upto vchip max - 200
    if (val != 0 && (val != LTC2620_GetPowerDownValue()) &&
        (val < min || val > (VCHIP_MAX_MV - VCHIP_POWER_INCRMNT))) {
        return 0;
    }
    return 1;
}

int getPower(enum DACINDEX ind) {
    // validate index & get adc index
    int adcIndex = getADCIndexFromDACIndex(ind);
    if (adcIndex == -1) {
        return -1;
    }

    // powered enable off
    {
        uint32_t addr = POWER_REG;
        uint32_t offset = POWER_ENBL_VLTG_RGLTR_OFST + adcIndex;
        uint32_t mask = (1 << offset);
        if (!(bus_r(addr) & mask))
            return 0;
    }

    // not set yet
    if (dacValues[ind] == -1) {
        LOG(logERROR,
            ("Power enabled, but unknown dac value for power index %d!", ind));
        return -1;
    }

    // dac powered off
    if (dacValues[ind] == LTC2620_GetPowerDownValue()) {
        LOG(logWARNING,
            ("Power %d enabled, dac value %d, voltage at minimum or 0\n", ind,
             LTC2620_GetPowerDownValue()));
        return LTC2620_GetPowerDownValue();
    }

    // vchip not set, weird error, should not happen (as vchip set to max in the
    // beginning) unless user set vchip to LTC2620_GetPowerDownValue()  and then
    // tried to get a power regulator value
    if (dacValues[D_PWR_CHIP] == -1 ||
        dacValues[D_PWR_CHIP] == LTC2620_GetPowerDownValue()) {
        LOG(logERROR, ("Cannot read power regulator %d (vchip not set)."
                       "Set a power regulator, which will also set vchip.\n"));
        return -1;
    }

    // convert dac to voltage
    int retval = -1;
    ConvertToDifferentRange(LTC2620_GetMaxInput(), LTC2620_GetMinInput(),
                            POWER_RGLTR_MIN, POWER_RGLTR_MAX, dacValues[ind],
                            &retval);
    return retval;
}

void setPower(enum DACINDEX ind, int val) {
    // validate index & get adc index
    int adcIndex = getADCIndexFromDACIndex(ind);
    if (adcIndex == -1) {
        return;
    }

    uint32_t addr = POWER_REG;
    uint32_t offset = POWER_ENBL_VLTG_RGLTR_OFST + adcIndex;
    uint32_t mask = (1 << offset);

    // set power
    if (val != -1) {
        LOG(logINFO, ("Setting Power to %d mV\n", val));

        // validate value (already checked at tcp)
        if (!isPowerValid(ind, val)) {
            LOG(logERROR,
                ("Invalid value of %d mV for Power %d. Is not between %d and "
                 "%d mV\n",
                 val, ind, (ind == D_PWR_IO ? VIO_MIN_MV : POWER_RGLTR_MIN),
                 POWER_RGLTR_MAX));
            return;
        }

        // get vchip to set vchip (calculated now before switching off power
        // enable)
        int vchip = getVChipToSet(ind, val);
        LOG(logDEBUG1, ("Vchip to set: %d\n", vchip));
        // index problem of vchip calculation problem
        if (vchip == -1)
            return;

        // Switch off power enable
        LOG(logDEBUG1, ("Switching off power enable\n"));
        bus_w(addr, bus_r(addr) & ~(mask));

        // power down dac
        LOG(logDEBUG1, ("Powering off P%d (DAC %d)\n", adcIndex, ind));
        setDAC(ind, LTC2620_GetPowerDownValue(), 0);

        // set vchip
        setVchip(vchip);
        if (getVchip() != vchip) {
            LOG(logERROR, ("Weird, Could not set vchip. Set %d, read %d\n.",
                           vchip, getVchip()));
            return;
        }

        //(power off is anyway done with power enable)
        if (val == 0)
            val = LTC2620_GetPowerDownValue();

        // convert it to dac (power off is anyway done with power enable)
        if (val != LTC2620_GetPowerDownValue()) {
            LOG(logDEBUG1, ("Convert Power of %d mV to dac units\n", val));

            int dacval = -1;
            // convert voltage to dac
            if (ConvertToDifferentRange(
                    POWER_RGLTR_MIN, POWER_RGLTR_MAX, LTC2620_GetMaxInput(),
                    LTC2620_GetMinInput(), val, &dacval) == FAIL) {
                LOG(logERROR,
                    ("\tPower index %d of value %d mV invalid. Is not between "
                     "%d and %d mV\n",
                     ind, val, POWER_RGLTR_MIN, vchip - VCHIP_POWER_INCRMNT));
                return;
            }

            // set and power on/ update dac
            LOG(logINFO, ("Setting P%d (DAC %d): %d dac (%d mV)\n", adcIndex,
                          ind, dacval, val));
            setDAC(ind, dacval, 0);

            // to be sure of valid conversion
            if (dacval >= 0) {
                LOG(logDEBUG1, ("Switching on power enable\n"));
                bus_w(addr, bus_r(addr) | mask);
            }
        }
    }
}

void powerOff() {
    uint32_t addr = POWER_REG;
    LOG(logINFO, ("Powering off all voltage regulators\n"));
    bus_w(addr, bus_r(addr) & (~POWER_ENBL_VLTG_RGLTR_MSK));
    LOG(logDEBUG1, ("Power Register: 0x%08x\n", bus_r(addr)));
}

int getADC(enum ADCINDEX ind) {
#ifdef VIRTUAL
    return 0;
#endif
    switch (ind) {
    case V_PWR_IO:
    case V_PWR_A:
    case V_PWR_B:
    case V_PWR_C:
    case V_PWR_D:
        LOG(logDEBUG1, ("Reading I2C Voltage for device Id: %d\n", (int)ind));
        return INA226_ReadVoltage(I2C_POWER_VIO_DEVICE_ID + (int)ind);
    case I_PWR_IO:
    case I_PWR_A:
    case I_PWR_B:
    case I_PWR_C:
    case I_PWR_D:
        LOG(logDEBUG1, ("Reading I2C Current for device Id: %d\n", (int)ind));
        return INA226_ReadCurrent(I2C_POWER_VIO_DEVICE_ID +
                                  (int)(ind - I_PWR_IO));

        // slow adcs
    case S_TMP:
        LOG(logDEBUG1, ("Reading Slow ADC Temperature\n"));
        return getSlowADCTemperature();
    case S_ADC0:
    case S_ADC1:
    case S_ADC2:
    case S_ADC3:
    case S_ADC4:
    case S_ADC5:
    case S_ADC6:
    case S_ADC7:
        LOG(logDEBUG1, ("Reading Slow ADC Channel %d\n", (int)ind - S_ADC0));
        return getSlowADC((int)ind - S_ADC0);
    default:
        LOG(logERROR, ("Adc Index %d not defined \n", (int)ind));
        return -1;
    }
}

int getSlowADC(int ichan) {
    LOG(logDEBUG1, ("Getting slow adc channel %d\n", ichan));

    // configure for channel
    bus_w(ADC_SLOW_CFG_REG,
          // don't read back config reg
          ADC_SLOW_CFG_RB_MSK |
              // disable sequencer (different from config)
              ADC_SLOW_CFG_SEQ_DSBLE_VAL |
              // Internal reference. REF = 2.5V buffered output. Temperature
              // sensor enabled.
              ADC_SLOW_CFG_REF_INT_2500MV_VAL |
              // full bandwidth of low pass filter
              ADC_SLOW_CFG_BW_FULL_VAL |
              // specific channel (different from config)
              ((ichan << ADC_SLOW_CFG_IN_OFST) & ADC_SLOW_CFG_IN_MSK) |
              // input channel configuration (unipolar. inx to gnd)
              ADC_SLOW_CFG_INCC_UNPLR_IN_GND_VAL |
              // overwrite configuration
              ADC_SLOW_CFG_CFG_OVRWRTE_VAL);

    // start converting
    bus_w(ADC_SLOW_CTRL_REG, bus_r(ADC_SLOW_CTRL_REG) | ADC_SLOW_CTRL_STRT_MSK);
    bus_w(ADC_SLOW_CTRL_REG,
          bus_r(ADC_SLOW_CTRL_REG) & ~ADC_SLOW_CTRL_STRT_MSK);

    // wait for it to be done
    volatile int done = ((bus_r(ADC_SLOW_CTRL_REG) & ADC_SLOW_CTRL_DONE_MSK) >>
                         ADC_SLOW_CTRL_DONE_OFST);
    while (!done) {
        done = ((bus_r(ADC_SLOW_CTRL_REG) & ADC_SLOW_CTRL_DONE_MSK) >>
                ADC_SLOW_CTRL_DONE_OFST);
    }

    // readout
    int regval = bus_r(ADC_SLOW_DATA_REG);

    // value in uV
    int refMaxuv = 2500 * 1000;
    int regMinuv = 0;
    int maxSteps = 0xFFFF + 1;
    int retval = 0;
    if (ConvertToDifferentRange(0, maxSteps, regMinuv, refMaxuv, regval,
                                &retval) == FAIL) {
        LOG(logERROR,
            ("Could not convert slow adc channel (regval:0x%x) to uv\n",
             regval));
        return -1;
    }

    LOG(logINFO,
        ("\tRead slow adc [%d]: %d uV (reg: 0x%x)\n", ichan, retval, regval));

    return retval;
}

int getSlowADCTemperature() {
    LOG(logDEBUG1, ("Getting slow adc temperature\n"));

    // configure for channel
    bus_w(ADC_SLOW_CFG_REG,
          // don't read back config reg
          ADC_SLOW_CFG_RB_MSK |
              // disable sequencer (different from config)
              ADC_SLOW_CFG_SEQ_DSBLE_VAL |
              // Internal reference. REF = 2.5V buffered output. Temperature
              // sensor enabled.
              ADC_SLOW_CFG_REF_INT_2500MV_VAL |
              // full bandwidth of low pass filter
              ADC_SLOW_CFG_BW_FULL_VAL |
              // all channels
              ADC_SLOW_CFG_IN_MSK |
              // temp sensor
              ADC_SLOW_CFG_INCC_TMP_VAL |
              // overwrite configuration
              ADC_SLOW_CFG_CFG_OVRWRTE_VAL);

    // start converting
    bus_w(ADC_SLOW_CTRL_REG, bus_r(ADC_SLOW_CTRL_REG) | ADC_SLOW_CTRL_STRT_MSK);
    bus_w(ADC_SLOW_CTRL_REG,
          bus_r(ADC_SLOW_CTRL_REG) & ~ADC_SLOW_CTRL_STRT_MSK);

    // wait for it to be done
    volatile int done = ((bus_r(ADC_SLOW_CTRL_REG) & ADC_SLOW_CTRL_DONE_MSK) >>
                         ADC_SLOW_CTRL_DONE_OFST);
    while (!done) {
        done = ((bus_r(ADC_SLOW_CTRL_REG) & ADC_SLOW_CTRL_DONE_MSK) >>
                ADC_SLOW_CTRL_DONE_OFST);
    }

    // readout
    int regval = bus_r(ADC_SLOW_DATA_REG);

    // value in mV FIXME: page 17? reference voltage temperature coefficient or
    // t do with -40 to 85 °C
    int retval = 0;
    int maxSteps = 0xFFFF + 1;
    int minmv = 0;
    int maxmv = 2500;
    if (ConvertToDifferentRange(0, maxSteps, minmv, maxmv, regval, &retval) ==
        FAIL) {
        LOG(logERROR,
            ("Could not convert slow adc temp (regval:0x%x) to uv\n", regval));
        return -1;
    }
    LOG(logDEBUG1, ("voltage read for temp: %d mV\n", retval));

    // value in °C
    double tempCFor1mv = (25.00 / 283.00);
    double tempValue = tempCFor1mv * (double)retval;
    LOG(logINFO, ("\tTemp slow adc : %f °C (reg: %d)\n", tempValue, regval));

    return tempValue;
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
    }
    return ((bus_r(addr) & CONFIG_GB10_SND_UDP_MSK) >>
            CONFIG_GB10_SND_UDP_OFST);
}

/* ctb specific - configure frequency, phase, pll */

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
    LOG(logINFO, ("\tSetting %s clock (%d) frequency to %d MHz\n",
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

void setDBITPipeline(int val) {
    if (val < 0) {
        return;
    }
    LOG(logINFO, ("Setting dbit pipeline to %d\n", val));
    uint32_t addr = ADC_OFFSET_REG;
    bus_w(addr, bus_r(addr) & ~ADC_OFFSET_DBT_PPLN_MSK);
    bus_w(addr, bus_r(addr) | ((val << ADC_OFFSET_DBT_PPLN_OFST) &
                               ADC_OFFSET_DBT_PPLN_MSK));
}

int getDBITPipeline() {
    return ((bus_r(ADC_OFFSET_REG) & ADC_OFFSET_DBT_PPLN_MSK) >>
            ADC_OFFSET_DBT_PPLN_OFST);
}

int setLEDEnable(int enable) {
    uint32_t addr = CONFIG_REG;

    // set
    if (enable >= 0) {
        LOG(logINFO, ("Switching LED %s\n", (enable > 0) ? "ON" : "OFF"));
        // disable
        if (enable == 0) {
            bus_w(addr, bus_r(addr) | CONFIG_LED_DSBL_MSK);
        }
        // enable
        else {
            bus_w(addr, bus_r(addr) & (~CONFIG_LED_DSBL_MSK));
        }
    }
    // ~ to get the opposite
    return (((~bus_r(addr)) & CONFIG_LED_DSBL_MSK) >> CONFIG_LED_DSBL_OFST);
}

void setDigitalIODelay(uint64_t pinMask, int delay) {
    LOG(logINFO, ("Setings Digital IO Delay (pinMask:0x%llx, delay: %d ps)\n",
                  (long long unsigned int)pinMask, delay));

    int delayunit = delay / OUTPUT_DELAY_0_OTPT_STTNG_STEPS;
    LOG(logDEBUG1, ("delay unit: 0x%x (steps of 25ps)\n", delayunit));

    // set pin mask
    bus_w(PIN_DELAY_1_REG, pinMask);

    uint32_t addr = OUTPUT_DELAY_0_REG;
    // set delay
    bus_w(addr, bus_r(addr) & (~OUTPUT_DELAY_0_OTPT_STTNG_MSK));
    bus_w(addr, (bus_r(addr) | ((delayunit << OUTPUT_DELAY_0_OTPT_STTNG_OFST) &
                                OUTPUT_DELAY_0_OTPT_STTNG_MSK)));

    // load value
    bus_w(addr, bus_r(addr) | OUTPUT_DELAY_0_OTPT_TRGGR_MSK);

    // trigger configuration
    bus_w(addr, bus_r(addr) & (~OUTPUT_DELAY_0_OTPT_TRGGR_MSK));
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

int startReadOut() {
    LOG(logINFOBLUE, ("Starting Readout\n"));
#ifdef VIRTUAL
    return startStateMachine();
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

    cleanFifos();
    usleep(1);
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
    // if scan active, stop scan first
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

        // 1g might still be transmitting or reading from fifo (not virtual)
        if (!enableTenGigabitEthernet(-1) && checkDataInFifo()) {
            LOG(logINFOBLUE, ("Status: Transmitting (Data in Fifo)\n"));
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

int validateUDPSocket() {
    if (getUdPSocketDescriptor(0, 0) <= 0) {
        return FAIL;
    }
    return OK;
}

void readandSendUDPFrames() {
    LOG(logDEBUG1, ("Reading from 1G UDP\n"));

    // read every frame
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
    LOG(logINFOBLUE, ("Transmitting frames done\n"));
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

void unsetFifoReadStrobes() {
    bus_w(DUMMY_REG, bus_r(DUMMY_REG) & (~DUMMY_ANLG_FIFO_RD_STRBE_MSK) &
                         (~DUMMY_DGTL_FIFO_RD_STRBE_MSK) &
                         (~DUMMY_TRNSCVR_FIFO_RD_STRBE_MSK));
}

int readSample(int ns) {
    int sampleRead = 0;
    uint32_t addr = DUMMY_REG;
    LOG(logDEBUG1, ("sample :%d\n", ns));

    // read adcs
    if (analogEnable && ns < naSamples) {

        uint32_t fifoAddr = FIFO_DATA_REG;

        if (!(ns % 1000)) {
            LOG(logDEBUG1, ("Reading sample ns:%d of %d AEmtpy:0x%x AFull:0x%x "
                            "Status:0x%x\n",
                            ns, naSamples, bus_r(FIFO_EMPTY_REG),
                            bus_r(FIFO_FULL_REG), bus_r(STATUS_REG)));
        }

        // loop through all channels
        for (int ich = 0; ich < NCHAN_ANALOG; ++ich) {

            // if channel is in enable mask
            if ((1 << ich) & (adcEnableMask_1g)) {

                // unselect channel
                bus_w(addr, bus_r(addr) & ~(DUMMY_FIFO_CHNNL_SLCT_MSK));

                // select channel
                bus_w(addr, bus_r(addr) | ((ich << DUMMY_FIFO_CHNNL_SLCT_OFST) &
                                           DUMMY_FIFO_CHNNL_SLCT_MSK));

                // wait for 1 us to latch different clocks of read and read
                // strobe
                for (int i = 0; i < WAIT_TIME_1US_FOR_LOOP_CNT; ++i)
                    ;

                // read fifo and write it to current position of data pointer
                *((uint16_t *)analogDataPtr) = bus_r16(fifoAddr);

                // keep reading till the value is the same
                /* while (*((uint16_t*)analogDataPtr) != bus_r16(fifoAddr)) {
                     LOG(logDEBUG1, ("%d ", ich));
                     *((uint16_t*)analogDataPtr) = bus_r16(fifoAddr);
                 }*/

                // increment pointer to data out destination
                analogDataPtr += 2;
                sampleRead = 1;
            }
        }

        // read strobe to all analog fifos
        bus_w(addr, bus_r(addr) | DUMMY_ANLG_FIFO_RD_STRBE_MSK);
        bus_w(addr, bus_r(addr) & (~DUMMY_ANLG_FIFO_RD_STRBE_MSK));

        // wait for 1 us to latch different clocks of read and read strobe
        for (int i = 0; i < WAIT_TIME_1US_FOR_LOOP_CNT; ++i)
            ;
    }

    // read digital output
    if (digitalEnable && ns < ndSamples) {

        if (!(ns % 1000)) {
            LOG(logDEBUG1,
                ("Reading sample ns:%d of %d DEmtpy:%d DFull:%d Status:0x%x\n",
                 ns, ndSamples,
                 ((bus_r(FIFO_DIN_STATUS_REG) &
                   FIFO_DIN_STATUS_FIFO_EMPTY_MSK) >>
                  FIFO_DIN_STATUS_FIFO_EMPTY_OFST),
                 ((bus_r(FIFO_DIN_STATUS_REG) &
                   FIFO_DIN_STATUS_FIFO_FULL_MSK) >>
                  FIFO_DIN_STATUS_FIFO_FULL_OFST),
                 bus_r(STATUS_REG)));
        }

        // read fifo and write it to current position of data pointer
        *((uint64_t *)digitalDataPtr) =
            get64BitReg(FIFO_DIN_LSB_REG, FIFO_DIN_MSB_REG);
        digitalDataPtr += 8;
        sampleRead = 1;

        // read strobe to digital fifo
        bus_w(addr, bus_r(addr) | DUMMY_DGTL_FIFO_RD_STRBE_MSK);
        bus_w(addr, bus_r(addr) & (~DUMMY_DGTL_FIFO_RD_STRBE_MSK));

        // wait for 1 us to latch different clocks of read and read strobe
        for (int i = 0; i < WAIT_TIME_1US_FOR_LOOP_CNT; ++i)
            ;
    }

    // read transceivers
    if (transceiverEnable && ns < ntSamples) {
        uint32_t tStatusAddr = FIFO_TIN_STATUS_REG;

        if (!(ns % 1000)) {
            LOG(logDEBUG1,
                ("Reading sample ns:%d of %d TReg:0x%x Status:0x%x\n", ns,
                 ntSamples, bus_r(tStatusAddr), bus_r(STATUS_REG)));
        }

        // loop through all channels
        for (int ich = 0; ich < NCHAN_TRANSCEIVER; ++ich) {

            // if channel is in enable mask
            if ((1 << ich) & (transceiverMask)) {

                // int offset = FIFO_TIN_STATUS_FIFO_EMPTY_1_OFST + ich;
                // uint32_t mask = (1 << offset);
                // int empty = ((bus_r(tStatusAddr) & mask) >> offset);

                // if fifo not empty
                // if (!empty) {
                LOG(logDEBUG1,
                    ("ns:%d Transceiver Fifo %d NOT Empty\n", ns, ich));

                // unselect channel
                bus_w(addr, bus_r(addr) & ~(DUMMY_TRNSCVR_FIFO_CHNNL_SLCT_MSK));

                // select channel
                bus_w(addr, bus_r(addr) |
                                ((ich << DUMMY_TRNSCVR_FIFO_CHNNL_SLCT_OFST) &
                                 DUMMY_TRNSCVR_FIFO_CHNNL_SLCT_MSK));

                // wait for 1 us to latch different clocks of read and read
                // strobe
                for (int i = 0; i < WAIT_TIME_1US_FOR_LOOP_CNT; ++i)
                    ;

                // read fifo and write it to current position of data
                // pointer
                *((uint64_t *)transceiverDataPtr) =
                    get64BitReg(FIFO_TIN_LSB_REG, FIFO_TIN_MSB_REG);
                transceiverDataPtr += 8;
                sampleRead = 1;
                //}
            }
        }

        // read strobe
        bus_w(addr, bus_r(addr) | DUMMY_TRNSCVR_FIFO_RD_STRBE_MSK);
        usleep(5 * 1000);
        bus_w(addr, bus_r(addr) & (~DUMMY_TRNSCVR_FIFO_RD_STRBE_MSK));

        // wait for 1 us to latch different clocks of read and read
        // strobe
        for (int i = 0; i < WAIT_TIME_1US_FOR_LOOP_CNT; ++i)
            ;
    }

    LOG(logDEBUG1, ("sample read:%d\n", sampleRead));
    return sampleRead;
}

uint32_t checkDataInFifo() {
    uint32_t dataPresent = 0;
    if (analogEnable) {
        uint32_t fifoEmtpy = bus_r(FIFO_EMPTY_REG);
        LOG(logDEBUG1,
            ("Analog Fifo Empty (32 channels): 0x%08x\n", fifoEmtpy));
        dataPresent = (~fifoEmtpy);
    }
    if (!dataPresent && digitalEnable) {
        int fifoEmtpy =
            ((bus_r(FIFO_DIN_STATUS_REG) & FIFO_DIN_STATUS_FIFO_EMPTY_MSK) >>
             FIFO_DIN_STATUS_FIFO_EMPTY_OFST);
        LOG(logDEBUG1, ("Digital Fifo Empty: %d\n", fifoEmtpy));
        dataPresent = (fifoEmtpy ? 0 : 1);
    }
    if (!dataPresent && transceiverEnable) {
        int fifoEmtpy = 1;
        for (int ich = 0; ich != 4; ++ich) {
            if ((1 << ich) & (transceiverMask)) {
                uint32_t mask = FIFO_TIN_STATUS_FIFO_EMPTY_1_MSK << ich;
                int offset = FIFO_TIN_STATUS_FIFO_EMPTY_1_OFST + ich;
                int iFifoEmpty =
                    ((bus_r(FIFO_TIN_STATUS_REG) & mask) >> offset);
                if (iFifoEmpty == 0) {
                    fifoEmtpy = 0;
                }
            }
        }
        LOG(logDEBUG1, ("Transceiver Fifo Empty: %d reg:0x%x\n", fifoEmtpy,
                        bus_r(FIFO_TIN_STATUS_REG)));
        dataPresent = (fifoEmtpy ? 0 : 1);
    }
    LOG(logDEBUG1, ("Data in Fifo :0x%x\n", dataPresent));
    return dataPresent;
}

// only called for starting of a new frame
int checkFifoForEndOfAcquisition() {
    LOG(logDEBUG1, ("Check for end of acq\n"));
    uint32_t dataPresent = checkDataInFifo();
    LOG(logDEBUG1, ("dataPresent:%d\n", dataPresent));

    // as long as no data
    while (!dataPresent) {
        // acquisition done
        if (!runBusy()) {
            LOG(logDEBUG1, ("Not running\n"));
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
    memset(analogData, 0, analogDataBytes);
    digitalDataPtr = digitalData;
    memset(digitalData, 0, digitalDataBytes);
    transceiverDataPtr = transceiverData;
    memset(transceiverData, 0, transceiverDataBytes);

    // no data for this frame
    /*if (!checkDataInFifo()) {
        return FAIL;
    }*/
    // for startacquistiion
    if (checkFifoForEndOfAcquisition() == FAIL) {
        return FAIL;
    }

    // read Sample
    int maxSamples = 0;
    if (analogEnable && naSamples > maxSamples)
        maxSamples = naSamples;
    if (digitalEnable && ndSamples > maxSamples)
        maxSamples = ndSamples;
    if (transceiverEnable && ntSamples > maxSamples)
        maxSamples = ntSamples;
    while (ns < maxSamples) {
        // chceck if no data in fifo, return ns?//FIXME: ask Anna
        if (!readSample(ns)) {
            LOG(logWARNING, ("No more samples to read\n"));
            return OK;
        }
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
    int nachans = 0, ndchans = 0, ntchans = 0;
    if (analogEnable) {
        uint32_t mask =
            enableTenGigabitEthernet(-1) ? adcEnableMask_10g : adcEnableMask_1g;
        if (mask == BIT32_MASK) {
            nachans = NCHAN_ANALOG;
        } else {
            for (int ich = 0; ich < NCHAN_ANALOG; ++ich) {
                if ((mask & (1 << ich)) != 0U)
                    ++nachans;
            }
        }
        LOG(logDEBUG1, ("Analog Channels: %d\n", nachans));
    }

    if (digitalEnable) {
        ndchans = 64;
        LOG(logDEBUG, ("Digital Channels: %d\n", ndchans));
    }
    if (transceiverEnable) {
        for (int ich = 0; ich < NCHAN_TRANSCEIVER; ++ich) {
            if ((transceiverMask & (1 << ich)) != 0U)
                ++ntchans;
        }

        LOG(logDEBUG1, ("Transceiver Channels: %d\n", ntchans));
    }
    *nchanx = nachans + ndchans + ntchans;
    LOG(logDEBUG, ("Total #Channels: %d\n", *nchanx));
    *nchany = 1;
}

int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }
