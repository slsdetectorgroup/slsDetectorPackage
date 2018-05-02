
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

#include "etaInterpolationPosXY.h"
#include "noInterpolation.h"
//#include "etaInterpolationAdaptiveBins.h"
//#include "etaInterpolationRandomBins.h"
using namespace std;
#define NC 400
#define NR 400


#define XTALK

int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [outfname]
 *
 */

  if (argc<9) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile " << " etafile outfile runmin runmax ns cmin cmax" << endl;
    return 1;
  }

  char infname[10000];
  char fname[10000];
  char outfname[10000];
  int runmin=atoi(argv[4]);
  int runmax=atoi(argv[5]);
  int nsubpix=atoi(argv[6]);
  float cmin=atof(argv[7]);
  float cmax=atof(argv[8]);
  
  int etabins=1000;//nsubpix*2*100;
  double etamin=-1, etamax=2;
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
#ifdef MANYFILES
  int ff=1000;
#endif
  int nph=0, badph=0, totph=0;
    FILE *f=NULL;

#ifdef DOUBLE_SPH
    single_photon_hit_double cl(3,3);
#endif

#ifndef DOUBLE_SPH
    single_photon_hit cl(3,3);
#endif

#ifdef XTALK
  int old_val[3][3];
  int new_val[3][3];
  double xcorr=0.04;
  
  //  int ix=0;
#endif


    eta2InterpolationPosXY *interp=new eta2InterpolationPosXY(NC, NR, nsubpix, etabins, etamin, etamax);
    eta3InterpolationPosXY *interp3=new eta3InterpolationPosXY(NC, NR, nsubpix, etabins, eta3min, eta3max);
    noInterpolation *dummy=new noInterpolation(NC, NR, nsubpix);
    noInterpolation *nointerp=new noInterpolation(NC, NR, nsubpix);
    noInterpolation *mult=new noInterpolation(NC, NR, nsubpix);
    //etaInterpolationAdaptiveBins *interp=new etaInterpolationAdaptiveBins (NC, NR, nsubpix, etabins, etamin, etamax);
    //etaInterpolationRandomBins *interp=new etaInterpolationRandomBins (NC, NR, nsubpix, etabins, etamin, etamax);
    //#ifndef FF
    cout << "read ff " << argv[2] << endl;
    sprintf(fname,"%s_eta2.tiff",argv[2]);
    interp->readFlatField(fname);
    interp->prepareInterpolation(ok);

    sprintf(fname,"%s_eta3.tiff",argv[2]);
    interp3->readFlatField(fname);
    interp3->prepareInterpolation(ok);
    //#endif

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
    
    float ff[nsubpix*nsubpix];
    float *ffimg=new float[NC*NR*nsubpix*nsubpix];
    float totff=0;

	
#endif
    int irun;
    for (irun=runmin; irun<runmax; irun++) {
      sprintf(infname,argv[1],irun);
#ifndef MANYFILES
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
#ifdef MANYFILES
	  if (nframes%ff==0) {
	    sprintf(outfname,argv[3],irun,nframes-ff);
	    cout << outfname << endl;
	    interp->writeInterpolatedImage(outfname);
	    interp->clearInterpolatedImage();
	  }
#endif
	 
	  }
#ifdef XTALK
	  if ((cl.x+1)%25!=0) {
	    for (int ix=-1; ix<2; ix++) {
	      for (int iy=-1; iy<2; iy++) {
		old_val[iy+1][ix+1]=cl.get_data(ix,iy);
		if (ix>=0) {
		  new_val[iy+1][ix+1]=old_val[iy+1][ix+1]-old_val[iy+1][ix]*xcorr;
		  cl.set_data(new_val[iy+1][ix+1],ix,iy);
		}
	      }
	    }
	  }
#endif
	  quad=interp->calcQuad(cl.get_cluster(), sum, totquad, sDum);
	    if (sum>cmin && totquad/sum>0.8 && totquad/sum<1.2 && totquad<cmax && quad<cmax) {
	      nph++;
	      //  if (sum>200 && sum<580) {
	      //  interp->getInterpolatedPosition(cl.x,cl.y, totquad,quad,cl.get_cluster(),int_x, int_y);
	    interp->getInterpolatedPosition(cl.x,cl.y, cl.get_cluster(),int_x, int_y);
	    interp->addToImage(int_x, int_y);
	    interp3->getInterpolatedPosition(cl.x,cl.y, cl.get_cluster(),int3_x, int3_y);
	    interp3->addToImage(int3_x, int3_y);
	    nointerp->getInterpolatedPosition(cl.x,cl.y, cl.get_cluster(),noint_x, noint_y);
	    nointerp->addToImage(noint_x, noint_y);
	    

	    d_x= (int_x-int3_x)*25.;
	    d_y= (int_y-int3_y)*25.;	    
	    dummy->calcEta(totquad, sDum, etax, etay); 
	    xx=int_x;
	    yy=int_y;
	    if (etax<0.1 || etax>0.9) xx=int3_x;
	    if (etay<0.1 || etay>0.9) yy=int3_y;
	    dummy->addToImage(xx,yy);

	    if (d_x>res || d_x<-res || d_y>res || d_y<-res) {
	      badph++;   
	      // cout << "delta (um): "<< d_x << " " << d_y << " " << cl.x << " " << cl.y <<  endl;
	      // cout << sum << " " << totquad << " " << etax << " "<< etay << endl;
	      // //cout<< int_x << " " << int_y << " " << int3_x << " " << int3_y << endl;
	    }
	    mult->addToImage(noint_x, noint_y);

	      if (nph%1000000==0) cout << nph << endl;
	      if (nph%100000000==0) { 
		sprintf(outfname,"%s_inteta2.tiff", argv[3]);
		interp->writeInterpolatedImage(outfname); 
		sprintf(outfname,"%s_inteta3.tiff", argv[3]);
		interp3->writeInterpolatedImage(outfname); 
		sprintf(outfname,"%s_mix.tiff", argv[3]);
		dummy->writeInterpolatedImage(outfname); 
		sprintf(outfname,"%s_noint.tiff", argv[3]);
		nointerp->writeInterpolatedImage(outfname); 
		sprintf(outfname,"%s_mult.tiff", argv[3]);
		mult->writeInterpolatedImage(outfname); 
	      }
	    
	    } else {
	      mult->getInterpolatedPosition(cl.x,cl.y, cl.get_cluster(),int_x, int_y);
	      for (int imult=0; imult<2.*sum/(cmax+cmin); imult++) mult->addToImage(int_x, int_y);
	    }

	    
	}
	fclose(f);
#ifdef FF
	img=interp->getInterpolatedImage();
	for (isx=0; isx<nsubpix; isx++) {
	  for (isy=0; isy<nsubpix; isy++) {
	    ff[isy*nsubpix+isx]=0;
	  }
	}
	totff=0;
	for (ix=0; ix<NC; ix++) {
	  for (iy=0; iy<NR-100; iy++) {
	    for (isx=0; isx<nsubpix; isx++) {
	      for (isy=0; isy<nsubpix; isy++) {
		ff[isy*nsubpix+isx]+=img[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)];
	      }
	    }
	  }
	}
	
	for (isx=0; isx<nsubpix; isx++) {
	  for (isy=0; isy<nsubpix; isy++) {
	    totff+=ff[isy*nsubpix+isx];
	  }
	}
	totff/=nsubpix*nsubpix;


	if (totff) {

	  cout << "ff: " << totff << endl;

	  for (isx=0; isx<nsubpix; isx++) {
	    for (isy=0; isy<nsubpix; isy++) {
	      ff[isy*nsubpix+isx]/=totff;
	      cout << ff[isy*nsubpix+isx] << "\t";
	    }
	    cout << endl;
	  }
	  


	  for (ix=0; ix<NC; ix++) {
	    for (iy=0; iy<NR; iy++) {
	      for (isx=0; isx<nsubpix; isx++) {
		for (isy=0; isy<nsubpix; isy++) {
		  ffimg[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)]=img[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)]/ff[isy*nsubpix+isx];
		  totimg[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)]+=ffimg[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)];
		}
	      }
	    }
	  }
	
	  cout << "writing eta!" << endl;
	  WriteToTiff(ffimg, outfname,NC*nsubpix,NR*nsubpix); 





	}	    
#endif
#ifndef FF

#ifdef MANYFILES
	sprintf(outfname,argv[3],irun,nframes-ff);
	cout << outfname << endl;
#endif
	sprintf(outfname,"%s_inteta2.tiff", argv[3]);
	interp->writeInterpolatedImage(outfname); 
	sprintf(outfname,"%s_inteta3.tiff", argv[3]);
	interp3->writeInterpolatedImage(outfname); 
	sprintf(outfname,"%s_mix.tiff", argv[3]);
	dummy->writeInterpolatedImage(outfname); 
	sprintf(outfname,"%s_mult.tiff", argv[3]);
	mult->writeInterpolatedImage(outfname); 

#ifndef MANYFILES
		img=interp->getInterpolatedImage();
	  for (ix=0; ix<NC; ix++) {
	    for (iy=0; iy<NR; iy++) {
	      for (isx=0; isx<nsubpix; isx++) {
		for (isy=0; isy<nsubpix; isy++) {
		  totimg[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)]+=img[ix*nsubpix+isx+(iy*nsubpix+isy)*(NC*nsubpix)];
		}
	      }
	    }
	  }
#endif

#endif
	  cout << "Read " << nframes << " frames (first frame: " << f0 << " last frame: " << lastframe << " delta:" << lastframe-f0 << ")"<<endl;
	interp->clearInterpolatedImage();
	interp3->clearInterpolatedImage();
	dummy->clearInterpolatedImage();
	mult->clearInterpolatedImage();

      } else
	cout << "could not open file " << infname << endl;
    }
    cout << irun << " " << runmax << endl;
#ifndef MANYFILES
    sprintf(outfname,argv[3],11111);
    WriteToTiff(totimg, outfname,NC*nsubpix,NR*nsubpix); 
#endif
    cout << "Filled " << nph << " (/"<< totph <<") of which " << badph << " badly interpolated " << endl; 
    return 0;
}

