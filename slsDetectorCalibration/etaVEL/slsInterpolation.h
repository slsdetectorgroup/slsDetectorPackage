#ifndef SLS_INTERPOLATION_H
#define SLS_INTERPOLATION_H

#ifdef MYROOT1
#include <TObject.h>
#include <TTree.h>
#include <TH2F.h>
#endif

#ifndef DEF_QUAD
#define DEF_QUAD
 enum quadrant {
    TOP_LEFT=0,
    TOP_RIGHT=1,
    BOTTOM_LEFT=2,
    BOTTOM_RIGHT=3,
    UNDEFINED_QUADRANT=-1
  };
#endif

//#ifdef MYROOT1
//: public TObject
//#endif
class slsInterpolation 
{

 public:
 slsInterpolation(int nx=400, int ny=400, int ns=25) :nPixelsX(nx), nPixelsY(ny),  nSubPixels(ns) {
   
#ifdef MYROOT1
hint=new TH2F("hint","hint",ns*nx, 0, nx, ns*ny, 0, ny);
#endif
   
#ifndef MYROOT1
 hint=new int[ns*nx*ns*ny];
#endif

};
  
  int getNSubPixels() {return nSubPixels;};
  
  
  //create eta distribution, eta rebinnining etc.
  //returns flat field image
  virtual void prepareInterpolation(int &ok)=0;

  //create interpolated image
  //returns interpolated image
#ifdef MYROOT1
  virtual TH2F *getInterpolatedImage(){return hint;};
#endif

#ifndef MYROOT1
  virtual int *getInterpolatedImage(){return hint;};
#endif
  //return position inside the pixel for the given photon
  virtual void getInterpolatedPosition(int x, int y, double *data, double &int_x, double &int_y)=0;
  //return position inside the pixel for the given photon
  virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int quad, double &int_x, double &int_y)=0;

#ifdef MYROOT1
  TH2F *addToImage(double int_x, double int_y){hint->Fill(int_x, int_y); return hint;};
#endif

#ifndef MYROOT1
  virtual int *addToImage(double int_x, double int_y){ int iy=nSubPixels*int_y; int ix=nSubPixels*int_x; 
    if (ix>=0 && ix<(nPixelsX*nSubPixels) && iy<(nSubPixels*nPixelsY) && iy>=0 )(*(hint+ix+iy*nPixelsX))+=1; 
    return hint;
  };
#endif


  virtual int addToFlatField(double *cluster, double &etax, double &etay)=0;
  virtual int addToFlatField(double etax, double etay)=0;
  
#ifdef MYROOT1
  virtual TH2D *getFlatField(){return NULL;};
#endif

#ifndef MYROOT1
  virtual int *getFlatField(){return NULL;};
#endif

  //virtual void Streamer(TBuffer &b);

  
  static int calcQuad(double *cl, double &sum, double &totquad, double sDum[2][2]){
    
    int corner = UNDEFINED_QUADRANT;
    double *cluster[3];
    cluster[0]=cl;
    cluster[1]=cl+3;
    cluster[2]=cl+6;

    sum = cluster[0][0] + cluster[1][0] + cluster[2][0] + cluster[0][1] + cluster[1][1] + cluster[2][1] + cluster[0][2] + cluster[1][2] + cluster[2][2];
    
    double sumBL = cluster[0][0] + cluster[1][0] + cluster[0][1] + cluster[1][1]; //2 ->BL
    double sumTL = cluster[1][0] + cluster[2][0] + cluster[2][1] + cluster[1][1]; //0 ->TL
    double sumBR = cluster[0][1] + cluster[0][2] + cluster[1][2] + cluster[1][1]; //3 ->BR
    double sumTR = cluster[1][2] + cluster[2][1] + cluster[2][2] + cluster[1][1]; //1 ->TR
    double sumMax = 0;
    double t, r;
    
    // if(sumTL  >= sumMax){
    sDum[0][0] = cluster[0][0]; sDum[1][0] = cluster[1][0];
    sDum[0][1] = cluster[0][1]; sDum[1][1] = cluster[1][1];
    corner = BOTTOM_LEFT;
    sumMax=sumBL;
    // } 
    
    if(sumTL  >= sumMax){
      sDum[0][0] = cluster[1][0]; sDum[1][0] = cluster[2][0];
      sDum[0][1] = cluster[1][1]; sDum[1][1] = cluster[2][1];

      corner = TOP_LEFT;
      sumMax=sumTL;
    } 

    if(sumBR  >= sumMax){
      sDum[0][0] = cluster[0][1]; sDum[1][0] = cluster[1][1];
      sDum[0][1] = cluster[0][2]; sDum[1][1] = cluster[1][2];
      
      corner = BOTTOM_RIGHT;
      sumMax=sumBR;
    }
    
    if(sumTR  >= sumMax){
      sDum[0][0] = cluster[1][1]; sDum[1][0] = cluster[2][1];
      sDum[0][1] = cluster[1][2]; sDum[1][1] = cluster[2][2];
      
      corner = TOP_RIGHT;
      sumMax=sumTR;
    }
    
    totquad=sumMax;
  
    return corner;
    
  } 

  static int calcEta(double totquad, double sDum[2][2], double &etax, double &etay){
    double t,r;
    
    if (totquad>0) {
      t = sDum[1][0] + sDum[1][1];
      r = sDum[0][1] + sDum[1][1];
      etax=r/totquad;
      etay=t/totquad;
    }
    return 0;
    
  } 


  static int calcEta(double *cl, double &etax, double &etay, double &sum, double &totquad, double sDum[2][2]) {
    int corner = calcQuad(cl,sum,totquad,sDum);
    calcEta(totquad, sDum, etax, etay);
    
    return corner;
  }


  static int calcEtaL(double totquad, int corner, double sDum[2][2], double &etax, double &etay){
    double t,r, toth, totv;
    if (totquad>0) {
      switch(corner) {
      case TOP_LEFT:
	t = sDum[1][1] ;
	r = sDum[0][1] ;
	toth=sDum[1][1]+sDum[1][0];
	totv=sDum[0][1]+sDum[1][1];
	break;
      case TOP_RIGHT:
	t = sDum[1][0] ;
	r = sDum[0][1] ;
	toth=sDum[0][0]+t;
	totv=sDum[0][0]+r;
	break;
      case BOTTOM_LEFT:
	r = sDum[1][1] ;
	t = sDum[1][1] ;
	toth=sDum[1][0]+t;
	totv=sDum[0][1]+r;
	break;
      case BOTTOM_RIGHT:
	t = sDum[1][0] ;
	r = sDum[1][1] ;
	toth=sDum[1][1]+t;
	totv=sDum[0][1]+r;
	break;
      default:
	etax=-1;
	etay=-1;
	return 0;
      }
      etax=r/totv;
      etay=t/toth;
    }
    return 0;
  }

  static int calcEtaL(double *cl, double &etax, double &etay, double &sum, double &totquad, double sDum[2][2]) {
    int corner = calcQuad(cl,sum,totquad,sDum);
    calcEtaL(totquad, corner, sDum, etax, etay);
    
    return corner;
  }



  static int calcEtaC3(double *cl, double &etax, double &etay, double &sum, double &totquad, double sDum[2][2]){
    
    int corner = calcQuad(cl,sum,totquad,sDum);
    calcEta(sum, sDum, etax, etay);
    return corner;
    
  }



  static int calcEta3(double *cl, double &etax, double &etay, double &sum) {
    double l,r,t,b;
    sum=cl[0]+cl[1]+cl[2]+cl[3]+cl[4]+cl[5]+cl[6]+cl[7]+cl[8];
    if (sum>0) {
      l=cl[0]+cl[3]+cl[6];
      r=cl[2]+cl[5]+cl[8];
      b=cl[0]+cl[1]+cl[2];
      t=cl[6]+cl[7]+cl[8];
      etax=(-l+r)/sum;
      etay=(-b+t)/sum;
    }
    
    return -1;
  }



  static int calcEta3X(double *cl, double &etax, double &etay, double &sum) {
    double l,r,t,b;
    sum=cl[0]+cl[1]+cl[2]+cl[3]+cl[4]+cl[5]+cl[6]+cl[7]+cl[8];
    if (sum>0) {
      l=cl[3];
      r=cl[5];
      b=cl[1];
      t=cl[7];
      etax=(-l+r)/sum;
      etay=(-b+t)/sum;
    }
    return -1;
  }






 protected:
  int nPixelsX, nPixelsY;
  int nSubPixels; 
#ifdef MYROOT1
  TH2F *hint;
#endif
#ifndef MYROOT1
  int *hint;
#endif

};

#endif
