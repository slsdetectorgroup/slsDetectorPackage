// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#include "sls/ansi.h"
#include <iostream>
//#define CORR
#undef CORR

#define C_GHOST 0 //0.0004

#define CM_ROWS 20

#define RAWDATA

#ifndef MOENCH04
#ifndef RECT
#include "moench03v2Data.h"
#endif
#endif

#ifdef RECT
#include "moench03T1ReceiverDataNewRect.h"
#endif


#ifdef MOENCH04
#include "moench04CtbZmq10GbData.h"
#endif

#include "multiThreadedCountingDetector.h"
#include "moench03CommonMode.h"
#include "moench03GhostSummation.h"
#include "singlePhotonDetector.h"

#include <fstream>
#include <map>
#include <stdio.h>
#include <sys/stat.h>

#include <ctime>
using namespace std;

int main(int argc, char *argv[]) {

  std::map<std::string, std::string> args = {
    {"numfiles","1"},
    {"nthreads","5"},
    {"fifosize","5000"},
    {"nsigma","5"},
    {"gainfile","none"},
    {"detectorMode","counting"},
    {"threshold","0"},
    {"pedestalfile","none"},
    {"nframes","0"},
    {"xMin","0"},
    {"xMax","400"},
    {"yMin","0"},
    {"yMax","400"},
    {"eMin","0"},
    {"eMax","16000"},
    {"outdir","./"},
    {"indir","./"},
    {"flist","none"},
    {"fformat","none"},
    {"runmin","0"},
    {"runmax","-1"},
    {"readnrows","400"}
  };
  //float *gm;
  
  int ff, np;
  // cout << " data size is " << dsize;
  
  ifstream filebin;
  if (argc < 4) {
    std::string name, value,sline;
    int ic=0;
    ifstream flist;
    flist.open (argv[1], std::ifstream::in);
    if (flist.is_open()) {
      cout << "Using config file " <<argv[1] << endl;
       while (std::getline(flist,sline)){
	 if (sline.at(0)!='#') {
	   ic=sline.find(' ');
	   name = sline.substr(0,ic); 
	   value = sline.substr(ic+1,sline.size()-ic); 
	   args[name]=value;
	 }
	 
       } 
       flist.close();
    } else {
        cout << "Usage is " << argv[0]
             << "indir outdir fname(no extension) [runmin] [runmax] [pedfile (raw or tiff)] [threshold] "
	  "[nframes] [xmin xmax ymin ymax] [gainmap]"
             << endl;
        cout << "threshold <0 means analog; threshold=0 means cluster finder; "
	  "threshold>0 means photon counting"
             << endl;
        cout << "nframes <0 means sum everything; nframes=0 means one file per "
	  "run; nframes>0 means one file every nframes"
             << endl;
	return EXIT_FAILURE;
      }
    } else {
      args["indir"]=argv[1];
      args["outdir"]=argv[1];
      args["fformat"]=argv[3];
      if (argc >= 5) {
	args["runmin"] = argv[4];
      }
	args["runmax"] = args["runmin"];

      if (argc >= 6) {
	args["runmax"] = argv[5];
      }
      if (argc >= 7) {
        args["pedestalfile"] = argv[6];
      }
      if (argc >= 8) {
        args["threshold"] = argv[7];
      }
      if (argc >= 9) {
        args["nframes"] = argv[8];
      }
      if (argc >= 13) {
        args["xMin"] = argv[9];
        args["xMax"] = argv[10];
        args["yMin"] = argv[11];
        args["yMax"] = argv[12];
      }
      if (argc > 13) {
	args["gainfile"] = argv[13];
      }
      
      if (atof(args["threshold"].c_str())<0) {
	args["detectorMode"]="analog";
      }

    }



    for (auto const& x : args)
      {
	std::cout << x.first  // string (key)
		  << ':' 
		  << x.second // string's value 
		  << std::endl;
      }


    string indir=args["indir"]; 
    string outdir = args["outdir"];
    string fformat= args["fformat"];
    int runmin = atoi(args["runmin"].c_str());
    int runmax = atoi(args["runmin"].c_str());
    string pedfile =args["pedestalfile"];
    double thr = atof(args["threshold"].c_str());
    double thr1 = 1;
    
    int nframes = atoi(args["nframes"].c_str());
    
    int xmin =  atoi(args["xMin"].c_str()), xmax =  atoi(args["xMax"].c_str()), ymin = atoi(args["yMin"].c_str()), ymax = atoi(args["yMax"].c_str());
    
    string gainfname=args["gainfile"];
    
    int fifosize = atoi(args["fifosize"].c_str());
    int nthreads = atoi(args["nthreads"].c_str());
    int nsigma = atoi(args["nsigma"].c_str());
    int nrows = atoi(args["readnrows"].c_str());
    float eMin = atof(args["eMin"].c_str());
    float eMax = atof(args["eMax"].c_str());
    int csize = 3;
    int nped = 1000;
    
    int cf = 0;
    int numberOfPackets=nrows/8;
#ifdef RECT
    cout << "Should be rectangular but now it will crash! No data structure defined!" << endl;
#endif
    
#ifndef MOENCH04
    moench03v2Data *decoder = new moench03v2Data(nrows/2);
    cout << "MOENCH03!" << endl;
#endif

#ifdef MOENCH04
#ifndef MOENCH04_DGS
    moench04CtbZmq10GbData *decoder = new moench04CtbZmq10GbData(5000,0);
    cout << "MOENCH04!" << endl;
#endif
    
#ifdef MOENCH04_DGS
    moench04CtbZmq10GbData *decoder = new moench04CtbZmq10GbData(5000,5000);
    cout << "MOENCH04 DGS!" << endl;
    numberOfPackets=45;
#endif
    
#endif

    int nx , ny;
    decoder->getDetectorSize(nx, ny);

    //Read detector size from decoder
   
    char ffname[10000];
    char fname[10000];
    char imgfname[10000];
    char cfname[10000];

    std::time_t end_time;

    FILE *of = NULL;
    uint32_t nnx, nny;


    moench03CommonMode *cm = nullptr;
    moench03GhostSummation *gs = nullptr;
    double *gainmap = nullptr;

#ifdef CORR
    int ncol_cm = CM_ROWS;
    double xt_ghost = C_GHOST;
    std::cout << "Applying common mode  " << ncol_cm << endl;
    cm = new moench03CommonMode(ncol_cm);

    // cout << "Applying ghost corrections " << xt_ghost << endl;
    // gs = new moench03GhostSummation(decoder, xt_ghost);
#endif

    singlePhotonDetector *filter = new singlePhotonDetector(
        decoder, csize, nsigma, 1, cm, nped, 200, -1, -1, gainmap, gs);

    //if (gainfname) {

    if (filter->readGainMap(gainfname.c_str()))
            cout << "using gain map " << gainfname << endl;
        else
            cout << "Could not open gain map " << gainfname << endl;
    // } else
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
    filter->setEnergyRange(eMin, eMax);
    std::time(&end_time);
    cout << std::ctime(&end_time) << endl;

    char *buff;
    
    // multiThreadedAnalogDetector *mt=new
    // multiThreadedAnalogDetector(filter,nthreads,fifosize);
    multiThreadedCountingDetector *mt =
        new multiThreadedCountingDetector(filter, nthreads, fifosize);
    
    if (args["detectorMode"]=="counting") {
      mt->setDetectorMode(ePhotonCounting);
      if (thr > 0) {
        cf = 0;
      }
    } else {
      mt->setDetectorMode(eAnalog);
      cf = 0;
    }
      
      
// #ifndef ANALOG
//     mt->setDetectorMode(ePhotonCounting);
//     cout << "Counting!" << endl;
//     if (thr > 0) {
//         cf = 0;
//     }
// #endif
// //{
// #ifdef ANALOG
//     mt->setDetectorMode(eAnalog);
//     cout << "Analog!" << endl;
//     cf = 0;
//     // thr1=thr;
// #endif
//     //  }

    mt->StartThreads();
    mt->popFree(buff);

    //  cout << "mt " << endl;

    int ifr = 0;
    char froot[1000];
    double *ped=new double[nx * ny];//, *ped1;
    int pos,pos1;
    //return  0;
  
    if (pedfile.find(".raw") != std::string::npos) {
      pos1=pedfile.rfind("/");
      strcpy(froot,pedfile.substr(pos1).c_str());
      pos=string(froot).find(".raw");
      froot[pos]='\0';
    }
    
    cout << "PEDESTAL " << endl;
    if (pedfile.find(".tif") == std::string::npos) {
      sprintf(fname, "%s", pedfile.c_str());
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
		  if (np == numberOfPackets) {
		    mt->pushData(buff);
                        mt->nextThread();
                        mt->popFree(buff);
                        ifr++;
			if (ifr % 100 == 0)
			  cout << ifr << " " << ff << " " << np << endl;
			    //	break;
		  } else {
		    cout << ifr << " " << ff << " " << np << endl;
		    break;
		  }
                    ff = -1;
                }
                filebin.close();
                while (mt->isBusy()) {
		          ;
		          }
		
		sprintf(imgfname, "%s/%s_ped.tiff", outdir.c_str(),froot);
		mt->writePedestal(imgfname);
		sprintf(imgfname, "%s/%s_var.tiff", outdir.c_str(),froot);
		mt->writePedestalRMS(imgfname);
            } else
	      cout << "Could not open pedestal file " << fname
		   << " for reading " << endl;
    } else {
      float *pp = ReadFromTiff(pedfile.c_str(), nny, nnx);
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

    
    ifr = 0;
    int ifile = 0;

    mt->setFrameMode(eFrame);
    //t filelist=0;
    ifstream flist;
    flist.open (args["flist"].c_str(), std::ifstream::in);
    if (flist.is_open()) {
      cout << "Using file list" << endl;
      runmin=0;
       runmax=0;
       while (flist.getline(ffname,10000)){
	 cout << ffname << endl;
	 runmax++;
       }
       runmax--;
       flist.close();
       cout << "Found " << runmax << " files " << endl;
       flist.open (fformat, std::ifstream::in);
    } 

    for (int irun = runmin; irun <= runmax; irun++) {
        cout << "DATA ";
        // sprintf(fn,fformat,irun);
        // sprintf(ffname, "%s/%s.raw", indir, fformat);
        // sprintf(fname, (const char*)ffname, irun);
        // sprintf(ffname, "%s/%s.tiff", outdir, fformat);
        // sprintf(imgfname, (const char*)ffname, irun);
        // sprintf(ffname, "%s/%s.clust", outdir, fformat);
        // sprintf(cfname, (const char*)ffname, irun);
	if (flist.is_open()) {
	  flist.getline(ffname,10000);
	  cout << "file list " << ffname << endl;
	} else {
	  //sprintf(ffname,(const char*)fformat,irun);
	  sprintf(ffname,args["fformat"].c_str(),irun);
	  cout << "loop " << ffname << endl;
	}
	cout << "ffname "<< ffname << endl;
	sprintf(fname,  "%s/%s.raw",indir.c_str(),ffname);
	sprintf(imgfname,  "%s/%s.tiff",outdir.c_str(),ffname);
	sprintf(cfname,  "%s/%s.clust",outdir.c_str(),ffname);


        cout << fname << " ";
        cout << imgfname << endl;
        std::time(&end_time);
        cout << std::ctime(&end_time) << endl;
        //  cout <<  fname << " " << outfname << " " << imgfname <<  endl;
        filebin.open((const char *)(fname), ios::in | ios::binary);
        //      //open file
        ifile = 0;
        if (filebin.is_open()) {
            if (cf != 0) { // cluster finder
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
	      if (np == numberOfPackets) {
                    //         //push
                    mt->pushData(buff);
                    // 	//         //pop
                    mt->nextThread();
                    mt->popFree(buff);

                    ifr++;
		    if (ifr % 100 == 0)
		      cout << ifr << " " << ff << " " << np << endl;
		    //break;
                    if (nframes > 0) {
                        if (ifr % nframes == 0) {
                            // sprintf(ffname, "%s/%s_f%05d.tiff", outdir, fformat,
                            //         ifile);
                            // sprintf(imgfname, (const char*)ffname, irun);
			  sprintf(imgfname,  "%s/%s_f%05d.tiff",outdir.c_str(),ffname,ifile);
                           while (mt->isBusy()) 
                             ;

                            mt->writeImage(imgfname, thr1);
                            mt->clearImage();
                            ifile++;
                        }
                    }
	      } else {
		cout << "bp " << ifr << " " << ff << " " << np << endl;
	      }
                ff = -1;
            }
            cout << "--" << endl;
            filebin.close();
            while (mt->isBusy()) {
                ;
            }
            if (nframes >= 0) {
                if (nframes > 0) {
		  sprintf(imgfname,  "%s/%s_f%05d.tiff",outdir.c_str(),ffname,ifile);
                  //  sprintf(ffname, "%s/%s_f%05d.tiff", outdir, fformat, ifile);
		  //sprintf(imgfname, (const char*)ffname, irun);
                } else {
		  sprintf(imgfname,  "%s/%s.tiff",outdir.c_str(),ffname);
                  //  sprintf(ffname, "%s/%s.tiff", outdir, fformat);
                  //  sprintf(imgfname, (const char*)ffname, irun);
                }
                cout << "Writing tiff to " << imgfname << " " << thr1 << endl; 
		while (mt->isBusy()) 
		  ;
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
      //sprintf(ffname, "%s/%s.tiff", outdir, fformat);
      //  strcpy(imgfname, ffname);
      sprintf(imgfname,  "%s/%s_tot.tiff",outdir.c_str(),ffname);
      cout << "Writing tiff to " << imgfname << " " << thr1 << endl;
      while (mt->isBusy()) 
	;
      mt->writeImage(imgfname, thr1);
    }
    if (flist.is_open()) {
      flist.close();
    }
    return 0;
}
