// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
// #include "sls/ansi.h"
#include <iostream>
#undef CORR

#define C_GHOST 0.0004

#define CM_ROWS 50

#define RAWDATA


#include "jungfrauLGADStrixelsDataQuadH5.h"

#include "multiThreadedCountingDetector.h"
#include "singlePhotonDetector.h"

#include <fstream>
#include <map>
#include <memory>
#include <stdio.h>
#include <sys/stat.h>

#include <ctime>
#include <fmt/core.h>

/*
#include <nlohmann/json.hpp>
using json = nlohmann::json;
*/


std::string getRootString( std::string const& filepath ) {
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
std::string createFileName( std::string const& dir, std::string const& fprefix="run",
			    std::string const& fsuffix="", std::string const& fext="raw", int const outfilecounter=-1 ) {
  if (outfilecounter >= 0)
    return fmt::format("{:s}/{:s}_{:s}_f{:05d}.{:s}", dir, fprefix, fsuffix, outfilecounter, fext);
  else if (fsuffix.length()!=0)
    return fmt::format("{:s}/{:s}_{:s}.{:s}", dir, fprefix, fsuffix, fext);
  else
    return fmt::format("{:s}/{:s}.{:s}", dir, fprefix, fext);
}


//NOTE THAT THE DATA FILES HAVE TO BE IN THE RIGHT ORDER SO THAT PEDESTAL TRACKING WORKS!
int main(int argc, char *argv[]) {

    if (argc < 4) {
        std::cout
            << "Usage is " << argv[0]
            << " filestxt outdir [pedfile (h5)] optional: [bool validate h5 rank] "
               "[xmin xmax ymin ymax] [threshold] [nframes] "
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

    int const fifosize = 100; //1000;
    int const nthreads = 10;
    int const csize = 3; // 3
    int const nsigma = 5;
    int const nped = 10000;

    int cf = 0;

    std::string const txtfilename(argv[1]);
    std::string const outdir(argv[2]);
    std::string const pedfilename(argv[3]);

    bool validate_rank=true;
    if (argc > 4)
      validate_rank = atoi(argv[4]);

    double thr = 0;
    double thr1 = 1;
    if (argc > 8)
      thr = atof(argv[8]);

    int nframes = 0;
    if (argc > 9)
      nframes = atoi(argv[9]);

    //Get vector of filenames from input txt-file
    std::vector<std::string> filenames{};
    //filenames.reserve(512);
    { //Safety scope for ifstream
      ifstream inputs( txtfilename, std::ios::in );
      if (inputs.is_open()) {
	    std::cout << "Reading imput filenames from txt-file ..." << std::endl;
	    std::string line{};
	    while (!inputs.eof()) {
	      std::getline(inputs, line);
        if(line.find(".h5") != std::string::npos) {
	        filenames.emplace_back(line);
	        std::cout << line << std::endl; //" line.max_size() " << line.max_size() << " filenames.capacity() " << filenames.capacity() << '\n';
        }
	    }
	    inputs.close();
	    std::cout << "---- Reached end of txt-file. ----" << std::endl;
      } else
	      std::cout << "Could not open " << txtfilename << std::endl;
      if (filenames.size()>0) {
	      std::cout << filenames.size() << " filenames found in " << txtfilename << std::endl;
	      std::cout << "The files will be processed in the same order as found in the txt-file." << std::endl;
      } else {
	      std::cout << "No files found in txt-file!" << std::endl;
	      return 1;
      }
    }

    std::cout << "###############" << std::endl;

    // Define decoder
    std::cout << "Jungfrau strixel quad h5" << std::endl;
    jungfrauLGADStrixelsDataQuadH5* decoder = new jungfrauLGADStrixelsDataQuadH5();
    //auto decoder = std::make_unique<jungfrauLGADStrixelsDataQuadH5>();
    int nx = 1024 / 3, ny = 512 * 3;

    //Cluster finder ROI
    int xmin = 0, xmax = nx-1, ymin = 0, ymax = ny-1;
    if (argc > 8) {
      xmin = atoi(argv[5]);
      xmax = atoi(argv[6]);
      ymin = atoi(argv[7]);
      ymax = atoi(argv[8]);
    }
    std::cout << "Cluster finder ROI: [" << xmin << ", " << xmax << ", " << ymin << ", " << ymax << "]"
              << std::endl;

    decoder->getDetectorSize(nx, ny);
    std::cout << "Detector size is " << nx << " " << ny << std::endl;

    std::time_t end_time;

    std::cout << "output directory is " << outdir << std::endl;
    if (pedfilename.length()!=0)
        std::cout << "pedestal file is " << pedfilename << std::endl;
    if (thr > 0)
        std::cout << "threshold is " << thr << std::endl;
    std::cout << "Nframes is " << nframes << std::endl;

    uint32_t nnx, nny;

    singlePhotonDetector* filter =
      new singlePhotonDetector(decoder, 3, nsigma, 1, NULL, nped, 200, -1, -1, NULL, NULL);
    //auto filter = std::make_unique<singlePhotonDetector>(decoder.get(), 3, nsigma, 1, nullptr, nped, 200, -1, -1, nullptr, nullptr);

    thr = 0.15 * thr;
    //filter->newDataSet(); //This only initializes the dataset for the first thread (the other threads are created via cloning)
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

    char* buff;

    multiThreadedCountingDetector* mt =
        new multiThreadedCountingDetector(filter, nthreads, fifosize);
    //auto mt = std::make_unique<multiThreadedCountingDetector>(filter.get(), nthreads, fifosize);
    mt->setClusterSize(csize, csize);
    mt->newDataSet(); //Initialize new dataset for each thread

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

    int ifr = 0; //frame counter of while loop
    int framenumber = -1; //framenumber as read from file (detector)
    int iframe = 0; //frame counter internal to HDF5File::ReadImage (provided for sanity check/debugging)

    if (pedfilename.length()>1) {

      std::string froot = getRootString(pedfilename);

      std::cout << "PEDESTAL " << std::endl;

      if (pedfilename.find(".tif") == std::string::npos) { //not a tiff file
	      std::string const fname(pedfilename);
	      std::cout << fname << std::endl;
	      std::time(&end_time);
	      std::cout << "aaa " << std::ctime(&end_time) << std::endl;

	      mt->setFrameMode(ePedestal);

	      //HDF5File pedefile;
	      auto pedefile = std::make_unique<HDF5File>();
	      //      //open file
	      if ( pedefile->OpenResources(fname.c_str(),validate_rank) ) {
	        std::cout << "bbbb " << std::ctime(&end_time) << std::endl;
	    
	        framenumber = -1;

	        while ( decoder->readNextFrame(*pedefile, framenumber, iframe, buff) ) {

	          if ((ifr + 1) % 100 == 0) {
	            std::cout
		            << " ****"
		            << decoder->getValue(buff, 20, 20); // << std::endl;
	          }
	          mt->pushData(buff);
	          mt->nextThread();
	          mt->popFree(buff);
	          ++ifr;
	          if (ifr % 100 == 0) {
	            std::cout << " ****" << ifr << " " << framenumber << " " << iframe
			                  << std::endl;
	          } // else
	      
	          if (ifr >= 1000)
	            break;
	          framenumber = -1;
	        }
	    
	        pedefile->CloseResources();
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

      } else { //is a tiff file

	      std::vector<double> ped(nx * ny);
	      float* pp = ReadFromTiff(pedfilename.c_str(), nny, nnx);
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

    FILE* of = nullptr;
    
    //NOTE THAT THE DATA FILES HAVE TO BE IN THE RIGHT ORDER SO THAT PEDESTAL TRACKING WORKS!
    for (unsigned int ifile = 0; ifile != filenames.size(); ++ifile) {
      std::cout << "DATA ";
      std::string fsuffix{};
      std::string const fprefix( getRootString(filenames[ifile]) );
      std::string imgfname( createFileName( outdir, fprefix, fsuffix, "tiff" ) );
      std::string const cfname( createFileName( outdir, fprefix, fsuffix, "clust" ) );
      std::cout << filenames[ifile] << " ";
      std::cout << imgfname << std::endl;
      std::time(&end_time);
      std::cout << std::ctime(&end_time) << std::endl;

      //HDF5File fileh5;
      auto fileh5 = std::make_unique<HDF5File>();
      //      //open file
      ioutfile = 0;
      if ( fileh5->OpenResources(filenames[ifile].c_str(), validate_rank) ) {
	      if (thr <= 0 && cf != 0) { // cluster finder
	        if (of == nullptr) {
	          of = fopen(cfname.c_str(), "w");
	          if (of) {
	            if (mt) {
		            mt->setFilePointer(of);
		            std::cout << "file pointer set " << std::endl;
	            } else {
		            std::cerr << "Error: mt is null." << std::endl;
		            return 1;
	            }
	            //mt->setFilePointer(of);
	            //std::cout << "file pointer set " << std::endl;
	            //std::cout << "Here! " << framenumber << " ";
	          } else {
	            std::cout << "Could not open " << cfname
			                  << " for writing " << std::endl;
	            mt->setFilePointer(nullptr);
	            return 1;
	          }
	        }
	      }
	
	//     //while read frame
	      framenumber = -1;
	      iframe = 0;
	      ifr = 0;
	      //std::cout << "Here! " << framenumber << " ";
	      while ( decoder->readNextFrame(*fileh5, framenumber, iframe, buff) ) {
	        //std::cout << "Here! " << framenumber << " ";
	        //         //push
	        if ((ifr + 1) % 1000 == 0) {
	          std::cout << " ****"
		                  << decoder->getValue(buff, 20, 20); // << std::endl;
	        }
	        mt->pushData(buff);

	        // 	//         //pop
	        mt->nextThread();
	        mt->popFree(buff); /* In the last execution of the loop,
                              * this leaves buff outside of the Fifo!
                              * Free explicitely at the end! */
	  
	        ++ifr;
	        if (ifr % 1000 == 0)
	          std::cout << " " << ifr << " " << framenumber << std::endl;
	        if (nframes > 0) {
	          if (ifr % nframes == 0) {
	            imgfname = createFileName( outdir, fprefix, fsuffix, "tiff", ioutfile );
	            mt->writeImage(imgfname.c_str(), thr1);
	            mt->clearImage();
	            ++ioutfile;
	          }
	        }

	        framenumber = -1;
	      }

	      //std::cout << "aa --" << std::endl;
	      fileh5->CloseResources();

	      //std::cout << "bb --" << std::endl;
	      while (mt->isBusy()) {
	        ;
	      }

	      //std::cout << "cc --" << std::endl;
	      if (nframes >= 0) {
	        if (nframes > 0)
	          imgfname = createFileName( outdir, fprefix, fsuffix, "tiff", ioutfile );
	        std::cout << "Writing tiff to " << imgfname << " " << thr1
		                << std::endl;
	        mt->writeImage(imgfname.c_str(), thr1);
	        mt->clearImage();
	        if (of) {
	          fclose(of);
	          of = nullptr;
	          mt->setFilePointer(nullptr);
	        }
	      }
	      std::time(&end_time);
	      std::cout << std::ctime(&end_time) << std::endl;
      } else
	      std::cout << "Could not open " << filenames[ifile] << " for reading "
		              << std::endl;
    }
    if (nframes < 0) {     
      std::string fprefix( getRootString(filenames[0]) ); //Possibly, non-ideal name choice for file
      std::string imgfname( createFileName( outdir, fprefix, "sum", "tiff" ) );
      std::cout << "Writing tiff to " << imgfname << " " << thr1 << std::endl;
      mt->writeImage(imgfname.c_str(), thr1);
    }


    //std::cout << "Calling delete..." << std::endl;
    /* Info: Previously, 'delete mt' caused crash
     (double calls of StopThread() in both destructors of
     multiThreadedAnalogDetector and threadedAnalogDetector)
     Now fixed! */
    delete mt; // triggers cleanup of all threads and singlePhotonDetector instances (delete filter is obsolete)
    delete decoder;
    free(buff); // Free explicitly as it gets popped out of the Fifo at termination of while(readNextFrame)

    return 0;
}
