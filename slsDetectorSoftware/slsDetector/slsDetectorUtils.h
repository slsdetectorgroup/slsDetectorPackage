

#ifndef SLS_DETECTOR_UTILS_H
#define SLS_DETECTOR_UTILS_H


#ifdef __CINT__
class pthread_mutex_t;
class pthread_t;
#endif


extern "C" {
#include <pthread.h>
}

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>


#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>
#include <semaphore.h>
#include <cstdlib>



#include "postProcessing.h"





/**
   @short class containing all the possible detector functionalities 

   (used in the PSi command line interface)
 */


class slsDetectorUtils : public postProcessing {


public:

	slsDetectorUtils();

	virtual ~slsDetectorUtils(){};


















protected:




};



#endif
