
#ifndef ANGULARCONVERSIONSTATIC_H
#define ANGULARCONVERSIONSTATIC_H

#ifdef __CINT
#define MYROOT
#endif




#include <string>
#include <fstream>

//#include "angleConversionConstant.h"


  //double angle(int ichan, double encoder, double totalOffset, double conv_r, double center, double offset, double tilt, int direction)

class angleConversionConstant;

;

/**
   @short Angular conversion constants needed  for a detector module
 */


/**

@short methods to set/unset the angular conversion and merge the data
class containing the methods to set/unset the angular conversion and merge the data


The angular conversion itself is defined by the angle() function defined in usersFunctions.cpp
   
*/
class angularConversionStatic
// : public virtual slsDetectorDefs
{

 public:
  /** default constructor */
  angularConversionStatic();
  /** virtual destructor */
  virtual ~angularConversionStatic();
 


  //virtual int readAngularConversion(std::string fname)=0;




  /**
   
      reads an angular conversion file
      \param fname file to be read
      \param nmod number of modules (maximum) to be read
      \param angOff pointer to array of angleConversionConstants
      \returns OK or FAIL
  */
   static int readAngularConversion(std::string fname, int nmod, angleConversionConstant *angOff); 
   
   /**
      reads an angular conversion file
      \param ifstream input file stream to be read
      \param nmod number of modules (maximum) to be read
      \param angOff pointer to array of angleConversionConstants
      \returns OK or FAIL
      
  */
   static int readAngularConversion(std::ifstream& ifs, int nmod, angleConversionConstant *angOff);
  /**
     writes an angular conversion file
      \param fname file to be written
      \param nmod number of modules to be written
      \param angOff pointer to array of angleConversionConstants
      \returns OK or FAIL
  */
   static int writeAngularConversion(std::string fname, int nmod, angleConversionConstant *angOff);

 /**
     writes an angular conversion file
      \param ofstream output file stream
      \param nmod number of modules to be written
      \param angOff pointer to array of angleConversionConstants
      \returns OK or FAIL
  */
   static int writeAngularConversion(std::ofstream& ofs, int nmod, angleConversionConstant *angOff);

   /** 
       sets the arrays of the merged data to 0. NB The array should be created with size nbins >= 360./getBinSize(); 
       \param mp already merged postions
       \param mv already merged data
       \param me already merged errors (squared sum)
       \param mm multiplicity of merged arrays
       \param nbins number of bins
       \returns OK or FAIL
   */
   static int resetMerging(double *mp, double *mv,double *me, int *mm, int nbins);

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

   static int  addToMerging(double *p1, double *v1, double *e1, double *mp, double *mv,double *me, int *mm, int nchans, double binsize,int nb, int *badChanMask=NULL);
  

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

   static int  addPointToMerging(double p1, double v1, double e1, double *mp, double *mv,double *me, int *mm, double binsize, int nb);
   

   /**
      calculates the "final" positions, data value and errors for the merged data
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \param nb number of bins
      \returns FAIL or the number of non empty bins (i.e. points belonging to the pattern)
  */

   static int finalizeMerging(double *mp, double *mv,double *me, int *mm, int nb);
 
  /**
     converts channel number to angle
     \param pos encoder position
     \returns array of angles corresponding to the channels
  */

  double* convertAngles(double pos, int nch, int *chansPerMod, angleConversionConstant **angOff, int *mF, double fo, double go, int angdir);

  double convertAngle(double pos, int ich, int *chansPerMod, angleConversionConstant **angOff, int *mF, double fo, double go, int angdir);


  double convertAngle(double pos, int ich, angleConversionConstant *angOff, int mF, double fo, double go, int angdir);



 protected:
    



  int registerAngleFunctionCallback(double (*fun)(double, double, double, double, double, double, double, int)) {angle = fun; return 0;};
  
  
  double (*angle)(double, double, double, double, double, double, double, int);


  // private:

  
};

#endif
