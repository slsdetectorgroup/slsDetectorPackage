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
#include <cmath>
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
      \param offset reference to the offset variable
      \returns OK if successful, else FAIL or -1

  */
  static int readCalibrationFile(string fname, double &gain, double &offset);
  
  /**
      writes a calibration file 
      \param fname file to be written
      \param gain 
      \param offset
      \returns OK if successful, else FAIL or -1
  */
  static int writeCalibrationFile(string fname, double gain, double offset);

  /**
      reads a calibration file 
      \param fname file to be read
      \param gain reference to the gain variable
      \param offset reference to the offset variable
      \returns OK if successful, else FAIL or -1
  */
  static int readCalibrationFile(string fname, int *gain, int *offset);
  
  /**
      writes a calibration file 
      \param fname file to be written
      \param gain reference to the gain variable
      \param offset reference to the offset variable
      \returns OK if successful, else FAIL or -1
  */
  static int writeCalibrationFile(string fname, int *gain, int *offset);



  //Template function to do linear interpolation between two points
  template <typename E, typename V>
  V linearInterpolation(const E x, const E x1, const E x2, const V y1, const V y2){
          double k = static_cast<double>(y2-y1)/(x2-x1);
          double m = y1-k*x1;
          int y = round( k*x+m );
          return static_cast<V>(y);
  }


  /**
   * interpolates dacs and trimbits between 2 trim files
   	 \param myDetectorType detector type (needed for which dacs that neeeds to be interpolated & which kept same)
     \param a first module structure
     \param b second module structure
     \param energy energy to trim at
     \param e1 reference trim value
     \param e2 reference trim value
     \returns  the pointer to the module structure with interpolated values or NULL if error
   */
  sls_detector_module* interpolateTrim(detectorType myDetectorType, sls_detector_module* a, sls_detector_module* b, const int energy, const int e1, const int e2);



#ifndef MYROOT

  /**
     reads a trim/settings file
     \param fname name of the file to be read
     \param myDetectorType detector type (needed for number of channels, chips, dacs etc.)
     \param iodelay io delay (detector specific)
     \param tau tau (detector specific)
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
  */

  sls_detector_module* readSettingsFile(string fname, detectorType myDetectorType, int& iodelay, int& tau, sls_detector_module* myMod=NULL);

  /**
     writes a trim/settings file
     \param fname name of the file to be written
     \param myDetectorType detector type (needed for number of channels, chips, dacs etc.)
     \param mod module structure which has to be written to file
     \param iodelay io delay (detector specific)
     \param tau tau (detector specific)
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module mythenDetector::writeSettingsFile(string, sls_detector_module)
  */
  int writeSettingsFile(string fname, detectorType myDetectorType, sls_detector_module mod, int iodelay, int tau);
  
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
