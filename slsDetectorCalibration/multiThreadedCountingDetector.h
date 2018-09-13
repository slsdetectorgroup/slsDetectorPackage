#ifndef MULTITHREADED_COUNTING_DETECTOR_H
#define MULTITHREADED_COUNTING_DETECTOR_H


#include "singlePhotonDetector.h"
#include "multiThreadedAnalogDetector.h"
//#include <mutex>


using namespace std;


class threadedCountingDetector : public threadedAnalogDetector
{
public:
 threadedCountingDetector(singlePhotonDetector *d, int fs=10000) : threadedAnalogDetector(d,fs) {};
  

  virtual double setNSigma(double n) {return ((singlePhotonDetector*)det)->setNSigma(n);};
  virtual void setEnergyRange(double emi, double ema) {return ((singlePhotonDetector*)det)->setEnergyRange(emi,ema);};


};



class multiThreadedCountingDetector : public multiThreadedAnalogDetector
{
public:
 multiThreadedCountingDetector(singlePhotonDetector *d, int n, int fs=1000) : multiThreadedAnalogDetector(d,n,fs) { };

  virtual double setNSigma(double n) {double ret; for (int i=0; i<nThreads; i++) ret=((threadedCountingDetector*)dets[i])->setNSigma(n); return ret;};
  virtual void setEnergyRange(double emi, double ema) {for (int i=0; i<nThreads; i++) ((threadedCountingDetector*)dets[i])->setEnergyRange(emi,ema);};
  
};

#endif


















