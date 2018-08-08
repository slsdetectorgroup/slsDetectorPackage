#ifndef MOENCH02CTBDATA_H
#define  MOENCH02CTBDATA_H
#include "slsDetectorData.h"



class moench02CtbData : public slsDetectorData<uint16_t> {

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
  

  moench02CtbData(int ns=6400): slsDetectorData<uint16_t>(160, 160, ns*2*32, NULL, NULL) , nadc(32), sc_width(40), sc_height(160) {

    
    int adc_off[4]={0,40,80,120};
    int adc_nr[4]={8,10,20,23};
    int row, col;

    int isample;
    int iadc, iiadc;
    int ix, iy;
    maplength=this->getDataSize()/2;
    //cout << maplength << endl;

    for (iiadc=0; iiadc<4; iiadc++) {
      
      iadc=adc_nr[iiadc];
      //cout << iiadc << endl;
      for (int i=0; i<sc_width*sc_height; i++) {
	
	col=adc_off[iiadc]+(i%sc_width);
	row=i/sc_width;
	dataMap[row][col]=(32*i+iadc)*2;
	if (dataMap[row][col]<0 || dataMap[row][col]>=dataSize) {
	  cout << "Error: pointer " << dataMap[row][col] << " out of range "<< endl;	  
	}
	  
      }
    }
    
    for (int i=0; i<maplength; i++) {
      //cout << i << endl;
      isample=i/32;
      iiadc=i%32;
      iadc=-1;
      for (int iii=0; iii<4; iii++) {
	if (iiadc==adc_nr[iii]) iadc=iii;
      }
      ix=isample%sc_width;
      iy=isample/sc_width;
      if(iadc>=0){
	xmap[i]=adc_off[iadc]+ix;
	ymap[i]=iy;
      }else{
	xmap[i]=-1;
	ymap[i]=-1;
      }
    }
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


    virtual  int getFrameNumber(char *buff){(void)buff; return iframe;};   

  /**

     Returns the packet number for the given dataset. purely virtual func
     \param buff pointer to the dataset
     \returns packet number number


  virtual int getPacketNumber(char *buff)=0;

  */

   /**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func
     \param data pointer to the memory to be analyzed
     \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot
     \param dsize size of the memory slot to be analyzed
     \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found 

  */
    virtual  char *findNextFrame(char *data, int &ndata, int dsize){ndata=dsize; setDataSize(dsize);  return data;};


   /**

     Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors! 
     \param filebin input file stream (binary)
     \returns pointer to the begin of the last good frame, NULL if no frame is found or last frame is incomplete

  */
    virtual char *readNextFrame(ifstream &filebin){
      //	int afifo_length=0;  
      uint16_t *afifo_cont; 
      int ib=0;
      if (filebin.is_open()) {
	afifo_cont=new uint16_t[dataSize/2];
 	while (filebin.read(((char*)afifo_cont)+ib,2)) {
	  ib+=2;
	  if (ib==dataSize) break;
	}
	if (ib>0) {
	  iframe++;
	  //cout << ib/2 << "-" << endl;
	  //for (int i=0; i<ib/2; i++)
	  //cout << i << " " << afifo_cont[i] << endl;
	  return (char*)afifo_cont;
	} else {
	  delete [] afifo_cont;
	  return NULL;
	}
      }     
      return NULL;
    };


  

};



#endif
