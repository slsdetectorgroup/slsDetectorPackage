#ifndef ANALOGDETECTOR_H
#define  ANALOGDETECTOR_H


#include "slsDetectorData.h"
#include "pedestalSubtraction.h"
#include "commonModeSubtraction.h"
#include "tiffIO.h"
#ifndef FRAMEMODE_DEF
#define FRAMEMODE_DEF
 enum frameMode { eFrame, ePedestal, eFlat };
#endif


template <class dataType> class analogDetector {

  /** @short class to perform pedestal subtraction etc. for an analog detector */

 public:

 

  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param d detector data structure to be used
     \param csize cluster size (should be an odd number). Defaults to 3
     \param nsigma number of rms to discriminate from the noise. Defaults to 5
     \param sign 1 if photons are positive, -1 if  negative
     \param cm common mode subtraction algorithm, if any. Defaults to NULL i.e. none
     \param nped number of samples for pedestal averaging
     \param nd number of dark frames to average as pedestals without photon discrimination at the beginning of the measurement


  */
  

 analogDetector(slsDetectorData<dataType> *d, int sign=1, 
		commonModeSubtraction *cm=NULL, int nped=1000, int nnx=-1, int nny=-1, double *gm=NULL) : det(d), nx(nnx), ny(nny), stat(NULL), cmSub(cm),  iframe(-1), dataSign(sign), gmap(gm) {
    
    if (det)
      det->getDetectorSize(nx,ny);
    
    stat=new pedestalSubtraction*[ny];
    for (int i=0; i<ny; i++) {
      stat[i]=new pedestalSubtraction[nx];
      for (int ix=0; ix<nx; ix++) {
	stat[i][ix].SetNPedestals(nped);
      }
    }
   
    
  };
  /**
     destructor. Deletes the cluster structure and the pdestalSubtraction array
  */
  virtual ~analogDetector() {for (int i=0; i<ny; i++) delete [] stat[i]; delete [] stat; };

  /**
     clone
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
  }

  virtual analogDetector *Clone() {
    return new analogDetector(this);
  }

  int getDataSize(){return det->getDataSize();}; 

  /**
     set gain map
  */
  double *setGainMap(double *gm) {gmap=gm; return gmap;};

   /**
     return gain map
  */
  double *getGainMap() {return gmap;};

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
  
  void *writeGainMap(const char * imgname) {
    float *gm=NULL;
    if (gmap)  {
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  gm[iy*nx+ix]=gmap[iy*nx+ix];
	}
      }
      return WriteToTiff(gm, imgname, ny, nx);    
    }
    return NULL;
  }
  
  
    
  /** resets the pedestalSubtraction array and the commonModeSubtraction */
  void newDataSet(){iframe=-1; for (int iy=0; iy<ny; iy++) for (int ix=0; ix<nx; ix++) stat[iy][ix].Clear(); if (cmSub) cmSub->Clear(); };  
  
  /** resets the eventMask to undefined and the commonModeSubtraction */
  void newFrame(){iframe++; if (cmSub) cmSub->newFrame();};
  

    /** sets the commonModeSubtraction algorithm to be used 
	\param cm commonModeSubtraction algorithm to be used (NULL unsets) 
	\returns pointer to the actual common mode subtraction algorithm
    */
    commonModeSubtraction *setCommonModeSubtraction(commonModeSubtraction *cm) {cmSub=cm; return cmSub;};


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
    */
    virtual double getPedestal(int ix, int iy, int cm=0){if (ix>=0 && ix<nx && iy>=0 && iy<ny) if (cmSub && cm>0) return stat[iy][ix].getPedestal()-cmSub->getCommonMode(); else return stat[iy][ix].getPedestal(); else return -1;};

    /**
       gets  pedestal rms (i.e. noise)
       \param ix pixel x coordinate
       \param iy pixel y coordinate
    */
    double getPedestalRMS(int ix, int iy){if (ix>=0 && ix<nx && iy>=0 && iy<ny) return stat[iy][ix].getPedestalRMS();else return -1;};

    
    /**
       sets  pedestal
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param val value to set
    */
    virtual void setPedestal(int ix, int iy, double val, double rms=0, int m=-1){if (ix>=0 && ix<nx && iy>=0 && iy<ny) stat[iy][ix].setPedestal(val,rms, m);};
    
    /**
       sets  pedestal
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param val value to set
    */
    virtual void setPedestalRMS(int ix, int iy, double rms=0){if (ix>=0 && ix<nx && iy>=0 && iy<ny) stat[iy][ix].setPedestalRMS(rms);};
    
   
    
  void *writePedestals(const char * imgname) {
    float *gm=NULL;
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  if (cmSub) 
	    gm[iy*nx+ix]=stat[iy][ix].getPedestal()-cmSub->getCommonMode();
	  else
	    gm[iy*nx+ix]=stat[iy][ix].getPedestal();
	}
      }
      WriteToTiff(gm, imgname, ny, nx);   
      delete [] gm;
      return NULL;
  }
  
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
  
    

    
  void *writePedestalRMS(const char * imgname) {
    float *gm=NULL;
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	    gm[iy*nx+ix]=stat[iy][ix].getPedestalRMS();
	}
      }
      WriteToTiff(gm, imgname, ny, nx);  
      delete [] gm;
      return   NULL;
    }
  
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
  
    



    
    virtual void addToPedestal(char *data) {
      
      
      newFrame();
      
      
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  
	  
	  addToPedestal(data,ix,iy);
	  
	  
	}
      }
      return ;
      
    };
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
      
 
        
   virtual  double *subtractPedestal(char *data, double *val=NULL) {
      
      newFrame();

      if (val==NULL)
	val=new double[nx*ny];

      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  val[iy*nx+ix]+=subtractPedestal(data, ix, iy);
	}
      }
      return  val;
    };
      




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
      


   virtual  int getNPhotons(char *data, int ix, int iy=0, int thr=-1) {
     double g=1.;
     if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	
       if (gmap) {
	  g=gmap[iy*nx+ix];
	  if (g==0) g=-1.;
	}

	if (thr<=0) thr=-1*thr*getPedestalRMS(ix,iy)/g;
	
	if (det)
	  return (dataSign*det->getValue(data, ix, iy)-getPedestal(ix,iy))/g/thr;
	else
	  return (((double*)data)[(iy)*nx+ix]-getPedestal(ix,iy))/g/thr;
	
      }
      return 0;
      
   };

   int *getNPhotons(char *data, double thr=-1, int *nph=NULL) {

      double val;
      if (nph==NULL)
	nph=new int[nx*ny];   
      double tthr=thr;
      newFrame();
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  nph[iy*nx+ix]+=getNPhotons(data, ix, iy, thr);
	}
      }
      return nph;

    }



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

    int GetNPedestals(int ix, int iy) {
      if  (ix>=0 &&  ix<nx && iy>=0 && iy<ny) 
	return stat[iy][ix].GetNPedestals(); 
      else
	return -1;
    };

 
    double getROI(char *data, int xmin, int xmax, int ymin=0, int ymax=1) {
      double val=0;
      for (int ix=xmin; ix<xmax; ix++)
	for (int iy=ymin; iy<ymax; iy++)
	  if (ix>=0 && ix<nx && iy>=0 && iy<ny) 
	    val+=subtractPedestal(data, ix, iy);
      return val;
      
    };


    virtual void processData(char *data, frameMode i=eFrame, int *val=NULL) {
      switch(i) {
      case ePedestal:
	addToPedestal(data);
	break;
      default:
	getNPhotons(data,-1,val);
      }
    };


 protected:
  
    slsDetectorData<dataType> *det; /**< slsDetectorData to be used */
    int nx; /**< Size of the detector in x direction */
    int ny; /**< Size of the detector in y direction */
    pedestalSubtraction **stat; /**< pedestalSubtraction class */
    commonModeSubtraction *cmSub;/**< commonModeSubtraction class */
    int dataSign; /**< sign of the data i.e. 1 if photon is positive, -1 if negative */
    int iframe;  /**< frame number (not from file but incremented within the dataset every time newFrame is called */
    double *gmap;
};

#endif
