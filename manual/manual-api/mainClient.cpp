/**
\file mainClient.cpp

This file is an example of how to implement the slsDetectorUsers class 
You can compile it linking it to the slsDetector library

g++ mainClient.cpp -L lib -lSlsDetector -L/usr/lib64/ -L lib2 -lzmq  -pthread -lrt -lm -lstdc++

where,

lib is the location of libSlsDetector.so

lib2 is the location of the libzmq.a.
[ libzmq.a is required only when using data call backs and enabling data streaming from receiver to client.
It is linked in manual/manual-api from slsReceiverSoftware/include ]

 */

#include "slsDetectorUsers.h"
#include "detectorData.h"
#include <iostream>
#include <cstdlib>

/**
 * Data Call back function defined
 * @param pData pointer to data structure received from the call back
 * @param iframe frame number of data passed
 * @param isubframe sub frame number of data passed ( only valid for EIGER in 32 bit mode)
 * @param pArg pointer to object
 * \returns integer that is currently ignored
 */
int dataCallback(detectorData *pData, int iframe, int isubframe, void *pArg)
{
	std::cout  	<< " DataCallback:"
				<< "\n nx           : " << pData->npoints
				<< "\n ny           : " << pData->npy
				<< "\n Frame number : " << iframe << std::endl;
}


/**
 * Example of a main program using the slsDetectorUsers class
 *
 * - Arguments are optional
 * 		- argv[1] : Configuration File
 * 		- argv[2] : Measurement Setup File
 * 		- argv[3] : Detector Id (default is zero)
 */
int main(int argc,  char **argv) {
	/** - if specified, set ID from argv[3] */
	int id=0;
	if (argc>=4)
		id=atoi(argv[3]);


	/** - slsDetectorUsers Object is instantiated with appropriate ID */
	int ret = 1;
	slsDetectorUsers *pDetector = new slsDetectorUsers (ret, id);
	if (ret == 1) {
		std::cout << "Error: Could not instantiate slsDetectorUsers" << std::endl;
		return EXIT_FAILURE;
	}


	/** - if specified, load configuration file (necessary at least the first time it is called to properly configure advanced settings in the shared memory) */
	if (argc>=2){
		pDetector->readConfigurationFile(argv[1]);
		std::cout << "Detector configured" << std::endl;
	}

	/** - set detector in shared memory online (in case no config file was used) */
	pDetector->setOnline(1);

	/** - set receiver in shared memory online (in case no config file was used) */
	pDetector->setReceiverOnline(1);

	/** - registering data callback */
	pDetector->registerDataCallback(&dataCallback, NULL);


	/** - ensuring detector status is idle before starting acquisition. exiting if not idle */
	int status = pDetector->getDetectorStatus();
	if (status  !=  0){
		std::cout << "Detector not ready: " << slsDetectorUsers::runStatusType(status) << std::endl;
		return 1;
	}

	/** - if provided, load detector settings */
	if (argc>=3){
		pDetector->retrieveDetectorSetup(argv[2]);
		std::cout << "Detector measurement set-up done" << std::endl;
	}


	/** - start measurement */
	pDetector->startMeasurement();
	std::cout << "measurement finished" << std::endl;

	/** - returning when acquisition is finished or data are avilable */

	/** - delete slsDetectorUsers object */
	delete pDetector;

	return 0;
}

