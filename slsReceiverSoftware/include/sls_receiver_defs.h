#pragma once


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

/** default maximum string length */
#define MAX_STR_LENGTH 1000
#define MAX_FRAMES_PER_FILE			20000
#define SHORT_MAX_FRAMES_PER_FILE	100000
#define MOENCH_MAX_FRAMES_PER_FILE	1000
#define EIGER_MAX_FRAMES_PER_FILE	10000
#define JFRAU_MAX_FRAMES_PER_FILE 	10000
#define JFCTB_MAX_FRAMES_PER_FILE   100000


#define DEFAULT_STREAMING_TIMER_IN_MS 200

/** default ports */
#define DEFAULT_PORTNO    		1952
#define DEFAULT_UDP_PORTNO 		50001
#define DEFAULT_GUI_PORTNO 		65001
#define DEFAULT_ZMQ_CL_PORTNO 	30001
#define DEFAULT_ZMQ_RX_PORTNO 	30001

#define SLS_DETECTOR_HEADER_VERSION         0x2
#define SLS_DETECTOR_JSON_HEADER_VERSION    0x3

/** 
    \file sls_receiver_defs.h
This file contains all the basic definitions common to the slsReceiver class
and to the server programs running on the receiver
 * @author Anna Bergamaschi
 * @version 0.1alpha (any string)
 * @see slsDetector
$Revision: 809 $
 */


#ifdef __cplusplus

/** @short class containing all the constants and enum definitions */
class slsReceiverDefs {
public:

	slsReceiverDefs(){};

#endif

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
		AGIPD, /**< agipd */
		MOENCH, /**< moench */
		JUNGFRAU, /**< jungfrau */
		JUNGFRAUCTB, /**< jungfrauCTBversion */
		PROPIX, /**< propix */
		MYTHEN3 /**< mythen 3 */
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

		PROGRESS, /**< fraction of measurement elapsed - only get! */
		MEASUREMENTS_NUMBER,
		FRAMES_FROM_START,
		FRAMES_FROM_START_PG,
		SAMPLES_JCTB,
		SUBFRAME_ACQUISITION_TIME, /**< subframe exposure time */
		STORAGE_CELL_NUMBER, /**<number of storage cells */
		SUBFRAME_DEADTIME, /**< subframe deadtime */
		MEASURED_PERIOD,	/**< measured period */
		MEASURED_SUBPERIOD,	/**< measured subperiod */
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
	      \param t string can be Mythen, Pilatus, Eiger, Gotthard, Agipd, Unknown
	      \returns MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, MÖNCH, GENERIC
	  */
	  static std::string getDetectorType(detectorType t){	\
	    switch (t) {										\
	    case MYTHEN:    	return std::string("Mythen");	\
	    case PILATUS:   	return std::string("Pilatus");	\
	    case EIGER:    		return std::string("Eiger");	\
	    case GOTTHARD:    	return std::string("Gotthard");	\
	    case AGIPD:    		return std::string("Agipd");	\
	    case MOENCH:    	return std::string("Moench");	\
	    case JUNGFRAU:    	return std::string("Jungfrau");	\
	    case JUNGFRAUCTB:   return std::string("JungfrauCTB");	\
	    case PROPIX:    	return std::string("Propix");	\
	    case MYTHEN3:		return std::string("Mythen3");  \
	    default:    		return std::string("Unknown");	\
	    }};

	  /** returns detector type index from detector type string
	      \param type can be MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, GENERIC
	      \returns Mythen, Pilatus, Eiger, Gotthard, Agipd, Mönch, Unknown
	  */
	  static detectorType getDetectorType(std::string const type){\
	    if (type=="Mythen")      	return MYTHEN;		\
	    if (type=="Pilatus")      	return PILATUS;		\
	    if (type=="Eiger")    		return EIGER;		\
	    if (type=="Gotthard")    	return GOTTHARD;	\
	    if (type=="Agipd")    		return AGIPD;		\
	    if (type=="Moench")    		return MOENCH;		\
	    if (type=="Jungfrau")    	return JUNGFRAU;	\
	    if (type=="JungfrauCTB") 	return JUNGFRAUCTB;	\
	    if (type=="Propix")    		return PROPIX;		\
	    if (type=="Mythen3")		return MYTHEN3;		\
	    							return GENERIC;		\
	  };


	  /** returns string from run status index
	      \param s can be ERROR, WAITING, RUNNING, TRANSMITTING, RUN_FINISHED
	      \returns string error, waiting, running, data, finished
	  */
	  static std::string runStatusType(runStatus s){\
	    switch (s) {				\
	    case ERROR:       	return std::string("error");	\
	    case  WAITING:      return std::string("waiting");	\
	    case RUNNING:      	return std::string("running");	\
	    case TRANSMITTING:  return std::string("data");		\
	    case  RUN_FINISHED: return std::string("finished");	\
	    case  STOPPED: return std::string("stopped");	\
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

#endif

#ifdef __cplusplus
protected:
#endif

#ifndef MYROOT
#include "sls_receiver_funcs.h"
#endif

#ifdef __cplusplus
};
#endif
;
