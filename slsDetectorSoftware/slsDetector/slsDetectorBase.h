
#ifndef SLS_DETECTOR_BASE_H
#define SLS_DETECTOR_BASE_H
/**
   \mainpage Common C++ library for SLS detectors data acquisition
   *
   * \section intro_sec Introduction

   * \subsection mot_sec Motivation
   Although the SLS detectors group delvelops several types of detectors (1/2D, counting/integrating etc.) it is common interest of the group to use a common platfor for data acquisition
   \subsection arch_sec System Architecture
   The architecture of the acquisitions system is intended as follows:
   \li A socket server running on the detector (or more than one in some special cases)
   \li C++ classes common to all detectors for client-server communication. These can be supplied to users as libraries and embedded also in acquisition systems which are not developed by the SLS
   \li the possibility of using a Qt-based graphical user interface (with eventually root analisys capabilities)
   \li the possibility of running all commands from command line. In order to ensure a fast operation of this so called "text client" the detector parameters should not be re-initialized everytime. For this reason a shared memory block is allocated where the main detector flags and parameters are stored 
   \li a Root library for data postprocessing and detector calibration (energy, angle).

   \section howto_sec How to use it

   The detectors can be simply operated by using the provided GUi or command line executable. <br>
   In case you need to embed the detector control e.g in the beamline control software, compile these classes using 
   <BR>
   make package
   <br>
   and link the shared library created to your software slsDetectorSoftware/bin/libSlsDetector.so
   <br>
   The software can also be installed (with super-user rights)<br>
   make install
   <br>
   <br>
   Most methods of interest for the user are implemented in the ::slsDetectorBase interface class, but the classes to be implemented in the main program are either ::slsDetector (for single controller detectors) or ::multiSlsDetector (for multiple controllers, but can work also for single controllers).

   @author Anna Bergamaschi
   @version 0.1alpha

*/



/**
 * 
 *
 *
 * @author Anna Bergamaschi
 * @version 0.1alpha
 */


#include "sls_detector_defs.h"
#include "sls_receiver_defs.h"
#include "slsDetectorUsers.h"
#include "error_defs.h"

#include <string>

/** 

@libdoc The slsDetectorBase contains also a set of purely virtual functions useful for the implementation of the derived classes


* @short This is the base class for all detector functionalities

*/

//public virtual slsDetectorUsers,
class slsDetectorBase :  public virtual slsDetectorDefs, public virtual errorDefs {

 public:

  /** default constructor */
  slsDetectorBase(){};


  /** virtual destructor */
  virtual ~slsDetectorBase(){};
  
   /** returns detector type std::string from detector type index
      \param t std::string can be  Eiger, Gotthard, Jungfrau, Unknown
      \returns  EIGER, GOTTHARD, JUNGFRAU, GENERIC
  */
  static std::string getDetectorType(detectorType t){\
    switch (t) {\
    case EIGER:    return std::string("Eiger");	\
    case GOTTHARD:    return std::string("Gotthard");	\
    case JUNGFRAU:    return std::string("Jungfrau");		\
    case JUNGFRAUCTB:    return std::string("JungfrauCTB");		\
    default:    return std::string("Unknown");		\
    }};

  /** returns detector type index from detector type std::string
      \param type can be EIGER, GOTTHARD, JUNGFRAU, GENERIC
      \returns Eiger, Gotthard, Jungfrau, Unknown
  */
  static detectorType getDetectorType(std::string const type){\
    if  (type=="Eiger")    return EIGER;		\
    if  (type=="Gotthard")    return GOTTHARD;	\
    if  (type=="Jungfrau")    return JUNGFRAU;		\
    if  (type=="JungfrauCTB")    return JUNGFRAUCTB;		\
    return GENERIC;};



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

  /** returns std::string from run status index
      \param s can be ERROR, WAITING, RUNNING, TRANSMITTING, RUN_FINISHED
      \returns std::string error, waiting, running, data, finished
  */
  static std::string runStatusType(runStatus s){\
    switch (s) {				\
    case ERROR:       return std::string("error");		\
    case  WAITING:      return  std::string("waiting");	\
    case RUNNING:      return std::string("running");\
    case TRANSMITTING:      return std::string("data");	\
    case  RUN_FINISHED:      return std::string("finished");	\
    default:       return std::string("idle");		\
    }};

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
      \param s can be FRAME_NUMBER,ACQUISITION_TIME,FRAME_PERIOD, DELAY_AFTER_TRIGGER,GATES_NUMBER, CYCLES_NUMBER, ACTUAL_TIME,MEASUREMENT_TIME, PROGRESS,MEASUREMENTS_NUMBER,FRAMES_FROM_START,FRAMES_FROM_START_PG,SAMPLES_JCTB,SUBFRAME_ACQUISITION_TIME,STORAGE_CELL_NUMBER, SUBFRAME_DEADTIME
      \returns std::string frame_number,acquisition_time,frame_period, delay_after_trigger,gates_number, cycles_number, actual_time,measurement_time, progress,measurements_number,frames_from_start,frames_from_start_pg,samples_jctb,subframe_acquisition_time,storage_cell_number, SUBFRAME_DEADTIME
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
    case SAMPLES_JCTB: 				return std::string("samples_jctb"); 				\
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


};


#endif
