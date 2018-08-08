#ifndef MULTITHREADED_ANALOG_DETECTOR_H
#define MULTITHREADED_ANALOG_DETECTOR_H

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

//#include "analogDetector.h"
#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"
#include "circularFifo.h"
#include "slsInterpolation.h"
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
  

  virtual int setFrameMode(int fm) {fMode=fm; if (fMode>=0) det->setFrameMode((frameMode)fMode);};
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
   virtual void *writeImage(const char * imgname) {cout << "a" <<endl; return det->writeImage(imgname);};

   virtual void clearImage(){det->clearImage();};

   virtual void prepareInterpolation(int &ok){
     slsInterpolation *interp=(slsInterpolation*)det->getInterpolation();
     if (interp) 
       interp->prepareInterpolation(ok);
   }

   virtual int *getFlatField(){
     slsInterpolation *interp=(slsInterpolation*)det->getInterpolation();
     if (interp) 
       return interp->getFlatField();
     else
       return NULL;
   }
   
   virtual int *setFlatField(int *ff, int nb, double emin, double emax){
     slsInterpolation *interp=(slsInterpolation*)det->getInterpolation();
     if (interp) 
       return interp->setFlatField(ff, nb, emin, emax);
     else
       return NULL;
   }
   
   virtual int *getFlatField(int &nb, double emi, double ema){
     slsInterpolation *interp=(slsInterpolation*)det->getInterpolation();
     int *ff;
     if (interp) { 
       ff=interp->getFlatField(nb,emi,ema);
       //   cout << "tdgff* ff has " << nb << " bins " << endl;
       return ff;
     }     else
       return NULL;
   }
   
   virtual char *getInterpolation() {
     return det->getInterpolation();
   }
   
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


 void setMutex(pthread_mutex_t *fmutex){det->setMutex(fmutex);};

private:
   analogDetector<uint16_t> *det;
   int fMode;
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
 multiThreadedAnalogDetector(analogDetector<uint16_t> *d, int n, int fs=1000) : nThreads(n), ithread(0) { 
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
    
    int nnx, nny, ns;
    int nn=dets[0]->getImageSize(nnx, nny,ns);
    image=new int[nn];
    ff=NULL;
    ped=NULL;
  }
  
  ~multiThreadedAnalogDetector() { 
    StopThreads();
    for (int i=0; i<nThreads; i++) 
      delete dets[i]; 
    for (int i=1; i<nThreads; i++) 
      delete dd[i];
    delete [] image;
  }


  int setFrameMode(int fm) { int ret; for (int i=0; i<nThreads; i++) ret=dets[i]->setFrameMode(fm); return ret;};
  
 void newDataSet(){for (int i=0; i<nThreads; i++) dets[i]->newDataSet();};
  int *getImage(int &nnx, int &nny, int &ns) {
    int *img;
    // int nnx, nny, ns;
    int nn=dets[0]->getImageSize(nnx, nny, ns);
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


  void clearImage() {
  
    for (int ii=0; ii<nThreads; ii++) {
      dets[ii]->clearImage();
    }
    
  }

   virtual void *writeImage(const char * imgname) {   
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
	 gm[ix]=image[ix];
	 // if (image[ix]>0 && ix/nnx<350) cout << ix/nnx << " " << ix%nnx << " " << image[ix]<< " " << gm[ix] << endl;
       }
       //cout << "image " << nnx << " " << nny << endl;
       WriteToTiff(gm,imgname ,nnx, nny); 
       delete [] gm;
     } else cout << "Could not allocate float image " << endl;
     return NULL;
   }
  

  virtual void StartThreads() { 
    for (int i=0; i<nThreads; i++) 
      dets[i]->StartThread(); 
      
    }
   


  virtual void StopThreads() { 
    for (int i=0; i<nThreads; i++) 
      dets[i]->StopThread(); 
      
    }

  
  int isBusy() {
    int ret=0, ret1;  
    for (int i=0; i<nThreads; i++) {
      ret1=dets[i]->isBusy();
      ret|=ret1;
      //   if (ret1) cout << "thread " << i <<" still busy " << endl;
    }
    return ret;
  }
  
   
   bool pushData(char* &ptr) {
     dets[ithread]->pushData(ptr);
   }

   bool popFree(char* &ptr) {
     dets[ithread]->popFree(ptr);
   }
   
   int nextThread() {
     ithread++;
     if (ithread==nThreads) ithread=0;
     return ithread;
   }

   virtual void prepareInterpolation(int &ok){
     getFlatField(); //sum up all etas
     setFlatField(); //set etas to all detectors
     for (int i=0; i<nThreads; i++) {
       dets[i]->prepareInterpolation(ok);
     }
   }
   
   virtual int *getFlatField(){
     int nb=0;
     double emi, ema;
     int *f0;
     slsInterpolation* inte=(slsInterpolation*)dets[0]->getInterpolation();
     if (inte) {
       if (inte->getFlatField(nb,emi,ema)) {
	 if (ff) delete [] ff;
	 ff=new int[nb*nb];
	 for (int i=0; i<nThreads; i++) {
	   //   cout << "mtgff* ff has " << nb << " bins " << endl;
	   inte=(slsInterpolation*)dets[i]->getInterpolation();
	   f0=inte->getFlatField();
	   if (f0) {
	     //    cout << "ff " << i << endl;
	     for (int ib=0; ib<nb*nb; ib++) {
	       if (i==0)
		   ff[ib]=f0[ib];
		 else
		   ff[ib]+=f0[ib];	  
	       /* if (i==0 && f0[ib]>0) */
	       /* 	 cout << i << " " << ib << " " << f0[ib] << " " << ff[ib] << endl; */

	     }
	   }
	 }
	 return ff;
       }
     } 
     return NULL;
   }
   
   
   virtual int *setFlatField(int *h=NULL, int nb=-1, double emin=1, double emax=0){
     //int nb=0;
     double emi, ema;
     slsInterpolation* inte=(slsInterpolation*)dets[0]->getFlatField(nb,emi,ema);
     if (inte) {
       if (h==NULL) h=ff;
      for (int i=0; i<nThreads; i++) {
	dets[i]->setFlatField(h, nb, emin, emax);
      }
     }
     return NULL;
   }; 

   void *writeFlatField(const char * imgname){
     
     int nb=0;
     double emi, ema;
     slsInterpolation* inte=(slsInterpolation*)dets[0]->getFlatField(nb,emi,ema);
     if (inte) {
      if (getFlatField()) {
	//	cout << "mtwff* ff has " << nb << " bins " << endl;
	float *gm=new float[nb*nb];
	if (gm) {
	  for (int ix=0; ix<nb*nb; ix++) {
	    gm[ix]=ff[ix];
	  }    
	  WriteToTiff(gm,imgname ,nb, nb); 
	  delete [] gm;
	} else cout << "Could not allocate float image " << endl;
      }
    } else
      cout << "Detector without flat field! " << endl;
    
    return NULL;
    
   };


  void *readFlatField(const char * imgname, int nb=-1, double emin=1, double emax=0){
    
    int* inte=(int*)dets[0]->getFlatField(nb,emin,emax);
    if (inte) {
    uint32 nnx;
    uint32 nny;
    float *gm=ReadFromTiff(imgname, nnx, nny);
    if (ff) delete [] ff;
    if (nnx>nb) nb=nnx;
    if (nny>nb) nb=nny;
    ff=new int[nb*nb];
    
    for (int ix=0; ix<nb*nb; ix++) {
      ff[ix]=gm[ix];
    }
    delete [] gm;
      return setFlatField(ff,nb,emin,emax);
    } else
       cout << "Not interpolating detector! " << endl;
    
    return NULL;
  };
  




  virtual double *getPedestal(){
    int nx, ny;
    dets[0]->getDetectorSize(nx,ny);
    if (ped) delete [] ped;
    ped=new double[nx*ny];
    double *p0=new double[nx*ny];
    
    for (int i=0; i<nThreads; i++) {
      //inte=(slsInterpolation*)dets[i]->getInterpolation(nb,emi,ema);
      p0=dets[i]->getPedestal(p0);
      if (p0) {
	for (int ib=0; ib<nx*ny; ib++) {
	  ped[ib]+=p0[ib]/((double)nThreads);
	}
      }
	
      delete [] p0;
    } 
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




   void *writePedestal(const char * imgname){     
    
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


  void *readPedestal(const char * imgname, int nb=-1, double emin=1, double emax=0){
    
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
   FILE *setFilePointer(FILE *f){
     for (int i=0; i<nThreads; i++) {
       dets[i]->setFilePointer(f);
       //dets[i]->setMutex(&fmutex);
     }
     return dets[0]->getFilePointer();
   };

/** gets file pointer where to write the clusters to 
    \returns current file pointer
*/
FILE *getFilePointer(){return dets[0]->getFilePointer();};



 private:
  bool stop;
  const int nThreads;
  threadedAnalogDetector *dets[20];
  analogDetector<uint16_t> *dd[20];
  int ithread;
  int *image;
  int *ff;
  double *ped;
  pthread_mutex_t fmutex;
};

#endif


















