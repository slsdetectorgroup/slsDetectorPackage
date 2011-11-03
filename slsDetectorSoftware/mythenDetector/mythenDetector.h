


#ifndef MYTHEN_DETECTOR_H
#define MYTHEN_DETECTOR_H

//#include <ostream>
#include "slsDetector.h"
#include "usersFunctions.h"

#define defaultTDead {170,90,750}

//using namespace std;
/**
 \mainpage C++ class with MYTHEN specific functions
 *
 

 @author Anna Bergamaschi

*/



class mythenDetector : public slsDetector{

  
  
 public:
  /** 
      (default) constructor 
  */
  mythenDetector(int id=0, detectorType t=MYTHEN) : slsDetector(t, id){};
  //slsDetector(string  const fname);
  //  ~slsDetector(){while(dataQueue.size()>0){}};
  /** destructor */ 
  virtual ~mythenDetector(){};




  /**
    reads configuration file fname calling executeLine
    \param fname file to be read
  */
  int readConfigurationFile(string const fname);  
  /**
     writes configuration file calling executeLine
    \param fname file to write to
  */
  int writeConfigurationFile(string const fname);
  /** 
    dumps all the possible detector parameters calling executeLine
    \param fname file to write to
    \param level 0 dumps only main parameters and filenames for flat field correction etc.; 2 dumps really the complete configuration including flat field files, badchannels files, trimbits, angular conversion etc.
  */
  int dumpDetectorSetup(string const fname, int level=0);  
  /** 
      retrieves all possible detector parameters from file calling executeLine
    \param fname file to be read
  */
  int retrieveDetectorSetup(string const fname, int level=0);

  /**
     reads a trim/settings file
     \param fname name of the file to be read
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
  */

  sls_detector_module* readSettingsFile(string fname,  sls_detector_module* myMod=NULL);

  /**
     writes a trim/settings file
     \param fname name of the file to be written
     \param mod module structure which has to be written to file
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module
  */
  int writeSettingsFile(string fname, sls_detector_module mod); 
  
  /**
     writes a trim/settings file for module number imod - the values will be read from the current detector structure
     \param fname name of the file to be written
     \param imod module number
     \returns OK or FAIL if the file could not be written   
     \sa ::sls_detector_module sharedSlsDetector
  */
  int writeSettingsFile(string fname, int imod);

    /**
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
       
 
  */
  int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1); 
  
  /**
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL
  */
  int writeDataFile(string fname, int *data);
  
  /**
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not read the file or data=NULL
       
 
  */
  int readDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=0);  

  /**
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(string fname, int *data);


  /**
      reads a calibration file
      \param fname file to be read
      \param gain reference to the gain variable
      \offset reference to the offset variable
  \sa  sharedSlsDetector
  */
  int readCalibrationFile(string fname, float &gain, float &offset);
  /**
      writes a clibration file
      \param fname file to be written
      \param gain 
      \param offset
  \sa  sharedSlsDetector
  */
  int writeCalibrationFile(string fname, float gain, float offset);





};
#endif
