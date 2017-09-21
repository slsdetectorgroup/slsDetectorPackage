#ifndef ETA_INTERPOLATION_BASE_H
#define ETA_INTERPOLATION_BASE_H

#ifdef MYROOT1
#include <TObject.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH2F.h>
#endif

#include "slsInterpolation.h"

class etaInterpolationBase : public slsInterpolation {
  
 public:
 etaInterpolationBase(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : slsInterpolation(nx,ny,ns), hhx(NULL), hhy(NULL), heta(NULL),nbeta(nb),etamin(emin), etamax(emax) {
    if (nb<=0) 
      nbeta=nSubPixels*10;   
    if (etamin>=etamax) {
      etamin=-0.1;
      etamax=1.1;
    }
    etastep=(etamax-etamin)/nbeta;
#ifdef MYROOT1
    heta=new TH2D("heta","heta",nbeta,etamin,etamax,nbeta,etamin,etamax);
    hhx=new TH2D("hhx","hhx",nbeta,etamin,etamax,nbeta,etamin,etamax);
    hhy=new TH2D("hhy","hhy",nbeta,etamin,etamax,nbeta,etamin,etamax);
#endif
#ifndef MYROOT1
    heta=new int[nbeta*nbeta];
    hhx=new int[nbeta*nbeta];
    hhy=new int[nbeta*nbeta];
    
#endif
    
  };
  
#ifdef MYROOT1
  TH2D *setEta(TH2D *h, int nb=-1, double emin=1, double emax=0)
  {  
     if (h) { heta=h;
       nbeta=heta->GetNbinsX();
       etamin=heta->GetXaxis()->GetXmin();
       etamax=heta->GetXaxis()->GetXmax();
       etastep=(etamax-etamin)/nbeta;
     }
    return heta;
  };
  TH2D *getFlatField(){return setEta(NULL);};
#endif
  
#ifndef MYROOT1
  int *setEta(int *h, int nb=-1, double emin=1, double emax=0)
  {  
    if (h) {heta=h;
      nbeta=nb;
      if (nb<=0) nbeta=nSubPixels*10;
      etamin=emin;
      etamax=emax;
      if (etamin>=etamax) {
	etamin=-0.1;
	etamax=1.1;
      }
      etastep=(etamax-etamin)/nbeta;
    }
    return heta;
  };
  int *getFlatField(){return setEta(NULL);};
#endif
  

  
virtual void prepareInterpolation(int &ok)=0;
 
  /* ////////////////////////////////////////////////////////////////////////////// */

#ifdef MYROOT1
TH2D *gethhx()
  {
    hhx->Scale((double)nSubPixels);
    return hhx;
  };
  
  TH2D *gethhy()
  {
    hhy->Scale((double)nSubPixels);
    return hhy;
    };
#endif
    
#ifndef MYROOT1
int *gethhx()
  {
    // hhx->Scale((double)nSubPixels);
    return hhx;
  };
  
  int *gethhy()
  {
    // hhy->Scale((double)nSubPixels);
    return hhy;
    };
#endif
    
  //////////////////////////////////////////////////////////////////////////////
  //////////// /*It return position hit for the event in input */ //////////////
  virtual void getInterpolatedPosition(int x, int y, double *data, double &int_x, double &int_y)
  {
    double sDum[2][2];
    double tot, totquad;
    double etax,etay;
    
    int corner;
    corner=calcQuad(data, tot, totquad, sDum); 
    
    calcEta(totquad, sDum, etax, etay); 
    getInterpolatedPosition(x,y,etax,etay,corner,int_x,int_y);
    return;
  };
  

  virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int corner, double &int_x, double &int_y)
  {


    double xpos_eta,ypos_eta;
    double dX,dY;
    double ex,ey;
    switch (corner)
      {
      case TOP_LEFT:
	dX=-1.; 
	dY=+1.; 
	break;
      case TOP_RIGHT:
	dX=+1.; 
	dY=+1.; 
	break;
      case BOTTOM_LEFT:
	dX=-1.; 
	dY=-1.; 
	break;
      case BOTTOM_RIGHT:
	dX=+1.; 
	dY=-1.; 
	break;
      default:
	dX=0.; 
	dY=0.;
      }
    


#ifdef MYROOT1
    xpos_eta=(hhx->GetBinContent(hhx->GetXaxis()->FindBin(etax),hhy->GetYaxis()->FindBin(etay)))/((double)nSubPixels);
    ypos_eta=(hhy->GetBinContent(hhx->GetXaxis()->FindBin(etax),hhy->GetYaxis()->FindBin(etay)))/((double)nSubPixels);
#endif
#ifndef MYROOT1
    ex=(etax-etamin)/etastep;
    ey=(etay-etamin)/etastep;
    if (ex<0) ex=0;
    if (ex>=nSubPixels) ex=nSubPixels-1;
    if (ey<0) ey=0;
    if (ey>=nSubPixels) ey=nSubPixels-1;
    
   
    xpos_eta=(((double)hhx[(int)(ey*nbeta+ex)]))/((double)nSubPixels);
    ypos_eta=(((double)hhy[(int)(ey*nbeta+ex)]))/((double)nSubPixels);
      //else
      //return 0;

#endif
    
    int_x=((double)x) + 0.5*dX + xpos_eta;
    int_y=((double)y) + 0.5*dY + ypos_eta;
    //return 1;

  }






  /////////////////////////////////////////////////////////////////////////////////////////////////
   virtual void getPositionETA3(int x, int y, double *data, double &int_x, double &int_y)
   {
    double sDum[2][2];
    double tot, totquad;
    double eta3x,eta3y;
    double ex,ey;
    
    calcQuad(data, tot, totquad, sDum); 
    calcEta3(data,eta3x, eta3y,tot); 
    
    double xpos_eta,ypos_eta;
    
#ifdef MYROOT1
    xpos_eta=((hhx->GetBinContent(hhx->GetXaxis()->FindBin(eta3x),hhy->GetYaxis()->FindBin(eta3y))))/((double)nSubPixels);
    ypos_eta=((hhy->GetBinContent(hhx->GetXaxis()->FindBin(eta3x),hhy->GetYaxis()->FindBin(eta3y))))/((double)nSubPixels);
    
#endif
#ifndef MYROOT1
    ex=(eta3x-etamin)/etastep;
    ey=(eta3y-etamin)/etastep;
    
    if (ex<0) ex=0;
    if (ex>=nSubPixels) ex=nSubPixels-1;
    if (ey<0) ey=0;
    if (ey>=nSubPixels) ey=nSubPixels-1;
      xpos_eta=(((double)hhx[(int)(ey*nbeta+ex)]))/((double)nSubPixels);
      ypos_eta=(((double)hhy[(int)(ey*nbeta+ex)]))/((double)nSubPixels);
#endif
    
    int_x=((double)x) + xpos_eta;
    int_y=((double)y) + ypos_eta;
    
    return;
  };
  
  //////////////////////////////////////////////////////////////////////////////////////
  virtual int addToFlatField(double *cluster, double &etax, double &etay){
    double sDum[2][2];
    double tot, totquad;
    int corner;
    corner=calcQuad(cluster, tot, totquad, sDum); 
    
    double xpos_eta,ypos_eta;
    double dX,dY;
  
    
    calcEta(totquad, sDum, etax, etay); 
   
    return addToFlatField(etax,etay);

    };


  virtual int addToFlatField(double etax, double etay){

    int ex,ey; 

#ifdef MYROOT1
    heta->Fill(etax,etay);
#endif
#ifndef MYROOT1
    ex=(etax-etamin)/etastep;
    ey=(etay-etamin)/etastep;
    if (ey<nbeta && ex<nbeta && ex>=0 && ey>=0)
      heta[ey*nbeta+ex]++;    
#endif
    return 0;    
};
  
 protected:
  
#ifdef MYROOT1
  TH2D *heta;
  TH2D *hhx;
  TH2D *hhy;
#endif
#ifndef MYROOT1
  int *heta;
  int *hhx;
  int *hhy;
#endif
  int nbeta;
  double etamin, etamax, etastep;

};

#endif
