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
    if (orig->interp)
      interp=(orig->interp)->Clone();
    else
      interp=orig->interp;
    id=orig->id;
  }


  virtual interpolatingDetector *Clone() {
    return new interpolatingDetector(this);
  }

  virtual int setId(int i) {id=i; interp->setId(id); return id;};

  virtual void prepareInterpolation(int &ok) {
   /*  cout << "*"<< endl; */
/* #ifdef SAVE_ALL */
/*     char tit[1000]; */
/*     sprintf(tit,"/scratch/ped_%d.tiff",id); */
/*     writePedestals(tit); */
/*     sprintf(tit,"/scratch/ped_rms_%d.tiff",id); */
/*     writePedestalRMS(tit); */
/*     if (gmap) { */
/*       sprintf(tit,"/scratch/gmap_%d.tiff",id); */
/*       writeGainMap(tit); */
/*     } */
/* #endif */
    if (interp)
      interp->prepareInterpolation(ok);
  } 


  void clearImage() {
    if (interp) interp->clearInterpolatedImage(); 
    else singlePhotonDetector::clearImage();
  };

  int getImageSize(int &nnx, int &nny, int &ns) {
    if (interp) return interp->getImageSize(nnx, nny, ns); 
    else return analogDetector<uint16_t>::getImageSize(nnx, nny, ns);
  };


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
  
/* int addFrame(char *data,  int *ph=NULL, int ff=0) { */

 
/*   int nph=0; */
/*   double val[ny][nx]; */
/*   int cy=(clusterSizeY+1)/2; */
/*   int cs=(clusterSize+1)/2; */
/*   int ir, ic; */
/*   double rms; */
/*     double int_x,int_y, eta_x, eta_y; */
/*   double max=0, tl=0, tr=0, bl=0,br=0, *v, vv; */
/*   // cout << "********** Add frame "<< iframe << endl; */
/*   if (ph==NULL) */
/*     ph=image; */

/*   if (iframe<nDark) { */
/*     addToPedestal(data); */
/*     return 0; */
/*   } */
/*   newFrame(); */
/*   // cout << "********** Data "<< endl; */
/*   for (int ix=xmin; ix<xmax; ix++) { */
/*     for (int iy=ymin; iy<ymax; iy++) { */
      
/*       max=0; */
/*       tl=0; */
/*       tr=0; */
/*       bl=0; */
/*       br=0; */
/*       tot=0; */
/*       quadTot=0; */
/*       quad=UNDEFINED_QUADRANT; */

     

/*       eventMask[iy][ix]=PEDESTAL; */
      
/* 	rms=getPedestalRMS(ix,iy); */
/* 	(clusters+nph)->rms=rms; */
      
      
      
/*       for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) { */
/* 	for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) { */
	  
/* 	  if ((iy+ir)>=iy && (iy+ir)<ny && (ix+ic)>=ix && (ix+ic)<nx) { */
/* 	    val[iy+ir][ix+ic]=subtractPedestal(data,ix+ic,iy+ir); */
/* 	  } */
	  
/* 	  v=&(val[iy+ir][ix+ic]); */
/* 	  tot+=*v; */
/* 	  if (ir<=0 && ic<=0) */
/* 	    bl+=*v; */
/* 	  if (ir<=0 && ic>=0) */
/* 	    br+=*v; */
/* 	  if (ir>=0 && ic<=0) */
/* 	    tl+=*v; */
/* 	  if (ir>=0 && ic>=0) */
/* 	    tr+=*v; */
/* 	  if (*v>max) { */
/* 	    max=*v; */
/* 	  } */
	  
	  
/* 	  if (ir==0 && ic==0) { */
/* 	    if (*v<-nSigma*rms) */
/* 	      eventMask[iy][ix]=NEGATIVE_PEDESTAL; */
/* 	  } */
	  
/* 	} */
/*       } */
      
/*       if (bl>=br && bl>=tl && bl>=tr) { */
/* 	(clusters+nph)->quad=BOTTOM_LEFT; */
/* 	(clusters+nph)->quadTot=bl; */
/*       } else if (br>=bl && br>=tl && br>=tr) { */
/* 	(clusters+nph)->quad=BOTTOM_RIGHT; */
/* 	(clusters+nph)->quadTot=br; */
/*       } else if (tl>=br && tl>=bl && tl>=tr) { */
/* 	(clusters+nph)->quad=TOP_LEFT; */
/* 	(clusters+nph)->quadTot=tl; */
/*       } else if   (tr>=bl && tr>=tl && tr>=br) { */
/* 	(clusters+nph)->quad=TOP_RIGHT; */
/* 	(clusters+nph)->quadTot=tr; */
/*       } */
      
/*       if (max>nSigma*rms || tot>sqrt(clusterSizeY*clusterSize)*nSigma*rms || ((clusters+nph)->quadTot)>sqrt(cy*cs)*nSigma*rms) { */
/* 	if (val[iy][ix]>=max) { */
/* 	  eventMask[iy][ix]=PHOTON_MAX; */
/* 	  (clusters+nph)->tot=tot; */
/* 	  (clusters+nph)->x=ix; */
/* 	  (clusters+nph)->y=iy; */
/* 	  (clusters+nph)->iframe=det->getFrameNumber(data); */
/* 	  (clusters+nph)->ped=getPedestal(ix,iy,0); */
/* 	  for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) { */
/* 	    for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) { */
/* 	      (clusters+nph)->set_data(val[iy+ir][ix+ic],ic,ir); */
/* 	    } */
/* 	  } */



/* 	  nph++; */
/* 	  image[iy*nx+ix]++; */

/* 	  } else { */
/* 	    eventMask[iy][ix]=PHOTON; */
/* 	  } */
/*       } else if (eventMask[iy][ix]==PEDESTAL) { */
/* 	addToPedestal(data,ix,iy); */
/*       } */


/*     } */
/*   } */
/*   nphFrame=nph; */
/*   nphTot+=nph; */
/*   writeClusters(); */

/*   return  nphFrame; */
  
/* }; */

int addFrame(char *data,  int *ph=NULL, int ff=0) {	    
  
  singlePhotonDetector::processData(data,ph);
  int nph=0;

  double int_x, int_y;
  double eta_x, eta_y;
  if (interp) {
    for (nph=0; nph<nphFrame; nph++) {
      if (ff) {
	interp->addToFlatField((clusters+nph)->quadTot,(clusters+nph)->quad,(clusters+nph)->get_cluster(),eta_x, eta_y);
      } else {
	interp->getInterpolatedPosition((clusters+nph)->x, (clusters+nph)->y, (clusters+nph)->quadTot,(clusters+nph)->quad,(clusters+nph)->get_cluster(),int_x, int_y);
	interp->addToImage(int_x, int_y);
      }
    }  
  }
  return  nphFrame;
  
};
  
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
    virtual char *setInterpolation(char *ii){int ok; interp=(slsInterpolation*)ii; interp->prepareInterpolation(ok); return (char*)interp;};
  
 protected:
  
  
  slsInterpolation *interp;
  int id;
};





#endif
