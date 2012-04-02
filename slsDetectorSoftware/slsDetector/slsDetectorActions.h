#ifndef SLS_DETECTOR_ACTIONS_H
#define SLS_DETECTOR_ACTIONS_H



#include "sls_detector_defs.h"
#include <string>
#include <fstream>



#define MAX_SCAN_LEVELS 2

using namespace std;

/**

class implementing the script and scan utilities of the detectors


*/

class slsDetectorActions : public slsDetectorDefs {
 public :
  slsDetectorActions(){};
  virtual ~slsDetectorActions(){};

  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \param par for script 
      \returns 0 if action disabled, >0 otherwise
  */
  int setAction(int iaction, string fname="", string par="");

  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionScript(int iaction, string fname="");
  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param par for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionParameter(int iaction, string par="");

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
  string getActionScript(int iaction);

    /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
  string getActionParameter(int iaction);

   /** 
	returns action mode
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action mode
    */
  int getActionMode(int iaction);


  /** 
      set scan 
      \param index of the scan (0,1)
      \param fname for script ("" disables, "none" disables and overwrites current)
      \param nvalues number of steps (0 disables, -1 leaves current value)
      \param values pointer to steps (if NULL leaves current values)
      \param par parameter for the scan script ("" leaves unchanged)
      \returns 0 is scan disabled, >0 otherwise
  */
  int setScan(int index, string script="", int nvalues=-1, float *values=NULL, string par="", int precision=-1);
  
  int setScanScript(int index, string script="");
  int setScanParameter(int index, string par="");
  int setScanPrecision(int index, int precision=-1);
  int setScanSteps(int index, int nvalues=-1, float *values=NULL);
  float getScanStep(int index, int istep){if (index<MAX_SCAN_LEVELS && istep<MAX_SCAN_STEPS) return scanSteps[index][istep]; else return -1;};
  /** 
      returns scan script
      \param iscan can be (0,1) 
      \returns scan script
  */
  string getScanScript(int iscan);

    /** 
	returns scan parameter
	\param iscan can be (0,1)
	\returns scan parameter
    */
  string getScanParameter(int iscan);

   /** 
	returns scan mode
	\param iscan can be (0,1)
	\returns scan mode
    */
  int getScanMode(int iscan);

   /** 
	returns scan steps
	\param iscan can be (0,1)
	\param v is the pointer to the scan steps
	\returns scan steps
    */
  int getScanSteps(int iscan, float *v=NULL);


   /** 
	returns scan precision
	\param iscan can be (0,1)
	\returns scan precision
    */
  int getScanPrecision(int iscan);


  virtual int setTotalProgress()=0;

  int getActionMask() {if (actionMask) return *actionMask; return 0;};

  float getCurrentScanVariable(int index) {return currentScanVariable[index];};
  //  int getScanPrecision(int index) {return scanPrecision[index];};

 protected:

  
  int *actionMask;  	   
  mystring *actionScript;	      
  mystring *actionParameter; 	      

  int *nScanSteps;		      
  mysteps *scanSteps;	 	    
  int *scanMode;		    
  int *scanPrecision;
  mystring *scanScript; 
  mystring *scanParameter; 






  /**
     current scan variable of the detector
  */
  float currentScanVariable[MAX_SCAN_LEVELS];
  
  /**
     current scan variable index of the detector
  */
  int currentScanIndex[MAX_SCAN_LEVELS];
  








};
#endif
