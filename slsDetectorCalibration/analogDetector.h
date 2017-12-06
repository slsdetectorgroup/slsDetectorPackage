#ifndef ANALOGDETECTOR_H
#define  ANALOGDETECTOR_H

//#include <mutex>

#include <pthread.h>
#include "slsDetectorData.h"
#include "pedestalSubtraction.h"
#include "commonModeSubtraction.h"
#include "tiffIO.h"


using namespace std;

#ifndef FRAMEMODE_DEF
#define FRAMEMODE_DEF
/** 
enum to define the flags of the data set, which are needed to seect the type of processing it should undergo: frame, pedestal, flat
*/
 enum frameMode { eFrame, ePedestal, eFlat };
#endif


template <class dataType> class analogDetector {

  /** @short class to perform pedestal subtraction etc. for an analog detector */

 public:

 

  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param d detector data structure to be used - if null it is assumed that the data are in ordered ip=iy*nx+ix
     \param sign is the sign of the data
     \param nped number of samples for pedestal averaging
     \param cm common mode subtraction algorithm, if any. Defaults to NULL i.e. none
     \param nnx detector size in x - must be specified if no data structure is defined, otherwise defaults to the size of the data structure.
     \param nny detector size in y - must be specified if no data structure is defined, otherwise defaults to the size of the data structure.
     \param gm pointer to tha gain map matrix


  */
  

 analogDetector(slsDetectorData<dataType> *d, int sign=1, 
		commonModeSubtraction *cm=NULL, int nped=1000, int nnx=-1, int nny=-1, double *gm=NULL) : det(d), nx(nnx), ny(nny), stat(NULL), cmSub(cm),  iframe(-1), dataSign(sign), gmap(gm), id(0) {
    
    if (det)
      det->getDetectorSize(nx,ny);
    
    stat=new pedestalSubtraction*[ny];
    for (int i=0; i<ny; i++) {
      stat[i]=new pedestalSubtraction[nx];
      for (int ix=0; ix<nx; ix++) {
	stat[i][ix].SetNPedestals(nped);
      }
    }
    image=new int[nx*ny];
    xmin=0;
    xmax=nx;
    ymin=0;
    ymax=ny;
    fMode=ePedestal;
    thr=0;
    myFile=NULL;
    fm=new pthread_mutex_t ;
  };
  /**
     destructor. Deletes the pdestalSubtraction array and the image
  */
  virtual ~analogDetector() {for (int i=0; i<ny; i++) delete [] stat[i]; delete [] stat; delete [] image;};

  /**
     constructor cloning another analog detector
     \param orig analog Detector structure to be cloned
   */
  analogDetector(analogDetector* orig) { 
  /* copy construction from orig*/ 
    det=orig->det;
    nx=orig->nx;
    ny=orig->ny;
    dataSign=orig->dataSign;
    iframe=orig->iframe;
    gmap=orig->gmap;
    cmSub=orig->cmSub;
    id=orig->id;
    xmin=orig->xmin;
    xmax=orig->xmax;
    ymin=orig->ymin;
    ymax=orig->ymax;
    thr=orig->thr;
    // nSigma=orig->nSigma;
    fMode=orig->fMode;
    myFile=orig->myFile;
    fm=orig->fm;
    

    stat=new pedestalSubtraction*[ny];
    for (int i=0; i<ny; i++) {
      stat[i]=new pedestalSubtraction[nx];
    }
    
    int nped=orig->SetNPedestals();
    //cout << nped << " " << orig->getPedestal(ix,iy) << orig->getPedestalRMS(ix,iy) << endl;
    for (int iy=0; iy<ny; iy++) {
      for (int ix=0; ix<nx; ix++) {
	stat[iy][ix].SetNPedestals(nped);
	setPedestal(ix,iy,orig->getPedestal(ix,iy),orig->getPedestalRMS(ix,iy),orig->GetNPedestals(ix,iy));
      }
    }
    image=new int[nx*ny];
    
  }


  /**
     clone. Must be virtual!
     \returns a clone of the original analog detector
   */
  virtual analogDetector *Clone() {
    return new analogDetector(this);
  }

  /**
     Gives an id to the structure. For debugging purposes in case of multithreading.
     \param i is to be set
     \returns current id
   */
  int setId(int i){id=i; return id;};

  /**
     Returns id of the structure. For debugging purposes in case of multithreading.
     \returns current id
   */
  int getId() {return id; };
 /**
     Returns data size of the detector data structure
     \returns data size of the detector data structurein bytes
   */
  int getDataSize(){return det->getDataSize();}; 
 /**
     Returns data size of the detector image matrix
     \param nnx reference to image size in x
     \param nny reference to image size in y
     \param nns reference to number of subpixels for interpolating detector, will always be 1 in this case
     \returns number of pixels of the detector image
   */
  virtual int getImageSize(int &nnx, int &nny, int &nns){nnx=nx; nny=ny; nns=1; return nx*ny;}; 
 /**
     Returns data size of the detector image matrix
     \param nnx reference to pixel size in x
     \param nny reference to pixel size in y
     \returns number of pixels of the detector image
   */
  virtual int getDetectorSize(int &nnx, int &nny){nnx=nx; nny=ny; return nx*ny;}; 

  /**
     set gain map
     \param gm pointer to gain map matrix to be set -  NULL unsets
     \returns pointer to current gain map
  */
  double *setGainMap(double *gm) {gmap=gm; return gmap;};

   /**
     return gain map
     \returns pointer to current gain map
  */
  double *getGainMap() {return gmap;};
  /**
     reads a 32 bit tiff file of the size of the detector and sets its values as gain map for the detector. If file does not exist returns NULL, but does not change gainmap compared to previous settings.
     \param imgname complete name of the file containing the gain map data
     \returns pointer to current gain map is file reading succeeded, NULL is file reading didn't work.
  */
  double *readGainMap(const char * imgname) {
    uint32 nnx, nny;
    float *gm=ReadFromTiff( imgname, nny, nnx);
    if (gm) {
      if (gmap) delete [] gmap; 
      gmap=new double[nnx*nny];
      for (int ix=0; ix<nnx; ix++) {
	for (int iy=0; iy<nny; iy++) {
	  gmap[iy*nnx+ix]=gm[iy*nnx+ix];
	}
      }
      return gmap;
    }
    return NULL;
  }
   /**
     writes a 32 bit tiff file of the size of the detector and contaning the gain map value, if any. If file doesn'e exist or gainmap is undefined, does not do anything.
     \param imgname complete name of the file to be written
     \returns NULL if file writing didn't succeed, else a pointer
  */
  void *writeGainMap(const char * imgname) {
    float *gm=NULL;
    void *ret;
    if (gmap)  {
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  gm[iy*nx+ix]=gmap[iy*nx+ix];
	}
      }
      ret=WriteToTiff(gm, imgname, ny, nx);   
      delete [] gm;
    }
    return ret;
  }
  
  
  /** resets the pedestalSubtraction array, the commonModeSubtraction and the image data*/
    
  virtual void newDataSet(){
    iframe=-1; 
    for (int iy=0; iy<ny; iy++) 
      for (int ix=0; ix<nx; ix++) {
	stat[iy][ix].Clear(); 
	image[iy*nx+ix]=0;
      }
    if (cmSub) cmSub->Clear(); 
  };  
  
  /** resets the commonModeSubtraction and increases the frame index */
  virtual void newFrame(){iframe++; if (cmSub) cmSub->newFrame();};
  

    /** sets the commonModeSubtraction algorithm to be used 
	\param cm commonModeSubtraction algorithm to be used (NULL unsets) 
	\returns pointer to the actual common mode subtraction algorithm
    */
    commonModeSubtraction *setCommonModeSubtraction(commonModeSubtraction *cm) {cmSub=cm; return cmSub;};
/** 
      gets the commonModeSubtraction algorithm to be used 
	\returns pointer to the actual common mode subtraction algorithm
    */
    commonModeSubtraction *getCommonModeSubtraction() {return cmSub;};


    /**
       sets the sign of the data
       \param sign 1 means positive values for photons, -1 negative, 0 gets
       \returns current sign for the data
    */
    int setDataSign(int sign=0) {if (sign==1 || sign==-1) dataSign=sign; return dataSign;};

    
    /**
       adds value to pedestal (and common mode) for the given pixel
       \param val value to be added
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param cm 1 adds the value to common mod, 0 skips it. Defaults to 0. - not properly implemented
    */
    virtual void addToPedestal(double val, int ix, int iy=0){ 
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	stat[iy][ix].addToPedestal(val); 
	if (cmSub)  {
	  if (det) if (det->isGood(ix, iy)==0) return;
	  cmSub->addToCommonMode(val, ix, iy);
	};
      };
    }
    
    /**
       gets  pedestal (and common mode)
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param cm 0 (default) without common mode subtraction, 1 with common mode subtraction (if defined)
       \returns pedestal value
    */
    virtual double getPedestal(int ix, int iy, int cm=0){if (ix>=0 && ix<nx && iy>=0 && iy<ny) if (cmSub && cm>0) return stat[iy][ix].getPedestal()-cmSub->getCommonMode(); else return stat[iy][ix].getPedestal(); else return -1;};

    /**
       gets  pedestal rms (i.e. noise)
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \returns pedestal rms
    */
    virtual double getPedestalRMS(int ix, int iy){if (ix>=0 && ix<nx && iy>=0 && iy<ny) return stat[iy][ix].getPedestalRMS();else return -1;};

   
    /**
       gets  pedestal (and common mode)
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param cm 0 (default) without common mode subtraction, 1 with common mode subtraction (if defined)
       \returns pedestal value
    */
    virtual double* getPedestal(double *ped){
      if (ped==NULL)
	ped=new double[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  ped[iy*nx+ix]=stat[iy][ix].getPedestal();
	}
      }
      return ped;
    };

    /**
       gets  pedestal rms (i.e. noise)
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \returns pedestal rms
    */
    virtual double* getPedestalRMS(double *ped=NULL){
      if (ped==NULL)
	ped=new double[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  ped[iy*nx+ix]=stat[iy][ix].getPedestalRMS();
	}
      }
      return ped;
    };













    
    /**
       sets  pedestal
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param val value to set
       \param rms rms to be set if any, defaults to 0
       \param m number of pedestal samples to be set or the moving stat structure is any, defaults to 0
    */
    virtual void setPedestal(int ix, int iy, double val, double rms=0, int m=-1){if (ix>=0 && ix<nx && iy>=0 && iy<ny) stat[iy][ix].setPedestal(val,rms, m);};
    
  /**
       sets  pedestal
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param val value to set
       \param rms rms to be set if any, defaults to 0
       \param m number of pedestal samples to be set or the moving stat structure is any, defaults to 0
    */
    virtual void setPedestal(double *ped, double *rms=NULL, int m=-1){
      double rr=0;
      for (int ix=xmin; ix<xmax; ix++) {
	for (int iy=ymin; iy<ymax; iy++) {
	  if (rms) rr=rms[iy*nx+ix];
	  stat[iy][ix].setPedestal(ped[iy*nx+ix],rr, m);
	};
      };

    }
    




    /**
       sets  pedestal rms
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param rms value to set
    */
    virtual void setPedestalRMS(int ix, int iy, double rms=0){if (ix>=0 && ix<nx && iy>=0 && iy<ny) stat[iy][ix].setPedestalRMS(rms);};
    



    /**
       sets  pedestal rms for all pixels
       \param rms pointer to array of pedestal rms
    */
 virtual void setPedestalRMS(double *rms){
      for (int ix=xmin; ix<xmax; ix++) {
	for (int iy=ymin; iy<ymax; iy++) {
	  stat[iy][ix].setPedestalRMS(rms[iy*nx+ix]);
	};
      };

    }
    


     /**
       write 32bit tiff file with detector image data
       \param imgname file name to be written
       \returns NULL if file writing didn't succed, otherwise a pointer
    */
  virtual void *writeImage(const char * imgname) {
    float *gm=NULL;
    void *ret;
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	    gm[iy*nx+ix]=image[iy*nx+ix];
	}
      }
      ret=WriteToTiff(gm, imgname, ny, nx);   
      delete [] gm;
      return ret;
  }
   
       /**
       write 32bit tiff file containing the pedestals
       \param imgname file name to be written
       \returns NULL if file writing didn't succed, otherwise a pointer
    */
    
  virtual void *writePedestals(const char * imgname) {
    float *gm=NULL;
    void *ret;
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  if (cmSub) 
	    gm[iy*nx+ix]=stat[iy][ix].getPedestal()-cmSub->getCommonMode();
	  else
	    gm[iy*nx+ix]=stat[iy][ix].getPedestal();
	}
      }
      ret=WriteToTiff(gm, imgname, ny, nx);   
      delete [] gm;
      return ret;
  }
  
 /**
      read 32bit tiff file containing the pedestals
       \param imgname file name to be read
       \returns 0 if file reading didn't succed, otherwise 1
    */
  int readPedestals(const char * imgname) {
    uint32 nnx, nny;
    float *gm=ReadFromTiff( imgname, nny, nnx);
    if (nnx>nx) nnx=nx;
    if (nny>ny) nny=ny;
    


    if (gm) {
      for (int ix=0; ix<nnx; ix++) {
	for (int iy=0; iy<nny; iy++) {
	  stat[iy][ix].setPedestal(gm[iy*nx+ix],-1,-1);
	}
      }
      delete [] gm;
      return 1;
    }
    return NULL;
  }
  
 /**
      read 32bit tiff file containing the image data
       \param imgname file name to be read
       \returns 0 if file reading didn't succed, otherwise 1
    */
  int readImage(const char * imgname) {
    uint32 nnx, nny;
    float *gm=ReadFromTiff( imgname, nny, nnx);
    if (nnx>nx) nnx=nx;
    if (nny>ny) nny=ny;
    


    if (gm) {
      for (int ix=0; ix<nnx; ix++) {
	for (int iy=0; iy<nny; iy++) {
	  image[iy*nx+ix]=gm[iy*nx+ix];
	}
      }
      delete [] gm;
      return 1;
    }
    return NULL;
  }
  
   
    /**
      returns pointer to image data
       \returns pointer to image data
    */  
  virtual int *getImage(){return image;};
      /**
      write 32bit tiff file containing the pedestals RMS
       \param imgname file name to be written
       \returns NULL if file writing didn't succed, otherwise a pointer
    */
  void *writePedestalRMS(const char * imgname) {
    float *gm=NULL;
    void *ret;
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	    gm[iy*nx+ix]=stat[iy][ix].getPedestalRMS();
	}
      }
      ret=WriteToTiff(gm, imgname, ny, nx);  
      delete [] gm;
      return ret;
    }
  
   /**
      read 32bit tiff file containing the pedestals RMS
       \param imgname file name to be read
       \returns 0 if file reading didn't succed, otherwise 1
    */

  int readPedestalRMS(const char * imgname) {
    uint32 nnx, nny;
    float *gm=ReadFromTiff( imgname, nny, nnx);
    if (nnx>nx) nnx=nx;
    if (nny>ny) nny=ny;
    if (gm) {
      for (int ix=0; ix<nnx; ix++) {
	for (int iy=0; iy<nny; iy++) {
	  stat[iy][ix].setPedestalRMS(gm[iy*nx+ix]);
	}
      }
      delete [] gm;
      return 1;
    }
    return 0;
  }
  
    




  /**
      Adds all the data for each pixels in the selected region of interest to the pedestal
       \param data pointer to the data
    */
    
    virtual void addToPedestal(char *data) {
      
      
      newFrame();
      
      
      for (int ix=xmin; ix<xmax; ix++) {
	for (int iy=ymin; iy<ymax; iy++) {
	  
	  
	  addToPedestal(data,ix,iy);
	  
	  
	}
      }
      return ;
      
    };



  /**
     Sets region of interest in which data should be processed
       \param xmi minimum x. if -1 or out of range remains unchanged
       \param xma maximum x. if -1 or out of range remains unchanged
       \param ymi minimum y. if -1 or out of range remains unchanged
       \param yma maximum y. if -1 or out of range remains unchanged
    */

    void setROI(int xmi=-1, int xma=-1, int ymi=-1, int yma=-1) {
      if (xmi>=0 && xmi<=nx) xmin=xmi;
      if (xma>=0 && xma<=nx) xmax=xma;
      if (xmax<xmin) {
	xmi=xmin;
	xmin=xmax;
	xmax=xmi;
      }
      
      if (ymi>=0 && ymi<=ny) ymin=ymi;
      if (yma>=0 && yma<=ny) ymax=yma;
      if (ymax<ymin) {
	ymi=ymin;
	ymin=ymax;
	ymax=ymi;
      }
      


    };
      /**
     Gets region of interest in which data are processed
       \param xmi reference to minimum x.
       \param xma reference to maximum x. 
       \param ymi reference to minimum y. 
       \param yma reference to maximum y. 
    */

    void getROI(int &xmi, int &xma, int &ymi, int &yma) {xmi=xmin; xma=xmax; ymi=ymin; yma=ymax;};




  /**
      Adds all the data for the selected pixel to the pedestal
       \param data pointer to the data
       \param ix pixel x coordinate
       \param iy pixel y coordinate
    */


    virtual void addToPedestal(char *data, int ix, int iy=0) {
      
      
      double val;
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	
	if (det)
	  val=dataSign*det->getValue(data, ix, iy);
	else
	  val=((double*)data)[iy*nx+ix];
	
	addToPedestal(val,ix,iy);
      }
      return  ;
      
    };
      
  
  /**
     Subtracts pedestal from the data array in the region of interest
       \param data pointer to the data
       \param val pointer where the pedestal subtracted data should be added. If NULL, the internal image is used
       \returns pointer to the pedestal subtracted data
    */

        
   virtual  double *subtractPedestal(char *data, double *val=NULL) {
      
      newFrame();

      if (val==NULL)
	val=new double[nx*ny];

      for (int ix=xmin; ix<xmax; ix++) {
	for (int iy=ymin; iy<ymax; iy++) {
	  val[iy*nx+ix]+=subtractPedestal(data, ix, iy);
	}
      }
      return  val;
    };
      



 /**
     Subtracts pedestal from the data for a selected pixel
       \param data pointer to the data
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \returns pedestal subtracted value
    */



   virtual  double subtractPedestal(char *data, int ix, int iy=0) {
     double g=1.;
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	if (gmap) {
	  g=gmap[iy*nx+ix];
	  if (g==0) g=-1.;
	}

	if (det)
	  return (dataSign*det->getValue(data, ix, iy)-getPedestal(ix,iy))/g;
	else
	  return (((double*)data)[iy*nx+ix]-getPedestal(ix,iy))/g;
      }
   };
      

   /**
      sets threshold value for conversion into number of photons
      \param t threshold to be set 
      \returns threshold value
    */
   double setThreshold(double t){thr=t; return thr;};
     
   /**
      gets threshold value for conversion into number of photons
      \returns threshold value
    */
   double getThreshold(){return thr;};
     /**
      converts the data into number of photons for the selected pixel
      \param data pointer to the data
      \param ix pixel x coordinate
      \param iy pixel y coordinate
      \returns converted number of photons. If no threshold is set, returns gain converted pedestal subtracted data.
    */

   virtual  int getNPhotons(char *data, int ix, int iy=0) {
     int nph=0;
     double v;
     if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
       v=subtractPedestal(data,ix,iy);
       if (thr>0) {
	 v+=0.5*thr;
	 nph=v/thr;
	 return nph;
       } else
	 return v;
     }
     return 0;  
   };
   
   /**
      converts the data into number of photons for all pixels
      \param data pointer to the data
      \param nph pointer where the photons should added. If NULL,the internal image is used
      \returns pointer to array containing the number of photons
   */
     int *getNPhotons(char *data,  int *nph=NULL) {
       
       double val;
     if (nph==NULL)
       nph=image;
     newFrame();
     for (int ix=xmin; ix<xmax; ix++) {
       for (int iy=ymin; iy<ymax; iy++) {
	 nph[iy*nx+ix]+=getNPhotons(data, ix, iy);
       }
     }
     return nph;
     
   }

 /**
      clears the image array
     
    */
   virtual void clearImage(){  
     for (int ix=0; ix<nx; ix++) {
       for (int iy=0; iy<ny; iy++) { 
	 image[iy*nx+ix]=0;
       }
     }
   };

    /** sets/gets number of samples for moving average pedestal calculation
	\param i number of samples to be set (0 or negative gets)
	\returns actual number of samples
    */
    int SetNPedestals(int i=-1) {
      int ix=0, iy=0; 
      if (i>0) 
	for (ix=0; ix<nx; ix++) 
	  for (iy=0; iy<ny; iy++) 
	    stat[iy][ix].SetNPedestals(i); 
      return stat[0][0].SetNPedestals();
    };

 /** gets number of samples for moving average pedestal calculation
	\returns actual number of samples
    */
    int GetNPedestals(int ix, int iy) {
      if  (ix>=0 &&  ix<nx && iy>=0 && iy<ny) 
	return stat[iy][ix].GetNPedestals(); 
      else
	return -1;
    };

 
 /** calculates the sum of photons in the specified region of interest. 
     \param data pointer to the data
     \param xmi minimum x for the calculation. If -1 the minimum x of the predefined region of interest is used
     \param xma maximum x for the calculation. If -1 the maximum x of the predefined region of interest is used
     \param ymi minimum y for the calculation. If -1 the minimum y of the predefined region of interest is used
     \param yma maximum y for the calculation. If -1 the maximum y of the predefined region of interest is used
     
	\returns total number of photons in
    */
    virtual int getTotalNumberOfPhotons(char *data, int xmi=-1, int xma=-1, int ymi=-1, int yma=-1) {
      int val=0;
      if (xmi<0) xmi=xmin;
      if (xma<0) xma=xmax;
      if (ymi<0) ymi=ymin;
      if (yma<0) yma=ymax;
      
      for (int ix=xmi; ix<xma; ix++)
	for (int iy=ymi; iy<yma; iy++)
	  if (ix>=0 && ix<nx && iy>=0 && iy<ny) 
	    val+=getNPhotons(data, ix, iy);
      return val;
      
    };


 /** 
     calculates the image converted into number of photons. If the frame mode is pedestal, it also it to the pdedestal subtraction.
     \param data pointer to the data to be processed
     \param val pointer of the data to be added to. If NULL, the internal image will be used
     \param pointer to the processed data
     \returns 
    */

    virtual void processData(char *data,int *val=NULL) {
      switch(fMode) {
      case ePedestal:
	addToPedestal(data);
	break;
      default:
	getNPhotons(data,val);
      }
    };

    virtual char *getInterpolation(){return NULL;};

 /** sets the current frame mode for the detector
     \param f frame mode to be set
     
     \returns current frame mode
    */
    frameMode setFrameMode(frameMode f) {fMode=f; return fMode;};
    
 /** gets the current frame mode for the detector
     \returns current frame mode
    */
    frameMode getFrameMode() {return fMode;};
    
/** sets file pointer where to write the clusters to 
    \param f file pointer
    \returns current file pointer
*/
FILE *setFilePointer(FILE *f){myFile=f; return myFile;};

/** gets file pointer where to write the clusters to 
    \returns current file pointer
*/
FILE *getFilePointer(){return myFile;};
 void setMutex(pthread_mutex_t *m){fm=m;};
 protected:
  
    slsDetectorData<dataType> *det; /**< slsDetectorData to be used */
    int nx; /**< Size of the detector in x direction */
    int ny; /**< Size of the detector in y direction */
    pedestalSubtraction **stat; /**< pedestalSubtraction class */
    commonModeSubtraction *cmSub;/**< commonModeSubtraction class */
    int dataSign; /**< sign of the data i.e. 1 if photon is positive, -1 if negative */
    int iframe;  /**< frame number (not from file but incremented within the dataset every time newFrame is called */
    double *gmap;
    int *image;
    int id;
    //int xmin, xmax, ymin, ymax;  int xmin; /**< minimum x of the region of interest */
    int xmin; /**< minimum x of the region of interest */
    int xmax; /**< maximum x of the region of interest */
    int ymin;/**< minimum y of the region of interest */
    int ymax;/**< maximum y of the region of interest */
    double thr; /**< threshold to be used for conversion into number of photons */
    //  int nSigma; /**< number of sigma to be used for conversion into number of photons if threshold is undefined */
    frameMode fMode; /**< current detector frame mode */
    FILE *myFile; /**< file pointer to write to */
    
    pthread_mutex_t *fm;
};

#endif
