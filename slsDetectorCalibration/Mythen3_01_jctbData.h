#ifndef MYTHEN301JCTBDATA_H
#define MYTHEN301JCTBDATA_H


class mythen3_01_jctbData : public slsDetectorData<short unsigned int> {


 public:
 mythen3_01_jctbData( int nch=64*3,int dr=24, int off=5): slsDetectorData<short unsigned int>(64*3,1,dr*8*nch,NULL,NULL,NULL), dynamicRange(dr), serialOffset(off), frameNumber(0), numberOfCounters(nch) {};

  virtual void getPixel(int ip, int &x, int &y) {x=-1; y=-1;};
  
  virtual short unsigned int getChannel(char *data, int ix, int iy=0) {
    int ret=-1;
    short unsigned int *val=mythen03_frame(data,dynamicRange,numberOfCounters,serialOffset);
    if (ix>=0 && ix<numberOfCounters) ret=val[ix];
    delete [] val;
    return ret;
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
 
 virtual short unsigned int **getData(char *ptr, int dsize=-1) {
   short unsigned int **val;
   val=new short unsigned int*[1];
   val[0]=mythen03_frame(ptr,dynamicRange,nx,serialOffset);
   return val;
   
 }
 

 static short unsigned int* mythen03_frame(char *ptr, int dr=24,  int nch=64*3, int off=5) {
   // off=0;
    int iarg;
    int64_t word, *wp;
    short unsigned int* val=new short unsigned int[nch];
    int bit[64];
    int nb=2;
    int ioff=0;
    int idr=0;
    int ib=0;
    int iw=0;
    int ii=0;
    bit[0]=19;
    bit[1]=8;
    idr=0;
    for (ib=0; ib<nch; ib++) {
      val[ib]=0;
    }
    wp=(int64_t*)ptr;
    
    for (iw=0; iw<nch/nb; iw) {
      word=*wp;;
      if (ioff<off) {
	ioff++;
	 cout <<"*";
      }  else {
	
	if (idr<16) {
	  for (ib=0; ib<nb; ib++) {
	    if (word&(1<<bit[ib]))  {
	      cout << "+" ;
	      val[iw+nch*(ib/nb)]|=(1<<idr);
	    }  else { 
	      	cout << "-" ; 
	       } 
	  }//end for()
	}
	
	idr++;
	
	
	if (idr==dr) {
	  idr=0;
	  //  cout << dec << " " << iw << " " << val[iw] << " " << val[iw+nch/2] << endl;
	  cout <<dec << iw<<endl;
	  iw++;
	}//end if()

      }//end else()
      wp+=1;
      ii++;
    }//end for
    
    cout << "Decoded "<<ii << " samples"<< endl;
    cout << "Should be "<< nch/nb*dr+off << " samples"<< endl;
  
    return val;
 }

 virtual int setFrameNumber(int f=0) {if (f>=0) frameNumber=f; return frameNumber; };
 virtual int setDynamicRange(int d=-1) {if (d>0 && d<=24) dynamicRange=d; return dynamicRange;};
 virtual int setSerialOffset(int d=-1) {if (d>=0) serialOffset=d; return serialOffset;};
 virtual int setNumberOfCounters(int d=-1) {if (d>=0) numberOfCounters=d; return numberOfCounters;};
  

 private:
 
 int dynamicRange;
 int serialOffset;
 int frameNumber;
 int numberOfCounters;
   
   


};

#endif
