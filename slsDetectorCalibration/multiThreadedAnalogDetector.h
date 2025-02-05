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
    multiThreadedAnalogDetector(analogDetector<uint16_t> *d, int n,
                                int fs = 1000)
        : stop(0), nThreads(n), ithread(0) {
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

    virtual bool pushData(char *&ptr, int SC_value) {

        // Get assigned threads for this storage cell
        std::vector<int>& assigned_threads = sc_to_threads[SC_value];

        return dets[assigned_thread]->pushData(ptr); 
    }

    virtual bool popFree(char *&ptr) {
        //  cout << ithread << endl;
        return dets[ithread]->popFree(ptr);
    }

    virtual int nextThread() {
        ithread++;
        if (ithread == nThreads)
            ithread = 0;
        return ithread;
    }

    virtual double *getPedestal() {
        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);
        if (ped)
            delete[] ped;
        ped = new double[nx * ny];
        double *p0 = new double[nx * ny];

        for (int i = 0; i < nThreads; i++) {
            // inte=(slsInterpolation*)dets[i]->getInterpolation(nb,emi,ema);
            //   cout << i << endl;
            p0 = dets[i]->getPedestal(p0);
            if (p0) {
                if (i == 0) {

                    for (int ib = 0; ib < nx * ny; ib++) {
                        ped[ib] = p0[ib] / ((double)nThreads);
                        //  cout << p0[ib] << " ";
                    }
                } else {
                    for (int ib = 0; ib < nx * ny; ib++) {
                        ped[ib] += p0[ib] / ((double)nThreads);
                        //  cout << p0[ib] << " ";
                    }
                }
            }
        }
        delete[] p0;
        return ped;
    };

    virtual double *getPedestalRMS() {
        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);
        // if (ped) delete [] ped;
        double *rms = new double[nx * ny];
        double *p0 = new double[nx * ny];

        for (int i = 0; i < nThreads; i++) {
            // inte=(slsInterpolation*)dets[i]->getInterpolation(nb,emi,ema);
            //   cout << i << endl;
            p0 = dets[i]->getPedestalRMS(p0);
            if (p0) {
                if (i == 0) {

                    for (int ib = 0; ib < nx * ny; ib++) {
                        rms[ib] = p0[ib] * p0[ib] / ((double)nThreads);
                        //  cout << p0[ib] << " ";
                    }
                } else {
                    for (int ib = 0; ib < nx * ny; ib++) {
                        rms[ib] += p0[ib] * p0[ib] / ((double)nThreads);
                        //  cout << p0[ib] << " ";
                    }
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

        return rms;
    };

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

    virtual void *writePedestal(const char *imgname) {

        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);

        getPedestal();
        float *gm = new float[nx * ny];
        if (gm) {
            for (int ix = 0; ix < nx * ny; ix++) {
                gm[ix] = ped[ix];
            }
            WriteToTiff(gm, imgname, nx, ny);
            delete[] gm;
        } else
            std::cout << "Could not allocate float image " << std::endl;

        if(ped)
            delete[] ped; //Memory cleanup

        return NULL;
    };

    virtual void *writePedestalRMS(const char *imgname) {

        int nx, ny;
        dets[0]->getDetectorSize(nx, ny);

        double *rms = getPedestalRMS();
        float *gm = new float[nx * ny];
        if (gm) {
            for (int ix = 0; ix < nx * ny; ix++) {
                gm[ix] = rms[ix];
            }
            WriteToTiff(gm, imgname, nx, ny);
            delete[] gm;
            delete[] rms;
        } else
            std::cout << "Could not allocate float image " << std::endl;

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
    int ithread;
    int* image;
    int* ff;
    double* ped;
    //pthread_mutex_t fmutex; //unused
    std::unordered_map< int, std::vector<int> > sc_to_threads; // Maps storage cell -> assigned threads
    std::mutex map_mutex; // Ensure thread safe access to the map
};

#endif
