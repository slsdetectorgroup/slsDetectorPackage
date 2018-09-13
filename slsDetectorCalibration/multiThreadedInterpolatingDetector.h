#ifndef MULTITHREADED_INTERPOLATING_DETECTOR_H
#define MULTITHREADED_INTERPOLATING_DETECTOR_H


#include "interpolatingDetector.h"
#include "multiThreadedCountingDetector.h"
//#include <mutex>


using namespace std;


class threadedInterpolatingDetector : public threadedCountingDetector
{
public:
 threadedInterpolatingDetector(interpolatingDetector *d, int fs=10000) : threadedCountingDetector(d,fs) {};
    
  virtual void prepareInterpolation(int &ok){
      slsInterpolation *interp=(slsInterpolation*)((interpolatingDetector*)det)->getInterpolation();
      if (interp) 
       interp->prepareInterpolation(ok);
   }

   virtual int *getFlatField(){
     slsInterpolation *interp=(slsInterpolation*)((interpolatingDetector*)det)->getInterpolation();
     if (interp) 
       return interp->getFlatField();
     else
       return NULL;
   }
   
   virtual int *setFlatField(int *ff, int nb, double emin, double emax){
     slsInterpolation *interp=(slsInterpolation*)((interpolatingDetector*)det)->getInterpolation();
     if (interp) 
       return interp->setFlatField(ff, nb, emin, emax);
     else
       return NULL;
   }

   void *writeFlatField(const char * imgname) {

     slsInterpolation *interp=(slsInterpolation*)((interpolatingDetector*)det)->getInterpolation();
     if (interp)  
       interp->writeFlatField(imgname);

   }
   void *readFlatField(const char * imgname, int nb=-1, double emin=1, double emax=0){
     slsInterpolation *interp=(slsInterpolation*)((interpolatingDetector*)det)->getInterpolation();
     if (interp)  
       interp->readFlatField(imgname, nb, emin, emax);
   }

   virtual int *getFlatField(int &nb, double emi, double ema){
     slsInterpolation *interp=(slsInterpolation*)((interpolatingDetector*)det)->getInterpolation();
     int *ff=NULL;
     if (interp) { 
       ff=interp->getFlatField(nb,emi,ema);
     }     
     return ff;
   }
   
   virtual char *getInterpolation() {
     return ((interpolatingDetector*)det)->getInterpolation();
   }
   
   virtual void resetFlatField() { ((interpolatingDetector*)det)->resetFlatField();};


   virtual int setNSubPixels(int ns)  { return ((interpolatingDetector*)det)->setNSubPixels(ns);};



};



class multiThreadedInterpolatingDetector : public multiThreadedCountingDetector
{
public:
 multiThreadedInterpolatingDetector(interpolatingDetector *d, int n, int fs=1000) : multiThreadedCountingDetector(d,n,fs) { };

  virtual void prepareInterpolation(int &ok){
    /* getFlatField(); //sum up all etas */
    /* setFlatField(); //set etas to all detectors */
    /* for (int i=0; i<nThreads; i++) { */
       ((threadedInterpolatingDetector*)dets[0])->prepareInterpolation(ok);
       //  }
  }
   
   virtual int *getFlatField(){
     return ((threadedInterpolatingDetector*)dets[0])->getFlatField();
   }
   
   virtual int *getFlatField(int &nb, double emi, double ema){
     return ((threadedInterpolatingDetector*)dets[0])->getFlatField(nb,emi,ema);
   }
   
   virtual int *setFlatField(int *h=NULL, int nb=-1, double emin=1, double emax=0){
     return ((threadedInterpolatingDetector*)dets[0])->setFlatField(h,nb,emin,emax);
   }; 

   void *writeFlatField(const char * imgname){
     ((threadedInterpolatingDetector*)dets[0])->writeFlatField(imgname);
   };


  void *readFlatField(const char * imgname, int nb=-1, double emin=1, double emax=0){
    ((threadedInterpolatingDetector*)dets[0])->readFlatField(imgname, nb, emin, emax);
  };
  

   virtual int setNSubPixels(int ns)  { return ((threadedInterpolatingDetector*)dets[0])->setNSubPixels(ns);};
   virtual void resetFlatField() {((threadedInterpolatingDetector*)dets[0])->resetFlatField();};


};

#endif


















