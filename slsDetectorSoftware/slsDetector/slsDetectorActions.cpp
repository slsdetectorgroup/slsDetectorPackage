#include "slsDetectorActions.h"
#include <iostream>
using namespace std;


  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable but leaves script unchanged, "none" disables and overwrites)
      \returns 0 if action disabled, >0 otherwise
  */
int slsDetectorActions::setAction(int iaction, string fname, string par) {

  int am;


  if (iaction>=0 && iaction<MAX_ACTIONS) {

    if (fname=="") {
      am=0;
    } else if (fname=="none") {
      am=0;
      strcpy(actionScript[iaction],fname.c_str());
    } else {
      strcpy(actionScript[iaction],fname.c_str());
      am=1;
    }
    
    if (par!="") {
      strcpy(actionParameter[iaction],par.c_str());
    }
    
    if (am) {
      
#ifdef VERBOSE
      cout << iaction << "  " << hex << (1 << iaction) << " " << *actionMask << dec;
#endif
      
      *actionMask |= (1 << iaction);
      
#ifdef VERBOSE
      cout << " set " << hex << *actionMask << dec << endl;
#endif
      
    } else {
#ifdef VERBOSE
    cout << iaction << "  " << hex << *actionMask << dec;
#endif
    
    *actionMask &= ~(1 << iaction);
    
#ifdef VERBOSE
    cout << "  unset " << hex << *actionMask << dec << endl;
#endif
    }
#ifdef VERBOSE
    cout << iaction << " Action mask set to " << hex << *actionMask << dec << endl;
#endif
    
    return am; 
  } else
    return -1;
}


int slsDetectorActions::setActionScript(int iaction, string fname) {
#ifdef VERBOSE
  
#endif
  return setAction(iaction,fname,"");
}



int slsDetectorActions::setActionParameter(int iaction, string par) {
  int am;

  if (iaction>=0 && iaction<MAX_ACTIONS) {
    am= 1& ( (*actionMask) << iaction);

    if (par!="") {
      strcpy(actionParameter[iaction],par.c_str());
    }
    
    if ((*actionMask) & (1 << iaction))
      return 1; 
    else
      return 0;
  } else
    return -1; 
}

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
string slsDetectorActions::getActionScript(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) 
    return string(actionScript[iaction]);
  else
    return string("wrong index");
};

    /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
string slsDetectorActions::getActionParameter(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) 
    return string(actionParameter[iaction]);
  else
    return string("wrong index");
}

   /** 
	returns action mode
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action mode
    */
int slsDetectorActions::getActionMode(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) {

    if ((*actionMask) & (1 << iaction))
      return 1; 
    else
      return 0;
  } else {
#ifdef VERBOSE
    cout << "slsDetetctor : wrong action index " << iaction <<  endl;
#endif
    return -1;
  }
}


  /** 
      set scan 
      \param index of the scan (0,1)
      \param fname for script ("" disable)
      \returns 0 if scan disabled, >0 otherwise
  */
int slsDetectorActions::setScan(int iscan, string script, int nvalues, float *values, string par, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {

    if (script=="") {
      scanMode[iscan]=0;
    } else {
      strcpy(scanScript[iscan],script.c_str());
      if (script=="none") {
	scanMode[iscan]=0;
      } else if (script=="energy") {
	scanMode[iscan]=1;
      }  else if (script=="threshold") {
	scanMode[iscan]=2;
      } else if (script=="trimbits") {
	scanMode[iscan]=3;
      } else {
	scanMode[iscan]=4;
      }  
    }
    
  

    

    
    if (par!="")
      strcpy(scanParameter[iscan],par.c_str());
      
      if (nvalues>=0) {    
	if (nvalues==0)
	  scanMode[iscan]=0;
	else {
	  nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values && 	scanMode[iscan]>0 ) {
	for (int iv=0; iv<nScanSteps[iscan]; iv++) {
	  scanSteps[iscan][iv]=values[iv];
	}
      }

      if (precision>=0)
	scanPrecision[iscan]=precision;
      
      if (scanMode[iscan]>0){
	*actionMask |= 1<< (iscan+MAX_ACTIONS);
      } else {
	*actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }



      setTotalProgress();










      return scanMode[iscan];
  }  else 
    return -1;
  
}

int slsDetectorActions::setScanScript(int iscan, string script) {
 if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (script=="") {
      scanMode[iscan]=0;
    } else {
      strcpy(scanScript[iscan],script.c_str());
      if (script=="none") {
	scanMode[iscan]=0;
      } else if (script=="energy") {
	scanMode[iscan]=1;
      }  else if (script=="threshold") {
	scanMode[iscan]=2;
      } else if (script=="trimbits") {
	scanMode[iscan]=3;
      } else {
	scanMode[iscan]=4;
      }  
    }
    
    if (scanMode[iscan]>0){
      *actionMask |= (1 << (iscan+MAX_ACTIONS));
    } else {
      *actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
    }

    setTotalProgress();
    
#ifdef VERBOSE
      cout << "Action mask is " << hex << actionMask << dec << endl;
#endif
    return scanMode[iscan];   
 } else 
   return -1;
}



int slsDetectorActions::setScanParameter(int iscan, string par) {


  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (par!="")
	strcpy(scanParameter[iscan],par.c_str());
      return scanMode[iscan];
  } else
    return -1;

}


int slsDetectorActions::setScanPrecision(int iscan, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (precision>=0)
      scanPrecision[iscan]=precision;
    return scanMode[iscan];
  } else
    return -1;

}

int slsDetectorActions::setScanSteps(int iscan, int nvalues, float *values) {

  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
  
      if (nvalues>=0) {    
	if (nvalues==0)
	  scanMode[iscan]=0;
	else {
	  nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values) {
	for (int iv=0; iv<nScanSteps[iscan]; iv++) {
	  scanSteps[iscan][iv]=values[iv];
	}
      }
      
      if (scanMode[iscan]>0){
	*actionMask |= (1 << (iscan+MAX_ACTIONS));
      } else {
	*actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }
      
#ifdef VERBOSE
      cout << "Action mask is " << hex << actionMask << dec << endl;
#endif
      setTotalProgress();




      return scanMode[iscan];
  

  } else 
      return -1;
  
  
  
  
}



  /** 
      returns scan script
      \param iscan can be (0,1) 
      \returns scan script
  */
string slsDetectorActions::getScanScript(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (scanMode[iscan])
	return string(scanScript[iscan]);
      else
	return string("none");
  } else
      return string("wrong index");
      
};

    /** 
	returns scan parameter
	\param iscan can be (0,1)
	\returns scan parameter
    */
string slsDetectorActions::getScanParameter(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (scanMode[iscan])
      return string(scanParameter[iscan]);
    else
      return string("none");
  }   else
      return string("wrong index");
}


   /** 
	returns scan mode
	\param iscan can be (0,1)
	\returns scan mode
    */
int slsDetectorActions::getScanMode(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS)
    return scanMode[iscan];
  else
    return -1;
}


   /** 
	returns scan steps
	\param iscan can be (0,1)
	\param v is the pointer to the scan steps
	\returns scan steps
    */
int slsDetectorActions::getScanSteps(int iscan, float *v) {

  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (v) {
      for (int iv=0; iv<nScanSteps[iscan]; iv++) {
	v[iv]=scanSteps[iscan][iv];
      }
    }


    setTotalProgress();


    if (scanMode[iscan])
      return nScanSteps[iscan];
    else
      return 0;
  } else
    return -1;
}


int slsDetectorActions::getScanPrecision(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    return scanPrecision[iscan];
  } else
    return -1;
}


