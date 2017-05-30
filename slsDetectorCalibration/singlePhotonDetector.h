#ifndef SINGLEPHOTONDETECTOR_H
#define  SINGLEPHOTONDETECTOR_H


#include "slsDetectorData.h"

#include "single_photon_hit.h"
#include "pedestalSubtraction.h"
#include "commonModeSubtraction.h"


//#define MYROOT1

#ifdef MYROOT1
#include <TTree.h>

#endif


#include <iostream>

using namespace std;


  enum eventType {
    PEDESTAL=0,
    NEIGHBOUR=1,
    PHOTON=2,
    PHOTON_MAX=3,
    NEGATIVE_PEDESTAL=4,
    UNDEFINED_EVENT=-1
  };

#ifndef DEF_QUAD
#define DEF_QUAD
  enum quadrant {
    TOP_LEFT=0,
    TOP_RIGHT=1,
    BOTTOM_LEFT=2,
    BOTTOM_RIGHT=3,
    UNDEFINED_QUADRANT=-1
  };
#endif


template <class dataType>
class singlePhotonDetector {

  /** @short class to perform pedestal subtraction etc. and find single photon clusters for an analog detector */

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
  

  singlePhotonDetector(slsDetectorData<dataType> *d, 
		       int csize=3, 
		       double nsigma=5,  
		       int sign=1, 
		       commonModeSubtraction *cm=NULL,
		       int nped=1000, 
		       int nd=100) : det(d), nx(0), ny(0), stat(NULL), cmSub(cm),  nDark(nd), eventMask(NULL),nSigma (nsigma), clusterSize(csize), clusterSizeY(csize), cluster(NULL), iframe(-1), dataSign(sign), quad(UNDEFINED_QUADRANT), tot(0), quadTot(0) {

   
    det->getDetectorSize(nx,ny);
    


    stat=new pedestalSubtraction*[ny];
    eventMask=new eventType*[ny];
    for (int i=0; i<ny; i++) {
      stat[i]=new pedestalSubtraction[nx];
      stat[i]->SetNPedestals(nped);
      eventMask[i]=new eventType[nx];
    }
   
    if (ny==1)
      clusterSizeY=1;

    cluster=new single_photon_hit(clusterSize,clusterSizeY);
    setClusterSize(csize);
    
  };
    /**
       destructor. Deletes the cluster structure and the pdestalSubtraction array
    */
    virtual ~singlePhotonDetector() {delete cluster; for (int i=0; i<ny; i++) delete [] stat[i]; delete [] stat;};

    
    /** resets the pedestalSubtraction array and the commonModeSubtraction */
    void newDataSet(){iframe=-1; for (int iy=0; iy<ny; iy++) for (int ix=0; ix<nx; ix++) stat[iy][ix].Clear(); if (cmSub) cmSub->Clear(); };  

    /** resets the eventMask to undefined and the commonModeSubtraction */
    void newFrame(){iframe++; for (int iy=0; iy<ny; iy++) for (int ix=0; ix<nx; ix++) eventMask[iy][ix]=UNDEFINED_EVENT; if (cmSub) cmSub->newFrame();};


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
    virtual void addToPedestal(double val, int ix, int iy){ 
      //	cout << "*"<< ix << " " << iy << " " << val << endl;
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	//	cout << ix << " " << iy << " " << val << endl;
	stat[iy][ix].addToPedestal(val); 
	if (cmSub && det->isGood(ix, iy) ) 
	  cmSub->addToCommonMode(val, ix, iy);
      };
    };

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

 

  

    /** sets/gets number of rms threshold to detect photons
	\param n number of sigma to be set (0 or negative gets)
	\returns actual number of sigma parameter
    */
    double setNSigma(double n=-1){if (n>0) nSigma=n; return nSigma;}
  
    /** sets/gets cluster size
	\param n cluster size to be set, (0 or negative gets). If even is incremented by 1.
	\returns actual cluster size
    */
    int setClusterSize(int n=-1){
      if (n>0 && n!=clusterSize) {
	if (n%2==0)
	  n+=1;
	clusterSize=n; 
	if (cluster)
	  delete cluster;    
	if (ny>1)
	  clusterSizeY=clusterSize;
	cluster=new single_photon_hit(clusterSize,clusterSizeY);
      }
      return clusterSize;
    }



    int *getNPhotons(char *data, double thr=-1) {

      double val;
      int *nph=new int[nx*ny];
      double rest[ny][nx];
      int cy=(clusterSizeY+1)/2;
      int cs=(clusterSize+1)/2;

      int ccs=clusterSize;
      int ccy=clusterSizeY;


      double tthr=thr;
      int nn;
      double max=0, tl=0, tr=0, bl=0,br=0, v;


      if (thr>=0) {
	cy=1;
	cs=1;
	ccs=1;
	ccy=1;
      }


      for (int ix=0; ix<nx; ix++) {
      for (int iy=0; iy<ny; iy++) {
	
	val=dataSign*(det->getValue(data, ix, iy)-getPedestal(ix,iy,0));
	
	if (thr<=0) tthr=nSigma*getPedestalRMS(ix,iy);
	
	nn=val/tthr;
	
	if (nn>0) {
	  rest[iy][ix]=val-nn*tthr;
	  nph[ix+nx*iy]=nn;
	} else {
	  rest[iy][ix]=val;
	  nph[ix+nx*iy]=0;
	}
      }
      
      }
      for (int ix=clusterSize/2; ix<clusterSize/2-1; ix++) {
	for (int iy=clusterSizeY/2; iy<ny-clusterSizeY/2; iy++) {
	  eventMask[iy][ix]=PEDESTAL;
	
	  if (thr<=0) tthr=nSigma*getPedestalRMS(ix,iy);
	  
	  max=0; 
	  tl=0;
	  tr=0; 
	  bl=0;
	  br=0;
	  
	  for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	    for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	      if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
		//cluster->set_data(rest[iy+ir][ix+ic], ic, ir);
		v=rest[iy+ir][ix+ic];//cluster->get_data(ic,ir);
		tot+=v;
		if (ir<=0 && ic<=0)
		  bl+=v;
		if (ir<=0 && ic>=0)
		  br+=v;
		if (ir>=0 && ic<=0)
		  tl+=v;
		if (ir>=0 && ic>=0)
		tr+=v;
		
		if (v>max) {
		  max=v;
		}
		if (ir==0 && ic==0) {
		  if (v>tthr) {
		    eventMask[iy][ix]=PHOTON;
		  } 
		}
	      }
	    }
	  }
	  

	  //if (cluster->get_data(0,0)>=max) {
	    if (rest[iy][ix]>=max) {
	    
	    if (bl>=br && bl>=tl && bl>=tr) {
	      quad=BOTTOM_LEFT;
	      quadTot=bl;
	    } else if (br>=bl && br>=tl && br>=tr) {
	      quad=BOTTOM_RIGHT;
	    quadTot=br;
	    } else if (tl>=br && tl>=bl && tl>=tr) {
	      quad=TOP_LEFT;
	      quadTot=tl;
	    } else if   (tr>=bl && tr>=tl && tr>=br) {
	      quad=TOP_RIGHT;
	      quadTot=tr;
	    } 
	    if (rest[iy][ix]>tthr || tot>sqrt(ccy*ccs)*tthr || quadTot>sqrt(cy*cs)*tthr) {
	      nph[ix+nx*iy]++;
	      rest[iy][ix]-=tthr;
	    }
	  }      
	}
	
      }

      
      return nph;

    }



    /** finds event type for pixel and fills cluster structure. The algorithm loops only if the evenMask for this pixel is still undefined.
	if pixel or cluster around it are above threshold (nsigma*pedestalRMS) cluster is filled and pixel mask is PHOTON_MAX (if maximum in cluster) or NEIGHBOUR; If PHOTON_MAX, the elements of the cluster are also set as NEIGHBOURs in order to speed up the looping
	if below threshold the pixel is either marked as PEDESTAL (and added to the pedestal calculator) or NEGATIVE_PEDESTAL is case it's lower than -threshold, otherwise the pedestal average would drift to negative values while it should be 0.

	/param data pointer to the data
	/param ix pixel x coordinate
	/param iy pixel y coordinate
	/param cm enable(1)/disable(0) common mode subtraction (if defined). 
	/returns event type for the given pixel
    */
    eventType getEventType(char *data, int ix, int iy, int cm=0) {

      // eventType ret=PEDESTAL;
      double max=0, tl=0, tr=0, bl=0,br=0, v;
      //  cout << iframe << endl;
    
      int cy=(clusterSizeY+1)/2;
      int cs=(clusterSize+1)/2;



      tot=0;
      quadTot=0;
      quad=UNDEFINED_QUADRANT;


      if (iframe<nDark) {
	if (cm==0) {
	  //  cout << "=" << endl;
	  addToPedestal(det->getValue(data, ix, iy),ix,iy);
	  cluster->set_data(dataSign*(det->getValue(data, ix, iy)-getPedestal(ix,iy,cm)), 0,0       );
	  // cout << "=" << endl;
	}
	return UNDEFINED_EVENT;
      }
      
      

      //   if (eventMask[iy][ix]==UNDEFINED) {
	
	eventMask[iy][ix]=PEDESTAL;
	
	
	cluster->x=ix;
	cluster->y=iy;
	cluster->rms=getPedestalRMS(ix,iy);
	cluster->ped=getPedestal(ix,iy, cm);
	

	for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	  for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	    if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
	      cluster->set_data(dataSign*(det->getValue(data, ix+ic, iy+ir)-getPedestal(ix+ic,iy+ir,cm)), ic, ir       );
	      v=cluster->get_data(ic,ir);
	      tot+=v;
	      if (ir<=0 && ic<=0)
		bl+=v;
	      if (ir<=0 && ic>=0)
		br+=v;
	      if (ir>=0 && ic<=0)
		tl+=v;
	      if (ir>=0 && ic>=0)
		tr+=v;
	      
	      if (v>max) {
		max=v;
	      }
	      if (ir==0 && ic==0) {
		if (v<-nSigma*cluster->rms)
		  eventMask[iy][ix]=NEGATIVE_PEDESTAL;
	      }
	    }
	  }
	}
	
	if (bl>=br && bl>=tl && bl>=tr) {
	  quad=BOTTOM_LEFT;
	  quadTot=bl;
	} else if (br>=bl && br>=tl && br>=tr) {
	  quad=BOTTOM_RIGHT;
	  quadTot=br;
	} else if (tl>=br && tl>=bl && tl>=tr) {
	  quad=TOP_LEFT;
	  quadTot=tl;
     	} else if   (tr>=bl && tr>=tl && tr>=br) {
	  quad=TOP_RIGHT;
	  quadTot=tr;
	} 
	
	if (max>nSigma*cluster->rms || tot>sqrt(clusterSizeY*clusterSize)*nSigma*cluster->rms || quadTot>cy*cs*nSigma*cluster->rms) {
	  if (cluster->get_data(0,0)>=max) {
	    eventMask[iy][ix]=PHOTON_MAX;
	  } else {
	    eventMask[iy][ix]=PHOTON;
	  }
	} else if (eventMask[iy][ix]==PEDESTAL) {
	  if (cm==0)
	    addToPedestal(det->getValue(data, ix, iy),ix,iy);
	}
      
	

      return  eventMask[iy][ix];

  };

    /**<
       retrurns the total signal in a cluster
       \param size cluser size  should be 1,2 or 3
       \returns cluster center if size=1, sum of the maximum quadrant if size=2, total of the cluster if size=3 or anything else
    */

    double getClusterTotal(int size) {
      switch (size) {
      case 1:
	return getClusterElement(0,0);
      case 2:
	return quadTot;
      default:
	return tot;
      };
    };

      /**<
       retrurns the quadrant with maximum signal
       \returns quadrant where the cluster is located    */

    quadrant getQuadrant() {return quad;};

    /** sets/gets number of samples for moving average pedestal calculation
	\param i number of samples to be set (0 or negative gets)
	\returns actual number of samples
    */
    int SetNPedestals(int i=-1) {int ix=0, iy=0; if (i>0) for (ix=0; ix<nx; ix++) for (iy=0; iy<ny; iy++) stat[iy][ix].SetNPedestals(i); return stat[0][0].SetNPedestals();};

    /** returns value for cluster element in relative coordinates
	\param ic x coordinate (center is (0,0))
	\param ir y coordinate (center is (0,0))
	\returns cluster element
    */
    double getClusterElement(int ic, int ir=0){return cluster->get_data(ic,ir);};

    /** returns event mask for the given pixel
	\param ic x coordinate (center is (0,0))
	\param ir y coordinate (center is (0,0))
	\returns event mask enum for the given pixel
    */
    eventType getEventMask(int ic, int ir=0){return eventMask[ir][ic];};
 

#ifdef MYROOT1  
    /** generates a tree and maps the branches
	\param tname name for the tree
	\param iFrame pointer to the frame number
	\returns returns pointer to the TTree
    */
    TTree *initEventTree(char *tname, int *iFrame=NULL) {
      TTree* tall=new TTree(tname,tname);

      if (iFrame)
	tall->Branch("iFrame",iFrame,"iframe/I");
      else
	tall->Branch("iFrame",&(cluster->iframe),"iframe/I");

      tall->Branch("x",&(cluster->x),"x/I");
      tall->Branch("y",&(cluster->y),"y/I");
      char tit[100];
      sprintf(tit,"data[%d]/D",clusterSize*clusterSizeY);
      tall->Branch("data",cluster->data,tit);
      tall->Branch("pedestal",&(cluster->ped),"pedestal/D");
      tall->Branch("rms",&(cluster->rms),"rms/D");
      return tall;
    };
#else
    /** write cluster to filer*/
    void writeCluster(FILE* myFile){cluster->write(myFile);};

#endif


 private:
  
    slsDetectorData<dataType> *det; /**< slsDetectorData to be used */
    int nx; /**< Size of the detector in x direction */
    int ny; /**< Size of the detector in y direction */


    pedestalSubtraction **stat; /**< pedestalSubtraction class */
    commonModeSubtraction *cmSub;/**< commonModeSubtraction class */
    int nDark; /**< number of frames to be used at the beginning of the dataset to calculate pedestal without applying photon discrimination */
    eventType **eventMask; /**< matrix of event type or each pixel */
    double nSigma; /**< number of sigma parameter for photon discrimination */
    int clusterSize; /**< cluster size in the x direction */
    int clusterSizeY; /**< cluster size in the y direction i.e. 1 for strips, clusterSize for pixels */
    single_photon_hit *cluster; /**< single photon hit data structure */
    int iframe;  /**< frame number (not from file but incremented within the dataset every time newFrame is called */
    int dataSign; /**< sign of the data i.e. 1 if photon is positive, -1 if negative */
    quadrant quad; /**< quadrant where the photon is located */
    double tot; /**< sum of the 3x3 cluster */
    double quadTot; /**< sum of the maximum 2x2cluster */


};





#endif
