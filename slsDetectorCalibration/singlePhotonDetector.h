#ifndef SINGLEPHOTONDETECTOR_H
#define  SINGLEPHOTONDETECTOR_H
#include "slsDetectorData.h"

#include "MovingStat.h"
#include "single_photon_hit.h"

#include <iostream>

using namespace std;


  enum eventType {
    PEDESTAL,
    NEIGHBOUR,
    PHOTON,
    PHOTON_MAX,
    NEGATIVE_PEDESTAL,
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
  

  singlePhotonDetector(slsDetectorData<dataType> *d, int csize=3, double nsigma=5) : det(d), clusterSize(csize), clusterSizeY(csize),nSigma(nsigma) {

   

    det->getDetectorSize(nx,ny);
    


    stat=new MovingStat*[ny];
    for (int i=0; i<ny; i++)
      stat[i]=new MovingStat;
   
    if (ny==1)
      clusterSizeY=1;

    cluster=new single_photon_hit(clusterSize,clusterSizeY);
    
  };
    
    ~singlePhotonDetector() {delete cluster; delete [] stat;};
  


    void addToPedestal(double val, int ix, int iy){ if (ix>=0 && ix<nx && iy>=0 && iy<ny) stat[iy][ix].Calc(val);};
    double getPedestal(int ix, int iy){if (ix>=0 && ix<nx && iy>=0 && iy<ny) return stat[iy][ix].Mean(); else return -1;};
    double getPedestalRMS(int ix, int iy){if (ix>=0 && ix<nx && iy>=0 && iy<ny) return stat[iy][ix].StandardDeviation();else return -1;};
  
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
    


    eventType getEventType(char *data, int ix, int iy, double hcorr=0, double tcorr=0, int sign=1, int cm=0) {

      eventType ret=PEDESTAL;
      double tot=0, v, max=0, sig;
      

      sig=getPedestalRMS(ix,iy);

      cluster->x=ix;
      cluster->y=iy;
      cluster->rms=sig;
      cluster->ped=getPedestal(ix,iy);
      

      for (int ir=-(clusterSizeY/2); ir<(clusterSizeY/2)+1; ir++) {
	for (int ic=-(clusterSize/2); ic<(clusterSize/2)+1; ic++) {
	  if ((iy+ir)>=0 && (iy+ir)<160 && (ix+ic)>=0 && (ix+ic)<160) {
	    v=sign*(det->getValue(data, ix+ic, iy+ir)-getPedestal(ix+ic,iy+ir));
	    // if (cm)
	    //  v-=sign*getCommonMode(ix+ic, iy+ic);
	    cluster->set_data(ic, ir, v);
	    tot+=v;
	    if (v>max) {
	      max=v;
	    }
	    if (ir==0 && ic==0) {
	      if (v>nSigma*getPedestalRMS(ix,iy)) {
		ret=PHOTON;
	      } else if (v<-nSigma*getPedestalRMS(ix,iy))
		ret=NEGATIVE_PEDESTAL;
	    }
	  }
	}
      }
      
      if (ret!=PHOTON && tot>3*nSigma*sig)
	ret=NEIGHBOUR;
      else if (ret==PHOTON) {
	if (cluster->get_data(0,0)>=max)
	  ret=PHOTON_MAX;
      }
      
      return ret;

  };







  double getClusterElement(int ic, int ir, int cmsub=0){return cluster->get_data(ic,ir);};
  

 private:
  
  slsDetectorData<dataType> *det;
  int nx, ny;



  MovingStat **stat;
  double nSigma;
  int clusterSize, clusterSizeY;
  single_photon_hit *cluster;

  MovingStat cmStat[4]; //stores the common mode average per supercolumn
  double cmPed[4]; // stores the common mode for this frame per supercolumn
  double nCm[4]; // stores the number of pedestals to calculate the cm per supercolumn
  int cx, cy;
};







    /////////////////////////////////////////////////////// DONE UNTIL HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 /*  void calculateCommonMode(int xmin=0, int xmax=160, int ymin=0, int ymax=160, double hc=0, double tc=0) { */

/*     int scmin=xmin/40; */
/*     int scmax=(xmax-1)/40+1; */
/*     int isc=0, ix, iy; */

/*     for (isc=scmin; isc<scmax; isc++) { */
/*       nCm[isc]=0; */
/*       cmPed[isc]=0; */
/*     } */
    
    


/*     for (ix=xmin-1; ix<xmax+1; ix++) { */
/*       isc=ix/40; */
/*       for (iy=ymin-1; iy<ymax+1; iy++) { */
/* 	if (isGood(ix,iy)) { */
/* 	  if (getEventType(ix, iy, hc, tc, 1,0)==PEDESTAL) { */
/* 	    cmPed[isc]+=getChannelShort(ix, iy, hc, tc); */
/* 	    nCm[isc]++; */
/* 	  } */
/* 	} */
	
/*       } */

/*     } */

    
/*     for (isc=scmin; isc<scmax; isc++) { */

/*       if (nCm[isc]>0) */
/* 	cmStat[isc].Calc(cmPed[isc]/nCm[isc]); */

/*       //   cout << "SC " << isc << nCm[isc] << " " << cmPed[isc]/nCm[isc] << " " << cmStat[isc].Mean() << endl; */

/*     } */
    

/*   }; */

/*   double getCommonMode(int ix, int iy) { */
/*     int isc=ix/40; */
/*     if (nCm[isc]>0) { */
/*      /\*  if (ix==20 && iy==20) *\/ */
/* /\* 	cout << cmPed[isc] << " " << nCm[isc] << " " << cmStat[isc].Mean() << " " << cmPed[isc]/nCm[isc]-cmStat[isc].Mean() << endl; *\/ */
/*       return cmPed[isc]/nCm[isc]-cmStat[isc].Mean(); */
/*     } else { */
/*   /\*     if (ix==20 && iy==20) *\/ */
/* /\* 	cout << "No common mode!" << endl; *\/ */
      
/*       return 0; */
/*     } */
/*   }; */



#endif
