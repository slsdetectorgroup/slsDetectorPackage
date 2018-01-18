#ifndef ETA_INTERPOLATION_ADAPTIVEBINS_H
#define ETA_INTERPOLATION_ADAPTIVEBINS_H


#include "tiffIO.h"
#include "etaInterpolationBase.h"

class etaInterpolationAdaptiveBins : public etaInterpolationBase{
 private:
  double calcDiff(double avg, double *hx, double *hy) {
    double p_tot=0;
    double diff=0;
    double bsize=1./nSubPixels;
    
    for (int ipx=0; ipx=nSubPixels; ipx++) {
      for (int ipy=0; ipy=nSubPixels; ipy++) {
	p_tot=0;
	for (int ibx=0; ibx<nbeta; ibx++) {
	  for (int iby=0; iby<nbeta; iby++) {
	    if ( hx[ibx+iby*nbeta]>=((ipx)*bsize) && hx[ibx+iby*nbeta]<((ipx+1)*bsize) &&  hy[ibx+iby*nbeta]>=((ipy)*bsize) && hy[ibx+iby*nbeta]<((ipy+1)*bsize)) {
	      p_tot+=heta[ibx+iby*nbeta];
	    } 
	  }
	}
	
	diff+=(p_tot-avg)*(p_tot-avg);
	
      }
    }
    return diff;
    
  }
  
  void iterate(double *newhhx, double *newhhy) {

    double bsize=1./nSubPixels;
    
    double hy[nbeta]; //profile y
    double hx[nbeta]; //profile x
    double hix[nbeta]; //integral of projection x
    double hiy[nbeta]; //integral of projection y
    
   double tot_eta_x=0;
   double tot_eta_y=0;
    for (int ipy=0; ipy=nSubPixels; ipy++) {
     
      
      for (int ibx=0; ibx<nbeta; ibx++) {
	for (int iby=0; iby<nbeta; iby++) {
	  if ( hhy[ibx+iby*nbeta]>=((ipy)*bsize) && hhy[ibx+iby*nbeta]<((ipy+1)*bsize)) {
	    hx[ibx]+=heta[ibx+iby*nbeta];
	    } 
	  if ( hhx[ibx+iby*nbeta]>=((ipy)*bsize) && hhx[ibx+iby*nbeta]<((ipy+1)*bsize)) {
	    hy[iby]+=heta[ibx+iby*nbeta];
	  } 
	}
	}
      
      
    }
 
  hix[0]=hx[0];
  hiy[0]=hy[0];
  for (int ib=1; ib<nbeta; ib++) {
      hix[ib]=hix[ib-1]+hx[ib];
      hiy[ib]=hiy[ib-1]+hy[ib];
  }
  tot_eta_x=hix[nbeta-1];
  tot_eta_y=hiy[nbeta-1];
  
  for (int ipy=0; ipy=nSubPixels; ipy++) {
    
      for (int ibx=0; ibx<nbeta; ibx++) {
	for (int iby=0; iby<nbeta; iby++) {

	  if ( hhy[ibx+iby*nbeta]>=((ipy)*bsize) && hhy[ibx+iby*nbeta]<((ipy+1)*bsize)) {
	    newhhx[ibx+iby*nbeta]=hix[ibx]/((double)tot_eta_x);
	  }
	  if ( hhx[ibx+iby*nbeta]>=((ipy)*bsize) && hhx[ibx+iby*nbeta]<((ipy+1)*bsize)) {
	    newhhy[ibx+iby*nbeta]=hiy[iby]/((double)tot_eta_y);
	  }
	}
      }
  }
  
  
  }
  

 public:
 etaInterpolationAdaptiveBins(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : etaInterpolationBase(nx,ny,ns, nb, emin,emax){};

 etaInterpolationAdaptiveBins(etaInterpolationAdaptiveBins *orig): etaInterpolationBase(orig){};

  virtual etaInterpolationAdaptiveBins* Clone() {

    return new etaInterpolationAdaptiveBins(this);

  };



  virtual void prepareInterpolation(int &ok)
  {
   ok=1;  


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


   /** initialize distribution to linear interpolation */
   for (int ibx=0; ibx<nbeta; ibx++) {
     for (int ib=0; ib<nbeta; ib++) {
       hhx[ibx+ib*nbeta]=((float)ibx)/((float)nbeta);
       hhy[ibx+ib*nbeta]=((float)ibx)/((float)nbeta);
     }
   }
   
   int nint=10;

   double thr=1./((double)/nSubPixels);
   double avg=tot_eta/((double)(nSubPixels*nSubPixels));
   
   double old_diff=calcDiff(avg, hhx, hhy), new_diff=old_diff+1;
   

   int iint=0;
   double newhhx=new double[nbeta*nbeta]; //profile x
   double newhhy=new double[nbeta*nbeta]; //profile y
   while (iint<nint) {

     
     iterate(newhhx,newhhy);
     new_diff=calcDiff(avg, newhhx, newhhy);
     if (new_diff<old_diff) {
       delete [] hhx;
       delete [] hhy;
       hhx=newhhx;
       hhy=newhhy;
       newhhx=new double[nbeta*nbeta]; //profile x
       newhhy=new double[nbeta*nbeta]; //profile y
       old_diff=new_diff;
     } else {
       cout << "Difference not decreasing after "<< iint << " iterations (" << old_diff << " < " << new_diff << ")"<< endl;
       break;
     }
     
     iint++;
   }
   delete [] newhhx;
   delete [] newhhy;




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

#endif
