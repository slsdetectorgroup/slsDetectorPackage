#ifndef ETA2_INTERPOLATION_BASE_H
#define ETA2_INTERPOLATION_BASE_H

#ifdef MYROOT1
#include <TObject.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH2F.h>
#endif

#include "etaInterpolationBase.h"

class eta2InterpolationBase : public virtual etaInterpolationBase {
  
 public:
 eta2InterpolationBase(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) :  etaInterpolationBase(nx,ny, ns, nb, emin, emax) {
    
    /* if (etamin>=etamax) { */
    /*   etamin=-1; */
    /*   etamax=2;  */
    /*   //  cout << ":" <<endl; */
    /* } */
    /* etastep=(etamax-etamin)/nbeta; */
    
  };
  
 eta2InterpolationBase(eta2InterpolationBase *orig): etaInterpolationBase(orig){ };

 
  //////////////////////////////////////////////////////////////////////////////
  //////////// /*It return position hit for the event in input */ //////////////
  virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x, double &int_y)
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
     int xoff, yoff;
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
       cc[0][0]=cl[xoff+3*yoff];
       cc[1][0]=cl[xoff+3*(yoff+1)];
       cc[0][1]=cl[xoff+1+3*yoff];
       cc[1][1]=cl[xoff+1+3*(yoff+1)];
       calcEta(totquad,cc,etax,etay);
     }
     return getInterpolatedPosition(x,y,etax, etay,quad,int_x,int_y);

  }



  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,int *cl,double &int_x, double &int_y) {
    
     double cc[2][2];
     int xoff, yoff;
     
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
       cc[0][0]=cl[xoff+3*yoff];
       cc[1][0]=cl[xoff+3*(yoff+1)];
       cc[0][1]=cl[xoff+1+3*yoff];
       cc[1][1]=cl[xoff+1+3*(xoff+1)];
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
	dX=-1.;
	dY=0;
	break;
      case TOP_RIGHT:
	;
	dX=0;
	dY=0;
	break;
      case BOTTOM_LEFT:
	dX=-1.;
	dY=-1.;
	break;
      case BOTTOM_RIGHT:
	dX=0;
	dY=-1.;
	break;
      default:
	cout << "bad quadrant" << endl;
	dX=0.; 
	dY=0.;
      }
    

     if (nSubPixels>2) { 

       ex=(etax-etamin)/etastep;
       ey=(etay-etamin)/etastep;
       if (ex<0) {
	 cout << "x*"<< ex << endl;
	 ex=0;
       } 
       if (ex>=nbeta) {
	 cout << "x?"<< ex << endl;
	 ex=nbeta-1;
       }
       if (ey<0) {
	 cout << "y*"<< ey << endl;
	 ey=0;
       } 
       if (ey>=nbeta) {
	 cout << "y?"<< ey << endl;
	 ey=nbeta-1;
       }
    
    
   
       xpos_eta=(((double)hhx[(ey*nbeta+ex)]))+dX ;///((double)nSubPixels);
       ypos_eta=(((double)hhy[(ey*nbeta+ex)]))+dY ;///((double)nSubPixels);
       
     } else {
       xpos_eta=0.5*dX+0.25;
       ypos_eta=0.5*dY+0.25;
     }
     
     int_x=((double)x) + xpos_eta+0.5;
     int_y=((double)y) +  ypos_eta+0.5;
     

  }
  

  
  virtual int addToFlatField(double totquad,int quad,int *cl,double &etax, double &etay) {
     double cc[2][2];
     int xoff, yoff;
     
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
     cc[0][0]=cl[xoff+3*yoff];
     cc[1][0]=cl[xoff+3*(yoff+1)];
     cc[0][1]=cl[xoff+1+3*yoff];
     cc[1][1]=cl[xoff+1+3*(yoff+1)];
     
     //calcMyEta(totquad,quad,cl,etax, etay);
     calcEta(totquad, cc,etax, etay);

     //     cout <<"******"<< etax << " " << etay << endl;


     return addToFlatField(etax,etay);
   }

   virtual int addToFlatField(double totquad,int quad,double *cl,double &etax, double &etay) {
     double cc[2][2];
     int xoff, yoff;
     
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
     cc[0][0]=cl[xoff+3*yoff];
     cc[1][0]=cl[(yoff+1)*3+xoff];
     cc[0][1]=cl[yoff*3+xoff+1];
     cc[1][1]=cl[(yoff+1)*3+xoff+1];
     
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

  virtual int addToFlatField(int *cluster, double &etax, double &etay){
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

 virtual int *getInterpolatedImage(){
   int ipx, ipy;
   // cout << "ff" << endl;
   calcDiff(1, hhx, hhy); //get flat
   double avg=0;
   for (ipx=0; ipx<nSubPixels; ipx++)
     for (ipy=0; ipy<nSubPixels; ipy++)
       avg+=flat[ipx+ipy*nSubPixels];
   avg/=nSubPixels*nSubPixels;

   for (int ibx=0 ; ibx<nSubPixels*nPixelsX; ibx++) {
     ipx=ibx%nSubPixels-nSubPixels/2;
       if (ipx<0) ipx=nSubPixels+ipx;
     for (int iby=0 ; iby<nSubPixels*nPixelsY; iby++) {
       ipy=iby%nSubPixels-nSubPixels/2;
     if (ipy<0) ipy=nSubPixels+ipy;
     // cout << ipx << " " << ipy << " " << ibx << " " << iby << endl;
     if (flat[ipx+ipy*nSubPixels]>0)
       hintcorr[ibx+iby*nSubPixels*nPixelsX]=hint[ibx+iby*nSubPixels*nPixelsX]*(avg/flat[ipx+ipy*nSubPixels]);
     else
       hintcorr[ibx+iby*nSubPixels*nPixelsX]=hint[ibx+iby*nSubPixels*nPixelsX];


     }
   }



    return hintcorr;
  };

/*  protected: */
  
/* #ifdef MYROOT1 */
/*   TH2D *heta; */
/*   TH2D *hhx; */
/*   TH2D *hhy; */
/* #endif */
/* #ifndef MYROOT1 */
/*   int *heta; */
/*   float *hhx; */
/*   float *hhy; */
/* #endif */
/*   int nbeta; */
/*   double etamin, etamax, etastep; */

};

#endif
