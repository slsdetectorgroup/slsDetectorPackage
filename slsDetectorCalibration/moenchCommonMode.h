#ifndef MOENCHCOMMONMODE_H
#define MOENCHCOMMONMODE_H

#include "commonModeSubtraction.h"

class moenchCommonMode : public commonModeSubtraction {

 public:
  moenchCommonMode(int nn=1000) : commonModeSubtraction(nn,4){} ;
   

    virtual void addToCommonMode(double val, int ix=0, int iy=0) {
      (void)iy;
      int isc=ix/40;
      if (isc>=0 && isc<4) {
	cmPed[isc]+=val; 
	nCm[isc]++;
      }

    };
    
    virtual double getCommonMode(int ix=0, int iy=0) {
      (void)iy;
      int isc=ix/40;
      if (isc>=0 && isc<4) {
	if (nCm[isc]>0) return cmPed[isc]/nCm[isc]-cmStat[isc].Mean(); 
      }
      return 0;};


};


#endif
