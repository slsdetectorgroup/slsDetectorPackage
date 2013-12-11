#ifndef COMMONMODESUBTRACTION_H
#define COMMONMODESUBTRACTION_H

#include "MovingStat.h"

class commonModeSubtraction {

 public:
  commonModeSubtraction(int nn=1000, int iroi=1) : cmStat(NULL), cmPed(NULL), nCm(NULL), nROI(iroi)  {cmStat=new MovingStat[nROI]; for (int i=0; i<nROI; i++) cmStat[i].SetN(nn); cmPed=new double[nROI]; nCm=new double[nROI];};
    virtual ~commonModeSubtraction() {delete [] cmStat; delete [] cmPed; delete [] nCm;};
    
    virtual void  Clear(){
      for (int i=0; i<nROI; i++) {
	cmStat[i].Clear();  
	nCm[i]=0; 
	cmPed[i]=0;
      }};
    
    virtual void  newFrame(){
      for (int i=0; i<nROI; i++) {
	if (nCm[i]>0) cmStat[i].Calc(cmPed[i]/nCm[i]);  
	nCm[i]=0; 
	cmPed[i]=0;
      }};
    
    virtual void addToCommonMode(double val, int ix=0, int iy=0) {
      (void)ix; (void)iy;
      cmPed[0]+=val; 
      nCm[0]++;};
    
    virtual double getCommonMode(int ix=0, int iy=0) {
      (void)ix; (void)iy;
      if (nCm[0]>0) return cmPed[0]/nCm[0]-cmStat[0].Mean(); 
      else return 0;};
    




 protected:
  MovingStat *cmStat; //stores the common mode average per supercolumn */
  double *cmPed; // stores the common mode for this frame per supercolumn */
  double *nCm; // stores the number of pedestals to calculate the cm per supercolumn */
  const int nROI;


};





#endif
