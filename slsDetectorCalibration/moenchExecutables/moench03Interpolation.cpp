
#include "ansi.h"
#include <iostream>

//#include "moench03T1ZmqData.h"
//#define DOUBLE_SPH
//#define MANYFILES

#ifdef DOUBLE_SPH
#include "single_photon_hit_double.h"
#endif

#ifndef DOUBLE_SPH
#include "single_photon_hit.h"
#endif

#ifdef RECT
#include "etaInterpolationGlobal.h"
#endif
#ifndef RECT
#include "etaInterpolationPosXY.h"
#endif
#include "noInterpolation.h"
//#include "etaInterpolationCleverAdaptiveBins.h"
//#include "etaInterpolationRandomBins.h"
using namespace std;
#ifndef RECT
#define NC 400
#define NR 400
#endif
#ifdef RECT
#define NC 200
#define NR 800
#endif


#define MAX_ITERATIONS (nSubPixels*100)

#define XTALK

int main(int argc, char *argv[]) {

#ifndef FF
  cout << "INTERPOLATING" << endl;
  if (argc<9) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile  etafile outfile runmin runmax ns cmin cmax" << endl;
    return 1;
  }
#endif
  
#ifdef FF
  cout << "CALC ETA" << endl;
  if (argc<7) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile  etafile runmin runmax cmin cmax" << endl;
    return 1;
  }
#endif
  int iarg=4;
  char infname[10000];
  char fname[10000];
  char outfname[10000];
#ifndef FF
  iarg=4;
#endif

#ifdef FF
  iarg=3;
#endif
  int runmin=atoi(argv[iarg++]);
  int runmax=atoi(argv[iarg++]); 
  cout << "Run min: " << runmin << endl;
  cout << "Run max: " << runmax << endl;
  
  int nsubpix=4;
#ifndef FF
  nsubpix=atoi(argv[iarg++]);
  cout << "Subpix: " << nsubpix << endl;
#endif
  float cmin=atof(argv[iarg++]);
  float cmax=atof(argv[iarg++]);
  cout << "Energy min: " << cmin << endl;
  cout << "Energy max: " << cmax << endl;
  //int etabins=500;
  int etabins=999;//nsubpix*2*100;
  int etabinsY=etabins;//nsubpix*2*100;
  double etamin=0, etamax=1;
  //double etamin=-0.1, etamax=1.1;
  double eta3min=-1, eta3max=1;
  int quad;
  double sum, totquad;
  double sDum[2][2];
  double etax, etay, int_x, int_y;
  double eta3x, eta3y, int3_x, int3_y, noint_x, noint_y;
  int ok;
  int f0=-1;
  int ix, iy, isx, isy;
  int nframes=0, lastframe=-1;
  double d_x, d_y, res=5, xx, yy;
  int nph=0, badph=0, totph=0;
    FILE *f=NULL;


    char fff[10000];

#ifdef DOUBLE_SPH
    single_photon_hit_double cl(3,3);
#endif

#ifndef DOUBLE_SPH
    single_photon_hit cl(3,3);
#endif
    int nSubPixels=nsubpix;
    int nSubPixelsY=nsubpix;
#ifdef RECT
    nSubPixels=5; //might make more sense using 2?
    // etabins=5;
#endif
#ifndef NOINTERPOLATION
    etaInterpolationBase *interp;
#ifndef RECT
    interp=new eta2InterpolationPosXY(NC, NR, nSubPixels, nSubPixelsY, etabins, etabinsY, etamin, etamax);
   //eta2InterpolationCleverAdaptiveBins *interp=new eta2InterpolationCleverAdaptiveBins(NC, NR, nsubpix, etabins, etamin, etamax);  
#endif
#ifdef RECT
    interp=new eta2InterpolationGlobal(NC, NR, nSubPixels, nSubPixelsY, etabins, etabinsY, eta3min, eta3max);
 
    //  eta3InterpolationPosXY *interpOdd=new eta3InterpolationPosXY(NC, NR, nSubPixels, nSubPixelsY, etabins, etabinsY, eta3min, eta3max);
   //eta2InterpolationCleverAdaptiveBins *interp=new eta2InterpolationCleverAdaptiveBins(NC, NR, nsubpix, etabins, etamin, etamax);  
 
#endif

#endif
#ifdef NOINTERPOLATION
    noInterpolation *interp=new noInterpolation(NC, NR, nsubpix, nSubPixelsY);  
#endif


 
#ifndef FF
#ifndef NOINTERPOLATION
    cout << "read ff " << argv[2] << endl;
    sprintf(fname,"%s",argv[2]);
    //#ifndef RECT
    interp->readFlatField(fname, etamin, etamax);
    interp->prepareInterpolation(ok);//, MAX_ITERATIONS);
    interp->debugSaveAll(0);
    //#endif
// #ifdef RECT
//     sprintf(fff,"%s.even",fname);
//     interpEven->readFlatField(fff, etamin, etamax);
//     interpEven->prepareInterpolation(ok);, MAX_ITERATIONS);
//     interpEven->debugSaveAll(1);
//     sprintf(fff,"%s.odd",fname);
//     interpOdd->readFlatField(fff, etamin, etamax);
//     interpOdd->prepareInterpolation(ok);, MAX_ITERATIONS);
//     interpOdd->debugSaveAll(2);
// #endif
#endif
    // return 0;
#endif
#ifdef FF
    cout << "Will write eta file " << argv[2] << endl;
#endif

    int *img;
    float *totimg=new float[NC*NR*nSubPixels*nSubPixelsY];
    for (ix=0; ix<NC; ix++) {
      for (iy=0; iy<NR; iy++) {
	for (isx=0; isx<nSubPixels; isx++) {
	  for (isy=0; isy<nSubPixelsY; isy++) {
	    totimg[ix*nSubPixels+isx+(iy*nSubPixelsY+isy)*(NC*nSubPixels)]=0;
	      }
	}
      }
    }

#ifdef FF
      sprintf(outfname,argv[2]);
#endif     
 

      int tl=0, tr=0, bl=0, br=0;

    int irun;
    for (irun=runmin; irun<runmax; irun++) {
      sprintf(infname,argv[1],irun);
#ifndef FF
      sprintf(outfname,argv[3],irun);
#endif
 
      f=fopen(infname,"r");
      if (f) {
	cout << infname << endl;
	nframes=0;
	f0=-1;

	while (cl.read(f)) {
	  totph++;
	  if (lastframe!=cl.iframe) {
	    lastframe=cl.iframe;
	    if (nframes==0) f0=lastframe;
	    nframes++;
	  }
#ifdef RECT
	  //	  quad=interp->calcEta3(cl.get_cluster(), etax, etay, sum);  
	  quad=interp->calcEta(cl.get_cluster(), etax, etay, sum, totquad, sDum);
	  // if (cl.y%2==0)
	  //   if (etay>0)
	  //     interp=interpEven;
	  //   else
	  //     interp=interpOdd;
	  // else
	  //   if (etay>0)
	  //     interp=interpOdd;
	  //   else
	  //     interp=interpEven;
	  
#endif

#ifndef RECT
	  quad=interp->calcEta(cl.get_cluster(), etax, etay, sum, totquad, sDum);

	
#endif
	  switch(quad) {
	  case TOP_LEFT:
	    tl++;
	    break;
	  case TOP_RIGHT:
	    tr++;
	    break;
	  case BOTTOM_LEFT:
	    bl++;
	    break;
	  case BOTTOM_RIGHT:
	    br++;
	    break;
	  }
	  //  if (sum>cmin && totquad/sum>0.8 && totquad/sum<1.2 && sum<cmax ) {
	  if (sum>cmin &&  sum<cmax ) {
	      nph++;
#ifndef FF 
	      interp->getInterpolatedPosition(cl.x,cl.y, etax, etay, quad,int_x, int_y);
	    interp->addToImage(int_x, int_y);
#endif
#ifdef FF
	      interp->addToFlatField(etax, etay);
#endif
	    
	    if (nph%1000000==0) cout << nph << endl;
	    if (nph%10000000==0) { 
#ifndef FF
	      
	      cout << "writing interpolated iamge" << endl;
	      //#ifndef RECT
	interp->writeInterpolatedImage(outfname); 
	//#endif
// #ifdef RECT
// 	sprintf(fff,"%s.even",outfname);
// 	interpEven->writeInterpolatedImage(fff); 
// 	sprintf(fff,"%s.odd",outfname);
// 	interpOdd->writeInterpolatedImage(fff); 
// #endif
		cout << "done" << endl;
#endif
#ifdef FF 
		cout << "writing eta" << endl;
		//#ifndef RECT
	interp->writeFlatField(outfname);
	//#endif
// #ifdef RECT
// 	sprintf(fff,"%s.even",outfname);
// 	interpEven->writeFlatField(fff);
// 	sprintf(fff,"%s.odd",outfname);
// 	interpOdd->writeFlatField(fff);
// #endif
		cout << "done" << endl;
#endif
		
		}
	  }
	    
	} 
	  
	  
	fclose(f);
#ifdef FF
	//#ifndef RECT
	interp->writeFlatField(outfname);
// #endif
// #ifdef RECT
// 	sprintf(fff,"%s.even",outfname);
// 	interpEven->writeFlatField(outfname);
// 	sprintf(fff,"%s.odd",outfname);
// 	interpOdd->writeFlatField(outfname);
// #endif
#endif
	
#ifndef FF
	  //#ifndef RECT
	interp->writeInterpolatedImage(outfname); 
// #endif
// #ifdef RECT
// 	sprintf(fff,"%s.even",outfname);
// 	interpEven->writeInterpolatedImage(fff); 
// 	sprintf(fff,"%s.odd",outfname);
// 	interpOdd->writeInterpolatedImage(fff); 
// #endif

//#ifdef RECT
	  //interp=interpEven;
	//#endif
	img=interp->getInterpolatedImage();
	for (ix=0; ix<NC; ix++) {
	  for (iy=0; iy<NR; iy++) {
	    for (isx=0; isx<nSubPixels; isx++) {
	      for (isy=0; isy<nSubPixelsY; isy++) {
		totimg[ix*nSubPixels+isx+(iy*nSubPixelsY+isy)*(NC*nSubPixels)]+=img[ix*nSubPixels+isx+(iy*nSubPixelsY+isy)*(NC*nSubPixels)];
		//	cout << "--" << ix << " " << iy <<" " << ix*nSubPixels+isx+(iy*nSubPixelsY+isy)*(NC*nSubPixels) << endl;
		}
	    }
	  }
	}
	cout << "Read " << nframes << " frames (first frame: " << f0 << " last frame: " << lastframe << " delta:" << lastframe-f0 << ") nph="<< nph <<endl;
	cout << "Top-Left=" << tl << " " << " Top-Right=" << tr << " Bottom-Left="<< bl << " Bottom-Right=" << br << endl;
	interp->clearInterpolatedImage();
// #ifdef RECT

// 	interp=interpOdd;
// 	img=interp->getInterpolatedImage();
// 	for (ix=0; ix<NC; ix++) {
// 	  for (iy=0; iy<NR; iy++) {
// 	    for (isx=0; isx<nSubPixels; isx++) {
// 	      for (isy=0; isy<nSubPixelsY; isy++) {
// 		totimg[ix*nSubPixels+isx+(iy*nSubPixelsY+isy)*(NC*nSubPixels)]+=img[ix*nSubPixels+isx+(iy*nSubPixelsY+isy)*(NC*nSubPixels)];
// 		//	cout << "--" << ix << " " << iy <<" " << ix*nSubPixels+isx+(iy*nSubPixelsY+isy)*(NC*nSubPixels) << endl;
// 		}
// 	    }
// 	  }
// 	}

// 	interp->clearInterpolatedImage();
// #endif


#endif
	
      } else
	cout << "could not open file " << infname << endl;
    }
#ifndef FF
    sprintf(outfname,argv[3],11111);
    WriteToTiff(totimg, outfname,NC*nSubPixels,NR*nSubPixelsY);
#endif 
    
#ifdef FF
    //#ifndef RECT
	interp->writeFlatField(outfname);
	//#endif
// #ifdef RECT
// 	sprintf(fff,"%s.even",outfname);
// 	interpEven->writeFlatField(fff);
// 	sprintf(fff,"%s.odd",outfname);
// 	interpOdd->writeFlatField(fff);
// #endif
#endif

    cout << "Filled " << nph << " (/"<< totph <<") " << endl; 
    return 0;
}

