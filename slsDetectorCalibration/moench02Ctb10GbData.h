#ifndef MOENCH02CTB10GBDATA_H
#define  MOENCH02CTB10GBDATA_H
#include <stdio.h>
#include "slsDetectorData.h"
#include "slsReceiverData.h"


class moench02Ctb10GbData : public slsReceiverData<uint16_t> {

 private:
  
  int iframe;
  // int *xmap, *ymap;
  int nadc;
  int sc_width;
  int sc_height;

  int maplength;



 public:




  /**
     Implements the slsReceiverData structure for the moench02 prototype read out by a module i.e. using the slsReceiver
     (160x160 pixels, 40 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

  */
  

 moench02Ctb10GbData(int ns=6400): slsReceiverData<uint16_t>(160, 160, 50, 8208) , nadc(4), sc_width(40), sc_height(160) {

    
    int adc_nr[4]={120,0,80,40};
    int row, col;

    int isample;
    int iadc;
    int ix, iy;
    int i;
    int npackets=50;
    maplength = 8208*npackets;
    //this->setDataSize(maplength);
    /* maplength=this->getDataSize()/2; */
    
    for (int ip=0; ip<npackets; ip++) {
      for (int is=0; is<128; is++) {

	for (iadc=0; iadc<nadc; iadc++) {
	  i=128*ip+is;
	  if (i<sc_width*sc_height) {
	    col=adc_nr[iadc]+(i%sc_width);
	    row=i/sc_width;
	    dataMap[row][col]=(32*i+iadc+2)*2+16*(ip+1);
	    if (dataMap[row][col]<0 || dataMap[row][col]>=8208*npackets) {
	      cout << "Error: pointer " << dataMap[row][col] << " out of range "<< endl;	  
	    }
	  
	  }
	}
      }
    }
    int ibyte;
    int ii=0;

    for (int ipacket=0; ipacket<npackets; ipacket++) {
      for (int ibyte=0;  ibyte< 8208/2; ibyte++) {
	i=ipacket*8208/2+ibyte;
	if(ibyte<8){
	  xmap[i]=-1;
	  ymap[i]=-1;
	}else{
	  isample=ii/32;
	  iadc=ii%32;
	  ix=isample%sc_width;
	  iy=isample/sc_width;
	  if(iadc>=2 && iadc<=5){
	    xmap[i]=adc_nr[iadc-2]+ix;
	    ymap[i]=iy;
	  }else{
	    xmap[i]=-1;
	    ymap[i]=-1;
	  }
	  ii++;
	}
      }//end loop on bytes
    }//end loop on packets

    iframe=0;
    cout << "data struct created" << endl;
  };
    
  void getPixel(int ip, int &x, int &y) {
    if(ip>=0 && ip<maplength){
      x=xmap[ip]; 
      y=ymap[ip];
    }else{
      cerr<<"WRONG ARRAY LENGTH"<<endl;
      cerr<<"Trying to access the "<<ip<<"-th element"<<endl;
    }

  };
  

  /**

     Returns the frame number for the given dataset. Purely virtual func.
     \param buff pointer to the dataset
     \returns frame number

  */

  int getFrameNumber(char *buff){return *((int*)buff)&0xffffffff;};  
  /* virtual  int getFrameNumber(char *buff){(void)buff; return iframe;};    */

  /**

     Returns the packet number for the given dataset. purely virtual func
     \param buff pointer to the dataset
     \returns packet number number

 
     virtual int getPacketNumber(char *buff)=0;

  */
  int getPacketNumber(char *buff){return ((*(((int*)(buff+4))))&0xff)+1;};  
  /**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func
     \param data pointer to the memory to be analyzed
     \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot
     \param dsize size of the memory slot to be analyzed
     \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found 

  */
  //virtual  char *findNextFrame(char *data, int &ndata, int dsize){ndata=dsize; setDataSize(dsize);  return data;};


  /**

     Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors! 
     \param filebin input file stream (binary)
     \returns pointer to the begin of the last good frame, NULL if no frame is found or last frame is incomplete

  */
  /* virtual char *readNextFrame(ifstream &filebin){ */
  /*   //	int afifo_length=0;   */
  /*   uint16_t *afifo_cont;  */
  /*   int ib=0; */
  /*   if (filebin.is_open()) { */
  /* 	afifo_cont=new uint16_t[dataSize/2]; */
  /* 	while (filebin.read(((char*)afifo_cont)+ib,2)) { */
  /* 	  ib+=2; */
  /* 	  if (ib==dataSize) break; */
  /* 	} */
  /* 	if (ib>0) { */
  /* 	  iframe++; */
  /* 	  //cout << ib << "-" << endl; */
  /* 	  return (char*)afifo_cont; */
  /* 	} else { */
  /* 	  delete [] afifo_cont; */
  /* 	  return NULL; */
  /* 	} */
  /*   }      */
  /*   return NULL; */
  /* }; */
  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np) {
    char *data=new char[packetSize*nPackets];
    char *retval=0;
    int nd;
    np=0;
    int  pn;
    char aa[8224]={0};
    char *packet=(char *)aa;
    int  fnum = -1;
    if (ff>=0)
      fnum=ff;

    if (filebin.is_open()) {


      cout << "+";

      while(filebin.read((char*)packet, 8208)){
	      
	pn=getPacketNumber(packet);
	if (fnum<0)
	  fnum= getFrameNumber(packet);
	

	if (fnum>=0) {
	  if (getFrameNumber(packet) !=fnum) { 
	    
	    if (np==0){
	      cout << "-";
	      delete [] data;
	      return NULL;
	    } else
	      filebin.seekg(-8208,ios_base::cur);
	    return data;
	  }
	
	  memcpy(data+(pn-1)*packetSize, packet, packetSize);

	  np++;
	  if (np==nPackets)
	    break;
	  if (pn==nPackets)
	    break;
	}
      }	    
    }else{
      cerr<<filebin<<" not open!!!"<<endl;
      return NULL;
    }
	  
    if (np==0){
      cout << "?";
      delete [] data;
      return NULL;
    }// else if (np<nPackets)
    //  cout << "Frame " << fnum << " lost "  << nPackets-np << " packets " << endl;

    
    return data;
	  
  };



  virtual char *readNextFrame(ifstream &filebin) {
    int fnum=-1, np;
    return readNextFrame(filebin, fnum, np);
  };

  

};



#endif
