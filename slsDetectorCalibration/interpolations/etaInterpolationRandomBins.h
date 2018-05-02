#ifndef ETA_INTERPOLATION_RANDOMBINS_H
#define ETA_INTERPOLATION_RANDOMBINS_H


#include "tiffIO.h"
//#include "etaInterpolationBase.h"
#include "etaInterpolationPosXY.h"
#include <cstdlib>
#include <algorithm>
//#include <math>
#include <cmath>        // std::abs

#define PI 3.14159265
#define TWOPI 2.*PI

using namespace std;

class etaInterpolationRandomBins : public etaInterpolationPosXY {

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
	
	//	cout <<  p_tot << " \t ";



	diff+=(p_tot-avg)*(p_tot-avg);
	
      }
      //  cout << "\n";
    }
    return diff;
  }
  
  double iterate(float *newhhx, float *newhhy, double avg) {

    double bsize=1./nSubPixels;
    
    double hy[nbeta]; //profile y
    double hx[nbeta]; //profile x
    double hix[nbeta]; //integral of projection x
    double hiy[nbeta]; //integral of projection y
    
   double tot_eta_x=0;
   double tot_eta_y=0;

   int p0;
   int vx[(nSubPixels+1)*(nSubPixels+1)], vy[(nSubPixels+1)*(nSubPixels+1)];

   int arrx[nSubPixels+1], arry[nSubPixels+1];

   int bad=1;


   int isby, isbx;
   int ii=0;
     

  // using default comparison (operator <):
  // std::sort (myvector.begin(), myvector.begin()+4);           //(12 32 45 71)26 80 53 33

   for (isby=0; isby<(nSubPixels+1)/2+1; isby++) {
     
     for (isbx=0; isbx<(nSubPixels+1)/2+1; isbx++) {
       p0=isby*(nSubPixels+1)+isbx;
  // for (int iv=0; iv<(nSubPixels+1)*(nSubPixels+1); iv++) {
       if (isbx==0) {
	 vy[p0]=isby*nbeta/nSubPixels;
	 vx[p0]=0;  
       } else if ( isby==0 ) {
       vy[p0]=0;
       vx[p0]=isbx*nbeta/nSubPixels;
       } 
       else {
	 vy[p0]=rand()%(nbeta/2);
	 vx[p0]=rand()%(nbeta/2);
	 if (nSubPixels%2==0 && isbx==nSubPixels/2) 
	   vx[p0]=nbeta/2;
	 if (nSubPixels%2==0 && isby==nSubPixels/2 )
	   vy[p0]=nbeta/2;
       }
     // cout << "("  << vx[p0] << " , " << vy[p0] << " ) \t" ;
     //  }
     }
     //cout << endl;
   }
   // cout << "rand" << endl;
   
   
   while (bad) {

     for (isby=0; isby<(nSubPixels+1)/2+1; isby++) {
       
       
       for (isbx=0; isbx<(nSubPixels+1)/2+1; isbx++) {
       arrx[isbx]=vx[isby*(nSubPixels+1)+isbx];
       arry[isbx]=vy[isbx*(nSubPixels+1)+isby];
       //cout << isbx << " " << arrx[isbx] << " " << isby << " " << arry[isbx] << endl;
       }

       sort(arrx,arrx+(nSubPixels+1)/2+1);
       sort(arry,arry+(nSubPixels+1)/2+1);
       
       //  cout << "*****"<< endl;
       // cout << endl;

       for (int isbx=0; isbx<(nSubPixels+1)/2+1; isbx++) {
	 vx[isby*(nSubPixels+1)+isbx]=arrx[isbx];
	 vy[isbx*(nSubPixels+1)+isby]=arry[isbx];
	 

	 vx[(nSubPixels-isby)*(nSubPixels+1)+(nSubPixels-isbx)]=nbeta-arrx[isbx];
	 vy[(nSubPixels-isbx)*(nSubPixels+1)+(nSubPixels-isby)]=nbeta-arry[isbx];

	 vx[isby*(nSubPixels+1)+(nSubPixels-isbx)]=nbeta-arrx[isbx];
	 vy[isbx*(nSubPixels+1)+(nSubPixels-isby)]=arry[isbx];
	  

	 vx[(nSubPixels-isby)*(nSubPixels+1)+(isbx)]=arrx[isbx];
	 vy[(nSubPixels-isbx)*(nSubPixels+1)+(isby)]=nbeta-arry[isbx];


       }
       
       
       
     }
     
     /* for (isby=0; isby<nSubPixels+1; isby++) { */
       
     /*   for (isbx=0; isbx<nSubPixels+1; isbx++) { */

     /* 	 cout << "("<< vx[isby*(nSubPixels+1)+isbx] << " " << vy[isby*(nSubPixels+1)+isbx] << ")\t";//<< endl; */
     /*   } */
     /*   cout << endl; */
     /* } */
     
     bad=0;
     for (isby=1; isby<(nSubPixels+1)/2+1; isby++) {
       
       for (isbx=1; isbx<(nSubPixels+1)/2+1; isbx++) {
      
  	 if (heta[vx[isby*(nSubPixels+1)+isbx]+vy[isby*(nSubPixels+1)+isbx]*nbeta]<avg*(nSubPixels*nSubPixels)/(nbeta*nbeta)) {
  	//	cout << ii << " " << isbx << " " << isby << " " << vx[isby*(nSubPixels+1)+isbx] << " " << vy[isby*(nSubPixels+1)+isbx] << " " << heta[vx[isby*(nSubPixels+1)+isbx]+vy[isby*(nSubPixels+1)+isbx]*nbeta] << endl;
	   if (nSubPixels%2==0 && isbx==nSubPixels/2)
	     ;
	   else
	     vx[isby*(nSubPixels+1)+isbx]=rand()%(nbeta/2);
	   
	   if (nSubPixels%2==0 && isbx==nSubPixels/2)
	     ;
	   else
	     vy[isby*(nSubPixels+1)+isbx]=rand()%(nbeta/2);

	   if (bad==0)
	     ii++;
	
	   bad=1;
  	//	break;
	 }
	 
       }
       //if (bad) break;
     }
   // cout << "sort" << endl;
     
   }


   cout << ii << " sub iteractions " << avg*(nSubPixels*nSubPixels)/(nbeta*nbeta) << endl;

   double m,q;
   int in_quad;
   int p[4];
   int p1x,p2x, p1y, p2y;
   //  cout << nbeta << endl;
   double angle;
  double dtheta,theta1,theta2;




       
       for (int ibx=0; ibx<nbeta; ibx++) {
	 
  	 for (int iby=0; iby<nbeta; iby++) {

	   in_quad=0;

	   /* if (ibx==0)  */
	   /*   isbx=0; */
	   /* else */
	   /*   isbx= (newhhx[ibx-1+iby*nbeta])/bsize-1; */
	   /* if (isbx<0) isbx=0; */
	   /* if (isbx>nSubPixels-1) isbx=nSubPixels-1; */

	   /* if (iby==0)  */
	   /*   isby=0; */
	   /* else */
	   /*   isby= (newhhx[ibx+(iby-1)*nbeta])/bsize-1; */
	   
	   /* if (isby<0) isbx=0; */
	   /* if (isby>nSubPixels-1) isby=nSubPixels-1; */
	   /* //  cout << isbx << " " << isby << endl; */

	   for (isby=0; isby<nSubPixels; isby++) {
     
	     for (isbx=0; isbx<nSubPixels; isbx++) {

	       //   cout << ibx << " " << iby << " " << isbx << " " << isby << endl;
	       p[0]=isby*(nSubPixels+1)+isbx;
	       p[1]=isby*(nSubPixels+1)+isbx+1;
	       p[2]=(isby+1)*(nSubPixels+1)+isbx+1;
	       p[3]=(isby+1)*(nSubPixels+1)+isbx;
      

	       angle=0;
	       for (int i=0;i<4;i++) {
		 p1x = vx[p[i]] - ibx;
		 p1y = vy[p[i]] - iby;
		 p2x = vx[p[(i+1)%4]] - ibx;
		 p2y = vy[p[(i+1)%4]] - iby;
		 theta1 = atan2(p1y,p1x);
		 theta2 = atan2(p2y,p2x);
		 dtheta = theta2 - theta1;

		 while (dtheta > PI)
		   dtheta -= TWOPI;
		 while (dtheta < -PI)
		   dtheta += TWOPI;

		 angle += dtheta;
	       }

	       if (abs((double)angle) < PI)
		 in_quad=0;
	       else
		 in_quad=1;
       	 
	       if (in_quad) {
		 newhhx[ibx+iby*nbeta]=bsize*((double)isbx);
		 newhhy[ibx+iby*nbeta]=bsize*((double)isby);
		 break;
	       }
	       
	       
	       
	       
	     }
	     if (in_quad)  break;
	   }
       
     }
   }
       
       //   cout << "hist" << endl;
       return calcDiff(avg, newhhx, newhhy);
  }
  

 public:
 etaInterpolationRandomBins(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : etaInterpolationPosXY(nx,ny,ns, nb, emin,emax){};

 etaInterpolationRandomBins(etaInterpolationRandomBins *orig): etaInterpolationPosXY(orig){};

  virtual etaInterpolationRandomBins* Clone() {

    return new etaInterpolationRandomBins(this);

  };



  virtual void prepareInterpolation(int &ok)
  {
   ok=1;  
   cout << "Adaptive bins" << endl;

   ///*Eta Distribution Rebinning*///
   double bsize=1./nSubPixels; //precision
   // cout<<"nPixelsX = "<<nPixelsX<<" nPixelsY = "<<nPixelsY<<" nSubPixels = "<<nSubPixels<<endl;
   double tot_eta=0;
   for (int ip=0; ip<nbeta*nbeta; ip++)
     tot_eta+=heta[ip];
   if (tot_eta<=0) {ok=0; return;};


   int ii=0;


  
   int nint=1000;

   double thr=1./((double)nSubPixels);
   double avg=tot_eta/((double)(nSubPixels*nSubPixels));
   cout << "total eta entries is :"<< tot_eta << " avg: "<< avg << endl;   
   cout << "Start " << endl;
   double old_diff=-1, new_diff=-1;
   // cout << " diff= " << new_diff << endl;
   




   
   etaInterpolationPosXY::prepareInterpolation(ok);
       
   old_diff=calcDiff(avg, hhx, hhy);
   cout << " diff= " << old_diff << endl;









   int iint=0;
   float *newhhx=new float[nbeta*nbeta]; //profile x
   float *newhhy=new float[nbeta*nbeta]; //profile y
   int igood=0, ibad=0;
#ifdef SAVE_ALL
   int etabins=nbeta;
   float *etah=new float[nbeta*nbeta];
   char tit[1000];
#endif

   while (iint<nint) {
     
     cout << "Iteration " << iint << endl;
     new_diff=iterate(newhhx,newhhy, avg);
     //new_diff=calcDiff(avg, newhhx, newhhy);
     cout << " diff= " << new_diff << " ( " << old_diff<< " ) " << endl;



/* #ifdef SAVE_ALL */
/*      for (int ii=0; ii<etabins*etabins; ii++) { */
/*        etah[ii]=newhhx[ii]; */
/*        if (etah[ii]>1 || etah[ii]<0  ) cout << "***"<< ii << etah[ii] << endl; */
/*      } */
/*      sprintf(tit,"/scratch/randeta_hhx_%d.tiff",iint); */
/*      WriteToTiff(etah, tit, etabins, etabins); */
	  
/*      for (int ii=0; ii<etabins*etabins; ii++) { */
/*        etah[ii]=newhhy[ii]; */
/*        if (etah[ii]>1 || etah[ii]<0  ) cout << "***"<< ii << etah[ii] << endl; */
/*      } */
/*      sprintf(tit,"/scratch/randeta_hhy_%d.tiff",iint); */
/*      WriteToTiff(etah, tit, etabins, etabins); */
/* #endif */
     
     if (new_diff<old_diff) {
      
       cout << "******************** GOOD! ***********************"<< endl;
       delete [] hhx;
       delete [] hhy;
       igood++;
       hhx=newhhx;
       hhy=newhhy; 
       newhhx=new float[nbeta*nbeta]; //profile x */
       newhhy=new float[nbeta*nbeta]; //profile y */
       old_diff=new_diff;
     } else
       ibad++;
     
     iint++;
   }
   delete [] newhhx;
   delete [] newhhy;

   cout << "performed " << iint << " iterations of which " << igood << " positive " << endl;


/* #ifdef SAVE_ALL */
   

/*   for (int ii=0; ii<etabins*etabins; ii++) { */
/*     etah[ii]=hhx[ii]; */
/*   } */
/*   sprintf(tit,"/scratch/eta_hhx_%d.tiff",id); */
/*   WriteToTiff(etah, tit, etabins, etabins); */
	  
/*   for (int ii=0; ii<etabins*etabins; ii++) { */
/*     etah[ii]=hhy[ii]; */
/*   } */
/*   sprintf(tit,"/scratch/eta_hhy_%d.tiff",id); */
/*   WriteToTiff(etah, tit, etabins, etabins); */
	  
/*   for (int ii=0; ii<etabins*etabins; ii++) { */
/*     etah[ii]=heta[ii]; */
/*   } */
/*   sprintf(tit,"/scratch/eta_%d.tiff",id); */
/*   WriteToTiff(etah, tit, etabins, etabins); */
/*   delete [] etah; */
/* #endif */
  return ;
  }

};

#endif
