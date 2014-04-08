#ifndef COMMONMODESUBTRACTION_H
#define COMMONMODESUBTRACTION_H

#include "MovingStat.h"




class commonModeSubtraction {

  /** @short class to calculate the common mode of the pedestals based on an approximated moving average*/ 

 public:
  
  /** constructor
      \param nn number of samples for the moving average to calculate the average common mode
      \param iroi number of regions on which one can calculate the common mode separately. Defaults to 1 i.e. whole detector

  */
  commonModeSubtraction(int nn=1000, int iroi=1) : cmStat(NULL), cmPed(NULL), nCm(NULL), nROI(iroi)  {cmStat=new MovingStat[nROI]; for (int i=0; i<nROI; i++) cmStat[i].SetN(nn); cmPed=new double[nROI]; nCm=new double[nROI];};

    /** destructor - deletes the moving average(s) and the sum of pedestals calculator(s) */
    virtual ~commonModeSubtraction() {delete [] cmStat; delete [] cmPed; delete [] nCm;};
    

    /** clears the moving average and the sum of pedestals calculation - virtual func*/
    virtual void  Clear(){
      for (int i=0; i<nROI; i++) {
	cmStat[i].Clear();  
	nCm[i]=0; 
	cmPed[i]=0;
      }};
    
    /** adds the average of pedestals to the moving average and reinitializes the calculation of the sum of pedestals for all ROIs. - virtual func*/
    virtual void  newFrame(){
      for (int i=0; i<nROI; i++) {
	if (nCm[i]>0) cmStat[i].Calc(cmPed[i]/nCm[i]);  
	nCm[i]=0; 
	cmPed[i]=0;
      }};
    
    /** adds the pixel to the sum of pedestals -- virtual func must be overloaded to define the regions of interest
	\param val value to add
	\param ix pixel x coordinate
	\param iy pixel y coordinate
    */
    virtual void addToCommonMode(double val, int ix=0, int iy=0) {
      (void) ix; (void) iy;
      
      //if (isc>=0 && isc<nROI) {
      cmPed[0]+=val; 
      nCm[0]++;//}
    };

    /** gets the common mode i.e. the difference between the current average sum of pedestals mode and the average pedestal -- virtual func must be overloaded to define the regions of interest
	\param ix pixel x coordinate
	\param iy pixel y coordinate
	\return the difference  between the current average sum of pedestals and the average pedestal
    */
    virtual double getCommonMode(int ix=0, int iy=0) {
      (void) ix; (void) iy;
      if (nCm[0]>0) return cmPed[0]/nCm[0]-cmStat[0].Mean(); 
      return 0;};
    




 protected:
  MovingStat *cmStat; /**<array of moving average of the pedestal average per region of interest */
  double *cmPed; /**< array storing the sum of pedestals per region of interest */
  double *nCm; /**< array storing the number of pixels currently contributing to the pedestals */
  const int nROI; /**< constant parameter for number of regions on which the common mode should be calculated separately e.g. supercolumns */


};





#endif
