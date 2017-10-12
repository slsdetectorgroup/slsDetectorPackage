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

#include <pthread.h>

#include "interpolatingDetector.h"
#include "circularFifo.h"





class threadedDetector
{
public:
  threadedDetector(interpolatingDetector *d, int fs=10000) {
    char *mem, *mm;
    det=d;
    fifoFree=new CircularFifo<char>(fs);
    fifoData=new CircularFifo<char>(fs);
    mem=(char*)malloc(fs*det->getDataSize());
    for (int i=0; i<fs; i++) {
      mm=mem+i*det->getDataSize();
      fifoFree->push(mm);
    }  
    stop=1;
    fMode=eFrame;
  }
  

  int setFrameMode(int fm) {if (fMode>=0) fMode=fm; return fMode;}

  void prepareInterpolation(int &ok) {
    cout << "-" << endl;
    det->prepareInterpolation(ok);
  };

  int *getInterpolatedImage() {
    return det->getInterpolatedImage();
  }
  int getImageSize(int &nnx, int &nny, int &ns) {return det->getImageSize(nnx, nny, ns);};
  virtual ~threadedDetector() {StopThread(); free(mem); delete fifoFree; delete fifoData;}

   /** Returns true if the thread was successfully started, false if there was an error starting the thread */
   bool StartThread()
   { stop=0;
     return (pthread_create(&_thread, NULL, processData, this) == 0);
   }

   void StopThread()
   { stop=1;
     (void) pthread_join(_thread, NULL);
   }
   
   
   bool pushData(char* &ptr) {
     fifoData->push(ptr);
   }
   bool popFree(char* &ptr) {
     fifoFree->pop(ptr);
   }

   //protected:
   /** Implement this method in your subclass with the code you want your thread to run. */
   //virtual void InternalThreadEntry() = 0;
   void *writeInterpolatedImage(const char * imgname) {cout << "a" <<endl; return det->writeInterpolatedImage(imgname);};
private:
   interpolatingDetector *det;
   int fMode;
   int *dataSize;
   pthread_t _thread;
   char *mem;
   CircularFifo<char> *fifoFree;
   CircularFifo<char> *fifoData;
   int stop;
   char *data;

   static void * processData(void * ptr) {
     threadedDetector *This=((threadedDetector *)ptr); 
     return This->processData();
   }

   void * processData() {
     while (!stop) {
       if (fifoData->isEmpty()) {
	 usleep(100);
       } else {
	 fifoData->pop(data); //blocking!
	 det->processData(data,(frameMode)fMode);
	 fifoFree->push(data); 
       }
     }
     return NULL;
   }






};



class multiThreadedDetector
{
public:
 multiThreadedDetector(interpolatingDetector *d, int n, int fs=1000) : nThreads(n), ithread(0) { 
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
      dets[i]=new threadedDetector(dd[i], fs);
    }
    
    int nnx, nny, ns;
    int nn=dets[0]->getImageSize(nnx, nny,ns);
    image=new int[nn];
    
  }
  
  ~multiThreadedDetector() { 
    StopThreads();
    for (int i=0; i<nThreads; i++) 
      delete dets[i]; 
    for (int i=1; i<nThreads; i++) 
      delete dd[i];
    delete [] image;
  }


  int setFrameMode(int fm) { int ret; for (int i=0; i<nThreads; i++) ret=dets[i]->setFrameMode(fm); return ret;};

  void prepareInterpolation(int &ok) {
    int oo;
    ok=1;
    for (int i=0; i<nThreads; i++) { 
      cout << i << endl;
      dets[i]->prepareInterpolation(oo);
      //if (oo<1) ok=0;
    }
  };

  int *getInterpolatedImage() {
    int *img;
    int nnx, nny, ns;
    int nn=dets[0]->getImageSize(nnx, nny, ns);
    //for (i=0; i<nn; i++) image[i]=0;
    
    for (int ii=0; ii<nThreads; ii++) {
      img=dets[ii]->getInterpolatedImage();
      for (int i=0; i<nn; i++) {
	if (ii==0)
	  image[i]=img[i];
	else
	  image[i]+=img[i];
      }
    
    }
    return image;
    
  }

   void *writeInterpolatedImage(const char * imgname) {   
#ifdef SAVE_ALL  
     for (int ii=0; ii<nThreads; ii++) {
       char tit[10000];cout << "m" <<endl;
       sprintf(tit,"/scratch/int_%d.tiff",ii);
       dets[ii]->writeInterpolatedImage(tit);
     }
#endif
     getInterpolatedImage();
     int nnx, nny, ns;
     int nn=dets[0]->getImageSize(nnx, nny, ns);
     float *gm=new float[ nn];
     if (gm) {
       for (int ix=0; ix<nn; ix++) {
	 gm[ix]=image[ix];
       }
     
       WriteToTiff(gm,imgname ,nnx, nny); 
       delete [] gm;
     } else cout << "Could not allocate float image " << endl;
     return NULL;
   }
  

  void StartThreads() { 
    for (int i=0; i<nThreads; i++) 
      dets[i]->StartThread(); 
      
    }
   


  void StopThreads() { 
    for (int i=0; i<nThreads; i++) 
      dets[i]->StopThread(); 
      
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

private:
    bool stop;
    const int nThreads;
    threadedDetector *dets[20];
    interpolatingDetector *dd[20];
    int ithread;
    int *image;
  };












































































/* void *moenchProcessFrame() { */
/*   char fname[10000]; */
/*   strcpy(fname,"/mnt/moench_data/m03-15_mufocustube/plant_40kV_10uA/m03-15_100V_g4hg_300us_dtf_0.raw"); */
  

/*   int nph, nph1; */
/*   single_photon_hit clusters[NR*NC]; */
/*   // cout << "hits "<< endl; */
/*   int etabins=550; */
/*   double etamin=-1, etamax=2; */
/*   int nsubpix=2; */
/*   float *etah=new float[etabins*etabins]; */
/*   // cout << "etah "<< endl; */
/*   cout << "image size "<< nsubpix*nsubpix*NC*NR << endl; */
/*   float *image=new float[nsubpix*nsubpix*NC*NR]; */
/*   int *heta, *himage; */

/*   moench03Ctb10GbT1Data *decoder=new  moench03Ctb10GbT1Data(); */
/*   // cout << "decoder "<< endl; */
/*   etaInterpolationPosXY *interp=new etaInterpolationPosXY(NC, NR, nsubpix, etabins, etamin, etamax); */
/*   // cout << "interp "<< endl; */
/*   //  linearInterpolation *interp=new linearInterpolation(NC, NR, nsubpix); */

/*   interpolatingDetector *filter=new interpolatingDetector(decoder,interp, 5, 1, 0, 1000, 100); */
/*   cout << "filter "<< endl; */
  

/*   char *buff; */
/*   int nf=0; */
/*   int ok=0; */
/*   ifstream filebin; */
/*   std::time_t end_time; */
 
/*   int iFrame=-1; */
/*   int np=-1; */

  
/*   filter->newDataSet(); */


/*   nph=0; */
/*   nph1=0; */
/*   int iph; */

/*   cout << "file name " << fname << endl; */
/*   filebin.open((const char *)(fname), ios::in | ios::binary); */
/*   if (filebin.is_open()) */
/*      cout << "Opened file " << fname<< endl; */
/*   else */
/*     cout << "Could not open file " << fname<< endl; */
/*     while ((buff=decoder->readNextFrame(filebin, iFrame)) && nf<1.5E4) {   */
/*       if (nf<9E3)  */
/* 	; */
/*       else if (nf<1.1E4) { */
/*       	filter->processData(buff,eFlat); */
/*       } else { */
/* 	if (ok==0) { */
/* 	  cout << "**************************************************************************"<< endl; */
/* 	  heta=interp->getFlatField(); */
/* 	  // for (int ii=0; ii<etabins*etabins; ii++) { */
/* 	  //   etah[ii]=(float)heta[ii]; */
/* 	  // } */


/* 	  std::time(&end_time); */
/* 	  cout << std::ctime(&end_time) << " " << nf <<  endl; */
/* 	  // WriteToTiff(etah, "/scratch/eta.tiff", etabins, etabins); */
	  
/* 	  interp->prepareInterpolation(ok); */
/* 	  cout << "**************************************************************************"<< endl; */
/* 	  std::time(&end_time); */
/* 	  cout << std::ctime(&end_time) << " " << nf <<  endl; */
/* 	} */
/* 	filter->processData(buff,eFrame); */
/* 	  } */
      
/*       nph+=nph1; */

/*       if (nf%1000==0) { */
/* 	std::time(&end_time); */
/* 	cout << std::ctime(&end_time) << " " << nf <<   endl; */
/*       } */

/*       nf++; */
/*       delete [] buff; */
/*       iFrame=-1; */
/*     }  */
 
/*     if (filebin.is_open()) */
/*       filebin.close();	   */
/*     else */
/*       cout << "could not open file " << fname << endl; */
    
    
/*     himage=interp->getInterpolatedImage(); */
/*     for (int ii=0; ii<nsubpix*nsubpix*NC*NR; ii++) { */
/*       image[ii]=(float)himage[ii]; */
/*     } */
/*     WriteToTiff(image, "/scratch/int_image.tiff", nsubpix*NR, nsubpix*NC); */
/*     delete [] etah; */
/*     delete [] image; */
/*     delete interp; */
/*     delete decoder; */
/*     cout << "Read " << nf << " frames" << endl; */
/*     return NULL; */
/* } */
  
/* int main(int argc, char *argv[]){ */

/*    moenchProcessFrame(); */

/* } */
