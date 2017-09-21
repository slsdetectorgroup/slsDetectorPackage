#ifndef ANALOGDETECTOR_H
#define  ANALOGDETECTOR_H


#include "slsDetectorData.h"
#include "pedestalSubtraction.h"
#include "commonModeSubtraction.h"


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
		       commonModeSubtraction *cm=NULL, int nnx=-1, int nny=-1) : det(d), nx(nnx), ny(nny), stat(NULL), cmSub(cm),  iframe(-1), dataSign(sign){
    
    if (det)
      det->getDetectorSize(nx,ny);
    
    stat=new pedestalSubtraction*[ny];
    for (int i=0; i<ny; i++) {
      stat[i]=new pedestalSubtraction[nx];
    }
   
    
  };
  /**
     destructor. Deletes the cluster structure and the pdestalSubtraction array
  */
  virtual ~analogDetector() {for (int i=0; i<ny; i++) delete [] stat[i]; delete [] stat;};
    
    
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
    virtual void setPedestal(int ix, int iy, double val, double rms=0){if (ix>=0 && ix<nx && iy>=0 && iy<ny) stat[iy][ix].setPedestal(val,rms);};
    
    
    
    
    
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
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	if (det)
	  return dataSign*det->getValue(data, ix, iy)-getPedestal(ix,iy);
	else
	  return ((double*)data)[iy*nx+ix]-getPedestal(ix,iy);
      }
   };
      


   virtual  int getNPhotons(char *data, int ix, int iy=0, int thr=-1) {
      
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	if (thr<=0) thr=-1*thr*getPedestalRMS(ix,iy);
	
	if (det)
	  return (dataSign*det->getValue(data, ix, iy)-getPedestal(ix,iy))/thr;
	else
	  return (((double*)data)[(iy)*nx+ix]-getPedestal(ix,iy))/thr;
	
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

 
    double getROI(char *data, int xmin, int xmax, int ymin=0, int ymax=1) {
      double val=0;
      for (int ix=xmin; ix<xmax; ix++)
	for (int iy=ymin; iy<ymax; iy++)
	  if (ix>=0 && ix<nx && iy>=0 && iy<ny) 
	    val+=subtractPedestal(data, ix, iy);
      return val;
      
    }



 protected:
  
    slsDetectorData<dataType> *det; /**< slsDetectorData to be used */
    int nx; /**< Size of the detector in x direction */
    int ny; /**< Size of the detector in y direction */
    pedestalSubtraction **stat; /**< pedestalSubtraction class */
    commonModeSubtraction *cmSub;/**< commonModeSubtraction class */
    int dataSign; /**< sign of the data i.e. 1 if photon is positive, -1 if negative */
    int iframe;  /**< frame number (not from file but incremented within the dataset every time newFrame is called */
    
};

#endif
