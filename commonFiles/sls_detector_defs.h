#ifndef SLS_DETECTOR_DEFS_H
#define SLS_DETECTOR_DEFS_H


#ifdef __CINT__
#define MYROOT
#define __cplusplus
#endif

//#include <stdint.h>
#include "sls_receiver_defs.h"


/** maximum rois */
#define MAX_ROIS 100

/** maximum trim en */
#define MAX_TRIMEN 100

/** maximum unit size of program sent to detector */
#define MAX_FPGAPROGRAMSIZE (2 * 1024 *1024)

typedef char mystring[MAX_STR_LENGTH];

typedef int dacs_t;


#define DEFAULT_DET_MAC         "00:aa:bb:cc:dd:ee"
#define DEFAULT_DET_IP          "129.129.202.46"


/** 
    \file sls_detector_defs.h
This file contains all the basic definitions common to the slsDetector class 
and to the server programs running on the detector


 * @author Anna Bergamaschi
 * @version 0.1alpha (any string)
 * @see slsDetector

*/


/** get flag form most functions */
#define GET_FLAG -1


#ifdef __cplusplus

/** @short class containing all the structures, constants and enum definitions */
class slsDetectorDefs: public virtual slsReceiverDefs{
 public:

  slsDetectorDefs(){};
  
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
typedef struct {
  int serialnumber;  /**< is the module serial number */
  int nchan; /**< is the number of channels on the module*/
  int nchip; /**< is the number of chips on the module */
  int ndac; /**< is the number of dacs on the module */
  int nadc; /**< is the number of adcs on the module */
  int reg; /**< is the module register (e.g. dynamic range?)
	      \see moduleRegisterBit */
  dacs_t *dacs; /**< is the pointer to the array of the dac values (in V) */
  dacs_t *adcs;  /**< is the pointer to the array of the adc values (in V) FLAT_FIELD_CORRECTION*/
  int *chipregs; /**< is the pointer to the array of the chip registers
		    \see ::chipRegisterBit */
  int *chanregs; /**< is the pointer to the array of the channel registers
		    \see ::channelRegisterBit */
  double gain;  /**< is the module gain (V/keV) */
  double offset;  /**< is the module offset (V) */
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
  MODULE_SERIAL_NUMBER, /**<return module serial number */
  MODULE_FIRMWARE_VERSION,  /**<return module firmware */
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
  CHIP_TEST, /**< test chips */
  MODULE_FIRMWARE_TEST,  /**< test module firmware */
  DETECTOR_FIRMWARE_TEST,  /**< test detector system firmware */
  DETECTOR_MEMORY_TEST,  /**< test detector system memory */
  DETECTOR_SOFTWARE_TEST,  /**< test detector system software */
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
  HV_POT,       /**< gotthard, chiptest board high voltage */
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
  HV_NEW,             /**< new hv index for jungfrau & c */
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
  V_LIMIT=111 /**new chiptest board */
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


//#if defined(__cplusplus) && !defined(EIGERD)
#ifdef __cplusplus
 protected:
#endif



#ifndef MYROOT
#include "sls_detector_funcs.h"
//#include "sls_receiver_funcs.h"
#endif


//#if defined(__cplusplus) && !defined(EIGERD)
#ifdef __cplusplus
};
#endif
;
#endif
;
