// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#include "sls/ansi.h"
#include <iostream>
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
#include "jungfrauLGADStrixelsData_new.h"
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

#include <ctime>
//using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 5) {
      std::cout << "Usage is " << argv[0]
             << "indir outdir fname(no extension) fextension [runmin] [runmax] [pedfile (raw or tiff)] [threshold] "
                "[nframes] [xmin xmax ymin ymax] [gainmap]"
		<< std::endl;
      std::cout << "threshold <0 means analog; threshold=0 means cluster finder; "
                "threshold>0 means photon counting"
		<< std::endl;
      std::cout << "nframes <0 means sum everything; nframes=0 means one file per "
                "run; nframes>0 means one file every nframes"
		<< std::endl;
        return 1;
    }

    int fifosize = 1000;
    int nthreads = 10;
    int csize = 3; //3
    int nsigma = 5;
    int nped = 10000;

    int cf = 0;

    double *gainmap = NULL;
    //float *gm;

    int ff, np;
    // cout << " data size is " << dsize;

    ifstream filebin;
    char *indir = argv[1];
    char *outdir = argv[2];
    char *fformat = argv[3];
    char *fext = argv[4];
    int runmin = 0;

    // cout << "argc is " << argc << endl;
    if (argc >= 6) {
        runmin = atoi(argv[5]);
    }

    int runmax = runmin;

    if (argc >= 7) {
        runmax = atoi(argv[6]);
    }

    char *pedfile = NULL;
    if (argc >= 8) {
        pedfile = argv[7];
    }
    double thr = 0;
    double thr1 = 1;

    if (argc >= 9) {
        thr = atof(argv[8]);
    }

    int nframes = 0;

    if (argc >= 10) {
        nframes = atoi(argv[9]);
    }

    char ffname[10000];
    char fname[10000];
    char imgfname[10000];
    char cfname[10000];


    //Define decoders...
#if !defined JFSTRX && !defined JFSTRXOLD && !defined JFSTRXCHIP1 && !defined JFSTRXCHIP6
#ifndef MODULE
    jungfrauHighZSingleChipData *decoder = new jungfrauHighZSingleChipData();
    int nx = 256, ny = 256;
#endif
#ifdef MODULE
    jungfrauModuleData *decoder = new jungfrauModuleData();
    int nx = 1024, ny = 512;
#endif
#endif

#ifdef JFSTRX
    cout << "Jungfrau strixel full module readout" << endl;
    //ROI
    uint16_t xxmin=0;
    uint16_t xxmax=0;
    uint16_t yymin=0;
    uint16_t yymax=0;

#ifndef ALDO
    using header = sls::defs::sls_receiver_header;
    //check if there is a roi in the header
    typedef struct {
      uint16_t xmin;
      uint16_t xmax;
      uint16_t ymin;
      uint16_t ymax;
    } receiverRoi_compact;
    receiverRoi_compact croi;
    sprintf(ffname, "%s/%s.%s", indir, fformat, fext);
    sprintf(fname, (const char*)ffname, runmin);
    std::cout << "Reading header of file " << fname << " to check for ROI " << std::endl; 
    filebin.open((const char *)(fname), ios::in | ios::binary);
    if (filebin.is_open()) {
      header hbuffer;
      std::cout << "sizeof(header) = " << sizeof(header) << std::endl;
      if ( filebin.read( (char *)&hbuffer, sizeof(header) ) ) {
	memcpy(&croi, &hbuffer.detHeader.detSpec1, 8);
	std::cout << "Read ROI [" << croi.xmin << ", " << croi.xmax << ", " << croi.ymin << ", " << croi.ymax << "]" << std::endl;
	xxmin = croi.xmin;
	xxmax = croi.xmax;
	yymin = croi.ymin;
	yymax = croi.ymax;
      } else
	std::cout << "reading error" << std::endl;
      filebin.close();
    } else
      std::cout << "Could not open " << fname << " for reading " << std::endl;
#endif

    jungfrauLGADStrixelsData *decoder = new jungfrauLGADStrixelsData( xxmin, xxmax, yymin, yymax );
    int nx = 1024/3, ny = 512*5;
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
    std::cout << "Detector size is " << nx << " " << ny << std::endl;


    int xmin = 0, xmax = nx, ymin = 0, ymax = ny;
    if (argc >= 14) {
        xmin = atoi(argv[10]);
        xmax = atoi(argv[11]);
        ymin = atoi(argv[12]);
        ymax = atoi(argv[13]);
    }
    std::cout << xmin << " " << xmax << " " << ymin << " " << ymax << " " << std::endl;

    char *gainfname = NULL;
    if (argc > 14) {
        gainfname = argv[14];
	std::cout << "Gain map file name is: " << gainfname << std::endl;
    }

    std::time_t end_time;

    FILE *of = NULL;
    std::cout << "input directory is " << indir << std::endl;
    std::cout << "output directory is " << outdir << std::endl;
    std::cout << "input file is " << fformat << std::endl;
    std::cout << "runmin is " << runmin << std::endl;
    std::cout << "runmax is " << runmax << std::endl;
    if (pedfile)
      std::cout << "pedestal file is " << pedfile << std::endl;
    if (thr > 0)
      std::cout << "threshold is " << thr << std::endl;
    std::cout << "Nframes is " << nframes << std::endl;

    //std::cout << "HHHEEEEEEEEEEEEEEEEEEEEEEERE!!!!!" << std::endl;
    uint32_t nnx, nny;



    singlePhotonDetector *filter = new singlePhotonDetector(
        decoder, 3, nsigma, 1, NULL, nped, 200, -1, -1, gainmap, NULL);

    if (gainfname) {

        if (filter->readGainMap(gainfname))
	  std::cout << "using gain map " << gainfname << std::endl;
        else
	  std::cout << "Could not open gain map " << gainfname << std::endl;
    } else
        thr = 0.15 * thr;
    filter->newDataSet();
    //int dsize = decoder->getDataSize();

    if (thr > 0) {
      std::cout << "threshold is " << thr << std::endl;
      filter->setThreshold(thr);
      cf = 0;

    } else
        cf = 1;

    filter->setROI(xmin, xmax, ymin, ymax);
    std::time(&end_time);
    std::cout << std::ctime(&end_time) << std::endl;

    char *buff;

    // multiThreadedAnalogDetector *mt=new
    // multiThreadedAnalogDetector(filter,nthreads,fifosize);
    multiThreadedCountingDetector *mt =
        new multiThreadedCountingDetector(filter, nthreads, fifosize);
    mt->setClusterSize(csize,csize);

#ifndef ANALOG
    mt->setDetectorMode(ePhotonCounting);
    std::cout << "Counting!" << std::endl;
    if (thr > 0) {
        cf = 0;
    }
#endif
//{
#ifdef ANALOG
    mt->setDetectorMode(eAnalog);
    std::cout << "Analog!" << std::endl;
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

    if (pedfile) {

      if (string(pedfile).find(".dat") != std::string::npos) {
	pos1=string(pedfile).rfind("/");
	strcpy(froot,pedfile+pos1);
	pos=string(froot).find(".dat");
	froot[pos]='\0';
      }

      std::cout << "PEDESTAL " << std::endl;
      // sprintf(imgfname, "%s/pedestals.tiff", outdir);

      if (string(pedfile).find(".tif") == std::string::npos) {
	sprintf(fname, "%s", pedfile);
	std::cout << fname << std::endl;
	std::time(&end_time);
	std::cout << "aaa" << std::ctime(&end_time) << std::endl;
	
	mt->setFrameMode(ePedestal);
	// sprintf(fn,fformat,irun);
	filebin.open((const char *)(fname), ios::in | ios::binary);
	//      //open file
	if (filebin.is_open()) {
	  std::cout << "bbbb" << std::ctime(&end_time) << std::endl;
	  
	  ff = -1;
	  while (decoder->readNextFrame(filebin, ff, np, buff)) {
	    // if (np == 40) {
	    if ((ifr+1) % 100 == 0) {
	      std::cout << " ****" << decoder->getValue(buff,20,20);// << endl;
	    }
	    mt->pushData(buff);
	    mt->nextThread();
	    mt->popFree(buff);
	    ifr++;
	    if (ifr % 100 == 0) {
	      std::cout << " ****" << ifr << " " << ff << " " << np << std::endl;
	    } //else
	    //cout << ifr << " " << ff << " " << np << endl;
	    if (ifr>=1000)
	      break;
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
	  std::cout << "Could not open pedestal file " << fname
		    << " for reading " << std::endl;
      } else {
	float *pp = ReadFromTiff(pedfile, nny, nnx);
	if (pp && (int)nnx == nx && (int)nny == ny) {
	  for (int i = 0; i < nx * ny; i++) {
	    ped[i] = pp[i];
	  }
	  delete[] pp;
	  mt->setPedestal(ped);
	  std::cout << "Pedestal set from tiff file " << pedfile << std::endl;
	} else {
	  std::cout << "Could not open pedestal tiff file " << pedfile
		    << " for reading " << std::endl;
	}
      }
      std::time(&end_time);
      std::cout << std::ctime(&end_time) << std::endl;
    }
    
    ifr = 0;
    int ifile = 0;

    mt->setFrameMode(eFrame);

    for (int irun = runmin; irun <= runmax; irun++) {
      std::cout << "DATA ";
      // sprintf(fn,fformat,irun);
      sprintf(ffname, "%s/%s.%s", indir, fformat, fext);
      sprintf(fname, (const char*)ffname, irun);
      sprintf(ffname, "%s/%s.tiff", outdir, fformat);
      sprintf(imgfname, (const char*)ffname, irun);
      sprintf(ffname, "%s/%s.clust", outdir, fformat);
      sprintf(cfname, (const char*)ffname, irun);
      std::cout << fname << " ";
      std::cout << imgfname << std::endl;
      std::time(&end_time);
      std::cout << std::ctime(&end_time) << std::endl;
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
	      std::cout << "file pointer set " << std::endl;
	    } else {
	      std::cout << "Could not open " << cfname << " for writing "
			<< std::endl;
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
	    std::cout << " ****" << decoder->getValue(buff,20,20);// << endl;
	  }
	  mt->pushData(buff);
	  // 	//         //pop
	  mt->nextThread();
	  mt->popFree(buff);
	  
	  ifr++;
	  if (ifr % 100 == 0)
	    std::cout << " " << ifr << " " << ff << std::endl;
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
	std::cout << "--" << std::endl;
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
	  std::cout << "Writing tiff to " << imgfname << " " << thr1 << std::endl;
	  mt->writeImage(imgfname, thr1);
	  mt->clearImage();
	  if (of) {
	    fclose(of);
	    of = NULL;
	    mt->setFilePointer(NULL);
	  }
	}
	std::time(&end_time);
	std::cout << std::ctime(&end_time) << std::endl;
      } else
	std::cout << "Could not open " << fname << " for reading " << std::endl;
    }
    if (nframes < 0) {
      sprintf(ffname, "%s/%s.tiff", outdir, fformat);
      strcpy(imgfname, ffname);
      std::cout << "Writing tiff to " << imgfname << " " << thr1 << std::endl;
      mt->writeImage(imgfname, thr1);
    }
    
    return 0;
}
