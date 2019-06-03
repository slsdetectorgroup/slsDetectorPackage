#ifndef MULTITHREADED_INTERPOLATING_DETECTOR_H
#define MULTITHREADED_INTERPOLATING_DETECTOR_H


#include "interpolatingDetector.h"
#include "multiThreadedCountingDetector.h"
//#include <mutex>


using namespace std;



class multiThreadedInterpolatingDetector : public multiThreadedCountingDetector
{
public:
 multiThreadedInterpolatingDetector(interpolatingDetector *d, int n, int fs=1000) : multiThreadedCountingDetector(d,n,fs) { };

  virtual void prepareInterpolation(int &ok){
    /* getFlatField(); //sum up all etas */
    /* setFlatField(); //set etas to all detectors */
    /* for (int i=0; i<nThreads; i++) { */
       (dets[0])->prepareInterpolation(ok);
       //  }
  }
   
   virtual int *getFlatField(){
     return (dets[0])->getFlatField();
   }
   
   virtual int *getFlatField(int &nb, double emi, double ema){
     return (dets[0])->getFlatField(nb,emi,ema);
   }
   
   virtual int *setFlatField(int *h=NULL, int nb=-1, double emin=1, double emax=0){
     return (dets[0])->setFlatField(h,nb,emin,emax);
   }; 

   void *writeFlatField(const char * imgname){
     dets[0]->writeFlatField(imgname);
   };


  void *readFlatField(const char * imgname, int nb=-1, double emin=1, double emax=0){
    (dets[0])->readFlatField(imgname, nb, emin, emax);
  };
  

   virtual int setNSubPixels(int ns)  { return (dets[0])->setNSubPixels(ns);};

   virtual void resetFlatField() {(dets[0])->resetFlatField();};
   

/** sets file pointer where to write the clusters to 
    \param f file pointer
    \returns current file pointer
*/
   virtual slsInterpolation *setInterpolation(slsInterpolation *f){
     int ok;
     for (int i=0; i<nThreads; i++)  
       (dets[i])->setInterpolation(f);
     return  (dets[0])->getInterpolation();
   };

   virtual slsInterpolation *getInterpolation(){
   
     return (dets[0])->getInterpolation();
   };



  virtual int *getImage(int &nnx, int &nny, int &ns) {
    if (getInterpolation()==NULL) return multiThreadedAnalogDetector::getImage(nnx,nny,ns);
    //if one interpolates, the whole image is stored in detector 0;
    int *img;
    // int nnx, nny, ns; 
    // int nnx, nny, ns;
    int nn=dets[0]->getImageSize(nnx, nny,ns);
    if (image) {
      delete image;
      image=NULL;
    }
    image=new int[nn];    
    img=dets[0]->getImage();
    for (int i=0; i<nn; i++) {
      image[i]=img[i];
    }
    return image;
  };


};

#endif


















