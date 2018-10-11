#ifndef MULTITHREADED_ANALOG_DETECTOR_H
#define MULTITHREADED_ANALOG_DETECTOR_H

#define MAXTHREADS 1000

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
//#include <deque>
//#include <list>
//#include <queue>
#include <fstream>
#include <cstdlib> 
#include <pthread.h>

#include "analogDetector.h"
#include "circularFifo.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
//#include <mutex>


using namespace std;


class threadedAnalogDetector
{
public:
  threadedAnalogDetector(analogDetector<uint16_t> *d, int fs=10000) {
    char *mem, *mm;
    det=d;
    fifoFree=new CircularFifo<char>(fs);
    fifoData=new CircularFifo<char>(fs);
    mem=(char*)malloc(fs*det->getDataSize());
    // cout << "data size is " << det->getDataSize()*fs << endl;
    for (int i=0; i<fs; i++) {
      mm=mem+i*det->getDataSize();
      fifoFree->push(mm);
    }  
    busy=0;
    stop=1;
    fMode=eFrame;
    ff=NULL;
  }
  

  virtual int setFrameMode(int fm) {
    if (fm>=0) {
      det->setFrameMode((frameMode)fm); 
      fMode=fm; 
    }
    return fMode;
  };
  virtual double setThreshold(double th) {return det->setThreshold(th);};



  virtual void setROI(int xmin, int xmax, int ymin, int ymax) {det->setROI(xmin,xmax,ymin,ymax);};
  virtual int setDetectorMode(int dm) {
    if (dm>=0) {
      det->setDetectorMode((detectorMode)dm); 
      dMode=dm; 
    }
    return dMode;
  };

  virtual void newDataSet(){det->newDataSet();};
      //fMode=fm; return fMode;}

  /* void prepareInterpolation(int &ok) { */
  /*   cout << "-" << endl; */
  /*   det->prepareInterpolation(ok); */
  /* }; */

  virtual int *getImage() {
    return det->getImage();
  }
  virtual int getImageSize(int &nnx, int &nny, int &ns) {return det->getImageSize(nnx, nny, ns);};
  virtual int getDetectorSize(int &nnx, int &nny) {return det->getDetectorSize(nnx, nny);};

  ~threadedAnalogDetector() {StopThread(); free(mem); delete fifoFree; delete fifoData;}

   /** Returns true if the thread was successfully started, false if there was an error starting the thread */
   virtual bool StartThread()
   { stop=0; 
     return (pthread_create(&_thread, NULL, processData, this) == 0);
   }

   virtual void StopThread()
   { stop=1;
     (void) pthread_join(_thread, NULL);
   }
   

   virtual bool pushData(char* &ptr) {
     fifoData->push(ptr);
   }

   virtual bool popFree(char* &ptr) {
     fifoFree->pop(ptr);
   }

   virtual int isBusy() {return busy;}
   
   //protected:
   /** Implement this method in your subclass with the code you want your thread to run. */
   //virtual void InternalThreadEntry() = 0;
   virtual void *writeImage(const char * imgname) {return det->writeImage(imgname);};

   virtual void clearImage(){det->clearImage();};

 
   virtual void setPedestal(double *ped, double *rms=NULL, int m=-1){det->setPedestal(ped,rms,m);};
   
   
   virtual void setPedestalRMS(double *rms){ det->setPedestalRMS(rms);};
   
 virtual double *getPedestal(double *ped=NULL){return det->getPedestal(ped);};
   
   
   virtual double *getPedestalRMS(double *rms=NULL){ return det->getPedestalRMS(rms);};
   
   
   

/** sets file pointer where to write the clusters to 
    \param f file pointer
    \returns current file pointer
*/
   FILE *setFilePointer(FILE *f){return det->setFilePointer(f); };

/** gets file pointer where to write the clusters to 
    \returns current file pointer
*/
FILE *getFilePointer(){return det->getFilePointer();};



  virtual double setNSigma(double n) {return det->setNSigma(n);};
  virtual void setEnergyRange(double emi, double ema) {det->setEnergyRange(emi,ema);};


  virtual void prepareInterpolation(int &ok){
       slsInterpolation *interp=det->getInterpolation(); 
       if (interp)  
        interp->prepareInterpolation(ok); 
   }

   virtual int *getFlatField(){
     slsInterpolation *interp=(det)->getInterpolation();
     if (interp) 
       return interp->getFlatField();
     else
       return NULL;
   }
   
   virtual int *setFlatField(int *ff, int nb, double emin, double emax){
     slsInterpolation *interp=(det)->getInterpolation();
     if (interp) 
       return interp->setFlatField(ff, nb, emin, emax);
     else
       return NULL;
   }

   void *writeFlatField(const char * imgname) {
     slsInterpolation *interp=(det)->getInterpolation();
     cout << "interp " << interp << endl;
     if (interp)  {
       cout << imgname << endl;
       interp->writeFlatField(imgname);
     }
   }
   void *readFlatField(const char * imgname, int nb=-1, double emin=1, double emax=0){
     slsInterpolation *interp=(det)->getInterpolation();
     if (interp)  
       interp->readFlatField(imgname, nb, emin, emax);
   }

   virtual int *getFlatField(int &nb, double emi, double ema){
     slsInterpolation *interp=(det)->getInterpolation();
     int *ff=NULL;
     if (interp) { 
       ff=interp->getFlatField(nb,emi,ema);
     }     
     return ff;
   }
   
   virtual slsInterpolation *getInterpolation() {
     return (det)->getInterpolation();
   }
   
   virtual void resetFlatField() {
     slsInterpolation *interp=(det)->getInterpolation();
     if (interp)   interp->resetFlatField();//((interpolatingDetector*)det)->resetFlatField();
   }


   virtual int setNSubPixels(int ns)  { 
     slsInterpolation *interp=(det)->getInterpolation();
     if (interp)  return interp->setNSubPixels(ns);
     else return 1;};


   virtual slsInterpolation *setInterpolation(slsInterpolation *f){
     return (det)->setInterpolation(f);
   };
protected:
   analogDetector<uint16_t> *det;
   int fMode;
   int dMode;
   int *dataSize;
   pthread_t _thread;
   char *mem;
   CircularFifo<char> *fifoFree;
   CircularFifo<char> *fifoData;
   int stop;
   int busy;
   char *data;
   int *ff;

   static void * processData(void * ptr) {
     threadedAnalogDetector *This=((threadedAnalogDetector *)ptr); 
     return This->processData();
   }

   void * processData() {
     busy=1;
     while (!stop) {
       if (fifoData->isEmpty()) {
	 busy=0;
	 usleep(100);
       } else {
	 busy=1;
	 fifoData->pop(data); //blocking!
	 det->processData(data);
	 fifoFree->push(data); 
       }
     }
     return NULL;
   }



};



class multiThreadedAnalogDetector
{
public:
 multiThreadedAnalogDetector(analogDetector<uint16_t> *d, int n, int fs=1000) : stop(0), nThreads(n), ithread(0) { 
    dd[0]=d;
    if (nThreads==1)
      dd[0]->setId(100);
    else 
      dd[0]->setId(0);
    for (int i=1; i<nThreads; i++) {
      dd[i]=d->Clone();
      dd[i]->setId(i);
    }
    
    for (int i=0; i<nThreads; i++) {
      dets[i]=new threadedAnalogDetector(dd[i], fs);
    }
    
    image=NULL;
    ff=NULL;
    ped=NULL;
    cout << "Ithread is " << ithread << endl;
  }
  
  ~multiThreadedAnalogDetector() { 
    StopThreads();
    for (int i=0; i<nThreads; i++) 
      delete dets[i]; 
    for (int i=1; i<nThreads; i++) 
      delete dd[i];
    //delete [] image;
  }


  virtual int setFrameMode(int fm) { int ret; for (int i=0; i<nThreads; i++) {  ret=dets[i]->setFrameMode(fm);} return ret;};
  virtual double setThreshold(int fm) { double ret; for (int i=0; i<nThreads; i++) ret=dets[i]->setThreshold(fm); return ret;};
  virtual int setDetectorMode(int dm) { int ret; for (int i=0; i<nThreads; i++) ret=dets[i]->setDetectorMode(dm); return ret;};
  virtual void setROI(int xmin, int xmax, int ymin, int ymax) { for (int i=0; i<nThreads; i++) dets[i]->setROI(xmin, xmax,ymin,ymax);};
  
   
  virtual void newDataSet(){for (int i=0; i<nThreads; i++) dets[i]->newDataSet();};
  
  virtual int *getImage(int &nnx, int &nny, int &ns) {
    int *img;
    // int nnx, nny, ns; 
    // int nnx, nny, ns;
    int nn=dets[0]->getImageSize(nnx, nny,ns);
    if (image) {
      delete image;
      image=NULL;
    }
    image=new int[nn];
    //int nn=dets[0]->getImageSize(nnx, nny, ns);
    //for (i=0; i<nn; i++) image[i]=0;
    
    for (int ii=0; ii<nThreads; ii++) {
      //cout << ii << " " << nn << " " << nnx << " " << nny << " " << ns << endl;
      img=dets[ii]->getImage();
      for (int i=0; i<nn; i++) {
	if (ii==0)
	  image[i]=img[i];
	else
	  image[i]+=img[i];
	//if (img[i])	  cout << "det " << ii << " pix " << i << " val " <<  img[i] << " " << image[i] << endl;
      }
    
    }
    return image;
    
  }


  virtual void clearImage() {
  
    for (int ii=0; ii<nThreads; ii++) {
      dets[ii]->clearImage();
    }
    
  }

  virtual void *writeImage(const char * imgname, double t=1) {   
/* #ifdef SAVE_ALL   */
/*      for (int ii=0; ii<nThreads; ii++) { */
/*        char tit[10000];cout << "m" <<endl; */
/*        sprintf(tit,"/scratch/int_%d.tiff",ii); */
/*        dets[ii]->writeImage(tit); */
/*      } */
/* #endif */
     int nnx, nny, ns;
     getImage(nnx, nny, ns);
     //int nnx, nny, ns;
     int nn=dets[0]->getImageSize(nnx, nny, ns);
     float *gm=new float[nn];
     if (gm) {
       for (int ix=0; ix<nn; ix++) {
	 if (t)
	   gm[ix]=(image[ix])/t;
	 else
	   gm[ix]=image[ix];
	   
	 //if (image[ix]>0 && ix/nnx<350) cout << ix/nnx << " " << ix%nnx << " " << image[ix]<< " " << gm[ix] << endl;
       }
       //cout << "image " << nnx << " " << nny << endl;
       WriteToTiff(gm,imgname ,nnx, nny); 
       delete [] gm;
     } else cout << "Could not allocate float image " << endl;
     return NULL;
   }
  

  virtual void StartThreads() { 
    for (int i=0; i<nThreads; i++) { 
      dets[i]->StartThread(); 
    }
    
  }
   


  virtual void StopThreads() { 
    for (int i=0; i<nThreads; i++) 
      dets[i]->StopThread(); 
      
    }

  
  virtual int isBusy() {
    int ret=0, ret1;  
    for (int i=0; i<nThreads; i++) {
      ret1=dets[i]->isBusy();
      ret|=ret1;
      //   if (ret1) cout << "thread " << i <<" still busy " << endl;
    }
    return ret;
  }
  
   
   virtual bool pushData(char* &ptr) {
     dets[ithread]->pushData(ptr);
   }

   virtual bool popFree(char* &ptr) {
     //  cout << ithread << endl;
     dets[ithread]->popFree(ptr);
   }
   
   virtual int nextThread() {
     ithread++;
     if (ithread==nThreads) ithread=0;
     return ithread;
   }


  virtual double *getPedestal(){
    int nx, ny;
    dets[0]->getDetectorSize(nx,ny);
    if (ped) delete [] ped;
    ped=new double[nx*ny];
    double *p0=new double[nx*ny];
    
    for (int i=0; i<nThreads; i++) {
      //inte=(slsInterpolation*)dets[i]->getInterpolation(nb,emi,ema);
      //  cout << i << endl;
      p0=dets[i]->getPedestal(p0);
      if (p0) {
	if (i==0) {

	for (int ib=0; ib<nx*ny; ib++) {
	  ped[ib]=p0[ib]/((double)nThreads);
	  //  cout << p0[ib] << " ";
	}
	} else {
	for (int ib=0; ib<nx*ny; ib++) {
	  ped[ib]+=p0[ib]/((double)nThreads);
	  //  cout << p0[ib] << " ";
	}
	}
      }
	
    } 
    delete [] p0;
    return ped;
  };
   
   
   virtual double *setPedestal(double *h=NULL){
     //int nb=0;
    
     int nx, ny;
     dets[0]->getDetectorSize(nx,ny);
     if (h==NULL) h=ped;
     for (int i=0; i<nThreads; i++) {
       dets[i]->setPedestal(h);
     }
     
     return NULL;
   }; 




   virtual void *writePedestal(const char * imgname){     
    
     int nx, ny;
     dets[0]->getDetectorSize(nx,ny);
   
      getPedestal();
      float *gm=new float[nx*ny];
      if (gm) {
       for (int ix=0; ix<nx*ny; ix++) {
	 gm[ix]=ped[ix];
       }    
       WriteToTiff(gm,imgname ,nx, ny); 
       delete [] gm;
     } else cout << "Could not allocate float image " << endl;
    
    return NULL;

   };


  virtual void *readPedestal(const char * imgname, int nb=-1, double emin=1, double emax=0){
    
     int nx, ny;
     dets[0]->getDetectorSize(nx,ny);
    uint32 nnx;
    uint32 nny;
    float *gm=ReadFromTiff(imgname, nnx, nny);
    if (ped) delete [] ped;
    if (nnx>nx) nx=nnx;
    if (nny>ny) ny=nny;
    ped=new double[nx*ny];
    
    for (int ix=0; ix<nx*ny; ix++) {
      ped[ix]=gm[ix];
    }
    delete [] gm;
    return setPedestal();
    
  };
  





/** sets file pointer where to write the clusters to 
    \param f file pointer
    \returns current file pointer
*/
   virtual FILE *setFilePointer(FILE *f){
     for (int i=0; i<nThreads; i++) {
       dets[i]->setFilePointer(f);
       //dets[i]->setMutex(&fmutex);
     }
     return dets[0]->getFilePointer();
   };

   /** gets file pointer where to write the clusters to 
       \returns current file pointer
   */
   virtual FILE *getFilePointer(){return dets[0]->getFilePointer();};



 protected:
  bool stop;
  const int nThreads;
  threadedAnalogDetector *dets[MAXTHREADS];
  analogDetector<uint16_t> *dd[MAXTHREADS];
  int ithread;
  int *image;
  int *ff;
  double *ped;
  pthread_mutex_t fmutex;
};

#endif


















