
/**
\file mainClient.cpp

This file is an example of how to implement the slsDetectorUsers class
You can compile it linking it to the slsDetector library

gcc mainClient.cpp -L lib -l SlsDetector -lm -pthread

where lib is the location of libSlsDetector.so
gcc mainClient.cpp -L . -l SlsDetector -lm -pthread -o users

*/

#include <iostream>
#include "slsDetectorUsers.h"
#include "detectorData.h"
#include <cstdlib>

/** Definition of the data callback which simply prints out the number of points received and teh frame number */
int dataCallback(detectorData *pData, int iframe, void *pArg)
{
  std::cout  << "dataCallback: " << pData->npoints  << " "  << pData->npy  << "Frame number: " << iframe << std::endl;
}


/**example of a main program using the slsDetectorUsers class */
int main(int argc,  char **argv) {
  int id=0;
  /** if specified, argv[3] is used as detector ID (default is 0)*/
  if (argc>=4)
    id=atoi(argv[3]);



  /** slsDetectorUsers is instantiated */
  slsDetectorUsers *pDetector = new  slsDetectorUsers (id);


	 char* argse[1];
        argse[0] = (char*)"free";
        pDetector->getCommand(1, argse, 0);

  /** if specified, argv[1] is used as detector config file (necessary at least the first time it is called to properly configure advanced settings in the shared memory)*/
  if (argc>=2){
    pDetector->readConfigurationFile(argv[1]);
    cout<<"Detector configured" << endl;
  }

 
   /** registering data callback */
   // pDetector->registerDataCallback(&dataCallback, NULL);

    /** checking detector status and exiting if not idle */
 /*  int status = pDetector->getDetectorStatus();
    if (status  !=  0){
      std::cout << "Detector not ready: " << slsDetectorUsers::runStatusType(status) << std::endl;
      return 1;
    }
*/
    /** load detector settings */
  /*  if (argc>=3){
    	pDetector->retrieveDetectorSetup(argv[2]);
    	cout<<"Detector measurement set-up done" << endl;
    }
    */
     /** start measurement */
  /*   pDetector->startMeasurement();
     cout<<"started measurement"<<endl;

     while (1) {
       usleep(100000);
       status = pDetector->getDetectorStatus();
        if (status  == 0 || status == 1|| status == 3)
	  break;
     }
     cout<<"measurement finished"<<endl;
     */
     /** returning when acquisition is finished or data are avilable */


     delete pDetector;

     return 0;
}

