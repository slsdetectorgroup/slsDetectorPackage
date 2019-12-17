#ifndef ETA_INTERPOLATION_BASE_H
#define ETA_INTERPOLATION_BASE_H

#ifdef MYROOT1
#include <TObject.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH2F.h>
#endif
#include <cmath> 
#include "slsInterpolation.h"
#include "tiffIO.h"

class etaInterpolationBase : public slsInterpolation {

 protected:
  float *hhx;
  float *hhy;
  int *heta;
  int nbetaX, nbetaY;
  double etamin, etamax, etastepX, etastepY;
  double rangeMin, rangeMax;
  
  
  
  double *flat;
  int *hintcorr;
  
 public:
 
 etaInterpolationBase(int nx=400, int ny=400, int ns=25, int nsy=25, int nb=-1, int nby=-1, double emin=1, double emax=0) : slsInterpolation(nx,ny,ns,nsy), hhx(NULL), hhy(NULL), heta(NULL), nbetaX(nb), nbetaY(nby), etamin(emin), etamax(emax) {
    // cout << "eb " << nb << " " << emin << " " << emax << endl;  
    // cout << nb << " " << etamin << " " << etamax << endl;
    if (nbetaX<=0) {
      //cout << "aaa:" <<endl;
      nbetaX=nSubPixelsX*10;
    } 
    if (nbetaY<=0) {
      //cout << "aaa:" <<endl;
      nbetaY=nSubPixelsY*10;
    }
    if (etamin>=etamax) {
      etamin=-1;
      etamax=2;
    }
    etastepX=(etamax-etamin)/nbetaX;
    etastepY=(etamax-etamin)/nbetaY;
    heta=new int[nbetaX*nbetaY];
    hhx=new float[nbetaX*nbetaY];
    hhy=new float[nbetaX*nbetaY];
    rangeMin=etamin;
    rangeMax=etamax;
    flat= new double[nSubPixelsX*nSubPixelsY];
    hintcorr=new int [nSubPixelsX*nSubPixelsY*nPixelsX*nPixelsY];
    
  };
  
 etaInterpolationBase(etaInterpolationBase *orig): slsInterpolation(orig){
   nbetaX=orig->nbetaX;
   nbetaY=orig->nbetaY;
   etamin=orig->etamin;
   etamax=orig->etamax;
   rangeMin=orig->rangeMin;
   rangeMax=orig->rangeMax;


    etastepX=(etamax-etamin)/nbetaX;
    etastepY=(etamax-etamin)/nbetaY;
    heta=new int[nbetaX*nbetaY];
    memcpy(heta,orig->heta,nbetaX*nbetaY*sizeof(int));
    hhx=new float[nbetaX*nbetaY];
    memcpy(hhx,orig->hhx,nbetaX*nbetaY*sizeof(float));
    hhy=new float[nbetaX*nbetaY];
    memcpy(hhy,orig->hhy,nbetaX*nbetaY*sizeof(float));
    hintcorr=new int [nSubPixelsX*nSubPixelsY*nPixelsX*nPixelsY];

 };

  



  virtual void resetFlatField() {
    for (int ibx=0; ibx<nbetaX*nbetaY; ibx++) {
      heta[ibx]=0;
      hhx[ibx]=0;
      hhy[ibx]=0;
    }


  };
    
  int *setEta(int *h, int nb=-1, int nby=-1, double emin=1, double emax=0)
  {  
    if (h) {
      if (heta) delete [] heta;
      heta=h;
      nbetaX=nb;
      nbetaY=nby;
      if (nbetaX<=0) nbetaX=nSubPixelsX*10;
      if (nbetaY<=0) nbetaY=nSubPixelsY*10;
      
      etamin=emin;
      etamax=emax;
      if (etamin>=etamax) {
	etamin=-1;
	etamax=2;
      }
      rangeMin=etamin;
      rangeMax=etamax;
      etastepX=(etamax-etamin)/nbetaX;
      etastepY=(etamax-etamin)/nbetaY;
    }
    return heta;
  };
  
   int *setFlatField(int *h, int nb=-1, int nby=-1, double emin=1, double emax=0)
  {  
    return setEta(h, nb, nby, emin, emax);
  };



  int *getFlatField(){return setEta(NULL);};
  
  int *getFlatField(int &nb, int &nby, double &emin, double &emax){
    nb=nbetaX; 
    nby=nbetaY; 
    emin=etamin; 
    emax=etamax; 
    return getFlatField();
  }; 
  
  
  void *writeFlatField(const char * imgname) {
    float *gm=NULL;
    gm=new float[nbetaX*nbetaY];
    for (int ix=0; ix<nbetaX; ix++) {
      for (int iy=0; iy<nbetaY; iy++) {
	gm[iy*nbetaX+ix]=heta[iy*nbetaX+ix];
      }
    } 
    WriteToTiff(gm, imgname, nbetaX, nbetaY);   
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
    
    etastepX=(etamax-etamin)/nbetaX;
    etastepY=(etamax-etamin)/nbetaY;
    uint32 nnx;
    uint32 nny;
    float *gm=ReadFromTiff(imgname, nnx, nny);
    /* if (nnx!=nny) { */
    /*   cout << "different number of bins in x " << nnx << "  and y " << nny<< " !"<< endl; */
    /*   cout << "Aborting read"<< endl; */
    /*   return 0; */
    /* } */
    nbetaX=nnx;
    nbetaY=nny;
    if (gm) {
      if (heta) {
	delete [] heta;
	delete [] hhx;
	delete [] hhy;
      }
      
      heta=new int[nbetaX*nbetaY];
      hhx=new float[nbetaX*nbetaY];
      hhy=new float[nbetaX*nbetaY];
      
      for (int ix=0; ix<nbetaX; ix++) {
	for (int iy=0; iy<nbetaY; iy++) {
	  heta[iy*nbetaX+ix]=gm[iy*nbetaX+ix];
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


  
  
  void debugSaveAll(int ind=0) {
    int  ibx, iby;
   char tit[10000];
   
   float tot_eta=0;

  float *etah=new float[nbetaX*nbetaY];
  // int etabins=nbeta;
  int ibb=0;

  for (int ii=0; ii<nbetaX*nbetaY; ii++) {
   
      etah[ii]=heta[ii];
      tot_eta+=heta[ii];
  }
  sprintf(tit,"/scratch/eta_%d.tiff",ind);
  WriteToTiff(etah, tit, nbetaX, nbetaY);


  for (int ii=0; ii<nbetaX*nbetaY; ii++) {
    //ibb=hhx[ii];//*nSubPixelsX);
    etah[ii]=hhx[ii];
  }
  sprintf(tit,"/scratch/eta_hhx_%d.tiff",ind);
  WriteToTiff(etah, tit, nbetaX, nbetaY);
	  
  for (int ii=0; ii<nbetaX*nbetaY; ii++) {
    //ibb=hhy[ii];//*nSubPixelsY;
    etah[ii]=hhy[ii];
  }
  sprintf(tit,"/scratch/eta_hhy_%d.tiff",ind);
  WriteToTiff(etah, tit, nbetaX, nbetaY);
	  
  
  float *ftest=new float[nSubPixelsX*nSubPixelsY];

  for (int ib=0; ib<nSubPixelsX*nSubPixelsY; ib++) ftest[ib]=0;
  

  //int ibx=0, iby=0;
  
  for (int ii=0; ii<nbetaX*nbetaY; ii++) {
    
    ibx=nSubPixelsX*hhx[ii];
    iby=nSubPixelsY*hhy[ii];
    if (ibx<0) ibx=0;
    if (iby<0) iby=0;
    if (ibx>=nSubPixelsX) ibx=nSubPixelsX-1;
    if (iby>=nSubPixelsY) iby=nSubPixelsY-1;
    

    if (ibx>=0 && ibx<nSubPixelsX && iby>=0 && iby<nSubPixelsY) {
      //
      // if (ibx>0 && iby>0) cout << ibx << " " << iby << " " << ii << endl;
      ftest[ibx+iby*nSubPixelsX]+=heta[ii];
    } else
      cout << "Bad interpolation "<< ii << " " << ibx << " " << iby<< endl; 
    
  }

  sprintf(tit,"/scratch/ftest_%d.tiff",ind);
  WriteToTiff(ftest, tit, nSubPixelsX, nSubPixelsY);

  //int ibx=0, iby=0;
  tot_eta/=nSubPixelsX*nSubPixelsY;
  int nbad=0;
  for (int ii=0; ii<nbetaX*nbetaY; ii++) {
    ibx=nSubPixelsX*hhx[ii];
    iby=nSubPixelsY*hhy[ii];
    if (ftest[ibx+iby*nSubPixelsX]<tot_eta*0.5) {
      etah[ii]=1;
      nbad++;
    } else if(ftest[ibx+iby*nSubPixelsX]>tot_eta*2.){
      etah[ii]=2;
      nbad++;
    } else
      etah[ii]=0;
  }
  sprintf(tit,"/scratch/eta_bad_%d.tiff",ind);
  WriteToTiff(etah, tit, nbetaX, nbetaY);
  // cout << "Index: " << ind << "\t Bad bins: "<< nbad << endl;
  //int ibx=0, iby=0;

  delete [] ftest;
  delete [] etah;

  }

  virtual int *getInterpolatedImage(){ 
  
   /* int ipx, ipy; */
   /* // cout << "ff" << endl; */
   /* calcDiff(1, hhx, hhy); //get flat */
   /* double avg=0; */
   /* for (ipx=0; ipx<nSubPixelsX; ipx++) */
   /*   for (ipy=0; ipy<nSubPixelsY; ipy++) */
   /*     avg+=flat[ipx+ipy*nSubPixelsX]; */
   /* avg/=nSubPixelsY*nSubPixelsX; */

   /* for (int ibx=0 ; ibx<nSubPixelsX*nPixelsX; ibx++) { */
   /*   ipx=ibx%nSubPixelsX-nSubPixelsX/2; */
   /*     if (ipx<0) ipx=nSubPixelsX+ipx; */
   /*   for (int iby=0 ; iby<nSubPixelsY*nPixelsY; iby++) { */
   /*     ipy=iby%nSubPixelsY-nSubPixelsY/2; */
   /*   if (ipy<0) ipy=nSubPixelsY+ipy; */
   /*   // cout << ipx << " " << ipy << " " << ibx << " " << iby << endl; */
   /*   if (flat[ipx+ipy*nSubPixelsX]>0) */
   /*     hintcorr[ibx+iby*nSubPixelsX*nPixelsX]=hint[ibx+iby*nSubPixelsX*nPixelsX]*(avg/flat[ipx+ipy*nSubPixelsX]); */
   /*   else */
   /*     hintcorr[ibx+iby*nSubPixelsX*nPixelsX]=hint[ibx+iby*nSubPixelsX*nPixelsX]; */
   /*   } */
   /* } */


      return hint;
  /*   return hintcorr; */
   }; 



 protected:
  

  double calcDiff(double avg, float *hx, float *hy) {
    //double p_tot=0;
    double diff=0, d;
    //double bsize=1./nSubPixels;
    int nbad=0;
    double p_tot_x[nSubPixelsX], p_tot_y[nSubPixelsY], p_tot[nSubPixelsX*nSubPixelsY];
    double maxdiff=0, mindiff=avg*nSubPixelsX*nSubPixelsY;
   
    int ipx, ipy;
    for (ipy=0; ipy<nSubPixelsY; ipy++) {
      for (ipx=0; ipx<nSubPixelsX; ipx++) {
	p_tot[ipx+ipy*nSubPixelsX]=0;
      }
      p_tot_y[ipy]=0;
      p_tot_x[ipy]=0;
    }
   
    for (int ibx=0; ibx<nbetaX; ibx++) {
      for (int iby=0; iby<nbetaY; iby++) {
	ipx=hx[ibx+iby*nbetaX]*nSubPixelsX;
	if (ipx<0) ipx=0;
	if (ipx>=nSubPixelsX) ipx=nSubPixelsX-1;
	
	ipy=hy[ibx+iby*nbetaX]*nSubPixelsY;
	if (ipy<0) ipy=0;
	if (ipy>=nSubPixelsY) ipy=nSubPixelsY-1;
	
	p_tot[ipx+ipy*nSubPixelsX]+=heta[ibx+iby*nbetaX];
	p_tot_y[ipy]+=heta[ibx+iby*nbetaX];
	p_tot_x[ipx]+=heta[ibx+iby*nbetaX];
      }
    }
    
    
    //  cout << endl << endl;
    for (ipy=0; ipy<nSubPixelsY; ipy++) { 
     cout.width(5);
     //flat_y[ipy]=p_tot_y[ipy];//avg/nSubPixels;
      for (ipx=0; ipx<nSubPixelsX; ipx++) {

	//	flat_x[ipx]=p_tot_x[ipx];///avg/nSubPixels;
	flat[ipx+nSubPixelsX*ipy]=p_tot[ipx+nSubPixelsX*ipy];///avg;
	d=p_tot[ipx+nSubPixelsX*ipy]-avg;
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
  

};




class eta2InterpolationBase : public virtual etaInterpolationBase {
  
 public:
 eta2InterpolationBase(int nx=400, int ny=400, int ns=25, int nsy=25, int nb=-1, int nby=-1, double emin=1, double emax=0) :  etaInterpolationBase(nx,ny, ns, nsy, nb, nby, emin, emax) {
    
   
    
  };
  
 eta2InterpolationBase(eta2InterpolationBase *orig): etaInterpolationBase(orig){ };

 
  //////////////////////////////////////////////////////////////////////////////
  //////////// /*It return position hit for the event in input */ //////////////
  virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x, double &int_y)
  {
    double sDum[2][2];
    double tot, totquad;
    double etax=0,etay=0;
    
    int corner;
    corner=calcQuad(data, tot, totquad, sDum); 
    if (nSubPixelsX>2 || nSubPixelsY>2) 
      calcEta(totquad, sDum, etax, etay); 
    getInterpolatedPosition(x,y,etax,etay,corner,int_x,int_y);

    return;
  };
  

  virtual void getInterpolatedPosition(int x, int y, double *data, double &int_x, double &int_y)
  {
    double sDum[2][2];
    double tot, totquad;
    double etax=0,etay=0;
    
    int corner;
    corner=calcQuad(data, tot, totquad, sDum); 
    if (nSubPixelsX>2 || nSubPixelsY>2  ) 
      calcEta(totquad, sDum, etax, etay); 
    getInterpolatedPosition(x,y,etax,etay,corner,int_x,int_y);

    return;
  };
  
  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,double *cl,double &int_x, double &int_y) {
    
     double cc[2][2];
     int xoff=0, yoff=0;
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
     double etax=0, etay=0;
     if (nSubPixelsX>2 || nSubPixelsY>2) { 
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
     int xoff=0, yoff=0;
     
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
     double etax=0, etay=0;
     if (nSubPixelsX>2 || nSubPixelsY>2) { 
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
    

     if (nSubPixelsX>2 || nSubPixelsY>2 ) { 

       ex=(etax-etamin)/etastepX;
       ey=(etay-etamin)/etastepY;
       if (ex<0) {
	 cout << "x*"<< ex << endl;
	 ex=0;
       } 
       if (ex>=nbetaX) {
	 cout << "x?"<< ex << endl;
	 ex=nbetaX-1;
       }
       if (ey<0) {
	 cout << "y*"<< ey << " " << nbetaY << endl;
	 ey=0;
       } 
       if (ey>=nbetaY) {
	 cout << "y?"<< ey << " " << nbetaY << endl;
	 ey=nbetaY-1;
       }
    
    
   
       xpos_eta=(((double)hhx[(ey*nbetaX+ex)]));
       ypos_eta=(((double)hhy[(ey*nbetaX+ex)]));
       
       
     if (xpos_eta<0 || xpos_eta>1)
       xpos_eta=-100;
     else
       xpos_eta+=dX ;///((double)nSubPixels);

     if (ypos_eta<0 || ypos_eta>1)
       ypos_eta=-100;
     else
       ypos_eta+=dY ;///((double)nSubPixels);
       
     } else {
       xpos_eta=0.5*dX+0.25;
       ypos_eta=0.5*dY+0.25;
     }
     
  
     if (xpos_eta<-1)
       int_x=-100;
     else
       int_x=((double)x) + xpos_eta+0.5;

     if (ypos_eta<-1)
       int_y=-100;
     else 
       int_y=((double)y) +  ypos_eta+0.5;
           
    
     

  }
  

  
  virtual int addToFlatField(double totquad,int quad,int *cl,double &etax, double &etay) {
     double cc[2][2];
     int xoff=0, yoff=0;
     
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
     int xoff=0, yoff=0;
     
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
    // int corner;
    //corner=
    calcQuad(cluster, tot, totquad, sDum); 
    
    //double xpos_eta,ypos_eta;
    //double dX,dY;
  
    
    calcEta(totquad, sDum, etax, etay); 
   
    return addToFlatField(etax,etay);

    };

  virtual int addToFlatField(int *cluster, double &etax, double &etay){
    double sDum[2][2];
    double tot, totquad;
    //int corner;
    //corner=
    calcQuad(cluster, tot, totquad, sDum); 
    
    // double xpos_eta,ypos_eta;
    //double dX,dY;
  
    
    calcEta(totquad, sDum, etax, etay); 
   
    return addToFlatField(etax,etay);

    };


  virtual int addToFlatField(double etax, double etay){
#ifdef MYROOT1
    heta->Fill(etax,etay);
#endif
#ifndef MYROOT1
    int ex,ey;
    ex=(etax-etamin)/etastepX;
    ey=(etay-etamin)/etastepY;
    if (ey<nbetaY && ex<nbetaX && ex>=0 && ey>=0)
      heta[ey*nbetaX+ex]++;
#endif
    return 0;
  };

};

















class eta3InterpolationBase : public virtual etaInterpolationBase  {
  
 public:
 eta3InterpolationBase(int nx=400, int ny=400, int ns=25, int nsy=25, int nb=-1, int nby=-1, double emin=1, double emax=0) : etaInterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax) {
    cout << "e3ib " << nb << " " << emin << " " << emax << endl; 
  
};
  
 eta3InterpolationBase(eta3InterpolationBase *orig): etaInterpolationBase(orig){ };


  //////////////////////////////////////////////////////////////////////////////
  //////////// /*It return position hit for the event in input */ //////////////
  virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x, double &int_y)
  {
   double tot;
    double etax,etay;
    
    int corner=calcEta3(data,etax,etay, tot);
    
    getInterpolatedPosition(x,y,etax,etay,corner,int_x,int_y);

    return;
  };
  

  virtual void getInterpolatedPosition(int x, int y, double *data, double &int_x, double &int_y)
  {
    //double sDum[2][2];
    double tot;
    double etax,etay;
    
    int corner=calcEta3(data,etax,etay, tot);
    
    getInterpolatedPosition(x,y,etax,etay,corner,int_x,int_y);

    return;
  };
  

  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,double *cl,double &int_x, double &int_y) {
    
  
    double etax, etay;
     if (nSubPixelsX>2 || nSubPixelsY>2 ) { 
       calcEta3(cl,etax,etay, totquad);
     } 
     return getInterpolatedPosition(x,y,etax, etay,quad,int_x,int_y);

  }



  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,int *cl,double &int_x, double &int_y) {
    
    
     double etax, etay;
     if (nSubPixelsX>2 || nSubPixelsY>2 ) { 
       calcEta3(cl,etax,etay, totquad);
     }
     return getInterpolatedPosition(x,y,etax, etay,quad,int_x,int_y);

  }



  virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int corner, double &int_x, double &int_y)
  {


    //cout << "**";
    double xpos_eta=0,ypos_eta=0;
    int ex,ey;

     if (nSubPixelsX>2 || nSubPixelsY>2 ) { 

#ifdef MYROOT1
    xpos_eta=(hhx->GetBinContent(hhx->GetXaxis()->FindBin(etax),hhy->GetYaxis()->FindBin(etay)))/((double)nSubPixelsX);
    ypos_eta=(hhy->GetBinContent(hhx->GetXaxis()->FindBin(etax),hhy->GetYaxis()->FindBin(etay)))/((double)nSubPixelsY);
#endif
#ifndef MYROOT1
    ex=(etax-etamin)/etastepX;
    ey=(etay-etamin)/etastepY;
    if (ex<0) {
      /* cout << etax << " " << etamin << " "; */
      /*  cout << "3x*"<< ex << endl; */
      ex=0;
	} 
    if (ex>=nbetaX) {
      /* cout << etax << " " << etamin << " "; */
      /* cout << "3x?"<< ex << endl; */
      ex=nbetaX-1;
    }
    if (ey<0) {
      /* cout << etay << " " << etamin << " "; */
      /* cout << "3y*"<< ey << endl; */
      ey=0;
	} 
    if (ey>=nbetaY) {
      /* cout << etay << " " << etamin << " "; */
      /* cout << "3y?"<< ey << endl; */
      ey=nbetaY-1;
      
    }
    xpos_eta=(((double)hhx[(ey*nbetaX+ex)]));///((double)nSubPixels);
    ypos_eta=(((double)hhy[(ey*nbetaX+ex)]));///((double)nSubPixels);

#endif
   
     } else {
       switch (corner) {
       case BOTTOM_LEFT:
	 xpos_eta=-0.25;
	 ypos_eta=-0.25;
	 break;
       case BOTTOM_RIGHT:
	 xpos_eta=0.25;
	 ypos_eta=-0.25;
	 break;
       case TOP_LEFT:
	 xpos_eta=-0.25;
	 ypos_eta=0.25;
	 break;
       case TOP_RIGHT:
	 xpos_eta=0.25;
	 ypos_eta=0.25;
	 break;
       default:
	 xpos_eta=0;
	 ypos_eta=0;
       } 
       
     }
     /* if (xpos_eta<0 || xpos_eta>1) */
     /*   int_x=-1; */
     /* else */
       int_x=((double)x) + xpos_eta;
     /* if (ypos_eta<0 || ypos_eta>1) */
     /*   int_y=-1; */
     /* else */
       int_y=((double)y) + ypos_eta;
     // int_x=5. + xpos_eta;
     //  int_y=5. + ypos_eta;
    

  }

  
   virtual int addToFlatField(double totquad,int quad,int *cl,double &etax, double &etay) {
   
     calcEta3(cl, etax, etay, totquad);
     return addToFlatField(etax,etay);
   }

   virtual int addToFlatField(double totquad,int quad,double *cl,double &etax, double &etay) {
    

     calcEta3(cl, etax, etay, totquad);
     return addToFlatField(etax,etay);
   }



  //////////////////////////////////////////////////////////////////////////////////////
  virtual int addToFlatField(double *cluster, double &etax, double &etay){
    double totquad;
     calcEta3(cluster, etax, etay, totquad);
    return addToFlatField(etax,etay);

    };

  virtual int addToFlatField(int *cluster, double &etax, double &etay){
  
    double totquad;
   
     calcEta3(cluster, etax, etay, totquad);
    return addToFlatField(etax,etay);

    };

  

  virtual int addToFlatField(double etax, double etay){
#ifdef MYROOT1
    heta->Fill(etax,etay);
#endif
#ifndef MYROOT1
    int ex,ey;
    ex=(etax-etamin)/etastepX;
    ey=(etay-etamin)/etastepY;
    if (ey<nbetaY && ex<nbetaX && ex>=0 && ey>=0)
      heta[ey*nbetaX+ex]++;
#endif
    return 0;
  };
  
  
};

#endif
