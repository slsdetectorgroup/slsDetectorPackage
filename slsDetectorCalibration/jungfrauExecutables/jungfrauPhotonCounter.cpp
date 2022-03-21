// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#include "sls/ansi.h"
#include <iostream>
#define CORR

#define C_GHOST 0.0004

#define CM_ROWS 50

//#define VERSION_V1

//#include "moench03T1ZmqData.h"
#ifdef NEWRECEIVER
#ifndef RECT
#include "moench03T1ReceiverDataNew.h"
#endif

#ifdef RECT
#include "moench03T1ReceiverDataNewRect.h"
#endif

#endif

#ifdef CSAXS_FP
#include "moench03T1ReceiverData.h"
#endif
#ifdef OLDDATA
#include "moench03Ctb10GbT1Data.h"
#endif

// #include "interpolatingDetector.h"
//#include "etaInterpolationPosXY.h"
// #include "linearInterpolation.h"
// #include "noInterpolation.h"
#include "multiThreadedCountingDetector.h"
//#include "multiThreadedAnalogDetector.h"
#include "moench03CommonMode.h"
#include "moench03GhostSummation.h"
#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"

#include <fstream>
#include <map>
#include <stdio.h>
#include <sys/stat.h>

#include <ctime>
using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 4) {
        cout << "Usage is " << argv[0]
             << "indir outdir fname [runmin] [runmax] [pedfile] [threshold] "
                "[nframes] [xmin xmax ymin ymax] [gainmap]"
             << endl;
        cout << "threshold <0 means analog; threshold=0 means cluster finder; "
                "threshold>0 means photon counting"
             << endl;
        cout << "nframes <0 means sum everything; nframes=0 means one file per "
                "run; nframes>0 means one file every nframes"
             << endl;
        return 1;
    }

    int p = 10000;
    int fifosize = 1000;
    int nthreads = 10;
    int nsubpix = 25;
    int etabins = nsubpix * 10;
    double etamin = -1, etamax = 2;
    int csize = 3;
    int save = 1;
    int nsigma = 5;
    int nped = 10000;
    int ndark = 100;
    int ok;
    int iprog = 0;

    int cf = 0;

#ifdef NEWRECEIVER
#ifdef RECT
    cout << "Should be rectangular!" << endl;
#endif
    moench03T1ReceiverDataNew *decoder = new moench03T1ReceiverDataNew();
    cout << "RECEIVER DATA WITH ONE HEADER!" << endl;
#endif

#ifdef CSAXS_FP
    moench03T1ReceiverData *decoder = new moench03T1ReceiverData();
    cout << "RECEIVER DATA WITH ALL HEADERS!" << endl;
#endif

#ifdef OLDDATA
    moench03Ctb10GbT1Data *decoder = new moench03Ctb10GbT1Data();
    cout << "OLD RECEIVER DATA!" << endl;
#endif

    int nx = 400, ny = 400;

    decoder->getDetectorSize(nx, ny);

    int ncol_cm = CM_ROWS;
    double xt_ghost = C_GHOST;
    moench03CommonMode *cm = NULL;
    moench03GhostSummation *gs;
    double *gainmap = NULL;
    float *gm;

    int size = 327680; ////atoi(argv[3]);

    int *image;
    // int* image =new int[327680/sizeof(int)];

    int ff, np;
    // cout << " data size is " << dsize;

    ifstream filebin;
    char *indir = argv[1];
    char *outdir = argv[2];
    char *fformat = argv[3];
    int runmin = 0;

    // cout << "argc is " << argc << endl;
    if (argc >= 5) {
        runmin = atoi(argv[4]);
    }

    int runmax = runmin;

    if (argc >= 6) {
        runmax = atoi(argv[5]);
    }

    char *pedfile = NULL;
    if (argc >= 7) {
        pedfile = argv[6];
    }
    double thr = 0;
    double thr1 = 1;

    if (argc >= 8) {
        thr = atof(argv[7]);
    }

    int nframes = 0;

    if (argc >= 9) {
        nframes = atoi(argv[8]);
    }

    int xmin = 0, xmax = nx, ymin = 0, ymax = ny;
    if (argc >= 13) {
        xmin = atoi(argv[9]);
        xmax = atoi(argv[10]);
        ymin = atoi(argv[11]);
        ymax = atoi(argv[12]);
    }

    char *gainfname = NULL;
    if (argc > 13) {
        gainfname = argv[13];
        cout << "Gain map file name is: " << gainfname << endl;
    }

    char ffname[10000];
    char fname[10000];
    char imgfname[10000];
    char cfname[10000];
    char fn[10000];

    std::time_t end_time;

    FILE *of = NULL;
    cout << "input directory is " << indir << endl;
    cout << "output directory is " << outdir << endl;
    cout << "input file is " << fformat << endl;
    cout << "runmin is " << runmin << endl;
    cout << "runmax is " << runmax << endl;
    if (pedfile)
        cout << "pedestal file is " << pedfile << endl;
    if (thr > 0)
        cout << "threshold is " << thr << endl;
    cout << "Nframes is " << nframes << endl;

    uint32_t nnx, nny;
    double *gmap;

    // if (gainfname) {
    //   gm=ReadFromTiff(gainfname, nny, nnx);
    //   if (gm && nnx==nx && nny==ny) {
    //     gmap=new double[nx*ny];
    //     for (int i=0; i<nx*ny; i++) {
    // 	gmap[i]=gm[i];
    //     }
    //     delete gm;
    //   } else
    //     cout << "Could not open gain map " << gainfname << endl;
    // }

#ifdef CORR
    cout << "Applying common mode  " << ncol_cm << endl;
    cm = new moench03CommonMode(ncol_cm);

    cout << "Applying ghost corrections " << xt_ghost << endl;
    gs = new moench03GhostSummation(decoder, xt_ghost);
#endif

    singlePhotonDetector *filter = new singlePhotonDetector(
        decoder, csize, nsigma, 1, cm, nped, 200, -1, -1, gainmap, gs);

    if (gainfname) {

        if (filter->readGainMap(gainfname))
            cout << "using gain map " << gainfname << endl;
        else
            cout << "Could not open gain map " << gainfname << endl;
    } else
        thr = 0.15 * thr;
    filter->newDataSet();
    int dsize = decoder->getDataSize();

    char data[dsize];

    //#ifndef ANALOG
    if (thr > 0) {
        cout << "threshold is " << thr << endl;
        //#ifndef ANALOG
        filter->setThreshold(thr);
        //#endif
        cf = 0;

    } else
        cf = 1;
    //#endif

    filter->setROI(xmin, xmax, ymin, ymax);
    std::time(&end_time);
    cout << std::ctime(&end_time) << endl;

    char *buff;

    // multiThreadedAnalogDetector *mt=new
    // multiThreadedAnalogDetector(filter,nthreads,fifosize);
    multiThreadedCountingDetector *mt =
        new multiThreadedCountingDetector(filter, nthreads, fifosize);
#ifndef ANALOG
    mt->setDetectorMode(ePhotonCounting);
    cout << "Counting!" << endl;
    if (thr > 0) {
        cf = 0;
    }
#endif
//{
#ifdef ANALOG
    mt->setDetectorMode(eAnalog);
    cout << "Analog!" << endl;
    cf = 0;
    // thr1=thr;
#endif
    //  }

    mt->StartThreads();
    mt->popFree(buff);

    //  cout << "mt " << endl;

    int ifr = 0;

    double ped[nx * ny], *ped1;

    if (pedfile) {

        cout << "PEDESTAL " << endl;
        sprintf(imgfname, "%s/pedestals.tiff", outdir);

        if (string(pedfile).find(".tif") == std::string::npos) {
            sprintf(fname, "%s.raw", pedfile);
            cout << fname << endl;
            std::time(&end_time);
            cout << "aaa" << std::ctime(&end_time) << endl;

            mt->setFrameMode(ePedestal);
            // sprintf(fn,fformat,irun);
            filebin.open((const char *)(fname), ios::in | ios::binary);
            //      //open file
            if (filebin.is_open()) {
                ff = -1;
                while (decoder->readNextFrame(filebin, ff, np, buff)) {
                    if (np == 40) {
                        mt->pushData(buff);
                        mt->nextThread();
                        mt->popFree(buff);
                        ifr++;
                        if (ifr % 100 == 0)
                            cout << ifr << " " << ff << " " << np << endl;
                    } else
                        cout << ifr << " " << ff << " " << np << endl;
                    ff = -1;
                }
                filebin.close();
                while (mt->isBusy()) {
                    ;
                }

            } else
                cout << "Could not open pedestal file " << fname
                     << " for reading " << endl;
        } else {
            float *pp = ReadFromTiff(pedfile, nny, nnx);
            if (pp && nnx == nx && nny == ny) {
                for (int i = 0; i < nx * ny; i++) {
                    ped[i] = pp[i];
                }
                delete[] pp;
                mt->setPedestal(ped);
                // ped1=mt->getPedestal();

                // for (int i=0; i<nx*ny; i++) {

                //   cout << ped[i]<<"/"<<ped1[i] << " " ;
                // }
                cout << "Pedestal set from tiff file " << pedfile << endl;
            } else {
                cout << "Could not open pedestal tiff file " << pedfile
                     << " for reading " << endl;
            }
        }
        mt->writePedestal(imgfname);
        std::time(&end_time);
        cout << std::ctime(&end_time) << endl;
    }

    ifr = 0;
    int ifile = 0;

    mt->setFrameMode(eFrame);

    for (int irun = runmin; irun <= runmax; irun++) {
        cout << "DATA ";
        // sprintf(fn,fformat,irun);
        sprintf(ffname, "%s/%s.raw", indir, fformat);
        sprintf(fname, ffname, irun);
        sprintf(ffname, "%s/%s.tiff", outdir, fformat);
        sprintf(imgfname, ffname, irun);
        sprintf(ffname, "%s/%s.clust", outdir, fformat);
        sprintf(cfname, ffname, irun);
        cout << fname << " ";
        cout << imgfname << endl;
        std::time(&end_time);
        cout << std::ctime(&end_time) << endl;
        //  cout <<  fname << " " << outfname << " " << imgfname <<  endl;
        filebin.open((const char *)(fname), ios::in | ios::binary);
        //      //open file
        ifile = 0;
        if (filebin.is_open()) {
            if (thr <= 0 && cf != 0) { // cluster finder
                if (of == NULL) {
                    of = fopen(cfname, "w");
                    if (of) {
                        mt->setFilePointer(of);
                        cout << "file pointer set " << endl;
                    } else {
                        cout << "Could not open " << cfname << " for writing "
                             << endl;
                        mt->setFilePointer(NULL);
                        return 1;
                    }
                }
            }
            //     //while read frame
            ff = -1;
            ifr = 0;
            while (decoder->readNextFrame(filebin, ff, np, buff)) {
                if (np == 40) {
                    //	cout << "*"<<ifr++<<"*"<<ff<< endl;
                    //	cout << ff << " " << np << endl;
                    //         //push
                    mt->pushData(buff);
                    // 	//         //pop
                    mt->nextThread();
                    // // 		//	cout << " " << (void*)buff;
                    mt->popFree(buff);

                    ifr++;
                    if (ifr % 100 == 0)
                        cout << ifr << " " << ff << endl;
                    if (nframes > 0) {
                        if (ifr % nframes == 0) {
                            // The name has an additional "_fXXXXX" at the end,
                            // where "XXXXX" is the initial frame number of the
                            // image (0,1000,2000...)

                            sprintf(ffname, "%s/%s_f%05d.tiff", outdir, fformat,
                                    ifile);
                            sprintf(imgfname, ffname, irun);
                            // cout << "Writing tiff to " << imgfname << " " <<
                            // thr1 << endl;
                            mt->writeImage(imgfname, thr1);
                            mt->clearImage();
                            ifile++;
                        }
                    }
                } else
                    cout << ifr << " " << ff << " " << np << endl;
                ff = -1;
            }
            cout << "--" << endl;
            filebin.close();
            //      //close file
            //     //join threads
            while (mt->isBusy()) {
                ;
            }
            if (nframes >= 0) {
                if (nframes > 0) {
                    sprintf(ffname, "%s/%s_f%05d.tiff", outdir, fformat, ifile);
                    sprintf(imgfname, ffname, irun);
                } else {
                    sprintf(ffname, "%s/%s.tiff", outdir, fformat);
                    sprintf(imgfname, ffname, irun);
                }
                cout << "Writing tiff to " << imgfname << " " << thr1 << endl;
                mt->writeImage(imgfname, thr1);
                mt->clearImage();
                if (of) {
                    fclose(of);
                    of = NULL;
                    mt->setFilePointer(NULL);
                }
            }
            std::time(&end_time);
            cout << std::ctime(&end_time) << endl;
        } else
            cout << "Could not open " << fname << " for reading " << endl;
    }
    if (nframes < 0) {
        sprintf(ffname, "%s/%s.tiff", outdir, fformat);
        strcpy(imgfname, ffname);
        cout << "Writing tiff to " << imgfname << " " << thr1 << endl;
        mt->writeImage(imgfname, thr1);
    }

    return 0;
}
