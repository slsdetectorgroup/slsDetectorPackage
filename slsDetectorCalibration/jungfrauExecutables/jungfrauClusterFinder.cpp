// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#include "sls/ansi.h"
#include <iostream>

//#include "moench03T1ZmqData.h"
#ifndef JFSTRX
#include "jungfrauHighZSingleChipData.h"
#endif
#ifdef JFSTRX
#include "jungfrauLGADStrixelsData.h"
#endif

#include "multiThreadedAnalogDetector.h"
#include "singlePhotonDetector.h"

#include <fstream>
#include <map>
#include <stdio.h>
#include <sys/stat.h>

#include <ctime>
using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 6) {
        cout << "Usage is " << argv[0] << "indir outdir fname runmin runmax "
             << endl;
        return 1;
    }
    int p = 10000;
    int fifosize = 1000;
    int nthreads = 1;
    int nsubpix = 25;
    int etabins = nsubpix * 10;
    double etamin = -1, etamax = 2;
    int csize = 3;
    int nx = 400, ny = 400;
    int save = 1;
    int nsigma = 5;
    int nped = 1000;
    int ndark = 100;
    int ok;
    int iprog = 0;
#ifndef JFSTRX
    jungfrauHighZSingleChipData *decoder = new jungfrauHighZSingleChipData();
#endif
#ifdef JFSTRX
    cout << "bbb" << endl;
    jungfrauLGADStrixelsData *decoder = new jungfrauLGADStrixelsData();
#endif

    decoder->getDetectorSize(nx, ny);
    cout << "nx " << nx << " ny " << ny << endl;

    // moench03T1ZmqData *decoder=new  moench03T1ZmqData();
    singlePhotonDetector *filter =
        new singlePhotonDetector(decoder, csize, nsigma, 1, 0, nped, 200);
    //  char tit[10000];
    cout << "filter " << endl;

    int *image;
    filter->newDataSet();

    int ff, np;
    int dsize = decoder->getDataSize();
    cout << " data size is " << dsize;

    char data[dsize];

    ifstream filebin;
    char *indir = argv[1];
    char *outdir = argv[2];
    char *fformat = argv[3];
    int runmin = atoi(argv[4]);
    int runmax = atoi(argv[5]);

    char fname[10000];
    char outfname[10000];
    char imgfname[10000];
    char pedfname[10000];
    char fn[10000];

    std::time_t end_time;

    FILE *of = NULL;
    cout << "input directory is " << indir << endl;
    cout << "output directory is " << outdir << endl;
    cout << "fileformat is " << fformat << endl;

    std::time(&end_time);
    cout << std::ctime(&end_time) << endl;

    char *buff;
    multiThreadedAnalogDetector *mt =
        new multiThreadedAnalogDetector(filter, nthreads, fifosize);

    mt->setDetectorMode(ePhotonCounting);
    mt->setFrameMode(eFrame);
    mt->StartThreads();
    mt->popFree(buff);

    cout << "mt " << endl;

    int ifr = 0;

    for (int irun = runmin; irun < runmax; irun++) {
        sprintf(fn, fformat, irun);
        sprintf(fname, "%s/%s.dat", indir, fn);
        sprintf(outfname, "%s/%s.clust", outdir, fn);
        sprintf(imgfname, "%s/%s.tiff", outdir, fn);
        std::time(&end_time);
        cout << std::ctime(&end_time) << endl;
        cout << fname << " " << outfname << " " << imgfname << endl;
        filebin.open((const char *)(fname), ios::in | ios::binary);
        //      //open file
        if (filebin.is_open()) {
            of = fopen(outfname, "w");
            if (of) {
                mt->setFilePointer(of);
                //	cout << "file pointer set " << endl;
            } else {
                cout << "Could not open " << outfname << " for writing "
                     << endl;
                mt->setFilePointer(NULL);
                return 1;
            }
            //     //while read frame
            ff = -1;
            while (decoder->readNextFrame(filebin, ff, np, buff)) {

                mt->pushData(buff);
                mt->nextThread();
                mt->popFree(buff);
                ifr++;
                if (ifr % 10000 == 0)
                    cout << ifr << " " << ff << endl;
                ff = -1;
            }
            cout << "--" << endl;
            filebin.close();
            while (mt->isBusy()) {
                ;
            } // wait until all data are processed from the queues
            if (of)
                fclose(of);

            mt->writeImage(imgfname);
            mt->clearImage();

            std::time(&end_time);
            cout << std::ctime(&end_time) << endl;

        } else
            cout << "Could not open " << fname << " for reading " << endl;
    }

    return 0;
}
