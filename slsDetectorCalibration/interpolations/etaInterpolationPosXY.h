#ifndef ETA_INTERPOLATION_POSXY_H
#define ETA_INTERPOLATION_POSXY_H


//#include "tiffIO.h"
#include "etaInterpolationBase.h"
#include "eta2InterpolationBase.h"
#include "eta3InterpolationBase.h"

class etaInterpolationPosXY : public virtual etaInterpolationBase{
 public:
 etaInterpolationPosXY(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : etaInterpolationBase(nx,ny,ns, nb, emin,emax){
    //  cout << "epxy " << nb << " " << emin << " " << emax << endl; cout << nbeta << " " << etamin << " " << etamax << endl; 
  };

 etaInterpolationPosXY(etaInterpolationPosXY *orig): etaInterpolationBase(orig) {};

  virtual etaInterpolationPosXY* Clone()=0; /* { */

    /* return new etaInterpolationPosXY(this); */

    /* }; */

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
   double bsize=1./nSubPixels; //precision
   // cout<<"nPixelsX = "<<nPixelsX<<" nPixelsY = "<<nPixelsY<<" nSubPixels = "<<nSubPixels<<endl;
   double tot_eta=0;
   double tot_eta_x=0;
   double tot_eta_y=0;
   for (int ip=0; ip<nbeta*nbeta; ip++)
     tot_eta+=heta[ip];
   cout << "total eta entries is :"<< tot_eta << endl;   
   if (tot_eta<=0) {ok=0; return;};


   double hx[nbeta]; //profile x
   double hy[nbeta]; //profile y
   double hix[nbeta]; //integral of projection x
   double hiy[nbeta]; //integral of projection y
   int ii=0;
   double etax, etay;
   for (int ib=0; ib<nbeta; ib++) {

     tot_eta_x=0;
     tot_eta_y=0;

     for (int iby=0; iby<nbeta; iby++) {
       etax=etamin+iby*etastep;
       //cout << etax << endl;
       if (etax>=0 && etax<=1)
	 hx[iby]=heta[iby+ib*nbeta];
       else {
	 hx[iby]=0;
       }
       // tot_eta_x+=hx[iby];
       if (etax>=0 && etax<=1)
	 hy[iby]=heta[ib+iby*nbeta];
       else
	 hy[iby]=0;
       // tot_eta_y+=hy[iby];
     }

     hix[0]=hx[0];
     hiy[0]=hy[0];

     for (int iby=1; iby<nbeta; iby++) {
       hix[iby]=hix[iby-1]+hx[iby];
       hiy[iby]=hiy[iby-1]+hy[iby];
     }

     ii=0;
     tot_eta_x=hix[nbeta-1]+1;
     tot_eta_y=hiy[nbeta-1]+1;
     
     for (int ibx=0; ibx<nbeta; ibx++) {
       if (tot_eta_x<=0) {
	 hhx[ibx+ib*nbeta]=-1;
	 //ii=(ibx)/nbeta;
       } else //if (hix[ibx]>(ii+1)*tot_eta_x*bsize) 
	 { 
	   //if (hix[ibx]>tot_eta_x*(ii+1)/nSubPixels) ii++;
	   hhx[ibx+ib*nbeta]=hix[ibx]/tot_eta_x;
	 }
     }
     /* if (ii!=(nSubPixels-1)) */
     /*   cout << ib << " x " <<  tot_eta_x << " " << (ii+1)*tot_eta_x*bsize << " " << ii << " " << hix[nbeta-1]<< endl; */
     
     ii=0;
     
     for (int ibx=0; ibx<nbeta; ibx++) {
       if (tot_eta_y<=0) {
	 hhy[ib+ibx*nbeta]=-1;
	 //ii=(ibx*nSubPixels)/nbeta;
       } else {   
	 //if (hiy[ibx]>tot_eta_y*(ii+1)/nSubPixels) ii++;
	 hhy[ib+ibx*nbeta]=hiy[ibx]/tot_eta_y;
       }
     }
   }


   int ibx, iby, ib; 
   
   iby=0;
   while (hhx[iby*nbeta+nbeta/2]<0) iby++;
   for (ib=0; ib<iby;ib++) {
     for (ibx=0; ibx<nbeta;ibx++)
       hhx[ibx+nbeta*ib]=hhx[ibx+nbeta*iby];
   }
   iby=nbeta-1;
   
   while (hhx[iby*nbeta+nbeta/2]<0) iby--;
   for (ib=iby+1; ib<nbeta;ib++) {
     for (ibx=0; ibx<nbeta;ibx++)
       hhx[ibx+nbeta*ib]=hhx[ibx+nbeta*iby];
   }
   
   iby=0;
   while (hhy[nbeta/2*nbeta+iby]<0) iby++;
   for (ib=0; ib<iby;ib++) {
     for (ibx=0; ibx<nbeta;ibx++)
       hhy[ib+nbeta*ibx]=hhy[iby+nbeta*ibx];
   }
   iby=nbeta-1;
   
   while (hhy[nbeta/2*nbeta+iby]<0) iby--;
   for (ib=iby+1; ib<nbeta;ib++) {
     for (ibx=0; ibx<nbeta;ibx++)
       hhy[ib+nbeta*ibx]=hhy[iby+nbeta*ibx];
   }



     
   


#ifdef SAVE_ALL
   debugSaveAll();
#endif	  
  return ;
  }

};

class eta2InterpolationPosXY : public virtual eta2InterpolationBase, public virtual etaInterpolationPosXY {
 public:
 eta2InterpolationPosXY(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : etaInterpolationBase(nx,ny,ns, nb, emin,emax),eta2InterpolationBase(nx,ny,ns, nb, emin,emax),etaInterpolationPosXY(nx,ny,ns, nb, emin,emax){
    //  cout << "e2pxy " << nb << " " << emin << " " << emax << endl; 
  };
 
 eta2InterpolationPosXY(eta2InterpolationPosXY *orig): etaInterpolationBase(orig), etaInterpolationPosXY(orig)  {};

  virtual eta2InterpolationPosXY* Clone() { return new eta2InterpolationPosXY(this);};

};



class eta3InterpolationPosXY : public virtual eta3InterpolationBase, public virtual etaInterpolationPosXY {
 public:
 eta3InterpolationPosXY(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : etaInterpolationBase(nx,ny,ns, nb, emin,emax),eta3InterpolationBase(nx,ny,ns, nb, emin,emax), etaInterpolationPosXY(nx,ny,ns, nb, emin,emax){
      cout << "e3pxy " << nbeta << " " << etamin << " " << etamax << " " << nSubPixels<< endl;  
  };

 eta3InterpolationPosXY(eta3InterpolationPosXY *orig): etaInterpolationBase(orig), etaInterpolationPosXY(orig)  {};

  virtual eta3InterpolationPosXY* Clone() { return new eta3InterpolationPosXY(this);};
};

#endif
