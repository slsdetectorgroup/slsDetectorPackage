
#include "ansi.h"
#include <iostream>

#include "moench03T1ZmqData.h"
#include "single_photon_hit.h"

 #include "etaInterpolationPosXY.h"

using namespace std;
#define NC 400
#define NR 400


int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [outfname]
 *
 */
  int nsubpix=10;
  int etabins=nsubpix*100;
  double etamin=-1, etamax=2;
  int quad;
  double sum, totquad;
  double sDum[2][2];
  char fname[10000];
  double etax, etay;
  int runmin, runmax;
    single_photon_hit cl(3,3);

  if (argc<5) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile " << " outfile runmin runmax" << endl;
    return 1;
  }

  etaInterpolationPosXY *interp=new etaInterpolationPosXY(NR, NC, nsubpix, etabins, etamin, etamax);
  runmin=atoi(argv[3]);
  runmax=atoi(argv[4]);
  

    FILE *f;
    for (int i=runmin; i<runmax; i++) {
	sprintf(fname,argv[1],i);
      f=fopen(fname,"r");
      if (f) {
	cout << "*" << endl;
	while (cl.read(f)) {
	  interp->calcQuad(cl.get_cluster(), sum, totquad, sDum);
	  if (sum>200 && sum<580 && cl.y<350)
	    interp->addToFlatField(cl.get_cluster(),etax, etay);
	}
	fclose(f);
	interp->writeFlatField(argv[2]);
	
      }
      else cout << "could not open file " << fname << endl;
    }
    
	interp->writeFlatField(argv[2]);
    return 0;
}

