#ifndef GOTTHARDDOUBLECOMMONMODESUBTRACTION_H
#define GOTTHARDDOUBLECOMMONMODESUBTRACTION_H


#include "commonModeSubtractionNew.h"

class gotthardDoubleModuleCommonModeSubtraction : public commonModeSubtraction {

  /** @short class to calculate the common mode of the pedestals based on an approximated moving average*/ 

 public:
  
  /** constructor
      \param nn number of samples for the moving average to calculate the average common mode
      \param iroi number of regions on which one can calculate the common mode separately. Defaults to 1 i.e. whole detector

  */
 gotthardDoubleModuleCommonModeSubtraction(int ns=3) : commonModeSubtraction(2, ns) {};

    /** 
	gets the common mode ROI for pixel ix, iy 
     */
  virtual int getROI(int ix, int iy){return ix%2;};


};



#endif
