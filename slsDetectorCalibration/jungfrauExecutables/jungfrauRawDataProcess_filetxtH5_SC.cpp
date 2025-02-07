// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
// #include "sls/ansi.h"
//#include <iostream>
#undef CORR

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


/*Dataset paths according to different beamlines*/
std::string const data_datasetname_furka("/data/JF18T01V01/data");
std::string const index_datasetname_furka("/data/JF18T01V01/frame_index");
std::string const data_datasetname_xfelSCS("/INSTRUMENT/SCS_HRIXS_JUNGF/DET/JNGFR01:daqOutput/data/adc");
std::string const index_datasetname_xfelSCS("/INSTRUMENT/SCS_HRIXS_JUNGF/DET/JNGFR01:daqOutput/data/frameNumber");

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

/* Create file name string
 * \param dir directory
 * \param fprefix fileprefix (without extension)
 * \param fsuffix filesuffix (for output files, e.g. "ped")
 * \param fext    file extension (e.g. "raw")
 */
std::string createFileName( std::string const& dir, std::string const& fprefix="run",
			                std::string const& fsuffix="", std::string const& fext="raw", int const outfilecounter=-1 ) {
    std::string return_string;
    if (outfilecounter >= 0)
        return_string = fmt::format("{:s}/{:s}_{:s}_f{:05d}", dir, fprefix, fsuffix, outfilecounter);
    else if (fsuffix.length()!=0)
        return_string = fmt::format("{:s}/{:s}_{:s}", dir, fprefix, fsuffix);
    else
        return_string = fmt::format("{:s}/{:s}", dir, fprefix);

    if (fext.length()!=0)
        return_string += "." + fext;

    return return_string;
}

/* Adjusts number of threads to be a multiple of number of storage cells
 * \param requestedThreads number of threads requested by the user
 * \param nSC number of storage cells
 */
int adjustThreads(int requestedThreads, int nSC) {
    if (nSC <= 0) {
        std::cerr << "Error: Number of S values must be greater than zero!" << std::endl;
        return requestedThreads; // Return the original value as a fallback
    }

    // Calculate the remainder
    int remainder = requestedThreads % nSC;

    // If remainder is non-zero, round up by adding the difference
    int adjustedThreads = (remainder == 0) ? requestedThreads : requestedThreads + (nSC - remainder);

    // Ensure at least `nSC` threads are used
    if (adjustedThreads < nSC) {
        adjustedThreads = nSC;
    }

    std::cout << "Adjusted thread count (rounded up): " << adjustedThreads << " (nearest multiple of " 
              << nSC << ")" << std::endl;

    return adjustedThreads;
}


int main(int argc, char *argv[]) {

    if (argc < 4) {
        std::cout
            << "Usage is " << argv[0]
            << " filestxt outdir [pedfile (h5)] "
               " optional: [int dataset path; 0 means Furka, 1 means XFEL; overwrites default given in HDF5File.h] "
               " [bool validate h5 rank] "
               " [xmin xmax ymin ymax] [nframes] "
	             " NOTE THAT THE DATA FILES HAVE TO BE IN THE RIGHT ORDER SO THAT PEDESTAL TRACKING WORKS! "
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

    std::string datasetpath{};
    std::string frameindexpath{};
    if (argc > 4) {
      switch (atoi(argv[4])) {
      case 0:
        datasetpath = data_datasetname_furka;
        frameindexpath = index_datasetname_furka;
        break;
      case 1:
        datasetpath = data_datasetname_xfelSCS;
        frameindexpath = index_datasetname_xfelSCS;
        break;
      default:
        break;
      }
    }

    bool validate_rank=true;
    if (argc > 5)
      validate_rank = atoi(argv[5]);

    int nframes = 0;
    if (argc > 10)
      nframes = atoi(argv[10]);

    //Get vector of filenames from input txt-file
    std::vector<std::string> filenames{};

    { //Safety scope for ifstream
      ifstream inputs( txtfilename, std::ios::in );
      if (inputs.is_open()) {
	    std::cout << "Reading imput filenames from txt-file ..." << std::endl;
	    std::string line{};
	    while (!inputs.eof()) {
	      std::getline(inputs, line);
            if(line.find(".h5") != std::string::npos) {
	            filenames.emplace_back(line);
	            std::cout << line << std::endl;
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
    if (argc > 9) {
      xmin = atoi(argv[6]);
      xmax = atoi(argv[7]);
      ymin = atoi(argv[8]);
      ymax = atoi(argv[9]);
    }
    std::cout << "Cluster finder ROI: [" << xmin << ", " << xmax << ", " << ymin << ", " << ymax << "]"
              << std::endl;

    decoder->getDetectorSize(nx, ny);
    std::cout << "Detector size is " << nx << " " << ny << std::endl;

    std::time_t end_time;

    std::cout << "output directory is " << outdir << std::endl;
    if (pedfilename.length()!=0)
        std::cout << "pedestal file is " << pedfilename << std::endl;
    
    std::cout << "Nframes is " << nframes << std::endl;

    uint32_t nnx, nny;

    singlePhotonDetector* filter =
      new singlePhotonDetector(decoder, 3, nsigma, 1, NULL, nped, 200, -1, -1, NULL, NULL);
    //auto filter = std::make_unique<singlePhotonDetector>(decoder.get(), 3, nsigma, 1, nullptr, nped, 200, -1, -1, nullptr, nullptr);

    filter->setROI(xmin, xmax, ymin, ymax);

    std::time(&end_time);
    std::cout << std::ctime(&end_time) << std::endl;

    // Validate number of threads for number of storage cells (if applicable)
    int nThreads = nthreads;
    int nSC = 1;
    // Determine the dimensions of the dataset from the first datafile
    auto firstfileh5 = std::make_unique<HDF5File>();
    firstfileh5->SetFrameIndexPath(frameindexpath);
    firstfileh5->SetImageDataPath(datasetpath);
    if ( firstfileh5->OpenResources(filenames[0].c_str(), validate_rank) ) {

        // Validate number of threads
        if( firstfileh5->GetRank() == 4 ) {
            auto h5dims = firstfileh5->GetDatasetDimensions();
            nSC = h5dims[1];
            nThreads = adjustThreads(nthreads,nSC);
        }

        firstfileh5->CloseResources();
    } else {
        std::cerr << "Could not open data file " << filenames[0]
		          << " for validating rank " << std::endl;
    }

    multiThreadedCountingDetector* mt =
        new multiThreadedCountingDetector(filter, nThreads, fifosize, nSC);
    //auto mt = std::make_unique<multiThreadedCountingDetector>(filter.get(), nthreads, fifosize);
    mt->setClusterSize(csize, csize);
    mt->newDataSet(); //Initialize new dataset for each thread
    mt->setDetectorMode(ePhotonCounting);

    char* buff;

    mt->StartThreads();
    mt->popFree(buff);

    int ifr = 0; //frame counter of while loop
    int framenumber = 0; //framenumber as read from file (detector)
    std::vector<hsize_t> h5offset; //hyperslab offset internal to HDF5File::ReadImage
    hsize_t h5rank;

    if (pedfilename.length()>1) {

        std::string froot = getRootString(pedfilename);

        std::cout << "PEDESTAL " << std::endl;

	    std::string const fname(pedfilename);
	    std::cout << fname << std::endl;
	    std::time(&end_time);
	    std::cout << "aaa " << std::ctime(&end_time) << std::endl;

	    mt->setFrameMode(ePedestal);

	    //HDF5File pedefile;
	    auto pedefile = std::make_unique<HDF5File>();
        pedefile->SetFrameIndexPath(frameindexpath);
        pedefile->SetImageDataPath(datasetpath);
	    //      //open file
	    if ( pedefile->OpenResources(fname.c_str(),validate_rank) ) {

            // Initialize offset vector to 0
            h5rank = pedefile->GetRank();
	        h5offset.resize(h5rank-2, 0);
	        framenumber = 0;

	        while ( decoder->readNextFrame(*pedefile, framenumber, h5offset, buff) ) {

	            if ((ifr + 1) % 100 == 0) {
	                std::cout
		                << " ****"
		                << decoder->getValue(buff, 20, 20); // << std::endl;
	            }

                int storageCell = 0;
                hsize_t n_storageCells = 1;
                if (h5rank == 4) {
                    storageCell = h5offset[1];
                    n_storageCells = pedefile->GetDatasetDimensions()[1];
                }

                // push buff into fifoData for a thread corresponding to the active storage cell
	            mt->pushData(buff, storageCell);
                // increment (round-robin) the internal thread counter for that storage cell
	            mt->nextThread(storageCell);
                // get a free memory address from fifoFree of the active storage cell for the next read operation
	            mt->popFree(buff, storageCell);
                /* NOTE: the buff that was popped free from the current thread, will be (likely) pushed into
                 * the fifoData of a different thread in the next iteration of the loop! */

	            ++ifr;
	            if (ifr % 100 == 0) {
	                std::cout << " ****" << ifr << " " << framenumber << " " << h5offset[0];
                    if (n_storageCells>1)
			            std::cout << " sc " << storageCell;
                    std::cout << "\n";
	            } // else
	      
	            if (ifr >= 1000*n_storageCells)
	                break;
	          //framenumber = 0;
	        }
	    
	        pedefile->CloseResources();
	        while (mt->isBusy()) {
	          ;
	        }

	        std::cout << "froot " << froot << std::endl;
	        auto imgfname = createFileName( outdir, froot, "ped", "" );
	        mt->writePedestal(imgfname.c_str());
	        imgfname = createFileName( outdir, froot, "rms", "");
	        mt->writePedestalRMS(imgfname.c_str());

	      } else
	        std::cout << "Could not open pedestal file " << fname
		                << " for reading " << std::endl;

      std::time(&end_time);
      std::cout << std::ctime(&end_time) << std::endl;
    }
    
    ifr = 0;
    int ioutfile = 0;

    mt->setFrameMode(eFrame);

    FILE* of = nullptr;
    
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
        fileh5->SetFrameIndexPath(frameindexpath);
        fileh5->SetImageDataPath(datasetpath);
        //      //open file
      
        if ( fileh5->OpenResources(filenames[ifile].c_str(), validate_rank) ) {
	      
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
	      
	
	//     //while read frame
	        framenumber = 0;
	        std::fill(h5offset.begin(), h5offset.end(), 0);
	        ifr = 0;
	        //std::cout << "Here! " << framenumber << " ";
	        while ( decoder->readNextFrame(*fileh5, framenumber, h5offset, buff) ) {
	        
	            if ((ifr + 1) % 1000 == 0) {
	                std::cout << " ****"
		                      << decoder->getValue(buff, 20, 20); // << std::endl;
	            }

                int storageCell = 0;
                hsize_t n_storageCells = 1;
                if (h5rank == 4) {
                    storageCell = h5offset[1];
                    n_storageCells = fileh5->GetDatasetDimensions()[1];
                }

	            // push buff into fifoData for a thread corresponding to the active storage cell
	            mt->pushData(buff, storageCell);
                // increment (round-robin) the internal thread counter for that storage cell
	            mt->nextThread(storageCell);
                // get a free memory address from fifoFree of the active storage cell for the next read operation
	            mt->popFree(buff, storageCell); /* In the last execution of the loop,
                                                * this leaves buff outside of the Fifo!
                                                * Free explicitely at the end! */
	  
	            ++ifr;
	            if (ifr % 1000 == 0) {
	                std::cout << " " << ifr << " " << framenumber << " " << h5offset[0];
                    if (n_storageCells>1)
			            std::cout << " sc " << storageCell;
                    std::cout << "\n";
                }

	            //framenumber = 0;
	        }

	        //std::cout << "aa --" << std::endl;
	        fileh5->CloseResources();

	        //std::cout << "bb --" << std::endl;
	        while (mt->isBusy()) {
	            ;
	        }

	        //std::cout << "cc --" << std::endl;
	      
	        imgfname = createFileName( outdir, fprefix, fsuffix, "" );
	        std::cout << "Writing tiff to " << imgfname << "_SCxx.tiff" << std::endl;
	        mt->writeImage(imgfname.c_str());
	        mt->clearImage();
	        if (of) {
	          fclose(of);
	          of = nullptr;
	          mt->setFilePointer(nullptr);
	        }
	      
	        std::time(&end_time);
	        std::cout << std::ctime(&end_time) << std::endl;
        } else {
	        std::cout << "Could not open " << filenames[ifile] << " for reading "
		              << std::endl;
        }
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
