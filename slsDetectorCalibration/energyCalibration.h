#ifndef ENERGYCALIBRATION_H
#define ENERGYCALIBRATION_H

#ifdef __CINT__
#define MYROOT
#endif

#ifdef G__ROOT
#define MYROOT
#endif


#ifdef __MAKECINT__
#define MYROOT
#endif

#ifdef ROOT_VERSION
#define MYROOT
#endif

#define MYROOT

#ifdef MYROOT
#include <TROOT.h>
#include <TF1.h>
class TH1F;
class TH2F;
class TGraphErrors;
#endif


using namespace std;




const double conven=1000./3.6; /**< electrons/keV */
const double el=1.67E-4; /**< electron charge in fC */



/**
 \mainpage Common Root library for SLS detectors data analysis
 *
 * \section intro_sec Introduction
 We know very well s-curves etc. but at the end everybody uses different functions ;-).

 * \subsection mot_sec Motivation
 It would be greate to use everybody the same functions...

*/


/**
 * 
 *
@libdoc The energiCalibration class contains all the necessary functions for s-curve fitting and linear calibration of the threshold.
 *
 * @short Energy calibration functions
 * @author Anna Bergamaschi
 * @version 0.1alpha


 */

/** 
  class containing all the possible energy calibration functions (scurves with and without charge sharing, gaussian spectrum with and without charge sharing, possibility of chosing the sign of the X-axis)

*/
class energyCalibrationFunctions {

 public:
  
  energyCalibrationFunctions(int s=-1) {setScanSign(s);};
  
    /** sets scan sign
	\param s can be 1 (energy and x-axis have the same direction) or -1 (energy and x-axis have opposite directions) otherwise gets
	\returns current scan sign can be 1 (energy and x-axis have the same direction) or -1 (energy and x-axis have opposite directions)
    */
  int setScanSign(int s=0) {if (s==1 || s==-1) sign=s; return sign;};;
  

#ifdef MYROOT
  /** 
      Gaussian Function with charge sharing pedestal
      par[0] is the absolute height of the background pedestal
      par[1] is the slope of the background pedestal
  */
  Double_t pedestal(Double_t *x, Double_t *par);

  /** 
      Gaussian Function with charge sharing pedestal
      par[0] is the absolute height of the background pedestal
      par[1] is the slope of the background pedestal
      par[2] is the gaussian peak position
      par[3] is the RMS of the gaussian (and of the pedestal)
      par[4] is the height of the function
      par[5] is the fractional height of the charge sharing pedestal (scales with par[3])
  */
  Double_t gaussChargeSharing(Double_t *x, Double_t *par);
  /** 
      Gaussian Function with charge sharing pedestal
      par[0] is the absolute height of the background pedestal
      par[1] is the slope of the background pedestal
      par[2] is the gaussian peak position
      par[3] is the RMS of the gaussian (and of the pedestal)
      par[4] is the height of the function
      par[5] is the fractional height of the charge sharing pedestal (scales with par[3])
  */
  Double_t gaussChargeSharingPixel(Double_t *x, Double_t *par);

  /** 
      Basic erf function
      par[0] is the inflection point
      par[1] is the RMS 
      par[2] is the amplitude
  */
Double_t erfFunction(Double_t *x, Double_t *par) ;
 Double_t erfBox(Double_t *z, Double_t *par);
  /** Erf function with charge sharing slope
      par[0] is the  pedestal 
      par[1] is the  slope of the pedestal
      par[2] is the inflection point
      par[3] is the RMS 
      par[4] is the amplitude
      par[5] is the angual coefficient of the charge sharing slope (scales with par[3])
  */
Double_t erfFunctionChargeSharing(Double_t *x, Double_t *par);
  
  /** Double Erf function with charge sharing slope
      par[0] is the  pedestal 
      par[1] is the  slope of the pedestal
      par[2] is the inflection point of the first energy
      par[3] is the RMS of the first energy
      par[4] is the amplitude of the first energy 
      par[5] is the angual coefficient of the charge sharing slope of the first energy (scales with par[3])
      par[6] is the inflection point of the second energy 
      par[7] is the RMS of the second energy 
      par[8] is the amplitude of the second energy 
      par[9] is the angual coefficient of the charge sharing slope  of the second energy (scales with par[8])
  */

Double_t erfFuncFluo(Double_t *x, Double_t *par);

  
  /** 
      static function Gaussian with charge sharing pedestal with the correct scan sign
      par[0] is the absolute height of the background pedestal
      par[1]  is the  slope of the pedestal 
      par[2] is the gaussian peak position
      par[3] is the RMS of the gaussian (and of the pedestal)
      par[4] is the height of the function
      par[5] is the fractional height of the charge sharing pedestal (scales with par[4]
  */
  Double_t spectrum(Double_t *x, Double_t *par);

  /** 
      static function Gaussian with charge sharing pedestal with the correct scan sign
      par[0] is the absolute height of the background pedestal
      par[1]  is the  slope of the pedestal 
      par[2] is the gaussian peak position
      par[3] is the RMS of the gaussian (and of the pedestal)
      par[4] is the height of the function
      par[5] is the fractional height of the charge sharing pedestal (scales with par[4]
  */
  Double_t spectrumPixel(Double_t *x, Double_t *par);


 /** Erf function with charge sharing slope with the correct scan sign
      par[0] is the  pedestal 
      par[1] is the  slope of the pedestal
      par[2] is the inflection point
      par[3] is the RMS 
      par[4] is the amplitude
      par[5] is the angual coefficient of the charge sharing slope (scales with par[3])
  */
  Double_t scurve(Double_t *x, Double_t *par);



   /** Double Erf function with charge sharing slope
      par[0] is the  pedestal 
      par[1] is the  slope of the pedestal 
      par[2] is the inflection point of the first energy
      par[3] is the RMS of the first energy
      par[4] is the amplitude of the first energy 
      par[5] is the angual coefficient of the charge sharing slope of the first energy (scales with par[3]) 
      par[6] is the inflection point of the second energy 
      par[7] is the RMS of the second energy 
      par[8] is the amplitude of the second energy 
      par[9] is the angual coefficient of the charge sharing slope  of the second energy (scales with par[8]) 
  */
  Double_t scurveFluo(Double_t *x, Double_t *par);

#endif

/** Calculates the median of an array of n elements */
  static double median(double *x, int n);
/**  Calculates the median of an array of n elements  (swaps the arrays!)*/
  static int quick_select(int arr[], int n);
/** Calculates the median of an array of n elements (swaps the arrays!)*/
  static int kth_smallest(int *a, int n, int k);

 private:
  int sign;
    

};

/**
   class alowing the energy calibration of photon counting and anlogue detectors

*/

class energyCalibration  {


 public:
  /** 
      default constructor  -  creates the function with which the s-curves will be fitted
  */
  energyCalibration();
    
  /** 
      default destructor - deletes the function with which the s-curves will be fitted 
  */
  ~energyCalibration();
    
    /** sets plot flag
	\param p plot flag (-1 gets, 0 unsets, >0 plot)
	\returns current plot flag
    */
  int setPlotFlag(int p=-1) {if (p>=0) plot_flag=p; return plot_flag;};

    /** sets scan sign
	\param s can be 1 (energy and x-axis have the same direction) or -1 (energy and x-axis have opposite directions) otherwise gets
	\returns current scan sign can be 1 (energy and x-axis have the same direction) or -1 (energy and x-axis have opposite directions)
    */
  int setScanSign(int s=0) {return funcs->setScanSign(s);};
  
    /** sets plot flag
	\param p plot flag (-1 gets, 0 unsets, >0 plot)
	\returns current plot flag
    */
  int setChargeSharing(int p=-1);


  void fixParameter(int ip, Double_t val);

  void releaseParameter(int ip);

#ifdef MYROOT

  /**
     Creates an histogram with the median of nchannels starting from a specified one. the direction on which it is mediated can be selected (defaults to x=0)
     \param h2 2D histogram on which the median will be calculated
     \param ch0 starting channel
     \param nch number of channels to be mediated
     \param direction can be either 0 (x, default) or 1 (y)
     \returns a TH1F histogram with the X-axis as a clone of the h2 Y (if direction=0) or X (if direction=0) axis, and on the Y axis the median of the counts of the mediated channels f h2
   */
  static TH1F* createMedianHistogram(TH2F* h2, int ch0, int nch, int direction=0);


  /** sets the s-curve fit range 
      \param mi  minimum of the fit range (-1 is histogram x-min)
      \param ma  maximum of the fit range (-1 is histogram x-max)
  */
  void setFitRange(Double_t mi, Double_t ma){fit_min=mi; fit_max=ma;};

  /** gets the s-curve fit range 
      \param mi reference for minimum of the fit range (-1 is histogram x-min)
      \param ma reference for maximum of the fit range (-1 is histogram x-max)
  */
  void getFitRange(Double_t &mi, Double_t &ma){mi=fit_min; ma=fit_max;};


/** set start parameters for the s-curve function
    \param par parameters, -1 sets to auto-calculation
      par[0] is the  pedestal 
      par[1] is the  slope of the pedestal 
      par[2] is the inflection point
      par[3] is the RMS 
      par[4] is the amplitude
      par[5] is the angual coefficient of the charge sharing slope (scales with par[3]) -- always positive
  */
  void setStartParameters(Double_t *par);
  
/** get start parameters for the s-curve function
    \param par parameters, -1 means auto-calculated
      par[0] is the  pedestal 
      par[1] is the  slope of the pedestal 
      par[2] is the inflection point
      par[3] is the RMS 
      par[4] is the amplitude
      par[5] is the angual coefficient of the charge sharing slope (scales with par[3]) -- always positive
  */
  void getStartParameters(Double_t *par);

  /**
     fits histogram with the s-curve function
     \param h1 1d-histogram to be fitted
     \param mypar pointer to fit parameters array 
     \param emypar pointer to fit parameter errors
     \returns the fitted function - can be used e.g. to get the Chi2 or similar
  */
  TF1 *fitSCurve(TH1 *h1, Double_t *mypar, Double_t *emypar);


  /**
     fits histogram with the spectrum
     \param h1 1d-histogram to be fitted
     \param mypar pointer to fit parameters array 
     \param emypar pointer to fit parameter errors
     \returns the fitted function - can be used e.g. to get the Chi2 or similar
  */
  TF1 *fitSpectrum(TH1 *h1, Double_t *mypar, Double_t *emypar);


  /**
     fits histogram with the spectrum
     \param h1 1d-histogram to be fitted
     \param mypar pointer to fit parameters array 
     \param emypar pointer to fit parameter errors
     \returns the fitted function - can be used e.g. to get the Chi2 or similar
  */
  TF1 *fitSpectrumPixel(TH1 *h1, Double_t *mypar, Double_t *emypar);


  /**
     calculates gain and offset for the set of inflection points
     \param nscan number of energy scans
     \param en array of energies (nscan long)
     \param een array of errors on energies (nscan long) - can be NULL!
     \param fl array of inflection points (nscan long)
     \param efl array of errors on the inflection points (nscan long)
     \param gain reference to gain resulting from the fit
     \param off reference to offset resulting from the fit
     \param egain reference to error on the gain resulting from the fit
     \param eoff reference to the error on the offset resulting from the fit
     \returns graph  energy vs inflection point
  */
  TGraphErrors* linearCalibration(int nscan, Double_t *en, Double_t *een, Double_t *fl, Double_t *efl, Double_t &gain, Double_t &off, Double_t &egain, Double_t &eoff);  

  /**
     calculates gain and offset for the set of energy scans
     \param nscan number of energy scans
     \param en array of energies (nscan long)
     \param een array of errors on energies (nscan long) - can be NULL!
     \param h1 array of TH1
     \param gain reference to gain resulting from the fit
     \param off reference to offset resulting from the fit
     \param egain reference to error on the gain resulting from the fit
     \param eoff reference to the error on the offset resulting from the fit
     \returns graph  energy vs inflection point 
  */
  TGraphErrors* calibrateScurves(int nscan, Double_t *en, Double_t *een, TH1F **h1, Double_t &gain, Double_t &off, Double_t &egain, Double_t &eoff){return calibrate(nscan, en, een, h1, gain, off, egain, eoff, 1);};    

  /**
     calculates gain and offset for the set of energy spectra
     \param nscan number of energy scans
     \param en array of energies (nscan long)
     \param een array of errors on energies (nscan long) - can be NULL!
     \param h1 array of TH1
     \param gain reference to gain resulting from the fit
     \param off reference to offset resulting from the fit
     \param egain reference to error on the gain resulting from the fit
     \param eoff reference to the error on the offset resulting from the fit
     \returns graph  energy vs peak
  */
  TGraphErrors* calibrateSpectra(int nscan, Double_t *en, Double_t *een, TH1F **h1, Double_t &gain, Double_t &off, Double_t &egain, Double_t &eoff){return calibrate(nscan, en, een, h1, gain, off, egain, eoff, 0);};  


#endif
 private:

#ifdef MYROOT
    /**
     calculates gain and offset for the set of energies
     \param nscan number of energy scans
     \param en array of energies (nscan long)
     \param een array of errors on energies (nscan long) - can be NULL!
     \param h1 array of TH1
     \param gain reference to gain resulting from the fit
     \param off reference to offset resulting from the fit
     \param egain reference to error on the gain resulting from the fit
     \param eoff reference to the error on the offset resulting from the fit
     \param integral 1 is an s-curve set (default), 0 spectra
     \returns graph  energy vs peak/inflection point 
  */
  TGraphErrors* calibrate(int nscan, Double_t *en, Double_t *een, TH1F **h1, Double_t &gain, Double_t &off, Double_t &egain, Double_t &eoff, int integral=1);  


  /** 
      Initializes the start parameters and the range of the fit depending on the histogram characteristics and/or on the start parameters specified by the user 
  \param fun pointer to function to be initialized
  \param h1 histogram from which to extract the range and start parameters, if not already specified by the user
  
*/
  
  void initFitFunction(TF1 *fun, TH1 *h1);


  /**
     Performs the fit according to the flags specified and returns the fitted function
     \param fun function to fit
     \param h1 histogram to fit
     \param mypar pointer to fit parameters array 
     \param emypar pointer to fit parameter errors
     \returns the fitted function - can be used e.g. to get the Chi2 or similar
  */
  TF1 *fitFunction(TF1 *fun, TH1 *h1, Double_t *mypar, Double_t *emypar);  

#endif

#ifdef MYROOT
  Double_t fit_min; /**< minimum of the s-curve fitting range, -1 is histogram x-min */
  Double_t fit_max; /**< maximum of the s-curve fitting range, -1 is histogram x-max */
  
  Double_t bg_offset; /**< start value for the background pedestal */
  Double_t bg_slope; /**< start value for the background slope */
  Double_t flex; /**< start value for the inflection point */
  Double_t noise; /**< start value for the noise */
  Double_t ampl; /**< start value for the number of photons */
  Double_t cs_slope; /**< start value for the charge sharing slope */


  TF1 *fscurve; /**< function with which the s-curve will be fitted */

  TF1 *fspectrum; /**< function with which the spectrum will be fitted */

  TF1 *fspixel; /**< function with which the spectrum will be fitted */

#endif

  energyCalibrationFunctions *funcs;
  int plot_flag; /**< 0 does not plot, >0 plots (flags?) */
  
  int cs_flag; /**< 0 functions without charge sharing contribution, >0 with charge sharing contribution */

};

#endif



















