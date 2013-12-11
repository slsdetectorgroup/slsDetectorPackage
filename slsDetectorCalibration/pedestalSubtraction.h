#ifndef PEDESTALSUBTRACTION_H
#define PEDESTALSUBTRACTION_H

#include "MovingStat.h"

class pedestalSubtraction  {

 public:
  pedestalSubtraction (int nn=1000) : stat(nn) {};
    virtual ~pedestalSubtraction() {};
    
    virtual void Clear() {stat.Clear();}
    virtual void addToPedestal(double val){ stat.Calc(val);};
    virtual double getPedestal(){return stat.Mean();};
    virtual double getPedestalRMS(){return stat.StandardDeviation();};
    virtual int SetNPedestals(int i=-1) {if (i>0) stat.SetN(i); return stat.GetN();};
    

  
 private:
  MovingStat stat;

};
#endif
