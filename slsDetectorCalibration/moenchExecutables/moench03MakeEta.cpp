
#include "ansi.h"
#include <iostream>

#include "moench03T1ZmqData.h"
#include "single_photon_hit.h"

 #include "etaInterpolationPosXY.h"

using namespace std;
#define NC 400
#define NR 400

#define XTALK

int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [outfname]
 *
 */
  int nsubpix=10;
  int etabins=nsubpix*100;
  double etamin=-1, etamax=2;
  double eta3min=-2, eta3max=2;
  int quad;
  double sum, totquad;
  double sDum[2][2];
  char fname[10000];
  double etax, etay;
  int runmin, runmax;
    single_photon_hit cl(3,3);
int iph=0;
 
  if (argc<7) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile " << " outfile runmin runmax cmin cmax" << endl;
    return 1;
  }

  eta2InterpolationPosXY *interp2=new eta2InterpolationPosXY(NR, NC, nsubpix, etabins, etamin, etamax);
  cout << "###########"<< endl;
  eta3InterpolationPosXY *interp3=new eta3InterpolationPosXY(NR, NC, nsubpix, etabins, eta3min, eta3max);
  // cout << eta3min << " " << eta3max << endl;
  runmin=atoi(argv[3]);
  runmax=atoi(argv[4]);
  double cmin=atof(argv[5]); //200
  double cmax=atof(argv[6]); //3000
  
#ifdef XTALK
  int old_val[3][3];
  int new_val[3][3];
  double xcorr=0.04;
  
  // int ix=0;
#endif

    FILE *f;
    for (int i=runmin; i<runmax; i++) {
	sprintf(fname,argv[1],i);
      f=fopen(fname,"r");
      if (f) {
	cout << "*" << endl;
	while (cl.read(f)) {
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
	   quad=interp2->calcQuad(cl.get_cluster(), sum, totquad, sDum);
	   
	    if (sum>cmin && totquad/sum>0.8 && totquad/sum<1.2 && totquad<cmax && quad<cmax) {
	      /* Cross talk corrections !!! */
	      //	for (int ix=0; ix<2; ix++) {
	  

	      interp2->addToFlatField(cl.get_cluster(),etax, etay);
	      // if (etax>0.49 && etax<0.51 && etay>0.49 && etay<0.51 ) {
	      // 	cout << cl.y << " " << cl.x << " " << quad << " "<< totquad << " " <<sum << endl;

	      // } 
	      interp3->addToFlatField(cl.get_cluster(),etax, etay);
	      iph++;
	      if (iph%1000000==0) cout << iph << endl;
	      if (iph%100000000==0) { 
		sprintf(fname,"%s_eta2.tiff",argv[2]);
		interp2->writeFlatField(fname);
		sprintf(fname,"%s_eta3.tiff",argv[2]);
		interp3->writeFlatField(fname);
	      }
	      // if (iph>1E8) break;
	    }
	    //	  }
	  
	}
	fclose(f);
	
      }
      else cout << "could not open file " << fname << endl;
    }
    
    sprintf(fname,"%s_eta2.tiff",argv[2]);
	interp2->writeFlatField(fname);
    sprintf(fname,"%s_eta3.tiff",argv[2]);
	interp3->writeFlatField(fname);
    return 0;
}

