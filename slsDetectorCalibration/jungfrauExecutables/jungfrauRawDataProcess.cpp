// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#include "sls/ansi.h"
#include <iostream>

//enable common mode subtraction
#define CMS
//disable common mode subtraction
//#undef CMS
#undef CORR
#define C_GHOST 0.0004

#define CM_ROWS 50

#define RAWDATA

#if !defined JFSTRX && !defined JFSTRXOLD && !defined JFSTRXCHIP1 && !defined JFSTRXCHIP6
#ifndef MODULE
#include "jungfrauHighZSingleChipData.h"
#endif
#ifdef MODULE
#include "jungfrauModuleData.h"
#endif
#endif

#ifdef JFSTRX
#include "jungfrauLGADStrixelsData.h"
#endif
#if defined JFSTRXCHIP1 || defined JFSTRXCHIP6
#include "jungfrauLGADStrixelsDataSingleChip.h"
#endif
#ifdef JFSTRXOLD
#include "jungfrauStrixelsHalfModuleOldDesign.h"
#endif

#include "multiThreadedCountingDetector.h"
#include "singlePhotonDetector.h"

#include <fstream>
#include <map>
#include <stdio.h>
#include <sys/stat.h>
//#include "commonModeSubtractionNew.h"
#include "jungfrauCommonMode.h"

#include <ctime>
using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 6) {
        cout << "Usage is " << argv[0]
             << " indir outdir fname(no extension) fextension csize [runmin] [runmax] [pedfile (raw or tiff)] [threshold]"
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

    int fifosize = 1000;
    int nthreads = 1;
    int csize = 3; //3
    int nsigma = 5;
    int nped = 10000;

    int cf = 0;


#if !defined JFSTRX && !defined JFSTRXOLD && !defined JFSTRXCHIP1 && !defined JFSTRXCHIP6
#ifndef MODULE
    cout << "This is a JF single chip!" <<endl;
    jungfrauHighZSingleChipData *decoder = new jungfrauHighZSingleChipData();
    int nx = 256, ny = 256;
#endif
#ifdef MODULE
    cout << "This is a JF module!" <<endl;
    jungfrauModuleData *decoder = new jungfrauModuleData();
    int nx = 1024, ny = 512;
#endif
#endif

#ifdef JFSTRX
    cout << "Jungfrau strixel full module readout" << endl;
    jungfrauLGADStrixelsData *decoder = new jungfrauLGADStrixelsData();
    int nx = 1024/5, ny = 512*5;
#endif 
#ifdef JFSTRXCHIP1
    std::cout << "Jungfrau strixel LGAD single chip 1" << std::endl;
    jungfrauLGADStrixelsDataSingleChip *decoder = new jungfrauLGADStrixelsDataSingleChip(1);
    int nx = 256/3, ny = 256*5;
#endif
#ifdef JFSTRXCHIP6
    std::cout << "Jungfrau strixel LGAD single chip 6" << std::endl;
    jungfrauLGADStrixelsDataSingleChip *decoder = new jungfrauLGADStrixelsDataSingleChip(6);
    int nx = 256/3, ny = 256*5;
#endif
#ifdef JFSTRXOLD
    std::cout << "Jungfrau strixels old design" << std::endl;
    jungfrauStrixelsHalfModuleOldDesign *decoder = new jungfrauStrixelsHalfModuleOldDesign();
    int nx = 1024*3, ny = 512/3;
#endif


    decoder->getDetectorSize(nx, ny);
    
    cout << "Detector size is " << nx << " " << ny << endl;

    double *gainmap = NULL;
    //float *gm;

    int ff, np;
    // cout << " data size is " << dsize;

    ifstream filebin;
    char *indir = argv[1];
    char *outdir = argv[2];
    char *fformat = argv[3];
    char *fext = argv[4];
    sscanf(argv[5], "%d", &csize);
    int runmin = 0;

    // cout << "argc is " << argc << endl;
    if (argc >= 7) {
        runmin = atoi(argv[6]);
    }

    int runmax = runmin;

    if (argc >= 8) {
        runmax = atoi(argv[7]);
    }

    char *pedfile = NULL;
    if (argc >= 9) {
        pedfile = argv[8];
    }
    double thr = 0;
    double thr1 = 1;

    if (argc >= 10) {
        thr = atof(argv[9]);
    }

    int nframes = 0;

    if (argc >= 11) {
        nframes = atoi(argv[10]);
    }

    int xmin = 0, xmax = nx, ymin = 0, ymax = ny;
    if (argc >= 15) {
        xmin = atoi(argv[11]);
        xmax = atoi(argv[12]);
        ymin = atoi(argv[13]);
        ymax = atoi(argv[14]);
    }

    char *gainfname = NULL;
    if (argc > 15) {
        gainfname = argv[15];
        cout << "Gain map file name is: " << gainfname << endl;
    }

    char ffname[10000];
    char fname[10000];
    char imgfname[10000];
    char cfname[10000];

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
    commonModeSubtraction *cm = NULL;
#ifdef CMS
    cm = new commonModeSubtractionChip();//commonModeSubtraction(1,5);//commonModeSubtractionSuperColumnJF();
    std::cout << "Enabled common mode subtraction" << std::endl;
#endif
    singlePhotonDetector *filter = new singlePhotonDetector(
        decoder, 3, nsigma, 1, cm, nped, 200, -1, -1, gainmap, NULL);

    if (gainfname) {

        if (filter->readGainMap(gainfname))
            cout << "using gain map " << gainfname << endl;
        else
            cout << "Could not open gain map " << gainfname << endl;
    } else
        thr = 0.15 * thr;
    filter->newDataSet();
    //int dsize = decoder->getDataSize();

    if (thr > 0) {
        cout << "threshold is " << thr << endl;
        filter->setThreshold(thr);
        cf = 0;

    } else
        cf = 1;

    filter->setROI(xmin, xmax, ymin, ymax);
    std::time(&end_time);
    cout << std::ctime(&end_time) << endl;

    char *buff;

    // multiThreadedAnalogDetector *mt=new
    // multiThreadedAnalogDetector(filter,nthreads,fifosize);
    multiThreadedCountingDetector *mt =
        new multiThreadedCountingDetector(filter, nthreads, fifosize);
    mt->setClusterSize(csize,csize);

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

    char froot[1000];
    double *ped=new double[nx * ny];//, *ped1;

    int pos,pos1;
    int ipixX=351, ipixY=331;

    if (pedfile) {

      if (string(pedfile).find(".raw") != std::string::npos) {
	pos1=string(pedfile).rfind("/");
	strcpy(froot,pedfile+pos1);
	pos=string(froot).find(".raw");
	froot[pos]='\0';
      }

        cout << "PEDESTAL " << endl;
	// sprintf(imgfname, "%s/pedestals.tiff", outdir);

        if (string(pedfile).find(".tif") == std::string::npos) {
            sprintf(fname, "%s", pedfile);
            cout << fname << endl;
            std::time(&end_time);
            //cout << "aaa" << std::ctime(&end_time) << endl;

            mt->setFrameMode(ePedestal);
            // sprintf(fn,fformat,irun);
            filebin.open((const char *)(fname), ios::in | ios::binary);
            //      //open file
            if (filebin.is_open()) {
                ff = -1;
                while (decoder->readNextFrame(filebin, ff, np, buff)) {
		  // if (np == 40) {
		  // if ((ifr+1) % 100 == 0) {
		  //   cout << " ****" << decoder->getValue(buff,20,20);// << endl;
		  // }
                        mt->pushData(buff);


			// while (mt->isBusy()) 
			//   ;
			//if ((ifr+1) % 100 == 0)

			// double pix=0,ped=0,sub=0,cmp=0, sub0=0;

			// for (int ix=-1; ix<2; ix++) {
			//   for (int iy=-1; iy<2; iy++) {
			//     pix+=decoder->getValue(buff, ipixX+ix, ipixY+iy);
			//     sub+=filter->subtractPedestal(buff, ipixX+ix, ipixY+iy, 1);
			//     sub0+=filter->subtractPedestal(buff, ipixX+ix, ipixY+iy, 0);
			//     ped+=filter->getPedestal(ipixX+ix, ipixY+iy);
			//     cmp+=filter->getCommonMode(ipixX+ix, ipixY+iy);
			//   }
			// }

			// cout << pix << " " << sub << " " sub0 << " " << ped << " " << cmp  << endl;


                        mt->nextThread();
                        mt->popFree(buff);
                        ifr++;
                        // if (ifr % 100 == 0) {
			//   cout << " ****" << ifr << " " << ff << " " << np << endl;
			// } 

			//else
                        //cout << ifr << " " << ff << " " << np << endl;
			// if (ifr>=1000)
			//   break;
			
                    ff = -1;
                }
                filebin.close();
                while (mt->isBusy()) {
                    ;
                }

		sprintf(imgfname, "%s/%s_ped.tiff", outdir, froot);
		mt->writePedestal(imgfname);
		sprintf(imgfname, "%s/%s_rms.tiff", outdir, froot);
		mt->writePedestalRMS(imgfname);

            } else
                cout << "Could not open pedestal file " << fname
                     << " for reading " << endl;
        } else {
            float *pp = ReadFromTiff(pedfile, nny, nnx);
            if (pp && (int)nnx == nx && (int)nny == ny) {
                for (int i = 0; i < nx * ny; i++) {
                    ped[i] = pp[i];
                }
                delete[] pp;
                mt->setPedestal(ped);
                cout << "Pedestal set from tiff file " << pedfile << endl;
            } else {
                cout << "Could not open pedestal tiff file " << pedfile
                     << " for reading " << endl;
            }
        }
        std::time(&end_time);
        cout << std::ctime(&end_time) << endl;
    }

    ifr = 0;
    int ifile = 0;

    mt->setFrameMode(eFrame);

    for (int irun = runmin; irun <= runmax; irun++) {
        cout << "DATA ";
        // sprintf(fn,fformat,irun);
        sprintf(ffname, "%s/%s.%s", indir, fformat, fext);
        sprintf(fname, (const char*)ffname, irun);
        sprintf(ffname, "%s/%s.tiff", outdir, fformat);
        sprintf(imgfname, (const char*)ffname, irun);
        sprintf(ffname, "%s/%s.clust", outdir, fformat);
        sprintf(cfname, (const char*)ffname, irun);
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
              //  if (np == 40) {
                    //         //push
	      
		  if ((ifr+1) % 100 == 0) {
		    cout << " ****" << decoder->getValue(buff,20,20);// << endl;
		  }
                    mt->pushData(buff);	


		    // while (mt->isBusy()) 

			// double pix=0,ped=0,sub=0,cmp=0, sub0=0;

			// for (int ix=-1; ix<2; ix++) {
			//   for (int iy=-1; iy<2; iy++) {
			//     pix+=decoder->getValue(buff, ipixX+ix, ipixY+iy);
			//     sub+=filter->subtractPedestal(buff, ipixX+ix, ipixY+iy, 1);
			//     sub0+=filter->subtractPedestal(buff, ipixX+ix, ipixY+iy, 0);
			//     ped+=filter->getPedestal(ipixX+ix, ipixY+iy);
			//     cmp+=filter->getCommonMode(ipixX+ix, ipixY+iy);
			//   }
			// }

			// cout << pix << " " << sub << " " sub0 << " " << ped << " " << cmp  << endl;

                    // 	//         //pop
                    mt->nextThread();
                    mt->popFree(buff);

                    ifr++;
                    if (ifr % 100 == 0)
		      cout << " " << ifr << " " << ff << endl;
                    if (nframes > 0) {
                        if (ifr % nframes == 0) {
                            sprintf(ffname, "%s/%s_f%05d.tiff", outdir, fformat,
                                    ifile);
                            sprintf(imgfname, (const char*)ffname, irun);
                            mt->writeImage(imgfname, thr1);
                            mt->clearImage();
                            ifile++;
                        }
                    }
                // } else
                //     cout << ifr << " " << ff << " " << np << endl;
                ff = -1;
            }
            cout << "--" << endl;
            filebin.close();
            while (mt->isBusy()) {
                ;
            }
            if (nframes >= 0) {
                if (nframes > 0) {
                    sprintf(ffname, "%s/%s_f%05d.tiff", outdir, fformat, ifile);
                    sprintf(imgfname, (const char*)ffname, irun);
                } else {
                    sprintf(ffname, "%s/%s.tiff", outdir, fformat);
                    sprintf(imgfname, (const char*)ffname, irun);
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
