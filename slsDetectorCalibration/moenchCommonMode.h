#ifndef MOENCHCOMMONMODE_H
#define MOENCHCOMMONMODE_H

#include "commonModeSubtraction.h"

class moenchCommonMode : public commonModeSubtraction {
  /** @short class to calculate the common mode noise for moench02 i.e. on 4 supercolumns separately */
 public:
  /** constructor -  initalizes a commonModeSubtraction with 4 different regions of interest 
      \param nn number of samples for the moving average
  */
  moenchCommonMode(int nn=1000) : commonModeSubtraction(nn,4){} ;
    

    /** add value to common mode as a function of the pixel value, subdividing the region of interest in the 4 supercolumns of 40 columns each;
	\param val value to add to the common mode
	\param ix pixel coordinate in the x direction
	\param iy pixel coordinate in the y direction
    */
    virtual void addToCommonMode(double val, int ix=0, int iy=0) {
      (void) iy;
      commonModeSubtraction::addToCommonMode(val, ix/40);
    };
     /**returns common mode value as a function of the pixel value, subdividing the region of interest in the 4 supercolumns of 40 columns each;
	\param ix pixel coordinate in the x direction
	\param iy pixel coordinate in the y direction
	\returns common mode value
    */
    virtual double getCommonMode(int ix=0, int iy=0) {
      (void) iy;
      return commonModeSubtraction::getCommonMode(ix/40);
    };

};


#endif
