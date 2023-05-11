// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
// #include "sls/ansi.h"
#include <iostream>
#undef CORR

#define C_GHOST 0.0004

#define CM_ROWS 50

#define RAWDATA

#if !defined JFSTRX && !defined JFSTRXOLD && !defined JFSTRXCHIP1 &&           \
    !defined JFSTRXCHIP6
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
#include <fmt/core.h>

std::string getRootString( const std::string filepath ) {
  size_t pos1;
  if (filepath.find("/") == std::string::npos )
    pos1 = 0;
  else
    pos1 = filepath.find_last_of("/")+1;
  size_t pos2 = filepath.find_last_of(".");
  //std::cout << "pos1 " << pos1 << " pos2 " << pos2 << " size " << filepath.length() << std::endl;
  return filepath.substr( pos1, pos2-pos1 );
}

//Create file name string
//   dir:     directory
//   fprefix:   fileprefix (without extension)
//   fsuffix:   filesuffix (for output files, e.g. "ped")
//   fext:    file extension (e.g. "raw")
std::string createFileName( const std::string dir, std::string fprefix="run", std::string fsuffix="", std::string fext="raw", int outfilecounter=-1 ) {
  std::string filename{};
  if (outfilecounter >= 0)
    filename = fmt::format("{:s}/{:s}_{:s}_f{:05d}.{:s}", dir, fprefix, fsuffix, outfilecounter, fext);
  else if (fsuffix.length()!=0)
    filename = fmt::format("{:s}/{:s}_{:s}.{:s}", dir, fprefix, fsuffix, fext);
  else
    filename = fmt::format("{:s}/{:s}.{:s}", dir, fprefix, fext);
  return filename;
}


//NOTE THAT THE DATA FILES HAVE TO BE IN THE RIGHT ORDER SO THAT PEDESTAL TRACKING WORKS!
int main(int argc, char *argv[]) {

    if (argc < 10) {
        std::cout
            << "Usage is " << argv[0]
            << "filestxt outdir [pedfile (raw or tiff)] [xmin xmax ymin ymax] "
               "[threshold] [nframes] "
	       "NOTE THAT THE DATA FILES HAVE TO BE IN THE RIGHT ORDER SO THAT PEDESTAL TRACKING WORKS! "
            << std::endl;
        std::cout
            << "threshold <0 means analog; threshold=0 means cluster finder; "
               "threshold>0 means photon counting"
            << std::endl;
        std::cout
            << "nframes <0 means sum everything; nframes=0 means one file per "
               "run; nframes>0 means one file every nframes"
            << std::endl;
        return 1;
    }

    int fifosize = 1000;
    int nthreads = 10;
    int csize = 3; // 3
    int nsigma = 5;
    int nped = 10000;

    int cf = 0;

    double *gainmap = NULL;
    // float *gm;

    int ff, np;
    // cout << " data size is " << dsize;

    std::string txtfilename(argv[1]);
    std::string outdir(argv[2]);
    std::string pedfilename(argv[3]);

    int xmin = atoi(argv[4]);
    int xmax = atoi(argv[5]);
    int ymin = atoi(argv[6]);
    int ymax = atoi(argv[7]);

    double thr = 0;
    double thr1 = 1;
    thr = atof(argv[8]);

    int nframes = 0;
    nframes = atoi(argv[9]);

    //Get vector of filenames from input txt-file
    std::vector<std::string> filenames{};
    { //Safety scope for ifstream
      ifstream inputs( txtfilename, std::ios::in );
      if (inputs.is_open()) {
	std::cout << "Reading imput filenames from txt-file ..." << std::endl;
	while (!inputs.eof()) {
	  std::string line{};
	  std::getline(inputs, line);
	  filenames.push_back(line);
	}
	std::cout << "---- Reached end of txt-file. ----" << std::endl;
	if (filenames.size()>0) {
	  std::cout << filenames.size() << " filenames found in " << txtfilename << std::endl;
	  std::cout << "The files will be processed in the same order as found in the txt-file." << std::endl;
	} else {
	  std::cout << "No files found in txt-file!" << std::endl;
	  return 1;
	}
      } else
	std::cout << "Could not open " << txtfilename << std::endl;
    }
    
    // Define decoders...
#if !defined JFSTRX && !defined JFSTRXOLD && !defined JFSTRXCHIP1 &&           \
    !defined JFSTRXCHIP6
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
    // ROI
    uint16_t xxmin = 0;
    uint16_t xxmax = 0;
    uint16_t yymin = 0;
    uint16_t yymax = 0;

#ifndef ALDO
    { //THIS SCOPE IS IMPORTANT! (To ensure proper destruction of ifstream)
      using header = sls::defs::sls_receiver_header;
      // check if there is a roi in the header
      typedef struct {
        uint16_t xmin;
        uint16_t xmax;
        uint16_t ymin;
        uint16_t ymax;
      } receiverRoi_compact;
      receiverRoi_compact croi;
      //std::string filepath(argv[9]); //This is a problem if the input files have different ROIs!
      std::cout << "Reading header of file " << filenames[0] << " to check for ROI "
		<< std::endl;
      ifstream firstfile(filenames[0], ios::in | ios::binary);
      if (firstfile.is_open()) {
        header hbuffer;
        std::cout << "sizeof(header) = " << sizeof(header) << std::endl;
        if (firstfile.read((char *)&hbuffer, sizeof(header))) {
	  memcpy(&croi, &hbuffer.detHeader.detSpec1, 8);
	  std::cout << "Read ROI [" << croi.xmin << ", " << croi.xmax << ", "
		    << croi.ymin << ", " << croi.ymax << "]" << std::endl;
	  xxmin = croi.xmin;
	  xxmax = croi.xmax;
	  yymin = croi.ymin;
	  yymax = croi.ymax;
        } else
	  std::cout << "reading error" << std::endl;
        firstfile.close();
      } else
        std::cout << "Could not open " << filenames[0] << " for reading " << std::endl;
    } //end of protective scope
#endif

    jungfrauLGADStrixelsData *decoder =
        new jungfrauLGADStrixelsData(xxmin, xxmax, yymin, yymax);
    int nx = 1024 / 3, ny = 512 * 5;
#endif
#ifdef JFSTRXCHIP1
    std::cout << "Jungfrau strixel LGAD single chip 1" << std::endl;
    jungfrauLGADStrixelsDataSingleChip *decoder =
        new jungfrauLGADStrixelsDataSingleChip(1);
    int nx = 256 / 3, ny = 256 * 5;
#endif
#ifdef JFSTRXCHIP6
    std::cout << "Jungfrau strixel LGAD single chip 6" << std::endl;
    jungfrauLGADStrixelsDataSingleChip *decoder =
        new jungfrauLGADStrixelsDataSingleChip(6);
    int nx = 256 / 3, ny = 256 * 5;
#endif
#ifdef JFSTRXOLD
    std::cout << "Jungfrau strixels old design" << std::endl;
    jungfrauStrixelsHalfModuleOldDesign *decoder =
        new jungfrauStrixelsHalfModuleOldDesign();
    int nx = 1024 * 3, ny = 512 / 3;
#endif

    decoder->getDetectorSize(nx, ny);
    std::cout << "Detector size is " << nx << " " << ny << std::endl;


    if ( xmin == xmax ) {
      xmin = 0;
      xmax = nx;
    }
    if ( ymin == ymax ) {
      ymin = 0;
      ymax = ny;
    }
    std::cout << xmin << " " << xmax << " " << ymin << " " << ymax << " "
              << std::endl;

    /*
    char *gainfname = NULL;
    if (argc > 14) {
        gainfname = argv[14];
        std::cout << "Gain map file name is: " << gainfname << std::endl;
    }
    */

    std::time_t end_time;

    std::cout << "output directory is " << outdir << std::endl;
    if (pedfilename.length()!=0)
        std::cout << "pedestal file is " << pedfilename << std::endl;
    if (thr > 0)
        std::cout << "threshold is " << thr << std::endl;
    std::cout << "Nframes is " << nframes << std::endl;

    uint32_t nnx, nny;

    singlePhotonDetector *filter = new singlePhotonDetector(
        decoder, 3, nsigma, 1, NULL, nped, 200, -1, -1, gainmap, NULL);

    /*
    if (gainfname) {

        if (filter->readGainMap(gainfname))
            std::cout << "using gain map " << gainfname << std::endl;
        else
            std::cout << "Could not open gain map " << gainfname << std::endl;
    } else
    */
    thr = 0.15 * thr;
    filter->newDataSet();
    // int dsize = decoder->getDataSize();

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

    multiThreadedCountingDetector *mt =
        new multiThreadedCountingDetector(filter, nthreads, fifosize);
    mt->setClusterSize(csize, csize);

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

    if (pedfilename.length()>1) {

      std::string froot = getRootString(pedfilename);

      std::cout << "PEDESTAL " << std::endl;

        if (pedfilename.find(".tif") == std::string::npos) {
	  std::string fname = pedfilename;
	  std::cout << fname << std::endl;
	  std::time(&end_time);
	  std::cout << "aaa " << std::ctime(&end_time) << std::endl;

	  mt->setFrameMode(ePedestal);

	  ifstream pedefile(fname, ios::in | ios::binary);
	  //      //open file
	  if (pedefile.is_open()) {
	    std::cout << "bbbb " << std::ctime(&end_time) << std::endl;
	    
	    ff = -1;
	    while (decoder->readNextFrame(pedefile, ff, np, buff)) {
	      // if (np == 40) {
	      if ((ifr + 1) % 100 == 0) {
		std::cout
		  << " ****"
		  << decoder->getValue(buff, 20, 20); // << std::endl;
	      }
	      mt->pushData(buff);
	      mt->nextThread();
	      mt->popFree(buff);
	      ifr++;
	      if (ifr % 100 == 0) {
		std::cout << " ****" << ifr << " " << ff << " " << np
			  << std::endl;
	      } // else
	      //std::cout << ifr << " " << ff << " " << np << std::endl;
	      if (ifr >= 1000)
		break;
	      ff = -1;
	    }
	    pedefile.close();
	    while (mt->isBusy()) {
	      ;
	    }

	    std::cout << "froot " << froot << std::endl;
	    auto imgfname = createFileName( outdir, froot, "ped", "tiff");
	    mt->writePedestal(imgfname.c_str());
	    imgfname = createFileName( outdir, froot, "rms", "tiff");
	    mt->writePedestalRMS(imgfname.c_str());

	  } else
	    std::cout << "Could not open pedestal file " << fname
		      << " for reading " << std::endl;
        } else {
	  std::vector<double> ped(nx * ny);
	  float *pp = ReadFromTiff(pedfilename.c_str(), nny, nnx);
	  if (pp && (int)nnx == nx && (int)nny == ny) {
	    for (int i = 0; i < nx * ny; i++) {
	      ped[i] = pp[i];
	    }
	    delete[] pp;
	    mt->setPedestal(ped.data());
	    std::cout << "Pedestal set from tiff file " << pedfilename
		      << std::endl;
	  } else {
	    std::cout << "Could not open pedestal tiff file " << pedfilename
		      << " for reading " << std::endl;
	  }
        }
        std::time(&end_time);
        std::cout << std::ctime(&end_time) << std::endl;
    }

    ifr = 0;
    int ioutfile = 0;

    mt->setFrameMode(eFrame);

    FILE *of = NULL;
    
    //NOTE THAT THE DATA FILES HAVE TO BE IN THE RIGHT ORDER SO THAT PEDESTAL TRACKING WORKS!
    for (unsigned int ifile = 0; ifile != filenames.size(); ++ifile) {
      std::cout << "DATA ";
      //std::string fname(argv[iargc]);
      std::string fsuffix{};
      std::string fprefix = getRootString(filenames[ifile]);
      std::string imgfname = createFileName( outdir, fprefix, fsuffix, "tiff" );
      std::string cfname = createFileName( outdir, fprefix, fsuffix, "clust" );
      std::cout << filenames[ifile] << " ";
      std::cout << imgfname << std::endl;
      std::time(&end_time);
      std::cout << std::ctime(&end_time) << std::endl;

      ifstream filebin(filenames[ifile], ios::in | ios::binary);
      //      //open file
      ifile = 0;
      if (filebin.is_open()) {
	if (thr <= 0 && cf != 0) { // cluster finder
	  if (of == NULL) {
	    of = fopen(cfname.c_str(), "w");
	    if (of) {
	      mt->setFilePointer(of);
	      std::cout << "file pointer set " << std::endl;
	    } else {
	      std::cout << "Could not open " << cfname
			<< " for writing " << std::endl;
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
	  
	  if ((ifr + 1) % 100 == 0) {
	    std::cout << " ****"
		      << decoder->getValue(buff, 20, 20); // << std::endl;
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
	      imgfname = createFileName( outdir, fprefix, fsuffix, "tiff", ioutfile );
	      mt->writeImage(imgfname.c_str(), thr1);
	      mt->clearImage();
	      ioutfile++;
	    }
	  }
	  // } else
	  //std::cout << ifr << " " << ff << " " << np << std::endl;
	  ff = -1;
	}
	std::cout << "--" << std::endl;
	filebin.close();
	while (mt->isBusy()) {
	  ;
	}
	if (nframes >= 0) {
	  if (nframes > 0)
	    imgfname = createFileName( outdir, fprefix, fsuffix, "tiff", ifile );
	  std::cout << "Writing tiff to " << imgfname << " " << thr1
		    << std::endl;
	  mt->writeImage(imgfname.c_str(), thr1);
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
	std::cout << "Could not open " << filenames[ifile] << " for reading "
		  << std::endl;
    }
    if (nframes < 0) {
      //std::string fname(argv[10]);
      auto fprefix = getRootString(filenames[0]); //This might by a non-ideal name choice for that file
      auto imgfname = createFileName( outdir, fprefix, "sum", "tiff" );
      std::cout << "Writing tiff to " << imgfname << " " << thr1 << std::endl;
      mt->writeImage(imgfname.c_str(), thr1);
    }

    return 0;
}
