#ifndef ANALOGDETECTOR_H
#define  ANALOGDETECTOR_H


#include "slsDetectorData.h"
#include "pedestalSubtraction.h"
#include "commonModeSubtraction.h"
#include "tiffIO.h"

#ifndef FRAMEMODE_DEF
#define FRAMEMODE_DEF
/** 
enum to define the flags of the data set, which are needed to seect the type of processing it should undergo
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
    
  };
  /**
     destructor. Deletes the cluster structure and the pdestalSubtraction array
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
     wrties a 32 bit tiff file of the size of the detector and contaning the gain map value, if any. If file doesn'e exist or gainmap is undefined, does not do anything.
     \param imgname complete name of the file to be written
     \returns NULL
  */
  void *writeGainMap(const char * imgname) {
    float *gm=NULL;
    if (gmap)  {
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  gm[iy*nx+ix]=gmap[iy*nx+ix];
	}
      }
      WriteToTiff(gm, imgname, ny, nx);   
      delete [] gm;
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
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
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
    
 virtual void setPedestalRMS(double *rms){
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  stat[iy][ix].setPedestalRMS(rms[iy*nx+ix]);
	};
      };

    }
    


     /**
       sets  pedestal rms
       \param ix pixel x coordinate
       \param iy pixel y coordinate
       \param rms value to set
    */
  virtual void *writeImage(const char * imgname) {
    float *gm=NULL;
      gm=new float[nx*ny];
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	    gm[iy*nx+ix]=image[iy*nx+ix];
	}
      }
      WriteToTiff(gm, imgname, ny, nx);   
      delete [] gm;
      return NULL;
  }
   
    
  virtual void *writePedestals(const char * imgname) {
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
  
    
  virtual int *getImage(){return image;};
    
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
     int v;
     if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	
       if (gmap) {
	  g=gmap[iy*nx+ix];
	  if (g==0) g=-1.;
	}

	if (thr<=0) thr=-1*thr*getPedestalRMS(ix,iy)/g;
	
	/* if (det) */
	/*   return (dataSign*det->getValue(data, ix, iy)-getPedestal(ix,iy))/g/thr; */
	/* else { */
	//if (ix==30 && iy==50 )
	  v=subtractPedestal(data,ix,iy)/g/thr;
	  if (v>0)
	  return v;
	  //cout << v << endl;
	  return 0;
	  //	}
      }
      return 0;
      
   };

   int *getNPhotons(char *data, double thr=-1, int *nph=NULL) {

      double val;
      if (nph==NULL)
	nph=image;//image=new int[nx*ny];   
      double tthr=thr;
      newFrame();
      for (int ix=0; ix<nx; ix++) {
	for (int iy=0; iy<ny; iy++) {
	  nph[iy*nx+ix]+=getNPhotons(data, ix, iy, thr);
	}
      }
      return nph;

    }

   virtual void clearImage(){  for (int ix=0; ix<nx; ix++) {
       for (int iy=0; iy<ny; iy++) { image[iy*nx+ix]=0;}}};

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

    virtual char *getInterpolation(){return NULL;};

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
    //int xmin, xmax, ymin, ymax;
};

#endif
