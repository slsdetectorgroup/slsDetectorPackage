#include "angularCalibration.h"

#include <iostream>
#ifdef ROOT
#include <TMath.h>
#include <TH1.h>
#endif

#include "usersFunctions.h"

#ifdef __CINT
#include "usersFunctions.cpp"
#endif

using namespace std;

angularCalibration::angularCalibration(int nm): direction(1),
#ifdef ROOT
					  fpeak(NULL),
					  fangle(NULL),
#endif
					  encoder(0),
					  totalOffset(0), 
					  ang_min(-180), 
					  ang_max(180),
					  nmod(nm),
					  nchmod(1280),
					  angConv(NULL)
{
 
#ifdef ROOT
// Creates a Root function based on function peakfunction
  TF1 *fpeak = new TF1("fpeak",this,&angularCalibration::peakFunction,ang_min,ang_max,5,"angularCalibration","peakFunction");

// Sets initial values and parameter names
  //  func->SetParameters((Double_t) PEAKHEIGHT,  (Double_t) maxch,(Double_t) PEAKWIDTH,(Double_t) PEAKBACK);
  fpeak->SetParNames("Number of Photons","Peak Position","Peak Width RMS","Background Offset", "Background Slope");


  TF1 *fangle = new TF1("fangle",this,&angularCalibration::angleFunction,0,1280,3,"angularCalibration","angleFunction");
  fangle->SetParNames("Center","Conversion Radius","Offset");
 
#endif

  angConv=new angleConversionConstant[nmod];

}

angularCalibration::~angularCalibration(){
#ifdef ROOT
  delete fpeak;
  delete fangle;

#endif
}



angleConversionConstant* angularCalibration::getAngularConversionConstant(int imod) {
  if (imod>=0 && imod<nmod)
    return angConv+imod;
  else
    return NULL;

}
angleConversionConstant* angularCalibration::setAngularConversionConstant(angleConversionConstant *a, int imod) {

  if (imod>=0 && imod<nmod) {
    angConv[imod].center=a->center;
    angConv[imod].ecenter=a->ecenter;
    angConv[imod].r_conversion=a->r_conversion;
    angConv[imod].er_conversion=a->er_conversion;
    angConv[imod].offset=a->offset;
    angConv[imod].eoffset=a->eoffset;
    angConv[imod].tilt=a->tilt;
    angConv[imod].etilt=a->etilt;

    return angConv+imod;
  }
  return NULL;

  
}





#ifdef ROOT

Double_t angularCalibration::peakFunction(Double_t *x, Double_t *par) {
   Double_t arg = 0;
   if (par[2] != 0) arg = (x[0] - par[1])/par[2];
   return  par[0]*TMath::Exp(-0.5*arg*arg)+par[3]+par[4]*(x[0]-par[1]);
 
}

Double_t angularCalibration::angleFunction(Double_t *x, Double_t *par) {  
  return par[2]-angle((int)x[0],0,0,par[1],par[0],0,0,direction);
}







TF1 *fitPeak(TH1 *h) {

  TF1 *fitfun=NULL;
  int chmod, imod;
  double ang;

// reads in a run and fits a gaussian to the peak as function
//   of channel number also reads optical encoder 


// find angular range in channels

// is it necessary to discard fit with too many points?
  for (int i=0;i<h->GetNbinsX();i++) {
    imod=i/nchmod;
    chmod=i%(imod*nchmod);
    ang=angle(chmod,encoder,totalOffset,angConv[imod].r_conversion, angConv[imod].center, angConv[imod].offset, angConv[imod].tilt, direction);
    if ((ang>ang_min) && (ang<ang_max)) {
      

    }
    


  }
 

//     for (i=0;i<nchannel;i++) {
//       if ( (angle(i)>minang) && (angle(i)<maxang) ) {
// 	x[npoints]=(double) i;
// 	y[npoints]=(double) data[i];
// 	ex[npoints]=0.001;
// 	ey[npoints]=dataerror[i];
// 	anglefit[npoints]=angle(i);
// 	npoints++;

// 	if (npoints>MAXINPEAK) {
// 	  printf("too many points in angular range !\n");
// 	  return -1;   // too many points in range
// 	}
// 	if ( data[i]> max) {
// 	  max = (int) data[i];
// 	  maxch = i;
// 	}
//       } 
//     }
//   } else
//     return -1;

//   npoints--;
//   chmin= (int) x[0];
//   chmax= (int) x[npoints];

//   printf("number of points in range %f-%f: %i \n",minang,maxang,npoints);  
//   printf("channel from minang to maxang %i - %i \n",chmin,chmax);  
//   printf("channel with max intensity %i \n",maxch);  

//   TCanvas *c1;

//   TGraph *gr1 = new TGraph(npoints,anglefit,y);
//   TGraph *gr2 = new TGraph(npoints,x,y);
//   if (plotflag) {
//   /* create canvas */
    
//     c1 = new TCanvas();
//     c1->SetTitle("Si calibration data");
//     c1->Divide(1,2);

//   /* create graph */

//   sprintf(name,"run number %i",nr);
//   gr1->SetTitle(name);
//   gr2->SetTitle(name);

//   c1->cd(1);
//   gr1->Draw("AL*");
//   c1->cd(2);
//   gr2->Draw("AL*");
//   }

//   /* do not fit if peak is close to edge of module */
//   if (abs(modfromchannel(maxch)*NCHMOD-maxch)<DISTANCE) {
//     printf("peak too close to border of module\n");
//     return -1;
//   }

//   /* do not fit if peak is close to edge of range */
//   if ( ((maxch-chmin)<DISTANCE) || ( (chmax-maxch)<DISTANCE) ) {
//     printf("peak too close to border of range\n");
//     return -1;
//   }

//   /* do not fit if nr of points is to small */
//   if (npoints<10) {
//     printf("too few points in range\n");
//     return -1;
//   }  

//    if (plotflag)
//      gr2->Fit("fitpeak","B");
//    else
//      gr2->Fit("fitpeak","B0");
 
//    TF1 *fit = gr2->GetFunction("fitpeak");

// // writes the fit results into the par array
//   fit->GetParameters(mypar);

//   printf("\n");
//   for (i=0;i<4;i++) {
//       myerr[i] = fit->GetParError(i);     // obtain fit parameter errors
//       printf("parameter %i: %f +- %f \n",i,mypar[i],myerr[i]);
//   }

//   chi2=fit->GetChisquare();
//   printf("chi2: %e\n",chi2);
//   printf("\n\n");

//   if (chi2>CHIMAX) {
//     printf("chi2 too large!\n");
//     return -1;
//   } 

//   if (plotflag)
//     c1->Update(); // necessary for axis titles!
//   //  c1->WaitPrimitive();

//   return 0;

  return fitfun;

}


#endif



// //
// // for detector angular calibration
// //
// // loops over runs fits a peak in each run and then fits the parameters for
// // the angular calibration to the fitted  peak and optical encoder values
// //
// //
// // note:
// // setting global offset is important to find peak in peak fitting!
// // also set peak height,width and background in defines at beginning
// //

// void fitangle(char fname[80],char extension[10], int start, int stop, double startangle, double stopangle) {

//   int i,nfit,mod,npoints,nnpoints;
//   double x[MAXINMODULE],y[MAXINMODULE],ex[MAXINMODULE],ey[MAXINMODULE],min,max;
//   double xx[MAXINMODULE],yy[MAXINMODULE],exx[MAXINMODULE],eyy[MAXINMODULE];

//   double channelfit[MAXRUN], channelerror[MAXRUN], encoderfit[MAXRUN];
//   int runnrfit[MAXRUN], modulenr[MAXRUN];


//   FILE *fp;
//   char name[80];
//   TCanvas *c1,*c2;
//   gROOT->Reset();  // reset root
//   //  gStyle->SetOptFit(1110);

//   nfit=0;
//   for (i=start;i<stop;i++) {
//     //    sprintf(name,"%s%i",fname,i);
//     if (fitpeak(fname,extension,i,startangle,stopangle)!=-1) {
//       printf("nfit %i encoder %f\n",nfit,encoder);
//       channelfit[nfit]=(double) mypar[1];	
//       channelerror[nfit]=(double) myerr[1];
//       encoderfit[nfit]=(double) encoder;	
//       modulenr[nfit]=modfromchannel( channelfit[nfit] );
//       runnrfit[nfit]=i;

//       // only use value if sigma is reasonable
//       if (channelerror[nfit]<MAXSIGMA) { 
// 	nfit++;
//       }
//     }
//   }

//   printf(" %i usable peak fits \n\n",nfit);

//   for (i=0;i<nfit;i++) {
//     printf("i %i run %i encoder %f fit %f module %i \n",i,runnrfit[i],encoderfit[i],channelfit[i],modulenr[i]);
//   }

//   TGraph *gr3 = new TGraph(nfit,channelfit,encoderfit);
//   if (plotflag) {
//   /* create canvas */
//     TCanvas *c1 = new TCanvas();
//     c1->SetTitle("Si calibration data");
//     /* create graph for angle vs fitted channel number */
//     gr3->Draw("AL*");
    
//     c1->Update(); // necessary for axis titles!
//     c1->WaitPrimitive();
    
//     delete c1;
//   }
//   TH1F *herr=new TH1F("herr","",100,0,0.002);
  

//   for (mod=0;mod<NMOD;mod++) {
//     npoints=0;
//     for (i=0;i<nfit;i++) {
//       if (modulenr[i]==mod) {
// 	x[npoints]=channelfit[i]-mod*1280;
// 	ex[npoints]=channelerror[i];
// 	y[npoints]=encoderfit[i];
// 	ey[npoints]=ENCODERERROR;
// 	npoints++;
//       }
//     }

//     if (npoints>5) {

// // create canvas 
//       if (plotflag) {
//       TCanvas *c2 = new TCanvas();
//       c2->SetTitle("Si calibration data");
//       c2->Divide(1,3);
//       }
// // create graph
//       TGraphErrors *gr1 = new TGraphErrors(npoints,x,y,ex,ey);
//       sprintf(name,"module number %i",mod);
//       gr1->SetTitle(name);
//       if (plotflag) {
// 	c2->cd(1);
// 	gr1->Draw("ALP");
//       }


// // Creates a Root function based on function anglefunction
//       if (x[0]>x[npoints-1]) {
// 	min=x[npoints-1];
// 	max=x[0];
//       } else {
// 	max=x[npoints-1];
// 	min=x[0];
//       }


//       TF1 *func = new TF1("fitangle",anglefunction,min,max,3);

// // Sets initial values and parameter names
//       func->SetParameters(640,0.0000656,-mod*5.0);
//       func->SetParNames("center","conversion","offset");
//       func->FixParameter(0,640.0);
//       if (plotflag) {
// 	gr1->Fit("fitangle"); // fit the function
//       } else
// 	gr1->Fit("fitangle","0"); // fit the function


// // calculate the deviations of data points from fitted function and plot them
//       for (i=0;i<npoints;i++) {
// 	ey[i]=func->Eval(x[i])-y[i];
//       }
//       TGraph *gr4 = new TGraph(npoints,x,ey);
//       sprintf(name,"module number %i deviations from fit",mod);
//       gr4->SetTitle(name);

//       if (plotflag) {
// 	gr4->SetMarkerStyle(24);
// 	c2->cd(2);
// 	gr4->Draw("ALP");
//       }


// // iterate fit with outlying points excluded
//       nnpoints=0;
//       for (i=0;i<npoints;i++) {
// 	if (fabs(ey[i])<DIFFANGLEFIT) {
// 	  xx[nnpoints]=x[i];
// 	  yy[nnpoints]=y[i];
// 	  exx[nnpoints]=ex[i];
// 	  eyy[nnpoints]=ENCODERERROR;
// 	  nnpoints++;
// 	}
//       }

//       TGraphErrors *gr3 = new TGraphErrors(nnpoints,xx,yy,exx,eyy);  // create graph  
//       if (plotflag) {
// 	gr3->Fit("fitangle"); // fit the function
//       } else
// 	gr3->Fit("fitangle","0"); // fit the function


// // calculate the deviations of data points from fitted function and plot them
//       for (i=0;i<nnpoints;i++) {
// 	eyy[i]=func->Eval(xx[i])-yy[i];
// 	herr->Fill(eyy[i]);
//       }
      
//       TGraph *gr5 = new TGraph(nnpoints,xx,eyy);
//       sprintf(name,"module number %i deviations from fit second iteration",mod);
//       if (plotflag) {
// 	c2->cd(3);
// 	gr5->SetTitle(name);
// 	gr5->SetMarkerStyle(24);

//       gr5->Draw("ALP");


//       c2->Update(); // necessary for axis titles?
//       c2->WaitPrimitive();

//       }

// // writes the fit results into the par array
// //
// // get fit parameter
//       func->GetParameters(mypar);

//       for (i=0;i<3;i++) {
// 	myerr[i] = func->GetParError(i);     // obtain fit parameter errors
// 	printf("parameter %i: %E +- %E \n",i,mypar[i],myerr[i]);
//       }
//       printf("\n\n");

//       center[mod]=mypar[0];
//       errcenter[mod]=myerr[0];
//       conversion[mod]=mypar[1];
//       errconversion[mod]=myerr[1];
//       moffset[mod]=mypar[2];
//       erroff[mod]=myerr[2];

//       delete gr1;
//       delete gr4;
//       delete gr5;
//       delete func;
//       delete c2;
//     }
//   }
//   //herr->GetXaxis()->SetMaxDigits(3);
//   herr->GetXaxis()->SetTitle("Deviations from fit (deg)"); 
//   herr->Draw();
  

//   printf("\n\n\n");
//   for (mod=0;mod<NMOD;mod++) {
//     printf(" module %i center %.3E +- %.2E conversion %.4E +- %.2E offset %.5f +- %.5f \n",mod,center[mod],errcenter[mod],conversion[mod],errconversion[mod],moffset[mod],erroff[mod]);
//   }
 

// // write file with offsets
//   fp = fopen("ang.off","w");
//   if (fp == NULL) {
//     printf("cant open parameter file !\n");
//     exit(1);
//   }

//   for (mod=0;mod<NMOD;mod++) {
//     fprintf(fp," module %i center %.3E +- %.2E conversion %.4E +- %.2E offset %.5f +- %.5f \n",mod,center[mod],errcenter[mod],conversion[mod],errconversion[mod],moffset[0]-moffset[mod],erroff[mod]);
//   }
  
//   fclose (fp);

// }

