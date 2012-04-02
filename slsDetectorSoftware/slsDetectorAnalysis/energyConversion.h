#ifndef ENERGYCONVERSION_H
#define ENERGYCONVERSION_H

#include "sls_detector_defs.h"
#include <string>

using namespace std;



class energyConversion: public slsDetectorDefs {

 public:
  energyConversion(){};
  virtual ~energyConversion(){};

  
  /**
   
      reads a calibration file MOVE TO ENERGY CALIBRATION?!?!??!?
      \param fname file to be read
      \param gain reference to the gain variable
      \offset reference to the offset variable
  */
  static int readCalibrationFile(string fname, float &gain, float &offset);
  
  /**
   
      writes a calibration file MOVE TO ENERGY CALIBRATION?!?!??!?
      \param fname file to be written
      \param gain 
      \param offset
  */
  static int writeCalibrationFile(string fname, float gain, float offset);


  /**
     reads a trim/settings file
     \param fname name of the file to be read
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
     \sa mythenDetector::readSettingsFile
  */

  sls_detector_module* readSettingsFile(string fname, detectorType myDetectorType, sls_detector_module* myMod=NULL);

  /**
     writes a trim/settings file
     \param fname name of the file to be written
     \param mod module structure which has to be written to file
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module mythenDetector::writeSettingsFile(string, sls_detector_module)
  */
  int writeSettingsFile(string fname, detectorType myDetectorType, sls_detector_module mod); 
  

  virtual sls_detector_module*  createModule(detectorType myDetectorType)=0;
  
  virtual void deleteModule(sls_detector_module *myMod)=0;
  
 protected:
  char *settingsFile;
  


};
#endif
