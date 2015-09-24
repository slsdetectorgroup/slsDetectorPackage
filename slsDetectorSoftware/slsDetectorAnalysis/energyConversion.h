#ifndef ENERGYCONVERSION_H
#define ENERGYCONVERSION_H

#ifdef __CINT__
#define MYROOT
#define __cplusplus
#endif

//#ifndef MYROOT
//#include "sls_receiver_defs.h"
#include "sls_detector_defs.h"
//#endif

#include <string>

using namespace std;
/**
   @short class handling the energy calibration and trim files IO
*/


class energyConversion 
//#ifndef MYROOT
: private virtual slsDetectorDefs
  //#endif
{
 public:
  /** default constrauctor */
  energyConversion(){};
  /** default destructor */
  virtual ~energyConversion(){};

  
  /**
      reads a calibration file 
      \param fname file to be read
      \param gain reference to the gain variable
      \offset reference to the offset variable
  */
  static int readCalibrationFile(string fname, double &gain, double &offset);
  
  /**
      writes a calibration file 
      \param fname file to be written
      \param gain 
      \param offset
  */
  static int writeCalibrationFile(string fname, double gain, double offset);



  
  /**
      reads a calibration file 
      \param fname file to be read
      \param gain reference to the gain variable
      \offset reference to the offset variable
  */
  static int readCalibrationFile(string fname, int *gain, int *offset, detectorType myDetectorType);
  
  /**
      writes a calibration file 
      \param fname file to be written
      \param gain 
      \param offset
  */
  static int writeCalibrationFile(string fname, int *gain, int *offset, detectorType myDetectorType);









#ifndef MYROOT

  /**
     reads a trim/settings file
     \param fname name of the file to be read
     \param myDetectorType detector type (needed for number of channels, chips, dacs etc.)
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
  */

  sls_detector_module* readSettingsFile(string fname, detectorType myDetectorType, sls_detector_module* myMod=NULL);

  /**
     writes a trim/settings file
     \param fname name of the file to be written
     \param myDetectorType detector type (needed for number of channels, chips, dacs etc.)
     \param mod module structure which has to be written to file
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module mythenDetector::writeSettingsFile(string, sls_detector_module)
  */
  int writeSettingsFile(string fname, detectorType myDetectorType, sls_detector_module mod);
  
  /** allocates the momery for a detector module structure
      \param myDetectorType detector type (needed for number of channels, chips, dacs etc.)
      \returns pointer to detector module
  */
  virtual sls_detector_module*  createModule(detectorType myDetectorType)=0;
  
  /**
     frees the memory of a detector module structure
     \param myMod pointer to memeory to be freed
  */
  virtual void deleteModule(sls_detector_module *myMod)=0;
  
 protected:
  /** pointer to settings file name */
  char *settingsFile;
  

#endif

};
#endif
