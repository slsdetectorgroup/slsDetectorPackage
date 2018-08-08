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
      cout << "aaa:" <<endl;
      nbeta=nSubPixels*10;
    }
    if (etamin>=etamax) {
      cout << "aaa:" <<endl;
      etamin=-1;
      etamax=2;
    }
    etastep=(etamax-etamin)/nbeta;
#ifdef MYROOT1
    heta=new TH2D("heta","heta",nbeta,etamin,etamax,nbeta,etamin,etamax);
    hhx=new TH2D("hhx","hhx",nbeta,etamin,etamax,nbeta,etamin,etamax);
    hhy=new TH2D("hhy","hhy",nbeta,etamin,etamax,nbeta,etamin,etamax);
#endif
#ifndef MYROOT1
    heta=new int[nbeta*nbeta];
    hhx=new float[nbeta*nbeta];
    hhy=new float[nbeta*nbeta];
    
#endif
    
    //cout << nbeta << " " << etamin << " " << etamax << endl;
  };
  
 etaInterpolationBase(etaInterpolationBase *orig): slsInterpolation(orig){
   nbeta=orig->nbeta;
   etamin=orig->etamin;
   etamax=orig->etamax;

    etastep=(etamax-etamin)/nbeta;
#ifdef MYROOT1
    heta=(TH2D*)(orig->heta)->Clone("heta");
    hhx=(TH2D*)(orig->hhx)->Clone("hhx");
    hhy=(TH2D*)(orig->hhy)->Clone("hhy");
#endif

#ifndef MYROOT1
    heta=new int[nbeta*nbeta];
    memcpy(heta,orig->heta,nbeta*nbeta*sizeof(int));
    hhx=new float[nbeta*nbeta];
    memcpy(hhx,orig->hhx,nbeta*nbeta*sizeof(float));
    hhy=new float[nbeta*nbeta];
    memcpy(hhy,orig->hhy,nbeta*nbeta*sizeof(float));
    
#endif
    

 };

  virtual etaInterpolationBase* Clone()=0;/*{
    return new etaInterpolationBase(this);
  };
					  */



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
   TH2D *setFlatField(TH2D *h, int nb=-1, double emin=1, double emax=0)
  {  
    return setEta(h, nb, emin, emax);
  };
  
  TH2D *getFlatField(){return setEta(NULL);};
#endif
  
#ifndef MYROOT1
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
    //cout << "igff* ff has " << nb << " bins " << endl; 
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
  
    


#endif
  

 
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
#endif
    
  //////////////////////////////////////////////////////////////////////////////
  //////////// /*It return position hit for the event in input */ //////////////
  /* virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x, double &int_y)=0; */
  /* virtual void getInterpolatedPosition(int x, int y, double *data, double &int_x, double &int_y)=0; */
  /* virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,double *cl,double &int_x, double &int_y)=0; */
  /* virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,int *cl,double &int_x, double &int_y)=0;  */
  /* virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int corner, double &int_x, double &int_y)=0; */


  /* virtual int addToFlatField(double totquad,int quad,int *cl,double &etax, double &etay)=0; */
  /* virtual int addToFlatField(double totquad,int quad,double *cl,double &etax, double &etay)=0; */
  /* virtual int addToFlatField(double *cluster, double &etax, double &etay)=0; */
  /* virtual int addToFlatField(int *cluster, double &etax, double &etay)=0; */
  
  virtual int addToFlatField(double etax, double etay){
#ifdef MYROOT1
    heta->Fill(etax,etay);
#endif
#ifndef MYROOT1
    int ex,ey; 
    ex=(etax-etamin)/etastep;
    ey=(etay-etamin)/etastep;
    if (ey<nbeta && ex<nbeta && ex>=0 && ey>=0)
      heta[ey*nbeta+ex]++; 
#endif
    return 0;    
  };
  
  
  // virtual void prepareInterpolation(int &ok)=0;

 protected:
  
#ifdef MYROOT1
  TH2D *heta;
  TH2D *hhx;
  TH2D *hhy;
#endif
#ifndef MYROOT1
  int *heta;
  float *hhx;
  float *hhy;
#endif
  int nbeta;
  double etamin, etamax, etastep;

};

#endif
