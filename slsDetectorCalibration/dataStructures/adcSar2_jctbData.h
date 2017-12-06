#ifndef ADCSAR2_JCTBDATA_H
#define ADCSAR2_JCTBDATA_H


class adcSar2_jctbData : public slsDetectorData<short unsigned int> {


 public:
 adcSar2_jctbData(int nsamples=1000): slsDetectorData<short unsigned int>(nsamples,1,nsamples*8,NULL,NULL,NULL){};

  virtual void getPixel(int ip, int &x, int &y) {x=ip/8; y=1;};
  
  virtual short unsigned int getChannel(char *data, int ix, int iy=0) {
    int adcvalue=0;
    int vv1= *((int16_t*) (data+8*ix));
    int vv2= *((int16_t*) (data+8*ix+2));
    for (int jj=0;jj<8;jj++){	
      adcvalue=adcvalue+   (((vv1>>(jj*2)) & 0x1)<<(jj));
    }
    for (int jj=0;jj<4;jj++){
      adcvalue=adcvalue+   (((vv2>>(jj*2)) & 0x1)<<(jj+8));
    }
    return adcvalue;
  };
  
  virtual int getFrameNumber(char *buff) {return frameNumber;};
 
  virtual  char *findNextFrame(char *data, int &ndata, int dsize) {
    ndata=dsize;
    return data;
  }
  
 virtual char *readNextFrame(ifstream &filebin) {
    char *data=NULL;
    if (filebin.is_open()) {
      data=new char[dataSize];
      filebin.read(data,dataSize);
    }
    return data;
  }
 
 /* virtual int **getData(char *ptr, int dsize=-1) { */
 /*   int **val; */
 /*   val=new int*[1]; */
 /*   val[0]=mythen03_frame(ptr,dynamicRange,nx,serialOffset); */
 /*   return val; */
   
 /* } */
 


 virtual int setFrameNumber(int f=0) {if (f>=0) frameNumber=f; return frameNumber; };

 private:
 
 int frameNumber;
   
   


};

#endif
