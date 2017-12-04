
#ifndef ANGULARCALIBRATION_H
#define ANGULARCALIBRATION_H


//#include "usersFunctions.h"


#ifdef ROOT
#include <TROOT.h>
#include <TF1.h>
class TH1;
#endif

  //double angle(int ichan, double encoder, double totalOffset, double conv_r, double center, double offset, double tilt, int direction)



class angularCalibration {

 public:
  angularCalibration(int nm=48);
  ~angularCalibration();
  
  /**
     sets the angular direction of the detector
     \par d 1 or -1 set the angular direction, other valuse simply get
     \returns the angular direction of the detector
  */
  int setDirection(int d=0){if (d==-1 || d==1) direction=d; return direction;};

  /** 
      sets the encoder position
      \param f encoder position to be set
      \returns current encoder position
  */
  double setEncoder(double f) {encoder=f; return encoder;};

  /** 
      gets the encoder position
      \returns encoder position
  */
  double getEncoder() {return encoder;};

  /** 
      sets the totalOffset of the detector
      \param f total offset to be set
      \returns current total offset
  */
  double setTotalOffset(double f) {totalOffset=f; return totalOffset;};

  /** 
      gets the encoder position
      \returns encoder position
  */
  double getTotalOffset() {return totalOffset;};




  /**
     sets the angular range for peak fitting
     \param mi minimum of the angular range
     \param ma maximum of the angular range
  */
  void setAngularRange(double mi, double ma){ang_min=mi; ang_max=ma;};


  /**
     gets the angular range for peak fitting
     \param mi reference to the minimum of the angular range
     \param ma reference to the maximum of the angular range
  */
  void getAngularRange(double &mi, double &ma){mi=ang_min; ma=ang_max;};


  /** sets and returns the number of modules
      \param nm number of modules to be set (<0 gets)
      \return current number of modules
  */
  int setNumberOfModules(int nm=-1) {if (nm>=0) nmod=nm; return nmod;};

  /** sets and returns the number of channels per module
      \param n number of channels per module to be set (<0 gets)
      \return current number of channels per module
  */
  int setChannelsPerModule(int n=-1) {if (n>0) nchmod=n; return nchmod;};

  angleConversionConstant *getAngularConversionConstant(int imod=0);
  angleConversionConstant *setAngularConversionConstant(angleConversionConstant *a, int imod=0);


#ifdef ROOT

  /**
     Gaussian with pedestal describing a peak
     par[0] is the heigh of the pean
     par[1] is the peak position
     par[2] is the peak width
     par[3] is the background offset
     par[4] is the background slope
  */
  Double_t peakFunction(Double_t *x, Double_t *par); 


  /**
     Angular conversion function 
     par[0] is the module center
     par[1] is the conversion radius (pitch/radius)
     par[2] is the module offset
  */
  Double_t angleFunction(Double_t *x, Double_t *par);

  /**
     Fits a peak for the angular calibration 
     \param h histogram channels versus intensity
     \returns fitted function or NULL if fit failed
  */
  TF1 *fitPeak(TH1 *h);

#endif
  

 private:
  
  int direction; /**< angular direction of the detector -can be +1 or -1 */

#ifdef ROOT
  TF1 *fpeak; /**< Root function based on function peakFunction */
  
  TF1 *fangle; /**< Root function based on function angleFunction */
  
#endif
  double encoder; /**< position of the detector encoder */
  double totalOffset; /**< total offset of the detector */
  double ang_min; /**< minimum of the angular range for peak fitting*/
  double ang_max; /**< maximum of the angular range for peak fitting */
   
  int nmod;
  int nchmod;

  angleConversionConstant angConv[MAXMOD*MAXDET];

  



/* void fitangle(char fname[80],char extension[10], int start, int stop, double startangle, double stopangle);  //fits all datasets and extracts the constants */
/* int fitpeak(char fname[80],char extension[10], int nr, double minang, double maxang); // fits a peak from a pattern using nominal calibration constant */


};

#endif
