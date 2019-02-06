
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

//#include "etaInterpolationPosXY.h"
#include "noInterpolation.h"
#include "etaInterpolationCleverAdaptiveBins.h"
//#include "etaInterpolationRandomBins.h"
using namespace std;
#define NC 400
#define NR 400
#define MAX_ITERATIONS (nSubPixels*100)
#define MAX_EBINS 100
#define XTALK

int main(int argc, char *argv[]) {

#ifndef FF
  if (argc<9) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile  etafile outfile runmin runmax ns cmin cmax" << endl;
    return 1;
  }
#endif
  
#ifdef FF
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
  int n_ebins=1;
  if (argc>iarg)
    n_ebins=atoi(argv[iarg++]);
  //int etabins=500;
  int etabins=1000;//nsubpix*2*100;
  double etamin=-1, etamax=2;
  //double etamin=-0.1, etamax=1.1;
  double eta3min=-2, eta3max=2;
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

#ifdef DOUBLE_SPH
    single_photon_hit_double cl(3,3);
#endif

#ifndef DOUBLE_SPH
    single_photon_hit cl(3,3);
#endif

    int nSubPixels=nsubpix;

    int iebin=0;
    double eb_size=(cmax-cmin)/n_ebins;
#ifndef NOINTERPOLATION
    //  eta2InterpolationPosXY *interp[MAX_EBINS];
    eta2InterpolationCleverAdaptiveBins *interp[MAX_EBINS];
    for (int i=0; i< n_ebins; i++) {
      //interp[i]=new eta2InterpolationPosXY(NC, NR, nsubpix, etabins, etamin, etamax);
      interp[i]=new eta2InterpolationCleverAdaptiveBins(NC, NR, nsubpix, etabins, etamin, etamax);  
    }
#endif
#ifdef NOINTERPOLATION
    noInterpolation *interp=new noInterpolation(NC, NR, nsubpix);  
#endif


 
#ifndef FF
#ifndef NOINTERPOLATION
    cout << "read ff " << argv[2] << endl; 
    for (int i=0; i< n_ebins; i++) {
      sprintf(fname,argv[2],i);
      interp[i]->readFlatField(fname);
      interp[i]->prepareInterpolation(ok);//, MAX_ITERATIONS);
    }
#endif
    // return 0;
#endif
#ifdef FF
    cout << "Will write eta file " << argv[2] << endl;
#endif

    int *img;
    float *totimg=new float[NC*NR*nsubpix*nsubpix];
    for (ix=0; ix<NC; ix++) {
      for (iy=0; iy<NR; iy++) {
	for (isx=0; isx<nsubpix; isx++) {
	  for (isy=0; isy<nsubpix; isy++) {
	    totimg[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)]=0;
	      }
	}
      }
    }

#ifdef FF
      sprintf(outfname,argv[2]);
#endif     

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
	    // cout << cl.iframe << endl;
	    // f0=cl.iframe;
	    if (nframes==0) f0=lastframe;
	    nframes++;
	  }
	  //quad=interp->calcQuad(cl.get_cluster(), sum, totquad, sDum);
	  quad=interp[0]->calcEta(cl.get_cluster(), etax, etay, sum, totquad, sDum);
	  if (sum>cmin && totquad/sum>0.8 && totquad/sum<1.2 && sum<cmax ) {
	    nph++;
	    iebin=(sum-cmin)/eb_size;
	    if (iebin>=0 && iebin<n_ebins) {
	    //  if (sum>200 && sum<580) {
	    //  interp->getInterpolatedPosition(cl.x,cl.y, totquad,quad,cl.get_cluster(),int_x, int_y);
// #ifdef SOLEIL
// 	    if (cl.x>210 && cl.x<240 && cl.y>210 && cl.y<240) {
// #endif
#ifndef FF
	    // interp->getInterpolatedPosition(cl.x,cl.y, cl.get_cluster(),int_x, int_y);  
	    interp[iebin]->getInterpolatedPosition(cl.x,cl.y, etax, etay, quad,int_x, int_y);
	      // cout <<"**************"<< endl;
	      // cout << cl.x << " " << cl.y << " " << sum << endl;
	      // cl.print();
	      // cout << int_x << " " << int_y << endl;
	      // cout <<"**************"<< endl;
	    if (etax!=0 && etay!=0 && etax!=1 && etay!=1)
	      interp[iebin]->addToImage(int_x, int_y);
#endif
#ifdef FF
	    //	interp->addToFlatField(cl.get_cluster(), etax, etay);
#ifdef UCL
	    if (cl.x>50)
#endif
	      if (etax!=0 && etay!=0 && etax!=1 && etay!=1)
		interp[iebin]->addToFlatField(etax, etay);
	    //  if (etax==0 || etay==0) cout << cl.x << " " << cl.y << endl;
	     
#endif
// #ifdef SOLEIL
// 	    }
// #endif
	    
	    if (nph%1000000==0) cout << nph << endl;
	    if (nph%10000000==0) { 
#ifndef FF
	      for (int i=0; i<n_ebins; i++) {
		sprintf(outfname,argv[3],i);
		interp[i]->writeInterpolatedImage(outfname);
	      }
#endif
#ifdef FF 
	      for (int i=0; i<n_ebins; i++) {
		sprintf(outfname,argv[2],i);
		cout << outfname << " " << argv[2] << " " << i << endl;
		interp[i]->writeFlatField(outfname);
	      }
#endif
	      
	    }
	    }
	    
	  } 
	  
	}
	fclose(f);
#ifdef FF
	for (int i=0; i<n_ebins; i++) {
	  sprintf(outfname,argv[2],i);
	  cout << outfname << " " << argv[2] << " " << i << endl;
	  interp[i]->writeFlatField(outfname);
	}
#endif
	
#ifndef FF 
	      for (int i=0; i<n_ebins; i++) {
		sprintf(outfname,argv[3],i,irun);
		interp[i]->writeInterpolatedImage(outfname);
		img=interp[i]->getInterpolatedImage();
		for (ix=0; ix<NC; ix++) {
		  for (iy=0; iy<NR; iy++) {
		    for (isx=0; isx<nsubpix; isx++) {
		      for (isy=0; isy<nsubpix; isy++) {
			totimg[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)]+=img[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)];
		      }
		    }
		  }
		}
		//interp[i]->clearInterpolatedImage();
	      }
	      cout << "Read " << nframes << " frames (first frame: " << f0 << " last frame: " << lastframe << " delta:" << lastframe-f0 << ")"<<endl;
#endif
	
      } else
	cout << "could not open file " << infname << endl;
    }
#ifndef FF
    sprintf(outfname,argv[3], 11111);
    WriteToTiff(totimg, outfname,NC*nsubpix,NR*nsubpix);
#endif 
    
#ifdef FF
	interp[iebin]->writeFlatField(outfname);
#endif

    cout << "Filled " << nph << " (/"<< totph <<") " << endl; 
    return 0;
}

