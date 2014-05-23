/**
\file mainClient.cpp

This file is an example of how to implement the slsDetectorUsers class 
You can compile it linking it to the slsDetector library

gcc mainClient.cpp -L lib -l SlsDetector -lm -lpthread

where lib is the location of libSlsDetector.so

*/

#include <iostream>  
#include "slsDetectorUsers.h"
#include "detectorData.h"


/** Definition of the data callback which simply prints out the number of points received and teh frame number */
int dataCallback(detectorData *pData, int iframe, void *pArg)
{
  std::cout  << "dataCallback: " << pData->npoints  << " "  << pData->npy  << "Frame number: " << iframe << std::endl; 
}


/**example of a main program using the slsDetectorUsers class */
int main(int argc,  char *argv[]) {
  int id=0;
  int status;
  /** if specified, argv[2] is used as detector ID (default is 0)*/
  if (argc>=3)
    id=atoi(argv[2]);
  


  /** slsDetectorUsers is instantiated */
  slsDetectorUsers *pDetector = new  slsDetectorUsers (id); 

  
  /** if specified, argv[1] is used as detector config file (necessary at least the first time it is called to properly configure advanced settings in the shared memory)*/
  if (argc>=2)
    pDetector->readConfigurationFile(argv[1]);
 
  /** Setting the detector online (should be by default */
   pDetector->setOnline(1);

   /** Load setup file if argv[2] specified */
   if (argc>=3)
	   pDetector->retrieveDetectorSetup( argv[2]);
   else{
   /** defining the detector size */
    int minX, minY=0, sizeX, sizeY=1;
    pDetector->getDetectorSize(minX, minY, sizeX,  sizeY); 
    std::cout  << "X: Start=" << minX << ", Size= "  << sizeX  << std::endl; 
    std::cout  << "Y: Start=" << minY << ", Size= "  << sizeY  << std::endl; 
    pDetector->setDetectorSize(0,0,7680,1);
    std::cout  <<  pDetector->getDetectorDeveloper()  << std::endl; 

    /** registering data callback */
    pDetector->registerDataCallback(&dataCallback, NULL); 

    /** checking detector status and exiting if not idle */
    status = pDetector->getDetectorStatus();
    if (status  !=  0){
      std::cout << "Detector not ready: " << slsDetectorUsers::runStatusType(status) << std::endl; 
      return 1; 
    }

    /** checking and setting detector settings */
    std::cout  << "settings: "  << slsDetectorUsers::getDetectorSettings(pDetector->setSettings()) << std::endl; 
    pDetector->setSettings(slsDetectorUsers::getDetectorSettings("veryhighgain"));
    std::cout  << "settings: "  << slsDetectorUsers::getDetectorSettings(pDetector->setSettings()) << std::endl; 

     /** Settings exposure time to 10ms */
     pDetector->setExposureTime(10000000); 

     /** Settings exposure time to 100ms */
     pDetector->setExposurePeriod(100000000);
 
     /** Settingsnumber of frames to 30 */
     pDetector->setNumberOfFrames(30);
}
     /** start measurement */
     pDetector->startMeasurement(); 

     while (1) {
       usleep(100000); 
        status = pDetector->getDetectorStatus();
        if (status  == 0 || status == 1|| status == 3)
	  break; 
     }

     char *temp[] = {"receiver", NULL};
     /** returning when acquisition is finished or data are avilable */
   	std::cout << "answer to a get command:" << pDetector->getCommand(1,temp,0) << std::endl;
    
     delete pDetector; 
     
     return 0; 
}

