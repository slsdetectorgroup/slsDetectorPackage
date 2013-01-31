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



/** @short class returning all error messages for error mask */
class errorDefs {


public:

	/** Constructor */
	errorDefs(){};

	/** Gets the error message
	 * param errorMask error mask
	 /returns error message from error mask
	*/
	static string getErrorMessage(int slsErrorMask){

		string retval = "";

		if(slsErrorMask&CANNOT_CONNECT_TO_DETECTOR)
			retval.append("Cannot connect to Detector\n");

		if(slsErrorMask&CANNOT_CONNECT_TO_RECEIVER)
			retval.append("Cannot connect to Receiver\n");

		if(slsErrorMask&COULD_NOT_CONFIGURE_MAC)
			retval.append("Could not configure mac\n");

		return retval;

	}


};

#endif /* ERROR_DEFS_H_ */
