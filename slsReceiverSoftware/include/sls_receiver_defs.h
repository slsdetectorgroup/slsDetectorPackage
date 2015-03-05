#ifndef SLS_RECEIVER_DEFS_H
#define SLS_RECEIVER_DEFS_H


#ifdef __CINT__
#define MYROOT
#define __cplusplus
#endif

#include <stdint.h> 
#include "ansi.h"

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;

/** default maximum string length */
#define MAX_STR_LENGTH 1000
#define MAX_FRAMES_PER_FILE			20000
#define SHORT_MAX_FRAMES_PER_FILE	100000
#define MOENCH_MAX_FRAMES_PER_FILE	1000
#define EIGER_MAX_FRAMES_PER_FILE	20000
#define JFCTB_MAX_FRAMES_PER_FILE      100000


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
		JUNGFRAUCTB /**< jungfrauCTBversion */
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
		FRAMES_FROM_START_PG
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
#endif
;
