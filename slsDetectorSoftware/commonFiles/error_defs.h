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

#define CANNOT_CONNECT_TO_DETECTOR  0x8000000000000000ULL
#define CANNOT_CONNECT_TO_RECEIVER  0x4000000000000000ULL
#define COULDNOT_SET_CONTROL_PORT	0x2000000000000000ULL
#define COULDNOT_SET_STOP_PORT		0x1000000000000000ULL
#define COULDNOT_SET_DATA_PORT		0x0800000000000000ULL



#define COULD_NOT_CONFIGURE_MAC				0x0000000000000001ULL
#define COULDNOT_SET_NETWORK_PARAMETER		0x0000000000000002ULL
#define COULDNOT_SET_ROI					0x0000000000000004ULL
#define FILE_PATH_DOES_NOT_EXIST			0x0000000000000008ULL
#define RECEIVER_READ_FREQUENCY				0x0000000000000010ULL
#define SETTINGS_NOT_SET					0x0000000000000020ULL

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



		if(slsErrorMask&COULD_NOT_CONFIGURE_MAC)
			retval.append("Could not configure mac\n");

		if(slsErrorMask&COULDNOT_SET_NETWORK_PARAMETER)
			retval.append("Could not set network parameter. Should be valid and in proper format\n");

		if(slsErrorMask&COULDNOT_SET_ROI)
			retval.append("Could not set the exact region of interest. Verify ROI set by detector.\n");

		if(slsErrorMask&FILE_PATH_DOES_NOT_EXIST)
			retval.append("Path to Output Directory does not exist.\n");

		if(slsErrorMask&RECEIVER_READ_FREQUENCY)
			retval.append("Could not set receiver read frequency.\n");

		if(slsErrorMask&SETTINGS_NOT_SET)
			retval.append("Could not set settings.\n");

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
