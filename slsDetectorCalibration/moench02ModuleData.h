#ifndef MOENCH02MODULEDATA_H
#define  MOENCH02MODULEDATA_H
#include "slsDetectorData.h"

#include "RunningStat.h"
#include "MovingStat.h"

#include <iostream>
#include <fstream>

using namespace std;



class moench02ModuleData : public slsDetectorData {
 public:


  enum eventType {
    PEDESTAL,
    NEIGHBOUR,
    PHOTON,
    PHOTON_MAX,
    NEGATIVE_PEDESTAL,
  };




  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param npx number of pixels in the x direction
     \param npy number of pixels in the y direction
     \param ds size of the dataset
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)


  */
  

  moench02ModuleData(int nbg=1000): slsDetectorData(160, 160, 1286*40), nSigma(5), buff(NULL), oldbuff(NULL), pedSub(1) {
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

	    dMap[iy][ix]=1280*(isc*10+ip)+2*ir*40+2*ic;
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
  
    int getFrameNumber(char *buff){
    return ((*(int*)buff)&(0xffffff00))>>8;;
  };

  int getPacketNumber(char *buff){
    return (*(int*)buff)&0xff;
  };


  char *readNextFrame(ifstream &filebin) {
    
    if (oldbuff)
      delete [] oldbuff;

    oldbuff=buff;
    
    char *data=new char[1280*40];
    char p[1286];
    
    int fn, fnum=-1,  np=0, pnum=-1;

    if (filebin.is_open()) {
      while (filebin.read(p,4)) { //read header
	pnum=getPacketNumber(p);
	fn=getFrameNumber(p);

	if (pnum<0 || pnum>39) {
	  cout << "Bad packet number " << pnum << " frame "<< fn << endl;
	  continue;
	}

	if (pnum==0)
	  pnum=40;
	
	if (pnum==1) {
	  fnum=fn;
	  if (np>0)
	    cout << "*Incomplete frame number " << fnum << endl;
	  np=0;
	} else if (fn!=fnum) {
	  if (fnum!=-1)
	    cout << " **Incomplete frame number " << fnum << " pnum " << pnum << " " << getFrameNumber(p) << endl;
	  np=0;
	}
	filebin.read(data+1280*(pnum-1),1280); //readdata
	np++;
	filebin.read(p,2); //readfooter

	if (np==40)
	  if (pnum==40)
	    break;
	  else
	    cout << "Too many packets for this frame! "<< fnum << " " << pnum << endl;

      }
    }
    if (np<40) {
      if (np>0)
	cout << "Too few packets for this frame! "<< fnum << " " << pnum << endl;
      delete [] data;
      buff=NULL;
    }
    buff=data;
    return data;
  };


  void addToPedestal(double val, int ix, int iy){stat[iy][ix].Calc(val);};
  double getPedestal(int ix, int iy){return stat[iy][ix].Mean();};
  double getPedestalRMS(int ix, int iy){return stat[iy][ix].StandardDeviation();};
  double setNSigma(double n){if (n>0) nSigma=n; return nSigma;}
  char *getBuff(){return buff;}
  char *getOldBuff(){return oldbuff;}

  int setPedestalSubstraction(int i=-1){if (i==0) pedSub=0; if (i==1) pedSub=1; return pedSub;};


  double getChannelShort(int ix, int iy, double hc=0, double tc=0) {
    
    double val=slsDetectorData::getChannelShort(buff,ix,iy);

    if (ix>0 && hc!=0)
      val-=hc*slsDetectorData::getChannelShort(buff,ix-1,iy);
    
    if (oldbuff && tc!=0)
      val+=tc*slsDetectorData::getChannelShort(oldbuff,ix,iy);


    return val;
  };
  


  eventType getEventType(int ix, int iy, double hcorr=0, double tcorr=0, int sign=1) {

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
	    v=getChannelShort(ix+ic, iy+ir, hcorr, tcorr);
	    if (pedSub)
	      v=sign*(v-getPedestal(ix+ic,iy+ir));
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
      
   


      return ret;

  };


  double getClusterElement(int ic, int ir){return cluster[ic+1][ir+1];};
  double  *getCluster(){return &cluster[0][0];};




 private:
  
  MovingStat stat[160][160];
  double nSigma;
  double cluster[3][3];
  char *buff;
  char *oldbuff;
  int pedSub;


};



#endif
