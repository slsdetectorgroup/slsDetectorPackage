#ifndef INTERPOLATINGDETECTOR_H
#define  INTERPOLATINGDETECTOR_H


#include "singlePhotonDetector.h"

#include "slsInterpolation.h"

//#define M015

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
  singlePhotonDetector(d, 3,nsigma,sign, cm, nped, nd, nnx, nny) , interp(inte), id(0)  {
    //cout << "**"<< xmin << " " << xmax << " " << ymin << " " << ymax << endl;

};
  
  
  
 interpolatingDetector(interpolatingDetector *orig) : singlePhotonDetector(orig) {
    interp=(orig->interp)->Clone();
    id=orig->id;
    /* xmin=orig->xmin; */
    /* xmax=orig->xmax; */
    /* ymin=orig->ymin; */
    /* ymax=orig->ymax; */
    
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
    if (interp)
      interp->prepareInterpolation(ok);
  } 


  void clearImage() {if (interp) interp->clearInterpolatedImage(); else singlePhotonDetector::clearImage();};

  int getImageSize(int &nnx, int &nny, int &ns) {if (interp) return interp->getImageSize(nnx, nny, ns); else return analogDetector<uint16_t>::getImageSize(nnx, nny, ns);};
  #ifdef MYROOT1
  virtual TH2F *getImage()
#endif
#ifndef MYROOT1
  virtual int *getImage()
#endif
 {  
   // cout << "image " << endl;
   if (interp)
     return interp->getInterpolatedImage();
   else 
     return analogDetector<uint16_t>::getImage();
   //cout << "null " << endl;
 }

#ifdef MYROOT1
  virtual TH2F *addToInterpolatedImage(char *data, int *val, int &nph)
#endif
#ifndef MYROOT1
    virtual int *addToInterpolatedImage(char *data, int *val, int &nph)
#endif
  {
    nph=addFrame(data,val,0); 
    if (interp)
      return interp->getInterpolatedImage();
    else
      singlePhotonDetector::getImage();
      //return NULL;
  };
  

#ifdef MYROOT1
  virtual TH2F *addToFlatField(char *data, int *val, int &nph)
#endif
#ifndef MYROOT1
    virtual int *addToFlatField(char *data, int *val, int &nph)
#endif
  {
    nph=addFrame(data,val,1); 
    if (interp)
      return interp->getFlatField();
    else
      return NULL;
  };
  
   void *writeImage(const char * imgname) {    
     //  cout << id << "=" << imgname<< endl;
     if (interp)
       interp->writeInterpolatedImage(imgname); 
     else
       analogDetector<uint16_t>::writeImage(imgname);
     return NULL;
   }
  
int addFrame(char *data,  int *ph=NULL, int ff=0) {

 
  int nph=0;
  double val[ny][nx];
  int cy=(clusterSizeY+1)/2;
  int cs=(clusterSize+1)/2;
  int ir, ic;
  double rms;
    double int_x,int_y, eta_x, eta_y;
  double max=0, tl=0, tr=0, bl=0,br=0, *v, vv;
  // cout << "********** Add frame "<< iframe << endl;
  if (ph==NULL)
    ph=image;

  if (iframe<nDark) {
    addToPedestal(data);
    return 0;
  }
  newFrame();
  // cout << "********** Data "<< endl;
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
      
	rms=getPedestalRMS(ix,iy);
	(clusters+nph)->rms=rms;
      
      
      
      for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	  
	  if ((iy+ir)>=iy && (iy+ir)<ny && (ix+ic)>=ix && (ix+ic)<nx) {
	    val[iy+ir][ix+ic]=subtractPedestal(data,ix+ic,iy+ir);
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
	    if (*v<-nSigma*rms)
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
      
      if (max>nSigma*rms || tot>sqrt(clusterSizeY*clusterSize)*nSigma*rms || ((clusters+nph)->quadTot)>sqrt(cy*cs)*nSigma*rms) {
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



	    
	    if (interp) {
	      if (ff) {
		  interp->addToFlatField((clusters+nph)->quadTot,(clusters+nph)->quad,(clusters+nph)->get_cluster(),eta_x, eta_y);
	      } else {
		interp->getInterpolatedPosition(ix, iy, (clusters+nph)->quadTot,(clusters+nph)->quad,(clusters+nph)->get_cluster(),int_x, int_y);
		interp->addToImage(int_x, int_y);
	      }
	    }  else 
	      image[ix+nx*iy]++;    










	  nph++;
	  image[iy*nx+ix]++;

	  } else {
	    eventMask[iy][ix]=PHOTON;
	  }
      } else if (eventMask[iy][ix]==PEDESTAL) {
	addToPedestal(data,ix,iy);
      }


    }
  }
  nphFrame=nph;
  nphTot+=nph;
  //cout << nphFrame << endl;
  // cout <<"**********************************"<< endl;
  writeClusters();
  return  nphFrame;
  
};

   /*********************************************************
  int addFrame(char *data, int ff=0) {
    
    double g=1;
    
    single_photon_hit *cl;
    single_photon_hit clust;
    if (clusters)
      cl=clusters;
    else
      cl=&clust;

    
      int ccs=clusterSize;
      int ccy=clusterSizeY;

    double int_x,int_y, eta_x, eta_y;
    int nph=0;
    double rest[ny][nx];
    int cy=(clusterSizeY+1)/2;
    int cs=(clusterSize+1)/2;
    int ir, ic;
    double cc[2][2];
    double max=0, tl=0, tr=0, bl=0,br=0, v, vv;
    int xoff,yoff;
    int skip=0;
    //  cout <<"fr"<< endl;
    double tthr;
    if (iframe<nDark) {
      //cout << iframe << "+"<< nDark <<endl;
      addToPedestal(data);
      return 0;
    }

    newFrame();




    //  cout << xmin << " " << xmax << " " << ymin << " " << ymax << endl;
	for (int ix=xmin; ix<xmax; ix++) {
	  for (int iy=ymin; iy<ymax; iy++) {
	    //  for (int ix=clusterSize/2; ix<clusterSize/2-1; ix++) {
	    // for (int iy=clusterSizeY/2; iy<ny-clusterSizeY/2; iy++) {

	    //	    cout << ix << " " << iy << endl;
	    eventMask[iy][ix]=PEDESTAL;
	    
	    
	      tthr=nSigma*getPedestalRMS(ix,iy)/g;
	      if (ix==xmin || iy==ymin)
		rest[iy][ix]=subtractPedestal(data,ix,iy);
	     
	    
	    max=0;
	    tl=0;
	    tr=0;
	    bl=0;
	    br=0;
	    tot=0;
	    quadTot=0;

	    for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	      for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
		if ((iy+ir)>=ymin && (iy+ir)<ymax && (ix+ic)>=xmin && (ix+ic)<xmax) {
		  //cluster->set_data(rest[iy+ir][ix+ic], ic, ir);
		  
		  if (ir>=0 && ic>=0 ) 
		    rest[iy+ir][ix+ic]=subtractPedestal(data,ix+ic,iy+ir);
		  
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
		  // if (ir==0 && ic==0) {
		    if (v>tthr) {
		      eventMask[iy][ix]=NEIGHBOUR;
		    }
		    //}
		}
	      }
	    }
	    if (rest[iy][ix]<=-tthr) {
	      eventMask[iy][ix]=NEGATIVE_PEDESTAL;
	      //if (cluster->get_data(0,0)>=max) {
	    } else  if (max>tthr || tot>sqrt(ccy*ccs)*tthr || quadTot>sqrt(cy*cs)*tthr) {
	      if (rest[iy][ix]>=max) { 
		if (bl>=br && bl>=tl && bl>=tr) {
		  cl->quad=BOTTOM_LEFT;
		cl->quadTot=bl;
		} else if (br>=bl && br>=tl && br>=tr) {
		  cl->quad=BOTTOM_RIGHT;
		cl->quadTot=br;
	      } else if (tl>=br && tl>=bl && tl>=tr) {
		cl->quad=TOP_LEFT;
		cl->quadTot=tl;
	      } else if   (tr>=bl && tr>=tl && tr>=br) {
		cl->quad=TOP_RIGHT;
		cl->quadTot=tr;
	      }
	     
	
		eventMask[iy][ix]=PHOTON_MAX;
		cl->tot=tot;
		cl->x=ix;
		cl->y=iy;
		cl->ped=getPedestal(ix,iy, 0);
		cl->rms=getPedestalRMS(ix,iy);
	    
	    for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	      for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
		if ((iy+ir)>=ymin && (iy+ir)<ymax && (ix+ic)>=xmin && (ix+ic)<xmax) {
		  cl->set_data(rest[iy+ir][ix+ic],ic,ir);
		}
	      }
	    }
	    
	    
	    if (interp) {
	      if (ff) {
#ifdef M015
		if (iy>100)
#endif
		  interp->addToFlatField(cl->quadTot,cl->quad,cl->get_cluster(),eta_x, eta_y);
		// if ((eta_x<0.1 || eta_x>0.9)&&(eta_y<0.1 || eta_y>0.9))
		//  cout << ix << " " << iy << " " << eta_x <<" " << eta_y << endl;  
		
	      } else {
		interp->getInterpolatedPosition(ix, iy, cl->quadTot,cl->quad,cl->get_cluster(),int_x, int_y);
		interp->addToImage(int_x, int_y);
	      }
	      
	    }  else 
	      image[ix+nx*iy]++;    

	    nph++;
	    
	    if (clusters) cl=(clusters+nph);	  
	  

		// rest[iy][ix]-=tthr;
	      } else 
		eventMask[iy][ix]=PHOTON;
	      //else if (thr<=0 ) {
		//addToPedestal(data,ix,iy);
	      // }
	    }
	    if (eventMask[iy][ix]==PEDESTAL) {
	      addToPedestal(data,ix,iy);
	    }
	  }
	}
	return nph;
  }
      
   ******************************************/













































  /*   for (int ix=0; ix<nx; ix++) { */
  /*     for (int iy=0; iy<ny; iy++) { */
  /* 	skip=0; */
  /* 	max=0; */
  /* 	tl=0; */
  /* 	tr=0; */
  /* 	bl=0; */
  /* 	br=0; */
  /* 	tot=0; */
  /* 	quadTot=0; */
  /* 	quad=UNDEFINED_QUADRANT; */
      
      
      
      
  /* 	cl->rms=getPedestalRMS(ix,iy); */
  /* 	//(clusters+nph)->rms=getPedestalRMS(ix,iy); */

  /* 	//	cout << iframe << " " << nph << " " << ix << " " << iy << endl; */
  /* 	if (ix==0 || iy==0) */
  /* 	   val[iy][ix]=subtractPedestal(data,ix,iy); */

  /* 	if (val[iy][ix]<-nSigma*cl->rms) { */
  /* 	  eventMask[iy][ix]=NEGATIVE_PEDESTAL; */
  /* 		//	cout << "neg ped" << endl; */
  /* 	} else { */
  /* 	  for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) { */
  /* 	    for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) { */
	  

  /* 	    if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) { */
	    
  /* 	  if (ir>=0 && ic>=0) { */
  /* 	    val[iy+ir][ix+ic]=subtractPedestal(data,ix+ic,iy+ir); */
  /* 	    eventMask[iy+ir][ix+ic]=PEDESTAL;  */
  /* 	  } */
	  
  /* 	  //   cout << ir << " " << ic << " " << val[iy+ir][ix+ic] << endl; */
  /* 	    v=&(val[iy+ir][ix+ic]); */
  /* 	    // if (skip==0) { */
  /* 	      tot+=*v; */
  /* 	      if (ir<=0 && ic<=0) */
  /* 		bl+=*v; */
  /* 	      if (ir<=0 && ic>=0) */
  /* 		br+=*v; */
  /* 	      if (ir>=0 && ic<=0) */
  /* 		tl+=*v; */
  /* 	      if (ir>=0 && ic>=0) */
  /* 		tr+=*v; */
  /* 	      if (*v>max) { */
  /* 		max=*v; */
  /* 	      } */
	     
  /* 	} */
  /*     } */
  /* 	  } */
	
  /* 	if (bl>=br && bl>=tl && bl>=tr) { */
  /* 	  cl->quad=BOTTOM_LEFT; */
  /* 	  cl->quadTot=bl; */
  /* 	} else if (br>=bl && br>=tl && br>=tr) { */
  /* 	  cl->quad=BOTTOM_RIGHT; */
  /* 	  cl->quadTot=br; */
  /* 	} else if (tl>=br && tl>=bl && tl>=tr) { */
  /* 	  cl->quad=TOP_LEFT; */
  /* 	  cl->quadTot=tl; */
  /* 	} else if   (tr>=bl && tr>=tl && tr>=br) { */
  /* 	  cl->quad=TOP_RIGHT; */
  /* 	  cl->quadTot=tr; */
  /* 	}  */
	
  /* 	if (max>nSigma*cl->rms || tot>sqrt(clusterSizeY*clusterSize)*nSigma*cl->rms || (cl->quadTot)>sqrt(cy*cs)*nSigma*cl->rms) { */
  /* 	  if (val[iy][ix]>=max) { */
  /* 	    eventMask[iy][ix]=PHOTON_MAX; */
  /* 	    cl->tot=tot; */
  /* 	    cl->x=ix; */
  /* 	    cl->y=iy; */
  /* 	    cl->ped=getPedestal(ix,iy, 0); */
	      
  /* 	      if (interp) { */
  /* 		if (ff) { */
  /* 		  interp->addToFlatField(cl->quadTot,cl->quad,cl->get_cluster(),eta_x, eta_y); */
  /* 		  // if ((eta_x<0.1 || eta_x>0.9)&&(eta_y<0.1 || eta_y>0.9)) */
  /* 		  //  cout << ix << " " << iy << " " << eta_x <<" " << eta_y << endl;   */
  /* 		} else { */
  /* 		  interp->getInterpolatedPosition(ix, iy, cl->quadTot,cl->quad,cl->get_cluster(),int_x, int_y); */
  /* 		  interp->addToImage(int_x, int_y); */
  /* 		} */
  /* 	      }  else  */
  /* 		image[ix+nx*iy]++;     */

  /* 	      if (clusters) cl=(clusters+nph);	   */
	  
  /* 	      nph++; */

  /* 	  } else { */
  /* 	    eventMask[iy][ix]=PHOTON; */
  /* 	  } */
  /* 	} else if (eventMask[iy][ix]==PEDESTAL) { */
  /* 	  addToPedestal(data,ix,iy); */
  /* 	} */
	  
  /* 	} */
  /*     } */
  /*     } */
  /*     return  nph; */
    
  /* }; */
  







  
    virtual void processData(char *data, int *val=NULL) {
      if (interp){
	switch(fMode) {
	case ePedestal:
	  addToPedestal(data);
	  break;
	case eFlat:
	  addFrame(data,val,1);
	  break;
	default:
	  addFrame(data,val,0);
	}
      } else
	singlePhotonDetector::processData(data,val);
      
	
    };

    virtual char *getInterpolation(){return (char*)interp;};
  
 protected:
  
  
  slsInterpolation *interp;
  int id;
};





#endif
