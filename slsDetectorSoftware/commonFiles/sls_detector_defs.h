#ifndef SLS_DETECTOR_DEFS_H
#define SLS_DETECTOR_DEFS_H

#include <stdint.h> 

/** default maximum string length */
#define MAX_STR_LENGTH 1000
/** default maximum string length */
#define MAX_SCAN_STEPS 2000


typedef float float32_t;
typedef int int32_t;   
typedef char mystring[MAX_STR_LENGTH];
typedef float mysteps[MAX_SCAN_STEPS]; 

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

enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS};



/** 
    structure for a detector channel
    should not be used by unexperienced users

    \see  ::channelRegisterBit
*/
typedef struct {
  int chan; /**< is the  channel number */
  int chip; /**< is the  chip number */
  int module; /**< is the  module number */
  int64_t reg;   /**< is the is the channel register (e.g. trimbits, calibration enable, comparator enable...) */
} sls_detector_channel;

/** 
    structure for a detector chip
    should not be used by unexperienced users
    \see  ::chipRegisterBit ::channelRegisterBit
*/
typedef struct  {
  int chip;   /**< is the chip number */
  int module;  /**< is the module number */
  int nchan; /**< is the number of channels in the chip */
  int reg; /**<is the chip register (e.g. output analogue buffer enable) 
	      \see ::chipRegisterBit */
  int *chanregs; /**< is the pointer to the array of the channel registers 
		    \see ::channelRegisterBit */
} sls_detector_chip;

/** 
    structure for a detector module
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
  int module; /**< is the module number */
  int serialnumber;  /**< is the module serial number */
  int nchan; /**< is the number of channels per chip */
  int nchip; /**< is the number of chips on the module */
  int ndac; /**< is the number of dacs on the module */
  int nadc; /**< is the number of adcs on the module */
  int reg; /**< is the module register (e.g. dynamic range?) 
	      \see moduleRegisterBit */
  float *dacs; /**< is the pointer to the array of the dac values (in V) */
  float *adcs;  /**< is the pointer to the array of the adc values (in V) */
  int *chipregs; /**< is the pointer to the array of the chip registers 
		    \see ::chipRegisterBit */
  int *chanregs; /**< is the pointer to the array of the channel registers 
		    \see ::channelRegisterBit */
  float gain;  /**< is the module gain (V/keV) */
  float offset;  /**< is the module offset (V) */
} sls_detector_module;

/** 
    structure for a region of interest
    
    xmin,xmax,ymin,ymax define the limits of the region
*/
typedef struct {
  int xmin;  /**< is the roi xmin (in channel number) */
  int xmax;  /**< is the roi xmax  (in channel number)*/
  int ymin;  /**< is the roi ymin  (in channel number)*/
  int ymax;   /**< is the roi ymax  (in channel number)*/
} ROI ;

/** 
    structure for a generic integer array
   
*/
typedef struct {
  int len;  /**< is the number of elements of the array */
  int *iptr; /**<  is the pointer to the array */
} iarray ;


/** 
     Type of the detector
*/
enum detectorType {
  GET_DETECTOR_TYPE=-1,   /**< the detector will return its type */
  GENERIC,  /**< generic sls detector */
  MYTHEN, /**< mythen */
  PILATUS, /**< pilatus */
  EIGER, /**< eiger */
  GOTTHARD, /**< gotthard */
  PICASSO, /**< picasso */
  AGIPD /**< agipd */
};

/** 
     Communication protocol (normally TCP)
*/
enum communicationProtocol{
  TCP,  /**< TCP/IP */
  UDP /**< UDP */
};

/** 
     Communication protocol (normally TCP)
*/
enum networkParameter {
  CLIENT_IP,  /**< client IP */
  CLIENT_MAC, /**< client mac */
  SERVER_MAC /**< server MAC */
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
    return values
*/
enum  {
  OK, /**< function succeeded */
  FAIL, /**< function failed */
  FINISHED, /**< acquisition finished */
  FORCE_UPDATE
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
  TRIGGER_COINCIDENCE_WITH_INTERNAL_ENABLE, /**< still to be implemented */
  GATE_FIX_NUMBER, /**< gated and reads out after a fixxed number of gates */
  GATE_FIX_DURATION, /**< still to be implemented */
  GATE_WITH_START_TRIGGER, /**< gated with start trigger */
  GATE_COINCIDENCE_WITH_INTERNAL_ENABLE /**< still to be implemented */
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
  THIS_SOFTWARE_VERSION  /**<return this software version */
};
/** 
    detector digital test modes
*/
enum digitalTestMode {
  CHIP_TEST, /**< test chips */
  MODULE_FIRMWARE_TEST,  /**< test module firmware */
  DETECTOR_FIRMWARE_TEST,  /**< test detector system firmware */
  DETECTOR_MEMORY_TEST,  /**< test detector system memory */
  DETECTOR_BUS_TEST,  /**< test detector system CPU-FPGA bus */
  DETECTOR_SOFTWARE_TEST,  /**< test detector system software */
  DIGITAL_BIT_TEST			/**< gotthard digital bit test */
};
/** 
    detector analogue test modes
*/
enum analogTestMode {
  CALIBRATION_PULSES,  /**< test using calibration pulses */
  MY_ANALOG_TEST_MODE  /**< other possible test modes */
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
  G_IB_TESTC   /**< gotthard */
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
  UNDEFINED,        /**< undefined or custom  settings */
  UNINITIALIZED     /**< uninitialiazed (status at startup) */
};
/** 
   meaning of the channel register bits
   \see ::sls_detector_channel
*/
enum channelRegisterBit {
  TRIMBIT_OFF=0,  /**< offset of trimbit value in the channel register  */
  COMPARATOR_ENABLE=0x100,  /**< mask of the comparator enable bit  */
  ANALOG_SIGNAL_ENABLE=0x200, /**< mask of the analogue output enable bit  */
  CALIBRATION_ENABLE=0x300, /**< mask of the calibration input enable bit  */
};
/** 
   meaning of the chip register bits
   \see ::sls_detector_chip
*/
enum chipRegisterBit {
  ENABLE_ANALOG_OUTPUT=0x1, /**< mask of the analogue output enable bit  */
  CHIP_OUTPUT_WIDTH=0x2 /**< mask of the chip output width  */
};
/** 
   meaning of the module register bits
*/
enum moduleRegisterBit {
  MY_MODULE_REGISTER_BIT,  /**< possible module register bit meaning  */
  MODULE_OUTPUT_WIDTH   /**< possibly module dynamic range  */
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
  PROBES_NUMBER, /**< number of probe types in pump-probe mode */
  CYCLES_NUMBER, /**< number of cycles: total number of acquisitions is number or frames*number of cycles */
  ACTUAL_TIME, /**< Actual time of the detector's internal timer */
  MEASUREMENT_TIME,  /**< Time of the measurement from the detector (fifo) */

  PROGRESS /**< fraction of measurement elapsed - only get! */
};

/** 
  important speed parameters
*/
enum speedVariable {
  CLOCK_DIVIDER, /**< readout clock divider */
  WAIT_STATES, /**< wait states for bus read */
  TOT_CLOCK_DIVIDER, /**< wait states for bus read */
  TOT_DUTY_CYCLE, /**< wait states for bus read */
  SET_SIGNAL_LENGTH /**< set/clear signal length */
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
  RUNNING /**< acquisition  running, no data in memory */
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
  TOT_MODE=0x2000,/**<pump-probe mode */
  CONTINOUS_RO=0x4000/**<pump-probe mode */
};
/** 
   trimming modes
*/
enum trimMode {
  NOISE_TRIMMING, /**< trim with noise */
  BEAM_TRIMMING, /**< trim with x-rays (on all 63 bits) */
  IMPROVE_TRIMMING, /**< trim with x-rays (on a limited range of bits - should start from an already trimmed mode) */
  FIXEDSETTINGS_TRIMMING,/**< trim without optimizing the threshold and the trimbit size */
  OFFLINE_TRIMMING /**< trimming is performed offline */
};
/** 
   data correction flags
*/
enum correctionFlags {
  DISCARD_BAD_CHANNELS, /**< bad channels are discarded */
  AVERAGE_NEIGHBOURS_FOR_BAD_CHANNELS,  /**< bad channels are replaced with the avergae of the neighbours */
  FLAT_FIELD_CORRECTION,  /**< data are flat field corrected */
  RATE_CORRECTION,  /**< data are rate corrected */
  ANGULAR_CONVERSION,/**< angular conversion is calculated */
  I0_NORMALIZATION
};
/** port type */
enum portType {
  CONTROL_PORT, /**< control port */ 
  STOP_PORT, /**<stop port */
  DATA_PORT /**< data port */
}; 

/** hierarchy in multi-detector structure, if any */
enum masterFlags {
  GET_MASTER=-1, /**< return master flag */
  NO_MASTER, /**< no master/slave hierarchy defined */
  IS_MASTER, /**<is master */
  IS_SLAVE /**< is slave */
}; 

/** synchronization in a multidetector structure, if any */
enum synchronizationMode {
  GET_SYNCHRONIZATION_MODE=-1, /**< the multidetector will return its synchronization mode */
  NO_SYNCHRONIZATION, /**< all detectors are independent (no cabling) */
  MASTER_GATES, /**< the master gates the other detectors */
  MASTER_TRIGGERS, /**< the master triggers the other detectors */
  SLAVE_STARTS_WHEN_MASTER_STOPS /**< the slave acquires when the master finishes, to avoid deadtime */
};

enum imageType {
  DARK_IMAGE,  /**< dark image */
  GAIN_IMAGE   /**< gain image */
};


/** 
   function indexes to call on the server


   All set functions with argument -1 work as get, when possible 

*/

enum {

  // General purpose functions
  F_EXEC_COMMAND=0, /**< command is executed */
  F_GET_ERROR,  /**< return detector error status */

  // configuration  functions
  F_GET_DETECTOR_TYPE, /**< return detector type */
  F_SET_NUMBER_OF_MODULES, /**< set/get number of installed modules */
  F_GET_MAX_NUMBER_OF_MODULES, /**< get maximum number of installed modules */
  F_SET_EXTERNAL_SIGNAL_FLAG,  /**< set/get flag for external signal */
  F_SET_EXTERNAL_COMMUNICATION_MODE, /**< set/get  external communication mode (obsolete) */


  // Tests and identification

  F_GET_ID, /**< get detector id of version */
  F_DIGITAL_TEST,  /**< digital test of the detector */
  F_ANALOG_TEST,  /**<analog test of the detector */
  F_ENABLE_ANALOG_OUT,  /**<enable the analog output */
  F_CALIBRATION_PULSE,  /**<pulse the calibration input */

  // Initialization functions
  F_SET_DAC,   /**< set DAC value */
  F_GET_ADC,  /**< get ADC value */
  F_WRITE_REGISTER,  /**< write to register */
  F_READ_REGISTER, /**< read register */
  F_WRITE_MEMORY,  /**< write to memory */
  F_READ_MEMORY, /**< read memory */


  F_SET_CHANNEL,  /**< initialize channel */
  F_GET_CHANNEL,  /**< get channel register */
  F_SET_ALL_CHANNELS,  /**< initialize all channels */

  F_SET_CHIP,  /**< initialize chip */
  F_GET_CHIP,  /**< get chip status */
  F_SET_ALL_CHIPS,  /**< initialize all chips */

  F_SET_MODULE, /**< initialize module */
  F_GET_MODULE,  /**< get module status */
  F_SET_ALL_MODULES,  /**< initialize all modules */


  F_SET_SETTINGS,  /**< set detector settings */
  F_GET_THRESHOLD_ENERGY,  /**< get detector threshold (in eV) */
  F_SET_THRESHOLD_ENERGY,  /**< set detector threshold (in eV) */


  // Acquisition functions
  F_START_ACQUISITION, /**< start acquisition */
  F_STOP_ACQUISITION, /**< stop acquisition */
  F_START_READOUT, /**< start readout */
  F_GET_RUN_STATUS,  /**< get acquisition status */
  F_START_AND_READ_ALL,  /**< start acquisition and read all frames*/
  F_READ_FRAME,  /**< read one frame */
  F_READ_ALL,  /**< read alla frames */
  
  //Acquisition setup functions
  F_SET_TIMER,  /**< set/get timer value */
  F_GET_TIME_LEFT,  /**< get current value of the timer (time left) */



  F_SET_DYNAMIC_RANGE,  /**< set/get detector dynamic range */
  F_SET_READOUT_FLAGS,  /**< set/get readout flags */
  F_SET_ROI,  /**< set/get region of interest */

  F_SET_SPEED,  /**< set/get readout speed parameters */

  //Trimming
  F_EXECUTE_TRIMMING,   /**< execute trimming */

  F_EXIT_SERVER,  /**< turn off detector server */

  F_LOCK_SERVER, /**< Locks/Unlocks server communication to the given client */ 

  F_GET_LAST_CLIENT_IP,  /**< returns the IP of the client last connected to the detector */ 
  
  F_SET_PORT, /**< Changes communication port of the server */

  F_UPDATE_CLIENT, /**< Returns all the important parameters to update the shared memory of the client */

  F_CONFIGURE_MAC, /**< Configures MAC for Gotthard readout */
  
  F_LOAD_IMAGE,   /**< Loads Dark/Gain image to the Gotthard detector */

  // multi detector structures

  F_SET_MASTER, /**< sets master/slave flag for multi detector structures */

  F_SET_SYNCHRONIZATION_MODE /**< sets master/slave synchronization mode for multidetector structures */


  /* Always append functions hereafter!!! */

};


/**
   angular conversion constant for a module
 */
typedef struct  {
  float center;  /**< center of the module (channel at which the radius is perpendicular to the module surface) */
  float ecenter; /**< error in the center determination */
  float r_conversion;  /**<  detector pixel size (or strip pitch) divided by the diffractometer radius */
  float er_conversion;  /**< error in the r_conversion determination */
  float offset; /**< the module offset i.e. the position of channel 0 with respect to the diffractometer 0 */
  float eoffset; /**< error in the offset determination */
  float tilt; /**< ossible tilt in the orthogonal direction (unused)*/
  float etilt; /**< error in the tilt determination */
} angleConversionConstant;


enum angleConversionParameter {
  ANGULAR_DIRECTION, /**< angular direction of the diffractometer */
  GLOBAL_OFFSET, /**< global offset of the diffractometer */
  FINE_OFFSET, /**< fine offset of the diffractometer */
  BIN_SIZE /**< angular bin size */
}; 




#endif
