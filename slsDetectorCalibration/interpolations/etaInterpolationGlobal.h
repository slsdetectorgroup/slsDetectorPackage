#ifndef ETA_INTERPOLATION_GLOBAL_H
#define ETA_INTERPOLATION_GLOBAL_H


#include "etaInterpolationBase.h"

class etaInterpolationGlobal : public virtual etaInterpolationBase {
 public:
 etaInterpolationGlobal(int nx=400, int ny=400, int ns=25, int nsy=25, int nb=-1, int nby=-1, double emin=1, double emax=0) : etaInterpolationBase(nx,ny,ns,nsy, nb, nby,emin,emax){};
  
  

  virtual void prepareInterpolation(int &ok)
  {
   ok=1;  
#ifdef MYROOT1   
   if (hhx) delete hhx;
   if (hhy) delete hhy;

   hhx=new TH2D("hhx","hhx",heta->GetNbinsX(),heta->GetXaxis()->GetXmin(),heta->GetXaxis()->GetXmax(), heta->GetNbinsY(),heta->GetYaxis()->GetXmin(),heta->GetYaxis()->GetXmax()); 
   hhy=new TH2D("hhy","hhy",heta->GetNbinsX(),heta->GetXaxis()->GetXmin(),heta->GetXaxis()->GetXmax(), heta->GetNbinsY(),heta->GetYaxis()->GetXmin(),heta->GetYaxis()->GetXmax()); 
   
#endif


   ///*Eta Distribution Rebinning*///
   // double bsizeX=1./nSubPixelsX; //precision
   // cout<<"nPixelsX = "<<nPixelsX<<" nPixelsY = "<<nPixelsY<<" nSubPixels = "<<nSubPixels<<endl;
   double tot_eta=0;
   for (int ip=0; ip<nbetaX*nbetaY; ip++)
     tot_eta+=heta[ip];
   cout << "total eta entries is :"<< tot_eta << endl;   
   if (tot_eta<=0) {ok=0; return;};


   double hx[nbetaX]; //projection x
   double hy[nbetaY]; //projection y

   for (int ibx=0; ibx<nbetaX; ibx++) { hx[ibx]=0;}

     for (int iby=0; iby<nbetaY; iby++) { hy[iby]=0;}

   for (int ibx=0; ibx<nbetaX; ibx++) {
     for (int iby=0; iby<nbetaY; iby++) {
       hx[ibx]=hx[ibx]+heta[ibx+iby*nbetaX];
       hy[iby]=hy[iby]+heta[ibx+iby*nbetaX];
     }
   }
  double hix[nbetaX]; //integral of projection x
  double hiy[nbetaY]; //integral of projection y
  hix[0]=hx[0];
  hiy[0]=hy[0];
  
  for (int ib=1; ib<nbetaX; ib++) {
    hix[ib]=hix[ib-1]+hx[ib];
  }

  for (int ib=1; ib<nbetaY; ib++) {
    hiy[ib]=hiy[ib-1]+hy[ib];
  }
  double tot_x=hix[nbetaX-1]+1;
  double tot_y=hiy[nbetaY-1]+1;
  int ib=0;

  for (int ibx=0; ibx<nbetaX; ibx++) {
    //     if (hix[ibx]>(ib+1)*tot_eta*bsize) ib++;
    for (int iby=0; iby<nbetaY; iby++) {

      if (tot_x>0)
	hhx[ibx+iby*nbetaX]=hix[ibx]/tot_x; 
      else
	hhx[ibx+iby*nbetaX]=-1; 
      
      if (tot_y>0)
       hhy[ibx+iby*nbetaX]=hiy[iby]/tot_y;  
      else
	hhy[ibx+iby*nbetaX]=-1; 
     }
   }
/*    ib=0; */
/*    for (int iby=0; iby<nbeta; iby++) { */
/*      if (hiy[iby]>(ib+1)*tot_eta*bsize) ib++; */
/*      for (int ibx=0; ibx<nbeta; ibx++) { */
/* #ifdef MYROOT1  */
/*        hhy->SetBinContent(ibx+1,iby+1,ib);  */
/* #endif */
/* #ifndef MYROOT1   */
/*        hhy[ibx+iby*nbeta]=ib;  */
/* #endif */
/*      } */
/*    } */

   return ;
  };
  
 etaInterpolationGlobal(etaInterpolationGlobal *orig): etaInterpolationBase(orig)  {};

};


//etaInterpolationBase(nx,ny, ns, nsy, nb, nby, emin, emax),

class eta2InterpolationGlobal : public virtual eta2InterpolationBase, public virtual etaInterpolationGlobal {
  
 public:
 eta2InterpolationGlobal(int nx=400, int ny=400, int ns=25, int nsy=25, int nb=-1, int nby=-1, double emin=1, double emax=0) : eta2InterpolationBase(nx,ny, ns, nsy, nb, nby, emin, emax), etaInterpolationGlobal(nx,ny, ns, nsy, nb, nby, emin, emax){
    cout << "e2pxy " << nbetaX << " " << nbetaY << etamin << " " << etamax << " " << nSubPixelsX<< " " << nSubPixelsY << endl;  
  };
 
 eta2InterpolationGlobal(eta2InterpolationGlobal *orig): etaInterpolationBase(orig), etaInterpolationGlobal(orig)  {};

  virtual eta2InterpolationGlobal* Clone() { return new eta2InterpolationGlobal(this);};

};

//etaInterpolationBase(nx,ny, ns, nsy, nb, nby, emin, emax),

class eta3InterpolationGlobal : public virtual eta3InterpolationBase, public virtual etaInterpolationGlobal {
 public:
 eta3InterpolationGlobal(int nx=400, int ny=400, int ns=25, int nsy=25, int nb=-1, int nby=-1, double emin=1, double emax=0) :  eta3InterpolationBase(nx,ny, ns, nsy, nb, nby, emin, emax), etaInterpolationGlobal(nx,ny, ns, nsy, nb, nby, emin, emax){
    cout << "e3pxy " << nbetaX << " " << nbetaY << etamin << " " << etamax << " " << nSubPixelsX<< " " << nSubPixelsY << endl;  
  };

 eta3InterpolationGlobal(eta3InterpolationGlobal *orig): etaInterpolationBase(orig), etaInterpolationGlobal(orig)  {};

  virtual eta3InterpolationGlobal* Clone() {return new eta3InterpolationGlobal(this);};
};

#endif
