#ifndef ETA_INTERPOLATION_BASE_H
#define ETA_INTERPOLATION_BASE_H

#ifdef MYROOT1
#include <TObject.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH2F.h>
#endif

#include "slsInterpolation.h"
#include "tiffIO.h"

class etaInterpolationBase : public slsInterpolation {
  
 public:
 
 etaInterpolationBase(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : slsInterpolation(nx,ny,ns), hhx(NULL), hhy(NULL), heta(NULL), nbeta(nb), etamin(emin), etamax(emax) {
    // cout << "eb " << nb << " " << emin << " " << emax << endl;  
    // cout << nb << " " << etamin << " " << etamax << endl;
    if (nbeta<=0) {
      //cout << "aaa:" <<endl;
      nbeta=nSubPixels*10;
    }
    if (etamin>=etamax) {
      etamin=-1;
      etamax=2;
    }
    etastep=(etamax-etamin)/nbeta;
    heta=new int[nbeta*nbeta];
    hhx=new float[nbeta*nbeta];
    hhy=new float[nbeta*nbeta];
    rangeMin=etamin;
    rangeMax=etamax;
    flat= new double[nSubPixels*nSubPixels];
    hintcorr=new int [nSubPixels*nSubPixels*nPixelsX*nPixelsY];
    
  };
  
 etaInterpolationBase(etaInterpolationBase *orig): slsInterpolation(orig){
   nbeta=orig->nbeta;
   etamin=orig->etamin;
   etamax=orig->etamax;
   rangeMin=orig->rangeMin;
   rangeMax=orig->rangeMax;


    etastep=(etamax-etamin)/nbeta;
    heta=new int[nbeta*nbeta];
    memcpy(heta,orig->heta,nbeta*nbeta*sizeof(int));
    hhx=new float[nbeta*nbeta];
    memcpy(hhx,orig->hhx,nbeta*nbeta*sizeof(float));
    hhy=new float[nbeta*nbeta];
    memcpy(hhy,orig->hhy,nbeta*nbeta*sizeof(float));
    hintcorr=new int [nSubPixels*nSubPixels*nPixelsX*nPixelsY];

 };

  



  virtual void resetFlatField() {
    for (int ibx=0; ibx<nbeta*nbeta; ibx++) {
      heta[ibx]=0;
      hhx[ibx]=0;
      hhy[ibx]=0;
    }


  };
    
  int *setEta(int *h, int nb=-1, double emin=1, double emax=0)
  {  
    if (h) {
      if (heta) delete [] heta;
      heta=h;
      nbeta=nb;
      if (nb<=0) nbeta=nSubPixels*10;
      etamin=emin;
      etamax=emax;
      if (etamin>=etamax) {
	etamin=-1;
	etamax=2;
      }
      rangeMin=etamin;
      rangeMax=etamax;
      etastep=(etamax-etamin)/nbeta;
    }
    return heta;
  };
  
   int *setFlatField(int *h, int nb=-1, double emin=1, double emax=0)
  {  
    return setEta(h, nb, emin, emax);
  };



  int *getFlatField(){return setEta(NULL);};
  
  int *getFlatField(int &nb, double &emin, double &emax){
    nb=nbeta; 
    emin=etamin; 
    emax=etamax; 
    return getFlatField();
  }; 
  
  
  void *writeFlatField(const char * imgname) {
    float *gm=NULL;
    gm=new float[nbeta*nbeta];
    for (int ix=0; ix<nbeta; ix++) {
      for (int iy=0; iy<nbeta; iy++) {
	gm[iy*nbeta+ix]=heta[iy*nbeta+ix];
      }
    } 
    WriteToTiff(gm, imgname, nbeta, nbeta);   
    delete [] gm;
    return NULL; 
  };
  
  int readFlatField(const char * imgname, double emin=1, double emax=0) {
    if (emax>=1) etamax=emax;
    if (emin<=0) etamin=emin;   
   
    if (etamin>=etamax) {
      etamin=-1;
      etamax=2;
    }
    
    etastep=(etamax-etamin)/nbeta;
    uint32 nnx;
    uint32 nny;
    float *gm=ReadFromTiff(imgname, nnx, nny);
    if (nnx!=nny) {
      cout << "different number of bins in x " << nnx << "  and y " << nny<< " !"<< endl;
      cout << "Aborting read"<< endl;
      return 0;
    }
    nbeta=nnx;
    if (gm) {
      if (heta) {
	delete [] heta;
	delete [] hhx;
	delete [] hhy;
      }
      
      heta=new int[nbeta*nbeta];
      hhx=new float[nbeta*nbeta];
      hhy=new float[nbeta*nbeta];
      
      for (int ix=0; ix<nbeta; ix++) {
	for (int iy=0; iy<nbeta; iy++) {
	  heta[iy*nbeta+ix]=gm[iy*nbeta+ix];
	}
      }
      delete [] gm;
      return 1;
    }
    return 0;
  };
  
    
  
    

float *gethhx()
  {
    // hhx->Scale((double)nSubPixels);
    return hhx;
  };
  
  float *gethhy()
  {
    // hhy->Scale((double)nSubPixels);
    return hhy;
    };
  virtual int addToFlatField(double etax, double etay){
    int ex,ey; 
    ex=(etax-etamin)/etastep;
    ey=(etay-etamin)/etastep;
    if (ey<nbeta && ex<nbeta && ex>=0 && ey>=0)
      heta[ey*nbeta+ex]++; 
    return 0;    
  };
  
  
  // virtual void prepareInterpolation(int &ok)=0;


  void debugSaveAll(int ind=0) {
    int ib, ibx, iby;
   char tit[10000];
   
   float tot_eta=0;

  float *etah=new float[nbeta*nbeta];
  int etabins=nbeta;
  int ibb=0;

  for (int ii=0; ii<etabins*etabins; ii++) {
   
      etah[ii]=heta[ii];
      tot_eta+=heta[ii];
  }
  sprintf(tit,"/scratch/eta.tiff",ind);
  WriteToTiff(etah, tit, etabins, etabins);


  for (int ii=0; ii<etabins*etabins; ii++) {
    ibb=(hhx[ii]*nSubPixels);
    etah[ii]=ibb;
  }
  sprintf(tit,"/scratch/eta_hhx_%d.tiff",ind);
  WriteToTiff(etah, tit, etabins, etabins);
	  
  for (int ii=0; ii<etabins*etabins; ii++) {
    ibb=hhy[ii]*nSubPixels;
    etah[ii]=ibb;
  }
  sprintf(tit,"/scratch/eta_hhy_%d.tiff",ind);
  WriteToTiff(etah, tit, etabins, etabins);
	  
  
  float *ftest=new float[nSubPixels*nSubPixels];

  for (int ib=0; ib<nSubPixels*nSubPixels; ib++) ftest[ib]=0;
  

  //int ibx=0, iby=0;
  
  for (int ii=0; ii<nbeta*nbeta; ii++) {
    
    ibx=nSubPixels*hhx[ii];
    iby=nSubPixels*hhy[ii];
    if (ibx<0) ibx=0;
    if (iby<0) iby=0;
    if (ibx>=nSubPixels) ibx=nSubPixels-1;
    if (iby>=nSubPixels) iby=nSubPixels-1;
    

    if (ibx>=0 && ibx<nSubPixels && iby>=0 && iby<nSubPixels) {
      //
      // if (ibx>0 && iby>0) cout << ibx << " " << iby << " " << ii << endl;
      ftest[ibx+iby*nSubPixels]+=heta[ii];
    } else
      cout << "Bad interpolation "<< ii << " " << ibx << " " << iby<< endl; 

  }

  sprintf(tit,"/scratch/ftest_%d.tiff",ind);
  WriteToTiff(ftest, tit, nSubPixels, nSubPixels);

  //int ibx=0, iby=0;
  tot_eta/=nSubPixels*nSubPixels;
  int nbad=0;
  for (int ii=0; ii<etabins*etabins; ii++) {
    ibx=nSubPixels*hhx[ii];
    iby=nSubPixels*hhy[ii];
    if (ftest[ibx+iby*nSubPixels]<tot_eta*0.5) {
      etah[ii]=1;
      nbad++;
    } else if(ftest[ibx+iby*nSubPixels]>tot_eta*2.){
      etah[ii]=2;
      nbad++;
    } else
      etah[ii]=0;
  }
  sprintf(tit,"/scratch/eta_bad_%d.tiff",ind);
  WriteToTiff(etah, tit, etabins, etabins);
  // cout << "Index: " << ind << "\t Bad bins: "<< nbad << endl;
  //int ibx=0, iby=0;

  delete [] ftest;
  delete [] etah;

  }


 protected:
  

  double calcDiff(double avg, float *hx, float *hy) {
    //double p_tot=0;
    double diff=0, d;
    double bsize=1./nSubPixels;
    int nbad=0;
    double p_tot_x[nSubPixels], p_tot_y[nSubPixels], p_tot[nSubPixels*nSubPixels];
    double maxdiff=0, mindiff=avg*nSubPixels*nSubPixels;
   
    int ipx, ipy;
    for (ipy=0; ipy<nSubPixels; ipy++) {
      for (ipx=0; ipx<nSubPixels; ipx++) {
	p_tot[ipx+ipy*nSubPixels]=0;
      }
      p_tot_y[ipy]=0;
      p_tot_x[ipy]=0;
    }
   
    for (int ibx=0; ibx<nbeta; ibx++) {
      for (int iby=0; iby<nbeta; iby++) {
	ipx=hx[ibx+iby*nbeta]*nSubPixels;
	if (ipx<0) ipx=0;
	if (ipx>=nSubPixels) ipx=nSubPixels-1;
	
	ipy=hy[ibx+iby*nbeta]*nSubPixels;
	if (ipy<0) ipy=0;
	if (ipy>=nSubPixels) ipy=nSubPixels-1;
	
	p_tot[ipx+ipy*nSubPixels]+=heta[ibx+iby*nbeta];
	p_tot_y[ipy]+=heta[ibx+iby*nbeta];
	p_tot_x[ipx]+=heta[ibx+iby*nbeta];
	

      }
    }
    
    
    //  cout << endl << endl;
    for (ipy=0; ipy<nSubPixels; ipy++) { 
     cout.width(5);
     //flat_y[ipy]=p_tot_y[ipy];//avg/nSubPixels;
      for (ipx=0; ipx<nSubPixels; ipx++) {

	//	flat_x[ipx]=p_tot_x[ipx];///avg/nSubPixels;
	flat[ipx+nSubPixels*ipy]=p_tot[ipx+nSubPixels*ipy];///avg;
	d=p_tot[ipx+nSubPixels*ipy]-avg;
	if (d<0) d*=-1.;
	if (d>5*sqrt(avg) )
	  nbad++;
	diff+=d*d;
	if (d<mindiff) mindiff=d;
	if (d>maxdiff) maxdiff=d;
	//	cout << setprecision(4) << p_tot[ipx+nSubPixels*ipy] << "   ";
      }

      /* cout << "** "  << setprecision(4) <<  flat_y[ipy]; */
      //cout << "\n";
    }
    /* cout << "**" << endl; cout.width(5); */
    /* for (ipx=0; ipx<nSubPixels; ipx++) { */
    /*   cout  << setprecision(4) <<  flat_x[ipx] << " "; */
    /* } */
    //cout << "**" << endl; cout.width(5);
    //cout << "Min diff: " << mindiff/sqrt(avg) << " Max diff: " << maxdiff/sqrt(avg) << " Nbad: " << nbad << endl;
    


    //   cout << "Bad pixels: " << 100.*(float)nbad/((float)(nSubPixels*nSubPixels)) << " %" << endl;
    return sqrt(diff);
  }
  
  int *heta;
  float *hhx;
  float *hhy;
  int nbeta;
  double etamin, etamax, etastep;
  double rangeMin, rangeMax;



  double *flat;
  int *hintcorr;
  

};

#endif
