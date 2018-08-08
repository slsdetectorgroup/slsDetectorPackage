#ifndef SINGLEPHOTONDETECTOR_H
#define SINGLEPHOTONDETECTOR_H

#include "analogDetector.h" 

#include "single_photon_hit.h"



//#define MYROOT1

#ifdef MYROOT1
#include <TTree.h>
#endif


#ifndef EVTYPE_DEF
#define EVTYPE_DEF
/** enum to define the even types */
enum eventType {
  PEDESTAL=0, /** pedestal */
  NEIGHBOUR=1, /** neighbour i.e. below threshold, but in the cluster of a photon */
  PHOTON=2, /** photon i.e. above threshold */
  PHOTON_MAX=3, /** maximum of a cluster satisfying the photon conditions */
  NEGATIVE_PEDESTAL=4, /** negative value, will not be accounted for as pedestal in order to avoid drift of the pedestal towards negative values */
  UNDEFINED_EVENT=-1 /** undefined */
};
#endif

//template <class dataType> class singlePhotonDetector : 
//public analogDetector<dataType> {
class singlePhotonDetector : 
public analogDetector<uint16_t> {

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
  

 singlePhotonDetector(slsDetectorData<uint16_t> *d,
		      int csize=3,
		      double nsigma=5,
		      int sign=1,
		      commonModeSubtraction *cm=NULL,
		      int nped=1000,
		      int nd=100, int nnx=-1, int nny=-1, double *gm=NULL) : analogDetector<uint16_t>(d, sign, cm, nped, nnx, nny, gm),   nDark(nd), eventMask(NULL),nSigma (nsigma), clusterSize(csize), clusterSizeY(csize), clusters(NULL),   quad(UNDEFINED_QUADRANT), tot(0), quadTot(0) {
    
    
    
    
    eventMask=new eventType*[ny];
    for (int i=0; i<ny; i++) {
      eventMask[i]=new eventType[nx];
    }
    
    if (ny==1)
      clusterSizeY=1;

    // cluster=new single_photon_hit(clusterSize,clusterSizeY);
    clusters=new single_photon_hit[nx*ny];
   
    //  cluster=clusters;
    setClusterSize(csize);
    nphTot=0;
    nphFrame=0;
  };
    /**
       destructor. Deletes the cluster structure, the pdestalSubtraction and the image array
    */
  virtual ~singlePhotonDetector() {delete [] clusters; for (int i=0; i<ny; i++) delete [] eventMask[i]; delete [] eventMask; };

    
  
  /**
     copy constructor
     \param orig detector to be copied
     
  */
 
 singlePhotonDetector(singlePhotonDetector *orig) : analogDetector<uint16_t>(orig) {

    nDark=orig->nDark;
    myFile=orig->myFile;
    
    eventMask=new eventType*[ny];
    for (int i=0; i<ny; i++) {
      eventMask[i]=new eventType[nx];
    }
    
    
    nSigma=orig->nSigma;
    clusterSize=orig->clusterSize;
    clusterSizeY=orig->clusterSizeY;
    // cluster=new single_photon_hit(clusterSize,clusterSizeY);
    clusters=new single_photon_hit[nx*ny];

    // cluster=clusters;
    
    setClusterSize(clusterSize);
    
    quad=UNDEFINED_QUADRANT;
    tot=0;
    quadTot=0;
    gmap=orig->gmap;
    nphTot=0;
    nphFrame=0;

  }
  

  
  /**
     duplicates the detector structure
     \returns new single photon detector with same parameters
     
  */
  virtual singlePhotonDetector *Clone() {
    return new singlePhotonDetector(this);
  }
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
	//	if (clusters)
	//  delete [] clusters;
	if (ny>clusterSize)
	  clusterSizeY=clusterSize;
	else
	  clusterSizeY=1;
	for (int ip=0; ip<nx*ny; ip++)
	  (clusters+ip)->set_cluster_size(clusterSize,clusterSizeY);
	    //cluster=new single_photon_hit(clusterSize,clusterSizeY);
      }
      return clusterSize;
    }



    /**
       converts the image into number of photons
       \param data pointer to data
       \param nph pointer where to add the calculated photons. If NULL, the internal image will be used
       \returns array with data converted into number of photons.
    */
    
    virtual int *getNPhotons(char *data,  int *nph=NULL) {

      nphFrame=0;
      double val;
      if (nph==NULL)
	nph=image;
      //nph=new int[nx*ny];
      
      double rest[ny][nx];
      int cy=(clusterSizeY+1)/2;
      int cs=(clusterSize+1)/2;
      
      int ccs=clusterSize;
      int ccy=clusterSizeY;
      
      double g=1.;

      
      double tthr=thr, tthr1, tthr2;
      int nn=0;
      double max=0, tl=0, tr=0, bl=0,br=0, v;
      double rms=0;
      
      int cm=0;
      if (cmSub) cm=1;

      if (thr>0) {
	cy=1;
	cs=1;
	ccs=1;
	ccy=1;
      }
      if (iframe<nDark) { 
	//	cout << "ped " << iframe << endl;
	//this already adds to common mode
	addToPedestal(data);
	return nph;
      }	else {
	if (thr>0) {
	  newFrame();
	  if (cmSub) {
	    addToCommonMode(data);
	  }
	  for (int ix=xmin; ix<xmax; ix++) {
	    for (int iy=ymin; iy<ymax; iy++) {
	      
	      val=subtractPedestal(data,ix,iy, cm);
	      
	      nn=val/tthr;//analogDetector<uint16_t>::getNPhotons(data,ix,iy);
	      nph[ix+nx*iy]+=nn;
	      rest[iy][ix]=(val-nn*tthr);
	      
	      nphFrame+=nn;
	      nphTot+=nn;
	    }
	  }
	  //	}
	for (int ix=xmin; ix<xmax; ix++) {
	  for (int iy=ymin; iy<ymax; iy++) {
	    
	    //  for (int ix=clusterSize/2; ix<clusterSize/2-1; ix++) {
	    // for (int iy=clusterSizeY/2; iy<ny-clusterSizeY/2; iy++) {
	    eventMask[iy][ix]=PEDESTAL;
	    max=0;
	    tl=0;
	    tr=0;
	    bl=0;
	    br=0;
	    tot=0;
	    quadTot=0;

	    if (rest[iy][ix]>0.25*tthr) {
	      eventMask[iy][ix]=NEIGHBOUR;
	      for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
		for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
		  if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
		    //clusters->set_data(rest[iy+ir][ix+ic], ic, ir);
		    
		
		    v=rest[iy+ir][ix+ic];//clusters->get_data(ic,ir);
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
		    // if (ir==0 && ic==0) {
		    //}
		  }
		}
	      }
	      
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
		rms=getPedestalRMS(ix,iy);
		tthr1=tthr-sqrt(ccy*ccs)*rms;
		tthr2=tthr-sqrt(cy*cs)*rms;
		if (tthr>sqrt(ccy*ccs)*rms) tthr1=tthr-sqrt(ccy*ccs)*rms; else tthr1=sqrt(ccy*ccs)*rms;
		if (tthr>sqrt(cy*cs)*rms) tthr2=tthr-sqrt(cy*cs)*rms; else tthr2=sqrt(cy*cs)*rms;
		if (tot>tthr1 || quadTot>tthr2) {
		  eventMask[iy][ix]=PHOTON;
		  nph[ix+nx*iy]++;
		  nphFrame++;
		  nphTot++;
		  
		} 
	      }
	    }
	  }
	}
	//	cout << iframe << " " << nphFrame << " " << nphTot << endl;
	//cout << iframe << " " << nph << endl;
	} else return getClusters(data, nph);
      }
      return NULL;
    };


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
      double val;
      tot=0;
      quadTot=0;
      quad=UNDEFINED_QUADRANT;

      if (iframe<nDark) {
	addToPedestal(data, ix,iy);
	return UNDEFINED_EVENT;
      }


     
      

      //   if (eventMask[iy][ix]==UNDEFINED) {
	
      eventMask[iy][ix]=PEDESTAL;
	
	
      clusters->x=ix;
      clusters->y=iy;
      clusters->rms=getPedestalRMS(ix,iy);
      clusters->ped=getPedestal(ix,iy, cm);
	

      for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	    if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
	      
	      v=subtractPedestal(data, ix+ic, iy+ir);
	      
	      clusters->set_data(v, ic, ir);
	      //  v=clusters->get_data(ic,ir);
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
		if (v<-nSigma*clusters->rms)
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
	
	if (max>nSigma*clusters->rms || tot>sqrt(clusterSizeY*clusterSize)*nSigma*clusters->rms || quadTot>cy*cs*nSigma*clusters->rms) {
	  if (clusters->get_data(0,0)>=max) {
	    eventMask[iy][ix]=PHOTON_MAX;
	  } else {
	    eventMask[iy][ix]=PHOTON;
	  }
	} else if (eventMask[iy][ix]==PEDESTAL) {
	  if (cm==0) {
	    if (det)
	      val=dataSign*det->getValue(data, ix, iy);
	    else
	      val=((double**)data)[iy][ix];
	    addToPedestal(val,ix,iy);
	  }
	}
      


      return  eventMask[iy][ix];

  };




/**
   Loops in the region of interest to find the clusters
   \param data pointer to the data structure
   \returns number of clusters found

 */

int *getClusters(char *data,  int *ph=NULL) {

 
  int nph=0;
  double val[ny][nx];
  int cy=(clusterSizeY+1)/2;
  int cs=(clusterSize+1)/2;
  int ir, ic;
  
  double max=0, tl=0, tr=0, bl=0,br=0, *v, vv;
  int cm=0;
  if (cmSub) cm=1;
  if (ph==NULL)
    ph=image;

  if (iframe<nDark) {
    addToPedestal(data);
    return 0;
  }
  newFrame();


  
  if (cm)
    addToCommonMode(data);


  for (int ix=xmin; ix<xmax; ix++) {
    for (int iy=ymin; iy<ymax; iy++) {
      
      max=0;
      tl=0;
      tr=0;
      bl=0;
      br=0;
      tot=0;
      quadTot=0;
      quad=UNDEFINED_QUADRANT;

     

      eventMask[iy][ix]=PEDESTAL;
      
	
      (clusters+nph)->rms=getPedestalRMS(ix,iy);
      // cluster=clusters+nph;
      
      
      for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	  
	  if ((iy+ir)>=iy && (iy+ir)<ny && (ix+ic)>=ix && (ix+ic)<nx) {
	    val[iy+ir][ix+ic]=subtractPedestal(data,ix+ic,iy+ir, cm);
	  }
	  
	  v=&(val[iy+ir][ix+ic]);
	  tot+=*v;
	  if (ir<=0 && ic<=0)
	    bl+=*v;
	  if (ir<=0 && ic>=0)
	    br+=*v;
	  if (ir>=0 && ic<=0)
	    tl+=*v;
	  if (ir>=0 && ic>=0)
	    tr+=*v;
	  if (*v>max) {
	    max=*v;
	  }
	  
	  
	  if (ir==0 && ic==0) {
	    if (*v<-nSigma*(clusters+nph)->rms)
	      eventMask[iy][ix]=NEGATIVE_PEDESTAL;
	  }
	  
	}
      }
      
      if (bl>=br && bl>=tl && bl>=tr) {
	(clusters+nph)->quad=BOTTOM_LEFT;
	(clusters+nph)->quadTot=bl;
      } else if (br>=bl && br>=tl && br>=tr) {
	(clusters+nph)->quad=BOTTOM_RIGHT;
	(clusters+nph)->quadTot=br;
      } else if (tl>=br && tl>=bl && tl>=tr) {
	(clusters+nph)->quad=TOP_LEFT;
	(clusters+nph)->quadTot=tl;
      } else if   (tr>=bl && tr>=tl && tr>=br) {
	(clusters+nph)->quad=TOP_RIGHT;
	(clusters+nph)->quadTot=tr;
      }
      
      if (max>nSigma*(clusters+nph)->rms || tot>sqrt(clusterSizeY*clusterSize)*nSigma*(clusters+nph)->rms || ((clusters+nph)->quadTot)>sqrt(cy*cs)*nSigma*(clusters+nph)->rms) {
	if (val[iy][ix]>=max) {
	  eventMask[iy][ix]=PHOTON_MAX;
	  (clusters+nph)->tot=tot;
	  (clusters+nph)->x=ix;
	  (clusters+nph)->y=iy;
	  (clusters+nph)->iframe=det->getFrameNumber(data);
	  (clusters+nph)->ped=getPedestal(ix,iy,0);
	  for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	    for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	      (clusters+nph)->set_data(val[iy+ir][ix+ic],ic,ir);
	    }
	  }
	  //	  cout << (clusters+nph)->iframe << " " << ix << " " << nph << " " << tot << " " << (clusters+nph)->quadTot << endl;
	  nph++;
	  image[iy*nx+ix]++;

	  } else {
	    eventMask[iy][ix]=PHOTON;
	  }
      } else if (eventMask[iy][ix]==PEDESTAL) {
	addToPedestal(data,ix,iy,cm);
      }


    }
  }
  nphFrame=nph;
  nphTot+=nph;
  //cout << nphFrame << endl;
  // cout <<"**********************************"<< endl;
  writeClusters();
  return  image;
  
};


    /**<
       returns the total signal in a cluster
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

  
    /** returns value for cluster element in relative coordinates
	\param ic x coordinate (center is (0,0))
	\param ir y coordinate (center is (0,0))
	\returns cluster element
    */
    double getClusterElement(int ic, int ir=0){return clusters->get_data(ic,ir);};

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
	tall->Branch("iFrame",&(clusters->iframe),"iframe/I");

      tall->Branch("x",&(clusters->x),"x/I");
      tall->Branch("y",&(clusters->y),"y/I");
      char tit[100];
      sprintf(tit,"data[%d]/D",clusterSize*clusterSizeY);
      tall->Branch("data",clusters->data,tit);
      tall->Branch("pedestal",&(clusters->ped),"pedestal/D");
      tall->Branch("rms",&(clusters->rms),"rms/D");
      tall->Branch("tot",&(clusters->tot),"tot/D");
      tall->Branch("quadTot",&(clusters->quadTot),"quadTot/D");
      tall->Branch("quad",&(clusters->quad),"quad/I");
      return tall;
    };
#else
/** write cluster to filer
     \param f file pointer
*/
    void writeCluster(FILE* f){clusters->write(f);};

/** 
    write clusters to file
    \param f file pointer
    \param clusters array of clusters structures
    \param nph number of clusters to be written to file

*/

static void writeClusters(FILE *f, single_photon_hit *clusters, int nph){for (int i=0; i<nph; i++) (clusters+i)->write(f);};
void writeClusters(FILE *f){for (int i=0; i<nphFrame; i++) (clusters+i)->write(f);};
 void writeClusters(){if (myFile) {  
     //cout << "++" << endl;  
     pthread_mutex_lock(fm);
     for (int i=0; i<nphFrame; i++) 
       (clusters+i)->write(myFile);
     pthread_mutex_unlock(fm); 
     //cout << "--" << endl; 
   } 
 };
#endif

    virtual void processData(char *data, int *val=NULL) {
      // cout << "sp" << endl;
      switch(fMode) {
      case ePedestal:
	addToPedestal(data);
	break;
      default:
	getNPhotons(data,val);
      }
      iframe++;
      //	cout << "done" << endl;
    };
    int getPhFrame(){return nphFrame;};
    int getPhTot(){return nphTot;};
 protected:

    int nDark; /**< number of frames to be used at the beginning of the dataset to calculate pedestal without applying photon discrimination */
    eventType **eventMask; /**< matrix of event type or each pixel */
    double nSigma; /**< number of sigma parameter for photon discrimination */
    int clusterSize; /**< cluster size in the x direction */
    int clusterSizeY; /**< cluster size in the y direction i.e. 1 for strips, clusterSize for pixels */
    //  single_photon_hit *cluster; /**< single photon hit data structure */
    single_photon_hit *clusters; /**< single photon hit data structure */
    quadrant quad; /**< quadrant where the photon is located */
    double tot; /**< sum of the 3x3 cluster */
    double quadTot; /**< sum of the maximum 2x2cluster */
    int nphTot;
    int nphFrame;


    };





#endif
