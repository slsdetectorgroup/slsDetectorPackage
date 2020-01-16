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

#include "ansi.h"

#ifdef __cplusplus
//C++ includes
#include "sls_detector_exceptions.h"
#include <algorithm>
#include <cstdint>
#include <bitset>
#include <string>
#else
//C includes
#include <stdint.h>
#endif


#define BIT32_MASK  0xFFFFFFFF
#define MAX_RX_DBIT 64

/** default ports */
#define DEFAULT_PORTNO 1952
#define DEFAULT_UDP_PORTNO 50001
#define DEFAULT_ZMQ_CL_PORTNO 30001
#define DEFAULT_ZMQ_RX_PORTNO 30001

#define SLS_DETECTOR_HEADER_VERSION 0x2
#define SLS_DETECTOR_JSON_HEADER_VERSION 0x3

// ctb/ moench 1g udp (read from fifo)
#define UDP_PACKET_DATA_BYTES (1344)

/** maximum rois */
#define MAX_ROIS 100

/** maximum trim en */
#define MAX_TRIMEN 100

/** maximum unit size of program sent to detector */
#define MAX_FPGAPROGRAMSIZE (2 * 1024 * 1024)

/** get flag form most functions */
#define GET_FLAG -1

#define DEFAULT_DET_MAC "00:aa:bb:cc:dd:ee"
#define DEFAULT_DET_IP "129.129.202.45"
#define DEFAULT_DET_MAC2 "00:aa:bb:cc:dd:ff"
#define DEFAULT_DET_IP2 "129.129.202.46"

/** default maximum string length */
#define MAX_STR_LENGTH 1000
#define MAX_FRAMES_PER_FILE 20000
#define SHORT_MAX_FRAMES_PER_FILE 100000
#define MOENCH_MAX_FRAMES_PER_FILE 100000
#define EIGER_MAX_FRAMES_PER_FILE 10000
#define JFRAU_MAX_FRAMES_PER_FILE 10000
#define CTB_MAX_FRAMES_PER_FILE 20000
#define MYTHEN3_MAX_FRAMES_PER_FILE 10000
#define GOTTHARD2_MAX_FRAMES_PER_FILE 20000

#define DEFAULT_STREAMING_TIMER_IN_MS 200


#ifdef __cplusplus
class slsDetectorDefs {
  public:
#endif

    /** Type of the detector */
    enum detectorType {
        GET_DETECTOR_TYPE = -1, 
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
    enum {
        OK,   /**< function succeeded */
        FAIL, /**< function failed */
        FORCE_UPDATE
    };

    /** staus mask */
    enum runStatus {
        IDLE,    /**< detector ready to start acquisition - no data in memory */
        ERROR,   /**< error i.e. normally fifo full */
        WAITING, /**< waiting for trigger or gate signal */
        RUN_FINISHED, /**< acquisition not running but data in memory */
        TRANSMITTING, /**< acquisition running and data in memory */
        RUNNING,      /**< acquisition  running, no data in memory */
        STOPPED       /**< acquisition stopped externally */
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
        uint64_t frameNumber; /**< is the frame number */
        uint32_t expLength;   /**< is the subframe number (32 bit eiger) or real
                                 time exposure time in 100ns (others) */
        uint32_t packetNumber; /**< is the packet number */
        uint64_t bunchId;      /**< is the bunch id from beamline */
        uint64_t timestamp;    /**< is the time stamp with 10 MHz clock */
        uint16_t modId; /**< is the unique module id (unique even for left,
                           right, top, bottom) */
        uint16_t row;   /**< is the row index in the complete detector system */
        uint16_t
            column; /**< is the column index in the complete detector system */
        uint16_t reserved;     /**< is reserved */
        uint32_t debug;        /**< is for debugging purposes */
        uint16_t roundRNumber; /**< is the round robin set number */
        uint8_t detType;       /**< is the detector type see :: detectorType */
        uint8_t version; /**< is the version number of this structure format */
    } sls_detector_header;

#ifdef __cplusplus
#define MAX_NUM_PACKETS 512
    using sls_bitset = std::bitset<MAX_NUM_PACKETS>;
    using bitset_storage = uint8_t[MAX_NUM_PACKETS / 8];
    struct sls_receiver_header{
        sls_detector_header detHeader; /**< is the detector header */
        sls_bitset packetsMask;        /**< is the packets caught bit mask */
    };
#endif
    enum frameDiscardPolicy {
        GET_FRAME_DISCARD_POLICY = -1, /**< to get the missing packet mode */
        NO_DISCARD, /**< pad incomplete packets with -1, default mode */
        DISCARD_EMPTY_FRAMES, /**< discard incomplete frames, fastest mode, save
                                 space, not suitable for multiple modules */
        DISCARD_PARTIAL_FRAMES, /**< ignore missing packets, must check with
                                   packetsMask for data integrity, fast mode and
                                   suitable for multiple modules */
        NUM_DISCARD_POLICIES
    };

    enum fileFormat {
        GET_FILE_FORMAT = -1, /**< the receiver will return its file format */
        BINARY,               /**< binary format */
        HDF5,                 /**< hdf5 format */
        NUM_FILE_FORMATS
    };

    /**
        @short structure for a region of interest
        xmin,xmax,ymin,ymax define the limits of the region
    */

   #ifdef __cplusplus
    struct ROI {
        int xmin{-1}; /**< is the roi xmin (in channel number) */
        int xmax{-1}; /**< is the roi xmax  (in channel number)*/
    };
    #else
    typedef struct {
        int xmin; /**< is the roi xmin (in channel number) */
        int xmax; /**< is the roi xmax  (in channel number)*/
     } ROI;
    #endif

    /**
        type of action performed (for text client)
    */
    enum { GET_ACTION, PUT_ACTION, READOUT_ACTION, HELP_ACTION };


    /**
        dimension indexes
    */
    enum dimension {
        X = 0, /**< X dimension */
        Y = 1  /**< Y dimension */
    };

#ifdef __cplusplus
    struct xy {
        int x{0};
        int y{0};
    };
#endif


    /**
      use of the external signals
    */
    enum externalSignalFlag {
        GET_EXTERNAL_SIGNAL_FLAG = -1, /**<return flag for signal */
        SIGNAL_OFF,                    /**<signal unused - tristate*/
        GATE_IN_ACTIVE_HIGH,           /**<input gate active high*/
        GATE_IN_ACTIVE_LOW,            /**<input gate active low */
        TRIGGER_IN_RISING_EDGE,    /**<input exposure trigger on rising edge */
        TRIGGER_IN_FALLING_EDGE,   /**<input exposure trigger on falling edge */
        RO_TRIGGER_IN_RISING_EDGE, /**<input raedout trigger on rising edge */
        RO_TRIGGER_IN_FALLING_EDGE, /**<input readout trigger on falling edge */
        GATE_OUT_ACTIVE_HIGH, /**<output active high when detector is exposing*/
        GATE_OUT_ACTIVE_LOW,  /**<output active low when detector is exposing*/
        TRIGGER_OUT_RISING_EDGE,     /**<output trigger rising edge at start of
                                        exposure */
        TRIGGER_OUT_FALLING_EDGE,    /**<output trigger falling edge at start of
                                        exposure */
        RO_TRIGGER_OUT_RISING_EDGE,  /**<output trigger rising edge at start of
                                        readout */
        RO_TRIGGER_OUT_FALLING_EDGE, /**<output trigger falling edge at start of
                                        readout */
        OUTPUT_LOW,                  /**< output always low */
        OUTPUT_HIGH,                 /**< output always high */
        MASTER_SLAVE_SYNCHRONIZATION /**< reserved for master/slave
                                        synchronization in multi detector
                                        systems */
    };

    /**
      communication mode using external signals
    */
    enum timingMode {
        GET_TIMING_MODE = -1, /**<return flag for communication mode */
        AUTO_TIMING,      /**< internal timing */
        TRIGGER_EXPOSURE, /**< trigger mode i.e. exposure is triggered */
        GATED,            /**< gated  */
        BURST_TRIGGER     /**< trigger a burst of frames */
    };

    /**
       detector dacs indexes
    */
    enum dacIndex {
        THRESHOLD,
        CALIBRATION_PULSE,
        TRIMBIT_SIZE,
        PREAMP,
        SHAPER1,
        SHAPER2,
        TEMPERATURE_ADC,
        TEMPERATURE_FPGA,
        VREF_DS,
        VCASCN_PB,
        VCASCP_PB,
        VOUT_CM,
        VCASC_OUT,
        VIN_CM,
        VREF_COMP,
        IB_TESTC,
        SVP,
        SVN,
        VTR,
        VRF,
        VRS,
        VTGSTV,
        VCMP_LL,
        VCMP_LR,
        CAL,
        VCMP_RL,
        VCMP_RR,
        RXB_RB,
        RXB_LB,
        VCP,
        VCN,
        VIS,
        IO_DELAY,
        ADC_VPP,
        HIGH_VOLTAGE,
        TEMPERATURE_FPGAEXT,
        TEMPERATURE_10GE,
        TEMPERATURE_DCDC,
        TEMPERATURE_SODL,
        TEMPERATURE_SODR,
        TEMPERATURE_FPGA2,
        TEMPERATURE_FPGA3,
        VIPRE,
        VIINSH,
        VDCSH,
        VTH2, 
        VPL,  
        VTH3, 
        CASSH,
        CAS,  
        VICIN,
        VIPRE_OUT,
        VREF_H_ADC,
        VB_COMP_FE,
        VB_COMP_ADC,
        VCOM_CDS,  
        VREF_RESTORE,
        VB_OPA_1ST,
        VREF_COMP_FE,
        VCOM_ADC1, 
        VREF_PRECH,
        VREF_L_ADC,
        VREF_CDS,  
        VB_CS,     
        VB_OPA_FD, 
        VCOM_ADC2, 
        VB_DS,
        VB_COMP,
        VB_PIXBUF,
        VIN_COM,
        VDD_PROT,
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
        GET_SETTINGS = -1, /**< return current detector settings */
        STANDARD,          /**< standard settings */
        FAST,              /**< fast settings */
        HIGHGAIN,          /**< highgain  settings */
        DYNAMICGAIN,       /**< dynamic gain  settings */
        LOWGAIN,           /**< low gain  settings */
        MEDIUMGAIN,        /**< medium gain  settings */
        VERYHIGHGAIN,      /**< very high gain  settings */
        DYNAMICHG0,        /**< dynamic high gain 0 */
        FIXGAIN1,          /**< fix gain 1 */
        FIXGAIN2,          /**< fix gain 2 */
        FORCESWITCHG1,     /**< force switch gain 1 */
        FORCESWITCHG2,     /**< force switch gain 2 */
        VERYLOWGAIN,       /**< very low gain settings */
        UNDEFINED = 200,   /**< undefined or custom  settings */
        UNINITIALIZED      /**< uninitialiazed (status at startup) */
    };

#define TRIMBITMASK 0x3f

    enum clockIndex {
        ADC_CLOCK,
        DBIT_CLOCK,
        RUN_CLOCK,
        SYNC_CLOCK
    };

    /**
     * read out mode (ctb, moench)
     */
    enum readoutMode {
        ANALOG_ONLY,
        DIGITAL_ONLY,
        ANALOG_AND_DIGITAL
    };

    /** chip speed */
    enum speedLevel {
        FULL_SPEED,
        HALF_SPEED,
        QUARTER_SPEED
    };

    /** port type */
    enum portType {
        CONTROL_PORT, /**< control port */
        STOP_PORT,    /**<stop port */
        DATA_PORT     /**< receiver tcp port with client*/
    };

    /** hierarchy in multi-detector structure, if any */
    enum masterFlags {
        GET_MASTER = -1, /**< return master flag */
        NO_MASTER,       /**< no master/slave hierarchy defined */
        IS_MASTER,       /**<is master */
        IS_SLAVE         /**< is slave */
    };

    /**
     * frame mode for processor
     */
    enum frameModeType {
        GET_FRAME_MODE = -1,
        PEDESTAL,     /** < pedestal */
        NEW_PEDESTAL, /** < new pedestal */
        FLATFIELD,    /** < flatfield */
        NEW_FLATFIELD /** < new flatfield */
    };

    /**
     * detector mode for processor
     */
    enum detectorModeType {
        GET_DETECTOR_MODE = -1,
        COUNTING,      /** < counting */
        INTERPOLATING, /** < interpolating */
        ANALOG         /** < analog */
    };


#ifdef __cplusplus
  protected:
#endif

#ifndef MYROOT
#include "sls_detector_funcs.h"
#endif

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
    int dynamicRange{0};
    int nGappixelsX{0};
    int nGappixelsY{0};

    detParameters() = default;
    explicit detParameters(slsDetectorDefs::detectorType type) {
        switch (type) {
        case slsDetectorDefs::detectorType::GOTTHARD:
            nChanX = 128;
            nChanY = 1;
            nChipX = 10;
            nChipY = 1;
            nDacs = 8;
            dynamicRange = 16;
            nGappixelsX = 0;
            nGappixelsY = 0;
            break;
        case slsDetectorDefs::detectorType::JUNGFRAU:
            nChanX = 256;
            nChanY = 256;
            nChipX = 4;
            nChipY = 2;
            nDacs = 8;
            dynamicRange = 16;
            nGappixelsX = 0;
            nGappixelsY = 0;
            break;
        case slsDetectorDefs::detectorType::CHIPTESTBOARD:
            nChanX = 36;
            nChanY = 1;
            nChipX = 1;
            nChipY = 1;
            nDacs = 24;
            dynamicRange = 16;
            nGappixelsX = 0;
            nGappixelsY = 0;
            break;
        case slsDetectorDefs::detectorType::MOENCH:
            nChanX = 32;
            nChanY = 1;
            nChipX = 1;
            nChipY = 1;
            nDacs = 8;
            dynamicRange = 16;
            nGappixelsX = 0;
            nGappixelsY = 0;
            break;
        case slsDetectorDefs::detectorType::EIGER:
            nChanX = 256;
            nChanY = 256;
            nChipX = 4;
            nChipY = 1;
            nDacs = 16;
            dynamicRange = 16;
            nGappixelsX = 6;
            nGappixelsY = 1;
            break;
        case slsDetectorDefs::detectorType::MYTHEN3:
            nChanX = 128 * 3;
            nChanY = 1;
            nChipX = 10;
            nChipY = 1;
            nDacs = 16;
            dynamicRange = 32;
            nGappixelsX = 0;
            nGappixelsY = 0;
            break;
        case slsDetectorDefs::detectorType::GOTTHARD2:
            nChanX = 128;
            nChanY = 1;
            nChipX = 10;
            nChipY = 1;
            nDacs = 14;
            dynamicRange = 16;
            nGappixelsX = 0;
            nGappixelsY = 0;
            break;    
        default:
            throw sls::RuntimeError("Unknown detector type! " + std::to_string(type));
        }
    }
};
#endif

/**
    @short  structure for a detector module

    should not be used by unexperienced users

    \see  :: moduleRegisterBit ::chipRegisterBit :channelRegisterBit

    @li reg is the module register (e.g. dynamic range? see moduleRegisterBit)
    @li dacs is the pointer to the array of dac values (in V)
    @li adcs is the pointer to the array of adc values (in V)
    @li chipregs is the pointer to the array of chip registers
    @li chanregs is the pointer to the array of channel registers
    @li gain is the module gain
    @li offset is the module offset
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
        : serialnumber(0), nchan(0), nchip(0), ndac(0), reg(0), iodelay(0),
          tau(0), eV(0), dacs(nullptr), chanregs(nullptr) {}

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
//TODO! discuss this
#include <vector> //hmm... but currently no way around
namespace sls{
using Positions = const std::vector<int> &;
using defs = slsDetectorDefs;
} // namespace sls
#endif
