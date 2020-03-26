#ifndef MOENCH04ZMQ10GBDATA_H
#define  MOENCH04ZMQ10GBDATA_H
#include "slsDetectorData.h"




class moench04CtbZmq10GbData : public slsDetectorData<uint16_t> {
 
 private:
  
  int iframe;
  int nadc;
  int sc_width;
  int sc_height;
  const int aSamples;
  const int dSamples;


 public:




  /**
     Implements the slsReceiverData structure for the moench02 prototype read out by a module i.e. using the slsReceiver
     (160x160 pixels, 40 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

  */
  //moench04CtbZmq10GbData(int nas=5000, int nds=0): slsDetectorData<uint16_t>(400, 400, nas*2*32+nds*8), aSamples(nas), dSamples(nds), nadc(32), sc_width(25), sc_height(200) {
 moench04CtbZmq10GbData(int nas=5000, int nds=0): slsDetectorData<uint16_t>(400, 400, (nas > 0) && (nds>0) ? max(nas,nds)*(32*2+8) : nas*32*2+nds*8), nadc(32), sc_width(25), sc_height(200), aSamples(nas), dSamples(nds) {

    /* int ds; */
    /* if (nas && nds) */
    /*   if (nds>nas) */
    /* 	ds=nds*(32*2+8); */
    /*   else */
    /* 	ds=nas*(32*2+8); */
    /* else */
    /*   ds=nas*32*2+nds*8; */

    /* new slsDetectorData<uint16_t>(400, 400, ds); */

    int adc_nr[32]={9, 8,11,10,13,12,15,14,1,0,3,2,5,4,7,6,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24 };

    /* 
       iadc=ptr%32
       isample=ptr/32

       col=(adc_nr[iadc]%16)*25+isample%25
       if (adc_nr[iadc]<16)
       row=199-isample/25;
       else
       row=200+isample/25


       adc0 col(9*25..10*25-1) row(199..0)
       adc1 col(8*25..9*25-1)  row(199..0)
       adc2 col(11*25..12*25-1) row(199..0)
       adc3 col(10*25..11*25-1) row(199..0)
       adc4 col(13*25..14*25-1) row(199..0)
       adc5 col(12*25..13*25-1) row(199..0)
       adc6 col(15*25..16*25-1) row(199..0)
       adc7 col(14*25..15*25-1) row(199..0)
       adc8 col(1*25..2*25-1) row(199..0)
     */

    int row, col;

    //int isample;
    int iadc;
   // int ix, iy;
    
    // int npackets=40;
    int i;
    //int adc4(0);

      for (int is=0; is<aSamples; is++) {
	
	for (iadc=0; iadc<nadc; iadc++) {
	  i=is;
	  //	  adc4=(int)iadc/4;
	  if (i<sc_width*sc_height) {
	    //  for (int i=0; i<sc_width*sc_height; i++) {
	    col=(adc_nr[iadc]%16)*sc_width+(i%sc_width);
	    // if (adc4%2==0) {
	    if (iadc<16) {
	      row=199-i/sc_width;
	    } else {
	      row=200+i/sc_width;
	    }
	    if (nds>0)
	      dataMap[row][col]=((nadc+4)*i+iadc)*2;//+16*(ip+1);
	    else
	      dataMap[row][col]=(nadc*i+iadc)*2;//+16*(ip+1);
	    if (dataMap[row][col]<0 || dataMap[row][col]>=aSamples*2*32)
	      cout << "Error: pointer " << dataMap[row][col] << " out of range "<< endl;
	  }
	}
      }
  

    /* for (ibyte=0; ibyte<sizeof(sls_detector_header)/2; ibyte++){ */
    /*   xmap[ibyte]=-1; */
    /*   ymap[ibyte]=-1; */
    /* } */
    /* int off=sizeof(sls_detector_header)/2; */
    
    /*   for (ibyte=0;  ibyte<dataSize; ibyte++) { */


    /* for (ipacket=0; ipacket<npackets; ipacket++) { */
    /*   for (ibyte=0;  ibyte< 8192/2; ibyte++) { */
    /* 	i=ipacket*8208/2+ibyte; */
    /* 	  isample=ii/nadc; */
    /* 	  if (isample<nSamples) { */
    /* 	  iadc=ii%nadc; */
    /* 	  adc4 = (int)iadc/4; */
    /* 	  ix=isample%sc_width; */
    /* 	  iy=isample/sc_width; */
    /* 	  if (adc4%2==0) { */
    /* 	    xmap[i+off]=adc_nr[iadc]+ix; */
    /* 	    ymap[i+off]=ny/2-1-iy; */
    /* 	  } else { */
    /* 	    xmap[i+off]=adc_nr[iadc]+ix; */
    /* 	    ymap[i+off]=ny/2+iy; */
    /* 	  } */
    /* 	  } */
    /* 	ii++; */
    /* 	//	} */
    /*   } */
    /* } */
    
    iframe=0;
    //  cout << "data struct created" << endl;
  }
    
  int getGain(char *data, int x, int y) {
   // int aoff=aSamples*2*32;
    int irow;
    int isc=x/sc_width;
    int icol=x%sc_width;
    if (y<200) irow=sc_height-1-y;
    else {
      irow=y-sc_height;
      isc++;
    }
    int ibit[32]={-1,-1,-1,-1,-1,-1,1,3,5,7,-1,-1,-1,-1,-1,-1,62,60,58,56,54,52,50,48,63,61,59,57,55,53,51,49};
    int isample=irow*sc_width+icol;
   
    uint64_t sample;
    char *ptr;
    if (isc<0 || isc>=32) return 0;
    if (ibit[isc]<0 || ibit[isc]>=64) return 0;
    if (dSamples>isample) {
      ptr=data+32*(isample+1)+8*isample;
      sample=*((uint64_t*)ptr);
    cout << isc << " " << ibit[isc] << " " << isample << hex << sample << dec << endl;
    if (sample & (1<<ibit[isc]))
      return 1;
    else
      return 0;
    } else
      return 0;
  }

     /**

     Returns the frame number for the given dataset. Purely virtual func.
     \param buff pointer to the dataset
     \returns frame number

  */

/* class jfrau_packet_header_t { */
/*  public: */
/* 	unsigned char reserved[4]; */
/* 	unsigned char packetNumber[1]; */
/* 	unsigned char frameNumber[3]; */
/* 	unsigned char bunchid[8]; */
/* }; */



  int getFrameNumber(char *buff){return iframe;};//((sls_detector_header*)buff)->frameNumber;};//*((int*)(buff+5))&0xffffff;};   

  /**

     Returns the packet number for the given dataset. purely virtual func
     \param buff pointer to the dataset
     \returns packet number number



  */
  //int getPacketNumber(char *buff){return ((sls_detector_header*)buff)->packetNumber;}//((*(((int*)(buff+4))))&0xff)+1;};   

/*    /\** */

/*      Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func */
/*      \param data pointer to the memory to be analyzed */
/*      \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot */
/*      \param dsize size of the memory slot to be analyzed */
/*      \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found  */

/*   *\/ */
/*     virtual  char *findNextFrame(char *data, int &ndata, int dsize){ndata=dsize; setDataSize(dsize);  return data;}; */


/*    /\** */

/*      Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors!  */
/*      \param filebin input file stream (binary) */
/*      \returns pointer to the begin of the last good frame, NULL if no frame is found or last frame is incomplete */

/*   *\/ */
/*     virtual char *readNextFrame(ifstream &filebin){ */
/*       //	int afifo_length=0;   */
/*       uint16_t *afifo_cont;  */
/*       int ib=0; */
/*       if (filebin.is_open()) { */
/* 	afifo_cont=new uint16_t[dataSize/2]; */
/*  	while (filebin.read(((char*)afifo_cont)+ib,2)) { */
/* 	  ib+=2; */
/* 	  if (ib==dataSize) break; */
/* 	} */
/* 	if (ib>0) { */
/* 	  iframe++; */
/* 	  // cout << ib << "-" << endl; */
/* 	  return (char*)afifo_cont; */
/* 	} else { */
/* 	  delete [] afifo_cont; */
/* 	  return NULL; */
/* 	} */
/*       }      */
/*       return NULL; */
/*     }; */


  virtual char *readNextFrame(ifstream &filebin) {
    int ff=-1, np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int &ff) {
    int np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np) {
	  char *data=new char[dataSize];
	  char *d=readNextFrame(filebin, ff, np, data);
	  if (d==NULL) {delete [] data; data=NULL;}
	  return data;
  }




  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np, char *data) {
	 // char *retval=0;
	 // int  nd;
	 // int fnum = -1;
	  np=0;
	  //int  pn;
	  
	  //  cout << dataSize << endl;
	  if (ff>=0)
	   // fnum=ff;

	  if (filebin.is_open()) {
	    if (filebin.read(data, dataSize) ){
	      ff=getFrameNumber(data);
	      //     np=getPacketNumber(data);
	      return data;
	    }
	  }
	    return NULL;
	    
	    
	  
  };



  /**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func
     \param data pointer to the memory to be analyzed
     \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot
     \param dsize size of the memory slot to be analyzed
     \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found 
     
  */
  virtual  char *findNextFrame(char *data, int &ndata, int dsize){
    if (dsize<dataSize) ndata=dsize;
    else ndata=dataSize;
    return data;

  }



  //int getPacketNumber(int x, int y) {return dataMap[y][x]/packetSize;};

};




#endif
