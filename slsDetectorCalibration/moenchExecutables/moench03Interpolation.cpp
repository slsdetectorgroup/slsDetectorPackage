// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "sls/ansi.h"
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
#include "etaInterpolationPosXY.h"
#include "noInterpolation.h"
//#include "etaInterpolationCleverAdaptiveBins.h"
//#include "etaInterpolationRandomBins.h"
using namespace std;
#define NC             400
#define NR             400
#define MAX_ITERATIONS (nSubPixels * 100)

#define XTALK

int main(int argc, char *argv[]) {

#ifndef FF
    if (argc < 9) {
        cout << "Wrong usage! Should be: " << argv[0]
             << " infile  etafile outfile runmin runmax ns [cmin cmax xmin xmax ymin ymax]" << endl;
        return 1;
    }
#endif

#ifdef FF
    if (argc < 7) {
        cout << "Wrong usage! Should be: " << argv[0]
             << " infile  etafile runmin runmax [cmin cmax xmin xmax ymin ymax]" << endl;
        return 1;
    }
#endif
    int iarg = 4;
    char infname[10000];
    char outfname[10000];
#ifndef FF
    iarg = 4;
#endif

#ifdef FF
    iarg = 3;
#endif
    int runmin = atoi(argv[iarg++]);
    int runmax = atoi(argv[iarg++]);
    cout << "Run min: " << runmin << endl;
    cout << "Run max: " << runmax << endl;

    int nsubpix = 4;
#ifndef FF
    nsubpix = atoi(argv[iarg++]);
    cout << "Subpix: " << nsubpix << endl;
#endif
    
    float cmin = 0;
    float cmax=1000000;
    if (argc>iarg)
      cmin=atof(argv[iarg++]);
    if (argc>iarg)
      cmax= atof(argv[iarg++]);
    cout << "Energy min: " << cmin << endl;
    cout << "Energy max: " << cmax << endl;
    int xmin=0, xmax=NC, ymin=0, ymax=NR;
    
    if (argc>iarg)
      xmin=atof(argv[iarg++]);
    if (argc>iarg)
      xmax= atof(argv[iarg++]);
    
    if (argc>iarg)
      ymin=atof(argv[iarg++]);
    if (argc>iarg)
      ymax= atof(argv[iarg++]);
    
    // int etabins=500;
    int etabins = 1000; // nsubpix*2*100;
    double etamin = -0.25, etamax = 1.25;
    // double etamin=-0.1, etamax=1.1;
    //   double eta3min = -2, eta3max = 2;
    double sum, totquad;
    double sDum[2][2];
    double etax, etay;
    // double eta3x, eta3y, int3_x, int3_y, noint_x, noint_y;
   
    int ix, iy, isx, isy;
    int nframes = 0, lastframe = -1, iframe, nphFrame;
    //double d_x, d_y, res = 5, xx, yy;
    int nph = 0, totph = 0;
    //badph = 0, 
    FILE *f = NULL;

#ifdef DOUBLE_SPH
    single_photon_hit_double cl(3, 3);
#endif

#ifndef DOUBLE_SPH
    single_photon_hit cl(3, 3);
#endif

    //int f0 = -1;
    //  int nSubPixels = nsubpix;
#ifndef NOINTERPOLATION
    eta2InterpolationPosXY *interp =
      new eta2InterpolationPosXY(NC, NR, nsubpix, nsubpix, etabins, etabins, etamin, etamax);
    // eta2InterpolationCleverAdaptiveBins *interp=new
    // eta2InterpolationCleverAdaptiveBins(NC, NR, nsubpix, etabins, etamin,
    // etamax);
#endif
#ifdef NOINTERPOLATION
    noInterpolation *interp = new noInterpolation(NC, NR, nsubpix);
#endif

    int quad; 
#ifndef FF
#ifndef NOINTERPOLATION
    char fname[10000];
    int ok;
    double int_x, int_y;
    int *img;
    cout << "read ff " << argv[2] << endl;
    sprintf(fname, "%s", argv[2]);
    interp->readFlatField(fname);
    interp->prepareInterpolation(ok); //, MAX_ITERATIONS);
#endif
    // return 0;
#endif
#ifdef FF
    cout << "Will write eta file " << argv[2] << endl;
#endif

    float *totimg = new float[NC * NR * nsubpix * nsubpix];
    for (ix = 0; ix < NC; ix++) {
        for (iy = 0; iy < NR; iy++) {
            for (isx = 0; isx < nsubpix; isx++) {
                for (isy = 0; isy < nsubpix; isy++) {
                    totimg[ix * nsubpix + isx +
                           (iy * nsubpix + isy) * (NC * nsubpix)] = 0;
                }
            }
        }
    }

#ifdef FF
    sprintf(outfname, argv[2]);
#endif

    int irun;
    for (irun = runmin; irun <= runmax; irun++) {
        sprintf(infname, argv[1], irun);
#ifndef FF
        sprintf(outfname, argv[3], irun);
#endif


        f = fopen(infname, "r");
        if (f) {
            cout << infname << endl;
            nframes = 0;
            //f0 = -1;

	    //iff=0;
	    while (fread((void*)&iframe, 1,  sizeof(int), f)) {
	      //n=0;
	      if (fread((void*)&nphFrame, 1,  sizeof(int), f)) {
		for (int iph=0; iph<nphFrame; iph++) {
		//while (cl.read(f)) {
		  if (cl.read(f)) {
                totph++;
                if (lastframe != cl.iframe) {
                    lastframe = cl.iframe;
                    // cout << cl.iframe << endl;
                    // f0=cl.iframe;
                    // if (nframes == 0)
                    //     f0 = lastframe;
                    nframes++;
                }
                // quad=interp->calcQuad(cl.get_cluster(), sum, totquad, sDum);
// #ifndef FF
//                 quad = interp->calcEta(cl.get_cluster(), etax, etay, sum,
//                                        totquad, sDum);
// #endif
// #ifdef FF
	
                quad = interp->calcEta(cl.get_cluster(), etax, etay, sum,
                                       totquad, sDum);
	
		
		/*	cl.print();
		cout << "(" << etax <<","<< etay <<")"<< quad<< endl;
		*/
		//#endif

		if (totquad > cmin && cl.x >= xmin && cl.x <= xmax && 
		    cl.y >= ymin && cl.y <= ymax &&
		    totquad < cmax) {

		  //	if (sum > cmin && totquad / sum > 0.8 && totquad / sum < 1.2 &&
		  //	sum < cmax) {
		  nph++;
                    //  if (sum>200 && sum<580) {
                    //  interp->getInterpolatedPosition(cl.x,cl.y,
                    //  totquad,quad,cl.get_cluster(),int_x, int_y);
// #ifdef SOLEIL
// 	    if (cl.x>210 && cl.x<240 && cl.y>210 && cl.y<240) {
// #endif
#ifndef FF
                    // interp->getInterpolatedPosition(cl.x,cl.y,
                    // cl.get_cluster(),int_x, int_y);
                    interp->getInterpolatedPosition(cl.x, cl.y, etax, etay,
                                                    quad, int_x, int_y);
                    // cout <<"**************"<< endl;
                    // cout << cl.x << " " << cl.y << " " << sum << endl;
                    // cl.print();
                    // cout << int_x << " " << int_y << endl;
                    // cout <<"**************"<< endl;
                    //  if (etax!=0 && etay!=0 && etax!=1 && etay!=1)
                    interp->addToImage(int_x, int_y);
                    if (int_x < 0 || int_y < 0 || int_x > 400 || int_y > 400) {
                        cout << "**************" << endl;
                        cout << cl.x << " " << cl.y << " " << sum << endl;
                        cl.print();
                        cout << int_x << " " << int_y << endl;
                        cout << "**************" << endl;
                    }
#endif
#ifdef FF
                    //	interp->addToFlatField(cl.get_cluster(), etax, etay);
                    // #ifdef UCL
                    // 	    if (cl.x>50)
                    // #endif
                    // 	    if (etax!=0 && etay!=0 && etax!=1 && etay!=1)
                    interp->addToFlatFieldDistribution(etax, etay);
                    //  if (etax==0 || etay==0) cout << cl.x << " " << cl.y <<
                    //  endl;

#endif
                    // #ifdef SOLEIL
                    // 	    }
                    // #endif

                    if (nph % 1000000 == 0)
                        cout << nph << endl;
                    if (nph % 10000000 == 0) {
#ifndef FF
                        interp->writeInterpolatedImage(outfname);
#endif
#ifdef FF
                        interp->writeFlatField(outfname);
#endif
		    }
                    }
                }
	      }
	    }
	    }

            fclose(f);
#ifdef FF
            interp->writeFlatField(outfname);
#endif

#ifndef FF
            interp->writeInterpolatedImage(outfname);

            img = interp->getInterpolatedImage();
            for (ix = 0; ix < NC; ix++) {
                for (iy = 0; iy < NR; iy++) {
                    for (isx = 0; isx < nsubpix; isx++) {
                        for (isy = 0; isy < nsubpix; isy++) {
                            totimg[ix * nsubpix + isx +
                                   (iy * nsubpix + isy) * (NC * nsubpix)] +=
                                img[ix * nsubpix + isx +
                                    (iy * nsubpix + isy) * (NC * nsubpix)];
                        }
                    }
                }
            }
            // cout << "Read " << nframes << " frames (first frame: " << f0
            //      << " last frame: " << lastframe << " delta:" << lastframe - f0
            //      << ") nph=" << nph << endl;
            interp->clearInterpolatedImage();
#endif

        } else
            cout << "could not open file " << infname << endl;
    }
#ifndef FF
    sprintf(outfname, argv[3], 11111);
    WriteToTiff(totimg, outfname, NC * nsubpix, NR * nsubpix);
#endif

#ifdef FF
    interp->writeFlatField(outfname);
#endif

    cout << "Filled " << nph << " (/" << totph << ") " << endl;
    return 0;
}
