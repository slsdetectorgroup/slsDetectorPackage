#ifndef ETA_INTERPOLATION_ADAPTIVEBINS_H
#define ETA_INTERPOLATION_ADAPTIVEBINS_H


#include "tiffIO.h"
//#include "etaInterpolationBase.h"
#include "etaInterpolationPosXY.h"

class etaInterpolationAdaptiveBins : public etaInterpolationPosXY {

 private:
  double calcDiff(double avg, float *hx, float *hy) {
    double p_tot=0;
    double diff=0;
    double bsize=1./nSubPixels;
    
    for (int ipx=0; ipx<nSubPixels; ipx++) {
      for (int ipy=0; ipy<nSubPixels; ipy++) {
	p_tot=0;
	for (int ibx=0; ibx<nbeta; ibx++) {
	  for (int iby=0; iby<nbeta; iby++) {
	    if ( hx[ibx+iby*nbeta]>=((ipx)*bsize) && hx[ibx+iby*nbeta]<((ipx+1)*bsize) &&  hy[ibx+iby*nbeta]>=((ipy)*bsize) && hy[ibx+iby*nbeta]<((ipy+1)*bsize)) {
	      p_tot+=heta[ibx+iby*nbeta];
	    } 
	  }
	}
	
	cout <<  p_tot << " \t ";



	diff+=(p_tot-avg)*(p_tot-avg);
	
      }
      cout << "\n";
    }
    return diff;
  }
  
  void iterate(float *newhhx, float *newhhy) {

    double bsize=1./nSubPixels;
    
    double hy[nbeta]; //profile y
    double hx[nbeta]; //profile x
    double hix[nbeta]; //integral of projection x
    double hiy[nbeta]; //integral of projection y
    
   double tot_eta_x=0;
   double tot_eta_y=0;
    for (int ipy=0; ipy<nSubPixels; ipy++) {
     
      for (int ibx=0; ibx<nbeta; ibx++) {
	hx[ibx]=0;
	hy[ibx]=0;
      }

      tot_eta_x=0;
      tot_eta_y=0;
      // cout << ipy << " " << ((ipy)*bsize) << " " << ((ipy+1)*bsize) << endl;
       for (int ibx=0; ibx<nbeta; ibx++) {
	 for (int iby=0; iby<nbeta; iby++) {
	   
	  if (hhy[ibx+iby*nbeta]>=((ipy)*bsize) && hhy[ibx+iby*nbeta]<=((ipy+1)*bsize)) {
	    hx[ibx]+=heta[ibx+iby*nbeta];
	    tot_eta_x+=heta[ibx+iby*nbeta];
	  } 
	  
	  
	  if (hhx[ibx+iby*nbeta]>=((ipy)*bsize) && hhx[ibx+iby*nbeta]<=((ipy+1)*bsize)) {
	    hy[iby]+=heta[ibx+iby*nbeta];
	    tot_eta_y+=heta[ibx+iby*nbeta];
	  } 
	 }
       }
      
 
       hix[0]=hx[0];
       hiy[0]=hy[0];
       for (int ib=1; ib<nbeta; ib++) {
	 hix[ib]=hix[ib-1]+hx[ib];
	 hiy[ib]=hiy[ib-1]+hy[ib];
       }
       // tot_eta_x=hix[nbeta-1];
       // tot_eta_y=hiy[nbeta-1];
       /* cout << "ipx " << ipy << " x: " << tot_eta_x << " " << hix[10]<< " " << hix[nbeta-1] << endl; */
       /* cout << "ipy " << ipy << " y: " << tot_eta_y << " " << hiy[10]<< " " << hiy[nbeta-1] << endl; */
  
  // for (int ipy=0; ipy<nSubPixels; ipy++) {
       
       for (int ibx=0; ibx<nbeta; ibx++) {
	 for (int iby=0; iby<nbeta; iby++) {
	   
	   if ( hhy[ibx+iby*nbeta]>=((ipy)*bsize) && hhy[ibx+iby*nbeta]<=((ipy+1)*bsize)) {
	     newhhx[ibx+iby*nbeta]=hix[ibx]/((double)tot_eta_x);
	     if (newhhx[ibx+iby*nbeta]>1) cout << "***"<< ibx << " " << iby << newhhx[ibx+iby*nbeta] << endl;
	     // if (ipy==3 && ibx==10) cout << newhhx[ibx+iby*nbeta] << " " << hix[ibx] << " " << ibx+iby*nbeta << endl;
	   }
	   if (hhx[ibx+iby*nbeta]>=((ipy)*bsize) && hhx[ibx+iby*nbeta]<=((ipy+1)*bsize)) {
	     newhhy[ibx+iby*nbeta]=hiy[iby]/((double)tot_eta_y);
	     if (newhhy[ibx+iby*nbeta]>1) cout << "***"<< ibx << " " << iby << newhhy[ibx+iby*nbeta] << endl;
	     //  if (ipy==3 && iby==10) cout << newhhy[ibx+iby*nbeta] << " " << hiy[iby] << " " << ibx+iby*nbeta << endl;
	   }
	 }
       }
  }
    
    
  }
  

 public:
 etaInterpolationAdaptiveBins(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : etaInterpolationPosXY(nx,ny,ns, nb, emin,emax){};

 etaInterpolationAdaptiveBins(etaInterpolationAdaptiveBins *orig): etaInterpolationPosXY(orig){};

  virtual etaInterpolationAdaptiveBins* Clone() {

    return new etaInterpolationAdaptiveBins(this);

  };



  virtual void prepareInterpolation(int &ok)
  {
   ok=1;  
   cout << "Adaptive bins" << endl;

   ///*Eta Distribution Rebinning*///
   double bsize=1./nSubPixels; //precision
   // cout<<"nPixelsX = "<<nPixelsX<<" nPixelsY = "<<nPixelsY<<" nSubPixels = "<<nSubPixels<<endl;
   double tot_eta=0;
   double tot_eta_x=0;
   double tot_eta_y=0;
   for (int ip=0; ip<nbeta*nbeta; ip++)
     tot_eta+=heta[ip];
   if (tot_eta<=0) {ok=0; return;};


   double hx[nbeta]; //profile x
   double hy[nbeta]; //profile y
   double hix[nbeta]; //integral of projection x
   double hiy[nbeta]; //integral of projection y
   int ii=0;


   /** initialize distribution to linear interpolation */
   // for (int ibx=0; ibx<nbeta; ibx++) {
   //  for (int ib=0; ib<nbeta; ib++) {
   //    hhx[ibx+ib*nbeta]=((float)ibx)/((float)nbeta);
   //    hhy[ibx+ib*nbeta]=((float)ib)/((float)nbeta);
   //  }
   // }
   
       etaInterpolationPosXY::prepareInterpolation(ok);

#ifdef SAVE_ALL
   
   char tit[10000];
   float *etah=new float[nbeta*nbeta];
  int etabins=nbeta;

  for (int ii=0; ii<etabins*etabins; ii++) {
    
    etah[ii]=hhx[ii];
  }
  sprintf(tit,"/scratch/start_hhx.tiff");
  WriteToTiff(etah, tit, etabins, etabins);
	  
  for (int ii=0; ii<etabins*etabins; ii++) {
    etah[ii]=hhy[ii];
  }
  sprintf(tit,"/scratch/start_hhy.tiff");
  WriteToTiff(etah, tit, etabins, etabins);
	  
#endif
   int nint=1000;

   double thr=1./((double)nSubPixels);
   double avg=tot_eta/((double)(nSubPixels*nSubPixels));
   cout << "total eta entries is :"<< tot_eta << " avg: "<< avg << endl;   
   cout << "Start " << endl;
   double old_diff=calcDiff(avg, hhx, hhy), new_diff=old_diff+1, best_diff=old_diff;
   cout << " diff= " << old_diff << endl;
   

   int iint=0;
   float *newhhx=new float[nbeta*nbeta]; //profile x
   float *newhhy=new float[nbeta*nbeta]; //profile y
   float *besthhx=hhx; //profile x
   float *besthhy=hhy; //profile y
   while (iint<nint) {
     
     cout << "Iteration " << iint << endl;
     iterate(newhhx,newhhy);
     new_diff=calcDiff(avg, newhhx, newhhy);
     cout << " diff= " << new_diff << endl;
/* #ifdef SAVE_ALL */
/*   for (int ii=0; ii<etabins*etabins; ii++) { */
/*     etah[ii]=newhhx[ii]; */
/*     if (etah[ii]>1 || etah[ii]<0  ) cout << "***"<< ii << etah[ii] << endl; */
   
/*   } */
/*   sprintf(tit,"/scratch/neweta_hhx_%d.tiff",iint); */
/*   WriteToTiff(etah, tit, etabins, etabins); */
	  
/*   for (int ii=0; ii<etabins*etabins; ii++) { */
/*     etah[ii]=newhhy[ii]; */
/*     if (etah[ii]>1 || etah[ii]<0  ) cout << "***"<< ii << etah[ii] << endl; */
/*   } */
/*   sprintf(tit,"/scratch/neweta_hhy_%d.tiff",iint); */
/*   WriteToTiff(etah, tit, etabins, etabins); */
/* #endif */
  if (new_diff<best_diff) {
    best_diff=new_diff;
    besthhx=newhhx;
    besthhy=newhhy;
  }
  
  if (hhx!=besthhx)
    delete [] hhx;
  if (hhy!=besthhy)
    delete [] hhy;
  
  hhx=newhhx;
  hhy=newhhy;
  newhhx=new float[nbeta*nbeta]; //profile x
  newhhy=new float[nbeta*nbeta]; //profile y

  old_diff=new_diff;
  //} /* else { */
     /*   cout << "Difference not decreasing after "<< iint << " iterations (" << old_diff << " < " << new_diff << ")"<< endl; */
     /*   break; */
     /* } */
  
  iint++;
   }
     delete [] newhhx;
     delete [] newhhy;
     
  if (hhx!=besthhx)
    delete [] hhx;
  if (hhy!=besthhy)
    delete [] hhy;
  
  hhx=besthhx;
  hhy=besthhy;

   



#ifdef SAVE_ALL
   

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

#endif
