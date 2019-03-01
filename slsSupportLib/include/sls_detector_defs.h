#pragma once
/************************************************
 * @file sls_detector_defs.h
 * @short contains all the constants, enum definitions and enum-string conversions
 ***********************************************/
/**
 *@short contains all the constants, enum definitions and enum-string conversions
 */


#ifdef __CINT__
#define MYROOT
#define __cplusplus
#endif

#include <stdint.h> 
#ifdef __cplusplus
#include <bitset>
#include <string>
#endif
#include "ansi.h"


typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;


/** default ports */
#define DEFAULT_PORTNO    		1952
#define DEFAULT_UDP_PORTNO 		50001
#define DEFAULT_GUI_PORTNO 		65001
#define DEFAULT_ZMQ_CL_PORTNO 	30001
#define DEFAULT_ZMQ_RX_PORTNO 	30001

#define SLS_DETECTOR_HEADER_VERSION         0x2
#define SLS_DETECTOR_JSON_HEADER_VERSION    0x3

// ctb/ moench 1g udp (read from fifo)
#define UDP_PACKET_DATA_BYTES		(1344)

/** maximum rois */
#define MAX_ROIS 100

/** maximum trim en */
#define MAX_TRIMEN 100

/** maximum unit size of program sent to detector */
#define MAX_FPGAPROGRAMSIZE (2 * 1024 *1024)

/** get flag form most functions */
#define GET_FLAG -1

#define DEFAULT_DET_MAC         "00:aa:bb:cc:dd:ee"
#define DEFAULT_DET_IP          "129.129.202.46"

/** default maximum string length */
#define MAX_STR_LENGTH 1000
#define MAX_FRAMES_PER_FILE			20000
#define SHORT_MAX_FRAMES_PER_FILE	100000
#define MOENCH_MAX_FRAMES_PER_FILE	100000
#define EIGER_MAX_FRAMES_PER_FILE	10000
#define JFRAU_MAX_FRAMES_PER_FILE 	10000
#define CTB_MAX_FRAMES_PER_FILE   20000

#define DEFAULT_STREAMING_TIMER_IN_MS 200



typedef char mystring[MAX_STR_LENGTH];


#ifdef __cplusplus
class slsDetectorDefs {
public:
	slsDetectorDefs(){};
#endif

	/**
       Type of the detector
	 */
	enum detectorType {
		GET_DETECTOR_TYPE=-1,   /**< the detector will return its type */
		GENERIC,  /**< generic sls detector */
		EIGER, /**< eiger */
		GOTTHARD, /**< gotthard */
		JUNGFRAU, /**< jungfrau */
		CHIPTESTBOARD, /**< CTB */
		MOENCH /**< moench */
	};


	/**
      return values
	 */
	enum  {
		OK, /**< function succeeded */
		FAIL, /**< function failed */
		FORCE_UPDATE
	};


	/**
     indexes for the acquisition timers
	 */
	enum timerIndex {
		FRAME_NUMBER, /**< number of real time frames: total number of acquisitions is number or frames*number of cycles */
		ACQUISITION_TIME, /**< exposure time */
		FRAME_PERIOD, /**< period between exposures */
		DELAY_AFTER_TRIGGER, /**< delay between trigger and start of exposure or readout (in triggered mode) */
		GATES_NUMBER, /**< number of gates per frame (in gated mode) */
		CYCLES_NUMBER, /**< number of cycles: total number of acquisitions is number or frames*number of cycles */
		ACTUAL_TIME, /**< Actual time of the detector's internal timer */
		MEASUREMENT_TIME,  /**< Time of the measurement from the detector (fifo) */

		PROGRESS, /**< fraction of measurement elapsed - only get! */
		MEASUREMENTS_NUMBER,
		FRAMES_FROM_START,
		FRAMES_FROM_START_PG,
		SAMPLES,
		SUBFRAME_ACQUISITION_TIME, /**< subframe exposure time */
		STORAGE_CELL_NUMBER, /**<number of storage cells */
		SUBFRAME_DEADTIME, /**< subframe deadtime */
		MEASURED_PERIOD,	/**< measured period */
		MEASURED_SUBPERIOD,	/**< measured subperiod */
		STORAGE_CELL_DELAY, /**< storage cell delay */
		MAX_TIMERS
	};


	/**
    staus mask
	 */
	enum runStatus {
		IDLE, /**< detector ready to start acquisition - no data in memory */
		ERROR, /**< error i.e. normally fifo full */
		WAITING, /**< waiting for trigger or gate signal */
		RUN_FINISHED, /**< acquisition not running but data in memory */
		TRANSMITTING, /**< acquisition running and data in memory */
		RUNNING, /**< acquisition  running, no data in memory */
		STOPPED /**< acquisition stopped externally */
	};



	/**
	    @short  structure for a Detector Packet or Image Header
	    @li frameNumber is the frame number
	    @li expLength is the subframe number (32 bit eiger) or real time exposure time in 100ns (others)
	    @li packetNumber is the packet number
	    @li bunchId is the bunch id from beamline
	    @li timestamp is the time stamp with 10 MHz clock
	    @li modId is the unique module id (unique even for left, right, top, bottom)
	    @li row is the row index in the complete detector system
	    @li column is the column index in the complete detector system
	    @li reserved is reserved
	    @li debug is for debugging purposes
	    @li roundRNumber is the round robin set number
	    @li detType is the detector type see :: detectorType
	    @li version is the version number of this structure format
	*/

	typedef struct {
		uint64_t frameNumber;			/**< is the frame number */
		uint32_t expLength;				/**< is the subframe number (32 bit eiger) or real time exposure time in 100ns (others) */
		uint32_t packetNumber;			/**< is the packet number */
		uint64_t bunchId;				/**< is the bunch id from beamline */
		uint64_t timestamp;				/**< is the time stamp with 10 MHz clock */
		uint16_t modId;					/**< is the unique module id (unique even for left, right, top, bottom) */
		uint16_t row;					/**< is the row index in the complete detector system */
		uint16_t column;				/**< is the column index in the complete detector system */
		uint16_t reserved;				/**< is reserved */
		uint32_t debug;					/**< is for debugging purposes */
		uint16_t roundRNumber;			/**< is the round robin set number */
		uint8_t detType;				/**< is the detector type see :: detectorType */
		uint8_t version;				/**< is the version number of this structure format */
	} sls_detector_header;

#ifdef __cplusplus
#define MAX_NUM_PACKETS	512

	typedef std::bitset<MAX_NUM_PACKETS> sls_bitset;

	typedef struct {
		sls_detector_header detHeader;	/**< is the detector header */
		sls_bitset packetsMask;			/**< is the packets caught bit mask */
	} sls_receiver_header;

	typedef uint8_t bitset_storage[MAX_NUM_PACKETS/8];

#endif
	/**
	 * frameDiscardPolicy
	 */
	enum frameDiscardPolicy {
		GET_FRAME_DISCARD_POLICY = -1,	/**< to get the missing packet mode */
		NO_DISCARD,						/**< pad incomplete packets with -1, default mode */
		DISCARD_EMPTY_FRAMES,			/**< discard incomplete frames, fastest mode, save space, not suitable for multiple modules */
		DISCARD_PARTIAL_FRAMES,			/**< ignore missing packets, must check with packetsMask for data integrity, fast mode and suitable for multiple modules */
		NUM_DISCARD_POLICIES
	};


	/**
    format
     */
	enum fileFormat {
		GET_FILE_FORMAT=-1,/**< the receiver will return its file format */
		BINARY, /**< binary format */
		ASCII, /**< ascii format */
		HDF5, /**< hdf5 format */
		NUM_FILE_FORMATS
	};


	/**
	    @short structure for a region of interest
	    xmin,xmax,ymin,ymax define the limits of the region
	*/
	typedef struct {
	  int xmin;  /**< is the roi xmin (in channel number) */
	  int xmax;  /**< is the roi xmax  (in channel number)*/
	  int ymin;  /**< is the roi ymin  (in channel number)*/
	  int ymax;   /**< is the roi ymax  (in channel number)*/
	} ROI ;

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
	typedef struct {
	  int serialnumber;  /**< is the module serial number */
	  int nchan; /**< is the number of channels on the module*/
	  int nchip; /**< is the number of chips on the module */
	  int ndac; /**< is the number of dacs on the module */
	  int reg; /**< is the module register settings (gain level) */
	  int iodelay;	/**< iodelay */
	  int tau;	/**< tau */
	  int eV;	/**< threshold energy */
	  int *dacs; /**< is the pointer to the array of the dac values (in V) */
	  int *chanregs; /**< is the pointer to the array of the channel registers */
	} sls_detector_module;


	/**
	     network parameters
	*/

	enum networkParameter {
	  DETECTOR_MAC, 	    	/**< detector MAC */
	  DETECTOR_IP,	 	    	/**< detector IP */
	  RECEIVER_HOSTNAME,  		/**< receiver IP/hostname */
	  RECEIVER_UDP_IP,			/**< receiever UDP IP */
	  RECEIVER_UDP_PORT,		/**< receiever UDP Port */
	  RECEIVER_UDP_MAC,			/**< receiever UDP MAC */
	  RECEIVER_UDP_PORT2,		/**< receiever UDP Port of second half module for eiger */
	  DETECTOR_TXN_DELAY_LEFT, 	/**< transmission delay on the (left) port for next frame */
	  DETECTOR_TXN_DELAY_RIGHT,	/**< transmission delay on the right port for next frame  */
	  DETECTOR_TXN_DELAY_FRAME, /**< transmission delay of a whole frame for all the ports */
	  FLOW_CONTROL_10G,			/**< flow control for 10GbE */
	  FLOW_CONTROL_WR_PTR,		/**< memory write pointer for flow control */
	  FLOW_CONTROL_RD_PTR,		/**< memory read pointer for flow control */
	  RECEIVER_STREAMING_PORT,	/**< receiever streaming TCP(ZMQ) port */
	  CLIENT_STREAMING_PORT,	/**< client streaming TCP(ZMQ) port */
	  RECEIVER_STREAMING_SRC_IP,/**< receiever streaming TCP(ZMQ) ip */
	  CLIENT_STREAMING_SRC_IP,	/**< client streaming TCP(ZMQ) ip */
	  ADDITIONAL_JSON_HEADER,    /**< additional json header (ZMQ) */
	  RECEIVER_UDP_SCKT_BUF_SIZE, /**< UDP socket buffer size */
	  RECEIVER_REAL_UDP_SCKT_BUF_SIZE /**< real UDP socket buffer size */
	};

	/**
	    type of action performed (for text client)
	*/
	enum {GET_ACTION, PUT_ACTION, READOUT_ACTION, HELP_ACTION};

	/** online flags enum \sa setOnline*/
	enum {GET_ONLINE_FLAG=-1, /**< returns wether the detector is in online or offline state */
	      OFFLINE_FLAG=0, /**< detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!) */
	      ONLINE_FLAG =1/**< detector in online state (i.e. communication to the detector updating the local structure) */
	};

	/**
	     flags to get (or set) the size of the detector
	*/
	enum numberOf {
	  MAXMODX, /**<maximum number of module in X direction */
	  MAXMODY, /**<maximum number of module in Y direction */
	  NMODX, /**<installed number of module in X direction */
	  NMODY, /**<installed number of module in Y direction */
	  NCHANSX, /**<number of channels in X direction */
	  NCHANSY, /**<number of channels in Y direction */
	  NCHIPSX, /**<number of chips in X direction */
	  NCHIPSY /**<number of chips in Y direction */
	};

	/**
	    dimension indexes
	*/
	enum dimension {
	  X=0, /**< X dimension */
	  Y=1, /**< Y dimension */
	  Z=2 /**< Z dimension */
	};

	/**
	   enable/disable flags
	*/
	enum {
	  DISABLED, /**<flag disabled */
	  ENABLED /**<flag enabled */
	};


	/**
	  use of the external signals
	*/
	enum externalSignalFlag {
	  GET_EXTERNAL_SIGNAL_FLAG=-1,  /**<return flag for signal */
	  SIGNAL_OFF, /**<signal unused - tristate*/
	  GATE_IN_ACTIVE_HIGH,  /**<input gate active high*/
	  GATE_IN_ACTIVE_LOW,  /**<input gate active low */
	  TRIGGER_IN_RISING_EDGE,  /**<input exposure trigger on rising edge */
	  TRIGGER_IN_FALLING_EDGE, /**<input exposure trigger on falling edge */
	  RO_TRIGGER_IN_RISING_EDGE,  /**<input raedout trigger on rising edge */
	  RO_TRIGGER_IN_FALLING_EDGE, /**<input readout trigger on falling edge */
	  GATE_OUT_ACTIVE_HIGH, /**<output active high when detector is exposing*/
	  GATE_OUT_ACTIVE_LOW,  /**<output active low when detector is exposing*/
	  TRIGGER_OUT_RISING_EDGE,  /**<output trigger rising edge at start of exposure */
	  TRIGGER_OUT_FALLING_EDGE, /**<output trigger falling edge at start of exposure */
	  RO_TRIGGER_OUT_RISING_EDGE,   /**<output trigger rising edge at start of readout */
	  RO_TRIGGER_OUT_FALLING_EDGE,  /**<output trigger falling edge at start of readout */
	  OUTPUT_LOW, /**< output always low */
	  OUTPUT_HIGH, /**< output always high */
	  MASTER_SLAVE_SYNCHRONIZATION /**< reserved for master/slave synchronization in multi detector systems */
	};

	/**
	  communication mode using external signals
	*/
	enum externalCommunicationMode{
	  GET_EXTERNAL_COMMUNICATION_MODE=-1,/**<return flag for communication mode */
	  AUTO_TIMING, /**< internal timing */
	  TRIGGER_EXPOSURE, /**< trigger mode i.e. exposure is triggered */
	  TRIGGER_READOUT, /**< stop trigger mode i.e. readout is triggered by external signal */
	  GATE_FIX_NUMBER, /**< gated and reads out after a fixed number of gates */
	  GATE_WITH_START_TRIGGER, /**< gated with start trigger */
	  BURST_TRIGGER	/**< trigger a burst of frames */
	};
	/**
	    detector IDs/versions
	*/
	enum idMode{
	  DETECTOR_SERIAL_NUMBER,  /**<return detector system serial number */
	  DETECTOR_FIRMWARE_VERSION,  /**<return detector system firmware version */
	  DETECTOR_SOFTWARE_VERSION,   /**<return detector system software version */
	  THIS_SOFTWARE_VERSION,  /**<return this software version */
	  RECEIVER_VERSION,		 /**<return receiver software version */
	  SOFTWARE_FIRMWARE_API_VERSION,		/** return software firmware API version **/
	  CLIENT_SOFTWARE_API_VERSION,	/** return detector software and client api version */
	  CLIENT_RECEIVER_API_VERSION /** return client and  receiver api version */
	};
	/**
	    detector digital test modes
	*/
	enum digitalTestMode {
	  DETECTOR_FIRMWARE_TEST,  /**< test detector system firmware */
	  DETECTOR_BUS_TEST,  /**< test detector system CPU-FPGA bus */
	  DIGITAL_BIT_TEST			/**< gotthard digital bit test */
	};


	/**
	   detector dacs indexes
	*/
	enum dacIndex {
	  THRESHOLD,    /**< comparator threshold level */
	  CALIBRATION_PULSE,  /**< calibration input pulse height */
	  TRIMBIT_SIZE, /**< voltage to determine the trimbits LSB */
	  PREAMP,       /**< preamp feedback */
	  SHAPER1,      /**< shaper1 feedback */
	  SHAPER2,      /**< shaper2 feedback */
	  TEMPERATURE_ADC,   /**< temperature sensor (adc) */
	  TEMPERATURE_FPGA,  /**< temperature sensor (fpga) */
	  HUMIDITY,     /**< humidity sensor (adc) */
	  DETECTOR_BIAS,/**< detector bias */
	  VA_POT,       /**< power supply va */
	  VDD_POT,      /**< chiptest board power supply vdd */
	  VSH_POT,      /**< chiptest board power supply vsh */
	  VIO_POT,      /**< chiptest board power supply va */
	  G_VREF_DS,    /**< gotthard */
	  G_VCASCN_PB,  /**< gotthard */
	  G_VCASCP_PB,  /**< gotthard */
	  G_VOUT_CM,    /**< gotthard */
	  G_VCASC_OUT,  /**< gotthard */
	  G_VIN_CM,     /**< gotthard */
	  G_VREF_COMP,  /**< gotthard */
	  G_IB_TESTC,   /**< gotthard */
	  E_SvP,		/**< eiger */
	  E_SvN,		/**< eiger */
	  E_Vtr,		/**< eiger */
	  E_Vrf,		/**< eiger */
	  E_Vrs,		/**< eiger */
	  E_Vtgstv ,	/**< eiger */
	  E_Vcmp_ll,	/**< eiger */
	  E_Vcmp_lr,	/**< eiger */
	  E_cal,		/**< eiger */
	  E_Vcmp_rl,	/**< eiger */
	  E_Vcmp_rr,	/**< eiger */
	  E_rxb_rb ,	/**< eiger */
	  E_rxb_lb,		/**< eiger */
	  E_Vcp,		/**< eiger */
	  E_Vcn,		/**< eiger */
	  E_Vis,		/**< eiger */
	  IO_DELAY,		/**< eiger io delay */
	  ADC_VPP,		/**< adc vpp for jctb */
	  HIGH_VOLTAGE,             /**< high voltage */
	  TEMPERATURE_FPGAEXT,	/**< temperature sensor (close to fpga) */
	  TEMPERATURE_10GE,		/**< temperature sensor (close to 10GE) */
	  TEMPERATURE_DCDC,		/**< temperature sensor (close to DCDC) */
	  TEMPERATURE_SODL,		/**< temperature sensor (close to SODL) */
	  TEMPERATURE_SODR,		/**< temperature sensor (close to SODR) */
	  TEMPERATURE_FPGA2, /**< temperature sensor (fpga2 (eiger:febl) */
	  TEMPERATURE_FPGA3, /**< temperature sensor (fpga3 (eiger:febr) */
	  M_vIpre,      /**< mythen 3 >*/
	  M_vIbias,     /**< mythen 3 >*/
	  M_vIinSh,     /**< mythen 3 >*/
	  M_VdcSh,      /**< mythen 3 >*/
	  M_Vth2,       /**< mythen 3 >*/
	  M_VPL,        /**< mythen 3 >*/
	  M_Vth3,       /**< mythen 3 >*/
	  M_casSh,      /**< mythen 3 >*/
	  M_cas,        /**< mythen 3 >*/
	  M_vIbiasSh,   /**< mythen 3 >*/
	  M_vIcin,      /**< mythen 3 >*/
	  M_vIpreOut,    /**< mythen 3 >*/
	  V_POWER_A = 100, /**new chiptest board */
	  V_POWER_B = 101, /**new chiptest board */
	  V_POWER_C = 102, /**new chiptest board */
	  V_POWER_D = 103, /**new chiptest board */
	  V_POWER_IO =104, /**new chiptest board */
	  V_POWER_CHIP=105 ,/**new chiptest board */
	  I_POWER_A=106 , /**new chiptest board */
	  I_POWER_B=107 , /**new chiptest board */
	  I_POWER_C=108 , /**new chiptest board */
	  I_POWER_D=109 , /**new chiptest board */
	  I_POWER_IO=110 , /**new chiptest board */
	  V_LIMIT=111, /**new chiptest board */
	  SLOW_ADC0=1000,
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
	enum detectorSettings{
	  GET_SETTINGS=-1,  /**< return current detector settings */
	  STANDARD,         /**< standard settings */
	  FAST,             /**< fast settings */
	  HIGHGAIN,         /**< highgain  settings */
	  DYNAMICGAIN,      /**< dynamic gain  settings */
	  LOWGAIN,          /**< low gain  settings */
	  MEDIUMGAIN,       /**< medium gain  settings */
	  VERYHIGHGAIN,     /**< very high gain  settings */
	  LOWNOISE,	    	/**< low noise settings */
	  DYNAMICHG0,		/**< dynamic high gain 0 */
	  FIXGAIN1,			/**< fix gain 1 */
	  FIXGAIN2,			/**< fix gain 2 */
	  FORCESWITCHG1,	/**< force switch gain 1 */
	  FORCESWITCHG2,	/**< force switch gain 2 */
	  VERYLOWGAIN,		/**< very low gain settings */
	  UNDEFINED=200,    /**< undefined or custom  settings */
	  UNINITIALIZED     /**< uninitialiazed (status at startup) */
	};


	#define TRIMBITMASK 0x3f


	/**
	  important speed parameters
	*/
	enum speedVariable {
	  CLOCK_DIVIDER, /**< readout clock divider */
	  PHASE_SHIFT, /**< adds phase shift */
	  OVERSAMPLING, /**< oversampling for analog detectors */
	  ADC_CLOCK, /**< adc clock divider */
	  ADC_PHASE, /**< adc clock phase */
	  ADC_PIPELINE, /**< adc pipeline */
	  DBIT_CLOCK, /**< adc clock divider */
	  DBIT_PHASE, /**< adc clock phase */
	  DBIT_PIPELINE /**< adc pipeline */
	};


	/**
	   readout flags
	*/
	enum readOutFlags {
	  GET_READOUT_FLAGS=-1, /**< return readout flags */
	  NORMAL_READOUT=0, /**< no flag */
	  STORE_IN_RAM=0x1, /**< data are stored in ram and sent only after end of acquisition for faster frame rate */
	  READ_HITS=0x2, /**< return only the number of the channel which counted ate least one */
	  ZERO_COMPRESSION=0x4,/**< returned data are 0-compressed */
	  PUMP_PROBE_MODE=0x8,/**<pump-probe mode */
	  BACKGROUND_CORRECTIONS=0x1000, /**<background corrections */
	  TOT_MODE=0x2000,/**< pump-probe mode */
	  CONTINOUS_RO=0x4000,/**< pump-probe mode */
	  PARALLEL=0x10000,/**< eiger parallel mode */
	  NONPARALLEL=0x20000,/**< eiger serial mode */
	  SAFE=0x40000/**< eiger safe mode */,
	  DIGITAL_ONLY=0x80000, /** chiptest board read only digital bits (not adc values)*/
	  ANALOG_AND_DIGITAL=0x100000, /** chiptest board read adc values and digital bits digital bits */
	  DUT_CLK=0x200000, /** chiptest board fifo clock comes from device under test */
	  SHOW_OVERFLOW=0x400000, /** eiger 32 bit mode, show saturated for overflow of single subframes */
	  NOOVERFLOW=0x800000 /** eiger 32 bit mode, do not show saturated for overflow of single subframes */
	};


	/** port type */
	enum portType {
	  CONTROL_PORT, /**< control port */
	  STOP_PORT, /**<stop port */
	  DATA_PORT /**< receiver tcp port with client*/
	};

	/** hierarchy in multi-detector structure, if any */
	enum masterFlags {
	  GET_MASTER=-1, /**< return master flag */
	  NO_MASTER, /**< no master/slave hierarchy defined */
	  IS_MASTER, /**<is master */
	  IS_SLAVE /**< is slave */
	};


	enum imageType {
	  DARK_IMAGE,   /**< dark image */
	  GAIN_IMAGE   /**< gain image */
	};


	/**
	 * frame mode for processor
	 */
	enum frameModeType {
	    GET_FRAME_MODE = -1,
	    PEDESTAL,       /** < pedestal */
	    NEW_PEDESTAL,   /** < new pedestal */
	    FLATFIELD,      /** < flatfield */
	    NEW_FLATFIELD   /** < new flatfield */
	};

    /**
     * detector mode for processor
     */
    enum detectorModeType {
        GET_DETECTOR_MODE = -1,
        COUNTING,       /** < counting */
        INTERPOLATING,  /** < interpolating */
        ANALOG          /** < analog */
    };


#ifdef __cplusplus
	  /** returns string from enabled/disabled
	      \param b true or false
	      \returns string enabled, disabled
	  */
	  static std::string stringEnable(bool b){\
		if(b) return std::string("enabled"); \
		else return std::string("disabled"); \
	  };

	  /** returns detector type string from detector type index
	      \param t string can be EIGER, GOTTHARD, JUNGFRAU, CHIPTESTBOARD
	      \returns Eiger, Gotthard, Jungfrau, JungfrauCTB, Unknown
	  */
	  static std::string detectorTypeToString(detectorType t){	\
	    switch (t) {										\
	    case EIGER:    		return std::string("Eiger");	\
	    case GOTTHARD:    	return std::string("Gotthard");	\
	    case JUNGFRAU:    	return std::string("Jungfrau");	\
	    case CHIPTESTBOARD:   return std::string("JungfrauCTB");	\
	    default:    		return std::string("Unknown");	\
	    }};

	  /** returns detector type index from detector type string
	      \param type can be Eiger, Gotthard, Jungfrau, JungfrauCTB
	      \returns  EIGER, GOTTHARD, JUNGFRAU, CHIPTESTBOARD, GENERIC
	  */
	  static detectorType detectorTypeToEnum(const std::string& type){\
	    if (type=="Eiger")    		return EIGER;		\
	    if (type=="Gotthard")    	return GOTTHARD;	\
	    if (type=="Jungfrau")    	return JUNGFRAU;	\
	    if (type=="JungfrauCTB") 	return CHIPTESTBOARD;	\
	    							return GENERIC;		\
	  };


	  /** returns string from run status index
	      \param s can be ERROR, WAITING, RUNNING, TRANSMITTING, RUN_FINISHED, STOPPED
	      \returns string error, waiting, running, data, finished, stopped, idle
	  */
	  static std::string runStatusType(runStatus s){\
	    switch (s) {				\
	    case ERROR:       	return std::string("error");	\
	    case WAITING:      	return std::string("waiting");	\
	    case RUNNING:      	return std::string("running");	\
	    case TRANSMITTING:  return std::string("data");		\
	    case RUN_FINISHED: 	return std::string("finished");	\
	    case STOPPED: 		return std::string("stopped");	\
	    default:       		return std::string("idle");		\
	    }};


	  /** returns string from file format index
	      \param s can be BINARY, ASCII, HDF5
	      \returns string binary, ascii, hdf5
	  */
	  static std::string getFileFormatType(fileFormat f){\
	    switch (f) {				\
	    case ASCII:     return std::string("ascii");	\
	    case HDF5:      return std::string("hdf5");	\
	    case BINARY:    return std::string("binary");	\
	    default:       		return std::string("unknown");		\
	    }};

	  /**
	   * Returns string of frame discard policy index
	   * @param f can be NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES
	   * @returns No Discard, Discard Empty Frames, Discard Partial Frames, unknown
	   */
	  static std::string getFrameDiscardPolicyType(frameDiscardPolicy f) {						\
		  switch (f) {																		\
		  case NO_DISCARD: 				return std::string("No Discard");					\
		  case DISCARD_EMPTY_FRAMES: 	return std::string("Discard Empty Frames");			\
		  case DISCARD_PARTIAL_FRAMES: 	return std::string("Discard Partial Frames");		\
		  default:       				return std::string("unknown");						\
		  }};																				\



		  /** returns std::string from external signal type index
		      \param f can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, =TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE, OUTPUT_LOW, OUTPUT_HIGH, MASTER_SLAVE_SYNCHRONIZATION,  GET_EXTERNAL_SIGNAL_FLAG
		      \returns std::string  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, gnd, vcc, sync, unknown
		  */
		  static std::string externalSignalType(externalSignalFlag f){\
		    switch(f) {						 \
		    case SIGNAL_OFF:      return std::string( "off");			\
		    case GATE_IN_ACTIVE_HIGH:    return std::string( "gate_in_active_high");	\
		    case GATE_IN_ACTIVE_LOW:    return std::string( "gate_in_active_low");	\
		    case TRIGGER_IN_RISING_EDGE:    return std::string( "trigger_in_rising_edge"); \
		    case TRIGGER_IN_FALLING_EDGE:    return std::string( "trigger_in_falling_edge");	\
		    case RO_TRIGGER_IN_RISING_EDGE:    return std::string( "ro_trigger_in_rising_edge"); \
		    case RO_TRIGGER_IN_FALLING_EDGE:    return std::string( "ro_trigger_in_falling_edge"); \
		    case GATE_OUT_ACTIVE_HIGH:    return std::string( "gate_out_active_high"); \
		    case GATE_OUT_ACTIVE_LOW:    return std::string( "gate_out_active_low");	\
		    case TRIGGER_OUT_RISING_EDGE:    return std::string( "trigger_out_rising_edge");	\
		    case TRIGGER_OUT_FALLING_EDGE:    return std::string( "trigger_out_falling_edge"); \
		    case RO_TRIGGER_OUT_RISING_EDGE:      return std::string( "ro_trigger_out_rising_edge");	\
		    case RO_TRIGGER_OUT_FALLING_EDGE:    return std::string( "ro_trigger_out_falling_edge");	\
		    case MASTER_SLAVE_SYNCHRONIZATION: return std::string("sync");		\
		    case OUTPUT_LOW: return std::string("gnd");		\
		    case OUTPUT_HIGH: return std::string("vcc");		\
		    default:    return std::string( "unknown");				\
		    }    };




		  /** returns external signal type index from std::string
		      \param sval  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, gnd, vcc, sync, unknown
		      \returns can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE, OUTPUT_LOW, OUTPUT_HIGH, MASTER_SLAVE_SYNCHRONIZATION,  GET_EXTERNAL_SIGNAL_FLAG (if unknown)
		  */

		  static externalSignalFlag externalSignalType(std::string sval){\
		    if (sval=="off")      return SIGNAL_OFF;\
		    if (sval=="gate_in_active_high")      return GATE_IN_ACTIVE_HIGH;	\
		    if  (sval=="gate_in_active_low") return GATE_IN_ACTIVE_LOW;\
		    if  (sval=="trigger_in_rising_edge")  return TRIGGER_IN_RISING_EDGE;\
		    if  (sval=="trigger_in_falling_edge") return TRIGGER_IN_FALLING_EDGE;\
		    if  (sval=="ro_trigger_in_rising_edge") return RO_TRIGGER_IN_RISING_EDGE;\
		    if  (sval=="ro_trigger_in_falling_edge") return RO_TRIGGER_IN_FALLING_EDGE;\
		    if (sval=="gate_out_active_high")      return GATE_OUT_ACTIVE_HIGH;\
		    if  (sval=="gate_out_active_low") return GATE_OUT_ACTIVE_LOW;\
		    if  (sval=="trigger_out_rising_edge") return TRIGGER_OUT_RISING_EDGE;\
		    if  (sval=="trigger_out_falling_edge") return TRIGGER_OUT_FALLING_EDGE;\
		    if  (sval=="ro_trigger_out_rising_edge") return RO_TRIGGER_OUT_RISING_EDGE;\
		    if  (sval=="ro_trigger_out_falling_edge") return RO_TRIGGER_OUT_FALLING_EDGE;\
		    if  (sval=="sync") return MASTER_SLAVE_SYNCHRONIZATION;\
		    if  (sval=="gnd") return OUTPUT_LOW;\
		    if  (sval=="vcc") return OUTPUT_HIGH;\
		    return GET_EXTERNAL_SIGNAL_FLAG ;};

		  /** returns detector settings std::string from index
		      \param s can be STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN, LOWNOISE,
		       DYNAMICHG0, FIXGAIN1, FIXGAIN2, FORCESWITCHG1, FORCESWITCHG2, GET_SETTINGS
		      \returns standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, lownoise,
		      dynamichg0, fixgain1, fixgain2, forceswitchg1, forceswitchg2, verylowgain, undefined
		  */
		  static std::string getDetectorSettings(detectorSettings s){\
		    switch(s) {											\
		    case STANDARD:      return std::string("standard");		\
		    case FAST:      	return std::string("fast");			\
		    case HIGHGAIN:      return std::string("highgain");		\
		    case DYNAMICGAIN:   return std::string("dynamicgain");	\
		    case LOWGAIN:    	return std::string("lowgain");		\
		    case MEDIUMGAIN:    return std::string("mediumgain");	\
		    case VERYHIGHGAIN:  return std::string("veryhighgain");	\
		    case LOWNOISE:      return  std::string("lownoise");		\
		    case DYNAMICHG0:    return  std::string("dynamichg0");	\
		    case FIXGAIN1:      return  std::string("fixgain1");		\
		    case FIXGAIN2:      return  std::string("fixgain2");		\
		    case FORCESWITCHG1: return  std::string("forceswitchg1");\
		    case FORCESWITCHG2: return  std::string("forceswitchg2");\
		    case VERYLOWGAIN: return  std::string("verylowgain");\
		    default:    		return std::string("undefined");		\
		    }};

		  /** returns detector settings std::string from index
		      \param s can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, lownoise,
		      dynamichg0, fixgain1, fixgain2, forceswitchg1, forceswitchg2, undefined
		      \returns   setting index STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN,LOWNOISE,
		      DYNAMICHG0, FIXGAIN1, FIXGAIN2, FORCESWITCHG1, FORCESWITCHG2, VERYLOWGAIN, GET_SETTINGS
		  */

		  static detectorSettings getDetectorSettings(std::string s){	\
		    if (s=="standard") 		return STANDARD;				\
		    if (s=="fast") 			return FAST;					\
		    if (s=="highgain") 		return HIGHGAIN;				\
		    if (s=="dynamicgain") 	return DYNAMICGAIN;				\
		    if (s=="lowgain") 		return LOWGAIN;					\
		    if (s=="mediumgain") 	return MEDIUMGAIN;				\
		    if (s=="veryhighgain") 	return VERYHIGHGAIN;			\
		    if (s=="lownoise") 		return LOWNOISE;				\
		    if (s=="dynamichg0") 	return DYNAMICHG0;				\
		    if (s=="fixgain1") 		return FIXGAIN1;				\
		    if (s=="fixgain2") 		return FIXGAIN2;				\
		    if (s=="forceswitchg1") return FORCESWITCHG1;			\
		    if (s=="forceswitchg2")	return FORCESWITCHG2;			\
		    if (s=="verylowgain")	return VERYLOWGAIN;				\
		    return GET_SETTINGS;									\
		  };


		  /**
		     returns external communication mode std::string from index
		     \param f can be AUTO_TIMING, TRIGGER_EXPOSURE, TRIGGER_READOUT, GATE_FIX_NUMBER, GATE_WITH_START_TRIGGER, BURST_TRIGGER, GET_EXTERNAL_COMMUNICATION_MODE
		     \returns  auto, trigger, ro_trigger, gating, triggered_gating, unknown
		  */

		  static std::string externalCommunicationType(externalCommunicationMode f){	\
		    switch(f) {						 \
		    case AUTO_TIMING:      return std::string( "auto");			\
		    case TRIGGER_EXPOSURE: return std::string("trigger");			\
		    case TRIGGER_READOUT: return std::string("ro_trigger");			\
		    case GATE_FIX_NUMBER: return std::string("gating");			\
		    case GATE_WITH_START_TRIGGER: return std::string("triggered_gating");	\
		    case BURST_TRIGGER: return std::string("burst_trigger");	\
		    default:    return std::string( "unknown");				\
		    }    };



		  /**
		     returns external communication mode index from std::string
		     \param sval can be auto, trigger, ro_trigger, gating, triggered_gating
		     \returns AUTO_TIMING, TRIGGER_EXPOSURE, TRIGGER_READOUT, GATE_FIX_NUMBER, GATE_WITH_START_TRIGGER, BURST_TRIGGER, GET_EXTERNAL_COMMUNICATION_MODE
		  */

		  static externalCommunicationMode externalCommunicationType(std::string sval){\
		    if (sval=="auto")      return AUTO_TIMING;\
		    if (sval=="trigger")     return TRIGGER_EXPOSURE;	\
		    if  (sval=="ro_trigger") return TRIGGER_READOUT;\
		    if  (sval=="gating") return GATE_FIX_NUMBER;\
		    if  (sval=="triggered_gating") return GATE_WITH_START_TRIGGER;\
		    if  (sval=="burst_trigger") return BURST_TRIGGER;\
		    return GET_EXTERNAL_COMMUNICATION_MODE;			\
		  };

		  /** returns std::string from file format index
		      \param s can be RAW, HDF5
		      \returns std::string raw, hdf5
		  */
		  static std::string fileFormats(fileFormat f){\
		    switch (f) {				\
		    case BINARY:       return std::string("binary");		\
		    case ASCII:       return std::string("ascii");		\
		    case HDF5:      return  std::string("hdf5");	\
		    default:       return std::string("unknown");		\
		    }};

		  /** returns std::string from timer index
		      \param s can be FRAME_NUMBER,ACQUISITION_TIME,FRAME_PERIOD, DELAY_AFTER_TRIGGER,GATES_NUMBER, CYCLES_NUMBER, ACTUAL_TIME,MEASUREMENT_TIME, PROGRESS,MEASUREMENTS_NUMBER,FRAMES_FROM_START,FRAMES_FROM_START_PG,SAMPLES,SUBFRAME_ACQUISITION_TIME,STORAGE_CELL_NUMBER, SUBFRAME_DEADTIME
		      \returns std::string frame_number,acquisition_time,frame_period, delay_after_trigger,gates_number, cycles_number, actual_time,measurement_time, progress,measurements_number,frames_from_start,frames_from_start_pg,samples,subframe_acquisition_time,storage_cell_number, SUBFRAME_DEADTIME
		  */
		  static std::string getTimerType(timerIndex t){										\
		    switch (t) {																\
		    case FRAME_NUMBER: 				return std::string("frame_number"); 				\
		    case ACQUISITION_TIME: 			return std::string("acquisition_time"); 			\
		    case FRAME_PERIOD: 				return std::string("frame_period"); 				\
		    case DELAY_AFTER_TRIGGER: 		return std::string("delay_after_trigger"); 		\
		    case GATES_NUMBER: 				return std::string("gates_number"); 				\
		    case CYCLES_NUMBER: 			return std::string("cycles_number"); 			\
		    case ACTUAL_TIME: 				return std::string("actual_time"); 				\
		    case MEASUREMENT_TIME: 			return std::string("measurement_time"); 			\
		    case PROGRESS: 					return std::string("progress"); 					\
		    case MEASUREMENTS_NUMBER: 		return std::string("measurements_number"); 		\
		    case FRAMES_FROM_START: 		return std::string("frames_from_start"); 		\
		    case FRAMES_FROM_START_PG: 		return std::string("frames_from_start_pg"); 		\
		    case SAMPLES: 				return std::string("samples"); 				\
		    case SUBFRAME_ACQUISITION_TIME:	return std::string("subframe_acquisition_time");	\
		    case SUBFRAME_DEADTIME:			return std::string("subframe_deadtime");			\
		    case STORAGE_CELL_NUMBER:       return std::string("storage_cell_number");       \
		    default:       					return std::string("unknown");					\
		    }};


		  /**
		     @short returns adc index from std::string
		     \param s can be temp_fpga, temp_fpgaext, temp_10ge, temp_dcdc, temp_sodl, temp_sodr, temp_fpgafl, temp_fpgafr
		     \returns  TEMPERATURE_FPGA, TEMPERATURE_FPGAEXT, TEMPERATURE_10GE, TEMPERATURE_DCDC, TEMPERATURE_SODL,
		     TEMPERATURE_SODR, TEMPERATURE_FPGA2, TEMPERATURE_FPGA3, -1 when unknown mode
		  */
		  static int getADCIndex(std::string s){					\
			  if (s=="temp_fpga")	  	return TEMPERATURE_FPGA;	\
			  if (s=="temp_fpgaext")	return TEMPERATURE_FPGAEXT;	\
			  if (s=="temp_10ge")	  	return TEMPERATURE_10GE;	\
			  if (s=="temp_dcdc")	  	return TEMPERATURE_DCDC;	\
			  if (s=="temp_sodl")	  	return TEMPERATURE_SODL;	\
			  if (s=="temp_sodr")	  	return TEMPERATURE_SODR;	\
			  if (s=="temp_fpgafl")		return TEMPERATURE_FPGA2;	\
			  if (s=="temp_fpgafr")		return TEMPERATURE_FPGA3;	\
			  return -1;											\
		  };														\


		  /**
		     @short returns dac index from std::string
		     \param s can be vcmp_ll, vcmp_lr, vcmp_rl, vcmp_rr, vthreshold, vrf, vrs, vtr, vcall, vcp
		     \returns E_Vcmp_ll, E_Vcmp_lr, E_Vcmp_rl, E_Vcmp_rr, THRESHOLD, E_Vrf, E_Vrs, E_Vtr, E_cal, E_Vcp , -1 when unknown mode
		  */
		  static int getDACIndex(std::string s){		\
			  if (s=="vcmp_ll")	  	return E_Vcmp_ll;	\
			  if (s=="vcmp_lr")	  	return E_Vcmp_lr;	\
			  if (s=="vcmp_rl")		return E_Vcmp_rl;	\
			  if (s=="vcmp_rr")	  	return E_Vcmp_rr;	\
			  if (s=="vthreshold")	return THRESHOLD;	\
			  if (s=="vrf")	  		return E_Vrf;		\
			  if (s=="vrs")	  		return E_Vrs;		\
			  if (s=="vtr")			return E_Vtr;		\
			  if (s=="vcall")		return E_cal;		\
			  if (s=="vcp")			return E_Vcp;		\
			  return -1;								\
		  };											\

		  /**
		     @short returns receiver frame discard policy from std::string
		     \param s can be nodiscard, discardempty, discardpartial
		     \returns NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES, GET_FRAME_DISCARD_POLICY when unknown mode
		  */
		  static frameDiscardPolicy getReceiverFrameDiscardPolicy(std::string s){		\
			  if (s=="nodiscard")	  	return NO_DISCARD;				\
			  if (s=="discardempty")	return DISCARD_EMPTY_FRAMES;	\
			  if (s=="discardpartial")	return DISCARD_PARTIAL_FRAMES;	\
			  return GET_FRAME_DISCARD_POLICY;							\
		  };															\

		  /** returns std::string from frame discard policy
		      \param f can be NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES
		      \returns std::string nodiscard, discardempty, discardpartial, unknown
		  */
		  static std::string getReceiverFrameDiscardPolicy(frameDiscardPolicy f){	\
		    switch (f) {															\
		    case NO_DISCARD: 				return std::string("nodiscard"); 		\
		    case DISCARD_EMPTY_FRAMES: 		return std::string("discardempty"); 	\
		    case DISCARD_PARTIAL_FRAMES: 	return std::string("discardpartial"); 	\
		    default:       					return std::string("unknown");			\
		    }};																		\


		    /**
		     * returns frameModeType as enum
		     * @param s pedestal, newpedestal, flatfield, newflatfield
		     * @returns PEDESTAL, NEW_PEDESTAL, FLATFIELD, NEW_FLATFIELD, GET_FRAME_MODE (if unknown)
		     */
		    static frameModeType getFrameModeType(std::string s) {  \
		        for (auto &c: s)                                    \
		            c = std::tolower(c);                           \
		        if (s == "pedestal")    return PEDESTAL;            \
		        if (s == "newpedestal") return NEW_PEDESTAL;        \
		        if (s == "flatfield")   return FLATFIELD;           \
		        if (s == "newflatfield")return NEW_FLATFIELD;       \
		        return GET_FRAME_MODE;                              \
		    }                                                       \

            /**
             * returns frameModeType as string
             * @param f PEDESTAL, NEW_PEDESTAL, FLATFIELD, NEW_FLATFIELD
             * @return string pedestal, newpedestal, flatfield, newflatfield, unknown
             */
		    static std::string getFrameModeType(frameModeType f) {      \
		        switch(f) {                                             \
		        case PEDESTAL:      return std::string("pedestal");     \
		        case NEW_PEDESTAL:  return std::string("newPedestal");  \
		        case FLATFIELD:     return std::string("flatfield");    \
		        case NEW_FLATFIELD: return std::string("newFlatfield"); \
		        default:            return std::string("unknown");      \
		        }                                                       \
		    }                                                           \

            /**
             * returns detectorModeType as enum
             * @param s counting, interpolating, analog
             * @returns COUNTING, INTERPOLATING, ANALOG, GET_DETECTOR_MODE (if unknown)
             */
            static detectorModeType getDetectorModeType(std::string s) {    \
                for (auto &c: s)                                            \
                    c = std::tolower(c);                                   \
                if (s == "counting")        return COUNTING;                \
                if (s == "interpolating")   return INTERPOLATING;           \
                if (s == "analog")          return ANALOG;                  \
                return GET_DETECTOR_MODE;                                   \
            }                                                               \

            /**
             * returns frameModeType as string
             * @param f COUNTING, INTERPOLATING, ANALOG
             * @return string counting, interpolating, analog, unknown
             */
            static std::string getDetectorModeType(detectorModeType f) {    \
                switch(f) {                                                 \
                case COUNTING:      return std::string("counting");         \
                case INTERPOLATING: return std::string("interpolating");    \
                case ANALOG:        return std::string("analog");           \
                default:            return std::string("unknown");          \
                }                                                           \
            }                                                               \

#endif

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
