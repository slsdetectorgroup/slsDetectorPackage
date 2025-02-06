// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MULTITHREADED_ANALOG_DETECTOR_H
#define MULTITHREADED_ANALOG_DETECTOR_H

#define MAXTHREADS 1000

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
//#include <deque>
//#include <list>
//#include <queue>
#include <cstdlib>
#include <fstream>
#include <pthread.h>

#include "analogDetector.h"
#include "circularFifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>

using namespace std;

class threadedAnalogDetector {
  public:
    threadedAnalogDetector(analogDetector<uint16_t> *d, int fs = 10000) {
        char *mm; //*mem,
        det = d;
        fifoFree = new CircularFifo<char>(fs);
        fifoData = new CircularFifo<char>(fs);
        // mem==NULL;
        /* mem=(char*)calloc(fs, det->getDataSize()); */
        /* if (mem) */
        /*   memset(mem,0, fs*det->getDataSize()); */
        int i;
        for (i = 0; i < fs; i++) {
            //
            //  mm=mem+i*det->getDataSize();
            // cout << i << endl;
            mm = (char *)calloc(1, det->getDataSize());

            if (mm) {
                // memset(mm,0, det->getDataSize());
                /*
                if (i == 0) { // debug
                    first_mm = mm;
                }
                */
                
                fifoFree->push(mm);
                //std::cout << "Allocated memory at: " << static_cast<void*>(mm) << " (fifoslot " << i << ")" << std::endl;
            } else
                break;
        }

        if (i < fs)
            std::cout << "Could allocate only " << i << " frames";

        busy = 0;
        stop = 1;
        fMode = eFrame;
        ff = NULL;
    }

    virtual int setFrameMode(int fm) {
        if (fm >= 0) {
            det->setFrameMode((frameMode)fm);
            fMode = fm;
        }
        return fMode;
    };
    virtual double setThreshold(double th) { return det->setThreshold(th); };

    virtual double setClusterSize(int csx, int csy) { 
      //cout << "44" << endl; 
      return det->setClusterSize(csx); 
    };

    virtual void setROI(int xmin, int xmax, int ymin, int ymax) {
        det->setROI(xmin, xmax, ymin, ymax);
    };
    virtual int setDetectorMode(int dm) {
        if (dm >= 0) {
            det->setDetectorMode((detectorMode)dm);
            dMode = dm;
        }
        return dMode;
    };

    virtual void newDataSet() { det->newDataSet(); };
    // fMode=fm; return fMode;}

    /* void prepareInterpolation(int &ok) { */
    /*   cout << "-" << endl; */
    /*   det->prepareInterpolation(ok); */
    /* }; */

    virtual int *getImage() { return det->getImage(); }
    virtual int getImageSize(int &nnx, int &nny, int &ns, int &nsy) {
        return det->getImageSize(nnx, nny, ns, nsy);
    };
    virtual int getDetectorSize(int &nnx, int &nny) {
        return det->getDetectorSize(nnx, nny);
    };

    virtual ~threadedAnalogDetector() {

        StopThread();

        delete fifoFree;
        delete fifoData;
        delete det; // Call destructor for singlePhotonDetector
    }

    /** Returns true if the thread was successfully started, false if there was
     * an error starting the thread */
    virtual bool StartThread() {
        stop = 0;
        std::cout << "Detector number " << det->getId() << std::endl;
        std::cout << "common mode is " << det->getCommonModeSubtraction() << std::endl;
        std::cout << "ghos summation is " << det->getGhostSummation() << std::endl;

        return (pthread_create(&_thread, NULL, processData, this) == 0);
    }

    virtual void StopThread() {
        stop = 1;
        //std::cout << "Attempting to stop thread..." << std::endl;

        // Free all remaining allocated memory in fifoFree
        char *mm = nullptr;
        while (fifoFree->pop(mm, true)) {  // Use no_block to avoid waiting
            //std::cout << "fifo Free: Freeing memory at: " << static_cast<void*>(mm) << std::endl;
            free(mm);  // Free the allocated memory
        }

        if (_thread) {
            //(void)pthread_join(_thread, NULL);
            //std::cout << "Calling pthread_join for thread: " << det->getId() << std::endl;
            pthread_join(_thread, NULL);
            _thread = 0;
            std::cout << "Thread " << det->getId() << " stopped and joined." << std::endl;
        } else {
            std::cout << "No thread to join." << std::endl;
        }
    }

    virtual bool pushData(char *&ptr) { return fifoData->push(ptr); }

    virtual bool popFree(char *&ptr) { return fifoFree->pop(ptr); }

    // virtual int isBusy() {if (fifoData->isEmpty() && busy==0) return 0;
    // return 1;}

    virtual int isBusy() {
      //std::cout << busy << " " << fifoData->isEmpty() << " " << fifoData->getDataValue() << " " << fifoData->getFreeValue() <<  std::endl;
        if (busy == 0) {
            usleep(100);
            if (busy == 0) {
                if (fifoData->isEmpty()) {
                    usleep(100);
                    return 0;
                }
            }
        }
        return 1;
    }
    // protected:
    /** Implement this method in your subclass with the code you want your
     * thread to run. */
    // virtual void InternalThreadEntry() = 0;
    virtual void *writeImage(const char *imgname) {
        return det->writeImage(imgname);
    };

    virtual void clearImage() { det->clearImage(); };

    virtual void setPedestal(double *ped, double *rms = NULL, int m = -1) {
        det->setPedestal(ped, rms, m);
    };

    virtual void setPedestalRMS(double *rms) { det->setPedestalRMS(rms); };

    virtual double *getPedestal(double *ped = NULL) {
        return det->getPedestal(ped);
    };

    virtual double *getPedestalRMS(double *rms = NULL) {
        return det->getPedestalRMS(rms);
    };

    /** sets file pointer where to write the clusters to
        \param f file pointer
        \returns current file pointer
    */
    FILE *setFilePointer(FILE *f) { return det->setFilePointer(f); };

    /** gets file pointer where to write the clusters to
        \returns current file pointer
    */
    FILE *getFilePointer() { return det->getFilePointer(); };

    virtual double setNSigma(double n) { return det->setNSigma(n); };
    virtual void setEnergyRange(double emi, double ema) {
        det->setEnergyRange(emi, ema);
    };

    virtual void prepareInterpolation(int &ok) {
        slsInterpolation *interp = det->getInterpolation();
        if (interp)
            interp->prepareInterpolation(ok);
    }

    virtual int *getFlatFieldDistribution() {
        slsInterpolation *interp = (det)->getInterpolation();
        if (interp)
            return interp->getFlatFieldDistribution();
        else
            return NULL;
    }

    virtual int *setFlatField(int *ff, int nb, double emin, double emax) {
        slsInterpolation *interp = (det)->getInterpolation();
        if (interp)
            return interp->setFlatField(ff, nb, emin, emax);
        else
            return NULL;
    }

    void *writeFlatField(const char *imgname) {
        slsInterpolation *interp = (det)->getInterpolation();
        // cout << "interp " << interp << endl;
        if (interp) {
            cout << imgname << endl;
            return interp->writeFlatField(imgname);
        }
        return NULL;
    }

    void *readFlatField(const char *imgname,  double emin = 1,
                        double emax = 0) {
        slsInterpolation *interp = (det)->getInterpolation();
        if (interp)
            return interp->readFlatField(imgname, emin, emax);
        return NULL;
    }

    virtual int *getFlatField(int &nbx, int &nby, double &emi, double &ema) {
        slsInterpolation *interp = (det)->getInterpolation();
        int *ff = NULL;
        if (interp) {
	  ff = interp->getFlatField(nbx, nby, emi, ema);
        }
        return ff;
    }

    virtual slsInterpolation *getInterpolation() {
        return (det)->getInterpolation();
    }

    virtual void resetFlatField() {
        slsInterpolation *interp = (det)->getInterpolation();
        if (interp)
            interp
                ->resetFlatField(); //((interpolatingDetector*)det)->resetFlatField();
    }

    virtual int setNSubPixels(int ns, int nsy) {
        slsInterpolation *interp = (det)->getInterpolation();
        if (interp)
            interp->setNSubPixels(ns, nsy);
        return 1;
    };

    virtual slsInterpolation *setInterpolation(slsInterpolation *f) {
        return (det)->setInterpolation(f);
    };

  protected:
    analogDetector<uint16_t> *det;
    int fMode;
    int dMode;
    int *dataSize;
    pthread_t _thread;
    CircularFifo<char> *fifoFree;
    CircularFifo<char> *fifoData;
    int stop;
    int busy;
    char *data;
    int *ff;

    //char* first_mm = nullptr; // For debug; to track first allocated block

    static void *processData(void *ptr) {
        threadedAnalogDetector *This = ((threadedAnalogDetector *)ptr);
        return This->processData();
    }

    void *processData() {
        //  busy=1;
        while (!stop) {
            if (fifoData->isEmpty()) {
                usleep(100);
                if (fifoData->isEmpty()) {
                    busy = 0;
                } else {
                    busy = 1;
                }
            } else {
                busy = 1;
            }

            if (busy == 1) {
                // Check stop flag before making a blocking call
                //if (stop) {
                //    break;
                //}

                // Blocking call
                fifoData->pop(data); // blocking!

                // Process data if not stopping
                //if (!stop) {
                det->processData(data);
                fifoFree->push(data);
                    
                //}
                // busy=0;
            }
        }

        return NULL;
    }
};

class multiThreadedAnalogDetector {
  public:
    multiThreadedAnalogDetector(analogDetector<uint16_t> *d, int num_threads,
                                int fs = 1000, int num_sc = 1)
        : stop(0), nThreads(num_threads), nSC(num_sc) {
        dd[0] = d;
        if (nThreads == 1)
            dd[0]->setId(100);
        else
            dd[0]->setId(0);
        for (int i = 1; i < nThreads; i++) {
            dd[i] = d->Clone();
            dd[i]->setId(i);
        }

        for (int i = 0; i < nThreads; i++) {
            cout << "**" << i << endl;
            dets[i] = new threadedAnalogDetector(dd[i], fs);
        }

        // Pre-assign threads to storage cells in a round-robin manner
        int threads_per_sc = nThreads / nSC;
        sc_to_threads.clear();
        for (int s = 0; s < nSC; ++s) {
            for (int t = 0; t < threads_per_sc; ++t) {
                int assigned_thread = s * threads_per_sc + t;
                sc_to_threads[s].push_back(assigned_thread);
            }
        }
        // Set all thread counter to zero for each storage cell
        thread_counters_by_sc.resize(nSC,0);

        image = nullptr;
        ff = NULL;
        ped = NULL;
        std::cout << "Ithread is " << ithread << std::endl;
    }

    virtual ~multiThreadedAnalogDetector() {
        //std::cout << "Destructing multiThreadedAnalogDetector..." << std::endl;
        //StopThreads(); // Superfluous, leads to double delete
        
        /* Reverse loop for destruction.
         * Deletes clones first, then root object, which owns the mutex
         * (ensure shared mutex is deleted last).
         * Optional solution: reference counting (safer but more complex) */
        for (int i = nThreads - 1; i >= 0; --i) {
            //std::cout << "Deleting dets[" << i << "]" << std::endl;
            delete dets[i]; //StopThread() called by each ~threadedAnalogDetector()
        }
        
    }

    virtual int setFrameMode(int fm) {
        int ret = dets[0]->setFrameMode(fm);
        for (int i = 1; i < nThreads; i++) {
            dets[i]->setFrameMode(fm);
        }
        return ret;
    };
    virtual double setThreshold(int fm) {
        double ret = dets[0]->setThreshold(fm);
        for (int i = 1; i < nThreads; i++)
            dets[i]->setThreshold(fm);
        return ret;
    };
    virtual int setDetectorMode(int dm) {
        int ret = dets[0]->setDetectorMode(dm);
        ;
        for (int i = 1; i < nThreads; i++)
            dets[i]->setDetectorMode(dm);
        return ret;
    };
    virtual void setROI(int xmin, int xmax, int ymin, int ymax) {
        for (int i = 0; i < nThreads; i++)
            dets[i]->setROI(xmin, xmax, ymin, ymax);
    };

    virtual void newDataSet() {
        for (int i = 0; i < nThreads; i++)
            dets[i]->newDataSet();
    };

    virtual int *getImage(int &nnx, int &nny, int &ns, int &nsy) {
        //int *img;
        // int nnx, nny, ns;
        // int nnx, nny, ns;
        int nn = dets[0]->getImageSize(nnx, nny, ns, nsy);
        if (image) {
            delete[] image;
            image = nullptr;
        }
        image = new int[nn];
        // int nn=dets[0]->getImageSize(nnx, nny, ns);
        // for (i=0; i<nn; i++) image[i]=0;

        for (int ii = 0; ii < nThreads; ii++) {
            
            int* tmp_img = dets[ii]->getImage();

            /* std::cout << "## Thread " << ii
                      << " # image size " << nn
                      << " # nnx " << nnx
                      << " # nny " << nny
                      << " # ns " << ns; */

            for (int i = 0; i < nn; i++) {

                /* std::cout << " # pixel " << i
                          << " # value " << tmp_img[i]
                          << " ## " << std::endl; */

                if (ii == 0)
                    //	  if (img[i]>0)
                    image[i] = tmp_img[i];
                // else
                //  image[i]=0;
                else // if (img[i]>0)
                    image[i] += tmp_img[i];
                // if (img[i])	  cout << "det " << ii << " pix " << i << " val
                // " <<  img[i] << " " << image[i] << endl;
            }
        }
        return image;
    }

    virtual void clearImage() {

        for (int ii = 0; ii < nThreads; ii++) {
            dets[ii]->clearImage();
        }
    }

    virtual void *writeImage(const char *imgname, double t = 1) {
        /* #ifdef SAVE_ALL   */
        /*      for (int ii=0; ii<nThreads; ii++) { */
        /*        char tit[10000];cout << "m" <<endl; */
        /*        sprintf(tit,"/scratch/int_%d.tiff",ii); */
        /*        dets[ii]->writeImage(tit); */
        /*      } */
        /* #endif */
        int nnx, nny, ns, nsy;
        getImage(nnx, nny, ns, nsy);
        // int nnx, nny, ns;
        int nn = dets[0]->getImageSize(nnx, nny, ns, nsy);
        float *gm = new float[nn];
        if (gm) {
            for (int ix = 0; ix < nn; ix++) {
                if (t) {
                    if (image[ix] < 0)
                        gm[ix] = 0;
                    else
                        gm[ix] = (image[ix]) / t;
                } else
                    gm[ix] = image[ix];

                // if (image[ix]>0 && ix/nnx<350) cout << ix/nnx << " " <<
                // ix%nnx << " " << image[ix]<< " " << gm[ix] << endl;
            }
            // cout << "image " << nnx << " " << nny << endl;
            WriteToTiff(gm, imgname, nnx, nny);
            delete[] gm;
        } else
            std::cout << "Could not allocate float image " << std::endl;

        if(image) {
            delete[] image; // Memory cleanup (VH)
            image = nullptr;
        }
        
        return NULL;
    }

    virtual void StartThreads() {
        for (int i = 0; i < nThreads; i++) {
            dets[i]->StartThread();
        }
    }

    virtual void StopThreads() {
        std::cout << "Stopping all threads ..." << std::endl;
        for (int i = 0; i < nThreads; i++)
            dets[i]->StopThread();
    }

    virtual int isBusy() {
        int ret = 0, ret1;
        for (int i = 0; i < nThreads; i++) {
            ret1 = dets[i]->isBusy();
            ret |= ret1;
	    //if (ret1) cout << "thread " << i <<" still busy " << endl;
        }
        return ret;
    }

/*
    virtual std::vector<int> getThreadsForSc(int sc) {
        return sc_to_threads[sc];
    }
*/
    virtual bool pushData(char *&ptr, int sc=0) {
        //Additional logic implemented to accommodate storage cells

        std::unique_lock<std::mutex> lock(map_mutex);

        // Get assigned threads for this storage cell
        auto& assigned_threads = sc_to_threads[sc];
        auto& counter = thread_counters_by_sc[sc];
        
        // Distribute workload among threads using round-robin
        int selected_thread = assigned_threads[counter % assigned_threads.size()];

        return dets[selected_thread]->pushData(ptr); 
    }

    virtual bool popFree(char *&ptr, int sc=0) {
        //Additional logic implemented to accommodate storage cells

        std::unique_lock<std::mutex> lock(map_mutex);

        // Get assigned threads for this storage cell
        auto& assigned_threads = sc_to_threads[sc];
        auto& counter = thread_counters_by_sc[sc];
        
        // Distribute workload among threads using round-robin
        int selected_thread = assigned_threads[counter % assigned_threads.size()];

        return dets[selected_thread]->popFree(ptr);
    }

    virtual int nextThread(int sc=0) {
        //Additional logic implemented to accommodate storage cells

        auto& counter = thread_counters_by_sc[sc];
        //counter++;
        if (++counter == nThreads/nSC)
            counter = 0;
        return counter;
    }

    // Storage cell sensitive
    virtual double *getPedestal(int sc = 0) {
        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);

        if (sc_pedestals.count(sc) && sc_pedestals[sc]) {
            delete[] sc_pedestals[sc];
            sc_pedestals[sc] = nullptr;
        }

        // allocate memory and initialize all values to zero
        sc_pedestals[sc] = new double[nx * ny](); // parentheses initialize elements to zero
        //std::fill(sc_pedestals[sc], sc_pedestals[sc] + (nx * ny), 0.0); // explicit zero initialization
        double *p0 = new double[nx * ny];

        // Get the threads assigned to this storage cell
        auto const& assigned_threads = sc_to_threads[sc];
        int num_threads = assigned_threads.size();

        // Only iterate over threads assigned to this storage cell
        for ( int thread_id : assigned_threads ) {
            // inte=(slsInterpolation*)dets[i]->getInterpolation(nb,emi,ema);
            //   cout << i << endl;
            //p0 = dets[thread_id]->getPedestal(p0);
            dets[thread_id]->getPedestal(p0);

            if (p0) { /*
                if (i == 0) {
                    // If first thread, initialize ped with first thread's values
                    for (int ib = 0; ib < nx * ny; ib++) {
                        ped[ib] = p0[ib] / ((double)nThreads);
                        //  cout << p0[ib] << " ";
                    }
                } else {
                */
                    // For subsequent threads, accumulate pedestal values
                // if ( i == 0 ) becomes superfluous if we zero-initialize earlier
                for (int ib = 0; ib < nx * ny; ib++) {
                    sc_pedestals[sc][ib] += p0[ib] / ((double)num_threads);
                    //  cout << p0[ib] << " ";
                }
                //}
            }
        }
        delete[] p0;
        return sc_pedestals[sc];
    };

    // Storage cell sensitive
    virtual double *getPedestalRMS(int sc = 0) {
        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);
        
        if (sc_pedestals_rms.count(sc) && sc_pedestals_rms[sc]) {
            delete[] sc_pedestals_rms[sc];
            sc_pedestals_rms = nullptr;
        }

        // allocate memory and initialize all values to zero
        sc_pedestals_rms[sc] = new double[nx * ny](); // Zero-initialize
        //std::fill(sc_pedestals_rms[sc], sc_pedestals_rms[sc] + (nx * ny), 0.0); // explicit zero initialization
        //double *rms = sc_pedestals_rms[sc];
        double *p0 = new double[nx * ny];

        // Get the threads assigned to this storage cell
        auto const& assigned_threads = sc_to_threads[sc];
        int num_threads = assigned_threads.size();

        // Only iterate over threads assigned to this storage cell
        for (int thread_id : assigned_threads) {
            // inte=(slsInterpolation*)dets[i]->getInterpolation(nb,emi,ema);
            //   cout << i << endl;
            //p0 = dets[thread_id]->getPedestalRMS(p0);
            dets[thread_id]->getPedestalRMS(p0);

            if (p0) {
                for (int ib = 0; ib < nx * ny; ib++) {
                    sc_pedestals_rms[sc][ib] += (p0[ib] * p0[ib]) / ((double)num_threads);
                        //  cout << p0[ib] << " ";
                }
            }
            
        }
        delete[] p0;

        /* for (int ib=0; ib<nx*ny; ib++) { */
        /*   if (rms[ib]>0) */
        /* 	rms[ib]=sqrt(ped[ib]); */
        /*   else */
        /* 	rms[ib]=0; */
        /* } */

        return sc_pedestals_rms[sc];
    };

    // Does not differentiate for storage cells!
    virtual double *setPedestal(double *h = NULL) {
        // int nb=0;

        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);
        if (h == NULL)
            h = ped;
        for (int i = 0; i < nThreads; i++) {
            dets[i]->setPedestal(h);
        }

        return NULL;
    };

    // Storage cell sensitive
    virtual void *writePedestal(char const* base_imgname) {

        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);

        //float *gm = new float[nx * ny];

        // Loop over each storage cell
        for ( auto const& entry : sc_to_threads ) {
            int sc = entry.first;
            std::string imgname = std::string(base_imgname);
            if (nSC>1)
                imgname += "_SC" + std::to_string(sc);
            imgname += ".tiff";

            getPedestal(sc); // Compute pedestal for this storage cell
            std::vector<float> gm(nx * ny);

            // Copy pedestal data into the float array
            /*
            for (int ix = 0; ix < nx * ny; ix++) {
                gm[ix] = sc_pedestals[sc][ix];
            }
            */
            std::copy(sc_pedestals[sc], sc_pedestals[sc] + (nx * ny), gm.data());
            WriteToTiff(gm.data(), imgname.c_str(), nx, ny);
            
            // Clean up memory
            if(sc_pedestals[sc]) {
                delete[] sc_pedestals[sc];
                sc_pedestals[sc] = nullptr;
            }
        }

        return NULL;
    };

    // Storage cell sensitive
    virtual void *writePedestalRMS(char const* base_imgname) {

        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);

        //float *gm = new float[nx * ny];

        // Loop over each stoarge cell
        for ( auto const& entry : sc_to_threads ) {
            int sc = entry.first;
            std::string imgname = std::string(base_imgname);
            if (nSC>1)
                imgname += "_SC" + std::to_string(sc);
            imgname += ".tiff";

            double *rms = getPedestalRMS(sc); // Compute pedestal RMS for this storage cell
            std::vector<float> gm(nx * ny);

            // Copy rms data into the float array
            /*
            for (int ix = 0; ix < nx * ny; ix++) {
                    gm[ix] = rms[ix];
            }
            */
            std::copy(rms, rms + (nx * ny), gm.data());
            WriteToTiff(gm.data(), imgname.c_str(), nx, ny);

            // Clean up memory
            delete[] rms;
            if(sc_pedestals_rms[sc]) {
                delete[] sc_pedestals_rms[sc]; 
                sc_pedestals_rms[sc] = nullptr;
            }  
        }

        return NULL;
    };

    virtual void *readPedestal(const char *imgname, int nb = -1,
                               double emin = 1, double emax = 0) {

        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);
        uint32_t nnx;
        uint32_t nny;
        float *gm = ReadFromTiff(imgname, nnx, nny);
        if (ped)
            delete[] ped;
        if (nnx > (uint)nx)
            nx = nnx;
        if (nny > (uint)ny)
            ny = nny;
        ped = new double[nx * ny];

        for (int ix = 0; ix < nx * ny; ix++) {
            ped[ix] = gm[ix];
        }
        delete[] gm;

        return setPedestal();
    };

    /** sets file pointer where to write the clusters to
        \param f file pointer
        \returns current file pointer
    */
    virtual FILE *setFilePointer(FILE *f) {
        for (int i = 0; i < nThreads; i++) {
            dets[i]->setFilePointer(f);
            // dets[i]->setMutex(&fmutex);
        }
        return dets[0]->getFilePointer();
    };

    /** gets file pointer where to write the clusters to
        \returns current file pointer
    */
    virtual FILE *getFilePointer() { return dets[0]->getFilePointer(); };

  protected:
    bool stop;
    const int nThreads;
    threadedAnalogDetector *dets[MAXTHREADS];
    analogDetector<uint16_t> *dd[MAXTHREADS];
    //int ithread{0}; // Thread index
    std::vector<int> thread_counters_by_sc{}; // Round-robin counters for threads for each storage cell 
    int* image;
    int* ff;
    double* ped;
    //pthread_mutex_t fmutex; //unused
    std::unordered_map< int, std::vector<int> > sc_to_threads; // Maps storage cell -> vector of assigned thread ids
    std::mutex map_mutex; // Ensure thread-safe access to the map
    int nSC{1}; // Number of storage cells
    
    std::unordered_map<int, double*> sc_pedestals; // Store pedestal arrays per storage cell
    std::unordered_map<int, double*> sc_pedestals_rms; // Store pedestal RMS arrays per storage cell
    // at the moment, these maps could be avoided, but this implementation is more robust in allowing future changes
};

#endif
