


#ifndef MYTHEN_DETECTOR_H
#define MYTHEN_DETECTOR_H

using namespace std;



/**
 * 
 *
@libdoc The mythenDetector class contains the functions specific to the mythen detector
 *
 * @short This calss contains all mythen specific functions
 * @author Anna Bergamaschi
 * @version 0.1alpha (any string)


 */
#include "slsDetector.h"

class mythenDetector : public slsDetector{



 public:
  


/** (default) constructor 

 \param  id is the detector index which is needed to define the shared memory id. Different physical detectors should have different IDs in order to work independently


*/
  mythenDetector(int id=0);
  //slsDetector(string  const fname);
  /** destructor */ 
  ~mythenDetector(){};



  /**
     reads a trim file
     \param fname name of the file to be read
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
  */

  sls_detector_module* readTrimFile(string fname,  sls_detector_module* myMod=NULL);

  /**
     writes a trim file
     \param fname name of the file to be written
     \param mod module structure which has to be written to file
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module
  */
  int writeTrimFile(string fname, sls_detector_module mod); 
  
  /**
     writes a trim file for module number imod - the values will be read from the current detector structure
     \param fname name of the file to be written
     \param imod module number
     \returns OK or FAIL if the file could not be written   
     \sa ::sls_detector_module sharedSlsDetector
  */
  int writeTrimFile(string fname, int imod);

 
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
     returns the location of the calibration files
  \sa  sharedSlsDetector
  */

  /**
  int readCalibrationFile(string fname, float &gain, float &offset);
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


  /**
      reads an angular conversion file
      \param fname file to be read
  \sa  angleConversionConstant
  */
  int readAngularConversion(string fname="");
  /**
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant
  */
  int writeAngularConversion(string fname="");
 


  /* Communication to server */

  // Tests and identification

  /** 
      set angular conversion
      \param fname file with angular conversion constants ("" disable)
      \returns 0 if angular conversion disabled, >0 otherwise
  */
  int setAngularConversion(string fname="");

  /** 
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
  */
  int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL);
  
  /** 
      set detector global offset
  */
  float setGlobalOffset(float f){thisDetector->globalOffset=f; return thisDetector->globalOffset;}; 

  /** 
      set detector fine offset
  */
  float setFineOffset(float f){thisDetector->fineOffset=f; return thisDetector->fineOffset;};
  /** 
      get detector fine offset
  */
  float getFineOffset(){return thisDetector->fineOffset;};
  
  /** 
      get detector global offset
  */
  float getGlobalOffset(){return thisDetector->globalOffset;};

  /** 
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
  int setPositions(int nPos, float *pos){thisDetector->numberOfPositions=nPos; for (int ip=0; ip<nPos; ip++) thisDetector->detPositions[ip]=pos[ip]; return thisDetector->numberOfPositions;};
   /** 
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
  */
  int getPositions(float *pos=NULL){ if (pos ) {for (int ip=0; ip<thisDetector->numberOfPositions; ip++) pos[ip]=thisDetector->detPositions[ip];} return thisDetector->numberOfPositions;};
  
  
  /** set detector bin size used for merging (approx angular resolution)*/
  float setBinSize(float bs) {thisDetector->binSize=bs; return thisDetector->binSize;}
  /** return detector bin size used for merging (approx angular resolution)*/
  float getBinSize() {return thisDetector->binSize;}






  /** 
     decode data from the detector converting them to an array of floats, one for each channle
     \param datain data from the detector
     \returns pointer to a float array with a data per channel
  */
  float* decodeData(int *datain);

  
  
  int resetMerging(float *mp, float *mv,float *me, int *mm);
  /** not yet implemented 
      merge dataset
      \param p1 angular positions of dataset
      \param v1 data
      \param e1 errors
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
  */
  int addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm);

  /** 
      calculates the "final" positions, data value and errors for the emrged data
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns FAIL or the 
  */
  int finalizeMerging(float *mp, float *mv,float *me, int *mm);

 

 private:
 

};


#endif
