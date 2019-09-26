
#include "ansi.h"
#include <iostream>
#include <math.h>

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
#include "slsInterpolation.h"
#include "Stat.h"
//#include "etaInterpolationRandomBins.h"
using namespace std;
#define NC 400
#define NR 400
#define MAX_EBINS 100

int main(int argc, char *argv[]) {

  if (argc<7) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile  outfile runmin runmax cmin cmax nb" << endl;
    return 1;
  }
  
  int iarg=4;
  char infname[10000];
  char fname[10000];
  char outfname[10000];

  float img[NC*NR];
  iarg=3;

  int runmin=atoi(argv[iarg++]);
  int runmax=atoi(argv[iarg++]); 
  cout << "Run min: " << runmin << endl;
  cout << "Run max: " << runmax << endl;
  

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

    single_photon_hit cl(3,3);


    int iebin=0;
    double eb_size=(cmax-cmin)/n_ebins;

    Stat *statsX=new Stat[(n_ebins+1)*NC*NR];
    Stat *statsY=new Stat[(n_ebins+1)*NC*NR];
    

    int irun;
    for (irun=runmin; irun<runmax; irun++) {
      sprintf(infname,argv[1],irun);
 
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
	  slsInterpolation::calcEta(cl.get_cluster(), etax, etay, sum, totquad, sDum);
	  if (sum>cmin && totquad/sum>0.8 && totquad/sum<1.2 && sum<cmax ) {
	    nph++;
	    iebin=(sum-cmin)/eb_size;
	    if (iebin>=0 && iebin<n_ebins && cl.x>=0 && cl.x<NC && cl.y>=0 && cl.y<NR) {
	      statsX[iebin+cl.x*(n_ebins+1)+cl.y*(NC*(n_ebins+1))].Push(etax);
	      statsY[iebin+cl.x*(n_ebins+1)+cl.y*(NC*(n_ebins+1))].Push(etay);  
	      statsX[n_ebins+cl.x*(n_ebins+1)+cl.y*(NC*(n_ebins+1))].Push(etax);
	      statsY[n_ebins+cl.x*(n_ebins+1)+cl.y*(NC*(n_ebins+1))].Push(etay);  
	    }
	    
	    if (nph%1000000==0) cout << nph << endl;
	   
	  }
	} 
	fclose(f);
      } else
	  cout << "could not open file " << infname << endl;
    
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_abs_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsX[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].NumDataValues();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etameanX_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsX[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].Mean();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etarmsX_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsX[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].StandardDeviation();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etameanY_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsY[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].Mean();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etarmsY_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsY[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].StandardDeviation();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      


      

    }



      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_abs_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsX[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].NumDataValues();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etameanX_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsX[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].Mean();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etarmsX_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsX[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].StandardDeviation();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etameanY_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsY[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].Mean();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      
      for (int ien=0; ien<n_ebins+1; ien++) {
	sprintf(outfname,"%s_etarmsY_energy%d.tiff",argv[2],ien);
	for (int iy=0; iy<NR; iy++) {
	  for (int ix=0; ix<NC; ix++) {
	    img[ix+NC*iy]= statsY[ien+ix*(n_ebins+1)+iy*(NC*(n_ebins+1))].StandardDeviation();
	  }
	}
	WriteToTiff(img, outfname, NR, NC);
      }
      
      


    cout << "Filled " << nph << " (/"<< totph <<") " << endl; 
    return 0;
}

