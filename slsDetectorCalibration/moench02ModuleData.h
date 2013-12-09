#ifndef MOENCH02MODULEDATA_H
#define  MOENCH02MODULEDATA_H
//#include "slsDetectorData.h"
#include "singlePhotonDetector.h"

#include "RunningStat.h"
#include "MovingStat.h"

#include <iostream>
#include <fstream>

#include <TTree.h>
using namespace std;



class moench02ModuleData : public slsDetectorData<uint16_t> {
 public:




  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param npx number of pixels in the x direction
     \param npy number of pixels in the y direction
     \param ds size of the dataset
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)


  */
  

  moench02ModuleData(int nbg=1000): slsDetectorData<uint16_t>(160, 160, 40, 1286), nSigma(5), buff(NULL), oldbuff(NULL), pedSub(1) {
    int headerOff=0;
    int footerOff=1284;
    int dataOff=4;

    int ix, iy, ir, ic, i, isc, ip;

    int **dMask;
    int **dMap;

    for (ir=0; ir<160; ir++) {
      for (ic=0; ic<160; ic++) {
	stat[ir][ic].Clear();
	stat[ir][ic].SetN(nbg);
      }
    }


    dMask=new int*[160];
    for(i = 0; i < 160; i++) {
      dMask[i] = new int[160];
    }
    dMap=new int*[160];
    for(i = 0; i < 160; i++) {
      dMap[i] = new int[160];
    }

    for (isc=0; isc<4; isc++) {
      for (ip=0; ip<10; ip++) {

	for (ir=0; ir<16; ir++) {
	  for (ic=0; ic<40; ic++) {

	    ix=isc*40+ic;
	    iy=ip*16+ir;

	    dMap[iy][ix]=1286*(isc*10+ip)+2*ir*40+2*ic;
	    // cout << ix << " " << iy << " " << dMap[ix][iy] << endl;
	  }
	}
      }
    }

    for (ix=0; ix<120; ix++) {
      for (iy=0; iy<160; iy++)
	dMask[iy][ix]=0x3fff;
    }
    for (ix=120; ix<160; ix++) {
      for (iy=0; iy<160; iy++)
	dMask[iy][ix]=0x0;
    }


    setDataMap(dMap);
    setDataMask(dMask);
  };
    
    ~moench02ModuleData() {if (buff) delete [] buff; if (oldbuff) delete [] oldbuff; };
  

  int getPacketNumber(char *buff){
    int np=(*(int*)buff)&0xff;
    if (np==0)
      np=40;
    return np;
  };


  void calculateCommonMode(int xmin=0, int xmax=160, int ymin=0, int ymax=160, double hc=0, double tc=0) {

    int scmin=xmin/40;
    int scmax=(xmax-1)/40+1;
    int isc=0, ix, iy;

    for (isc=scmin; isc<scmax; isc++) {
      nCm[isc]=0;
      cmPed[isc]=0;
    }
    
    


    for (ix=xmin-1; ix<xmax+1; ix++) {
      isc=ix/40;
      for (iy=ymin-1; iy<ymax+1; iy++) {
	if (isGood(ix,iy)) {
	  if (getEventType(ix, iy, hc, tc, 1,0)==PEDESTAL) {
	    cmPed[isc]+=getChannelShort(ix, iy, hc, tc);
	    nCm[isc]++;
	  }
	}
	
      }

    }

    
    for (isc=scmin; isc<scmax; isc++) {

      if (nCm[isc]>0)
	cmStat[isc].Calc(cmPed[isc]/nCm[isc]);

      //   cout << "SC " << isc << nCm[isc] << " " << cmPed[isc]/nCm[isc] << " " << cmStat[isc].Mean() << endl;

    }
    

  };

  double getCommonMode(int ix, int iy) {
    int isc=ix/40;
    if (nCm[isc]>0) {
     /*  if (ix==20 && iy==20) */
/* 	cout << cmPed[isc] << " " << nCm[isc] << " " << cmStat[isc].Mean() << " " << cmPed[isc]/nCm[isc]-cmStat[isc].Mean() << endl; */
      return cmPed[isc]/nCm[isc]-cmStat[isc].Mean();
    } else {
  /*     if (ix==20 && iy==20) */
/* 	cout << "No common mode!" << endl; */
      
      return 0;
    }
  };




  void addToPedestal(double val, int ix, int iy){stat[iy][ix].Calc(val);};
  double getPedestal(int ix, int iy){return stat[iy][ix].Mean();};
  double getPedestalRMS(int ix, int iy){return stat[iy][ix].StandardDeviation();};
  double setNSigma(double n){if (n>0) nSigma=n; return nSigma;}
  char *getBuff(){return buff;}
  char *getOldBuff(){return oldbuff;}

  int setPedestalSubstraction(int i=-1){if (i>=0) pedSub=i; return pedSub;};


  double getChannelShort(int ix, int iy, double hc=0, double tc=0) {
    
    double val=slsDetectorData<uint16_t>::getChannel(buff,ix,iy);

    if (ix>0 && hc!=0)
      val-=hc*slsDetectorData<uint16_t>::getChannel(buff,ix-1,iy);
    
    if (oldbuff && tc!=0) 
      val+=tc*slsDetectorData<uint16_t>::getChannel(oldbuff,ix,iy);


    return val;
  };
  


  eventType getEventType(int ix, int iy, double hcorr=0, double tcorr=0, int sign=1, int cm=0) {

 /*    PEDESTAL, */
/*     NEIGHBOUR, */
/*     PHOTON, */
/*     PHOTON_MAX, */
 /*    NEGATIVE_PEDESTAL */
      eventType ret=PEDESTAL;
      double tot=0, v, max=0, sig;
      

      sig=getPedestalRMS(ix,iy);

      for (int ir=-1; ir<2; ir++) {
	for (int ic=-1; ic<2; ic ++) {
	  if ((iy+ir)>=0 && (iy+ir)<160 && (ix+ic)>=0 && (ix+ic)<160) {
	    v=sign*getChannelShort(ix+ic, iy+ir, hcorr, tcorr);
	    if (pedSub)
	      v-=sign*getPedestal(ix+ic,iy+ir);
	    if (cm)
	      v-=sign*getCommonMode(ix+ic, iy+ic);
	    cluster[ic+1][ir+1]=v;
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
	if (cluster[1][1]>=max)
	  ret=PHOTON_MAX;
      }
      
   
      cx=ix;
      cy=iy;

      return ret;

  };


  double getClusterElement(int ic, int ir, int cmsub=0){return cluster[ic+1][ir+1];};
  
  double  *getCluster(){return &cluster[0][0];};


  TTree *initMoenchTree(char *tname, Int_t *iframe, Int_t * x, Int_t* y, Double_t data[3][3], Double_t *ped, Double_t *sigma) {
    TTree* tall=new TTree(tname,tname);
    tall->Branch("iFrame",iframe,"iframe/I");
    tall->Branch("x",x,"x/I");
    tall->Branch("y",y,"y/I");
    tall->Branch("data",data,"data[3][3]/D");
    tall->Branch("pedestal",ped,"pedestal/D");
    tall->Branch("rms",sigma,"rms/D");
    return tall;

  };

  

 private:
  
  MovingStat stat[160][160];
  double nSigma;
  double cluster[3][3];
  char *buff;
  char *oldbuff;
  int pedSub;
  int cmSub;

  MovingStat cmStat[4]; //stores the common mode average per supercolumn
  double cmPed[4]; // stores the common mode for this frame per supercolumn
  double nCm[4]; // stores the number of pedestals to calculate the cm per supercolumn
  int cx, cy;
};



#endif
