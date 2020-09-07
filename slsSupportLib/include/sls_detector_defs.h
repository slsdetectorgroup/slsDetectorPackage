#pragma once
/************************************************
 * @file sls_detector_defs.h
 * @short contains all the constants, enum definitions and enum-string
 *conversions
 ***********************************************/
/**
 *@short contains all the constants, enum definitions and enum-string
 *conversions
 */

#ifdef __CINT__
#define MYROOT
#define __cplusplus
#endif

#ifdef __cplusplus
// C++ includes
#include "sls_detector_exceptions.h"
#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <string>
#else
// C includes
#include <stdint.h>
#endif

#define BIT32_MASK  0xFFFFFFFF
#define MAX_RX_DBIT 64

/** default ports */
#define DEFAULT_PORTNO        1952
#define DEFAULT_UDP_PORTNO    50001
#define DEFAULT_ZMQ_CL_PORTNO 30001
#define DEFAULT_ZMQ_RX_PORTNO 30001

#define SLS_DETECTOR_HEADER_VERSION      0x2
#define SLS_DETECTOR_JSON_HEADER_VERSION 0x4

// ctb/ moench 1g udp (read from fifo)
#define UDP_PACKET_DATA_BYTES (1344)

/** maximum rois */
#define MAX_ROIS 100

/** maximum trim en */
#define MAX_TRIMEN 100

/** maximum unit size of program sent to detector */
#define MAX_FPGAPROGRAMSIZE (2 * 1024 * 1024)

#define GET_FLAG -1

#define DEFAULT_DET_MAC  "00:aa:bb:cc:dd:ee"
#define DEFAULT_DET_IP   "129.129.202.45"
#define DEFAULT_DET_MAC2 "00:aa:bb:cc:dd:ff"
#define DEFAULT_DET_IP2  "129.129.202.46"

/** default maximum string length */
#define MAX_STR_LENGTH   1000
#define SHORT_STR_LENGTH 20

#define MAX_PATTERN_LENGTH 0x2000

#define DEFAULT_STREAMING_TIMER_IN_MS 200

#define NUM_RX_THREAD_IDS 8

#ifdef __cplusplus
class slsDetectorDefs {
  public:
#endif

    /** Type of the detector */
    enum detectorType {
        GENERIC,
        EIGER,
        GOTTHARD,
        JUNGFRAU,
        CHIPTESTBOARD,
        MOENCH,
        MYTHEN3,
        GOTTHARD2,
    };

    /**  return values */
    enum { OK, FAIL };

    /** staus mask */
    enum runStatus {
        IDLE,
        ERROR,
        WAITING,
        RUN_FINISHED,
        TRANSMITTING,
        RUNNING,
        STOPPED
    };

    /**
        @short  structure for a Detector Packet or Image Header
        @li frameNumber is the frame number
        @li expLength is the subframe number (32 bit eiger) or real time
       exposure time in 100ns (others)
        @li packetNumber is the packet number
        @li bunchId is the bunch id from beamline
        @li timestamp is the time stamp with 10 MHz clock
        @li modId is the unique module id (unique even for left, right, top,
       bottom)
        @li row is the row index in the complete detector system
        @li column is the column index in the complete detector system
        @li reserved is reserved
        @li debug is for debugging purposes
        @li roundRNumber is the round robin set number
        @li detType is the detector type see :: detectorType
        @li version is the version number of this structure format
    */

    typedef struct {
        uint64_t frameNumber;
        uint32_t expLength;
        uint32_t packetNumber;
        uint64_t bunchId;
        uint64_t timestamp;
        uint16_t modId;
        uint16_t row;
        uint16_t column;
        uint16_t reserved;
        uint32_t debug;
        uint16_t roundRNumber;
        uint8_t detType;
        uint8_t version;
    } sls_detector_header;

#ifdef __cplusplus
    // For sending and receiving data
    static_assert(sizeof(detectorType) == sizeof(int),
                  "enum and int differ in size");
#define MAX_NUM_PACKETS 512
    using sls_bitset = std::bitset<MAX_NUM_PACKETS>;
    using bitset_storage = uint8_t[MAX_NUM_PACKETS / 8];
    struct sls_receiver_header {
        sls_detector_header detHeader; /**< is the detector header */
        sls_bitset packetsMask;        /**< is the packets caught bit mask */
    };
#endif
    enum frameDiscardPolicy {
        NO_DISCARD,
        DISCARD_EMPTY_FRAMES,
        DISCARD_PARTIAL_FRAMES,
        NUM_DISCARD_POLICIES
    };

    enum fileFormat { BINARY, HDF5, NUM_FILE_FORMATS };

    /**
        @short structure for a region of interest
        xmin,xmax,ymin,ymax define the limits of the region
    */
#ifdef __cplusplus
    struct ROI {
        int xmin{-1};
        int xmax{-1};
        ROI() = default;
        ROI(int xmin, int xmax) : xmin(xmin), xmax(xmax){};
    } __attribute__((packed));
#else
typedef struct {
    int xmin;
    int xmax;
} ROI;
#endif

    /**
        type of action performed (for text client)
    */
    enum { GET_ACTION, PUT_ACTION, READOUT_ACTION, HELP_ACTION };

    /**
        dimension indexes
    */
    enum dimension { X, Y };

#ifdef __cplusplus
    struct xy {
        int x{0};
        int y{0};
        xy() = default;
        xy(int x, int y) : x(x), y(y){};
    } __attribute__((packed));
#endif

    /**
      use of the external signals
    */
    enum externalSignalFlag {
        TRIGGER_IN_RISING_EDGE,
        TRIGGER_IN_FALLING_EDGE,
        INVERSION_ON,
        INVERSION_OFF
    };

    /**
      communication mode using external signals
    */
    enum timingMode {
        AUTO_TIMING,
        TRIGGER_EXPOSURE,
        GATED,
        BURST_TRIGGER,
        TRIGGER_GATED,
        NUM_TIMING_MODES
    };

    /**
       detector dacs indexes
    */
    enum dacIndex {
        DAC_0,
        DAC_1,
        DAC_2,
        DAC_3,
        DAC_4,
        DAC_5,
        DAC_6,
        DAC_7,
        DAC_8,
        DAC_9,
        DAC_10,
        DAC_11,
        DAC_12,
        DAC_13,
        DAC_14,
        DAC_15,
        DAC_16,
        DAC_17,
        VSVP,
        VTRIM,
        VRPREAMP,
        VRSHAPER,
        VSVN,
        VTGSTV,
        VCMP_LL,
        VCMP_LR,
        VCAL,
        VCMP_RL,
        RXB_RB,
        RXB_LB,
        VCMP_RR,
        VCP,
        VCN,
        VISHAPER,
        VTHRESHOLD,
        IO_DELAY,
        VREF_DS,
        VCASCN_PB,
        VCASCP_PB,
        VOUT_CM,
        VCASC_OUT,
        VIN_CM,
        VREF_COMP,
        IB_TESTC,
        VB_COMP,
        VDD_PROT,
        VIN_COM,
        VREF_PRECH,
        VB_PIXBUF,
        VB_DS,
        VREF_H_ADC,
        VB_COMP_FE,
        VB_COMP_ADC,
        VCOM_CDS,
        VREF_RSTORE,
        VB_OPA_1ST,
        VREF_COMP_FE,
        VCOM_ADC1,
        VREF_L_ADC,
        VREF_CDS,
        VB_CS,
        VB_OPA_FD,
        VCOM_ADC2,
        VCASSH,
        VTH2,
        VRSHAPER_N,
        VIPRE_OUT,
        VTH3,
        VTH1,
        VICIN,
        VCAS,
        VCAL_N,
        VIPRE,
        VCAL_P,
        VDCSH,
        VBP_COLBUF,
        VB_SDA,
        VCASC_SFP,
        VIPRE_CDS,
        IBIAS_SFP,
        ADC_VPP,
        HIGH_VOLTAGE,
        TEMPERATURE_ADC,
        TEMPERATURE_FPGA,
        TEMPERATURE_FPGAEXT,
        TEMPERATURE_10GE,
        TEMPERATURE_DCDC,
        TEMPERATURE_SODL,
        TEMPERATURE_SODR,
        TEMPERATURE_FPGA2,
        TEMPERATURE_FPGA3,
        TRIMBIT_SCAN,
        V_POWER_A = 100,
        V_POWER_B = 101,
        V_POWER_C = 102,
        V_POWER_D = 103,
        V_POWER_IO = 104,
        V_POWER_CHIP = 105,
        I_POWER_A = 106,
        I_POWER_B = 107,
        I_POWER_C = 108,
        I_POWER_D = 109,
        I_POWER_IO = 110,
        V_LIMIT = 111,
        SLOW_ADC0 = 1000,
        SLOW_ADC1,
        SLOW_ADC2,
        SLOW_ADC3,
        SLOW_ADC4,
        SLOW_ADC5,
        SLOW_ADC6,
        SLOW_ADC7,
        SLOW_ADC_TEMP
    };

    /**
       detector settings indexes
    */
    enum detectorSettings {
        STANDARD,
        FAST,
        HIGHGAIN,
        DYNAMICGAIN,
        LOWGAIN,
        MEDIUMGAIN,
        VERYHIGHGAIN,
        DYNAMICHG0,
        FIXGAIN1,
        FIXGAIN2,
        FORCESWITCHG1,
        FORCESWITCHG2,
        VERYLOWGAIN,
        G1_HIGHGAIN,
        G1_LOWGAIN,
        G2_HIGHCAP_HIGHGAIN,
        G2_HIGHCAP_LOWGAIN,
        G2_LOWCAP_HIGHGAIN,
        G2_LOWCAP_LOWGAIN,
        G4_HIGHGAIN,
        G4_LOWGAIN,
        UNDEFINED = 200,
        UNINITIALIZED
    };

#define TRIMBITMASK 0x3f

    enum clockIndex { ADC_CLOCK, DBIT_CLOCK, RUN_CLOCK, SYNC_CLOCK };

    /**
     * read out mode (ctb, moench)
     */
    enum readoutMode { ANALOG_ONLY, DIGITAL_ONLY, ANALOG_AND_DIGITAL };

    /** chip speed */
    enum speedLevel { FULL_SPEED, HALF_SPEED, QUARTER_SPEED };

    /** hierarchy in multi-detector structure, if any */
    enum masterFlags { NO_MASTER, IS_MASTER, IS_SLAVE };

    /**
     * frame mode for processor
     */
    enum frameModeType { PEDESTAL, NEW_PEDESTAL, FLATFIELD, NEW_FLATFIELD };

    /**
     * detector mode for processor
     */
    enum detectorModeType { COUNTING, INTERPOLATING, ANALOG };

    /**
     * burst mode for gotthard2
     */
    enum burstMode {
        BURST_INTERNAL,
        BURST_EXTERNAL,
        CONTINUOUS_INTERNAL,
        CONTINUOUS_EXTERNAL,
        NUM_BURST_MODES
    };

    /**
     * timing source for gotthard2
     */
    enum timingSourceType { TIMING_INTERNAL, TIMING_EXTERNAL };

#ifdef __cplusplus
    /**
     * structure to udpate receiver
     */
    struct rxParameters {
        detectorType detType{GENERIC};
        xy numberOfDetector;
        int moduleId{0};
        char hostname[MAX_STR_LENGTH];
        int udpInterfaces{1};
        int udp_dstport{0};
        uint32_t udp_dstip{0U};
        uint64_t udp_dstmac{0LU};
        int udp_dstport2{0};
        uint32_t udp_dstip2{0U};
        uint64_t udp_dstmac2{0LU};
        int64_t frames{0};
        int64_t triggers{0};
        int64_t bursts{0};
        int analogSamples{0};
        int digitalSamples{0};
        int64_t expTimeNs{0};
        int64_t periodNs{0};
        int64_t subExpTimeNs{0};
        int64_t subDeadTimeNs{0};
        int activate{0};
        int quad{0};
        int dynamicRange{16};
        timingMode timMode{AUTO_TIMING};
        int tenGiga{0};
        readoutMode roMode{ANALOG_ONLY};
        uint32_t adcMask{0};
        uint32_t adc10gMask{0};
        ROI roi;
        uint32_t countermask{0};
        burstMode burstType{BURST_INTERNAL};
        int64_t expTime1Ns{0};
        int64_t expTime2Ns{0};
        int64_t expTime3Ns{0};
        int64_t gateDelay1Ns{0};
        int64_t gateDelay2Ns{0};
        int64_t gateDelay3Ns{0};
        int gates{0};
    } __attribute__((packed));

    /** pattern structure */
    struct patternParameters {
        uint64_t word[MAX_PATTERN_LENGTH]{};
        uint64_t patioctrl{0};
        uint32_t patlimits[2]{};
        uint32_t patloop[6]{};
        uint32_t patnloop[3]{};
        uint32_t patwait[3]{};
        uint64_t patwaittime[3]{};
    } __attribute__((packed));

    /** scan structure */
    struct scanParameters {
        int enable;
        dacIndex dacInd;
        int startOffset;
        int stopOffset;
        int stepSize;
        int64_t dacSettleTime_ns;

        /** disable scan */
        scanParameters()
            : enable(0), dacInd(DAC_0), startOffset(0), stopOffset(0),
              stepSize(0), dacSettleTime_ns{0} {}
        /** enable scan */
        scanParameters(
            dacIndex dac, int start, int stop, int step,
            std::chrono::nanoseconds t = std::chrono::milliseconds{1})
            : enable(1), dacInd(dac), startOffset(start), stopOffset(stop),
              stepSize(step) {
            dacSettleTime_ns = t.count();
        }
        bool operator==(const scanParameters &other) const {
            return ((enable == other.enable) && (dacInd == other.dacInd) &&
                    (startOffset == other.startOffset) &&
                    (stopOffset == other.stopOffset) &&
                    (stepSize == other.stepSize) &&
                    (dacSettleTime_ns == other.dacSettleTime_ns));
        }
    } __attribute__((packed));
#endif

#ifdef __cplusplus
  protected:
#endif

    // #ifndef MYROOT
    // #include "sls_detector_funcs.h"
    // #endif

#ifdef __cplusplus
};
#endif
;

#ifdef __cplusplus
struct detParameters {
    int nChanX{0};
    int nChanY{0};
    int nChipX{0};
    int nChipY{0};
    int nDacs{0};

    detParameters() = default;
    explicit detParameters(slsDetectorDefs::detectorType type) {
        switch (type) {
        case slsDetectorDefs::detectorType::GOTTHARD:
            nChanX = 128;
            nChanY = 1;
            nChipX = 10;
            nChipY = 1;
            nDacs = 8;
            break;
        case slsDetectorDefs::detectorType::JUNGFRAU:
            nChanX = 256;
            nChanY = 256;
            nChipX = 4;
            nChipY = 2;
            nDacs = 8;
            break;
        case slsDetectorDefs::detectorType::CHIPTESTBOARD:
            nChanX = 36;
            nChanY = 1;
            nChipX = 1;
            nChipY = 1;
            nDacs = 24;
            break;
        case slsDetectorDefs::detectorType::MOENCH:
            nChanX = 32;
            nChanY = 1;
            nChipX = 1;
            nChipY = 1;
            nDacs = 8;
            break;
        case slsDetectorDefs::detectorType::EIGER:
            nChanX = 256;
            nChanY = 256;
            nChipX = 4;
            nChipY = 1;
            nDacs = 16;
            break;
        case slsDetectorDefs::detectorType::MYTHEN3:
            nChanX = 128 * 3;
            nChanY = 1;
            nChipX = 10;
            nChipY = 1;
            nDacs = 16;
            break;
        case slsDetectorDefs::detectorType::GOTTHARD2:
            nChanX = 128;
            nChanY = 1;
            nChipX = 10;
            nChipY = 1;
            nDacs = 14;
            break;
        default:
            throw sls::RuntimeError("Unknown detector type! " +
                                    std::to_string(type));
        }
    }
};
#endif

/**
    @short  structure for a detector module
*/
#ifdef __cplusplus
struct sls_detector_module {
#else
typedef struct {
#endif
    int serialnumber; /**< is the module serial number */
    int nchan;        /**< is the number of channels on the module*/
    int nchip;        /**< is the number of chips on the module */
    int ndac;         /**< is the number of dacs on the module */
    int reg;          /**< is the module register settings (gain level) */
    int iodelay;      /**< iodelay */
    int tau;          /**< tau */
    int eV;           /**< threshold energy */
    int *dacs;     /**< is the pointer to the array of the dac values (in V) */
    int *chanregs; /**< is the pointer to the array of the channel registers */

#ifdef __cplusplus
    sls_detector_module()
        : serialnumber(0), nchan(0), nchip(0), ndac(0), reg(-1), iodelay(0),
          tau(0), eV(-1), dacs(nullptr), chanregs(nullptr) {}

    explicit sls_detector_module(slsDetectorDefs::detectorType type)
        : sls_detector_module() {
        detParameters parameters{type};
        int nch = parameters.nChanX * parameters.nChanY;
        int nc = parameters.nChipX * parameters.nChipY;
        ndac = parameters.nDacs;
        nchip = nc;
        nchan = nch * nc;
        dacs = new int[ndac];
        chanregs = new int[nchan];
    }

    sls_detector_module(const sls_detector_module &other)
        : dacs(nullptr), chanregs(nullptr) {
        *this = other;
    }

    sls_detector_module &operator=(const sls_detector_module &other) {
        delete[] dacs;
        delete[] chanregs;
        serialnumber = other.serialnumber;
        nchan = other.nchan;
        nchip = other.nchip;
        ndac = other.ndac;
        reg = other.reg;
        iodelay = other.iodelay;
        tau = other.tau;
        eV = other.eV;
        dacs = new int[ndac];
        std::copy(other.dacs, other.dacs + ndac, dacs);
        chanregs = new int[nchan];
        std::copy(other.chanregs, other.chanregs + nchan, chanregs);
        return *this;
    }

    ~sls_detector_module() {
        delete[] dacs;
        delete[] chanregs;
    }
};
#else
} sls_detector_module;
#endif

#ifdef __cplusplus
// TODO! discuss this
#include <vector> //hmm... but currently no way around
namespace sls {
using Positions = const std::vector<int> &;
using defs = slsDetectorDefs;
} // namespace sls
#endif
