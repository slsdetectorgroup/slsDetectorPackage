/*
 * error_defs.h
 *
 *  Created on: Jan 18, 2013
 *      Author: l_maliakal_d
 */

#ifndef ERROR_DEFS_H_
#define ERROR_DEFS_H_


#include <string>
using namespace std;


#include "sls_detector_defs.h"



/** Error flags */
#define NUM_ERROR_FLAGS 32
#define CRITICAL_ERROR_MASK 0xFFFFFFFF

#define CANNOT_CONNECT_TO_DETECTOR  		0x8000000000000000ULL
#define CANNOT_CONNECT_TO_RECEIVER  		0x4000000000000000ULL
#define COULDNOT_SET_CONTROL_PORT			0x2000000000000000ULL
#define COULDNOT_SET_STOP_PORT				0x1000000000000000ULL
#define COULDNOT_SET_DATA_PORT				0x0800000000000000ULL
#define FILE_PATH_DOES_NOT_EXIST			0x0400000000000000ULL
#define COULDNOT_CREATE_UDP_SOCKET			0x0200000000000000ULL
#define COULDNOT_CREATE_FILE				0x0100000000000000ULL
#define COULDNOT_ENABLE_COMPRESSION			0x0080000000000000ULL
#define RECEIVER_DET_HOSTNAME_NOT_SET		0x0040000000000000ULL
#define RECEIVER_DET_HOSTTYPE_NOT_SET		0x0020000000000000ULL
#define DETECTOR_TEN_GIGA					0x0010000000000000ULL

//											0xFFFFFFFF00000000ULL

#define COULD_NOT_CONFIGURE_MAC				0x0000000000000001ULL
#define COULDNOT_SET_NETWORK_PARAMETER		0x0000000000000002ULL
#define COULDNOT_SET_ROI					0x0000000000000004ULL
#define RECEIVER_READ_FREQUENCY				0x0000000000000008ULL
#define SETTINGS_NOT_SET					0x0000000000000010ULL
#define SETTINGS_FILE_NOT_OPEN				0x0000000000000020ULL
#define COULDNOT_START_RECEIVER				0x0000000000000040ULL // default error like starting threads
#define COULDNOT_STOP_RECEIVER				0x0000000000000080ULL
#define DETECTOR_TIMER_VALUE_NOT_SET		0x0000000000000100ULL
#define RECEIVER_ACQ_PERIOD_NOT_SET			0x0000000000000200ULL
#define RECEIVER_FRAME_NUM_NOT_SET			0x0000000000000400ULL
#define RECEIVER_DYNAMIC_RANGE				0x0000000000000800ULL
#define RECEIVER_TEN_GIGA					0x0000000000001000ULL
#define ALLTIMBITS_NOT_SET					0x0000000000002000ULL

//											0x00000000FFFFFFFFULL
/** @short class returning all error messages for error mask */
class errorDefs {


public:

	/** Constructor */
	errorDefs():errorMask(0){};

	/** Gets the error message
	 * param errorMask error mask
	 /returns error message from error mask
	*/
	static string getErrorMessage(int64_t slsErrorMask){

		string retval = "";

		if(slsErrorMask&CANNOT_CONNECT_TO_DETECTOR)
			retval.append("Cannot connect to Detector\n");

		if(slsErrorMask&CANNOT_CONNECT_TO_RECEIVER)
			retval.append("Cannot connect to Receiver\n");

		if(slsErrorMask&COULDNOT_SET_CONTROL_PORT)
			retval.append("Could not set control port\n");

		if(slsErrorMask&COULDNOT_SET_STOP_PORT)
			retval.append("Could not set stop port\n");

		if(slsErrorMask&COULDNOT_SET_DATA_PORT)
			retval.append("Could not set receiver port\n");

		if(slsErrorMask&FILE_PATH_DOES_NOT_EXIST)
			retval.append("Path to Output Directory does not exist\n");

		if(slsErrorMask&COULDNOT_CREATE_UDP_SOCKET)
			retval.append("Could not create UDP socket to start receiver\n");

		if(slsErrorMask&COULDNOT_CREATE_FILE)
			retval.append("Could not create file to start receiver.\nCheck permissions of output directory or the overwrite flag\n");

		if(slsErrorMask&COULDNOT_ENABLE_COMPRESSION)
			retval.append("Could not enable/disable data compression in receiver.\nThread creation failed or recompile code with MYROOT1 flag.\n");

		if(slsErrorMask&RECEIVER_DET_HOSTNAME_NOT_SET)
			retval.append("Could not send the detector hostname to the receiver.\n");

		if(slsErrorMask&RECEIVER_DET_HOSTTYPE_NOT_SET)
			retval.append("Could not send the detector type to the receiver.\n");

		if(slsErrorMask&DETECTOR_TEN_GIGA)
			retval.append("Could not enable/disable 10GbE in the detector.\n");




		if(slsErrorMask&COULD_NOT_CONFIGURE_MAC)
			retval.append("Could not configure mac\n");

		if(slsErrorMask&COULDNOT_SET_NETWORK_PARAMETER)
			retval.append("Could not set network parameter. Should be valid and in proper format\n");

		if(slsErrorMask&COULDNOT_SET_ROI)
			retval.append("Could not set the exact region of interest. Verify ROI set by detector.\n");

		if(slsErrorMask&RECEIVER_READ_FREQUENCY)
			retval.append("Could not set receiver read frequency.\n");

		if(slsErrorMask&SETTINGS_NOT_SET)
			retval.append("Could not set settings.\n");

		if(slsErrorMask&SETTINGS_FILE_NOT_OPEN)
			retval.append("Could not open settings file. Verify if it exists.\n");

		if(slsErrorMask&COULDNOT_START_RECEIVER)
			retval.append("Could not start receiver.\n");

		if(slsErrorMask&COULDNOT_STOP_RECEIVER)
			retval.append("Could not stop receiver.\n");

		if(slsErrorMask&DETECTOR_TIMER_VALUE_NOT_SET)
			retval.append("Could not set one of timer values in detector.\n");

		if(slsErrorMask&RECEIVER_ACQ_PERIOD_NOT_SET)
			retval.append("Could not set acquisition period in receiver.\n");

		if(slsErrorMask&RECEIVER_FRAME_NUM_NOT_SET)
			retval.append("Could not set frame number in receiver.\n");

		if(slsErrorMask&RECEIVER_DYNAMIC_RANGE)
			retval.append("Could not set dynamic range in receiver.\n");

		if(slsErrorMask&RECEIVER_TEN_GIGA)
			retval.append("Could not enable/disable 10GbE in the receiver.\n");

		if(slsErrorMask&ALLTIMBITS_NOT_SET)
			retval.append("Could not set all trimbits to value.\n");



		return retval;

	}


	  /** Sets multi error mask
	     @param multi error mask to be set to
	     /returns multi error mask
	  */
	  int64_t  setErrorMask(int64_t i){errorMask=i;return getErrorMask();};

	  /**returns multi error mask  */
	   int64_t  getErrorMask(){return errorMask;};

	   /** Clears error mask
	      /returns error mask
	   */
	   int64_t clearErrorMask(){errorMask=0;return errorMask;};


protected:

	  /** Error Mask */
	  int64_t errorMask;

};

#endif /* ERROR_DEFS_H_ */
