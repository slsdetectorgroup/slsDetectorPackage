// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef INTERPOLATINGDETECTOR_H
#define INTERPOLATINGDETECTOR_H

#include "singlePhotonDetector.h"

#include "slsInterpolation.h"

//#define M015

#ifdef MYROOT1
#include <TTree.h>

#endif

#include <iostream>

using namespace std;

class interpolatingDetector : public singlePhotonDetector {

    /** @short class to perform pedestal subtraction etc. and find single photon
     * clusters for an analog detector */

  public:
    /**

       Constructor (no error checking if datasize and offsets are compatible!)
       \param d detector data structure to be used
       \param csize cluster size (should be an odd number). Defaults to 3
       \param nsigma number of rms to discriminate from the noise. Defaults to 5
       \param sign 1 if photons are positive, -1 if  negative
       \param cm common mode subtraction algorithm, if any. Defaults to NULL
       i.e. none \param nped number of samples for pedestal averaging \param nd
       number of dark frames to average as pedestals without photon
       discrimination at the beginning of the measurement


    */

    interpolatingDetector(slsDetectorData<uint16_t> *d, slsInterpolation *inte,
                          double nsigma = 5, int sign = 1,
                          commonModeSubtraction *cm = NULL, int nped = 1000,
                          int nd = 100, int nnx = -1, int nny = -1,
                          double *gm = NULL,
                          ghostSummation<uint16_t> *gs = NULL)
        : singlePhotonDetector(d, 3, nsigma, sign, cm, nped, nd, nnx, nny, gm,
                               gs),
          interp(inte), id(0) {
        // cout << "**"<< xmin << " " << xmax << " " << ymin << " " << ymax <<
        // endl;
        fi = new pthread_mutex_t;
        pthread_mutex_init(fi, NULL);
    };

    interpolatingDetector(interpolatingDetector *orig)
        : singlePhotonDetector(orig) {
        // if (orig->interp)
        //  interp=(orig->interp)->Clone();
        // else

        interp = orig->interp;

        id = orig->id;
        fi = orig->fi;
    }

    virtual interpolatingDetector *Clone() {
        return new interpolatingDetector(this);
    }

    virtual int setId(int i) {
        id = i;
        //  interp->setId(id);
        return id;
    };

    virtual void prepareInterpolation(int &ok) {
        /*  cout << "*"<< endl; */
        /* #ifdef SAVE_ALL */
        /*     char tit[1000]; */
        /*     sprintf(tit,"/scratch/ped_%d.tiff",id); */
        /*     writePedestals(tit); */
        /*     sprintf(tit,"/scratch/ped_rms_%d.tiff",id); */
        /*     writePedestalRMS(tit); */
        /*     if (gmap) { */
        /*       sprintf(tit,"/scratch/gmap_%d.tiff",id); */
        /*       writeGainMap(tit); */
        /*     } */
        /* #endif */
        if (interp) {
            pthread_mutex_lock(fi);
            interp->prepareInterpolation(ok);
            pthread_mutex_unlock(fi);
        }
    }

    void clearImage() {
        if (interp) {
            pthread_mutex_lock(fi);
            interp->clearInterpolatedImage();
            pthread_mutex_unlock(fi);
        } else
            singlePhotonDetector::clearImage();
    };

    int getImageSize(int &nnx, int &nny, int &nsx, int &nsy) {
        if (interp)
            return interp->getImageSize(nnx, nny, nsx, nsy);
        else
            return analogDetector<uint16_t>::getImageSize(nnx, nny, nsx, nsy);
    };

#ifdef MYROOT1
    virtual TH2F *getImage()
#endif
#ifndef MYROOT1
        virtual int *getImage()
#endif
    {
        // cout << "image " << endl;
        if (interp)
            return interp->getInterpolatedImage();
        else
            return analogDetector<uint16_t>::getImage();
    }

#ifdef MYROOT1
    virtual TH2F *addToInterpolatedImage(char *data, int *val, int &nph)
#endif
#ifndef MYROOT1
        virtual int *addToInterpolatedImage(char *data, int *val, int &nph)
#endif
    {
        nph = addFrame(data, val, 0);
        if (interp)
            return interp->getInterpolatedImage();
        // else
        return singlePhotonDetector::getImage();
        // return NULL;
    };

#ifdef MYROOT1
    virtual TH2F *addToFlatField(char *data, int *val, int &nph)
#endif
#ifndef MYROOT1
        virtual int *addToFlatField(char *data, int *val, int &nph)
#endif
    {
        nph = addFrame(data, val, 1);
        if (interp)
            return interp->getFlatFieldDistribution();
        else
            return NULL;
    };

    void *writeImage(const char *imgname) {
        //  cout << id << "=" << imgname<< endl;
        if (interp)
            interp->writeInterpolatedImage(imgname);
        else
            analogDetector<uint16_t>::writeImage(imgname);
        return NULL;
    }

    int addFrame(char *data, int *ph = NULL, int ff = 0) {

        singlePhotonDetector::processData(data, ph);
        int nph = 0;

        double int_x, int_y;
        double eta_x, eta_y;
        if (interp) {
            // cout << "int" << endl;
            pthread_mutex_lock(fi);
            for (nph = 0; nph < nphFrame; nph++) {
                if (ff) {
                    interp->addToFlatField(
                        (clusters + nph)->quadTot, (clusters + nph)->quad,
                        (clusters + nph)->get_cluster(), eta_x, eta_y);
                } else {
                    interp->getInterpolatedPosition(
                        (clusters + nph)->x, (clusters + nph)->y,
                        (clusters + nph)->quadTot, (clusters + nph)->quad,
                        (clusters + nph)->get_cluster(), int_x, int_y);
                    interp->addToImage(int_x, int_y);
                }
            }
            pthread_mutex_unlock(fi);
        }
        return nphFrame;
    };

    virtual void processData(char *data, int *val = NULL) {
        switch (dMode) {
        case eAnalog:
            // cout << "an" << endl;
            analogDetector<uint16_t>::processData(data, val);
            break;
        case ePhotonCounting:
            // cout << "spc" << endl;
            singlePhotonDetector::processData(data, val);
            break;
        default:
            // cout << "int" << endl;
            switch (fMode) {
            case ePedestal:
                addToPedestal(data);
                break;
            case eFlat:
                if (interp)
                    addFrame(data, val, 1);
                else
                    singlePhotonDetector::processData(data, val);
                break;
            default:
                if (interp)
                    addFrame(data, val, 0);
                else
                    singlePhotonDetector::processData(data, val);
            }
        }
    };

    virtual slsInterpolation *getInterpolation() { return interp; };

    virtual slsInterpolation *setInterpolation(slsInterpolation *ii) {
        // int ok;
        interp = ii;
        /*  pthread_mutex_lock(fi);
        if (interp)
          interp->prepareInterpolation(ok);
          pthread_mutex_unlock(fi);  */
        // cout << "det" << endl;
        return interp;
    };

    virtual void resetFlatField() {
        if (interp) {
            pthread_mutex_lock(fi);
            interp->resetFlatField();
            pthread_mutex_unlock(fi);
        }
    }

    virtual int getNSubPixels() {
        if (interp)
            return interp->getNSubPixels();
        else
            return 1;
    }

    virtual int setNSubPixels(int ns) {
        if (interp) {
            pthread_mutex_lock(fi);
            interp->getNSubPixels();
            pthread_mutex_unlock(fi);
        }
        return getNSubPixels();
    }

  protected:
    slsInterpolation *interp;
    int id;
    pthread_mutex_t *fi;
};

#endif
