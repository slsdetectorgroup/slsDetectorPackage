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
   for (int ib=0; ib<nbeta; ib++) {

     tot_eta_x=0;
     tot_eta_y=0;

     for (int iby=0; iby<nbeta; iby++) {
       hx[iby]=heta[iby+ib*nbeta];
       tot_eta_x+=hx[iby];
       hy[iby]=heta[ib+iby*nbeta];
       tot_eta_y+=hy[iby];
     }

     hix[0]=hx[0];
     hiy[0]=hy[0];

     for (int iby=1; iby<nbeta; iby++) {
       hix[iby]=hix[iby-1]+hx[iby];
       hiy[iby]=hiy[iby-1]+hy[iby];
     }

     ii=0;
     
     for (int ibx=0; ibx<nbeta; ibx++) {
       if (tot_eta_x==0) {
	 hhx[ibx+ib*nbeta]=((float)ibx)/((float)nbeta);
	 ii=(ibx)/nbeta;
       } else //if (hix[ibx]>(ii+1)*tot_eta_x*bsize) 
	 {
	 //ii++;
	 // cout << ib << " x " << ibx << " " << tot_eta_x << " " << (ii)*tot_eta_x*bsize << " " << ii << endl;
	 // }
#ifdef MYROOT1  
       hhx->SetBinContent(ibx+1,ib+1,ii);
#endif
#ifndef MYROOT1  
       hhx[ibx+ib*nbeta]=hix[ibx]/((float)tot_eta_x);//ii; 
#endif
	 }
     }
     /* if (ii!=(nSubPixels-1)) */
     /*   cout << ib << " x " <<  tot_eta_x << " " << (ii+1)*tot_eta_x*bsize << " " << ii << " " << hix[nbeta-1]<< endl; */
     
     ii=0;
     
     for (int ibx=0; ibx<nbeta; ibx++) {
       if (tot_eta_y==0) {
	 hhx[ibx+ib*nbeta]=((float)ibx)/((float)nbeta);
	 ii=(ibx*nSubPixels)/nbeta;
       } else //if (hiy[ibx]>(ii+1)*tot_eta_y*bsize) 
	 {
	   //ii++;
	 //cout << ib << " y " << ibx << " " << tot_eta_y << " "<< (ii)*tot_eta_y*bsize << " " << ii << endl;
	   //}
#ifdef MYROOT1  
       hhy->SetBinContent(ib+1,ibx+1,ii);
#endif
#ifndef MYROOT1  
       hhy[ib+ibx*nbeta]=hiy[ibx]/((float)tot_eta_y);//ii; 
#endif
	 }
     }
     /* if (ii!=(nSubPixels-1)) */
     /*   cout << ib << " y " <<  tot_eta_y << " " << (ii+1)*tot_eta_y*bsize << " " << ii << " " << hiy[nbeta-1]<< endl; */
     
     //	 cout << "y " << nbeta << " " << (ii+1)*tot_eta_x*bsize << " " << ii << endl;
     
   }

#ifdef SAVE_ALL
   char tit[10000];
   
  float *etah=new float[nbeta*nbeta];
  int etabins=nbeta;

  for (int ii=0; ii<etabins*etabins; ii++) {
    etah[ii]=hhx[ii];
  }
  sprintf(tit,"/scratch/eta_hhx_%d.tiff",id);
  WriteToTiff(etah, tit, etabins, etabins);
	  
  for (int ii=0; ii<etabins*etabins; ii++) {
    etah[ii]=hhy[ii];
  }
  sprintf(tit,"/scratch/eta_hhy_%d.tiff",id);
  WriteToTiff(etah, tit, etabins, etabins);
	  
  for (int ii=0; ii<etabins*etabins; ii++) {
    etah[ii]=heta[ii];
  }
  sprintf(tit,"/scratch/eta_%d.tiff",id);
  WriteToTiff(etah, tit, etabins, etabins);
  delete [] etah;
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
