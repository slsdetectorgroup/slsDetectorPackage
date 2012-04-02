
#ifndef ANGULARCONVERSION_H
#define ANGULARCONVERSION_H


#include "sls_detector_defs.h"
#include <string>
#include <fstream>



  //float angle(int ichan, float encoder, float totalOffset, float conv_r, float center, float offset, float tilt, int direction)


using namespace std;

/**
   angular conversion constant for a module
 */
typedef struct  {
  float center;  /**< center of the module (channel at which the radius is perpendicular to the module surface) */
  float ecenter; /**< error in the center determination */
  float r_conversion;  /**<  detector pixel size (or strip pitch) divided by the diffractometer radius */
  float er_conversion;  /**< error in the r_conversion determination */
  float offset; /**< the module offset i.e. the position of channel 0 with respect to the diffractometer 0 */
  float eoffset; /**< error in the offset determination */
  float tilt; /**< ossible tilt in the orthogonal direction (unused)*/
  float etilt; /**< error in the tilt determination */
} angleConversionConstant;


/**
   class containing the methods to set/unset the angular conversion and merge the data
   The angular conversion itself is stored in the slsDetector/multiSlsDetector class!

*/
class angularConversion : public slsDetectorDefs {

 public:
  angularConversion();
  virtual ~angularConversion();
 


  //virtual int readAngularConversion(string fname)=0;




  /**
   
      reads an angular conversion file
      \param fname file to be read
  */
   static int readAngularConversion(string fname, int nmod, angleConversionConstant *angOff); 
   
   /**
   
      MOVE TO ANGULAR CALIBRATION?!?!??!?
      reads an angular conversion file
      \param fname file to be read
  */
   static int readAngularConversion(ifstream& ifs, int nmod, angleConversionConstant *angOff);
  /**
      MOVE TO ANGULAR CALIBRATION?!?!??!?
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
   static int writeAngularConversion(string fname, int nmod, angleConversionConstant *angOff);
 /**
      MOVE TO ANGULAR CALIBRATION?!?!??!?
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
   static int writeAngularConversion(ofstream& ofs, int nmod, angleConversionConstant *angOff);

   virtual int writeAngularConversion(string fname)=0;

   /** 
      MOVE TO ANGULAR CALIBRATION?!?!??!?
      sets the arrays of the merged data to 0. NB The array should be created with size >= 360./getBinSize(); 
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns OK or FAIL
      \sa mythenDetector::resetMerging
  */
   static int resetMerging(float *mp, float *mv,float *me, int *mm, int nbins);
   int resetMerging(float *mp, float *mv,float *me, int *mm);
   int resetMerging();

  /** 
      merge dataset
      \param p1 angular positions of dataset
      \param v1 data
      \param e1 errors
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \param nchans number of channels
      \param binsize size of angular bin
      \param nb number of angular bins
      \param badChanMask badchannelmask (if NULL does not correct for bad channels)
      \returns OK or FAIL
  */

   static int  addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm, int nchans, float binsize,int nb, int *badChanMask );
   
   int  addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm, int *badChanMask);

   int addToMerging(float *p1, float *v1, float *e1,int *badChanMask);

   /**
      MOVE TO ANGULAR CALIBRATION?!?!??!?
      calculates the "final" positions, data value and errors for the emrged data
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns FAIL or the number of non empty bins (i.e. points belonging to the pattern)
      \sa mythenDetector::finalizeMerging
  */

   static int finalizeMerging(float *mp, float *mv,float *me, int *mm, int nb);

   int finalizeMerging(float *mp, float *mv,float *me, int *mm);
   int finalizeMerging();


  /**
      pure virtual function
      set detector global offset
      \sa mythenDetector::setGlobalOffset
  */
  float setGlobalOffset(float f){return setAngularConversionParameter(GLOBAL_OFFSET,f);};

  /**
      pure virtual function
      set detector fine offset
      \sa mythenDetector::setFineOffset
  */
  float setFineOffset(float f){return setAngularConversionParameter(FINE_OFFSET,f);};
  /**
      pure virtual function
      get detector fine offset
      \sa mythenDetector::getFineOffset
  */
  float getFineOffset(){return getAngularConversionParameter(FINE_OFFSET);};
  
  /**
      pure virtual function
      get detector global offset
      \sa mythenDetector::getGlobalOffset
  */
  float getGlobalOffset(){return getAngularConversionParameter(GLOBAL_OFFSET);};

  /** 
      
      set detector bin size used for merging (approx angular resolution)
      \param bs bin size in degrees
      \returns current bin size
      \sa mythenDetector::setBinSize
*/
  float setBinSize(float bs){return setAngularConversionParameter(BIN_SIZE,bs);};

  /** 
      return detector bin size used for merging (approx angular resolution)
      \sa mythenDetector::getBinSize
  */
  float getBinSize() {return getAngularConversionParameter(BIN_SIZE);};



  int getAngularDirection(){return (int)getAngularConversionParameter(ANGULAR_DIRECTION);};

  
  int setAngularDirection(int d){return (int)setAngularConversionParameter(ANGULAR_DIRECTION, (float)d);};

  
  int getNumberOfAngularBins(){return nBins;};

  /**
      get angular conversion
      \param direction reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
  */
  virtual int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL)=0;

   float setAngularConversionParameter(angleConversionParameter c, float v);

   float getAngularConversionParameter(angleConversionParameter c);
   
   
   virtual int getTotalNumberOfChannels()=0;



  /** 
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
  virtual int setPositions(int nPos, float *pos);
   /** 
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
  */
  virtual int getPositions(float *pos=NULL);
 
  int deleteMerging();

  float *getMergedPositions(){return mergingBins;};
  float *getMergedCounts(){return mergingCounts;};
  float *getMergedErrors(){return mergingErrors;};
  


  /**
     sets  the angular conversion file
     \param fname file to read
     \returns angular conversion flag
  */

  int setAngularConversionFile(string fname);

  
  /**
      returns the angular conversion file
     */
  string getAngularConversionFile(){if (setAngularCorrectionMask()) return string(angConvFile); else return string("none");};


  virtual int readAngularConversionFile(string fname="")=0;


  // int setAngularConversionPointer(angleConversionConstant *p, int *nm, int nch, int idet=0); 




  virtual int getNMods()=0;
  virtual int getChansPerMod(int imod=0)=0;
  virtual angleConversionConstant *getAngularConversionPointer(int imod=0)=0;
  
  float* convertAngles(float pos);
  
  float *convertAngles(){return convertAngles(currentPosition);};
  
  virtual int getMoveFlag(int imod)=0;

  int getCurrentPositionIndex() {return currentPositionIndex;};
  int getNumberOfPositions() {return *numberOfPositions;};

 protected:
    
  
   int *numberOfPositions;
   float *detPositions;
      
   char *angConvFile;
   
   float *binSize;
   float *fineOffset;
   float *globalOffset;
   int *angDirection;
   int *moveFlag;


   int nBins;
   
   

  /**
     current position of the detector
  */
  float currentPosition;
  
  /**
     current position index of the detector
  */
  int currentPositionIndex;
  
  virtual int setAngularCorrectionMask(int i=-1)=0;
  

 private:
  /** merging bins */
  float *mergingBins;
  
  /** merging counts */
  float *mergingCounts;
  
  /** merging errors */
  float *mergingErrors;
  
  /** merging multiplicity */
  int *mergingMultiplicity;
  
  /** pointer to the angular conversion constants for the (multi)detector class*/

/*   angleConversionConstant *angOff[MAXDET]; */
/*   int *nMods[MAXDET]; */
/*   int nCh[MAXDET]; */



};

#endif
