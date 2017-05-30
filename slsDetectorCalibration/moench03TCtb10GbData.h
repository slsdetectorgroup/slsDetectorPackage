#ifndef MOENCH03TCTB10GBDATA_H
#define  MOENCH03TCTB10GBDATA_H
#include "moench03Ctb10GbData.h"



class moench03TCtb10GbData : public moench03Ctb10GbData { //slsReceiverData<uint16_t> {
 

 public:




  /**
     Implements the slsReceiverData structure for the moench02 prototype read out by a module i.e. using the slsReceiver
     (160x160 pixels, 40 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

  */
  

  //  moench03TCtb10GbData(int ns=5000): slsDetectorData<uint16_t>(400, 400, 8208*40, NULL, NULL) , nadc(32), sc_width(25), sc_height(200) {
 moench03TCtb10GbData(int ns=5000): moench03Ctb10GbData(ns) {//slsReceiverData<uint16_t>(400, 400, 40, 8208), nadc(32), sc_width(25), sc_height(200) {
    // cout <<"constructor"<< endl;
      // nadc=32;
    int adc_nr[32]={300,325,350,375,300,325,350,375,		\
		    200,225,250,275,200,225,250,275,\
		    100,125,150,175,100,125,150,175,\
		    0,25,50,75,0,25,50,75};
    int row, col;

    int isample;
    int iadc;
    int ix, iy;
    
    int npackets=40;
    int i;
    int adc4(0);

    for (int ip=0; ip<npackets; ip++) {
      for (int is=0; is<128; is++) {

	for (iadc=0; iadc<nadc; iadc++) {
	  i=128*ip+is;
	  //cout << i << endl;
	   adc4=(int)iadc/4;
	  if (i<sc_width*sc_height) {
	    //  for (int i=0; i<sc_width*sc_height; i++) {
	    col=adc_nr[iadc]+(i%sc_width);
	    if (adc4%2==0) {
	      // if (iadc<(nadc/2)) {
	      row=199-i/sc_width; 	            
	    } else {
	      row=200+i/sc_width;
	    }
	    dataMap[row][col]=(nadc*i+iadc)*2+16*(ip+1);
	    if (dataMap[row][col]<0 || dataMap[row][col]>=8208*40)
	      cout << "Error: pointer " << dataMap[row][col] << " out of range "<< endl;
	  }
	}
      }
    }
    //  cout <<"map"<< endl;
    int ipacket;
    int ibyte;
    int ii=0;
    for (int ipacket=0; ipacket<npackets; ipacket++) {
      for (int ibyte=0;  ibyte< 8208/2; ibyte++) {
    	i=ipacket*8208/2+ibyte;
    	//	cout << i << " ";
    	if (ibyte<8) {
    	//header!
    	  xmap[i]=-1;
    	  ymap[i]=-1;
    	} else {
    	  // ii=ibyte+128*32*ipacket;
    	  // cout << nadc << endl;
    	  isample=ii/nadc;
    	  iadc=ii%nadc;
	
    	  adc4 = (int)iadc/4;

    	  ix=isample%sc_width;
    	  iy=isample/sc_width;
    	  //  cout << isample << " " << iadc << " " << ix << " " << iy ;//<< endl;
    	  if (adc4%2==0) {//iadc<(nadc/2)) {
    	  //if (iadc<(nadc/2)) {
    	    xmap[i]=adc_nr[iadc]+ix;
    	    ymap[i]=ny/2-1-iy;
    	  } else {
    	    xmap[i]=adc_nr[iadc]+ix;
    	    ymap[i]=ny/2+iy;
    	  }
	  
    	ii++;
    	}
      }
    }
    

    
    /* int ipacket; */
    /* int ibyte; */
    /* int ii=0; */
    
    /* for (int ipacket=0; ipacket<npackets; ipacket++) { */
    /*   for (int ibyte=0;  ibyte< 8208/2; ibyte++) { */
    /* 	i=ipacket*8208/2+ibyte; */
    /* 	cout << i << endl; */
    /* 	if (ibyte<8) { */
    /* 	//header! */
    /* 	  xmap[i]=-1; */
    /* 	  ymap[i]=-1; */
    /* 	} else { */
    /* 	  ii=ibyte+128*32*ipacket; */
    /* 	  isample=ii/nadc; */
    /* 	  iadc=ii%nadc; */
    /* 	  adc4 = (int)iadc/4; */

    /* 	  ix=isample%sc_width; */
    /* 	  iy=isample/sc_width; */
    /* 	  if (adc4%2==0) { */
    /* 	    xmap[i]=adc_nr[iadc]+ix; */
    /* 	    ymap[i]=ny/2-1-iy; */



    /* 	  } else { */
    /* 	    xmap[i]=adc_nr[iadc]+ix; */
    /* 	    ymap[i]=ny/2+iy; */

    /* 	  } */
	  
    /* 	ii++; */
    /* 	} */
    /*   } */
    /* } */
    

    
    //  cout <<"done"<< endl;
    
    iframe=0;
    //  cout << "data struct created" << endl;
  };
    

/*      /\** */

/*      Returns the frame number for the given dataset. Purely virtual func. */
/*      \param buff pointer to the dataset */
/*      \returns frame number */

/*   *\/ */


/*   int getFrameNumber(char *buff){return *((int*)buff)&0xffffffff;};    */

/*   /\** */

/*      Returns the packet number for the given dataset. purely virtual func */
/*      \param buff pointer to the dataset */
/*      \returns packet number number */



/*   *\/ */
/*   int getPacketNumber(char *buff){return ((*(((int*)(buff+4))))&0xff)+1;};    */

/* /\*    /\\** *\/ */

/* /\*      Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func *\/ */
/* /\*      \param data pointer to the memory to be analyzed *\/ */
/* /\*      \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot *\/ */
/* /\*      \param dsize size of the memory slot to be analyzed *\/ */
/* /\*      \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found  *\/ */

/* /\*   *\\/ *\/ */
/* /\*     virtual  char *findNextFrame(char *data, int &ndata, int dsize){ndata=dsize; setDataSize(dsize);  return data;}; *\/ */


/* /\*    /\\** *\/ */

/* /\*      Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors!  *\/ */
/* /\*      \param filebin input file stream (binary) *\/ */
/* /\*      \returns pointer to the begin of the last good frame, NULL if no frame is found or last frame is incomplete *\/ */

/* /\*   *\\/ *\/ */
/* /\*     virtual char *readNextFrame(ifstream &filebin){ *\/ */
/* /\*       //	int afifo_length=0;   *\/ */
/* /\*       uint16_t *afifo_cont;  *\/ */
/* /\*       int ib=0; *\/ */
/* /\*       if (filebin.is_open()) { *\/ */
/* /\* 	afifo_cont=new uint16_t[dataSize/2]; *\/ */
/* /\*  	while (filebin.read(((char*)afifo_cont)+ib,2)) { *\/ */
/* /\* 	  ib+=2; *\/ */
/* /\* 	  if (ib==dataSize) break; *\/ */
/* /\* 	} *\/ */
/* /\* 	if (ib>0) { *\/ */
/* /\* 	  iframe++; *\/ */
/* /\* 	  // cout << ib << "-" << endl; *\/ */
/* /\* 	  return (char*)afifo_cont; *\/ */
/* /\* 	} else { *\/ */
/* /\* 	  delete [] afifo_cont; *\/ */
/* /\* 	  return NULL; *\/ */
/* /\* 	} *\/ */
/* /\*       }      *\/ */
/* /\*       return NULL; *\/ */
/* /\*     }; *\/ */


/*   virtual char *readNextFrame(ifstream &filebin, int& ff, int &np) { */
/* 	  char *data=new char[packetSize*nPackets]; */
/* 	  char *retval=0; */
/* 	  int nd; */
/* 	  np=0; */
/* 	  int  pn; */
/* 	  char aa[8224]={0}; */
/* 	  char *packet=(char *)aa; */
/* 	  int  fnum = -1; */
	  
/* 	  if (ff>=0) */
/* 	    fnum=ff; */

/* 	  if (filebin.is_open()) { */


/* 	    cout << "+"; */

/* 	    while(filebin.read((char*)packet, 8208)){ */
	      
/* 	      pn=getPacketNumber(packet); */
	      
/* 	      if (fnum<0) */
/* 		fnum= getFrameNumber(packet); */
	      
/* 	      if (fnum>=0) { */
/* 		if (getFrameNumber(packet) !=fnum) {  */
		  
/* 		  if (np==0){ */
/* 		    cout << "-"; */
/* 		    delete [] data; */
/* 		    return NULL; */
/* 		  } else */
/* 		    filebin.seekg(-8208,ios_base::cur); */

/* 		    return data; */
/* 		} */
		
/* 		memcpy(data+(pn-1)*packetSize, packet, packetSize); */
/* 		np++; */
/* 		if (np==nPackets) */
/* 		  break; */
/* 		if (pn==nPackets) */
/* 		  break; */
/* 	      } */
/* 	    } */
	    
/* 	  } */
	  
/* 	  if (np==0){ */
/* 	    cout << "?"; */
/* 	    delete [] data; */
/* 	    return NULL; */
/* 	  }// else if (np<nPackets) */
/* 	    //  cout << "Frame " << fnum << " lost "  << nPackets-np << " packets " << endl; */

/* 	  return data; */
	  
/* 	}; */

/* int getPacketNumber(int x, int y) {return dataMap[y][x]/8208;}; */

/*   	virtual char *readNextFrame(ifstream &filebin) { */
/* 	  int fnum=-1, np; */
/* 	  return readNextFrame(filebin, fnum, np); */
/* 	}; */

};




#endif
