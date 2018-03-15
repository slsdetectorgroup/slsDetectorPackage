#ifndef PEDESTALSUBTRACTION_H
#define PEDESTALSUBTRACTION_H

#include "MovingStat.h"

class pedestalSubtraction  {
  /** @short class defining the pedestal subtraction based on an approximated moving average */
 public:
  /** constructor 
      \param nn number of samples to calculate the moving average (defaults to 1000)
  */
  pedestalSubtraction (int nn=1000) : stat(nn) {};
    
    /** virtual destructorr 
     */
    virtual ~pedestalSubtraction() {};
    
    /** clears the moving average */
    virtual void Clear() {stat.Clear();}
    
    /** adds the element to the moving average 
	\param val value to be added
    */
    virtual void addToPedestal(double val){stat.Calc(val);};

  /** returns the average value of the pedestal
      \returns mean of the moving average
    */
    virtual double getPedestal(){return stat.Mean();};

  /** returns the standard deviation of the moving average
      \returns standard deviation of the moving average
    */
    virtual double getPedestalRMS(){return stat.StandardDeviation();};

  /**sets/gets the number of samples for the moving average
      \param i number of elements for the moving average. If -1 (default) or negative, gets.
      \returns actual number of samples for the moving average
    */
    virtual int SetNPedestals(int i=-1) {return stat.SetN(i);};

  /**sets/gets the number of samples for the moving average
      \returns actual number of samples for the moving average
    */
    virtual int GetNPedestals() {return stat.GetN();};
    
    /** sets the moving average */
    virtual void setPedestal(double val, double rms=0, int m=-1) {stat.Set(val, rms, m);}

  
    /** sets the moving average */
    virtual void setPedestalRMS(double rms) {stat.SetRMS(rms);}

    /**sets/gets the number of samples for the moving average
      \returns actual number of samples for the moving average
    */
    virtual int getNumpedestals() {return stat.NumDataValues();};

 private:
    MovingStat stat; /**< approximated moving average struct */

};
#endif
