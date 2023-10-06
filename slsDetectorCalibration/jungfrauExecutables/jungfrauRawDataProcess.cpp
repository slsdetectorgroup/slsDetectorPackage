// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
// #include "sls/ansi.h"
#include <iostream>
#undef CORR

#define C_GHOST 0.0004

#define CM_ROWS 50

#define RAWDATA

#if !defined JFSTRX && !defined JFSTRXOLD && !defined JFSTRXCHIP1 &&           \
    !defined JFSTRXCHIP6 && !defined CHIP
#ifndef MODULE
#include "jungfrauHighZSingleChipData.h"
#endif
#ifdef MODULE
#include "jungfrauModuleData.h"
#endif
#endif

#ifdef CHIP
#include "jungfrauSingleChipData.h"
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

#include <nlohmann/json.hpp>
using json = nlohmann::json;


std::string getRootString( const std::string& filepath ) {
  size_t pos1 = filepath.find_last_of("/");
  size_t pos2 = filepath.find_last_of(".");
  std::cout << "pos1 " << pos1 << " pos2 " << pos2 << " size " << filepath.length() << std::endl;
  return filepath.substr( pos1+1, pos2-pos1-1 );
}

//Create file name string according to slsDetectorPackage format
//   dir:     directory
//   fprefix:   fileprefix (without extension)
//   fsuffix:   filesuffix (for output files, e.g. "ped")
//   fext:    file extension (e.g. "raw")
//   mindex:  module index ("d0" in standard)
//   findex:  file index for one acquisition ("f0")
//   aindex:  acquisition index (i.e. "run number")

std::string createFileName( const std::string& dir, const std::string& fprefix="run", const std::string& fsuffix="", const std::string& fext="raw", int aindex=0, int mindex=0, int findex=0, int outfilecounter=-1 ) {
  if (outfilecounter >= 0)
    return fmt::format("{:s}/{:s}_d{:d}_f{:d}_{:d}_f{:05d}.{:s}", dir, fprefix, mindex, findex, aindex, outfilecounter, fext);
  else if (fsuffix.length()!=0) {
    if (fsuffix == "master")
      return fmt::format("{:s}/{:s}_master_{:d}.{:s}", dir, fprefix, aindex, fext);
    else
      return fmt::format("{:s}/{:s}_{:s}.{:s}", dir, fprefix, fsuffix, fext);
  }
  else
    return fmt::format("{:s}/{:s}_d{:d}_f{:d}_{:d}.{:s}", dir, fprefix, mindex, findex, aindex, fext);
}

int main(int argc, char *argv[]) {

    if (argc < 6) {
        std::cout
            << "Usage is " << argv[0]
            << "indir outdir [fprefix(excluding slsDetector standard suffixes and extension)] [fextension] "
	       "[fmin] [fmax] [runmin] [runmax] [pedfile (raw or tiff)] [threshold] "
               "[nframes] [xmin xmax ymin ymax] [optional: bool read rxroi from data file header] [gainmap]"
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

    //ifstream filebin;
    std::string indir(argv[1]);
    std::string outdir(argv[2]);
    std::string fprefix(argv[3]);
    std::string fext(argv[4]);

    int fmin = 0;
    if (argc >= 6)
        fmin = atoi(argv[5]);
    int fmax = fmin;
    if (argc >= 7)
        fmax = atoi(argv[6]);

    int runmin = 0;
    // cout << "argc is " << argc << endl;
    if (argc >= 8) {
        runmin = atoi(argv[7]);
    }
    int runmax = runmin;
    if (argc >= 9) {
        runmax = atoi(argv[8]);
    }

    std::string pedfilename{};
    if (argc >= 10) {
        pedfilename = argv[9];
    }
    double thr = 0;
    double thr1 = 1;

    if (argc >= 11) {
        thr = atof(argv[10]);
    }

    int nframes = 0;

    if (argc >= 12) {
        nframes = atoi(argv[11]);
    }

    bool readrxroifromdatafile = false;
    if (argc >= 17)
      readrxroifromdatafile = atoi(argv[16]);
    
    // Receiver ROI
    uint16_t rxroi_xmin = 0;
    uint16_t rxroi_xmax = 0;
    uint16_t rxroi_ymin = 0;
    uint16_t rxroi_ymax = 0;

    { //protective scope so ifstream gets destroyed properly

      auto jsonmastername = createFileName( indir, fprefix, "master", "json", runmin );
      std::cout << "json master file " << jsonmastername << std::endl;
      std::ifstream masterfile(jsonmastername); //, ios::in | ios::binary);
      if (masterfile.is_open()) {
	json j;
	masterfile >> j;
	rxroi_xmin = j["Receiver Roi"]["xmin"];
	rxroi_xmax = j["Receiver Roi"]["xmax"];
	rxroi_ymin = j["Receiver Roi"]["ymin"];
	rxroi_ymax = j["Receiver Roi"]["ymax"];
	masterfile.close();
	std::cout << "Read Receiver ROI [" << rxroi_xmin << ", " << rxroi_xmax << ", "
		  << rxroi_ymin << ", " << rxroi_ymax << "] from json master file" << std::endl;
      } else 
	std::cout << "Could not open master file " << jsonmastername << std::endl;
    
    }

    // Define decoders...
#if !defined JFSTRX && !defined JFSTRXOLD && !defined JFSTRXCHIP1 &&           \
    !defined JFSTRXCHIP6 && !defined CHIP
#ifndef MODULE
    jungfrauHighZSingleChipData *decoder = new jungfrauHighZSingleChipData();
    int nx = 256, ny = 256;
#endif
#ifdef MODULE
    jungfrauModuleData *decoder = new jungfrauModuleData(rxroi_xmin, rxroi_xmax, rxroi_ymin, rxroi_ymax);
    int nx = 1024, ny = 512;
#endif
#endif

#ifdef CHIP
    std::cout << "Jungfrau pixel module single chip readout" << std::endl;
    jungfrauSingleChipData *decoder = new jungfrauSingleChipData();
    int nx = 256, ny = 256;
#endif

#ifdef JFSTRX
    std::cout << "Jungfrau strixel full module readout" << std::endl;

#ifndef ALDO
    if (readrxroifromdatafile)
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
	std::string fsuffix{};
	auto filename = createFileName( indir, fprefix, fsuffix, fext, runmin );
	std::cout << "Reading header of file " << filename << " to check for ROI "
		  << std::endl;
	ifstream firstfile(filename, ios::in | ios::binary);
	if (firstfile.is_open()) {
	  header hbuffer;
	  std::cout << "sizeof(header) = " << sizeof(header) << std::endl;
	  if (firstfile.read((char *)&hbuffer, sizeof(header))) {
	    memcpy(&croi, &hbuffer.detHeader.detSpec1, 8);
	    std::cout << "Read ROI [" << croi.xmin << ", " << croi.xmax << ", "
		      << croi.ymin << ", " << croi.ymax << "]" << std::endl;
	    rxroi_xmin = croi.xmin;
	    rxroi_xmax = croi.xmax;
	    rxroi_ymin = croi.ymin;
	    rxroi_ymax = croi.ymax;
	  } else
	    std::cout << "reading error" << std::endl;
	  firstfile.close();
	} else
	  std::cout << "Could not open " << filename << " for reading " << std::endl;
      } //end of protective scope
#endif

    jungfrauLGADStrixelsData *decoder =
      new jungfrauLGADStrixelsData(rxroi_xmin, rxroi_xmax, rxroi_ymin, rxroi_ymax);
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

    //Cluster finder ROI
    int xmin = 0, xmax = nx-1, ymin = 0, ymax = ny-1;
    if (argc >= 16) {
        xmin = atoi(argv[12]);
        xmax = atoi(argv[13]);
        ymin = atoi(argv[14]);
        ymax = atoi(argv[15]);
    }
    std::cout << "Cluster finder ROI: [" << xmin << ", " << xmax << ", " << ymin << ", " << ymax << "]"
              << std::endl;

    char *gainfname = NULL;
    if (argc > 17) {
        gainfname = argv[17];
        std::cout << "Gain map file name is: " << gainfname << std::endl;
    }

    std::time_t end_time;

    std::cout << "input directory is " << indir << std::endl;
    std::cout << "output directory is " << outdir << std::endl;
    std::cout << "input file prefix is " << fprefix << std::endl;
    std::cout << "fmin is " << fmin << std::endl;
    std::cout << "fmax is " << fmax << std::endl;
    std::cout << "runmin is " << runmin << std::endl;
    std::cout << "runmax is " << runmax << std::endl;
    if (pedfilename.length()!=0)
        std::cout << "pedestal file is " << pedfilename << std::endl;
    if (thr > 0)
        std::cout << "threshold is " << thr << std::endl;
    std::cout << "Nframes is " << nframes << std::endl;

    // std::cout << "HHHEEEEEEEEEEEEEEEEEEEEEEERE!!!!!" << std::endl;
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

    if (pedfilename.length()!=0) {

      std::string froot = getRootString(pedfilename);

      std::cout << "PEDESTAL " << std::endl;

        if (pedfilename.find(".tif") == std::string::npos) {
	  std::string fname = pedfilename;
	  std::cout << fname << std::endl;
	  std::time(&end_time);
	  std::cout << "aaa " << std::ctime(&end_time) << std::endl;

	  mt->setFrameMode(ePedestal);

	  std::ifstream pedefile(fname, ios::in | ios::binary);
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
    
    for (int irun = runmin; irun <= runmax; ++irun) {
      for (int ifile = fmin; ifile <= fmax; ++ifile) {
        std::cout << "DATA ";
	std::string fsuffix{};
	auto fname = createFileName( indir, fprefix, fsuffix, fext, irun, 0, ifile );
	auto imgfname = createFileName( outdir, fprefix, fsuffix, "tiff", irun, 0, ifile );
	auto cfname = createFileName( outdir, fprefix, fsuffix, "clust", irun, 0, ifile );
        std::cout << fname << " ";
        std::cout << imgfname << std::endl;
        std::time(&end_time);
        std::cout << std::ctime(&end_time) << std::endl;
        //  std::cout <<  fname << " " << outfname << " " << imgfname <<  std::endl;
	std::ifstream filebin(fname, ios::in | ios::binary);
        //      //open file
        ioutfile = 0;
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
		      imgfname = createFileName( outdir, fprefix, fsuffix, "tiff", irun, 0, 0, ioutfile );
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
		  imgfname = createFileName( outdir, fprefix, fsuffix, "tiff", irun, 0, 0, ioutfile );
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
            std::cout << "Could not open " << fname << " for reading "
                      << std::endl;
      }
    }
    if (nframes < 0) {
      auto imgfname = createFileName( outdir, fprefix, "sum", "tiff", runmin, 0, fmin, -1 );
        std::cout << "Writing tiff to " << imgfname << " " << thr1 << std::endl;
        mt->writeImage(imgfname.c_str(), thr1);
    }

    return 0;
}
