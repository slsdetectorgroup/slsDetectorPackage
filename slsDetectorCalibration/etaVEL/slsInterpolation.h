#ifndef SLS_INTERPOLATION_H
#define SLS_INTERPOLATION_H

#include <TObject.h>
#include <TTree.h>
#include <TH2F.h>

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

class slsInterpolation : public TObject{

 public:
 slsInterpolation(int nx=40, int ny=160, int ns=25) :nPixelsX(nx), nPixelsY(ny),  nSubPixels(ns) {hint=new TH2F("hint","hint",ns*nx, 0, nx, ns*ny, 0, ny);};
 
  //create eta distribution, eta rebinnining etc.
  //returns flat field image
  virtual void prepareInterpolation(int &ok)=0;

  //create interpolated image
  //returns interpolated image
  virtual TH2F *getInterpolatedImage(){return hint;};
  //return position inside the pixel for the given photon
  virtual void getInterpolatedPosition(Int_t x, Int_t y, Double_t *data, Double_t &int_x, Double_t &int_y)=0;

  TH2F *addToImage(Double_t int_x, Double_t int_y){hint->Fill(int_x, int_y); return hint;};



  virtual int addToFlatField(Double_t *cluster, Double_t &etax, Double_t &etay)=0;
  virtual int addToFlatField(Double_t etax, Double_t etay)=0;
  
  //virtual void Streamer(TBuffer &b);

  
  static int calcQuad(Double_t *cl, Double_t &sum, Double_t &totquad, Double_t sDum[2][2]){
    
    int corner = UNDEFINED_QUADRANT;
    Double_t *cluster[3];
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

  static int calcEta(Double_t totquad, Double_t sDum[2][2], Double_t &etax, Double_t &etay){
    Double_t t,r;
    
    if (totquad>0) {
      t = sDum[1][0] + sDum[1][1];
      r = sDum[0][1] + sDum[1][1];
      etax=r/totquad;
      etay=t/totquad;
    }
    return 0;
    
  } 


  static int calcEta(Double_t *cl, Double_t &etax, Double_t &etay, Double_t &sum, Double_t &totquad, Double_t sDum[2][2]) {
    int corner = calcQuad(cl,sum,totquad,sDum);
    calcEta(totquad, sDum, etax, etay);
    
    return corner;
  }


  static int calcEtaL(Double_t totquad, int corner, Double_t sDum[2][2], Double_t &etax, Double_t &etay){
    Double_t t,r, toth, totv;
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

  static int calcEtaL(Double_t *cl, Double_t &etax, Double_t &etay, Double_t &sum, Double_t &totquad, Double_t sDum[2][2]) {
    int corner = calcQuad(cl,sum,totquad,sDum);
    calcEtaL(totquad, corner, sDum, etax, etay);
    
    return corner;
  }



  static int calcEtaC3(Double_t *cl, Double_t &etax, Double_t &etay, Double_t &sum, Double_t &totquad, Double_t sDum[2][2]){
    
    int corner = calcQuad(cl,sum,totquad,sDum);
    calcEta(sum, sDum, etax, etay);
    return corner;
    
  }



  static int calcEta3(Double_t *cl, Double_t &etax, Double_t &etay, Double_t &sum) {
    Double_t l,r,t,b;
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



  static int calcEta3X(Double_t *cl, Double_t &etax, Double_t &etay, Double_t &sum) {
    Double_t l,r,t,b;
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
  TH2F *hint;


  //  ClassDefNV(slsInterpolation,1);
  // #pragma link C++ class slsInterpolation-;
};

#endif
