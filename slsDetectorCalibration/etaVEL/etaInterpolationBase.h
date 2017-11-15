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
 etaInterpolationBase(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : slsInterpolation(nx,ny,ns), hhx(NULL), hhy(NULL), heta(NULL),nbeta(nb),etamin(emin), etamax(emax) {
    if (nb<=0) 
      nbeta=nSubPixels*10;   
    if (etamin>=etamax) {
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
    hhx=new int[nbeta*nbeta];
    hhy=new int[nbeta*nbeta];
    
#endif
    
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
    hhx=new int[nbeta*nbeta];
    memcpy(hhx,orig->hhx,nbeta*nbeta*sizeof(int));
    hhy=new int[nbeta*nbeta];
    memcpy(hhy,orig->hhy,nbeta*nbeta*sizeof(int));
    
#endif
    

 };
  virtual etaInterpolationBase* Clone() {

    return new etaInterpolationBase(this);

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
   TH2D *setFlatField(TH2D *h, int nb=-1, double emin=1, double emax=0)
  {  
    return setEta(h, nb, emin, emax);
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
      return NULL;
    }
    nbeta=nnx;
    if (gm) {
      if (heta) {
	delete [] heta;
	delete [] hhx;
	delete [] hhy;
      }
      
      heta=new int[nbeta*nbeta];
      hhx=new int[nbeta*nbeta];
      hhy=new int[nbeta*nbeta];
      
      for (int ix=0; ix<nbeta; ix++) {
	for (int iy=0; iy<nbeta; iy++) {
	  heta[iy*nbeta+ix]=gm[iy*nbeta+ix];
	}
      }
      delete [] gm;
      return 1;
    }
    return NULL;
  };
  
    


#endif
  

  
  virtual void prepareInterpolation(int &ok){};
 
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
    if (nSubPixels>2) 
      calcEta(totquad, sDum, etax, etay); 
      getInterpolatedPosition(x,y,etax,etay,corner,int_x,int_y);

    return;
  };
  
  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,double *cl,double &int_x, double &int_y) {
    
     double cc[2][2];
     double *cluster[3];
     int xoff, yoff;
     cluster[0]=cl;
     cluster[1]=cl+3;
     cluster[2]=cl+6;
     
     switch (quad) {
     case BOTTOM_LEFT:
       xoff=0;
       yoff=0;
       break;
     case BOTTOM_RIGHT:
       xoff=1;
       yoff=0;
       break;
     case TOP_LEFT:
       xoff=0;
       yoff=1;
       break;
     case TOP_RIGHT:
       xoff=1;
       yoff=1;
       break;
     default:
       ;
     } 
     double etax, etay;
     if (nSubPixels>2) { 
       cc[0][0]=cluster[yoff][xoff];
       cc[1][0]=cluster[yoff+1][xoff];
       cc[0][1]=cluster[yoff][xoff+1];
       cc[1][1]=cluster[yoff+1][xoff+1];
       calcEta(totquad,cc,etax,etay);
     }
     return getInterpolatedPosition(x,y,etax, etay,quad,int_x,int_y);

  }






  virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int corner, double &int_x, double &int_y)
  {


    double xpos_eta=0,ypos_eta=0;
    double dX,dY;
    int ex,ey;
    switch (corner)
      {
      case TOP_LEFT:
	dX=-.99; 
	dY=+.99; 
	break;
      case TOP_RIGHT:
	dX=+.99; 
	dY=+.99; 
	break;
      case BOTTOM_LEFT:
	dX=-.99; 
	dY=-.99; 
	break;
      case BOTTOM_RIGHT:
	dX=+.99; 
	dY=-.99; 
	break;
      default:
	cout << "bad quadrant" << endl;
	dX=0.; 
	dY=0.;
      }
    

     if (nSubPixels>2) { 

#ifdef MYROOT1
    xpos_eta=(hhx->GetBinContent(hhx->GetXaxis()->FindBin(etax),hhy->GetYaxis()->FindBin(etay)))/((double)nSubPixels);
    ypos_eta=(hhy->GetBinContent(hhx->GetXaxis()->FindBin(etax),hhy->GetYaxis()->FindBin(etay)))/((double)nSubPixels);
#endif
#ifndef MYROOT1
    ex=(etax-etamin)/etastep;
    ey=(etay-etamin)/etastep;
    if (ex<0) ex=0;
    if (ex>=nbeta) ex=nbeta-1;
    if (ey<0) ey=0;
    if (ey>=nbeta) ey=nbeta-1;
    
   
    xpos_eta=(((double)hhx[(ey*nbeta+ex)]))/((double)nSubPixels);
    ypos_eta=(((double)hhy[(ey*nbeta+ex)]))/((double)nSubPixels);
      //else
      //return 0;

#endif
     } else {
       xpos_eta=-dX*0.25;
       ypos_eta=-0.25*dY;
     }
       
    int_x=((double)x) + 0.5*dX + xpos_eta;
    int_y=((double)y) + 0.5*dY + ypos_eta;
    // cout << "***"<< x <<" " << y << " " << int_x << " " << int_y << endl;
    //   cout << etax << " " << ex << " " << etay << " " << ey << " " << xpos_eta << " " << int_x << " " << ypos_eta << " " << int_y << endl;
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
    if (ex>=nbeta) ex=nbeta-1;
    if (ey<0) ey=0;
    if (ey>=nbeta) ey=nbeta-1;

    xpos_eta=(((double)hhx[(int)(ey*nbeta+ex)]))/((double)nSubPixels);
    ypos_eta=(((double)hhy[(int)(ey*nbeta+ex)]))/((double)nSubPixels);
#endif
    
    int_x=((double)x) + xpos_eta;
    int_y=((double)y) + ypos_eta;
    
    return;
  };
  
   virtual int addToFlatField(double totquad,int quad,double *cl,double &etax, double &etay) {
     double cc[2][2];
     double *cluster[3];
     int xoff, yoff;
     cluster[0]=cl;
     cluster[1]=cl+3;
     cluster[2]=cl+6;
     
     switch (quad) {
     case BOTTOM_LEFT:
       xoff=0;
       yoff=0;
       break;
     case BOTTOM_RIGHT:
       xoff=1;
       yoff=0;
       break;
     case TOP_LEFT:
       xoff=0;
       yoff=1;
       break;
     case TOP_RIGHT:
       xoff=1;
       yoff=1;
       break;
     default:
       ;
     } 
     cc[0][0]=cluster[yoff][xoff];
     cc[1][0]=cluster[yoff+1][xoff];
     cc[0][1]=cluster[yoff][xoff+1];
     cc[1][1]=cluster[yoff+1][xoff+1];
     
      /* cout << cl[0] << " " << cl[1] << " " << cl[2] << endl;   */
      /* cout << cl[3] << " " << cl[4] << " " << cl[5] << endl;   */
      /* cout << cl[6] << " " << cl[7] << " " << cl[8] << endl;   */
      /* cout <<"******"<<totquad << " " << quad << endl;  */
      /* cout << cc[0][0]<< " " << cc[0][1] << endl;  */
      /* cout << cc[1][0]<< " " << cc[1][1] << endl;  */
     //calcMyEta(totquad,quad,cl,etax, etay);
     calcEta(totquad, cc,etax, etay);

     //     cout <<"******"<< etax << " " << etay << endl;


     return addToFlatField(etax,etay);
   }



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
    // cout << etax << " " << ex << " " << etay << " " << ey << " " << ey*nbeta+ex << endl;
    if (ey<nbeta && ex<nbeta && ex>=0 && ey>=0)
      heta[ey*nbeta+ex]++; 
    // cout << "*"<< etax << " " << etay << endl;
    /* cout << etax << " " << etay << " " << ex << " " << ey << " " << ey*nbeta+ex << endl; */
    /* cout <<"********"<< endl << endl ; */
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
