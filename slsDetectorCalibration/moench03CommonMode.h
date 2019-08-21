#ifndef MOENCH03COMMONMODE_H
#define MOENCH03COMMONMODE_H

#include "commonModeSubtractionNew.h"

class commonModeSubtractionColumn:  public commonModeSubtraction{
public:
 commonModeSubtractionColumn(int nr=200) : commonModeSubtraction(800), rows(nr) {};
  virtual int getROI(int ix, int iy){return ix+(iy/200)*400;};
  
   virtual void addToCommonMode(double val, int ix=0, int iy=0) {
     if (ix<rows || ix>399-rows) {
       int iroi=getROI(ix,iy);
       if (iroi>=0 && iroi<nROI) {
	 mean[iroi]+=val; 
	 mean2[iroi]+=val*val; 
	 nCm[iroi]++;
       }
     }
    };

 private:
  int rows;
};


class commonModeSubtractionSuperColumn:  public commonModeSubtraction{
public:
commonModeSubtractionSuperColumn() : commonModeSubtraction(32) {};
  virtual int getROI(int ix, int iy){  return ix/25+(iy/200)*16;};
};


class commonModeSubtractionHalf:  public commonModeSubtraction{
public:
commonModeSubtractionHalf() : commonModeSubtraction(2) {};
  virtual int getROI(int ix, int iy){  (void) ix; return iy/200;};
};


class moench03CommonMode : public commonModeSubtractionColumn {
  /** @short class to calculate the common mode noise for moench02 i.e. on 4 supercolumns separately */
 public:
  /** constructor -  initalizes a commonModeSubtraction with 4 different regions of interest 
      \param nn number of samples for the moving average
  */
  moench03CommonMode(int nr=20) : commonModeSubtractionColumn(nr){} ;
    


};


#endif
