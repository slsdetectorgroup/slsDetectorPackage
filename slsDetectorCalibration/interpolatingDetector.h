#ifndef INTERPOLATINGDETECTOR_H
#define  INTERPOLATINGDETECTOR_H


#include "singlePhotonDetector.h"

#include "slsInterpolation.h"


#ifdef MYROOT1
#include <TTree.h>

#endif


#include <iostream>

using namespace std;


class interpolatingDetector : public singlePhotonDetector {

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
  
  
 interpolatingDetector(slsDetectorData<uint16_t> *d, slsInterpolation *inte,
		       double nsigma=5,  
		       int sign=1, 
		       commonModeSubtraction *cm=NULL,
		       int nped=1000, 
		       int nd=100, int nnx=-1, int nny=-1) : 
  singlePhotonDetector(d, 3,nsigma,sign, cm, nped, nd, nnx, nny) , interp(inte), id(0)  {};
  
  
  
 interpolatingDetector(interpolatingDetector *orig) : singlePhotonDetector(orig) {
    interp=(orig->interp)->Clone();
    id=orig->id;
  }


  virtual interpolatingDetector *Clone() {
    return new interpolatingDetector(this);
  }

  virtual int setId(int i) {id=i; interp->setId(id); return id;};

  virtual void prepareInterpolation(int &ok) {
    cout << "*"<< endl;
#ifdef SAVE_ALL
    char tit[1000];
    sprintf(tit,"/scratch/ped_%d.tiff",id);
    writePedestals(tit);
    sprintf(tit,"/scratch/ped_rms_%d.tiff",id);
    writePedestalRMS(tit);
    if (gmap) {
      sprintf(tit,"/scratch/gmap_%d.tiff",id);
      writeGainMap(tit);
    }
#endif
    interp->prepareInterpolation(ok);
  } 

  int getImageSize(int &nnx, int &nny, int &ns) {return interp->getImageSize(nnx, nny, ns);};
  #ifdef MYROOT1
  virtual TH2F *getInterpolatedImage()
#endif
#ifndef MYROOT1
  virtual int *getInterpolatedImage()
#endif
 {  
   if (interp)
     return interp->getInterpolatedImage();
 }

#ifdef MYROOT1
  virtual TH2F *addToInterpolatedImage(char *data, single_photon_hit *clusters, int &nph)
#endif
#ifndef MYROOT1
  virtual int *addToInterpolatedImage(char *data, single_photon_hit *clusters, int &nph)
#endif
  {
    nph=addFrame(data,clusters,0); 
    if (interp)
      return interp->getInterpolatedImage();
    else
      return NULL;
  };
  

#ifdef MYROOT1
  virtual TH2F *addToFlatField(char *data, single_photon_hit *clusters, int &nph)
#endif
#ifndef MYROOT1
    virtual int *addToFlatField(char *data, single_photon_hit *clusters, int &nph)
#endif
  {
    nph=addFrame(data,clusters,1); 
    if (interp)
      return interp->getFlatField();
    else
      return NULL;
  };
  
   void *writeInterpolatedImage(const char * imgname) {    
     cout << id << "=" << imgname<< endl;
     interp->writeInterpolatedImage(imgname);  
     return NULL;
   }
  
  
  int addFrame(char *data, single_photon_hit *clusters=NULL, int ff=0) {
    
    
    single_photon_hit *cl;
    single_photon_hit clust;
    if (clusters)
      cl=clusters;
    else
      cl=&clust;

    double int_x,int_y, eta_x, eta_y;
    int nph=0;
    double val[ny][nx];
    int cy=(clusterSizeY+1)/2;
    int cs=(clusterSize+1)/2;
    int ir, ic;
    double cc[2][2];
    double max=0, tl=0, tr=0, bl=0,br=0, *v, vv;
    int xoff,yoff;
    int skip=0;
    if (iframe<nDark) {
      //cout << iframe << "+"<< nDark <<endl;
      addToPedestal(data);
      return 0;
    }

    newFrame();
    for (int ix=1; ix<nx-1; ix++) {
      for (int iy=1; iy<ny-1; iy++) {
	skip=0;
	max=0;
	tl=0;
	tr=0;
	bl=0;
	br=0;
	tot=0;
	quadTot=0;
	quad=UNDEFINED_QUADRANT;
      
      
      
      
	cl->rms=getPedestalRMS(ix,iy);
	//(clusters+nph)->rms=getPedestalRMS(ix,iy);

	//	cout << iframe << " " << nph << " " << ix << " " << iy << endl;
	
      for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	  

	  //  if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
	    
	    if ((iy>1 && ir<=0)) {
	      ;
	    } else if (ix>1 && ic<=0) {
	      ;
	    } else {
	      val[iy+ir][ix+ic]=subtractPedestal(data,ix+ic,iy+ir);
	      eventMask[iy+ir][ix+ic]=PEDESTAL; 
	    }

	    /* if ((iy+ir)>=iy  && (ix+ic)>=ix) { */
	    /*   val[iy+ir][ix+ic]=subtractPedestal(data,ix+ic,iy+ir); */
	    /*   eventMask[iy+ir][ix+ic]=PEDESTAL;  */
	    /* }  */
	  
	  //   cout << ir << " " << ic << " " << val[iy+ir][ix+ic] << endl;
	    v=&(val[iy+ir][ix+ic]);
	
	    if (ir==0 && ic==0) {
	      if (*v<-nSigma*cl->rms) {
		eventMask[iy][ix]=NEGATIVE_PEDESTAL;
		//	cout << "neg ped" << endl;
	      }
	    } 

	    // if (skip==0) {
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
	      // }
	    
	    //  } else skip=1;
	}
      }
	
	if (bl>=br && bl>=tl && bl>=tr) {
	  cl->quad=BOTTOM_LEFT;
	  cl->quadTot=bl;
	  xoff=0;
	  yoff=0;
	} else if (br>=bl && br>=tl && br>=tr) {
	  cl->quad=BOTTOM_RIGHT;
	  cl->quadTot=br;
	  xoff=1;
	  yoff=0;
	} else if (tl>=br && tl>=bl && tl>=tr) {
	  cl->quad=TOP_LEFT;
	  cl->quadTot=tl;
	  xoff=0;
	  yoff=1;
	} else if   (tr>=bl && tr>=tl && tr>=br) {
	  cl->quad=TOP_RIGHT;
	  cl->quadTot=tr;
	  xoff=1;
	  yoff=1;
	} 
	
	if (max>nSigma*cl->rms || tot>sqrt(clusterSizeY*clusterSize)*nSigma*cl->rms || (cl->quadTot)>sqrt(cy*cs)*nSigma*cl->rms) {
	  if (val[iy][ix]>=max) {
	    // cout << "max" << endl;
	    eventMask[iy][ix]=PHOTON_MAX;
	    cl->tot=tot;
	    cl->x=ix;
	    cl->y=iy;
	    cl->ped=getPedestal(ix,iy, 0);
	    //	  cout << iframe << " " << ix  << " " << iy << " "<< (clusters+nph)->tot << " " << (clusters+nph)->quadTot << " " << (clusters+nph)->ped<< " " << (clusters+nph)->rms << endl; 
	    for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	      for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
		if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
		  cl->set_data(val[iy+ir][ix+ic],ic,ir);
		  
		    
		  //  cout << val[iy+ir][ix+ic] << " " ;
		}
	      }
	      //   cout << endl << " " ;
	    }
	    
	    // cout << endl << " " ;
	    
	  cc[0][0]=cl->get_data(-1+xoff,-1+yoff);
	  cc[1][0]=cl->get_data(-1+xoff,0+yoff);
	  cc[0][1]=cl->get_data(0+xoff,-1+yoff);
	  cc[1][1]=cl->get_data(0+xoff,0+yoff);
	  
	  //  cout << cc[0][0] << " " << cc[0][1] << endl;
	  // cout << cc[1][0] << " " << cc[1][1] << endl;
	  

	  // cout << endl << " " ;
	  
	if (interp) {
	  //interp->calcEta((clusters+nph)->quadTot,cc,eta_x, eta_y);
	  interp->calcEtaL(cl->quadTot,cl->quad,cc,eta_x, eta_y);
	  // cout << eta_x << " " << eta_y << endl;
	  // cout << "eta" << endl;
	  if (ff) {
	    interp->addToFlatField(eta_x,eta_y);
	    // cout << "**************************************************************************"<< endl;
	  } else {
	    //   cout << "interp" << endl;
	    interp->getInterpolatedPosition(ix,iy,eta_x,eta_y,cl->quad,int_x,int_y);
	    // cout << "add" << endl;
	    interp->addToImage(int_x, int_y);
	  }
	  // cout << "done" << endl;
	}	      
	if (clusters) 	  cl=(clusters+nph);	  
	
	nph++;
	  } else {
	    //   cout << "ph" << endl;
	    eventMask[iy][ix]=PHOTON;
	  }
      } else if (eventMask[iy][ix]==PEDESTAL) {
	  //  cout << "ped" << endl;
	addToPedestal(data,ix,iy);
      }
	
      }
    }
    
    return  nph;
    
  };

  
    virtual void processData(char *data, frameMode i=eFrame, int *val=NULL) {
      switch(i) {
      case ePedestal:
	addToPedestal(data);
	break;
      case eFlat:
	addFrame(data,NULL,1);
	break;
      default:
	 addFrame(data);
      }
    };

  
 protected:
  
  
  slsInterpolation *interp;
  int id;
};





#endif
