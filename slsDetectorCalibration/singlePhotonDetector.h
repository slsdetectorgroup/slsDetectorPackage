#ifndef SINGLEPHOTONDETECTOR_H
#define  SINGLEPHOTONDETECTOR_H


#include <inttypes.h>
#include "slsDetectorData.h"

#include "single_photon_hit.h"
#include "pedestalSubtraction.h"
#include "commonModeSubtraction.h"


#define MYROOT1

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
    UNDEFINED=-1
  };



template <class dataType>
class singlePhotonDetector {


 public:


  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param npx number of pixels in the x direction
     \param npy number of pixels in the y direction
     \param ds size of the dataset
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)


  */
  

  singlePhotonDetector(slsDetectorData<dataType> *d, 
		       int csize=3, 
		       double nsigma=5,  
		       int sign=1, 
		       commonModeSubtraction *cm=NULL,
		       int nped=1000, 
		       int nd=100) : det(d), nx(0), ny(0), stat(NULL), cmSub(cm),  nDark(nd), eventMask(NULL),nSigma (nsigma), clusterSize(csize), clusterSizeY(csize), cluster(NULL), iframe(-1), dataSign(sign) {

   

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
    
  };
    
    virtual ~singlePhotonDetector() {delete cluster; for (int i=0; i<ny; i++) delete [] stat[i]; delete [] stat;};


    void newDataSet(){iframe=-1; for (int iy=0; iy<ny; iy++) for (int ix=0; ix<nx; ix++) stat[iy][ix].Clear(); if (cmSub) cmSub->Clear(); };  
    void newFrame(){iframe++; for (int iy=0; iy<ny; iy++) for (int ix=0; ix<nx; ix++) eventMask[iy][ix]=UNDEFINED; if (cmSub) cmSub->newFrame();};


    commonModeSubtraction setCommonModeSubtraction(commonModeSubtraction *cm) {cmSub=cm; return cmSub;};

    int setDataSign(int sign=0) {if (sign==1 || sign==-1) dataSign=sign; return dataSign;};

    virtual void addToPedestal(double val, int ix, int iy){ if (ix>=0 && ix<nx && iy>=0 && iy<ny) { stat[iy][ix].addToPedestal(val); if (cmSub && det->isGood(ix, iy) ) cmSub->addToCommonMode(val, ix, iy);}};


    virtual double getPedestal(int ix, int iy, int cm=0){if (ix>=0 && ix<nx && iy>=0 && iy<ny) if (cmSub && cm>0) return stat[iy][ix].getPedestal()-cmSub->getCommonMode(); else return stat[iy][ix].getPedestal(); else return -1;};


    double getPedestalRMS(int ix, int iy){if (ix>=0 && ix<nx && iy>=0 && iy<ny) return stat[iy][ix].getPedestalRMS();else return -1;};
  
    double setNSigma(double n=-1){if (n>0) nSigma=n; return nSigma;}
 
    int setClusterSize(int n=-1){
      if (n>0 && n!=clusterSize) {
	if (n%2==0)
	  n+=1;
	clusterSize=n; 
	delete cluster;    
	if (ny>1)
	  clusterSizeY=clusterSize;

	cluster=new single_photon_hit(clusterSize,clusterSizeY);
      }
      return clusterSize;
    }
    


    eventType getEventType(char *data, int ix, int iy, int cm=0) {

      // eventType ret=PEDESTAL;
      double tot=0, max=0;
  
      if (iframe<nDark) {
	if (cm==0)
	  addToPedestal(det->getValue(data, ix, iy),ix,iy);
	return UNDEFINED;
      }
      
      

      if (eventMask[iy][ix]==UNDEFINED) {
	
	eventMask[iy][ix]=PEDESTAL;
	
	
	cluster->x=ix;
	cluster->y=iy;
	cluster->rms=getPedestalRMS(ix,iy);
	cluster->ped=getPedestal(ix,iy, cm);
	

	for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	  for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	    if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
	      cluster->set_data(dataSign*(det->getValue(data, ix+ic, iy+ir)-getPedestal(ix+ic,iy+ir,cm)), ic, ir       );
	      tot+=cluster->get_data(ic,ir);
	      if (cluster->get_data(ic,ir)>max) {
		max=cluster->get_data(ic,ir);
	      }
	      if (ir==0 && ic==0) {
		if (cluster->get_data(ic,ir)>nSigma*cluster->rms) {
		  eventMask[iy][ix]=PHOTON;
		} else if (cluster->get_data(ic,ir)<-nSigma*cluster->rms)
		  eventMask[iy][ix]=NEGATIVE_PEDESTAL;
	      }
	    }
	  }
	}
	
	if (eventMask[iy][ix]!=PHOTON && tot>sqrt(clusterSizeY*clusterSize)*nSigma*cluster->rms) {
	  eventMask[iy][ix]=NEIGHBOUR;
	  
	} else if (eventMask[iy][ix]==PHOTON) {
	  if (cluster->get_data(0,0)>=max) {
	    for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	      for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
		
		if ((iy+ir)>=0 && (iy+ir)<ny && (ix+ic)>=0 && (ix+ic)<nx) {
		  if (eventMask[iy+ir][ix+ic]==UNDEFINED) 
		    if (ir==0 && ic==0)   eventMask[iy+ir][ix+ic]=PHOTON_MAX;
		    else eventMask[iy+ir][ix+ic]=NEIGHBOUR;
		}
		
	      }
	    }
	    

	  }
	} else if (eventMask[iy][ix]==PEDESTAL) {
	  if (cm==0)
	    addToPedestal(det->getValue(data, ix, iy),ix,iy);
	}
      }
           
      return  eventMask[iy][ix];

  };





    int SetNPedestals(int i=-1) {int ix=0, iy=0; if (i>0) for (ix=0; ix<nx; ix++) for (iy=0; iy<ny; iy++) stat[iy][ix].SetNPedestals(i); return stat[0][0].SetNPedestals();};

    double getClusterElement(int ic, int ir){return cluster->get_data(ic,ir);};
    eventType getEventMask(int ic, int ir){return eventMask[ir][ic];};
 
#ifdef MYROOT1
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
#endif


 private:
  
  slsDetectorData<dataType> *det;
  int nx, ny;


  pedestalSubtraction **stat;
  commonModeSubtraction *cmSub;
  //MovingStat **stat;
  int nDark;
  eventType **eventMask;
  double nSigma;
  int clusterSize, clusterSizeY;
  single_photon_hit *cluster;
  int iframe;
  int dataSign;

  /*   int cx, cy; */
};





#endif
