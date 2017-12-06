
#include "ansi.h"
#include <iostream>

#include "moench03T1ZmqData.h"
#include "single_photon_hit.h"

#include "noInterpolation.h"

using namespace std;
#define NC 400
#define NR 400


int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [outfname]
 *
 */

  if (argc<7) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile " << " etafile outfile runmin runmax ns" << endl;
    return 1;
  }

  char infname[10000];
  char outfname[10000];
  int runmin=atoi(argv[4]);
  int runmax=atoi(argv[5]);
  int nsubpix=atoi(argv[6]);
  
  int etabins=1000;//nsubpix*2*100;
  double etamin=-1, etamax=2;
  int quad;
  double sum, totquad;
  double sDum[2][2];
  double etax, etay, int_x, int_y;
  int ok;

  int ix, iy, isx, isy;


    FILE *f=NULL;

    single_photon_hit cl(3,3);
    // etaInterpolationPosXY *interp=new etaInterpolationPosXY(NC, NR, nsubpix, etabins, etamin, etamax);
    noInterpolation *interp=new noInterpolation(NC, NR, nsubpix);
    //  interp->readFlatField(argv[2]);
    // interp->prepareInterpolation(ok);

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

    for (int irun=runmin; irun<runmax; irun++) {
      sprintf(infname,argv[1],irun);
      sprintf(outfname,argv[3],irun);
      
      f=fopen(infname,"r");
      
      if (f) {
	while (cl.read(f)) {
	  quad=interp->calcQuad(cl.get_cluster(), sum, totquad, sDum);
	  if (sum>200 && sum<580) {
	    interp->getInterpolatedPosition(cl.x,cl.y, totquad,quad,cl.get_cluster(),int_x, int_y);
	    interp->addToImage(int_x, int_y);
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
	

	  WriteToTiff(ffimg, outfname,NC*nsubpix,NR*nsubpix); 





	}	    
#endif
#ifndef FF
	interp->writeInterpolatedImage(outfname); 
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

	interp->clearInterpolatedImage();

      }
    }

    sprintf(outfname,argv[3],11111);
    WriteToTiff(totimg, outfname,NC*nsubpix,NR*nsubpix); 
    
    return 0;
}

